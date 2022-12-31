/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_BASEKEYFRAMECOMMAND_H_
#define NANOEM_EMAPP_COMMAND_BASEKEYFRAMECOMMAND_H_

#include "emapp/command/BaseUndoCommand.h"

#include "emapp/Motion.h"

namespace nanoem {
namespace command {

class BaseKeyframeCommand : public BaseUndoCommand {
public:
    virtual ~BaseKeyframeCommand() NANOEM_DECL_NOEXCEPT;

protected:
    struct Keyframe {
        Keyframe(nanoem_frame_index_t frameIndex)
            : m_frameIndex(frameIndex)
            , m_updated(frameIndex == 0)
            , m_selected(false)
        {
        }
        virtual ~Keyframe()
        {
        }
        virtual void setFrameIndex(const nanoem_motion_t *motion, nanoem_frame_index_t value, bool removing) = 0;
        static bool
        isRemovable(nanoem_frame_index_t value, bool removing)
        {
            return removing && value > 0;
        }
        nanoem_frame_index_t m_frameIndex;
        bool m_updated;
        bool m_selected;
    };
    typedef tinystl::vector<Keyframe, nanoem::TinySTLAllocator> KeyframeList;

    void commit(nanoem_mutable_motion_t *motion);
    void resetTransformPerformedAt();

    BaseKeyframeCommand(Motion *motion);

    Motion *m_motion;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_BASEKEYFRAMECOMMAND_H_ */
