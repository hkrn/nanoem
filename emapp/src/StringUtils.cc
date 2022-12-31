/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#if defined(_WIN32)
#include <stdio.h>
#else
#include "stb/stb_sprintf.h"
#endif

#include <ctype.h>
#include <limits.h>
#include <time.h>

namespace nanoem {

StringUtils::UnicodeStringScope::UnicodeStringScope(nanoem_unicode_string_factory_t *factory)
    : m_factory(factory)
    , m_value(nullptr)
{
}

StringUtils::UnicodeStringScope::~UnicodeStringScope() NANOEM_DECL_NOEXCEPT
{
    destroy();
}

nanoem_unicode_string_t *
StringUtils::UnicodeStringScope::value()
{
    return m_value;
}

void
StringUtils::UnicodeStringScope::reset(nanoem_unicode_string_t *value)
{
    destroy();
    m_value = value;
}

void
StringUtils::UnicodeStringScope::destroy()
{
    if (m_factory && m_value) {
        nanoemUnicodeStringFactoryDestroyString(m_factory, m_value);
    }
}

nanoem_rsize_t
StringUtils::length(const char *value) NANOEM_DECL_NOEXCEPT
{
    return length(value, INT_MAX);
}

nanoem_rsize_t
StringUtils::length(const char *value, nanoem_rsize_t max) NANOEM_DECL_NOEXCEPT
{
    return bx::strLen(value, Inline::saturateInt32(max));
}

const char *
StringUtils::indexOf(const String &value, char needle) NANOEM_DECL_NOEXCEPT
{
    return indexOf(value.c_str(), needle);
}

const char *
StringUtils::indexOf(const char *value, char needle) NANOEM_DECL_NOEXCEPT
{
    return strchr(value, needle);
}

char *
StringUtils::indexOf(char *value, char needle) NANOEM_DECL_NOEXCEPT
{
    return strchr(value, needle);
}

int
StringUtils::compare(const char *left, const char *right) NANOEM_DECL_NOEXCEPT
{
    return compare(left, right, INT32_MAX);
}

int
StringUtils::compare(const char *left, const char *right, size_t length) NANOEM_DECL_NOEXCEPT
{
    return bx::strCmp(left, right, Inline::saturateInt32(length));
}

bool
StringUtils::equals(const char *left, const char *right) NANOEM_DECL_NOEXCEPT
{
    return equals(left, right, INT32_MAX);
}

bool
StringUtils::equals(const char *left, const char *right, size_t length) NANOEM_DECL_NOEXCEPT
{
    return compare(left, right, length) == 0;
}

bool
StringUtils::equalsIgnoreCase(const char *left, const char *right) NANOEM_DECL_NOEXCEPT
{
    return equalsIgnoreCase(left, right, INT32_MAX);
}

bool
StringUtils::equalsIgnoreCase(const char *left, const char *right, size_t length) NANOEM_DECL_NOEXCEPT
{
    return bx::strCmpI(left, right, Inline::saturateInt32(length)) == 0;
}

bool
StringUtils::hasPrefix(const char *value, const char *prefix) NANOEM_DECL_NOEXCEPT
{
    return hasPrefix(value, prefix, length(prefix));
}

bool
StringUtils::hasPrefix(const char *value, const char *prefix, size_t length) NANOEM_DECL_NOEXCEPT
{
    return bx::strCmp(value, prefix, Inline::saturateInt32(length)) == 0;
}

char *
StringUtils::trimLineFeed(char *s) NANOEM_DECL_NOEXCEPT
{
    unsigned char *p = reinterpret_cast<unsigned char *>(s);
    while (*p++) {
        if (*p == 0xa) {
            *p = '\0';
        }
    }
    return s;
}

nanoem_f32_t
StringUtils::parseFloat(const String &input, char **ptr) NANOEM_DECL_NOEXCEPT
{
    return parseFloat(input.c_str(), ptr);
}

nanoem_f32_t
StringUtils::parseFloat(const char *input, char **ptr) NANOEM_DECL_NOEXCEPT
{
    return nanoem_f32_t(::strtod(input, ptr));
}

unsigned int
StringUtils::parseUnsignedInteger(const String &input, char **ptr) NANOEM_DECL_NOEXCEPT
{
    return parseUnsignedInteger(input.c_str(), ptr);
}

unsigned int
StringUtils::parseUnsignedInteger(const char *input, char **ptr) NANOEM_DECL_NOEXCEPT
{
    return Inline::saturateInt32U(::strtoul(input, ptr, 10));
}

int
StringUtils::parseInteger(const String &input, char **ptr) NANOEM_DECL_NOEXCEPT
{
    return parseInteger(input.c_str(), ptr);
}

int
StringUtils::parseInteger(const char *input, char **ptr) NANOEM_DECL_NOEXCEPT
{
    return Inline::saturateInt32(::strtol(input, ptr, 10));
}

void
StringUtils::parseComma(const char **stringPtr) NANOEM_DECL_NOEXCEPT
{
    bx::StringView p = bx::strLTrimSpace(*stringPtr);
    if (*p.getPtr() == ',') {
        p.set(p.getPtr() + 1);
    }
    *stringPtr = p.getPtr();
}

String
StringUtils::skipWhiteSpaces(const char *p)
{
    // use own isspace definition due to isspace assertion
    struct SpaceTrimmer {
        static inline bool
        isSpace(int c)
        {
            return c == ' ' || c == '\t' || c == '\r' || c == '\n';
        }
        static inline const char *
        prefix(const char *_p)
        {
            for (; isSpace(*_p); ++_p) {
            };
            return _p;
        }
    };
    const char *q = SpaceTrimmer::prefix(p), *s = q + length(q) - 1;
    while (SpaceTrimmer::isSpace(*s)) {
        s--;
    }
    size_t len = size_t(s - q + 1);
    return String(q, len);
}

int
StringUtils::format(String &dst, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int estimatedSize = formatVA(dst, fmt, ap);
    va_end(ap);
    return estimatedSize;
}

int
StringUtils::formatVA(String &dst, const char *fmt, va_list ap)
{
    char stackBuffer[4096];
    const int stackBufferSize = Inline::saturateInt32(sizeof(stackBuffer));
    va_list ap2;
    va_copy(ap2, ap);
#if defined(STB_SPRINTF_H_INCLUDE)
    int estimatedSize = stbsp_vsnprintf(stackBuffer, stackBufferSize, fmt, ap);
#else
    int estimatedSize = vsnprintf(stackBuffer, stackBufferSize, fmt, ap);
#endif
    if (estimatedSize >= stackBufferSize) {
        MutableString heapBuffer(estimatedSize + 1);
#if defined(STB_SPRINTF_H_INCLUDE)
        stbsp_vsnprintf(heapBuffer.data(), heapBuffer.size(), fmt, ap2);
#else
        vsnprintf(heapBuffer.data(), heapBuffer.size(), fmt, ap2);
#endif
        dst.append(heapBuffer.data(), heapBuffer.data() + estimatedSize);
    }
    else {
        dst.append(stackBuffer, stackBuffer + estimatedSize);
    }
    return estimatedSize;
}

int
StringUtils::format(char *buffer, nanoem_rsize_t size, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int estimatedSize = formatVA(buffer, size, fmt, ap);
    va_end(ap);
    return estimatedSize;
}

int
StringUtils::formatVA(char *buffer, nanoem_rsize_t size, const char *fmt, va_list ap)
{
#if defined(STB_SPRINTF_H_INCLUDE)
    return stbsp_vsnprintf(buffer, Inline::saturateInt32(size), fmt, ap);
#else
    return vsnprintf(buffer, size, fmt, ap);
#endif
}

void
StringUtils::formatDateTimeUTC(char *buffer, nanoem_rsize_t size, const char *fmt)
{
    time_t timer = ::time(nullptr);
    struct tm *tm = ::gmtime(&timer);
    ::strftime(buffer, size, fmt, tm);
}

void
StringUtils::formatDateTimeLocal(char *buffer, nanoem_rsize_t size, const char *fmt)
{
    time_t timer = ::time(nullptr);
    struct tm *tm = ::localtime(&timer);
    ::strftime(buffer, size, fmt, tm);
}

String
StringUtils::substitutedPrefixString(const char *prefix, const char *name)
{
    String newName;
    newName.append(prefix);
    newName.append(name + length(prefix));
    return newName;
}

String
StringUtils::toUpperCase(const String &value)
{
    return toUpperCase(value.c_str());
}

String
StringUtils::toUpperCase(const char *value)
{
    MutableString mut;
    mut.reserve(length(value));
    while (char c = *value++) {
        mut.push_back(toupper(c));
    }
    return String(mut.data(), mut.size());
}

String
StringUtils::toLowerCase(const String &value)
{
    return toLowerCase(value.c_str());
}

String
StringUtils::toLowerCase(const char *value)
{
    MutableString mut;
    mut.reserve(length(value));
    while (char c = *value++) {
        mut.push_back(tolower(c));
    }
    return String(mut.data(), mut.size());
}

void
StringUtils::copyString(char *dest, const char *src, size_t max)
{
    bx::strCopy(dest, Inline::saturateInt32(max), src);
}

void
StringUtils::copyString(const String &src, MutableString &dest)
{
    dest.resize(src.size() + 1);
    copyString(dest.data(), src.c_str(), dest.size());
}

char *
StringUtils::cloneString(const char *source, MutableString &dest)
{
    dest.resize(length(source) + 1);
    copyString(dest.data(), source, dest.size());
    return dest.data();
}

void
StringUtils::getUtf8String(const char *value, nanoem_rsize_t length, nanoem_codec_type_t codec,
    nanoem_unicode_string_factory_t *factory, String &result)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_t *s = nanoemUnicodeStringFactoryCreateStringWithEncoding(
        factory, reinterpret_cast<const nanoem_u8_t *>(value), length, codec, &status);
    if (status == NANOEM_STATUS_SUCCESS) {
        getUtf8String(s, factory, result);
        nanoemUnicodeStringFactoryDestroyString(factory, s);
    }
}

void
StringUtils::getUtf8String(
    const String &value, nanoem_codec_type_t codec, nanoem_unicode_string_factory_t *factory, String &result)
{
    getUtf8String(value.c_str(), value.size(), codec, factory, result);
}

void
StringUtils::getUtf8String(const nanoem_unicode_string_t *s, nanoem_unicode_string_factory_t *factory, String &result)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    getUtf8String(s, factory, &status, result);
}

void
StringUtils::getUtf8String(
    const nanoem_unicode_string_t *s, nanoem_unicode_string_factory_t *factory, nanoem_status_t *status, String &result)
{
    nanoem_parameter_assert(factory, "must not be nullptr");
    nanoem_rsize_t length;
    nanoem_u8_t buffer[Inline::kNameStackBufferSize];
    if (s) {
        if (nanoemUnicodeStringGetLength(s) <= Inline::kNameStackBufferSize) {
            nanoemUnicodeStringFactoryToUtf8OnStackEXT(factory, s, &length, buffer, sizeof(buffer), status);
            if (*status == NANOEM_STATUS_SUCCESS) {
                result = String(reinterpret_cast<const char *>(buffer), length);
            }
        }
        if (result.empty()) {
            nanoem_u8_t *ptr = nanoemUnicodeStringFactoryGetByteArray(factory, s, &length, status);
            result = String(reinterpret_cast<const char *>(ptr), length);
            nanoemUnicodeStringFactoryDestroyByteArray(factory, ptr);
        }
    }
}

bool
StringUtils::tryGetString(nanoem_unicode_string_factory_t *factory, const String &value, UnicodeStringScope &scope)
{
    return tryGetString(factory, value.c_str(), value.size(), scope);
}

bool
StringUtils::tryGetString(
    nanoem_unicode_string_factory_t *factory, const char *value, size_t length, UnicodeStringScope &scope)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_t *s =
        nanoemUnicodeStringFactoryCreateString(factory, reinterpret_cast<const nanoem_u8_t *>(value), length, &status);
    bool succeeded = status == NANOEM_STATUS_SUCCESS;
    if (succeeded) {
        scope.reset(s);
    }
    return succeeded;
}

#if BX_PLATFORM_WINDOWS

void
StringUtils::getWideCharString(LPCSTR source, MutableWideString &dest, int codepage)
{
    if (source) {
        const int len = Inline::saturateInt32(length(source));
        dest.resize(MultiByteToWideChar(codepage, 0, source, len, nullptr, 0) + 1);
        MultiByteToWideChar(codepage, 0, source, len, dest.data(), Inline::saturateInt32(dest.size()));
    }
}

void
StringUtils::getMultiBytesString(LPCWSTR source, MutableString &dest, int codepage)
{
    if (source) {
        const int length = Inline::saturateInt32(wcslen(source));
        dest.resize(WideCharToMultiByte(codepage, 0, source, length, nullptr, 0, nullptr, nullptr) + 1);
        WideCharToMultiByte(
            codepage, 0, source, length, dest.data(), Inline::saturateInt32(dest.size()), nullptr, nullptr);
    }
}

#endif /* BX_PLATFORM_WINDOWS */

} /* namespace nanoem */
