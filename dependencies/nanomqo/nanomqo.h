/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#ifndef NANOMQO_H_
#define NANOMQO_H_

#define NANOMQO_VERSION_MAJOR 0
#define NANOMQO_VERSION_MINOR 1
#define NANOMQO_VERSION_RELEASE 0
#define NANOMQO_VERSION_PATCH 0
#define NANOMQO_VERSION_STRING \
    NANOMQO_VERSION_MAJOR##"."##NANOMQO_VERSION_MINOR##"."##NANOMQO_VERSION_RELEASE##"."##NANOMQO_VERSION_PATCH

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L /* C11 */
#define __STDC_WANT_LIB_EXT1__ 1 /* define for rsize_t */
#endif

#include <stddef.h> /* size_t */
#ifdef _WIN32
#include <windows.h>
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L /* C11 */
#ifndef NANOMQO_STATIC_ASSERT
#define NANOMQO_STATIC_ASSERT _Static_assert
#endif /* NANOMQO_STATIC_ASSERT */
typedef rsize_t nanomqo_rsize_t;
#elif !defined(__cplusplus)
#ifndef NANOMQO_STATIC_ASSERT
#define __NANOMQO_SA_CONCAT(a, b) __NANOMQO_SA_CONCAT2(a, b)
#define __NANOMQO_SA_CONCAT2(a, b) a##b
#define NANOMQO_STATIC_ASSERT(cond, message)                               \
    enum { __NANOMQO_SA_CONCAT(NANOMQO_STATIC_ASSERT_LINE_AT_, __LINE__) = \
               sizeof(struct __NANOMQO_SA_CONCAT(__nanomqo_static_assert_line_at_, __LINE__) { int static_assertion_failed[(cond) ? 1 : -1]; }) }
#endif /* NANOMQO_STATIC_ASSERT */
typedef size_t nanomqo_rsize_t;
#else
#define NANOMQO_STATIC_ASSERT(cond, message)
typedef size_t nanomqo_rsize_t;
#endif /* __STDC_VERSION__ >= 201112L */

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L /* C99 */
#include <stdbool.h>
#include <stdint.h>
#ifndef NANOMQO_DECL_INLINE
#define NANOMQO_DECL_INLINE inline
#endif /* NANOMQO_DECL_INLINE */
typedef _Bool nanomqo_bool_t;
typedef uint8_t nanomqo_uint8_t;
typedef int16_t nanomqo_int16_t;
typedef uint16_t nanomqo_uint16_t;
typedef int32_t nanomqo_int32_t;
typedef uint32_t nanomqo_uint32_t;
#define nanomqo_false false
#define nanomqo_true true
#else
#ifndef NANOMQO_DECL_INLINE
#ifdef _WIN32
typedef unsigned __int8 nanomqo_uint8_t;
typedef signed __int16 nanomqo_int16_t;
typedef unsigned __int16 nanomqo_uint16_t;
typedef signed __int32 nanomqo_int32_t;
typedef unsigned __int32 nanomqo_uint32_t;
#define NANOMQO_DECL_INLINE __inline
#else
typedef unsigned char nanomqo_uint8_t;
typedef short nanomqo_int16_t;
typedef unsigned short nanomqo_uint16_t;
typedef int nanomqo_int32_t;
typedef unsigned int nanomqo_uint32_t;
#define NANOMQO_DECL_INLINE
#endif
#endif /* NANOMQO_DECL_INLINE */
#ifdef __cplusplus
typedef bool nanomqo_bool_t;
#define nanomqo_false false
#define nanomqo_true true
#else
enum nanomqo_bool_t {
    nanomqo_false,
    nanomqo_true,
    NANOMQO_BOOL_SIZE_ALIGNMENT = 0x7fffffff
};
typedef enum nanomqo_bool_t nanomqo_bool_t;
#endif /* __cplusplus */
#endif /* __STDC_VERSION__ >= 199901L */

typedef float nanomqo_float32_t;
typedef double nanomqo_float64_t;

/* branch predication macros */
#ifdef NANOMQO_ENABLE_BRANCH_PREDICTION
#define nanomqo_likely(expr) __builtin_expected(!!(expr), 1)
#define nanomqo_unlikely(expr) __builtin_expected(!!(expr), 0)
#else
#define nanomqo_likely(expr) (expr)
#define nanomqo_unlikely(expr) (expr)
#endif

/* utility macros */
#define nanomqo_loop for (;;)
#define nanomqo_has_error(cond) (nanomqo_unlikely((cond) != NANOMQO_STATUS_SUCCESS))
#define nanomqo_is_null(cond) (nanomqo_unlikely((cond) == NULL))
#define nanomqo_is_not_null(cond) (nanomqo_likely((cond) != NULL))
#define nanomqo_mark_unused(cond)              \
    nanomqo_loop                               \
    {                                          \
        (void) (0 ? (void) (cond) : (void) 0); \
        break;                                 \
    }

#if defined(NANOMQO_BUILDING_DLL)
#if defined(_WIN32)
#ifdef NANOMQO_DLL_EXPORTS
#define NANOMQO_DECL_API __declspec(dllexport)
#else
#define NANOMQO_DECL_API __declspec(dllimport)
#endif
#elif defined(__GNUC__)
#ifdef __cplusplus
#define NANOMQO_DECL_API __attribute__((visibility("default"))) extern "C"
#else
#define NANOMQO_DECL_API __attribute__((visibility("default"))) extern
#endif /* __cplusplus */
#endif
#endif /* NANOMQO_BUILDING_DLL */

#ifndef NANOMQO_DECL_API
#ifdef __cplusplus
#define NANOMQO_DECL_API extern "C"
#else
#define NANOMQO_DECL_API extern
#endif /* __cplusplus */
#endif /* NANOMQO_DECL_API */

#ifdef RSIZE_MAX
#define NANOMQO_RSIZE_MAX RSIZE_MAX
#else
#define NANOMQO_RSIZE_MAX ((~(size_t)(0)) >> 1)
#endif /* RSIZE_MAX */

#ifndef APIENTRY
#define APIENTRY
#endif /* APIENTRY */

NANOMQO_STATIC_ASSERT(sizeof(nanomqo_uint8_t) == 1, "size of nanomqo_uint8_t must be 1");
NANOMQO_STATIC_ASSERT(sizeof(nanomqo_int16_t) == 2, "size of nanomqo_int16_t must be 2");
NANOMQO_STATIC_ASSERT(sizeof(nanomqo_uint16_t) == 2, "size of nanomqo_uint16_t must be 2");
NANOMQO_STATIC_ASSERT(sizeof(nanomqo_int32_t) == 4, "size of nanomqo_int32_t must be 4");
NANOMQO_STATIC_ASSERT(sizeof(nanomqo_uint32_t) == 4, "size of nanomqo_uint32_t must be 4");
NANOMQO_STATIC_ASSERT(sizeof(nanomqo_float32_t) == 4, "size of nanomqo_float32_t must be 4");
NANOMQO_STATIC_ASSERT(sizeof(nanomqo_float64_t) == 8, "size of nanomqo_float64_t must be 8");

typedef struct nanomqo_global_allocator_t nanomqo_global_allocator_t;
typedef struct nanomqo_buffer_t nanomqo_buffer_t;
typedef struct nanomqo_document_t nanomqo_document_t;
typedef struct nanomqo_scene_t nanomqo_scene_t;
typedef struct nanomqo_material_t nanomqo_material_t;
typedef struct nanomqo_object_t nanomqo_object_t;
typedef struct nanomqo_vertex_t nanomqo_vertex_t;
typedef struct nanomqo_face_t nanomqo_face_t;
typedef struct nanomqo_color_t nanomqo_color_t;

enum nanomqo_status_t {
    NANOMQO_STATUS_SUCCESS,
    NANOMQO_STATUS_ERROR_NULL_BUFFER,
    NANOMQO_STATUS_ERROR_NULL_DOCUMENT,
    NANOMQO_STATUS_ERROR_NULL_SCENE,
    NANOMQO_STATUS_ERROR_NULL_MATERIAL,
    NANOMQO_STATUS_ERROR_NULL_OBJECT,
    NANOMQO_STATUS_ERROR_NULL_VERTEX,
    NANOMQO_STATUS_ERROR_NULL_FACE,
    NANOMQO_STATUS_ERROR_INVALID_SIGNATURE = 100,
    NANOMQO_STATUS_ERROR_INVALID_VERSION,
    NANOMQO_STATUS_ERROR_INVALID_SCENE_CHUNK,
    NANOMQO_STATUS_ERROR_INVALID_MATERIAL_CHUNK,
    NANOMQO_STATUS_ERROR_INVALID_OBJECT_CHUNK,
    NANOMQO_STATUS_ERROR_INVALID_VERTEX_CHUNK,
    NANOMQO_STATUS_ERROR_INVALID_FACE_CHUNK,
    NANOMQO_STATUS_ERROR_INVALID_EOF,
    NANOMQO_STATUS_ERROR_UNKNOWN_CHUNK_TYPE
};
typedef enum nanomqo_status_t nanomqo_status_t;

enum nanomqo_shader_type_t {
    NANOMQO_SHADER_TYPE_CLASSIC,
    NANOMQO_SHADER_TYPE_CONSTANT,
    NANOMQO_SHADER_TYPE_LAMBERT,
    NANOMQO_SHADER_TYPE_PHONG,
    NANOMQO_SHADER_TYPE_BLINN,
    NANOMQO_SHADER_TYPE_MAX_ENUM
};
typedef enum nanomqo_shader_type_t nanomqo_shader_type_t;

enum nanomqo_projection_type_t {
    NANOMQO_PROJECTION_TYPE_UV,
    NANOMQO_PROJECTION_TYPE_PLANE,
    NANOMQO_PROJECTION_TYPE_CYLINDER,
    NANOMQO_PROJECTION_TYPE_SPHERE,
    NANOMQO_PROJECTION_TYPE_MAX_ENUM
};
typedef enum nanomqo_projection_type_t nanomqo_projection_type_t;

enum nanomqo_patch_type_t {
    NANOMQO_PATCH_TYPE_PLANE,
    NANOMQO_PATCH_TYPE_SPLINE_TYPE1,
    NANOMQO_PATCH_TYPE_SPLINE_TYPE2,
    NANOMQO_PATCH_TYPE_CATMULL_CLARK,
    NANOMQO_PATCH_TYPE_OPENSUBDIV,
    NANOMQO_PATCH_TYPE_MAX_ENUM
};
typedef enum nanomqo_patch_type_t nanomqo_patch_type_t;

enum nanomqo_shading_type_t {
    NANOMQO_SHADING_TYPE_FLAT,
    NANOMQO_SHADING_TYPE_GOURAUT,
    NANOMQO_SHADING_TYPE_MAX_ENUM
};
typedef enum nanomqo_shading_type_t nanomqo_shading_type_t;

enum nanomqo_mirror_type_t {
    NANOMQO_MIRROR_TYPE_NONE,
    NANOMQO_MIRROR_TYPE_SEPARATED,
    NANOMQO_MIRROR_TYPE_CONNECTED,
    NANOMQO_MIRROR_TYPE_MAX_ENUM
};
typedef enum nanomqo_mirror_type_t nanomqo_mirror_type_t;

enum nanomqo_lathe_type_t {
    NANOMQO_LATHE_TYPE_NONE,
    NANOMQO_LATHE_TYPE_TWOSIDES = 3
};
typedef enum nanomqo_lathe_type_t nanomqo_lathe_type_t;

enum nanomqo_axis_type_t {
    NANOMQO_AXIS_TYPE_X,
    NANOMQO_AXIS_TYPE_Y,
    NANOMQO_AXIS_TYPE_Z
};
typedef enum nanomqo_axis_type_t nanomqo_axis_type_t;

struct nanomqo_global_allocator_t {
    void *opaque;
    void *(*malloc)(void *, nanomqo_rsize_t, const char *file, int line);
    void *(*calloc)(void *, nanomqo_rsize_t, nanomqo_rsize_t, const char *file, int line);
    void *(*realloc)(void *, void *, nanomqo_rsize_t, const char *file, int line);
    void (*free)(void *, void *, const char *file, int line);
};

NANOMQO_DECL_API void APIENTRY
nanomqoGlobalSetCustomAllocator(const nanomqo_global_allocator_t *allocator);

NANOMQO_DECL_API nanomqo_buffer_t *APIENTRY
nanomqoBufferCreate(const nanomqo_uint8_t *data, nanomqo_rsize_t length);
NANOMQO_DECL_API nanomqo_rsize_t APIENTRY
nanomqoBufferGetLength(const nanomqo_buffer_t *buffer);
NANOMQO_DECL_API nanomqo_rsize_t APIENTRY
nanomqoBufferGetOffset(const nanomqo_buffer_t *buffer);
NANOMQO_DECL_API const nanomqo_uint8_t *APIENTRY
nanomqoBufferGetDataPtr(const nanomqo_buffer_t *buffer);
NANOMQO_DECL_API int APIENTRY
nanomqoBufferGetRowOffset(const nanomqo_buffer_t *buffer);
NANOMQO_DECL_API int APIENTRY
nanomqoBufferGetColumnOffset(const nanomqo_buffer_t *buffer);
NANOMQO_DECL_API nanomqo_bool_t APIENTRY
nanomqoBufferCanReadLength(const nanomqo_buffer_t *buffer, nanomqo_rsize_t size);
NANOMQO_DECL_API void APIENTRY
nanomqoBufferSkip(nanomqo_buffer_t *buffer, nanomqo_rsize_t skip, nanomqo_bool_t *ok);
NANOMQO_DECL_API void APIENTRY
nanomqoBufferDestroy(nanomqo_buffer_t *buffer);

NANOMQO_DECL_API const nanomqo_float32_t *APIENTRY
nanomqoVertexGetOrigin(const nanomqo_vertex_t *vertex);
NANOMQO_DECL_API nanomqo_float32_t APIENTRY
nanomqoVertexGetWeight(const nanomqo_vertex_t *vertex);
NANOMQO_DECL_API nanomqo_uint32_t APIENTRY
nanomqoVertexGetColor(const nanomqo_vertex_t *vertex);

NANOMQO_DECL_API const int *APIENTRY
nanomqoFaceGetVertexIndices(const nanomqo_face_t *face, int *length);
NANOMQO_DECL_API int APIENTRY
nanomqoFaceGetMaterialIndex(const nanomqo_face_t *face);
NANOMQO_DECL_API const nanomqo_float32_t *APIENTRY
nanomqoFaceGetUVs(const nanomqo_face_t *face);
NANOMQO_DECL_API const nanomqo_float32_t *APIENTRY
nanomqoFaceGetCreases(const nanomqo_face_t *face);
NANOMQO_DECL_API const nanomqo_uint32_t *APIENTRY
nanomqoFaceGetColors(const nanomqo_face_t *face);
NANOMQO_DECL_API int APIENTRY
nanomqoFaceGetUID(const nanomqo_face_t *face);

NANOMQO_DECL_API const char *APIENTRY
nanomqoMaterialGetName(const nanomqo_material_t *material);
NANOMQO_DECL_API const char *APIENTRY
nanomqoMaterialGetAlphaPlanePath(const nanomqo_material_t *material);
NANOMQO_DECL_API const char *APIENTRY
nanomqoMaterialGetBumpMapPath(const nanomqo_material_t *material);
NANOMQO_DECL_API const char *APIENTRY
nanomqoMaterialGetTexturePath(const nanomqo_material_t *material);
NANOMQO_DECL_API const nanomqo_float32_t *APIENTRY
nanomqoMaterialGetProjectionPosition(const nanomqo_material_t *material);
NANOMQO_DECL_API const nanomqo_float32_t *APIENTRY
nanomqoMaterialGetProjectionScale(const nanomqo_material_t *material);
NANOMQO_DECL_API const nanomqo_float32_t *APIENTRY
nanomqoMaterialGetProjectionAngle(const nanomqo_material_t *material);
NANOMQO_DECL_API const nanomqo_float32_t *APIENTRY
nanomqoMaterialGetColor(const nanomqo_material_t *material);
NANOMQO_DECL_API const nanomqo_float32_t *APIENTRY
nanomqoMaterialGetAmbientColor(const nanomqo_material_t *material);
NANOMQO_DECL_API nanomqo_float32_t APIENTRY
nanomqoMaterialGetAmbient(const nanomqo_material_t *material);
NANOMQO_DECL_API nanomqo_float32_t APIENTRY
nanomqoMaterialGetDiffuse(const nanomqo_material_t *material);
NANOMQO_DECL_API nanomqo_float32_t APIENTRY
nanomqoMaterialGetEmissive(const nanomqo_material_t *material);
NANOMQO_DECL_API nanomqo_float32_t APIENTRY
nanomqoMaterialGetSpecular(const nanomqo_material_t *material);
NANOMQO_DECL_API nanomqo_float32_t APIENTRY
nanomqoMaterialGetPower(const nanomqo_material_t *material);
NANOMQO_DECL_API nanomqo_float32_t APIENTRY
nanomqoMaterialGetReflect(const nanomqo_material_t *material);
NANOMQO_DECL_API nanomqo_float32_t APIENTRY
nanomqoMaterialGetRefract(const nanomqo_material_t *material);
NANOMQO_DECL_API nanomqo_shader_type_t APIENTRY
nanomqoMaterialGetShaderType(const nanomqo_material_t *material);
NANOMQO_DECL_API nanomqo_projection_type_t APIENTRY
nanomqoMaterialGetProjectionType(const nanomqo_material_t *material);
NANOMQO_DECL_API nanomqo_bool_t APIENTRY
nanomqoMaterialHasVertexColor(const nanomqo_material_t *material);
NANOMQO_DECL_API nanomqo_bool_t APIENTRY
nanomqoMaterialIsCullingDisabled(const nanomqo_material_t *material);

NANOMQO_DECL_API const char *APIENTRY
nanomqoObjectGetName(const nanomqo_object_t *object);
NANOMQO_DECL_API const nanomqo_float32_t *APIENTRY
nanomqoObjectGetScale(const nanomqo_object_t *object);
NANOMQO_DECL_API const nanomqo_float32_t *APIENTRY
nanomqoObjectGetTranslation(const nanomqo_object_t *object);
NANOMQO_DECL_API const nanomqo_float32_t *APIENTRY
nanomqoObjectGetOrientation(const nanomqo_object_t *object);
NANOMQO_DECL_API const nanomqo_float32_t *APIENTRY
nanomqoObjectGetColor(const nanomqo_object_t *object);
NANOMQO_DECL_API int APIENTRY
nanomqoObjectGetUID(const nanomqo_object_t *object);
NANOMQO_DECL_API int APIENTRY
nanomqoObjectGetDepth(const nanomqo_object_t *object);
NANOMQO_DECL_API nanomqo_patch_type_t APIENTRY
nanomqoObjectGetPatchType(const nanomqo_object_t *object);
NANOMQO_DECL_API int APIENTRY
nanomqoObjectGetPatchTriangulationType(const nanomqo_object_t *object);
NANOMQO_DECL_API int APIENTRY
nanomqoObjectGetNumSegments(const nanomqo_object_t *object);
NANOMQO_DECL_API nanomqo_shading_type_t APIENTRY
nanomqoObjectGetShadingType(const nanomqo_object_t *object);
NANOMQO_DECL_API nanomqo_float32_t APIENTRY
nanomqoObjectGetFacet(const nanomqo_object_t *object);
NANOMQO_DECL_API int APIENTRY
nanomqoObjectGetColorType(const nanomqo_object_t *object);
NANOMQO_DECL_API nanomqo_mirror_type_t APIENTRY
nanomqoObjectGetMirrorType(const nanomqo_object_t *object);
NANOMQO_DECL_API int APIENTRY
nanomqoObjectGetMirrorAxis(const nanomqo_object_t *object);
NANOMQO_DECL_API nanomqo_float32_t APIENTRY
nanomqoObjectGetMirrorDistance(const nanomqo_object_t *object);
NANOMQO_DECL_API nanomqo_lathe_type_t APIENTRY
nanomqoObjectGetLatheType(const nanomqo_object_t *object);
NANOMQO_DECL_API nanomqo_axis_type_t APIENTRY
nanomqoObjectGetLatheAxis(const nanomqo_object_t *object);
NANOMQO_DECL_API int APIENTRY
nanomqoObjectGetNumLatheSegments(const nanomqo_object_t *object);
NANOMQO_DECL_API nanomqo_bool_t APIENTRY
nanomqoObjectIsVisible(const nanomqo_object_t *object);
NANOMQO_DECL_API nanomqo_bool_t APIENTRY
nanomqoObjectIsLocked(const nanomqo_object_t *object);
NANOMQO_DECL_API nanomqo_bool_t APIENTRY
nanomqoObjectIsFolding(const nanomqo_object_t *object);
NANOMQO_DECL_API nanomqo_vertex_t *const *APIENTRY
nanomqoObjectGetAllVertices(const nanomqo_object_t *object, nanomqo_rsize_t *nvertices);
NANOMQO_DECL_API const int *APIENTRY
nanomqoObjectGetAllVertexAttributeUID(const nanomqo_object_t *object, nanomqo_rsize_t *nuids);
NANOMQO_DECL_API nanomqo_face_t *const *APIENTRY
nanomqoObjectGetAllFaces(const nanomqo_object_t *object, nanomqo_rsize_t *nfaces);

NANOMQO_DECL_API const nanomqo_float32_t *APIENTRY
nanomqoSceneGetAmbient(const nanomqo_scene_t *scene);
NANOMQO_DECL_API const nanomqo_float32_t *APIENTRY
nanomqoSceneGetPosition(const nanomqo_scene_t *scene);
NANOMQO_DECL_API const nanomqo_float32_t *APIENTRY
nanomqoSceneGetLookAt(const nanomqo_scene_t *scene);
NANOMQO_DECL_API const nanomqo_float32_t *APIENTRY
nanomqoSceneGetAngle(const nanomqo_scene_t *scene);
NANOMQO_DECL_API nanomqo_float32_t APIENTRY
nanomqoSceneGetZoom(const nanomqo_scene_t *scene);
NANOMQO_DECL_API nanomqo_float32_t APIENTRY
nanomqoSceneGetZoom2(const nanomqo_scene_t *scene);
NANOMQO_DECL_API nanomqo_float32_t APIENTRY
nanomqoSceneGetPerspective(const nanomqo_scene_t *scene);
NANOMQO_DECL_API nanomqo_float32_t APIENTRY
nanomqoSceneGetFrontClip(const nanomqo_scene_t *scene);
NANOMQO_DECL_API nanomqo_float32_t APIENTRY
nanomqoSceneGetBackClip(const nanomqo_scene_t *scene);
NANOMQO_DECL_API nanomqo_bool_t APIENTRY
nanomqoSceneIsOrtho(const nanomqo_scene_t *scene);

NANOMQO_DECL_API nanomqo_document_t *APIENTRY
nanomqoDocumentCreate(void);
NANOMQO_DECL_API nanomqo_status_t APIENTRY
nanomqoDocumentParse(nanomqo_document_t *document, nanomqo_buffer_t *buffer);
NANOMQO_DECL_API nanomqo_float32_t APIENTRY
nanomqoDocumentGetVersion(const nanomqo_document_t *document);
NANOMQO_DECL_API const nanomqo_scene_t *APIENTRY
nanomqoDocumentGetScene(const nanomqo_document_t *document);
NANOMQO_DECL_API nanomqo_material_t *const *APIENTRY
nanomqoDocumentGetAllMaterials(const nanomqo_document_t *document, nanomqo_rsize_t *nmaterials);
NANOMQO_DECL_API nanomqo_object_t *const *APIENTRY
nanomqoDocumentGetAllObjects(const nanomqo_document_t *document, nanomqo_rsize_t *nobjects);
NANOMQO_DECL_API void APIENTRY
nanomqoDocumentDestroy(nanomqo_document_t *document);

#endif /* NANOMQO_H_ */
