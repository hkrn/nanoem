/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/TransformMorphCommand.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"
#include "emapp/model/Morph.h"
#include "emapp/private/CommonInclude.h"

#include "undo/undo.h"

namespace nanoem {
namespace command {

TransformMorphCommand::~TransformMorphCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
TransformMorphCommand::create(const void *messagePtr, Model *model, Project *project)
{
    TransformMorphCommand *self =
        nanoem_new(TransformMorphCommand(WeightStateList(), model::BindPose(), model, project));
    self->read(messagePtr);
    return self->createCommand();
}

undo_command_t *
TransformMorphCommand::create(
    const WeightStateList &weightStates, const model::BindPose &currentBindPose, Model *model, Project *project)
{
    TransformMorphCommand *self = nanoem_new(TransformMorphCommand(weightStates, currentBindPose, model, project));
    return self->createCommand();
}

void
TransformMorphCommand::undo(Error &error)
{
    BX_UNUSED_1(error);
    execute(true);
}

void
TransformMorphCommand::redo(Error &error)
{
    BX_UNUSED_1(error);
    execute(false);
    /* call setTransformPerformedAt after resetTransformPerformedAt in execute */
    currentProject()->setTransformPerformedAt(
        tinystl::make_pair(m_localFrameIndex, undoStackGetOffset(m_model->undoStack())));
}

const char *
TransformMorphCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "TransformMorphCommand";
}

void
TransformMorphCommand::execute(bool undo)
{
    Project *project = currentProject();
    project->resetTransformPerformedAt();
    project->seek(m_localFrameIndex, true);
    m_model->restoreBindPose(m_currentBindPose);
    for (WeightStateList::const_iterator it = m_weightStates.begin(), end = m_weightStates.end(); it != end; ++it) {
        const WeightState &state = *it;
        if (model::Morph *morph = model::Morph::cast(state.first)) {
            morph->setWeight(undo ? state.second.second : state.second.first);
        }
    }
    m_model->resetAllMorphDeformStates();
    m_model->deformAllMorphs(true);
    m_model->performAllBonesTransform();
}

TransformMorphCommand::TransformMorphCommand(
    const WeightStateList &weightStates, const model::BindPose &currentBindPose, Model *model, Project *project)
    : BaseUndoCommand(project)
    , m_localFrameIndex(project->currentLocalFrameIndex())
    , m_weightStates(weightStates)
    , m_model(model)
    , m_currentBindPose(currentBindPose)
{
    nanoem_assert(m_model, "must not be nullptr");
}

void
TransformMorphCommand::read(const void *messagePtr)
{
    const Nanoem__Application__RedoTransformMorphCommand *command =
        static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_transform_morph;
    const nanoem_rsize_t numCurrentItems = command->n_current_bind_pose;
    const nanoem_rsize_t numLastItems = command->n_last_bind_pose;
    Model *model = currentProject()->resolveRedoModel(command->model_handle);
    if (model && numCurrentItems == numLastItems) {
        m_model = model;
        m_weightStates.resize(numCurrentItems);
        for (nanoem_rsize_t i = 0; i < numCurrentItems; i++) {
            const Nanoem__Application__RedoTransformMorphCommand__State *state = command->current_bind_pose[i];
            const nanoem_model_morph_t *morph = model->findRedoMorph(state->morph_index);
            WeightState &s = m_weightStates[i];
            s.first = morph;
            s.second.first = state->weight;
        }
        for (nanoem_rsize_t i = 0; i < numLastItems; i++) {
            const Nanoem__Application__RedoTransformMorphCommand__State *state = command->last_bind_pose[i];
            WeightState &s = m_weightStates[i];
            s.second.second = state->weight;
        }
    }
}

void
TransformMorphCommand::write(void *messagePtr)
{
    Nanoem__Application__RedoTransformMorphCommand *command =
        nanoem_new(Nanoem__Application__RedoTransformMorphCommand);
    nanoem__application__redo_transform_morph_command__init(command);
    command->model_handle = m_model->handle();
    command->n_current_bind_pose = m_weightStates.size();
    command->current_bind_pose =
        new Nanoem__Application__RedoTransformMorphCommand__State *[command->n_current_bind_pose];
    nanoem_rsize_t offset = 0;
    for (WeightStateList::const_iterator it = m_weightStates.begin(), end = m_weightStates.end(); it != end; ++it) {
        Nanoem__Application__RedoTransformMorphCommand__State *state =
            nanoem_new(Nanoem__Application__RedoTransformMorphCommand__State);
        nanoem__application__redo_transform_morph_command__state__init(state);
        state->morph_index = model::Morph::index(it->first);
        state->weight = it->second.first;
        command->current_bind_pose[offset++] = state;
    }
    command->n_last_bind_pose = m_weightStates.size();
    command->last_bind_pose = new Nanoem__Application__RedoTransformMorphCommand__State *[command->n_last_bind_pose];
    offset = 0;
    for (WeightStateList::const_iterator it = m_weightStates.begin(), end = m_weightStates.end(); it != end; ++it) {
        Nanoem__Application__RedoTransformMorphCommand__State *state =
            nanoem_new(Nanoem__Application__RedoTransformMorphCommand__State);
        nanoem__application__redo_transform_morph_command__state__init(state);
        state->morph_index = model::Morph::index(it->first);
        state->weight = it->second.second;
        command->last_bind_pose[offset++] = state;
    }
    writeCommandMessage(command, NANOEM__APPLICATION__COMMAND__TYPE_REDO_TRANSFORM_MORPH, messagePtr);
}

void
TransformMorphCommand::release(void *messagePtr)
{
    Nanoem__Application__RedoTransformMorphCommand *command =
        static_cast<Nanoem__Application__Command *>(messagePtr)->redo_transform_morph;
    for (nanoem_rsize_t i = 0, numStates = command->n_current_bind_pose; i < numStates; i++) {
        nanoem_delete(command->current_bind_pose[i]);
    }
    delete[] command->current_bind_pose;
    for (nanoem_rsize_t i = 0, numStates = command->n_last_bind_pose; i < numStates; i++) {
        nanoem_delete(command->last_bind_pose[i]);
    }
    delete[] command->last_bind_pose;
    nanoem_delete(command);
}

} /* namespace command */
} /* namespace nanoem */
