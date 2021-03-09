/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#ifndef NANODXM_H_
#define NANODXM_H_

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L /* C11 */
#define __STDC_WANT_LIB_EXT1__ 1 /* define for rsize_t */
#endif

#include <stddef.h> /* size_t */
#ifdef _WIN32
#include <windows.h>
#endif

/**
 * \defgroup nanodxm nanodxm
 * @{
 */

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L /* C11 */
#ifndef NANODXM_STATIC_ASSERT
#define NANODXM_STATIC_ASSERT _Static_assert
#endif /* NANODXM_STATIC_ASSERT */
typedef rsize_t nanodxm_rsize_t;
#elif !defined(__cplusplus)
#ifndef NANODXM_STATIC_ASSERT
#define __NANODXM_SA_CONCAT(a, b) __NANODXM_SA_CONCAT2(a, b)
#define __NANODXM_SA_CONCAT2(a, b) a##b
#define NANODXM_STATIC_ASSERT(cond, message)                              \
    enum { __NANODXM_SA_CONCAT(NANODXM_STATIC_ASSERT_LINE_AT_, __LINE__) = \
               sizeof(struct __NANODXM_SA_CONCAT(__nanodxm_static_assert_line_at_, __LINE__) { int static_assertion_failed[(cond) ? 1 : -1]; }) }
#endif /* NANODXM_STATIC_ASSERT */
typedef size_t nanodxm_rsize_t;
#else
#define NANODXM_STATIC_ASSERT(cond, message)
typedef size_t nanodxm_rsize_t;
#endif /* __STDC_VERSION__ >= 201112L */

#ifndef NANODXM_DECL_ENUM
#if defined(__cplusplus) && __cplusplus >= 201103L
#define NANODXM_DECL_ENUM(type, name) enum name : type
#else
#define NANODXM_DECL_ENUM(type, name) \
    typedef type name;               \
    enum
#endif /*__STDC_VERSION__ >= 201112L */
#endif /* NANODXM_DECL_ENUM */

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L /* C99 */
#include <stdbool.h>
#include <stdint.h>
#ifndef NANODXM_DECL_INLINE
#define NANODXM_DECL_INLINE inline
#endif /* NANODXM_DECL_INLINE */
typedef _Bool nanodxm_bool_t;
typedef uint8_t nanodxm_uint8_t;
typedef int16_t nanodxm_int16_t;
typedef uint16_t nanodxm_uint16_t;
typedef int32_t nanodxm_int32_t;
typedef uint32_t nanodxm_uint32_t;
#define nanodxm_false false
#define nanodxm_true true
#elif defined(_MSC_VER) && _MSC_VER >= 1300
typedef unsigned __int8 nanodxm_uint8_t;
typedef signed __int16 nanodxm_int16_t;
typedef unsigned __int16 nanodxm_uint16_t;
typedef signed __int32 nanodxm_int32_t;
typedef unsigned __int32 nanodxm_uint32_t;
NANODXM_DECL_ENUM(int, nanodxm_bool_t){
    nanodxm_false,
    nanodxm_true
};
#ifndef NANODXM_DECL_INLINE
#define NANODXM_DECL_INLINE __inline
#endif
#else
typedef unsigned char nanodxm_uint8_t;
typedef short nanodxm_int16_t;
typedef unsigned short nanodxm_uint16_t;
typedef int nanodxm_int32_t;
typedef unsigned int nanodxm_uint32_t;
#ifndef NANODXM_DECL_INLINE
#define NANODXM_DECL_INLINE
#endif /* NANODXM_DECL_INLINE */
#if defined(__cplusplus) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
typedef bool nanodxm_bool_t;
#define nanodxm_true true
#define nanodxm_false false
#else
NANODXM_DECL_ENUM(int, nanodxm_bool_t){
    nanodxm_false,
    nanodxm_true
};
#endif
#endif /* __STDC_VERSION__ >= 199901L */

typedef float nanodxm_float32_t;
typedef double nanodxm_float64_t;

/* utility macros */
#ifdef NANODXM_ENABLE_BRANCH_PREDICTION
#define nanodxm_likely(expr) __builtin_expected(!!(expr), 1)
#define nanodxm_unlikely(expr) __builtin_expected(!!(expr), 0)
#else
#define nanodxm_likely(expr) (expr)
#define nanodxm_unlikely(expr) (expr)
#endif /* NANODXM_ENABLE_BRANCH_PREDICTION */

#define nanodxm_loop for (;;)
#define nanodxm_has_error(cond) (nanodxm_unlikely((cond) != NANODXM_STATUS_SUCCESS))
#define nanodxm_is_null(cond) (nanodxm_unlikely((cond) == NULL))
#define nanodxm_is_not_null(cond) (nanodxm_likely((cond) != NULL))
#define nanodxm_mark_unused(cond)              \
    nanodxm_loop                               \
    {                                         \
        (void)(0 ? (void) (cond) : (void) 0); \
        break;                                \
    }
#define nanodxm_fourcc(a, b, c, d) ((a) << 0 | (b) << 8 | (c) << 16 | (d) << 24)

#if defined(NANODXM_BUILDING_DLL)
#if defined(_WIN32)
#ifdef NANODXM_DLL_EXPORTS
#define NANODXM_DECL_API __declspec(dllexport)
#else
#define NANODXM_DECL_API __declspec(dllimport)
#endif
#elif defined(__GNUC__)
#ifdef __cplusplus
#define NANODXM_DECL_API __attribute__((visibility("default"))) extern "C"
#else
#define NANODXM_DECL_API __attribute__((visibility("default"))) extern
#endif /* __cplusplus */
#endif
#endif /* NANODXM_BUILDING_DLL */

#ifndef NANODXM_DECL_API
#ifdef __cplusplus
#define NANODXM_DECL_API extern "C"
#else
#define NANODXM_DECL_API extern
#endif /* __cplusplus */
#endif /* NANODXM_DECL_API */

#ifdef RSIZE_MAX
#define NANODXM_RSIZE_MAX RSIZE_MAX
#else
#define NANODXM_RSIZE_MAX ((~(size_t)(0)) >> 1)
#endif /* RSIZE_MAX */

#ifndef APIENTRY
#define APIENTRY
#endif /* APIENTRY */

NANODXM_STATIC_ASSERT(sizeof(nanodxm_uint8_t) == 1, "size of nanodxm_uint8_t must be 1");
NANODXM_STATIC_ASSERT(sizeof(nanodxm_int16_t) == 2, "size of nanodxm_int16_t must be 2");
NANODXM_STATIC_ASSERT(sizeof(nanodxm_uint16_t) == 2, "size of nanodxm_uint16_t must be 2");
NANODXM_STATIC_ASSERT(sizeof(nanodxm_int32_t) == 4, "size of nanodxm_int32_t must be 4");
NANODXM_STATIC_ASSERT(sizeof(nanodxm_uint32_t) == 4, "size of nanodxm_uint32_t must be 4");
NANODXM_STATIC_ASSERT(sizeof(nanodxm_float32_t) == 4, "size of nanodxm_float32_t must be 4");
NANODXM_STATIC_ASSERT(sizeof(nanodxm_float64_t) == 8, "size of nanodxm_float64_t must be 8");

typedef struct nanodxm_global_allocator_t nanodxm_global_allocator_t;
typedef struct nanodxm_buffer_t nanodxm_buffer_t;
typedef struct nanodxm_document_t nanodxm_document_t;
typedef struct nanodxm_material_t nanodxm_material_t;

struct nanodxm_vector3_t {
    nanodxm_float32_t x;
    nanodxm_float32_t y;
    nanodxm_float32_t z;
};
typedef struct nanodxm_vector3_t nanodxm_vector3_t;

struct nanodxm_color_t {
    nanodxm_float32_t r;
    nanodxm_float32_t g;
    nanodxm_float32_t b;
    nanodxm_float32_t a;
};
typedef struct nanodxm_color_t nanodxm_color_t;

struct nanodxm_texcoord_t {
    nanodxm_float32_t u;
    nanodxm_float32_t v;
};
typedef struct nanodxm_texcoord_t nanodxm_texcoord_t;

struct nanodxm_face_t {
    nanodxm_rsize_t num_indices;
    int *indices;
};
typedef struct nanodxm_face_t nanodxm_face_t;

enum nanodxm_status_t {
    NANODXM_STATUS_SUCCESS,
    NANODXM_STATUS_ERROR_NULL_BUFFER,
    NANODXM_STATUS_ERROR_NULL_DOCUMENT,
    NANODXM_STATUS_ERROR_NULL_SCENE,
    NANODXM_STATUS_ERROR_NULL_MATERIAL,
    NANODXM_STATUS_ERROR_NULL_OBJECT,
    NANODXM_STATUS_ERROR_NULL_VERTEX,
    NANODXM_STATUS_ERROR_NULL_FACE,
    NANODXM_STATUS_ERROR_INVALID_SIGNATURE = 100,
    NANODXM_STATUS_ERROR_INVALID_VERSION,
    NANODXM_STATUS_ERROR_INVALID_DATA_TYPE,
    NANODXM_STATUS_ERROR_INVALID_FLOAT_SIZE,
    NANODXM_STATUS_ERROR_INVALID_TOKEN,
    NANODXM_STATUS_ERROR_INVALID_EOF,
    NANODXM_STATUS_ERROR_NOT_SUPPORTED_DATA_TYPE,
    NANODXM_STATUS_ERROR_NOT_SUPPORTED_TOKEN,
    NANODXM_STATUS_ERROR_UNKNOWN_CHUNK_TYPE
};
typedef enum nanodxm_status_t nanodxm_status_t;

enum nanodxm_data_type_t {
    NANODXM_DATA_TYPE_ASCII,
    NANODXM_DATA_TYPE_BINARY,
    NANODXM_DATA_TYPE_ASCII_COMPRESSED,
    NANODXM_DATA_TYPE_BINARY_COMPRESSED,
    NANODXM_DATA_TYPE_MAX_ENUM
};
typedef enum nanodxm_data_type_t nanodxm_data_type_t;

typedef void *(*nanodxm_global_allocator_malloc_t)(void *, nanodxm_rsize_t, const char *file, int line);
typedef void *(*nanodxm_global_allocator_calloc_t)(void *, nanodxm_rsize_t, nanodxm_rsize_t, const char *file, int line);
typedef void *(*nanodxm_global_allocator_realloc_t)(void *, void *, nanodxm_rsize_t, const char *file, int line);
typedef void (*nanodxm_global_allocator_free_t)(void *, void *, const char *filename, int line);

struct nanodxm_global_allocator_t {
    void *opaque;
    nanodxm_global_allocator_malloc_t malloc;
    nanodxm_global_allocator_calloc_t calloc;
    nanodxm_global_allocator_realloc_t realloc;
    nanodxm_global_allocator_free_t free;
};

/**
 * \defgroup nanodxm_custom_allocator Custom Allocator
 * @{
 */
NANODXM_DECL_API const nanodxm_global_allocator_t *APIENTRY
nanodxmGlobalGetCustomAllocator(void);
NANODXM_DECL_API void APIENTRY
nanodxmGlobalSetCustomAllocator(const nanodxm_global_allocator_t *allocator);
/** @} */

/**
 * \defgroup nanodxm_buffer Buffer
 * @{
 */
NANODXM_DECL_API nanodxm_buffer_t *APIENTRY
nanodxmBufferCreate(const nanodxm_uint8_t *data, nanodxm_rsize_t length);
NANODXM_DECL_API nanodxm_rsize_t APIENTRY
nanodxmBufferGetLength(const nanodxm_buffer_t *buffer);
NANODXM_DECL_API nanodxm_rsize_t APIENTRY
nanodxmBufferGetOffset(const nanodxm_buffer_t *buffer);
NANODXM_DECL_API const nanodxm_uint8_t *APIENTRY
nanodxmBufferGetDataPtr(const nanodxm_buffer_t *buffer);
NANODXM_DECL_API int APIENTRY
nanodxmBufferGetRowOffset(const nanodxm_buffer_t *buffer);
NANODXM_DECL_API int APIENTRY
nanodxmBufferGetColumnOffset(const nanodxm_buffer_t *buffer);
NANODXM_DECL_API nanodxm_bool_t APIENTRY
nanodxmBufferCanReadLength(const nanodxm_buffer_t *buffer, nanodxm_rsize_t size);
NANODXM_DECL_API void APIENTRY
nanodxmBufferSkip(nanodxm_buffer_t *buffer, nanodxm_rsize_t skip, nanodxm_bool_t *ok);
NANODXM_DECL_API void APIENTRY
nanodxmBufferDestroy(nanodxm_buffer_t *buffer);
/** @} */

/**
 * \defgroup nanodxm_material Material
 * @{
 */
NANODXM_DECL_API const char *APIENTRY
nanodxmMaterialGetName(const nanodxm_material_t *material);
NANODXM_DECL_API nanodxm_color_t APIENTRY
nanodxmMaterialGetDiffuse(const nanodxm_material_t *material);
NANODXM_DECL_API nanodxm_color_t APIENTRY
nanodxmMaterialGetEmissive(const nanodxm_material_t *material);
NANODXM_DECL_API nanodxm_color_t APIENTRY
nanodxmMaterialGetSpecular(const nanodxm_material_t *material);
NANODXM_DECL_API nanodxm_float32_t APIENTRY
nanodxmMaterialGetShininess(const nanodxm_material_t *material);
NANODXM_DECL_API const nanodxm_uint8_t *APIENTRY
nanodxmMaterialGetTextureFilename(const nanodxm_material_t *material);
NANODXM_DECL_API const nanodxm_uint8_t *APIENTRY
nanodxmMaterialGetNormalMapFilename(const nanodxm_material_t *material);
/** @} */

/**
 * \defgroup nanodxm_document Document
 * @{
 */
NANODXM_DECL_API nanodxm_document_t *APIENTRY
nanodxmDocumentCreate(void);
NANODXM_DECL_API nanodxm_status_t APIENTRY
nanodxmDocumentParse(nanodxm_document_t *document, nanodxm_buffer_t *buffer);
NANODXM_DECL_API nanodxm_float32_t APIENTRY
nanodxmDocumentGetVersion(const nanodxm_document_t *document);
NANODXM_DECL_API const nanodxm_vector3_t *APIENTRY
nanodxmDocumentGetVertices(const nanodxm_document_t *document, nanodxm_rsize_t *length);
NANODXM_DECL_API const nanodxm_color_t *APIENTRY
nanodxmDocumentGetColors(const nanodxm_document_t *document, nanodxm_rsize_t *length);
NANODXM_DECL_API const nanodxm_vector3_t *APIENTRY
nanodxmDocumentGetNormals(const nanodxm_document_t *document, nanodxm_rsize_t *length);
NANODXM_DECL_API const nanodxm_texcoord_t *APIENTRY
nanodxmDocumentGetTexCoords(const nanodxm_document_t *document, nanodxm_rsize_t *length);
NANODXM_DECL_API const nanodxm_face_t *APIENTRY
nanodxmDocumentGetVertexFaces(const nanodxm_document_t *document, nanodxm_rsize_t *length);
NANODXM_DECL_API const nanodxm_face_t *APIENTRY
nanodxmDocumentGetNormalFaces(const nanodxm_document_t *document, nanodxm_rsize_t *length);
NANODXM_DECL_API nanodxm_material_t *const *APIENTRY
nanodxmDocumentGetMaterials(const nanodxm_document_t *document, nanodxm_rsize_t *length);
NANODXM_DECL_API const int *APIENTRY
nanodxmDocumentGetFaceMaterialIndices(const nanodxm_document_t *document, nanodxm_rsize_t *length);
NANODXM_DECL_API void APIENTRY
nanodxmDocumentDestroy(nanodxm_document_t *document);
/** @} */

#endif /* NANODXM_H_ */
