/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/ApplicationMenuBuilder.h"

#include "emapp/EnumUtils.h"
#include "emapp/Grid.h"
#include "emapp/IFileManager.h"
#include "emapp/Model.h"
#include "emapp/Project.h"
#include "emapp/ShadowCamera.h"
#include "emapp/StringUtils.h"
#include "emapp/ThreadedApplicationClient.h"
#include "emapp/private/CommonInclude.h"

#include "bx/handlealloc.h"

namespace nanoem {
namespace {

class Utils NANOEM_DECL_SEALED : private NonCopyable {
public:
    enum PhysicsSimulationDebugDrawType {
        kDrawWireframe = 1 << 0,
        kDrawAabb = 1 << 1,
        kDrawContactPoints = 1 << 3,
        kDrawConstraints = 1 << 11,
        kDrawConstraintLimits = 1 << 12
    };
};

}

const char *
ApplicationMenuBuilder::menuItemString(MenuItemType type) NANOEM_DECL_NOEXCEPT
{
    const char *text = nullptr;
    switch (type) {
    case kMenuItemTypeFileTitle:
        text = "nanoem.menu.file.title";
        break;
    case kMenuItemTypeFileNewProject:
        text = "nanoem.menu.file.new.project";
        break;
    case kMenuItemTypeFileNewModel:
        text = "nanoem.menu.file.new.model";
        break;
    case kMenuItemTypeFileOpenProject:
        text = "nanoem.menu.file.open";
        break;
    case kMenuItemTypeFileImportTitle:
        text = "nanoem.menu.file.import.title";
        break;
    case kMenuItemTypeFileImportModel:
        text = "nanoem.menu.file.import.model";
        break;
    case kMenuItemTypeFileImportAccessory:
        text = "nanoem.menu.file.import.accessory";
        break;
    case kMenuItemTypeFileImportModelPose:
        text = "nanoem.menu.file.import.pose";
        break;
    case kMenuItemTypeFileImportMotionTitle:
        text = "nanoem.menu.file.import.motion.title";
        break;
    case kMenuItemTypeFileImportModelMotion:
        text = "nanoem.menu.file.import.motion.model";
        break;
    case kMenuItemTypeFileImportCameraMotion:
        text = "nanoem.menu.file.import.motion.camera";
        break;
    case kMenuItemTypeFileImportLightMotion:
        text = "nanoem.menu.file.import.motion.light";
        break;
    case kMenuItemTypeFileImportAudioSource:
        text = "nanoem.menu.file.import.audio";
        break;
    case kMenuItemTypeFileImportBackgroundVideo:
        text = "nanoem.menu.file.import.video";
        break;
    case kMenuItemTypeFileSaveProject:
        text = "nanoem.menu.file.save";
        break;
    case kMenuItemTypeFileSaveAsProject:
        text = "nanoem.menu.file.save-as";
        break;
    case kMenuItemTypeFileExportTitle:
        text = "nanoem.menu.file.export.title";
        break;
    case kMenuItemTypeFileExportModelPose:
        text = "nanoem.menu.file.export.pose";
        break;
    case kMenuItemTypeFileExportModelMotion:
        text = "nanoem.menu.file.export.motion.model";
        break;
    case kMenuItemTypeFileExportCameraMotion:
        text = "nanoem.menu.file.export.motion.camera";
        break;
    case kMenuItemTypeFileExportLightMotion:
        text = "nanoem.menu.file.export.motion.light";
        break;
    case kMenuItemTypeFileExportModel:
        text = "nanoem.menu.file.export.model";
        break;
    case kMenuItemTypeFileExportImage:
        text = "nanoem.menu.file.export.image";
        break;
    case kMenuItemTypeFileExportVideo:
        text = "nanoem.menu.file.export.video";
        break;
    case kMenuItemTypeFileExit:
        text = "nanoem.menu.file.exit";
        break;
    case kMenuItemTypeEditTitle:
        text = "nanoem.menu.edit.title";
        break;
    case kMenuItemTypeEditUndo:
        text = "nanoem.menu.edit.undo";
        break;
    case kMenuItemTypeEditRedo:
        text = "nanoem.menu.edit.redo";
        break;
    case kMenuItemTypeEditCut:
        text = "nanoem.menu.edit.cut";
        break;
    case kMenuItemTypeEditCopy:
        text = "nanoem.menu.edit.copy";
        break;
    case kMenuItemTypeEditPaste:
        text = "nanoem.menu.edit.paste";
        break;
    case kMenuItemTypeEditSelectAll:
        text = "nanoem.menu.edit.select-all-keyframes";
        break;
    case kMenuItemTypeEditBoneTitle:
        text = "nanoem.menu.edit.bone.title";
        break;
    case kMenuItemTypeEditBoneOpenParameterDialog:
        text = "nanoem.menu.edit.bone.parameter";
        break;
    case kMenuItemTypeEditBoneResetAngle:
        text = "nanoem.menu.edit.bone.reset-orientation";
        break;
    case kMenuItemTypeEditBoneOpenCorrectionDialog:
        text = "nanoem.menu.edit.bone.correction";
        break;
    case kMenuItemTypeEditBoneOpenBiasDialog:
        text = "nanoem.menu.edit.bone.bias";
        break;
    case kMenuItemTypeEditCameraTitle:
        text = "nanoem.menu.edit.camera.title";
        break;
    case kMenuItemTypeEditCameraOpenParameterDialog:
        text = "nanoem.menu.edit.camera.parameter";
        break;
    case kMenuItemTypeEditCameraResetAngle:
        text = "nanoem.menu.edit.camera.reset-angle";
        break;
    case kMenuItemTypeEditCameraOpenCorrectionDialog:
        text = "nanoem.menu.edit.camera.correction";
        break;
    case kMenuItemTypeEditMorphTitle:
        text = "nanoem.menu.edit.morph.title";
        break;
    case kMenuItemTypeEditMorphOpenCorrectionDialog:
        text = "nanoem.menu.edit.morph.correction";
        break;
    case kMenuItemTypeEditMorphRemoveAllLipKeyframes:
        text = "nanoem.menu.edit.morph.remove-all-keyframes.lip";
        break;
    case kMenuItemTypeEditMorphRemoveAllEyeKeyframes:
        text = "nanoem.menu.edit.morph.remove-all-keyframes.eye";
        break;
    case kMenuItemTypeEditMorphRemoveAllEyebrowKeyframes:
        text = "nanoem.menu.edit.morph.remove-all-keyframes.eyebrow";
        break;
    case kMenuItemTypeEditMorphResetAllMorphs:
        text = "nanoem.menu.edit.morph.reset-all-morphs";
        break;
    case kMenuItemTypeEditMorphRegisterAllMorphs:
        text = "nanoem.menu.edit.morph.register-all-morphs";
        break;
    case kMenuItemTypeEditModelPluginTitle:
        text = "nanoem.menu.edit.plugin.model";
        break;
    case kMenuItemTypeEditMotionPluginTitle:
        text = "nanoem.menu.edit.plugin.motion";
        break;
    case kMenuItemTypeEditOpenEffectParameterWindow:
        text = "nanoem.menu.edit.window.effect";
        break;
    case kMenuItemTypeEditOpenModelParameterWindow:
        text = "nanoem.menu.edit.window.model";
        break;
    case kMenuItemTypeEditPreference:
        text = "nanoem.menu.edit.preference";
        break;
    case kMenuItemTypeProjectTitle:
        text = "nanoem.menu.project.title";
        break;
    case kMenuItemTypeProjectPlay:
        text = "nanoem.menu.project.play";
        break;
    case kMenuItemTypeProjectStop:
        text = "nanoem.menu.project.stop";
        break;
    case kMenuItemTypeProjectOpenViewportDialog:
        text = "nanoem.menu.project.viewport";
        break;
    case kMenuItemTypeProjectOpenDrawOrderDialog:
        text = "nanoem.menu.project.order.draw";
        break;
    case kMenuItemTypeProjectOpenTransformOrderDialog:
        text = "nanoem.menu.project.order.transform";
        break;
    case kMenuItemTypeProjectDetachViewportWindow:
        text = "nanoem.menu.project.viewport.detach";
        break;
    case kMenuItemTypeProjectEnableLoop:
        text = "nanoem.menu.project.enable.loop";
        break;
    case kMenuItemTypeProjectEnableGrid:
        text = "nanoem.menu.project.enable.grid";
        break;
    case kMenuItemTypeProjectEnableGroundShadow:
        text = "nanoem.menu.project.enable.ground-shadow";
        break;
    case kMenuItemTypeProjectEnableEffect:
        text = "nanoem.menu.project.enable.effect";
        break;
    case kMenuItemTypeProjectEnableHighResolutionViewport:
        text = "nanoem.menu.project.enable.high-resolution-viewport";
        break;
    case kMenuItemTypeProjectMSAATitle:
        text = "nanoem.menu.project.msaa.title";
        break;
    case kMenuItemTypeProjectEnableMSAAx16:
        text = "nanoem.menu.project.msaa.x16";
        break;
    case kMenuItemTypeProjectEnableMSAAx8:
        text = "nanoem.menu.project.msaa.x8";
        break;
    case kMenuItemTypeProjectEnableMSAAx4:
        text = "nanoem.menu.project.msaa.x4";
        break;
    case kMenuItemTypeProjectEnableMSAAx2:
        text = "nanoem.menu.project.msaa.x2";
        break;
    case kMenuItemTypeProjectDisableMSAA:
        text = "nanoem.menu.project.msaa.disabled";
        break;
    case kMenuItemTypeProjectPhysicsSimulationTitle:
        text = "nanoem.menu.project.physics-simulation.title";
        break;
    case kMenuItemTypeProjectPhysicsSimulationEnableAnytime:
        text = "nanoem.menu.project.physics-simulation.enable.anytime";
        break;
    case kMenuItemTypeProjectPhysicsSimulationEnablePlaying:
        text = "nanoem.menu.project.physics-simulation.enable.playing";
        break;
    case kMenuItemTypeProjectPhysicsSimulationEnableTracing:
        text = "nanoem.menu.project.physics-simulation.enable.tracing";
        break;
    case kMenuItemTypeProjectPhysicsSimulationDisable:
        text = "nanoem.menu.project.physics-simulation.disabled";
        break;
    case kMenuItemTypeProjectPhysicsSimulationBakeAllMotions:
        text = "nanoem.menu.project.physics-simulation.bake";
        break;
    case kMenuItemTypeProjectPhysicsSimulationBakeAllMotionsWithIK:
        text = "nanoem.menu.project.physics-simulation.bake2";
        break;
    case kMenuItemTypeProjectPhysicsSimulationConfiguration:
        text = "nanoem.menu.project.physics-simulation.configuration";
        break;
    case kMenuItemTypeProjectPhysicsSimulationDebugDrawTitle:
        text = "nanoem.menu.project.physics-simulation.enable.drawing.title";
        break;
    case kMenuItemTypeProjectPhysicsSimulationEnableDrawingWireframe:
        text = "nanoem.menu.project.physics-simulation.enable.drawing.wireframe";
        break;
    case kMenuItemTypeProjectPhysicsSimulationEnableDrawingAABB:
        text = "nanoem.menu.project.physics-simulation.enable.drawing.aabb";
        break;
    case kMenuItemTypeProjectPhysicsSimulationEnableDrawingContactPoints:
        text = "nanoem.menu.project.physics-simulation.enable.drawing.contact-points";
        break;
    case kMenuItemTypeProjectPhysicsSimulationEnableDrawingConstraints:
        text = "nanoem.menu.project.physics-simulation.enable.drawing.constraints";
        break;
    case kMenuItemTypeProjectPhysicsSimulationEnableDrawingConstraintLimit:
        text = "nanoem.menu.project.physics-simulation.enable.drawing.constraint-limit";
        break;
    case kMenuItemTypeProjectPreferredFPSTitle:
        text = "nanoem.menu.project.fps.title";
        break;
    case kMenuItemTypeProjectPreferredMotionFPSUnlimited:
        text = "nanoem.menu.project.fps.unlimited";
        break;
    case kMenuItemTypeProjectPreferredMotionFPS60:
        text = "nanoem.menu.project.fps.60";
        break;
    case kMenuItemTypeProjectPreferredMotionFPS30:
        text = "nanoem.menu.project.fps.30";
        break;
    case kMenuItemTypeProjectClearAudioSource:
        text = "nanoem.menu.project.clear.audio";
        break;
    case kMenuItemTypeProjectClearBackgroundVideo:
        text = "nanoem.menu.project.clear.video";
        break;
    case kMenuItemTypeProjectEnableFPSCounter:
        text = "nanoem.menu.project.enable.fps-counter";
        break;
    case kMenuItemTypeProjectEnablePerformanceMonitor:
        text = "nanoem.menu.project.enable.performance-monitor";
        break;
    case kMenuItemTypeCameraTitle:
        text = "nanoem.menu.camera.title";
        break;
    case kMenuItemTypeCameraPresetTitle:
        text = "nanoem.menu.camera.preset.title";
        break;
    case kMenuItemTypeCameraPresetTop:
        text = "nanoem.menu.camera.preset.top";
        break;
    case kMenuItemTypeCameraPresetLeft:
        text = "nanoem.menu.camera.preset.left";
        break;
    case kMenuItemTypeCameraPresetRight:
        text = "nanoem.menu.camera.preset.right";
        break;
    case kMenuItemTypeCameraPresetBottom:
        text = "nanoem.menu.camera.preset.bottom";
        break;
    case kMenuItemTypeCameraPresetFront:
        text = "nanoem.menu.camera.preset.front";
        break;
    case kMenuItemTypeCameraPresetBack:
        text = "nanoem.menu.camera.preset.back";
        break;
    case kMenuItemTypeCameraRegisterKeyframe:
        text = "nanoem.menu.camera.register-keyframe";
        break;
    case kMenuItemTypeCameraRemoveAllSelectedKeyframes:
        text = "nanoem.menu.camera.remove-keyframe";
        break;
    case kMenuItemTypeCameraReset:
        text = "nanoem.menu.camera.reset";
        break;
    case kMenuItemTypeLightTitle:
        text = "nanoem.menu.light.title";
        break;
    case kMenuItemTypeLightSelfShadowTitle:
        text = "nanoem.menu.light.self-shadow.title";
        break;
    case kMenuItemTypeLightSelfShadowDisable:
        text = "nanoem.menu.light.self-shadow.disable";
        break;
    case kMenuItemTypeLightSelfShadowEnableWithMode1:
        text = "nanoem.menu.light.self-shadow.enable.mode1";
        break;
    case kMenuItemTypeLightSelfShadowEnableWithMode2:
        text = "nanoem.menu.light.self-shadow.enable.mode2";
        break;
    case kMenuItemTypeLightSelfShadowRegisterKeyframe:
        text = "nanoem.menu.light.self-shadow.register-keyframe";
        break;
    case kMenuItemTypeLightSelfShadowRemoveAllSelectedKeyframes:
        text = "nanoem.menu.light.self-shadow.remove-keyframe";
        break;
    case kMenuItemTypeLightSelfShadowReset:
        text = "nanoem.menu.light.self-shadow.reset";
        break;
    case kMenuItemTypeLightRegisterKeyframe:
        text = "nanoem.menu.light.register-keyframe";
        break;
    case kMenuItemTypeLightRemoveAllSelectedKeyframes:
        text = "nanoem.menu.light.remove-keyframe";
        break;
    case kMenuItemTypeLightReset:
        text = "nanoem.menu.light.reset";
        break;
    case kMenuItemTypeModelTitle:
        text = "nanoem.menu.model.title";
        break;
    case kMenuItemTypeModelSelectTitle:
        text = "nanoem.menu.model.select.title";
        break;
    case kMenuItemTypeModelSelectBoneTitle:
        text = "nanoem.menu.model.select.bone.title";
        break;
    case kMenuItemTypeModelSelectMorphTitle:
        text = "nanoem.menu.model.select.morph.title";
        break;
    case kMenuItemTypeModelSelectMorphEyeTitle:
        text = "nanoem.menu.model.select.morph.eye.title";
        break;
    case kMenuItemTypeModelSelectMorphLipTitle:
        text = "nanoem.menu.model.select.morph.lip.title";
        break;
    case kMenuItemTypeModelSelectMorphEyebrowTitle:
        text = "nanoem.menu.model.select.morph.eyebrow.title";
        break;
    case kMenuItemTypeModelSelectMorphOtherTitle:
        text = "nanoem.menu.model.select.morph.other.title";
        break;
    case kMenuItemTypeModelSelectAllBoneKeyframesFromSelectedBoneSet:
        text = "nanoem.menu.model.select.bone.keyframes";
        break;
    case kMenuItemTypeModelExpandAllTracks:
        text = "nanoem.menu.model.select.expand-all-tracks";
        break;
    case kMenuItemTypeModelCollapseAllTracks:
        text = "nanoem.menu.model.select.collapse-all-tracks";
        break;
    case kMenuItemTypeModelMeasureHeight:
        text = "nanoem.menu.model.measure-height";
        break;
    case kMenuItemTypeModelPerformValidation:
        text = "nanoem.menu.model.validation";
        break;
    case kMenuItemTypeModelEditModeSelect:
        text = "nanoem.menu.model.edit-mode.select";
        break;
    case kMenuItemTypeModelEditModeRotate:
        text = "nanoem.menu.model.edit-mode.rotate";
        break;
    case kMenuItemTypeModelEditModeMove:
        text = "nanoem.menu.model.edit-mode.move";
        break;
    case kMenuItemTypeModelResetBoneAxisX:
        text = "nanoem.menu.model.reset.bone.x";
        break;
    case kMenuItemTypeModelResetBoneAxisY:
        text = "nanoem.menu.model.reset.bone.y";
        break;
    case kMenuItemTypeModelResetBoneAxisZ:
        text = "nanoem.menu.model.reset.bone.z";
        break;
    case kMenuItemTypeModelResetBoneOrientation:
        text = "nanoem.menu.model.reset.bone.orientation";
        break;
    case kMenuItemTypeModelMorphSetZero:
        text = "nanoem.menu.model.set-morph.zero";
        break;
    case kMenuItemTypeModelMorphSetHalf:
        text = "nanoem.menu.model.set-morph.half";
        break;
    case kMenuItemTypeModelMorphSetOne:
        text = "nanoem.menu.model.set-morph.one";
        break;
    case kMenuItemTypeModelEnableAddBlend:
        text = "nanoem.menu.model.enable.add-blend";
        break;
    case kMenuItemTypeModelEnableShadowMap:
        text = "nanoem.menu.model.enable.shadow-map";
        break;
    case kMenuItemTypeModelEnableVisible:
        text = "nanoem.menu.model.enable.visible";
        break;
    case kMenuItemTypeModelRegisterKeyframe:
        text = "nanoem.menu.model.register-keyframe";
        break;
    case kMenuItemTypeModelRemoveAllSelectedKeyframes:
        text = "nanoem.menu.model.remove-keyframe";
        break;
    case kMenuItemTypeModelPreferenceTitle:
        text = "nanoem.menu.model.preference.title";
        break;
    case kMenuItemTypeModelPreferenceEnableShowAllBones:
        text = "nanoem.menu.model.preference.enable.show-all-bones";
        break;
    case kMenuItemTypeModelPreferenceEnableShowRigidBodies:
        text = "nanoem.menu.model.preference.enable.show-rigid-bodies";
        break;
    case kMenuItemTypeModelPreferenceEnableShowJoints:
        text = "nanoem.menu.model.preference.enable.show-joints";
        break;
    case kMenuItemTypeModelPreferenceEnableShowVertexFaces:
        text = "nanoem.menu.model.preference.enable.show-vertex-faces";
        break;
    case kMenuItemTypeModelPreferenceEnableShowVertexPoints:
        text = "nanoem.menu.model.preference.enable.show-vertex-points";
        break;
    case kMenuItemTypeModelEdgeConfiguraiton:
        text = "nanoem.menu.model.edge";
        break;
    case kMenuItemTypeModelReset:
        text = "nanoem.menu.model.reset";
        break;
    case kMenuItemTypeModelDelete:
        text = "nanoem.menu.model.delete-active";
        break;
    case kMenuItemTypeAccessoryTitle:
        text = "nanoem.menu.accessory.title";
        break;
    case kMenuItemTypeAccessorySelectTitle:
        text = "nanoem.menu.accessory.select.title";
        break;
    case kMenuItemTypeAccessoryRegisterKeyframe:
        text = "nanoem.menu.accessory.register-keyframe";
        break;
    case kMenuItemTypeAccessoryRemoveAllSelectedKeyframes:
        text = "nanoem.menu.accessory.remove-keyframe";
        break;
    case kMenuItemTypeAccessoryEnableVisible:
        text = "nanoem.menu.accessory.enable.visible";
        break;
    case kMenuItemTypeAccessoryEnableAddBlend:
        text = "nanoem.menu.accessory.enable.add-blend";
        break;
    case kMenuItemTypeAccessoryEnableShadow:
        text = "nanoem.menu.accessory.enable.shadow";
        break;
    case kMenuItemTypeAccessoryReset:
        text = "nanoem.menu.accessory.reset";
        break;
    case kMenuItemTypeAccessoryConvertToModel:
        text = "nanoem.menu.accessory.convert-to-model";
        break;
    case kMenuItemTypeAccessoryDelete:
        text = "nanoem.menu.accessory.delete-active";
        break;
    case kMenuItemTypeMotionTitle:
        text = "nanoem.menu.project.motion.title";
        break;
    case kMenuItemTypeMotionRemoveTimelineFrame:
        text = "nanoem.menu.project.motion.shift-keyframes-forward";
        break;
    case kMenuItemTypeMotionInsertEmptyTimelineFrame:
        text = "nanoem.menu.project.motion.shift-keyframes-backward";
        break;
    case kMenuItemTypeMotionReset:
        text = "nanoem.menu.project.motion.reset";
        break;
    case kMenuItemTypeWindowTitle:
        text = "nanoem.menu.window.title";
        break;
    case kMenuItemTypeWindowMaximize:
        text = "nanoem.menu.window.maximize";
        break;
    case kMenuItemTypeWindowMinimize:
        text = "nanoem.menu.window.minimize";
        break;
    case kMenuItemTypeWindowRestore:
        text = "nanoem.menu.window.restore";
        break;
    case kMenuItemTypeWindowFullscreen:
        text = "nanoem.menu.window.fullscreen";
        break;
    case kMenuItemTypeHelpTitle:
        text = "nanoem.menu.help.title";
        break;
    case kMenuItemTypeHelpOnline:
        text = "nanoem.menu.help.online";
        break;
    case kMenuItemTypeHelpAbout:
        text = "nanoem.menu.help.about";
        break;
    default:
        assert(0);
        text = "";
        break;
    }
    return text;
}

const char *
ApplicationMenuBuilder::stripMnemonic(char *buffer, size_t size, const char *text)
{
    const char *p = text;
    size_t q = 0;
    for (size_t i = 0; i < size && *p; i++) {
        char c = *p;
        if (size - i >= 4 && c == '(' && *(p + 1) == '&' && bx::isAlpha(*(p + 2)) && *(p + 3) == ')') {
            p += 4;
        }
        else if (c == '&') {
            p++;
        }
        else {
            buffer[q++] = c;
            p++;
        }
    }
    buffer[glm::min(q, size - 1)] = 0;
    return buffer;
}

bool
ApplicationMenuBuilder::validateMenuItem(const Project *project, MenuItemType type, MenuItemCheckedState &state)
{
    bool result = type == ApplicationMenuBuilder::kMenuItemTypeFileExit;
    state = ApplicationMenuBuilder::kMenuItemCheckedStateIndetermine;
    if (project) {
        const bool isPlaying = project->isPlaying();
        result = !isPlaying;
        switch (type) {
        case ApplicationMenuBuilder::kMenuItemTypeFileExit: {
            result = true;
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeEditUndo: {
            result &= project->canUndo();
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeEditRedo: {
            result &= project->canRedo();
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeEditCut: {
            result &= project->canCut();
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectStop: {
            result = isPlaying;
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectEnableGrid: {
            state = convertCheckedState(project->grid()->isVisible());
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectEnableGroundShadow: {
            state = convertCheckedState(project->isGroundShadowEnabled());
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectEnableEffect: {
            state = convertCheckedState(project->isEffectPluginEnabled());
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectEnableHighResolutionViewport: {
            const nanoem_f32_t dpr = project->windowDevicePixelRatio();
            state = convertCheckedState(dpr > 1.0f && dpr == project->viewportDevicePixelRatio());
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx16: {
            state = convertCheckedState(project->sampleLevel() == 4);
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx8: {
            state = convertCheckedState(project->sampleLevel() == 3);
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx4: {
            state = convertCheckedState(project->sampleLevel() == 2);
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx2: {
            state = convertCheckedState(project->sampleLevel() == 1);
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectDisableMSAA: {
            state = convertCheckedState(project->sampleLevel() == 0);
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnableAnytime: {
            state = convertCheckedState(
                project->physicsEngine()->simulationMode() == PhysicsEngine::kSimulationModeEnableAnytime);
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnablePlaying: {
            state = convertCheckedState(
                project->physicsEngine()->simulationMode() == PhysicsEngine::kSimulationModeEnablePlaying);
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnableTracing: {
            state = convertCheckedState(
                project->physicsEngine()->simulationMode() == PhysicsEngine::kSimulationModeEnableTracing);
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationDisable: {
            state = convertCheckedState(
                project->physicsEngine()->simulationMode() == PhysicsEngine::kSimulationModeDisable);
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnableDrawingWireframe: {
            state = convertCheckedState(
                EnumUtils::isEnabled(project->physicsEngine()->debugGeometryFlags(), Utils::kDrawWireframe));
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnableDrawingAABB: {
            state = convertCheckedState(
                EnumUtils::isEnabled(project->physicsEngine()->debugGeometryFlags(), Utils::kDrawAabb));
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnableDrawingContactPoints: {
            state = convertCheckedState(
                EnumUtils::isEnabled(project->physicsEngine()->debugGeometryFlags(), Utils::kDrawContactPoints));
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnableDrawingConstraints: {
            state = convertCheckedState(
                EnumUtils::isEnabled(project->physicsEngine()->debugGeometryFlags(), Utils::kDrawConstraints));
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnableDrawingConstraintLimit: {
            state = convertCheckedState(
                EnumUtils::isEnabled(project->physicsEngine()->debugGeometryFlags(), Utils::kDrawConstraintLimits));
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectPreferredMotionFPSUnlimited: {
            state = convertCheckedState(project->isDisplaySyncDisabled());
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectPreferredMotionFPS60: {
            state = convertCheckedState(project->preferredMotionFPS() == 60);
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectPreferredMotionFPS30: {
            state = convertCheckedState(project->preferredMotionFPS() == 30);
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectEnableFPSCounter: {
            state = convertCheckedState(project->isFPSCounterEnabled());
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeProjectEnablePerformanceMonitor: {
            state = convertCheckedState(project->isPerformanceMonitorEnabled());
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeLightSelfShadowDisable: {
            state = convertCheckedState(!project->isShadowMapEnabled());
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeLightSelfShadowEnableWithMode1: {
            state = convertCheckedState(project->isShadowMapEnabled() && project->shadowCamera()->coverageMode() == 1);
            break;
        }
        case ApplicationMenuBuilder::kMenuItemTypeLightSelfShadowEnableWithMode2: {
            state = convertCheckedState(project->isShadowMapEnabled() && project->shadowCamera()->coverageMode() == 2);
            break;
        }
        default: {
            if ((type >= ApplicationMenuBuilder::kMenuItemTypeEditBoneFirstEnum &&
                    type < ApplicationMenuBuilder::kMenuItemTypeEditBoneMaxEnum) ||
                (type >= ApplicationMenuBuilder::kMenuItemTypeEditMorphFirstEnum &&
                    type < ApplicationMenuBuilder::kMenuItemTypeEditMorphMaxEnum) ||
                (type >= ApplicationMenuBuilder::kMenuItemTypeModelFirstEnum &&
                    type < ApplicationMenuBuilder::kMenuItemTypeModelMaxEnum)) {
                if (const Model *activeModel = project->activeModel()) {
                    switch (type) {
                    case ApplicationMenuBuilder::kMenuItemTypeModelEnableAddBlend:
                        state = convertCheckedState(activeModel->isAddBlendEnabled());
                        break;
                    case ApplicationMenuBuilder::kMenuItemTypeModelEnableShadowMap:
                        state = convertCheckedState(activeModel->isShadowMapEnabled());
                        break;
                    case ApplicationMenuBuilder::kMenuItemTypeModelEnableVisible:
                        state = convertCheckedState(activeModel->isVisible());
                        break;
                    case ApplicationMenuBuilder::kMenuItemTypeModelPreferenceEnableShowAllBones:
                        state = convertCheckedState(activeModel->isShowAllBones());
                        break;
                    case ApplicationMenuBuilder::kMenuItemTypeModelPreferenceEnableShowRigidBodies:
                        state = convertCheckedState(activeModel->isShowAllRigidBodyShapes());
                        break;
                    case ApplicationMenuBuilder::kMenuItemTypeModelPreferenceEnableShowJoints:
                        state = convertCheckedState(activeModel->isShowAllJointShapes());
                        break;
                    case ApplicationMenuBuilder::kMenuItemTypeModelPreferenceEnableShowVertexFaces:
                        state = convertCheckedState(activeModel->isShowAllVertexFaces());
                        break;
                    case ApplicationMenuBuilder::kMenuItemTypeModelPreferenceEnableShowVertexPoints:
                        state = convertCheckedState(activeModel->isShowAllVertexPoints());
                        break;
                    default:
                        break;
                    }
                }
                else {
                    result = false;
                }
            }
            else if (type >= ApplicationMenuBuilder::kMenuItemTypeAccessoryFirstEnum &&
                type < ApplicationMenuBuilder::kMenuItemTypeAccessoryMaxEnum) {
                const Accessory *activeAccessory = project->activeAccessory();
                result &= activeAccessory != nullptr;
            }
            break;
        }
        }
    }
    return result;
}

ApplicationMenuBuilder::ApplicationMenuBuilder(BaseApplicationClient *client, bool enableModelEditing)
    : m_client(client)
    , m_fileMenu(nullptr)
    , m_importMenu(nullptr)
    , m_exportMenu(nullptr)
    , m_editMenu(nullptr)
    , m_editBoneMenu(nullptr)
    , m_editCameraMenu(nullptr)
    , m_editMorphMenu(nullptr)
    , m_editModelPluginMenu(nullptr)
    , m_editMotionPluginMenu(nullptr)
    , m_projectMenu(nullptr)
    , m_msaaMenu(nullptr)
    , m_physicsSimulationMenu(nullptr)
    , m_physicsSimulationDebugDrawMenu(nullptr)
    , m_preferredMotionFPSMenu(nullptr)
    , m_cameraMenu(nullptr)
    , m_cameraPresetMenu(nullptr)
    , m_lightMenu(nullptr)
    , m_selfShadowMenu(nullptr)
    , m_modelMenu(nullptr)
    , m_modelPreferenceMenu(nullptr)
    , m_selectModelMenu(nullptr)
    , m_selectBoneMenu(nullptr)
    , m_accessoryMenu(nullptr)
    , m_motionMenu(nullptr)
    , m_selectAccessoryMenu(nullptr)
    , m_windowMenu(nullptr)
    , m_helpMenu(nullptr)
    , m_lazyActiveAccessoryHandle(bx::kInvalidHandle, false)
    , m_lazyActiveModelHandle(bx::kInvalidHandle, false)
    , m_enableModelEditing(enableModelEditing)
{
    Inline::clearZeroMemory(m_selectMorphMenu);
}

ApplicationMenuBuilder::~ApplicationMenuBuilder() NANOEM_DECL_NOEXCEPT
{
}

void
ApplicationMenuBuilder::build()
{
    m_client->addUndoEventListener(handleUndoEvent, this, false);
    m_client->addRedoEventListener(handleRedoEvent, this, false);
    m_client->addUndoChangeEventListener(handleUndoChangeEvent, this, false);
    m_client->addAddAccessoryEventListener(handleAddAccessoryEvent, this, false);
    m_client->addAddModelEventListener(handleAddModelEvent, this, false);
    m_client->addSetActiveAccessoryEventListener(handleSetActiveAccessoryEvent, this, false);
    m_client->addSetActiveModelEventListener(handleSetActiveModelEvent, this, false);
    m_client->addRemoveAccessoryEventListener(handleRemoveAccessoryEvent, this, false);
    m_client->addRemoveModelEventListener(handleRemoveModelEvent, this, false);
    m_client->addPlayEvent(handlePlayEvent, this, false);
    m_client->addStopEvent(handleStopEvent, this, false);
    m_client->addPauseEvent(handlePauseEvent, this, false);
    m_client->addResumeEvent(handleResumeEvent, this, false);
    m_client->addToggleActiveModelAddBlendEnabledEvent(handleActiveModelAddBlendEnabledEvent, this, false);
    m_client->addToggleActiveModelShadowMapEnabledEvent(handleToggleActiveModelShadowMapEnabled, this, false);
    m_client->addToggleActiveModelShowAllBonesEvent(handleToggleActiveModelShowAllBones, this, false);
    m_client->addToggleActiveModelShowAllRigidBodiesEvent(handleToggleActiveModelShowAllRigidBodiesEvent, this, false);
    m_client->addToggleActiveModelShowAllVertexFacesEvent(handleToggleActiveModelShowAllVertexFacesEvent, this, false);
    m_client->addToggleActiveModelShowAllVertexPointsEvent(
        handleToggleActiveModelShowAllVertexPointsEvent, this, false);
    m_client->addToggleActiveModelVisibleEvent(handleToggleActiveModelVisibleEvent, this, false);
    m_client->addToggleGridEnabledEvent(handleToggleGridEnabledEvent, this, false);
    m_client->addToggleProjectEffectEnabledEvent(handleToggleProjectEffectEnabledEvent, this, false);
    m_client->addToggleProjectGroundShadowEnabledEvent(handleToggleProjectGroundShadowEnabledEvent, this, false);
    m_client->addSetPhysicsSimulationModeEvent(handleSetPhysicsSimulationModeEvent, this, false);
    m_client->addSetPreferredMotionFPSEvent(handleSetPreferredMotionFPSEvent, this, false);
    m_client->addSetProjectSampleLevelEvent(handleSetProjectSampleLevelEvent, this, false);
    m_client->addSetShadowMapModeEvent(handleSetShadowMapModeEvent, this, false);
    m_client->addAddModalDialogEventListener(handleAddModalDialogEvent, this, false);
    m_client->addClearModalDialogEventListener(handleClearModalDialogEvent, this, false);
    m_client->addSetLanguageEventListener(handleSetLanguageEvent, this, false);
    m_client->addToggleProjectPlayingWithLoopEventListener(handleToggleProjectPlayingWithLoopEvent, this, false);
    m_client->addToggleActiveAccessoryAddBlendEnabledEventListener(
        handleToggleActiveAccessoryAddBlendEnabledEvent, this, false);
    m_client->addToggleActiveAccessoryShadowEnabledEventListener(
        handleToggleActiveAccessoryShadowEnabledEvent, this, false);
    m_client->addToggleActiveAccessoryVisibleEventListener(handleToggleActiveAccessoryVisibleEvent, this, false);
    m_client->addSetupProjectEventListener(handleSetupProjectEvent, this, false);
    m_client->addSetEditingModeEventListener(handleSetEditingModeEvent, this, false);
    m_client->addSetShadowMapModeEvent(handleShadowMapModeEvent, this, false);
    m_client->addCompleteLoadingAllModelPluginsEventListener(handleCompleteLoadingAllModelPluginsEvent, this, false);
    m_client->addCompleteLoadingAllMotionPluginsEventListener(handleCompleteLoadingAllMotionPluginsEvent, this, false);
    m_client->addCanCopyEventListener(handleCanCopyEvent, this, false);
    m_client->addCanPasteEventListener(handleCanPasteEvent, this, false);
    m_client->addSetWindowDevicePixelRatioEventListener(handleSetWindowPixelRatioEvent, this, false);
    m_client->addSetViewportDevicePixelRatioEventListener(handleSetViewportPixelRatioEvent, this, false);
    m_client->addToggleModelEditingEnabledEventListener(handleToggleModelEditingEnabledEvent, this, false);
}

void
ApplicationMenuBuilder::initialize()
{
    m_wasEnabledItemMap.clear();
    m_menuItems.clear();
}

void
ApplicationMenuBuilder::togglePlaying(bool value)
{
    bool disablePlaying = value ? false : true;
    setMenuItemEnable(kMenuItemTypeProjectPlay, disablePlaying);
    setMenuItemEnable(kMenuItemTypeProjectStop, value ? true : false);
    togglAllMutatingActionMenuItems(value);
}

void
ApplicationMenuBuilder::updateActiveModelMenu(nanoem_u16_t handle)
{
    m_client->sendGetAllModelBonesRequestMessage(handle, handleGetAllModelBonesRequest, this);
    m_client->sendGetAllModelMorphsRequestMessages(handle, handleGetAllModelMorphsRequest, this);
}

void
ApplicationMenuBuilder::createFileMenu(MainMenuBarHandle mainMenu)
{
    MenuItemHandle fileMenu = createMenuItem(mainMenu);
    m_fileMenu = createMenuBar(kMenuItemTypeFileTitle);
    setParentMenu(fileMenu, m_fileMenu);
    appendMenuItem(m_fileMenu, kMenuItemTypeFileNewProject);
    if (m_enableModelEditing) {
        appendMenuItem(m_fileMenu, kMenuItemTypeFileNewModel);
    }
    appendMenuSeparator(m_fileMenu);
    appendMenuItem(m_fileMenu, kMenuItemTypeFileOpenProject);
    m_importMenu = createMenuBar();
    appendMenuItem(m_importMenu, kMenuItemTypeFileImportModel);
    appendMenuItem(m_importMenu, kMenuItemTypeFileImportAccessory);
    appendMenuSeparator(m_importMenu);
    setMenuItemEnabled(appendMenuItem(m_importMenu, kMenuItemTypeFileImportModelMotion), false);
    appendMenuItem(m_importMenu, kMenuItemTypeFileImportCameraMotion);
    appendMenuItem(m_importMenu, kMenuItemTypeFileImportLightMotion);
    appendMenuSeparator(m_importMenu);
    setMenuItemEnabled(appendMenuItem(m_importMenu, kMenuItemTypeFileImportModelPose), false);
    appendMenuSeparator(m_importMenu);
    appendMenuItem(m_importMenu, kMenuItemTypeFileImportAudioSource);
    appendMenuItem(m_importMenu, kMenuItemTypeFileImportBackgroundVideo);
    MenuItemHandle importMenuItem = appendMenuItem(m_fileMenu, kMenuItemTypeFileImportTitle);
    setParentMenu(importMenuItem, m_importMenu);
    appendMenuSeparator(m_fileMenu);
    appendMenuItem(m_fileMenu, kMenuItemTypeFileSaveProject);
    appendMenuItem(m_fileMenu, kMenuItemTypeFileSaveAsProject);
    appendMenuSeparator(m_fileMenu);
    m_exportMenu = createMenuBar();
    setMenuItemEnabled(appendMenuItem(m_exportMenu, kMenuItemTypeFileExportModelPose), false);
    appendMenuSeparator(m_exportMenu);
    setMenuItemEnabled(appendMenuItem(m_exportMenu, kMenuItemTypeFileExportModelMotion), false);
    appendMenuItem(m_exportMenu, kMenuItemTypeFileExportCameraMotion);
    appendMenuItem(m_exportMenu, kMenuItemTypeFileExportLightMotion);
    if (m_enableModelEditing) {
        appendMenuSeparator(m_exportMenu);
        appendMenuItem(m_exportMenu, kMenuItemTypeFileExportModel);
    }
    appendMenuSeparator(m_exportMenu);
    appendMenuItem(m_exportMenu, kMenuItemTypeFileExportImage);
    appendMenuItem(m_exportMenu, kMenuItemTypeFileExportVideo);
    MenuItemHandle exportMenuItem = appendMenuItem(m_fileMenu, kMenuItemTypeFileExportTitle);
    setParentMenu(exportMenuItem, m_exportMenu);
    appendMenuSeparator(m_fileMenu);
    appendMenuItem(m_fileMenu, kMenuItemTypeFileExit);
}

void
ApplicationMenuBuilder::createEditMenu(MainMenuBarHandle bar)
{
    MenuItemHandle editMenuItem = createMenuItem(bar);
    m_editMenu = createMenuBar(kMenuItemTypeEditTitle);
    setParentMenu(editMenuItem, m_editMenu);
    setMenuItemEnabled(appendMenuItem(m_editMenu, kMenuItemTypeEditUndo), false);
    setMenuItemEnabled(appendMenuItem(m_editMenu, kMenuItemTypeEditRedo), false);
    appendMenuSeparator(m_editMenu);
    setMenuItemEnabled(appendMenuItem(m_editMenu, kMenuItemTypeEditCopy), false);
    setMenuItemEnabled(appendMenuItem(m_editMenu, kMenuItemTypeEditPaste), false);
    setMenuItemEnabled(appendMenuItem(m_editMenu, kMenuItemTypeEditCut), false);
    appendMenuSeparator(m_editMenu);
    createEditMotionMenu(m_editMenu);
    createEditBoneMenu(m_editMenu);
    createEditCameraMenu(m_editMenu);
    createEditMorphMenu(m_editMenu);
    appendMenuSeparator(m_editMenu);
    createEditModelPluginMenu(m_editMenu);
    createEditMotionPluginMenu(m_editMenu);
    appendMenuSeparator(m_editMenu);
    appendMenuItem(m_editMenu, kMenuItemTypeEditOpenEffectParameterWindow);
    if (m_enableModelEditing) {
        setMenuItemEnabled(appendMenuItem(m_editMenu, kMenuItemTypeEditOpenModelParameterWindow), false);
    }
    appendMenuSeparator(m_editMenu);
    appendMenuItem(m_editMenu, kMenuItemTypeEditSelectAll);
}

void
ApplicationMenuBuilder::createEditMotionMenu(MenuBarHandle editMenu)
{
    MenuItemHandle motionMenuItem = appendMenuItem(editMenu, kMenuItemTypeMotionTitle);
    m_motionMenu = createMenuBar();
    setParentMenu(motionMenuItem, m_motionMenu);
    appendMenuSeparator(m_motionMenu);
    appendMenuItem(m_motionMenu, kMenuItemTypeMotionInsertEmptyTimelineFrame);
    appendMenuItem(m_motionMenu, kMenuItemTypeMotionRemoveTimelineFrame);
    appendMenuSeparator(m_motionMenu);
    appendMenuItem(m_motionMenu, kMenuItemTypeMotionReset);
}

void
ApplicationMenuBuilder::createEditBoneMenu(MenuBarHandle editMenu)
{
    MenuItemHandle editBoneMenuItem = appendMenuItem(editMenu, kMenuItemTypeEditBoneTitle);
    m_editBoneMenu = createMenuBar();
    appendMenuItem(m_editBoneMenu, kMenuItemTypeEditBoneOpenParameterDialog);
    appendMenuItem(m_editBoneMenu, kMenuItemTypeEditBoneOpenCorrectionDialog);
    appendMenuItem(m_editBoneMenu, kMenuItemTypeEditBoneOpenBiasDialog);
    appendMenuSeparator(m_editBoneMenu);
    appendMenuItem(m_editBoneMenu, kMenuItemTypeModelEditModeSelect);
    appendMenuItem(m_editBoneMenu, kMenuItemTypeModelEditModeRotate);
    appendMenuItem(m_editBoneMenu, kMenuItemTypeModelEditModeMove);
    appendMenuSeparator(m_editBoneMenu);
    appendMenuItem(m_editBoneMenu, kMenuItemTypeModelResetBoneAxisX);
    appendMenuItem(m_editBoneMenu, kMenuItemTypeModelResetBoneAxisY);
    appendMenuItem(m_editBoneMenu, kMenuItemTypeModelResetBoneAxisZ);
    appendMenuSeparator(m_editBoneMenu);
    appendMenuItem(m_editBoneMenu, kMenuItemTypeEditBoneResetAngle);
    setParentMenu(editBoneMenuItem, m_editBoneMenu);
}

void
ApplicationMenuBuilder::createEditCameraMenu(MenuBarHandle editMenu)
{
    MenuItemHandle editCameraMenuItem = appendMenuItem(editMenu, kMenuItemTypeEditCameraTitle);
    m_editCameraMenu = createMenuBar();
    appendMenuItem(m_editCameraMenu, kMenuItemTypeEditCameraOpenParameterDialog);
    appendMenuItem(m_editCameraMenu, kMenuItemTypeEditCameraOpenCorrectionDialog);
    appendMenuSeparator(m_editCameraMenu);
    appendMenuItem(m_editCameraMenu, kMenuItemTypeEditCameraResetAngle);
    setParentMenu(editCameraMenuItem, m_editCameraMenu);
}

void
ApplicationMenuBuilder::createEditMorphMenu(MenuBarHandle editMenu)
{
    MenuItemHandle editMorphMenuItem = appendMenuItem(editMenu, kMenuItemTypeEditMorphTitle);
    m_editMorphMenu = createMenuBar();
    appendMenuItem(m_editMorphMenu, kMenuItemTypeEditMorphOpenCorrectionDialog);
    appendMenuSeparator(m_editMorphMenu);
    appendMenuItem(m_editMorphMenu, kMenuItemTypeEditMorphRemoveAllLipKeyframes);
    appendMenuItem(m_editMorphMenu, kMenuItemTypeEditMorphRemoveAllEyeKeyframes);
    appendMenuItem(m_editMorphMenu, kMenuItemTypeEditMorphRemoveAllEyebrowKeyframes);
    appendMenuSeparator(m_editMorphMenu);
    appendMenuItem(m_editMorphMenu, kMenuItemTypeModelMorphSetZero);
    appendMenuItem(m_editMorphMenu, kMenuItemTypeModelMorphSetHalf);
    appendMenuItem(m_editMorphMenu, kMenuItemTypeModelMorphSetOne);
    appendMenuSeparator(m_editMorphMenu);
    appendMenuItem(m_editMorphMenu, kMenuItemTypeEditMorphResetAllMorphs);
    appendMenuItem(m_editMorphMenu, kMenuItemTypeEditMorphRegisterAllMorphs);
    setParentMenu(editMorphMenuItem, m_editMorphMenu);
}

void
ApplicationMenuBuilder::createEditModelPluginMenu(MenuBarHandle editMenu)
{
    MenuItemHandle editModelPluginMenuItem = appendMenuItem(editMenu, kMenuItemTypeEditModelPluginTitle);
    m_editModelPluginMenu = createMenuBar();
    setParentMenu(editModelPluginMenuItem, m_editModelPluginMenu);
}

void
ApplicationMenuBuilder::createEditMotionPluginMenu(MenuBarHandle editMenu)
{
    MenuItemHandle editMotionPluginMenuItem = appendMenuItem(editMenu, kMenuItemTypeEditMotionPluginTitle);
    m_editMotionPluginMenu = createMenuBar();
    setParentMenu(editMotionPluginMenuItem, m_editMotionPluginMenu);
}

void
ApplicationMenuBuilder::createProjectMSAAMenu(MenuBarHandle projectMenu)
{
    MenuItemHandle msaaMenuItem = appendMenuItem(projectMenu, kMenuItemTypeProjectMSAATitle);
    m_msaaMenu = createMenuBar();
    setParentMenu(msaaMenuItem, m_msaaMenu);
}

void
ApplicationMenuBuilder::createProjectPhysicsSimulationMenu(MenuBarHandle projectMenu)
{
    MenuItemHandle physicsSimulationMenuItem = appendMenuItem(projectMenu, kMenuItemTypeProjectPhysicsSimulationTitle);
    m_physicsSimulationMenu = createMenuBar();
    setParentMenu(physicsSimulationMenuItem, m_physicsSimulationMenu);
    appendMenuItem(m_physicsSimulationMenu, kMenuItemTypeProjectPhysicsSimulationEnableAnytime);
    appendMenuItem(m_physicsSimulationMenu, kMenuItemTypeProjectPhysicsSimulationEnablePlaying);
    appendMenuItem(m_physicsSimulationMenu, kMenuItemTypeProjectPhysicsSimulationEnableTracing);
    appendMenuItem(m_physicsSimulationMenu, kMenuItemTypeProjectPhysicsSimulationDisable);
    appendMenuSeparator(m_physicsSimulationMenu);
    appendMenuItem(m_physicsSimulationMenu, kMenuItemTypeProjectPhysicsSimulationBakeAllMotions);
    appendMenuItem(m_physicsSimulationMenu, kMenuItemTypeProjectPhysicsSimulationBakeAllMotionsWithIK);
    appendMenuSeparator(m_physicsSimulationMenu);
    appendMenuItem(m_physicsSimulationMenu, kMenuItemTypeProjectPhysicsSimulationConfiguration);
    appendMenuSeparator(m_physicsSimulationMenu);
    createProjectPhysicsSimulationDebugDrawMenu(m_physicsSimulationMenu);
}

void
ApplicationMenuBuilder::createProjectPreferredMotionFPSMenu(MenuBarHandle projectMenu)
{
    MenuItemHandle preferredFPSMenuItem = appendMenuItem(projectMenu, kMenuItemTypeProjectPreferredFPSTitle);
    m_preferredMotionFPSMenu = createMenuBar();
    setParentMenu(preferredFPSMenuItem, m_preferredMotionFPSMenu);
    appendMenuItem(m_preferredMotionFPSMenu, kMenuItemTypeProjectPreferredMotionFPSUnlimited);
    appendMenuItem(m_preferredMotionFPSMenu, kMenuItemTypeProjectPreferredMotionFPS60);
    appendMenuItem(m_preferredMotionFPSMenu, kMenuItemTypeProjectPreferredMotionFPS30);
}

void
ApplicationMenuBuilder::createProjectPhysicsSimulationDebugDrawMenu(MenuBarHandle physicsSimulationMenu)
{
    MenuItemHandle physicsSimulationDebugDrawMenuItem =
        appendMenuItem(physicsSimulationMenu, kMenuItemTypeProjectPhysicsSimulationDebugDrawTitle);
    m_physicsSimulationDebugDrawMenu = createMenuBar();
    setParentMenu(physicsSimulationDebugDrawMenuItem, m_physicsSimulationDebugDrawMenu);
    appendMenuItem(m_physicsSimulationDebugDrawMenu, kMenuItemTypeProjectPhysicsSimulationEnableDrawingAABB);
    appendMenuItem(m_physicsSimulationDebugDrawMenu, kMenuItemTypeProjectPhysicsSimulationEnableDrawingWireframe);
    appendMenuItem(m_physicsSimulationDebugDrawMenu, kMenuItemTypeProjectPhysicsSimulationEnableDrawingContactPoints);
    appendMenuItem(m_physicsSimulationDebugDrawMenu, kMenuItemTypeProjectPhysicsSimulationEnableDrawingConstraints);
    appendMenuItem(m_physicsSimulationDebugDrawMenu, kMenuItemTypeProjectPhysicsSimulationEnableDrawingConstraintLimit);
}

void
ApplicationMenuBuilder::createCameraMenu(MainMenuBarHandle bar)
{
    MenuItemHandle cameraMenuItem = createMenuItem(bar);
    m_cameraMenu = createMenuBar(kMenuItemTypeCameraTitle);
    setParentMenu(cameraMenuItem, m_cameraMenu);
    MenuItemHandle cameraPresetMenuItem = appendMenuItem(m_cameraMenu, kMenuItemTypeCameraPresetTitle);
    m_cameraPresetMenu = createMenuBar();
    appendMenuItem(m_cameraPresetMenu, kMenuItemTypeCameraPresetTop);
    appendMenuItem(m_cameraPresetMenu, kMenuItemTypeCameraPresetLeft);
    appendMenuItem(m_cameraPresetMenu, kMenuItemTypeCameraPresetRight);
    appendMenuItem(m_cameraPresetMenu, kMenuItemTypeCameraPresetBottom);
    appendMenuItem(m_cameraPresetMenu, kMenuItemTypeCameraPresetFront);
    appendMenuItem(m_cameraPresetMenu, kMenuItemTypeCameraPresetBack);
    setParentMenu(cameraPresetMenuItem, m_cameraPresetMenu);
    appendMenuSeparator(m_cameraMenu);
    appendMenuItem(m_cameraMenu, kMenuItemTypeCameraRegisterKeyframe);
    appendMenuItem(m_cameraMenu, kMenuItemTypeCameraRemoveAllSelectedKeyframes);
    appendMenuSeparator(m_cameraMenu);
    appendMenuItem(m_cameraMenu, kMenuItemTypeCameraReset);
}

void
ApplicationMenuBuilder::createLightMenu(MainMenuBarHandle bar)
{
    MenuItemHandle lightMenuItem = createMenuItem(bar);
    m_lightMenu = createMenuBar(kMenuItemTypeLightTitle);
    setParentMenu(lightMenuItem, m_lightMenu);
    MenuItemHandle selfShadowMenuItem = appendMenuItem(m_lightMenu, kMenuItemTypeLightSelfShadowTitle);
    m_selfShadowMenu = createMenuBar();
    setParentMenu(selfShadowMenuItem, m_selfShadowMenu);
    appendMenuItem(m_selfShadowMenu, kMenuItemTypeLightSelfShadowDisable);
    appendMenuItem(m_selfShadowMenu, kMenuItemTypeLightSelfShadowEnableWithMode1);
    appendMenuItem(m_selfShadowMenu, kMenuItemTypeLightSelfShadowEnableWithMode2);
    appendMenuSeparator(m_selfShadowMenu);
    appendMenuItem(m_selfShadowMenu, kMenuItemTypeLightSelfShadowRegisterKeyframe);
    appendMenuItem(m_selfShadowMenu, kMenuItemTypeLightSelfShadowRemoveAllSelectedKeyframes);
    appendMenuSeparator(m_selfShadowMenu);
    appendMenuItem(m_selfShadowMenu, kMenuItemTypeLightSelfShadowReset);
    appendMenuSeparator(m_lightMenu);
    appendMenuItem(m_lightMenu, kMenuItemTypeLightRegisterKeyframe);
    appendMenuItem(m_lightMenu, kMenuItemTypeLightRemoveAllSelectedKeyframes);
    appendMenuSeparator(m_lightMenu);
    appendMenuItem(m_lightMenu, kMenuItemTypeLightReset);
    setMenuItemChecked(kMenuItemTypeLightSelfShadowEnableWithMode1, true);
}

void
ApplicationMenuBuilder::createModelMenu(MainMenuBarHandle bar)
{
    MenuItemHandle modelMenuItem = createMenuItem(bar);
    m_modelMenu = createMenuBar(kMenuItemTypeModelTitle);
    setParentMenu(modelMenuItem, m_modelMenu);
    MenuItemHandle selectModelMenuItem = appendMenuItem(m_modelMenu, kMenuItemTypeModelSelectTitle);
    m_selectModelMenu = createMenuBar();
    setParentMenu(selectModelMenuItem, m_selectModelMenu);
    appendMenuSeparator(m_modelMenu);
    createModelBoneMenu(m_modelMenu);
    createModelMorphMenu(m_modelMenu);
    appendMenuItem(m_modelMenu, kMenuItemTypeModelEdgeConfiguraiton);
    appendMenuItem(m_modelMenu, kMenuItemTypeModelMeasureHeight);
    appendMenuItem(m_modelMenu, kMenuItemTypeModelPerformValidation);
    appendMenuSeparator(m_modelMenu);
    appendMenuItem(m_modelMenu, kMenuItemTypeModelRegisterKeyframe);
    appendMenuItem(m_modelMenu, kMenuItemTypeModelRemoveAllSelectedKeyframes);
    appendMenuSeparator(m_modelMenu);
    appendMenuItem(m_modelMenu, kMenuItemTypeModelEnableAddBlend);
    appendMenuItem(m_modelMenu, kMenuItemTypeModelEnableShadowMap);
    appendMenuItem(m_modelMenu, kMenuItemTypeModelEnableVisible);
    createModelPreferenceMenu(m_modelMenu);
    appendMenuSeparator(m_modelMenu);
    appendMenuItem(m_modelMenu, kMenuItemTypeModelReset);
    appendMenuSeparator(m_modelMenu);
    appendMenuItem(m_modelMenu, kMenuItemTypeModelDelete);
    setAllModelMenuItemsEnabled(false);
}

void
ApplicationMenuBuilder::createModelBoneMenu(MenuBarHandle modelMenu)
{
    MenuItemHandle selectBoneMenuItem = appendMenuItem(modelMenu, kMenuItemTypeModelSelectBoneTitle);
    m_selectBoneMenu = createMenuBar();
    setParentMenu(selectBoneMenuItem, m_selectBoneMenu);
}

void
ApplicationMenuBuilder::createModelMorphMenu(MenuBarHandle modelMenu)
{
    MenuItemHandle selectMorphMenuItem = appendMenuItem(modelMenu, kMenuItemTypeModelSelectMorphTitle);
    MenuBarHandle selectMorphMenu = m_selectMorphMenu[NANOEM_MODEL_MORPH_CATEGORY_BASE] = createMenuBar();
    setParentMenu(selectMorphMenuItem, selectMorphMenu);
    MenuBarHandle selectMorphEyeMenu = m_selectMorphMenu[NANOEM_MODEL_MORPH_CATEGORY_EYE] = createMenuBar();
    setParentMenu(appendMenuItem(selectMorphMenu, kMenuItemTypeModelSelectMorphEyeTitle), selectMorphEyeMenu);
    MenuBarHandle selectMorphEyebrowMenu = m_selectMorphMenu[NANOEM_MODEL_MORPH_CATEGORY_EYEBROW] = createMenuBar();
    setParentMenu(appendMenuItem(selectMorphMenu, kMenuItemTypeModelSelectMorphEyebrowTitle), selectMorphEyebrowMenu);
    MenuBarHandle selectMorphLipMenu = m_selectMorphMenu[NANOEM_MODEL_MORPH_CATEGORY_LIP] = createMenuBar();
    setParentMenu(appendMenuItem(selectMorphMenu, kMenuItemTypeModelSelectMorphLipTitle), selectMorphLipMenu);
    MenuBarHandle selectMorphOtherMenu = m_selectMorphMenu[NANOEM_MODEL_MORPH_CATEGORY_OTHER] = createMenuBar();
    setParentMenu(appendMenuItem(selectMorphMenu, kMenuItemTypeModelSelectMorphOtherTitle), selectMorphOtherMenu);
}

void
ApplicationMenuBuilder::createModelPreferenceMenu(MenuBarHandle modelMenu)
{
    MenuItemHandle modelPreferenceMenuItem = appendMenuItem(modelMenu, kMenuItemTypeModelPreferenceTitle);
    m_modelPreferenceMenu = createMenuBar();
    setParentMenu(modelPreferenceMenuItem, m_modelPreferenceMenu);
    appendMenuItem(m_modelPreferenceMenu, kMenuItemTypeModelPreferenceEnableShowAllBones);
    appendMenuItem(m_modelPreferenceMenu, kMenuItemTypeModelPreferenceEnableShowRigidBodies);
    appendMenuItem(m_modelPreferenceMenu, kMenuItemTypeModelPreferenceEnableShowJoints);
    appendMenuItem(m_modelPreferenceMenu, kMenuItemTypeModelPreferenceEnableShowVertexFaces);
    appendMenuItem(m_modelPreferenceMenu, kMenuItemTypeModelPreferenceEnableShowVertexPoints);
}

void
ApplicationMenuBuilder::createAccessoryMenu(MainMenuBarHandle bar)
{
    MenuItemHandle accessoryMenuItem = createMenuItem(bar);
    m_accessoryMenu = createMenuBar(kMenuItemTypeAccessoryTitle);
    setParentMenu(accessoryMenuItem, m_accessoryMenu);
    MenuItemHandle selectAccessoryMenuItem = appendMenuItem(m_accessoryMenu, kMenuItemTypeAccessorySelectTitle);
    m_selectAccessoryMenu = createMenuBar();
    setParentMenu(selectAccessoryMenuItem, m_selectAccessoryMenu);
    appendMenuSeparator(m_accessoryMenu);
    appendMenuItem(m_accessoryMenu, kMenuItemTypeAccessoryRegisterKeyframe);
    appendMenuItem(m_accessoryMenu, kMenuItemTypeAccessoryRemoveAllSelectedKeyframes);
    appendMenuSeparator(m_accessoryMenu);
    appendMenuItem(m_accessoryMenu, kMenuItemTypeAccessoryEnableAddBlend);
    appendMenuItem(m_accessoryMenu, kMenuItemTypeAccessoryEnableShadow);
    appendMenuItem(m_accessoryMenu, kMenuItemTypeAccessoryEnableVisible);
    appendMenuSeparator(m_accessoryMenu);
    appendMenuItem(m_accessoryMenu, kMenuItemTypeAccessoryReset);
    appendMenuSeparator(m_accessoryMenu);
    if (m_enableModelEditing) {
        appendMenuItem(m_accessoryMenu, kMenuItemTypeAccessoryConvertToModel);
    }
    appendMenuItem(m_accessoryMenu, kMenuItemTypeAccessoryDelete);
    setAllAccessoryMenuItemsEnabled(false);
}

void
ApplicationMenuBuilder::createProjectMenu(MainMenuBarHandle bar)
{
    MenuItemHandle projectMenuItem = createMenuItem(bar);
    m_projectMenu = createMenuBar(kMenuItemTypeProjectTitle);
    setParentMenu(projectMenuItem, m_projectMenu);
    appendMenuItem(m_projectMenu, kMenuItemTypeProjectPlay);
    setMenuItemEnabled(appendMenuItem(m_projectMenu, kMenuItemTypeProjectStop), false);
    appendMenuSeparator(m_projectMenu);
    appendMenuItem(m_projectMenu, kMenuItemTypeProjectOpenViewportDialog);
    appendMenuItem(m_projectMenu, kMenuItemTypeProjectOpenDrawOrderDialog);
    appendMenuItem(m_projectMenu, kMenuItemTypeProjectOpenTransformOrderDialog);
    appendMenuItem(m_projectMenu, kMenuItemTypeProjectDetachViewportWindow);
    appendMenuSeparator(m_projectMenu);
    appendMenuItem(m_projectMenu, kMenuItemTypeProjectEnableLoop);
    appendMenuItem(m_projectMenu, kMenuItemTypeProjectEnableGrid);
    appendMenuItem(m_projectMenu, kMenuItemTypeProjectEnableGroundShadow);
    appendMenuItem(m_projectMenu, kMenuItemTypeProjectEnableEffect);
    appendMenuSeparator(m_projectMenu);
    createProjectMSAAMenu(m_projectMenu);
    createProjectPhysicsSimulationMenu(m_projectMenu);
    createProjectPreferredMotionFPSMenu(m_projectMenu);
    appendMenuSeparator(m_projectMenu);
    appendMenuItem(m_projectMenu, kMenuItemTypeProjectClearAudioSource);
    appendMenuItem(m_projectMenu, kMenuItemTypeProjectClearBackgroundVideo);
    appendMenuSeparator(m_projectMenu);
    appendMenuItem(m_projectMenu, kMenuItemTypeProjectEnableHighResolutionViewport);
    appendMenuItem(m_projectMenu, kMenuItemTypeProjectEnableFPSCounter);
    appendMenuItem(m_projectMenu, kMenuItemTypeProjectEnablePerformanceMonitor);
    setMenuItemChecked(kMenuItemTypeProjectEnableMSAAx2, true);
    setMenuItemChecked(kMenuItemTypeProjectEnableFPSCounter, true);
    setMenuItemChecked(kMenuItemTypeProjectEnablePerformanceMonitor, true);
}

bool
ApplicationMenuBuilder::isMenuItemEnabled(MenuItemType type) const NANOEM_DECL_NOEXCEPT
{
    MenuItemMap::const_iterator it = m_menuItems.find(type);
    bool enabled = false;
    if (it != m_menuItems.end()) {
        enabled = isMenuItemEnabled(it->second);
    }
    return enabled;
}

void
ApplicationMenuBuilder::setMenuItemEnable(MenuItemType first, MenuItemType last, bool value)
{
    for (size_t i = size_t(first); i < size_t(last); i++) {
        MenuItemType type = static_cast<MenuItemType>(i);
        setMenuItemEnable(type, value);
    }
}

bool
ApplicationMenuBuilder::isMenuItemChecked(MenuItemType type) const NANOEM_DECL_NOEXCEPT
{
    MenuItemMap::const_iterator it = m_menuItems.find(type);
    bool checked = false;
    if (it != m_menuItems.end()) {
        checked = isMenuItemChecked(it->second);
    }
    return checked;
}

void
ApplicationMenuBuilder::setMenuItemEnable(MenuItemType type, bool value)
{
    if (m_wasEnabledItemMap.empty()) {
        MenuItemMap::const_iterator it = m_menuItems.find(type);
        if (it != m_menuItems.end()) {
            setMenuItemEnabled(it->second, value);
        }
    }
}

void
ApplicationMenuBuilder::setMenuItemChecked(MenuItemType type, bool value)
{
    MenuItemMap::const_iterator it = m_menuItems.find(type);
    if (it != m_menuItems.end()) {
        setMenuItemChecked(it->second, value);
    }
}

void
ApplicationMenuBuilder::setAllAccessoryMenuItemsEnabled(bool value)
{
    setMenuItemEnable(kMenuItemTypeAccessoryFirstEnum, kMenuItemTypeAccessoryMaxEnum, value);
    setMenuItemEnable(kMenuItemTypeAccessorySelectTitle, true);
}

void
ApplicationMenuBuilder::setAllModelMenuItemsEnabled(bool value)
{
    setMenuItemEnable(kMenuItemTypeEditModelPluginTitle, value);
    setMenuItemEnable(kMenuItemTypeEditBoneFirstEnum, kMenuItemTypeEditBoneMaxEnum, value);
    setMenuItemEnable(kMenuItemTypeEditMorphFirstEnum, kMenuItemTypeEditMorphMaxEnum, value);
    setMenuItemEnable(kMenuItemTypeModelFirstEnum, kMenuItemTypeModelMaxEnum, value);
    setMenuItemEnable(kMenuItemTypeModelSelectTitle, true);
}

void
ApplicationMenuBuilder::startModalDialog()
{
    if (m_wasEnabledItemMap.empty()) {
        for (MenuItemMap::const_iterator it = m_menuItems.begin(), end = m_menuItems.end(); it != end; ++it) {
            m_wasEnabledItemMap[it->first] = isMenuItemEnabled(it->second);
            setMenuItemEnabled(it->second, false);
        }
    }
}

void
ApplicationMenuBuilder::stopModalDialog()
{
    if (!m_wasEnabledItemMap.empty()) {
        for (MenuItemMap::const_iterator it = m_menuItems.begin(), end = m_menuItems.end(); it != end; ++it) {
            setMenuItemEnabled(it->second, m_wasEnabledItemMap[it->first]);
        }
        m_wasEnabledItemMap.clear();
        if (m_lazyActiveAccessoryHandle.second) {
            handleSetActiveAccessoryEvent(this, m_lazyActiveAccessoryHandle.first, nullptr);
            m_lazyActiveAccessoryHandle.second = false;
        }
        if (m_lazyActiveModelHandle.second) {
            handleSetActiveModelEvent(this, m_lazyActiveModelHandle.first, nullptr);
            m_lazyActiveModelHandle.second = false;
        }
    }
}

ApplicationMenuBuilder::MenuItemCheckedState
ApplicationMenuBuilder::convertCheckedState(bool value)
{
    return value ? kMenuItemCheckedStateTrue : kMenuItemCheckedStateFalse;
}

void
ApplicationMenuBuilder::handleUndoEvent(void *userData, bool canUndo, bool canRedo)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemEnable(kMenuItemTypeEditUndo, canUndo);
    self->setMenuItemEnable(kMenuItemTypeEditRedo, canRedo);
}

void
ApplicationMenuBuilder::handleRedoEvent(void *userData, bool canRedo, bool canUndo)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemEnable(kMenuItemTypeEditUndo, canUndo);
    self->setMenuItemEnable(kMenuItemTypeEditRedo, canRedo);
}

void
ApplicationMenuBuilder::handleUndoChangeEvent(void *userData)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemEnable(kMenuItemTypeEditUndo, true);
}

void
ApplicationMenuBuilder::handleAddAccessoryEvent(void *userData, nanoem_u16_t handle, const char *name)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->createSelectAccessoryMenuItem(self->m_selectAccessoryMenu, handle, name);
}

void
ApplicationMenuBuilder::handleAddModelEvent(void *userData, nanoem_u16_t handle, const char *name)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->createSelectModelMenuItem(self->m_selectModelMenu, handle, name);
}

void
ApplicationMenuBuilder::handleSetActiveAccessoryEvent(void *userData, nanoem_u16_t handle, const char * /* name */)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    const bool enabled = handle != bx::kInvalidHandle;
    /* in the progress dialog, m_enabledItemMap is not empty */
    if (self->m_wasEnabledItemMap.empty()) {
        self->setAllAccessoryMenuItemsEnabled(enabled);
        self->updateAllSelectDrawableItems(self->m_selectAccessoryMenu, handle);
    }
    else {
        self->m_lazyActiveAccessoryHandle.first = handle;
        self->m_lazyActiveAccessoryHandle.second = true;
    }
}

void
ApplicationMenuBuilder::handleSetActiveModelEvent(void *userData, nanoem_u16_t handle, const char * /* name */)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    const bool enabled = handle != bx::kInvalidHandle;
    /* in the progress dialog, skip process due to m_enabledItemMap is not empty */
    if (self->m_wasEnabledItemMap.empty()) {
        self->setAllModelMenuItemsEnabled(enabled);
        if (enabled) {
            self->updateActiveModelMenu(handle);
        }
        if (self->m_enableModelEditing) {
            self->setMenuItemEnable(kMenuItemTypeEditOpenModelParameterWindow, enabled);
        }
        self->setMenuItemEnable(kMenuItemTypeFileImportModelMotion, enabled);
        self->setMenuItemEnable(kMenuItemTypeFileImportModelPose, enabled);
        self->setMenuItemEnable(kMenuItemTypeFileExportModel, enabled);
        self->setMenuItemEnable(kMenuItemTypeFileExportModelMotion, enabled);
        self->setMenuItemEnable(kMenuItemTypeFileExportModelPose, enabled);
        self->setMenuItemChecked(kMenuItemTypeModelEditModeSelect, enabled);
        self->updateAllSelectDrawableItems(self->m_selectModelMenu, handle);
        self->m_client->sendLoadAllModelPluginsMessage(URIList());
    }
    else {
        self->m_lazyActiveModelHandle.first = handle;
        self->m_lazyActiveModelHandle.second = true;
    }
}

void
ApplicationMenuBuilder::handleRemoveAccessoryEvent(void *userData, nanoem_u16_t handle, const char * /* name */)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->removeMenuItemById(self->m_selectAccessoryMenu, handle);
}

void
ApplicationMenuBuilder::handleRemoveModelEvent(void *userData, nanoem_u16_t handle, const char * /* name */)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->clearAllMenuItems(self->m_selectBoneMenu);
    for (size_t i = 0; i < BX_COUNTOF(self->m_selectMorphMenu); i++) {
        self->clearAllMenuItems(self->m_selectMorphMenu[i]);
    }
    self->removeMenuItemById(self->m_selectModelMenu, handle);
}

void
ApplicationMenuBuilder::handlePlayEvent(
    void *userData, nanoem_frame_index_t /* duration */, nanoem_frame_index_t /* localFrameIndex */)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->togglePlaying(true);
}

void
ApplicationMenuBuilder::handleStopEvent(
    void *userData, nanoem_frame_index_t /* duration */, nanoem_frame_index_t /* localFrameIndex */)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->togglePlaying(false);
}

void
ApplicationMenuBuilder::handlePauseEvent(
    void *userData, nanoem_frame_index_t /* duration */, nanoem_frame_index_t /* localFrameIndex */)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemEnable(kMenuItemTypeProjectPlay, true);
    self->setMenuItemEnable(kMenuItemTypeProjectStop, true);
    self->togglAllMutatingActionMenuItems(false);
}

void
ApplicationMenuBuilder::handleResumeEvent(
    void *userData, nanoem_frame_index_t /* duration */, nanoem_frame_index_t /* localFrameIndex */)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->togglePlaying(true);
}

void
ApplicationMenuBuilder::handleActiveModelAddBlendEnabledEvent(void *userData, bool value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeModelEnableAddBlend, value);
}

void
ApplicationMenuBuilder::handleToggleActiveModelShadowMapEnabled(void *userData, bool value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeModelEnableShadowMap, value);
}

void
ApplicationMenuBuilder::handleToggleActiveModelShowAllBones(void *userData, bool value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeModelPreferenceEnableShowAllBones, value);
}

void
ApplicationMenuBuilder::handleToggleActiveModelShowAllRigidBodiesEvent(void *userData, bool value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeModelPreferenceEnableShowRigidBodies, value);
}

void
ApplicationMenuBuilder::handleToggleActiveModelShowAllVertexFacesEvent(void *userData, bool value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeModelPreferenceEnableShowVertexFaces, value);
}

void
ApplicationMenuBuilder::handleToggleActiveModelShowAllVertexPointsEvent(void *userData, bool value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeModelPreferenceEnableShowVertexPoints, value);
}

void
ApplicationMenuBuilder::handleToggleActiveModelVisibleEvent(void *userData, bool value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeModelEnableVisible, value);
}

void
ApplicationMenuBuilder::handleToggleGridEnabledEvent(void *userData, bool value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeProjectEnableGrid, value);
}

void
ApplicationMenuBuilder::handleToggleProjectEffectEnabledEvent(void *userData, bool value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeProjectEnableEffect, value);
}

void
ApplicationMenuBuilder::handleToggleProjectGroundShadowEnabledEvent(void *userData, bool value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeProjectEnableGroundShadow, value);
}

void
ApplicationMenuBuilder::handleSetPhysicsSimulationModeEvent(void *userData, nanoem_u32_t value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(
        kMenuItemTypeProjectPhysicsSimulationDisable, value == PhysicsEngine::kSimulationModeDisable);
    self->setMenuItemChecked(
        kMenuItemTypeProjectPhysicsSimulationEnableAnytime, value == PhysicsEngine::kSimulationModeEnableAnytime);
    self->setMenuItemChecked(
        kMenuItemTypeProjectPhysicsSimulationEnablePlaying, value == PhysicsEngine::kSimulationModeEnablePlaying);
    self->setMenuItemChecked(
        kMenuItemTypeProjectPhysicsSimulationEnableTracing, value == PhysicsEngine::kSimulationModeEnableTracing);
}

void
ApplicationMenuBuilder::handleSetPreferredMotionFPSEvent(void *userData, nanoem_u32_t value, bool unlimited)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeProjectPreferredMotionFPS30, value == 30 && !unlimited);
    self->setMenuItemChecked(kMenuItemTypeProjectPreferredMotionFPS60, value == 60 && !unlimited);
    self->setMenuItemChecked(kMenuItemTypeProjectPreferredMotionFPSUnlimited, unlimited);
}

void
ApplicationMenuBuilder::handleSetProjectSampleLevelEvent(void *userData, nanoem_u32_t value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeProjectDisableMSAA, value == 0);
    self->setMenuItemChecked(kMenuItemTypeProjectEnableMSAAx2, value == 1);
    self->setMenuItemChecked(kMenuItemTypeProjectEnableMSAAx4, value == 2);
    self->setMenuItemChecked(kMenuItemTypeProjectEnableMSAAx8, value == 3);
    self->setMenuItemChecked(kMenuItemTypeProjectEnableMSAAx16, value == 4);
}

void
ApplicationMenuBuilder::handleSetShadowMapModeEvent(void *userData, nanoem_u32_t value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeLightSelfShadowDisable, value == 0);
    self->setMenuItemChecked(kMenuItemTypeLightSelfShadowEnableWithMode1, value == 1);
    self->setMenuItemChecked(kMenuItemTypeLightSelfShadowEnableWithMode2, value == 2);
}

void
ApplicationMenuBuilder::handleAddModalDialogEvent(void *userData)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->startModalDialog();
}

void
ApplicationMenuBuilder::handleClearModalDialogEvent(void *userData)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->stopModalDialog();
}

void
ApplicationMenuBuilder::handleSetLanguageEvent(void *userData, nanoem_u32_t /* value */)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    tinystl::vector<tinystl::pair<bool, bool>, TinySTLAllocator> states;
    for (int i = kMenuItemTypeFirstEnum; i < kMenuItemTypeMaxEnum; i++) {
        MenuItemType type = static_cast<MenuItemType>(i);
        states.push_back(tinystl::make_pair(self->isMenuItemEnabled(type), self->isMenuItemChecked(type)));
    }
    self->createAllMenus();
    self->m_client->sendGetAllAccessoriesRequestMessage(rebuildAllAccessoryNames, self);
    self->m_client->sendGetAllModelsRequestMessage(rebuildAllModelNames, self);
    self->m_client->sendLoadAllModelPluginsMessage(URIList());
    self->m_client->sendLoadAllMotionPluginsMessage(URIList());
    MenuBarHandle msaaMenu = self->m_msaaMenu;
    self->appendMenuItem(msaaMenu, kMenuItemTypeProjectEnableMSAAx16);
    self->appendMenuItem(msaaMenu, kMenuItemTypeProjectEnableMSAAx8);
    self->appendMenuItem(msaaMenu, kMenuItemTypeProjectEnableMSAAx4);
    self->appendMenuItem(msaaMenu, kMenuItemTypeProjectEnableMSAAx2);
    self->appendMenuItem(msaaMenu, kMenuItemTypeProjectDisableMSAA);
    for (int i = kMenuItemTypeFirstEnum; i < kMenuItemTypeMaxEnum; i++) {
        MenuItemType type = static_cast<MenuItemType>(i);
        self->setMenuItemEnable(type, states[i].first);
        self->setMenuItemChecked(type, states[i].second);
    }
}

void
ApplicationMenuBuilder::handleToggleProjectPlayingWithLoopEvent(void *userData, bool value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeProjectEnableLoop, value);
}

void
ApplicationMenuBuilder::handleToggleActiveAccessoryAddBlendEnabledEvent(void *userData, bool value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeAccessoryEnableAddBlend, value);
}

void
ApplicationMenuBuilder::handleToggleActiveAccessoryShadowEnabledEvent(void *userData, bool value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeAccessoryEnableShadow, value);
}

void
ApplicationMenuBuilder::handleToggleActiveAccessoryVisibleEvent(void *userData, bool value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeAccessoryEnableVisible, value);
}

void
ApplicationMenuBuilder::handleGetAllModelBonesRequest(
    void *userData, nanoem_u16_t /* handle */, const StringList &values)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->clearAllMenuItems(self->m_selectBoneMenu);
    for (StringList::const_iterator it = values.begin(), end = values.end(); it != end; ++it) {
        self->createSelectBoneMenuItem(self->m_selectBoneMenu, it->c_str());
    }
}

void
ApplicationMenuBuilder::handleGetAllModelMorphsRequest(void *userData, nanoem_u16_t /* handle */,
    const StringList &eyes, const StringList &eyebrows, const StringList &lips, const StringList &others)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->addMorphMenuItems(eyes, NANOEM_MODEL_MORPH_CATEGORY_EYE);
    self->addMorphMenuItems(eyebrows, NANOEM_MODEL_MORPH_CATEGORY_EYEBROW);
    self->addMorphMenuItems(lips, NANOEM_MODEL_MORPH_CATEGORY_LIP);
    self->addMorphMenuItems(others, NANOEM_MODEL_MORPH_CATEGORY_OTHER);
}

void
ApplicationMenuBuilder::handleSetupProjectEvent(void *userData, const Vector2 & /* windowSize */,
    nanoem_f32_t windowDevicePixelRatio, nanoem_f32_t viewportDevicePixelRatio)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->createAllMenus();
    MenuBarHandle msaaMenu = self->m_msaaMenu;
    self->appendMenuItem(msaaMenu, kMenuItemTypeProjectEnableMSAAx16);
    self->appendMenuItem(msaaMenu, kMenuItemTypeProjectEnableMSAAx8);
    self->appendMenuItem(msaaMenu, kMenuItemTypeProjectEnableMSAAx4);
    self->appendMenuItem(msaaMenu, kMenuItemTypeProjectEnableMSAAx2);
    self->appendMenuItem(msaaMenu, kMenuItemTypeProjectDisableMSAA);
    self->setMenuItemEnable(kMenuItemTypeProjectEnableHighResolutionViewport, windowDevicePixelRatio > 1.0f);
    self->setMenuItemChecked(kMenuItemTypeProjectDisableMSAA, true);
    self->setMenuItemChecked(kMenuItemTypeProjectEnableHighResolutionViewport,
        windowDevicePixelRatio > 1.0f && windowDevicePixelRatio == viewportDevicePixelRatio);
}

void
ApplicationMenuBuilder::handleSetEditingModeEvent(void *userData, nanoem_u32_t value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeModelEditModeSelect, value == Project::kEditingModeSelect);
    self->setMenuItemChecked(kMenuItemTypeModelEditModeMove, value == Project::kEditingModeMove);
    self->setMenuItemChecked(kMenuItemTypeModelEditModeRotate, value == Project::kEditingModeRotate);
}

void
ApplicationMenuBuilder::handleShadowMapModeEvent(void *userData, nanoem_u32_t value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeLightSelfShadowDisable, value == ShadowCamera::kCoverageModeTypeNone);
    self->setMenuItemChecked(kMenuItemTypeLightSelfShadowEnableWithMode1, value == ShadowCamera::kCoverageModeType1);
    self->setMenuItemChecked(kMenuItemTypeLightSelfShadowEnableWithMode2, value == ShadowCamera::kCoverageModeType2);
}

void
ApplicationMenuBuilder::rebuildAllAccessoryNames(
    void *userData, const ThreadedApplicationClient::DrawableItemList &items)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->clearAllMenuItems(self->m_selectAccessoryMenu);
    for (ThreadedApplicationClient::DrawableItemList::const_iterator it = items.begin(), end = items.end(); it != end;
         ++it) {
        self->createSelectAccessoryMenuItem(self->m_selectAccessoryMenu, it->m_handle, it->m_name.c_str());
    }
}

void
ApplicationMenuBuilder::rebuildAllModelNames(void *userData, const ThreadedApplicationClient::DrawableItemList &items)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->clearAllMenuItems(self->m_selectModelMenu);
    for (ThreadedApplicationClient::DrawableItemList::const_iterator it = items.begin(), end = items.end(); it != end;
         ++it) {
        self->createSelectModelMenuItem(self->m_selectModelMenu, it->m_handle, it->m_name.c_str());
        if (it->m_active) {
            self->updateActiveModelMenu(it->m_handle);
        }
    }
}

void
ApplicationMenuBuilder::handleCompleteLoadingAllModelPluginsEvent(
    void *userData, const ThreadedApplicationClient::PluginItemList &items)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->clearAllMenuItems(self->m_editModelPluginMenu);
    nanoem_u16_t i = 0;
    for (ThreadedApplicationClient::PluginItemList::const_iterator it = items.begin(), end = items.end(); it != end;
         ++it) {
        const ThreadedApplicationClient::PluginItem &item = *it;
        self->createPluginMenuItem(
            self->m_editModelPluginMenu, kMenuItemTypeModelPluginExecute, i++, item.m_name, item.m_functions);
    }
}

void
ApplicationMenuBuilder::handleCompleteLoadingAllMotionPluginsEvent(
    void *userData, const ThreadedApplicationClient::PluginItemList &items)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->clearAllMenuItems(self->m_editMotionPluginMenu);
    nanoem_u16_t i = 0;
    for (ThreadedApplicationClient::PluginItemList::const_iterator it = items.begin(), end = items.end(); it != end;
         ++it) {
        const ThreadedApplicationClient::PluginItem &item = *it;
        self->createPluginMenuItem(
            self->m_editMotionPluginMenu, kMenuItemTypeMotionPluginExecute, i++, item.m_name, item.m_functions);
    }
}

void
ApplicationMenuBuilder::handleCanCopyEvent(void *userData, bool value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemEnable(kMenuItemTypeEditCopy, value);
}

void
ApplicationMenuBuilder::handleCanPasteEvent(void *userData, bool value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemEnable(kMenuItemTypeEditPaste, value);
    self->setMenuItemEnable(kMenuItemTypeEditCut, value);
}

void
ApplicationMenuBuilder::handleSetWindowPixelRatioEvent(void *userData, nanoem_f32_t value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemEnable(kMenuItemTypeProjectEnableHighResolutionViewport, value > 1.0f);
}

void
ApplicationMenuBuilder::handleSetViewportPixelRatioEvent(void *userData, nanoem_f32_t value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    self->setMenuItemChecked(kMenuItemTypeProjectEnableHighResolutionViewport, value > 1.0f);
}

void
ApplicationMenuBuilder::handleToggleModelEditingEnabledEvent(void *userData, bool value)
{
    ApplicationMenuBuilder *self = static_cast<ApplicationMenuBuilder *>(userData);
    const bool invert = value ? false : true;
    self->setMenuItemEnable(kMenuItemTypeProjectPlay, invert);
    self->setMenuItemEnable(kMenuItemTypeFileImportCameraMotion, invert);
    self->setMenuItemEnable(kMenuItemTypeFileImportLightMotion, invert);
    self->setMenuItemEnable(kMenuItemTypeFileImportModelMotion, invert);
    self->setMenuItemEnable(kMenuItemTypeModelRegisterKeyframe, invert);
    self->setMenuItemEnable(kMenuItemTypeMotionInsertEmptyTimelineFrame, invert);
    self->setMenuItemEnable(kMenuItemTypeMotionRemoveTimelineFrame, invert);
    self->setMenuItemEnable(kMenuItemTypeMotionReset, invert);
    self->setMenuItemEnable(kMenuItemTypeEditSelectAll, invert);
    self->setMenuItemEnable(kMenuItemTypeEditMorphRegisterAllMorphs, invert);
    self->setMenuItemEnable(kMenuItemTypeEditMorphRemoveAllEyeKeyframes, invert);
    self->setMenuItemEnable(kMenuItemTypeEditMorphRemoveAllEyebrowKeyframes, invert);
    self->setMenuItemEnable(kMenuItemTypeEditMorphRemoveAllLipKeyframes, invert);
    self->setMenuItemEnable(kMenuItemTypeEditMotionPluginTitle, invert);
    self->setMenuItemEnable(kMenuItemTypeModelRemoveAllSelectedKeyframes, invert);
    self->setMenuItemEnable(kMenuItemTypeCameraRegisterKeyframe, invert);
    self->setMenuItemEnable(kMenuItemTypeCameraRemoveAllSelectedKeyframes, invert);
    self->setMenuItemEnable(kMenuItemTypeLightRegisterKeyframe, invert);
    self->setMenuItemEnable(kMenuItemTypeLightRemoveAllSelectedKeyframes, invert);
}

void
ApplicationMenuBuilder::addMorphMenuItems(const StringList &morphs, nanoem_model_morph_category_t category)
{
    const bool enabled = !morphs.empty();
    MenuBarHandle handle = m_selectMorphMenu[category];
    clearAllMenuItems(handle);
    switch (category) {
    case NANOEM_MODEL_MORPH_CATEGORY_EYE: {
        setMenuItemEnable(kMenuItemTypeModelSelectMorphEyeTitle, enabled);
        break;
    }
    case NANOEM_MODEL_MORPH_CATEGORY_LIP: {
        setMenuItemEnable(kMenuItemTypeModelSelectMorphLipTitle, enabled);
        break;
    }
    case NANOEM_MODEL_MORPH_CATEGORY_EYEBROW: {
        setMenuItemEnable(kMenuItemTypeModelSelectMorphEyebrowTitle, enabled);
        break;
    }
    case NANOEM_MODEL_MORPH_CATEGORY_OTHER: {
        setMenuItemEnable(kMenuItemTypeModelSelectMorphOtherTitle, enabled);
        break;
    }
    default:
        break;
    }
    if (enabled) {
        for (StringList::const_iterator it = morphs.begin(), end = morphs.end(); it != end; ++it) {
            createSelectMorphMenuItem(handle, category, it->c_str());
        }
    }
}

void
ApplicationMenuBuilder::togglAllMutatingActionMenuItems(bool value)
{
    bool disablePlaying = value ? false : true;
    setMenuItemEnable(kMenuItemTypeMotionInsertEmptyTimelineFrame, disablePlaying);
    setMenuItemEnable(kMenuItemTypeMotionRemoveTimelineFrame, disablePlaying);
    setMenuItemEnable(kMenuItemTypeMotionReset, disablePlaying);
    setMenuItemEnable(kMenuItemTypeEditSelectAll, disablePlaying);
    setMenuItemEnable(kMenuItemTypeEditBoneResetAngle, disablePlaying);
    setMenuItemEnable(kMenuItemTypeEditCameraResetAngle, disablePlaying);
    setMenuItemEnable(kMenuItemTypeCameraReset, disablePlaying);
    setMenuItemEnable(kMenuItemTypeCameraRegisterKeyframe, disablePlaying);
    setMenuItemEnable(kMenuItemTypeCameraRemoveAllSelectedKeyframes, disablePlaying);
    setMenuItemEnable(kMenuItemTypeLightReset, disablePlaying);
    setMenuItemEnable(kMenuItemTypeLightRegisterKeyframe, disablePlaying);
    setMenuItemEnable(kMenuItemTypeLightRemoveAllSelectedKeyframes, disablePlaying);
    setMenuItemEnable(kMenuItemTypeLightSelfShadowReset, disablePlaying);
    setMenuItemEnable(kMenuItemTypeLightSelfShadowRegisterKeyframe, disablePlaying);
    setMenuItemEnable(kMenuItemTypeLightSelfShadowRemoveAllSelectedKeyframes, disablePlaying);
    for (nanoem_u32_t i = kMenuItemTypeFileFirstEnum; i < kMenuItemTypeFileExit; i++) {
        setMenuItemEnable(static_cast<MenuItemType>(i), disablePlaying);
    }
}

} /* namespace nanoem */
