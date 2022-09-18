/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/FileUtils.h"

#include "emapp/Error.h"
#include "emapp/ITranslator.h"
#include "emapp/StringUtils.h"
#include "emapp/URI.h"
#include "emapp/private/CommonInclude.h"

#include <stdio.h>
#if !BX_PLATFORM_WINDOWS
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace nanoem {
namespace {

#if BX_PLATFORM_WINDOWS
static void
setErrorMessage(Error &error)
{
    wchar_t buffer[Error::kMaxReasonLength];
    MutableString msg;
    const DWORD err = GetLastError();
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, err, LANG_USER_DEFAULT, buffer, ARRAYSIZE(buffer), 0);
    StringUtils::getMultiBytesString(buffer, msg);
    error = Error(msg.data(), err, Error::kDomainTypeOS);
}

static const wchar_t *const kTemporaryPathExtension = L".tmp";

class Win32FileReader NANOEM_DECL_SEALED : public IFileReader, private NonCopyable {
public:
    Win32FileReader(const ITranslator *translator);
    ~Win32FileReader() NANOEM_DECL_OVERRIDE;

    bool open(const URI &fileURI, Error &error) NANOEM_DECL_OVERRIDE;
    bool close(Error &error) NANOEM_DECL_OVERRIDE;
    nanoem_i64_t seek(nanoem_i64_t offset, SeekType whence, Error &error) NANOEM_DECL_OVERRIDE;
    nanoem_i32_t read(void *data, nanoem_i32_t size, Error &error) NANOEM_DECL_OVERRIDE;
    nanoem_rsize_t size() NANOEM_DECL_OVERRIDE;
    URI fileURI() const NANOEM_DECL_OVERRIDE;

private:
    const ITranslator *m_translator;
    HANDLE m_file;
    URI m_fileURI;
};

Win32FileReader::Win32FileReader(const ITranslator *translator)
    : m_translator(translator)
    , m_file(INVALID_HANDLE_VALUE)
{
}

Win32FileReader::~Win32FileReader() NANOEM_DECL_NOEXCEPT
{
}

bool
Win32FileReader::open(const URI &fileURI, Error &error)
{
    MutableWideString ws;
    StringUtils::getWideCharString(fileURI.absolutePathConstString(), ws);
    m_file = CreateFileW(ws.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_READONLY | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
    if (m_file != INVALID_HANDLE_VALUE) {
        m_fileURI = fileURI;
    }
    else {
        const DWORD err = GetLastError();
        wchar_t buffer[Error::kMaxReasonLength];
        MutableString msg;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, err, LANG_USER_DEFAULT, buffer, ARRAYSIZE(buffer), 0);
        StringUtils::getMultiBytesString(buffer, msg);
        if ((err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND) && m_translator) {
            char buffer[Error::kMaxRecoverySuggestionLength];
            StringUtils::format(buffer, sizeof(buffer),
                m_translator->translate("nanoem.window.dialog.error.no-such-file.recovery-suggestion"),
                fileURI.absolutePathConstString());
            error = Error(msg.data(), buffer, Error::kDomainTypeOS);
        }
        else {
            error = Error(msg.data(), err, Error::kDomainTypeOS);
        }
    }
    return INVALID_HANDLE_VALUE != m_file;
}

bool
Win32FileReader::close(Error &error)
{
    bool result = true;
    if (m_file != INVALID_HANDLE_VALUE) {
        if (CloseHandle(m_file)) {
            m_file = INVALID_HANDLE_VALUE;
        }
        else {
            setErrorMessage(error);
            result = false;
        }
    }
    return result;
}

nanoem_i64_t
Win32FileReader::seek(nanoem_i64_t offset, ISeekable::SeekType whence, Error &error)
{
    nanoem_i64_t result = 0;
    if (m_file != INVALID_HANDLE_VALUE) {
        DWORD actual = SetFilePointer(m_file, LONG(offset), nullptr, whence);
        if (actual != INVALID_SET_FILE_POINTER) {
            result = actual;
        }
        else {
            setErrorMessage(error);
        }
    }
    return result;
}

nanoem_i32_t
Win32FileReader::read(void *data, nanoem_i32_t size, Error &error)
{
    DWORD read = 0;
    if (m_file != INVALID_HANDLE_VALUE && !ReadFile(m_file, data, DWORD(size), &read, nullptr)) {
        setErrorMessage(error);
    }
    return read;
}

nanoem_rsize_t
Win32FileReader::size()
{
    LARGE_INTEGER actual;
    nanoem_rsize_t result = 0;
    if (m_file != INVALID_HANDLE_VALUE && GetFileSizeEx(m_file, &actual)) {
        result = static_cast<nanoem_rsize_t>(actual.QuadPart);
    }
    return result;
}

URI
Win32FileReader::fileURI() const
{
    return m_fileURI;
}

class Win32FileWriter NANOEM_DECL_SEALED : public IFileWriter, private NonCopyable {
public:
    Win32FileWriter();
    ~Win32FileWriter() NANOEM_DECL_OVERRIDE;

    bool open(const URI &fileURI, bool append, Error &error) NANOEM_DECL_OVERRIDE;
    bool close(Error &error) NANOEM_DECL_OVERRIDE;
    nanoem_i64_t seek(nanoem_i64_t offset, SeekType whence, Error &error) NANOEM_DECL_OVERRIDE;
    nanoem_i32_t write(const void *data, nanoem_i32_t size, Error &error) NANOEM_DECL_OVERRIDE;
    bool commit(Error &error) NANOEM_DECL_OVERRIDE;
    bool rollback(Error &error) NANOEM_DECL_OVERRIDE;
    URI fileURI() const NANOEM_DECL_OVERRIDE;
    URI writeDestinationURI() const NANOEM_DECL_OVERRIDE;

    HANDLE m_file;
    URI m_fileURI;
    URI m_writeDestinationURI;
};

Win32FileWriter::Win32FileWriter()
    : m_file(INVALID_HANDLE_VALUE)
{
}

Win32FileWriter::~Win32FileWriter() NANOEM_DECL_NOEXCEPT
{
}

bool
Win32FileWriter::open(const URI &fileURI, bool append, Error &error)
{
    URI tempURI;
    MutableWideString originPath;
    StringUtils::getWideCharString(fileURI.absolutePathConstString(), originPath);
    if (append) {
        m_file = CreateFileW(
            originPath.data(), FILE_APPEND_DATA, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    }
    else {
        MutableWideString tempPath(originPath);
        tempPath.insert(
            tempPath.end() - 1, kTemporaryPathExtension, kTemporaryPathExtension + wcslen(kTemporaryPathExtension));
        m_file = CreateFileW(tempPath.data(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        MutableString utf8TempPath;
        StringUtils::getMultiBytesString(tempPath.data(), utf8TempPath);
        tempURI = URI::createFromFilePath(utf8TempPath.data());
    }
    if (m_file != INVALID_HANDLE_VALUE) {
        m_fileURI = fileURI;
        m_writeDestinationURI = tempURI;
    }
    else {
        setErrorMessage(error);
    }
    return INVALID_HANDLE_VALUE != m_file;
}

bool
Win32FileWriter::close(Error &error)
{
    bool result = true;
    if (m_file != INVALID_HANDLE_VALUE) {
        if (CloseHandle(m_file)) {
            m_file = INVALID_HANDLE_VALUE;
        }
        else {
            setErrorMessage(error);
            result = false;
        }
    }
    return result;
}

nanoem_i64_t
Win32FileWriter::seek(nanoem_i64_t offset, SeekType whence, Error &error)
{
    nanoem_i64_t result = 0;
    if (m_file != INVALID_HANDLE_VALUE) {
        LONG high = LONG(offset >> 32) & 0x7fffffff;
        DWORD actual = SetFilePointer(m_file, offset & 0x7fffffff, &high, whence);
        if (actual != INVALID_SET_FILE_POINTER) {
            result = actual;
        }
        else {
            setErrorMessage(error);
        }
    }
    return result;
}

nanoem_i32_t
Win32FileWriter::write(const void *data, nanoem_i32_t size, Error &error)
{
    DWORD written = 0;
    if (m_file != INVALID_HANDLE_VALUE && !WriteFile(m_file, data, DWORD(size), &written, nullptr)) {
        setErrorMessage(error);
    }
    return written;
}

bool
Win32FileWriter::commit(Error &error)
{
    bool result = true;
    MutableWideString src, dst;
    StringUtils::getWideCharString(m_writeDestinationURI.absolutePathConstString(), src);
    StringUtils::getWideCharString(m_fileURI.absolutePathConstString(), dst);
    if (!MoveFileExW(src.data(), dst.data(), MOVEFILE_REPLACE_EXISTING)) {
        setErrorMessage(error);
        result = false;
    }
    return result;
}

bool
Win32FileWriter::rollback(Error &error)
{
    bool result = true;
    MutableWideString fn;
    StringUtils::getWideCharString(m_writeDestinationURI.absolutePathConstString(), fn);
    fn.insert(fn.end() - 1, kTemporaryPathExtension, kTemporaryPathExtension + wcslen(kTemporaryPathExtension));
    if (!DeleteFileW(fn.data())) {
        setErrorMessage(error);
        result = false;
    }
    return result;
}

URI
Win32FileWriter::fileURI() const
{
    return m_fileURI;
}

URI
Win32FileWriter::writeDestinationURI() const
{
    return m_writeDestinationURI;
}

#else

static void
assignError(Error &error, int err)
{
    error = Error(strerror(err), err, Error::kDomainTypeOS);
}

static void
assignError(Error &error)
{
    assignError(error, errno);
}

class PosixFileReader NANOEM_DECL_SEALED : public IFileReader, private NonCopyable {
public:
    PosixFileReader(const ITranslator *translator);
    ~PosixFileReader() NANOEM_DECL_NOEXCEPT_OVERRIDE;

    bool open(const URI &fileURI, Error &error) NANOEM_DECL_OVERRIDE;
    nanoem_i32_t read(void *_data, nanoem_i32_t _size, Error &error) NANOEM_DECL_OVERRIDE;
    nanoem_i64_t seek(nanoem_i64_t offset, SeekType whence, Error &error) NANOEM_DECL_OVERRIDE;
    nanoem_rsize_t size() NANOEM_DECL_OVERRIDE;
    bool close(Error &error) NANOEM_DECL_OVERRIDE;
    URI fileURI() const NANOEM_DECL_OVERRIDE;

private:
    const ITranslator *m_translator;
    URI m_fileURI;
    int m_fd;
};

PosixFileReader::PosixFileReader(const ITranslator *translator)
    : m_translator(translator)
    , m_fd(-1)
{
}

PosixFileReader::~PosixFileReader() NANOEM_DECL_NOEXCEPT
{
}

bool
PosixFileReader::open(const URI &fileURI, Error &error)
{
    m_fd = ::open(fileURI.absolutePathConstString(), O_RDONLY);
    int err = 0;
    bool succeeded = m_fd != -1;
    if (succeeded) {
        struct stat st;
        succeeded = ::fstat(m_fd, &st) != -1;
        if (S_ISREG(st.st_mode)) {
            m_fileURI = fileURI;
        }
        else if (succeeded) {
            succeeded = false;
            err = ENOENT;
        }
        else {
            err = errno;
        }
    }
    else {
        err = errno;
    }
    if (!succeeded) {
        if (err == ENOENT && m_translator) {
            char recoverySuggestion[Error::kMaxRecoverySuggestionLength];
            StringUtils::format(recoverySuggestion, sizeof(recoverySuggestion),
                m_translator->translate("nanoem.window.dialog.error.no-such-file.recovery-suggestion"),
                fileURI.absolutePathConstString());
            error = Error(strerror(err), recoverySuggestion, Error::kDomainTypeOS);
        }
        else {
            assignError(error);
        }
    }
    return succeeded;
}

nanoem_i32_t
PosixFileReader::read(void *data, nanoem_i32_t size, Error &error)
{
    ssize_t actual = ::read(m_fd, data, size);
    if (actual == -1) {
        assignError(error);
    }
    return nanoem_i32_t(actual);
}

nanoem_rsize_t
PosixFileReader::size()
{
    nanoem_rsize_t result = 0;
    struct stat st;
    if (::fstat(m_fd, &st) == 0 && S_ISREG(st.st_mode)) {
        result = st.st_size;
    }
    return result;
}

nanoem_i64_t
PosixFileReader::seek(nanoem_i64_t offset, SeekType whence, Error &error)
{
    nanoem_i64_t value = ::lseek(m_fd, offset, whence);
    if (value == -1) {
        assignError(error);
    }
    return value;
}

bool
PosixFileReader::close(Error &error)
{
    bool succeeded = true;
    if (m_fd != -1) {
        int result = ::close(m_fd);
        if (result == -1) {
            assignError(error);
            succeeded = false;
        }
        else {
            m_fd = -1;
        }
    }
    return succeeded;
}

URI
PosixFileReader::fileURI() const
{
    return m_fileURI;
}

class PosixFileWriter NANOEM_DECL_SEALED : public IFileWriter, private NonCopyable {
public:
    static void appendTemporaryFileExtensionPrefix(MutableString &value);

    PosixFileWriter();
    ~PosixFileWriter() NANOEM_DECL_NOEXCEPT_OVERRIDE;

    bool open(const URI &fileURI, bool append, Error &error) NANOEM_DECL_OVERRIDE;
    nanoem_i32_t write(const void *data, nanoem_i32_t size, Error &error) NANOEM_DECL_OVERRIDE;
    nanoem_i64_t seek(nanoem_i64_t offset, SeekType whence, Error &error) NANOEM_DECL_OVERRIDE;
    bool close(Error &error) NANOEM_DECL_OVERRIDE;
    bool commit(Error &error) NANOEM_DECL_OVERRIDE;
    bool rollback(Error &error) NANOEM_DECL_OVERRIDE;
    URI fileURI() const NANOEM_DECL_OVERRIDE;
    URI writeDestinationURI() const NANOEM_DECL_OVERRIDE;

private:
    URI m_fileURI;
    URI m_writeDestinationURI;
    int m_fd;
    bool m_append;
    bool m_done;
};

void
PosixFileWriter::appendTemporaryFileExtensionPrefix(MutableString &value)
{
    value.push_back('.');
    value.push_back('t');
    value.push_back('m');
    value.push_back('p');
    value.push_back(0);
}

PosixFileWriter::PosixFileWriter()
    : m_fd(-1)
    , m_append(false)
    , m_done(false)
{
}

PosixFileWriter::~PosixFileWriter() NANOEM_DECL_NOEXCEPT
{
    Error error;
    if (!m_done && !m_append) {
        bx::debugPrintf("forgot to call commit/rollback: fd=%d, path=%s", m_fd, m_fileURI.absolutePathConstString());
        rollback(error);
    }
}

bool
PosixFileWriter::open(const URI &fileURI, bool append, Error &error)
{
    URI tempURI;
    int flags = O_WRONLY | O_CREAT;
    if (append) {
        flags |= O_APPEND;
        m_fd = ::open(fileURI.absolutePathConstString(), flags, 0600);
        m_append = true;
    }
    else {
        const char *path = fileURI.absolutePathConstString();
        MutableString tempPath(path, path + StringUtils::length(path));
        appendTemporaryFileExtensionPrefix(tempPath);
        flags |= O_TRUNC;
        m_fd = ::open(tempPath.data(), flags, 0600);
        tempURI = URI::createFromFilePath(tempPath.data());
    }
    bool succeeded = m_fd != -1;
    if (succeeded) {
        m_fileURI = fileURI;
        m_writeDestinationURI = tempURI;
    }
    else if (!succeeded) {
        assignError(error);
    }
    return succeeded;
}

nanoem_i32_t
PosixFileWriter::write(const void *data, nanoem_i32_t size, Error &error)
{
    ssize_t written = ::write(m_fd, data, static_cast<nanoem_rsize_t>(size));
    if (written == -1) {
        assignError(error);
    }
    return nanoem_i32_t(written);
}

nanoem_i64_t
PosixFileWriter::seek(nanoem_i64_t offset, SeekType whence, Error &error)
{
    nanoem_i64_t value = ::lseek(m_fd, offset, whence);
    if (value == -1) {
        assignError(error);
    }
    return value;
}

bool
PosixFileWriter::close(Error &error)
{
    bool succeeded = true;
    if (m_fd != -1) {
        int result = ::close(m_fd);
        if (result == -1) {
            assignError(error);
            succeeded = false;
        }
        else {
            m_fd = -1;
        }
    }
    return succeeded;
}

bool
PosixFileWriter::commit(Error &error)
{
    int rc = -1;
    if (!m_writeDestinationURI.isEmpty()) {
        rc = ::rename(m_writeDestinationURI.absolutePathConstString(), m_fileURI.absolutePathConstString());
        if (rc == 0) {
            m_done = true;
        }
        else if (rc != 0 && !error.hasReason()) {
            error = Error(strerror(rc), rc, Error::kDomainTypeOS);
        }
    }
    return m_done;
}

bool
PosixFileWriter::rollback(Error &error)
{
    int rc = -1;
    if (!m_writeDestinationURI.isEmpty()) {
        rc = ::unlink(m_writeDestinationURI.absolutePathConstString());
        if (rc == 0) {
            m_done = true;
        }
        else if (rc != 0 && !error.hasReason()) {
            error = Error(strerror(rc), rc, Error::kDomainTypeOS);
        }
    }
    return m_done;
}

URI
PosixFileWriter::fileURI() const
{
    return m_fileURI;
}

URI
PosixFileWriter::writeDestinationURI() const
{
    return m_writeDestinationURI;
}

#endif /* BX_PLATFORM_WINDOWS */

} /* namespace anonymous */

FileReaderScope::FileReaderScope(const ITranslator *translator)
    : m_reader(FileUtils::createFileReader(translator))
{
}

FileReaderScope::~FileReaderScope() NANOEM_DECL_NOEXCEPT
{
    FileUtils::destroyFileReader(m_reader);
    m_reader = nullptr;
}

bool
FileReaderScope::open(const URI &fileURI, Error &error)
{
    return m_reader && m_reader->open(fileURI, error);
}

IFileReader *
FileReaderScope::reader() NANOEM_DECL_NOEXCEPT
{
    return m_reader;
}

IFileReader *
FileReaderScope::release() NANOEM_DECL_NOEXCEPT
{
    IFileReader *reader = m_reader;
    m_reader = nullptr;
    return reader;
}

FileWriterScope::FileWriterScope()
    : m_writer(FileUtils::createFileWriter())
{
}

FileWriterScope::~FileWriterScope() NANOEM_DECL_NOEXCEPT
{
    FileUtils::destroyFileWriter(m_writer);
    m_writer = nullptr;
}

bool
FileWriterScope::open(const URI &fileURI, Error &error)
{
    return open(fileURI, false, error);
}

bool
FileWriterScope::open(const URI &fileURI, bool append, Error &error)
{
    return m_writer && m_writer->open(fileURI, append, error);
}

void
FileWriterScope::commit(Error &error)
{
    if (m_writer->close(error)) {
        m_writer->commit(error);
    }
}

void
FileWriterScope::rollback(Error &error)
{
    if (m_writer->close(error)) {
        m_writer->rollback(error);
    }
}

IFileWriter *
FileWriterScope::writer() NANOEM_DECL_NOEXCEPT
{
    return m_writer;
}

IFileWriter *
FileWriterScope::release() NANOEM_DECL_NOEXCEPT
{
    IFileWriter *writer = m_writer;
    m_writer = nullptr;
    return writer;
}

MemoryReader::MemoryReader(const ByteArray *bytes)
    : m_bytesPtr(bytes)
    , m_offset(0)
{
}

MemoryReader::~MemoryReader() NANOEM_DECL_NOEXCEPT
{
}

nanoem_i32_t
MemoryReader::read(void *data, nanoem_i32_t size, Error & /* error */)
{
    nanoem_rsize_t rest = m_bytesPtr->size() - nanoem_rsize_t(m_offset),
                   actual = glm::min(static_cast<nanoem_rsize_t>(size), rest);
    if (actual > 0) {
        memcpy(data, m_bytesPtr->data() + m_offset, actual);
        m_offset += actual;
    }
    return Inline::saturateInt32(actual);
}

nanoem_rsize_t
MemoryReader::size()
{
    return m_bytesPtr->size();
}

nanoem_i64_t
MemoryReader::seek(nanoem_i64_t offset, SeekType whence, Error & /*error */)
{
    nanoem_i64_t ret = m_offset;
    switch (whence) {
    case kSeekTypeBegin:
        m_offset = offset;
        break;
    case kSeekTypeCurrent:
        m_offset = m_offset + offset;
        break;
    case kSeekTypeEnd:
        m_offset = m_offset + m_bytesPtr->size();
        break;
    }
    m_offset = glm::min(m_offset, static_cast<nanoem_i64_t>(m_bytesPtr->size()));
    return ret;
}

MemoryWriter::MemoryWriter(ByteArray *bytes)
    : m_bytesPtr(bytes)
    , m_offset(0)
{
    bytes->clear();
}

MemoryWriter::~MemoryWriter() NANOEM_DECL_NOEXCEPT
{
    m_bytesPtr = nullptr;
}

nanoem_i32_t
MemoryWriter::write(const void *data, nanoem_i32_t size, Error & /* error */)
{
    const nanoem_u8_t *dataPtr = static_cast<const nanoem_u8_t *>(data);
    m_bytesPtr->insert(m_bytesPtr->begin() + m_offset, dataPtr, dataPtr + size);
    m_offset += size;
    return size;
}

nanoem_i64_t
MemoryWriter::seek(nanoem_i64_t offset, SeekType whence, Error & /* error */)
{
    nanoem_i64_t ret = m_offset;
    switch (whence) {
    case kSeekTypeBegin:
        m_offset = offset;
        break;
    case kSeekTypeCurrent:
        m_offset = m_offset + offset;
        break;
    case kSeekTypeEnd:
        m_offset = m_offset + m_bytesPtr->size();
        break;
    }
    m_bytesPtr->reserve(static_cast<nanoem_rsize_t>(m_offset));
    return ret;
}

void
MemoryWriter::clear()
{
    m_bytesPtr->clear();
    m_offset = 0;
}

FileUtils::TransientPath::TransientPath()
    : m_handle(INTPTR_MAX)
    , m_valid(false)
{
}

FileUtils::TransientPath::TransientPath(const String &path)
    : m_path(path)
    , m_handle(INTPTR_MAX)
    , m_valid(false)
{
}

nanoem_u64_t
FileUtils::timestamp(const char *filePath) NANOEM_DECL_NOEXCEPT
{
    nanoem_u64_t value = 0;
#if BX_PLATFORM_WINDOWS
    MutableWideString newPath;
    StringUtils::getWideCharString(filePath, newPath);
    FILETIME time;
    GetFileTime(newPath.data(), nullptr, nullptr, &time);
    ULARGE_INTEGER ul;
    ul.HighPart = time.dwHighDateTime;
    ul.LowPart = time.dwLowDateTime;
    value = ul.QuadPart;
#elif BX_PLATFORM_OSX || BX_PLATFORM_IOS
    struct stat st;
    ::stat(filePath, &st);
    value = static_cast<nanoem_u64_t>(st.st_mtimespec.tv_sec) * 1000000000 + st.st_mtimespec.tv_nsec;
#endif
    return value;
}

nanoem_u64_t
FileUtils::timestamp(const URI &fileURI) NANOEM_DECL_NOEXCEPT
{
    return timestamp(fileURI.absolutePathConstString());
}

bool
FileUtils::exists(const char *filePath) NANOEM_DECL_NOEXCEPT
{
    bool exists;
#if BX_PLATFORM_WINDOWS
    MutableWideString newPath;
    StringUtils::getWideCharString(filePath, newPath);
    DWORD attributes = GetFileAttributesW(newPath.data());
    exists = attributes != INVALID_FILE_ATTRIBUTES;
#else
    int rc = ::access(filePath, F_OK);
    exists = (rc == 0);
#endif
    return exists;
}

bool
FileUtils::exists(const URI &fileURI) NANOEM_DECL_NOEXCEPT
{
    return fileURI.isEmpty() ? false : exists(fileURI.absolutePathConstString());
}

bool
FileUtils::deleteFile(const char *filePath)
{
    bool deleted;
#if BX_PLATFORM_WINDOWS
    MutableWideString newPath;
    StringUtils::getWideCharString(filePath, newPath);
    deleted = DeleteFileW(newPath.data()) != 0;
#else
    int rc = ::unlink(filePath);
    deleted = (rc == 0);
#endif
    return deleted;
}

bool
FileUtils::deleteFile(const URI &fileURI)
{
    return fileURI.isEmpty() ? false : deleteFile(fileURI.absolutePathConstString());
}

bool
FileUtils::createTransientFile(const String &source, TransientPath &destination)
{
    bool created = false;
#if BX_PLATFORM_WINDOWS
    MutableWideString newSourcePath;
    wchar_t workingDirectory[MAX_PATH], destinationPath[MAX_PATH];
    GetTempPathW(ARRAYSIZE(workingDirectory), workingDirectory);
    GetTempFileNameW(workingDirectory, L"nan03m", 0, destinationPath);
    StringUtils::getWideCharString(source.c_str(), newSourcePath);
    DeleteFileW(destinationPath);
    destination.m_valid = created = CreateHardLinkW(destinationPath, newSourcePath.data(), nullptr) != 0;
    MutableString path;
    StringUtils::getMultiBytesString(destinationPath, path);
    destination.m_path = String(path.data(), path.size());
    destination.m_handle = 1;
#elif BX_PLATFORM_OSX || BX_PLATFORM_IOS
    char suffix[] = "nan03m_XXXXXXXXXXXXXXXX";
    int fd = ::mkstemp(suffix);
    if (fd != -1) {
        char path[PATH_MAX];
        destination.m_handle = fd;
        if (::fcntl(fd, F_GETPATH, path) != -1) {
            ::unlink(path);
            created = destination.m_valid = ::link(source.c_str(), path) == 0;
            destination.m_path = path;
        }
    }
#elif BX_PLATFORM_LINUX
    char suffix[] = "nan03m_XXXXXX";
    int fd = ::mkstemp(suffix);
    if (fd != -1) {
        char proc[PATH_MAX], path[PATH_MAX];
        destination.m_handle = fd;
        StringUtils::format(proc, sizeof(proc), "/proc/self/fd/%d", fd);
        ::readlink(proc, path, sizeof(path));
        ::unlink(path);
        created = destination.m_valid = ::link(source.c_str(), path) == 0;
        destination.m_path = path;
    }
#else
    char *destinationPath = ::tempnam(nullptr, "nan03m_XXXXXX");
    int rc = ::link(source.c_str(), destinationPath);
    destination.m_path = destinationPath;
    free(destinationPath);
    created = destination.m_valid = (rc == 0);
#endif
    return created;
}

bool
FileUtils::deleteTransientFile(TransientPath &path)
{
    bool deleted = false;
#if !BX_PLATFORM_WINDOWS
    intptr_t &handle = path.m_handle;
    if (handle != INTPTR_MAX) {
        ::close(handle);
        handle = 0;
    }
#endif
    if (path.m_valid) {
        String &p = path.m_path;
        if (!p.empty()) {
            deleted = deleteFile(p.c_str());
            p = String();
        }
        path.m_valid = false;
    }
    return deleted;
}

bool
FileUtils::isLoadableExtension(const String &value, const StringSet &supported) NANOEM_DECL_NOEXCEPT
{
    char extension[8], *p = extension;
    bx::strCopy(extension, sizeof(extension), value.c_str(), Inline::saturateInt32(value.size()));
    while (*p) {
        *p = bx::toLower(*p);
        p++;
    }
    return supported.find(extension) != supported.end();
}

void
FileUtils::canonicalizePathSeparator(MutableString &output)
{
    const String source(output.data());
    String dest;
    canonicalizePathSeparator(source, dest);
    output.clear();
    output = MutableString(dest.c_str(), dest.c_str() + dest.size() + 1);
}

void
FileUtils::canonicalizePathSeparator(const String &path, String &output)
{
    const char *ptr = path.c_str();
    nanoem_rsize_t size = path.size();
    if (!output.empty()) {
        output = String();
    }
    output.reserve(path.size());
    for (nanoem_rsize_t i = 0; i < size; i++) {
        const nanoem_u8_t c = static_cast<nanoem_u8_t>(ptr[i]);
        if (c == 0xc2 && i < size - 1) {
            const nanoem_u8_t c2 = static_cast<nanoem_u8_t>(ptr[++i]);
            if (c2 == 0xa5) {
                output.append("/");
            }
        }
        else if (c == '\\') {
            if (i < size && ptr[i + 1] == '\\') {
                output.append("/");
                i++;
            }
            else {
                output.append("/");
            }
        }
        else {
            output.append(&ptr[i], &ptr[i + 1]);
        }
    }
}

String
FileUtils::canonicalizePath(const String &originPath, const String &filename)
{
    int cdup = 0;
    const char *p = filename.c_str(), *s = p;
    while (*p) {
        if (StringUtils::equals(p, "../", 3)) {
            cdup++;
            p += 3;
            s += 3;
        }
        else if (StringUtils::equals(p, "./", 2)) {
            p += 2;
            s += 2;
        }
        else {
            p++;
        }
    }
    String newPath(originPath);
    while (--cdup >= 0) {
        newPath = URI::stringByDeletingLastPathComponent(newPath);
    }
    if (StringUtils::equals(newPath.c_str(), "./", 2)) {
        newPath = String(newPath.c_str() + 2, newPath.size() - 2);
    }
    if (!newPath.empty()) {
        newPath.append("/");
    }
    newPath.append(s);
    return newPath;
}

String
FileUtils::relativePath(const String &value, const String &basePath)
{
    struct PathComponent {
        static void
        split(const String &value, StringList &components, bool &hasDriveLetter)
        {
            const char *q = value.c_str();
            if (*q != '/') {
                const char *p = strchr(q, '/');
                const String part(q, p - q);
                if (!part.empty()) {
                    components.push_back(part);
                }
                q = p;
            }
            else {
                hasDriveLetter = false;
            }
            while (const char *p = strchr(q + 1, '/')) {
                const String part(q + 1, p - (q + 1));
                if (!part.empty()) {
                    components.push_back(part);
                }
                q = p;
            }
            const String part(q + 1);
            if (!part.empty()) {
                components.push_back(part);
            }
        }
    };
    String newPath;
    if (!value.empty() && !basePath.empty()) {
        StringList valuePathComponents, basePathComponents;
        bool hasDriveLetter = true;
        PathComponent::split(value, valuePathComponents, hasDriveLetter);
        PathComponent::split(basePath, basePathComponents, hasDriveLetter);
        nanoem_rsize_t numMatched = 0;
        for (nanoem_rsize_t numComponents = glm::min(valuePathComponents.size(), basePathComponents.size());
             numMatched < numComponents; numMatched++) {
            if (!(valuePathComponents[numMatched] == basePathComponents[numMatched])) {
                break;
            }
        }
        if (!(hasDriveLetter && numMatched == 0)) {
            for (nanoem_rsize_t i = 0, numCount = basePathComponents.size() - numMatched; i < numCount; i++) {
                newPath.append("../");
            }
            for (nanoem_rsize_t i = numMatched, numCount = valuePathComponents.size(); i < numCount; i++) {
                newPath.append(valuePathComponents[i].c_str());
                if (i < numCount - 1) {
                    newPath.append("/");
                }
            }
        }
    }
    return newPath;
}

nanoem_i32_t
FileUtils::read(FileReaderScope &scope, void *data, nanoem_rsize_t size, Error &error)
{
    return read(scope.reader(), data, size, error);
}
nanoem_i32_t
FileUtils::read(FileReaderScope &scope, ByteArray &bytes, Error &error)
{
    return read(scope.reader(), bytes, error);
}

nanoem_i32_t
FileUtils::read(IReader *reader, void *data, nanoem_rsize_t size, Error &error)
{
    nanoem_i32_t read = -1;
    if (reader) {
        read = reader->read(data, Inline::saturateInt32(size), error);
    }
    return read;
}

nanoem_i32_t
FileUtils::read(IReader *reader, ByteArray &bytes, Error &error)
{
    nanoem_i32_t read = -1;
    if (reader) {
        bytes.resize(reader->size());
        read = reader->read(bytes.data(), Inline::saturateInt32(bytes.size()), error);
    }
    return read;
}

nanoem_i32_t
FileUtils::write(IWriter *writer, const void *data, nanoem_rsize_t size, Error &error)
{
    return writer ? writer->write(data, Inline::saturateInt32(size), error) : -1;
}

nanoem_i32_t
FileUtils::write(IWriter *writer, const ByteArray &bytes, Error &error)
{
    return write(writer, bytes.data(), bytes.size(), error);
}

nanoem_i32_t
FileUtils::write(IWriter *writer, const String &string, Error &error)
{
    return write(writer, string.c_str(), string.size(), error);
}

nanoem_i32_t
FileUtils::write(IWriter *writer, const nanoem_buffer_t *buffer, Error &error)
{
    return write(writer, nanoemBufferGetDataPtr(buffer), nanoemBufferGetLength(buffer), error);
}

IFileReader *
FileUtils::createFileReader(const ITranslator *translator)
{
#if BX_PLATFORM_WINDOWS
    return nanoem_new(Win32FileReader(translator));
#else
    return nanoem_new(PosixFileReader(translator));
#endif
}

IFileWriter *
FileUtils::createFileWriter()
{
#if BX_PLATFORM_WINDOWS
    return nanoem_new(Win32FileWriter);
#else
    return nanoem_new(PosixFileWriter);
#endif
}

void
FileUtils::destroyFileReader(IFileReader *reader)
{
    if (reader) {
        Error error;
        reader->close(error);
        nanoem_delete(reader);
    }
}

void
FileUtils::destroyFileWriter(IFileWriter *writer)
{
    if (writer) {
        Error error;
        writer->close(error);
        nanoem_delete(writer);
    }
}

} /* namespace nanoem */
