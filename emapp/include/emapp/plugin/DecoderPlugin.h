/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_PLUGIN_DECODERPLUGIN_H_
#define NANOEM_EMAPP_PLUGIN_DECODERPLUGIN_H_

#include "emapp/plugin/BasePlugin.h"

struct nanoem_application_plugin_decoder_t;

namespace nanoem {
namespace plugin {

class DecoderPlugin NANOEM_DECL_SEALED : public BasePlugin {
public:
    DecoderPlugin(IEventPublisher *publisher);
    ~DecoderPlugin() NANOEM_DECL_NOEXCEPT;

    bool load(const URI &fileURI) NANOEM_DECL_OVERRIDE;
    void unload() NANOEM_DECL_OVERRIDE;
    bool create() NANOEM_DECL_OVERRIDE;
    void destroy() NANOEM_DECL_OVERRIDE;

    bool open(const URI &fileURI, Error &error);
    void setOption(nanoem_u32_t key, int value, Error &error);
    void setOption(nanoem_u32_t key, nanoem_u32_t value, Error &error);
    void setOption(nanoem_u32_t key, const char *value, Error &error);
    nanoem_frame_index_t audioDuration() const NANOEM_DECL_NOEXCEPT;
    nanoem_frame_index_t videoDuration() const NANOEM_DECL_NOEXCEPT;
    nanoem_u32_t audioFormatValue(nanoem_u32_t key) const NANOEM_DECL_NOEXCEPT;
    nanoem_u32_t videoFormatValue(nanoem_u32_t key) const NANOEM_DECL_NOEXCEPT;
    bool decodeAudioFrame(nanoem_frame_index_t currentLocalFrameIndex, ByteArray &bytes, Error &error);
    bool decodeVideoFrame(nanoem_frame_index_t currentLocalFrameIndex, ByteArray &bytes, Error &error);
    void getAvailableAudioFormatExtensions(StringList &formatExtensionList) const;
    void getAvailableVideoFormatExtensions(StringList &formatExtensionList) const;
    void close(Error &error);

    const char *failureReason() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const char *recoverySuggestion() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    typedef nanoem_application_plugin_decoder_t *(APIENTRY *PFN_nanoemApplicationPluginDecoderCreate)();
    typedef nanoem_u32_t(APIENTRY *PFN_nanoemApplicationPluginDecoderGetABIVersion)();
    typedef nanoem_u32_t(APIENTRY *PFN_nanoemApplicationPluginDecoderGetAPIVersion)();
    typedef void(APIENTRY *PFN_nanoemApplicationPluginDecoderInitialize)();
    typedef int(APIENTRY *PFN_nanoemApplicationPluginDecoderOpen)(
        nanoem_application_plugin_decoder_t *, const char *, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginDecoderSetOption)(
        nanoem_application_plugin_decoder_t *, nanoem_u32_t, const void *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginDecoderGetAudioFormatValue)(
        nanoem_application_plugin_decoder_t *, nanoem_u32_t, void *, nanoem_u32_t *, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginDecoderGetVideoFormatValue)(
        nanoem_application_plugin_decoder_t *, nanoem_u32_t, void *, nanoem_u32_t *, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginDecoderDecodeAudioFrame)(
        nanoem_application_plugin_decoder_t *, nanoem_frame_index_t, nanoem_u8_t **, nanoem_u32_t *, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginDecoderDecodeVideoFrame)(
        nanoem_application_plugin_decoder_t *, nanoem_frame_index_t, nanoem_u8_t **, nanoem_u32_t *, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginDecoderDestroyAudioFrame)(
        nanoem_application_plugin_decoder_t *, nanoem_frame_index_t, nanoem_u8_t *, nanoem_rsize_t);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginDecoderDestroyVideoFrame)(
        nanoem_application_plugin_decoder_t *, nanoem_frame_index_t, nanoem_u8_t *, nanoem_rsize_t);
    typedef const char *const *(APIENTRY *PFN_nanoemApplicationPluginDecoderGetAllAvailableAudioFormatExtensions)(
        nanoem_application_plugin_decoder_t *, nanoem_u32_t *);
    typedef const char *const *(APIENTRY *PFN_nanoemApplicationPluginDecoderGetAllAvailableVideoFormatExtensions)(
        nanoem_application_plugin_decoder_t *, nanoem_u32_t *);
    typedef const char *(APIENTRY *PFN_nanoemApplicationPluginDecoderGetFailureReason)(
        const nanoem_application_plugin_decoder_t *);
    typedef const char *(APIENTRY *PFN_nanoemApplicationPluginDecoderGetRecoverySuggestion)(
        const nanoem_application_plugin_decoder_t *);
    typedef int(APIENTRY *PFN_nanoemApplicationPluginDecoderClose)(nanoem_application_plugin_decoder_t *, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginDecoderDestroy)(nanoem_application_plugin_decoder_t *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginDecoderTerminate)();

    nanoem_application_plugin_decoder_t *m_decoder;
    bool m_interrupted;
    PFN_nanoemApplicationPluginDecoderInitialize _decoderInitialize;
    PFN_nanoemApplicationPluginDecoderCreate _decoderCreate;
    PFN_nanoemApplicationPluginDecoderOpen _decoderOpen;
    PFN_nanoemApplicationPluginDecoderSetOption _decoderSetOption;
    PFN_nanoemApplicationPluginDecoderGetAudioFormatValue _decoderGetAudioFormatValue;
    PFN_nanoemApplicationPluginDecoderGetVideoFormatValue _decoderGetVideoFormatValue;
    PFN_nanoemApplicationPluginDecoderDecodeAudioFrame _decoderDecodeAudioFrame;
    PFN_nanoemApplicationPluginDecoderDecodeVideoFrame _decoderDecodeVideoFrame;
    PFN_nanoemApplicationPluginDecoderDestroyAudioFrame _decoderDestroyDecodedAudioFrame;
    PFN_nanoemApplicationPluginDecoderDestroyVideoFrame _decoderDestroyDecodedVideoFrame;
    PFN_nanoemApplicationPluginDecoderGetAllAvailableAudioFormatExtensions _decoderGetAllAvailableAudioFormatExtensions;
    PFN_nanoemApplicationPluginDecoderGetAllAvailableVideoFormatExtensions _decoderGetAllAvailableVideoFormatExtensions;
    PFN_nanoemApplicationPluginDecoderGetFailureReason _decoderGetFailureReason;
    PFN_nanoemApplicationPluginDecoderGetRecoverySuggestion _decoderGetRecoverySuggestion;
    PFN_nanoemApplicationPluginDecoderClose _decoderClose;
    PFN_nanoemApplicationPluginDecoderDestroy _decoderDestroy;
    PFN_nanoemApplicationPluginDecoderTerminate _decoderTerminate;
};

} /* namespace plugin */
} /* namespace nanoem */

#endif /* NANOEM_PLUGIN_DECODERPLUGIN_H_ */
