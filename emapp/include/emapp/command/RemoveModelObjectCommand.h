/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_REMOVEMODELOBJECTCOMMAND_H_
#define NANOEM_EMAPP_COMMAND_REMOVEMODELOBJECTCOMMAND_H_

#include "emapp/command/BaseModelObjectCommand.h"

namespace nanoem {
namespace command {

class RemoveModelObjectCommand NANOEM_DECL_SEALED : public BaseModelObjectCommand {
public:
    ~RemoveModelObjectCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(Model *model, Object *o);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    RemoveModelObjectCommand(Model *model, ExecutionBlock *block);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_REMOVEMODELOBJECTCOMMAND_H_ */
