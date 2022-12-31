/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/UberMotionKeyframeSelection.h"

#include "emapp/ListUtils.h"

namespace nanoem {
namespace internal {
namespace imgui {

UberMotionKeyframeSelection::UberMotionKeyframeSelection()
{
}

UberMotionKeyframeSelection::~UberMotionKeyframeSelection() NANOEM_DECL_NOEXCEPT
{
}

bool
UberMotionKeyframeSelection::contains(const nanoem_motion_accessory_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT
{
    return m_accessoryKeyframeSelection.find(keyframe) != m_accessoryKeyframeSelection.end();
}

void
UberMotionKeyframeSelection::getAll(Motion::AccessoryKeyframeList &keyframes, int *) const
{
    keyframes = ListUtils::toListFromSet(m_accessoryKeyframeSelection);
}

void
UberMotionKeyframeSelection::add(const nanoem_motion_accessory_keyframe_t *keyframe)
{
    m_accessoryKeyframeSelection.insert(keyframe);
}

void
UberMotionKeyframeSelection::remove(const nanoem_motion_accessory_keyframe_t *keyframe)
{
    m_accessoryKeyframeSelection.erase(keyframe);
}

bool
UberMotionKeyframeSelection::contains(const nanoem_motion_bone_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT
{
    return m_boneKeyframeSelection.find(keyframe) != m_boneKeyframeSelection.end();
}

void
UberMotionKeyframeSelection::getAll(Motion::BoneKeyframeList &keyframes, int *) const
{
    keyframes = ListUtils::toListFromSet(m_boneKeyframeSelection);
}

void
UberMotionKeyframeSelection::add(const nanoem_motion_bone_keyframe_t *keyframe)
{
    m_boneKeyframeSelection.insert(keyframe);
}

void
UberMotionKeyframeSelection::remove(const nanoem_motion_bone_keyframe_t *keyframe)
{
    m_boneKeyframeSelection.erase(keyframe);
}

bool
UberMotionKeyframeSelection::contains(const nanoem_motion_camera_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT
{
    return m_cameraKeyframeSelection.find(keyframe) != m_cameraKeyframeSelection.end();
}

void
UberMotionKeyframeSelection::getAll(Motion::CameraKeyframeList &keyframes, int *) const
{
    keyframes = ListUtils::toListFromSet(m_cameraKeyframeSelection);
}

void
UberMotionKeyframeSelection::add(const nanoem_motion_camera_keyframe_t *keyframe)
{
    m_cameraKeyframeSelection.insert(keyframe);
}

void
UberMotionKeyframeSelection::remove(const nanoem_motion_camera_keyframe_t *keyframe)
{
    m_cameraKeyframeSelection.erase(keyframe);
}

bool
UberMotionKeyframeSelection::contains(const nanoem_motion_light_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT
{
    return m_lightKeyframeSelection.find(keyframe) != m_lightKeyframeSelection.end();
}

void
UberMotionKeyframeSelection::getAll(Motion::LightKeyframeList &keyframes, int *) const
{
    keyframes = ListUtils::toListFromSet(m_lightKeyframeSelection);
}

void
UberMotionKeyframeSelection::add(const nanoem_motion_light_keyframe_t *keyframe)
{
    m_lightKeyframeSelection.insert(keyframe);
}

void
UberMotionKeyframeSelection::remove(const nanoem_motion_light_keyframe_t *keyframe)
{
    m_lightKeyframeSelection.erase(keyframe);
}

bool
UberMotionKeyframeSelection::contains(const nanoem_motion_model_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT
{
    return m_modelKeyframeSelection.find(keyframe) != m_modelKeyframeSelection.end();
}

void
UberMotionKeyframeSelection::getAll(Motion::ModelKeyframeList &keyframes, int *) const
{
    keyframes = ListUtils::toListFromSet(m_modelKeyframeSelection);
}

void
UberMotionKeyframeSelection::add(const nanoem_motion_model_keyframe_t *keyframe)
{
    m_modelKeyframeSelection.insert(keyframe);
}

void
UberMotionKeyframeSelection::remove(const nanoem_motion_model_keyframe_t *keyframe)
{
    m_modelKeyframeSelection.erase(keyframe);
}

bool
UberMotionKeyframeSelection::contains(const nanoem_motion_morph_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT
{
    return m_morphKeyframeSelection.find(keyframe) != m_morphKeyframeSelection.end();
}

void
UberMotionKeyframeSelection::getAll(Motion::MorphKeyframeList &keyframes, int *) const
{
    keyframes = ListUtils::toListFromSet(m_morphKeyframeSelection);
}

void
UberMotionKeyframeSelection::add(const nanoem_motion_morph_keyframe_t *keyframe)
{
    m_morphKeyframeSelection.insert(keyframe);
}

void
UberMotionKeyframeSelection::remove(const nanoem_motion_morph_keyframe_t *keyframe)
{
    m_morphKeyframeSelection.erase(keyframe);
}

bool
UberMotionKeyframeSelection::contains(const nanoem_motion_self_shadow_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT
{
    return m_selfShadowKeyframeSelection.find(keyframe) != m_selfShadowKeyframeSelection.end();
}

void
UberMotionKeyframeSelection::getAll(Motion::SelfShadowKeyframeList &keyframes, int *) const
{
    keyframes = ListUtils::toListFromSet(m_selfShadowKeyframeSelection);
}

void
UberMotionKeyframeSelection::add(const nanoem_motion_self_shadow_keyframe_t *keyframe)
{
    m_selfShadowKeyframeSelection.insert(keyframe);
}

void
UberMotionKeyframeSelection::remove(const nanoem_motion_self_shadow_keyframe_t *keyframe)
{
    m_selfShadowKeyframeSelection.erase(keyframe);
}

void
UberMotionKeyframeSelection::addAllKeyframes(nanoem_u32_t)
{
}

bool
UberMotionKeyframeSelection::hasAllKeyframes(nanoem_u32_t) const NANOEM_DECL_NOEXCEPT
{
    return false;
}

void
UberMotionKeyframeSelection::clearAllKeyframes(nanoem_u32_t)
{
}

void
UberMotionKeyframeSelection::addAccessoryKeyframes(nanoem_frame_index_t, nanoem_frame_index_t)
{
}

void
UberMotionKeyframeSelection::addBoneKeyframes(const nanoem_model_bone_t *, nanoem_frame_index_t, nanoem_frame_index_t)
{
}

void
UberMotionKeyframeSelection::addCameraKeyframes(nanoem_frame_index_t, nanoem_frame_index_t)
{
}

void
UberMotionKeyframeSelection::addLightKeyframes(nanoem_frame_index_t, nanoem_frame_index_t)
{
}

void
UberMotionKeyframeSelection::addModelKeyframes(nanoem_frame_index_t, nanoem_frame_index_t)
{
}

void
UberMotionKeyframeSelection::addMorphKeyframes(const nanoem_model_morph_t *, nanoem_frame_index_t, nanoem_frame_index_t)
{
}

void
UberMotionKeyframeSelection::addSelfShadowKeyframes(nanoem_frame_index_t, nanoem_frame_index_t)
{
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
