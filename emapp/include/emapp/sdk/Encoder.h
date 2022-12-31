/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef EMAPP_PLUGIN_SDK_ENCODER_H_
#define EMAPP_PLUGIN_SDK_ENCODER_H_

#include "Common.h"

/**
 * \defgroup emapp nanoem Application (emapp)
 * @{
 */

/**
 * \defgroup emapp_plugin_encoder nanoem Application Encoder Plugin
 * @{
 */

#define NANOEM_APPLICATION_PLUGIN_ENCODER_ABI_VERSION_MAJOR 2
#define NANOEM_APPLICATION_PLUGIN_ENCODER_ABI_VERSION_MINOR 0
#define NANOEM_APPLICATION_PLUGIN_ENCODER_ABI_VERSION                                                                  \
    NANOEM_APPLICATION_PLUGIN_MAKE_ABI_VERSION(                                                                        \
        NANOEM_APPLICATION_PLUGIN_ENCODER_ABI_VERSION_MAJOR, NANOEM_APPLICATION_PLUGIN_ENCODER_ABI_VERSION_MINOR)

/**
 * \brief The opaque encoder plugin object
 */
typedef struct nanoem_application_plugin_encoder_t nanoem_application_plugin_encoder_t;

NANOEM_DECL_ENUM(int,
    nanoem_application_plugin_encoder_option_t) { NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_UNKNOWN = -1, ///< Unknown
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_FIRST_ENUM,
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_FPS =
        NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_FIRST_ENUM, ///< Setting encode FPS value (u32)
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_DURATION, ///< Setting encode duration (u32)
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_LOCATION, ///< Setting encode audio/video location (string)
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_FREQUENCY, ///< Setting encode audio number of frequency (u32)
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_CHANNELS, ///< Setting encode audio number of channels (u32)
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_BITS, ///< Setting encode audio number of bits (u32)
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_WIDTH, ///< Setting encode video width (u32)
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HEIGHT, ///< Setting encode video height (u32)
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_YFLIP, ///< Setting encode video with Y-flip enabled (u32)
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HDR_NUM_BITS, ///< Setting encode video color depth (u32)
    NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_MAX_ENUM };

/**
 * \brief Get plugin's ABI version
 *
 * Plugin ABI version consists major version and minor version.
 *
 * If the major version differs from nanoem runtime version, The plugin will not be loaded.
 * If the minor version is greater than nanoem runtime version, corresponding functions will not be called.
 *
 * \return The plugin ABI version made with \b NANOEM_APPLICATION_PLUGIN_MODEL_ABI_VERSION
 * \since Encoder Plugin ABI 2.0
 */
NANOEM_DECL_API nanoem_u32_t APIENTRY nanoemApplicationPluginEncoderGetABIVersion(void);

/**
 * \brief Initialize the plugin
 *
 * The function will call once at the plugin initialization.
 *
 * \since Encoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderInitialize(void);

/**
 * \brief Create an opaque encoder plugin object
 *
 * \return The opaque encoder plugin object
 * \since Encoder Plugin ABI 2.0
 */
NANOEM_DECL_API nanoem_application_plugin_encoder_t *APIENTRY nanoemApplicationPluginEncoderCreate();

/**
 * \brief Open the opaque encoder plugin object
 *
 * \param encoder The opaque encoder plugin object
 * \param filePath The file path to encode
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Encoder Plugin ABI 2.0
 */
NANOEM_DECL_API int APIENTRY nanoemApplicationPluginEncoderOpen(
    nanoem_application_plugin_encoder_t *encoder, const char *filePath, nanoem_i32_t *status);

/**
 * \brief Set the encoding audio/video option
 *
 * \param encoder The opaque encoder plugin object
 * \param key The key of \b nanoem_application_plugin_encoder_option_t
 * \param value The value corresponding of the \b key
 * \param size The size of \b value in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Encoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderSetOption(nanoem_application_plugin_encoder_t *encoder,
    nanoem_u32_t key, const void *value, nanoem_u32_t size, nanoem_i32_t *status);

/**
 * \brief Encode audio frame data
 *
 * \param encoder The opaque encoder plugin object
 * \param currentFrameIndex The frame index to encode
 * \param data The audio frame data to encode
 * \param size The size of \b data in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Encoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderEncodeAudioFrame(
    nanoem_application_plugin_encoder_t *encoder, nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data,
    nanoem_u32_t size, nanoem_i32_t *status);

/**
 * \brief Encode video frame data
 *
 * \param encoder The opaque encoder plugin object
 * \param currentFrameIndex The frame index to encode
 * \param data The video frame data to encode
 * \param size The size of \b data in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Encoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderEncodeVideoFrame(
    nanoem_application_plugin_encoder_t *encoder, nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data,
    nanoem_u32_t size, nanoem_i32_t *status);

/**
 * \brief Interrupt encoding audio/video frame
 *
 * \param encoder The opaque encoder plugin object
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Encoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderInterrupt(
    nanoem_application_plugin_encoder_t *encoder, nanoem_i32_t *status);

/**
 * \brief Get all encodeable extension strings
 *
 * \param encoder The opaque encoder plugin object
 * \param[out] length Number of all encodeable extensions
 * \since Encoder Plugin ABI 2.0
 */
NANOEM_DECL_API const char *const *APIENTRY nanoemApplicationPluginEncoderGetAllAvailableVideoFormatExtensions(
    const nanoem_application_plugin_encoder_t *encoder, nanoem_u32_t *length);

/**
 * \brief Initialize and loads plugin UI window layout
 *
 * \param encoder The opaque encoder plugin object
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Encoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderLoadUIWindowLayout(
    nanoem_application_plugin_encoder_t *encoder, nanoem_i32_t *status);

/**
 * \brief Get plugin UI window layout data size
 *
 * \param encoder The opaque encoder plugin object
 * \param[out] length plugin window layout data size in byte unit
 * \since Encoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderGetUIWindowLayoutDataSize(
    nanoem_application_plugin_encoder_t *encoder, nanoem_u32_t *length);

/**
 * \brief Get plugin UI window layout data
 *
 * UI window layout data must be encoded with \b nanoem.application.plugin.UIWindow defined in
 * \e emapp/resources/protobuf/plugin.proto
 *
 * \param encoder The opaque encoder plugin object
 * \param data Protocol buffer data serialized with \b nanoem.application.plugin.UIWindow
 * \param length \b data in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Encoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderGetUIWindowLayoutData(
    nanoem_application_plugin_encoder_t *encoder, nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set plugin UI window component data corresponding plugin UI component ID
 *
 * UI component data must be encoded with \b nanoem.application.plugin.UIComponent defined in
 * \e emapp/resources/protobuf/plugin.proto
 *
 * \param encoder The opaque encoder plugin object
 * \param id ID to update component
 * \param data Protocol buffer data serialized with \b nanoem.application.plugin.UIComponent
 * \param length \b data in byte unit
 * \param[out] reloadLayout Whether UI window must be reloaded
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Encoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderSetUIComponentLayoutData(
    nanoem_application_plugin_encoder_t *encoder, const char *id, const nanoem_u8_t *data, nanoem_u32_t length,
    int *reloadLayout, nanoem_i32_t *status);

/**
 * \brief Get error failure reason text
 *
 * The function should be able to get failure reason when \b NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON is set
 * in status.
 *
 * \param encoder The opaque encoder plugin object
 * \return The error reason text to show it to the user via error dialog
 * \since Encoder Plugin ABI 2.0
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginEncoderGetFailureReason(
    const nanoem_application_plugin_encoder_t *encoder);

/**
 * \brief Get error recovery suggestion text
 *
 * The function should be able to get recovery suggestion to resolve the issue by user when \b
 * NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON is set in status.
 *
 * \param encoder The opaque encoder plugin object
 * \return The recovery suggestion text to show it to the user via error dialog
 * \since Encoder Plugin ABI 2.0
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginEncoderGetRecoverySuggestion(
    const nanoem_application_plugin_encoder_t *encoder);

/**
 * \brief Close the opaque encoder plugin object
 *
 * \param encoder The opaque encoder plugin object
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Encoder Plugin ABI 2.0
 */
NANOEM_DECL_API int APIENTRY nanoemApplicationPluginEncoderClose(
    nanoem_application_plugin_encoder_t *encoder, nanoem_i32_t *status);

/**
 * \brief Destroy an opaque encoder plugin object
 *
 * \param encoder The opaque encoder plugin object
 * \since Encoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderDestroy(nanoem_application_plugin_encoder_t *encoder);

/**
 * \brief Terminate the plugin
 *
 * \since Encoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEncoderTerminate(void);

/** @} */

/** @} */

#endif /* EMAPP_PLUGIN_SDK_ENCODER_H_ */
