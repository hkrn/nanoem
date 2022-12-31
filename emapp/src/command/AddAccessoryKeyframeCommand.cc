/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/AddAccessoryKeyframeCommand.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace command {

AddAccessoryKeyframeCommand::~AddAccessoryKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
AddAccessoryKeyframeCommand::create(const void *messagePtr, const Accessory *accessory, Motion *motion)
{
    /* accessory keyframe state is read from message so no need to feed frame indices (to construct keyframe list) */
    AddAccessoryKeyframeCommand *command =
        nanoem_new(AddAccessoryKeyframeCommand(accessory, Motion::FrameIndexList(), motion));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
AddAccessoryKeyframeCommand::create(
    const Accessory *accessory, const Motion::FrameIndexList &targetFrameIndices, Motion *motion)
{
    AddAccessoryKeyframeCommand *command =
        nanoem_new(AddAccessoryKeyframeCommand(accessory, targetFrameIndices, motion));
    return command->createCommand();
}

void
AddAccessoryKeyframeCommand::undo(Error &error)
{
    removeKeyframe(error);
}

void
AddAccessoryKeyframeCommand::redo(Error &error)
{
    addKeyframe(error);
}

const char *
AddAccessoryKeyframeCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "AddAccessoryKeyframeCommand";
}

BaseAccessoryKeyframeCommand::AccessoryKeyframeList
AddAccessoryKeyframeCommand::toKeyframeList(
    const Accessory *accessory, const Motion::FrameIndexList &frameIndices, const Motion *motion)
{
    AccessoryKeyframeList newKeyframes;
    const Motion::FrameIndexList &newFrameIndices = motion ? frameIndices : Motion::FrameIndexList();
    for (Motion::FrameIndexList::const_iterator it = newFrameIndices.begin(), end = newFrameIndices.end(); it != end;
         ++it) {
        const nanoem_frame_index_t frameIndex = *it;
        AccessoryKeyframe keyframe(frameIndex);
        keyframe.m_state.first.assign(accessory);
        if (const nanoem_motion_accessory_keyframe_t *ko = motion->findAccessoryKeyframe(frameIndex)) {
            keyframe.m_state.second.assign(ko, accessory);
            keyframe.m_updated = true;
        }
        else {
            keyframe.m_selected = true;
        }
        newKeyframes.push_back(keyframe);
    }
    return newKeyframes;
}

AddAccessoryKeyframeCommand::AddAccessoryKeyframeCommand(
    const Accessory *accessory, const Motion::FrameIndexList &targetFrameIndices, Motion *motion)
    : BaseAccessoryKeyframeCommand(toKeyframeList(accessory, targetFrameIndices, motion), accessory, motion)
{
}

void
AddAccessoryKeyframeCommand::read(const void *messagePtr)
{
    readMessage(static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_add_accessory_keyframe, false);
}

void
AddAccessoryKeyframeCommand::write(void *messagePtr)
{
    writeMessage(messagePtr, NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_ACCESSORY_KEYFRAME);
}

void
AddAccessoryKeyframeCommand::release(void *messagePtr)
{
    releaseMessage(static_cast<Nanoem__Application__Command *>(messagePtr)->redo_add_accessory_keyframe);
}

} /* namespace command */
} /* namespace nanoem */
