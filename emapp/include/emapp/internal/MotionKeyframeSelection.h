/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_MOTIONKEYFRAMESELECTION_H_
#define NANOEM_EMAPP_INTERNAL_MOTIONKEYFRAMESELECTION_H_

#include "emapp/IMotionKeyframeSelection.h"

namespace nanoem {
namespace internal {

class MotionKeyframeSelection NANOEM_DECL_SEALED : public IMotionKeyframeSelection {
public:
    MotionKeyframeSelection(Motion *parent);
    ~MotionKeyframeSelection() NANOEM_DECL_NOEXCEPT;

    bool contains(const nanoem_motion_accessory_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void getAll(Motion::AccessoryKeyframeList &keyframes, int *startOffset) const NANOEM_DECL_OVERRIDE;
    void add(const nanoem_motion_accessory_keyframe_t *keyframe) NANOEM_DECL_OVERRIDE;
    void remove(const nanoem_motion_accessory_keyframe_t *keyframe) NANOEM_DECL_OVERRIDE;
    bool contains(const nanoem_motion_bone_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void getAll(Motion::BoneKeyframeList &keyframes, int *startOffset) const NANOEM_DECL_OVERRIDE;
    void add(const nanoem_motion_bone_keyframe_t *keyframe) NANOEM_DECL_OVERRIDE;
    void remove(const nanoem_motion_bone_keyframe_t *keyframe) NANOEM_DECL_OVERRIDE;
    bool contains(const nanoem_motion_camera_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void getAll(Motion::CameraKeyframeList &keyframes, int *startOffset) const NANOEM_DECL_OVERRIDE;
    void add(const nanoem_motion_camera_keyframe_t *keyframe) NANOEM_DECL_OVERRIDE;
    void remove(const nanoem_motion_camera_keyframe_t *keyframe) NANOEM_DECL_OVERRIDE;
    bool contains(const nanoem_motion_light_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void getAll(Motion::LightKeyframeList &keyframes, int *startOffset) const NANOEM_DECL_OVERRIDE;
    void add(const nanoem_motion_light_keyframe_t *keyframe) NANOEM_DECL_OVERRIDE;
    void remove(const nanoem_motion_light_keyframe_t *keyframe) NANOEM_DECL_OVERRIDE;
    bool contains(const nanoem_motion_model_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void getAll(Motion::ModelKeyframeList &keyframes, int *startOffset) const NANOEM_DECL_OVERRIDE;
    void add(const nanoem_motion_model_keyframe_t *keyframe) NANOEM_DECL_OVERRIDE;
    void remove(const nanoem_motion_model_keyframe_t *keyframe) NANOEM_DECL_OVERRIDE;
    bool contains(const nanoem_motion_morph_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void getAll(Motion::MorphKeyframeList &keyframes, int *startOffset) const NANOEM_DECL_OVERRIDE;
    void add(const nanoem_motion_morph_keyframe_t *keyframe) NANOEM_DECL_OVERRIDE;
    void remove(const nanoem_motion_morph_keyframe_t *keyframe) NANOEM_DECL_OVERRIDE;
    bool contains(const nanoem_motion_self_shadow_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void getAll(Motion::SelfShadowKeyframeList &keyframes, int *startOffset) const NANOEM_DECL_OVERRIDE;
    void add(const nanoem_motion_self_shadow_keyframe_t *keyframe) NANOEM_DECL_OVERRIDE;
    void remove(const nanoem_motion_self_shadow_keyframe_t *keyframe) NANOEM_DECL_OVERRIDE;
    void addAllKeyframes(nanoem_u32_t flags) NANOEM_DECL_OVERRIDE;
    bool hasAllKeyframes(nanoem_u32_t flags) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void clearAllKeyframes(nanoem_u32_t flags) NANOEM_DECL_OVERRIDE;
    void addAccessoryKeyframes(nanoem_frame_index_t start, nanoem_frame_index_t end) NANOEM_DECL_OVERRIDE;
    void addBoneKeyframes(
        const nanoem_model_bone_t *bone, nanoem_frame_index_t start, nanoem_frame_index_t end) NANOEM_DECL_OVERRIDE;
    void addCameraKeyframes(nanoem_frame_index_t start, nanoem_frame_index_t end) NANOEM_DECL_OVERRIDE;
    void addLightKeyframes(nanoem_frame_index_t start, nanoem_frame_index_t end) NANOEM_DECL_OVERRIDE;
    void addModelKeyframes(nanoem_frame_index_t start, nanoem_frame_index_t end) NANOEM_DECL_OVERRIDE;
    void addMorphKeyframes(
        const nanoem_model_morph_t *morph, nanoem_frame_index_t start, nanoem_frame_index_t end) NANOEM_DECL_OVERRIDE;
    void addSelfShadowKeyframes(nanoem_frame_index_t start, nanoem_frame_index_t end) NANOEM_DECL_OVERRIDE;

private:
    bool isParentOf(const nanoem_motion_keyframe_object_t *ko) const NANOEM_DECL_NOEXCEPT;

    Motion *m_parent;
    Motion::AccessoryKeyframeSet m_accessoryKeyframeSelection;
    Motion::BoneKeyframeSet m_boneKeyframeSelection;
    Motion::CameraKeyframeSet m_cameraKeyframeSelection;
    Motion::LightKeyframeSet m_lightKeyframeSelection;
    Motion::ModelKeyframeSet m_modelKeyframeSelection;
    Motion::MorphKeyframeSet m_morphKeyframeSelection;
    Motion::SelfShadowKeyframeSet m_selfShadowKeyframeSelection;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_MOTIONKEYFRAMESELECTION_H_ */
