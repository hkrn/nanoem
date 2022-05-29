/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#ifndef NANODXM_PRIVATE_H_
#define NANODXM_PRIVATE_H_

/* THIS IS A PRIVATE DECLARATIONS OF NANODXM_C DO NOT INCLUDE DIRECTLY */

#include "./nanodxm.h"
#include "./kvec.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#define NANODXM_BUFFER_CAPACITY 256

#define nanodxm_string_equals(s1, s2) (nanodxm_is_not_null((s1)) && nanodxm_is_not_null((s2)) && strcmp((s1), (s2)) == 0)
#define nanodxm_string_equals_n(s1, s2, n) (nanodxm_is_not_null((s1)) && nanodxm_is_not_null((s2)) && strncmp((s1), (s2), (n)) == 0)
#define nanodxm_string_equals_const(s, c) (nanodxm_is_not_null((s)) && strncmp((s), (c), (sizeof((c)) - 1)) == 0)
#define nanodxm_strtof(a, b) ((float) strtod(a, b))
#define nanodxm_eos '\0'

extern const nanodxm_global_allocator_t *__nanodxm_global_allocator;

NANODXM_DECL_INLINE static void *
nanodxmMalloc(nanodxm_rsize_t size, const char *filename, int line)
{
    void *p = NULL;
    if (nanodxm_likely(size <= NANODXM_RSIZE_MAX)) {
        if (__nanodxm_global_allocator != NULL) {
            p = __nanodxm_global_allocator->malloc(__nanodxm_global_allocator->opaque, size, filename, line);
        }
        else {
            p = malloc(size);
        }
    }
    return p;
}

NANODXM_DECL_INLINE static void *
nanodxmRealloc(void *ptr, nanodxm_rsize_t size, const char *filename, int line)
{
    void *p = NULL;
    if (nanodxm_likely(size > 0 && size <= NANODXM_RSIZE_MAX)) {
        if (__nanodxm_global_allocator != NULL) {
            p = __nanodxm_global_allocator->realloc(__nanodxm_global_allocator->opaque, ptr, size, filename, line);
        }
        else {
            p = realloc(ptr, size);
        }
    }
    return p;
}

NANODXM_DECL_INLINE static void *
nanodxmCalloc(nanodxm_rsize_t length, nanodxm_rsize_t size, const char *filename, int line)
{
    void *p = NULL;
    if (nanodxm_likely(length > 0 && size > 0 && length <= (NANODXM_RSIZE_MAX / size))) {
        if (__nanodxm_global_allocator != NULL) {
            p = __nanodxm_global_allocator->calloc(__nanodxm_global_allocator->opaque, length, size, filename, line);
        }
        else {
            p = calloc(length, size);
        }
    }
    return p;
}

NANODXM_DECL_INLINE static void
nanodxmFree(void *ptr, const char *filename, int line)
{
    if (ptr) {
        if (__nanodxm_global_allocator != NULL) {
            __nanodxm_global_allocator->free(__nanodxm_global_allocator->opaque, ptr, filename, line);
        }
        else {
            free(ptr);
        }
    }
}

/* allocation macros */
#ifdef NANODXM_ENABLE_DEBUG_ALLOCATOR
#define nanodxm_malloc(size) nanodxmMalloc((size), __FILE__, __LINE__)
#define nanodxm_realloc(ptr, size) nanodxmRealloc((ptr), (size), __FILE__, __LINE__)
#define nanodxm_calloc(length, size) nanodxmCalloc((length), (size), __FILE__, __LINE__)
#define nanodxm_free(ptr) nanodxmFree((ptr), __FILE__, __LINE__)
#else
#define nanodxm_malloc(size) nanodxmMalloc((size), NULL, 0)
#define nanodxm_realloc(ptr, size) nanodxmRealloc((ptr), (size), NULL, 0)
#define nanodxm_calloc(length, size) nanodxmCalloc((length), (size), NULL, 0)
#define nanodxm_free(ptr) nanodxmFree((ptr), NULL, 0)
#endif /* NANODXM_ENABLE_DEBUG_ALLOCATOR */

#define NANODXM_CHECK_CLOSE_BRACE(ptr, err) \
    for (;;) {                              \
        if ((*ptr) == ')') {                \
            (ptr)++;                        \
            nanodxmUtilSkipSpaces(&(ptr));  \
            break;                          \
        }                                   \
        else {                              \
            return (err);                   \
        }                                   \
    }

#define NANODXM_FACE_CHECK_CLOSE_BLACE(ptr) NANODXM_CHECK_CLOSE_BRACE((ptr), NANODXM_STATUS_ERROR_INVALID_FACE_CHUNK)
#define NANODXM_MATERIAL_CHECK_CLOSE_BLACE(ptr) NANODXM_CHECK_CLOSE_BRACE((ptr), NANODXM_STATUS_ERROR_INVALID_MATERIAL_CHUNK)

struct nanodxm_buffer_t {
    const nanodxm_uint8_t *data;
    nanodxm_rsize_t length;
    nanodxm_rsize_t offset;
    int rows;
    int columns;
};
struct nanodxm_material_t {
    nanodxm_uint8_t *texture_filename;
    nanodxm_uint8_t *normal_map_filename;
    nanodxm_color_t diffuse;
    nanodxm_color_t emissive;
    nanodxm_color_t specular;
    nanodxm_float32_t shininess;
};
struct nanodxm_document_t {
    nanodxm_data_type_t data_type;
    int version_major;
    int version_minor;
    int binary_float_size;
    kvec_t(nanodxm_material_t *) materials;
    nanodxm_rsize_t num_vertices;
    nanodxm_vector3_t *vertices;
    nanodxm_rsize_t num_vertex_colors;
    nanodxm_color_t *vertex_colors;
    nanodxm_rsize_t num_vertex_faces;
    nanodxm_face_t *vertex_faces;
    nanodxm_rsize_t num_normals;
    nanodxm_vector3_t *normals;
    nanodxm_rsize_t num_normal_faces;
    nanodxm_face_t *normal_faces;
    nanodxm_rsize_t num_texcoords;
    nanodxm_texcoord_t *texcoords;
    nanodxm_rsize_t num_face_material_indices;
    int *face_material_indices;
};
struct nanodxm_parser_state_t {
    nanodxm_material_t *current_material;
    int depth;
};
typedef struct nanodxm_parser_state_t nanodxm_parser_state_t;

NANODXM_DECL_INLINE static void
nanodxmUtilSkipSpaces(char **ptr)
{
    char *p = *ptr;
    while (isspace(*p)) {
        p++;
    }
    *ptr = p;
}

NANODXM_DECL_INLINE static nanodxm_bool_t
nanodxmUtilExtractToken(char **ptr, const char *const str)
{
    if (nanodxm_string_equals_n(*ptr, str, strlen(str))) {
        *ptr += strlen(str);
        nanodxmUtilSkipSpaces(ptr);
        return nanodxm_true;
    }
    return nanodxm_false;
}

NANODXM_DECL_INLINE static nanodxm_uint8_t
nanodxmBufferGetChar(const nanodxm_buffer_t *buffer)
{
    return nanodxm_likely(buffer->offset < buffer->length) ? *(buffer->data + buffer->offset) : nanodxm_eos;
}

NANODXM_DECL_INLINE static nanodxm_bool_t
nanodxmBufferTestString(const nanodxm_buffer_t *buffer, const char *const text)
{
    const char *ptr = (const char *) nanodxmBufferGetDataPtr(buffer);
    return nanodxmBufferCanReadLength(buffer, strlen(text)) &&
        *ptr == *text &&
        nanodxm_string_equals_n(ptr, text, strlen(text));
}

NANODXM_DECL_INLINE static void
nanodxmBufferAdvanceOffset(nanodxm_buffer_t *buffer, nanodxm_uint8_t c)
{
    buffer->offset++;
    if (c == '\n') {
        buffer->rows++;
        buffer->columns = 0;
    }
    else {
        buffer->columns++;
    }
}

NANODXM_DECL_INLINE static void
nanodxmBufferSkipSpaces(nanodxm_buffer_t *buffer)
{
    nanodxm_uint8_t c = nanodxmBufferGetChar(buffer);
    while (c != nanodxm_eos && isspace(c)) {
        nanodxmBufferAdvanceOffset(buffer, c);
        c = nanodxmBufferGetChar(buffer);
    }
}

NANODXM_DECL_INLINE static void
nanodxmBufferSkipSeparator(nanodxm_buffer_t *buffer)
{
    nanodxm_uint8_t c = nanodxmBufferGetChar(buffer);
    while (c != nanodxm_eos && (isspace(c) || c == ';' || c == ',')) {
        nanodxmBufferAdvanceOffset(buffer, c);
        c = nanodxmBufferGetChar(buffer);
    }
}

NANODXM_DECL_INLINE static void
nanodxmBufferSkipUntilTerminator(nanodxm_buffer_t *buffer)
{
    nanodxm_uint8_t c1 = nanodxmBufferGetChar(buffer), c2;
    nanodxmBufferAdvanceOffset(buffer, c1);
    while (c1 != nanodxm_eos) {
        c2 = nanodxmBufferGetChar(buffer);
        nanodxmBufferAdvanceOffset(buffer, c2);
        if (c1 == ';' && c2 == ';') {
            break;
        }
        c1 = c2;
    }
}

static void
nanodxmBufferSkipLine(nanodxm_buffer_t *buffer)
{
    nanodxm_uint8_t c1 = nanodxmBufferGetChar(buffer), c2;
    while (c1 != nanodxm_eos) {
        buffer->offset++;
        if (c1 == '\n') {
            buffer->rows++;
            buffer->columns = 0;
            break;
        }
        c2 = nanodxmBufferGetChar(buffer);
        if (c1 == '\r' && c2 == '\n') {
            buffer->offset++;
            buffer->rows++;
            buffer->columns = 0;
            break;
        }
        c1 = c2;
    }
}

static void
nanodxmBufferSkipUntilEndChunk(nanodxm_buffer_t *buffer, nanodxm_parser_state_t *state)
{
    nanodxm_uint8_t c1 = nanodxmBufferGetChar(buffer), c2;
    int depth = 0;
    while (c1 != nanodxm_eos && depth >= 0) {
        buffer->offset++;
        buffer->columns++;
        if (c1 == '{') {
            state->depth++;
            depth++;
        }
        else if (c1 == '}') {
            state->depth--;
            depth--;
        }
        c2 = nanodxmBufferGetChar(buffer);
        if (c1 == '\r' && c2 == '\n') {
            buffer->rows++;
            buffer->columns = 0;
        }
        c1 = c2;
    }
    if (state->depth < 0) {
        state->depth = 0;
    }
}

static void
nanodxmBufferGetLine(nanodxm_buffer_t *buffer, char *input, nanodxm_rsize_t length)
{
    char *ptr;
    nanodxm_rsize_t len = length - 1;
    if (nanodxmBufferCanReadLength(buffer, len)) {
        memcpy(input, nanodxmBufferGetDataPtr(buffer), len);
    }
    else if (buffer->length > buffer->offset) {
        len = buffer->length - buffer->offset;
        memcpy(input, nanodxmBufferGetDataPtr(buffer), len);
    }
    else {
        *input = nanodxm_eos;
    }
    if (nanodxm_is_not_null(ptr = (char *) memchr(input, '\r', len))) {
        *ptr = nanodxm_eos;
    }
    if (nanodxm_is_not_null(ptr = (char *) memchr(input, '\n', len))) {
        *ptr = nanodxm_eos;
    }
    input[len] = nanodxm_eos;
    nanodxmBufferSkipLine(buffer);
}

NANODXM_DECL_INLINE static void
nanodxmBufferEndChunk(nanodxm_buffer_t *buffer, nanodxm_parser_state_t *state)
{
    char input[NANODXM_BUFFER_CAPACITY];
    nanodxmBufferSkipSpaces(buffer);
    nanodxmBufferGetLine(buffer, input, sizeof(input));
    state->depth -= *input == '}';
}

NANODXM_DECL_INLINE static void
nanodxmBufferReadStackString(nanodxm_buffer_t *buffer, char *stack, nanodxm_rsize_t size)
{
    nanodxm_rsize_t len;
    size -= 1;
    if (nanodxmBufferCanReadLength(buffer, size)) {
        memcpy(stack, nanodxmBufferGetDataPtr(buffer), size);
        stack[size] = 0;
    }
    else if (buffer->length > buffer->offset) {
        len = buffer->length - buffer->offset;
        memcpy(stack, nanodxmBufferGetDataPtr(buffer), len);
    }
    else {
        stack[0] = 0;
    }
}

NANODXM_DECL_INLINE static void
nanodxmBufferExtractInt(nanodxm_buffer_t *buffer, int *value)
{
    char stack[16], *ptr, *end = NULL;
    nanodxmBufferSkipSpaces(buffer);
    if ((ptr = (char *) nanodxmBufferGetDataPtr(buffer)) != NULL) {
        nanodxmBufferReadStackString(buffer, stack, sizeof(stack));
        *value = strtol(stack, &end, 10);
        buffer->offset += end - stack;
        nanodxmBufferSkipSpaces(buffer);
    }
    else {
        *value = 0;
    }
}

NANODXM_DECL_INLINE static void
nanodxmBufferExtractSize(nanodxm_buffer_t *buffer, nanodxm_rsize_t *value)
{
    char stack[16], *ptr, *end = NULL;
    nanodxmBufferSkipSpaces(buffer);
    if ((ptr = (char *) nanodxmBufferGetDataPtr(buffer)) != NULL) {
        nanodxmBufferReadStackString(buffer, stack, sizeof(stack));
        *value = (nanodxm_rsize_t) strtol(stack, &end, 10);
        buffer->offset += end - stack;
        nanodxmBufferSkipSpaces(buffer);
    }
    else {
        *value = 0;
    }
}

NANODXM_DECL_INLINE static void
nanodxmBufferExtractFloat(nanodxm_buffer_t *buffer, float *value)
{
    char stack[16], *ptr, *end = NULL;
    nanodxmBufferSkipSpaces(buffer);
    if ((ptr = (char *) nanodxmBufferGetDataPtr(buffer)) != NULL) {
        nanodxmBufferReadStackString(buffer, stack, sizeof(stack));
        *value = (float) strtod(stack, &end);
        buffer->offset += end - stack;
        nanodxmBufferSkipSpaces(buffer);
    }
    else {
        *value = 0;
    }
}

#endif /* NANODXM_PRIVATE_H_ */
