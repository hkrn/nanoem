/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#pragma once
#ifndef NANOEM_EMAPP_FILEUTILS_H_
#define NANOEM_EMAPP_FILEUTILS_H_

#include "bx/readerwriter.h"

#include "emapp/Forward.h"

namespace nanoem {

class Error;
class ITranslator;
class URI;

class ISeekable {
public:
    enum SeekType {
        kSeekTypeBegin,
        kSeekTypeCurrent,
        kSeekTypeEnd,
    };
    virtual nanoem_i64_t seek(nanoem_i64_t offset, SeekType whence, Error &error) = 0;
};

class IReader {
public:
    virtual nanoem_i32_t read(void *data, nanoem_i32_t size, Error &error) = 0;
    virtual nanoem_rsize_t size() = 0;
};

class ISeekableReader : public IReader, public ISeekable { };

class IFileReader : public ISeekableReader {
public:
    virtual ~IFileReader() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual bool open(const URI &fileURI, Error &error) = 0;
    virtual bool close(Error &error) = 0;
    virtual URI fileURI() const = 0;
};

class IWriter {
public:
    virtual nanoem_i32_t write(const void *data, nanoem_i32_t size, Error &error) = 0;
};

class ISeekableWriter : public IWriter, public ISeekable { };

class IFileWriter : public ISeekableWriter {
public:
    virtual ~IFileWriter() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual bool open(const URI &fileURI, bool append, Error &error) = 0;
    virtual bool commit(Error &error) = 0;
    virtual bool rollback(Error &error) = 0;
    virtual bool close(Error &error) = 0;
    virtual URI fileURI() const = 0;
    virtual URI writeDestinationURI() const = 0;
};

class FileReaderScope NANOEM_DECL_SEALED : private NonCopyable {
public:
    FileReaderScope(const ITranslator *translator);
    ~FileReaderScope() NANOEM_DECL_NOEXCEPT;

    bool open(const URI &fileURI, Error &error);
    IFileReader *reader() NANOEM_DECL_NOEXCEPT;
    IFileReader *release() NANOEM_DECL_NOEXCEPT;

private:
    IFileReader *m_reader;
};

class FileWriterScope NANOEM_DECL_SEALED : private NonCopyable {
public:
    FileWriterScope();
    ~FileWriterScope() NANOEM_DECL_NOEXCEPT;

    bool open(const URI &fileURI, Error &error);
    bool open(const URI &fileURI, bool append, Error &error);
    void commit(Error &error);
    void rollback(Error &error);
    IFileWriter *writer() NANOEM_DECL_NOEXCEPT;
    IFileWriter *release() NANOEM_DECL_NOEXCEPT;

private:
    IFileWriter *m_writer;
};

class MemoryReader NANOEM_DECL_SEALED : public ISeekableReader, private NonCopyable {
public:
    MemoryReader(const ByteArray *bytes);
    ~MemoryReader() NANOEM_DECL_NOEXCEPT;

    nanoem_i32_t read(void *data, nanoem_i32_t size, Error &error) NANOEM_DECL_OVERRIDE;
    nanoem_rsize_t size() NANOEM_DECL_OVERRIDE;
    nanoem_i64_t seek(nanoem_i64_t offset, SeekType whence, Error &error) NANOEM_DECL_OVERRIDE;

private:
    const ByteArray *m_bytesPtr;
    nanoem_i64_t m_offset;
};

class MemoryWriter NANOEM_DECL_SEALED : public ISeekableWriter, private NonCopyable {
public:
    MemoryWriter(ByteArray *bytes);
    ~MemoryWriter() NANOEM_DECL_NOEXCEPT;

    nanoem_i32_t write(const void *data, nanoem_i32_t size, Error &error) NANOEM_DECL_OVERRIDE;
    nanoem_i64_t seek(nanoem_i64_t offset, SeekType whence, Error &error) NANOEM_DECL_OVERRIDE;

    void clear();

private:
    ByteArray *m_bytesPtr;
    nanoem_i64_t m_offset;
};

class FileUtils NANOEM_DECL_SEALED : private NonCopyable {
public:
    struct TransientPath {
        TransientPath();
        TransientPath(const String &path);
        String m_path;
        intptr_t m_handle;
        bool m_valid;
    };

    static nanoem_u64_t timestamp(const char *filePath) NANOEM_DECL_NOEXCEPT;
    static nanoem_u64_t timestamp(const URI &fileURI) NANOEM_DECL_NOEXCEPT;
    static bool exists(const char *filePath) NANOEM_DECL_NOEXCEPT;
    static bool exists(const URI &fileURI) NANOEM_DECL_NOEXCEPT;
    static bool deleteFile(const char *filePath);
    static bool deleteFile(const URI &fileURI);

    static bool createTransientFile(const String &source, TransientPath &dest);
    static bool deleteTransientFile(TransientPath &path);
    static bool isLoadableExtension(const String &value, const StringSet &supported) NANOEM_DECL_NOEXCEPT;
    static void canonicalizePathSeparator(MutableString &output);
    static void canonicalizePathSeparator(const String &path, String &output);
    static String canonicalizePath(const String &originPath, const String &filename);
    static String relativePath(const String &value, const String &basePath);

    static nanoem_i32_t read(FileReaderScope &reader, void *data, nanoem_rsize_t size, Error &error);
    static nanoem_i32_t read(FileReaderScope &reader, ByteArray &bytes, Error &error);
    static nanoem_i32_t read(IReader *reader, void *data, nanoem_rsize_t size, Error &error);
    static nanoem_i32_t read(IReader *reader, ByteArray &bytes, Error &error);
    static nanoem_i32_t write(IWriter *writer, const void *data, nanoem_rsize_t size, Error &error);
    static nanoem_i32_t write(IWriter *writer, const ByteArray &bytes, Error &error);
    static nanoem_i32_t write(IWriter *writer, const String &string, Error &error);
    static nanoem_i32_t write(IWriter *writer, const nanoem_buffer_t *buffer, Error &error);

    template <typename T>
    static inline nanoem_i32_t
    readTyped(IReader *reader, T &data, Error &error)
    {
        return read(reader, &data, sizeof(data), error);
    }
    template <typename T>
    static inline nanoem_i32_t
    writeTyped(IWriter *writer, const T &data, Error &error)
    {
        return write(writer, &data, sizeof(data), error);
    }

    static IFileReader *createFileReader(const ITranslator *translator);
    static IFileWriter *createFileWriter();
    static void destroyFileReader(IFileReader *reader);
    static void destroyFileWriter(IFileWriter *writer);
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_FILEUTILS_H_ */
