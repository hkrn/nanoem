/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/MotionKeyframeSelection.h"

#include "emapp/EnumUtils.h"
#include "emapp/ListUtils.h"

namespace nanoem {
namespace internal {

MotionKeyframeSelection::MotionKeyframeSelection(Motion *parent)
    : m_parent(parent)
{
    nanoem_parameter_assert(m_parent, "must NOT be NULL");
}

MotionKeyframeSelection::~MotionKeyframeSelection() NANOEM_DECL_NOEXCEPT
{
}

void
MotionKeyframeSelection::getAll(Motion::AccessoryKeyframeList &keyframes, int *startOffset) const
{
    keyframes = ListUtils::toListFromSet(m_accessoryKeyframeSelection);
    if (startOffset) {
        *startOffset = 0;
        if (!keyframes.empty()) {
            nanoem_frame_index_t offset = INT32_MAX;
            for (Motion::AccessoryKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end;
                 ++it) {
                nanoem_frame_index_t frameIndex =
                    nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionAccessoryKeyframeGetKeyframeObject(*it));
                offset = glm::min(offset, frameIndex);
            }
            *startOffset = offset;
        }
    }
}

bool
MotionKeyframeSelection::contains(const nanoem_motion_accessory_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT
{
    return m_accessoryKeyframeSelection.find(keyframe) != m_accessoryKeyframeSelection.end();
}

void
MotionKeyframeSelection::add(const nanoem_motion_accessory_keyframe_t *keyframe)
{
    if (keyframe && isParentOf(nanoemMotionAccessoryKeyframeGetKeyframeObject(keyframe))) {
        m_accessoryKeyframeSelection.insert(keyframe);
    }
}

void
MotionKeyframeSelection::remove(const nanoem_motion_accessory_keyframe_t *keyframe)
{
    m_accessoryKeyframeSelection.erase(keyframe);
}

bool
MotionKeyframeSelection::contains(const nanoem_motion_bone_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT
{
    return m_boneKeyframeSelection.find(keyframe) != m_boneKeyframeSelection.end();
}

void
MotionKeyframeSelection::getAll(Motion::BoneKeyframeList &keyframes, int *startOffset) const
{
    keyframes = ListUtils::toListFromSet(m_boneKeyframeSelection);
    if (startOffset) {
        *startOffset = 0;
        if (!keyframes.empty()) {
            nanoem_frame_index_t offset = INT32_MAX;
            for (Motion::BoneKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end;
                 ++it) {
                nanoem_frame_index_t frameIndex =
                    nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(*it));
                offset = glm::min(offset, frameIndex);
            }
            *startOffset = offset;
        }
    }
}

void
MotionKeyframeSelection::add(const nanoem_motion_bone_keyframe_t *keyframe)
{
    if (keyframe && isParentOf(nanoemMotionBoneKeyframeGetKeyframeObject(keyframe))) {
        m_boneKeyframeSelection.insert(keyframe);
    }
}

void
MotionKeyframeSelection::remove(const nanoem_motion_bone_keyframe_t *keyframe)
{
    m_boneKeyframeSelection.erase(keyframe);
}

bool
MotionKeyframeSelection::contains(const nanoem_motion_camera_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT
{
    return m_cameraKeyframeSelection.find(keyframe) != m_cameraKeyframeSelection.end();
}

void
MotionKeyframeSelection::getAll(Motion::CameraKeyframeList &keyframes, int *startOffset) const
{
    keyframes = ListUtils::toListFromSet(m_cameraKeyframeSelection);
    if (startOffset) {
        *startOffset = 0;
        if (!keyframes.empty()) {
            nanoem_frame_index_t offset = INT32_MAX;
            for (Motion::CameraKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end;
                 ++it) {
                nanoem_frame_index_t frameIndex =
                    nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(*it));
                offset = glm::min(offset, frameIndex);
            }
            *startOffset = offset;
        }
    }
}

void
MotionKeyframeSelection::add(const nanoem_motion_camera_keyframe_t *keyframe)
{
    if (keyframe && isParentOf(nanoemMotionCameraKeyframeGetKeyframeObject(keyframe))) {
        m_cameraKeyframeSelection.insert(keyframe);
    }
}

void
MotionKeyframeSelection::remove(const nanoem_motion_camera_keyframe_t *keyframe)
{
    m_cameraKeyframeSelection.erase(keyframe);
}

bool
MotionKeyframeSelection::contains(const nanoem_motion_light_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT
{
    return m_lightKeyframeSelection.find(keyframe) != m_lightKeyframeSelection.end();
}

void
MotionKeyframeSelection::getAll(Motion::LightKeyframeList &keyframes, int *startOffset) const
{
    keyframes = ListUtils::toListFromSet(m_lightKeyframeSelection);
    if (startOffset) {
        *startOffset = 0;
        if (!keyframes.empty()) {
            nanoem_frame_index_t offset = INT32_MAX;
            for (Motion::LightKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end;
                 ++it) {
                nanoem_frame_index_t frameIndex =
                    nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionLightKeyframeGetKeyframeObject(*it));
                offset = glm::min(offset, frameIndex);
            }
            *startOffset = offset;
        }
    }
}

void
MotionKeyframeSelection::add(const nanoem_motion_light_keyframe_t *keyframe)
{
    if (keyframe && isParentOf(nanoemMotionLightKeyframeGetKeyframeObject(keyframe))) {
        m_lightKeyframeSelection.insert(keyframe);
    }
}

void
MotionKeyframeSelection::remove(const nanoem_motion_light_keyframe_t *keyframe)
{
    m_lightKeyframeSelection.erase(keyframe);
}

bool
MotionKeyframeSelection::contains(const nanoem_motion_model_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT
{
    return m_modelKeyframeSelection.find(keyframe) != m_modelKeyframeSelection.end();
}

void
MotionKeyframeSelection::getAll(Motion::ModelKeyframeList &keyframes, int *startOffset) const
{
    keyframes = ListUtils::toListFromSet(m_modelKeyframeSelection);
    if (startOffset) {
        *startOffset = 0;
        if (!keyframes.empty()) {
            nanoem_frame_index_t offset = INT32_MAX;
            for (Motion::ModelKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end;
                 ++it) {
                nanoem_frame_index_t frameIndex =
                    nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionModelKeyframeGetKeyframeObject(*it));
                offset = glm::min(offset, frameIndex);
            }
            *startOffset = offset;
        }
    }
}

void
MotionKeyframeSelection::add(const nanoem_motion_model_keyframe_t *keyframe)
{
    if (keyframe && isParentOf(nanoemMotionModelKeyframeGetKeyframeObject(keyframe))) {
        m_modelKeyframeSelection.insert(keyframe);
    }
}

void
MotionKeyframeSelection::remove(const nanoem_motion_model_keyframe_t *keyframe)
{
    m_modelKeyframeSelection.erase(keyframe);
}

bool
MotionKeyframeSelection::contains(const nanoem_motion_morph_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT
{
    return m_morphKeyframeSelection.find(keyframe) != m_morphKeyframeSelection.end();
}

void
MotionKeyframeSelection::getAll(Motion::MorphKeyframeList &keyframes, int *startOffset) const
{
    keyframes = ListUtils::toListFromSet(m_morphKeyframeSelection);
    if (startOffset) {
        *startOffset = 0;
        if (!keyframes.empty()) {
            nanoem_frame_index_t offset = INT32_MAX;
            for (Motion::MorphKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end;
                 ++it) {
                nanoem_frame_index_t frameIndex =
                    nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(*it));
                offset = glm::min(offset, frameIndex);
            }
            *startOffset = offset;
        }
    }
}

void
MotionKeyframeSelection::add(const nanoem_motion_morph_keyframe_t *keyframe)
{
    if (keyframe && isParentOf(nanoemMotionMorphKeyframeGetKeyframeObject(keyframe))) {
        m_morphKeyframeSelection.insert(keyframe);
    }
}

void
MotionKeyframeSelection::remove(const nanoem_motion_morph_keyframe_t *keyframe)
{
    m_morphKeyframeSelection.erase(keyframe);
}

bool
MotionKeyframeSelection::contains(const nanoem_motion_self_shadow_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT
{
    return m_selfShadowKeyframeSelection.find(keyframe) != m_selfShadowKeyframeSelection.end();
}

void
MotionKeyframeSelection::getAll(Motion::SelfShadowKeyframeList &keyframes, int *startOffset) const
{
    keyframes = ListUtils::toListFromSet(m_selfShadowKeyframeSelection);
    if (startOffset) {
        *startOffset = 0;
        if (!keyframes.empty()) {
            nanoem_frame_index_t offset = INT32_MAX;
            for (Motion::SelfShadowKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end();
                 it != end; ++it) {
                nanoem_frame_index_t frameIndex =
                    nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionSelfShadowKeyframeGetKeyframeObject(*it));
                offset = glm::min(offset, frameIndex);
            }
            *startOffset = offset;
        }
    }
}

void
MotionKeyframeSelection::add(const nanoem_motion_self_shadow_keyframe_t *keyframe)
{
    if (keyframe && isParentOf(nanoemMotionSelfShadowKeyframeGetKeyframeObject(keyframe))) {
        m_selfShadowKeyframeSelection.insert(keyframe);
    }
}

void
MotionKeyframeSelection::remove(const nanoem_motion_self_shadow_keyframe_t *keyframe)
{
    m_selfShadowKeyframeSelection.erase(keyframe);
}

void
MotionKeyframeSelection::addAllKeyframes(nanoem_u32_t flags)
{
    const nanoem_motion_t *opaque = m_parent->data();
    nanoem_rsize_t numKeyframes;
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY)) {
        nanoem_motion_accessory_keyframe_t *const *accessoryKeyframes =
            nanoemMotionGetAllAccessoryKeyframeObjects(opaque, &numKeyframes);
        for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
            nanoem_motion_accessory_keyframe_t *keyframe = accessoryKeyframes[i];
            m_accessoryKeyframeSelection.insert(keyframe);
        }
    }
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE)) {
        nanoem_motion_bone_keyframe_t *const *boneKeyframes =
            nanoemMotionGetAllBoneKeyframeObjects(opaque, &numKeyframes);
        for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
            nanoem_motion_bone_keyframe_t *keyframe = boneKeyframes[i];
            m_boneKeyframeSelection.insert(keyframe);
        }
    }
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA)) {
        nanoem_motion_camera_keyframe_t *const *cameraKeyframes =
            nanoemMotionGetAllCameraKeyframeObjects(opaque, &numKeyframes);
        for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
            nanoem_motion_camera_keyframe_t *keyframe = cameraKeyframes[i];
            m_cameraKeyframeSelection.insert(keyframe);
        }
    }
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT)) {
        nanoem_motion_light_keyframe_t *const *lightKeyframes =
            nanoemMotionGetAllLightKeyframeObjects(opaque, &numKeyframes);
        for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
            nanoem_motion_light_keyframe_t *keyframe = lightKeyframes[i];
            m_lightKeyframeSelection.insert(keyframe);
        }
    }
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL)) {
        nanoem_motion_model_keyframe_t *const *modelKeyframes =
            nanoemMotionGetAllModelKeyframeObjects(opaque, &numKeyframes);
        for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
            nanoem_motion_model_keyframe_t *keyframe = modelKeyframes[i];
            m_modelKeyframeSelection.insert(keyframe);
        }
    }
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH)) {
        nanoem_motion_morph_keyframe_t *const *morphKeyframes =
            nanoemMotionGetAllMorphKeyframeObjects(opaque, &numKeyframes);
        for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
            nanoem_motion_morph_keyframe_t *keyframe = morphKeyframes[i];
            m_morphKeyframeSelection.insert(keyframe);
        }
    }
}

void
MotionKeyframeSelection::addAccessoryKeyframes(nanoem_frame_index_t start, nanoem_frame_index_t end)
{
    for (nanoem_frame_index_t i = start; i <= end; i++) {
        add(m_parent->findAccessoryKeyframe(i));
    }
}

void
MotionKeyframeSelection::addBoneKeyframes(
    const nanoem_model_bone_t *bone, nanoem_frame_index_t start, nanoem_frame_index_t end)
{
    const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
    for (nanoem_frame_index_t i = start; i <= end; i++) {
        add(m_parent->findBoneKeyframe(name, i));
    }
}

void
MotionKeyframeSelection::addCameraKeyframes(nanoem_frame_index_t start, nanoem_frame_index_t end)
{
    for (nanoem_frame_index_t i = start; i <= end; i++) {
        add(m_parent->findCameraKeyframe(i));
    }
}

void
MotionKeyframeSelection::addLightKeyframes(nanoem_frame_index_t start, nanoem_frame_index_t end)
{
    for (nanoem_frame_index_t i = start; i <= end; i++) {
        add(m_parent->findLightKeyframe(i));
    }
}

void
MotionKeyframeSelection::addModelKeyframes(nanoem_frame_index_t start, nanoem_frame_index_t end)
{
    for (nanoem_frame_index_t i = start; i <= end; i++) {
        add(m_parent->findModelKeyframe(i));
    }
}

void
MotionKeyframeSelection::addMorphKeyframes(
    const nanoem_model_morph_t *morph, nanoem_frame_index_t start, nanoem_frame_index_t end)
{
    const nanoem_unicode_string_t *name = nanoemModelMorphGetName(morph, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
    for (nanoem_frame_index_t i = start; i <= end; i++) {
        add(m_parent->findMorphKeyframe(name, i));
    }
}

void
MotionKeyframeSelection::addSelfShadowKeyframes(nanoem_frame_index_t start, nanoem_frame_index_t end)
{
    for (nanoem_frame_index_t i = start; i <= end; i++) {
        add(m_parent->findSelfShadowKeyframe(i));
    }
}

bool
MotionKeyframeSelection::hasAllKeyframes(nanoem_u32_t flags) const NANOEM_DECL_NOEXCEPT
{
    bool hasSelection = false;
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY)) {
        hasSelection |= !m_accessoryKeyframeSelection.empty();
    }
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE)) {
        hasSelection |= !m_boneKeyframeSelection.empty();
    }
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA)) {
        hasSelection |= !m_cameraKeyframeSelection.empty();
    }
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT)) {
        hasSelection |= !m_lightKeyframeSelection.empty();
    }
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL)) {
        hasSelection |= !m_modelKeyframeSelection.empty();
    }
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH)) {
        hasSelection |= !m_morphKeyframeSelection.empty();
    }
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW)) {
        hasSelection |= !m_selfShadowKeyframeSelection.empty();
    }
    return hasSelection;
}

void
MotionKeyframeSelection::clearAllKeyframes(nanoem_u32_t flags)
{
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY)) {
        m_accessoryKeyframeSelection.clear();
    }
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE)) {
        m_boneKeyframeSelection.clear();
    }
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA)) {
        m_cameraKeyframeSelection.clear();
    }
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT)) {
        m_lightKeyframeSelection.clear();
    }
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL)) {
        m_modelKeyframeSelection.clear();
    }
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH)) {
        m_morphKeyframeSelection.clear();
    }
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW)) {
        m_selfShadowKeyframeSelection.clear();
    }
}

bool
MotionKeyframeSelection::isParentOf(const nanoem_motion_keyframe_object_t *ko) const NANOEM_DECL_NOEXCEPT
{
    return m_parent->data() == nanoemMotionKeyframeObjectGetParentMotion(ko);
}

} /* namespace internal */
} /* namespace nanoem */
