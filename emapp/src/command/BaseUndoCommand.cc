/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/BaseUndoCommand.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"
#include "emapp/Error.h"
#include "emapp/Project.h"
#include "emapp/private/CommonInclude.h"

#include "lz4/lib/lz4.h"
#include "sokol/sokol_time.h"
#include "undo/undo.h"

namespace nanoem {
namespace command {

void
BaseUndoCommand::LZ4Data::deflate(const ByteArray &input)
{
    deflate(input.data(), input.size());
}

void
BaseUndoCommand::LZ4Data::deflate(const nanoem_u8_t *data, size_t size)
{
    int inflatedSize = m_inflatedSize = Inline::saturateInt32(size);
    m_deflatedBytes.resize(LZ4_compressBound(inflatedSize));
    int deflatedSize =
        LZ4_compress_fast(reinterpret_cast<const char *>(data), reinterpret_cast<char *>(m_deflatedBytes.data()),
            inflatedSize, Inline::saturateInt32(m_deflatedBytes.capacity()), 1);
    if (deflatedSize > 0) {
        m_deflatedBytes.resize(deflatedSize);
        m_deflatedBytes.shrink_to_fit();
    }
}

bool
BaseUndoCommand::LZ4Data::inflate(ByteArray &output) const
{
    output.resize(m_inflatedSize);
    int deflatedSize = Inline::saturateInt32(m_deflatedBytes.size()),
        inflatedSize = LZ4_decompress_safe(reinterpret_cast<const char *>(m_deflatedBytes.data()),
            reinterpret_cast<char *>(output.data()), deflatedSize, Inline::saturateInt32(output.capacity()));
    return Inline::saturateInt32(output.size()) == inflatedSize;
}

bool
BaseUndoCommand::LZ4Data::inflate(ProtobufCBinaryData *binary) const
{
    ByteArray bytes;
    bool result = inflate(bytes);
    if (result) {
        binary->data = new nanoem_u8_t[bytes.size()];
        memcpy(binary->data, bytes.data(), bytes.size());
        binary->len = bytes.size();
    }
    return result;
}

BaseUndoCommand::BaseUndoCommand(Project *project)
    : m_project(project)
{
}

const Project *
BaseUndoCommand::currentProject() const NANOEM_DECL_NOEXCEPT
{
    return m_project;
}

Project *
BaseUndoCommand::currentProject()
{
    return m_project;
}

undo_command_t *
BaseUndoCommand::createCommand(bool enablePersistence)
{
    undo_command_t *command = undoCommandCreate();
    undoCommandSetOnDestroyCallback(command, BaseUndoCommand::onDestroy);
    undoCommandSetOnUndoCallback(command, BaseUndoCommand::onUndo);
    undoCommandSetOnRedoCallback(command, BaseUndoCommand::onRedo);
    undoCommandSetName(command, name());
    if (enablePersistence) {
        undoCommandSetOnPersistUndoCallback(command, BaseUndoCommand::onPersistUndo);
        undoCommandSetOnPersistRedoCallback(command, BaseUndoCommand::onPersistRedo);
    }
    undoCommandSetOpaqueData(command, this);
    return command;
}

void
BaseUndoCommand::assignError(nanoem_status_t status, Error &error)
{
    if (status != NANOEM_STATUS_SUCCESS) {
        const char *reason = Error::convertStatusToMessage(status, m_project->translator());
        error = Error(reason, status, Error::kDomainTypeNanoem);
    }
}

void
BaseUndoCommand::writeCommandMessage(void *opaque, nanoem_u32_t type, void *messagePtr)
{
    Nanoem__Application__Command *action = static_cast<Nanoem__Application__Command *>(messagePtr);
    action->timestamp = stm_now();
    action->undo = static_cast<Nanoem__Application__UndoCommand *>(opaque);
    action->type_case = static_cast<Nanoem__Application__Command__TypeCase>(type);
}

void
BaseUndoCommand::onUndo(const undo_command_t *command)
{
    Error error;
    IUndoCommand *commandPtr = static_cast<IUndoCommand *>(undoCommandGetOpaqueData(command));
    commandPtr->undo(error);
    if (error.hasReason()) {
        error.notify(commandPtr->currentProject()->eventPublisher());
    }
}

void
BaseUndoCommand::onRedo(const undo_command_t *command)
{
    Error error;
    IUndoCommand *commandPtr = static_cast<IUndoCommand *>(undoCommandGetOpaqueData(command));
    commandPtr->redo(error);
    if (error.hasReason()) {
        error.notify(commandPtr->currentProject()->eventPublisher());
    }
}

int
BaseUndoCommand::onPersistUndo(const undo_command_t *command)
{
    Error error;
    BaseUndoCommand *commandPtr = static_cast<BaseUndoCommand *>(undoCommandGetOpaqueData(command));
    Project *project = commandPtr->m_project;
    Nanoem__Application__UndoCommand undo = NANOEM__APPLICATION__UNDO_COMMAND__INIT;
    if (Model *model = project->activeModel()) {
        undo.has_model_handle = 1;
        undo.model_handle = model->handle();
    }
    Nanoem__Application__Command action = NANOEM__APPLICATION__COMMAND__INIT;
    writeCommandMessage(&undo, NANOEM__APPLICATION__COMMAND__TYPE_UNDO, &action);
    project->writeRedoMessage(&action, error);
    if (error.hasReason()) {
        error.notify(project->eventPublisher());
    }
    return 1;
}

int
BaseUndoCommand::onPersistRedo(const undo_command_t *command)
{
    Error error;
    BaseUndoCommand *commandPtr = static_cast<BaseUndoCommand *>(undoCommandGetOpaqueData(command));
    Nanoem__Application__Command action = NANOEM__APPLICATION__COMMAND__INIT;
    commandPtr->write(&action);
    Project *project = commandPtr->m_project;
    project->writeRedoMessage(&action, error);
    commandPtr->release(&action);
    if (error.hasReason()) {
        error.notify(project->eventPublisher());
    }
    return 1;
}

void
BaseUndoCommand::onDestroy(const undo_command_t *command)
{
    IUndoCommand *commandPtr = static_cast<IUndoCommand *>(undoCommandGetOpaqueData(command));
    nanoem_delete(commandPtr);
}

} /* namespace command */
} /* namespace nanoem */
