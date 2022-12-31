/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_IEVENTPUBLISHER_H_
#define NANOEM_EMAPP_IEVENTPUBLISHER_H_

#include "emapp/Forward.h"

struct undo_command_t;

namespace nanoem {

class Accessory;
class Error;
class Model;
class Motion;

class IEventPublisher {
public:
    virtual ~IEventPublisher() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual void publishTrackingEvent(
        const char *screen, const char *action, const char *category, const char *label) = 0;
    virtual void publishUndoEvent(bool canUndo, bool canRedo) = 0;
    virtual void publishRedoEvent(bool canRedo, bool canUndo) = 0;
    virtual void publishPushUndoCommandEvent(const undo_command_t *commandPtr) = 0;
    virtual void publishAddModelEvent(const Model *model) = 0;
    virtual void publishSetActiveModelEvent(const Model *model) = 0;
    virtual void publishSetActiveBoneEvent(const Model *model, const char *boneName) = 0;
    virtual void publishSetActiveMorphEvent(const Model *model, const char *morphName) = 0;
    virtual void publishRemoveModelEvent(const Model *model) = 0;
    virtual void publishAddAccessoryEvent(const Accessory *accessory) = 0;
    virtual void publishSetActiveAccessoryEvent(const Accessory *accessory) = 0;
    virtual void publishRemoveAccessoryEvent(const Accessory *accessory) = 0;
    virtual void publishAddMotionEvent(const Motion *motion) = 0;
    virtual void publishRemoveMotionEvent(const Motion *motion) = 0;
    virtual void publishPlayEvent(nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex) = 0;
    virtual void publishPauseEvent(nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex) = 0;
    virtual void publishResumeEvent(nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex) = 0;
    virtual void publishStopEvent(nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex) = 0;
    virtual void publishSeekEvent(
        nanoem_frame_index_t duration, nanoem_frame_index_t frameIndexTo, nanoem_frame_index_t frameIndexFrom) = 0;
    virtual void publishUpdateDurationEvent(
        nanoem_frame_index_t currentDuration, nanoem_frame_index_t lastDuration) = 0;
    virtual void publishToggleProjectEffectEnabledEvent(bool value) = 0;
    virtual void publishToggleProjectGroundShadowEnabledEvent(bool value) = 0;
    virtual void publishToggleProjectVertexShaderSkinningEnabledEvent(bool value) = 0;
    virtual void publishToggleProjectComputeShaderSkinningEnabledEvent(bool value) = 0;
    virtual void publishSetProjectSampleLevelEvent(nanoem_u32_t value) = 0;
    virtual void publishToggleGridEnabledEvent(bool value) = 0;
    virtual void publishSetGridCellEvent(const Vector2 &value) = 0;
    virtual void publishSetGridSizeEvent(const Vector2 &value) = 0;
    virtual void publishSetPreferredMotionFPSEvent(nanoem_u32_t value, bool unlimited) = 0;
    virtual void publishSetPreferredEditingFPSEvent(nanoem_u32_t value) = 0;
    virtual void publishSetPhysicsSimulationModeEvent(nanoem_u32_t value) = 0;
    virtual void publishSetPhysicsSimulationEngineDebugFlagEvent(nanoem_u32_t value) = 0;
    virtual void publishToggleShadowMapEnabledEvent(bool value) = 0;
    virtual void publishSetShadowMapModeEvent(nanoem_u32_t value) = 0;
    virtual void publishSetShadowMapDistanceEvent(nanoem_f32_t value) = 0;
    virtual void publishToggleActiveModelAddBlendEnabledEvent(bool value) = 0;
    virtual void publishToggleActiveModelShadowMapEnabledEvent(bool value) = 0;
    virtual void publishToggleActiveModelVisibleEvent(bool value) = 0;
    virtual void publishToggleActiveModelComputeShaderSkinningEnabledEvent(bool value) = 0;
    virtual void publishToggleActiveModelShowAllBonesEvent(bool value) = 0;
    virtual void publishToggleActiveModelShowAllRigidBodiesEvent(bool value) = 0;
    virtual void publishToggleActiveModelShowAllVertexFacesEvent(bool value) = 0;
    virtual void publishToggleActiveModelShowAllVertexPointsEvent(bool value) = 0;
    virtual void publishToggleActiveModelVertexShaderSkinningEnabledEvent(bool value) = 0;
    virtual void publishToggleProjectPlayingWithLoopEnabledEvent(bool value) = 0;
    virtual void publishToggleActiveAccessoryAddBlendEnabledEvent(bool value) = 0;
    virtual void publishToggleActiveAccessoryGroundShadowEnabledEvent(bool value) = 0;
    virtual void publishToggleActiveAccessoryVisibleEvent(bool value) = 0;
    virtual void publishToggleModelEditingEnabledEvent(bool value) = 0;
    virtual void publishConsumePassEvent(nanoem_u64_t globalFrameIndex) = 0;
    virtual void publishAddModalDialogEvent() = 0;
    virtual void publishClearModalDialogEvent() = 0;
    virtual void publishSetLanguageEvent(nanoem_u32_t value) = 0;
    virtual void publishQueryOpenSingleFileDialogEvent(nanoem_u32_t value, const StringList &allowedExtensions) = 0;
    virtual void publishQueryOpenMultipleFilesDialogEvent(nanoem_u32_t value, const StringList &allowedExtensions) = 0;
    virtual void publishQuerySaveFileDialogEvent(nanoem_u32_t value, const StringList &allowedExtensions) = 0;
    virtual void publishUpdateProgressEvent(
        nanoem_u32_t value, nanoem_u32_t total, nanoem_u32_t type, const char *text) = 0;
    virtual void publishStartProgressEvent(const char *title, const char *text, nanoem_u32_t total) = 0;
    virtual void publishStopProgressEvent() = 0;
    virtual void publishSetupProjectEvent(
        const Vector2 &windowSize, nanoem_f32_t windowDevicePixelRatio, nanoem_f32_t viewportDevicePixelRatio) = 0;
    virtual void publishSetEditingModeEvent(nanoem_u32_t value) = 0;
    virtual void publishCompleteExportingImageConfigurationEvent(const StringList &availableExtensions) = 0;
    virtual void publishCompleteExportingVideoConfigurationEvent(const StringList &availableExtensions) = 0;
    virtual void publishCanCopyEvent(bool value) = 0;
    virtual void publishCanPasteEvent(bool value) = 0;
    virtual void publishSetWindowDevicePixelRatioEvent(nanoem_f32_t value) = 0;
    virtual void publishSetViewportDevicePixelRatioEvent(nanoem_f32_t value) = 0;
    virtual void publishDisableCursorEvent(const Vector2 &value) = 0;
    virtual void publishEnableCursorEvent(const Vector2 &value) = 0;
    virtual void publishQuitApplicationEvent() = 0;
    virtual void publishErrorEvent(const Error &error) = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_IEVENTPUBLISHER_H_ */
