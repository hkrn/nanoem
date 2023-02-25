/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/AddLightKeyframeCommand.h"

#include "../CommandMessage.inl"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace command {

AddLightKeyframeCommand::~AddLightKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
AddLightKeyframeCommand::create(const void *messagePtr, const ILight *light, Motion *motion)
{
    /* light keyframe state is read from message so no need to feed frame indices (to construct keyframe list) */
    AddLightKeyframeCommand *command = nanoem_new(AddLightKeyframeCommand(light, Motion::FrameIndexList(), motion));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
AddLightKeyframeCommand::create(const ILight *light, const Motion::FrameIndexList &targetFrameIndices, Motion *motion)
{
    AddLightKeyframeCommand *command = nanoem_new(AddLightKeyframeCommand(light, targetFrameIndices, motion));
    return command->createCommand();
}

void
AddLightKeyframeCommand::undo(Error &error)
{
    removeKeyframe(error);
}

void
AddLightKeyframeCommand::redo(Error &error)
{
    addKeyframe(error);
}

const char *
AddLightKeyframeCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "AddLightKeyframeCommand";
}

BaseLightKeyframeCommand::LightKeyframeList
AddLightKeyframeCommand::toKeyframeList(
    const ILight *light, const Motion::FrameIndexList &frameIndices, const Motion *motion)
{
    LightKeyframeList newKeyframes;
    const Motion::FrameIndexList &newFrameIndices = motion ? frameIndices : Motion::FrameIndexList();
    for (Motion::FrameIndexList::const_iterator it = newFrameIndices.begin(), end = newFrameIndices.end(); it != end;
         ++it) {
        const nanoem_frame_index_t frameIndex = *it;
        LightKeyframe keyframe(frameIndex);
        keyframe.m_state.first.assign(light);
        if (const nanoem_motion_light_keyframe_t *ko = motion->findLightKeyframe(frameIndex)) {
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

AddLightKeyframeCommand::AddLightKeyframeCommand(
    const ILight *light, const Motion::FrameIndexList &targetFrameIndices, Motion *motion)
    : BaseLightKeyframeCommand(toKeyframeList(light, targetFrameIndices, motion), light, motion)
{
}

void
AddLightKeyframeCommand::read(const void *messagePtr)
{
    readMessage(static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_add_light_keyframe, false);
}

void
AddLightKeyframeCommand::write(void *messagePtr)
{
    writeMessage(messagePtr, NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_LIGHT_KEYFRAME);
}

void
AddLightKeyframeCommand::release(void *messagePtr)
{
    releaseMessage(static_cast<Nanoem__Application__Command *>(messagePtr)->redo_add_light_keyframe);
}

} /* namespace command */
} /* namespace nanoem */
