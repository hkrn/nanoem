/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#ifndef UNDO_H_
#define UNDO_H_

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L /* C11 */
#define __STDC_WANT_LIB_EXT1__ 1 /* define for rsize_t */
#endif

#include <stddef.h> /* size_t */
#ifdef _WIN32
#include <windows.h>
#endif

/**
 * \defgroup undo undo
 * @{
 */

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L /* C11 */
#ifndef UNDO_STATIC_ASSERT
#define UNDO_STATIC_ASSERT _Static_assert
#endif /* UNDO_STATIC_ASSERT */
typedef rsize_t undo_rsize_t;
#elif !defined(__cplusplus)
#ifndef UNDO_STATIC_ASSERT
#define __UNDO_SA_CONCAT(a, b) __UNDO_SA_CONCAT2(a, b)
#define __UNDO_SA_CONCAT2(a, b) a##b
#define UNDO_STATIC_ASSERT(cond, message)                              \
    enum { __UNDO_SA_CONCAT(UNDO_STATIC_ASSERT_LINE_AT_, __LINE__) = \
               sizeof(struct __UNDO_SA_CONCAT(__undo_static_assert_line_at_, __LINE__) { int static_assertion_failed[(cond) ? 1 : -1]; }) }
#endif /* UNDO_STATIC_ASSERT */
typedef size_t undo_rsize_t;
#else
#define UNDO_STATIC_ASSERT(cond, message)
typedef size_t undo_rsize_t;
#endif /* __STDC_VERSION__ >= 201112L */

#ifndef UNDO_DECL_ENUM
#if defined(__cplusplus) && __cplusplus >= 201103L
#define UNDO_DECL_ENUM(type, name) enum name : type
#else
#define UNDO_DECL_ENUM(type, name) \
    typedef type name;               \
    enum
#endif /*__STDC_VERSION__ >= 201112L */
#endif /* UNDO_DECL_ENUM */

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L /* C99 */
#include <stdint.h>
#ifndef UNDO_DECL_INLINE
#define UNDO_DECL_INLINE inline
#endif /* UNDO_DECL_INLINE */
typedef uint8_t undo_uint8_t;
typedef int16_t undo_int16_t;
typedef uint16_t undo_uint16_t;
typedef int32_t undo_int32_t;
typedef uint32_t undo_uint32_t;
#define undo_false false
#define undo_true true
#elif defined(_MSC_VER) && _MSC_VER >= 1300
typedef unsigned __int8 undo_uint8_t;
typedef signed __int16 undo_int16_t;
typedef unsigned __int16 undo_uint16_t;
typedef signed __int32 undo_int32_t;
typedef unsigned __int32 undo_uint32_t;
#ifndef UNDO_DECL_INLINE
#define UNDO_DECL_INLINE __inline
#endif
#else
typedef unsigned char undo_uint8_t;
typedef short undo_int16_t;
typedef unsigned short undo_uint16_t;
typedef int undo_int32_t;
typedef unsigned int undo_uint32_t;
#ifndef UNDO_DECL_INLINE
#define UNDO_DECL_INLINE
#endif /* UNDO_DECL_INLINE */
#endif /* __STDC_VERSION__ >= 199901L */

typedef int undo_bool_t;
typedef float undo_float32_t;
typedef double undo_float64_t;

/* utility macros */
#ifdef UNDO_ENABLE_BRANCH_PREDICTION
#define undo_likely(expr) __builtin_expected(!!(expr), 1)
#define undo_unlikely(expr) __builtin_expected(!!(expr), 0)
#else
#define undo_likely(expr) (expr)
#define undo_unlikely(expr) (expr)
#endif /* UNDO_ENABLE_BRANCH_PREDICTION */

#define undo_loop for (;;)
#define undo_has_error(cond) (undo_unlikely((cond) != UNDO_STATUS_SUCCESS))
#define undo_is_null(cond) (undo_unlikely((cond) == NULL))
#define undo_is_not_null(cond) (undo_likely((cond) != NULL))
#define undo_mark_unused(cond)              \
    undo_loop                               \
    {                                         \
        (void)(0 ? (void) (cond) : (void) 0); \
        break;                                \
    }
#define undo_fourcc(a, b, c, d) ((a) << 0 | (b) << 8 | (c) << 16 | (d) << 24)

#if defined(UNDO_BUILDING_DLL)
#if defined(_WIN32)
#ifdef UNDO_DLL_EXPORTS
#define UNDO_DECL_API __declspec(dllexport)
#else
#define UNDO_DECL_API __declspec(dllimport)
#endif
#elif defined(__GNUC__)
#ifdef __cplusplus
#define UNDO_DECL_API __attribute__((visibility("default"))) extern "C"
#else
#define UNDO_DECL_API __attribute__((visibility("default"))) extern
#endif /* __cplusplus */
#endif
#endif /* UNDO_BUILDING_DLL */

#ifndef UNDO_DECL_API
#ifdef __cplusplus
#define UNDO_DECL_API extern "C"
#else
#define UNDO_DECL_API extern
#endif /* __cplusplus */
#endif /* UNDO_DECL_API */

#ifdef RSIZE_MAX
#define UNDO_RSIZE_MAX RSIZE_MAX
#else
#define UNDO_RSIZE_MAX ((~(size_t)(0)) >> 1)
#endif /* RSIZE_MAX */

#ifndef APIENTRY
#define APIENTRY
#endif /* APIENTRY */

UNDO_STATIC_ASSERT(sizeof(undo_uint8_t) == 1, "size of undo_uint8_t must be 1");
UNDO_STATIC_ASSERT(sizeof(undo_int16_t) == 2, "size of undo_int16_t must be 2");
UNDO_STATIC_ASSERT(sizeof(undo_uint16_t) == 2, "size of undo_uint16_t must be 2");
UNDO_STATIC_ASSERT(sizeof(undo_int32_t) == 4, "size of undo_int32_t must be 4");
UNDO_STATIC_ASSERT(sizeof(undo_uint32_t) == 4, "size of undo_uint32_t must be 4");
UNDO_STATIC_ASSERT(sizeof(undo_float32_t) == 4, "size of undo_float32_t must be 4");
UNDO_STATIC_ASSERT(sizeof(undo_float64_t) == 8, "size of undo_float64_t must be 8");

typedef struct undo_global_allocator_t undo_global_allocator_t;
typedef struct undo_stack_t undo_stack_t;
typedef struct undo_command_t undo_command_t;

typedef void *(*undo_global_allocator_malloc_t)(void *, undo_rsize_t, const char *file, int line);
typedef void *(*undo_global_allocator_calloc_t)(void *, undo_rsize_t, undo_rsize_t, const char *file, int line);
typedef void *(*undo_global_allocator_realloc_t)(void *, void *, undo_rsize_t, const char *file, int line);
typedef void (*undo_global_allocator_free_t)(void *, void *, const char *filename, int line);

struct undo_global_allocator_t {
    void *opaque;
    undo_global_allocator_malloc_t malloc;
    undo_global_allocator_calloc_t calloc;
    undo_global_allocator_realloc_t realloc;
    undo_global_allocator_free_t free;
};

typedef void (*undo_command_on_undo_t)(const undo_command_t *command);
typedef void (*undo_command_on_redo_t)(const undo_command_t *command);
typedef undo_bool_t (*undo_command_on_persist_undo_t)(const undo_command_t *command);
typedef undo_bool_t (*undo_command_on_persist_redo_t)(const undo_command_t *command);
typedef void (*undo_command_on_destroy_t)(const undo_command_t *command);
typedef int (*undo_command_can_undo_t)(const undo_command_t *command);
typedef int (*undo_command_can_redo_t)(const undo_command_t *command);

/**
 * \defgroup undo_custom_allocator Custom Allocator
 * @{
 */
UNDO_DECL_API const undo_global_allocator_t *APIENTRY
undoGlobalGetCustomAllocator(void);
UNDO_DECL_API void APIENTRY
undoGlobalSetCustomAllocator(const undo_global_allocator_t *allocator);
/** @} */

/**
 * \defgroup undo_stack Undo Stack
 * @{
 */
UNDO_DECL_API int APIENTRY
undoStackGetHardLimit(void);
UNDO_DECL_API undo_stack_t * APIENTRY
undoStackCreate(void);
UNDO_DECL_API undo_stack_t * APIENTRY
undoStackCreateWithSoftLimit(int value);
UNDO_DECL_API void APIENTRY
undoStackPushCommand(undo_stack_t *stack, undo_command_t *command);
UNDO_DECL_API void APIENTRY
undoStackUndo(undo_stack_t *stack);
UNDO_DECL_API void APIENTRY
undoStackRedo(undo_stack_t *stack);
UNDO_DECL_API int APIENTRY
undoStackGetOffset(const undo_stack_t *stack);
UNDO_DECL_API void APIENTRY
undoStackSetOffset(undo_stack_t *stack, int value);
UNDO_DECL_API void APIENTRY
undoStackClear(undo_stack_t *stack);
UNDO_DECL_API int APIENTRY
undoStackCountCommands(const undo_stack_t *stack);
UNDO_DECL_API int APIENTRY
undoStackGetMaxStackSize(const undo_stack_t *stack);
UNDO_DECL_API int APIENTRY
undoStackGetSoftLimit(const undo_stack_t *stack);
UNDO_DECL_API void APIENTRY
undoStackSetSoftLimit(undo_stack_t *stack, int value);
UNDO_DECL_API undo_bool_t APIENTRY
undoStackCanPush(const undo_stack_t *stack);
UNDO_DECL_API undo_bool_t APIENTRY
undoStackCanUndo(const undo_stack_t *stack);
UNDO_DECL_API undo_bool_t APIENTRY
undoStackCanRedo(const undo_stack_t *stack);
UNDO_DECL_API undo_bool_t APIENTRY
undoStackIsDirty(const undo_stack_t *stack);
UNDO_DECL_API void APIENTRY
undoStackDestroy(undo_stack_t *stack);
/** @} */

/**
 * \defgroup undo_command Undo Command
 * @{
 */
UNDO_DECL_API undo_command_t * APIENTRY
undoCommandCreate(void);
UNDO_DECL_API undo_command_on_undo_t APIENTRY
undoCommandGetOnUndoCallback(undo_command_t *command);
UNDO_DECL_API void APIENTRY
undoCommandSetOnUndoCallback(undo_command_t *command, undo_command_on_undo_t value);
UNDO_DECL_API undo_command_on_redo_t APIENTRY
undoCommandGetOnRedoCallback(undo_command_t *command);
UNDO_DECL_API void APIENTRY
undoCommandSetOnRedoCallback(undo_command_t *command, undo_command_on_redo_t value);
UNDO_DECL_API undo_command_on_destroy_t APIENTRY
undoCommandGetOnDestroyCallback(undo_command_t *command);
UNDO_DECL_API void APIENTRY
undoCommandSetOnDestroyCallback(undo_command_t *command, undo_command_on_destroy_t value);
UNDO_DECL_API undo_command_on_persist_undo_t APIENTRY
undoCommandGetOnPersistUndoCallback(undo_command_t *command);
UNDO_DECL_API void APIENTRY
undoCommandSetOnPersistUndoCallback(undo_command_t *command, undo_command_on_persist_undo_t value);
UNDO_DECL_API undo_command_on_persist_redo_t APIENTRY
undoCommandGetOnPersistRedoCallback(undo_command_t *command);
UNDO_DECL_API void APIENTRY
undoCommandSetOnPersistRedoCallback(undo_command_t *command, undo_command_on_persist_redo_t value);
UNDO_DECL_API undo_command_can_undo_t APIENTRY
undoCommandGetCanUndoCallback(undo_command_t *command);
UNDO_DECL_API void APIENTRY
undoCommandSetCanUndoCallback(undo_command_t *command, undo_command_can_undo_t value);
UNDO_DECL_API undo_command_can_redo_t APIENTRY
undoCommandGetCanRedoCallback(undo_command_t *command);
UNDO_DECL_API void APIENTRY
undoCommandSetCanRedoCallback(undo_command_t *command, undo_command_can_redo_t value);
UNDO_DECL_API const char * APIENTRY
undoCommandGetName(const undo_command_t *command);
UNDO_DECL_API void APIENTRY
undoCommandSetName(undo_command_t *command, const char *value);
UNDO_DECL_API void * APIENTRY
undoCommandGetOpaqueData(const undo_command_t *command);
UNDO_DECL_API void APIENTRY
undoCommandSetOpaqueData(undo_command_t *command, void *value);
UNDO_DECL_API void APIENTRY
undoCommandDestroy(undo_command_t *command);
/** @} */

#endif
