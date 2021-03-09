/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_MOTIONSNAPSHOTCOMMAND_H_
#define NANOEM_EMAPP_COMMAND_MOTIONSNAPSHOTCOMMAND_H_

#include "emapp/command/BaseUndoCommand.h"

#include "emapp/Motion.h"

namespace nanoem {
namespace command {

class MotionSnapshotCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    ~MotionSnapshotCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(const void *messagePtr, Project *project);
    static undo_command_t *create(Motion *motion, const Model *model, const ByteArray &snapshot, nanoem_u32_t types);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    void execute(const LZ4Data &input, Error &error);
    MotionSnapshotCommand(Project *project);
    MotionSnapshotCommand(Motion *motion, const Model *model, const ByteArray &snapshot, nanoem_u32_t types);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);

    Motion *m_motion;
    Motion::SelectionState *m_state;
    tinystl::pair<LZ4Data, LZ4Data> m_snapshot;
    nanoem_u32_t m_types;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_MOTIONSNAPSHOTCOMMAND_H_ */
