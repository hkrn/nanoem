/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/AddSelfShadowKeyframeCommand.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace command {

AddSelfShadowKeyframeCommand::~AddSelfShadowKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
AddSelfShadowKeyframeCommand::create(const void *messagePtr, const ShadowCamera *shadow, Motion *motion)
{
    /* self shadow keyframe state is read from message so no need to feed frame indices (to construct keyframe list) */
    AddSelfShadowKeyframeCommand *command =
        nanoem_new(AddSelfShadowKeyframeCommand(shadow, Motion::FrameIndexList(), motion));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
AddSelfShadowKeyframeCommand::create(
    const ShadowCamera *shadow, const Motion::FrameIndexList &targetFrameIndices, Motion *motion)
{
    AddSelfShadowKeyframeCommand *command =
        nanoem_new(AddSelfShadowKeyframeCommand(shadow, targetFrameIndices, motion));
    return command->createCommand();
}

void
AddSelfShadowKeyframeCommand::undo(Error &error)
{
    removeKeyframe(error);
}

void
AddSelfShadowKeyframeCommand::redo(Error &error)
{
    addKeyframe(error);
}

const char *
AddSelfShadowKeyframeCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "AddSelfShadowKeyframeCommand";
}

BaseSelfShadowKeyframeCommand::SelfShadowKeyframeList
AddSelfShadowKeyframeCommand::toKeyframeList(
    const ShadowCamera *light, const Motion::FrameIndexList &frameIndices, const Motion *motion)
{
    SelfShadowKeyframeList newKeyframes;
    const Motion::FrameIndexList &newFrameIndices = motion ? frameIndices : Motion::FrameIndexList();
    for (Motion::FrameIndexList::const_iterator it = newFrameIndices.begin(), end = newFrameIndices.end(); it != end;
         ++it) {
        const nanoem_frame_index_t frameIndex = *it;
        SelfShadowKeyframe keyframe(frameIndex);
        keyframe.m_state.first.assign(light);
        if (const nanoem_motion_self_shadow_keyframe_t *ko = motion->findSelfShadowKeyframe(frameIndex)) {
            keyframe.m_state.second.assign(ko);
            keyframe.m_updated = true;
        }
        else {
            keyframe.m_selected = true;
        }
        newKeyframes.push_back(keyframe);
    }
    return newKeyframes;
}

AddSelfShadowKeyframeCommand::AddSelfShadowKeyframeCommand(
    const ShadowCamera *light, const Motion::FrameIndexList &targetFrameIndices, Motion *motion)
    : BaseSelfShadowKeyframeCommand(toKeyframeList(light, targetFrameIndices, motion), light, motion)
{
}

void
AddSelfShadowKeyframeCommand::read(const void *messagePtr)
{
    readMessage(static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_add_self_shadow_keyframe, false);
}

void
AddSelfShadowKeyframeCommand::write(void *messagePtr)
{
    writeMessage(messagePtr, NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_SELF_SHADOW_KEYFRAME);
}

void
AddSelfShadowKeyframeCommand::release(void *messagePtr)
{
    releaseMessage(static_cast<Nanoem__Application__Command *>(messagePtr)->redo_add_self_shadow_keyframe);
}

} /* namespace command */
} /* namespace nanoem */
