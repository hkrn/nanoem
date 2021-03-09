/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/RemoveTimelineFrameCommand.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"
#include "bx/handlealloc.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace command {

RemoveTimelineFrameCommand::~RemoveTimelineFrameCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
RemoveTimelineFrameCommand::create(const void *messagePtr, Project *project)
{
    RemoveTimelineFrameCommand *command = nanoem_new(RemoveTimelineFrameCommand(project));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
RemoveTimelineFrameCommand::create(Motion *motion, nanoem_frame_index_t frameIndex, nanoem_u32_t types)
{
    RemoveTimelineFrameCommand *c = nanoem_new(RemoveTimelineFrameCommand(motion, frameIndex, types));
    return c->createCommand();
}

void
RemoveTimelineFrameCommand::undo(Error &error)
{
    shiftAllKeyframesBackward(error);
}

void
RemoveTimelineFrameCommand::redo(Error &error)
{
    shiftAllKeyframesForward(error);
}

void
RemoveTimelineFrameCommand::read(const void *messagePtr)
{
    const Nanoem__Application__RedoRemoveTimelineFrameCommand *command =
        static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_remove_timeline_frame;
    m_localFrameIndex = CommandMessageUtil::saturatedFrameIndex(command->frame_index);
    m_types = command->types;
    if (Motion *motion = resolveMotion(command->handle)) {
        m_motion = motion;
    }
}

void
RemoveTimelineFrameCommand::write(void *messagePtr)
{
    Nanoem__Application__RedoRemoveTimelineFrameCommand *command =
        nanoem_new(Nanoem__Application__RedoRemoveTimelineFrameCommand);
    nanoem__application__redo_remove_timeline_frame_command__init(command);
    command->frame_index = m_localFrameIndex;
    command->types = m_types;
    nanoem_u16_t handle = resolveHandle();
    if (handle != bx::kInvalidHandle) {
        command->handle = handle;
        command->has_handle = 1;
    }
    writeCommandMessage(command, NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_TIMELINE_FRAME, messagePtr);
}

void
RemoveTimelineFrameCommand::release(void *messagePtr)
{
    Nanoem__Application__RedoRemoveTimelineFrameCommand *command =
        static_cast<Nanoem__Application__Command *>(messagePtr)->redo_remove_timeline_frame;
    nanoem_delete(command);
}

const char *
RemoveTimelineFrameCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "ShiftAllKeyframesForwardCommand";
}

RemoveTimelineFrameCommand::RemoveTimelineFrameCommand(Project *project)
    : BaseShiftingAllKeyframesCommand(project)
{
}

RemoveTimelineFrameCommand::RemoveTimelineFrameCommand(
    Motion *motion, nanoem_frame_index_t frameIndex, nanoem_u32_t types)
    : BaseShiftingAllKeyframesCommand(motion, frameIndex, types)
{
}

} /* namespace command */
} /* namespace nanoem */
