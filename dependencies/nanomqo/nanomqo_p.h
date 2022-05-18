/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#ifndef NANOMQO_PRIVATE_H_
#define NANOMQO_PRIVATE_H_

/* THIS IS A PRIVATE DECLARATIONS OF NANOMQO_C DO NOT INCLUDE DIRECTLY */

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#endif /* _WIN32 */

#include "./nanomqo.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NANOMQO_BUFFER_CAPACITY 256
#define nanomqo_string_equals(s1, s2) (nanomqo_is_not_null((s1)) && nanomqo_is_not_null((s2)) && strcmp((s1), (s2)) == 0)
#define nanomqo_string_equals_n(s1, s2, n) (nanomqo_is_not_null((s1)) && nanomqo_is_not_null((s2)) && strncmp((s1), (s2), (n)) == 0)
#define nanomqo_string_equals_const(s, c) (nanomqo_is_not_null((s)) && strncmp((s), (c), (sizeof((c)) - 1)) == 0)
#define nanomqo_strtof(a, b) ((float) strtod(a, b))
#define nanomqo_eos '\0'

static const nanomqo_global_allocator_t *
    __nanomqo_global_allocator = NULL;

NANOMQO_DECL_INLINE static void *
nanomqoMalloc(nanomqo_rsize_t size, const char *file, int line)
{
    void *p = NULL;
    if (nanomqo_likely(size <= NANOMQO_RSIZE_MAX)) {
        if (__nanomqo_global_allocator != NULL) {
            p = __nanomqo_global_allocator->malloc(__nanomqo_global_allocator->opaque, size, file, line);
        }
        else {
            p = malloc(size);
        }
    }
    return p;
}

NANOMQO_DECL_INLINE static void *
nanomqoRealloc(void *ptr, nanomqo_rsize_t size, const char *file, int line)
{
    void *p = NULL;
    if (nanomqo_likely(size <= NANOMQO_RSIZE_MAX)) {
        if (__nanomqo_global_allocator != NULL) {
            p = __nanomqo_global_allocator->realloc(__nanomqo_global_allocator->opaque, ptr, size, file, line);
        }
        else {
            p = realloc(ptr, size);
        }
    }
    return p;
}

NANOMQO_DECL_INLINE static void *
nanomqoCalloc(nanomqo_rsize_t length, nanomqo_rsize_t size, const char *file, int line)
{
    void *p = NULL;
    if (nanomqo_likely(length <= (NANOMQO_RSIZE_MAX / size))) {
        if (__nanomqo_global_allocator != NULL) {
            p = __nanomqo_global_allocator->calloc(__nanomqo_global_allocator->opaque, length, size, file, line);
        }
        else {
            p = calloc(length, size);
        }
    }
    return p;
}

NANOMQO_DECL_INLINE static void
nanomqoFree(void *ptr, const char *file, int line)
{
    if (__nanomqo_global_allocator != NULL) {
        __nanomqo_global_allocator->free(__nanomqo_global_allocator->opaque, ptr, file, line);
    }
    else {
        free(ptr);
    }
}

/* allocation macros */
#ifdef NANOMQO_ENABLE_DEBUG_ALLOCATOR
#define nanomqo_free(ptr) nanomqoFree((ptr), __FILE__, __LINE__)
#define nanomqo_malloc(size) nanomqoMalloc((size), __FILE__, __LINE__)
#define nanomqo_realloc(ptr, size) nanomqoRealloc((ptr), (size), __FILE__, __LINE__)
#define nanomqo_calloc(length, size) nanomqoCalloc((length), (size), __FILE__, __LINE__);
#else
#define nanomqo_free(ptr) nanomqoFree((ptr), NULL, 0)
#define nanomqo_malloc(size) nanomqoMalloc((size), NULL, 0)
#define nanomqo_realloc(ptr, size) nanomqoRealloc((ptr), (size), NULL, 0)
#define nanomqo_calloc(length, size) nanomqoCalloc((length), (size), NULL, 0);
#endif

#define NANOMQO_CHECK_CLOSE_BRACE(ptr, err) \
    for (;;) {                              \
        if ((*ptr) == ')') {                \
            (ptr)++;                        \
            nanomqoUtilSkipSpaces(&(ptr));  \
            break;                          \
        }                                   \
        else {                              \
            return (err);                   \
        }                                   \
    }

#define NANOMQO_FACE_CHECK_CLOSE_BLACE(ptr) NANOMQO_CHECK_CLOSE_BRACE((ptr), NANOMQO_STATUS_ERROR_INVALID_FACE_CHUNK)
#define NANOMQO_MATERIAL_CHECK_CLOSE_BLACE(ptr) NANOMQO_CHECK_CLOSE_BRACE((ptr), NANOMQO_STATUS_ERROR_INVALID_MATERIAL_CHUNK)

struct nanomqo_buffer_t {
    const nanomqo_uint8_t *data;
    nanomqo_rsize_t length;
    nanomqo_rsize_t offset;
    int rows;
    int columns;
    int depth;
};
struct nanomqo_document_t {
    nanomqo_float32_t version;
    nanomqo_scene_t *scene;
    int num_materials;
    nanomqo_material_t **materials;
    int num_objects;
    nanomqo_object_t **objects;
    int num_allocated_objects;
};
struct nanomqo_scene_t {
    nanomqo_float32_t ambient[4];
    nanomqo_float32_t position[4];
    nanomqo_float32_t lookat[4];
    nanomqo_float32_t angle[4];
    nanomqo_float32_t zoom;
    nanomqo_float32_t zoom2;
    nanomqo_float32_t perspective;
    nanomqo_float32_t frontclip;
    nanomqo_float32_t backclip;
    int is_ortho;
};
struct nanomqo_material_t {
    char *name;
    nanomqo_shader_type_t shader_type;
    int uid;
    int has_vertex_color;
    int disable_culling;
    nanomqo_float32_t color[4];
    nanomqo_float32_t ambient_color[4];
    nanomqo_float32_t ambient;
    nanomqo_float32_t diffusion;
    nanomqo_float32_t emissive;
    nanomqo_float32_t specular;
    nanomqo_float32_t power;
    nanomqo_float32_t reflect;
    nanomqo_float32_t refract;
    char *texture_path;
    char *alpha_plane_path;
    char *bump_map_path;
    nanomqo_projection_type_t projection_type;
    nanomqo_float32_t projection_position[4];
    nanomqo_float32_t projection_scale[4];
    nanomqo_float32_t projection_angle[4];
};
struct nanomqo_object_t {
    char *name;
    int uid;
    int depth;
    int is_folding;
    nanomqo_float32_t scale[4];
    nanomqo_float32_t orientation[4];
    nanomqo_float32_t translation[4];
    nanomqo_patch_type_t patch_type;
    int patch_triangulation_type;
    int num_segments;
    int is_visible;
    int is_locked;
    nanomqo_shading_type_t shading_type;
    nanomqo_float32_t facet;
    nanomqo_float32_t color[4];
    int color_type;
    nanomqo_mirror_type_t mirror_type;
    int mirror_axis;
    nanomqo_float32_t mirror_distance;
    nanomqo_lathe_type_t lathe_type;
    nanomqo_axis_type_t lathe_axis;
    int num_lathe_segments;
    int num_vertices;
    nanomqo_vertex_t **vertices;
    int num_vertex_attr_uids;
    int *vertex_attr_uids;
    int num_faces;
    nanomqo_face_t **faces;
};
struct nanomqo_vertex_t {
    int uid;
    nanomqo_float32_t weight;
    nanomqo_float32_t origin[4];
    nanomqo_uint32_t color;
};
struct nanomqo_face_t {
    int material_index;
    int num_vertex_indices;
    int *vertex_indices;
    int uid;
    nanomqo_uint32_t *color_values;
    nanomqo_float32_t *uv_values;
    nanomqo_float32_t *crease_values;
};

NANOMQO_DECL_INLINE static void
nanomqoUtilSkipSpaces(char **ptr)
{
    char *p = *ptr;
    while (isspace(*p)) {
        p++;
    }
    *ptr = p;
}

NANOMQO_DECL_INLINE static nanomqo_bool_t
nanomqoUtilExtractToken(char **ptr, const char *const str)
{
    if (nanomqo_string_equals_n(*ptr, str, strlen(str))) {
        *ptr += strlen(str);
        nanomqoUtilSkipSpaces(ptr);
        return nanomqo_true;
    }
    return nanomqo_false;
}

static void
nanomqoUtilExtractName(char **name, char **ptr)
{
    int length;
    char *p, *s;
    nanomqoUtilSkipSpaces(ptr);
    p = *ptr;
    if (*p == '"') {
        p++;
    }
    s = p;
    while (*p != '"') {
        p++;
    }
    *p++ = nanomqo_eos;
    *ptr = p;
    nanomqoUtilSkipSpaces(ptr);
    if (nanomqo_is_not_null(s)) {
        length = strlen(s);
        *name = (char *) nanomqo_malloc(length + 1);
        if (nanomqo_is_not_null(*name)) {
            memcpy(*name, s, length);
            *(*name + length) = nanomqo_eos;
        }
    }
    else {
        *(*name) = nanomqo_eos;
    }
}

NANOMQO_DECL_INLINE static nanomqo_uint8_t
nanomqoBufferGetChar(const nanomqo_buffer_t *buffer)
{
    return nanomqo_likely(buffer->offset < buffer->length) ? *(buffer->data + buffer->offset) : nanomqo_eos;
}

NANOMQO_DECL_INLINE static void
nanomqoBufferAdvanceChar(nanomqo_buffer_t *buffer)
{
    buffer->offset++;
    buffer->columns++;
}

NANOMQO_DECL_INLINE static nanomqo_uint8_t
nanomqoBufferReadNextChar(nanomqo_buffer_t *buffer)
{
    nanomqoBufferAdvanceChar(buffer);
    return nanomqoBufferGetChar(buffer);
}

NANOMQO_DECL_INLINE static nanomqo_bool_t
nanomqoBufferTestString(const nanomqo_buffer_t *buffer, const char *const text)
{
    const char *ptr = (const char *) nanomqoBufferGetDataPtr(buffer);
    return nanomqoBufferCanReadLength(buffer, strlen(text)) &&
        *ptr == *text &&
        nanomqo_string_equals_n(ptr, text, strlen(text));
}

NANOMQO_DECL_INLINE static void
nanomqoBufferSkipSpaces(nanomqo_buffer_t *buffer)
{
    nanomqo_uint8_t c = nanomqoBufferGetChar(buffer);
    while (c != nanomqo_eos && isspace(c)) {
        c = nanomqoBufferReadNextChar(buffer);
    }
}

static void
nanomqoBufferSkipLine(nanomqo_buffer_t *buffer)
{
    nanomqo_uint8_t c1 = nanomqoBufferGetChar(buffer), c2;
    while (c1 != nanomqo_eos) {
        nanomqoBufferAdvanceChar(buffer);
        if (c1 == '\n') {
            buffer->rows++;
            buffer->columns = 0;
            break;
        }
        c2 = nanomqoBufferGetChar(buffer);
        if (c1 == '\r' && c2 == '\n') {
            nanomqoBufferAdvanceChar(buffer);
            buffer->rows++;
            buffer->columns = 0;
            break;
        }
        c1 = c2;
    }
}

static void
nanomqoBufferSkipUntilEndChunk(nanomqo_buffer_t *buffer)
{
    nanomqo_uint8_t c1 = nanomqoBufferGetChar(buffer), c2;
    int depth = 0;
    while (c1 != nanomqo_eos && depth >= 0) {
        nanomqoBufferAdvanceChar(buffer);
        if (c1 == '{') {
            buffer->depth++;
            depth++;
        }
        else if (c1 == '}') {
            buffer->depth--;
            depth--;
        }
        c2 = nanomqoBufferGetChar(buffer);
        if (c1 == '\r' && c2 == '\n') {
            buffer->rows++;
            buffer->columns = 0;
        }
        c1 = c2;
    }
    if (buffer->depth < 0) {
        buffer->depth = 0;
    }
    nanomqoBufferSkipLine(buffer);
}

static void
nanomqoBufferGetLine(nanomqo_buffer_t *buffer, char *input, nanomqo_rsize_t length)
{
    char *ptr;
    nanomqo_rsize_t len = length - 1;
    if (nanomqoBufferCanReadLength(buffer, len)) {
        memcpy(input, nanomqoBufferGetDataPtr(buffer), len);
    }
    else if (buffer->length > buffer->offset) {
        len = buffer->length - buffer->offset;
        memcpy(input, nanomqoBufferGetDataPtr(buffer), len);
    }
    else {
        *input = nanomqo_eos;
    }
    if (nanomqo_is_not_null(ptr = (char *) memchr(input, '\r', len))) {
        *ptr = nanomqo_eos;
    }
    if (nanomqo_is_not_null(ptr = (char *) memchr(input, '\n', len))) {
        *ptr = nanomqo_eos;
    }
    input[len] = nanomqo_eos;
    nanomqoBufferSkipLine(buffer);
}

NANOMQO_DECL_INLINE static void
nanomqoBufferEndChunk(nanomqo_buffer_t *buffer)
{
    char input[NANOMQO_BUFFER_CAPACITY];
    nanomqoBufferSkipSpaces(buffer);
    nanomqoBufferGetLine(buffer, input, sizeof(input));
    buffer->depth -= *input == '}';
}

NANOMQO_DECL_INLINE static void
nanomqoBufferExtractInt(nanomqo_buffer_t *buffer, char *ptr, int *value)
{
    nanomqoUtilSkipSpaces(&ptr);
    *value = strtol(ptr, &ptr, 10);
    nanomqoUtilSkipSpaces(&ptr);
    buffer->depth += *ptr == '{';
}

static void
nanomqoVertexParseWeightChunk(nanomqo_buffer_t *buffer, nanomqo_vertex_t **vertices, int nvertices)
{
    char input[NANOMQO_BUFFER_CAPACITY], *ptr = input;
    int vertex_index;
    nanomqo_loop
    {
        ptr = input;
        nanomqoBufferSkipSpaces(buffer);
        nanomqoBufferGetLine(buffer, input, sizeof(input));
        if (*input == nanomqo_eos || *input == '}') {
            buffer->depth--;
            break;
        }
        else {
            vertex_index = strtol(ptr, &ptr, 10);
            if (vertex_index >= 0 && vertex_index < nvertices) {
                nanomqoUtilSkipSpaces(&ptr);
                vertices[vertex_index]->weight = nanomqo_strtof(ptr, &ptr);
            }
        }
    }
}

static void
nanomqoVertexParseColorChunk(nanomqo_buffer_t *buffer, nanomqo_vertex_t **vertices, int nvertices)
{
    char input[NANOMQO_BUFFER_CAPACITY], *ptr = input;
    int vertex_index;
    nanomqo_loop
    {
        ptr = input;
        nanomqoBufferSkipSpaces(buffer);
        nanomqoBufferGetLine(buffer, input, sizeof(input));
        if (*input == nanomqo_eos || *input == '}') {
            buffer->depth--;
            break;
        }
        else {
            vertex_index = strtol(ptr, &ptr, 10);
            if (vertex_index >= 0 && vertex_index < nvertices) {
                nanomqoUtilSkipSpaces(&ptr);
                vertices[vertex_index]->color = strtoul(ptr, &ptr, 10);
            }
        }
    }
}

nanomqo_vertex_t *
nanomqoVertexCreate(void)
{
    nanomqo_vertex_t *vertex;
    vertex = (nanomqo_vertex_t *) nanomqo_calloc(1, sizeof(*vertex));
    return vertex;
}

void
nanomqoVertexDestroy(nanomqo_vertex_t *vertex)
{
    if (nanomqo_is_not_null(vertex)) {
        nanomqo_free(vertex);
    }
}

nanomqo_face_t *
nanomqoFaceCreate(void)
{
    nanomqo_face_t *face;
    face = (nanomqo_face_t *) nanomqo_calloc(1, sizeof(*face));
    return face;
}

nanomqo_status_t
nanomqoFaceParse(nanomqo_face_t *face, nanomqo_buffer_t *buffer)
{
    nanomqo_bool_t is_parsing = nanomqo_true;
    char input[NANOMQO_BUFFER_CAPACITY], *ptr = input;
    int nvertices, i;
    if (nanomqo_is_null(buffer)) {
        return NANOMQO_STATUS_ERROR_NULL_BUFFER;
    }
    if (nanomqo_is_null(face)) {
        return NANOMQO_STATUS_ERROR_NULL_FACE;
    }
    nanomqoBufferGetLine(buffer, input, sizeof(input));
    nvertices = strtol(ptr, &ptr, 10);
    nanomqoUtilSkipSpaces(&ptr);
    while (is_parsing) {
        switch (*ptr) {
        case 'C':
            if (nanomqoUtilExtractToken(&ptr, "COL(")) {
                face->color_values = (nanomqo_uint32_t *) nanomqo_calloc(nvertices, sizeof(*face->color_values));
                for (i = 0; i < nvertices; i++) {
                    face->color_values[i] = strtoul(ptr, &ptr, 10);
                    nanomqoUtilSkipSpaces(&ptr);
                }
                NANOMQO_FACE_CHECK_CLOSE_BLACE(ptr);
            }
            else if (nanomqoUtilExtractToken(&ptr, "CRS(")) {
                face->crease_values = (nanomqo_float32_t *) nanomqo_calloc(nvertices, sizeof(*face->crease_values));
                for (i = 0; i < nvertices; i++) {
                    face->crease_values[i] = nanomqo_strtof(ptr, &ptr);
                    nanomqoUtilSkipSpaces(&ptr);
                }
                NANOMQO_FACE_CHECK_CLOSE_BLACE(ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_FACE_CHUNK;
            }
            break;
        case 'M':
            if (nanomqoUtilExtractToken(&ptr, "M(")) {
                face->material_index = strtol(ptr, &ptr, 10);
                NANOMQO_FACE_CHECK_CLOSE_BLACE(ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_FACE_CHUNK;
            }
            break;
        case 'U':
            if (nanomqoUtilExtractToken(&ptr, "UID(")) {
                face->uid = strtol(ptr, &ptr, 10);
                NANOMQO_FACE_CHECK_CLOSE_BLACE(ptr);
            }
            else if (nanomqoUtilExtractToken(&ptr, "UV(")) {
                face->uv_values = (nanomqo_float32_t *) nanomqo_calloc(nvertices * 2, sizeof(*face->uv_values));
                for (i = 0; i < nvertices * 2; i++) {
                    face->uv_values[i] = nanomqo_strtof(ptr, &ptr);
                    nanomqoUtilSkipSpaces(&ptr);
                }
                NANOMQO_FACE_CHECK_CLOSE_BLACE(ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_FACE_CHUNK;
            }
            break;
        case 'V':
            if (nanomqoUtilExtractToken(&ptr, "V(")) {
                face->num_vertex_indices = nvertices;
                face->vertex_indices = (int *) nanomqo_calloc(nvertices, sizeof(*face->vertex_indices));
                for (i = 0; i < nvertices; i++) {
                    face->vertex_indices[i] = strtol(ptr, &ptr, 10);
                    nanomqoUtilSkipSpaces(&ptr);
                }
                NANOMQO_FACE_CHECK_CLOSE_BLACE(ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_FACE_CHUNK;
            }
            break;
        default:
            is_parsing = nanomqo_false;
            break;
        }
    }
    return NANOMQO_STATUS_SUCCESS;
}

void
nanomqoFaceDestroy(nanomqo_face_t *face)
{
    if (nanomqo_is_not_null(face)) {
        if (nanomqo_is_not_null(face->vertex_indices)) {
            nanomqo_free(face->vertex_indices);
        }
        if (nanomqo_is_not_null(face->uv_values)) {
            nanomqo_free(face->uv_values);
        }
        if (nanomqo_is_not_null(face->crease_values)) {
            nanomqo_free(face->crease_values);
        }
        if (nanomqo_is_not_null(face->color_values)) {
            nanomqo_free(face->color_values);
        }
        nanomqo_free(face);
    }
}

nanomqo_scene_t *
nanomqoSceneCreate(void)
{
    nanomqo_scene_t *scene;
    scene = (nanomqo_scene_t *) nanomqo_calloc(1, sizeof(*scene));
    return scene;
}

nanomqo_status_t
nanomqoSceneParse(nanomqo_scene_t *scene, nanomqo_buffer_t *buffer)
{
    nanomqo_bool_t is_parsing = nanomqo_true;
    nanomqo_uint8_t c;
    char input[NANOMQO_BUFFER_CAPACITY], *ptr = input;
    if (nanomqo_is_null(buffer)) {
        return NANOMQO_STATUS_ERROR_NULL_BUFFER;
    }
    if (nanomqo_is_null(scene)) {
        return NANOMQO_STATUS_ERROR_NULL_SCENE;
    }
    if (!nanomqoBufferTestString(buffer, "Scene {")) {
        return NANOMQO_STATUS_ERROR_UNKNOWN_CHUNK_TYPE;
    }
    buffer->depth++;
    nanomqoBufferGetLine(buffer, input, sizeof(input));
    while (is_parsing) {
        nanomqoBufferSkipSpaces(buffer);
        ptr = input;
        c = nanomqoBufferGetChar(buffer);
        switch (c) {
        case 'a':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "amb")) {
                scene->ambient[0] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                scene->ambient[1] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                scene->ambient[2] = nanomqo_strtof(ptr, &ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_SCENE_CHUNK;
            }
            break;
        case 'b':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "backclip")) {
                scene->backclip = nanomqo_strtof(ptr, &ptr);
            }
            if (nanomqoUtilExtractToken(&ptr, "bank")) {
                scene->angle[2] = nanomqo_strtof(ptr, &ptr);
            }
            break;
        case 'd':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "dirlight")) {
                /* XXX: directional light */
                nanomqoBufferSkipUntilEndChunk(buffer);
            }
            break;
        case 'f':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "frontclip")) {
                scene->frontclip = nanomqo_strtof(ptr, &ptr);
            }
            break;
        case 'p':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "pers")) {
                scene->perspective = nanomqo_strtof(ptr, &ptr);
            }
            else if (nanomqoUtilExtractToken(&ptr, "pich")) {
                scene->angle[1] = nanomqo_strtof(ptr, &ptr);
            }
            else if (nanomqoUtilExtractToken(&ptr, "pos")) {
                scene->position[0] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                scene->position[1] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                scene->position[2] = nanomqo_strtof(ptr, &ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_SCENE_CHUNK;
            }
            break;
        case 'l':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "lookat")) {
                scene->lookat[0] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                scene->lookat[1] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                scene->lookat[2] = nanomqo_strtof(ptr, &ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_SCENE_CHUNK;
            }
            break;
        case 'h':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "head")) {
                scene->angle[0] = nanomqo_strtof(ptr, &ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_SCENE_CHUNK;
            }
            break;
        case 'o':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "ortho")) {
                scene->is_ortho = strtol(ptr, &ptr, 10);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_SCENE_CHUNK;
            }
            break;
        case 'z':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "zoom2")) {
                scene->zoom2 = nanomqo_strtof(ptr, &ptr);
            }
            else if (nanomqoUtilExtractToken(&ptr, "zoom")) {
                scene->zoom = nanomqo_strtof(ptr, &ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_SCENE_CHUNK;
            }
            break;
        case '}':
            nanomqoBufferSkipLine(buffer);
            buffer->depth--;
            is_parsing = nanomqo_false;
            break;
        default:
            nanomqoBufferSkipLine(buffer);
            break;
        }
    }
    return NANOMQO_STATUS_SUCCESS;
}

void
nanomqoSceneDestroy(nanomqo_scene_t *scene)
{
    if (nanomqo_is_not_null(scene)) {
        nanomqo_free(scene);
    }
}

nanomqo_material_t *
nanomqoMaterialCreate(void)
{
    nanomqo_material_t *material;
    material = (nanomqo_material_t *) nanomqo_calloc(1, sizeof(*material));
    return material;
}

nanomqo_status_t
nanomqoMaterialParse(nanomqo_material_t *material, nanomqo_buffer_t *buffer)
{
    nanomqo_bool_t is_parsing = nanomqo_true;
    char input[NANOMQO_BUFFER_CAPACITY], *ptr = input, *p;
    union nanomqo_int_to_projection_type_cast_t {
        int v;
        nanomqo_projection_type_t e;
    } u1;
    union nanomqo_int_to_shader_type_cast_t {
        int v;
        nanomqo_shader_type_t e;
    } u2;
    if (nanomqo_is_null(buffer)) {
        return NANOMQO_STATUS_ERROR_NULL_BUFFER;
    }
    if (nanomqo_is_null(material)) {
        return NANOMQO_STATUS_ERROR_NULL_MATERIAL;
    }
    nanomqoBufferSkipSpaces(buffer);
    nanomqoBufferGetLine(buffer, input, sizeof(input));
    ptr = input;
    nanomqoUtilExtractName(&material->name, &ptr);
    while (is_parsing) {
        switch (*ptr) {
        case 'a':
            if (nanomqoUtilExtractToken(&ptr, "amb_col(")) {
                material->ambient_color[0] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                material->ambient_color[1] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                material->ambient_color[2] = nanomqo_strtof(ptr, &ptr);
                material->ambient_color[3] = 1.0f;
                NANOMQO_MATERIAL_CHECK_CLOSE_BLACE(ptr);
            }
            else if (nanomqoUtilExtractToken(&ptr, "amb(")) {
                material->ambient = nanomqo_strtof(ptr, &ptr);
                NANOMQO_MATERIAL_CHECK_CLOSE_BLACE(ptr);
            }
            else if (nanomqoUtilExtractToken(&ptr, "aplane(")) {
                p = (char *) memchr(ptr, ')', ptr - input);
                if (p) {
                    *p = nanomqo_eos;
                    nanomqoUtilExtractName(&material->alpha_plane_path, &ptr);
                    ptr = p;
                }
                else {
                    return NANOMQO_STATUS_ERROR_INVALID_MATERIAL_CHUNK;
                }
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_MATERIAL_CHUNK;
            }
            break;
        case 'b':
            if (nanomqoUtilExtractToken(&ptr, "bump(")) {
                p = (char *) memchr(ptr, ')', ptr - input);
                if (p) {
                    *p = nanomqo_eos;
                    nanomqoUtilExtractName(&material->bump_map_path, &ptr);
                    ptr = p;
                }
                else {
                    return NANOMQO_STATUS_ERROR_INVALID_MATERIAL_CHUNK;
                }
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_MATERIAL_CHUNK;
            }
            break;
        case 'c':
            if (nanomqoUtilExtractToken(&ptr, "col(")) {
                material->color[0] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                material->color[1] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                material->color[2] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                material->color[3] = nanomqo_strtof(ptr, &ptr);
                NANOMQO_MATERIAL_CHECK_CLOSE_BLACE(ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_MATERIAL_CHUNK;
            }
            break;
        case 'd':
            if (nanomqoUtilExtractToken(&ptr, "dbls(")) {
                material->disable_culling = strtol(ptr, &ptr, 10);
                NANOMQO_MATERIAL_CHECK_CLOSE_BLACE(ptr);
            }
            else if (nanomqoUtilExtractToken(&ptr, "dif(")) {
                material->diffusion = nanomqo_strtof(ptr, &ptr);
                NANOMQO_MATERIAL_CHECK_CLOSE_BLACE(ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_MATERIAL_CHUNK;
            }
            break;
        case 'e':
            if (nanomqoUtilExtractToken(&ptr, "emi(")) {
                material->emissive = nanomqo_strtof(ptr, &ptr);
                NANOMQO_MATERIAL_CHECK_CLOSE_BLACE(ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_MATERIAL_CHUNK;
            }
            break;
        case 'p':
            if (nanomqoUtilExtractToken(&ptr, "power(")) {
                material->power = nanomqo_strtof(ptr, &ptr);
                NANOMQO_MATERIAL_CHECK_CLOSE_BLACE(ptr);
            }
            else if (nanomqoUtilExtractToken(&ptr, "proj_type(")) {
                u1.v = strtol(ptr, &ptr, 10);
                material->projection_type = u1.e;
                NANOMQO_MATERIAL_CHECK_CLOSE_BLACE(ptr);
            }
            else if (nanomqoUtilExtractToken(&ptr, "proj_pos(")) {
                material->projection_position[0] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                material->projection_position[1] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                material->projection_position[2] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                NANOMQO_MATERIAL_CHECK_CLOSE_BLACE(ptr);
            }
            else if (nanomqoUtilExtractToken(&ptr, "proj_scale(")) {
                material->projection_scale[0] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                material->projection_scale[1] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                material->projection_scale[2] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                NANOMQO_MATERIAL_CHECK_CLOSE_BLACE(ptr);
            }
            else if (nanomqoUtilExtractToken(&ptr, "proj_angle(")) {
                material->projection_angle[0] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                material->projection_angle[1] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                material->projection_angle[2] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                NANOMQO_MATERIAL_CHECK_CLOSE_BLACE(ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_MATERIAL_CHUNK;
            }
            break;
        case 'r':
            if (nanomqoUtilExtractToken(&ptr, "reflect(")) {
                material->reflect = nanomqo_strtof(ptr, &ptr);
                NANOMQO_MATERIAL_CHECK_CLOSE_BLACE(ptr);
            }
            else if (nanomqoUtilExtractToken(&ptr, "refract(")) {
                material->refract = nanomqo_strtof(ptr, &ptr);
                NANOMQO_MATERIAL_CHECK_CLOSE_BLACE(ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_MATERIAL_CHUNK;
            }
            break;
        case 's':
            if (nanomqoUtilExtractToken(&ptr, "shader(")) {
                u2.v = strtol(ptr, &ptr, 10);
                material->shader_type = u2.e;
                NANOMQO_MATERIAL_CHECK_CLOSE_BLACE(ptr);
            }
            else if (nanomqoUtilExtractToken(&ptr, "spc(")) {
                material->specular = nanomqo_strtof(ptr, &ptr);
                NANOMQO_MATERIAL_CHECK_CLOSE_BLACE(ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_MATERIAL_CHUNK;
            }
            break;
        case 't':
            if (nanomqoUtilExtractToken(&ptr, "tex(")) {
                p = (char *) memchr(ptr, ')', ptr - input);
                if (p) {
                    *p = nanomqo_eos;
                    nanomqoUtilExtractName(&material->texture_path, &ptr);
                    ptr = p;
                }
                else {
                    return NANOMQO_STATUS_ERROR_INVALID_MATERIAL_CHUNK;
                }
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_MATERIAL_CHUNK;
            }
            break;
        case 'u':
            if (nanomqoUtilExtractToken(&ptr, "uid(")) {
                material->uid = strtol(ptr, &ptr, 10);
                NANOMQO_MATERIAL_CHECK_CLOSE_BLACE(ptr);
            }
            break;
        case 'v':
            if (nanomqoUtilExtractToken(&ptr, "vcol(")) {
                material->has_vertex_color = strtol(ptr, &ptr, 10);
                NANOMQO_MATERIAL_CHECK_CLOSE_BLACE(ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_MATERIAL_CHUNK;
            }
            break;
        default:
            is_parsing = nanomqo_false;
            break;
        }
    }
    return NANOMQO_STATUS_SUCCESS;
}

void
nanomqoMaterialDestroy(nanomqo_material_t *material)
{
    if (nanomqo_is_not_null(material)) {
        if (nanomqo_is_not_null(material->name)) {
            nanomqo_free(material->name);
        }
        if (nanomqo_is_not_null(material->texture_path)) {
            nanomqo_free(material->texture_path);
        }
        if (nanomqo_is_not_null(material->alpha_plane_path)) {
            nanomqo_free(material->alpha_plane_path);
        }
        if (nanomqo_is_not_null(material->bump_map_path)) {
            nanomqo_free(material->bump_map_path);
        }
        nanomqo_free(material);
    }
}

nanomqo_object_t *
nanomqoObjectCreate(void)
{
    nanomqo_object_t *object;
    object = (nanomqo_object_t *) nanomqo_calloc(1, sizeof(*object));
    return object;
}

nanomqo_status_t
nanomqoObjectParse(nanomqo_object_t *object, nanomqo_buffer_t *buffer)
{
    char input[NANOMQO_BUFFER_CAPACITY], *ptr = input;
    int bvertex_count, bvertex_size, num_vertices = 0, num_faces, i;
    nanomqo_vertex_t *vertex;
    nanomqo_face_t *face;
    nanomqo_uint8_t c;
    nanomqo_bool_t is_parsing = nanomqo_true;
    union nanomqo_int_to_mirror_type_cast_t {
        int v;
        nanomqo_mirror_type_t e;
    } u1;
    union nanomqo_int_to_patch_type_cast_t {
        int v;
        nanomqo_patch_type_t e;
    } u2;
    union nanomqo_int_to_shading_type_cast_t {
        int v;
        nanomqo_shading_type_t e;
    } u3;
    if (nanomqo_is_null(buffer)) {
        return NANOMQO_STATUS_ERROR_NULL_BUFFER;
    }
    if (nanomqo_is_null(object)) {
        return NANOMQO_STATUS_ERROR_NULL_OBJECT;
    }
    nanomqoBufferGetLine(buffer, input, sizeof(input));
    if (!nanomqoUtilExtractToken(&ptr, "Object")) {
        return NANOMQO_STATUS_ERROR_UNKNOWN_CHUNK_TYPE;
    }
    nanomqoUtilExtractName(&object->name, &ptr);
    nanomqoUtilSkipSpaces(&ptr);
    buffer->depth += *ptr == '{';
    while (is_parsing) {
        nanomqoBufferSkipSpaces(buffer);
        ptr = input;
        c = nanomqoBufferGetChar(buffer);
        switch (c) {
        case 'c':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "color_type")) {
                object->color_type = strtol(ptr, &ptr, 10);
            }
            else if (nanomqoUtilExtractToken(&ptr, "color")) {
                object->color[0] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                object->color[1] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                object->color[2] = nanomqo_strtof(ptr, &ptr);
                object->color[3] = 1.0f;
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_OBJECT_CHUNK;
            }
            break;
        case 'd':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "depth")) {
                object->depth = strtol(ptr, &ptr, 10);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_OBJECT_CHUNK;
            }
            break;
        case 'f':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "facet")) {
                object->facet = nanomqo_strtof(ptr, &ptr);
            }
            else if (nanomqo_string_equals_const(input, "face")) {
                ptr = input + sizeof("face") - 1;
                nanomqoBufferExtractInt(buffer, ptr, &num_faces);
                if (num_faces > 0) {
                    object->num_faces = num_faces;
                    object->faces = (nanomqo_face_t **) nanomqo_calloc(num_faces, sizeof(*object->faces));
                    for (i = 0; i < num_faces; i++) {
                        nanomqoBufferSkipSpaces(buffer);
                        face = object->faces[i] = nanomqoFaceCreate();
                        nanomqoFaceParse(face, buffer);
                    }
                }
                nanomqoBufferEndChunk(buffer);
            }
            else if (nanomqoUtilExtractToken(&ptr, "folding")) {
                object->is_folding = strtol(ptr, &ptr, 10);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_OBJECT_CHUNK;
            }
            break;
        case 'l':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "locking")) {
                object->is_locked = strtol(ptr, &ptr, 10);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_OBJECT_CHUNK;
            }
            break;
        case 'm':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "mirror_axis")) {
                object->mirror_axis = strtol(ptr, &ptr, 10);
            }
            else if (nanomqoUtilExtractToken(&ptr, "mirror_dis")) {
                object->mirror_distance = nanomqo_strtof(ptr, &ptr);
            }
            else if (nanomqoUtilExtractToken(&ptr, "mirror")) {
                u1.v = strtol(ptr, &ptr, 10);
                object->mirror_type = u1.e;
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_OBJECT_CHUNK;
            }
            break;
        case 'p':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "patchtri")) {
                object->patch_triangulation_type = strtol(ptr, &ptr, 10);
            }
            else if (nanomqoUtilExtractToken(&ptr, "patch")) {
                u2.v = strtol(ptr, &ptr, 10);
                object->patch_type = u2.e;
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_OBJECT_CHUNK;
            }
            break;
        case 'r':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "rotation")) {
                object->orientation[0] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                object->orientation[1] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                object->orientation[2] = nanomqo_strtof(ptr, &ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_OBJECT_CHUNK;
            }
            break;
        case 's':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "segment")) {
                object->num_segments = strtol(ptr, &ptr, 10);
            }
            else if (nanomqoUtilExtractToken(&ptr, "shading")) {
                u3.v = strtol(ptr, &ptr, 10);
                object->shading_type = u3.e;
            }
            else if (nanomqoUtilExtractToken(&ptr, "scale")) {
                object->scale[0] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                object->scale[1] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                object->scale[2] = nanomqo_strtof(ptr, &ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_OBJECT_CHUNK;
            }
            break;
        case 't':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "translation")) {
                object->translation[0] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                object->translation[1] = nanomqo_strtof(ptr, &ptr);
                nanomqoUtilSkipSpaces(&ptr);
                object->translation[2] = nanomqo_strtof(ptr, &ptr);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_OBJECT_CHUNK;
            }
            break;
        case 'u':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqoUtilExtractToken(&ptr, "uid")) {
                object->uid = strtol(ptr, &ptr, 10);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_OBJECT_CHUNK;
            }
            break;
        case 'v':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqo_string_equals_const(input, "vertexattr")) {
                nanomqoBufferGetLine(buffer, input, sizeof(input));
                nanomqoUtilSkipSpaces(&ptr);
                if (nanomqo_string_equals_const(ptr, "uid")) {
                    nanomqoUtilSkipSpaces(&ptr);
                    buffer->depth += *ptr == '{';
                    object->num_vertex_attr_uids = num_vertices;
                    object->vertex_attr_uids = (int *) nanomqo_calloc(num_vertices, sizeof(*object->vertex_attr_uids));
                    for (i = 0; i < num_vertices; i++) {
                        nanomqoBufferSkipSpaces(buffer);
                        nanomqoBufferGetLine(buffer, input, sizeof(input));
                        ptr = input;
                        object->vertex_attr_uids[i] = strtol(ptr, &ptr, 10);
                    }
                    nanomqoBufferEndChunk(buffer);
                }
                nanomqoBufferEndChunk(buffer);
            }
            else if (nanomqo_string_equals_const(input, "vertex")) {
                ptr = input + sizeof("vertex") - 1;
                nanomqoBufferExtractInt(buffer, ptr, &num_vertices);
                if (num_vertices > 0) {
                    object->num_vertices = num_vertices;
                    object->vertices = (nanomqo_vertex_t **) nanomqo_calloc(num_vertices, sizeof(*object->vertices));
                    for (i = 0; i < num_vertices; i++) {
                        nanomqoBufferSkipSpaces(buffer);
                        nanomqoBufferGetLine(buffer, input, sizeof(input));
                        ptr = input;
                        vertex = object->vertices[i] = nanomqoVertexCreate();
                        vertex->origin[0] = nanomqo_strtof(ptr, &ptr);
                        nanomqoUtilSkipSpaces(&ptr);
                        vertex->origin[1] = nanomqo_strtof(ptr, &ptr);
                        nanomqoUtilSkipSpaces(&ptr);
                        vertex->origin[2] = nanomqo_strtof(ptr, &ptr);
                    }
                }
                nanomqoBufferEndChunk(buffer);
            }
            else if (nanomqoUtilExtractToken(&ptr, "visible")) {
                object->is_visible = strtol(ptr, &ptr, 10);
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_OBJECT_CHUNK;
            }
            break;
        case 'B':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqo_string_equals_const(input, "BVertex")) {
                ptr = input + sizeof("BVertex") - 1;
                nanomqoBufferExtractInt(buffer, ptr, &num_vertices);
                if (num_vertices > 0) {
                    nanomqoBufferGetLine(buffer, input, sizeof(input));
                    if (nanomqo_string_equals_const(input, "Vector")) {
                        ptr = input + sizeof("Vector") - 1;
                        nanomqoUtilSkipSpaces(&ptr);
                        bvertex_count = strtol(ptr, &ptr, 10);
                        nanomqoUtilSkipSpaces(&ptr);
                        if (*ptr == '[')
                            ptr++;
                        bvertex_size = strtol(ptr, &ptr, 10);
                        if (*ptr == ']')
                            ptr++;
                        if (bvertex_count * 12 == bvertex_size && nanomqoBufferCanReadLength(buffer, bvertex_size)) {
                            object->vertices = (nanomqo_vertex_t **) nanomqo_calloc(bvertex_count, sizeof(*object->vertices));
                            for (i = 0; i < bvertex_count; i++) {
                                vertex = object->vertices[i] = nanomqoVertexCreate();
                                memcpy(&vertex->origin, nanomqoBufferGetDataPtr(buffer), 12);
                                buffer->offset += 12;
                                buffer->columns += 12;
                            }
                            object->num_vertices = bvertex_count;
                            nanomqoBufferSkipSpaces(buffer);
                            if (nanomqoBufferTestString(buffer, "weit")) {
                                nanomqoBufferSkipSpaces(buffer);
                                buffer->depth++;
                                nanomqoBufferSkipLine(buffer);
                                nanomqoVertexParseWeightChunk(buffer, object->vertices, object->num_vertices);
                            }
                            else if (nanomqoBufferTestString(buffer, "color")) {
                                buffer->depth++;
                                nanomqoBufferSkipLine(buffer);
                                nanomqoVertexParseColorChunk(buffer, object->vertices, object->num_vertices);
                            }
                            nanomqoBufferEndChunk(buffer);
                            break;
                        }
                    }
                }
                return NANOMQO_STATUS_ERROR_INVALID_VERTEX_CHUNK;
            }
            break;
        case '}':
            nanomqoBufferSkipLine(buffer);
            buffer->depth--;
            is_parsing = nanomqo_false;
            break;
        default:
            nanomqoBufferSkipLine(buffer);
            break;
        }
    }
    return NANOMQO_STATUS_SUCCESS;
}

void
nanomqoObjectDestroy(nanomqo_object_t *object)
{
    nanomqo_rsize_t nvertices, nfaces, i;
    if (nanomqo_is_not_null(object)) {
        if (nanomqo_is_not_null(object->vertices)) {
            nvertices = object->num_vertices;
            for (i = 0; i < nvertices; i++) {
                nanomqoVertexDestroy(object->vertices[i]);
            }
            nanomqo_free(object->vertices);
        }
        if (nanomqo_is_not_null(object->faces)) {
            nfaces = object->num_faces;
            for (i = 0; i < nfaces; i++) {
                nanomqoFaceDestroy(object->faces[i]);
            }
            nanomqo_free(object->faces);
        }
        if (nanomqo_is_not_null(object->name)) {
            nanomqo_free(object->name);
        }
        nanomqo_free(object);
    }
}

#endif /* NANOMQO_PRIVATE_H_ */
