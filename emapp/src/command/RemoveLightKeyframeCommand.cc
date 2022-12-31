/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/RemoveLightKeyframeCommand.h"

#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/private/CommonInclude.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"

namespace nanoem {
namespace command {

RemoveLightKeyframeCommand::~RemoveLightKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
RemoveLightKeyframeCommand::create(const void *messagePtr, const ILight *light, Motion *motion)
{
    /* light keyframe list is read from message so no need to feed keyframe list */
    RemoveLightKeyframeCommand *command =
        nanoem_new(RemoveLightKeyframeCommand(Motion::LightKeyframeList(), light, motion));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
RemoveLightKeyframeCommand::create(const Motion::LightKeyframeList &keyframes, const ILight *light, Motion *motion)
{
    RemoveLightKeyframeCommand *command = nanoem_new(RemoveLightKeyframeCommand(keyframes, light, motion));
    return command->createCommand();
}

void
RemoveLightKeyframeCommand::undo(Error &error)
{
    addKeyframe(error);
}

void
RemoveLightKeyframeCommand::redo(Error &error)
{
    removeKeyframe(error);
}

const char *
RemoveLightKeyframeCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "RemoveLightKeyframeCommand";
}

BaseLightKeyframeCommand::LightKeyframeList
RemoveLightKeyframeCommand::toKeyframeList(const Motion::LightKeyframeList &keyframes, const Motion *motion)
{
    const IMotionKeyframeSelection *selection = motion->selection();
    LightKeyframeList newKeyframes;
    for (Motion::LightKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        const nanoem_motion_light_keyframe_t *ko = *it;
        const nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionLightKeyframeGetKeyframeObject(ko));
        LightKeyframe keyframe(frameIndex);
        keyframe.m_state.first.assign(ko);
        keyframe.m_selected = selection->contains(ko);
        newKeyframes.push_back(keyframe);
    }
    return newKeyframes;
}

RemoveLightKeyframeCommand::RemoveLightKeyframeCommand(
    const Motion::LightKeyframeList &keyframes, const ILight *light, Motion *motion)
    : BaseLightKeyframeCommand(toKeyframeList(keyframes, motion), light, motion)
{
}

void
RemoveLightKeyframeCommand::read(const void *messagePtr)
{
    readMessage(static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_remove_light_keyframe, true);
}

void
RemoveLightKeyframeCommand::write(void *messagePtr)
{
    writeMessage(messagePtr, NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_LIGHT_KEYFRAME);
}

void
RemoveLightKeyframeCommand::release(void *messagePtr)
{
    releaseMessage(static_cast<Nanoem__Application__Command *>(messagePtr)->redo_remove_light_keyframe);
}

} /* namespace command */
} /* namespace nanoem */
