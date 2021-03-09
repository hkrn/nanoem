/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/RemoveBoneKeyframeCommand.h"

#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/private/CommonInclude.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"

namespace nanoem {
namespace command {

RemoveBoneKeyframeCommand::~RemoveBoneKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
RemoveBoneKeyframeCommand::create(const void *messagePtr, const Model *model, Motion *motion)
{
    /* bone keyframe list is read from message so no need to feed keyframe list */
    RemoveBoneKeyframeCommand *command =
        nanoem_new(RemoveBoneKeyframeCommand(Motion::BoneKeyframeList(), model, motion));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
RemoveBoneKeyframeCommand::create(const Motion::BoneKeyframeList &keyframes, const Model *model, Motion *motion)
{
    RemoveBoneKeyframeCommand *command = nanoem_new(RemoveBoneKeyframeCommand(keyframes, model, motion));
    return command->createCommand();
}

void
RemoveBoneKeyframeCommand::undo(Error &error)
{
    addKeyframe(error);
}

void
RemoveBoneKeyframeCommand::redo(Error &error)
{
    removeKeyframe(error);
}

const char *
RemoveBoneKeyframeCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "RemoveBoneKeyframeCommand";
}

BaseBoneKeyframeCommand::BoneKeyframeList
RemoveBoneKeyframeCommand::toKeyframeList(
    const Motion::BoneKeyframeList &keyframes, const Model *model, const Motion *motion)
{
    const IMotionKeyframeSelection *selection = motion->selection();
    nanoem_unicode_string_factory_t *factory = model->project()->unicodeStringFactory();
    BoneKeyframeList newKeyframes;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (Motion::BoneKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        const nanoem_motion_bone_keyframe_t *ko = *it;
        const nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(ko));
        const nanoem_unicode_string_t *name = nanoemMotionBoneKeyframeGetName(ko);
        BoneKeyframe keyframe(frameIndex);
        keyframe.m_name = nanoemUnicodeStringFactoryCloneString(factory, name, &status);
        keyframe.m_bezierCurveOverrideTargetFrameIndex = Motion::kMaxFrameIndex; /* join bezier curve not supported */
        keyframe.m_state.first.assign(ko);
        keyframe.m_selected = selection->contains(ko);
        newKeyframes.push_back(keyframe);
    }
    return newKeyframes;
}

RemoveBoneKeyframeCommand::RemoveBoneKeyframeCommand(
    const Motion::BoneKeyframeList &keyframes, const Model *model, Motion *motion)
    : BaseBoneKeyframeCommand(toKeyframeList(keyframes, model, motion), model, motion)
{
}

void
RemoveBoneKeyframeCommand::read(const void *messagePtr)
{
    readMessage(static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_remove_bone_keyframe, true);
}

void
RemoveBoneKeyframeCommand::write(void *messagePtr)
{
    writeMessage(messagePtr, NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_BONE_KEYFRAME);
}

void
RemoveBoneKeyframeCommand::release(void *messagePtr)
{
    releaseMessage(static_cast<Nanoem__Application__Command *>(messagePtr)->redo_remove_bone_keyframe);
}

} /* namespace command */
} /* namespace nanoem */
