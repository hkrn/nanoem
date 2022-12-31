/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_PLUGIN_ENCODERPLUGIN_H_
#define NANOEM_EMAPP_PLUGIN_ENCODERPLUGIN_H_

#include "emapp/plugin/BasePlugin.h"

#include "bx/os.h"
#if BX_CONFIG_SUPPORTS_THREADING
#include "bx/thread.h"
#else
#include "emapp/internal/Stub.h"
#endif /* BX_CONFIG_SUPPORTS_THREADING */

struct nanoem_application_plugin_encoder_t;

namespace nanoem {
namespace plugin {

class EncoderPlugin NANOEM_DECL_SEALED : public BasePlugin {
public:
    EncoderPlugin(IEventPublisher *publisher);
    ~EncoderPlugin() NANOEM_DECL_NOEXCEPT;

    bool load(const URI &fileURI) NANOEM_DECL_OVERRIDE;
    void unload() NANOEM_DECL_OVERRIDE;
    bool create() NANOEM_DECL_OVERRIDE;
    void destroy() NANOEM_DECL_OVERRIDE;

    bool open(const URI &fileURI, Error &error);
    void setOption(nanoem_u32_t key, int value, Error &error);
    void setOption(nanoem_u32_t key, nanoem_u32_t value, Error &error);
    void setOption(nanoem_u32_t key, const char *value, Error &error);
    bool encodeAudioFrame(
        nanoem_frame_index_t currentLocalFrameIndex, const nanoem_u8_t *data, size_t size, Error &error);
    bool encodeVideoFrame(
        nanoem_frame_index_t currentLocalFrameIndex, const nanoem_u8_t *data, size_t size, Error &error);
    void interrupt();
    void getAvailableVideoFormatExtensions(StringList &formatExtensionList) const;
    void getUIWindowLayout(ByteArray &bytes, Error &error);
    void setUIComponentLayout(const char *id, const ByteArray &bytes, bool &reloadLayout, Error &error);
    void close(Error &error);
    void wait();
    bool isInterrupted() const NANOEM_DECL_NOEXCEPT;

    const char *failureReason() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const char *recoverySuggestion() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    String name() const;

private:
    typedef nanoem_u32_t(APIENTRY *PFN_nanoemApplicationPluginEncoderGetABIVersion)();
    typedef nanoem_u32_t(APIENTRY *PFN_nanoemApplicationPluginEncoderGetAPIVersion)();
    typedef void(APIENTRY *PFN_nanoemApplicationPluginEncoderInitialize)();
    typedef nanoem_application_plugin_encoder_t *(APIENTRY *PFN_nanoemApplicationPluginEncoderCreate)();
    typedef int(APIENTRY *PFN_nanoemApplicationPluginEncoderOpen)(
        nanoem_application_plugin_encoder_t *, const char *, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginEncoderSetOption)(
        nanoem_application_plugin_encoder_t *, nanoem_u32_t, const void *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginEncoderEncodeAudioFrame)(
        nanoem_application_plugin_encoder_t *, nanoem_frame_index_t, const nanoem_u8_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginEncoderEncodeVideoFrame)(
        nanoem_application_plugin_encoder_t *, nanoem_frame_index_t, const nanoem_u8_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginEncoderInterrupt)(nanoem_application_plugin_encoder_t *, int *);
    typedef const char *const *(APIENTRY *PFN_nanoemApplicationPluginEncoderGetAllAvailableVideoFormats)(
        const nanoem_application_plugin_encoder_t *, nanoem_u32_t *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginEncoderLoadUIWindowLayout)(
        nanoem_application_plugin_encoder_t *, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginEncoderGetUIWindowLayoutDataSize)(
        nanoem_application_plugin_encoder_t *, nanoem_u32_t *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginEncoderGetUIWindowLayoutData)(
        nanoem_application_plugin_encoder_t *, nanoem_u8_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginEncoderSetUIComponentLayoutData)(
        nanoem_application_plugin_encoder_t *, const char *, const nanoem_u8_t *, nanoem_u32_t, int *, int *);
    typedef const char *(APIENTRY *PFN_nanoemApplicationPluginEncoderGetFailureReason)(
        const nanoem_application_plugin_encoder_t *);
    typedef const char *(APIENTRY *PFN_nanoemApplicationPluginEncoderGetRecoverySuggestion)(
        const nanoem_application_plugin_encoder_t *);
    typedef int(APIENTRY *PFN_nanoemApplicationPluginEncoderClose)(nanoem_application_plugin_encoder_t *, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginEncoderDestroy)(nanoem_application_plugin_encoder_t *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginEncoderTerminate)();

    nanoem_application_plugin_encoder_t *m_encoder;
    bx::Semaphore *m_semaphore;
    bool m_interrupted;
    PFN_nanoemApplicationPluginEncoderInitialize _encoderInitialize;
    PFN_nanoemApplicationPluginEncoderCreate _encoderCreate;
    PFN_nanoemApplicationPluginEncoderOpen _encoderOpen;
    PFN_nanoemApplicationPluginEncoderSetOption _encoderSetOption;
    PFN_nanoemApplicationPluginEncoderEncodeAudioFrame _encoderEncodeAudioFrame;
    PFN_nanoemApplicationPluginEncoderEncodeVideoFrame _encoderEncodeVideoFrame;
    PFN_nanoemApplicationPluginEncoderInterrupt _encoderInterrupt;
    PFN_nanoemApplicationPluginEncoderGetAllAvailableVideoFormats _encoderGetAllAvailableVideoFormatExtensions;
    PFN_nanoemApplicationPluginEncoderLoadUIWindowLayout _encoderLoadUIWindowLayout;
    PFN_nanoemApplicationPluginEncoderGetUIWindowLayoutDataSize _encoderGetUIWindowLayoutDataSize;
    PFN_nanoemApplicationPluginEncoderGetUIWindowLayoutData _encoderGetUIWindowLayoutData;
    PFN_nanoemApplicationPluginEncoderSetUIComponentLayoutData _encoderSetUIComponentLayoutData;
    PFN_nanoemApplicationPluginEncoderGetFailureReason _encoderGetFailureReason;
    PFN_nanoemApplicationPluginEncoderGetRecoverySuggestion _encoderGetRecoverySuggestion;
    PFN_nanoemApplicationPluginEncoderClose _encoderClose;
    PFN_nanoemApplicationPluginEncoderDestroy _encoderDestroy;
    PFN_nanoemApplicationPluginEncoderTerminate _encoderTerminate;
};

} /* namespace plugin */
} /* namespace nanoem */

#endif /* NANOEM_PLUGIN_ENCODERPLUGIN_H_ */
