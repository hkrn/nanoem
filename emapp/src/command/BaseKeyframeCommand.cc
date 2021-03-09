/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/BaseKeyframeCommand.h"

#include "emapp/Error.h"
#include "emapp/IEventPublisher.h"
#include "emapp/Project.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace command {

BaseKeyframeCommand::~BaseKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

void
BaseKeyframeCommand::commit(nanoem_mutable_motion_t *motion)
{
    const Project *project = m_motion->project();
    nanoem_frame_index_t lastDuration = project->duration();
    nanoemMutableMotionSortAllKeyframes(motion);
    m_motion->setDirty(true);
    nanoem_frame_index_t currentDuration = project->duration();
    if (currentDuration != lastDuration) {
        m_motion->project()->eventPublisher()->publishUpdateDurationEvent(currentDuration, lastDuration);
    }
}

void
BaseKeyframeCommand::resetTransformPerformedAt()
{
    m_motion->project()->resetTransformPerformedAt();
}

BaseKeyframeCommand::BaseKeyframeCommand(Motion *motion)
    : BaseUndoCommand(motion->project())
    , m_motion(motion)
{
    nanoem_assert(m_motion, "must not be nullptr");
}

} /* namespace command */
} /* namespace nanoem */
