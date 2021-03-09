/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_BASELIGHTKEYFRAMECOMMAND_H_
#define NANOEM_EMAPP_COMMAND_BASELIGHTKEYFRAMECOMMAND_H_

#include "emapp/command/BaseKeyframeCommand.h"

namespace nanoem {
namespace command {

class BaseLightKeyframeCommand : public BaseKeyframeCommand {
public:
    virtual ~BaseLightKeyframeCommand() NANOEM_DECL_NOEXCEPT;

protected:
    struct LightKeyframe : Keyframe {
        struct State {
            State();
            ~State() NANOEM_DECL_NOEXCEPT;
            Vector4 m_color;
            Vector4 m_direction;
            void assign(const ILight *light);
            void assign(const nanoem_motion_light_keyframe_t *keyframe);
        };
        LightKeyframe()
            : Keyframe(0)
        {
        }
        LightKeyframe(nanoem_frame_index_t frameIndex)
            : Keyframe(frameIndex)
        {
        }
        void
        setFrameIndex(const nanoem_motion_t *motion, nanoem_frame_index_t value, bool removing)
        {
            m_frameIndex = value;
            m_updated = !isRemovable(value, removing) && nanoemMotionFindLightKeyframeObject(motion, value) != NULL;
        }
        tinystl::pair<State, State> m_state;
    };
    typedef tinystl::vector<LightKeyframe, nanoem::TinySTLAllocator> LightKeyframeList;

    BaseLightKeyframeCommand(const LightKeyframeList &keyframes, const ILight *light, Motion *motion);

    void restoreKeyframeState(nanoem_mutable_motion_light_keyframe_t *keyframe, const LightKeyframe::State &state);
    void addKeyframe(Error &error);
    void removeKeyframe(Error &error);
    void readMessage(const void *messagePtr, bool removing);
    void writeMessage(void *messagePtr, nanoem_u32_t type);
    void releaseMessage(void *messagePtr);
    void readStateMessage(const void *opaque, LightKeyframe::State &s);
    void writeStateMessage(const LightKeyframe::State &s, void *opaque);
    void releaseStateMessage(void *opaque);

    const ILight *m_light;
    LightKeyframeList m_keyframes;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_BASELIGHTKEYFRAMECOMMAND_H_ */
