/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/RemoveMorphKeyframeCommand.h"

#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/private/CommonInclude.h"

#include "../CommandMessage.inl"

namespace nanoem {
namespace command {

RemoveMorphKeyframeCommand::~RemoveMorphKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
RemoveMorphKeyframeCommand::create(const void *messagePtr, const Model *model, Motion *motion)
{
    /* morph keyframe list is read from message so no need to feed keyframe list */
    RemoveMorphKeyframeCommand *command =
        nanoem_new(RemoveMorphKeyframeCommand(Motion::MorphKeyframeList(), model, motion));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
RemoveMorphKeyframeCommand::create(const Motion::MorphKeyframeList &keyframes, const Model *model, Motion *motion)
{
    RemoveMorphKeyframeCommand *command = nanoem_new(RemoveMorphKeyframeCommand(keyframes, model, motion));
    return command->createCommand();
}

void
RemoveMorphKeyframeCommand::undo(Error &error)
{
    addKeyframe(error);
}

void
RemoveMorphKeyframeCommand::redo(Error &error)
{
    removeKeyframe(error);
}

const char *
RemoveMorphKeyframeCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "RemoveMorphKeyframeCommand";
}

BaseMorphKeyframeCommand::MorphKeyframeList
RemoveMorphKeyframeCommand::toKeyframeList(
    const Motion::MorphKeyframeList &keyframes, const Model *model, const Motion *motion)
{
    const IMotionKeyframeSelection *selection = motion->selection();
    nanoem_unicode_string_factory_t *factory = model->project()->unicodeStringFactory();
    MorphKeyframeList newKeyframes;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (Motion::MorphKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        const nanoem_motion_morph_keyframe_t *ko = *it;
        const nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(ko));
        const nanoem_unicode_string_t *name = nanoemMotionMorphKeyframeGetName(ko);
        MorphKeyframe keyframe(frameIndex);
        keyframe.m_name = nanoemUnicodeStringFactoryCloneString(factory, name, &status);
        keyframe.m_state.first.assign(ko);
        keyframe.m_selected = selection->contains(ko);
        newKeyframes.push_back(keyframe);
    }
    return newKeyframes;
}

RemoveMorphKeyframeCommand::RemoveMorphKeyframeCommand(
    const Motion::MorphKeyframeList &keyframes, const Model *model, Motion *motion)
    : BaseMorphKeyframeCommand(toKeyframeList(keyframes, model, motion), model, motion)
{
}

void
RemoveMorphKeyframeCommand::read(const void *messagePtr)
{
    readMessage(static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_remove_morph_keyframe, true);
}

void
RemoveMorphKeyframeCommand::write(void *messagePtr)
{
    writeMessage(messagePtr, NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_MORPH_KEYFRAME);
}

void
RemoveMorphKeyframeCommand::release(void *messagePtr)
{
    releaseMessage(static_cast<Nanoem__Application__Command *>(messagePtr)->redo_remove_morph_keyframe);
}

} /* namespace command */
} /* namespace nanoem */
