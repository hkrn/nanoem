/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef EMAPP_PLUGIN_SDK_MODEL_H_
#define EMAPP_PLUGIN_SDK_MODEL_H_

#define NANOEM_APPLICATION_PLUGIN_MODEL_ABI_VERSION_MAJOR 2
#define NANOEM_APPLICATION_PLUGIN_MODEL_ABI_VERSION_MINOR 1
#define NANOEM_APPLICATION_PLUGIN_MODEL_ABI_VERSION                                                                    \
    NANOEM_APPLICATION_PLUGIN_MAKE_ABI_VERSION(                                                                        \
        NANOEM_APPLICATION_PLUGIN_MODEL_ABI_VERSION_MAJOR, NANOEM_APPLICATION_PLUGIN_MODEL_ABI_VERSION_MINOR)

#include "Common.h"

typedef struct nanoem_application_plugin_model_io_t nanoem_application_plugin_model_io_t;

NANOEM_DECL_API nanoem_u32_t APIENTRY nanoemApplicationPluginModelIOGetABIVersion(void);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOInitialize(void);
NANOEM_DECL_API nanoem_application_plugin_model_io_t *APIENTRY nanoemApplicationPluginModelIOCreate(void);
NANOEM_DECL_API nanoem_application_plugin_model_io_t *APIENTRY nanoemApplicationPluginModelIOCreateWithLocation(
    const char *location);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetLanguage(
    nanoem_application_plugin_model_io_t *plugin, int value, nanoem_i32_t *status);
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginModelIOGetName(
    const nanoem_application_plugin_model_io_t *plugin);
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginModelIOGetDescription(
    const nanoem_application_plugin_model_io_t *plugin);
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginModelIOGetVersion(
    const nanoem_application_plugin_model_io_t *plugin);
NANOEM_DECL_API int APIENTRY nanoemApplicationPluginModelIOCountAllFunctions(
    const nanoem_application_plugin_model_io_t *plugin);
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginModelIOGetFunctionName(
    const nanoem_application_plugin_model_io_t *plugin, int index);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetFunction(
    nanoem_application_plugin_model_io_t *plugin, int index, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllSelectedVertexObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllSelectedMaterialObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllSelectedBoneObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllSelectedConstraintObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllSelectedMorphObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllSelectedLabelObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllSelectedRigidBodyObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllSelectedJointObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllSelectedSoftBodyObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllMaskedVertexObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllMaskedMaterialObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllMaskedBoneObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllMaskedConstraintObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllMaskedMorphObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllMaskedLabelObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllMaskedRigidBodyObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllMaskedJointObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAllMaskedSoftBodyObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetAudioDescription(
    nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetCameraDescription(
    nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetLightDescription(
    nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetInputModelData(
    nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetInputAudioData(
    nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOExecute(
    nanoem_application_plugin_model_io_t *plugin, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOGetOutputModelDataSize(
    nanoem_application_plugin_model_io_t *plugin, nanoem_u32_t *length);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOGetOutputModelData(
    nanoem_application_plugin_model_io_t *plugin, nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOLoadUIWindowLayout(
    nanoem_application_plugin_model_io_t *plugin, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOGetUIWindowLayoutDataSize(
    nanoem_application_plugin_model_io_t *plugin, nanoem_u32_t *length);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOGetUIWindowLayoutData(
    nanoem_application_plugin_model_io_t *plugin, nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOSetUIComponentLayoutData(
    nanoem_application_plugin_model_io_t *plugin, const char *id, const nanoem_u8_t *data, nanoem_u32_t length,
    int *reloadLayout, nanoem_i32_t *status);
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginModelIOGetFailureReason(
    const nanoem_application_plugin_model_io_t *plugin);
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginModelIOGetRecoverySuggestion(
    const nanoem_application_plugin_model_io_t *plugin);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIODestroy(nanoem_application_plugin_model_io_t *plugin);
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginModelIOTerminate(void);

#endif /* EMAPP_PLUGIN_SDK_MODEL_H_ */
