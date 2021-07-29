/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef EMAPP_PLUGIN_SDK_MODEL_H_
#define EMAPP_PLUGIN_SDK_MODEL_H_

#include "Common.h"

/**
 * \defgroup emapp
 * @{
 */

/**
 * \defgroup emapp_plugin_model_io nanoem Model I/O Plugin
 * @{
 */

#define NANOEM_APPLICATION_PLUGIN_MODEL_ABI_VERSION_MAJOR 2
#define NANOEM_APPLICATION_PLUGIN_MODEL_ABI_VERSION_MINOR 1
#define NANOEM_APPLICATION_PLUGIN_MODEL_ABI_VERSION                                                                    \
    NANOEM_APPLICATION_PLUGIN_MAKE_ABI_VERSION(                                                                        \
        NANOEM_APPLICATION_PLUGIN_MODEL_ABI_VERSION_MAJOR, NANOEM_APPLICATION_PLUGIN_MODEL_ABI_VERSION_MINOR)

/**
 * \brief The opaque model plugin object
 */
typedef struct nanoem_application_plugin_model_io_t nanoem_application_plugin_model_io_t;

/**
 * \brief Get plugin's ABI version
 *
 * Plugin ABI version consists major version and minor version.
 *
 * If the major version differs from nanoem runtime version, The plugin will not be loaded.
 * If the minor version is greater than nanoem runtime version, corresponding functions will not be called.
 *
 * \return The plugin ABI version made with \b NANOEM_APPLICATION_PLUGIN_MODEL_ABI_VERSION
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API nanoem_u32_t APIENTRY nanoemApplicationPluginModelIOGetABIVersion(void);

/**
 * \brief Initialize the plugin
 *
 * The function will call once at the plugin initialization.
 *
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOInitialize(void);

/**
 * \brief Create an opaque model plugin object
 *
 * \return The opaque model plugin object
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API nanoem_application_plugin_model_io_t *APIENTRY nanoemApplicationPluginModelIOCreate(void);

/**
 * \brief Create an opaque model plugin object with the plugin path
 *
 * If both ::nanoemApplicationPluginModelIOCreate and ::nanoemApplicationPluginModelIOCreateWithLocation are defined,
 * only ::nanoemApplicationPluginModelIOCreateWithLocation will be called.
 *
 * \param location The absolute path of the plugin canonicalized with slash
 * \return The opaque model plugin object
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API nanoem_application_plugin_model_io_t *APIENTRY nanoemApplicationPluginModelIOCreateWithLocation(
    const char *location);

/**
 * \brief Set current language value of the plugin
 *
 * If the function is not defined, assume the plugin as Japanese
 *
 * \param plugin The opaque model plugin object
 * \param value 0 for Japanese, 1 for English
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if
 * succeeded, otherwise sets the others \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetLanguage(
    nanoem_application_plugin_model_io_t *plugin, int value, nanoem_i32_t *status);

/**
 * \brief Get the plugin name corresponding current language
 *
 * If the function is not defined, constructs from the plugin filename.
 *
 * \param plugin The opaque model plugin object
 * \return The plugin name with UTF-8
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginModelIOGetName(
    const nanoem_application_plugin_model_io_t *plugin);

/**
 * \brief Get the plugin description corresponding current language
 *
 * \param plugin The opaque model plugin object
 * \return The plugin description with UTF-8
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginModelIOGetDescription(
    const nanoem_application_plugin_model_io_t *plugin);

/**
 * \brief Get the plugin version
 *
 * The plugin version string should be semantic version \link https://semver.org \endlink
 *
 * \param plugin The opaque model plugin object
 * \return The plugin version string with UTF-8
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginModelIOGetVersion(
    const nanoem_application_plugin_model_io_t *plugin);

/**
 * \brief Get number of all the plugin functions
 *
 * \param plugin The opaque model plugin object
 * \return number of all plugin functions
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API int APIENTRY nanoemApplicationPluginModelIOCountAllFunctions(
    const nanoem_application_plugin_model_io_t *plugin);

/**
 * \brief Get the function name corresponding current language and function index in the plugin
 *
 * \param plugin The opaque model plugin object
 * \param index The corresponding function index
 * \return The plugin function name with UTF-8
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginModelIOGetFunctionName(
    const nanoem_application_plugin_model_io_t *plugin, int index);

/**
 * \brief Set the function index in the plugin
 *
 * The plugin implementation should handle out of index properly even nanoem doesn't occur
 *
 * \param plugin The opaque model plugin object
 * \param index The corresponding function index
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetFunction(
    nanoem_application_plugin_model_io_t *plugin, int index, nanoem_i32_t *status);

/**
 * \brief Set all selected vertex object indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even selected object is empty, then \b length will be zero.
 *
 * \param plugin The opaque model plugin object
 * \param data All selected vertex object indices
 * \param length \b data count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllSelectedVertexObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all selected material object indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even selected object is empty, then \b length will be zero.
 *
 * \param plugin The opaque model plugin object
 * \param data All selected material object indices
 * \param length \b data count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllSelectedMaterialObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all selected bone object indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even selected object is empty, then \b length will be zero.
 *
 * \param plugin The opaque model plugin object
 * \param data All selected bone object indices
 * \param length \b data count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllSelectedBoneObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all selected constraint object indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even selected object is empty, then \b length will be zero.
 *
 * \param plugin The opaque model plugin object
 * \param data All selected constraint object indices
 * \param length \b data count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllSelectedConstraintObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all selected morph object indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even selected object is empty, then \b length will be zero.
 *
 * \param plugin The opaque model plugin object
 * \param data All selected morph object indices
 * \param length \b data count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllSelectedMorphObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all selected label object indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even selected object is empty, then \b length will be zero.
 *
 * \param plugin The opaque model plugin object
 * \param data All selected label object indices
 * \param length \b data count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllSelectedLabelObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all selected rigid body object indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even selected object is empty, then \b length will be zero.
 *
 * \param plugin The opaque model plugin object
 * \param data All selected rigid body object indices
 * \param length \b data count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllSelectedRigidBodyObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all selected joint object indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even selected object is empty, then \b length will be zero.
 *
 * \param plugin The opaque model plugin object
 * \param data All selected joint object indices
 * \param length \b data count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllSelectedJointObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all selected soft body object indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even selected object is empty, then \b length will be zero.
 *
 * \param plugin The opaque model plugin object
 * \param data All selected soft body object indices
 * \param length \b data count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.1
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllSelectedSoftBodyObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all masked vertex object indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even masked object is empty, then \b length will be zero.
 *
 * \param plugin The opaque model plugin object
 * \param data All masked vertex object indices
 * \param length \b data count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.1
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllMaskedVertexObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all mask material object indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even masked object is empty, then \b length will be zero.
 *
 * \param plugin The opaque model plugin object
 * \param data All masked material object indices
 * \param length \b data count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.1
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllMaskedMaterialObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all masked bone object indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even masked object is empty, then \b length will be zero.
 *
 * \param plugin The opaque model plugin object
 * \param data All masked bone object indices
 * \param length \b data count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.1
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllMaskedBoneObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all masked constraint object indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even masked object is empty, then \b length will be zero.
 *
 * \param plugin The opaque model plugin object
 * \param data All masked constraint object indices
 * \param length \b data count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.1
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllMaskedConstraintObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all masked morph object indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even masked object is empty, then \b length will be zero.
 *
 * \param plugin The opaque model plugin object
 * \param data All masked morph object indices
 * \param length \b data count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.1
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllMaskedMorphObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all masked label object indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even masked object is empty, then \b length will be zero.
 *
 * \param plugin The opaque model plugin object
 * \param data All masked label object indices
 * \param length \b data count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.1
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllMaskedLabelObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all masked rigid body object indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even masked object is empty, then \b length will be zero.
 *
 * \param plugin The opaque model plugin object
 * \param data All masked rigid body object indices
 * \param length \b data count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.1
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllMaskedRigidBodyObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all masked joint object indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even masked object is empty, then \b length will be zero.
 *
 * \param plugin The opaque model plugin object
 * \param data All masked joint object indices
 * \param length \b data count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.1
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllMaskedJointObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set all masked soft body object indices to the plugin
 *
 * The plugin implementation must copy \b data and retain to the instance due to \b data will be freed after calling the
 * function.
 *
 * The function will be called even masked object is empty, then \b length will be zero.
 *
 * \param plugin The opaque model plugin object
 * \param data All masked soft body object indices
 * \param length \b data count
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.1
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllMaskedSoftBodyObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set whether model is editing
 *
 * \param plugin The opaque model plugin object
 * \param value boolean value
 * \since Model I/O Plugin ABI 2.1
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetEditingModeEnabled(
    nanoem_application_plugin_model_io_t *plugin, int value);

/**
 * \brief Set audio description data
 *
 * Audio description data is encoded with \b nanoem.application.plugin.AudioDescription defined in
 * \e emapp/resources/protobuf/plugin.proto
 *
 * \param plugin The opaque model plugin object
 * \param data Protocol buffer data serialized with \b nanoem.application.plugin.AudioDescription
 * \param length \b data size in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAudioDescription(
    nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set camera description data
 *
 * Camera description data is encoded with \b nanoem.application.plugin.CameraDescription defined in
 * \e emapp/resources/protobuf/plugin.proto
 *
 * \param plugin The opaque model plugin object
 * \param data Protocol buffer data serialized with \b nanoem.application.plugin.CameraDescription
 * \param length \b data size in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetCameraDescription(
    nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set light description data
 *
 * Light description data is encoded with \b nanoem.application.plugin.LightDescription defined in
 * \e emapp/resources/protobuf/plugin.proto
 *
 * \param plugin The opaque model plugin object
 * \param data Protocol buffer data serialized with \b nanoem.application.plugin.LightDescription
 * \param length \b data size in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetLightDescription(
    nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set model data
 *
 * Input model data will be encoded with PMX (UTF-8).
 *
 * \param plugin The opaque model plugin object
 * \param data PMX model data
 * \param length \b data size in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetInputModelData(
    nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set audio data
 *
 * Input audio data will be set PCM audio data and described in ::nanoemApplicationPluginModelIOSetAudioDescription.
 *
 * \param plugin The opaque model plugin object
 * \param data PCM audio data
 * \param length \b data size in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetInputAudioData(
    nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Execute the plugin corresponding function
 *
 * \param plugin The opaque model plugin object
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOExecute(
    nanoem_application_plugin_model_io_t *plugin, nanoem_i32_t *status);

/**
 * \brief Get the executed output PMX model data size
 *
 * \param plugin The opaque model plugin object
 * \param[out] length PMX model data size in byte unit
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOGetOutputModelDataSize(
    nanoem_application_plugin_model_io_t *plugin, nanoem_u32_t *length);

/**
 * \brief Get the executed output PMX model data
 *
 * The output PMX model data must be valid. If loading its data failed, the plugin exection will be marked as failure
 * and the output model data will not be loaded.
 *
 * \param plugin The opaque model plugin object
 * \param data PMX model data
 * \param length \b data size in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOGetOutputModelData(
    nanoem_application_plugin_model_io_t *plugin, nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Initialize and loads plugin UI window layout
 *
 * \param plugin The opaque model plugin object
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOLoadUIWindowLayout(
    nanoem_application_plugin_model_io_t *plugin, nanoem_i32_t *status);

/**
 * \brief Get plugin UI window layout data size
 *
 * \param plugin The opaque model plugin object
 * \param[out] length plugin window layout data size in byte unit
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOGetUIWindowLayoutDataSize(
    nanoem_application_plugin_model_io_t *plugin, nanoem_u32_t *length);

/**
 * \brief Get plugin UI window layout data
 *
 * \param plugin The opaque model plugin object
 * \param data Protocol buffer data serialized with \b nanoem.application.plugin.UIWindow
 * \param length \b data in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOGetUIWindowLayoutData(
    nanoem_application_plugin_model_io_t *plugin, nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);

/**
 * \brief Set plugin UI window component data corresponding plugin UI component ID
 *
 * \param plugin The opaque model plugin object
 * \param id ID to update component
 * \param data Protocol buffer data serialized with \b nanoem.application.plugin.UIComponent
 * \param length \b data in byte unit
 * \param[out] reloadLayout Whether UI window must be reloaded
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetUIComponentLayoutData(
    nanoem_application_plugin_model_io_t *plugin, const char *id, const nanoem_u8_t *data, nanoem_u32_t length,
    int *reloadLayout, nanoem_i32_t *status);

/**
 * \brief Get error failure reason text
 *
 * The function should be able to get failure reason when \b NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON is set
 * in status.
 *
 * \param plugin The opaque model plugin object
 * \return NANOEM_DECL_API const char* APIENTRY
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginModelIOGetFailureReason(
    const nanoem_application_plugin_model_io_t *plugin);

/**
 * \brief Get error recovery suggestion text
 *
 * The function should be able to get recovery suggestion to resolve the issue by user when \b
 * NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON is set in status.
 *
 * \param plugin The opaque model plugin object
 * \return NANOEM_DECL_API const char* APIENTRY
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginModelIOGetRecoverySuggestion(
    const nanoem_application_plugin_model_io_t *plugin);

/**
 * \brief Destroy an opaque model plugin object
 *
 * \param plugin The opaque model plugin object
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIODestroy(nanoem_application_plugin_model_io_t *plugin);

/**
 * \brief Terminate the plugin
 *
 * \since Model I/O Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOTerminate(void);

/** @} */

/** @} */

#endif /* EMAPP_PLUGIN_SDK_MODEL_H_ */
