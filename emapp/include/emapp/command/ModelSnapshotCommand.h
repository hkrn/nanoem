/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_MODELSNAPSHOTCOMMAND_H_
#define NANOEM_EMAPP_COMMAND_MODELSNAPSHOTCOMMAND_H_

#include "emapp/command/BaseUndoCommand.h"

#include "emapp/Model.h"

namespace nanoem {
namespace command {

class ModelSnapshotCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    ~ModelSnapshotCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(const void *messagePtr, Project *project);
    static undo_command_t *create(Model *model, const ByteArray &snapshot);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    void execute(const LZ4Data &input, Error &error);

    ModelSnapshotCommand(Project *project);
    ModelSnapshotCommand(Model *model, const ByteArray &snapshot);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);

    Model *m_model;
    tinystl::pair<LZ4Data, LZ4Data> m_snapshot;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_MOTIONSNAPSHOTCOMMAND_H_ */
