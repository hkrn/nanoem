/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/RemoveAccessoryKeyframeCommand.h"

#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/private/CommonInclude.h"

#include "../CommandMessage.inl"

namespace nanoem {
namespace command {

RemoveAccessoryKeyframeCommand::~RemoveAccessoryKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
RemoveAccessoryKeyframeCommand::create(const void *messagePtr, const Accessory *accessory, Motion *motion)
{
    /* accessory keyframe list is read from message so no need to feed keyframe list */
    RemoveAccessoryKeyframeCommand *command =
        nanoem_new(RemoveAccessoryKeyframeCommand(Motion::AccessoryKeyframeList(), accessory, motion));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
RemoveAccessoryKeyframeCommand::create(
    const Motion::AccessoryKeyframeList &keyframes, const Accessory *accessory, Motion *motion)
{
    RemoveAccessoryKeyframeCommand *command = nanoem_new(RemoveAccessoryKeyframeCommand(keyframes, accessory, motion));
    return command->createCommand();
}

void
RemoveAccessoryKeyframeCommand::undo(Error &error)
{
    addKeyframe(error);
}

void
RemoveAccessoryKeyframeCommand::redo(Error &error)
{
    removeKeyframe(error);
}

const char *
RemoveAccessoryKeyframeCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "RemoveAccessoryKeyframeCommand";
}

BaseAccessoryKeyframeCommand::AccessoryKeyframeList
RemoveAccessoryKeyframeCommand::toKeyframeList(
    const Motion::AccessoryKeyframeList &keyframes, const Accessory *accessory, const Motion *motion)
{
    const IMotionKeyframeSelection *selection = motion->selection();
    AccessoryKeyframeList newKeyframes;
    for (Motion::AccessoryKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        const nanoem_motion_accessory_keyframe_t *ko = *it;
        const nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionAccessoryKeyframeGetKeyframeObject(ko));
        AccessoryKeyframe keyframe(frameIndex);
        keyframe.m_state.first.assign(ko, accessory);
        keyframe.m_selected = selection->contains(ko);
        newKeyframes.push_back(keyframe);
    }
    return newKeyframes;
}

RemoveAccessoryKeyframeCommand::RemoveAccessoryKeyframeCommand(
    const Motion::AccessoryKeyframeList &keyframes, const Accessory *accessory, Motion *motion)
    : BaseAccessoryKeyframeCommand(toKeyframeList(keyframes, accessory, motion), accessory, motion)
{
}

void
RemoveAccessoryKeyframeCommand::read(const void *messagePtr)
{
    readMessage(static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_remove_accessory_keyframe, true);
}

void
RemoveAccessoryKeyframeCommand::write(void *messagePtr)
{
    writeMessage(messagePtr, NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_ACCESSORY_KEYFRAME);
}

void
RemoveAccessoryKeyframeCommand::release(void *messagePtr)
{
    releaseMessage(static_cast<Nanoem__Application__Command *>(messagePtr)->redo_remove_accessory_keyframe);
}

} /* namespace command */
} /* namespace nanoem */
