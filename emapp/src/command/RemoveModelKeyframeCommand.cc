/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/RemoveModelKeyframeCommand.h"

#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/private/CommonInclude.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"

namespace nanoem {
namespace command {

RemoveModelKeyframeCommand::~RemoveModelKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
RemoveModelKeyframeCommand::create(const void *messagePtr, const Model *model, Motion *motion)
{
    /* model keyframe list is read from message so no need to feed keyframe list */
    RemoveModelKeyframeCommand *command =
        nanoem_new(RemoveModelKeyframeCommand(Motion::ModelKeyframeList(), model, motion));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
RemoveModelKeyframeCommand::create(const Motion::ModelKeyframeList &keyframes, const Model *model, Motion *motion)
{
    RemoveModelKeyframeCommand *command = nanoem_new(RemoveModelKeyframeCommand(keyframes, model, motion));
    return command->createCommand();
}

void
RemoveModelKeyframeCommand::undo(Error &error)
{
    addKeyframe(error);
}

void
RemoveModelKeyframeCommand::redo(Error &error)
{
    removeKeyframe(error);
}

const char *
RemoveModelKeyframeCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "RemoveModelKeyframeCommand";
}

BaseModelKeyframeCommand::ModelKeyframeList
RemoveModelKeyframeCommand::toKeyframeList(
    const Motion::ModelKeyframeList &keyframes, const Model *model, const Motion *motion)
{
    const IMotionKeyframeSelection *selection = motion->selection();
    ModelKeyframeList newKeyframes;
    for (Motion::ModelKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        const nanoem_motion_model_keyframe_t *ko = *it;
        const nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionModelKeyframeGetKeyframeObject(ko));
        ModelKeyframe keyframe(frameIndex);
        keyframe.m_state.first.assign(ko, model);
        keyframe.m_selected = selection->contains(ko);
        newKeyframes.push_back(keyframe);
    }
    return newKeyframes;
}

RemoveModelKeyframeCommand::RemoveModelKeyframeCommand(
    const Motion::ModelKeyframeList &keyframes, const Model *model, Motion *motion)
    : BaseModelKeyframeCommand(toKeyframeList(keyframes, model, motion), model, motion)
{
}

void
RemoveModelKeyframeCommand::read(const void *messagePtr)
{
    readMessage(static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_remove_model_keyframe, true);
}

void
RemoveModelKeyframeCommand::write(void *messagePtr)
{
    writeMessage(messagePtr, NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_MODEL_KEYFRAME);
}

void
RemoveModelKeyframeCommand::release(void *messagePtr)
{
    releaseMessage(static_cast<Nanoem__Application__Command *>(messagePtr)->redo_remove_model_keyframe);
}

} /* namespace command */
} /* namespace nanoem */
