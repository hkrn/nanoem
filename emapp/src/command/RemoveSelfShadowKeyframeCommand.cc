/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/RemoveSelfShadowKeyframeCommand.h"

#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/private/CommonInclude.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"

namespace nanoem {
namespace command {

RemoveSelfShadowKeyframeCommand::~RemoveSelfShadowKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
RemoveSelfShadowKeyframeCommand::create(const void *messagePtr, const ShadowCamera *shadow, Motion *motion)
{
    RemoveSelfShadowKeyframeCommand *command =
        nanoem_new(RemoveSelfShadowKeyframeCommand(Motion::SelfShadowKeyframeList(), shadow, motion));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
RemoveSelfShadowKeyframeCommand::create(
    const Motion::SelfShadowKeyframeList &keyframes, const ShadowCamera *shadow, Motion *motion)
{
    RemoveSelfShadowKeyframeCommand *command = nanoem_new(RemoveSelfShadowKeyframeCommand(keyframes, shadow, motion));
    return command->createCommand();
}

void
RemoveSelfShadowKeyframeCommand::undo(Error &error)
{
    addKeyframe(error);
}

void
RemoveSelfShadowKeyframeCommand::redo(Error &error)
{
    removeKeyframe(error);
}

const char *
RemoveSelfShadowKeyframeCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "RemoveSelfShadowKeyframeCommand";
}

BaseSelfShadowKeyframeCommand::SelfShadowKeyframeList
RemoveSelfShadowKeyframeCommand::toKeyframeList(const Motion::SelfShadowKeyframeList &keyframes, const Motion *motion)
{
    const IMotionKeyframeSelection *selection = motion->selection();
    SelfShadowKeyframeList newKeyframes;
    for (Motion::SelfShadowKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end;
         ++it) {
        const nanoem_motion_self_shadow_keyframe_t *ko = *it;
        const nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionSelfShadowKeyframeGetKeyframeObject(ko));
        SelfShadowKeyframe keyframe(frameIndex);
        keyframe.m_state.first.assign(ko);
        keyframe.m_selected = selection->contains(ko);
        newKeyframes.push_back(keyframe);
    }
    return newKeyframes;
}

RemoveSelfShadowKeyframeCommand::RemoveSelfShadowKeyframeCommand(
    const Motion::SelfShadowKeyframeList &keyframes, const ShadowCamera *light, Motion *motion)
    : BaseSelfShadowKeyframeCommand(toKeyframeList(keyframes, motion), light, motion)
{
}

void
RemoveSelfShadowKeyframeCommand::read(const void *messagePtr)
{
    readMessage(static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_remove_self_shadow_keyframe, true);
}

void
RemoveSelfShadowKeyframeCommand::write(void *messagePtr)
{
    writeMessage(messagePtr, NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_SELF_SHADOW_KEYFRAME);
}

void
RemoveSelfShadowKeyframeCommand::release(void *messagePtr)
{
    releaseMessage(static_cast<Nanoem__Application__Command *>(messagePtr)->redo_remove_self_shadow_keyframe);
}

} /* namespace command */
} /* namespace nanoem */
