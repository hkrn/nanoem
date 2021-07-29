/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef EMAPP_PLUGIN_SDK_MOTION_H_
#define EMAPP_PLUGIN_SDK_MOTION_H_

#include "Common.h"

/**
 * \defgroup emapp
 * @{
 */

/**
 * \defgroup emapp_plugin_motion_io nanoem Motion I/O Plugin
 * @{
 */

#define NANOEM_APPLICATION_PLUGIN_MOTION_ABI_VERSION_MAJOR 2
#define NANOEM_APPLICATION_PLUGIN_MOTION_ABI_VERSION_MINOR 0
#define NANOEM_APPLICATION_PLUGIN_MOTION_ABI_VERSION                                                                   \
    NANOEM_APPLICATION_PLUGIN_MAKE_ABI_VERSION(                                                                        \
        NANOEM_APPLICATION_PLUGIN_MOTION_ABI_VERSION_MAJOR, NANOEM_APPLICATION_PLUGIN_MOTION_ABI_VERSION_MINOR)

/**
 * \brief The opaque motion plugin object
 */
typedef struct nanoem_application_plugin_motion_io_t nanoem_application_plugin_motion_io_t;

/**
 * \brief
 *
 * \return NANOEM_DECL_API nanoem_u32_t APIENTRY
 */
NANOEM_DECL_API nanoem_u32_t APIENTRY nanoemApplicationPluginMotionIOGetABIVersion(void);

/**
 * \brief
 *
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOInitialize(void);

/**
 * \brief Create an opaque motion plugin object
 *
 * \return NANOEM_DECL_API nanoem_application_plugin_motion_io_t* APIENTRY
 */
NANOEM_DECL_API nanoem_application_plugin_motion_io_t *APIENTRY nanoemApplicationPluginMotionIOCreate(void);

/**
 * \brief Create an opaque model plugin object with the plugin path
 *
 * \param location
 * \return NANOEM_DECL_API nanoem_application_plugin_motion_io_t* APIENTRY
 */
NANOEM_DECL_API nanoem_application_plugin_motion_io_t *APIENTRY nanoemApplicationPluginMotionIOCreateWithLocation(
    const char *location);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param language
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetLanguage(
    nanoem_application_plugin_motion_io_t *plugin, int language, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \return NANOEM_DECL_API const char* APIENTRY
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginMotionIOGetName(
    const nanoem_application_plugin_motion_io_t *plugin);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \return NANOEM_DECL_API const char* APIENTRY
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginMotionIOGetDescription(
    const nanoem_application_plugin_motion_io_t *plugin);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \return NANOEM_DECL_API const char* APIENTRY
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginMotionIOGetVersion(
    const nanoem_application_plugin_motion_io_t *plugin);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \return NANOEM_DECL_API int APIENTRY
 */
NANOEM_DECL_API int APIENTRY nanoemApplicationPluginMotionIOCountAllFunctions(
    const nanoem_application_plugin_motion_io_t *plugin);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param index
 * \return NANOEM_DECL_API const char* APIENTRY
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginMotionIOGetFunctionName(
    const nanoem_application_plugin_motion_io_t *plugin, int index);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param index
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetFunction(
    nanoem_application_plugin_motion_io_t *plugin, int index, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param name
 * \param frameIndices
 * \param length
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetAllNamedSelectedBoneKeyframes(
    nanoem_application_plugin_motion_io_t *plugin, const char *name, const nanoem_frame_index_t *frameIndices,
    nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param name
 * \param frameIndices
 * \param length
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetAllNamedSelectedMorphKeyframes(
    nanoem_application_plugin_motion_io_t *plugin, const char *name, const nanoem_frame_index_t *frameIndices,
    nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param frameIndices
 * \param length
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetAllSelectedAccessoryKeyframes(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_frame_index_t *frameIndices, nanoem_u32_t length,
    nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param frameIndices
 * \param length
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetAllSelectedCameraKeyframes(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_frame_index_t *frameIndices, nanoem_u32_t length,
    nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param frameIndices
 * \param length
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetAllSelectedLightKeyframes(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_frame_index_t *frameIndices, nanoem_u32_t length,
    nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param frameIndices
 * \param length
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetAllSelectedModelKeyframes(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_frame_index_t *frameIndices, nanoem_u32_t length,
    nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param frameIndices
 * \param length
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetAllSelectedSelfShadowKeyframes(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_frame_index_t *frameIndices, nanoem_u32_t length,
    nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param data
 * \param length
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetAudioDescription(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param data
 * \param length
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetCameraDescription(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param data
 * \param length
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetLightDescription(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param data
 * \param length
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetInputMotionData(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param data
 * \param length
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetInputActiveModelData(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param data
 * \param length
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetInputAudioData(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOExecute(
    nanoem_application_plugin_motion_io_t *plugin, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param length
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOGetOutputMotionDataSize(
    nanoem_application_plugin_motion_io_t *plugin, nanoem_u32_t *length);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param data
 * \param length
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOGetOutputMotionData(
    nanoem_application_plugin_motion_io_t *plugin, nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOLoadUIWindowLayout(
    nanoem_application_plugin_motion_io_t *plugin, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param length
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOGetUIWindowLayoutDataSize(
    nanoem_application_plugin_motion_io_t *plugin, nanoem_u32_t *length);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param data
 * \param length
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOGetUIWindowLayoutData(
    nanoem_application_plugin_motion_io_t *plugin, nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \param id
 * \param data
 * \param length
 * \param reloadLayout
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetUIComponentLayoutData(
    nanoem_application_plugin_motion_io_t *plugin, const char *id, const nanoem_u8_t *data, nanoem_u32_t length,
    int *reloadLayout, nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \return NANOEM_DECL_API const char* APIENTRY
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginMotionIOGetFailureReason(
    const nanoem_application_plugin_motion_io_t *plugin);

/**
 * \brief
 *
 * \param plugin The opaque motion plugin object
 * \return NANOEM_DECL_API const char* APIENTRY
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginMotionIOGetRecoverySuggestion(
    const nanoem_application_plugin_motion_io_t *plugin);

/**
 * \brief Destroy an opaque motion plugin object
 *
 * \param plugin The opaque motion plugin object
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIODestroy(nanoem_application_plugin_motion_io_t *plugin);

/**
 * \brief
 *
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOTerminate(void);

#endif /* EMAPP_PLUGIN_SDK_MOTION_H_ */
