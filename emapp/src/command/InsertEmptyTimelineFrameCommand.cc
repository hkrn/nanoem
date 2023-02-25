/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/InsertEmptyTimelineFrameCommand.h"

#include "emapp/private/CommonInclude.h"

#include "../CommandMessage.inl"
#include "bx/handlealloc.h"

namespace nanoem {
namespace command {

InsertEmptyTimelineFrameCommand::~InsertEmptyTimelineFrameCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
InsertEmptyTimelineFrameCommand::create(const void *messagePtr, Project *project)
{
    InsertEmptyTimelineFrameCommand *command = nanoem_new(InsertEmptyTimelineFrameCommand(project));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
InsertEmptyTimelineFrameCommand::create(Motion *motion, nanoem_frame_index_t frameIndex, nanoem_u32_t types)
{
    InsertEmptyTimelineFrameCommand *c = nanoem_new(InsertEmptyTimelineFrameCommand(motion, frameIndex, types));
    return c->createCommand();
}

void
InsertEmptyTimelineFrameCommand::undo(Error &error)
{
    shiftAllKeyframesForward(error);
}

void
InsertEmptyTimelineFrameCommand::redo(Error &error)
{
    shiftAllKeyframesBackward(error);
}

void
InsertEmptyTimelineFrameCommand::read(const void *messagePtr)
{
    const Nanoem__Application__RedoInsertEmptyTimelineFrameCommand *command =
        static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_insert_empty_timeline_frame;
    m_localFrameIndex = CommandMessageUtil::saturatedFrameIndex(command->frame_index);
    m_types = command->types;
    if (Motion *motion = resolveMotion(command->handle)) {
        m_motion = motion;
    }
}

void
InsertEmptyTimelineFrameCommand::write(void *messagePtr)
{
    Nanoem__Application__RedoInsertEmptyTimelineFrameCommand *command =
        nanoem_new(Nanoem__Application__RedoInsertEmptyTimelineFrameCommand);
    nanoem__application__redo_insert_empty_timeline_frame_command__init(command);
    command->frame_index = m_localFrameIndex;
    command->types = m_types;
    nanoem_u16_t handle = resolveHandle();
    if (handle != bx::kInvalidHandle) {
        command->handle = handle;
        command->has_handle = 1;
    }
    writeCommandMessage(command, NANOEM__APPLICATION__COMMAND__TYPE_REDO_INSERT_EMPTY_TIMELINE_FRAME, messagePtr);
}

void
InsertEmptyTimelineFrameCommand::release(void *messagePtr)
{
    Nanoem__Application__RedoInsertEmptyTimelineFrameCommand *command =
        static_cast<Nanoem__Application__Command *>(messagePtr)->redo_insert_empty_timeline_frame;
    nanoem_delete(command);
}

const char *
InsertEmptyTimelineFrameCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "ShiftAllKeyframesBackwardCommand";
}

InsertEmptyTimelineFrameCommand::InsertEmptyTimelineFrameCommand(Project *project)
    : BaseShiftingAllKeyframesCommand(project)
{
}

InsertEmptyTimelineFrameCommand::InsertEmptyTimelineFrameCommand(
    Motion *motion, nanoem_frame_index_t frameIndex, nanoem_u32_t types)
    : BaseShiftingAllKeyframesCommand(motion, frameIndex, types)
{
}

} /* namespace command */
} /* namespace nanoem */
