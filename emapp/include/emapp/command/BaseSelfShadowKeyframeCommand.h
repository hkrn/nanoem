/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_BASESELFSHADOWKEYFRAMECOMMAND_H_
#define NANOEM_EMAPP_COMMAND_BASESELFSHADOWKEYFRAMECOMMAND_H_

#include "emapp/command/BaseKeyframeCommand.h"

namespace nanoem {
namespace command {

class BaseSelfShadowKeyframeCommand : public BaseKeyframeCommand {
public:
    virtual ~BaseSelfShadowKeyframeCommand() NANOEM_DECL_NOEXCEPT;

protected:
    struct SelfShadowKeyframe : Keyframe {
        struct State {
            State();
            ~State() NANOEM_DECL_NOEXCEPT;
            nanoem_f32_t m_distance;
            int m_mode;
            void assign(const ShadowCamera *shadow);
            void assign(const nanoem_motion_self_shadow_keyframe_t *keyframe);
        };
        SelfShadowKeyframe()
            : Keyframe(0)
        {
        }
        SelfShadowKeyframe(nanoem_frame_index_t frameIndex)
            : Keyframe(frameIndex)
        {
        }
        void
        setFrameIndex(const nanoem_motion_t *motion, nanoem_frame_index_t value, bool removing)
        {
            m_frameIndex = value;
            m_updated =
                !isRemovable(value, removing) && nanoemMotionFindSelfShadowKeyframeObject(motion, value) != NULL;
        }
        tinystl::pair<State, State> m_state;
    };
    typedef tinystl::vector<SelfShadowKeyframe, nanoem::TinySTLAllocator> SelfShadowKeyframeList;

    BaseSelfShadowKeyframeCommand(const SelfShadowKeyframeList &keyframes, const ShadowCamera *shadow, Motion *motion);

    void restoreKeyframeState(
        nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, const SelfShadowKeyframe::State &state);
    void addKeyframe(Error &error);
    void removeKeyframe(Error &error);
    void readMessage(const void *messagePtr, bool removing);
    void writeMessage(void *messagePtr, nanoem_u32_t type);
    void releaseMessage(void *messagePtr);
    void readStateMessage(const void *opaque, SelfShadowKeyframe::State &s);
    void writeStateMessage(const SelfShadowKeyframe::State &s, void *opaque);
    void releaseStateMessage(void *opaque);

    const ShadowCamera *m_shadowCameraPtr;
    SelfShadowKeyframeList m_keyframes;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_BASESELFSHADOWKEYFRAMECOMMAND_H_ */
