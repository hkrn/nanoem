/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/BaseMorphKeyframeCommand.h"

#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/private/CommonInclude.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"

namespace nanoem {
namespace command {

BaseMorphKeyframeCommand::~BaseMorphKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
    nanoem_unicode_string_factory_t *factory = currentProject()->unicodeStringFactory();
    for (MorphKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
        nanoemUnicodeStringFactoryDestroyString(factory, it->m_name);
    }
}

BaseMorphKeyframeCommand::MorphKeyframe::State::State()
    : m_weight(0)
{
}

BaseMorphKeyframeCommand::MorphKeyframe::State::~State() NANOEM_DECL_NOEXCEPT
{
}

void
BaseMorphKeyframeCommand::MorphKeyframe::State::assign(const model::Morph *morph)
{
    m_weight = morph->weight();
}

void
BaseMorphKeyframeCommand::MorphKeyframe::State::assign(const nanoem_motion_morph_keyframe_t *keyframe)
{
    m_weight = nanoemMotionMorphKeyframeGetWeight(keyframe);
}

BaseMorphKeyframeCommand::BaseMorphKeyframeCommand(
    const MorphKeyframeList &keyframes, const Model *model, Motion *motion)
    : BaseKeyframeCommand(motion)
    , m_model(model)
    , m_keyframes(keyframes)
{
    nanoem_assert(m_model, "must not be nullptr");
}

void
BaseMorphKeyframeCommand::restoreKeyframeState(
    nanoem_mutable_motion_morph_keyframe_t *keyframe, const MorphKeyframe::State &state)
{
    nanoem_assert(keyframe, "must NOT be nullptr");
    nanoemMutableMotionMorphKeyframeSetWeight(keyframe, state.m_weight);
}

void
BaseMorphKeyframeCommand::addKeyframe(Error &error)
{
    if (currentProject()->containsMotion(m_motion)) {
        IMotionKeyframeSelection *selection = m_motion->selection();
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_motion_t *motion = m_motion->data();
        nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion, &status);
        resetTransformPerformedAt();
        for (MorphKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
            MorphKeyframe &keyframe = *it;
            nanoem_mutable_motion_morph_keyframe_t *mutableKeyframe;
            if (keyframe.m_updated) {
                mutableKeyframe = nanoemMutableMotionMorphKeyframeCreateByFound(
                    motion, keyframe.m_name, keyframe.m_frameIndex, &status);
            }
            else {
                mutableKeyframe = nanoemMutableMotionMorphKeyframeCreate(motion, &status);
                nanoemMutableMotionAddMorphKeyframe(
                    mutableMotion, mutableKeyframe, keyframe.m_name, keyframe.m_frameIndex, &status);
                if (keyframe.m_selected) {
                    selection->add(nanoemMutableMotionMorphKeyframeGetOriginObject(mutableKeyframe));
                }
            }
            restoreKeyframeState(mutableKeyframe, keyframe.m_state.first);
            nanoemMutableMotionMorphKeyframeDestroy(mutableKeyframe);
            if (status != NANOEM_STATUS_SUCCESS && status != NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_ALREADY_EXISTS) {
                break;
            }
        }
        commit(mutableMotion);
        nanoemMutableMotionDestroy(mutableMotion);
        assignError(status, error);
    }
}

void
BaseMorphKeyframeCommand::removeKeyframe(Error &error)
{
    if (currentProject()->containsMotion(m_motion)) {
        IMotionKeyframeSelection *selection = m_motion->selection();
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_motion_t *motion = m_motion->data();
        nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion, &status);
        resetTransformPerformedAt();
        for (MorphKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
            MorphKeyframe &keyframe = *it;
            nanoem_mutable_motion_morph_keyframe_t *mutableKeyframe =
                nanoemMutableMotionMorphKeyframeCreateByFound(motion, keyframe.m_name, keyframe.m_frameIndex, &status);
            if (keyframe.m_updated) {
                restoreKeyframeState(mutableKeyframe, keyframe.m_state.second);
            }
            else {
                selection->remove(nanoemMutableMotionMorphKeyframeGetOriginObject(mutableKeyframe));
                nanoemMutableMotionRemoveMorphKeyframe(mutableMotion, mutableKeyframe, &status);
            }
            nanoemMutableMotionMorphKeyframeDestroy(mutableKeyframe);
            if (status != NANOEM_STATUS_SUCCESS && status != NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_NOT_FOUND) {
                break;
            }
        }
        commit(mutableMotion);
        nanoemMutableMotionDestroy(mutableMotion);
        assignError(status, error);
    }
}

void
BaseMorphKeyframeCommand::readMessage(const void *messagePtr, bool removing)
{
    const Nanoem__Application__RedoMorphKeyframeCommand *commandPtr =
        static_cast<const Nanoem__Application__RedoMorphKeyframeCommand *>(messagePtr);
    Model *model = currentProject()->resolveRedoModel(commandPtr->model_handle);
    if (model) {
        m_model = model;
        nanoem_rsize_t numKeyframes = commandPtr->n_keyframes;
        m_keyframes.resize(numKeyframes);
        nanoem_unicode_string_factory_t *factory = currentProject()->unicodeStringFactory();
        for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
            const Nanoem__Application__RedoMorphKeyframeCommand__Keyframe *k = commandPtr->keyframes[i];
            MorphKeyframe &keyframe = m_keyframes[i];
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            readStateMessage(k->current_state, keyframe.m_state.first);
            readStateMessage(k->last_state, keyframe.m_state.second);
            keyframe.m_name = nanoemUnicodeStringFactoryCloneString(factory,
                nanoemModelMorphGetName(m_model->findRedoMorph(k->morph_index), NANOEM_LANGUAGE_TYPE_FIRST_ENUM),
                &status);
            keyframe.setFrameIndex(m_motion->data(), CommandMessageUtil::saturatedFrameIndex(k->frame_index), removing);
        }
    }
}

void
BaseMorphKeyframeCommand::writeMessage(void *messagePtr, nanoem_u32_t type)
{
    Nanoem__Application__RedoMorphKeyframeCommand__Keyframe **keyframes =
        new Nanoem__Application__RedoMorphKeyframeCommand__Keyframe *[m_keyframes.size()];
    nanoem_rsize_t offset = 0;
    for (MorphKeyframeList::const_iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
        const MorphKeyframe &keyframe = *it;
        Nanoem__Application__RedoMorphKeyframeCommand__Keyframe *k =
            nanoem_new(Nanoem__Application__RedoMorphKeyframeCommand__Keyframe);
        nanoem__application__redo_morph_keyframe_command__keyframe__init(k);
        k->frame_index = keyframe.m_frameIndex;
        k->morph_index = model::Morph::index(m_model->findMorph(keyframe.m_name));
        writeStateMessage(keyframe.m_state.first, &k->current_state);
        writeStateMessage(keyframe.m_state.second, &k->last_state);
        keyframes[offset++] = k;
    }
    Nanoem__Application__RedoMorphKeyframeCommand *command = nanoem_new(Nanoem__Application__RedoMorphKeyframeCommand);
    nanoem__application__redo_morph_keyframe_command__init(command);
    command->model_handle = m_model->handle();
    command->n_keyframes = m_keyframes.size();
    command->keyframes = keyframes;
    writeCommandMessage(command, type, messagePtr);
}

void
BaseMorphKeyframeCommand::releaseMessage(void *messagePtr)
{
    Nanoem__Application__RedoMorphKeyframeCommand *command =
        static_cast<Nanoem__Application__RedoMorphKeyframeCommand *>(messagePtr);
    nanoem_rsize_t numKeyframes = command->n_keyframes;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        Nanoem__Application__RedoMorphKeyframeCommand__Keyframe *k = command->keyframes[i];
        releaseStateMessage(k->current_state);
        releaseStateMessage(k->last_state);
        nanoem_delete(k);
    }
    delete[] command->keyframes;
    nanoem_delete(command);
}

void
BaseMorphKeyframeCommand::readStateMessage(const void *opaque, MorphKeyframe::State &s)
{
    const Nanoem__Application__RedoMorphKeyframeCommand__State *state =
        static_cast<const Nanoem__Application__RedoMorphKeyframeCommand__State *>(opaque);
    s.m_weight = state->weight;
}

void
BaseMorphKeyframeCommand::writeStateMessage(const MorphKeyframe::State &s, void *opaque)
{
    Nanoem__Application__RedoMorphKeyframeCommand__State *
        *statePtr = static_cast<Nanoem__Application__RedoMorphKeyframeCommand__State **>(opaque),
       *state = nanoem_new(Nanoem__Application__RedoMorphKeyframeCommand__State);
    nanoem__application__redo_morph_keyframe_command__state__init(state);
    state->weight = s.m_weight;
    *statePtr = state;
}

void
BaseMorphKeyframeCommand::releaseStateMessage(void *opaque)
{
    Nanoem__Application__RedoMorphKeyframeCommand__State *statePtr =
        static_cast<Nanoem__Application__RedoMorphKeyframeCommand__State *>(opaque);
    nanoem_delete(statePtr);
}

} /* namespace command */
} /* namespace nanoem */
