/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/ModelSnapshotCommand.h"

#include "../CommandMessage.inl"
#include "emapp/Error.h"
#include "emapp/Progress.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace command {

ModelSnapshotCommand::~ModelSnapshotCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
ModelSnapshotCommand::create(const void *messagePtr, Project *project)
{
    ModelSnapshotCommand *command = nanoem_new(ModelSnapshotCommand(project));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
ModelSnapshotCommand::create(Model *model, const ByteArray &snapshot)
{
    ModelSnapshotCommand *command = nanoem_new(ModelSnapshotCommand(model, snapshot));
    return command->createCommand();
}

void
ModelSnapshotCommand::undo(Error &error)
{
    execute(m_snapshot.second, error);
}

void
ModelSnapshotCommand::redo(Error &error)
{
    execute(m_snapshot.first, error);
}

void
ModelSnapshotCommand::read(const void *messagePtr)
{
    if (const Nanoem__Application__RedoSaveModelSnapshotCommand *command =
            static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_save_model_snapshot) {
        m_snapshot.first.deflate(command->current_model.data, command->current_model.len);
        m_snapshot.second.deflate(command->last_model.data, command->last_model.len);
    }
}

void
ModelSnapshotCommand::write(void *messagePtr)
{
    Nanoem__Application__RedoSaveModelSnapshotCommand *command =
        nanoem_new(Nanoem__Application__RedoSaveModelSnapshotCommand);
    nanoem__application__redo_save_model_snapshot_command__init(command);
    m_snapshot.first.inflate(&command->current_model);
    m_snapshot.second.inflate(&command->last_model);
    command->handle = m_model->handle();
    writeCommandMessage(command, NANOEM__APPLICATION__COMMAND__TYPE_REDO_SAVE_MOTION_SNAPSHOT, messagePtr);
}

void
ModelSnapshotCommand::release(void *messagePtr)
{
    Nanoem__Application__RedoSaveModelSnapshotCommand *command =
        static_cast<Nanoem__Application__Command *>(messagePtr)->redo_save_model_snapshot;
    delete[] command->current_model.data;
    delete[] command->last_model.data;
    nanoem_delete(command);
}

const char *
ModelSnapshotCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "ModelSnapshotCommand";
}

void
ModelSnapshotCommand::execute(const LZ4Data &input, Error &error)
{
    if (m_model) {
        Project *project = currentProject();
        Progress progress(project, 0);
        ByteArray inflated;
        if (input.inflate(inflated)) {
            nanoem_frame_index_t frameIndex = project->currentLocalFrameIndex();
            const Motion *motion = project->resolveMotion(m_model);
            /* preserve editing mode because setActiveModel changes editing mode */
            const Project::EditingMode mode = project->editingMode();
            project->setActiveModel(0);
            m_model->clear();
            m_model->load(inflated, error);
            m_model->setupAllBindings();
            if (!project->isHiddenBoneBoundsRigidBodyDisabled()) {
                m_model->createAllBoneBoundsRigidBodies();
            }
            m_model->createAllImages();
            m_model->upload();
            m_model->loadAllImages(progress, error);
            if (Motion *motion = project->resolveMotion(m_model)) {
                motion->initialize(m_model);
            }
            m_model->synchronizeMotion(motion, frameIndex, 0, PhysicsEngine::kSimulationTimingBefore);
            m_model->setDirty(false);
            project->setActiveModel(m_model);
            project->setEditingMode(mode);
        }
    }
}

ModelSnapshotCommand::ModelSnapshotCommand(Project *project)
    : BaseUndoCommand(project)
    , m_model(0)
{
    m_snapshot.first.m_inflatedSize = m_snapshot.second.m_inflatedSize = 0;
}

ModelSnapshotCommand::ModelSnapshotCommand(Model *model, const ByteArray &snapshot)
    : BaseUndoCommand(model->project())
    , m_model(model)
{
    ByteArray current;
    Error error;
    model->save(current, error);
    m_snapshot.first.deflate(current);
    m_snapshot.second.deflate(snapshot);
    error.notify(currentProject()->eventPublisher());
}

} /* namespace command */
} /* namespace nanoem */
