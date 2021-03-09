/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/MotionSnapshotCommand.h"

#include "../CommandMessage.inl"

#include "emapp/Accessory.h"
#include "emapp/EnumUtils.h"
#include "emapp/Error.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace command {

MotionSnapshotCommand::~MotionSnapshotCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
MotionSnapshotCommand::create(const void *messagePtr, Project *project)
{
    MotionSnapshotCommand *command = nanoem_new(MotionSnapshotCommand(project));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
MotionSnapshotCommand::create(Motion *motion, const Model *model, const ByteArray &snapshot, nanoem_u32_t types)
{
    MotionSnapshotCommand *command = nanoem_new(MotionSnapshotCommand(motion, model, snapshot, types));
    return command->createCommand();
}

void
MotionSnapshotCommand::undo(Error &error)
{
    execute(m_snapshot.second, error);
}

void
MotionSnapshotCommand::redo(Error &error)
{
    execute(m_snapshot.first, error);
}

void
MotionSnapshotCommand::read(const void *messagePtr)
{
    if (const Nanoem__Application__RedoSaveMotionSnapshotCommand *command =
            static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_save_motion_snapshot) {
        Project *project = currentProject();
        m_snapshot.first.deflate(command->current_motion.data, command->current_motion.len);
        m_snapshot.second.deflate(command->last_motion.data, command->last_motion.len);
        if (command->has_handle) {
            nanoem_u16_t handle = command->handle;
            if (Accessory *accessory = project->resolveRedoAccessory(handle)) {
                m_motion = project->resolveMotion(accessory);
            }
            else if (Model *model = project->resolveRedoModel(handle)) {
                m_motion = project->resolveMotion(model);
            }
        }
        m_types = command->types;
        if (!m_motion) {
            if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA, m_types)) {
                m_motion = project->cameraMotion();
            }
            else if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT, m_types)) {
                m_motion = project->lightMotion();
            }
            else if (EnumUtils::isEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW, m_types)) {
                m_motion = project->selfShadowMotion();
            }
        }
    }
}

void
MotionSnapshotCommand::write(void *messagePtr)
{
    Nanoem__Application__RedoSaveMotionSnapshotCommand *command =
        nanoem_new(Nanoem__Application__RedoSaveMotionSnapshotCommand);
    nanoem__application__redo_save_motion_snapshot_command__init(command);
    m_snapshot.first.inflate(&command->current_motion);
    m_snapshot.second.inflate(&command->last_motion);
    command->types = m_types;
    if (const IDrawable *drawable = currentProject()->resolveDrawable(m_motion)) {
        command->handle = drawable->handle();
        command->has_handle = 1;
    }
    writeCommandMessage(command, NANOEM__APPLICATION__COMMAND__TYPE_REDO_SAVE_MOTION_SNAPSHOT, messagePtr);
}

void
MotionSnapshotCommand::release(void *messagePtr)
{
    Nanoem__Application__RedoSaveMotionSnapshotCommand *command =
        static_cast<Nanoem__Application__Command *>(messagePtr)->redo_save_motion_snapshot;
    delete[] command->current_motion.data;
    delete[] command->last_motion.data;
    nanoem_delete(command);
    if (m_state) {
        m_motion->destroyState(m_state);
        m_state = nullptr;
    }
}

const char *
MotionSnapshotCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "MotionSnapshotCommand";
}

void
MotionSnapshotCommand::execute(const LZ4Data &input, Error &error)
{
    Project *project = currentProject();
    if (project->containsMotion(m_motion)) {
        ByteArray inflated;
        if (input.inflate(inflated)) {
            m_motion->clearAllKeyframes();
            m_motion->load(inflated, 0, error);
            m_motion->restoreState(m_state);
            project->setBaseDuration(m_motion->duration());
        }
    }
}

MotionSnapshotCommand::MotionSnapshotCommand(Project *project)
    : BaseUndoCommand(project)
    , m_motion(0)
    , m_state(0)
    , m_types(0)
{
    m_snapshot.first.m_inflatedSize = m_snapshot.second.m_inflatedSize = 0;
}

MotionSnapshotCommand::MotionSnapshotCommand(
    Motion *motion, const Model *model, const ByteArray &snapshot, nanoem_u32_t types)
    : BaseUndoCommand(motion->project())
    , m_motion(motion)
    , m_state(0)
    , m_types(types)
{
    ByteArray current;
    Error error;
    motion->save(current, model, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
    motion->saveState(m_state);
    m_snapshot.first.deflate(current);
    m_snapshot.second.deflate(snapshot);
    error.notify(currentProject()->eventPublisher());
}

} /* namespace command */
} /* namespace nanoem */
