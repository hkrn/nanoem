/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#pragma once
#ifndef NANOEM_EMAPP_STRINGUTILS_H_
#define NANOEM_EMAPP_STRINGUTILS_H_

#include "emapp/Forward.h"

#ifndef NANOEM_DECL_PRINTF_LIKE
#if defined(__clang__)
#define NANOEM_DECL_PRINTF_LIKE(a, b) __attribute__((__format__(__printf__, (a), (b))))
#elif defined(__gcc__)
#define NANOEM_DECL_PRINTF_LIKE(a, b) __attribute__((format(printf, (a), (b))))
#else
#define NANOEM_DECL_PRINTF_LIKE(a, b)
#endif
#endif /* NANOEM_DECL_PRINTF_LIKE */

namespace nanoem {

class StringUtils NANOEM_DECL_SEALED : private NonCopyable {
public:
    class UnicodeStringScope : private NonCopyable {
    public:
        UnicodeStringScope(nanoem_unicode_string_factory_t *factory);
        ~UnicodeStringScope() NANOEM_DECL_NOEXCEPT;

        nanoem_unicode_string_t *value();
        void reset(nanoem_unicode_string_t *value);

    private:
        void destroy();

        nanoem_unicode_string_factory_t *m_factory;
        nanoem_unicode_string_t *m_value;
    };

    static nanoem_rsize_t length(const char *value) NANOEM_DECL_NOEXCEPT;
    static nanoem_rsize_t length(const char *value, nanoem_rsize_t max) NANOEM_DECL_NOEXCEPT;
    static const char *indexOf(const String &value, char needle) NANOEM_DECL_NOEXCEPT;
    static const char *indexOf(const char *value, char needle) NANOEM_DECL_NOEXCEPT;
    static char *indexOf(char *value, char needle) NANOEM_DECL_NOEXCEPT;
    static int compare(const char *left, const char *right) NANOEM_DECL_NOEXCEPT;
    static int compare(const char *left, const char *right, size_t length) NANOEM_DECL_NOEXCEPT;
    static bool equals(const char *left, const char *right) NANOEM_DECL_NOEXCEPT;
    static bool equals(const char *left, const char *right, size_t length) NANOEM_DECL_NOEXCEPT;
    static bool equalsIgnoreCase(const char *left, const char *right) NANOEM_DECL_NOEXCEPT;
    static bool equalsIgnoreCase(const char *left, const char *right, size_t length) NANOEM_DECL_NOEXCEPT;
    static bool hasPrefix(const char *value, const char *prefix) NANOEM_DECL_NOEXCEPT;
    static bool hasPrefix(const char *value, const char *prefix, size_t length) NANOEM_DECL_NOEXCEPT;
    static char *trimLineFeed(char *s) NANOEM_DECL_NOEXCEPT;
    static nanoem_f32_t parseFloat(const String &input, char **ptr) NANOEM_DECL_NOEXCEPT;
    static nanoem_f32_t parseFloat(const char *input, char **ptr) NANOEM_DECL_NOEXCEPT;
    static unsigned int parseUnsignedInteger(const String &input, char **ptr) NANOEM_DECL_NOEXCEPT;
    static unsigned int parseUnsignedInteger(const char *input, char **ptr) NANOEM_DECL_NOEXCEPT;
    static int parseInteger(const String &input, char **ptr) NANOEM_DECL_NOEXCEPT;
    static int parseInteger(const char *input, char **ptr) NANOEM_DECL_NOEXCEPT;
    static void parseComma(const char **stringPtr) NANOEM_DECL_NOEXCEPT;
    static int format(String &dst, const char *fmt, ...) NANOEM_DECL_PRINTF_LIKE(2, 3);
    static int formatVA(String &dst, const char *fmt, va_list ap) NANOEM_DECL_PRINTF_LIKE(2, 0);
    static int format(char *buffer, nanoem_rsize_t size, const char *fmt, ...) NANOEM_DECL_PRINTF_LIKE(3, 4);
    static int formatVA(char *buffer, nanoem_rsize_t size, const char *fmt, va_list ap) NANOEM_DECL_PRINTF_LIKE(3, 0);
    static void formatDateTimeUTC(char *buffer, nanoem_rsize_t size, const char *fmt);
    static void formatDateTimeLocal(char *buffer, nanoem_rsize_t size, const char *fmt);
    static String skipWhiteSpaces(const char *p);
    static String substitutedPrefixString(const char *prefix, const char *name);
    static String toUpperCase(const String &value);
    static String toUpperCase(const char *value);
    static String toLowerCase(const String &value);
    static String toLowerCase(const char *value);
    static void copyString(char *dest, const char *src, size_t max);
    static void copyString(const String &src, MutableString &dest);
    static char *cloneString(const char *source, MutableString &dest);
    static void getUtf8String(const char *value, nanoem_rsize_t length, nanoem_codec_type_t codec,
        nanoem_unicode_string_factory_t *factory, String &result);
    static void getUtf8String(
        const String &value, nanoem_codec_type_t codec, nanoem_unicode_string_factory_t *factory, String &result);
    static void getUtf8String(
        const nanoem_unicode_string_t *s, nanoem_unicode_string_factory_t *factory, String &result);
    static void getUtf8String(const nanoem_unicode_string_t *s, nanoem_unicode_string_factory_t *factory,
        nanoem_status_t *status, String &result);
    static bool tryGetString(nanoem_unicode_string_factory_t *factory, const String &value, UnicodeStringScope &scope);
    static bool tryGetString(
        nanoem_unicode_string_factory_t *factory, const char *value, size_t length, UnicodeStringScope &scope);
#if BX_PLATFORM_WINDOWS
    static void getWideCharString(LPCSTR source, MutableWideString &dest, int codepage = CP_UTF8);
    static void getMultiBytesString(LPCWSTR source, MutableString &dest, int codepage = CP_UTF8);
#endif
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_FILEUTILS_H_ */
