/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/UpdateLightCommand.h"

#include "emapp/Constants.h"
#include "emapp/ILight.h"
#include "emapp/private/CommonInclude.h"

#include "../CommandMessage.inl"

#include "undo/undo.h"

namespace nanoem {
namespace command {

UpdateLightCommand::~UpdateLightCommand() NANOEM_DECL_NOEXCEPT
{
    m_light = 0;
}

undo_command_t *
UpdateLightCommand::create(const void *messagePtr, ILight *light, Project *project)
{
    UpdateLightCommand *command =
        nanoem_new(UpdateLightCommand(project, light, Constants::kZeroV3, Constants::kZeroV3));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
UpdateLightCommand::create(Project *project, ILight *light, const Vector3 &color, const Vector3 &direction)
{
    nanoem_parameter_assert(project->globalLight() == light, "must be global light");
    UpdateLightCommand *command = nanoem_new(UpdateLightCommand(project, light, color, direction));
    return command->createCommand();
}

undo_command_t *
UpdateLightCommand::create(Project *project, ILight *light, const ILight &source)
{
    return create(project, light, source.color(), source.direction());
}

void
UpdateLightCommand::undo(Error &error)
{
    BX_UNUSED_1(error);
    Project *project = currentProject();
    project->resetTransformPerformedAt();
    project->seek(m_localFrameIndex, true);
    m_light->setColor(m_color.second);
    m_light->setDirection(m_direction.second);
}

void
UpdateLightCommand::redo(Error &error)
{
    BX_UNUSED_1(error);
    Project *project = currentProject();
    project->resetTransformPerformedAt();
    project->seek(m_localFrameIndex, true);
    project->updateTransformPerformedAt(
        tinystl::make_pair(m_localFrameIndex, undoStackGetOffset(currentProject()->undoStack())));
    m_light->setColor(m_color.first);
    m_light->setDirection(m_direction.first);
}

void
UpdateLightCommand::read(const void *messagePtr)
{
    const Nanoem__Application__RedoUpdateLightCommand *command =
        static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_update_light;
    const Nanoem__Application__RedoUpdateLightCommand__State *current = command->current_state;
    CommandMessageUtil::getColor(current->color, m_color.first);
    CommandMessageUtil::getVector(current->direction, m_direction.first);
    const Nanoem__Application__RedoUpdateLightCommand__State *last = command->last_state;
    CommandMessageUtil::getColor(last->color, m_color.second);
    CommandMessageUtil::getVector(last->direction, m_direction.second);
}

void
UpdateLightCommand::write(void *messagePtr)
{
    Nanoem__Application__RedoUpdateLightCommand *command = nanoem_new(Nanoem__Application__RedoUpdateLightCommand);
    nanoem__application__redo_update_light_command__init(command);
    Nanoem__Application__RedoUpdateLightCommand__State *current = command->current_state =
        nanoem_new(Nanoem__Application__RedoUpdateLightCommand__State);
    nanoem__application__redo_update_light_command__state__init(current);
    CommandMessageUtil::setColor(Vector4(m_color.first, 1), current->color);
    CommandMessageUtil::setVector(m_direction.first, current->direction);
    Nanoem__Application__RedoUpdateLightCommand__State *last = command->last_state =
        nanoem_new(Nanoem__Application__RedoUpdateLightCommand__State);
    nanoem__application__redo_update_light_command__state__init(last);
    CommandMessageUtil::setColor(Vector4(m_color.first, 1), last->color);
    CommandMessageUtil::setVector(m_direction.first, last->direction);
    writeCommandMessage(command, NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_LIGHT, messagePtr);
}

void
UpdateLightCommand::release(void *messagePtr)
{
    Nanoem__Application__RedoUpdateLightCommand *command =
        static_cast<Nanoem__Application__Command *>(messagePtr)->redo_update_light;
    Nanoem__Application__RedoUpdateLightCommand__State *current = command->current_state;
    CommandMessageUtil::releaseColor(current->color);
    CommandMessageUtil::releaseVector(current->direction);
    nanoem_delete(current);
    Nanoem__Application__RedoUpdateLightCommand__State *last = command->last_state;
    CommandMessageUtil::releaseColor(last->color);
    CommandMessageUtil::releaseVector(last->direction);
    nanoem_delete(last);
    nanoem_delete(command);
}

const char *
UpdateLightCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "UpdateLightCommand";
}

UpdateLightCommand::UpdateLightCommand(Project *project, ILight *light, const Vector3 &color, const Vector3 &direction)
    : BaseUndoCommand(project)
    , m_localFrameIndex(project->currentLocalFrameIndex())
    , m_light(light)
    , m_color(tinystl::make_pair(light->color(), color))
    , m_direction(tinystl::make_pair(light->direction(), direction))
{
}

} /* namespace command */
} /* namespace nanoem */
