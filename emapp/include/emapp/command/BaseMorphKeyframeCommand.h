/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_BASEMORPHKEYFRAMECOMMAND_H_
#define NANOEM_EMAPP_COMMAND_BASEMORPHKEYFRAMECOMMAND_H_

#include "emapp/command/BaseKeyframeCommand.h"

#include "emapp/model/Morph.h"

namespace nanoem {
namespace command {

class BaseMorphKeyframeCommand : public BaseKeyframeCommand {
public:
    virtual ~BaseMorphKeyframeCommand() NANOEM_DECL_NOEXCEPT;

protected:
    struct MorphKeyframe : Keyframe {
        struct State {
            State();
            ~State() NANOEM_DECL_NOEXCEPT;
            void assign(const model::Morph *morph);
            void assign(const nanoem_motion_morph_keyframe_t *keyframe);
            nanoem_f32_t m_weight;
        };
        MorphKeyframe()
            : Keyframe(0)
            , m_name(NULL)
        {
        }
        MorphKeyframe(nanoem_frame_index_t frameIndex)
            : Keyframe(frameIndex)
            , m_name(NULL)
        {
        }
        void
        setFrameIndex(const nanoem_motion_t *motion, nanoem_frame_index_t value, bool removing)
        {
            m_frameIndex = value;
            m_updated =
                !isRemovable(value, removing) && nanoemMotionFindMorphKeyframeObject(motion, m_name, value) != NULL;
        }
        nanoem_unicode_string_t *m_name;
        tinystl::pair<State, State> m_state;
    };
    typedef tinystl::vector<MorphKeyframe, nanoem::TinySTLAllocator> MorphKeyframeList;

    BaseMorphKeyframeCommand(const MorphKeyframeList &keyframes, const Model *model, Motion *motion);

    void restoreKeyframeState(nanoem_mutable_motion_morph_keyframe_t *keyframe, const MorphKeyframe::State &state);
    void addKeyframe(Error &error);
    void removeKeyframe(Error &error);
    void readMessage(const void *messagePtr, bool removing);
    void writeMessage(void *messagePtr, nanoem_u32_t type);
    void releaseMessage(void *messagePtr);
    void readStateMessage(const void *opaque, MorphKeyframe::State &s);
    void writeStateMessage(const MorphKeyframe::State &s, void *opaque);
    void releaseStateMessage(void *opaque);

    const Model *m_model;
    MorphKeyframeList m_keyframes;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_BASEMORPHKEYFRAMECOMMAND_H_ */
