/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/AddModelKeyframeCommand.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace command {

AddModelKeyframeCommand::~AddModelKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
AddModelKeyframeCommand::create(const void *messagePtr, const Model *model, Motion *motion)
{
    /* accessory keyframe state is read from message so no need to feed frame indices (to construct keyframe list) */
    AddModelKeyframeCommand *command = nanoem_new(AddModelKeyframeCommand(model, Motion::FrameIndexList(), motion));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
AddModelKeyframeCommand::create(const Model *model, const Motion::FrameIndexList &targetFrameIndices, Motion *motion)
{
    AddModelKeyframeCommand *command = nanoem_new(AddModelKeyframeCommand(model, targetFrameIndices, motion));
    return command->createCommand();
}

void
AddModelKeyframeCommand::undo(Error &error)
{
    removeKeyframe(error);
}

void
AddModelKeyframeCommand::redo(Error &error)
{
    addKeyframe(error);
}

const char *
AddModelKeyframeCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "AddModelKeyframeCommand";
}

BaseModelKeyframeCommand::ModelKeyframeList
AddModelKeyframeCommand::toKeyframeList(
    const Model *model, const Motion::FrameIndexList &frameIndices, const Motion *motion)
{
    ModelKeyframeList newKeyframes;
    const Motion::FrameIndexList &newFrameIndices = motion ? frameIndices : Motion::FrameIndexList();
    for (Motion::FrameIndexList::const_iterator it = newFrameIndices.begin(), end = newFrameIndices.end(); it != end;
         ++it) {
        const nanoem_frame_index_t frameIndex = *it;
        ModelKeyframe keyframe(frameIndex);
        keyframe.m_state.first.assign(model);
        if (const nanoem_motion_model_keyframe_t *ko =
                nanoemMotionFindModelKeyframeObject(motion->data(), frameIndex)) {
            keyframe.m_state.second.assign(ko, model);
            keyframe.m_updated = true;
        }
        else {
            keyframe.m_selected = true;
        }
        newKeyframes.push_back(keyframe);
    }
    return newKeyframes;
}

AddModelKeyframeCommand::AddModelKeyframeCommand(
    const Model *model, const Motion::FrameIndexList &targetFrameIndices, Motion *motion)
    : BaseModelKeyframeCommand(toKeyframeList(model, targetFrameIndices, motion), model, motion)
{
}

void
AddModelKeyframeCommand::read(const void *messagePtr)
{
    readMessage(static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_add_model_keyframe, false);
}

void
AddModelKeyframeCommand::write(void *messagePtr)
{
    writeMessage(messagePtr, NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_MODEL_KEYFRAME);
}

void
AddModelKeyframeCommand::release(void *messagePtr)
{
    releaseMessage(static_cast<Nanoem__Application__Command *>(messagePtr)->redo_add_model_keyframe);
}

} /* namespace command */
} /* namespace nanoem */
