/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/AddMorphKeyframeCommand.h"

#include "../CommandMessage.inl"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace command {

AddMorphKeyframeCommand::~AddMorphKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
AddMorphKeyframeCommand::create(const void *messagePtr, const Model *model, Motion *motion)
{
    /* morph keyframe state is read from message so no need to feed frame index map (to construct keyframe list) */
    AddMorphKeyframeCommand *command =
        nanoem_new(AddMorphKeyframeCommand(Motion::MorphFrameIndexSetMap(), model, motion));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
AddMorphKeyframeCommand::create(const Motion::MorphFrameIndexSetMap &morphs, const Model *model, Motion *motion)
{
    AddMorphKeyframeCommand *command = nanoem_new(AddMorphKeyframeCommand(morphs, model, motion));
    return command->createCommand();
}

void
AddMorphKeyframeCommand::undo(Error &error)
{
    removeKeyframe(error);
}

void
AddMorphKeyframeCommand::redo(Error &error)
{
    addKeyframe(error);
}

const char *
AddMorphKeyframeCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "AddMorphKeyframeCommand";
}

BaseMorphKeyframeCommand::MorphKeyframeList
AddMorphKeyframeCommand::toKeyframeList(
    const Motion::MorphFrameIndexSetMap &morphs, const Model *model, const Motion *motion)
{
    MorphKeyframeList newKeyframes;
    nanoem_unicode_string_factory_t *factory = model->project()->unicodeStringFactory();
    const Motion::MorphFrameIndexSetMap &newMorphs = motion ? morphs : Motion::MorphFrameIndexSetMap();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (Motion::MorphFrameIndexSetMap::const_iterator it = newMorphs.begin(), end = newMorphs.end(); it != end; ++it) {
        if (const nanoem_model_morph_t *morphPtr = it->first) {
            const model::Morph *morph = model::Morph::cast(morphPtr);
            const nanoem_unicode_string_t *name = nanoemModelMorphGetName(morphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
            for (Motion::FrameIndexSet::const_iterator it2 = it->second.begin(), end2 = it->second.end(); it2 != end2;
                 ++it2) {
                MorphKeyframe keyframe(*it2);
                keyframe.m_name = nanoemUnicodeStringFactoryCloneString(factory, name, &status);
                keyframe.m_state.first.assign(morph);
                if (const nanoem_motion_morph_keyframe_t *ko =
                        nanoemMotionFindMorphKeyframeObject(motion->data(), name, keyframe.m_frameIndex)) {
                    keyframe.m_state.second.assign(ko);
                    keyframe.m_updated = true;
                }
                else {
                    keyframe.m_selected = true;
                }
                newKeyframes.push_back(keyframe);
            }
        }
    }
    return newKeyframes;
}

AddMorphKeyframeCommand::AddMorphKeyframeCommand(
    const Motion::MorphFrameIndexSetMap &morphs, const Model *model, Motion *motion)
    : BaseMorphKeyframeCommand(toKeyframeList(morphs, model, motion), model, motion)
{
}

void
AddMorphKeyframeCommand::read(const void *messagePtr)
{
    readMessage(static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_add_morph_keyframe, false);
}

void
AddMorphKeyframeCommand::write(void *messagePtr)
{
    writeMessage(messagePtr, NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_MORPH_KEYFRAME);
}

void
AddMorphKeyframeCommand::release(void *messagePtr)
{
    releaseMessage(static_cast<Nanoem__Application__Command *>(messagePtr)->redo_add_morph_keyframe);
}

} /* namespace command */
} /* namespace nanoem */
