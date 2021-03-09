/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_BASEAPPLICATIONCLIENT_H_
#define NANOEM_EMAPP_BASEAPPLICATIONCLIENT_H_

#include "emapp/URI.h"

typedef struct _Nanoem__Application__Command Nanoem__Application__Command;

namespace nanoem {

class BaseApplicationClient : private NonCopyable {
public:
    struct InitializeMessageDescription {
        InitializeMessageDescription(const Vector2UI32 &logicalWindowSize, sg_pixel_format pixelFormat,
            nanoem_f32_t windowDevicePixelRatio, const String &sokolPath);
        nanoem_u32_t m_vendorId;
        nanoem_u32_t m_deviceId;
        nanoem_u32_t m_rendererType;
        Vector2UI32 m_logicalWindowSize;
        nanoem_f32_t m_windowDevicePixelRatio;
        nanoem_f32_t m_viewportDevicePixelRatio;
        nanoem_u32_t m_preferredEditingFPS;
        String m_sokolDllPath;
        nanoem_u16_t m_bufferPoolSize;
        nanoem_u16_t m_imagePoolSize;
        nanoem_u16_t m_shaderPoolSize;
        nanoem_u16_t m_pipelinePoolSize;
        nanoem_u16_t m_passPoolSize;
        nanoem_u32_t m_metalGlobalUniformBufferSize;
        sg_pixel_format m_pixelFormat;
    };
    struct DrawableItem {
        nanoem_u32_t m_handle;
        String m_name;
        bool m_active;
    };
    struct PluginItem {
        String m_name;
        String m_description;
        StringList m_functions;
    };
    typedef tinystl::vector<DrawableItem, TinySTLAllocator> DrawableItemList;
    typedef tinystl::vector<PluginItem, TinySTLAllocator> PluginItemList;

    typedef void (*pfn_handleTrackEvent)(
        void *userData, const char *screen, const char *category, const char *action, const char *label);
    typedef void (*pfn_handleUndoEvent)(void *userData, bool canUndo, bool canRedo);
    typedef void (*pfn_handleRedoEvent)(void *userData, bool canRedo, bool canUndo);
    typedef void (*pfn_handleUndoChangeEvent)(void *userData);
    typedef void (*pfn_handleAddModelEvent)(void *userData, nanoem_u16_t handle, const char *name);
    typedef void (*pfn_handleSetActiveModelEvent)(void *userData, nanoem_u16_t handle, const char *name);
    typedef void (*pfn_handleRemoveModelEvent)(void *userData, nanoem_u16_t handle, const char *name);
    typedef void (*pfn_handleAddAccessoryEvent)(void *userData, nanoem_u16_t handle, const char *name);
    typedef void (*pfn_handleSetActiveAccessoryEvent)(void *userData, nanoem_u16_t handle, const char *name);
    typedef void (*pfn_handleRemoveAccessoryEvent)(void *userData, nanoem_u16_t handle, const char *name);
    typedef void (*pfn_handlePlayEvent)(
        void *userData, nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex);
    typedef void (*pfn_handleStopEvent)(
        void *userData, nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex);
    typedef void (*pfn_handlePauseEvent)(
        void *userData, nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex);
    typedef void (*pfn_handleResumeEvent)(
        void *userData, nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex);
    typedef void (*pfn_handleSeekEvent)(void *userData, nanoem_frame_index_t duration,
        nanoem_frame_index_t localFrameIndexTo, nanoem_frame_index_t localFrameIndexFrom);
    typedef void (*pfn_handleUpdateDurationEvent)(
        void *userData, nanoem_frame_index_t currentDuration, nanoem_frame_index_t lastDuration);
    typedef void (*pfn_handleSaveProjectAfterConfirmEvent)(void *userData);
    typedef void (*pfn_handleDiscardProjectAfterConfirmEvent)(void *userData);
    typedef void (*pfn_handleToggleProjectEffectEnabledEvent)(void *userData, bool value);
    typedef void (*pfn_handleToggleProjectGroundShadowEnabledEvent)(void *userData, bool value);
    typedef void (*pfn_handleToggleProjectVertexShaderSkinningEnabledEvent)(void *userData, bool value);
    typedef void (*pfn_handleToggleProjectComputeShaderSkinningEnabledEvent)(void *userData, bool value);
    typedef void (*pfn_handleSetProjectSampleLevelEvent)(void *userData, nanoem_u32_t value);
    typedef void (*pfn_handleToggleGridEnabledEvent)(void *userData, bool value);
    typedef void (*pfn_handleSetGridCellEvent)(void *userData, const Vector2 &value);
    typedef void (*pfn_handleSetGridSizeEvent)(void *userData, const Vector2 &value);
    typedef void (*pfn_handleSetPreferredMotionFPSEvent)(void *userData, nanoem_u32_t value, bool unlimited);
    typedef void (*pfn_handleSetPhysicsSimulationModeEvent)(void *userData, nanoem_u32_t value);
    typedef void (*pfn_handleSetPhysicsSimulationEngineDebugFlagEvent)(void *userData, nanoem_u32_t value);
    typedef void (*pfn_handleToggleShadowMapEnabledEvent)(void *userData, bool value);
    typedef void (*pfn_handleSetShadowMapModeEvent)(void *userData, nanoem_u32_t value);
    typedef void (*pfn_handleSetShadowMapDistanceEvent)(void *userData, nanoem_f32_t value);
    typedef void (*pfn_handleToggleActiveModelAddBlendEnabledEvent)(void *userData, bool value);
    typedef void (*pfn_handleToggleActiveModelShadowMapEnabledEvent)(void *userData, bool value);
    typedef void (*pfn_handleToggleActiveModelVisibleEvent)(void *userData, bool value);
    typedef void (*pfn_handleToggleActiveModelComputeShaderSkinningEnabledEvent)(void *userData, bool value);
    typedef void (*pfn_handleToggleActiveModelShowAllBonesEvent)(void *userData, bool value);
    typedef void (*pfn_handleToggleActiveModelShowAllRigidBodiesEvent)(void *userData, bool value);
    typedef void (*pfn_handleToggleActiveModelShowAllVertexFacesEvent)(void *userData, bool value);
    typedef void (*pfn_handleToggleActiveModelShowAllVertexPointsEvent)(void *userData, bool value);
    typedef void (*pfn_handleToggleActiveModelVertexShaderSkinningEnabledEvent)(void *userData, bool value);
    typedef void (*pfn_handleAvailableAllImportingAudioExtensionsEvent)(void *userData, const StringList &extensions);
    typedef void (*pfn_handleAvailableAllImportingVideoExtensionsEvent)(void *userData, const StringList &extensions);
    typedef void (*pfn_handleAvailableAllExportingImageExtensionsEvent)(void *userData, const StringList &extensions);
    typedef void (*pfn_handleAvailableAllExportingVideoExtensionsEvent)(void *userData, const StringList &extensions);
    typedef void (*pfn_handleInitializationCompleteEvent)(void *userData);
    typedef void (*pfn_handleCompleteDestructionEvent)(void *userData);
    typedef void (*pfn_handleCompleteTerminationEvent)(void *userData);
    typedef void (*pfn_handleDisableCursorEvent)(void *userData, const Vector2SI32 &value);
    typedef void (*pfn_handleEnableCursorEvent)(void *userData, const Vector2SI32 &value);
    typedef void (*pfn_handleErrorEvent)(void *userData, int code, const char *reason, const char *recoverySuggestion);
    typedef void (*pfn_handleStartRecordingViewportPassEvent)(void *userData, const Vector2UI32 &value);
    typedef void (*pfn_handleStopRecordingViewportPassEvent)(void *userData);
    typedef void (*pfn_handleCompleteLoadingFileEvent)(
        void *userData, const URI &fileURI, nanoem_u32_t type, nanoem_u64_t ticks);
    typedef void (*pfn_handleCompleteSavingFileEvent)(
        void *userData, const URI &fileURI, nanoem_u32_t type, nanoem_u64_t ticks);
    typedef void (*pfn_handleSetPreferredEditingFPSEvent)(void *userData, nanoem_u32_t value);
    typedef void (*pfn_handleConsumePassEvent)(void *userData, nanoem_u64_t value);
    typedef void (*pfn_handleAddModalDialogEvent)(void *userData);
    typedef void (*pfn_handleClearModalDialogEvent)(void *userData);
    typedef void (*pfn_handleSetLanguageEvent)(void *userData, nanoem_u32_t value);
    typedef void (*pfn_handleToggleProjectPlayingWithLoopEnabledEvent)(void *userData, bool value);
    typedef void (*pfn_handleToggleActiveAccessoryAddBlendEnabledEvent)(void *userData, bool value);
    typedef void (*pfn_handleToggleActiveAccessoryShadowEnabledEvent)(void *userData, bool value);
    typedef void (*pfn_handleToggleActiveAccessoryVisibleEvent)(void *userData, bool value);
    typedef void (*pfn_handleUpdateProgressEvent)(
        void *userData, nanoem_u32_t value, nanoem_u32_t total, nanoem_u32_t type, const char *text);
    typedef void (*pfn_handleStartProgressEvent)(
        void *userData, const char *title, const char *text, nanoem_u32_t total);
    typedef void (*pfn_handleStopProgressEvent)(void *userData);
    typedef void (*pfn_handleSetupProjectEvent)(void *userData, const Vector2 &windowSize,
        nanoem_f32_t windowDevicePixelRatio, nanoem_f32_t viewportDevicePixelRatio);
    typedef void (*pfn_handleSetEditingModeEvent)(void *userData, nanoem_u32_t value);
    typedef void (*pfn_handleCompleteLoadingAllModelPluginsEvent)(void *userData, const PluginItemList &plugins);
    typedef void (*pfn_handleCompleteLoadingAllMotionPluginsEvent)(void *userData, const PluginItemList &plugins);
    typedef void (*pfn_handleCompleteExportImageConfigurationEvent)(
        void *userData, const StringList &availableExtensions);
    typedef void (*pfn_handleCompleteExportVideoConfigurationEvent)(
        void *userData, const StringList &availableExtensions);
    typedef void (*pfn_handleQueryOpenSingleFileDialogEvent)(
        void *userData, nanoem_u32_t value, const StringList &allowedExtensions);
    typedef void (*pfn_handleQueryOpenMultipleFilesDialogEvent)(
        void *userData, nanoem_u32_t value, const StringList &allowedExtensions);
    typedef void (*pfn_handleQuerySaveFileDialogEvent)(
        void *userData, nanoem_u32_t value, const StringList &allowedExtensions);
    typedef void (*pfn_handleCanCopyEvent)(void *userData, bool value);
    typedef void (*pfn_handleCanPasteEvent)(void *userData, bool value);
    typedef void (*pfn_handleSetWindowDevicePixelRatioEvent)(void *userData, nanoem_f32_t value);
    typedef void (*pfn_handleSetViewportDevicePixelRatioEvent)(void *userData, nanoem_f32_t value);
    typedef void (*pfn_isProjectDirtyCallback)(void *userData, bool value);
    typedef void (*pfn_getProjectFileURICallback)(void *userData, const URI &value);
    typedef void (*pfn_getAllModelBonesCallback)(void *userData, nanoem_u16_t handle, const StringList &values);
    typedef void (*pfn_getAllModelMorphsCallback)(void *userData, nanoem_u16_t handle, const StringList &eyes,
        const StringList &eyebrows, const StringList &lips, const StringList &others);
    typedef void (*pfn_getBackgroundTextureImageHandleCallback)(void *userData, nanoem_u32_t handle);
    typedef void (*pfn_getAllAccessoriesCallback)(void *userData, const DrawableItemList &items);
    typedef void (*pfn_getAllModelsCallback)(void *userData, const DrawableItemList &items);
    typedef void (*pfn_getHandleFileURICallback)(void *userData, nanoem_u16_t handle, const URI &value);

    BaseApplicationClient();
    virtual ~BaseApplicationClient() NANOEM_DECL_NOEXCEPT;

    void sendInitializeMessage(const InitializeMessageDescription &desc);
    void sendRenderFrameMessage();
    void sendActivateMessage();
    void sendDeactivateMessage();
    void sendResizeWindowMessage(const Vector2UI32 &size);
    void sendResizeViewportMessage(const Vector2UI32 &size);
    void sendCursorPressMessage(const Vector2SI32 &coord, int type, int modifiers);
    void sendCursorMoveMessage(const Vector2SI32 &coord, const Vector2SI32 &delta, int type, int modifiers);
    void sendCursorReleaseMessage(const Vector2SI32 &coord, int type, int modifiers);
    void sendCursorScrollMessage(const Vector2SI32 &coord, const Vector2SI32 &delta, int modifiers);
    void sendPointerPressMessage(const Vector2 &coord, int type);
    void sendPointerMoveMessage(const Vector2 &coord, const Vector2 &delta, int type);
    void sendPointerReleaseMessage(const Vector2 &coord, int type);
    void sendPointerScrollMessage(const Vector2 &coord, const Vector2 &delta, int modifiers);
    void sendKeyPressMessage(nanoem_u32_t value);
    void sendKeyReleaseMessage(nanoem_u32_t value);
    void sendUnicodeInputMessage(nanoem_u32_t value);
    void sendMenuActionMessage(nanoem_u32_t value);
    void sendLoadFileMessage(const URI &fileURI, nanoem_u32_t type);
    void sendSaveFileMessage(const URI &fileURI, nanoem_u32_t type);
    void sendDropFileMessage(const URI &fileURI);
    void sendNewProjectMessage();
    void sendConfirmBeforeNewProjectMessage();
    void sendConfirmBeforeOpenProjectMessage();
    void sendConfirmBeforeExitApplicationMessage();
    void sendConfirmBeforeExportingImageMessage();
    void sendConfirmBeforeExportingVideoMessage();
    void sendChangeDevicePixelRatioMessage(nanoem_f32_t value);
    void sendSetActiveAccessoryMessage(nanoem_u16_t handle);
    void sendSetActiveModelMessage(nanoem_u16_t handle);
    void sendSetActiveModelBoneMessage(const char *name);
    void sendSetActiveModelMorphMessage(const char *name, bool applyCategory);
    void sendLoadAllDecoderPluginsMessage(const URIList &values);
    void sendLoadAllEncoderPluginsMessage(const URIList &values);
    void sendSeekMessage(nanoem_frame_index_t value);
    void sendSetCameraLookAtMessage(const Vector3 &value, bool canUndo);
    void sendSetCameraAngleMessage(const Vector3 &value, bool canUndo);
    void sendSetCameraFovMessage(nanoem_f32_t value, bool canUndo);
    void sendSetCameraDistanceMessage(nanoem_f32_t value, bool canUndo);
    void sendSetLightColorMessage(const Vector3 &value, bool canUndo);
    void sendSetLightDirectionMessage(const Vector3 &value, bool canUndo);
    void sendSetAccessoryTranslationMessage(nanoem_u16_t handle, const Vector3 &value, bool canUndo);
    void sendSetAccessoryOrientationMessage(nanoem_u16_t handle, const Vector3 &value, bool canUndo);
    void sendSetAccessoryScaleFactorMessage(nanoem_u16_t handle, nanoem_f32_t value, bool canUndo);
    void sendSetAccessoryOpacityMessage(nanoem_u16_t handle, nanoem_f32_t value, bool canUndo);
    void sendSetModelMorphWeightMessage(nanoem_u16_t handle, const String &name, nanoem_f32_t value, bool canUndo);
    void sendSetModelBoneTranslationMessage(
        nanoem_u16_t handle, const String &name, const Vector3 &value, bool canUndo);
    void sendSetModelBoneOrientationMessage(
        nanoem_u16_t handle, const String &name, const Quaternion &value, bool canUndo);
    void sendUpdateModelMessage(nanoem_u16_t handle);
    void sendRegisterAccessoryKeyframeMessage(nanoem_u16_t handle);
    void sendRegisterAllSelectedBoneKeyframesMessage(nanoem_u16_t handle, const StringList &names);
    void sendRegisterCameraKeyframeMessage();
    void sendRegisterLightKeyframeMessage();
    void sendRegisterModelKeyframeMessage(nanoem_u16_t handle);
    void sendRegisterAllSelectedMorphKeyframesMessage(nanoem_u16_t handle, const StringList &names);
    void sendRegisterSelfShadowKeyframeMessage();
    void sendRemoveAccessoryKeyframeMessage(nanoem_u16_t handle);
    void sendRemoveAllSelectedBoneKeyframesMessage(nanoem_u16_t handle, const StringList &names);
    void sendRemoveCameraKeyframeMessage();
    void sendRemoveLightKeyframeMessage();
    void sendRemoveModelKeyframeMessage(nanoem_u16_t handle);
    void sendRemoveAllSelectedMorphKeyframesMessage(nanoem_u16_t handle, const StringList &names);
    void sendRemoveSelfShadowKeyframeMessage();
    void sendBoneBezierControlPointMessage(
        nanoem_u16_t handle, const String &name, const glm::u8vec4 &value, nanoem_u32_t type);
    void sendCameraBezierControlPointMessage(const glm::u8vec4 &value, nanoem_u32_t type);
    void sendSelectBoneMessage(nanoem_u16_t handle, const String &name);
    void sendSelectAllBonesMessage(nanoem_u16_t handle);
    void sendSelectAllDirtyBonesMessage(nanoem_u16_t handle);
    void sendSelectAllMovableDirtyBonesMessage(nanoem_u16_t handle);
    void sendClearSelectBoneMessage(nanoem_u16_t handle, const String &name);
    void sendClearSelectAllBonesMessage(nanoem_u16_t handle);
    void sendSelectMotionKeyframeMessage(nanoem_u16_t handle, nanoem_frame_index_t frameIndex, const String &trackName);
    void sendSelectAllRowMotionKeyframesMessage(nanoem_u16_t handle, const String &trackName);
    void sendSelectAllColumnMotionKeyframesMessage(nanoem_u16_t handle, nanoem_frame_index_t frameIndex);
    void sendClearSelectAllMotionKeyframesMessage(nanoem_u16_t handle);
    void sendUpdateCurrentFPSMessage(nanoem_u32_t value);
    void sendReloadAccessoryEffectMessage(nanoem_u16_t handle);
    void sendReloadModelEffectMessage(nanoem_u16_t handle);
    void sendSetModelVisibleMessage(nanoem_u16_t handle, bool value);
    void sendSetModelEdgeColorMessage(nanoem_u16_t handle, const Vector4 &value);
    void sendSetModelEdgeSizeMessage(nanoem_u16_t handle, nanoem_f32_t value);
    void sendSetModelConstraintStateMessage(nanoem_u16_t handle, const String &constraintName, bool value);
    void sendSetModelOutsideParentMessage(
        nanoem_u16_t handle, const String &boneName, const String &parentModelName, const String &parentModelBoneName);
    void sendSetAccessoryOutsideParentMessage(
        nanoem_u16_t handle, const String &parentModelName, const String &parentModelBoneName);
    void sendRecoveryMessage(const URI &fileURI);
    void sendSetAccessoryAddBlendEnabledMessage(nanoem_u16_t handle, bool value);
    void sendSetAccessoryShadowEnabledMessage(nanoem_u16_t handle, bool value);
    void sendSetAccessoryVisibleMessage(nanoem_u16_t handle, bool value);
    void sendSetModelAddBlendEnabledMessage(nanoem_u16_t handle, bool value);
    void sendSetModelShadowEnabledMessage(nanoem_u16_t handle, bool value);
    void sendSetModelShadowMapEnabledMessage(nanoem_u16_t handle, bool value);
    void sendSetDrawableOrderIndexMessage(nanoem_u16_t handle, int value);
    void sendSetModelTransformOrderIndexMessage(nanoem_u16_t handle, int value);
    void sendSetModelBoneKeyframeInterpolationMessage(nanoem_u16_t handle, const glm::u8vec4 *values);
    void sendUpdatePerformanceMonitorMessage(
        nanoem_f32_t cpu, nanoem_u64_t currentMemorySize, nanoem_u64_t maxMemorySize);
    void sendLoadAllModelPluginsMessage(const URIList &values);
    void sendLoadAllMotionPluginsMessage(const URIList &values);
    void sendExecuteModelPluginMessage(nanoem_u16_t plugin, nanoem_u32_t function);
    void sendExecuteMotionPluginMessage(nanoem_u16_t plugin, nanoem_u32_t function);
    void sendRequestExportImageConfigurationMessage();
    void sendRequestExportVideoConfigurationMessage();
    void sendExecuteExportingImageMessage(const URI &fileURI);
    void sendExecuteExportingVideoMessage(const URI &fileURI);
    void sendScreenCursorPressMessage(const Vector2SI32 &coord, int type, int modifiers);
    void sendScreenCursorMoveMessage(const Vector2SI32 &coord, int type, int modifiers);
    void sendScreenCursorReleaseMessage(const Vector2SI32 &coord, int type, int modifiers);
    void sendSetCameraKeyframeInterpolationMessage(const glm::u8vec4 *values);
    void sendIsProjectDirtyRequestMessage(pfn_isProjectDirtyCallback callback, void *userData);
    void sendGetProjectFileURIRequestMessage(pfn_getProjectFileURICallback callback, void *userData);
    void sendGetAllModelBonesRequestMessage(nanoem_u16_t handle, pfn_getAllModelBonesCallback callback, void *userData);
    void sendGetAllModelMorphsRequestMessages(
        nanoem_u16_t handle, pfn_getAllModelMorphsCallback callback, void *userData);
    void sendGetBackgroundImageTextureHandleMessage(
        pfn_getBackgroundTextureImageHandleCallback callback, void *userData);
    void sendGetAllAccessoriesRequestMessage(pfn_getAllAccessoriesCallback callback, void *userData);
    void sendGetAllModelsRequestMessage(pfn_getAllModelsCallback callback, void *userData);
    void sendQueryOpenSingleFileDialogMessage(nanoem_u32_t type, const URI &fileURI);
    void sendQueryOpenMultipleFilesDialogMessage(nanoem_u32_t type, const URIList &fileURIs);
    void sendQuerySaveFileDialogMessage(nanoem_u32_t type, const URI &fileURI);
    void sendGetHandleFileURIRequestMessage(nanoem_u16_t handle, pfn_getHandleFileURICallback callback, void *userData);
    void sendStartDebugCaptureMessage();
    void sendStopDebugCaptureMessage();
    void sendDestroyMessage();
    void sendTerminateMessage();

    void addTrackEventListener(pfn_handleTrackEvent listener, void *userData, bool once);
    void addUndoEventListener(pfn_handleUndoEvent listener, void *userData, bool once);
    void addRedoEventListener(pfn_handleRedoEvent listener, void *userData, bool once);
    void addUndoChangeEventListener(pfn_handleUndoChangeEvent listener, void *userData, bool once);
    void addAddModelEventListener(pfn_handleAddModelEvent listener, void *userData, bool once);
    void addSetActiveModelEventListener(pfn_handleSetActiveModelEvent listener, void *userData, bool once);
    void addRemoveModelEventListener(pfn_handleRemoveModelEvent listener, void *userData, bool once);
    void addAddAccessoryEventListener(pfn_handleAddAccessoryEvent listener, void *userData, bool once);
    void addSetActiveAccessoryEventListener(pfn_handleSetActiveAccessoryEvent listener, void *userData, bool once);
    void addRemoveAccessoryEventListener(pfn_handleRemoveAccessoryEvent listener, void *userData, bool once);
    void addPlayEvent(pfn_handlePlayEvent listener, void *userData, bool once);
    void addStopEvent(pfn_handleStopEvent listener, void *userData, bool once);
    void addPauseEvent(pfn_handlePauseEvent listener, void *userData, bool once);
    void addResumeEvent(pfn_handleResumeEvent listener, void *userData, bool once);
    void addSeekEvent(pfn_handleSeekEvent listener, void *userData, bool once);
    void addUpdateDurationEvent(pfn_handleUpdateDurationEvent listener, void *userData, bool once);
    void addSaveProjectAfterConfirmEventListener(
        pfn_handleSaveProjectAfterConfirmEvent listener, void *userData, bool once);
    void addDiscardProjectAfterConfirmEventListener(
        pfn_handleDiscardProjectAfterConfirmEvent listener, void *userData, bool once);
    void addToggleProjectEffectEnabledEvent(
        pfn_handleToggleProjectEffectEnabledEvent listener, void *userData, bool once);
    void addToggleProjectGroundShadowEnabledEvent(
        pfn_handleToggleProjectGroundShadowEnabledEvent listener, void *userData, bool once);
    void addToggleProjectVertexShaderSkinningEnabledEvent(
        pfn_handleToggleProjectVertexShaderSkinningEnabledEvent listener, void *userData, bool once);
    void addToggleProjectComputeShaderSkinningEnabledEvent(
        pfn_handleToggleProjectComputeShaderSkinningEnabledEvent listener, void *userData, bool once);
    void addSetProjectSampleLevelEvent(pfn_handleSetProjectSampleLevelEvent listener, void *userData, bool once);
    void addToggleGridEnabledEvent(pfn_handleToggleGridEnabledEvent listener, void *userData, bool once);
    void addSetGridCellEvent(pfn_handleSetGridCellEvent listener, void *userData, bool once);
    void addSetGridSizeEvent(pfn_handleSetGridSizeEvent listener, void *userData, bool once);
    void addSetPreferredMotionFPSEvent(pfn_handleSetPreferredMotionFPSEvent listener, void *userData, bool once);
    void addSetPhysicsSimulationModeEvent(pfn_handleSetPhysicsSimulationModeEvent listener, void *userData, bool once);
    void addSetPhysicsSimulationEngineDebugFlagEvent(
        pfn_handleSetPhysicsSimulationEngineDebugFlagEvent listener, void *userData, bool once);
    void addToggleShadowMapEnabledEvent(pfn_handleToggleShadowMapEnabledEvent listener, void *userData, bool once);
    void addSetShadowMapModeEvent(pfn_handleSetShadowMapModeEvent listener, void *userData, bool once);
    void addSetShadowMapDistanceEvent(pfn_handleSetShadowMapDistanceEvent listener, void *userData, bool once);
    void addToggleActiveModelAddBlendEnabledEvent(
        pfn_handleToggleActiveModelAddBlendEnabledEvent listener, void *userData, bool once);
    void addToggleActiveModelShadowMapEnabledEvent(
        pfn_handleToggleActiveModelShadowMapEnabledEvent listener, void *userData, bool once);
    void addToggleActiveModelVisibleEvent(pfn_handleToggleActiveModelVisibleEvent listener, void *userData, bool once);
    void addToggleActiveModelComputeShaderSkinningEnabledEvent(
        pfn_handleToggleActiveModelComputeShaderSkinningEnabledEvent listener, void *userData, bool once);
    void addToggleActiveModelShowAllBonesEvent(
        pfn_handleToggleActiveModelShowAllBonesEvent listener, void *userData, bool once);
    void addToggleActiveModelShowAllRigidBodiesEvent(
        pfn_handleToggleActiveModelShowAllRigidBodiesEvent listener, void *userData, bool once);
    void addToggleActiveModelShowAllVertexFacesEvent(
        pfn_handleToggleActiveModelShowAllVertexFacesEvent listener, void *userData, bool once);
    void addToggleActiveModelShowAllVertexPointsEvent(
        pfn_handleToggleActiveModelShowAllVertexPointsEvent listener, void *userData, bool once);
    void addToggleActiveModelVertexShaderSkinningEnabledEvent(
        pfn_handleToggleActiveModelVertexShaderSkinningEnabledEvent listener, void *userData, bool once);
    void addAvailableAllImportingAudioExtensionsEvent(
        pfn_handleAvailableAllImportingAudioExtensionsEvent listener, void *userData, bool once);
    void addAvailableAllImportingVideoExtensionsEvent(
        pfn_handleAvailableAllImportingVideoExtensionsEvent listener, void *userData, bool once);
    void addAvailableAllExportingImageExtensionsEvent(
        pfn_handleAvailableAllExportingImageExtensionsEvent listener, void *userData, bool once);
    void addAvailableAllExportingVideoExtensionsEvent(
        pfn_handleAvailableAllExportingVideoExtensionsEvent listener, void *userData, bool once);
    void addDisableCursorEventListener(pfn_handleDisableCursorEvent listener, void *userData, bool once);
    void addEnableCursorEventListener(pfn_handleEnableCursorEvent listener, void *userData, bool once);
    void addInitializationCompleteEventListener(
        pfn_handleInitializationCompleteEvent listener, void *userData, bool once);
    void addCompleteDestructionEventListener(pfn_handleCompleteDestructionEvent listener, void *userData, bool once);
    void addCompleteTerminationEventListener(pfn_handleCompleteTerminationEvent listener, void *userData, bool once);
    void addErrorEventListener(pfn_handleErrorEvent listener, void *userData, bool once);
    void addStartRecordingViewportPassEventListener(
        pfn_handleStartRecordingViewportPassEvent listener, void *userData, bool once);
    void addStopRecordingViewportPassEventListener(
        pfn_handleStopRecordingViewportPassEvent listener, void *userData, bool once);
    void addCompleteLoadingFileEventListener(pfn_handleCompleteLoadingFileEvent listener, void *userData, bool once);
    void addCompleteSavingFileEventListener(pfn_handleCompleteSavingFileEvent listener, void *userData, bool once);
    void addSetPreferredEditingFPSEvent(pfn_handleSetPreferredEditingFPSEvent listener, void *userData, bool once);
    void addConsumePassEventListener(pfn_handleConsumePassEvent listener, void *userData, bool once);
    void addAddModalDialogEventListener(pfn_handleAddModalDialogEvent listener, void *userData, bool once);
    void addClearModalDialogEventListener(pfn_handleClearModalDialogEvent listener, void *userData, bool once);
    void addSetLanguageEventListener(pfn_handleSetLanguageEvent listener, void *userData, bool once);
    void addToggleProjectPlayingWithLoopEventListener(
        pfn_handleToggleProjectPlayingWithLoopEnabledEvent listener, void *userData, bool once);
    void addToggleActiveAccessoryAddBlendEnabledEventListener(
        pfn_handleToggleActiveAccessoryAddBlendEnabledEvent listener, void *userData, bool once);
    void addToggleActiveAccessoryShadowEnabledEventListener(
        pfn_handleToggleActiveAccessoryShadowEnabledEvent listener, void *userData, bool once);
    void addToggleActiveAccessoryVisibleEventListener(
        pfn_handleToggleActiveAccessoryVisibleEvent listener, void *userData, bool once);
    void addUpdateProgressEventListener(pfn_handleUpdateProgressEvent listener, void *userData, bool once);
    void addStartProgressEventListener(pfn_handleStartProgressEvent listener, void *userData, bool once);
    void addStopProgressEventListener(pfn_handleStopProgressEvent listener, void *userData, bool once);
    void addSetupProjectEventListener(pfn_handleSetupProjectEvent listener, void *userData, bool once);
    void addSetEditingModeEventListener(pfn_handleSetEditingModeEvent listener, void *userData, bool once);
    void addCompleteLoadingAllModelPluginsEventListener(
        pfn_handleCompleteLoadingAllModelPluginsEvent listener, void *userData, bool once);
    void addCompleteLoadingAllMotionPluginsEventListener(
        pfn_handleCompleteLoadingAllMotionPluginsEvent listener, void *userData, bool once);
    void addCompleteExportImageConfigurationEventListener(
        pfn_handleCompleteExportImageConfigurationEvent listener, void *userData, bool once);
    void addCompleteExportVideoConfigurationEventListener(
        pfn_handleCompleteExportVideoConfigurationEvent listener, void *userData, bool once);
    void addQueryOpenSingleFileDialogEventListener(
        pfn_handleQueryOpenSingleFileDialogEvent listener, void *userData, bool once);
    void addQueryOpenMultipleFilesDialogEventListener(
        pfn_handleQueryOpenMultipleFilesDialogEvent listener, void *userData, bool once);
    void addQuerySaveFileDialogEventListener(pfn_handleQuerySaveFileDialogEvent listener, void *userData, bool once);
    void addCanCopyEventListener(pfn_handleCanCopyEvent listener, void *userData, bool once);
    void addCanPasteEventListener(pfn_handleCanPasteEvent listener, void *userData, bool once);
    void addSetWindowDevicePixelRatioEventListener(
        pfn_handleSetWindowDevicePixelRatioEvent listener, void *userData, bool once);
    void addSetViewportDevicePixelRatioEventListener(
        pfn_handleSetViewportDevicePixelRatioEvent listener, void *userData, bool once);
    void clearAllProjectAfterConfirmOnceEventListeners();
    void clearAllCompleteLoadingFileOnceEventListeners();
    void clearAllCompleteSavingFileOnceEventListeners();

protected:
    virtual void sendCommandMessage(const Nanoem__Application__Command *command) = 0;
    nanoem_rsize_t sizeofCommandMessage(const Nanoem__Application__Command *command) NANOEM_DECL_NOEXCEPT;
    void packCommandMessage(const Nanoem__Application__Command *command, nanoem_u8_t *data);
    void dispatchEventMessage(const nanoem_u8_t *data, size_t size);

private:
    struct EventListener {
        void *userData;
        bool once;
        union {
            pfn_handleTrackEvent handleTrack;
            pfn_handleUndoEvent handleUndo;
            pfn_handleRedoEvent handleRedo;
            pfn_handleUndoChangeEvent handleUndoChange;
            pfn_handleAddModelEvent handleAddModel;
            pfn_handleSetActiveModelEvent handleSetActiveModel;
            pfn_handleRemoveModelEvent handleRemoveModel;
            pfn_handleAddAccessoryEvent handleAddAccessory;
            pfn_handleSetActiveAccessoryEvent handleSetActiveAccessory;
            pfn_handleRemoveAccessoryEvent handleRemoveAccessory;
            pfn_handlePlayEvent handlePlay;
            pfn_handleStopEvent handleStop;
            pfn_handlePauseEvent handlePause;
            pfn_handleResumeEvent handleResume;
            pfn_handleSeekEvent handleSeek;
            pfn_handleUpdateDurationEvent handleUpdateDuration;
            pfn_handleSaveProjectAfterConfirmEvent handleSaveProjectAfterConfirm;
            pfn_handleDiscardProjectAfterConfirmEvent handleDiscardProjectAfterConfirm;
            pfn_handleToggleProjectEffectEnabledEvent handleToggleProjectEffectEnabled;
            pfn_handleToggleProjectGroundShadowEnabledEvent handleToggleProjectGroundShadowEnabled;
            pfn_handleToggleProjectVertexShaderSkinningEnabledEvent handleToggleProjectVertexShaderSkinningEnabled;
            pfn_handleToggleProjectComputeShaderSkinningEnabledEvent handleToggleProjectComputeShaderSkinningEnabled;
            pfn_handleSetProjectSampleLevelEvent handleSetProjectSampleLevel;
            pfn_handleToggleGridEnabledEvent handleToggleGridEnabled;
            pfn_handleSetGridCellEvent handleSetGridCell;
            pfn_handleSetGridSizeEvent handleSetGridSize;
            pfn_handleSetPreferredMotionFPSEvent handleSetPreferredMotionFPS;
            pfn_handleSetPhysicsSimulationModeEvent handleSetPhysicsSimulationMode;
            pfn_handleSetPhysicsSimulationEngineDebugFlagEvent handleSetPhysicsSimulationEngineDebugFlag;
            pfn_handleToggleShadowMapEnabledEvent handleToggleShadowMapEnabled;
            pfn_handleSetShadowMapModeEvent handleSetShadowMapMode;
            pfn_handleSetShadowMapDistanceEvent handleSetShadowMapDistance;
            pfn_handleToggleActiveModelAddBlendEnabledEvent handleToggleActiveModelAddBlendEnabled;
            pfn_handleToggleActiveModelShadowMapEnabledEvent handleToggleActiveModelShadowMapEnabled;
            pfn_handleToggleActiveModelVisibleEvent handleToggleActiveModelVisible;
            pfn_handleToggleActiveModelComputeShaderSkinningEnabledEvent
                handleToggleActiveModelComputeShaderSkinningEnabled;
            pfn_handleToggleActiveModelShowAllBonesEvent handleToggleActiveModelShowAllBones;
            pfn_handleToggleActiveModelShowAllRigidBodiesEvent handleToggleActiveModelShowAllRigidBodies;
            pfn_handleToggleActiveModelShowAllVertexFacesEvent handleToggleActiveModelShowAllVertexFaces;
            pfn_handleToggleActiveModelShowAllVertexPointsEvent handleToggleActiveModelShowAllVertexPoints;
            pfn_handleToggleActiveModelVertexShaderSkinningEnabledEvent
                handleToggleActiveModelVertexShaderSkinningEnabled;
            pfn_handleAvailableAllImportingAudioExtensionsEvent handleAvailableAllImportingAudioExtensions;
            pfn_handleAvailableAllImportingVideoExtensionsEvent handleAvailableAllImportingVideoExtensions;
            pfn_handleAvailableAllExportingImageExtensionsEvent handleAvailableAllExportingImageExtensions;
            pfn_handleAvailableAllExportingVideoExtensionsEvent handleAvailableAllExportingVideoExtensions;
            pfn_handleDisableCursorEvent handleDisableCursor;
            pfn_handleEnableCursorEvent handleEnableCursor;
            pfn_handleErrorEvent handleError;
            pfn_handleInitializationCompleteEvent handleInitializationComplete;
            pfn_handleCompleteDestructionEvent handleCompleteDestruction;
            pfn_handleCompleteTerminationEvent handleCompleteTermination;
            pfn_handleStartRecordingViewportPassEvent handleStartRecordingViewportPass;
            pfn_handleStopRecordingViewportPassEvent handleStopRecordingViewportPass;
            pfn_handleCompleteLoadingFileEvent handleCompleteLoadingFile;
            pfn_handleCompleteSavingFileEvent handleCompleteSavingFile;
            pfn_handleSetPreferredEditingFPSEvent handleSetPreferredEditingFPS;
            pfn_handleConsumePassEvent handleConsumePass;
            pfn_handleAddModalDialogEvent handleAddModalDialog;
            pfn_handleClearModalDialogEvent handleClearModalDialog;
            pfn_handleSetLanguageEvent handleSetLanguage;
            pfn_handleToggleProjectPlayingWithLoopEnabledEvent handleToggleProjectPlayingWithLoopEnabledEvent;
            pfn_handleToggleActiveAccessoryAddBlendEnabledEvent handleToggleActiveAccessoryAddBlendEnabledEvent;
            pfn_handleToggleActiveAccessoryShadowEnabledEvent handleToggleActiveAccessoryShadowEnabledEvent;
            pfn_handleToggleActiveAccessoryVisibleEvent handleToggleActiveAccessoryVisibleEvent;
            pfn_handleUpdateProgressEvent handleUpdateProgressEvent;
            pfn_handleStartProgressEvent handleStartProgressEvent;
            pfn_handleStopProgressEvent handleStopProgressEvent;
            pfn_handleSetupProjectEvent handleSetupProjectEvent;
            pfn_handleSetEditingModeEvent handleSetEditingModeEvent;
            pfn_handleCompleteLoadingAllModelPluginsEvent handleCompleteLoadingAllModelPluginsEvent;
            pfn_handleCompleteLoadingAllMotionPluginsEvent handleCompleteLoadingAllMotionPluginsEvent;
            pfn_handleCompleteExportImageConfigurationEvent handleCompleteExportImageConfigurationEvent;
            pfn_handleCompleteExportVideoConfigurationEvent handleCompleteExportVideoConfigurationEvent;
            pfn_handleQueryOpenSingleFileDialogEvent handleQueryOpenSingleFileDialogEvent;
            pfn_handleQueryOpenMultipleFilesDialogEvent handleQueryOpenMultipleFilesDialogEvent;
            pfn_handleQuerySaveFileDialogEvent handleQuerySaveFileDialogEvent;
            pfn_handleCanCopyEvent handleCanCopyEvent;
            pfn_handleCanPasteEvent handleCanPasteEvent;
            pfn_handleSetWindowDevicePixelRatioEvent handleSetWindowDevicePixelRatioEvent;
            pfn_handleSetViewportDevicePixelRatioEvent handleSetViewportDevicePixelRatioEvent;
        } u;
    };
    typedef tinystl::vector<EventListener, TinySTLAllocator> EventListenerList;
    typedef tinystl::unordered_map<nanoem_u32_t, EventListenerList, TinySTLAllocator> EventListenerListMap;
    struct RequestCallback {
        nanoem_u32_t type;
        void *userData;
        union {
            pfn_getAllModelBonesCallback getAllModelBones;
            pfn_getAllModelMorphsCallback getAllModelMorphs;
            pfn_getProjectFileURICallback getProjectFileURI;
            pfn_getBackgroundTextureImageHandleCallback getBackgroundTextureImageHandle;
            pfn_isProjectDirtyCallback isProjectDirty;
            pfn_getAllAccessoriesCallback getAllAccessories;
            pfn_getAllModelsCallback getAllModels;
            pfn_getHandleFileURICallback getHandleFileURI;
        } u;
    };
    typedef tinystl::unordered_map<nanoem_u64_t, RequestCallback, TinySTLAllocator> RequestCallbackMap;

    void clearAllOnceEventListeners(nanoem_u32_t type);

    EventListenerListMap m_eventListeners;
    RequestCallbackMap m_requestCallbacks;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_BASEAPPLICATIONCLIENT_H_ */
