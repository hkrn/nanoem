/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_BASEACCESSORYKEYFRAMECOMMAND_H_
#define NANOEM_EMAPP_COMMAND_BASEACCESSORYKEYFRAMECOMMAND_H_

#include "emapp/command/BaseKeyframeCommand.h"

#include "emapp/IDrawable.h"

namespace nanoem {

class Accessory;

namespace command {

class BaseAccessoryKeyframeCommand : public BaseKeyframeCommand {
public:
    virtual ~BaseAccessoryKeyframeCommand() NANOEM_DECL_NOEXCEPT;

protected:
    struct AccessoryKeyframe : Keyframe {
        struct State {
            State();
            ~State();
            void assign(const Accessory *accessory);
            void assign(const nanoem_motion_accessory_keyframe_t *keyframe, const Accessory *accessory);
            Vector4 m_translation;
            Quaternion m_orientation;
            nanoem_f32_t m_scaleFactor;
            nanoem_f32_t m_opacity;
            bool m_shadow;
            bool m_blend;
            bool m_visible;
            StringPair m_outsideParent;
        };
        AccessoryKeyframe()
            : Keyframe(0)
        {
        }
        AccessoryKeyframe(nanoem_frame_index_t frameIndex)
            : Keyframe(frameIndex)
        {
        }
        void
        setFrameIndex(const nanoem_motion_t *motion, nanoem_frame_index_t value, bool removing)
        {
            m_frameIndex = value;
            m_updated = !isRemovable(value, removing) && nanoemMotionFindAccessoryKeyframeObject(motion, value) != NULL;
        }
        tinystl::pair<State, State> m_state;
    };
    typedef tinystl::vector<AccessoryKeyframe, nanoem::TinySTLAllocator> AccessoryKeyframeList;

    BaseAccessoryKeyframeCommand(const AccessoryKeyframeList &keyframes, const Accessory *accessory, Motion *motion);

    void restoreKeyframeState(nanoem_mutable_motion_accessory_keyframe_t *keyframe,
        const AccessoryKeyframe::State &state, nanoem_status_t &status);
    void addKeyframe(Error &error);
    void removeKeyframe(Error &error);
    void readMessage(const void *messagePtr, bool removing);
    void writeMessage(void *messagPtr, nanoem_u32_t type);
    void releaseMessage(void *messagePtr);
    void readStateMessage(const void *opaque, AccessoryKeyframe::State &s);
    void writeStateMessage(const AccessoryKeyframe::State &s, void *opaque);
    void releaseStateMessage(void *opaque);

    const Accessory *m_accessory;
    AccessoryKeyframeList m_keyframes;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_BASEACCESSORYKEYFRAMECOMMAND_H_ */
