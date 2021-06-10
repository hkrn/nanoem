/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_STUBEVENTPUBLISHER_H_
#define NANOEM_EMAPP_INTERNAL_STUBEVENTPUBLISHER_H_

#include "emapp/Forward.h"

#include "emapp/Error.h"
#include "emapp/IEventPublisher.h"

namespace nanoem {
namespace internal {

class StubEventPublisher NANOEM_DECL_SEALED : public IEventPublisher, private NonCopyable {
public:
    StubEventPublisher()
    {
    }
    ~StubEventPublisher()
    {
    }

    void
    publishTrackingEvent(
        const char *screen, const char *action, const char *category, const char *label) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_4(screen, action, category, label);
    }
    void
    publishUndoEvent(bool canUndo, bool canRedo) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_2(canUndo, canRedo);
    }
    void
    publishRedoEvent(bool canRedo, bool canUndo) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_2(canUndo, canRedo);
    }
    void
    publishPushUndoCommandEvent(const undo_command_t *commandPtr) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(commandPtr);
    }
    void
    publishAddModelEvent(const Model *model) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(model);
    }
    void
    publishSetActiveModelEvent(const Model *model) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(model);
    }
    void
    publishSetActiveBoneEvent(const Model *model, const char *boneName) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_2(model, boneName);
    }
    void
    publishSetActiveMorphEvent(const Model *model, const char *morphName) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_2(model, morphName);
    }
    void
    publishRemoveModelEvent(const Model *model) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(model);
    }
    void
    publishAddAccessoryEvent(const Accessory *accessory) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(accessory);
    }
    void
    publishSetActiveAccessoryEvent(const Accessory *accessory) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(accessory);
    }
    void
    publishRemoveAccessoryEvent(const Accessory *accessory) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(accessory);
    }
    void
    publishAddMotionEvent(const Motion *motion) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(motion);
    }
    void
    publishRemoveMotionEvent(const Motion *motion) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(motion);
    }
    void
    publishPlayEvent(nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_2(duration, localFrameIndex);
    }
    void
    publishPauseEvent(nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_2(duration, localFrameIndex);
    }
    void
    publishResumeEvent(nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_2(duration, localFrameIndex);
    }
    void
    publishStopEvent(nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_2(duration, localFrameIndex);
    }
    void
    publishSeekEvent(nanoem_frame_index_t duration, nanoem_frame_index_t frameIndexTo,
        nanoem_frame_index_t frameIndexFrom) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_3(duration, frameIndexTo, frameIndexFrom);
    }
    void
    publishUpdateDurationEvent(
        nanoem_frame_index_t currentDuration, nanoem_frame_index_t lastDuration) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_2(currentDuration, lastDuration);
    }
    void
    publishToggleProjectEffectEnabledEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishToggleProjectGroundShadowEnabledEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishToggleProjectVertexShaderSkinningEnabledEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishToggleProjectComputeShaderSkinningEnabledEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishSetProjectSampleLevelEvent(nanoem_u32_t value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishToggleGridEnabledEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishSetGridCellEvent(const Vector2 &value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishSetGridSizeEvent(const Vector2 &value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishSetPreferredMotionFPSEvent(nanoem_u32_t value, bool unlimited) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_2(value, unlimited);
    }
    void
    publishSetPreferredEditingFPSEvent(nanoem_u32_t value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishSetPhysicsSimulationModeEvent(nanoem_u32_t value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishSetPhysicsSimulationEngineDebugFlagEvent(nanoem_u32_t value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishToggleShadowMapEnabledEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishSetShadowMapModeEvent(nanoem_u32_t value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishSetShadowMapDistanceEvent(nanoem_f32_t value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishToggleActiveModelAddBlendEnabledEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishToggleActiveModelShadowMapEnabledEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishToggleActiveModelVisibleEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishToggleActiveModelComputeShaderSkinningEnabledEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishToggleActiveModelShowAllBonesEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishToggleActiveModelShowAllRigidBodiesEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishToggleActiveModelShowAllVertexFacesEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishToggleActiveModelShowAllVertexPointsEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishToggleActiveModelVertexShaderSkinningEnabledEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishConsumePassEvent(nanoem_u64_t globalFrameIndex) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(globalFrameIndex);
    }
    void
    publishAddModalDialogEvent() NANOEM_DECL_OVERRIDE
    {
    }
    void
    publishClearModalDialogEvent() NANOEM_DECL_OVERRIDE
    {
    }
    void
    publishSetLanguageEvent(nanoem_u32_t value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishQueryOpenSingleFileDialogEvent(nanoem_u32_t value, const StringList &allowedExtensions) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_2(value, allowedExtensions);
    }
    void
    publishQueryOpenMultipleFilesDialogEvent(
        nanoem_u32_t value, const StringList &allowedExtensions) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_2(value, allowedExtensions);
    }
    void
    publishQuerySaveFileDialogEvent(nanoem_u32_t value, const StringList &allowedExtensions) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_2(value, allowedExtensions);
    }
    void
    publishToggleProjectPlayingWithLoopEnabledEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishToggleActiveAccessoryAddBlendEnabledEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishToggleActiveAccessoryGroundShadowEnabledEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishToggleActiveAccessoryVisibleEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishToggleModelEditingEnabledEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishUpdateProgressEvent(
        nanoem_u32_t value, nanoem_u32_t total, nanoem_u32_t type, const char *text) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_4(value, total, type, text);
    }
    void
    publishStartProgressEvent(const char *title, const char *text, nanoem_u32_t total) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_3(title, text, total);
    }
    void
    publishStopProgressEvent() NANOEM_DECL_OVERRIDE
    {
    }
    void
    publishSetupProjectEvent(const Vector2 &windowSize, nanoem_f32_t windowDevicePixelRatio,
        nanoem_f32_t viewportDevicePixelRatio) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_3(windowSize, windowDevicePixelRatio, viewportDevicePixelRatio);
    }
    void
    publishCompleteExportingImageConfigurationEvent(const StringList &availableExtensions) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(availableExtensions);
    }
    void
    publishCompleteExportingVideoConfigurationEvent(const StringList &availableExtensions) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(availableExtensions);
    }
    void
    publishSetEditingModeEvent(nanoem_u32_t value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishCanCopyEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishCanPasteEvent(bool value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishSetWindowDevicePixelRatioEvent(nanoem_f32_t value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishSetViewportDevicePixelRatioEvent(nanoem_f32_t value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishDisableCursorEvent(const Vector2 &value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishEnableCursorEvent(const Vector2 &value) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_1(value);
    }
    void
    publishQuitApplicationEvent() NANOEM_DECL_OVERRIDE
    {
    }
    void
    publishErrorEvent(const Error &error) NANOEM_DECL_OVERRIDE
    {
        if (error.hasReason()) {
            m_errors.push_back(error);
        }
    }

    Error
    lastError() const
    {
        return m_errors.back();
    }
    bool
    hasError() const
    {
        return !m_errors.empty();
    }

private:
    tinystl::vector<Error, TinySTLAllocator> m_errors;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_STUBEVENTPUBLISHER_H_ */
