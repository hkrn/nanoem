/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/internal/project/Redo.h"

#if defined(NANOEM_ENABLE_NANOMSG)

#include "../../protoc/application.pb-c.h"
#include "emapp/BaseApplicationService.h"
#include "emapp/Error.h"
#include "emapp/IEventPublisher.h"
#include "emapp/IFileManager.h"
#include "emapp/IModalDialog.h"
#include "emapp/Project.h"
#include "emapp/ThreadedApplicationService.h"
#include "emapp/private/CommonInclude.h"

#include "bx/readerwriter.h"
#include "lz4/lib/lz4.h"
#include "sokol/sokol_time.h"

#define NN_STATIC_LIB
#include "nanomsg/nn.h"
#include "nanomsg/pubsub.h"

namespace nanoem {
namespace internal {
namespace project {

const nanoem_u32_t Redo::kFileMagic = nanoem_fourcc('n', 'm', 'C', 'S');

Redo::Redo(Project *project)
    : m_project(project)
{
}

Redo::~Redo() NANOEM_DECL_NOEXCEPT
{
}

void
Redo::loadAll(ISeekableReader *reader, BaseApplicationService *application, Error &error)
{
    nanoem_u32_t sig;
    nanoem_u16_t commandType;
    if (FileUtils::readTyped(reader, sig, error) && sig == kFileMagic) {
        nanoem_u32_t offset = 0, count = 0, lastSequnce = 0, deflatedCommandSize, sequence;
        getCountOffset(reader, count, offset, error);
        m_project->setWritingRedoMessageDisabled(true);
        ByteArray deflated, inflated;
        while (FileUtils::readTyped(reader, sequence, error) == sizeof(sequence)) {
            FileUtils::readTyped(reader, commandType, error);
            FileUtils::readTyped(reader, deflatedCommandSize, error);
            if (isAccepted(commandType)) {
                deflated.resize(deflatedCommandSize);
                FileUtils::read(reader, deflated.data(), Inline::saturateInt32(deflated.size()), error);
                if (offset <= sequence && (lastSequnce == 0 || lastSequnce < sequence)) {
                    inflateChunk(deflated, inflated);
                    application->dispatchCommandMessage(inflated.data(), inflated.size(), m_project,
                        commandType == NANOEM__APPLICATION__COMMAND__TYPE_UNDO);
                }
            }
            else {
                reader->seek(deflatedCommandSize, ISeekable::kSeekTypeCurrent, error);
            }
            lastSequnce = sequence;
        }
        m_project->setWritingRedoMessageDisabled(false);
    }
}

void
Redo::loadAllAsync(ISeekableReader *reader, IModalDialog *dialog, const volatile bool *cancelled, Error &error)
{
    int commandStreamSocket = nn_socket(AF_SP, NN_PUB);
    int eventStreamSocket = nn_socket(AF_SP, NN_SUB);
    nn_connect(commandStreamSocket, ThreadedApplicationService::kCommandStreamURI);
    nn_connect(eventStreamSocket, ThreadedApplicationService::kEventStreamURI);
    nn_setsockopt(eventStreamSocket, NN_SUB, NN_SUB_SUBSCRIBE, "", 0);
    nanoem_u32_t sig;
    nanoem_u16_t commandType;
    if (FileUtils::readTyped(reader, sig, error) && sig == kFileMagic) {
        nanoem_u32_t offset = 0, count = 0, lastSequnce = 0, deflatedCommandSize, sequence;
        getCountOffset(reader, count, offset, error);
        dialog->setProgress(0);
        m_project->setWritingRedoMessageDisabled(true);
        const URI &fileURI = m_project->redoFileURI();
        IFileWriter *writer = nullptr;
        if (!fileURI.isEmpty()) {
#ifdef NDEBUG
            writer = FileUtils::createFileWriter();
            if (writer->open(fileURI, false, error)) {
                FileUtils::writeTyped(writer, kFileMagic, error);
            }
#endif
        }
        ByteArray deflated, inflated;
        while (!*cancelled && FileUtils::readTyped(reader, sequence, error) == sizeof(sequence)) {
            FileUtils::readTyped(reader, commandType, error);
            FileUtils::readTyped(reader, deflatedCommandSize, error);
            if (isAccepted(commandType)) {
                deflated.resize(deflatedCommandSize);
                FileUtils::read(reader, deflated.data(), Inline::saturateInt32(deflated.size()), error);
                if (offset <= sequence && (lastSequnce == 0 || lastSequnce < sequence)) {
                    inflateChunk(deflated, inflated);
                    sendCommandMessage(commandStreamSocket, inflated, commandType, writer, sequence);
                    waitEventMessage(commandStreamSocket, eventStreamSocket, cancelled);
                    dialog->setProgress(nanoem_f32_t((sequence - offset) / nanoem_f64_t(count - offset)));
                }
            }
            else {
                reader->seek(deflatedCommandSize, ISeekable::kSeekTypeCurrent, error);
            }
            lastSequnce = sequence;
        }
        if (writer) {
            Error error;
            if (*cancelled) {
                writer->rollback(error);
            }
            else {
                writer->commit(error);
            }
            FileUtils::destroyFileWriter(writer);
        }
        m_project->setWritingRedoMessageDisabled(false);
    }
    nn_close(commandStreamSocket);
    nn_close(eventStreamSocket);
}

bool
Redo::save(IWriter *writer, nanoem_u32_t sequence, const Nanoem__Application__Command *command, Error &error)
{
    bool saved = false;
    if (!saved) {
        ByteArray bytes;
        bytes.resize(nanoem__application__command__get_packed_size(command));
        nanoem__application__command__pack(command, bytes.data());
        if (sequence == 0) {
            FileUtils::writeTyped(writer, kFileMagic, error);
        }
        ByteArray compressed;
        compressed.reserve(LZ4_compressBound(Inline::saturateInt32(bytes.size())));
        int compressedSize =
            LZ4_compress_fast(reinterpret_cast<const char *>(bytes.data()), reinterpret_cast<char *>(compressed.data()),
                Inline::saturateInt32(bytes.size()), Inline::saturateInt32(compressed.capacity()), 1);
        if (compressedSize > 0) {
            bytes.clear();
            const nanoem_u16_t type = nanoem_u16_t(command->type_case);
            MemoryWriter memoryWriter(&bytes);
            FileUtils::writeTyped(&memoryWriter, sequence, error);
            FileUtils::writeTyped(&memoryWriter, type, error);
            FileUtils::writeTyped(&memoryWriter, compressedSize, error);
            FileUtils::write(&memoryWriter, compressed.data(), compressedSize, error);
            FileUtils::write(writer, bytes, error);
        }
        else {
            const nanoem_u16_t type = 0;
            const nanoem_u32_t size = 0;
            FileUtils::writeTyped(writer, sequence, error);
            FileUtils::writeTyped(writer, type, error);
            FileUtils::writeTyped(writer, size, error);
        }
        saved = !error.hasReason();
    }
    return saved;
}

bool
Redo::isAccepted(nanoem_u32_t value) NANOEM_DECL_NOEXCEPT
{
    switch (static_cast<Nanoem__Application__Command__TypeCase>(value)) {
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_ACCESSORY_KEYFRAME:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_BONE_KEYFRAME:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_CAMERA_KEYFRAME:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_LIGHT_KEYFRAME:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_MODEL_KEYFRAME:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_MORPH_KEYFRAME:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_SELF_SHADOW_KEYFRAME:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_BATCH_UNDO_COMMAND_LIST:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_DELETE_ACCESSORY:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_DELETE_MODEL:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_LOAD_ACCESSORY:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_LOAD_AUDIO:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_LOAD_MODEL:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_LOAD_MOTION:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_LOAD_VIDEO:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_ACCESSORY_KEYFRAME:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_BONE_KEYFRAME:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_CAMERA_KEYFRAME:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_LIGHT_KEYFRAME:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_MODEL_KEYFRAME:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_MORPH_KEYFRAME:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_SELF_SHADOW_KEYFRAME:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_SAVE_MOTION_SNAPSHOT:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_INSERT_EMPTY_TIMELINE_FRAME:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_TIMELINE_FRAME:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_TRANSFORM_BONE:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_TRANSFORM_MORPH:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_ACCESSORY:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_CAMERA:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_LIGHT:
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_ACCESSORY_OUTSIDE_PARENT:
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_ACTIVE_ACCESSORY:
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_ACTIVE_MODEL:
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_ACTIVE_MODEL_BONE:
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_ACTIVE_MODEL_MORPH:
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_EFFECT_PLUGIN_ENABLED:
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_CONSTRAINT_STATE:
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_EDGE_COLOR:
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_EDGE_SIZE:
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_OUTSIDE_PARENT:
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_VISIBLE:
    case NANOEM__APPLICATION__COMMAND__TYPE_SAVE_POINT:
    case NANOEM__APPLICATION__COMMAND__TYPE_UNDO: {
        return true;
    }
    default:
        return false;
    }
}

void
Redo::getCountOffset(ISeekableReader *reader, nanoem_u32_t &count, nanoem_u32_t &offset, Error &error)
{
    nanoem_u32_t deflatedCommandSize, sequence;
    nanoem_u16_t commandType;
    while (FileUtils::readTyped(reader, sequence, error) == sizeof(sequence)) {
        FileUtils::readTyped(reader, commandType, error);
        if (commandType == NANOEM__APPLICATION__COMMAND__TYPE_SAVE_POINT) {
            offset = count;
        }
        FileUtils::readTyped(reader, deflatedCommandSize, error);
        reader->seek(deflatedCommandSize, ISeekable::kSeekTypeCurrent, error);
        count++;
    }
    reader->seek(4, ISeekable::kSeekTypeBegin, error);
}

void
Redo::inflateChunk(const ByteArray &input, ByteArray &output)
{
    m_interm.reserve(input.size() * 100);
    int decompressedSize =
        LZ4_decompress_safe(reinterpret_cast<const char *>(input.data()), reinterpret_cast<char *>(m_interm.data()),
            Inline::saturateInt32(input.size()), Inline::saturateInt32(m_interm.capacity()));
    output.assign(m_interm.data(), m_interm.data() + decompressedSize);
}

void
Redo::sendCommandMessage(int commandStreamSocket, const ByteArray &inflated, nanoem_u16_t commandType, IWriter *writer,
    nanoem_u32_t sequence)
{
    if (Nanoem__Application__Command *command =
            nanoem__application__command__unpack(g_protobufc_allocator, inflated.size(), inflated.data())) {
        if (command->type_case == NANOEM__APPLICATION__COMMAND__TYPE__NOT_SET &&
            commandType == NANOEM__APPLICATION__COMMAND__TYPE_UNDO) {
            command->type_case = NANOEM__APPLICATION__COMMAND__TYPE_UNDO;
        }
        if (command->type_case == commandType) {
            void *msg = nn_allocmsg(inflated.size(), 0);
            memcpy(msg, inflated.data(), inflated.size());
            if (nn_send(commandStreamSocket, &msg, NN_MSG, 0) < 0) {
                // data->m_self->handleSocketError("nn_send");
            }
            else if (writer) {
                Error error;
                save(writer, sequence, command, error);
            }
        }
        nanoem__application__command__free_unpacked(command, g_protobufc_allocator);
    }
}

void
Redo::waitEventMessage(int commandStreamSocket, int eventStreamSocket, const volatile bool *cancelled)
{
    Nanoem__Application__PingPongCommand base = NANOEM__APPLICATION__PING_PONG_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = stm_now();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_PING_PONG;
    command.ping_pong = &base;
    nanoem_u8_t ppData[128];
    size_t ppSize = nanoem__application__command__get_packed_size(&command);
    nanoem__application__command__pack(&command, ppData);
    if (nn_send(commandStreamSocket, ppData, ppSize, 0) < 0) {
        // data->m_self->handleSocketError("nn_send");
    }
    while (!*cancelled) {
        nanoem_u8_t *body = nullptr;
        struct nn_iovec iov = { &body, NN_MSG };
        void *control = nullptr;
        struct nn_msghdr hdr = { &iov, 1, &control, NN_MSG };
        int rc = nn_recvmsg(eventStreamSocket, &hdr, NN_DONTWAIT);
        if (rc < 0 && nn_errno() != EAGAIN) {
            break;
        }
        else if (rc > 0) {
            bool breakable = false;
            if (Nanoem__Application__Event *event =
                    nanoem__application__event__unpack(g_protobufc_allocator, rc, body)) {
                if (event->type_case == NANOEM__APPLICATION__EVENT__TYPE_PING_PONG && event->has_requested_timestamp &&
                    event->requested_timestamp == command.timestamp) {
                    breakable = true;
                }
                nanoem__application__event__free_unpacked(event, g_protobufc_allocator);
            }
            nn_freemsg(body);
            nn_freemsg(control);
            if (breakable) {
                break;
            }
        }
    }
}

} /* namespace project */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_ENABLE_NANOMSG */
