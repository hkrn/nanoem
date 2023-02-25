/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/BaseShiftingAllKeyframesCommand.h"

#include "emapp/Accessory.h"
#include "emapp/EnumUtils.h"

#include "../CommandMessage.inl"
#include "bx/handlealloc.h"

namespace nanoem {
namespace command {

BaseShiftingAllKeyframesCommand::~BaseShiftingAllKeyframesCommand() NANOEM_DECL_NOEXCEPT
{
    for (MutableAccessoryKeyframeList::const_iterator it = m_accessoryKeyframes.begin(),
                                                      end = m_accessoryKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionAccessoryKeyframeDestroy(*it);
    }
    m_accessoryKeyframes.clear();
    for (MutableModelBoneKeyframeList::const_iterator it = m_boneKeyframes.begin(), end = m_boneKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionBoneKeyframeDestroy(*it);
    }
    m_boneKeyframes.clear();
    for (MutableCameraKeyframeList::const_iterator it = m_cameraKeyframes.begin(), end = m_cameraKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionCameraKeyframeDestroy(*it);
    }
    m_cameraKeyframes.clear();
    for (MutableLightKeyframeList::const_iterator it = m_lightKeyframes.begin(), end = m_lightKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionLightKeyframeDestroy(*it);
    }
    m_lightKeyframes.clear();
    for (MutableModelKeyframeList::const_iterator it = m_modelKeyframes.begin(), end = m_modelKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionModelKeyframeDestroy(*it);
    }
    m_modelKeyframes.clear();
    for (MutableModelMorphKeyframeList::const_iterator it = m_morphKeyframes.begin(), end = m_morphKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionMorphKeyframeDestroy(*it);
    }
    m_morphKeyframes.clear();
    for (MutableSelfShadowKeyframeList::const_iterator it = m_selfShadowKeyframes.begin(),
                                                       end = m_selfShadowKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionSelfShadowKeyframeDestroy(*it);
    }
    m_selfShadowKeyframes.clear();
    m_motion = 0;
    m_localFrameIndex = 0;
    m_types = 0;
}

Motion *
BaseShiftingAllKeyframesCommand::resolveMotion(nanoem_u32_t handle)
{
    Motion *motion = 0;
    Project *project = currentProject();
    if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA, m_types)) {
        motion = project->cameraMotion();
    }
    else if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT, m_types)) {
        motion = project->lightMotion();
    }
    else if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW, m_types)) {
        motion = project->selfShadowMotion();
    }
    else if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY, m_types)) {
        motion = project->resolveMotion(project->resolveRedoAccessory(handle));
    }
    else if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE, m_types) ||
        EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL, m_types) ||
        EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH, m_types)) {
        motion = project->resolveMotion(project->resolveRedoModel(handle));
    }
    return motion;
}

void
BaseShiftingAllKeyframesCommand::shiftAllKeyframesBackward(Error &error)
{
    if (m_localFrameIndex > 0 && currentProject()->containsMotion(m_motion)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(m_motion->data(), &status);
        if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY, m_types)) {
            shiftAllAccessoryKeyframesBackward(mutableMotion, &status);
        }
        if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE, m_types)) {
            shiftAllBoneKeyframesBackward(mutableMotion, &status);
        }
        if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA, m_types)) {
            shiftAllCameraKeyframesBackward(mutableMotion, &status);
        }
        if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT, m_types)) {
            shiftAllLightKeyframesBackward(mutableMotion, &status);
        }
        if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL, m_types)) {
            shiftAllModelKeyframesBackward(mutableMotion, &status);
        }
        if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH, m_types)) {
            shiftAllMorphKeyframesBackward(mutableMotion, &status);
        }
        if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW, m_types)) {
            shiftAllSelfShadowKeyframesBackward(mutableMotion, &status);
        }
        nanoemMutableMotionSortAllKeyframes(mutableMotion);
        nanoemMutableMotionDestroy(mutableMotion);
        m_motion->setDirty(true);
        assignError(status, error);
    }
}

void
BaseShiftingAllKeyframesCommand::shiftAllKeyframesForward(Error &error)
{
    if (currentProject()->containsMotion(m_motion)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(m_motion->data(), &status);
        if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY, m_types)) {
            shiftAllAccessoryKeyframesForward(mutableMotion, &status);
        }
        if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE, m_types)) {
            shiftAllBoneKeyframesForward(mutableMotion, &status);
        }
        if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA, m_types)) {
            shiftAllCameraKeyframesForward(mutableMotion, &status);
        }
        if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT, m_types)) {
            shiftAllLightKeyframesForward(mutableMotion, &status);
        }
        if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL, m_types)) {
            shiftAllModelKeyframesForward(mutableMotion, &status);
        }
        if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH, m_types)) {
            shiftAllMorphKeyframesForward(mutableMotion, &status);
        }
        if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW, m_types)) {
            shiftAllSelfShadowKeyframesForward(mutableMotion, &status);
        }
        nanoemMutableMotionSortAllKeyframes(mutableMotion);
        nanoemMutableMotionDestroy(mutableMotion);
        m_motion->setDirty(true);
        assignError(status, error);
    }
}

nanoem_u16_t
BaseShiftingAllKeyframesCommand::resolveHandle() const
{
    nanoem_u16_t handle = bx::kInvalidHandle;
    if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY, m_types) ||
        EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE, m_types) ||
        EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL, m_types) ||
        EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH, m_types)) {
        if (const IDrawable *drawable = currentProject()->resolveDrawable(m_motion)) {
            handle = drawable->handle();
        }
    }
    return handle;
}

BaseShiftingAllKeyframesCommand::BaseShiftingAllKeyframesCommand(Project *project)
    : BaseUndoCommand(project)
    , m_motion(0)
    , m_localFrameIndex(0)
    , m_types(0)
{
}

BaseShiftingAllKeyframesCommand::BaseShiftingAllKeyframesCommand(
    Motion *motion, nanoem_frame_index_t frameIndex, nanoem_u32_t types)
    : BaseUndoCommand(motion->project())
    , m_motion(motion)
    , m_localFrameIndex(frameIndex)
    , m_types(types)
{
}

void
BaseShiftingAllKeyframesCommand::shiftAllAccessoryKeyframesBackward(
    nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_accessory_keyframe_t *const *keyframes =
        nanoemMotionGetAllAccessoryKeyframeObjects(m_motion->data(), &numKeyframes);
    MutableAccessoryKeyframeList targets;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_accessory_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionAccessoryKeyframeGetKeyframeObject(keyframe));
        if (frameIndex >= m_localFrameIndex) {
            targets.push_back(nanoemMutableMotionAccessoryKeyframeCreateAsReference(keyframe, status));
        }
    }
    shiftAllAccessoryKeyframesWithOffset(mutableMotion, targets, 1, status);
    for (MutableAccessoryKeyframeList::const_iterator it = m_accessoryKeyframes.begin(),
                                                      end = m_accessoryKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionAddAccessoryKeyframe(mutableMotion, *it, m_localFrameIndex, status);
        nanoemMutableMotionAccessoryKeyframeDestroy(*it);
    }
    m_accessoryKeyframes.clear();
}

void
BaseShiftingAllKeyframesCommand::shiftAllBoneKeyframesBackward(
    nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_bone_keyframe_t *const *keyframes =
        nanoemMotionGetAllBoneKeyframeObjects(m_motion->data(), &numKeyframes);
    MutableModelBoneKeyframeList targets;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_bone_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(keyframe));
        if (frameIndex >= m_localFrameIndex) {
            targets.push_back(nanoemMutableMotionBoneKeyframeCreateAsReference(keyframe, status));
        }
    }
    shiftAllBoneKeyframesWithOffset(mutableMotion, targets, 1, status);
    for (MutableModelBoneKeyframeList::const_iterator it = m_boneKeyframes.begin(), end = m_boneKeyframes.end();
         it != end; ++it) {
        nanoem_mutable_motion_bone_keyframe_t *keyframe = *it;
        const nanoem_unicode_string_t *name =
            nanoemMotionBoneKeyframeGetName(nanoemMutableMotionBoneKeyframeGetOriginObject(keyframe));
        nanoemMutableMotionAddBoneKeyframe(mutableMotion, keyframe, name, m_localFrameIndex, status);
        nanoemMutableMotionBoneKeyframeDestroy(keyframe);
    }
    m_boneKeyframes.clear();
}

void
BaseShiftingAllKeyframesCommand::shiftAllCameraKeyframesBackward(
    nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_camera_keyframe_t *const *keyframes =
        nanoemMotionGetAllCameraKeyframeObjects(m_motion->data(), &numKeyframes);
    MutableCameraKeyframeList targets;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_camera_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(keyframe));
        if (frameIndex >= m_localFrameIndex) {
            targets.push_back(nanoemMutableMotionCameraKeyframeCreateAsReference(keyframe, status));
        }
    }
    shiftAllCameraKeyframesWithOffset(mutableMotion, targets, 1, status);
    for (MutableCameraKeyframeList::const_iterator it = m_cameraKeyframes.begin(), end = m_cameraKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionAddCameraKeyframe(mutableMotion, *it, m_localFrameIndex, status);
        nanoemMutableMotionCameraKeyframeDestroy(*it);
    }
    m_cameraKeyframes.clear();
}

void
BaseShiftingAllKeyframesCommand::shiftAllLightKeyframesBackward(
    nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_light_keyframe_t *const *keyframes =
        nanoemMotionGetAllLightKeyframeObjects(m_motion->data(), &numKeyframes);
    MutableLightKeyframeList targets;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_light_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionLightKeyframeGetKeyframeObject(keyframe));
        if (frameIndex >= m_localFrameIndex) {
            targets.push_back(nanoemMutableMotionLightKeyframeCreateAsReference(keyframe, status));
        }
    }
    shiftAllLightKeyframesWithOffset(mutableMotion, targets, 1, status);
    for (MutableLightKeyframeList::const_iterator it = m_lightKeyframes.begin(), end = m_lightKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionAddLightKeyframe(mutableMotion, *it, m_localFrameIndex, status);
        nanoemMutableMotionLightKeyframeDestroy(*it);
    }
    m_lightKeyframes.clear();
}

void
BaseShiftingAllKeyframesCommand::shiftAllModelKeyframesBackward(
    nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_model_keyframe_t *const *keyframes =
        nanoemMotionGetAllModelKeyframeObjects(m_motion->data(), &numKeyframes);
    MutableModelKeyframeList targets;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_model_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionModelKeyframeGetKeyframeObject(keyframe));
        if (frameIndex >= m_localFrameIndex) {
            targets.push_back(nanoemMutableMotionModelKeyframeCreateAsReference(keyframe, status));
        }
    }
    shiftAllModelKeyframesWithOffset(mutableMotion, targets, 1, status);
    for (MutableModelKeyframeList::const_iterator it = m_modelKeyframes.begin(), end = m_modelKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionAddModelKeyframe(mutableMotion, *it, m_localFrameIndex, status);
        nanoemMutableMotionModelKeyframeDestroy(*it);
    }
    m_modelKeyframes.clear();
}

void
BaseShiftingAllKeyframesCommand::shiftAllMorphKeyframesBackward(
    nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_morph_keyframe_t *const *keyframes =
        nanoemMotionGetAllMorphKeyframeObjects(m_motion->data(), &numKeyframes);
    MutableModelMorphKeyframeList targets;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_morph_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(keyframe));
        if (frameIndex >= m_localFrameIndex) {
            targets.push_back(nanoemMutableMotionMorphKeyframeCreateAsReference(keyframe, status));
        }
    }
    shiftAllMorphKeyframesWithOffset(mutableMotion, targets, 1, status);
    for (MutableModelMorphKeyframeList::const_iterator it = m_morphKeyframes.begin(), end = m_morphKeyframes.end();
         it != end; ++it) {
        nanoem_mutable_motion_morph_keyframe_t *keyframe = *it;
        const nanoem_unicode_string_t *name =
            nanoemMotionMorphKeyframeGetName(nanoemMutableMotionMorphKeyframeGetOriginObject(keyframe));
        nanoemMutableMotionAddMorphKeyframe(mutableMotion, keyframe, name, m_localFrameIndex, status);
        nanoemMutableMotionMorphKeyframeDestroy(keyframe);
    }
    m_morphKeyframes.clear();
}

void
BaseShiftingAllKeyframesCommand::shiftAllSelfShadowKeyframesBackward(
    nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_self_shadow_keyframe_t *const *keyframes =
        nanoemMotionGetAllSelfShadowKeyframeObjects(m_motion->data(), &numKeyframes);
    MutableSelfShadowKeyframeList targets;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_self_shadow_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionSelfShadowKeyframeGetKeyframeObject(keyframe));
        if (frameIndex >= m_localFrameIndex) {
            targets.push_back(nanoemMutableMotionSelfShadowKeyframeCreateAsReference(keyframe, status));
        }
    }
    shiftAllSelfShadowKeyframesWithOffset(mutableMotion, targets, 1, status);
    for (MutableSelfShadowKeyframeList::const_iterator it = m_selfShadowKeyframes.begin(),
                                                       end = m_selfShadowKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionAddSelfShadowKeyframe(mutableMotion, *it, m_localFrameIndex, status);
        nanoemMutableMotionSelfShadowKeyframeDestroy(*it);
    }
    m_selfShadowKeyframes.clear();
}

void
BaseShiftingAllKeyframesCommand::shiftAllAccessoryKeyframesForward(
    nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_accessory_keyframe_t *const *keyframes =
        nanoemMotionGetAllAccessoryKeyframeObjects(m_motion->data(), &numKeyframes);
    MutableAccessoryKeyframeList targets;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_accessory_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionAccessoryKeyframeGetKeyframeObject(keyframe));
        if (frameIndex > m_localFrameIndex) {
            targets.push_back(nanoemMutableMotionAccessoryKeyframeCreateAsReference(keyframe, status));
        }
        else if (frameIndex == m_localFrameIndex) {
            m_accessoryKeyframes.push_back(nanoemMutableMotionAccessoryKeyframeCreateAsReference(keyframe, status));
        }
    }
    for (MutableAccessoryKeyframeList::const_iterator it = m_accessoryKeyframes.begin(),
                                                      end = m_accessoryKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionRemoveAccessoryKeyframe(mutableMotion, *it, status);
    }
    shiftAllAccessoryKeyframesWithOffset(mutableMotion, targets, -1, status);
}

void
BaseShiftingAllKeyframesCommand::shiftAllBoneKeyframesForward(
    nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_bone_keyframe_t *const *keyframes =
        nanoemMotionGetAllBoneKeyframeObjects(m_motion->data(), &numKeyframes);
    MutableModelBoneKeyframeList targets;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_bone_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(keyframe));
        if (frameIndex > m_localFrameIndex) {
            targets.push_back(nanoemMutableMotionBoneKeyframeCreateAsReference(keyframe, status));
        }
        else if (frameIndex == m_localFrameIndex) {
            m_boneKeyframes.push_back(nanoemMutableMotionBoneKeyframeCreateAsReference(keyframe, status));
        }
    }
    for (MutableModelBoneKeyframeList::const_iterator it = m_boneKeyframes.begin(), end = m_boneKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionRemoveBoneKeyframe(mutableMotion, *it, status);
    }
    shiftAllBoneKeyframesWithOffset(mutableMotion, targets, -1, status);
}

void
BaseShiftingAllKeyframesCommand::shiftAllCameraKeyframesForward(
    nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_camera_keyframe_t *const *keyframes =
        nanoemMotionGetAllCameraKeyframeObjects(m_motion->data(), &numKeyframes);
    MutableCameraKeyframeList targets;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_camera_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(keyframe));
        if (frameIndex > m_localFrameIndex) {
            targets.push_back(nanoemMutableMotionCameraKeyframeCreateAsReference(keyframe, status));
        }
        else if (frameIndex == m_localFrameIndex) {
            m_cameraKeyframes.push_back(nanoemMutableMotionCameraKeyframeCreateAsReference(keyframe, status));
        }
    }
    for (MutableCameraKeyframeList::const_iterator it = m_cameraKeyframes.begin(), end = m_cameraKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionRemoveCameraKeyframe(mutableMotion, *it, status);
    }
    shiftAllCameraKeyframesWithOffset(mutableMotion, targets, -1, status);
}

void
BaseShiftingAllKeyframesCommand::shiftAllLightKeyframesForward(
    nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_light_keyframe_t *const *keyframes =
        nanoemMotionGetAllLightKeyframeObjects(m_motion->data(), &numKeyframes);
    MutableLightKeyframeList targets;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_light_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionLightKeyframeGetKeyframeObject(keyframe));
        if (frameIndex > m_localFrameIndex) {
            targets.push_back(nanoemMutableMotionLightKeyframeCreateAsReference(keyframe, status));
        }
        else if (frameIndex == m_localFrameIndex) {
            m_lightKeyframes.push_back(nanoemMutableMotionLightKeyframeCreateAsReference(keyframe, status));
        }
    }
    for (MutableLightKeyframeList::const_iterator it = m_lightKeyframes.begin(), end = m_lightKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionRemoveLightKeyframe(mutableMotion, *it, status);
    }
    shiftAllLightKeyframesWithOffset(mutableMotion, targets, -1, status);
}

void
BaseShiftingAllKeyframesCommand::shiftAllModelKeyframesForward(
    nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_model_keyframe_t *const *keyframes =
        nanoemMotionGetAllModelKeyframeObjects(m_motion->data(), &numKeyframes);
    MutableModelKeyframeList targets;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_model_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionModelKeyframeGetKeyframeObject(keyframe));
        if (frameIndex > m_localFrameIndex) {
            targets.push_back(nanoemMutableMotionModelKeyframeCreateAsReference(keyframe, status));
        }
        else if (frameIndex == m_localFrameIndex) {
            m_modelKeyframes.push_back(nanoemMutableMotionModelKeyframeCreateAsReference(keyframe, status));
        }
    }
    for (MutableModelKeyframeList::const_iterator it = m_modelKeyframes.begin(), end = m_modelKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionRemoveModelKeyframe(mutableMotion, *it, status);
    }
    shiftAllModelKeyframesWithOffset(mutableMotion, targets, -1, status);
}

void
BaseShiftingAllKeyframesCommand::shiftAllMorphKeyframesForward(
    nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_morph_keyframe_t *const *keyframes =
        nanoemMotionGetAllMorphKeyframeObjects(m_motion->data(), &numKeyframes);
    MutableModelMorphKeyframeList targets;
    const bool isEmpty = m_morphKeyframes.empty();
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_morph_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(keyframe));
        if (frameIndex > m_localFrameIndex) {
            targets.push_back(nanoemMutableMotionMorphKeyframeCreateAsReference(keyframe, status));
        }
        else if (frameIndex == m_localFrameIndex && isEmpty) {
            m_morphKeyframes.push_back(nanoemMutableMotionMorphKeyframeCreateAsReference(keyframe, status));
        }
    }
    for (MutableModelMorphKeyframeList::const_iterator it = m_morphKeyframes.begin(), end = m_morphKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionRemoveMorphKeyframe(mutableMotion, *it, status);
    }
    shiftAllMorphKeyframesWithOffset(mutableMotion, targets, -1, status);
}

void
BaseShiftingAllKeyframesCommand::shiftAllSelfShadowKeyframesForward(
    nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_self_shadow_keyframe_t *const *keyframes =
        nanoemMotionGetAllSelfShadowKeyframeObjects(m_motion->data(), &numKeyframes);
    MutableSelfShadowKeyframeList targets;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_self_shadow_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionSelfShadowKeyframeGetKeyframeObject(keyframe));
        if (frameIndex > m_localFrameIndex) {
            targets.push_back(nanoemMutableMotionSelfShadowKeyframeCreateAsReference(keyframe, status));
        }
        else if (frameIndex == m_localFrameIndex) {
            m_selfShadowKeyframes.push_back(nanoemMutableMotionSelfShadowKeyframeCreateAsReference(keyframe, status));
        }
    }
    for (MutableSelfShadowKeyframeList::const_iterator it = m_selfShadowKeyframes.begin(),
                                                       end = m_selfShadowKeyframes.end();
         it != end; ++it) {
        nanoemMutableMotionRemoveSelfShadowKeyframe(mutableMotion, *it, status);
    }
    shiftAllSelfShadowKeyframesWithOffset(mutableMotion, targets, -1, status);
}

void
BaseShiftingAllKeyframesCommand::shiftAllAccessoryKeyframesWithOffset(nanoem_mutable_motion_t *mutableMotion,
    const MutableAccessoryKeyframeList &keyframes, int offset, nanoem_status_t *status)
{
    for (MutableAccessoryKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        nanoemMutableMotionRemoveAccessoryKeyframe(mutableMotion, *it, status);
    }
    for (MutableAccessoryKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        nanoem_mutable_motion_accessory_keyframe_t *mutableKeyframe = *it;
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionAccessoryKeyframeGetKeyframeObject(
                nanoemMutableMotionAccessoryKeyframeGetOriginObject(mutableKeyframe)));
        nanoemMutableMotionAddAccessoryKeyframe(mutableMotion, mutableKeyframe, frameIndex + offset, status);
        nanoemMutableMotionAccessoryKeyframeDestroy(mutableKeyframe);
    }
}

void
BaseShiftingAllKeyframesCommand::shiftAllBoneKeyframesWithOffset(nanoem_mutable_motion_t *mutableMotion,
    const MutableModelBoneKeyframeList &keyframes, int offset, nanoem_status_t *status)
{
    for (MutableModelBoneKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        nanoemMutableMotionRemoveBoneKeyframe(mutableMotion, *it, status);
    }
    for (MutableModelBoneKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        nanoem_mutable_motion_bone_keyframe_t *mutableKeyframe = *it;
        nanoem_motion_bone_keyframe_t *origin = nanoemMutableMotionBoneKeyframeGetOriginObject(mutableKeyframe);
        const nanoem_unicode_string_t *name = nanoemMotionBoneKeyframeGetName(origin);
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(origin));
        nanoemMutableMotionAddBoneKeyframe(mutableMotion, mutableKeyframe, name, frameIndex + offset, status);
        nanoemMutableMotionBoneKeyframeDestroy(mutableKeyframe);
    }
}

void
BaseShiftingAllKeyframesCommand::shiftAllCameraKeyframesWithOffset(nanoem_mutable_motion_t *mutableMotion,
    const MutableCameraKeyframeList &keyframes, int offset, nanoem_status_t *status)
{
    for (MutableCameraKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        nanoemMutableMotionRemoveCameraKeyframe(mutableMotion, *it, status);
    }
    for (MutableCameraKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        nanoem_mutable_motion_camera_keyframe_t *mutableKeyframe = *it;
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(
                nanoemMutableMotionCameraKeyframeGetOriginObject(mutableKeyframe)));
        nanoemMutableMotionAddCameraKeyframe(mutableMotion, mutableKeyframe, frameIndex + offset, status);
        nanoemMutableMotionCameraKeyframeDestroy(mutableKeyframe);
    }
}

void
BaseShiftingAllKeyframesCommand::shiftAllLightKeyframesWithOffset(nanoem_mutable_motion_t *mutableMotion,
    const MutableLightKeyframeList &keyframes, int offset, nanoem_status_t *status)
{
    for (MutableLightKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        nanoemMutableMotionRemoveLightKeyframe(mutableMotion, *it, status);
    }
    for (MutableLightKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        nanoem_mutable_motion_light_keyframe_t *mutableKeyframe = *it;
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionLightKeyframeGetKeyframeObject(
                nanoemMutableMotionLightKeyframeGetOriginObject(mutableKeyframe)));
        nanoemMutableMotionAddLightKeyframe(mutableMotion, mutableKeyframe, frameIndex + offset, status);
        nanoemMutableMotionLightKeyframeDestroy(mutableKeyframe);
    }
}

void
BaseShiftingAllKeyframesCommand::shiftAllModelKeyframesWithOffset(nanoem_mutable_motion_t *mutableMotion,
    const MutableModelKeyframeList &keyframes, int offset, nanoem_status_t *status)
{
    for (MutableModelKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        nanoemMutableMotionRemoveModelKeyframe(mutableMotion, *it, status);
    }
    for (MutableModelKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        nanoem_mutable_motion_model_keyframe_t *mutableKeyframe = *it;
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionModelKeyframeGetKeyframeObject(
                nanoemMutableMotionModelKeyframeGetOriginObject(mutableKeyframe)));
        nanoemMutableMotionAddModelKeyframe(mutableMotion, mutableKeyframe, frameIndex + offset, status);
        nanoemMutableMotionModelKeyframeDestroy(mutableKeyframe);
    }
}

void
BaseShiftingAllKeyframesCommand::shiftAllMorphKeyframesWithOffset(nanoem_mutable_motion_t *mutableMotion,
    const MutableModelMorphKeyframeList &keyframes, int offset, nanoem_status_t *status)
{
    for (MutableModelMorphKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        nanoemMutableMotionRemoveMorphKeyframe(mutableMotion, *it, status);
    }
    for (MutableModelMorphKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        nanoem_mutable_motion_morph_keyframe_t *mutableKeyframe = *it;
        nanoem_motion_morph_keyframe_t *origin = nanoemMutableMotionMorphKeyframeGetOriginObject(mutableKeyframe);
        const nanoem_unicode_string_t *name = nanoemMotionMorphKeyframeGetName(origin);
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(origin));
        nanoemMutableMotionAddMorphKeyframe(mutableMotion, mutableKeyframe, name, frameIndex + offset, status);
        nanoemMutableMotionMorphKeyframeDestroy(mutableKeyframe);
    }
}

void
BaseShiftingAllKeyframesCommand::shiftAllSelfShadowKeyframesWithOffset(nanoem_mutable_motion_t *mutableMotion,
    const MutableSelfShadowKeyframeList &keyframes, int offset, nanoem_status_t *status)
{
    for (MutableSelfShadowKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        nanoemMutableMotionRemoveSelfShadowKeyframe(mutableMotion, *it, status);
    }
    for (MutableSelfShadowKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        nanoem_mutable_motion_self_shadow_keyframe_t *mutableKeyframe = *it;
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionSelfShadowKeyframeGetKeyframeObject(
                nanoemMutableMotionSelfShadowKeyframeGetOriginObject(mutableKeyframe)));
        nanoemMutableMotionAddSelfShadowKeyframe(mutableMotion, mutableKeyframe, frameIndex + offset, status);
        nanoemMutableMotionSelfShadowKeyframeDestroy(mutableKeyframe);
    }
}

} /* namespace command */
} /* namespace nanoem */
