/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/UpdateAccessoryCommand.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"

#include "emapp/Accessory.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace command {

UpdateAccessoryCommand::~UpdateAccessoryCommand() NANOEM_DECL_NOEXCEPT
{
    m_accessory = 0;
}

undo_command_t *
UpdateAccessoryCommand::create(const void *messagePtr, Accessory *accessory)
{
    UpdateAccessoryCommand *command =
        nanoem_new(UpdateAccessoryCommand(accessory, Constants::kZeroV3, Constants::kZeroV3, 0, 0));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
UpdateAccessoryCommand::create(
    Accessory *accessory, const Vector3 &lookAt, const Vector3 &angle, nanoem_f32_t distance, nanoem_f32_t fov)
{
    UpdateAccessoryCommand *command = nanoem_new(UpdateAccessoryCommand(accessory, lookAt, angle, distance, fov));
    return command->createCommand();
}

void
UpdateAccessoryCommand::undo(Error &error)
{
    BX_UNUSED_1(error);
    m_accessory->setTranslation(m_translation.second);
    m_accessory->setOrientation(m_orientation.second);
    m_accessory->setOpacity(m_opacity.second);
    m_accessory->setScaleFactor(m_scaleFactor.second);
}

void
UpdateAccessoryCommand::redo(Error &error)
{
    BX_UNUSED_1(error);
    m_accessory->setTranslation(m_translation.first);
    m_accessory->setOrientation(m_orientation.first);
    m_accessory->setOpacity(m_opacity.first);
    m_accessory->setScaleFactor(m_scaleFactor.first);
}

void
UpdateAccessoryCommand::read(const void *messagePtr)
{
    const Nanoem__Application__RedoUpdateAccessoryCommand *command =
        static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_update_accessory;
    const Nanoem__Application__RedoUpdateAccessoryCommand__State *current = command->current_state;
    CommandMessageUtil::getVector(current->translation, m_translation.first);
    CommandMessageUtil::getVector(current->orientation, m_orientation.first);
    m_opacity.first = current->opacity;
    m_scaleFactor.first = current->scale_factor;
    const Nanoem__Application__RedoUpdateAccessoryCommand__State *last = command->last_state;
    CommandMessageUtil::getVector(last->translation, m_translation.second);
    CommandMessageUtil::getVector(last->orientation, m_orientation.second);
    m_opacity.second = last->opacity;
    m_scaleFactor.second = last->scale_factor;
}

void
UpdateAccessoryCommand::write(void *messagePtr)
{
    Nanoem__Application__RedoUpdateAccessoryCommand *command =
        nanoem_new(Nanoem__Application__RedoUpdateAccessoryCommand);
    nanoem__application__redo_update_accessory_command__init(command);
    Nanoem__Application__RedoUpdateAccessoryCommand__State *current = command->current_state =
        nanoem_new(Nanoem__Application__RedoUpdateAccessoryCommand__State);
    nanoem__application__redo_update_accessory_command__state__init(current);
    CommandMessageUtil::setVector(m_translation.first, current->translation);
    CommandMessageUtil::setVector(m_orientation.first, current->orientation);
    current->opacity = m_opacity.first;
    current->scale_factor = m_scaleFactor.first;
    Nanoem__Application__RedoUpdateAccessoryCommand__State *last = command->last_state =
        nanoem_new(Nanoem__Application__RedoUpdateAccessoryCommand__State);
    nanoem__application__redo_update_accessory_command__state__init(last);
    CommandMessageUtil::setVector(m_translation.second, last->translation);
    CommandMessageUtil::setVector(m_orientation.second, last->orientation);
    last->opacity = m_opacity.second;
    last->scale_factor = m_scaleFactor.second;
    writeCommandMessage(command, NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_ACCESSORY, messagePtr);
}

void
UpdateAccessoryCommand::release(void *messagePtr)
{
    Nanoem__Application__RedoUpdateAccessoryCommand *command =
        static_cast<Nanoem__Application__Command *>(messagePtr)->redo_update_accessory;
    Nanoem__Application__RedoUpdateAccessoryCommand__State *current = command->current_state;
    CommandMessageUtil::releaseVector(current->translation);
    CommandMessageUtil::releaseVector(current->orientation);
    nanoem_delete(current);
    Nanoem__Application__RedoUpdateAccessoryCommand__State *last = command->last_state;
    CommandMessageUtil::releaseVector(last->translation);
    CommandMessageUtil::releaseVector(last->orientation);
    nanoem_delete(last);
    nanoem_delete(command);
}

const char *
UpdateAccessoryCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "UpdateAccessoryCommand";
}

UpdateAccessoryCommand::UpdateAccessoryCommand(Accessory *accessory, const Vector3 &translation,
    const Vector3 &orientation, nanoem_f32_t opacity, nanoem_f32_t scaleFactor)
    : BaseUndoCommand(accessory->project())
    , m_accessory(accessory)
    , m_translation(tinystl::make_pair(accessory->translation(), translation))
    , m_orientation(tinystl::make_pair(accessory->orientation(), orientation))
    , m_opacity(tinystl::make_pair(accessory->opacity(), opacity))
    , m_scaleFactor(tinystl::make_pair(accessory->scaleFactor(), scaleFactor))
{
}

} /* namespace command */
} /* namespace nanoem */
