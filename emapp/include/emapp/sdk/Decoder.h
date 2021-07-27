/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef EMAPP_PLUGIN_SDK_DECODER_H_
#define EMAPP_PLUGIN_SDK_DECODER_H_

#include "Common.h"

#define NANOEM_APPLICATION_PLUGIN_DECODER_ABI_VERSION_MAJOR 2
#define NANOEM_APPLICATION_PLUGIN_DECODER_ABI_VERSION_MINOR 0
#define NANOEM_APPLICATION_PLUGIN_DECODER_ABI_VERSION                                                                  \
    NANOEM_APPLICATION_PLUGIN_MAKE_ABI_VERSION(                                                                        \
        NANOEM_APPLICATION_PLUGIN_DECODER_ABI_VERSION_MAJOR, NANOEM_APPLICATION_PLUGIN_DECODER_ABI_VERSION_MINOR)

typedef struct nanoem_application_plugin_decoder_t nanoem_application_plugin_decoder_t;

NANOEM_DECL_ENUM(int, nanoem_application_plugin_decoder_option_t) {
    NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_UNKNOWN = -1, NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_FIRST_ENUM,
    NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_FPS = NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_FIRST_ENUM,
    NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_AUDIO_LOCATION, NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_VIDEO_LOCATION,
    NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_MAX_ENUM
};

NANOEM_DECL_ENUM(int,
    nanoem_application_plugin_decoder_audio_format_t) { NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_UNKNOWN = -1,
    NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_FIRST_ENUM,
    NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_NUM_BITS = NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_FIRST_ENUM,
    NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_NUM_CHANNELS,
    NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_FREQUENCY, NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_DURATION,
    NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_MAX_ENUM };

NANOEM_DECL_ENUM(int,
    nanoem_application_plugin_decoder_video_format_t) { NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_UNKNOWN = -1,
    NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_FIRST_ENUM,
    NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_WIDTH = NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_FIRST_ENUM,
    NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_HEIGHT, NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_STRIDE,
    NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_DURATION, NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_MAX_ENUM };

/**
 * \brief
 *
 * \return NANOEM_DECL_API nanoem_u32_t APIENTRY
 */
NANOEM_DECL_API nanoem_u32_t APIENTRY nanoemApplicationPluginDecoderGetABIVersion(void);

/**
 * \brief
 *
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderInitialize(void);

/**
 * \brief
 *
 * \return NANOEM_DECL_API nanoem_application_plugin_decoder_t* APIENTRY
 */
NANOEM_DECL_API nanoem_application_plugin_decoder_t *APIENTRY nanoemApplicationPluginDecoderCreate();

/**
 * \brief
 *
 * \param decoder
 * \param filePath
 * \param status
 * \return NANOEM_DECL_API int APIENTRY
 */
NANOEM_DECL_API int APIENTRY nanoemApplicationPluginDecoderOpen(
    nanoem_application_plugin_decoder_t *decoder, const char *filePath, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param decoder
 * \param key
 * \param value
 * \param size
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderSetOption(nanoem_application_plugin_decoder_t *decoder,
    nanoem_u32_t key, const void *value, nanoem_u32_t size, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param decoder
 * \param key
 * \param value
 * \param size
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderGetAudioFormatValue(
    nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t key, void *value, nanoem_u32_t *size,
    nanoem_i32_t *status);

/**
 * \brief
 *
 * \param decoder
 * \param key
 * \param value
 * \param size
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderGetVideoFormatValue(
    nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t key, void *value, nanoem_u32_t *size,
    nanoem_i32_t *status);
/**
 * \brief
 *
 * \param decoder
 * \param currentFrameIndex
 * \param data
 * \param size
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderDecodeAudioFrame(
    nanoem_application_plugin_decoder_t *decoder, nanoem_frame_index_t currentFrameIndex, nanoem_u8_t **data,
    nanoem_u32_t *size, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param decoder
 * \param currentFrameIndex
 * \param data
 * \param size
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderDecodeVideoFrame(
    nanoem_application_plugin_decoder_t *decoder, nanoem_frame_index_t currentFrameIndex, nanoem_u8_t **data,
    nanoem_u32_t *size, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param decoder
 * \param currentFrameIndex
 * \param data
 * \param size
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderDestroyAudioFrame(
    nanoem_application_plugin_decoder_t *decoder, nanoem_frame_index_t currentFrameIndex, nanoem_u8_t *data,
    nanoem_u32_t size);

/**
 * \brief
 *
 * \param decoder
 * \param currentFrameIndex
 * \param data
 * \param size
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderDestroyVideoFrame(
    nanoem_application_plugin_decoder_t *decoder, nanoem_frame_index_t currentFrameIndex, nanoem_u8_t *data,
    nanoem_u32_t size);

/**
 * \brief
 *
 * \param decoder
 * \param length
 * \return NANOEM_DECL_API const char* const* APIENTRY
 */
NANOEM_DECL_API const char *const *APIENTRY nanoemApplicationPluginDecoderGetAllAvailableAudioFormatExtensions(
    nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t *length);

/**
 * \brief
 *
 * \param decoder
 * \param length
 * \return NANOEM_DECL_API const char* const* APIENTRY
 */
NANOEM_DECL_API const char *const *APIENTRY nanoemApplicationPluginDecoderGetAllAvailableVideoFormatExtensions(
    nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t *length);

/**
 * \brief
 *
 * \param decoder
 * \return NANOEM_DECL_API const char* APIENTRY
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginDecoderGetFailureReason(
    nanoem_application_plugin_decoder_t *decoder);

/**
 * \brief
 *
 * \param decoder
 * \return NANOEM_DECL_API const char* APIENTRY
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginDecoderGetRecoverySuggestion(
    nanoem_application_plugin_decoder_t *decoder);

/**
 * \brief
 *
 * \param decoder
 * \param status
 * \return NANOEM_DECL_API int APIENTRY
 */
NANOEM_DECL_API int APIENTRY nanoemApplicationPluginDecoderClose(
    nanoem_application_plugin_decoder_t *decoder, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param decoder
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderDestroy(nanoem_application_plugin_decoder_t *decoder);

/**
 * \brief
 *
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderTerminate(void);

#endif /* EMAPP_PLUGIN_SDK_DECODER_H_ */
