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
 * \brief Get plugin's ABI version
 *
 * Plugin ABI version consists major version and minor version.
 *
 * If the major version differs from nanoem runtime version, The plugin will not be loaded.
 * If the minor version is greater than nanoem runtime version, corresponding functions will not be called.
 *
 * \return The plugin ABI version made with \b NANOEM_APPLICATION_PLUGIN_MOTION_ABI_VERSION
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API nanoem_u32_t APIENTRY nanoemApplicationPluginMotionIOGetABIVersion(void);

/**
 * \brief Initialize the plugin
 *
 * The function will call once at the plugin initialization.
 *
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOInitialize(void);

/**
 * \brief Create an opaque motion plugin object
 *
 * \return The opaque motion plugin object
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API nanoem_application_plugin_motion_io_t *APIENTRY nanoemApplicationPluginMotionIOCreate(void);

/**
 * \brief Create an opaque motion plugin object with the plugin path
 *
 * If both ::nanoemApplicationPluginMotionIOCreate and ::nanoemApplicationPluginMotionIOCreateWithLocation are defined,
 * only ::nanoemApplicationPluginMotionIOCreateWithLocation will be called.
 *
 * \param location The absolute path of the plugin canonicalized with slash
 * \return The opaque motion plugin object
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API nanoem_application_plugin_motion_io_t *APIENTRY nanoemApplicationPluginMotionIOCreateWithLocation(
    const char *location);

/**
 * \brief Set current language value of the plugin
 *
 * If the function is not defined, assume the plugin as Japanese
 *
 * \param plugin The opaque motion plugin object
 * \param value 0 for Japanese, 1 for English
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if
 * succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetLanguage(
    nanoem_application_plugin_motion_io_t *plugin, int language, nanoem_i32_t *status);

/**
 * \brief Get the plugin name corresponding current language
 *
 * If the function is not defined, constructs from the plugin filename.
 *
 * \param plugin The opaque motion plugin object
 * \return The plugin name with UTF-8
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginMotionIOGetName(
    const nanoem_application_plugin_motion_io_t *plugin);

/**
 * \brief Get the plugin description corresponding current language
 *
 * \param plugin The opaque motion plugin object
 * \return The plugin description with UTF-8
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginMotionIOGetDescription(
    const nanoem_application_plugin_motion_io_t *plugin);

/**
 * \brief Get the plugin version
 *
 * The plugin version string should be semantic version \link https://semver.org \endlink
 *
 * \param plugin The opaque motion plugin object
 * \return The plugin version string with UTF-8
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginMotionIOGetVersion(
    const nanoem_application_plugin_motion_io_t *plugin);

/**
 * \brief Get the function name corresponding current language and function index in the plugin
 *
 * \param plugin The opaque motion plugin object
 * \param index The corresponding function index
 * \return The plugin function name with UTF-8
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API int APIENTRY nanoemApplicationPluginMotionIOCountAllFunctions(
    const nanoem_application_plugin_motion_io_t *plugin);

/**
 * \brief Get the function name corresponding current language and function index in the plugin
 *
 * \param plugin The opaque motion plugin object
 * \param index The corresponding function index
 * \return The plugin function name with UTF-8
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginMotionIOGetFunctionName(
    const nanoem_application_plugin_motion_io_t *plugin, int index);

/**
 * \brief Set the function index in the plugin
 *
 * The plugin implementation should handle out of index properly even nanoem doesn't occur
 *
 * \param plugin The opaque motion plugin object
 * \param index The corresponding function index
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetFunction(
    nanoem_application_plugin_motion_io_t *plugin, int index, nanoem_i32_t *status);

/**
 * \brief Set all selected bone keyframe indices corresponding name to the plugin
 *
 * The plugin implementation must copy \b name and \b data , retain to the instance due to \b data will be freed after
 * calling the function.
 *
 * The function will be called each all bones of the active model even selected keyframe is empty, then \b length will
 * be zero.
 *
 * \param plugin The opaque motion plugin object
 * \param name Current target name
 * \param frameIndices All selected bone keyframes corresponding \b name
 * \param length \b frameIndices count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetAllNamedSelectedBoneKeyframes(
    nanoem_application_plugin_motion_io_t *plugin, const char *name, const nanoem_frame_index_t *frameIndices,
    nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all selected morph keyframe indices corresponding name to the plugin
 *
 * The plugin implementation must copy \b name and \b data , retain to the instance due to \b data will be freed after
 * calling the function.
 *
 * The function will be called each all morphs of the active model even selected keyframe is empty, then \b length will
 * be zero.
 *
 * \param plugin The opaque motion plugin object
 * \param name Current target name
 * \param frameIndices All selected morph keyframes corresponding \b name
 * \param length \b frameIndices count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetAllNamedSelectedMorphKeyframes(
    nanoem_application_plugin_motion_io_t *plugin, const char *name, const nanoem_frame_index_t *frameIndices,
    nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all selected accessory keyframe indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even selected keyframe is empty, then \b length will be zero.
 *
 * \param plugin The opaque motion plugin object
 * \param frameIndices All selected accessory keyframes
 * \param length \b frameIndices count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetAllSelectedAccessoryKeyframes(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_frame_index_t *frameIndices, nanoem_u32_t length,
    nanoem_i32_t *status);

/**
 * \brief Set all selected camera keyframe indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even selected keyframe is empty, then \b length will be zero.
 *
 * \param plugin The opaque motion plugin object
 * \param frameIndices All selected camera keyframes
 * \param length \b frameIndices count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetAllSelectedCameraKeyframes(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_frame_index_t *frameIndices, nanoem_u32_t length,
    nanoem_i32_t *status);

/**
 * \brief Set all selected light keyframe indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even selected keyframe is empty, then \b length will be zero.
 *
 * \param plugin The opaque motion plugin object
 * \param frameIndices All selected light keyframes
 * \param length \b frameIndices count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetAllSelectedLightKeyframes(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_frame_index_t *frameIndices, nanoem_u32_t length,
    nanoem_i32_t *status);

/**
 * \brief Set all selected model keyframe indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even selected keyframe is empty, then \b length will be zero.
 *
 * \param plugin The opaque motion plugin object
 * \param frameIndices All selected model keyframes
 * \param length \b frameIndices count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetAllSelectedModelKeyframes(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_frame_index_t *frameIndices, nanoem_u32_t length,
    nanoem_i32_t *status);

/**
 * \brief Set all selected self shadow keyframe indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even selected keyframe is empty, then \b length will be zero.
 *
 * \param plugin The opaque motion plugin object
 * \param frameIndices All selected self shadow keyframes
 * \param length \b frameIndices count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetAllSelectedSelfShadowKeyframes(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_frame_index_t *frameIndices, nanoem_u32_t length,
    nanoem_i32_t *status);

/**
 * \brief Set audio description data
 *
 * Audio description data is encoded with \b nanoem.application.plugin.AudioDescription defined in
 * \e emapp/resources/protobuf/plugin.proto
 *
 * \param plugin The opaque motion plugin object
 * \param data Protocol buffer data serialized with \b nanoem.application.plugin.AudioDescription
 * \param length \b data size in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetAudioDescription(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set camera description data
 *
 * Camera description data is encoded with \b nanoem.application.plugin.CameraDescription defined in
 * \e emapp/resources/protobuf/plugin.proto
 *
 * \param plugin The opaque motion plugin object
 * \param data Protocol buffer data serialized with \b nanoem.application.plugin.CameraDescription
 * \param length \b data size in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetCameraDescription(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set light description data
 *
 * Light description data is encoded with \b nanoem.application.plugin.LightDescription defined in
 * \e emapp/resources/protobuf/plugin.proto
 *
 * \param plugin The opaque motion plugin object
 * \param data Protocol buffer data serialized with \b nanoem.application.plugin.LightDescription
 * \param length \b data size in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetLightDescription(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set current motion data
 *
 * Input motion data will be encoded with with \b nanoem.motion.Motion defined in \e nanoem/proto/motion.proto.
 *
 * If the motion is model motion, ::nanoemApplicationPluginMotionIOSetInputActiveModelData is also called.
 *
 * \param plugin The opaque motion plugin object
 * \param data NMD motion data
 * \param length \b data size in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetInputMotionData(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set current active model data
 *
 * Input model data will be encoded with PMX (UTF-8). If active model is not set (such as camera/light motion), the
 * function will not be called.
 *
 * \param plugin The opaque motion plugin object
 * \param data PMX model data encoded with UTF-8
 * \param length \b data size in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetInputActiveModelData(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set audio data
 *
 * Input audio data will be set PCM audio data and described in ::nanoemApplicationPluginMotionIOSetAudioDescription.
 *
 * \param plugin The opaque motion plugin object
 * \param data PCM audio data
 * \param length \b data size in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetInputAudioData(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Execute the plugin corresponding function
 *
 * \param plugin The opaque motion plugin object
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOExecute(
    nanoem_application_plugin_motion_io_t *plugin, nanoem_i32_t *status);

/**
 * \brief Get the executed output NMD motion data size
 *
 * \param plugin The opaque motion plugin object
 * \param[out] length NMD motion data size in byte unit
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOGetOutputMotionDataSize(
    nanoem_application_plugin_motion_io_t *plugin, nanoem_u32_t *length);

/**
 * \brief Get the executed output NMD motion data
 *
 * NMD motion data must be encoded with \b nanoem.motion.Motion defined in \e nanoem/proto/motion.proto
 *
 * The output NMD motion data must be valid. If loading its data failed, the plugin exection will be marked as failure
 * and the output motion data will not be loaded and discarded.
 *
 * \param plugin The opaque motion plugin object
 * \param data NMD motion data
 * \param length \b data size in byte unit  (same as ::nanoemApplicationPluginMotionIOGetOutputMotionDataSize)
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOGetOutputMotionData(
    nanoem_application_plugin_motion_io_t *plugin, nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Initialize and loads plugin UI window layout
 *
 * \param plugin The opaque motion plugin object
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOLoadUIWindowLayout(
    nanoem_application_plugin_motion_io_t *plugin, nanoem_i32_t *status);

/**
 * \brief Get plugin UI window layout data size
 *
 * \param plugin The opaque motion plugin object
 * \param[out] length plugin window layout data size in byte unit
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOGetUIWindowLayoutDataSize(
    nanoem_application_plugin_motion_io_t *plugin, nanoem_u32_t *length);

/**
 * \brief Get plugin UI window layout data
 *
 * UI window layout data must be encoded with \b nanoem.application.plugin.UIWindow defined in
 * \e emapp/resources/protobuf/plugin.proto
 *
 * \param plugin The opaque motion plugin object
 * \param data Protocol buffer data serialized with \b nanoem.application.plugin.UIWindow
 * \param length \b data in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOGetUIWindowLayoutData(
    nanoem_application_plugin_motion_io_t *plugin, nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set plugin UI window component data corresponding plugin UI component ID
 *
 * UI component data must be encoded with \b nanoem.application.plugin.UIComponent defined in
 * \e emapp/resources/protobuf/plugin.proto
 *
 * \param plugin The opaque motion plugin object
 * \param id ID to update component
 * \param data Protocol buffer data serialized with \b nanoem.application.plugin.UIComponent
 * \param length \b data in byte unit
 * \param[out] reloadLayout Whether UI window must be reloaded
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOSetUIComponentLayoutData(
    nanoem_application_plugin_motion_io_t *plugin, const char *id, const nanoem_u8_t *data, nanoem_u32_t length,
    int *reloadLayout, nanoem_i32_t *status);

/**
 * \brief Get error failure reason text
 *
 * The function should be able to get failure reason when \b NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON is set
 * in status.
 *
 * \param plugin The opaque motion plugin object
 * \return The error reason text to show it to the user via error dialog
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginMotionIOGetFailureReason(
    const nanoem_application_plugin_motion_io_t *plugin);

/**
 * \brief Get error recovery suggestion text
 *
 * The function should be able to get recovery suggestion to resolve the issue by user when \b
 * NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON is set in status.
 *
 * \param plugin The opaque motion plugin object
 * \return The recovery suggestion text to show it to the user via error dialog
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginMotionIOGetRecoverySuggestion(
    const nanoem_application_plugin_motion_io_t *plugin);

/**
 * \brief Destroy an opaque motion plugin object
 *
 * \param plugin The opaque motion plugin object
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIODestroy(nanoem_application_plugin_motion_io_t *plugin);

/**
 * \brief Terminate the plugin
 *
 * \since Motion I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginMotionIOTerminate(void);

#endif /* EMAPP_PLUGIN_SDK_MOTION_H_ */
