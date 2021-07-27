/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef EMAPP_PLUGIN_SDK_ENCODER_H_
#define EMAPP_PLUGIN_SDK_ENCODER_H_

#include "Common.h"

#define NANOEM_APPLICATION_PLUGIN_ENCODER_ABI_VERSION_MAJOR 2
#define NANOEM_APPLICATION_PLUGIN_ENCODER_ABI_VERSION_MINOR 0
#define NANOEM_APPLICATION_PLUGIN_ENCODER_ABI_VERSION                                                                  \
    NANOEM_APPLICATION_PLUGIN_MAKE_ABI_VERSION(                                                                        \
        NANOEM_APPLICATION_PLUGIN_ENCODER_ABI_VERSION_MAJOR, NANOEM_APPLICATION_PLUGIN_ENCODER_ABI_VERSION_MINOR)

typedef struct nanoem_application_plugin_encoder_t nanoem_application_plugin_encoder_t;

NANOEM_DECL_ENUM(int, nanoem_application_plugin_encoder_option_t) {
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_UNKNOWN = -1, NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_FIRST_ENUM,
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_FPS = NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_FIRST_ENUM,
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_DURATION, NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_LOCATION,
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_FREQUENCY,
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_CHANNELS,
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_BITS, NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_WIDTH,
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HEIGHT, NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_YFLIP,
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HDR_NUM_BITS, NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_MAX_ENUM
};

/**
 * \brief
 *
 * \return NANOEM_DECL_API nanoem_u32_t APIENTRY
 */
NANOEM_DECL_API nanoem_u32_t APIENTRY nanoemApplicationPluginEncoderGetABIVersion(void);

/**
 * \brief
 *
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderInitialize(void);

/**
 * \brief
 *
 * \return NANOEM_DECL_API nanoem_application_plugin_encoder_t* APIENTRY
 */
NANOEM_DECL_API nanoem_application_plugin_encoder_t *APIENTRY nanoemApplicationPluginEncoderCreate();

/**
 * \brief
 *
 * \param encoder
 * \param filePath
 * \param status
 * \return NANOEM_DECL_API int APIENTRY
 */
NANOEM_DECL_API int APIENTRY nanoemApplicationPluginEncoderOpen(
    nanoem_application_plugin_encoder_t *encoder, const char *filePath, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param encoder
 * \param key
 * \param value
 * \param size
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderSetOption(nanoem_application_plugin_encoder_t *encoder,
    nanoem_u32_t key, const void *value, nanoem_u32_t size, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param encoder
 * \param currentFrameIndex
 * \param data
 * \param size
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderEncodeAudioFrame(
    nanoem_application_plugin_encoder_t *encoder, nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data,
    nanoem_u32_t size, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param encoder
 * \param currentFrameIndex
 * \param data
 * \param size
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderEncodeVideoFrame(
    nanoem_application_plugin_encoder_t *encoder, nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data,
    nanoem_u32_t size, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param encoder
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderInterrupt(
    nanoem_application_plugin_encoder_t *encoder, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param encoder
 * \param length
 * \return NANOEM_DECL_API const char* const* APIENTRY
 */
NANOEM_DECL_API const char *const *APIENTRY nanoemApplicationPluginEncoderGetAllAvailableVideoFormatExtensions(
    const nanoem_application_plugin_encoder_t *encoder, nanoem_u32_t *length);

/**
 * \brief
 *
 * \param plugin
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderLoadUIWindowLayout(
    nanoem_application_plugin_encoder_t *plugin, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin
 * \param length
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderGetUIWindowLayoutDataSize(
    nanoem_application_plugin_encoder_t *plugin, nanoem_u32_t *length);

/**
 * \brief
 *
 * \param plugin
 * \param data
 * \param length
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderGetUIWindowLayoutData(
    nanoem_application_plugin_encoder_t *plugin, nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin
 * \param id
 * \param data
 * \param length
 * \param reloadLayout
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderSetUIComponentLayoutData(
    nanoem_application_plugin_encoder_t *plugin, const char *id, const nanoem_u8_t *data, nanoem_u32_t length,
    int *reloadLayout, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param encoder
 * \return NANOEM_DECL_API const char* APIENTRY
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginEncoderGetFailureReason(
    const nanoem_application_plugin_encoder_t *encoder);

/**
 * \brief
 *
 * \param encoder
 * \return NANOEM_DECL_API const char* APIENTRY
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginEncoderGetRecoverySuggestion(
    const nanoem_application_plugin_encoder_t *encoder);

/**
 * \brief
 *
 * \param encoder
 * \param status
 * \return NANOEM_DECL_API int APIENTRY
 */
NANOEM_DECL_API int APIENTRY nanoemApplicationPluginEncoderClose(
    nanoem_application_plugin_encoder_t *encoder, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param encoder
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderDestroy(nanoem_application_plugin_encoder_t *encoder);

/**
 * \brief
 *
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderTerminate(void);

#endif /* EMAPP_PLUGIN_SDK_ENCODER_H_ */
