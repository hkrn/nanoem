/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */
#include <stdio.h>

#define NOMINMAX
#include "emapp/sdk/Encoder.h"

#include "tinystl/allocator.h"
#include "tinystl/hash_base.h"
#include "tinystl/string.h"
#include "tinystl/vector.h"

#define BX_CONFIG_ALLOCATOR_CRT 1
#include "bx/endian.h"
#include "bx/readerwriter.h"
#include "bx/string.h"

#define STBI_WRITE_NO_STDIO
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "zip.h"

struct nanoem_application_plugin_encoder_t {
    static void
    writeData(void *context, void *data, int size)
    {
        bx::WriterI *writer = static_cast<bx::WriterI *>(context);
        bx::write(writer, data, size);
    }

    nanoem_application_plugin_encoder_t()
        : m_zip(0)
        , m_fps(0)
        , m_duration(0)
        , m_width(0)
        , m_height(0)
        , m_format(0)
        , m_yflip(0)
        , m_hdr(0)
    {
    }
    ~nanoem_application_plugin_encoder_t()
    {
    }

    int
    open()
    {
        m_zip = zipOpen64(m_location.c_str(), APPEND_STATUS_CREATE);
        return m_zip ? NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS : NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
    int
    setOption(nanoem_u32_t key, const void *value, nanoem_rsize_t /* size */)
    {
        int ret = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_FPS:
            m_fps = *static_cast<const nanoem_u32_t *>(value);
            break;
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_DURATION:
            m_duration = *static_cast<const nanoem_frame_index_t *>(value);
            break;
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_LOCATION:
            m_location = static_cast<const char *>(value);
            break;
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_FREQUENCY:
            m_numFrequency = *static_cast<const nanoem_u32_t *>(value);
            break;
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_CHANNELS:
            m_numChannels = *static_cast<const nanoem_u32_t *>(value);
            break;
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_BITS:
            m_numBits = *static_cast<const nanoem_u32_t *>(value);
            break;
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_WIDTH:
            m_width = *static_cast<const nanoem_u32_t *>(value);
            break;
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HEIGHT:
            m_height = *static_cast<const nanoem_u32_t *>(value);
            break;
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_YFLIP:
            m_yflip = *static_cast<const nanoem_u32_t *>(value);
            break;
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HDR_NUM_BITS:
            m_hdr = *static_cast<const nanoem_u32_t *>(value);
            break;
        default:
            ret = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION;
            break;
        }
        return ret;
    }
    int
    encodeAudioFrame(nanoem_frame_index_t /* current_frame_index */, const nanoem_u8_t *data, nanoem_rsize_t size)
    {
        m_audioData.insert(m_audioData.end(), data, data + size);
        return NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    }
    int
    encodeVideoFrame(nanoem_frame_index_t current_frame_index, const nanoem_u8_t *data, nanoem_rsize_t size)
    {
        bx::DefaultAllocator allocator;
        bx::MemoryBlock imageBlock(&allocator);
        bx::MemoryWriter imageWriter(&imageBlock);
        bx::Error error;
        char filename[32];
        bx::snprintf(filename, sizeof(filename), "frames/%08d.bmp", current_frame_index);
        const nanoem_u8_t *sourceData = static_cast<const nanoem_u8_t *>(data);
        tinystl::vector<nanoem_u8_t> destData(size);
        const nanoem_u32_t stride = nanoem_u32_t(size / m_height);
        if (m_yflip) {
            const nanoem_u8_t *sourcePtr = data + stride * m_height - stride;
            for (nanoem_u32_t y = 0; y < m_height; ++y) {
                nanoem_u8_t *destPtr = destData.data() + y * stride;
                memcpy(destPtr, sourcePtr, stride);
                for (nanoem_u32_t x = 0; x < m_width; x++) {
                    const nanoem_u8_t *p = &sourcePtr[x * 4];
                    nanoem_u8_t *mp = &destPtr[x * 4];
                    nanoem_u8_t t = p[0];
                    mp[0] = p[2];
                    mp[2] = t;
                }
                sourcePtr -= stride;
            }
        }
        else {
            const nanoem_u8_t *sourcePtr = data;
            for (nanoem_u32_t y = 0; y < m_height; ++y) {
                nanoem_u8_t *destPtr = destData.data() + y * stride;
                memcpy(destPtr, sourcePtr, stride);
                for (nanoem_u32_t x = 0; x < m_width; x++) {
                    const nanoem_u8_t *p = &data[x * 4];
                    nanoem_u8_t *mp = &destPtr[x * 4];
                    nanoem_u8_t t = p[0];
                    mp[0] = p[2];
                    mp[2] = t;
                }
                sourcePtr += stride;
            }
        }
        if (stbi_write_bmp_to_func(writeData, &imageWriter, m_width, m_height, 4, destData.data()) == 0) {
            return NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON;
        }
        int rc = ZIP_OK;
        zip_fileinfo info;
        memset(&info, 0, sizeof(info));
        if ((rc = zipOpenNewFileInZip64(m_zip, filename, &info, NULL, 0, NULL, 0, NULL, 0, Z_BEST_SPEED, 1)) !=
            ZIP_OK) {
            return NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON;
        }
        if ((rc = zipWriteInFileInZip(m_zip, imageBlock.more(0), imageBlock.getSize())) != ZIP_OK) {
            return NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON;
        }
        if ((rc = zipCloseFileInZip(m_zip)) != ZIP_OK) {
            return NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON;
        }
        return NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    }
    int
    interrupt()
    {
        return close();
    }
    const char *
    failureReason() const
    {
        return m_reason.c_str();
    }
    const char *
    recoverySuggestion() const
    {
        return NULL;
    }
    int
    close()
    {
        int rc = ZIP_OK;
        if (!m_audioData.empty()) {
            zip_fileinfo info;
            memset(&info, 0, sizeof(info));
            writeAudioHeader();
            if ((rc = zipOpenNewFileInZip64(m_zip, "audio.wav", &info, NULL, 0, NULL, 0, NULL, 0, Z_BEST_SPEED, 1)) !=
                ZIP_OK) {
                return NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON;
            }
            if ((rc = zipWriteInFileInZip(m_zip, m_audioData.data(), nanoem_u32_t(m_audioData.size()))) != ZIP_OK) {
                return NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON;
            }
            if ((rc = zipCloseFileInZip(m_zip)) != ZIP_OK) {
                return NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON;
            }
            m_audioData.clear();
        }
        rc = zipClose(m_zip, NULL);
        return rc == ZIP_OK ? NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS
                            : NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON;
    }

    void
    writeAudioHeader()
    {
        struct wave_header_t {
            nanoem_u32_t chunkID;
            nanoem_u32_t chunkSize;
            nanoem_u32_t format;
            nanoem_u32_t subchunk1ID;
            nanoem_u32_t subchunk1Size;
            nanoem_u16_t audioFormat;
            nanoem_u16_t numChannels;
            nanoem_u32_t sampleRate;
            nanoem_u32_t byteRate;
            nanoem_u16_t blockAlign;
            nanoem_u16_t bitsPerSample;
            nanoem_u32_t subchunk2ID;
            nanoem_u32_t subchunk2Size;
        } header;
        memset(&header, 0, sizeof(header));
        const nanoem_u16_t blockAlign = nanoem_u16_t((m_numBits / 8) * m_numChannels);
        header.chunkID = nanoem_fourcc('R', 'I', 'F', 'F');
        header.format = nanoem_fourcc('W', 'A', 'V', 'E');
        header.subchunk1ID = nanoem_fourcc('f', 'm', 't', ' ');
        header.subchunk1Size = 16;
        header.audioFormat = 1;
        header.numChannels = nanoem_u16_t(m_numChannels);
        header.sampleRate = m_numFrequency;
        header.byteRate = blockAlign * m_numFrequency;
        header.blockAlign = blockAlign;
        header.bitsPerSample = nanoem_u16_t(m_numBits);
        header.subchunk2ID = nanoem_fourcc('d', 'a', 't', 'a');
        const nanoem_u8_t *headerPtr = reinterpret_cast<const nanoem_u8_t *>(&header);
        m_audioData.insert(m_audioData.begin(), headerPtr, headerPtr + sizeof(header));
    }

    zipFile m_zip;
    tinystl::vector<nanoem_u8_t> m_audioData;
    tinystl::string m_location;
    tinystl::string m_reason;
    nanoem_u32_t m_fps;
    nanoem_u32_t m_duration;
    nanoem_u32_t m_numFrequency;
    nanoem_u32_t m_numChannels;
    nanoem_u32_t m_numBits;
    nanoem_u32_t m_width;
    nanoem_u32_t m_height;
    nanoem_u32_t m_format;
    nanoem_u32_t m_yflip;
    nanoem_u32_t m_hdr;
};

nanoem_u32_t
nanoemApplicationPluginEncoderGetABIVersion()
{
    return NANOEM_APPLICATION_PLUGIN_ENCODER_ABI_VERSION;
}

nanoem_u32_t
nanoemApplicationPluginEncoderGetAPIVersion()
{
    return NANOEM_APPLICATION_PLUGIN_ENCODER_API_VERSION;
}

void APIENTRY
nanoemApplicationPluginEncoderInitialize(void)
{
}

nanoem_application_plugin_encoder_t *APIENTRY
nanoemApplicationPluginEncoderCreate()
{
    nanoem_application_plugin_encoder_t *opaque = new nanoem_application_plugin_encoder_t();
    return opaque;
}

int APIENTRY
nanoemApplicationPluginEncoderOpen(nanoem_application_plugin_encoder_t *encoder)
{
    return nanoem_is_not_null(encoder) ? encoder->open() : NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
}

int APIENTRY
nanoemApplicationPluginEncoderSetOption(
    nanoem_application_plugin_encoder_t *encoder, nanoem_u32_t key, const void *value, nanoem_rsize_t size)
{
    return nanoem_is_not_null(encoder) ? encoder->setOption(key, value, size)
                                       : NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
}

int APIENTRY
nanoemApplicationPluginEncoderEncodeAudioFrame(nanoem_application_plugin_encoder_t *encoder,
    nanoem_frame_index_t current_frame_index, const nanoem_u8_t *data, nanoem_rsize_t size)
{
    return nanoem_is_not_null(encoder) ? encoder->encodeAudioFrame(current_frame_index, data, size)
                                       : NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
}

int APIENTRY
nanoemApplicationPluginEncoderEncodeVideoFrame(nanoem_application_plugin_encoder_t *encoder,
    nanoem_frame_index_t current_frame_index, const nanoem_u8_t *data, nanoem_rsize_t size)
{
    return nanoem_is_not_null(encoder) ? encoder->encodeVideoFrame(current_frame_index, data, size)
                                       : NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
}

int APIENTRY
nanoemApplicationPluginEncoderInterrupt(nanoem_application_plugin_encoder_t *encoder)
{
    return nanoem_is_not_null(encoder) ? encoder->interrupt() : NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
}

const char *const *APIENTRY
nanoemApplicationPluginEncoderGetAllAvailableVideoFormatExtensions(
    const nanoem_application_plugin_encoder_t * /* encoder */, nanoem_rsize_t *length)
{
    static const char *kFormatExtensions[] = { "zip" };
    *length = sizeof(kFormatExtensions) / sizeof(kFormatExtensions[0]);
    return kFormatExtensions;
}

const char *APIENTRY
nanoemApplicationPluginEncoderGetFailureReason(const nanoem_application_plugin_encoder_t *encoder)
{
    return nanoem_is_not_null(encoder) ? encoder->failureReason() : NULL;
}

const char *APIENTRY
nanoemApplicationPluginEncoderGetRecoverySuggestion(const nanoem_application_plugin_encoder_t *encoder)
{
    return nanoem_is_not_null(encoder) ? encoder->recoverySuggestion() : NULL;
}

int APIENTRY
nanoemApplicationPluginEncoderClose(nanoem_application_plugin_encoder_t *encoder)
{
    return nanoem_is_not_null(encoder) ? encoder->close() : NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
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
