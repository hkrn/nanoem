/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/UpdateModelObjectCommand.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace command {

UpdateModelObjectCommand::~UpdateModelObjectCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
UpdateModelObjectCommand::create(Model *model, Object *o, const tinystl::pair<bool, bool> &value)
{
    UpdateModelObjectCommand *command =
        nanoem_new(UpdateModelObjectCommand(model, nanoem_new(ExecutionBlock(o, model, value))));
    return command->createCommand();
}

undo_command_t *
UpdateModelObjectCommand::create(Model *model, Object *o, const tinystl::pair<int, int> &value)
{
    UpdateModelObjectCommand *command =
        nanoem_new(UpdateModelObjectCommand(model, nanoem_new(ExecutionBlock(o, model, value))));
    return command->createCommand();
}

undo_command_t *
UpdateModelObjectCommand::create(Model *model, Object *o, const tinystl::pair<nanoem_f32_t, nanoem_f32_t> &value)
{
    UpdateModelObjectCommand *command =
        nanoem_new(UpdateModelObjectCommand(model, nanoem_new(ExecutionBlock(o, model, value))));
    return command->createCommand();
}

undo_command_t *
UpdateModelObjectCommand::create(Model *model, Object *o, const tinystl::pair<void *, void *> &value,
    destructor_t destructor, void *context, nanoem_rsize_t index)
{
    UpdateModelObjectCommand *command = nanoem_new(
        UpdateModelObjectCommand(model, nanoem_new(ExecutionBlock(o, model, value, destructor, context, index))));
    return command->createCommand();
}

undo_command_t *
UpdateModelObjectCommand::create(
    Model *model, Object *o, const tinystl::pair<const void *, const void *> &value, nanoem_rsize_t index)
{
    UpdateModelObjectCommand *command =
        nanoem_new(UpdateModelObjectCommand(model, nanoem_new(ExecutionBlock(o, model, value, index))));
    return command->createCommand();
}

void
UpdateModelObjectCommand::undo(Error &error)
{
    m_block->undo(error);
}

void
UpdateModelObjectCommand::redo(Error &error)
{
    m_block->redo(error);
}

void
UpdateModelObjectCommand::read(const void * /* messagePtr */)
{
}

void
UpdateModelObjectCommand::write(void * /* messagePtr */)
{
}

void
UpdateModelObjectCommand::release(void * /* messagePtr */)
{
}

const char *
UpdateModelObjectCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "UpdateModelObjectCommand";
}

UpdateModelObjectCommand::UpdateModelObjectCommand(Model *model, ExecutionBlock *block)
    : BaseModelObjectCommand(model, block)
{
}

} /* namespace command */
} /* namespace nanoem */
