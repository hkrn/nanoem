/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_BASEUNDOCOMMAND_H_
#define NANOEM_EMAPP_COMMAND_BASEUNDOCOMMAND_H_

#include "emapp/Forward.h"
#include "emapp/command/IUndoCommand.h"

struct ProtobufCBinaryData;
struct undo_command_t;

namespace nanoem {

class Project;

namespace command {

class BaseUndoCommand : public IUndoCommand, private NonCopyable {
public:
    const Project *currentProject() const NANOEM_DECL_NOEXCEPT;
    Project *currentProject() NANOEM_DECL_OVERRIDE;

    virtual void read(const void *messagePtr) = 0;
    virtual void write(void *messagePtr) = 0;
    virtual void release(void *messagePtr) = 0;

protected:
    struct LZ4Data {
        ByteArray m_deflatedBytes;
        int m_inflatedSize;
        void deflate(const ByteArray &input);
        void deflate(const nanoem_u8_t *data, size_t size);
        bool inflate(ByteArray &output) const;
        bool inflate(ProtobufCBinaryData *binary) const;
    };
    BaseUndoCommand(Project *project);

    undo_command_t *createCommand(bool enablePersistence = true);
    void assignError(nanoem_status_t status, Error &error);

    static void writeCommandMessage(void *opaque, nanoem_u32_t type, void *messagePtr);

private:
    static void onUndo(const undo_command_t *command);
    static void onRedo(const undo_command_t *command);
    static int onPersistUndo(const undo_command_t *command);
    static int onPersistRedo(const undo_command_t *command);
    static void onDestroy(const undo_command_t *command);

    Project *m_project;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_BASEUNDOCOMMAND_H_ */
