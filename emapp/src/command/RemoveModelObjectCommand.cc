/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/RemoveModelObjectCommand.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace command {

RemoveModelObjectCommand::~RemoveModelObjectCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
RemoveModelObjectCommand::create(Model *model, Object *o)
{
    RemoveModelObjectCommand *command =
        nanoem_new(RemoveModelObjectCommand(model, nanoem_new(ExecutionBlock(o, model))));
    return command->createCommand();
}

void
RemoveModelObjectCommand::undo(Error &error)
{
    m_block->undo(error);
}

void
RemoveModelObjectCommand::redo(Error &error)
{
    m_block->redo(error);
}

void
RemoveModelObjectCommand::read(const void * /* messagePtr */)
{
}

void
RemoveModelObjectCommand::write(void * /* messagePtr */)
{
}

void
RemoveModelObjectCommand::release(void * /* messagePtr */)
{
}

const char *
RemoveModelObjectCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "RemoveModelObjectCommand";
}

RemoveModelObjectCommand::RemoveModelObjectCommand(Model *model, ExecutionBlock *block)
    : BaseModelObjectCommand(model, block)
{
}

} /* namespace command */
} /* namespace nanoem */
