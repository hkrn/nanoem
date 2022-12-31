/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/Archiver.h"

#include "emapp/Error.h"
#include "emapp/FileUtils.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#include "mz.h"
#include "mz_strm.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"
#include "zlib.h"

namespace nanoem {

struct Archiver::Opaque : mz_stream {
    static nanoem_i32_t
    internalSeek(ISeekable *seekable, int64_t offset, int origin)
    {
        Error error;
        switch (origin) {
        case MZ_SEEK_SET:
        default:
            return seekable->seek(int64_t(offset), ISeekable::kSeekTypeBegin, error) >= 0 ? MZ_OK : MZ_STREAM_ERROR;
        case MZ_SEEK_CUR:
            return seekable->seek(int64_t(offset), ISeekable::kSeekTypeCurrent, error) >= 0 ? MZ_OK : MZ_STREAM_ERROR;
        case MZ_SEEK_END:
            return seekable->seek(int64_t(offset), ISeekable::kSeekTypeEnd, error) >= 0 ? MZ_OK : MZ_STREAM_ERROR;
        }
    }
    static int64_t
    internalTell(ISeekable *seeker)
    {
        Error error;
        return int64_t(seeker->seek(0, ISeekable::kSeekTypeCurrent, error));
    }
    static nanoem_i32_t
    internalOpen(void *stream, const char *filename, int mode)
    {
        BX_UNUSED_3(stream, filename, mode);
        return MZ_OK;
    }
    static nanoem_i32_t
    internalIsOpened(void *stream)
    {
        BX_UNUSED_1(stream);
        return MZ_OK;
    }
    static nanoem_i32_t
    internalClose(void *stream)
    {
        BX_UNUSED_1(stream);
        return MZ_OK;
    }
    static nanoem_i32_t
    internalRead(void *stream, void *buf, nanoem_i32_t size)
    {
        Opaque *o = static_cast<Opaque *>(stream);
        nanoem_i32_t read = FileUtils::read(o->m_reader, buf, size, o->m_error);
        return read;
    }
    static nanoem_i32_t
    internalWrite(void *stream, const void *buf, nanoem_i32_t size)
    {
        Opaque *o = static_cast<Opaque *>(stream);
        nanoem_i32_t written = o->m_writer->write(buf, size, o->m_error);
        return written;
    }
    static int64_t
    internalTell(void *stream)
    {
        Opaque *o = static_cast<Opaque *>(stream);
        return o->m_reader ? internalTell(o->m_reader) : internalTell(o->m_writer);
    }
    static nanoem_i32_t
    internalSeek(void *stream, int64_t offset, int origin)
    {
        Opaque *o = static_cast<Opaque *>(stream);
        return o->m_reader ? internalSeek(o->m_reader, offset, origin) : internalSeek(o->m_writer, offset, origin);
    }
    static nanoem_i32_t
    internalError(void *stream)
    {
        Opaque *o = static_cast<Opaque *>(stream);
        return !o->m_error.hasReason() ? MZ_OK : o->m_error.code();
    }
    static void *
    internalCreate(void **stream)
    {
        Opaque *o = static_cast<Opaque *>(*stream);
        return o;
    }
    static void
    internalDestroy(void **stream)
    {
        BX_UNUSED_1(stream);
    }
    static nanoem_u16_t
    saturateInt16(size_t value)
    {
        return static_cast<nanoem_u16_t>(glm::min(value, size_t(0xffffu)));
    }
    static void
    fromEntry(const Archiver::Entry &entry, mz_zip_file &info)
    {
        Inline::clearZeroMemory(info);
        info.comment = entry.m_comment.c_str();
        info.comment_size = Opaque::saturateInt16(entry.m_comment.size());
        info.compression_method = entry.m_method;
        info.crc = entry.m_crc;
        info.extrafield = entry.m_fileExtraField.data();
        info.extrafield_size = Opaque::saturateInt16(entry.m_fileExtraField.size());
        info.filename = entry.m_path.c_str();
        info.filename_size = Opaque::saturateInt16(entry.m_path.size());
    }
    static void
    toEntry(const mz_zip_file *info, Archiver::Entry &entry)
    {
        entry.m_path = String(info->filename, info->filename_size);
        entry.m_comment = String(info->comment, info->comment_size);
        entry.m_compressedSize = info->compressed_size;
        entry.m_uncompressedSize = info->uncompressed_size;
        entry.m_fileExtraField.assign(info->extrafield, info->extrafield + info->extrafield_size);
    }

    Opaque(ISeekableReader *reader)
        : m_reader(reader)
        , m_writer(nullptr)
        , m_zip(nullptr)
    {
        initialize();
    }
    Opaque(ISeekableWriter *writer)
        : m_reader(nullptr)
        , m_writer(writer)
        , m_zip(nullptr)
    {
        initialize();
    }
    ~Opaque() NANOEM_DECL_NOEXCEPT
    {
        Error error;
        handleClose(error);
    }
    bool
    handleOpen(Error &error)
    {
        int rc = MZ_OK;
        if (m_reader) {
            void *stream = this;
            mz_stream_create(&stream, &m_vtable);
            mz_zip_create(&m_zip);
            rc = mz_zip_open(m_zip, stream, MZ_OPEN_MODE_READ);
            if (rc == MZ_OK) {
                rc = mz_zip_goto_first_entry(m_zip);
            }
        }
        else if (m_writer) {
            void *stream = this;
            mz_stream_create(&stream, &m_vtable);
            mz_zip_create(&m_zip);
            rc = mz_zip_open(m_zip, stream, MZ_OPEN_MODE_CREATE | MZ_OPEN_MODE_WRITE | MZ_OPEN_MODE_APPEND);
        }
        if (rc != MZ_OK) {
            error = Error("Cannot open zip file", rc, Error::kDomainTypeMinizip);
        }
        return m_zip && rc == MZ_OK;
    }
    bool
    handleClose(Error &error)
    {
        int rc = MZ_OK;
        if (m_zip) {
            rc = mz_zip_close(m_zip);
            mz_zip_delete(&m_zip);
            m_zip = nullptr;
        }
        if (rc != MZ_OK) {
            error = Error("Cannot close zip file", rc, Error::kDomainTypeMinizip);
        }
        return rc == MZ_OK;
    }
    void
    initialize()
    {
        m_vtable.open = internalOpen;
        m_vtable.is_open = internalIsOpened;
        m_vtable.read = internalRead;
        m_vtable.write = internalWrite;
        m_vtable.tell = internalTell;
        m_vtable.seek = internalSeek;
        m_vtable.close = internalClose;
        m_vtable.error = internalError;
        m_vtable.create = internalCreate;
        m_vtable.destroy = internalDestroy;
        m_vtable.get_prop_int64 = nullptr;
        m_vtable.set_prop_int64 = nullptr;
        vtbl = &m_vtable;
        base = this;
    }

    mz_stream_vtbl m_vtable;
    ISeekableReader *m_reader;
    ISeekableWriter *m_writer;
    Error m_error;
    void *m_zip;
};

Archiver::Entry::Entry()
    : m_crc(0)
    , m_compressedSize(0)
    , m_uncompressedSize(0)
    , m_method(Z_DEFLATED)
    , m_level(Z_BEST_SPEED)
    , m_raw(0)
{
}

String
Archiver::Entry::basePath() const
{
    String s;
    if (!m_path.empty()) {
        const char *p = m_path.c_str();
        if (isDirectory()) {
            s.append(p, p + m_path.size() - 1);
            const char *r = strrchr(s.c_str(), '/');
            s = r ? String(p, size_t(r - s.c_str())) : "/";
        }
        else {
            s.append(p, filenamePtr());
        }
    }
    return s;
}

String
Archiver::Entry::lastPathComponent() const
{
    String s;
    if (isDirectory()) {
        const char *basePtr = m_path.c_str(), *ptr = basePtr + m_path.size(), *startPtr = nullptr, *endPtr = nullptr;
        while (ptr > basePtr) {
            if (endPtr && *ptr == '/') {
                startPtr = ptr + 1;
                break;
            }
            else if (*ptr == '/') {
                endPtr = ptr;
            }
            ptr--;
        }
        if (startPtr && endPtr) {
            s.append(startPtr, endPtr);
        }
        else if (endPtr) {
            s.append(basePtr, endPtr);
        }
        else {
            s = m_path;
        }
    }
    else {
        s = filenamePtr();
    }
    return s;
}

const char *
Archiver::Entry::filenamePtr() const NANOEM_DECL_NOEXCEPT
{
    const char *basePtr = m_path.c_str(), *p = strrchr(basePtr, '/');
    return p ? p + 1 : basePtr;
}

const char *
Archiver::Entry::extensionPtr() const NANOEM_DECL_NOEXCEPT
{
    const char *extension = nullptr;
    if (const char *q = strrchr(filenamePtr(), '.')) {
        extension = q + 1;
    }
    return extension;
}

bool
Archiver::Entry::isDirectory() const NANOEM_DECL_NOEXCEPT
{
    return !m_path.empty() && *(m_path.c_str() + m_path.size() - 1) == '/';
}

Archiver::Archiver(ISeekableReader *reader)
    : m_opaque(nanoem_new(Opaque(reader)))
{
}

Archiver::Archiver(ISeekableWriter *writer)
    : m_opaque(nanoem_new(Opaque(writer)))
{
}

Archiver::~Archiver() NANOEM_DECL_NOEXCEPT
{
    nanoem_delete_safe(m_opaque);
}

bool
Archiver::open(Error &error)
{
    return m_opaque->handleOpen(error);
}

bool
Archiver::close(Error &error)
{
    return m_opaque->handleClose(error);
}

bool
Archiver::addEntry(const Entry &entry, const ByteArray &bytes, Error &error)
{
    int rc = MZ_PARAM_ERROR;
    if (void *file = m_opaque->m_zip) {
        mz_zip_file info;
        Opaque::fromEntry(entry, info);
        rc = mz_zip_entry_write_open(file, &info, entry.m_level, entry.m_raw, nullptr);
        if (rc == MZ_OK) {
            int size = Inline::saturateInt32(bytes.size());
            rc = mz_zip_entry_write(file, bytes.data(), size);
            if (rc == size) {
                rc = mz_zip_entry_close(file);
            }
        }
    }
    if (rc != MZ_OK) {
        char buffer[Inline::kLongNameStackBufferSize];
        StringUtils::format(buffer, sizeof(buffer), "Cannot add file entry to the zip: %s", entry.m_path.c_str());
        error = Error(buffer, rc, Error::kDomainTypeMinizip);
    }
    return rc == MZ_OK;
}

bool
Archiver::addEntry(const Entry &entry, IReader *reader, Error &error)
{
    int rc = MZ_PARAM_ERROR;
    if (void *file = m_opaque->m_zip) {
        mz_zip_file info;
        Opaque::fromEntry(entry, info);
        rc = mz_zip_entry_write_open(file, &info, entry.m_level, entry.m_raw, nullptr);
        if (rc == MZ_OK) {
            nanoem_u8_t buffer[Inline::kReadingFileContentsBufferSize];
            while (!error.hasReason()) {
                rc = FileUtils::read(reader, buffer, sizeof(buffer), error);
                if (rc > 0) {
                    mz_zip_entry_write(file, buffer, rc);
                }
                else if (rc <= 0) {
                    break;
                }
            }
            if (rc == MZ_OK) {
                rc = mz_zip_entry_close(file);
            }
        }
    }
    if (rc != MZ_OK && !error.hasReason()) {
        char buffer[Inline::kLongNameStackBufferSize];
        StringUtils::format(buffer, sizeof(buffer), "Cannot add file entry to the zip: %s", entry.m_path.c_str());
        error = Error(buffer, rc, Error::kDomainTypeMinizip);
    }
    return rc == MZ_OK;
}

bool
Archiver::findEntry(const String &location, Entry &entry, Error &error) const
{
    int rc = MZ_PARAM_ERROR;
    if (void *file = m_opaque->m_zip) {
        rc = mz_zip_locate_entry(file, location.c_str(), 0);
        if (rc == MZ_OK) {
            MutableString filename, comment;
            mz_zip_file *info;
            rc = mz_zip_entry_get_info(file, &info);
            if (rc == MZ_OK) {
                Opaque::toEntry(info, entry);
            }
        }
    }
    if (rc != MZ_OK && rc != MZ_END_OF_LIST) {
        char buffer[Inline::kLongNameStackBufferSize];
        StringUtils::format(buffer, sizeof(buffer), "Cannot find file entry to the zip: %s", location.c_str());
        error = Error(buffer, rc, Error::kDomainTypeMinizip);
    }
    return rc == MZ_OK;
}

bool
Archiver::extract(const Entry &entry, ByteArray &bytes, Error &error) const
{
    int rc = MZ_PARAM_ERROR;
    if (void *file = m_opaque->m_zip) {
        bytes.resize(size_t(entry.m_uncompressedSize));
        rc = mz_zip_entry_read_open(file, entry.m_raw, nullptr);
        if (rc == MZ_OK) {
            nanoem_i32_t size = Inline::saturateInt32(bytes.size());
            rc = mz_zip_entry_read(file, bytes.data(), size);
            if (rc == size) {
                rc = mz_zip_entry_close(file);
            }
        }
    }
    if (rc != MZ_OK) {
        char buffer[Inline::kLongNameStackBufferSize];
        StringUtils::format(buffer, sizeof(buffer), "Cannot extract file entry to the zip: %s", entry.m_path.c_str());
        error = Error(buffer, rc, Error::kDomainTypeMinizip);
    }
    return rc == MZ_OK;
}

bool
Archiver::extract(const Entry &entry, IWriter *writer, Error &error) const
{
    int rc = MZ_PARAM_ERROR;
    if (void *file = m_opaque->m_zip) {
        rc = mz_zip_entry_read_open(file, entry.m_raw, nullptr);
        if (rc == MZ_OK) {
            nanoem_u8_t buffer[Inline::kReadingFileContentsBufferSize];
            while (!error.hasReason()) {
                rc = mz_zip_entry_read(file, buffer, sizeof(buffer));
                if (rc > 0) {
                    writer->write(buffer, rc, error);
                }
                else if (rc <= 0) {
                    break;
                }
            }
            if (rc == MZ_OK) {
                rc = mz_zip_entry_close(file);
            }
        }
    }
    if (rc != MZ_OK && !error.hasReason()) {
        char buffer[Inline::kLongNameStackBufferSize];
        StringUtils::format(buffer, sizeof(buffer), "Cannot extract file entry to the zip: %s", entry.m_path.c_str());
        error = Error(buffer, rc, Error::kDomainTypeMinizip);
    }
    return rc == MZ_OK;
}

Archiver::EntryList
Archiver::allEntries(Error &error) const
{
    EntryList entries;
    if (void *file = m_opaque->m_zip) {
        int rc = mz_zip_goto_first_entry(file);
        MutableString fn, comment;
        Entry entry;
        while (rc == MZ_OK) {
            mz_zip_file *info;
            rc = mz_zip_entry_get_info(file, &info);
            if (rc != MZ_OK) {
                error = Error("Cannot get file entry in the zip", rc, Error::kDomainTypeMinizip);
                break;
            }
            Opaque::toEntry(info, entry);
            entries.push_back(entry);
            rc = mz_zip_goto_next_entry(file);
            if (rc != MZ_OK && rc != MZ_END_OF_LIST) {
                error = Error("Cannot seek next file entry in the zip", rc, Error::kDomainTypeMinizip);
                break;
            }
        }
    }
    return entries;
}

Archiver::EntryList
Archiver::entries(const String &prefix, Error &error) const
{
    const EntryList &entries = allEntries(error);
    Archiver::EntryList newEntries;
    for (EntryList::const_iterator it = entries.begin(), end = entries.end(); it != end; ++it) {
        const Archiver::Entry &entry = *it;
        if (prefix == entry.basePath() || (StringUtils::equals(prefix.c_str(), "/") && entry.basePath().empty())) {
            newEntries.push_back(*it);
        }
    }
    return newEntries;
}

} /* namespace */
