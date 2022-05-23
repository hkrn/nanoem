/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#define NOMINMAX
#include "emapp/sdk/Encoder.h"

#include "bx/bx.h"
#include "bx/mutex.h"
#include "bx/thread.h"
#if BX_COMPILER_MSVC && BX_ARCH_32BIT
#define _InterlockedExchangeAdd64(a, b) ((*a) + (b))
#endif /* BX_COMPILER_MSVC */

#include "./jo_gif.cpp"
#include "tinystl/allocator.h"
#include "tinystl/hash_base.h"
#include "tinystl/string.h"
#include "tinystl/vector.h"

namespace {

using namespace nanoem::application::plugin;

struct GIFEncoder {
    typedef tinystl::vector<nanoem_u8_t> Frame;
    typedef tinystl::vector<Frame> FrameQueue;

    static nanoem_i32_t
    threadEntry(bx::Thread * /* thread */, void *userData)
    {
        GIFEncoder *self = static_cast<GIFEncoder *>(userData);
        nanoem_u32_t height = self->m_height;
        while (true) {
            size_t count = 0;
            FrameQueue &queue = self->m_queue;
            {
                bx::MutexScope scope(self->m_mutex);
                BX_UNUSED_1(scope);
                count = queue.size();
            }
            if (count == 0) {
                self->m_sema.wait();
            }
            Frame sourceFrame;
            {
                bx::MutexScope scope(self->m_mutex);
                BX_UNUSED_1(scope);
                sourceFrame = queue.front();
                queue.erase(queue.begin(), queue.begin() + 1);
            }
            if (sourceFrame.empty()) {
                bx::MutexScope scope(self->m_mutex);
                BX_UNUSED_1(scope);
                queue.clear();
                break;
            }
            Frame destFrame = sourceFrame;
            destFrame.resize(sourceFrame.size());
            const nanoem_u8_t *data = sourceFrame.data();
            nanoem_u8_t *mutableCopy = destFrame.data();
            nanoem_u32_t size = sourceFrame.size();
            const nanoem_u32_t stride = nanoem_u32_t(size / height);
            if (self->m_yflip) {
                const nanoem_u8_t *sourcePtr = data + stride * height - stride;
                for (nanoem_u32_t y = 0; y < height; ++y) {
                    nanoem_u8_t *destPtr = mutableCopy + y * stride;
                    memcpy(destPtr, sourcePtr, stride);
                    sourcePtr -= stride;
                }
            }
            else {
                const nanoem_u8_t *sourcePtr = data;
                for (nanoem_u32_t y = 0; y < height; ++y) {
                    nanoem_u8_t *destPtr = mutableCopy + y * stride;
                    memcpy(destPtr, sourcePtr, stride);
                    sourcePtr += stride;
                }
            }
            jo_gif_frame(&self->m_gif, mutableCopy, 0, false);
        }
        jo_gif_end(&self->m_gif);
        return 0;
    }

    GIFEncoder()
        : m_fps(0)
        , m_duration(0)
        , m_width(0)
        , m_height(0)
        , m_format(0)
        , m_yflip(0)
    {
    }
    ~GIFEncoder()
    {
    }

    int
    open(const char *filePath, nanoem_application_plugin_status_t * /* status */)
    {
        m_gif = jo_gif_start(filePath, nanoem_i16_t(m_width), nanoem_i16_t(m_height), 0, 255);
        m_thread.init(threadEntry, this);
        return 1;
    }
    void
    setOption(nanoem_u32_t key, const void *value, nanoem_u32_t size, nanoem_application_plugin_status_t *status)
    {
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_FPS: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_fps = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_DURATION: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_duration = *static_cast<const nanoem_frame_index_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_LOCATION: {
            /* unused */
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_WIDTH: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_width = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HEIGHT: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_height = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_YFLIP: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_yflip = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HDR_NUM_BITS:
            /* do nothing */
            break;
        default:
            nanoem_application_plugin_status_assign_error(
                status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION);
            break;
        }
    }
    void
    encodeAudioFrame(nanoem_frame_index_t /* currentFrameIndex */, const nanoem_u8_t * /* data */,
        nanoem_u32_t /* size */, nanoem_application_plugin_status_t * /* status */)
    {
    }
    void
    encodeVideoFrame(nanoem_frame_index_t /* currentFrameIndex */, const nanoem_u8_t *data, nanoem_u32_t size,
        nanoem_application_plugin_status_t * /* status */)
    {
        {
            bx::MutexScope scope(m_mutex);
            BX_UNUSED_1(scope);
            m_queue.push_back(Frame(data, data + size));
        }
        m_sema.post();
    }
    int
    interrupt(nanoem_application_plugin_status_t * /* status */)
    {
        {
            bx::MutexScope scope(m_mutex);
            BX_UNUSED_1(scope);
            m_queue.insert(m_queue.begin(), Frame());
        }
        m_sema.post();
        m_thread.shutdown();
        return 1;
    }
    const char *
    failureReason() const
    {
        return nullptr;
    }
    const char *
    recoverySuggestion() const
    {
        return nullptr;
    }
    int
    close(nanoem_application_plugin_status_t * /* status */)
    {
        {
            bx::MutexScope scope(m_mutex);
            BX_UNUSED_1(scope);
            m_queue.push_back(Frame());
        }
        m_sema.post();
        m_thread.shutdown();
        return 1;
    }

    jo_gif_t m_gif;
    bx::Thread m_thread;
    bx::Mutex m_mutex;
    bx::Semaphore m_sema;
    FrameQueue m_queue;
    nanoem_u32_t m_fps;
    nanoem_u32_t m_duration;
    nanoem_u32_t m_width;
    nanoem_u32_t m_height;
    nanoem_u32_t m_format;
    nanoem_u32_t m_yflip;
};

} /* namespace anonymous */

struct nanoem_application_plugin_encoder_t : GIFEncoder { };

nanoem_u32_t APIENTRY
nanoemApplicationPluginEncoderGetABIVersion()
{
    return NANOEM_APPLICATION_PLUGIN_ENCODER_ABI_VERSION;
}

void APIENTRY
nanoemApplicationPluginEncoderInitialize()
{
}

nanoem_application_plugin_encoder_t *APIENTRY
nanoemApplicationPluginEncoderCreate()
{
    nanoem_application_plugin_encoder_t *opaque = new nanoem_application_plugin_encoder_t();
    return opaque;
}

int APIENTRY
nanoemApplicationPluginEncoderOpen(
    nanoem_application_plugin_encoder_t *encoder, const char *filePath, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    int result = 0;
    if (nanoem_is_not_null(encoder)) {
        result = encoder->open(filePath, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
    return result;
}

void APIENTRY
nanoemApplicationPluginEncoderSetOption(nanoem_application_plugin_encoder_t *encoder, nanoem_u32_t key,
    const void *value, nanoem_u32_t size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(encoder)) {
        encoder->setOption(key, value, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginEncoderEncodeAudioFrame(nanoem_application_plugin_encoder_t *encoder,
    nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data, nanoem_u32_t size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(encoder)) {
        encoder->encodeAudioFrame(currentFrameIndex, data, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginEncoderEncodeVideoFrame(nanoem_application_plugin_encoder_t *encoder,
    nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data, nanoem_u32_t size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(encoder)) {
        encoder->encodeVideoFrame(currentFrameIndex, data, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginEncoderInterrupt(nanoem_application_plugin_encoder_t *encoder, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(encoder)) {
        encoder->interrupt(statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

const char *const *APIENTRY
nanoemApplicationPluginEncoderGetAllAvailableVideoFormatExtensions(
    const nanoem_application_plugin_encoder_t * /* encoder */, nanoem_u32_t *length)
{
    static const char *kFormatExtensions[] = { "gif" };
    *length = Inline::saturateInt32(sizeof(kFormatExtensions) / sizeof(kFormatExtensions[0]));
    return kFormatExtensions;
}

void APIENTRY
nanoemApplicationPluginEncoderLoadUIWindowLayout(
    nanoem_application_plugin_encoder_t * /* plugin */, nanoem_i32_t * /* status */)
{
}

void APIENTRY
nanoemApplicationPluginEncoderGetUIWindowLayoutDataSize(
    nanoem_application_plugin_encoder_t * /* plugin */, nanoem_u32_t * /* length */)
{
}

void APIENTRY
nanoemApplicationPluginEncoderGetUIWindowLayoutData(nanoem_application_plugin_encoder_t * /* plugin */,
    nanoem_u8_t * /* data */, nanoem_u32_t /* length */, nanoem_i32_t * /* status */)
{
}

void APIENTRY
nanoemApplicationPluginEncoderSetUIComponentLayoutData(nanoem_application_plugin_encoder_t * /* plugin */,
    const char * /* id */, const nanoem_u8_t * /* data */, nanoem_u32_t /* length */, int * /* reloadLayout */,
    nanoem_i32_t * /* status */)
{
}

const char *APIENTRY
nanoemApplicationPluginEncoderGetFailureReason(const nanoem_application_plugin_encoder_t *encoder)
{
    return nanoem_is_not_null(encoder) ? encoder->failureReason() : nullptr;
}

const char *APIENTRY
nanoemApplicationPluginEncoderGetRecoverySuggestion(const nanoem_application_plugin_encoder_t *encoder)
{
    return nanoem_is_not_null(encoder) ? encoder->recoverySuggestion() : nullptr;
}

int APIENTRY
nanoemApplicationPluginEncoderClose(nanoem_application_plugin_encoder_t *encoder, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    int result = 0;
    if (nanoem_is_not_null(encoder)) {
        result = encoder->close(statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
    return result;
}

void APIENTRY
nanoemApplicationPluginEncoderDestroy(nanoem_application_plugin_encoder_t *encoder)
{
    delete encoder;
}

void APIENTRY
nanoemApplicationPluginEncoderTerminate()
{
}
