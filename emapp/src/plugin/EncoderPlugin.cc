/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/plugin/EncoderPlugin.h"

#include "emapp/Error.h"
#include "emapp/IEventPublisher.h"
#include "emapp/StringUtils.h"
#include "emapp/URI.h"
#include "emapp/private/CommonInclude.h"
#include "emapp/sdk/Encoder.h"

namespace nanoem {
namespace plugin {

EncoderPlugin::EncoderPlugin(IEventPublisher *publisher)
    : BasePlugin(publisher)
    , m_encoder(nullptr)
    , m_semaphore(nullptr)
    , m_interrupted(false)
    , _encoderInitialize(nullptr)
    , _encoderCreate(nullptr)
    , _encoderOpen(nullptr)
    , _encoderSetOption(nullptr)
    , _encoderEncodeAudioFrame(nullptr)
    , _encoderEncodeVideoFrame(nullptr)
    , _encoderInterrupt(nullptr)
    , _encoderGetAllAvailableVideoFormatExtensions(nullptr)
    , _encoderLoadUIWindowLayout(nullptr)
    , _encoderGetUIWindowLayoutDataSize(nullptr)
    , _encoderGetUIWindowLayoutData(nullptr)
    , _encoderSetUIComponentLayoutData(nullptr)
    , _encoderGetFailureReason(nullptr)
    , _encoderGetRecoverySuggestion(nullptr)
    , _encoderClose(nullptr)
    , _encoderDestroy(nullptr)
    , _encoderTerminate(nullptr)
{
}

EncoderPlugin::~EncoderPlugin() NANOEM_DECL_NOEXCEPT
{
    unload();
}

bool
EncoderPlugin::load(const URI &fileURI)
{
#if defined(NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN)
    BX_UNUSED_1(fileURI);
    bool succeeded = true;
    _encoderInitialize = nanoemApplicationPluginEncoderInitialize;
    _encoderCreate = nanoemApplicationPluginEncoderCreate;
    _encoderOpen = nanoemApplicationPluginEncoderOpen;
    _encoderSetOption = nanoemApplicationPluginEncoderSetOption;
    _encoderEncodeAudioFrame = nanoemApplicationPluginEncoderEncodeAudioFrame;
    _encoderEncodeVideoFrame = nanoemApplicationPluginEncoderEncodeVideoFrame;
    _encoderInterrupt = nanoemApplicationPluginEncoderInterrupt;
    _encoderGetAllAvailableVideoFormatExtensions = nanoemApplicationPluginEncoderGetAllAvailableVideoFormatExtensions;
    _encoderGetFailureReason = nanoemApplicationPluginEncoderGetFailureReason;
    _encoderGetRecoverySuggestion = nanoemApplicationPluginEncoderGetRecoverySuggestion;
    _encoderClose = nanoemApplicationPluginEncoderClose;
    _encoderDestroy = nanoemApplicationPluginEncoderDestroy;
    _encoderTerminate = nanoemApplicationPluginEncoderTerminate;
#else /* NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN */
    bool succeeded = m_handle != nullptr;
    if (!succeeded) {
        bool valid = true;
        if (void *handle = bx::dlopen(fileURI.absolutePathConstString())) {
            PFN_nanoemApplicationPluginEncoderGetABIVersion _encoderGetABIVersion = nullptr;
            EMLOG_DEBUG("Loading encoder plugin path={} handle={}", fileURI.absolutePathConstString(), handle);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginEncoderGetABIVersion", _encoderGetABIVersion, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginEncoderInitialize", _encoderInitialize, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginEncoderCreate", _encoderCreate, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginEncoderOpen", _encoderOpen, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginEncoderSetOption", _encoderSetOption, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginEncoderEncodeAudioFrame", _encoderEncodeAudioFrame, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginEncoderEncodeVideoFrame", _encoderEncodeVideoFrame, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginEncoderInterrupt", _encoderInterrupt, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginEncoderGetAllAvailableVideoFormatExtensions",
                _encoderGetAllAvailableVideoFormatExtensions, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginEncoderGetFailureReason", _encoderGetFailureReason, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginEncoderGetRecoverySuggestion", _encoderGetRecoverySuggestion, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginEncoderClose", _encoderClose, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginEncoderDestroy", _encoderDestroy, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginEncoderTerminate", _encoderTerminate, valid);
            if (valid &&
                isABICompatible(_encoderGetABIVersion(), NANOEM_APPLICATION_PLUGIN_ENCODER_ABI_VERSION_MAJOR)) {
                Inline::resolveSymbol(
                    handle, "nanoemApplicationPluginEncoderLoadUIWindowLayout", _encoderLoadUIWindowLayout, valid);
                Inline::resolveSymbol(handle, "nanoemApplicationPluginEncoderGetUIWindowLayoutDataSize",
                    _encoderGetUIWindowLayoutDataSize, valid);
                Inline::resolveSymbol(handle, "nanoemApplicationPluginEncoderGetUIWindowLayoutData",
                    _encoderGetUIWindowLayoutData, valid);
                Inline::resolveSymbol(handle, "nanoemApplicationPluginEncoderSetUIComponentLayoutData",
                    _encoderSetUIComponentLayoutData, valid);
                m_handle = handle;
                m_name = fileURI.lastPathComponent();
                _encoderInitialize();
                succeeded = true;
            }
            else {
                _encoderTerminate = nullptr;
                bx::dlclose(handle);
            }
        }
    }
#endif /* NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN */
    return succeeded;
}

void
EncoderPlugin::unload()
{
    if (_encoderTerminate) {
        _encoderTerminate();
        _encoderTerminate = nullptr;
    }
#if !defined(NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN)
    if (m_handle) {
        bx::dlclose(m_handle);
        m_handle = nullptr;
    }
#endif /* NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN */
}

bool
EncoderPlugin::create()
{
    if (!m_encoder) {
        m_encoder = _encoderCreate();
    }
    nanoem_delete(m_semaphore);
    m_semaphore = nanoem_new(bx::Semaphore);
    return m_encoder != nullptr;
}

void
EncoderPlugin::destroy()
{
    if (_encoderDestroy) {
        _encoderDestroy(m_encoder);
        m_encoder = nullptr;
    }
    nanoem_delete_safe(m_semaphore);
}

bool
EncoderPlugin::open(const URI &fileURI, Error &error)
{
    int status = 0, ret = _encoderOpen(m_encoder, fileURI.absolutePathConstString(), &status);
    handlePluginStatus(status, error);
    return ret != 0;
}

void
EncoderPlugin::setOption(nanoem_u32_t key, int value, Error &error)
{
    const int v = value;
    int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    _encoderSetOption(m_encoder, key, &v, Inline::saturateInt32U(sizeof(v)), &status);
    handlePluginStatus(status, error);
}

void
EncoderPlugin::setOption(nanoem_u32_t key, nanoem_u32_t value, Error &error)
{
    const nanoem_u32_t v = value;
    int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    _encoderSetOption(m_encoder, key, &v, Inline::saturateInt32U(sizeof(v)), &status);
    handlePluginStatus(status, error);
}

void
EncoderPlugin::setOption(nanoem_u32_t key, const char *value, Error &error)
{
    int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    _encoderSetOption(m_encoder, key, value, value ? Inline::saturateInt32U(StringUtils::length(value)) : 0, &status);
    handlePluginStatus(status, error);
}

bool
EncoderPlugin::encodeAudioFrame(
    nanoem_frame_index_t currentLocalFrameIndex, const nanoem_u8_t *data, size_t size, Error &error)
{
    bool succeeded = true;
    if (!m_interrupted) {
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        _encoderEncodeAudioFrame(m_encoder, currentLocalFrameIndex, data, Inline::saturateInt32U(size), &status);
        handlePluginStatus(status, error);
        succeeded = status == NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    }
    return succeeded;
}

bool
EncoderPlugin::encodeVideoFrame(
    nanoem_frame_index_t currentLocalFrameIndex, const nanoem_u8_t *data, size_t size, Error &error)
{
    bool succeeded = true;
    if (!m_interrupted) {
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        _encoderEncodeVideoFrame(m_encoder, currentLocalFrameIndex, data, Inline::saturateInt32U(size), &status);
        handlePluginStatus(status, error);
        succeeded = status == NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    }
    return succeeded;
}

void
EncoderPlugin::interrupt()
{
    int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    _encoderInterrupt(m_encoder, &status);
    m_interrupted = true;
}

void
EncoderPlugin::getAvailableVideoFormatExtensions(StringList &formatExtensionList) const
{
    nanoem_u32_t length;
    if (const char *const *allFormats = _encoderGetAllAvailableVideoFormatExtensions(m_encoder, &length)) {
        formatExtensionList.clear();
        for (nanoem_u32_t i = 0; i < length; i++) {
            formatExtensionList.push_back(allFormats[i]);
        }
    }
}

void
EncoderPlugin::getUIWindowLayout(ByteArray &bytes, Error &error)
{
    if (_encoderLoadUIWindowLayout && _encoderGetUIWindowLayoutDataSize && _encoderGetUIWindowLayoutData) {
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        _encoderLoadUIWindowLayout(m_encoder, &status);
        if (status == NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS) {
            nanoem_u32_t size = 0;
            _encoderGetUIWindowLayoutDataSize(m_encoder, &size);
            if (size > 0) {
                bytes.resize(size);
                _encoderGetUIWindowLayoutData(m_encoder, bytes.data(), size, &status);
                handlePluginStatus(status, error);
            }
        }
        else {
            handlePluginStatus(status, error);
        }
    }
}

void
EncoderPlugin::setUIComponentLayout(const char *id, const ByteArray &bytes, bool &reloadLayout, Error &error)
{
    if (_encoderSetUIComponentLayoutData) {
        int status = 0, reload = 0;
        _encoderSetUIComponentLayoutData(
            m_encoder, id, bytes.data(), Inline::saturateInt32U(bytes.size()), &reload, &status);
        handlePluginStatus(status, error);
        reloadLayout = reload != 0;
    }
}

const char *
EncoderPlugin::failureReason() const NANOEM_DECL_NOEXCEPT
{
    const char *ptr = _encoderGetFailureReason(m_encoder);
    return ptr ? ptr : "";
}

const char *
EncoderPlugin::recoverySuggestion() const NANOEM_DECL_NOEXCEPT
{
    const char *ptr = _encoderGetRecoverySuggestion(m_encoder);
    return ptr ? ptr : "";
}

String
EncoderPlugin::name() const
{
    return m_name;
}

void
EncoderPlugin::close(Error &error)
{
    if (m_interrupted) {
        m_interrupted = false;
    }
    else if (_encoderClose) {
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        _encoderClose(m_encoder, &status);
        handlePluginStatus(status, error);
    }
    if (m_semaphore) {
        m_semaphore->post();
    }
}

void
EncoderPlugin::wait()
{
    if (m_semaphore) {
        m_semaphore->wait();
        nanoem_delete_safe(m_semaphore);
    }
}

bool
EncoderPlugin::isInterrupted() const NANOEM_DECL_NOEXCEPT
{
    return m_interrupted;
}

} /* namespace plugin */
} /* namespace nanoem */
