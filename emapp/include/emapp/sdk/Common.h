/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef EMAPP_PLUGIN_SDK_COMMON_H_
#define EMAPP_PLUGIN_SDK_COMMON_H_

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L /* C11 */
#define __STDC_WANT_LIB_EXT1__ 1 /* define for rsize_t */
#endif

#include <stddef.h> /* size_t */
#ifdef _WIN32
#include <windows.h>
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L /* C11 */
#ifndef NANOEM_STATIC_ASSERT
#define NANOEM_STATIC_ASSERT _Static_assert
#endif /* NANOEM_STATIC_ASSERT */
typedef rsize_t nanoem_rsize_t;
#elif !defined(__cplusplus)
#ifndef NANOEM_STATIC_ASSERT
#define __NANOEM_SA_CONCAT(a, b) __NANOEM_SA_CONCAT2(a, b)
#define __NANOEM_SA_CONCAT2(a, b) a##b
#define NANOEM_STATIC_ASSERT(cond, message)                                                                            \
    enum {                                                                                                             \
        __NANOEM_SA_CONCAT(NANOEM_STATIC_ASSERT_LINE_AT_, __LINE__) = sizeof(struct __NANOEM_SA_CONCAT(                \
            __nanoem_static_assert_line_at_, __LINE__) { int static_assertion_failed[(cond) ? 1 : -1]; })              \
    }
#endif /* NANOEM_STATIC_ASSERT */
typedef size_t nanoem_rsize_t;
#else
#define NANOEM_STATIC_ASSERT(cond, message)
typedef size_t nanoem_rsize_t;
#endif /* __STDC_VERSION__ >= 201112L */

#ifndef NANOEM_DECL_ENUM
#if defined(__cplusplus) && __cplusplus >= 201103L
#define NANOEM_DECL_ENUM(type, name) enum name : type
#else
#define NANOEM_DECL_ENUM(type, name)                                                                                   \
    typedef type name;                                                                                                 \
    enum
#endif /*__STDC_VERSION__ >= 201112L */
#endif /* NANOEM_DECL_ENUM */

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L /* C99 */
#include <stdint.h>
#ifndef NANOEM_DECL_INLINE
#define NANOEM_DECL_INLINE inline
#endif /* NANOEM_DECL_INLINE */
typedef uint8_t nanoem_u8_t;
typedef int16_t nanoem_i16_t;
typedef uint16_t nanoem_u16_t;
typedef int32_t nanoem_i32_t;
typedef uint32_t nanoem_u32_t;
typedef int64_t nanoem_i64_t;
typedef uint64_t nanoem_u64_t;
#elif defined(_MSC_VER) && _MSC_VER >= 1300
typedef unsigned __int8 nanoem_u8_t;
typedef signed __int16 nanoem_i16_t;
typedef unsigned __int16 nanoem_u16_t;
typedef signed __int32 nanoem_i32_t;
typedef unsigned __int32 nanoem_u32_t;
typedef signed __int64 nanoem_i64_t;
typedef unsigned __int64 nanoem_u64_t;
#ifndef NANOEM_DECL_INLINE
#define NANOEM_DECL_INLINE __inline
#endif /* NANOEM_DECL_INLINE */
#else /* defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L */
typedef unsigned char nanoem_u8_t;
typedef short nanoem_i16_t;
typedef unsigned short nanoem_u16_t;
typedef int nanoem_i32_t;
typedef unsigned int nanoem_u32_t;
typedef long long nanoem_i64_t;
typedef unsigned long long nanoem_u64_t;
#ifndef NANOEM_DECL_INLINE
#define NANOEM_DECL_INLINE
#endif /* NANOEM_DECL_INLINE */
#endif /* defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L */

typedef int nanoem_bool_t;
typedef float nanoem_f32_t;
typedef double nanoem_f64_t;

typedef int nanoem_bool_t;
typedef float nanoem_f32_t;
typedef double nanoem_f64_t;

/* utility macros */
#ifdef NANOEM_ENABLE_BRANCH_PREDICTION
#ifndef nanoem_likely
#define nanoem_likely(expr) __builtin_expected(!!(expr), 1)
#endif /* nanoem_likely */
#ifndef nanoem_unlikely
#define nanoem_unlikely(expr) __builtin_expected(!!(expr), 0)
#endif /* nanoem_unlikely */
#else
#ifndef nanoem_likely
#define nanoem_likely(expr) (expr)
#endif /* nanoem_likely */
#ifndef nanoem_unlikely
#define nanoem_unlikely(expr) (expr)
#endif /* nanoem_unlikely */
#endif /* NANOEM_ENABLE_BRANCH_PREDICTION */

#ifndef nanoem_is_null
#define nanoem_is_null(cond) (nanoem_unlikely((cond) == NULL))
#endif /* nanoem_is_null */
#ifndef nanoem_is_not_null
#define nanoem_is_not_null(cond) (nanoem_likely((cond) != NULL))
#endif /* nanoem_is_not_null */
#ifndef nanoem_mark_unused
#define nanoem_mark_unused(cond)                                                                                       \
    for (;;) {                                                                                                         \
        (void) (0 ? (void) (cond) : (void) 0);                                                                         \
        break;                                                                                                         \
    }
#endif /* nanoem_mark_unused */
#ifndef nanoem_fourcc
#define nanoem_fourcc(a, b, c, d) ((a) << 0 | (b) << 8 | (c) << 16 | (d) << 24)
#endif /* nanoem_fourcc */

#if defined(NANOEM_BUILDING_DLL)
#ifdef __cplusplus
#define NANOEM_DECL_API_PREFIX extern "C"
#else
#define _NANOEM_DECL_API extern
#endif /* __cplusplus */
#if defined(_WIN32)
#ifdef NANOEM_DLL_EXPORTS
#define NANOEM_DECL_API NANOEM_DECL_API_PREFIX __declspec(dllexport)
#else
#define NANOEM_DECL_API NANOEM_DECL_API_PREFIX __declspec(dllimport)
#endif
#elif defined(__GNUC__)
#ifdef __cplusplus
#define NANOEM_DECL_API __attribute__((visibility("default"))) _NANOEM_DECL_API
#else
#define NANOEM_DECL_API __attribute__((visibility("default"))) _NANOEM_DECL_API
#endif /* __cplusplus */
#endif
#endif /* NANOEM_BUILDING_DLL */

#ifndef NANOEM_DECL_API
#ifdef __cplusplus
#define NANOEM_DECL_API extern "C"
#else
#define NANOEM_DECL_API extern
#endif /* __cplusplus */
#endif /* NANOEM_DECL_API */

#ifndef NANOEM_RSIZE_MAX
#ifdef RSIZE_MAX
#define NANOEM_RSIZE_MAX RSIZE_MAX
#else
#define NANOEM_RSIZE_MAX ((~(size_t) (0)) >> 1)
#endif /* RSIZE_MAX */
#endif /* NANOEM_RSIZE_MAX */

#ifndef APIENTRY
#define APIENTRY
#endif /* APIENTRY */

/**
 * \defgroup emapp
 * @{
 */

/**
 * \defgroup emapp_plugin_common nanoem Plugin Common Definition
 * @{
 */

NANOEM_DECL_ENUM(int, nanoem_application_plugin_status_t) { NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON = -3,
    NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION = -2, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT = -1,
    NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS = 0 };

#define NANOEM_APPLICATION_PLUGIN_MAKE_ABI_VERSION(major, minor) ((major) << 16 | (minor))

#ifdef __cplusplus

#include <stdint.h>
#include <string.h>
#if !defined(NDEBUG)
#include <stdarg.h>
#include <stdio.h>
#endif

/**
 * \brief
 *
 * \param status_ptr
 */
static inline void
nanoem_application_plugin_status_assign_success(nanoem_application_plugin_status_t *&status_ptr)
{
    if (nanoem_likely(status_ptr)) {
        *status_ptr = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    }
}

/**
 * \brief
 *
 * \param status_ptr
 * \param value
 */
static inline void
nanoem_application_plugin_status_assign_error(
    nanoem_application_plugin_status_t *&status_ptr, nanoem_application_plugin_status_t value)
{
    if (nanoem_likely(status_ptr && value != NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS)) {
        *status_ptr = value;
    }
}

namespace nanoem {
namespace application {
namespace plugin {

class StringUtils {
public:
    /**
     * \brief
     *
     * \param left
     * \param right
     * \return true
     * \return false
     */
    static inline bool
    equals(const char *left, const char *right)
    {
        return nanoem_likely(left && right) && strcmp(left, right) == 0;
    }

private:
    StringUtils();
    ~StringUtils();
};

class Inline {
public:
    /**
     * \brief
     *
     * \tparam T
     * \param result
     * \param value
     * \param size
     * \param status
     */
    template <typename T>
    static inline void
    assignOption(T result, void *value, nanoem_u32_t *size, nanoem_application_plugin_status_t *status)
    {
        bool succeeded = true;
        if (nanoem_likely(value && size)) {
            *static_cast<T *>(value) = result;
            *size = Inline::saturateInt32U(sizeof(T));
        }
        else if (size) {
            *size = Inline::saturateInt32U(sizeof(T));
        }
        else {
            succeeded = false;
        }
        nanoem_application_plugin_status_assign_error(status,
            succeeded ? NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS : NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }

    /**
     * \brief
     *
     * \tparam T
     * \param value
     * \param size
     * \param status
     * \return true
     * \return false
     */
    template <typename T>
    static inline bool
    validateArgument(const void *value, nanoem_rsize_t size, nanoem_application_plugin_status_t *status)
    {
        bool succeeded = false;
        if (nanoem_likely(size == sizeof(T) && value)) {
            succeeded = true;
        }
        else {
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
        }
        return succeeded;
    }

    /**
     * \brief
     *
     * \param value
     * \return int
     */
    static inline int
    saturateInt32(size_t value)
    {
        return static_cast<int>(nanoem_likely(value < INT32_MAX) ? value : INT32_MAX);
    }

    /**
     * \brief
     *
     * \param value
     * \return nanoem_u32_t
     */
    static inline nanoem_u32_t
    saturateInt32U(size_t value)
    {
        return static_cast<nanoem_u32_t>(nanoem_likely(value < UINT32_MAX) ? value : UINT32_MAX);
    }

    /**
     * \brief
     *
     * \param value
     * \return nanoem_u32_t
     */
    static inline nanoem_u32_t
    roundInt32(int value)
    {
        return static_cast<nanoem_u32_t>(nanoem_likely(value >= 0) ? value : 0);
    }

    /**
     * \brief
     *
     * \param format
     * \param ...
     */
    static inline void
    debugPrintf(const char *format, ...)
    {
#if !defined(NDEBUG)
#if defined(_WIN32)
        char source[1024];
        va_list args;
        va_start(args, format);
        int sourceLength = vsprintf_s(source, sizeof(source), format, args);
        va_end(args);
        wchar_t dest[1024];
        int destLength = MultiByteToWideChar(CP_UTF8, 0, source, sourceLength, nullptr, 0);
        MultiByteToWideChar(CP_UTF8, 0, source, sourceLength, dest, destLength);
        dest[destLength] = 0;
        OutputDebugStringW(dest);
#else
        va_list args;
        va_start(args, format);
        fprintf(stderr, format, args);
        va_end(args);
#endif /* _WIN32 */
#else /* NDEBUG */
        (void) format;
#endif /* NDEBUG */
    }

private:
    Inline();
    ~Inline();
};

} /* namespace plugin */
} /* namespace application */
} /* namespace nanoem */

#else /* __cplusplus */

#define nanoem_application_plugin_status_assign_success(status_ptr)                                                    \
    if (status_ptr) {                                                                                                  \
        *status_ptr = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;                                                        \
    }

#define nanoem_application_plugin_status_assign_error(status_ptr, value)                                               \
    if (status_ptr && (value) != NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS) {                                           \
        *status_ptr = (value);                                                                                         \
    }

#endif /* __cplusplus */

typedef nanoem_u32_t nanoem_frame_index_t;

/** @} */

/** @} */

#endif /* EMAPP_PLUGIN_SDK_COMMON_H_ */
