/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_ARCHIVER_H_
#define NANOEM_EMAPP_ARCHIVER_H_

#include "emapp/Forward.h"

namespace nanoem {

class Error;
class ISeekableReader;
class ISeekableWriter;
class IReader;
class IWriter;

class Archiver NANOEM_DECL_SEALED : private NonCopyable {
public:
    struct Entry {
        String m_path;
        ByteArray m_fileExtraField;
        ByteArray m_localExtraField;
        ByteArray m_globalExtraField;
        String m_comment;
        String m_password;
        nanoem_u32_t m_crc;
        nanoem_u64_t m_compressedSize;
        nanoem_u64_t m_uncompressedSize;
        nanoem_u16_t m_method;
        nanoem_i16_t m_level;
        nanoem_u8_t m_raw;
        Entry();
        String basePath() const;
        String lastPathComponent() const;
        const char *filenamePtr() const NANOEM_DECL_NOEXCEPT;
        const char *extensionPtr() const NANOEM_DECL_NOEXCEPT;
        bool isDirectory() const NANOEM_DECL_NOEXCEPT;
    };
    typedef tinystl::vector<Entry, TinySTLAllocator> EntryList;

    Archiver(ISeekableReader *reader);
    Archiver(ISeekableWriter *writer);
    ~Archiver() NANOEM_DECL_NOEXCEPT;

    bool open(Error &error);
    bool close(Error &error);
    bool addEntry(const Entry &entry, const ByteArray &bytes, Error &error);
    bool addEntry(const Entry &entry, IReader *reader, Error &error);
    bool findEntry(const String &location, Entry &entry, Error &error) const;
    bool extract(const Entry &entry, ByteArray &bytes, Error &error) const;
    bool extract(const Entry &entry, IWriter *writer, Error &error) const;
    EntryList allEntries(Error &error) const;
    EntryList entries(const String &entry, Error &error) const;

private:
    struct Opaque;
    Opaque *m_opaque;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_ARCHIVER_H_ */
