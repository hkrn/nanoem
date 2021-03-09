/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_IMOTIONKEYFRAMESELECTION_H_
#define NANOEM_EMAPP_IMOTIONKEYFRAMESELECTION_H_

#include "emapp/Motion.h"

namespace nanoem {

class IMotionKeyframeSelection {
public:
    virtual ~IMotionKeyframeSelection() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual bool contains(const nanoem_motion_accessory_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT = 0;
    virtual void getAll(Motion::AccessoryKeyframeList &keyframes, int *startOffset) const = 0;
    virtual void add(const nanoem_motion_accessory_keyframe_t *keyframe) = 0;
    virtual void remove(const nanoem_motion_accessory_keyframe_t *keyframe) = 0;
    virtual bool contains(const nanoem_motion_bone_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT = 0;
    virtual void getAll(Motion::BoneKeyframeList &keyframes, int *startOffset) const = 0;
    virtual void add(const nanoem_motion_bone_keyframe_t *keyframe) = 0;
    virtual void remove(const nanoem_motion_bone_keyframe_t *keyframe) = 0;
    virtual bool contains(const nanoem_motion_camera_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT = 0;
    virtual void getAll(Motion::CameraKeyframeList &keyframes, int *startOffset) const = 0;
    virtual void add(const nanoem_motion_camera_keyframe_t *keyframe) = 0;
    virtual void remove(const nanoem_motion_camera_keyframe_t *keyframe) = 0;
    virtual bool contains(const nanoem_motion_light_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT = 0;
    virtual void getAll(Motion::LightKeyframeList &keyframes, int *startOffset) const = 0;
    virtual void add(const nanoem_motion_light_keyframe_t *keyframe) = 0;
    virtual void remove(const nanoem_motion_light_keyframe_t *keyframe) = 0;
    virtual bool contains(const nanoem_motion_model_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT = 0;
    virtual void getAll(Motion::ModelKeyframeList &keyframes, int *startOffset) const = 0;
    virtual void add(const nanoem_motion_model_keyframe_t *keyframe) = 0;
    virtual void remove(const nanoem_motion_model_keyframe_t *keyframe) = 0;
    virtual bool contains(const nanoem_motion_morph_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT = 0;
    virtual void getAll(Motion::MorphKeyframeList &keyframes, int *startOffset) const = 0;
    virtual void add(const nanoem_motion_morph_keyframe_t *keyframe) = 0;
    virtual void remove(const nanoem_motion_morph_keyframe_t *keyframe) = 0;
    virtual bool contains(const nanoem_motion_self_shadow_keyframe_t *keyframe) const NANOEM_DECL_NOEXCEPT = 0;
    virtual void getAll(Motion::SelfShadowKeyframeList &keyframes, int *startOffset) const = 0;
    virtual void add(const nanoem_motion_self_shadow_keyframe_t *keyframe) = 0;
    virtual void remove(const nanoem_motion_self_shadow_keyframe_t *keyframe) = 0;
    virtual void addAllKeyframes(nanoem_u32_t flags) = 0;
    virtual bool hasAllKeyframes(nanoem_u32_t flags) const NANOEM_DECL_NOEXCEPT = 0;
    virtual void clearAllKeyframes(nanoem_u32_t flags) = 0;
    virtual void addAccessoryKeyframes(nanoem_frame_index_t start, nanoem_frame_index_t end) = 0;
    virtual void addBoneKeyframes(
        const nanoem_model_bone_t *bone, nanoem_frame_index_t start, nanoem_frame_index_t end) = 0;
    virtual void addCameraKeyframes(nanoem_frame_index_t start, nanoem_frame_index_t end) = 0;
    virtual void addLightKeyframes(nanoem_frame_index_t start, nanoem_frame_index_t end) = 0;
    virtual void addModelKeyframes(nanoem_frame_index_t start, nanoem_frame_index_t end) = 0;
    virtual void addMorphKeyframes(
        const nanoem_model_morph_t *morph, nanoem_frame_index_t start, nanoem_frame_index_t end) = 0;
    virtual void addSelfShadowKeyframes(nanoem_frame_index_t start, nanoem_frame_index_t end) = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_IMOTIONKEYFRAMESELECTION_H_ */
