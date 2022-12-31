/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/TransformBoneCommand.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"
#include "emapp/private/CommonInclude.h"

#include "undo/undo.h"

namespace nanoem {
namespace command {

TransformBoneCommand::~TransformBoneCommand() NANOEM_DECL_NOEXCEPT
{
    m_dirtyStates.clear();
}

undo_command_t *
TransformBoneCommand::create(const void *messagePtr, Model *model, Project *project)
{
    TransformBoneCommand *self =
        nanoem_new(TransformBoneCommand(model::BindPose(), model::BindPose(), model::Bone::List(), model, project));
    self->read(messagePtr);
    return self->createCommand();
}

undo_command_t *
TransformBoneCommand::create(const model::BindPose &lastBindPose, const model::BindPose &currentBindPose,
    const model::Bone::List &targetBones, Model *model, Project *project)
{
    TransformBoneCommand *self =
        nanoem_new(TransformBoneCommand(lastBindPose, currentBindPose, targetBones, model, project));
    return self->createCommand();
}

void
TransformBoneCommand::undo(Error &error)
{
    BX_UNUSED_1(error);
    execute(m_lastBindPose, false);
}

void
TransformBoneCommand::redo(Error &error)
{
    BX_UNUSED_1(error);
    execute(m_currentBindPose, true);
    /* call setTransformPerformedAt after resetTransformPerformedAt in execute */
    currentProject()->setTransformPerformedAt(
        tinystl::make_pair(m_localFrameIndex, undoStackGetOffset(m_model->undoStack())));
}

const char *
TransformBoneCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "TransformBoneCommand";
}

void
TransformBoneCommand::execute(const model::BindPose &value, bool markAllDirty)
{
    Project *project = currentProject();
    project->resetTransformPerformedAt();
    project->seek(m_localFrameIndex, true);
    m_model->restoreBindPose(value);
    m_model->performAllBonesTransform();
    /* call performAllBonesTransform again to solve inherent parent IK bone */
    m_model->performAllBonesTransform();
    if (markAllDirty) {
        for (DirtyStateList::const_iterator it = m_dirtyStates.begin(), end = m_dirtyStates.end(); it != end; ++it) {
            const DirtyState &state = *it;
            if (model::Bone *bone = model::Bone::cast(state.first)) {
                bone->setDirty(true);
            }
        }
    }
    else {
        for (DirtyStateList::const_iterator it = m_dirtyStates.begin(), end = m_dirtyStates.end(); it != end; ++it) {
            const DirtyState &state = *it;
            if (model::Bone *bone = model::Bone::cast(state.first)) {
                bone->setDirty(state.second);
            }
        }
    }
}

TransformBoneCommand::DirtyStateList
TransformBoneCommand::createDirtyStates(const model::Bone::List &bones, const Model *model)
{
    DirtyStateList result;
    for (model::Bone::List::const_iterator it = bones.begin(), end = bones.end(); it != end; ++it) {
        const nanoem_model_bone_t *bonePtr = *it;
        if (!model->isConstraintJointBoneActive(bonePtr)) {
            if (const model::Bone *bone = model::Bone::cast(bonePtr)) {
                result.push_back(tinystl::make_pair(bonePtr, bone->isDirty()));
            }
        }
    }
    return result;
}

TransformBoneCommand::TransformBoneCommand(const model::BindPose &lastBindPose, const model::BindPose &currentBindPose,
    const model::Bone::List &targetBones, Model *model, Project *project)
    : BaseUndoCommand(project)
    , m_localFrameIndex(project->currentLocalFrameIndex())
    , m_model(model)
    , m_lastBindPose(lastBindPose)
    , m_currentBindPose(currentBindPose)
    , m_dirtyStates(createDirtyStates(targetBones, model))
{
    nanoem_assert(m_model, "must not be nullptr");
}

void
TransformBoneCommand::read(const void *messagePtr)
{
    const Nanoem__Application__RedoTransformBoneCommand *command =
        static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_transform_bone;
    Model *model = currentProject()->resolveRedoModel(command->model_handle);
    if (model) {
        m_model = model;
        m_currentBindPose.restoreAllMessages(model, command->current_bone_states, command->n_current_bone_states);
        m_lastBindPose.restoreAllMessages(model, command->last_bone_states, command->n_last_bone_states);
        const nanoem_rsize_t numItems = command->n_dirty_bone_indices;
        for (nanoem_rsize_t i = 0; i < numItems; i++) {
            const nanoem_model_bone_t *bonePtr = model->findRedoBone(command->dirty_bone_indices[i]);
            m_dirtyStates.push_back(tinystl::make_pair(bonePtr, true));
        }
    }
}

void
TransformBoneCommand::write(void *messagePtr)
{
    Nanoem__Application__RedoTransformBoneCommand *command = nanoem_new(Nanoem__Application__RedoTransformBoneCommand);
    nanoem__application__redo_transform_bone_command__init(command);
    command->model_handle = m_model->handle();
    m_currentBindPose.saveAllMessages(&command->current_bone_states, command->n_current_bone_states);
    m_lastBindPose.saveAllMessages(&command->last_bone_states, command->n_last_bone_states);
    command->dirty_bone_indices = new nanoem_i32_t[m_dirtyStates.size()];
    nanoem_rsize_t numDirtyBones = 0;
    for (DirtyStateList::const_iterator it = m_dirtyStates.begin(), end = m_dirtyStates.end(); it != end; ++it) {
        if (it->second) {
            command->dirty_bone_indices[numDirtyBones++] = model::Bone::index(it->first);
        }
    }
    command->n_dirty_bone_indices = numDirtyBones;
    writeCommandMessage(command, NANOEM__APPLICATION__COMMAND__TYPE_REDO_TRANSFORM_BONE, messagePtr);
}

void
TransformBoneCommand::release(void *messagePtr)
{
    Nanoem__Application__RedoTransformBoneCommand *command =
        static_cast<Nanoem__Application__Command *>(messagePtr)->redo_transform_bone;
    model::BindPose::releaseAllMessages(command->current_bone_states, command->n_current_bone_states);
    model::BindPose::releaseAllMessages(command->last_bone_states, command->n_last_bone_states);
    delete[] command->dirty_bone_indices;
    nanoem_delete(command);
}

} /* namespace command */
} /* namespace nanoem */
