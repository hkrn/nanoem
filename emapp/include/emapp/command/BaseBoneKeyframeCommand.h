/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_BASEBONEKEYFRAMECOMMAND_H_
#define NANOEM_EMAPP_COMMAND_BASEBONEKEYFRAMECOMMAND_H_

#include "emapp/command/BaseKeyframeCommand.h"

#include "emapp/model/Bone.h"

namespace nanoem {
namespace command {

class BaseBoneKeyframeCommand : public BaseKeyframeCommand {
public:
    ~BaseBoneKeyframeCommand() NANOEM_DECL_NOEXCEPT;

protected:
    struct BezierControlPointParameter {
        glm::u8vec4 m_value[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM];
    };
    struct BezierControlPointParameterTranslationOnly {
        glm::u8vec4 m_value[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION];
    };
    typedef tinystl::pair<BezierControlPointParameterTranslationOnly, BezierControlPointParameterTranslationOnly>
        BezierControlPointParameterTranslationOnlyPair;
    struct BoneKeyframe : Keyframe {
        struct State {
            State();
            ~State() NANOEM_DECL_NOEXCEPT;
            void assign(const model::Bone *bone);
            void assign(const nanoem_motion_bone_keyframe_t *keyframe);
            Vector4 m_translation;
            Quaternion m_orientation;
            nanoem_u32_t m_stageIndex;
            BezierControlPointParameter m_parameter;
            bool m_enablePhysicsSimulation;
        };
        BoneKeyframe()
            : Keyframe(0)
            , m_name(NULL)
            , m_bezierCurveOverrideTargetFrameIndex(Motion::kMaxFrameIndex)
            , wasDirty(false)
        {
        }
        BoneKeyframe(nanoem_frame_index_t frameIndex)
            : Keyframe(frameIndex)
            , m_name(NULL)
            , m_bezierCurveOverrideTargetFrameIndex(Motion::kMaxFrameIndex)
            , wasDirty(false)
        {
        }
        void
        setFrameIndex(const nanoem_motion_t *motion, nanoem_frame_index_t value, bool removing)
        {
            m_frameIndex = value;
            m_updated =
                !isRemovable(value, removing) && nanoemMotionFindBoneKeyframeObject(motion, m_name, value) != NULL;
        }
        nanoem_unicode_string_t *m_name;
        tinystl::pair<State, State> m_state;
        nanoem_frame_index_t m_bezierCurveOverrideTargetFrameIndex;
        BezierControlPointParameterTranslationOnlyPair m_translationParameters;
        bool wasDirty;
    };
    typedef tinystl::vector<BoneKeyframe, nanoem::TinySTLAllocator> BoneKeyframeList;

    BaseBoneKeyframeCommand(const BoneKeyframeList &keyframes, const Model *model, Motion *motion);

    void restoreKeyframeState(
        nanoem_mutable_motion_bone_keyframe_t *keyframe, const BoneKeyframe::State &state, bool &linear);
    void addKeyframe(Error &error);
    void removeKeyframe(Error &error);
    void readMessage(const void *messagePtr, bool removing);
    void writeMessage(void *messagePtr, nanoem_u32_t type);
    void releaseMessage(void *messagePtr);
    void readStateMessage(const void *opaque, BoneKeyframe::State &s);
    void writeStateMessage(const BoneKeyframe::State &s, void *opaque);
    void releaseStateMessage(void *opaque);

    const Model *m_model;
    BoneKeyframeList m_keyframes;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_BASEBONEKEYFRAMECOMMAND_H_ */
