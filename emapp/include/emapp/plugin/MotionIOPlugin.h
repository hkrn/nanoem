/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_PLUGIN_MOTIONIOPLUGIN_H_
#define NANOEM_EMAPP_PLUGIN_MOTIONIOPLUGIN_H_

#include "emapp/URI.h"
#include "emapp/plugin/BasePlugin.h"

struct nanoem_application_plugin_motion_io_t;

namespace nanoem {

class Model;

namespace plugin {

class MotionIOPlugin NANOEM_DECL_SEALED : public BasePlugin {
public:
    MotionIOPlugin(IEventPublisher *publisher);
    ~MotionIOPlugin() NANOEM_DECL_NOEXCEPT;

    bool load(const URI &fileURI) NANOEM_DECL_OVERRIDE;
    void unload() NANOEM_DECL_OVERRIDE;
    bool create() NANOEM_DECL_OVERRIDE;
    void destroy() NANOEM_DECL_OVERRIDE;

    void setLanguage(int value);
    const char *name() const NANOEM_DECL_NOEXCEPT;
    const char *description() const NANOEM_DECL_NOEXCEPT;
    const char *version() const NANOEM_DECL_NOEXCEPT;
    int countAllFunctions() const NANOEM_DECL_NOEXCEPT;
    const char *functionName(int value) const NANOEM_DECL_NOEXCEPT;
    void setFunction(int value, Error &error);
    void setAllNamedSelectedBoneKeyframes(
        const char *name, const nanoem_frame_index_t *values, nanoem_rsize_t size, Error &error);
    void setAllNamedSelectedMorphKeyframes(
        const char *name, const nanoem_frame_index_t *values, nanoem_rsize_t size, Error &error);
    void setAllSelectedAccessoryKeyframes(const nanoem_frame_index_t *values, nanoem_rsize_t size, Error &error);
    void setAllSelectedCameraKeyframes(const nanoem_frame_index_t *values, nanoem_rsize_t size, Error &error);
    void setAllSelectedLightKeyframes(const nanoem_frame_index_t *values, nanoem_rsize_t size, Error &error);
    void setAllSelectedModelKeyframes(const nanoem_frame_index_t *values, nanoem_rsize_t size, Error &error);
    void setAllSelectedSelfShadowKeyframes(const nanoem_frame_index_t *values, nanoem_rsize_t size, Error &error);
    void setInputAudio(const IAudioPlayer *player, Error &error);
    void setInputCamera(const ICamera *camera, Error &error);
    void setInputLight(const ILight *light, Error &error);
    void setInputActiveModel(Model *activeModel, Error &error);
    void setInputMotionData(const ByteArray &bytes, Error &error);
    bool execute(Error &error);
    void getOutputData(ByteArray &bytes, Error &error);
    void getUIWindowLayout(ByteArray &bytes, Error &error);
    void setUIComponentLayout(const char *id, const ByteArray &bytes, bool &reloadLayout, Error &error);

    const char *failureReason() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const char *recoverySuggestion() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    typedef nanoem_u32_t(APIENTRY *PFN_nanoemApplicationPluginMotionIOGetABIVersion)(void);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOInitialize)(void);
    typedef nanoem_application_plugin_motion_io_t *(APIENTRY *PFN_nanoemApplicationPluginMotionIOCreate)(void);
    typedef nanoem_application_plugin_motion_io_t *(APIENTRY *PFN_nanoemApplicationPluginMotionIOCreateWithLocation)(
        const char *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOSetLanguage)(
        nanoem_application_plugin_motion_io_t *, int);
    typedef const char *(APIENTRY *PFN_nanoemApplicationPluginMotionIOGetName)(
        const nanoem_application_plugin_motion_io_t *);
    typedef const char *(APIENTRY *PFN_nanoemApplicationPluginMotionIOGetDescription)(
        const nanoem_application_plugin_motion_io_t *);
    typedef const char *(APIENTRY *PFN_nanoemApplicationPluginMotionIOGetVersion)(
        const nanoem_application_plugin_motion_io_t *);
    typedef int(APIENTRY *PFN_nanoemApplicationPluginMotionIOCountAllFunctions)(
        const nanoem_application_plugin_motion_io_t *);
    typedef const char *(APIENTRY *PFN_nanoemApplicationPluginMotionIOGetFunctionName)(
        const nanoem_application_plugin_motion_io_t *, int);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOSetFunction)(
        nanoem_application_plugin_motion_io_t *, int, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOSetAllNamedSelectedBoneKeyframes)(
        nanoem_application_plugin_motion_io_t *, const char *, const nanoem_frame_index_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOSetAllNamedSelectedMorphKeyframes)(
        nanoem_application_plugin_motion_io_t *, const char *, const nanoem_frame_index_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOSetAllSelectedAccessoryKeyframes)(
        nanoem_application_plugin_motion_io_t *, const nanoem_frame_index_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOSetAllSelectedCameraKeyframes)(
        nanoem_application_plugin_motion_io_t *, const nanoem_frame_index_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOSetAllSelectedLightKeyframes)(
        nanoem_application_plugin_motion_io_t *, const nanoem_frame_index_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOSetAllSelectedModelKeyframes)(
        nanoem_application_plugin_motion_io_t *, const nanoem_frame_index_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOSetAllSelectedSelfShadowKeyframes)(
        nanoem_application_plugin_motion_io_t *, const nanoem_frame_index_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOSetAudioDescription)(
        nanoem_application_plugin_motion_io_t *, const nanoem_u8_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOSetCameraDescription)(
        nanoem_application_plugin_motion_io_t *, const nanoem_u8_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOSetLightDescription)(
        nanoem_application_plugin_motion_io_t *, const nanoem_u8_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOSetInputMotionData)(
        nanoem_application_plugin_motion_io_t *, const nanoem_u8_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOSetInputActiveModelData)(
        nanoem_application_plugin_motion_io_t *, const nanoem_u8_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOSetInputAudioData)(
        nanoem_application_plugin_motion_io_t *, const nanoem_u8_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOExecute)(nanoem_application_plugin_motion_io_t *, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOGetOutputMotionDataSize)(
        nanoem_application_plugin_motion_io_t *, nanoem_u32_t *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOGetOutputMotionData)(
        nanoem_application_plugin_motion_io_t *, nanoem_u8_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOLoadUIWindowLayout)(
        nanoem_application_plugin_motion_io_t *, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOGetUIWindowLayoutDataSize)(
        nanoem_application_plugin_motion_io_t *, nanoem_u32_t *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOGetUIWindowLayoutData)(
        nanoem_application_plugin_motion_io_t *, nanoem_u8_t *, nanoem_u32_t, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOSetUIComponentLayoutData)(
        nanoem_application_plugin_motion_io_t *, const char *, const nanoem_u8_t *, nanoem_u32_t, int *, int *);
    typedef const char *(APIENTRY *PFN_nanoemApplicationPluginMotionIOGetFailureReason)(
        const nanoem_application_plugin_motion_io_t *);
    typedef const char *(APIENTRY *PFN_nanoemApplicationPluginMotionIOGetRecoverySuggestion)(
        const nanoem_application_plugin_motion_io_t *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIODestroy)(nanoem_application_plugin_motion_io_t *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginMotionIOTerminate)(void);

    nanoem_application_plugin_motion_io_t *m_motionIO;
    PFN_nanoemApplicationPluginMotionIOInitialize _motionIOInitialize;
    PFN_nanoemApplicationPluginMotionIOCreate _motionIOCreate;
    PFN_nanoemApplicationPluginMotionIOCreateWithLocation _motionIOCreateWithLocation;
    PFN_nanoemApplicationPluginMotionIOSetLanguage _motionIOSetLanguage;
    PFN_nanoemApplicationPluginMotionIOGetName _motionIOGetName;
    PFN_nanoemApplicationPluginMotionIOGetDescription _motionIOGetDescription;
    PFN_nanoemApplicationPluginMotionIOGetVersion _motionIOGetVersion;
    PFN_nanoemApplicationPluginMotionIOCountAllFunctions _motionIOCountAllFunctions;
    PFN_nanoemApplicationPluginMotionIOGetFunctionName _motionIOGetFunctionName;
    PFN_nanoemApplicationPluginMotionIOSetFunction _motionIOSetFunction;
    PFN_nanoemApplicationPluginMotionIOSetAllNamedSelectedBoneKeyframes _motionIOSetAllNamedSelectedBoneKeyframes;
    PFN_nanoemApplicationPluginMotionIOSetAllNamedSelectedMorphKeyframes _motionIOSetAllNamedSelectedMorphKeyframes;
    PFN_nanoemApplicationPluginMotionIOSetAllSelectedAccessoryKeyframes _motionIOSetAllSelectedAccessoryKeyframes;
    PFN_nanoemApplicationPluginMotionIOSetAllSelectedCameraKeyframes _motionIOSetAllSelectedCameraKeyframes;
    PFN_nanoemApplicationPluginMotionIOSetAllSelectedLightKeyframes _motionIOSetAllSelectedLightKeyframes;
    PFN_nanoemApplicationPluginMotionIOSetAllSelectedModelKeyframes _motionIOSetAllSelectedModelKeyframes;
    PFN_nanoemApplicationPluginMotionIOSetAllSelectedSelfShadowKeyframes _motionIOSetAllSelectedSelfShadowKeyframes;
    PFN_nanoemApplicationPluginMotionIOSetAudioDescription _motionIOSetAudioDescription;
    PFN_nanoemApplicationPluginMotionIOSetCameraDescription _motionIOSetCameraDescription;
    PFN_nanoemApplicationPluginMotionIOSetLightDescription _motionIOSetLightDescription;
    PFN_nanoemApplicationPluginMotionIOSetInputMotionData _motionIOSetInputMotionData;
    PFN_nanoemApplicationPluginMotionIOSetInputActiveModelData _motionIOSetInputActiveModelData;
    PFN_nanoemApplicationPluginMotionIOSetInputAudioData _motionIOSetInputAudioData;
    PFN_nanoemApplicationPluginMotionIOExecute _motionIOExecute;
    PFN_nanoemApplicationPluginMotionIOLoadUIWindowLayout _motionIOLoadUIWindowLayout;
    PFN_nanoemApplicationPluginMotionIOGetOutputMotionDataSize _motionIOGetOutputMotionDataSize;
    PFN_nanoemApplicationPluginMotionIOGetOutputMotionData _motionIOGetOutputMotionData;
    PFN_nanoemApplicationPluginMotionIOGetUIWindowLayoutDataSize _motionIOGetUIWindowLayoutDataSize;
    PFN_nanoemApplicationPluginMotionIOGetUIWindowLayoutData _motionIOGetUIWindowLayoutData;
    PFN_nanoemApplicationPluginMotionIOSetUIComponentLayoutData _motionIOSetUIComponentLayoutData;
    PFN_nanoemApplicationPluginMotionIOGetFailureReason _motionIOGetFailureReason;
    PFN_nanoemApplicationPluginMotionIOGetRecoverySuggestion _motionIOGetRecoverySuggestion;
    PFN_nanoemApplicationPluginMotionIODestroy _motionIODestroy;
    PFN_nanoemApplicationPluginMotionIOTerminate _motionIOTerminate;
    URI m_fileURI;
};

} /* namespace plugin */
} /* namespace nanoem */

#endif /* NANOEM_PLUGIN_MOTIONIOPLUGIN_H_ */
