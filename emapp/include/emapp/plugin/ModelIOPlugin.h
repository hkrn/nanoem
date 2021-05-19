/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_PLUGIN_MODELIOPLUGIN_H_
#define NANOEM_EMAPP_PLUGIN_MODELIOPLUGIN_H_

#include "emapp/URI.h"
#include "emapp/plugin/BasePlugin.h"

struct nanoem_application_plugin_model_io_t;

namespace nanoem {
namespace plugin {

class ModelIOPlugin NANOEM_DECL_SEALED : public BasePlugin {
public:
    ModelIOPlugin(IEventPublisher *publisher);
    ~ModelIOPlugin() NANOEM_DECL_NOEXCEPT;

    bool load(const URI &fileURI) NANOEM_DECL_OVERRIDE;
    void unload() NANOEM_DECL_OVERRIDE;
    bool create() NANOEM_DECL_OVERRIDE;
    void destroy() NANOEM_DECL_OVERRIDE;

    void setLanguage(int value);
    const char *name() const NANOEM_DECL_NOEXCEPT;
    const char *description() const NANOEM_DECL_NOEXCEPT;
    const char *version() const NANOEM_DECL_NOEXCEPT;
    int countAllFunctions() const NANOEM_DECL_NOEXCEPT;
    const char *functionName(int value) NANOEM_DECL_NOEXCEPT;
    void setFunction(int value, Error &error);
    void setAllSelectedVertexObjectIndices(const int *indices, nanoem_rsize_t size, Error &error);
    void setAllSelectedMaterialObjectIndices(const int *indices, nanoem_rsize_t size, Error &error);
    void setAllSelectedBoneObjectIndices(const int *indices, nanoem_rsize_t size, Error &error);
    void setAllSelectedMorphObjectIndices(const int *indices, nanoem_rsize_t size, Error &error);
    void setAllSelectedLabelObjectIndices(const int *indices, nanoem_rsize_t size, Error &error);
    void setAllSelectedRigidBodyObjectIndices(const int *indices, nanoem_rsize_t size, Error &error);
    void setAllSelectedJointObjectIndices(const int *indices, nanoem_rsize_t size, Error &error);
    void setAllSelectedSoftBodyObjectIndices(const int *indices, nanoem_rsize_t size, Error &error);
    void setAllMaskedVertexObjectIndices(const int *indices, nanoem_rsize_t size, Error &error);
    void setAllMaskedMaterialObjectIndices(const int *indices, nanoem_rsize_t size, Error &error);
    void setAllMaskedBoneObjectIndices(const int *indices, nanoem_rsize_t size, Error &error);
    void setAllMaskedRigidBodyObjectIndices(const int *indices, nanoem_rsize_t size, Error &error);
    void setAllMaskedJointObjectIndices(const int *indices, nanoem_rsize_t size, Error &error);
    void setAllMaskedSoftBodyObjectIndices(const int *indices, nanoem_rsize_t size, Error &error);
    void setInputAudio(const IAudioPlayer *player, Error &error);
    void setInputCamera(const ICamera *camera, Error &error);
    void setInputLight(const ILight *light, Error &error);
    void setInputData(const ByteArray &bytes, Error &error);
    bool execute(Error &error);
    void getOutputData(ByteArray &bytes, Error &error);
    void getUIWindowLayout(ByteArray &bytes, Error &error);
    void setUIComponentLayout(const char *id, const ByteArray &bytes, bool &reloadLayout, Error &error);

    const char *failureReason() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const char *recoverySuggestion() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    typedef nanoem_u32_t(APIENTRY *PFN_nanoemApplicationPluginModelIOGetABIVersion)(void);
    typedef nanoem_u32_t(APIENTRY *PFN_nanoemApplicationPluginModelIOGetAPIVersion)(void);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOInitialize)(void);
    typedef nanoem_application_plugin_model_io_t *(APIENTRY *PFN_nanoemApplicationPluginModelIOCreate)(void);
    typedef nanoem_application_plugin_model_io_t *(APIENTRY *PFN_nanoemApplicationPluginModelIOCreateWithLocation)(
        const char *);
    typedef int(APIENTRY *PFN_nanoemApplicationPluginModelIOSetLanguage)(nanoem_application_plugin_model_io_t *, int);
    typedef const char *(APIENTRY *PFN_nanoemApplicationPluginModelIOGetName)(
        const nanoem_application_plugin_model_io_t *);
    typedef const char *(APIENTRY *PFN_nanoemApplicationPluginModelIOGetDescription)(
        const nanoem_application_plugin_model_io_t *);
    typedef const char *(APIENTRY *PFN_nanoemApplicationPluginModelIOGetVersion)(
        const nanoem_application_plugin_model_io_t *);
    typedef int(APIENTRY *PFN_nanoemApplicationPluginModelIOCountAllFunctions)(
        const nanoem_application_plugin_model_io_t *);
    typedef const char *(APIENTRY *PFN_nanoemApplicationPluginModelIOGetFunctionName)(
        const nanoem_application_plugin_model_io_t *, int);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetFunction)(
        nanoem_application_plugin_model_io_t *, int, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetAllSelectedVertexObjectIndices)(
        nanoem_application_plugin_model_io_t *, const int *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetAllSelectedMaterialObjectIndices)(
        nanoem_application_plugin_model_io_t *, const int *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetAllSelectedBoneObjectIndices)(
        nanoem_application_plugin_model_io_t *, const int *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetAllSelectedConstraintObjectIndices)(
        nanoem_application_plugin_model_io_t *, const int *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetAllSelectedMorphObjectIndices)(
        nanoem_application_plugin_model_io_t *, const int *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetAllSelectedLabelObjectIndices)(
        nanoem_application_plugin_model_io_t *, const int *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetAllSelectedRigidBodyObjectIndices)(
        nanoem_application_plugin_model_io_t *, const int *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetAllSelectedJointObjectIndices)(
        nanoem_application_plugin_model_io_t *, const int *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetAllSelectedSoftBodyObjectIndices)(
        nanoem_application_plugin_model_io_t *, const int *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetAllMaskedVertexObjectIndices)(
        nanoem_application_plugin_model_io_t *, const int *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetAllMaskedMaterialObjectIndices)(
        nanoem_application_plugin_model_io_t *, const int *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetAllMaskedBoneObjectIndices)(
        nanoem_application_plugin_model_io_t *, const int *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetAllMaskedRigidBodyObjectIndices)(
        nanoem_application_plugin_model_io_t *, const int *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetAllMaskedJointObjectIndices)(
        nanoem_application_plugin_model_io_t *, const int *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetAllMaskedSoftBodyObjectIndices)(
        nanoem_application_plugin_model_io_t *, const int *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetAudioDescription)(
        nanoem_application_plugin_model_io_t *, const nanoem_u8_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetCameraDescription)(
        nanoem_application_plugin_model_io_t *, const nanoem_u8_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetLightDescription)(
        nanoem_application_plugin_model_io_t *, const nanoem_u8_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetInputModelData)(
        nanoem_application_plugin_model_io_t *, const nanoem_u8_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOSetInputAudioData)(
        nanoem_application_plugin_model_io_t *, const nanoem_u8_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOExecute)(nanoem_application_plugin_model_io_t *, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOGetOutputModelDataSize)(
        nanoem_application_plugin_model_io_t *, nanoem_u32_t *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOGetOutputModelData)(
        nanoem_application_plugin_model_io_t *, nanoem_u8_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOLoadUIWindowLayout)(
        nanoem_application_plugin_model_io_t *, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOGetUIWindowLayoutDataSize)(
        nanoem_application_plugin_model_io_t *, nanoem_u32_t *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOGetUIWindowLayoutData)(
        nanoem_application_plugin_model_io_t *, nanoem_u8_t *, nanoem_u32_t, int *);
    typedef int(APIENTRY *PFN_nanoemApplicationPluginModelIOSetUIComponentLayoutData)(
        nanoem_application_plugin_model_io_t *, const char *, const nanoem_u8_t *, nanoem_u32_t, int *, int *);
    typedef const char *(APIENTRY *PFN_nanoemApplicationPluginModelIOGetFailureReason)(
        const nanoem_application_plugin_model_io_t *);
    typedef const char *(APIENTRY *PFN_nanoemApplicationPluginModelIOGetRecoverySuggestion)(
        const nanoem_application_plugin_model_io_t *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIODestroy)(nanoem_application_plugin_model_io_t *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginModelIOTerminate)(void);

    nanoem_application_plugin_model_io_t *m_modelIO;
    PFN_nanoemApplicationPluginModelIOInitialize _modelIOInitialize;
    PFN_nanoemApplicationPluginModelIOCreate _modelIOCreate;
    PFN_nanoemApplicationPluginModelIOCreateWithLocation _modelIOCreateWithLocation;
    PFN_nanoemApplicationPluginModelIOSetLanguage _modelIOSetLanguage;
    PFN_nanoemApplicationPluginModelIOGetName _modelIOGetName;
    PFN_nanoemApplicationPluginModelIOGetDescription _modelIOGetDescription;
    PFN_nanoemApplicationPluginModelIOGetVersion _modelIOGetVersion;
    PFN_nanoemApplicationPluginModelIOCountAllFunctions _modelIOCountAllFunctions;
    PFN_nanoemApplicationPluginModelIOGetFunctionName _modelIOGetFunctionName;
    PFN_nanoemApplicationPluginModelIOSetFunction _modelIOSetFunction;
    PFN_nanoemApplicationPluginModelIOSetAllSelectedVertexObjectIndices _modelIOSetAllSelectedVertexObjectIndices;
    PFN_nanoemApplicationPluginModelIOSetAllSelectedMaterialObjectIndices _modelIOSetAllSelectedMaterialObjectIndices;
    PFN_nanoemApplicationPluginModelIOSetAllSelectedBoneObjectIndices _modelIOSetAllSelectedBoneObjectIndices;
    PFN_nanoemApplicationPluginModelIOSetAllSelectedConstraintObjectIndices
        _modelIOSetAllSelectedConstraintObjectIndices;
    PFN_nanoemApplicationPluginModelIOSetAllSelectedMorphObjectIndices _modelIOSetAllSelectedMorphObjectIndices;
    PFN_nanoemApplicationPluginModelIOSetAllSelectedLabelObjectIndices _modelIOSetAllSelectedLabelObjectIndices;
    PFN_nanoemApplicationPluginModelIOSetAllSelectedRigidBodyObjectIndices _modelIOSetAllSelectedRigidBodyObjectIndices;
    PFN_nanoemApplicationPluginModelIOSetAllSelectedJointObjectIndices _modelIOSetAllSelectedJointObjectIndices;
    PFN_nanoemApplicationPluginModelIOSetAllSelectedSoftBodyObjectIndices _modelIOSetAllSelectedSoftBodyObjectIndices;
    PFN_nanoemApplicationPluginModelIOSetAllMaskedVertexObjectIndices _modelIOSetAllMaskedVertexObjectIndices;
    PFN_nanoemApplicationPluginModelIOSetAllMaskedMaterialObjectIndices _modelIOSetAllMaskedMaterialObjectIndices;
    PFN_nanoemApplicationPluginModelIOSetAllMaskedBoneObjectIndices _modelIOSetAllMaskedBoneObjectIndices;
    PFN_nanoemApplicationPluginModelIOSetAllMaskedRigidBodyObjectIndices _modelIOSetAllMaskedRigidBodyObjectIndices;
    PFN_nanoemApplicationPluginModelIOSetAllMaskedJointObjectIndices _modelIOSetAllMaskedJointObjectIndices;
    PFN_nanoemApplicationPluginModelIOSetAllMaskedSoftBodyObjectIndices _modelIOSetAllMaskedSoftBodyObjectIndices;
    PFN_nanoemApplicationPluginModelIOSetAudioDescription _modelIOSetAudioDescription;
    PFN_nanoemApplicationPluginModelIOSetCameraDescription _modelIOSetCameraDescription;
    PFN_nanoemApplicationPluginModelIOSetLightDescription _modelIOSetLightDescription;
    PFN_nanoemApplicationPluginModelIOSetInputAudioData _modelIOSetInputAudioData;
    PFN_nanoemApplicationPluginModelIOSetInputModelData _modelIOSetInputModelData;
    PFN_nanoemApplicationPluginModelIOExecute _modelIOExecute;
    PFN_nanoemApplicationPluginModelIOGetOutputModelDataSize _modelIOGetOutputModelDataSize;
    PFN_nanoemApplicationPluginModelIOGetOutputModelData _modelIOGetOutputModelData;
    PFN_nanoemApplicationPluginModelIOLoadUIWindowLayout _modelIOLoadUIWindowLayout;
    PFN_nanoemApplicationPluginModelIOGetUIWindowLayoutDataSize _modelIOGetUIWindowLayoutDataSize;
    PFN_nanoemApplicationPluginModelIOGetUIWindowLayoutData _modelIOGetUIWindowLayoutData;
    PFN_nanoemApplicationPluginModelIOSetUIComponentLayoutData _modelIOSetUIComponentLayoutData;
    PFN_nanoemApplicationPluginModelIOGetFailureReason _modelIOGetFailureReason;
    PFN_nanoemApplicationPluginModelIOGetRecoverySuggestion _modelIOGetRecoverySuggestion;
    PFN_nanoemApplicationPluginModelIODestroy _modelIODestroy;
    PFN_nanoemApplicationPluginModelIOTerminate _modelIOTerminate;
    URI m_fileURI;
};

} /* namespace plugin */
} /* namespace nanoem */

#endif /* NANOEM_PLUGIN_MODELIOPLUGIN_H_ */
