/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/plugin/DecoderPlugin.h"

#include "emapp/Error.h"
#include "emapp/IEventPublisher.h"
#include "emapp/StringUtils.h"
#include "emapp/URI.h"
#include "emapp/internal/Stub.h"
#include "emapp/private/CommonInclude.h"
#include "emapp/sdk/Decoder.h"

#include "bx/os.h"
#if BX_CONFIG_SUPPORTS_THREADING
#include "bx/thread.h"
#else
#include "emapp/internal/Stub.h"
#endif /* BX_CONFIG_SUPPORTS_THREADING */

namespace nanoem {
namespace plugin {

DecoderPlugin::DecoderPlugin(IEventPublisher *publisher)
    : BasePlugin(publisher)
    , m_decoder(nullptr)
    , m_interrupted(false)
    , _decoderInitialize(nullptr)
    , _decoderCreate(nullptr)
    , _decoderOpen(nullptr)
    , _decoderSetOption(nullptr)
    , _decoderGetAudioFormatValue(nullptr)
    , _decoderGetVideoFormatValue(nullptr)
    , _decoderDecodeAudioFrame(nullptr)
    , _decoderDecodeVideoFrame(nullptr)
    , _decoderDestroyDecodedAudioFrame(nullptr)
    , _decoderDestroyDecodedVideoFrame(nullptr)
    , _decoderGetAllAvailableAudioFormatExtensions(nullptr)
    , _decoderGetAllAvailableVideoFormatExtensions(nullptr)
    , _decoderGetFailureReason(nullptr)
    , _decoderGetRecoverySuggestion(nullptr)
    , _decoderClose(nullptr)
    , _decoderDestroy(nullptr)
    , _decoderTerminate(nullptr)
{
}

DecoderPlugin::~DecoderPlugin() NANOEM_DECL_NOEXCEPT
{
    unload();
}

bool
DecoderPlugin::load(const URI &fileURI)
{
#if defined(NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN)
    BX_UNUSED_1(fileURI);
    bool succeeded = true;
#else /* NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN */
    bool succeeded = m_handle != nullptr;
    if (!succeeded) {
        bool valid = true;
        if (void *handle = bx::dlopen(fileURI.absolutePathConstString())) {
            PFN_nanoemApplicationPluginDecoderGetABIVersion _decoderGetABIVersion = nullptr;
            EMLOG_DEBUG("Loading decoder plugin path={} handle={}", fileURI.absolutePathConstString(), handle);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginDecoderGetABIVersion", _decoderGetABIVersion, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginDecoderInitialize", _decoderInitialize, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginDecoderCreate", _decoderCreate, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginDecoderOpen", _decoderOpen, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginDecoderSetOption", _decoderSetOption, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginDecoderGetAudioFormatValue", _decoderGetAudioFormatValue, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginDecoderGetVideoFormatValue", _decoderGetVideoFormatValue, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginDecoderDecodeAudioFrame", _decoderDecodeAudioFrame, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginDecoderDecodeVideoFrame", _decoderDecodeVideoFrame, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginDecoderDestroyAudioFrame", _decoderDestroyDecodedAudioFrame, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginDecoderDestroyVideoFrame", _decoderDestroyDecodedVideoFrame, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginDecoderGetAllAvailableAudioFormatExtensions",
                _decoderGetAllAvailableAudioFormatExtensions, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginDecoderGetAllAvailableVideoFormatExtensions",
                _decoderGetAllAvailableVideoFormatExtensions, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginDecoderGetFailureReason", _decoderGetFailureReason, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginDecoderGetRecoverySuggestion", _decoderGetRecoverySuggestion, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginDecoderClose", _decoderClose, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginDecoderDestroy", _decoderDestroy, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginDecoderTerminate", _decoderTerminate, valid);
            if (valid &&
                isABICompatible(_decoderGetABIVersion(), NANOEM_APPLICATION_PLUGIN_DECODER_ABI_VERSION_MAJOR)) {
                m_handle = handle;
                m_name = fileURI.lastPathComponent();
                _decoderInitialize();
                succeeded = true;
            }
            else {
                _decoderTerminate = nullptr;
                bx::dlclose(handle);
            }
        }
    }
#endif /* NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN */
    return succeeded;
}

void
DecoderPlugin::unload()
{
    if (_decoderTerminate) {
        _decoderTerminate();
        _decoderTerminate = nullptr;
    }
#if !defined(NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN)
    if (m_handle) {
        bx::dlclose(m_handle);
        m_handle = nullptr;
    }
#endif /* NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN */
}

bool
DecoderPlugin::create()
{
    if (!m_decoder && _decoderCreate) {
        m_decoder = _decoderCreate();
    }
    return m_decoder != nullptr;
}

void
DecoderPlugin::destroy()
{
    if (_decoderDestroy) {
        _decoderDestroy(m_decoder);
        m_decoder = nullptr;
    }
}

bool
DecoderPlugin::open(const URI &fileURI, Error &error)
{
    int status = 0, ret = _decoderOpen(m_decoder, fileURI.absolutePathConstString(), &status);
    handlePluginStatus(status, error);
    return ret != 0;
}

void
DecoderPlugin::setOption(nanoem_u32_t key, int value, Error &error)
{
    const int v = value;
    int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    _decoderSetOption(m_decoder, key, &v, Inline::saturateInt32U(sizeof(v)), &status);
    handlePluginStatus(status, error);
}

void
DecoderPlugin::setOption(nanoem_u32_t key, nanoem_u32_t value, Error &error)
{
    const nanoem_u32_t v = value;
    int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    _decoderSetOption(m_decoder, key, &v, Inline::saturateInt32U(sizeof(v)), &status);
    handlePluginStatus(status, error);
}

void
DecoderPlugin::setOption(nanoem_u32_t key, const char *value, Error &error)
{
    int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    _decoderSetOption(m_decoder, key, value, value ? StringUtils::length(value) : 0, &status);
    handlePluginStatus(status, error);
}

nanoem_frame_index_t
DecoderPlugin::audioDuration() const NANOEM_DECL_NOEXCEPT
{
    nanoem_frame_index_t value = 0;
    nanoem_u32_t size = 0;
    int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    _decoderGetAudioFormatValue(
        m_decoder, NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_DURATION, &value, &size, &status);
    nanoem_assert(size == sizeof(value), "must be same");
    return value;
}

nanoem_frame_index_t
DecoderPlugin::videoDuration() const NANOEM_DECL_NOEXCEPT
{
    nanoem_frame_index_t value = 0;
    nanoem_u32_t size = 0;
    int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    _decoderGetVideoFormatValue(
        m_decoder, NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_DURATION, &value, &size, &status);
    nanoem_assert(size == sizeof(value), "must be same");
    return value;
}

nanoem_u32_t
DecoderPlugin::audioFormatValue(nanoem_u32_t key) const NANOEM_DECL_NOEXCEPT
{
    nanoem_u32_t value = 0;
    nanoem_u32_t size = 0;
    int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    _decoderGetAudioFormatValue(m_decoder, key, &value, &size, &status);
    nanoem_assert(size == sizeof(value), "must be same");
    return value;
}

nanoem_u32_t
DecoderPlugin::videoFormatValue(nanoem_u32_t key) const NANOEM_DECL_NOEXCEPT
{
    nanoem_u32_t value = 0;
    nanoem_u32_t size = 0;
    int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    _decoderGetVideoFormatValue(m_decoder, key, &value, &size, &status);
    nanoem_assert(size == sizeof(value), "must be same");
    return value;
}

bool
DecoderPlugin::decodeAudioFrame(nanoem_frame_index_t currentLocalFrameIndex, ByteArray &bytes, Error &error)
{
    bool succeeded = true;
    if (!m_interrupted) {
        nanoem_u8_t *ptr = nullptr;
        nanoem_u32_t size = 0;
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        _decoderDecodeAudioFrame(m_decoder, currentLocalFrameIndex, &ptr, &size, &status);
        succeeded = status == NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        if (succeeded) {
            bytes.resize(size);
            memcpy(bytes.data(), ptr, size);
        }
        else {
            handlePluginStatus(status, error);
        }
        _decoderDestroyDecodedAudioFrame(m_decoder, currentLocalFrameIndex, ptr, size);
    }
    return succeeded;
}

bool
DecoderPlugin::decodeVideoFrame(nanoem_frame_index_t currentLocalFrameIndex, ByteArray &bytes, Error &error)
{
    bool succeeded = true;
    if (!m_interrupted) {
        nanoem_u8_t *ptr = nullptr;
        nanoem_u32_t size = 0;
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        _decoderDecodeVideoFrame(m_decoder, currentLocalFrameIndex, &ptr, &size, &status);
        succeeded = status == NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        if (succeeded) {
            bytes.resize(size);
            memcpy(bytes.data(), ptr, size);
        }
        else {
            handlePluginStatus(status, error);
        }
        _decoderDestroyDecodedVideoFrame(m_decoder, currentLocalFrameIndex, ptr, size);
    }
    return succeeded;
}

void
DecoderPlugin::getAvailableAudioFormatExtensions(StringList &formatExtensionList) const
{
    nanoem_u32_t length;
    if (const char *const *allFormats = _decoderGetAllAvailableAudioFormatExtensions(m_decoder, &length)) {
        formatExtensionList.clear();
        for (nanoem_u32_t i = 0; i < length; i++) {
            formatExtensionList.push_back(allFormats[i]);
        }
    }
}

void
DecoderPlugin::getAvailableVideoFormatExtensions(StringList &formatExtensionList) const
{
    nanoem_u32_t length;
    if (const char *const *allFormats = _decoderGetAllAvailableVideoFormatExtensions(m_decoder, &length)) {
        formatExtensionList.clear();
        for (nanoem_u32_t i = 0; i < length; i++) {
            formatExtensionList.push_back(allFormats[i]);
        }
    }
}

void
DecoderPlugin::close(Error &error)
{
    if (m_interrupted) {
        m_interrupted = false;
    }
    else if (_decoderClose) {
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        _decoderClose(m_decoder, &status);
        handlePluginStatus(status, error);
        m_decoder = nullptr;
    }
}

const char *
DecoderPlugin::failureReason() const NANOEM_DECL_NOEXCEPT
{
    const char *ptr = _decoderGetFailureReason(m_decoder);
    return ptr ? ptr : "";
}

const char *
DecoderPlugin::recoverySuggestion() const NANOEM_DECL_NOEXCEPT
{
    const char *ptr = _decoderGetRecoverySuggestion(m_decoder);
    return ptr ? ptr : "";
}

} /* namespace plugin */
} /* namespace nanoem */
