/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#ifndef NANOEM_H_
#define NANOEM_H_

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L /* C11 */
#define __STDC_WANT_LIB_EXT1__ 1 /* define for rsize_t */
#endif

#include <stddef.h> /* size_t */
#ifdef _WIN32
#include <windows.h>
#endif /* _WIN32 */

/**
 * \defgroup nanoem nanoem
 * @{
 */

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L /* C11 */
#ifndef NANOEM_STATIC_ASSERT
#define NANOEM_STATIC_ASSERT _Static_assert
#endif /* NANOEM_STATIC_ASSERT */
#ifdef RSIZE_MAX
typedef rsize_t nanoem_rsize_t;
#else
typedef size_t nanoem_rsize_t;
#endif /* RSIZE_MAX */
#elif !defined(__cplusplus)
#ifndef NANOEM_STATIC_ASSERT
#define __NANOEM_SA_CONCAT(a, b) __NANOEM_SA_CONCAT2(a, b)
#define __NANOEM_SA_CONCAT2(a, b) a##b
#define NANOEM_STATIC_ASSERT(cond, message)                              \
    enum { __NANOEM_SA_CONCAT(NANOEM_STATIC_ASSERT_LINE_AT_, __LINE__) = \
               sizeof(struct __NANOEM_SA_CONCAT(__nanoem_static_assert_line_at_, __LINE__) { int static_assertion_failed[(cond) ? 1 : -1]; }) }
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
#define NANOEM_DECL_ENUM(type, name) \
    typedef type name;               \
    enum
#endif /*__STDC_VERSION__ >= 201112L */
#endif /* NANOEM_DECL_ENUM */

#ifndef NANOEM_DECL_OPAQUE
#define NANOEM_DECL_OPAQUE(type) typedef struct type type;
#endif /* NANOEM_DECL_OPAQUE */

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

/* utility macros */
#ifdef NANOEM_ENABLE_BRANCH_PREDICTION
#ifndef nanoem_likely
#define nanoem_likely(expr) __builtin_expect(!!(expr), 1)
#endif /* nanoem_likely */
#ifndef nanoem_unlikely
#define nanoem_unlikely(expr) __builtin_expect(!!(expr), 0)
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
#define nanoem_mark_unused(cond)              \
    for (;;) {                                \
        (void)(0 ? (void) (cond) : (void) 0); \
        break;                                \
    }
#endif /* nanoem_mark_unused */

#ifndef nanoem_fourcc
#define nanoem_fourcc(a, b, c, d) (nanoem_u32_t) ((a) << 0 | (b) << 8 | (c) << 16 | (d) << 24)
#endif /* nanoem_fourcc */

#if defined(NANOEM_BUILDING_DLL)
#ifdef __cplusplus
#define _NANOEM_DECL_API extern "C"
#else
#define _NANOEM_DECL_API extern
#endif /* __cplusplus */
#if defined(_WIN32)
#ifdef NANOEM_DLL_EXPORTS
#define NANOEM_DECL_API _NANOEM_DECL_API __declspec(dllexport)
#else
#define NANOEM_DECL_API _NANOEM_DECL_API __declspec(dllimport)
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
#define NANOEM_RSIZE_MAX ((~(size_t)(0)) >> 1)
#endif /* RSIZE_MAX */
#endif /* NANOEM_RSIZE_MAX */

#ifndef APIENTRY
#define APIENTRY
#endif /* APIENTRY */

NANOEM_STATIC_ASSERT(sizeof(nanoem_u8_t) == 1, "size of nanoem_u8_t must be 1");
NANOEM_STATIC_ASSERT(sizeof(nanoem_i16_t) == 2, "size of nanoem_i16_t must be 2");
NANOEM_STATIC_ASSERT(sizeof(nanoem_u16_t) == 2, "size of nanoem_u16_t must be 2");
NANOEM_STATIC_ASSERT(sizeof(nanoem_i32_t) == 4, "size of nanoem_i32_t must be 4");
NANOEM_STATIC_ASSERT(sizeof(nanoem_u32_t) == 4, "size of nanoem_u32_t must be 4");
NANOEM_STATIC_ASSERT(sizeof(nanoem_i64_t) == 8, "size of nanoem_i64_t must be 8");
NANOEM_STATIC_ASSERT(sizeof(nanoem_u64_t) == 8, "size of nanoem_u64_t must be 8");
NANOEM_STATIC_ASSERT(sizeof(nanoem_f32_t) == 4, "size of nanoem_f32_t must be 4");
NANOEM_STATIC_ASSERT(sizeof(nanoem_f64_t) == 8, "size of nanoem_f64_t must be 8");

NANOEM_DECL_OPAQUE(nanoem_user_data_t);

NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_status_t){
    NANOEM_STATUS_UNKNOWN = -1,
    NANOEM_STATUS_FIRST_ENUM,
    NANOEM_STATUS_SUCCESS = NANOEM_STATUS_FIRST_ENUM,
    NANOEM_STATUS_ERROR_MALLOC_FAILED,
    NANOEM_STATUS_ERROR_REALLOC_FAILED,
    NANOEM_STATUS_ERROR_NULL_OBJECT,
    NANOEM_STATUS_ERROR_BUFFER_END,
    NANOEM_STATUS_ERROR_DECODE_UNICODE_STRING_FAILED,
    NANOEM_STATUS_ERROR_ENCODE_UNICODE_STRING_FAILED,
    NANOEM_STATUS_ERROR_INVALID_SIGNATURE = 100,
    NANOEM_STATUS_ERROR_MODEL_VERTEX_CORRUPTED,
    NANOEM_STATUS_ERROR_MODEL_FACE_CORRUPTED,
    NANOEM_STATUS_ERROR_MODEL_MATERIAL_CORRUPTED,
    NANOEM_STATUS_ERROR_MODEL_BONE_CORRUPTED,
    NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_CORRUPTED,
    NANOEM_STATUS_ERROR_MODEL_TEXTURE_CORRUPTED,
    NANOEM_STATUS_ERROR_MODEL_MORPH_CORRUPTED,
    NANOEM_STATUS_ERROR_MODEL_LABEL_CORRUPTED,
    NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_CORRUPTED,
    NANOEM_STATUS_ERROR_MODEL_JOINT_CORRUPTED,
    NANOEM_STATUS_ERROR_PMD_ENGLISH_CORRUPTED,
    NANOEM_STATUS_ERROR_PMX_INFO_CORRUPUTED,
    NANOEM_STATUS_ERROR_MOTION_TARGET_NAME_CORRUPTED,
    NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_CORRUPTED,
    NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_CORRUPTED,
    NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_CORRUPTED,
    NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_CORRUPTED,
    NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_CORRUPTED,
    NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_CORRUPTED,
    NANOEM_STATUS_ERROR_MODEL_SOFT_BODY_CORRUPTED,
    NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_REFERENCE = 200,
    NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_NOT_FOUND,
    NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_REFERENCE,
    NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_NOT_FOUND,
    NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_REFERENCE,
    NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_NOT_FOUND,
    NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_REFERENCE,
    NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_NOT_FOUND,
    NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_REFERENCE,
    NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_NOT_FOUND,
    NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_REFERENCE,
    NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_NOT_FOUND,
    NANOEM_STATUS_ERROR_MOTION_ACCESSORY_KEYFRAME_REFERENCE,
    NANOEM_STATUS_ERROR_MOTION_ACCESSORY_KEYFRAME_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MOTION_ACCESSORY_KEYFRAME_NOT_FOUND,
    NANOEM_STATUS_ERROR_EFFECT_PARAMETER_REFERENCE,
    NANOEM_STATUS_ERROR_EFFECT_PARAMETER_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_EFFECT_PARAMETER_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_STATE_REFERENCE,
    NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_STATE_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_STATE_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_BINDING_REFERENCE,
    NANOEM_STATUS_ERROR_MODEL_BINDING_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MODEL_BINDING_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_VERTEX_REFERENCE = 300,
    NANOEM_STATUS_ERROR_MODEL_VERTEX_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MODEL_VERTEX_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_MATERIAL_REFERENCE,
    NANOEM_STATUS_ERROR_MODEL_MATERIAL_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MODEL_MATERIAL_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_BONE_REFERENCE,
    NANOEM_STATUS_ERROR_MODEL_BONE_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MODEL_BONE_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_REFERENCE,
    NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_JOINT_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_TEXTURE_REFERENCE,
    NANOEM_STATUS_ERROR_MODEL_TEXTURE_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MODEL_TEXTURE_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_MORPH_REFERENCE,
    NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MODEL_MORPH_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_MORPH_TYPE_MISMATCH,
    NANOEM_STATUS_ERROR_MODEL_MORPH_BONE_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_MORPH_FLIP_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_MORPH_GROUP_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_MORPH_IMPULSE_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_MORPH_MATERIAL_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_MORPH_UV_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_MORPH_VERTEX_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_LABEL_REFERENCE,
    NANOEM_STATUS_ERROR_MODEL_LABEL_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MODEL_LABEL_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_LABEL_ITEM_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_REFERENCE,
    NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_JOINT_REFERENCE,
    NANOEM_STATUS_ERROR_MODEL_JOINT_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MODEL_JOINT_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_SOFT_BODY_REFERENCE,
    NANOEM_STATUS_ERROR_MODEL_SOFT_BODY_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MODEL_SOFT_BODY_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_SOFT_BODY_ANCHOR_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_MODEL_SOFT_BODY_ANCHOR_NOT_FOUND,
    NANOEM_STATUS_ERROR_MODEL_VERSION_INCOMPATIBLE = 400,
    NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_ALREADY_EXISTS = 1000,
    NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_NOT_FOUND,
    NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_KEYFRAME_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_KEYFRAME_NOT_FOUND,
    NANOEM_STATUS_ERROR_DOCUMENT_CAMERA_KEYFRAME_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_DOCUMENT_CAMERA_KEYFRAME_NOT_FOUND,
    NANOEM_STATUS_ERROR_DOCUMENT_GRAVITY_KEYFRAME_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_DOCUMENT_GRAVITY_KEYFRAME_NOT_FOUND,
    NANOEM_STATUS_ERROR_DOCUMENT_LIGHT_KEYFRAME_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_DOCUMENT_LIGHT_KEYFRAME_NOT_FOUND,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_NOT_FOUND,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_KEYFRAME_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_KEYFRAME_NOT_FOUND,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_STATE_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_STATE_NOT_FOUND,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_CONSTRAINT_STATE_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_CONSTRAINT_STATE_NOT_FOUND,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MODEL_KEYFRAME_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MODEL_KEYFRAME_NOT_FOUND,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_KEYFRAME_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_KEYFRAME_NOT_FOUND,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_STATE_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_STATE_NOT_FOUND,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_NOT_FOUND,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_STATE_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_STATE_NOT_FOUND,
    NANOEM_STATUS_ERROR_DOCUMENT_SELF_SHADOW_KEYFRAME_ALREADY_EXISTS,
    NANOEM_STATUS_ERROR_DOCUMENT_SELF_SHADOW_KEYFRAME_NOT_FOUND,
    NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_CORRUPTED,
    NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_KEYFRAME_CORRUPTED,
    NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_OUTSIDE_PARENT_CORRUPTED,
    NANOEM_STATUS_ERROR_DOCUMENT_CAMERA_CORRUPTED,
    NANOEM_STATUS_ERROR_DOCUMENT_CAMERA_KEYFRAME_CORRUPTED,
    NANOEM_STATUS_ERROR_DOCUMENT_GRAVITY_CORRUPTED,
    NANOEM_STATUS_ERROR_DOCUMENT_GRAVITY_KEYFRAME_CORRUPTED,
    NANOEM_STATUS_ERROR_DOCUMENT_LIGHT_CORRUPTED,
    NANOEM_STATUS_ERROR_DOCUMENT_LIGHT_KEYFRAME_CORRUPTED,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_CORRUPTED,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_KEYFRAME_CORRUPTED,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_KEYFRAME_CORRUPTED,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_STATE_CORRUPTED,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_CONSTRAINT_STATE_CORRUPTED,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_KEYFRAME_CORRUPTED,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_STATE_CORRUPTED,
    NANOEM_STATUS_ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_CORRUPTED,
    NANOEM_STATUS_ERROR_DOCUMENT_SELF_SHADOW_CORRUPTED,
    NANOEM_STATUS_ERROR_DOCUMENT_SELF_SHADOW_KEYFRAME_CORRUPTED,
    NANOEM_STATUS_MAX_ENUM
};

NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_language_type_t){
    NANOEM_LANGUAGE_TYPE_UNKNOWN = -1,
    NANOEM_LANGUAGE_TYPE_FIRST_ENUM,
    NANOEM_LANGUAGE_TYPE_JAPANESE = NANOEM_LANGUAGE_TYPE_FIRST_ENUM,
    NANOEM_LANGUAGE_TYPE_ENGLISH,
    NANOEM_LANGUAGE_TYPE_MAX_ENUM
};

NANOEM_DECL_API const char *APIENTRY
nanoemGetVersionString(void);

/**
 * \defgroup nanoem_custom_allocator Custom Allocator
 * @{
 */

/**
 * \defgroup nanoem_custom_allocator_callback Custom Allocator Callback
 * @{
 */
typedef void *(*nanoem_global_allocator_malloc_t)(void *, nanoem_rsize_t, const char *file, int line);
typedef void *(*nanoem_global_allocator_calloc_t)(void *, nanoem_rsize_t, nanoem_rsize_t, const char *file, int line);
typedef void *(*nanoem_global_allocator_realloc_t)(void *, void *, nanoem_rsize_t, const char *file, int line);
typedef void (*nanoem_global_allocator_free_t)(void *, void *, const char *filename, int line);
/** @} */

typedef struct nanoem_global_allocator_t nanoem_global_allocator_t;
struct nanoem_global_allocator_t {
    void *opaque;
    nanoem_global_allocator_malloc_t malloc;
    nanoem_global_allocator_calloc_t calloc;
    nanoem_global_allocator_realloc_t realloc;
    nanoem_global_allocator_free_t free;
};

NANOEM_DECL_API const nanoem_global_allocator_t *APIENTRY
nanoemGlobalGetCustomAllocator(void);
NANOEM_DECL_API void APIENTRY
nanoemGlobalSetCustomAllocator(const nanoem_global_allocator_t *allocator);
/** @} */

/**
 * \defgroup nanoem_unicode_string_factory Unicode String Factory
 * @{
 */
NANOEM_DECL_OPAQUE(nanoem_unicode_string_t);
struct nanoem_unicode_string_t {
    const void *opaque;
};

NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_codec_type_t){
    NANOEM_CODEC_TYPE_UNKNOWN = -1,
    NANOEM_CODEC_TYPE_FIRST_ENUM,
    NANOEM_CODEC_TYPE_SJIS = NANOEM_CODEC_TYPE_FIRST_ENUM,
    NANOEM_CODEC_TYPE_UTF8,
    NANOEM_CODEC_TYPE_UTF16,
    NANOEM_CODEC_TYPE_MAX_ENUM
};

/**
 * \defgroup nanoem_unicode_string_factory_callback Unicode String Factory Callback
 * @{
 */
NANOEM_DECL_OPAQUE(nanoem_unicode_string_factory_t);
typedef nanoem_unicode_string_t *(*nanoem_unicode_string_factory_from_codec_t)(void *, const nanoem_u8_t *, nanoem_rsize_t, nanoem_status_t *);
typedef nanoem_u8_t *(*nanoem_unicode_string_factory_to_codec_t)(void *, const nanoem_unicode_string_t *, nanoem_rsize_t *, nanoem_status_t *);
typedef nanoem_i32_t (*nanoem_unicode_string_factory_hash_t)(void *, const nanoem_unicode_string_t *);
typedef int (*nanoem_unicode_string_factory_compare_t)(void *, const nanoem_unicode_string_t *, const nanoem_unicode_string_t *);
typedef const nanoem_u8_t * (*nanoem_unicode_string_factory_get_cache_t)(void *, const nanoem_unicode_string_t *, nanoem_rsize_t *, nanoem_codec_type_t, nanoem_status_t *);
typedef void (*nanoem_unicode_string_factory_set_cache_t)(void *, nanoem_unicode_string_t *, nanoem_rsize_t *, nanoem_codec_type_t, nanoem_status_t *);
typedef void (*nanoem_unicode_string_factory_destroy_string_t)(void *, nanoem_unicode_string_t *);
typedef void (*nanoem_unicode_string_factory_destroy_byte_array_t)(void *, nanoem_u8_t *);
/** @} */

NANOEM_DECL_API nanoem_unicode_string_factory_t *APIENTRY
nanoemUnicodeStringFactoryCreate(nanoem_status_t *status);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactorySetConvertFromCp932Callback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_from_codec_t value);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactorySetConvertFromUtf8Callback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_from_codec_t value);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactorySetConvertFromUtf16Callback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_from_codec_t value);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactorySetConvertToCp932Callback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_to_codec_t value);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactorySetConvertToUtf8Callback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_to_codec_t value);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactorySetConvertToUtf16Callback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_to_codec_t value);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactorySetHashCallback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_hash_t value);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactorySetCompareCallback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_compare_t value);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactorySetGetCacheCallback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_get_cache_t value);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactorySetSetCacheCallback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_set_cache_t value);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactorySetDestroyStringCallback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_destroy_string_t value);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactorySetDestroyByteArrayCallback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_destroy_byte_array_t value);
NANOEM_DECL_API void *APIENTRY
nanoemUnicodeStringFactoryGetOpaqueData(const nanoem_unicode_string_factory_t *factory);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactorySetOpaqueData(nanoem_unicode_string_factory_t *factory, void *value);
NANOEM_DECL_API nanoem_unicode_string_t *APIENTRY
nanoemUnicodeStringFactoryCreateString(nanoem_unicode_string_factory_t *factory, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status);
NANOEM_DECL_API nanoem_u8_t *APIENTRY
nanoemUnicodeStringFactoryGetByteArray(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status);
NANOEM_DECL_API nanoem_unicode_string_t *APIENTRY
nanoemUnicodeStringFactoryCreateStringWithEncoding(nanoem_unicode_string_factory_t *factory, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_codec_type_t codec, nanoem_status_t *status);
NANOEM_DECL_API nanoem_u8_t *APIENTRY
nanoemUnicodeStringFactoryGetByteArrayEncoding(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_codec_type_t codec, nanoem_status_t *status);
NANOEM_DECL_API nanoem_unicode_string_t *APIENTRY
nanoemUnicodeStringFactoryCloneString(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *source, nanoem_status_t *status);
NANOEM_DECL_API int APIENTRY
nanoemUnicodeStringFactoryCompareString(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *lvalue, const nanoem_unicode_string_t *rvalue);
NANOEM_DECL_API const nanoem_u8_t * APIENTRY
nanoemUnicodeStringFactoryGetCacheString(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_codec_type_t codec, nanoem_status_t *status);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactorySetCacheString(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_codec_type_t codec, nanoem_status_t *status);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactoryDestroyByteArray(nanoem_unicode_string_factory_t *factory, nanoem_u8_t *bytes);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactoryDestroyString(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_t *string);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactoryDestroy(nanoem_unicode_string_factory_t *factory);
/** @} */

/**
 * \defgroup nanoem_buffer Buffer
 * @{
 */
NANOEM_DECL_OPAQUE(nanoem_buffer_t);

NANOEM_DECL_API nanoem_buffer_t *APIENTRY
nanoemBufferCreate(const nanoem_u8_t *data, nanoem_rsize_t length, nanoem_status_t *status);
NANOEM_DECL_API nanoem_rsize_t APIENTRY
nanoemBufferGetLength(const nanoem_buffer_t *buffer);
NANOEM_DECL_API nanoem_rsize_t APIENTRY
nanoemBufferGetOffset(const nanoem_buffer_t *buffer);
NANOEM_DECL_API const nanoem_u8_t *APIENTRY
nanoemBufferGetDataPtr(const nanoem_buffer_t *buffer);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemBufferCanReadLength(const nanoem_buffer_t *buffer, nanoem_rsize_t size);
NANOEM_DECL_API void APIENTRY
nanoemBufferSkip(nanoem_buffer_t *buffer, nanoem_rsize_t skip, nanoem_status_t *status);
NANOEM_DECL_API void APIENTRY
nanoemBufferSeek(nanoem_buffer_t *buffer, nanoem_rsize_t position, nanoem_status_t *status);
NANOEM_DECL_API nanoem_u8_t APIENTRY
nanoemBufferReadByte(nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_API nanoem_i16_t APIENTRY
nanoemBufferReadInt16LittleEndian(nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_API nanoem_u16_t APIENTRY
nanoemBufferReadInt16LittleEndianUnsigned(nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_API nanoem_i32_t APIENTRY
nanoemBufferReadInt32LittleEndian(nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemBufferReadFloat32LittleEndian(nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_API char *APIENTRY
nanoemBufferReadBuffer(nanoem_buffer_t *buffer, nanoem_rsize_t len, nanoem_status_t *status);
NANOEM_DECL_API void APIENTRY
nanoemBufferDestroy(nanoem_buffer_t *buffer);
/** @} */

/**
 * \defgroup nanoem_model Model
 * @{
 */
NANOEM_DECL_OPAQUE(nanoem_model_t);
NANOEM_DECL_OPAQUE(nanoem_model_object_t);
NANOEM_DECL_OPAQUE(nanoem_model_vertex_t);
NANOEM_DECL_OPAQUE(nanoem_model_material_t);
NANOEM_DECL_OPAQUE(nanoem_model_bone_t);
NANOEM_DECL_OPAQUE(nanoem_model_constraint_joint_t);
NANOEM_DECL_OPAQUE(nanoem_model_constraint_t);
NANOEM_DECL_OPAQUE(nanoem_model_morph_group_t);
NANOEM_DECL_OPAQUE(nanoem_model_morph_vertex_t);
NANOEM_DECL_OPAQUE(nanoem_model_morph_bone_t);
NANOEM_DECL_OPAQUE(nanoem_model_morph_uv_t);
NANOEM_DECL_OPAQUE(nanoem_model_morph_material_t);
NANOEM_DECL_OPAQUE(nanoem_model_morph_flip_t);
NANOEM_DECL_OPAQUE(nanoem_model_morph_impulse_t);
NANOEM_DECL_OPAQUE(nanoem_model_morph_t);
NANOEM_DECL_OPAQUE(nanoem_model_label_item_t);
NANOEM_DECL_OPAQUE(nanoem_model_label_t);
NANOEM_DECL_OPAQUE(nanoem_model_rigid_body_t);
NANOEM_DECL_OPAQUE(nanoem_model_joint_t);
NANOEM_DECL_OPAQUE(nanoem_model_soft_body_t);
NANOEM_DECL_OPAQUE(nanoem_model_soft_body_anchor_t);
NANOEM_DECL_OPAQUE(nanoem_model_texture_t);

NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_model_format_type_t){
    NANOEM_MODEL_FORMAT_TYPE_UNKNOWN = -1,
    NANOEM_MODEL_FORMAT_TYPE_FIRST_ENUM,
    NANOEM_MODEL_FORMAT_TYPE_PMD_1_0 = NANOEM_MODEL_FORMAT_TYPE_FIRST_ENUM,
    NANOEM_MODEL_FORMAT_TYPE_PMX_2_0,
    NANOEM_MODEL_FORMAT_TYPE_PMX_2_1,
    NANOEM_MODEL_FORMAT_TYPE_MAX_ENUM
};

/**
 * \defgroup nanoem_model_object Model Object
 * @{
 */
NANOEM_DECL_API const nanoem_model_t *APIENTRY
nanoemModelObjectGetParentModel(const nanoem_model_object_t *object);
NANOEM_DECL_API int APIENTRY
nanoemModelObjectGetIndex(const nanoem_model_object_t *object);
NANOEM_DECL_API const nanoem_user_data_t *APIENTRY
nanoemModelObjectGetUserData(const nanoem_model_object_t *object);
NANOEM_DECL_API void APIENTRY
nanoemModelObjectSetUserData(nanoem_model_object_t *object, nanoem_user_data_t *user_data);
/** @} */

/**
 * \defgroup nanoem_model_vertex Vertex
 * @{
 */
NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_model_vertex_type_t){
    NANOEM_MODEL_VERTEX_TYPE_UNKNOWN = -1,
    NANOEM_MODEL_VERTEX_TYPE_FIRST_ENUM,
    NANOEM_MODEL_VERTEX_TYPE_BDEF1 = NANOEM_MODEL_VERTEX_TYPE_FIRST_ENUM,
    NANOEM_MODEL_VERTEX_TYPE_BDEF2,
    NANOEM_MODEL_VERTEX_TYPE_BDEF4,
    NANOEM_MODEL_VERTEX_TYPE_SDEF,
    NANOEM_MODEL_VERTEX_TYPE_QDEF,
    NANOEM_MODEL_VERTEX_TYPE_MAX_ENUM
};

NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelVertexGetOrigin(const nanoem_model_vertex_t *vertex);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelVertexGetNormal(const nanoem_model_vertex_t *vertex);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelVertexGetTexCoord(const nanoem_model_vertex_t *vertex);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelVertexGetAdditionalUV(const nanoem_model_vertex_t *vertex, nanoem_rsize_t index);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelVertexGetSdefC(const nanoem_model_vertex_t *vertex);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelVertexGetSdefR0(const nanoem_model_vertex_t *vertex);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelVertexGetSdefR1(const nanoem_model_vertex_t *vertex);
NANOEM_DECL_API const nanoem_model_bone_t *APIENTRY
nanoemModelVertexGetBoneObject(const nanoem_model_vertex_t *vertex, nanoem_rsize_t index);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelVertexGetBoneWeight(const nanoem_model_vertex_t *vertex, nanoem_rsize_t index);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelVertexGetEdgeSize(const nanoem_model_vertex_t *vertex);
NANOEM_DECL_API nanoem_model_vertex_type_t APIENTRY
nanoemModelVertexGetType(const nanoem_model_vertex_t *vertex);
NANOEM_DECL_API const nanoem_model_object_t *APIENTRY
nanoemModelVertexGetModelObject(const nanoem_model_vertex_t *vertex);
NANOEM_DECL_API nanoem_model_object_t *APIENTRY
nanoemModelVertexGetModelObjectMutable(nanoem_model_vertex_t *vertex);
/** @} */

/**
 * \defgroup nanoem_model_material Material
 * @{
 */
NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_model_material_sphere_map_texture_type_t){
    NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_UNKNOWN = -1,
    NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_FIRST_ENUM,
    NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE = NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_FIRST_ENUM,
    NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_MULTIPLY,
    NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_ADD,
    NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_SUB_TEXTURE,
    NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_MAX_ENUM
};

NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemModelMaterialGetName(const nanoem_model_material_t *material, nanoem_language_type_t language);
NANOEM_DECL_API const nanoem_model_texture_t *APIENTRY
nanoemModelMaterialGetDiffuseTextureObject(const nanoem_model_material_t *material);
NANOEM_DECL_API const nanoem_model_texture_t *APIENTRY
nanoemModelMaterialGetSphereMapTextureObject(const nanoem_model_material_t *material);
NANOEM_DECL_API const nanoem_model_texture_t *APIENTRY
nanoemModelMaterialGetToonTextureObject(const nanoem_model_material_t *material);
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemModelMaterialGetClob(const nanoem_model_material_t *material);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelMaterialGetAmbientColor(const nanoem_model_material_t *material);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelMaterialGetDiffuseColor(const nanoem_model_material_t *material);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelMaterialGetSpecularColor(const nanoem_model_material_t *material);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelMaterialGetEdgeColor(const nanoem_model_material_t *material);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelMaterialGetDiffuseOpacity(const nanoem_model_material_t *material);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelMaterialGetEdgeOpacity(const nanoem_model_material_t *material);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelMaterialGetEdgeSize(const nanoem_model_material_t *material);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelMaterialGetSpecularPower(const nanoem_model_material_t *material);
NANOEM_DECL_API nanoem_model_material_sphere_map_texture_type_t APIENTRY
nanoemModelMaterialGetSphereMapTextureType(const nanoem_model_material_t *material);
NANOEM_DECL_API nanoem_rsize_t APIENTRY
nanoemModelMaterialGetNumVertexIndices(const nanoem_model_material_t *material);
NANOEM_DECL_API int APIENTRY
nanoemModelMaterialGetToonTextureIndex(const nanoem_model_material_t *material);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelMaterialIsToonShared(const nanoem_model_material_t *material);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelMaterialIsCullingDisabled(const nanoem_model_material_t *material);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelMaterialIsCastingShadowEnabled(const nanoem_model_material_t *material);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelMaterialIsCastingShadowMapEnabled(const nanoem_model_material_t *material);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelMaterialIsShadowMapEnabled(const nanoem_model_material_t *material);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelMaterialIsEdgeEnabled(const nanoem_model_material_t *material);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelMaterialIsVertexColorEnabled(const nanoem_model_material_t *material);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelMaterialIsPointDrawEnabled(const nanoem_model_material_t *material);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelMaterialIsLineDrawEnabled(const nanoem_model_material_t *material);
NANOEM_DECL_API const nanoem_model_object_t *APIENTRY
nanoemModelMaterialGetModelObject(const nanoem_model_material_t *material);
NANOEM_DECL_API nanoem_model_object_t *APIENTRY
nanoemModelMaterialGetModelObjectMutable(nanoem_model_material_t *material);
/** @} */

/**
 * \defgroup nanoem_model_bone Bone
 * @{
 */
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemModelBoneGetName(const nanoem_model_bone_t *bone, nanoem_language_type_t language);
NANOEM_DECL_API const nanoem_model_bone_t *APIENTRY
nanoemModelBoneGetParentBoneObject(const nanoem_model_bone_t *bone);
NANOEM_DECL_API const nanoem_model_bone_t *APIENTRY
nanoemModelBoneGetInherentParentBoneObject(const nanoem_model_bone_t *bone);
NANOEM_DECL_API const nanoem_model_bone_t *APIENTRY
nanoemModelBoneGetEffectorBoneObject(const nanoem_model_bone_t *bone);
NANOEM_DECL_API const nanoem_model_bone_t *APIENTRY
nanoemModelBoneGetTargetBoneObject(const nanoem_model_bone_t *bone);
NANOEM_DECL_API const nanoem_model_constraint_t *APIENTRY
nanoemModelBoneGetConstraintObject(const nanoem_model_bone_t *bone);
NANOEM_DECL_API nanoem_model_constraint_t *APIENTRY
nanoemModelBoneGetConstraintObjectMutable(nanoem_model_bone_t *bone);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelBoneGetOrigin(const nanoem_model_bone_t *bone);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelBoneGetDestinationOrigin(const nanoem_model_bone_t *bone);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelBoneGetFixedAxis(const nanoem_model_bone_t *bone);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelBoneGetLocalXAxis(const nanoem_model_bone_t *bone);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelBoneGetLocalZAxis(const nanoem_model_bone_t *bone);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelBoneGetInherentCoefficient(const nanoem_model_bone_t *bone);
NANOEM_DECL_API int APIENTRY
nanoemModelBoneGetStageIndex(const nanoem_model_bone_t *bone);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelBoneHasDestinationBone(const nanoem_model_bone_t *bone);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelBoneIsRotateable(const nanoem_model_bone_t *bone);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelBoneIsMovable(const nanoem_model_bone_t *bone);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelBoneIsVisible(const nanoem_model_bone_t *bone);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelBoneIsUserHandleable(const nanoem_model_bone_t *bone);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelBoneHasConstraint(const nanoem_model_bone_t *bone);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelBoneHasLocalInherent(const nanoem_model_bone_t *bone);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelBoneHasInherentTranslation(const nanoem_model_bone_t *bone);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelBoneHasInherentOrientation(const nanoem_model_bone_t *bone);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelBoneHasFixedAxis(const nanoem_model_bone_t *bone);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelBoneHasLocalAxes(const nanoem_model_bone_t *bone);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelBoneIsAffectedByPhysicsSimulation(const nanoem_model_bone_t *bone);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelBoneHasExternalParentBone(const nanoem_model_bone_t *bone);
NANOEM_DECL_API const nanoem_model_object_t *APIENTRY
nanoemModelBoneGetModelObject(const nanoem_model_bone_t *bone);
NANOEM_DECL_API nanoem_model_object_t *APIENTRY
nanoemModelBoneGetModelObjectMutable(nanoem_model_bone_t *bone);
/** @} */

/**
 * \defgroup nanoem_model_constraint Constraint
 * @{
 */
NANOEM_DECL_API const nanoem_model_bone_t *APIENTRY
nanoemModelConstraintGetEffectorBoneObject(const nanoem_model_constraint_t *constraint);
NANOEM_DECL_API const nanoem_model_bone_t *APIENTRY
nanoemModelConstraintGetTargetBoneObject(const nanoem_model_constraint_t *constraint);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelConstraintGetAngleLimit(const nanoem_model_constraint_t *constraint);
NANOEM_DECL_API int APIENTRY
nanoemModelConstraintGetNumIterations(const nanoem_model_constraint_t *constraint);
NANOEM_DECL_API nanoem_model_constraint_joint_t *const *APIENTRY
nanoemModelConstraintGetAllJointObjects(const nanoem_model_constraint_t *constraints, nanoem_rsize_t *num_joints);
NANOEM_DECL_API const nanoem_model_object_t *APIENTRY
nanoemModelConstraintGetModelObject(const nanoem_model_constraint_t *constraint);
NANOEM_DECL_API nanoem_model_object_t *APIENTRY
nanoemModelConstraintGetModelObjectMutable(nanoem_model_constraint_t *constraint);

/**
 * \defgroup nanoem_model_constraint_joint Constraint Joint
 * @{
 */
NANOEM_DECL_API const nanoem_model_bone_t *APIENTRY
nanoemModelConstraintJointGetBoneObject(const nanoem_model_constraint_joint_t *joint);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelConstraintJointGetUpperLimit(const nanoem_model_constraint_joint_t *joint);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelConstraintJointGetLowerLimit(const nanoem_model_constraint_joint_t *joint);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelConstraintJointHasAngleLimit(const nanoem_model_constraint_joint_t *joint);
/** @} */

/** @} */

/**
 * \defgroup nanoem_model_texture Texture
 * @{
 */
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemModelTextureGetPath(const nanoem_model_texture_t *texture);
NANOEM_DECL_API const nanoem_model_object_t *APIENTRY
nanoemModelTextureGetModelObject(const nanoem_model_texture_t *texture);
NANOEM_DECL_API nanoem_model_object_t *APIENTRY
nanoemModelTextureGetModelObjectMutable(nanoem_model_texture_t *texture);
/** @} */

/**
 * \defgroup nanoem_model_morph Morph
 * @{
 */
NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_model_morph_category_t){
    NANOEM_MODEL_MORPH_CATEGORY_UNKNOWN = -1,
    NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM,
    NANOEM_MODEL_MORPH_CATEGORY_BASE = NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM,
    NANOEM_MODEL_MORPH_CATEGORY_EYEBROW,
    NANOEM_MODEL_MORPH_CATEGORY_EYE,
    NANOEM_MODEL_MORPH_CATEGORY_LIP,
    NANOEM_MODEL_MORPH_CATEGORY_OTHER,
    NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM
};

NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_model_morph_type_t){
    NANOEM_MODEL_MORPH_TYPE_UNKNOWN = -1,
    NANOEM_MODEL_MORPH_TYPE_FIRST_ENUM,
    NANOEM_MODEL_MORPH_TYPE_GROUP = NANOEM_MODEL_MORPH_TYPE_FIRST_ENUM,
    NANOEM_MODEL_MORPH_TYPE_VERTEX,
    NANOEM_MODEL_MORPH_TYPE_BONE,
    NANOEM_MODEL_MORPH_TYPE_TEXTURE,
    NANOEM_MODEL_MORPH_TYPE_UVA1,
    NANOEM_MODEL_MORPH_TYPE_UVA2,
    NANOEM_MODEL_MORPH_TYPE_UVA3,
    NANOEM_MODEL_MORPH_TYPE_UVA4,
    NANOEM_MODEL_MORPH_TYPE_MATERIAL,
    NANOEM_MODEL_MORPH_TYPE_FLIP,
    NANOEM_MODEL_MORPH_TYPE_IMPULUSE,
    NANOEM_MODEL_MORPH_TYPE_MAX_ENUM
};

/**
 * \defgroup nanoem_model_morph_bone Bone Morph
 * @{
 */
NANOEM_DECL_API const nanoem_model_bone_t *APIENTRY
nanoemModelMorphBoneGetBoneObject(const nanoem_model_morph_bone_t *morph);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelMorphBoneGetTranslation(const nanoem_model_morph_bone_t *morph);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelMorphBoneGetOrientation(const nanoem_model_morph_bone_t *morph);
/** @} */

/**
 * \defgroup nanoem_model_morph_flip Flip Morph
 * @{
 */
NANOEM_DECL_API const nanoem_model_morph_t *APIENTRY
nanoemModelMorphFlipGetMorphObject(const nanoem_model_morph_flip_t *morph);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelMorphFlipGetWeight(const nanoem_model_morph_flip_t *morph);
/** @} */

/**
 * \defgroup nanoem_model_morph_group Group Morph
 * @{
 */
NANOEM_DECL_API const nanoem_model_morph_t *APIENTRY
nanoemModelMorphGroupGetMorphObject(const nanoem_model_morph_group_t *morph);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelMorphGroupGetWeight(const nanoem_model_morph_group_t *morph);
/** @} */

/**
 * \defgroup nanoem_model_morph_impulse Impulse Morph
 * @{
 */
NANOEM_DECL_API const nanoem_model_rigid_body_t *APIENTRY
nanoemModelMorphImpulseGetRigidBodyObject(const nanoem_model_morph_impulse_t *morph);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelMorphImpulseGetTorque(const nanoem_model_morph_impulse_t *morph);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelMorphImpulseGetVelocity(const nanoem_model_morph_impulse_t *morph);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelMorphImpulseIsLocal(const nanoem_model_morph_impulse_t *morph);
/** @} */

/**
 * \defgroup nanoem_model_morph_material Material Morph
 * @{
 */
NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_model_morph_material_operation_type_t){
    NANOEM_MODEL_MORPH_MATERIAL_OPERATION_TYPE_UNKNOWN = -1,
    NANOEM_MODEL_MORPH_MATERIAL_OPERATION_TYPE_FIRST_ENUM,
    NANOEM_MODEL_MORPH_MATERIAL_OPERATION_TYPE_MULTIPLY = NANOEM_MODEL_MORPH_MATERIAL_OPERATION_TYPE_FIRST_ENUM,
    NANOEM_MODEL_MORPH_MATERIAL_OPERATION_TYPE_ADD,
    NANOEM_MODEL_MORPH_MATERIAL_OPERATION_TYPE_MAX_ENUM
};

NANOEM_DECL_API const nanoem_model_material_t *APIENTRY
nanoemModelMorphMaterialGetMaterialObject(const nanoem_model_morph_material_t *morph);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelMorphMaterialGetAmbientColor(const nanoem_model_morph_material_t *morph);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelMorphMaterialGetDiffuseColor(const nanoem_model_morph_material_t *morph);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelMorphMaterialGetSpecularColor(const nanoem_model_morph_material_t *morph);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelMorphMaterialGetEdgeColor(const nanoem_model_morph_material_t *morph);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelMorphMaterialGetDiffuseTextureBlend(const nanoem_model_morph_material_t *morph);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelMorphMaterialGetSphereMapTextureBlend(const nanoem_model_morph_material_t *morph);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelMorphMaterialGetToonTextureBlend(const nanoem_model_morph_material_t *morph);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelMorphMaterialGetDiffuseOpacity(const nanoem_model_morph_material_t *morph);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelMorphMaterialGetEdgeOpacity(const nanoem_model_morph_material_t *morph);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelMorphMaterialGetSpecularPower(const nanoem_model_morph_material_t *morph);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelMorphMaterialGetEdgeSize(const nanoem_model_morph_material_t *morph);
NANOEM_DECL_API nanoem_model_morph_material_operation_type_t APIENTRY
nanoemModelMorphMaterialGetOperationType(const nanoem_model_morph_material_t *morph);
/** @} */

/**
 * \defgroup nanoem_model_morph_uv UV Morph
 * @{
 */
NANOEM_DECL_API const nanoem_model_vertex_t *APIENTRY
nanoemModelMorphUVGetVertexObject(const nanoem_model_morph_uv_t *morph);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelMorphUVGetPosition(const nanoem_model_morph_uv_t *morph);
/** @} */

/**
 * \defgroup nanoem_model_morph_vertex Vertex Morph
 * @{
 */
NANOEM_DECL_API const nanoem_model_vertex_t *APIENTRY
nanoemModelMorphVertexGetVertexObject(const nanoem_model_morph_vertex_t *morph);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelMorphVertexGetPosition(const nanoem_model_morph_vertex_t *morph);
/** @} */

NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemModelMorphGetName(const nanoem_model_morph_t *morph, nanoem_language_type_t language);
NANOEM_DECL_API nanoem_model_morph_category_t APIENTRY
nanoemModelMorphGetCategory(const nanoem_model_morph_t *morph);
NANOEM_DECL_API nanoem_model_morph_type_t APIENTRY
nanoemModelMorphGetType(const nanoem_model_morph_t *morph);
NANOEM_DECL_API nanoem_model_morph_bone_t *const *APIENTRY
nanoemModelMorphGetAllBoneMorphObjects(const nanoem_model_morph_t *morph, nanoem_rsize_t *num_objects);
NANOEM_DECL_API nanoem_model_morph_flip_t *const *APIENTRY
nanoemModelMorphGetAllFlipMorphObjects(const nanoem_model_morph_t *morph, nanoem_rsize_t *num_objects);
NANOEM_DECL_API nanoem_model_morph_group_t *const *APIENTRY
nanoemModelMorphGetAllGroupMorphObjects(const nanoem_model_morph_t *morph, nanoem_rsize_t *num_objects);
NANOEM_DECL_API nanoem_model_morph_impulse_t *const *APIENTRY
nanoemModelMorphGetAllImpulseMorphObjects(const nanoem_model_morph_t *morph, nanoem_rsize_t *num_objects);
NANOEM_DECL_API nanoem_model_morph_material_t *const *APIENTRY
nanoemModelMorphGetAllMaterialMorphObjects(const nanoem_model_morph_t *morph, nanoem_rsize_t *num_objects);
NANOEM_DECL_API nanoem_model_morph_uv_t *const *APIENTRY
nanoemModelMorphGetAllUVMorphObjects(const nanoem_model_morph_t *morph, nanoem_rsize_t *num_objects);
NANOEM_DECL_API nanoem_model_morph_vertex_t *const *APIENTRY
nanoemModelMorphGetAllVertexMorphObjects(const nanoem_model_morph_t *morph, nanoem_rsize_t *num_objects);
NANOEM_DECL_API const nanoem_model_object_t *APIENTRY
nanoemModelMorphGetModelObject(const nanoem_model_morph_t *morph);
NANOEM_DECL_API nanoem_model_object_t *APIENTRY
nanoemModelMorphGetModelObjectMutable(nanoem_model_morph_t *morph);
/** @} */

/**
 * \defgroup nanoem_model_label Label
 * @{
 */
NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_model_label_item_type_t){
    NANOEM_MODEL_LABEL_ITEM_TYPE_UNKNOWN = -1,
    NANOEM_MODEL_LABEL_ITEM_TYPE_FIRST_ENUM,
    NANOEM_MODEL_LABEL_ITEM_TYPE_BONE = NANOEM_MODEL_LABEL_ITEM_TYPE_FIRST_ENUM,
    NANOEM_MODEL_LABEL_ITEM_TYPE_MORPH,
    NANOEM_MODEL_LABEL_ITEM_TYPE_MAX_ENUM
};

NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemModelLabelGetName(const nanoem_model_label_t *label, nanoem_language_type_t language);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelLabelIsSpecial(const nanoem_model_label_t *label);
NANOEM_DECL_API nanoem_model_label_item_t *const *APIENTRY
nanoemModelLabelGetAllItemObjects(const nanoem_model_label_t *label, nanoem_rsize_t *num_objects);
NANOEM_DECL_API nanoem_model_label_item_type_t APIENTRY
nanoemModelLabelItemGetType(const nanoem_model_label_item_t *label_item);
NANOEM_DECL_API const nanoem_model_bone_t *APIENTRY
nanoemModelLabelItemGetBoneObject(const nanoem_model_label_item_t *label_item);
NANOEM_DECL_API const nanoem_model_morph_t *APIENTRY
nanoemModelLabelItemGetMorphObject(const nanoem_model_label_item_t *label_item);
NANOEM_DECL_API const nanoem_model_object_t *APIENTRY
nanoemModelLabelGetModelObject(const nanoem_model_label_t *label);
NANOEM_DECL_API nanoem_model_object_t *APIENTRY
nanoemModelLabelGetModelObjectMutable(nanoem_model_label_t *label);
/** @} */

/**
 * \defgroup nanoem_model_rigid_body Rigid Body
 * @{
 */
NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_model_rigid_body_shape_type_t){
    NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_UNKNOWN = -1,
    NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_FIRST_ENUM,
    NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_SPHERE = NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_FIRST_ENUM,
    NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_BOX,
    NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_CAPSULE,
    NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_MAX_ENUM
};

NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_model_rigid_body_transform_type_t){
    NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_UNKNOWN = -1,
    NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FIRST_ENUM,
    NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_TO_SIMULATION = NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FIRST_ENUM,
    NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_SIMULATION_TO_BONE,
    NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_ORIENTATION_AND_SIMULATION_TO_BONE,
    NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_MAX_ENUM
};

NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemModelRigidBodyGetName(const nanoem_model_rigid_body_t *rigid_body, nanoem_language_type_t language);
NANOEM_DECL_API const nanoem_model_bone_t *APIENTRY
nanoemModelRigidBodyGetBoneObject(const nanoem_model_rigid_body_t *rigid_body);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelRigidBodyGetOrigin(const nanoem_model_rigid_body_t *rigid_body);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelRigidBodyGetOrientation(const nanoem_model_rigid_body_t *rigid_body);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelRigidBodyGetShapeSize(const nanoem_model_rigid_body_t *rigid_body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelRigidBodyGetMass(const nanoem_model_rigid_body_t *rigid_body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelRigidBodyGetLinearDamping(const nanoem_model_rigid_body_t *rigid_body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelRigidBodyGetAngularDamping(const nanoem_model_rigid_body_t *rigid_body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelRigidBodyGetFriction(const nanoem_model_rigid_body_t *rigid_body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelRigidBodyGetRestitution(const nanoem_model_rigid_body_t *rigid_body);
NANOEM_DECL_API nanoem_model_rigid_body_shape_type_t APIENTRY
nanoemModelRigidBodyGetShapeType(const nanoem_model_rigid_body_t *rigid_body);
NANOEM_DECL_API nanoem_model_rigid_body_transform_type_t APIENTRY
nanoemModelRigidBodyGetTransformType(const nanoem_model_rigid_body_t *rigid_body);
NANOEM_DECL_API int APIENTRY
nanoemModelRigidBodyGetCollisionGroupId(const nanoem_model_rigid_body_t *rigid_body);
NANOEM_DECL_API int APIENTRY
nanoemModelRigidBodyGetCollisionMask(const nanoem_model_rigid_body_t *rigid_body);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelRigidBodyIsBoneRelativePosition(const nanoem_model_rigid_body_t *rigid_body);
NANOEM_DECL_API const nanoem_model_object_t *APIENTRY
nanoemModelRigidBodyGetModelObject(const nanoem_model_rigid_body_t *rigid_body);
NANOEM_DECL_API nanoem_model_object_t *APIENTRY
nanoemModelRigidBodyGetModelObjectMutable(nanoem_model_rigid_body_t *rigid_body);
/** @} */

/**
 * \defgroup nanoem_model_joint Joint
 * @{
 */
NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_model_joint_type_t){
    NANOEM_MODEL_JOINT_TYPE_UNKNOWN = -1,
    NANOEM_MODEL_JOINT_TYPE_FIRST_ENUM,
    NANOEM_MODEL_JOINT_TYPE_GENERIC_6DOF_SPRING_CONSTRAINT = NANOEM_MODEL_JOINT_TYPE_FIRST_ENUM,
    NANOEM_MODEL_JOINT_TYPE_GENERIC_6DOF_CONSTRAINT,
    NANOEM_MODEL_JOINT_TYPE_POINT2POINT_CONSTRAINT,
    NANOEM_MODEL_JOINT_TYPE_CONE_TWIST_CONSTRAINT,
    NANOEM_MODEL_JOINT_TYPE_SLIDER_CONSTRAINT,
    NANOEM_MODEL_JOINT_TYPE_HINGE_CONSTRAINT,
    NANOEM_MODEL_JOINT_TYPE_MAX_ENUM
};

NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemModelJointGetName(const nanoem_model_joint_t *joints, nanoem_language_type_t language);
NANOEM_DECL_API const nanoem_model_rigid_body_t *APIENTRY
nanoemModelJointGetRigidBodyAObject(const nanoem_model_joint_t *joint);
NANOEM_DECL_API const nanoem_model_rigid_body_t *APIENTRY
nanoemModelJointGetRigidBodyBObject(const nanoem_model_joint_t *joint);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelJointGetOrigin(const nanoem_model_joint_t *joint);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelJointGetOrientation(const nanoem_model_joint_t *joint);
NANOEM_DECL_API nanoem_model_joint_type_t APIENTRY
nanoemModelJointGetType(const nanoem_model_joint_t *joint);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelJointGetLinearUpperLimit(const nanoem_model_joint_t *joint);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelJointGetLinearLowerLimit(const nanoem_model_joint_t *joint);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelJointGetLinearStiffness(const nanoem_model_joint_t *joint);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelJointGetAngularUpperLimit(const nanoem_model_joint_t *joint);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelJointGetAngularLowerLimit(const nanoem_model_joint_t *joint);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemModelJointGetAngularStiffness(const nanoem_model_joint_t *joint);
NANOEM_DECL_API const nanoem_model_object_t *APIENTRY
nanoemModelJointGetModelObject(const nanoem_model_joint_t *vertex);
NANOEM_DECL_API nanoem_model_object_t *APIENTRY
nanoemModelJointGetModelObjectMutable(nanoem_model_joint_t *vertex);
/** @} */

/**
 * \defgroup nanoem_model_softbody Soft Body
 * @{
 */
NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_model_soft_body_shape_type_t){
    NANOEM_MODEL_SOFT_BODY_SHAPE_TYPE_UNKNOWN = -1,
    NANOEM_MODEL_SOFT_BODY_SHAPE_TYPE_FIRST_ENUM,
    NANOEM_MODEL_SOFT_BODY_SHAPE_TYPE_TRI_MESH = NANOEM_MODEL_SOFT_BODY_SHAPE_TYPE_FIRST_ENUM,
    NANOEM_MODEL_SOFT_BODY_SHAPE_TYPE_ROPE,
    NANOEM_MODEL_SOFT_BODY_SHAPE_TYPE_MAX_ENUM
};
NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_model_soft_body_aero_model_type_t){
    NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_UNKNOWN = -1,
    NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_FIRST_ENUM,
    NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_VERTEX_POINT = NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_FIRST_ENUM,
    NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_VERTEX_TWO_SIDED,
    NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_VERTEX_ONE_SIDED,
    NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_FACE_TWO_SIDED,
    NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_FACE_ONE_SIDED,
    NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_MAX_ENUM
};

NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemModelSoftBodyGetName(const nanoem_model_soft_body_t *body, nanoem_language_type_t language);
NANOEM_DECL_API nanoem_model_soft_body_anchor_t *const * APIENTRY
nanoemModelSoftBodyGetAllAnchorObjects(const nanoem_model_soft_body_t *body, nanoem_rsize_t *num_objects);
NANOEM_DECL_API const nanoem_u32_t * APIENTRY
nanoemModelSoftBodyGetAllPinnedVertexIndices(const nanoem_model_soft_body_t *body, nanoem_rsize_t *num_objects);
NANOEM_DECL_API const nanoem_model_material_t *APIENTRY
nanoemModelSoftBodyGetMaterialObject(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_model_soft_body_shape_type_t APIENTRY
nanoemModelSoftBodyGetShapeType(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_model_soft_body_aero_model_type_t APIENTRY
nanoemModelSoftBodyGetAeroModel(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetTotalMass(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetCollisionMargin(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetVelocityCorrectionFactor(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetDampingCoefficient(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetDragCoefficient(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetLiftCoefficient(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetPressureCoefficient(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetVolumeConversationCoefficient(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetDynamicFrictionCoefficient(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetPoseMatchingCoefficient(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetRigidContactHardness(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetKineticContactHardness(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetSoftContactHardness(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetAnchorHardness(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetSoftVSRigidHardness(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetSoftVSKineticHardness(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetSoftVSSoftHardness(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetSoftVSRigidImpulseSplit(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetSoftVSKineticImpulseSplit(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetSoftVSSoftImpulseSplit(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetLinearStiffnessCoefficient(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetAngularStiffnessCoefficient(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetVolumeStiffnessCoefficient(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API int APIENTRY
nanoemModelSoftBodyGetCollisionGroupId(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API int APIENTRY
nanoemModelSoftBodyGetCollisionMask(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API int APIENTRY
nanoemModelSoftBodyGetBendingConstraintsDistance(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API int APIENTRY
nanoemModelSoftBodyGetClusterCount(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API int APIENTRY
nanoemModelSoftBodyGetVelocitySolverIterations(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API int APIENTRY
nanoemModelSoftBodyGetPositionsSolverIterations(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API int APIENTRY
nanoemModelSoftBodyGetDriftSolverIterations(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API int APIENTRY
nanoemModelSoftBodyGetClusterSolverIterations(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelSoftBodyIsBendingConstraintsEnabled(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelSoftBodyIsClustersEnabled(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelSoftBodyIsRandomizeConstraintsNeeded(const nanoem_model_soft_body_t *body);

/**
 * \defgroup nanoem_model_softbody_anchor Soft Body Anchor
 * @{
 */
NANOEM_DECL_API const nanoem_model_rigid_body_t *APIENTRY
nanoemModelSoftBodyAnchorGetRigidBodyObject(const nanoem_model_soft_body_anchor_t *anchor);
NANOEM_DECL_API const nanoem_model_vertex_t *APIENTRY
nanoemModelSoftBodyAnchorGetVertexObject(const nanoem_model_soft_body_anchor_t *anchor);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelSoftBodyAnchorIsNearEnabled(const nanoem_model_soft_body_anchor_t *anchor);
NANOEM_DECL_API const nanoem_model_object_t *APIENTRY
nanoemModelSoftBodyGetModelObject(const nanoem_model_soft_body_t *body);
NANOEM_DECL_API nanoem_model_object_t *APIENTRY
nanoemModelSoftBodyGetModelObjectMutable(nanoem_model_soft_body_t *body);
/** @} */

/** @} */

NANOEM_DECL_API nanoem_model_t *APIENTRY
nanoemModelCreate(nanoem_unicode_string_factory_t *factory, nanoem_status_t *status);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelLoadFromBufferPMD(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelLoadFromBufferPMX(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemModelLoadFromBuffer(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_API nanoem_model_format_type_t APIENTRY
nanoemModelGetFormatType(const nanoem_model_t *model);
NANOEM_DECL_API nanoem_codec_type_t APIENTRY
nanoemModelGetCodecType(const nanoem_model_t *model);
NANOEM_DECL_API nanoem_rsize_t APIENTRY
nanoemModelGetAdditionalUVSize(const nanoem_model_t *model);
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemModelGetName(const nanoem_model_t *model, nanoem_language_type_t language);
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemModelGetComment(const nanoem_model_t *model, nanoem_language_type_t language);
NANOEM_DECL_API nanoem_model_vertex_t *const *APIENTRY
nanoemModelGetAllVertexObjects(const nanoem_model_t *model, nanoem_rsize_t *num_vertices);
NANOEM_DECL_API const nanoem_u32_t *APIENTRY
nanoemModelGetAllVertexIndices(const nanoem_model_t *model, nanoem_rsize_t *num_indices);
NANOEM_DECL_API nanoem_model_material_t *const *APIENTRY
nanoemModelGetAllMaterialObjects(const nanoem_model_t *model, nanoem_rsize_t *num_materials);
NANOEM_DECL_API nanoem_model_bone_t *const *APIENTRY
nanoemModelGetAllBoneObjects(const nanoem_model_t *model, nanoem_rsize_t *num_bones);
NANOEM_DECL_API nanoem_model_bone_t *const *APIENTRY
nanoemModelGetAllOrderedBoneObjects(const nanoem_model_t *model, nanoem_rsize_t *num_bones);
NANOEM_DECL_API nanoem_model_constraint_t *const *APIENTRY
nanoemModelGetAllConstraintObjects(const nanoem_model_t *model, nanoem_rsize_t *num_constraints);
NANOEM_DECL_API nanoem_model_texture_t *const *APIENTRY
nanoemModelGetAllTextureObjects(const nanoem_model_t *model, nanoem_rsize_t *num_textures);
NANOEM_DECL_API nanoem_model_morph_t *const *APIENTRY
nanoemModelGetAllMorphObjects(const nanoem_model_t *model, nanoem_rsize_t *num_morphs);
NANOEM_DECL_API nanoem_model_label_t *const *APIENTRY
nanoemModelGetAllLabelObjects(const nanoem_model_t *model, nanoem_rsize_t *num_labels);
NANOEM_DECL_API nanoem_model_rigid_body_t *const *APIENTRY
nanoemModelGetAllRigidBodyObjects(const nanoem_model_t *model, nanoem_rsize_t *num_rigid_bodies);
NANOEM_DECL_API nanoem_model_joint_t *const *APIENTRY
nanoemModelGetAllJointObjects(const nanoem_model_t *model, nanoem_rsize_t *num_joints);
NANOEM_DECL_API nanoem_model_soft_body_t *const *APIENTRY
nanoemModelGetAllSoftBodyObjects(const nanoem_model_t *model, nanoem_rsize_t *num_soft_bodies);
NANOEM_DECL_API void APIENTRY
nanoemModelDestroy(nanoem_model_t *model);
/** @} */

/**
 * \defgroup nanoem_motion_keyframe Motion
 * @{
 */
NANOEM_DECL_OPAQUE(nanoem_motion_keyframe_object_t);
NANOEM_DECL_OPAQUE(nanoem_motion_effect_parameter_t);
NANOEM_DECL_OPAQUE(nanoem_motion_outside_parent_t);
NANOEM_DECL_OPAQUE(nanoem_motion_accessory_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_motion_bone_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_motion_camera_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_motion_light_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_motion_model_keyframe_constraint_state_t);
NANOEM_DECL_OPAQUE(nanoem_motion_model_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_motion_morph_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_motion_physics_world_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_motion_self_shadow_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_motion_t);
typedef nanoem_u32_t nanoem_frame_index_t;

NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_motion_format_type_t){
    NANOEM_MOTION_FORMAT_TYPE_UNKNOWN = -1,
    NANOEM_MOTION_FORMAT_TYPE_FIRST_ENUM,
    NANOEM_MOTION_FORMAT_TYPE_VMD = NANOEM_MOTION_FORMAT_TYPE_FIRST_ENUM,
    NANOEM_MOTION_FORMAT_TYPE_NMD,
    NANOEM_MOTION_FORMAT_TYPE_MAX_ENUM
};

/**
 * \defgroup nanoem_keyframe_object Keyframe Object
 * @{
 */
NANOEM_DECL_API const nanoem_motion_t *APIENTRY
nanoemMotionKeyframeObjectGetParentMotion(const nanoem_motion_keyframe_object_t *object);
NANOEM_DECL_API int APIENTRY
nanoemMotionKeyframeObjectGetIndex(const nanoem_motion_keyframe_object_t *object);
NANOEM_DECL_API nanoem_frame_index_t APIENTRY
nanoemMotionKeyframeObjectGetFrameIndex(const nanoem_motion_keyframe_object_t *object);
NANOEM_DECL_API nanoem_frame_index_t APIENTRY
nanoemMotionKeyframeObjectGetFrameIndexWithOffset(const nanoem_motion_keyframe_object_t *object, int offset);
NANOEM_DECL_API nanoem_user_data_t *APIENTRY
nanoemMotionKeyframeObjectGetUserDataObject(const nanoem_motion_keyframe_object_t *object);
NANOEM_DECL_API void APIENTRY
nanoemMotionKeyframeObjectSetUserDataObject(nanoem_motion_keyframe_object_t *object, nanoem_user_data_t *user_data);
NANOEM_DECL_API const char *APIENTRY
nanoemMotionKeyframeObjectGetAnnotation(const nanoem_motion_keyframe_object_t *object, const char *key);
NANOEM_DECL_API void APIENTRY
nanoemMotionKeyframeObjectSetAnnotation(nanoem_motion_keyframe_object_t *object, const char *key, const char *value, nanoem_status_t *status);
/** @} */

/**
 * \defgroup nanoem_motion_effect_parameter Effect Parameter
 * @{
 */
NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_motion_effect_parameter_type_t) {
    NANOEM_MOTION_EFFECT_PARAMETER_TYPE_UNKNOWN = -1,
    NANOEM_MOTION_EFFECT_PARAMETER_TYPE_FIRST_ENUM,
    NANOEM_MOTION_EFFECT_PARAMETER_TYPE_BOOL = NANOEM_MOTION_EFFECT_PARAMETER_TYPE_FIRST_ENUM,
    NANOEM_MOTION_EFFECT_PARAMETER_TYPE_INT,
    NANOEM_MOTION_EFFECT_PARAMETER_TYPE_FLOAT,
    NANOEM_MOTION_EFFECT_PARAMETER_TYPE_VECTOR4,
    NANOEM_MOTION_EFFECT_PARAMETER_TYPE_MAX_ENUM_ENUM
};

NANOEM_DECL_API const nanoem_motion_t *APIENTRY
nanoemMotionEffectParameterGetParentMotion(const nanoem_motion_effect_parameter_t *parameter);
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemMotionEffectParameterGetName(const nanoem_motion_effect_parameter_t *parameter);
NANOEM_DECL_API nanoem_motion_effect_parameter_type_t APIENTRY
nanoemMotionEffectParameterGetType(const nanoem_motion_effect_parameter_t *parameter);
NANOEM_DECL_API const void *APIENTRY
nanoemMotionEffectParameterGetValue(const nanoem_motion_effect_parameter_t *parameter);
/** @} */

/**
 * \defgroup nanoem_motion_outside_parent Outside Parent
 * @{
 */
NANOEM_DECL_API const nanoem_motion_t *APIENTRY
nanoemMotionOutsideParentGetParentMotion(const nanoem_motion_outside_parent_t *op);
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemMotionOutsideParentGetTargetBoneName(const nanoem_motion_outside_parent_t *op);
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemMotionOutsideParentGetTargetObjectName(const nanoem_motion_outside_parent_t *op);
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemMotionOutsideParentGetSubjectBoneName(const nanoem_motion_outside_parent_t *op);
/** @} */

/**
 * \defgroup nanoem_motion_accessory_keyframe Accessory Keyframe
 * @{
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemMotionAccessoryKeyframeGetTranslation(const nanoem_motion_accessory_keyframe_t *keyframe);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemMotionAccessoryKeyframeGetOrientation(const nanoem_motion_accessory_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemMotionAccessoryKeyframeGetScaleFactor(const nanoem_motion_accessory_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemMotionAccessoryKeyframeGetOpacity(const nanoem_motion_accessory_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMotionAccessoryKeyframeIsVisible(const nanoem_motion_accessory_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMotionAccessoryKeyframeIsAddBlendEnabled(const nanoem_motion_accessory_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMotionAccessoryKeyframeIsShadowEnabled(const nanoem_motion_accessory_keyframe_t *keyframe);
NANOEM_DECL_API const nanoem_motion_outside_parent_t *APIENTRY
nanoemMotionAccessoryKeyframeGetOutsideParent(const nanoem_motion_accessory_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_motion_outside_parent_t *APIENTRY
nanoemMotionAccessoryKeyframeGetOutsideParentMutable(nanoem_motion_accessory_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_motion_effect_parameter_t *const *APIENTRY
nanoemMotionAccessoryKeyframeGetAllEffectParameterObjects(const nanoem_motion_accessory_keyframe_t *keyframe, nanoem_rsize_t *num_object);
NANOEM_DECL_API const nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionAccessoryKeyframeGetKeyframeObject(const nanoem_motion_accessory_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionAccessoryKeyframeGetKeyframeObjectMutable(nanoem_motion_accessory_keyframe_t *keyframe);
/** @} */

/**
 * \defgroup nanoem_motion_bone_keyframe Bone Keyframe
 * @{
 */
NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_motion_bone_keyframe_interpolation_type_t) {
    NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_UNKNOWN = -1,
    NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM,
    NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM,
    NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y,
    NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z,
    NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION,
    NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM
};

NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemMotionBoneKeyframeGetName(const nanoem_motion_bone_keyframe_t *keyframe);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemMotionBoneKeyframeGetTranslation(const nanoem_motion_bone_keyframe_t *keyframe);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemMotionBoneKeyframeGetOrientation(const nanoem_motion_bone_keyframe_t *keyframe);
NANOEM_DECL_API int APIENTRY
nanoemMotionBoneKeyframeGetId(const nanoem_motion_bone_keyframe_t *keyframe);
NANOEM_DECL_API const nanoem_u8_t *APIENTRY
nanoemMotionBoneKeyframeGetInterpolation(const nanoem_motion_bone_keyframe_t *keyframe, nanoem_motion_bone_keyframe_interpolation_type_t index);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMotionBoneKeyframeIsLinearInterpolation(const nanoem_motion_bone_keyframe_t *keyframe, nanoem_motion_bone_keyframe_interpolation_type_t index);
NANOEM_DECL_API nanoem_u32_t APIENTRY
nanoemMotionBoneKeyframeGetStageIndex(const nanoem_motion_bone_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMotionBoneKeyframeIsPhysicsSimulationEnabled(const nanoem_motion_bone_keyframe_t *keyframe);
NANOEM_DECL_API const nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionBoneKeyframeGetKeyframeObject(const nanoem_motion_bone_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionBoneKeyframeGetKeyframeObjectMutable(nanoem_motion_bone_keyframe_t *keyframe);
/** @} */

/**
 * \defgroup nanoem_motion_camera_keyframe Camera Keyframe
 * @{
 */
NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_motion_camera_keyframe_interpolation_type_t) {
    NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_UNKNOWN = -1,
    NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM,
    NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM,
    NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y,
    NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z,
    NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE,
    NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV,
    NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE,
    NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM
};

NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemMotionCameraKeyframeGetLookAt(const nanoem_motion_camera_keyframe_t *keyframe);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemMotionCameraKeyframeGetAngle(const nanoem_motion_camera_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemMotionCameraKeyframeGetDistance(const nanoem_motion_camera_keyframe_t *keyframe);
NANOEM_DECL_API int APIENTRY
nanoemMotionCameraKeyframeGetFov(const nanoem_motion_camera_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMotionCameraKeyframeIsPerspectiveView(const nanoem_motion_camera_keyframe_t *keyframe);
NANOEM_DECL_API const nanoem_u8_t *APIENTRY
nanoemMotionCameraKeyframeGetInterpolation(const nanoem_motion_camera_keyframe_t *keyframe, nanoem_motion_camera_keyframe_interpolation_type_t index);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMotionCameraKeyframeIsLinearInterpolation(const nanoem_motion_camera_keyframe_t *keyframe, nanoem_motion_camera_keyframe_interpolation_type_t index);
NANOEM_DECL_API nanoem_u32_t APIENTRY
nanoemMotionCameraKeyframeGetStageIndex(const nanoem_motion_camera_keyframe_t *keyframe);
NANOEM_DECL_API const nanoem_motion_outside_parent_t *APIENTRY
nanoemMotionCameraKeyframeGetOutsideParent(const nanoem_motion_camera_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_motion_outside_parent_t *APIENTRY
nanoemMotionCameraKeyframeGetOutsideParentMutable(nanoem_motion_camera_keyframe_t *keyframe);
NANOEM_DECL_API const nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionCameraKeyframeGetKeyframeObject(const nanoem_motion_camera_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionCameraKeyframeGetKeyframeObjectMutable(nanoem_motion_camera_keyframe_t *keyframe);
/** @} */

/**
 * \defgroup nanoem_motion_light_keyframe Light Keyframe
 * @{
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemMotionLightKeyframeGetColor(const nanoem_motion_light_keyframe_t *keyframe);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemMotionLightKeyframeGetDirection(const nanoem_motion_light_keyframe_t *keyframe);
NANOEM_DECL_API const nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionLightKeyframeGetKeyframeObject(const nanoem_motion_light_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionLightKeyframeGetKeyframeObjectMutable(nanoem_motion_light_keyframe_t *keyframe);
/** @} */

/**
 * \defgroup nanoem_motion_model_keyframe Model Keyframe
 * @{
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMotionModelKeyframeIsVisible(const nanoem_motion_model_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemMotionModelKeyframeGetEdgeScaleFactor(const nanoem_motion_model_keyframe_t *keyframe);
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemMotionModelKeyframeGetEdgeColor(const nanoem_motion_model_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMotionModelKeyframeIsAddBlendEnabled(const nanoem_motion_model_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMotionModelKeyframeIsPhysicsSimulationEnabled(const nanoem_motion_model_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_motion_model_keyframe_constraint_state_t *const *APIENTRY
nanoemMotionModelKeyframeGetAllConstraintStateObjects(const nanoem_motion_model_keyframe_t *keyframe, nanoem_rsize_t *num_objects);
NANOEM_DECL_API nanoem_motion_effect_parameter_t *const *APIENTRY
nanoemMotionModelKeyframeGetAllEffectParameterObjects(const nanoem_motion_model_keyframe_t *keyframe, nanoem_rsize_t *num_objects);
NANOEM_DECL_API nanoem_motion_outside_parent_t *const *APIENTRY
nanoemMotionModelKeyframeGetAllOutsideParentObjects(const nanoem_motion_model_keyframe_t *keyframe, nanoem_rsize_t *num_objects);
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemMotionModelKeyframeConstraintStateGetBoneName(const nanoem_motion_model_keyframe_constraint_state_t *state);
NANOEM_DECL_API int APIENTRY
nanoemMotionModelKeyframeConstraintStateGetBoneId(const nanoem_motion_model_keyframe_constraint_state_t *state);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMotionModelKeyframeConstraintStateIsEnabled(const nanoem_motion_model_keyframe_constraint_state_t *state);
NANOEM_DECL_API const nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionModelKeyframeGetKeyframeObject(const nanoem_motion_model_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionModelKeyframeGetKeyframeObjectMutable(nanoem_motion_model_keyframe_t *keyframe);
/** @} */

/**
 * \defgroup nanoem_motion_morph_keyframe Morph Keyframe
 * @{
 */
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemMotionMorphKeyframeGetName(const nanoem_motion_morph_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemMotionMorphKeyframeGetWeight(const nanoem_motion_morph_keyframe_t *keyframe);
NANOEM_DECL_API int APIENTRY
nanoemMotionMorphKeyframeGetId(const nanoem_motion_morph_keyframe_t *keyframe);
NANOEM_DECL_API const nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionMorphKeyframeGetKeyframeObject(const nanoem_motion_morph_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionMorphKeyframeGetKeyframeObjectMutable(nanoem_motion_morph_keyframe_t *keyframe);
/** @} */

/**
 * \defgroup nanoem_motion_morph_keyframe Morph Keyframe
 * @{
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemMotionSelfShadowKeyframeGetDistance(const nanoem_motion_self_shadow_keyframe_t *keyframe);
NANOEM_DECL_API int APIENTRY
nanoemMotionSelfShadowKeyframeGetMode(const nanoem_motion_self_shadow_keyframe_t *keyframe);
NANOEM_DECL_API const nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionSelfShadowKeyframeGetKeyframeObject(const nanoem_motion_self_shadow_keyframe_t *keyframe);
NANOEM_DECL_API nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionSelfShadowKeyframeGetKeyframeObjectMutable(nanoem_motion_self_shadow_keyframe_t *keyframe);
/** @} */

NANOEM_DECL_API nanoem_motion_t *APIENTRY
nanoemMotionCreate(nanoem_unicode_string_factory_t *factory, nanoem_status_t *status);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMotionLoadFromBufferVMD(nanoem_motion_t *motion, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status);
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMotionLoadFromBuffer(nanoem_motion_t *motion, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status);
NANOEM_DECL_API nanoem_motion_format_type_t APIENTRY
nanoemMotionGetFormatType(const nanoem_motion_t *motion);
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemMotionGetTargetModelName(const nanoem_motion_t *motion);
NANOEM_DECL_API nanoem_frame_index_t APIENTRY
nanoemMotionGetMaxFrameIndex(const nanoem_motion_t *motion);
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemMotionGetPreferredFPS(const nanoem_motion_t *motion);
NANOEM_DECL_API const char *APIENTRY
nanoemMotionGetAnnotation(const nanoem_motion_t *motion, const char *key);
NANOEM_DECL_API nanoem_motion_accessory_keyframe_t *const *APIENTRY
nanoemMotionGetAllAccessoryKeyframeObjects(const nanoem_motion_t *motion, nanoem_rsize_t *num_keyframes);
NANOEM_DECL_API nanoem_motion_bone_keyframe_t *const *APIENTRY
nanoemMotionGetAllBoneKeyframeObjects(const nanoem_motion_t *motion, nanoem_rsize_t *num_keyframes);
NANOEM_DECL_API nanoem_motion_camera_keyframe_t *const *APIENTRY
nanoemMotionGetAllCameraKeyframeObjects(const nanoem_motion_t *motion, nanoem_rsize_t *num_keyframes);
NANOEM_DECL_API nanoem_motion_light_keyframe_t *const *APIENTRY
nanoemMotionGetAllLightKeyframeObjects(const nanoem_motion_t *motion, nanoem_rsize_t *num_keyframes);
NANOEM_DECL_API nanoem_motion_model_keyframe_t *const *APIENTRY
nanoemMotionGetAllModelKeyframeObjects(const nanoem_motion_t *motion, nanoem_rsize_t *num_keyframes);
NANOEM_DECL_API nanoem_motion_morph_keyframe_t *const *APIENTRY
nanoemMotionGetAllMorphKeyframeObjects(const nanoem_motion_t *motion, nanoem_rsize_t *num_keyframes);
NANOEM_DECL_API nanoem_motion_self_shadow_keyframe_t *const *APIENTRY
nanoemMotionGetAllSelfShadowKeyframeObjects(const nanoem_motion_t *motion, nanoem_rsize_t *num_keyframes);
NANOEM_DECL_API nanoem_motion_bone_keyframe_t *const *APIENTRY
nanoemMotionExtractBoneTrackKeyframes(const nanoem_motion_t *motion, const nanoem_unicode_string_t *name, nanoem_rsize_t *num_keyframes, nanoem_status_t *status);
NANOEM_DECL_API nanoem_motion_morph_keyframe_t *const *APIENTRY
nanoemMotionExtractMorphTrackKeyframes(const nanoem_motion_t *motion, const nanoem_unicode_string_t *name, nanoem_rsize_t *num_keyframes, nanoem_status_t *status);
NANOEM_DECL_API const nanoem_motion_accessory_keyframe_t *APIENTRY
nanoemMotionFindAccessoryKeyframeObject(const nanoem_motion_t *motion, nanoem_frame_index_t index);
NANOEM_DECL_API const nanoem_motion_bone_keyframe_t *APIENTRY
nanoemMotionFindBoneKeyframeObject(const nanoem_motion_t *motion, const nanoem_unicode_string_t *name, nanoem_frame_index_t index);
NANOEM_DECL_API const nanoem_motion_camera_keyframe_t *APIENTRY
nanoemMotionFindCameraKeyframeObject(const nanoem_motion_t *motion, nanoem_frame_index_t index);
NANOEM_DECL_API const nanoem_motion_light_keyframe_t *APIENTRY
nanoemMotionFindLightKeyframeObject(const nanoem_motion_t *motion, nanoem_frame_index_t index);
NANOEM_DECL_API const nanoem_motion_model_keyframe_t *APIENTRY
nanoemMotionFindModelKeyframeObject(const nanoem_motion_t *motion, nanoem_frame_index_t index);
NANOEM_DECL_API const nanoem_motion_morph_keyframe_t *APIENTRY
nanoemMotionFindMorphKeyframeObject(const nanoem_motion_t *motion, const nanoem_unicode_string_t *name, nanoem_frame_index_t index);
NANOEM_DECL_API const nanoem_motion_self_shadow_keyframe_t *APIENTRY
nanoemMotionFindSelfShadowKeyframeObject(const nanoem_motion_t *motion, nanoem_frame_index_t index);
NANOEM_DECL_API void APIENTRY
nanoemMotionSearchClosestAccessoryKeyframes(const nanoem_motion_t *motion, nanoem_frame_index_t base_index, nanoem_motion_accessory_keyframe_t **prev_keyframe, nanoem_motion_accessory_keyframe_t **next_keyframe);
NANOEM_DECL_API void APIENTRY
nanoemMotionSearchClosestBoneKeyframes(const nanoem_motion_t *motion, const nanoem_unicode_string_t *name, nanoem_frame_index_t base_index, nanoem_motion_bone_keyframe_t **prev_keyframe, nanoem_motion_bone_keyframe_t **next_keyframe);
NANOEM_DECL_API void APIENTRY
nanoemMotionSearchClosestCameraKeyframes(const nanoem_motion_t *motion, nanoem_frame_index_t base_index, nanoem_motion_camera_keyframe_t **prev_keyframe, nanoem_motion_camera_keyframe_t **next_keyframe);
NANOEM_DECL_API void APIENTRY
nanoemMotionSearchClosestLightKeyframes(const nanoem_motion_t *motion, nanoem_frame_index_t base_index, nanoem_motion_light_keyframe_t **prev_keyframe, nanoem_motion_light_keyframe_t **next_keyframe);
NANOEM_DECL_API void APIENTRY
nanoemMotionSearchClosestModelKeyframes(const nanoem_motion_t *motion, nanoem_frame_index_t base_index, nanoem_motion_model_keyframe_t **prev_keyframe, nanoem_motion_model_keyframe_t **next_keyframe);
NANOEM_DECL_API void APIENTRY
nanoemMotionSearchClosestMorphKeyframes(const nanoem_motion_t *motion, const nanoem_unicode_string_t *name, nanoem_frame_index_t base_index, nanoem_motion_morph_keyframe_t **prev_keyframe, nanoem_motion_morph_keyframe_t **next_keyframe);
NANOEM_DECL_API void APIENTRY
nanoemMotionSearchClosestSelfShadowKeyframes(const nanoem_motion_t *motion, nanoem_frame_index_t base_index, nanoem_motion_self_shadow_keyframe_t **prev_keyframe, nanoem_motion_self_shadow_keyframe_t **next_keyframe);
NANOEM_DECL_API void APIENTRY
nanoemMotionDestroy(nanoem_motion_t *motion);
/** @} */

/**
 * \defgroup nanoem_userdata Custom User Data
 * @{
 */

/**
 * \defgroup nanoem_userdata_callbacks Custom User Data Callbacks
 * @{
 */
typedef void (*nanoem_user_data_on_destroy_model_t)(void *, nanoem_model_t *);
typedef void (*nanoem_user_data_on_destroy_motion_t)(void *, nanoem_motion_t *);
typedef void (*nanoem_user_data_on_destroy_model_object_t)(void *, nanoem_model_object_t *);
typedef void (*nanoem_user_data_on_destroy_keyframe_object_t)(void *, nanoem_motion_keyframe_object_t *);
/** @} */

NANOEM_DECL_API nanoem_user_data_t *APIENTRY
nanoemUserDataCreate(nanoem_status_t *status);
NANOEM_DECL_API void *APIENTRY
nanoemUserDataGetOpaqueData(const nanoem_user_data_t *user_data);
NANOEM_DECL_API void APIENTRY
nanoemUserDataSetOpaqueData(nanoem_user_data_t *user_data, void *opaque);
NANOEM_DECL_API void APIENTRY
nanoemUserDataSetOnDestroyModelCallback(nanoem_user_data_t *user_data, nanoem_user_data_on_destroy_model_t value);
NANOEM_DECL_API void APIENTRY
nanoemUserDataSetOnDestroyMotionCallback(nanoem_user_data_t *user_data, nanoem_user_data_on_destroy_motion_t value);
NANOEM_DECL_API void APIENTRY
nanoemUserDataSetOnDestroyModelObjectCallback(nanoem_user_data_t *user_data, nanoem_user_data_on_destroy_model_object_t value);
NANOEM_DECL_API void APIENTRY
nanoemUserDataSetOnDestroyKeyframeObjectCallback(nanoem_user_data_t *user_data, nanoem_user_data_on_destroy_keyframe_object_t value);
NANOEM_DECL_API void APIENTRY
nanoemUserDataDestroy(nanoem_user_data_t *user_data);
/** @} */

/** @} */

#endif /* NANOEM_H_ */
