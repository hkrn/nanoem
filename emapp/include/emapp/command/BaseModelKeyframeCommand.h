/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_BASEMODELKEYFRAMECOMMAND_H_
#define NANOEM_EMAPP_COMMAND_BASEMODELKEYFRAMECOMMAND_H_

#include "emapp/command/BaseKeyframeCommand.h"

namespace nanoem {
namespace command {

class BaseModelKeyframeCommand : public BaseKeyframeCommand {
public:
    typedef tinystl::unordered_map<const nanoem_model_bone_t *, bool, TinySTLAllocator> ConstraintStateMap;
    virtual ~BaseModelKeyframeCommand() NANOEM_DECL_NOEXCEPT;

protected:
    struct ModelKeyframe : Keyframe {
        struct State {
            State();
            ~State() NANOEM_DECL_NOEXCEPT;
            static ConstraintStateMap toCreateConstraintStateMapFromModel(const Model *model);
            static StringPairMap toOutsideParentMapFromModel(const Model *model);
            static ConstraintStateMap toConstraintStateMapFromKeyframe(
                const nanoem_motion_model_keyframe_t *keyframe, const Model *model);
            static StringPairMap toOutsideParentMapFromKeyframe(
                const nanoem_motion_model_keyframe_t *keyframe, const Model *model);
            void assign(const Model *model);
            void assign(const nanoem_motion_model_keyframe_t *keyframe, const Model *model);
            ConstraintStateMap m_constraintStates;
            StringPairMap m_outsideParents;
            tinystl::pair<Vector4, nanoem_f32_t> m_edge;
            bool m_enablePhysicsSimulation;
            bool m_visible;
        };
        ModelKeyframe()
            : Keyframe(0)
        {
        }
        ModelKeyframe(nanoem_frame_index_t frameIndex)
            : Keyframe(frameIndex)
        {
        }
        void
        setFrameIndex(const nanoem_motion_t *motion, nanoem_frame_index_t value, bool removing)
        {
            m_frameIndex = value;
            m_updated = !isRemovable(value, removing) && nanoemMotionFindModelKeyframeObject(motion, value) != NULL;
        }
        tinystl::pair<State, State> m_state;
    };
    typedef tinystl::vector<ModelKeyframe, TinySTLAllocator> ModelKeyframeList;

    BaseModelKeyframeCommand(const ModelKeyframeList &keyframes, const Model *model, Motion *motion);

    void updateConstraintState(nanoem_mutable_motion_model_keyframe_t *mutableKeyframe, const ConstraintStateMap &value,
        nanoem_status_t &status);
    void updateOutsideParent(
        nanoem_mutable_motion_model_keyframe_t *mutableKeyframe, const StringPairMap &value, nanoem_status_t &status);
    void restoreKeyframeState(nanoem_mutable_motion_model_keyframe_t *keyframe, const ModelKeyframe::State &state);
    void addKeyframe(Error &error);
    void removeKeyframe(Error &error);
    void readMessage(const void *messagePtr, bool removing);
    void writeMessage(void *messagePtr, nanoem_u32_t type);
    void releaseMessage(void *messagePtr);
    void readStateMessage(const void *opaque, ModelKeyframe::State &s);
    void writeStateMessage(const ModelKeyframe::State &s, void *opaque);
    void releaseStateMessage(void *opaque);

    const Model *m_model;
    ModelKeyframeList m_keyframes;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_BASEMODELKEYFRAMECOMMAND_H_ */
