/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef EMAPP_PLUGIN_SDK_DECODER_H_
#define EMAPP_PLUGIN_SDK_DECODER_H_

#include "Common.h"

/**
 * \defgroup emapp nanoem Application (emapp)
 * @{
 */

/**
 * \defgroup emapp_plugin_decoder nanoem Application Decoder Plugin
 * @{
 */

#define NANOEM_APPLICATION_PLUGIN_DECODER_ABI_VERSION_MAJOR 2
#define NANOEM_APPLICATION_PLUGIN_DECODER_ABI_VERSION_MINOR 0
#define NANOEM_APPLICATION_PLUGIN_DECODER_ABI_VERSION                                                                  \
    NANOEM_APPLICATION_PLUGIN_MAKE_ABI_VERSION(                                                                        \
        NANOEM_APPLICATION_PLUGIN_DECODER_ABI_VERSION_MAJOR, NANOEM_APPLICATION_PLUGIN_DECODER_ABI_VERSION_MINOR)

/**
 * \brief The opaque decoder plugin object
 */
typedef struct nanoem_application_plugin_decoder_t nanoem_application_plugin_decoder_t;

NANOEM_DECL_ENUM(int, nanoem_application_plugin_decoder_option_t) {
    NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_UNKNOWN = -1, ///< Unknown
    NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_FIRST_ENUM,
    NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_FPS = NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_FIRST_ENUM, ///< Setting FPS value (u32)
    NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_AUDIO_LOCATION,  ///< Setting decode audio location (string)
    NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_VIDEO_LOCATION, ///< Setting decode video location (string)
    NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_MAX_ENUM
};

NANOEM_DECL_ENUM(int,
    nanoem_application_plugin_decoder_audio_format_t) {
    NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_UNKNOWN = -1, ///< Unknown
    NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_FIRST_ENUM,
    NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_NUM_BITS = NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_FIRST_ENUM, ///< Getting decode audio number of bits (u32)
    NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_NUM_CHANNELS, ///< Getting decode audio number of channels (u32)
    NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_FREQUENCY, ///< Getting decode audio number of frequency (u32)
    NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_DURATION, ///< Getting decode audio duration (u32)
    NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_MAX_ENUM };

NANOEM_DECL_ENUM(int,
    nanoem_application_plugin_decoder_video_format_t) {
    NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_UNKNOWN = -1, ///< Unknown
    NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_FIRST_ENUM,
    NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_WIDTH = NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_FIRST_ENUM, ///< Getting decode video width (u32)
    NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_HEIGHT, ///< Getting decode video height (u32)
    NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_STRIDE, ///< Getting decode video stride (u32)
    NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_DURATION, ///< Getting decode video duration (u32)
    NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_MAX_ENUM };

/**
 * \brief Get plugin's ABI version
 *
 * Plugin ABI version consists major version and minor version.
 *
 * If the major version differs from nanoem runtime version, The plugin will not be loaded.
 * If the minor version is greater than nanoem runtime version, corresponding functions will not be called.
 *
 * \return The plugin ABI version made with \b NANOEM_APPLICATION_PLUGIN_MODEL_ABI_VERSION
 * \since Decoder Plugin ABI 2.0
 */
NANOEM_DECL_API nanoem_u32_t APIENTRY nanoemApplicationPluginDecoderGetABIVersion(void);

/**
 * \brief Initialize the plugin
 *
 * The function will call once at the plugin initialization.
 *
 * \since Decoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderInitialize(void);

/**
 * \brief Create an opaque decoder plugin object
 *
 * \return The opaque decoder plugin object
 * \since Decoder Plugin ABI 2.0
 */
NANOEM_DECL_API nanoem_application_plugin_decoder_t *APIENTRY nanoemApplicationPluginDecoderCreate();

/**
 * \brief Open the opaque decoder plugin object
 *
 * \param decoder The opaque decoder plugin object
 * \param filePath The file path to decode
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Decoder Plugin ABI 2.0
 */
NANOEM_DECL_API int APIENTRY nanoemApplicationPluginDecoderOpen(
    nanoem_application_plugin_decoder_t *decoder, const char *filePath, nanoem_i32_t *status);

/**
 * \brief Set the decoding audio/video option
 *
 * \param decoder The opaque decoder plugin object
 * \param key The key of \b nanoem_application_plugin_decoder_option_t
 * \param value The value corresponding of the \b key
 * \param size The size of \b value in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Decoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderSetOption(nanoem_application_plugin_decoder_t *decoder,
    nanoem_u32_t key, const void *value, nanoem_u32_t size, nanoem_i32_t *status);

/**
 * \brief Get the decoding audio format value
 *
 * \param decoder The opaque decoder plugin object
 * \param key The key of \b nanoem_application_plugin_decoder_audio_format_t
 * \param value The value corresponding of the \b key
 * \param[out] size The size of \b value in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Decoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderGetAudioFormatValue(
    nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t key, void *value, nanoem_u32_t *size,
    nanoem_i32_t *status);

/**
 * \brief Get the decoding video format value
 *
 * \param decoder The opaque decoder plugin object
 * \param key The key of \b nanoem_application_plugin_decoder_video_format_t
 * \param value The value corresponding of the \b key
 * \param[out] size The size of \b value in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Decoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderGetVideoFormatValue(
    nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t key, void *value, nanoem_u32_t *size,
    nanoem_i32_t *status);
/**
 * \brief Decode audio frame data
 *
 * \param decoder The opaque decoder plugin object
 * \param currentFrameIndex The frame index to be decoded
 * \param[out] data The audio frame data pointer to be decoded
 * \param[out] size The size pointer of \b data in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Decoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderDecodeAudioFrame(
    nanoem_application_plugin_decoder_t *decoder, nanoem_frame_index_t currentFrameIndex, nanoem_u8_t **data,
    nanoem_u32_t *size, nanoem_i32_t *status);

/**
 * \brief Decode video frame data
 *
 * \param decoder The opaque decoder plugin object
 * \param currentFrameIndex The frame index to be decoded
 * \param[out] data The video frame data pointer to be decoded
 * \param[out] size The size of \b data in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Decoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderDecodeVideoFrame(
    nanoem_application_plugin_decoder_t *decoder, nanoem_frame_index_t currentFrameIndex, nanoem_u8_t **data,
    nanoem_u32_t *size, nanoem_i32_t *status);

/**
 * \brief Destroy decoded audio frame data
 *
 * \param decoder The opaque decoder plugin object
 * \param currentFrameIndex The frame index to be freed
 * \param data The decoded audio frame data to be freed
 * \param size The size of \b data in byte unit
 * \since Decoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderDestroyAudioFrame(
    nanoem_application_plugin_decoder_t *decoder, nanoem_frame_index_t currentFrameIndex, nanoem_u8_t *data,
    nanoem_u32_t size);

/**
 * \brief Destroy decoded video frame data
 *
 * \param decoder The opaque decoder plugin object
 * \param currentFrameIndex The frame index to be freed
 * \param data The decoded video frame data to be freed
 * \param size The size of \b data in byte unit
 * \since Decoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderDestroyVideoFrame(
    nanoem_application_plugin_decoder_t *decoder, nanoem_frame_index_t currentFrameIndex, nanoem_u8_t *data,
    nanoem_u32_t size);

/**
 * \brief Get all decodeable extension strings
 *
 * \param decoder The opaque decoder plugin object
 * \param[out] length Number of all decodable extensions
 * \since Decoder Plugin ABI 2.0
 */
NANOEM_DECL_API const char *const *APIENTRY nanoemApplicationPluginDecoderGetAllAvailableAudioFormatExtensions(
    nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t *length);

/**
 * \brief Get all decodeable extension strings
 *
 * \param decoder The opaque decoder plugin object
 * \param[out] length Number of all decodeable extensions
 * \since Decoder Plugin ABI 2.0
 */
NANOEM_DECL_API const char *const *APIENTRY nanoemApplicationPluginDecoderGetAllAvailableVideoFormatExtensions(
    nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t *length);

/**
 * \brief Get error failure reason text
 *
 * The function should be able to get failure reason when \b NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON is set
 * in status.
 *
 * \param decoder The opaque decoder plugin object
 * \return The error reason text to show it to the user via error dialog
 * \since Decoder Plugin ABI 2.0
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginDecoderGetFailureReason(
    nanoem_application_plugin_decoder_t *decoder);

/**
 * \brief Get error recovery suggestion text
 *
 * The function should be able to get recovery suggestion to resolve the issue by user when \b
 * NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON is set in status.
 *
 * \param decoder The opaque decoder plugin object
 * \return The recovery suggestion text to show it to the user via error dialog
 * \since Decoder Plugin ABI 2.0
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginDecoderGetRecoverySuggestion(
    nanoem_application_plugin_decoder_t *decoder);

/**
 * \brief Close the opaque decoder plugin object
 *
 * \param decoder The opaque decoder plugin object
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Decoder Plugin ABI 2.0
 */
NANOEM_DECL_API int APIENTRY nanoemApplicationPluginDecoderClose(
    nanoem_application_plugin_decoder_t *decoder, nanoem_i32_t *status);

/**
 * \brief Destroy an opaque decoder plugin object
 *
 * \param plugin The opaque decoder plugin object
 * \since Decoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderDestroy(nanoem_application_plugin_decoder_t *decoder);

/**
 * \brief Terminate the plugin
 *
 * \since Decoder Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginDecoderTerminate(void);

/** @} */

/** @} */

#endif /* EMAPP_PLUGIN_SDK_DECODER_H_ */
