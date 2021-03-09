/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/Forward.h"

typedef struct _Nanoem__Application__Command Nanoem__Application__Command;

namespace nanoem {

class BaseApplicationService;
class Error;
class ISeekableReader;
class IWriter;
class IModalDialog;
class Project;

namespace internal {
namespace project {

class Redo NANOEM_DECL_SEALED : private NonCopyable {
public:
    static const nanoem_u32_t kFileMagic;

    Redo(Project *project);
    ~Redo() NANOEM_DECL_NOEXCEPT;

    void loadAll(ISeekableReader *reader, BaseApplicationService *application, Error &error);
    void loadAllAsync(ISeekableReader *reader, IModalDialog *dialog, bool *cancelled, Error &error);
    bool save(IWriter *writer, nanoem_u32_t sequence, const Nanoem__Application__Command *command, Error &error);

private:
    static bool isAccepted(nanoem_u32_t value) NANOEM_DECL_NOEXCEPT;
    static void getCountOffset(ISeekableReader *reader, nanoem_u32_t &count, nanoem_u32_t &offset, Error &error);
    void inflateChunk(const ByteArray &input, ByteArray &output);
    void sendCommandMessage(int commandStreamSocket, const ByteArray &inflated, nanoem_u16_t commandType,
        IWriter *writer, nanoem_u32_t sequence);
    void waitEventMessage(int commandStreamSocket, int eventStreamSocket, bool *cancelled);

    ByteArray m_interm;
    Project *m_project;
};

} /* namespace project */
} /* namespace internal */
} /* namespace nanoem */
