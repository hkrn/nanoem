/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/UpdateCameraCommand.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"
#include "emapp/ICamera.h"
#include "emapp/private/CommonInclude.h"

#include "undo/undo.h"

namespace nanoem {
namespace command {

UpdateCameraCommand::~UpdateCameraCommand() NANOEM_DECL_NOEXCEPT
{
    m_camera = 0;
}

undo_command_t *
UpdateCameraCommand::create(void *messagePtr, ICamera *camera, Project *project)
{
    UpdateCameraCommand *command =
        nanoem_new(UpdateCameraCommand(project, camera, Constants::kZeroV3, Constants::kZeroV3, 0, 0, false));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
UpdateCameraCommand::create(Project *project, ICamera *camera, const Vector3 &lookAt, const Vector3 &angle,
    nanoem_f32_t distance, nanoem_f32_t fov, bool isPerspective)
{
    nanoem_parameter_assert(project->globalCamera() == camera, "must be global camera");
    UpdateCameraCommand *command =
        nanoem_new(UpdateCameraCommand(project, camera, lookAt, angle, distance, fov, isPerspective));
    return command->createCommand();
}

undo_command_t *
UpdateCameraCommand::create(Project *project, ICamera *camera, const ICamera &source)
{
    return create(project, camera, source.lookAt(), source.angle(), source.distance(), source.fovRadians(),
        source.isPerspective());
}

void
UpdateCameraCommand::undo(Error &error)
{
    BX_UNUSED_1(error);
    Project *project = currentProject();
    project->resetTransformPerformedAt();
    project->seek(m_localFrameIndex, true);
    m_camera->setLookAt(m_lookAt.second);
    m_camera->setAngle(m_angle.second);
    m_camera->setDistance(m_distance.second);
    m_camera->setFovRadians(m_fov.second);
    m_camera->setPerspective(m_perspective.second);
    m_camera->update();
    project->resetAllModelEdges();
}

void
UpdateCameraCommand::redo(Error &error)
{
    BX_UNUSED_1(error);
    Project *project = currentProject();
    project->resetTransformPerformedAt();
    project->seek(m_localFrameIndex, true);
    project->updateTransformPerformedAt(
        tinystl::make_pair(m_localFrameIndex, undoStackGetOffset(currentProject()->undoStack())));
    m_camera->setLookAt(m_lookAt.first);
    m_camera->setAngle(m_angle.first);
    m_camera->setDistance(m_distance.first);
    m_camera->setFovRadians(m_fov.first);
    m_camera->setPerspective(m_perspective.first);
    m_camera->update();
    project->resetAllModelEdges();
}

const char *
UpdateCameraCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "UpdateCameraCommand";
}

UpdateCameraCommand::UpdateCameraCommand(Project *project, ICamera *camera, const Vector3 &lookAt, const Vector3 &angle,
    nanoem_f32_t distance, nanoem_f32_t fov, bool isPerspective)
    : BaseUndoCommand(project)
    , m_localFrameIndex(project->currentLocalFrameIndex())
    , m_camera(camera)
    , m_lookAt(tinystl::make_pair(camera->lookAt(), lookAt))
    , m_angle(tinystl::make_pair(camera->angle(), angle))
    , m_distance(tinystl::make_pair(camera->distance(), distance))
    , m_fov(tinystl::make_pair(camera->fovRadians(), fov))
    , m_perspective(tinystl::make_pair(camera->isPerspective(), isPerspective))
{
}

void
UpdateCameraCommand::read(const void *messagePtr)
{
    const Nanoem__Application__RedoUpdateCameraCommand *command =
        static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_update_camera;
    const Nanoem__Application__RedoUpdateCameraCommand__State *current = command->current_state;
    CommandMessageUtil::getVector(current->look_at, m_lookAt.first);
    CommandMessageUtil::getVector(current->angle, m_angle.first);
    m_fov.first = current->fov;
    m_distance.first = current->distance;
    m_perspective.first = current->perspective != 0;
    const Nanoem__Application__RedoUpdateCameraCommand__State *last = command->last_state;
    CommandMessageUtil::getVector(last->look_at, m_lookAt.second);
    CommandMessageUtil::getVector(last->angle, m_angle.second);
    m_fov.second = last->fov;
    m_distance.second = last->distance;
    m_perspective.second = last->perspective != 0;
}

void
UpdateCameraCommand::write(void *messagePtr)
{
    Nanoem__Application__RedoUpdateCameraCommand *command = nanoem_new(Nanoem__Application__RedoUpdateCameraCommand);
    nanoem__application__redo_update_camera_command__init(command);
    Nanoem__Application__RedoUpdateCameraCommand__State *current = command->current_state =
        nanoem_new(Nanoem__Application__RedoUpdateCameraCommand__State);
    nanoem__application__redo_update_camera_command__state__init(current);
    CommandMessageUtil::setVector(m_angle.first, current->angle);
    CommandMessageUtil::setVector(m_lookAt.first, current->look_at);
    current->distance = m_distance.first;
    current->fov = m_fov.first;
    Nanoem__Application__RedoUpdateCameraCommand__State *last = command->last_state =
        nanoem_new(Nanoem__Application__RedoUpdateCameraCommand__State);
    nanoem__application__redo_update_camera_command__state__init(last);
    CommandMessageUtil::setVector(m_angle.second, last->angle);
    CommandMessageUtil::setVector(m_lookAt.second, last->look_at);
    last->distance = m_distance.second;
    last->fov = m_fov.second;
    last->perspective = m_perspective.second;
    writeCommandMessage(command, NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_CAMERA, messagePtr);
}

void
UpdateCameraCommand::release(void *messagePtr)
{
    Nanoem__Application__RedoUpdateCameraCommand *command =
        static_cast<Nanoem__Application__Command *>(messagePtr)->redo_update_camera;
    Nanoem__Application__RedoUpdateCameraCommand__State *current = command->current_state;
    CommandMessageUtil::releaseVector(current->angle);
    CommandMessageUtil::releaseVector(current->look_at);
    nanoem_delete(current);
    Nanoem__Application__RedoUpdateCameraCommand__State *last = command->last_state;
    CommandMessageUtil::releaseVector(last->angle);
    CommandMessageUtil::releaseVector(last->look_at);
    nanoem_delete(last);
    nanoem_delete(command);
}

} /* namespace command */
} /* namespace nanoem */
