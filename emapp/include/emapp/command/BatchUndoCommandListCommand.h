/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_BATCHUNDOCOMMANDLISTCOMMAND_H_
#define NANOEM_EMAPP_COMMAND_BATCHUNDOCOMMANDLISTCOMMAND_H_

#include "emapp/command/BaseUndoCommand.h"

namespace nanoem {

class Model;

namespace command {

class BatchUndoCommandListCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    typedef tinystl::vector<undo_command_t *, TinySTLAllocator> UndoCommandList;

    ~BatchUndoCommandListCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(const void *messagePtr, const Model *model, Project *project);
    static undo_command_t *create(const UndoCommandList &commands, const Model *model, Project *project);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    BatchUndoCommandListCommand(const UndoCommandList &commands, const Model *model, Project *project);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);

    const Model *m_model;
    UndoCommandList m_commands;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_BATCHUNDOCOMMANDLISTCOMMAND_H_ */
