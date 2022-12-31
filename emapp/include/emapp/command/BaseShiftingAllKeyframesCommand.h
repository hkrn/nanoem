/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_BASESHIFTALLKEYFRAMESCOMMAND_H_
#define NANOEM_EMAPP_COMMAND_BASESHIFTALLKEYFRAMESCOMMAND_H_

#include "emapp/command/BaseUndoCommand.h"

#include "nanoem/ext/mutable.h"

namespace nanoem {

class Motion;

namespace command {

class BaseShiftingAllKeyframesCommand : public BaseUndoCommand {
public:
    virtual ~BaseShiftingAllKeyframesCommand() NANOEM_DECL_NOEXCEPT;

protected:
    Motion *resolveMotion(nanoem_u32_t handle);
    void shiftAllKeyframesBackward(Error &error);
    void shiftAllKeyframesForward(Error &error);
    nanoem_u16_t resolveHandle() const;

    BaseShiftingAllKeyframesCommand(Project *project);
    BaseShiftingAllKeyframesCommand(Motion *motion, nanoem_frame_index_t frameIndex, nanoem_u32_t types);

    Motion *m_motion;
    nanoem_frame_index_t m_localFrameIndex;
    nanoem_u32_t m_types;

private:
    typedef tinystl::vector<nanoem_mutable_motion_accessory_keyframe_t *, TinySTLAllocator>
        MutableAccessoryKeyframeList;
    typedef tinystl::vector<nanoem_mutable_motion_bone_keyframe_t *, TinySTLAllocator> MutableModelBoneKeyframeList;
    typedef tinystl::vector<nanoem_mutable_motion_camera_keyframe_t *, TinySTLAllocator> MutableCameraKeyframeList;
    typedef tinystl::vector<nanoem_mutable_motion_light_keyframe_t *, TinySTLAllocator> MutableLightKeyframeList;
    typedef tinystl::vector<nanoem_mutable_motion_model_keyframe_t *, TinySTLAllocator> MutableModelKeyframeList;
    typedef tinystl::vector<nanoem_mutable_motion_morph_keyframe_t *, TinySTLAllocator> MutableModelMorphKeyframeList;
    typedef tinystl::vector<nanoem_mutable_motion_self_shadow_keyframe_t *, TinySTLAllocator>
        MutableSelfShadowKeyframeList;

    void shiftAllAccessoryKeyframesBackward(nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status);
    void shiftAllBoneKeyframesBackward(nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status);
    void shiftAllCameraKeyframesBackward(nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status);
    void shiftAllLightKeyframesBackward(nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status);
    void shiftAllModelKeyframesBackward(nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status);
    void shiftAllMorphKeyframesBackward(nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status);
    void shiftAllSelfShadowKeyframesBackward(nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status);

    void shiftAllAccessoryKeyframesForward(nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status);
    void shiftAllBoneKeyframesForward(nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status);
    void shiftAllCameraKeyframesForward(nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status);
    void shiftAllLightKeyframesForward(nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status);
    void shiftAllModelKeyframesForward(nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status);
    void shiftAllMorphKeyframesForward(nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status);
    void shiftAllSelfShadowKeyframesForward(nanoem_mutable_motion_t *mutableMotion, nanoem_status_t *status);

    void shiftAllAccessoryKeyframesWithOffset(nanoem_mutable_motion_t *mutableMotion,
        const MutableAccessoryKeyframeList &keyframes, int offset, nanoem_status_t *status);
    void shiftAllBoneKeyframesWithOffset(nanoem_mutable_motion_t *mutableMotion,
        const MutableModelBoneKeyframeList &keyframes, int offset, nanoem_status_t *status);
    void shiftAllCameraKeyframesWithOffset(nanoem_mutable_motion_t *mutableMotion,
        const MutableCameraKeyframeList &keyframes, int offset, nanoem_status_t *status);
    void shiftAllLightKeyframesWithOffset(nanoem_mutable_motion_t *mutableMotion,
        const MutableLightKeyframeList &keyframes, int offset, nanoem_status_t *status);
    void shiftAllModelKeyframesWithOffset(nanoem_mutable_motion_t *mutableMotion,
        const MutableModelKeyframeList &keyframes, int offset, nanoem_status_t *status);
    void shiftAllMorphKeyframesWithOffset(nanoem_mutable_motion_t *mutableMotion,
        const MutableModelMorphKeyframeList &keyframes, int offset, nanoem_status_t *status);
    void shiftAllSelfShadowKeyframesWithOffset(nanoem_mutable_motion_t *mutableMotion,
        const MutableSelfShadowKeyframeList &keyframes, int offset, nanoem_status_t *status);

    MutableAccessoryKeyframeList m_accessoryKeyframes;
    MutableModelBoneKeyframeList m_boneKeyframes;
    MutableCameraKeyframeList m_cameraKeyframes;
    MutableLightKeyframeList m_lightKeyframes;
    MutableModelKeyframeList m_modelKeyframes;
    MutableModelMorphKeyframeList m_morphKeyframes;
    MutableSelfShadowKeyframeList m_selfShadowKeyframes;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_BASESHIFTALLKEYFRAMESCOMMAND_H_ */
