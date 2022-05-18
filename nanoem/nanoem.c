/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./nanoem_p.h"

#include <math.h>

#define nanoem_crt_sqrtf(a) sqrtf(a)

NANOEM_DECL_TLS const nanoem_global_allocator_t *__nanoem_global_allocator = NULL;

const nanoem_global_allocator_t *APIENTRY
nanoemGlobalGetCustomAllocator(void)
{
    return __nanoem_global_allocator;
}

void APIENTRY
nanoemGlobalSetCustomAllocator(const nanoem_global_allocator_t *allocator)
{
    if (nanoem_is_not_null(allocator)) {
        __nanoem_global_allocator = allocator;
    }
    else {
        __nanoem_global_allocator = NULL;
    }
}

union nanoem_char_to_unicode_string_cast_t {
    char *s;
    nanoem_unicode_string_t *v;
};

union nanoem_const_char_to_const_unicode_string_cast_t {
    const char *s;
    const nanoem_unicode_string_t *v;
};

static nanoem_unicode_string_t *
nanoemUnicodeStringFactoryCreateStringConvertPlainFromCallback(void *opaque, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    union nanoem_char_to_unicode_string_cast_t u;
    nanoem_mark_unused(opaque);
    nanoem_mark_unused(length);
    u.s = nanoemUtilCloneString((const char *) string, status);
    nanoem_status_ptr_assign(status, nanoem_is_not_null(u.s) ? NANOEM_STATUS_SUCCESS : NANOEM_STATUS_ERROR_NULL_OBJECT);
    return u.v;
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryCreateStringConvertPlainToCallback(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    union nanoem_const_char_to_const_unicode_string_cast_t u;
    nanoem_u8_t *plain = NULL;
    nanoem_mark_unused(opaque);
    if (nanoem_is_not_null(string)) {
        u.v = string;
        *length = nanoem_crt_strlen(u.s);
        nanoem_status_ptr_assign(status, NANOEM_STATUS_SUCCESS);
        plain = (nanoem_u8_t *) nanoemUtilCloneString(u.s, status);
    }
    else {
        *length = 0;
        nanoem_status_ptr_assign_null_object(status);
    }
    return plain;
}

static nanoem_i32_t
nanoemUnicodeStringFactoryHashCallback(void *opaque, const nanoem_unicode_string_t *string)
{
    nanoem_mark_unused(opaque);
    return __ac_X31_hash_string((const char *) string);
}

static int
nanoemUnicodeStringFactoryCompareCallback(void *opaque, const nanoem_unicode_string_t *left, const nanoem_unicode_string_t *right)
{
    nanoem_mark_unused(opaque);
    return nanoem_crt_strcmp((const char *) left, (const char *) right);
}

static const nanoem_u8_t *
nanoemUnicodeStringFactoryGetCacheCallback(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_codec_type_t codec, nanoem_status_t *status)
{
    nanoem_mark_unused(opaque);
    nanoem_mark_unused(codec);
    nanoem_status_ptr_assign(status, NANOEM_STATUS_SUCCESS);
    *length = nanoem_crt_strlen((const char *) string);
    return (const nanoem_u8_t *) string;
}

static void
nanoemUnicodeStringFactorySetCacheCallback(void *opaque, nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_codec_type_t codec, nanoem_status_t *status)
{
    nanoem_mark_unused(opaque);
    nanoem_mark_unused(string);
    nanoem_mark_unused(length);
    nanoem_mark_unused(codec);
    nanoem_status_ptr_assign(status, NANOEM_STATUS_SUCCESS);
}

static void
nanoemUnicodeStringFactoryDestroyStringCallback(void *opaque, nanoem_unicode_string_t *string)
{
    union nanoem_char_to_unicode_string_cast_t u;
    nanoem_mark_unused(opaque);
    u.v = string;
    nanoem_free(u.s);
}

static void
nanoemUnicodeStringFactoryDestroyByteArrayCallback(void *opaque, nanoem_u8_t *string)
{
    nanoem_mark_unused(opaque);
    nanoem_free(string);
}

nanoem_unicode_string_factory_t *APIENTRY
nanoemUnicodeStringFactoryCreate(nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    factory = (nanoem_unicode_string_factory_t *) nanoem_calloc(1, sizeof(*factory), status);
    if (nanoem_is_not_null(factory)) {
        factory->compare = nanoemUnicodeStringFactoryCompareCallback;
        factory->destroy_string = nanoemUnicodeStringFactoryDestroyStringCallback;
        factory->destroy_byte_array = nanoemUnicodeStringFactoryDestroyByteArrayCallback;
        factory->from_cp932 = nanoemUnicodeStringFactoryCreateStringConvertPlainFromCallback;
        factory->from_utf8 = nanoemUnicodeStringFactoryCreateStringConvertPlainFromCallback;
        factory->from_utf16 = nanoemUnicodeStringFactoryCreateStringConvertPlainFromCallback;
        factory->hash = nanoemUnicodeStringFactoryHashCallback;
        factory->get_cache = nanoemUnicodeStringFactoryGetCacheCallback;
        factory->set_cache = nanoemUnicodeStringFactorySetCacheCallback;
        factory->to_cp932 = nanoemUnicodeStringFactoryCreateStringConvertPlainToCallback;
        factory->to_utf8 = nanoemUnicodeStringFactoryCreateStringConvertPlainToCallback;
        factory->to_utf16 = nanoemUnicodeStringFactoryCreateStringConvertPlainToCallback;
    }
    return factory;
}

void APIENTRY
nanoemUnicodeStringFactorySetConvertFromCp932Callback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_from_codec_t value)
{
    if (nanoem_is_not_null(factory)) {
        factory->from_cp932 = value;
    }
}

void APIENTRY
nanoemUnicodeStringFactorySetConvertFromUtf8Callback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_from_codec_t value)
{
    if (nanoem_is_not_null(factory)) {
        factory->from_utf8 = value;
    }
}

void APIENTRY
nanoemUnicodeStringFactorySetConvertFromUtf16Callback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_from_codec_t value)
{
    if (nanoem_is_not_null(factory)) {
        factory->from_utf16 = value;
    }
}

void APIENTRY
nanoemUnicodeStringFactorySetConvertToCp932Callback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_to_codec_t value)
{
    if (nanoem_is_not_null(factory)) {
        factory->to_cp932 = value;
    }
}

void APIENTRY
nanoemUnicodeStringFactorySetConvertToUtf8Callback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_to_codec_t value)
{
    if (nanoem_is_not_null(factory)) {
        factory->to_utf8 = value;
    }
}

void APIENTRY
nanoemUnicodeStringFactorySetConvertToUtf16Callback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_to_codec_t value)
{
    if (nanoem_is_not_null(factory)) {
        factory->to_utf16 = value;
    }
}

void APIENTRY
nanoemUnicodeStringFactorySetHashCallback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_hash_t value)
{
    if (nanoem_is_not_null(factory)) {
        factory->hash = value;
    }
}

void APIENTRY
nanoemUnicodeStringFactorySetCompareCallback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_compare_t value)
{
    if (nanoem_is_not_null(factory)) {
        factory->compare = value;
    }
}

void APIENTRY
nanoemUnicodeStringFactorySetGetCacheCallback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_get_cache_t value)
{
    if (nanoem_is_not_null(factory)) {
        factory->get_cache = value;
    }
}

void APIENTRY
nanoemUnicodeStringFactorySetSetCacheCallback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_set_cache_t value)
{
    if (nanoem_is_not_null(factory)) {
        factory->set_cache = value;
    }
}

void APIENTRY
nanoemUnicodeStringFactorySetDestroyStringCallback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_destroy_string_t value)
{
    if (nanoem_is_not_null(factory)) {
        factory->destroy_string = value;
    }
}

void APIENTRY
nanoemUnicodeStringFactorySetDestroyByteArrayCallback(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_factory_destroy_byte_array_t value)
{
    if (nanoem_is_not_null(factory)) {
        factory->destroy_byte_array = value;
    }
}

void *APIENTRY
nanoemUnicodeStringFactoryGetOpaqueData(const nanoem_unicode_string_factory_t *factory)
{
    return nanoem_is_not_null(factory) ? factory->opaque_data : NULL;
}

void APIENTRY
nanoemUnicodeStringFactorySetOpaqueData(nanoem_unicode_string_factory_t *factory, void *value)
{
    if (nanoem_is_not_null(factory)) {
        factory->opaque_data = value;
    }
}
nanoem_unicode_string_t *APIENTRY
nanoemUnicodeStringFactoryCreateString(nanoem_unicode_string_factory_t *factory, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    return nanoemUnicodeStringFactoryCreateStringWithEncoding(factory, string, length, NANOEM_CODEC_TYPE_UTF8, status);
}

nanoem_u8_t *APIENTRY
nanoemUnicodeStringFactoryGetByteArray(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    return nanoemUnicodeStringFactoryGetByteArrayEncoding(factory, string, length, NANOEM_CODEC_TYPE_UTF8, status);
}

nanoem_unicode_string_t *APIENTRY
nanoemUnicodeStringFactoryCreateStringWithEncoding(nanoem_unicode_string_factory_t *factory, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_codec_type_t codec, nanoem_status_t *status)
{
    nanoem_unicode_string_t *s = NULL;
    if (nanoem_is_not_null(factory) && nanoem_is_not_null(string) && codec >= NANOEM_CODEC_TYPE_FIRST_ENUM && codec < NANOEM_CODEC_TYPE_MAX_ENUM) {
        switch (codec) {
        case NANOEM_CODEC_TYPE_SJIS:
            s = factory->from_cp932(factory->opaque_data, string, length, status);
            break;
        case NANOEM_CODEC_TYPE_UTF8:
            s = factory->from_utf8(factory->opaque_data, string, length, status);
            break;
        case NANOEM_CODEC_TYPE_UTF16:
            s = factory->from_utf16(factory->opaque_data, string, length, status);
            break;
        case NANOEM_CODEC_TYPE_UNKNOWN:
        case NANOEM_CODEC_TYPE_MAX_ENUM:
        default:
            break;
        }
    }
    return s;
}

nanoem_u8_t *APIENTRY
nanoemUnicodeStringFactoryGetByteArrayEncoding(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_codec_type_t codec, nanoem_status_t *status)
{
    nanoem_u8_t *s = NULL;
    if (nanoem_is_not_null(factory) && nanoem_is_not_null(string) && nanoem_is_not_null(length)) {
        switch (codec) {
        case NANOEM_CODEC_TYPE_SJIS:
            s = factory->to_cp932(factory->opaque_data, string, length, status);
            break;
        case NANOEM_CODEC_TYPE_UTF8:
            s = factory->to_utf8(factory->opaque_data, string, length, status);
            break;
        case NANOEM_CODEC_TYPE_UTF16:
            s = factory->to_utf16(factory->opaque_data, string, length, status);
            break;
        case NANOEM_CODEC_TYPE_UNKNOWN:
        case NANOEM_CODEC_TYPE_MAX_ENUM:
        default:
            break;
        }
    }
    return s;
}

nanoem_unicode_string_t *APIENTRY
nanoemUnicodeStringFactoryCloneString(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *source, nanoem_status_t *status)
{
    nanoem_unicode_string_t *dest = NULL;
    nanoem_rsize_t length;
    nanoem_u8_t *bytes;
    void *opaque;
    if (nanoem_is_not_null(factory) && nanoem_is_not_null(source)) {
        opaque = factory->opaque_data;
        bytes = factory->to_utf8(opaque, source, &length, status);
        dest = factory->from_utf8(opaque, bytes, length, status);
        factory->destroy_byte_array(opaque, bytes);
    }
    return dest;
}

void APIENTRY
nanoemUnicodeStringFactoryDestroyByteArray(nanoem_unicode_string_factory_t *factory, nanoem_u8_t *bytes)
{
    if (nanoem_is_not_null(factory)) {
        factory->destroy_byte_array(factory->opaque_data, bytes);
    }
}

int APIENTRY
nanoemUnicodeStringFactoryCompareString(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *lvalue, const nanoem_unicode_string_t *rvalue)
{
    int result = -1;
    if (nanoem_is_not_null(factory) && nanoem_is_not_null(lvalue) && nanoem_is_not_null(rvalue)) {
        result = factory->compare(factory->opaque_data, lvalue, rvalue);
    }
    return result;
}

const nanoem_u8_t * APIENTRY
nanoemUnicodeStringFactoryGetCacheString(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_codec_type_t codec, nanoem_status_t *status)
{
    const nanoem_u8_t *result = NULL;
    if (nanoem_is_not_null(factory) && nanoem_is_not_null(string) && nanoem_is_not_null(length) && codec >= NANOEM_CODEC_TYPE_FIRST_ENUM && codec < NANOEM_CODEC_TYPE_MAX_ENUM) {
        result = factory->get_cache(factory->opaque_data, string, length, codec, status);
    }
    return result;
}

void APIENTRY
nanoemUnicodeStringFactorySetCacheString(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_codec_type_t codec, nanoem_status_t *status)
{
    if (nanoem_is_not_null(factory) && nanoem_is_not_null(string) && nanoem_is_not_null(length) && codec >= NANOEM_CODEC_TYPE_FIRST_ENUM && codec < NANOEM_CODEC_TYPE_MAX_ENUM) {
        factory->set_cache(factory->opaque_data, string, length, codec, status);
    }
}

void APIENTRY
nanoemUnicodeStringFactoryDestroyString(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_t *string)
{
    if (nanoem_is_not_null(factory)) {
        factory->destroy_string(factory->opaque_data, string);
    }
}

void APIENTRY
nanoemUnicodeStringFactoryDestroy(nanoem_unicode_string_factory_t *factory)
{
    if (nanoem_is_not_null(factory)) {
        nanoem_free(factory);
    }
}

nanoem_buffer_t *APIENTRY
nanoemBufferCreate(const nanoem_u8_t *data, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_buffer_t *buffer;
    buffer = (nanoem_buffer_t *) nanoem_calloc(1, sizeof(*buffer), status);
    if (nanoem_is_not_null(buffer)) {
        buffer->data = data;
        buffer->length = length;
    }
    return buffer;
}

nanoem_rsize_t APIENTRY
nanoemBufferGetLength(const nanoem_buffer_t *buffer)
{
    return nanoem_is_not_null(buffer) ? buffer->length : 0;
}

nanoem_rsize_t APIENTRY
nanoemBufferGetOffset(const nanoem_buffer_t *buffer)
{
    return nanoem_is_not_null(buffer) ? buffer->offset : 0;
}

const nanoem_u8_t *APIENTRY
nanoemBufferGetDataPtr(const nanoem_buffer_t *buffer)
{
    return (nanoem_is_not_null(buffer) && buffer->offset < buffer->length) ? (buffer->data + buffer->offset) : NULL;
}

nanoem_bool_t APIENTRY
nanoemBufferCanReadLength(const nanoem_buffer_t *buffer, nanoem_rsize_t size)
{
    return size > 0 && nanoemBufferCanReadLengthInternal(buffer, size);
}

void APIENTRY
nanoemBufferSkip(nanoem_buffer_t *buffer, nanoem_rsize_t skip, nanoem_status_t *status)
{
    if (!nanoem_status_ptr_has_error(status)) {
        nanoem_status_ptr_assign(status, nanoemBufferCanReadLength(buffer, skip) ? NANOEM_STATUS_SUCCESS : NANOEM_STATUS_ERROR_BUFFER_END);
        if (!nanoem_status_ptr_has_error(status)) {
            buffer->offset += skip;
        }
    }
}

void APIENTRY
nanoemBufferSeek(nanoem_buffer_t *buffer, nanoem_rsize_t position, nanoem_status_t *status)
{
    if (!nanoem_status_ptr_has_error(status)) {
        nanoem_status_ptr_assign(status, nanoem_is_not_null(buffer) && position < buffer->length ? NANOEM_STATUS_SUCCESS : NANOEM_STATUS_ERROR_BUFFER_END);
        if (!nanoem_status_ptr_has_error(status)) {
            buffer->offset = position;
        }
    }
}

nanoem_u8_t APIENTRY
nanoemBufferReadByte(nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_u8_t result = 0;
    if (!nanoem_status_ptr_has_error(status)) {
        nanoem_status_ptr_assign(status, nanoemBufferCanReadLength(buffer, sizeof(result)) ? NANOEM_STATUS_SUCCESS : NANOEM_STATUS_ERROR_BUFFER_END);
        if (!nanoem_status_ptr_has_error(status)) {
            result = *(buffer->data + buffer->offset++);
        }
    }
    return result;
}

nanoem_i16_t APIENTRY
nanoemBufferReadInt16LittleEndian(nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    union nanoem_u16_to_int16_cast_t {
        nanoem_i16_t i;
        nanoem_u16_t u;
    } u;
    u.u = nanoemBufferReadInt16LittleEndianUnsigned(buffer, status);
    return u.i;
}

nanoem_u16_t APIENTRY
nanoemBufferReadInt16LittleEndianUnsigned(nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_u16_t result = 0;
    const nanoem_u8_t *data;
    if (!nanoem_status_ptr_has_error(status)) {
        nanoem_status_ptr_assign(status, nanoemBufferCanReadLength(buffer, sizeof(result)) ? NANOEM_STATUS_SUCCESS : NANOEM_STATUS_ERROR_BUFFER_END);
        if (!nanoem_status_ptr_has_error(status)) {
            data = buffer->data;
            result |= *(data + buffer->offset++);
            result |= *(data + buffer->offset++) << 8;
        }
    }
    return result;
}

nanoem_i32_t APIENTRY
nanoemBufferReadInt32LittleEndian(nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_u32_t result = 0;
    const nanoem_u8_t *data;
    if (!nanoem_status_ptr_has_error(status)) {
        nanoem_status_ptr_assign(status, nanoemBufferCanReadLength(buffer, sizeof(result)) ? NANOEM_STATUS_SUCCESS : NANOEM_STATUS_ERROR_BUFFER_END);
        if (!nanoem_status_ptr_has_error(status)) {
            data = buffer->data;
            result |= (nanoem_u32_t)(*(data + buffer->offset++));
            result |= (nanoem_u32_t)(*(data + buffer->offset++)) << 8;
            result |= (nanoem_u32_t)(*(data + buffer->offset++)) << 16;
            result |= (nanoem_u32_t)(*(data + buffer->offset++)) << 24;
        }
    }
    union nanoem_u32_to_int32_cast_t {
        nanoem_i32_t i;
        nanoem_u32_t u;
    } u;
    u.u = result;
    return u.i;
}

nanoem_f32_t APIENTRY
nanoemBufferReadFloat32LittleEndian(nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    union nanoem_i32_to_float32_cast_t {
        nanoem_i32_t i;
        nanoem_f32_t f;
    } u;
    u.i = nanoemBufferReadInt32LittleEndian(buffer, status);
    return u.f;
}

char *APIENTRY
nanoemBufferReadBuffer(nanoem_buffer_t *buffer, nanoem_rsize_t len, nanoem_status_t *status)
{
    char *bytes = NULL;
    if (!nanoem_status_ptr_has_error(status)) {
        nanoem_status_ptr_assign(status, nanoemBufferCanReadLength(buffer, len) ? NANOEM_STATUS_SUCCESS : NANOEM_STATUS_ERROR_BUFFER_END);
        if (!nanoem_status_ptr_has_error(status)) {
            bytes = (char *)nanoem_malloc(len + 1, status);
            if (nanoem_is_not_null(bytes)) {
                bytes[len] = '\0';
                nanoem_crt_memcpy(bytes, buffer->data + buffer->offset, len);
                buffer->offset += len;
            }
        }
        else {
            bytes = (char *)nanoem_calloc(1, sizeof(*bytes), status);
        }
    }
    return bytes;
}

void APIENTRY
nanoemBufferDestroy(nanoem_buffer_t *buffer)
{
    if (nanoem_is_not_null(buffer)) {
        nanoem_free(buffer);
    }
}

static int
nanoemModelGetBoneGetDepth(const nanoem_model_bone_t *bone, const nanoem_model_bone_t **parent)
{
    const nanoem_model_bone_t *p = nanoemModelBoneGetParentBoneObject(bone);
    int i = 0;
    if (p) {
        while (p && i < PMD_BONE_MAX_DEPTH) {
            i++;
            *parent = p = nanoemModelBoneGetParentBoneObject(p);
        }
    }
    else {
        *parent = bone;
    }
    return i;
}

int
nanoemModelCompareBonePMD(const void *a, const void *b)
{
    union nanoem_void_to_bone_cast_t {
        const void *p;
        const nanoem_model_bone_t **b;
    } u;
    const nanoem_model_bone_t *left, *right, *left_parent, *right_parent;
    int left_depth, right_depth, compare_result;
    u.p = a;
    left = *u.b;
    u.p = b;
    right = *u.b;
    left_depth = nanoemModelGetBoneGetDepth(left, &left_parent);
    right_depth = nanoemModelGetBoneGetDepth(right, &right_parent);
    if (left_parent && right_parent && left_depth == right_depth) {
        compare_result = left_parent->base.index - right_parent->base.index;
    }
    else {
        compare_result = left_depth - right_depth;
    }
    return compare_result;
}

int
nanoemModelCompareBonePMX(const void *a, const void *b)
{
    union nanoem_void_to_bone_cast_t {
        const void *p;
        const nanoem_model_bone_t **b;
    } u;
    const nanoem_model_bone_t *left, *right;
    int left_layer_index, right_layer_index, left_index, right_index;
    u.p = a;
    left = *u.b;
    u.p = b;
    right = *u.b;
    if (left->u.flags.is_affected_by_physics_simulation == right->u.flags.is_affected_by_physics_simulation) {
        left_layer_index = nanoemModelBoneGetStageIndex(left);
        right_layer_index = nanoemModelBoneGetStageIndex(right);
        if (left_layer_index == right_layer_index) {
            left_index = left->base.index;
            right_index = right->base.index;
            return left_index - right_index;
        }
        return left_layer_index - right_layer_index;
    }
    return right->u.flags.is_affected_by_physics_simulation ? NANOEM_MODEL_OBJECT_NOT_FOUND : 1;
}

nanoem_model_vertex_t *
nanoemModelVertexCreate(const nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_vertex_t *vertex;
    vertex = (nanoem_model_vertex_t *) nanoem_calloc(1, sizeof(*vertex), status);
    if (nanoem_is_not_null(vertex)) {
        nanoemModelObjectInitialize(&vertex->base, model);
        vertex->type = NANOEM_MODEL_VERTEX_TYPE_UNKNOWN;
    }
    return vertex;
}

static void
nanoemModelParseVertexBlockPMD(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_vertex_t *vertex;
    nanoem_rsize_t num_vertices, i;
    num_vertices = nanoemBufferReadLength(buffer, status);
    if (num_vertices > 0) {
        model->vertices = (nanoem_model_vertex_t **) nanoem_calloc(num_vertices, sizeof(*model->vertices), status);
        if (nanoem_is_not_null(model->vertices)) {
            model->num_vertices = num_vertices;
            for (i = 0; i < num_vertices; i++) {
                vertex = nanoemModelVertexCreate(model, status);
                nanoemModelVertexParsePMD(vertex, buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemModelVertexDestroy(vertex);
                    model->num_vertices = i;
                    break;
                }
                vertex->base.index = (int) i;
                model->vertices[i] = vertex;
            }
        }
    }
}

static void
nanoemModelParseVertexIndexBlockPMD(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_rsize_t num_vertex_indices, num_vertices, i;
    nanoem_u16_t vertex_index;
    num_vertex_indices = nanoemBufferReadLength(buffer, status);
    num_vertices = model->num_vertices;
    if ((num_vertex_indices == 0 && num_vertices > 0) || num_vertex_indices % 3 != 0) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_FACE_CORRUPTED);
    }
    else if (num_vertex_indices > 0) {
        model->vertex_indices = (nanoem_u32_t *) nanoem_calloc(num_vertex_indices, sizeof(*model->vertex_indices), status);
        if (nanoem_is_not_null(model->vertex_indices)) {
            model->num_vertex_indices = num_vertex_indices;
            for (i = 0; i < num_vertex_indices; i++) {
                vertex_index = nanoemBufferReadInt16LittleEndianUnsigned(buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    model->num_vertex_indices = i;
                    nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_FACE_CORRUPTED);
                    break;
                }
                model->vertex_indices[i] = vertex_index < num_vertices ? vertex_index : 0;
            }
        }
    }
}

static void
nanoemModelParseMaterialBlockPMD(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_material_t *material;
    nanoem_rsize_t num_materials, i;
    num_materials = nanoemBufferReadLength(buffer, status);
    if (num_materials > 0) {
        model->materials = (nanoem_model_material_t **) nanoem_calloc(num_materials, sizeof(*model->materials), status);
        if (nanoem_is_not_null(model->materials)) {
            model->num_materials = num_materials;
            for (i = 0; i < num_materials; i++) {
                material = nanoemModelMaterialCreate(model, status);
                nanoemModelMaterialParsePMD(material, buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemModelMaterialDestroy(material);
                    model->num_materials = i;
                    break;
                }
                material->base.index = (int) i;
                model->materials[i] = material;
            }
        }
    }
}

static nanoem_unicode_string_t *
nanoemBufferGetStringFromCp932(nanoem_buffer_t *buffer, nanoem_rsize_t max_capacity, nanoem_unicode_string_factory_t *factory, nanoem_status_t *status)
{
    nanoem_unicode_string_t *dst = NULL;
    nanoem_rsize_t length;
    char *src;
    if (!nanoem_status_ptr_has_error(status)) {
        nanoem_status_ptr_assign(status, nanoemBufferCanReadLength(buffer, max_capacity) ? NANOEM_STATUS_SUCCESS : NANOEM_STATUS_ERROR_BUFFER_END);
        if (!nanoem_status_ptr_has_error(status) && nanoem_is_not_null(factory)) {
            src = nanoemBufferReadBuffer(buffer, max_capacity, status);
            if (nanoem_is_not_null(src)) {
                length = nanoem_crt_strlen(src);
                dst = nanoemUnicodeStringFactoryCreateStringWithEncoding(factory, (nanoem_u8_t *) src, length > max_capacity ? max_capacity : length, NANOEM_CODEC_TYPE_SJIS, status);
                nanoem_free(src);
            }
        }
    }
    return dst;
}

static nanoem_unicode_string_t *
nanoemModelGetStringPMX(const nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory = model->factory;
    nanoem_unicode_string_t *dst = NULL;
    nanoem_codec_type_t codec;
    nanoem_rsize_t length = nanoemBufferReadLength(buffer, status);
    char *src = NULL;
    if (!nanoem_status_ptr_has_error(status)) {
        src = nanoemBufferReadBuffer(buffer, length, status);
    }
    if (nanoem_is_not_null(src)) {
        if (model->info.codec_type == 1) {
            codec = NANOEM_CODEC_TYPE_UTF8;
        }
        else {
            codec = NANOEM_CODEC_TYPE_UTF16;
        }
        dst = nanoemUnicodeStringFactoryCreateStringWithEncoding(factory, (nanoem_u8_t *) src, length,  codec, status);
        nanoem_free(src);
    }
    return dst;
}

static nanoem_bool_t
nanoemModelBoneHasWristBoneNameSuffixPMD(nanoem_u8_t *name_buffer, nanoem_rsize_t length)
{
    return name_buffer && length > 2 && name_buffer[length - 1] == 0x80 && name_buffer[length - 2] == 0x9d;
}

static void
nanoemModelBoneProcessWristBonePMD(nanoem_model_bone_t *bone, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model = nanoemModelBoneGetParentModel(bone);
    nanoem_unicode_string_factory_t *factory = nanoem_is_not_null(parent_model) ? parent_model->factory : NULL;
    const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_JAPANESE);
    const nanoem_f32_t *source_bone_origin, *target_bone_origin;
    nanoem_f32_t fixed_axis[4], vector_length;
    nanoem_rsize_t length;
    nanoem_u8_t *name_buffer = nanoemUnicodeStringFactoryGetByteArrayEncoding(factory, name, &length, NANOEM_CODEC_TYPE_SJIS, status);
    if (nanoemModelBoneHasWristBoneNameSuffixPMD(name_buffer, length)) {
        source_bone_origin = nanoemModelBoneGetOrigin(bone);
        target_bone_origin = nanoemModelBoneGetOrigin(nanoemModelBoneGetTargetBoneObject(bone));
        fixed_axis[0] = target_bone_origin[0] - source_bone_origin[0];
        fixed_axis[1] = target_bone_origin[1] - source_bone_origin[1];
        fixed_axis[2] = target_bone_origin[2] - source_bone_origin[2];
        fixed_axis[3] = 0;
        vector_length = 1.0 / nanoem_crt_sqrtf(fixed_axis[0] * fixed_axis[0] + fixed_axis[1] * fixed_axis[1] + fixed_axis[2] * fixed_axis[2]);
        fixed_axis[0] *= vector_length;
        fixed_axis[1] *= vector_length;
        fixed_axis[2] *= vector_length;
        bone->u.flags.has_fixed_axis = 1;
        bone->u.flags.has_destination_bone_index = 0;
        nanoem_crt_memcpy(bone->fixed_axis.values, fixed_axis, sizeof(fixed_axis));
    }
    else{
        nanoemUnicodeStringFactoryDestroyByteArray(factory, name_buffer);
        name = nanoemModelBoneGetName(nanoemModelBoneGetTargetBoneObject(bone), NANOEM_LANGUAGE_TYPE_JAPANESE);
        name_buffer = nanoemUnicodeStringFactoryGetByteArrayEncoding(factory, name, &length, NANOEM_CODEC_TYPE_SJIS, status);
        if (nanoemModelBoneHasWristBoneNameSuffixPMD(name_buffer, length)) {
            bone->u.flags.has_destination_bone_index = 0;
        }
    }
    nanoemUnicodeStringFactoryDestroyByteArray(factory, name_buffer);
}

static void
nanoemModelParseBoneBlockPMD(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_bone_t *bone;
    nanoem_rsize_t num_bones, i;
    num_bones = nanoemBufferReadInt16LittleEndianUnsigned(buffer, status);
    if (num_bones > 0) {
        model->bones = (nanoem_model_bone_t **) nanoem_calloc(num_bones, sizeof(*model->bones), status);
        model->ordered_bones = (nanoem_model_bone_t **) nanoem_calloc(num_bones, sizeof(*model->ordered_bones), status);
        if (nanoem_is_not_null(model->bones) && nanoem_is_not_null(model->ordered_bones)) {
            model->num_bones = num_bones;
            for (i = 0; i < num_bones; i++) {
                bone = nanoemModelBoneCreate(model, status);
                nanoemModelBoneParsePMD(bone, buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemModelBoneDestroy(bone);
                    num_bones = model->num_bones = i;
                    break;
                }
                bone->base.index = (int) i;
                model->ordered_bones[i] = model->bones[i] = bone;
            }
            for (i = 0; i < num_bones; i++) {
                nanoemModelBoneProcessWristBonePMD(model->bones[i], status);
            }
            if (!nanoem_status_ptr_has_error(status)) {
                nanoem_crt_qsort(model->ordered_bones, num_bones, sizeof(*model->ordered_bones), nanoemModelCompareBonePMD);
            }
        }
    }
}

static void
nanoemModelParseConstraintBlockPMD(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_constraint_t *constraint;
    nanoem_rsize_t num_constraints, i;
    num_constraints = nanoemBufferReadInt16LittleEndianUnsigned(buffer, status);
    if (num_constraints > 0) {
        model->constraints = (nanoem_model_constraint_t **) nanoem_calloc(num_constraints, sizeof(*model->constraints), status);
        if (nanoem_is_not_null(model->constraints)) {
            model->num_constraints = num_constraints;
            for (i = 0; i < num_constraints; i++) {
                constraint = nanoemModelConstraintCreate(model, status);
                nanoemModelConstraintParsePMD(constraint, buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemModelConstraintDestroy(constraint);
                    model->num_constraints = i;
                    break;
                }
                constraint->base.index = (int) i;
                model->constraints[i] = constraint;
            }
        }
    }
}

static void
nanoemModelParseMorphBlockPMD(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_morph_vertex_t *const *vertex_morphs, *vertex_morph;
    nanoem_model_morph_t *morph, *base_morph = NULL;
    nanoem_rsize_t num_vertices, base_morph_num_objects, num_morphs, i, j;
    int relative_index;
    num_morphs = nanoemBufferReadInt16LittleEndianUnsigned(buffer, status);
    if (num_morphs > 0) {
        model->morphs = (nanoem_model_morph_t **) nanoem_calloc(num_morphs, sizeof(*model->morphs), status);
        if (nanoem_is_not_null(model->morphs)) {
            model->num_morphs = num_morphs;
            for (i = 0; i < num_morphs; i++) {
                morph = nanoemModelMorphCreate(model, status);
                nanoemModelMorphParsePMD(morph, buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemModelMorphDestroy(morph);
                    model->num_morphs = i;
                    break;
                }
                if (morph->category == NANOEM_MODEL_MORPH_CATEGORY_BASE) {
                    base_morph = morph;
                }
                morph->base.index = (int) i;
                model->morphs[i] = morph;
            }
            if (base_morph) {
                base_morph_num_objects = base_morph->num_objects;
                for (i = 0; i < num_morphs; i++) {
                    morph = model->morphs[i];
                    if (morph && morph->category != NANOEM_MODEL_MORPH_CATEGORY_BASE) {
                        vertex_morphs = morph->u.vertices;
                        num_vertices = morph->num_objects;
                        for (j = 0; j < num_vertices; j++) {
                            vertex_morph = vertex_morphs[j];
                            relative_index = vertex_morph->vertex_index;
                            if (relative_index >= 0 && (nanoem_rsize_t) relative_index < base_morph_num_objects) {
                                vertex_morph->relative_index = relative_index;
                                vertex_morph->vertex_index = base_morph->u.vertices[relative_index]->vertex_index;
                            }
                        }
                    }
                }
            }
        }
    }
}

static void
nanoemModelParseLabelBlockPMD(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    nanoem_model_label_t *bone_label, *morph_label, **category_labels = NULL;
    nanoem_model_label_item_t *label_item, **label_items = NULL;
    nanoem_rsize_t *label_current_indices = NULL, num_morph_labels, num_bone_category_names, num_bone_labels, i;
    int bone_index, label_index;
    morph_label = nanoemModelLabelCreate(model, status);
    num_morph_labels = nanoemBufferReadByte(buffer, status);
    if (num_morph_labels > 0) {
        morph_label->is_special = nanoem_true;
        morph_label->items = label_items = (nanoem_model_label_item_t **) nanoem_calloc(num_morph_labels, sizeof(*label_item), status);
        if (nanoem_is_not_null(morph_label->items)) {
            morph_label->num_items = num_morph_labels;
            for (i = 0; i < num_morph_labels; i++) {
                label_item = nanoemModelLabelItemCreate(morph_label, status);
                label_item->type = NANOEM_MODEL_LABEL_ITEM_TYPE_MORPH;
                label_item->u.morph = nanoemModelGetOneMorphObject(model, nanoemBufferReadInt16LittleEndianUnsigned(buffer, status));
                label_items[i] = label_item;
            }
        }
    }
    num_bone_category_names = nanoemBufferReadByte(buffer, status);
    if (num_bone_category_names > 0) {
        label_current_indices = (nanoem_rsize_t *) nanoem_calloc(num_bone_category_names + 1, sizeof(*label_current_indices), status);
        model->labels = category_labels = (nanoem_model_label_t **) nanoem_calloc(num_bone_category_names + 1, sizeof(*category_labels), status);
        if (nanoem_is_not_null(category_labels)) {
            factory = model->factory;
            model->num_labels = num_bone_category_names + 1;
            category_labels[0] = morph_label;
            for (i = 1; i <= num_bone_category_names; i++) {
                bone_label = nanoemModelLabelCreate(model, status);
                if (nanoem_is_not_null(bone_label)) {
                    bone_label->name_ja = nanoemBufferGetStringFromCp932(buffer, PMD_BONE_CATEGORY_NAME_LENGTH, factory, status);
                    category_labels[i] = bone_label;
                }
                else {
                    break;
                }
            }
        }
    }
    else {
        model->labels = category_labels = (nanoem_model_label_t **) nanoem_calloc(1, sizeof(*category_labels), status);
        if (nanoem_is_not_null(category_labels)) {
            model->num_labels = 1;
            category_labels[0] = morph_label;
        }
    }
    if (nanoem_is_null(category_labels)) {
        nanoemModelLabelDestroy(morph_label);
    }
    num_bone_labels = nanoemBufferReadLength(buffer, status);
    if (num_bone_labels > 0 && nanoem_is_not_null(category_labels) && nanoem_is_not_null(label_current_indices)) {
        for (i = 0; i < num_bone_labels; i++) {
            bone_index = nanoemBufferReadInt16LittleEndian(buffer, status);
            label_index = nanoemBufferReadByte(buffer, status);
            if (label_index > 0 && (nanoem_rsize_t) label_index <= num_bone_category_names) {
                bone_label = category_labels[label_index];
                label_items = bone_label->items;
                if (!label_items) {
                    bone_label->items = label_items = (nanoem_model_label_item_t **) nanoem_calloc(num_bone_labels, sizeof(*bone_label->items), status);
                }
                if (nanoem_is_not_null(label_items)) {
                    label_item = label_items[label_current_indices[label_index]] = (nanoem_model_label_item_t *) nanoem_calloc(1, sizeof(*label_item), status);
                    if (nanoem_is_not_null(label_item)) {
                        label_item->type = NANOEM_MODEL_LABEL_ITEM_TYPE_BONE;
                        label_item->u.bone = nanoemModelGetOneBoneObject(model, bone_index);
                        bone_label->num_items = ++label_current_indices[label_index];
                    }
                }
                else {
                    break;
                }
            }
        }
    }
    nanoem_free(label_current_indices);
}

static void
nanoemModelParseEnglishBlockPMD(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    nanoem_rsize_t num_objects, i;
    if (nanoemBufferReadByte(buffer, status) != 0) {
        factory = model->factory;
        model->name_en = nanoemBufferGetStringFromCp932(buffer, PMD_MODEL_NAME_LENGTH, factory, status);
        if (nanoem_status_ptr_has_error(status)) {
            return;
        }
        model->comment_en = nanoemBufferGetStringFromCp932(buffer, PMD_MODEL_COMMENT_LENGTH, factory, status);
        if (nanoem_status_ptr_has_error(status)) {
            return;
        }
        num_objects = model->num_bones;
        for (i = 0; i < num_objects; i++) {
            model->bones[i]->name_en = nanoemBufferGetStringFromCp932(buffer, PMD_BONE_NAME_LENGTH, factory, status);
            if (nanoem_status_ptr_has_error(status)) {
                break;
            }
        }
        num_objects = model->num_morphs;
        if (num_objects > 0) {
            /* first one for base morph */
            num_objects -= 1;
            for (i = 0; i < num_objects; i++) {
                model->morphs[i + 1]->name_en = nanoemBufferGetStringFromCp932(buffer, PMD_MORPH_NAME_LENGTH, factory, status);
                if (nanoem_status_ptr_has_error(status)) {
                    break;
                }
            }
        }
        num_objects = model->num_labels;
        if (num_objects > 0) {
            /* first one for reserved morph label */
            num_objects -= 1;
            for (i = 0; i < num_objects; i++) {
                model->labels[i + 1]->name_en = nanoemBufferGetStringFromCp932(buffer, PMD_BONE_CATEGORY_NAME_LENGTH, factory, status);
                if (nanoem_status_ptr_has_error(status)) {
                    break;
                }
            }
        }
    }
}

static void
nanoemModelParseCustomToonTextureBlockPMD(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_texture_t *texture;
    nanoem_rsize_t num_textures = 10, i;
    if (num_textures > 0) {
        model->textures = (nanoem_model_texture_t **) nanoem_calloc(10, sizeof(*model->textures), status);
        if (nanoem_is_not_null(model->textures)) {
            model->num_textures = num_textures;
            for (i = 0; i < num_textures; i++) {
                texture = nanoemModelTextureCreate(model, status);
                nanoemModelTextureParsePMD(texture, buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemModelTextureDestroy(texture);
                    model->num_textures = i;
                    break;
                }
                texture->base.index = (int) i;
                model->textures[i] = texture;
            }
        }
    }
}

static void
nanoemModelParseRigidBodyBlockPMD(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_rigid_body_t *rigid_body;
    nanoem_rsize_t num_rigid_bodies, i;
    num_rigid_bodies = nanoemBufferReadLength(buffer, status);
    if (num_rigid_bodies > 0) {
        model->rigid_bodies = (nanoem_model_rigid_body_t **) nanoem_calloc(num_rigid_bodies, sizeof(*model->rigid_bodies), status);
        if (nanoem_is_not_null(model->rigid_bodies)) {
            model->num_rigid_bodies = num_rigid_bodies;
            for (i = 0; i < num_rigid_bodies; i++) {
                rigid_body = nanoemModelRigidBodyCreate(model, status);
                nanoemModelRigidBodyParsePMD(rigid_body, buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemModelRigidBodyDestroy(rigid_body);
                    model->num_rigid_bodies = i;
                    break;
                }
                rigid_body->base.index = (int) i;
                model->rigid_bodies[i] = rigid_body;
            }
        }
    }
}

static void
nanoemModelParseJointBlockPMD(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_joint_t *joint;
    nanoem_rsize_t num_joints, i;
    num_joints = nanoemBufferReadLength(buffer, status);
    if (num_joints > 0) {
        model->joints = (nanoem_model_joint_t **) nanoem_calloc(num_joints, sizeof(*model->rigid_bodies), status);
        if (nanoem_is_not_null(model->joints)) {
            model->num_joints = num_joints;
            for (i = 0; i < num_joints; i++) {
                joint = nanoemModelJointCreate(model, status);
                nanoemModelJointParsePMD(joint, buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemModelJointDestroy(joint);
                    model->num_joints = i;
                    break;
                }
                joint->base.index = (int) i;
                model->joints[i] = joint;
            }
        }
    }
}

static nanoem_bool_t
nanoemModelParsePMD(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemModelParseVertexBlockPMD(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    nanoemModelParseVertexIndexBlockPMD(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    nanoemModelParseMaterialBlockPMD(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    nanoemModelParseBoneBlockPMD(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    nanoemModelParseConstraintBlockPMD(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    nanoemModelParseMorphBlockPMD(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    nanoemModelParseLabelBlockPMD(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    if (nanoemBufferIsEnd(buffer)) {
        return nanoem_true;
    }
    nanoemModelParseEnglishBlockPMD(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    nanoemModelParseCustomToonTextureBlockPMD(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    if (nanoemBufferIsEnd(buffer)) {
        return nanoem_true;
    }
    nanoemModelParseRigidBodyBlockPMD(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    nanoemModelParseJointBlockPMD(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    return nanoemBufferIsEnd(buffer);
}

static void
nanoemModelParseVertexBlockPMX(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_vertex_t *vertex;
    nanoem_rsize_t num_vertices, i;
    num_vertices = nanoemBufferReadLength(buffer, status);
    if (num_vertices > 0) {
        model->vertices = (nanoem_model_vertex_t **) nanoem_calloc(num_vertices, sizeof(*model->vertices), status);
        if (nanoem_is_not_null(model->vertices)) {
            model->num_vertices = num_vertices;
            for (i = 0; i < num_vertices; i++) {
                vertex = nanoemModelVertexCreate(model, status);
                nanoemModelVertexParsePMX(vertex, buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemModelVertexDestroy(vertex);
                    model->num_vertices = i;
                    break;
                }
                vertex->base.index = (int) i;
                model->vertices[i] = vertex;
            }
        }
    }
}

static void
nanoemModelParseVertexIndexBlockPMX(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_u32_t vertex_index;
    nanoem_rsize_t vertex_index_size, num_vertices, num_vertex_indices, i;
    vertex_index_size = model->info.vertex_index_size;
    num_vertex_indices = nanoemBufferReadLength(buffer, status);
    num_vertices = model->num_vertices;
    if ((num_vertex_indices == 0 && num_vertices > 0) || num_vertex_indices % 3 != 0) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_FACE_CORRUPTED);
    }
    else if (num_vertex_indices > 0) {
        model->vertex_indices = (nanoem_u32_t *) nanoem_calloc(num_vertex_indices, sizeof(*model->vertex_indices), status);
        if (nanoem_is_not_null(model->vertex_indices)) {
            model->num_vertex_indices = num_vertex_indices;
            for (i = 0; i < num_vertex_indices; i++) {
                vertex_index = (nanoem_u32_t) nanoemBufferReadInteger(buffer, vertex_index_size, status);
                if (nanoem_status_ptr_has_error(status)) {
                    model->num_vertex_indices = i;
                    nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_FACE_CORRUPTED);
                    break;
                }
                model->vertex_indices[i] = vertex_index < num_vertices ? vertex_index : 0;
            }
        }
    }
}

static void
nanoemModelParseTextureBlockPMX(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_texture_t *texture;
    nanoem_rsize_t num_textures, i;
    num_textures = nanoemBufferReadLength(buffer, status);
    if (num_textures > 0) {
        model->textures = (nanoem_model_texture_t **) nanoem_calloc(num_textures, sizeof(*model->textures), status);
        if (nanoem_is_not_null(model->textures)) {
            model->num_textures = num_textures;
            for (i = 0; i < num_textures; i++) {
                texture = nanoemModelTextureCreate(model, status);
                nanoemModelTextureParsePMX(texture, buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemModelTextureDestroy(texture);
                    model->num_textures = i;
                    break;
                }
                texture->base.index = (int) i;
                model->textures[i] = texture;
            }
        }
    }
}

static void
nanoemModelParseMaterialBlockPMX(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_material_t *material;
    nanoem_rsize_t num_materials, i;
    num_materials = nanoemBufferReadLength(buffer, status);
    if (num_materials > 0) {
        model->materials = (nanoem_model_material_t **) nanoem_calloc(num_materials, sizeof(*model->materials), status);
        if (nanoem_is_not_null(model->materials)) {
            model->num_materials = num_materials;
            for (i = 0; i < num_materials; i++) {
                material = nanoemModelMaterialCreate(model, status);
                nanoemModelMaterialParsePMX(material, buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemModelMaterialDestroy(material);
                    model->num_materials = i;
                    break;
                }
                material->base.index = (int) i;
                model->materials[i] = material;
            }
        }
    }
}

static void
nanoemModelParseBoneBlockPMX(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_bone_t *bone;
    nanoem_rsize_t num_bones, i;
    num_bones = nanoemBufferReadLength(buffer, status);
    if (num_bones > 0) {
        model->bones = (nanoem_model_bone_t **) nanoem_calloc(num_bones, sizeof(*model->bones), status);
        model->ordered_bones = (nanoem_model_bone_t **) nanoem_calloc(num_bones, sizeof(*model->ordered_bones), status);
        if (nanoem_is_not_null(model->bones) && nanoem_is_not_null(model->ordered_bones)) {
            model->num_bones = num_bones;
            for (i = 0; i < num_bones; i++) {
                bone = nanoemModelBoneCreate(model, status);
                nanoemModelBoneParsePMX(bone, buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemModelBoneDestroy(bone);
                    model->num_bones = i;
                    break;
                }
                bone->base.index = (int) i;
                if (bone->constraint) {
                    bone->constraint->target_bone_index = (int) i;
                }
                model->ordered_bones[i] = model->bones[i] = bone;
            }
            if (!nanoem_status_ptr_has_error(status)) {
                nanoem_crt_qsort(model->ordered_bones, num_bones, sizeof(*model->ordered_bones), nanoemModelCompareBonePMX);
            }
        }
    }
}

static void
nanoemModelParseMorphBlockPMX(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_morph_t *morph;
    nanoem_rsize_t num_morphs, i;
    num_morphs = nanoemBufferReadLength(buffer, status);
    if (num_morphs > 0) {
        model->morphs = (nanoem_model_morph_t **) nanoem_calloc(num_morphs, sizeof(*model->morphs), status);
        if (nanoem_is_not_null(model->morphs)) {
            model->num_morphs = num_morphs;
            for (i = 0; i < num_morphs; i++) {
                morph = nanoemModelMorphCreate(model, status);
                nanoemModelMorphParsePMX(morph, buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemModelMorphDestroy(morph);
                    model->num_morphs = i;
                    break;
                }
                morph->base.index = (int) i;
                model->morphs[i] = morph;
            }
        }
    }
}

static void
nanoemModelParseLabelBlockPMX(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_label_t *label;
    nanoem_rsize_t num_labels, i;
    num_labels = nanoemBufferReadLength(buffer, status);
    if (num_labels > 0) {
        model->labels = (nanoem_model_label_t **) nanoem_calloc(num_labels, sizeof(*model->labels), status);
        if (nanoem_is_not_null(model->labels)) {
            model->num_labels = num_labels;
            for (i = 0; i < num_labels; i++) {
                label = nanoemModelLabelCreate(model, status);
                nanoemModelLabelParsePMX(label, buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemModelLabelDestroy(label);
                    model->num_labels = i;
                    break;
                }
                label->base.index = (int) i;
                model->labels[i] = label;
            }
        }
    }
}

static void
nanoemModelParseRigidBodyBlockPMX(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_rigid_body_t *rigid_body;
    nanoem_rsize_t num_rigid_bodies, i;
    num_rigid_bodies = nanoemBufferReadLength(buffer, status);
    if (num_rigid_bodies > 0) {
        model->rigid_bodies = (nanoem_model_rigid_body_t **) nanoem_calloc(num_rigid_bodies, sizeof(*model->rigid_bodies), status);
        if (nanoem_is_not_null(model->rigid_bodies)) {
            model->num_rigid_bodies = num_rigid_bodies;
            for (i = 0; i < num_rigid_bodies; i++) {
                rigid_body = nanoemModelRigidBodyCreate(model, status);
                nanoemModelRigidBodyParsePMX(rigid_body, buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemModelRigidBodyDestroy(rigid_body);
                    model->num_rigid_bodies = i;
                    break;
                }
                rigid_body->base.index = (int) i;
                model->rigid_bodies[i] = rigid_body;
            }
        }
    }
}

static void
nanoemModelParseJointBlockPMX(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_joint_t *joint;
    nanoem_rsize_t num_joints, i;
    num_joints = nanoemBufferReadLength(buffer, status);
    if (num_joints > 0) {
        model->joints = (nanoem_model_joint_t **) nanoem_calloc(num_joints, sizeof(*model->joints), status);
        if (nanoem_is_not_null(model->joints)) {
            model->num_joints = num_joints;
            for (i = 0; i < num_joints; i++) {
                joint = nanoemModelJointCreate(model, status);
                nanoemModelJointParsePMX(joint, buffer,status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemModelJointDestroy(joint);
                    model->num_joints = i;
                    break;
                }
                joint->base.index = (int) i;
                model->joints[i] = joint;
            }
        }
    }
}

static void
nanoemModelParseSoftBodyBlockPMX(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_soft_body_t *soft_body;
    nanoem_rsize_t num_soft_bodies, i;
    num_soft_bodies = nanoemBufferReadLength(buffer, status);
    if (num_soft_bodies > 0) {
        model->soft_bodies = (nanoem_model_soft_body_t **) nanoem_calloc(num_soft_bodies, sizeof(*model->soft_bodies), status);
        if (nanoem_is_not_null(model->soft_bodies)) {
            model->num_soft_bodies = num_soft_bodies;
            for (i = 0; i < num_soft_bodies; i++) {
                soft_body = nanoemModelSoftBodyCreate(model, status);
                nanoemModelSoftBodyParsePMX(soft_body, buffer,status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemModelSoftBodyDestroy(soft_body);
                    model->num_soft_bodies = i;
                    break;
                }
                soft_body->base.index = (int) i;
                model->soft_bodies[i] = soft_body;
            }
        }
    }
}

static nanoem_bool_t
nanoemModelParsePMX(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemModelParseVertexBlockPMX(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    nanoemModelParseVertexIndexBlockPMX(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    nanoemModelParseTextureBlockPMX(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    nanoemModelParseMaterialBlockPMX(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    nanoemModelParseBoneBlockPMX(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    nanoemModelParseMorphBlockPMX(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    nanoemModelParseLabelBlockPMX(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    nanoemModelParseRigidBodyBlockPMX(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    nanoemModelParseJointBlockPMX(model, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    if (model->version > 2.0 && !nanoemBufferIsEnd(buffer)) {
        nanoemModelParseSoftBodyBlockPMX(model, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            return nanoem_false;
        }
    }
    return nanoemBufferIsEnd(buffer);
}

void
nanoemModelVertexParsePMD(nanoem_model_vertex_t *vertex, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    if (nanoem_is_not_null(vertex) && nanoem_is_not_null(buffer)) {
        nanoemBufferReadFloat32x3LittleEndian(buffer, &vertex->origin, status);
        nanoemBufferReadFloat32x3LittleEndian(buffer, &vertex->normal, status);
        vertex->uv.values[0] = nanoemBufferReadFloat32LittleEndian(buffer, status);
        vertex->uv.values[1] = nanoemBufferReadFloat32LittleEndian(buffer, status);
        vertex->bone_indices[0] = nanoemBufferReadInt16LittleEndian(buffer, status);
        vertex->bone_indices[1] = nanoemBufferReadInt16LittleEndian(buffer, status);
        vertex->bone_weight_origin = nanoemBufferReadByte(buffer, status);
        vertex->bone_weights.values[0] = vertex->bone_weight_origin / 100.0f;
        vertex->bone_weights.values[1] = 1.0f - vertex->bone_weights.values[0];
        vertex->type = NANOEM_MODEL_VERTEX_TYPE_BDEF2;
        vertex->num_bone_indices = vertex->num_bone_weights = 2;
        vertex->edge_size = nanoemBufferReadByte(buffer, status) == 0 ? 1.0f : 0;
        nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_VERTEX_CORRUPTED);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemModelVertexParsePMX(nanoem_model_vertex_t *vertex, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_rsize_t bone_index_size;
    int additional_uv_size, type, i;
    if (nanoem_is_not_null(vertex) && nanoem_is_not_null(buffer)) {
        parent_model = nanoemModelVertexGetParentModel(vertex);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
            return;
        }
        nanoemBufferReadFloat32x3LittleEndian(buffer, &vertex->origin, status);
        nanoemBufferReadFloat32x3LittleEndian(buffer, &vertex->normal, status);
        vertex->uv.values[0] = nanoemBufferReadFloat32LittleEndian(buffer, status);
        vertex->uv.values[1] = nanoemBufferReadFloat32LittleEndian(buffer, status);
        additional_uv_size = parent_model->info.additional_uv_size;
        for (i = 0; i < additional_uv_size; i++) {
            nanoemBufferReadFloat32x4LittleEndian(buffer, &vertex->additional_uv[i], status);
        }
        bone_index_size = parent_model->info.bone_index_size;
        type = nanoemBufferReadByte(buffer, status);
        switch (type) {
        case NANOEM_MODEL_VERTEX_TYPE_BDEF1:
            vertex->bone_indices[0] = nanoemBufferReadIntegerNullable(buffer, bone_index_size, status);
            vertex->bone_weights.values[0] = 1.0f;
            vertex->num_bone_indices = vertex->num_bone_weights = 1;
            vertex->type = (nanoem_model_vertex_type_t) type;
            break;
        case NANOEM_MODEL_VERTEX_TYPE_BDEF2:
            vertex->bone_indices[0] = nanoemBufferReadIntegerNullable(buffer, bone_index_size, status);
            vertex->bone_indices[1] = nanoemBufferReadIntegerNullable(buffer, bone_index_size, status);
            vertex->bone_weights.values[0] = nanoemBufferReadClampedLittleEndian(buffer, status);
            vertex->bone_weights.values[1] = 1.0f - vertex->bone_weights.values[0];
            vertex->num_bone_indices = vertex->num_bone_weights = 2;
            vertex->type = (nanoem_model_vertex_type_t) type;
            break;
        case NANOEM_MODEL_VERTEX_TYPE_BDEF4:
        case NANOEM_MODEL_VERTEX_TYPE_QDEF:
            vertex->bone_indices[0] = nanoemBufferReadIntegerNullable(buffer, bone_index_size, status);
            vertex->bone_indices[1] = nanoemBufferReadIntegerNullable(buffer, bone_index_size, status);
            vertex->bone_indices[2] = nanoemBufferReadIntegerNullable(buffer, bone_index_size, status);
            vertex->bone_indices[3] = nanoemBufferReadIntegerNullable(buffer, bone_index_size, status);
            nanoemBufferReadFloat32x4LittleEndian(buffer, &vertex->bone_weights, status);
            vertex->num_bone_indices = vertex->num_bone_weights = 4;
            vertex->type = (nanoem_model_vertex_type_t) type;
            break;
        case NANOEM_MODEL_VERTEX_TYPE_SDEF:
            vertex->bone_indices[0] = nanoemBufferReadIntegerNullable(buffer, bone_index_size, status);
            vertex->bone_indices[1] = nanoemBufferReadIntegerNullable(buffer, bone_index_size, status);
            vertex->bone_weights.values[0] = nanoemBufferReadClampedLittleEndian(buffer, status);
            vertex->bone_weights.values[1] = 1.0f - vertex->bone_weights.values[0];
            vertex->num_bone_indices = vertex->num_bone_weights = 2;
            nanoemBufferReadFloat32x3LittleEndian(buffer, &vertex->sdef_c, status);
            nanoemBufferReadFloat32x3LittleEndian(buffer, &vertex->sdef_r0, status);
            nanoemBufferReadFloat32x3LittleEndian(buffer, &vertex->sdef_r1, status);
            vertex->sdef_c.values[3] = vertex->sdef_r0.values[3] = vertex->sdef_r1.values[3] = 1.0f;
            vertex->type = (nanoem_model_vertex_type_t) type;
            break;
        case NANOEM_MODEL_VERTEX_TYPE_MAX_ENUM:
        case NANOEM_MODEL_VERTEX_TYPE_UNKNOWN:
        default:
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_VERTEX_CORRUPTED);
            break;
        }
        vertex->edge_size = nanoemBufferReadFloat32LittleEndian(buffer, status);
        nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_VERTEX_CORRUPTED);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

static void
nanoemModelObjectDestroy(nanoem_model_object_t *object)
{
    nanoem_user_data_t *user_data = object->user_data;
    if (user_data && user_data->type == NANOEM_USER_DATA_DESTROY_CALLBACK_MODEL_OBJECT) {
        user_data->destroy.model_object(user_data->opaque, object);
    }
    nanoemUserDataDestroy(user_data);
    object->parent.model = NULL;
    object->index = NANOEM_MODEL_OBJECT_NOT_FOUND;
}

void
nanoemModelVertexDestroy(nanoem_model_vertex_t *vertex)
{
    if (nanoem_is_not_null(vertex)) {
        nanoemModelObjectDestroy(&vertex->base);
        nanoem_free(vertex);
    }
}

nanoem_model_material_t *
nanoemModelMaterialCreate(const nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_material_t *material;
    material = (nanoem_model_material_t *) nanoem_calloc(1, sizeof(*material), status);
    if (nanoem_is_not_null(material)) {
        nanoemModelObjectInitialize(&material->base, model);
        material->sphere_map_texture_type = NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_UNKNOWN;
        material->diffuse_texture_index = NANOEM_MODEL_OBJECT_NOT_FOUND;
        material->sphere_map_texture_index = NANOEM_MODEL_OBJECT_NOT_FOUND;
        material->toon_texture_index = NANOEM_MODEL_OBJECT_NOT_FOUND;
    }
    return material;
}

static void
nanoemModelMaterialSetTexturePMD(nanoem_model_material_t *material, const char *s, nanoem_rsize_t len, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model = nanoemModelMaterialGetParentModel(material);
    nanoem_unicode_string_factory_t *factory = nanoem_is_not_null(parent_model) ? parent_model->factory : NULL;
    nanoem_model_texture_t *texture = nanoemModelTextureCreate(parent_model, status);
    if (nanoem_is_not_null(texture)) {
        texture->path = nanoemUnicodeStringFactoryCreateStringWithEncoding(factory, (const nanoem_u8_t *) s, len, NANOEM_CODEC_TYPE_SJIS, status);
        if (len >= 4 && s[len - 1] == 'a' && s[len - 2] == 'p' && s[len - 3] == 's' && s[len - 4] == '.') {
            nanoemModelTextureDestroy(material->sphere_map_texture_spa);
            material->sphere_map_texture_spa = texture;
            material->sphere_map_texture_type = NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_ADD;
        }
        else if (len >= 4 && s[len - 1] == 'h' && s[len - 2] == 'p' && s[len - 3] == 's' && s[len - 4] == '.') {
            nanoemModelTextureDestroy(material->sphere_map_texture_sph);
            material->sphere_map_texture_sph = texture;
            material->sphere_map_texture_type = NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_MULTIPLY;
        }
        else {
            nanoemModelTextureDestroy(material->diffuse_texture);
            material->diffuse_texture = texture;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemModelMaterialParsePMD(nanoem_model_material_t *material, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const char *ptr;
    char str[PMD_MATERIAL_DIFFUSE_TEXTURE_NAME_LENGTH + 1];
    if (nanoem_is_not_null(material) && nanoem_is_not_null(buffer)) {
        nanoemBufferReadFloat32x3LittleEndian(buffer, &material->diffuse_color, status);
        material->diffuse_opacity = nanoemBufferReadFloat32LittleEndian(buffer, status);
        material->edge_size = 1.0f;
        material->edge_opacity = 1.0f;
        material->specular_power = nanoemBufferReadFloat32LittleEndian(buffer, status);
        nanoemBufferReadFloat32x3LittleEndian(buffer, &material->specular_color, status);
        nanoemBufferReadFloat32x3LittleEndian(buffer, &material->ambient_color, status);
        material->toon_texture_index = nanoemBufferReadByte(buffer, status);
        material->is_toon_shared = material->toon_texture_index != 0xff;
        material->sphere_map_texture_type = NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE;
        material->u.flags.is_edge_enabled = nanoemBufferReadByte(buffer, status) ? 1 : 0;
        material->num_vertex_indices = nanoemBufferReadInt32LittleEndian(buffer, status);
        material->u.flags.is_culling_disabled = material->diffuse_opacity < 1.0f;
        material->u.flags.is_casting_shadow_enabled = material->u.flags.is_edge_enabled;
        material->u.flags.is_casting_shadow_map_enabled = !(material->diffuse_opacity >= 0.98f && material->diffuse_opacity < 0.99f);
        material->u.flags.is_shadow_map_enabled = 1;
        if (nanoemBufferCanReadLength(buffer, PMD_MATERIAL_DIFFUSE_TEXTURE_NAME_LENGTH)) {
            nanoemUtilCopyString(str, sizeof(str), (const char *) nanoemBufferGetDataPtr(buffer), PMD_MATERIAL_DIFFUSE_TEXTURE_NAME_LENGTH);
            nanoemBufferSkip(buffer, PMD_MATERIAL_DIFFUSE_TEXTURE_NAME_LENGTH, status);
            if (*str) {
                ptr = nanoem_crt_strchr(str, '*');
                if (ptr) {
                    nanoemModelMaterialSetTexturePMD(material, str, ptr - str, status);
                    nanoemModelMaterialSetTexturePMD(material, ptr + 1, nanoem_crt_strlen(ptr + 1), status);
                }
                else {
                    nanoemModelMaterialSetTexturePMD(material, str, nanoem_crt_strlen(str), status);
                }
            }
            nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_MATERIAL_CORRUPTED);
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_MATERIAL_CORRUPTED);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemModelMaterialParsePMX(nanoem_model_material_t *material, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_rsize_t texture_index_size;
    int sphere_map_texture_type;
    if (nanoem_is_not_null(material) && nanoem_is_not_null(buffer)) {
        parent_model = nanoemModelMaterialGetParentModel(material);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
            return;
        }
        texture_index_size = parent_model->info.texture_index_size;
        material->name_ja = nanoemModelGetStringPMX(parent_model, buffer, status);
        material->name_en = nanoemModelGetStringPMX(parent_model, buffer, status);
        nanoemBufferReadFloat32x3LittleEndian(buffer, &material->diffuse_color, status);
        material->diffuse_opacity = nanoemBufferReadFloat32LittleEndian(buffer, status);
        nanoemBufferReadFloat32x3LittleEndian(buffer, &material->specular_color, status);
        material->specular_power = nanoemBufferReadFloat32LittleEndian(buffer, status);
        nanoemBufferReadFloat32x3LittleEndian(buffer, &material->ambient_color, status);
        material->u.value = nanoemBufferReadByte(buffer, status);
        if (material->u.flags.is_point_draw_enabled) {
            material->u.flags.is_casting_shadow_enabled = 0;
            material->u.flags.is_casting_shadow_map_enabled = 0;
            material->u.flags.is_shadow_map_enabled = 0;
        }
        else if (material->u.flags.is_line_draw_enabled) {
            material->u.flags.is_edge_enabled = 0;
        }
        nanoemBufferReadFloat32x3LittleEndian(buffer, &material->edge_color, status);
        material->edge_opacity = nanoemBufferReadFloat32LittleEndian(buffer, status);
        material->edge_size = nanoemBufferReadFloat32LittleEndian(buffer, status);
        material->diffuse_texture_index = nanoemBufferReadIntegerNullable(buffer, texture_index_size, status);
        material->sphere_map_texture_index = nanoemBufferReadIntegerNullable(buffer, texture_index_size, status);
        sphere_map_texture_type = nanoemBufferReadByte(buffer, status);
        switch (sphere_map_texture_type) {
        case NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_ADD:
        case NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_MULTIPLY:
        case NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE:
        case NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_SUB_TEXTURE:
            material->sphere_map_texture_type = (nanoem_model_material_sphere_map_texture_type_t) sphere_map_texture_type;
            break;
        case 0xff:
            material->sphere_map_texture_type = NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE;
            break;
        default:
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_MATERIAL_CORRUPTED);
            break;
        }
        material->is_toon_shared = nanoemBufferReadByte(buffer, status) != 0;
        if (material->is_toon_shared) {
            material->toon_texture_index = nanoemBufferReadByte(buffer, status);
        }
        else {
            material->toon_texture_index = nanoemBufferReadIntegerNullable(buffer, texture_index_size, status);
        }
        material->clob = nanoemModelGetStringPMX(parent_model, buffer, status);
        material->num_vertex_indices = nanoemBufferReadInt32LittleEndian(buffer, status);
        nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_MATERIAL_CORRUPTED);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemModelMaterialDestroy(nanoem_model_material_t *material)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(material)) {
        parent_model = material->base.parent.model;
        nanoemModelObjectDestroy(&material->base);
        nanoemModelTextureDestroy(material->diffuse_texture);
        nanoemModelTextureDestroy(material->sphere_map_texture_spa);
        nanoemModelTextureDestroy(material->sphere_map_texture_sph);
        if (nanoem_is_not_null(parent_model)) {
            factory = parent_model->factory;
            nanoemUtilDestroyString(material->name_ja, factory);
            nanoemUtilDestroyString(material->name_en, factory);
            nanoemUtilDestroyString(material->clob, factory);
        }
        nanoem_free(material);
    }
}

nanoem_model_bone_t *
nanoemModelBoneCreate(const nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_bone_t *bone;
    bone = (nanoem_model_bone_t *) nanoem_calloc(1, sizeof(*bone), status);
    if (nanoem_is_not_null(bone)) {
        nanoemModelObjectInitialize(&bone->base, model);
        bone->parent_bone_index = NANOEM_MODEL_OBJECT_NOT_FOUND;
        bone->parent_inherent_bone_index = NANOEM_MODEL_OBJECT_NOT_FOUND;
        bone->effector_bone_index = NANOEM_MODEL_OBJECT_NOT_FOUND;
        bone->target_bone_index = NANOEM_MODEL_OBJECT_NOT_FOUND;
    }
    return bone;
}

void
nanoemModelBoneParsePMD(nanoem_model_bone_t *bone, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    int bone_type;
    if (nanoem_is_not_null(bone) && nanoem_is_not_null(buffer)) {
        factory = bone->base.parent.model->factory;
        bone->name_ja = nanoemBufferGetStringFromCp932(buffer, PMD_BONE_NAME_LENGTH, factory, status);
        bone->parent_bone_index = nanoemBufferReadInt16LittleEndian(buffer, status);
        bone->target_bone_index = nanoemBufferReadInt16LittleEndian(buffer, status);
        bone->u.flags.is_user_handleable = bone->u.flags.is_rotateable = bone->u.flags.is_visible = 1;
        bone_type = nanoemBufferReadByte(buffer, status);
        bone->effector_bone_index = nanoemBufferReadInt16LittleEndian(buffer, status);
        bone->inherent_coefficient = 1.0f;
        bone->local_x_axis.values[0] = bone->local_z_axis.values[2] = 1.0f;
        nanoemBufferReadFloat32x3LittleEndian(buffer, &bone->origin, status);
        switch (bone_type) {
        case NANOEM_MOTION_BONE_TYPE_CONSTRAINT_EFFECTOR:
            bone->u.flags.is_movable = bone->u.flags.has_destination_bone_index = bone->u.flags.has_constraint = 1;
            bone->type = (nanoem_model_bone_type_t) bone_type;
            break;
        case NANOEM_MOTION_BONE_TYPE_CONSTRAINT_JOINT:
            bone->u.flags.has_destination_bone_index = 1;
            bone->type = (nanoem_model_bone_type_t) bone_type;
            break;
        case NANOEM_MOTION_BONE_TYPE_FIXED_AXIS:
            bone->u.flags.has_fixed_axis = 1;
            bone->type = (nanoem_model_bone_type_t) bone_type;
            break;
        case NANOEM_MOTION_BONE_TYPE_INHERENT_ORIENTATION_JOINT:
            bone->u.flags.has_inherent_orientation = bone->u.flags.has_destination_bone_index = 1;
            bone->parent_inherent_bone_index = bone->effector_bone_index;
            bone->inherent_coefficient = 1.0f;
            bone->type = (nanoem_model_bone_type_t) bone_type;
            break;
        case NANOEM_MOTION_BONE_TYPE_INHERENT_ORIENTATION_EFFECTOR:
            bone->u.flags.has_inherent_orientation = 1;
            bone->u.flags.is_visible = 0;
            bone->parent_inherent_bone_index = bone->target_bone_index;
            bone->inherent_coefficient = bone->effector_bone_index * 0.01f;
            bone->type = (nanoem_model_bone_type_t) bone_type;
            break;
        case NANOEM_MOTION_BONE_TYPE_ROTATABLE:
            bone->u.flags.has_destination_bone_index = 1;
            bone->type = (nanoem_model_bone_type_t) bone_type;
            break;
        case NANOEM_MOTION_BONE_TYPE_ROTATABLE_AND_MOVABLE:
            bone->u.flags.is_movable = bone->u.flags.has_destination_bone_index = 1;
            bone->type = (nanoem_model_bone_type_t) bone_type;
            break;
        case NANOEM_MOTION_BONE_TYPE_INVISIBLE:
            bone->u.flags.is_visible = 0;
            bone->type = (nanoem_model_bone_type_t) bone_type;
            break;
        case NANOEM_MOTION_BONE_TYPE_CONSTRAINT_ROOT:
            bone->u.flags.has_destination_bone_index = 1;
            bone->u.flags.is_visible = 0;
            bone->type = (nanoem_model_bone_type_t) bone_type;
            break;
        case NANOEM_MOTION_BONE_TYPE_MAX_ENUM:
        case NANOEM_MOTION_BONE_TYPE_UNKNOWN:
        default:
            break;
        }
        if (bone->target_bone_index == 0) {
            bone->target_bone_index = NANOEM_MODEL_OBJECT_NOT_FOUND;
        }
        nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_BONE_CORRUPTED);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemModelBoneParsePMX(nanoem_model_bone_t *bone, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_constraint_t *constraint;
    nanoem_rsize_t bone_index_size;
    if (nanoem_is_not_null(bone) && nanoem_is_not_null(buffer)) {
        parent_model = nanoemModelBoneGetParentModel(bone);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
            return;
        }
        bone_index_size = parent_model->info.bone_index_size;
        bone->name_ja = nanoemModelGetStringPMX(parent_model, buffer, status);
        bone->name_en = nanoemModelGetStringPMX(parent_model, buffer, status);
        nanoemBufferReadFloat32x3LittleEndian(buffer, &bone->origin, status);
        bone->parent_bone_index = nanoemBufferReadIntegerNullable(buffer, bone_index_size, status);
        bone->stage_index = nanoemBufferReadInt32LittleEndian(buffer, status);
        bone->type = NANOEM_MOTION_BONE_TYPE_UNKNOWN;
        bone->u.value = nanoemBufferReadInt16LittleEndianUnsigned(buffer, status);
        if (bone->u.flags.has_destination_bone_index) {
            bone->target_bone_index = nanoemBufferReadIntegerNullable(buffer, bone_index_size, status);
        }
        else {
            nanoemBufferReadFloat32x3LittleEndian(buffer, &bone->destination_origin, status);
        }
        if (bone->u.flags.has_inherent_orientation || bone->u.flags.has_inherent_translation) {
            bone->parent_inherent_bone_index = nanoemBufferReadIntegerNullable(buffer, bone_index_size, status);
            bone->inherent_coefficient = nanoemBufferReadFloat32LittleEndian(buffer, status);
        }
        else {
            bone->inherent_coefficient = 1.0f;
        }
        if (bone->u.flags.has_fixed_axis) {
            nanoemBufferReadFloat32x3LittleEndian(buffer, &bone->fixed_axis, status);
        }
        if (bone->u.flags.has_local_axes) {
            nanoemBufferReadFloat32x3LittleEndian(buffer, &bone->local_x_axis, status);
            nanoemBufferReadFloat32x3LittleEndian(buffer, &bone->local_z_axis, status);
        }
        else {
            bone->local_x_axis.values[0] = bone->local_z_axis.values[2] = 1.0f;
        }
        if (bone->u.flags.has_external_parent_bone) {
            bone->global_bone_index = nanoemBufferReadInt32LittleEndian(buffer, status);
        }
        if (bone->u.flags.has_constraint) {
            bone->constraint = constraint = nanoemModelConstraintCreate(parent_model, status);
            nanoemModelConstraintParsePMX(constraint, bone, buffer, status);
            if (nanoem_status_ptr_has_error(status)) {
                return;
            }
        }
        nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_BONE_CORRUPTED);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemModelBoneDestroy(nanoem_model_bone_t *bone)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(bone)) {
        parent_model = bone->base.parent.model;
        nanoemModelObjectDestroy(&bone->base);
        nanoemModelConstraintDestroy(bone->constraint);
        if (nanoem_is_not_null(parent_model)) {
            factory = parent_model->factory;
            nanoemUtilDestroyString(bone->name_ja, factory);
            nanoemUtilDestroyString(bone->name_en, factory);
        }
        nanoem_free(bone);
    }
}

nanoem_model_constraint_joint_t *
nanoemModelConstraintJointCreate(const nanoem_model_constraint_t *constraint, nanoem_status_t *status)
{
    nanoem_model_constraint_joint_t *joint;
    joint = (nanoem_model_constraint_joint_t *) nanoem_calloc(1, sizeof(*joint), status);
    if (nanoem_is_not_null(joint)) {
        joint->base.parent.constraint = constraint;
        joint->bone_index = NANOEM_MODEL_OBJECT_NOT_FOUND;
    }
    return joint;
}

void
nanoemModelConstraintJointDestroy(nanoem_model_constraint_joint_t *joint)
{
    if (nanoem_is_not_null(joint)) {
        nanoem_free(joint);
    }
}

nanoem_model_constraint_t *
nanoemModelConstraintCreate(const nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_constraint_t *constraint;
    constraint = (nanoem_model_constraint_t *) nanoem_calloc(1, sizeof(*constraint), status);
    if (nanoem_is_not_null(constraint)) {
        nanoemModelObjectInitialize(&constraint->base, model);
        constraint->effector_bone_index = -1;
        constraint->target_bone_index = -1;
    }
    return constraint;
}

void
nanoemModelConstraintParsePMD(nanoem_model_constraint_t *constraint, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_constraint_joint_t *joint;
    nanoem_rsize_t num_constraint_joints, i;
    if (nanoem_is_not_null(constraint) && nanoem_is_not_null(buffer)) {
        constraint->target_bone_index = nanoemBufferReadInt16LittleEndian(buffer, status);
        constraint->effector_bone_index = nanoemBufferReadInt16LittleEndian(buffer, status);
        num_constraint_joints = nanoemBufferReadByte(buffer, status);
        constraint->num_iterations = nanoemBufferReadInt16LittleEndianUnsigned(buffer, status);
        constraint->angle_limit = nanoemBufferReadFloat32LittleEndian(buffer, status);
        if (num_constraint_joints > 0) {
            constraint->joints = (nanoem_model_constraint_joint_t **) nanoem_calloc(num_constraint_joints, sizeof(*constraint->joints), status);
            if (nanoem_is_not_null(constraint->joints)) {
                constraint->num_joints = num_constraint_joints;
                for (i = 0; i < num_constraint_joints; i++) {
                    joint = nanoemModelConstraintJointCreate(constraint, status);
                    if (nanoem_is_not_null(joint)) {
                        joint->bone_index = nanoemBufferReadInt16LittleEndian(buffer, status);
                        constraint->joints[i] = joint;
                    }
                    else {
                        constraint->num_joints = i;
                        break;
                    }
                }
                if (i == num_constraint_joints) {
                    nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_CORRUPTED);
                }
            }
        }
        else {
            nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_CORRUPTED);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemModelConstraintParsePMX(nanoem_model_constraint_t *constraint, nanoem_model_bone_t *bone, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_constraint_joint_t *joint;
    nanoem_rsize_t bone_index_size, num_joints, i;
    if (nanoem_is_not_null(constraint) && nanoem_is_not_null(bone) && nanoem_is_not_null(buffer)) {
        parent_model = nanoemModelConstraintGetParentModel(constraint);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
            return;
        }
        bone_index_size = parent_model->info.bone_index_size;
        constraint->effector_bone_index = nanoemBufferReadIntegerNullable(buffer, bone_index_size, status);
        constraint->num_iterations = nanoemBufferReadInt32LittleEndian(buffer, status);
        constraint->angle_limit = nanoemBufferReadFloat32LittleEndian(buffer, status);
        num_joints = nanoemBufferReadLength(buffer, status);
        if (num_joints > 0) {
            constraint->joints = (nanoem_model_constraint_joint_t **)nanoem_calloc(num_joints, sizeof(*constraint->joints), status);
            if (nanoem_is_not_null(constraint->joints)) {
                constraint->num_joints = num_joints;
                for (i = 0; i < num_joints; i++) {
                    joint = nanoemModelConstraintJointCreate(constraint, status);
                    if (nanoem_is_not_null(joint)) {
                        joint->bone_index = nanoemBufferReadIntegerNullable(buffer, bone_index_size, status);
                        joint->has_angle_limit = nanoemBufferReadByte(buffer, status) != 0;
                        if (joint->has_angle_limit) {
                            nanoemBufferReadFloat32x3LittleEndian(buffer, &joint->lower_limit, status);
                            nanoemBufferReadFloat32x3LittleEndian(buffer, &joint->upper_limit, status);
                        }
                        constraint->joints[i] = joint;
                    }
                    else {
                        constraint->num_joints = i;
                        break;
                    }
                }
                if (i == num_joints) {
                    nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_CORRUPTED);
                }
            }
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemModelConstraintDestroy(nanoem_model_constraint_t *constraint)
{
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(constraint)) {
        nanoemModelObjectDestroy(&constraint->base);
        if (nanoem_is_not_null(constraint->joints)) {
            num_objects = constraint->num_joints;
            for (i = 0; i < num_objects; i++) {
                nanoemModelConstraintJointDestroy(constraint->joints[num_objects - i - 1]);
            }
            nanoem_free(constraint->joints);
        }
        nanoem_free(constraint);
    }
}

nanoem_model_texture_t *
nanoemModelTextureCreate(const nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_texture_t *texture;
    texture = (nanoem_model_texture_t *) nanoem_calloc(1, sizeof(*texture), status);
    if (nanoem_is_not_null(texture)) {
        nanoemModelObjectInitialize(&texture->base, model);
    }
    return texture;
}

void
nanoemModelTextureParsePMD(nanoem_model_texture_t *texture, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(texture) && nanoem_is_not_null(buffer)) {
        factory = texture->base.parent.model->factory;
        texture->path = nanoemBufferGetStringFromCp932(buffer, PMD_TOON_TEXTURE_PATH_LENGTH, factory, status);
        nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_TEXTURE_CORRUPTED);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemModelTextureParsePMX(nanoem_model_texture_t *texture, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    if (nanoem_is_not_null(texture) && nanoem_is_not_null(buffer)) {
        texture->path = nanoemModelGetStringPMX(nanoemModelTextureGetParentModel(texture), buffer, status);
        nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_TEXTURE_CORRUPTED);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemModelTextureDestroy(nanoem_model_texture_t *texture)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(texture)) {
        parent_model = texture->base.parent.model;
        nanoemModelObjectDestroy(&texture->base);
        if (nanoem_is_not_null(parent_model)) {
            factory = parent_model->factory;
            nanoemUtilDestroyString(texture->path, factory);
        }
        nanoem_free(texture);
    }
}

nanoem_model_morph_bone_t *
nanoemModelMorphBoneCreate(const nanoem_model_morph_t *parent, nanoem_status_t *status)
{
    nanoem_model_morph_bone_t *morph;
    morph = (nanoem_model_morph_bone_t *) nanoem_calloc(1, sizeof(*morph), status);
    if (nanoem_is_not_null(morph)) {
        morph->base.parent.morph = parent;
    }
    return morph;
}

void
nanoemModelMorphBoneDestroy(nanoem_model_morph_bone_t *morph)
{
    if (nanoem_is_not_null(morph)) {
        nanoem_free(morph);
    }
}

nanoem_model_morph_flip_t *
nanoemModelMorphFlipCreate(const nanoem_model_morph_t *parent, nanoem_status_t *status)
{
    nanoem_model_morph_flip_t *morph;
    morph = (nanoem_model_morph_flip_t *) nanoem_calloc(1, sizeof(*morph), status);
    if (nanoem_is_not_null(morph)) {
        morph->base.parent.morph = parent;
    }
    return morph;
}

void
nanoemModelMorphFlipDestroy(nanoem_model_morph_flip_t *morph)
{
    if (nanoem_is_not_null(morph)) {
        nanoem_free(morph);
    }
}

nanoem_model_morph_group_t *
nanoemModelMorphGroupCreate(const nanoem_model_morph_t *parent, nanoem_status_t *status)
{
    nanoem_model_morph_group_t *morph;
    morph = (nanoem_model_morph_group_t *) nanoem_calloc(1, sizeof(*morph), status);
    if (nanoem_is_not_null(morph)) {
        morph->base.parent.morph = parent;
    }
    return morph;
}

void
nanoemModelMorphGroupDestroy(nanoem_model_morph_group_t *morph)
{
    if (nanoem_is_not_null(morph)) {
        nanoem_free(morph);
    }
}

nanoem_model_morph_impulse_t *
nanoemModelMorphImpulseCreate(const nanoem_model_morph_t *parent, nanoem_status_t *status)
{
    nanoem_model_morph_impulse_t *morph;
    morph = (nanoem_model_morph_impulse_t *) nanoem_calloc(1, sizeof(*morph), status);
    if (nanoem_is_not_null(morph)) {
        morph->base.parent.morph = parent;
    }
    return morph;
}

void
nanoemModelMorphImpulseDestroy(nanoem_model_morph_impulse_t *morph)
{
    if (nanoem_is_not_null(morph)) {
        nanoem_free(morph);
    }
}

nanoem_model_morph_material_t *
nanoemModelMorphMaterialCreate(const nanoem_model_morph_t *parent, nanoem_status_t *status)
{
    nanoem_model_morph_material_t *morph;
    morph = (nanoem_model_morph_material_t *) nanoem_calloc(1, sizeof(*morph), status);
    if (nanoem_is_not_null(morph)) {
        morph->base.parent.morph = parent;
        morph->operation = NANOEM_MODEL_MORPH_MATERIAL_OPERATION_TYPE_UNKNOWN;
    }
    return morph;
}

void
nanoemModelMorphMaterialDestroy(nanoem_model_morph_material_t *morph)
{
    if (nanoem_is_not_null(morph)) {
        nanoem_free(morph);
    }
}

nanoem_model_morph_uv_t *
nanoemModelMorphUVCreate(const nanoem_model_morph_t *parent, nanoem_status_t *status)
{
    nanoem_model_morph_uv_t *morph;
    morph = (nanoem_model_morph_uv_t *) nanoem_calloc(1, sizeof(*morph), status);
    if (nanoem_is_not_null(morph)) {
        morph->base.parent.morph = parent;
    }
    return morph;
}

void
nanoemModelMorphUVDestroy(nanoem_model_morph_uv_t *morph)
{
    if (nanoem_is_not_null(morph)) {
        nanoem_free(morph);
    }
}

nanoem_model_morph_vertex_t *
nanoemModelMorphVertexCreate(const nanoem_model_morph_t *parent, nanoem_status_t *status)
{
    nanoem_model_morph_vertex_t *morph;
    morph = (nanoem_model_morph_vertex_t *) nanoem_calloc(1, sizeof(*morph), status);
    if (nanoem_is_not_null(morph)) {
        morph->base.parent.morph = parent;
        morph->relative_index = NANOEM_MODEL_OBJECT_NOT_FOUND;
    }
    return morph;
}

void
nanoemModelMorphVertexDestroy(nanoem_model_morph_vertex_t *morph)
{
    if (nanoem_is_not_null(morph)) {
        nanoem_free(morph);
    }
}

nanoem_model_morph_t *
nanoemModelMorphCreate(const nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_morph_t *morph;
    morph = (nanoem_model_morph_t *) nanoem_calloc(1, sizeof(*morph), status);
    if (nanoem_is_not_null(morph)) {
        nanoemModelObjectInitialize(&morph->base, model);
        morph->category = NANOEM_MODEL_MORPH_CATEGORY_UNKNOWN;
        morph->type = NANOEM_MODEL_MORPH_TYPE_UNKNOWN;
    }
    return morph;
}

void
nanoemModelMorphParsePMD(nanoem_model_morph_t *morph, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    nanoem_model_morph_vertex_t *vertex;
    nanoem_rsize_t num_vertices, i;
    int category;
    parent_model = nanoemModelMorphGetParentModel(morph);
    if (nanoem_is_null(parent_model)) {
        nanoem_status_ptr_assign_null_object(status);
        return;
    }
    factory = parent_model->factory;
    morph->name_ja = nanoemBufferGetStringFromCp932(buffer, PMD_MORPH_NAME_LENGTH, factory, status);
    num_vertices = nanoemBufferReadLength(buffer, status);
    category = nanoemBufferReadByte(buffer, status);
    switch (category) {
    case NANOEM_MODEL_MORPH_CATEGORY_BASE:
    case NANOEM_MODEL_MORPH_CATEGORY_EYE:
    case NANOEM_MODEL_MORPH_CATEGORY_EYEBROW:
    case NANOEM_MODEL_MORPH_CATEGORY_LIP:
    case NANOEM_MODEL_MORPH_CATEGORY_OTHER:
        morph->category = (nanoem_model_morph_category_t) category;
        break;
    default:
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_MORPH_CORRUPTED);
        break;
    }
    morph->type = NANOEM_MODEL_MORPH_TYPE_VERTEX;
    if (num_vertices > 0) {
        morph->u.vertices = (nanoem_model_morph_vertex_t **) nanoem_calloc(num_vertices, sizeof(*morph->u.vertices), status);
        if (nanoem_is_not_null(morph->u.vertices)) {
            morph->num_objects = num_vertices;
            for (i = 0; i < num_vertices; i++) {
                vertex = nanoemModelMorphVertexCreate(morph, status);
                if (nanoem_is_not_null(vertex)) {
                    vertex->vertex_index = nanoemBufferReadInt32LittleEndian(buffer, status);
                    nanoemBufferReadFloat32x3LittleEndian(buffer, &vertex->position, status);
                    morph->u.vertices[i] = vertex;
                }
                else {
                    morph->num_objects = i;
                    break;
                }
            }
            if (i == num_vertices) {
                nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_MORPH_CORRUPTED);
            }
        }
    }
    else {
        nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_MORPH_CORRUPTED);
    }
}

static void
nanoemModelMorphParseBonePMX(nanoem_model_morph_t *morph, nanoem_buffer_t *buffer, nanoem_rsize_t num_objects, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_bone_t *item;
    nanoem_rsize_t bone_index_size, i;
    if (num_objects > 0) {
        morph->u.bones = (nanoem_model_morph_bone_t **) nanoem_calloc(num_objects, sizeof(*morph->u.bones), status);
        if (nanoem_is_not_null(morph->u.bones)) {
            parent_model = nanoemModelMorphGetParentModel(morph);
            if (nanoem_is_null(parent_model)) {
                nanoem_status_ptr_assign_null_object(status);
                return;
            }
            bone_index_size = parent_model->info.bone_index_size;
            morph->num_objects = num_objects;
            for (i = 0; i < num_objects; i++) {
                item = nanoemModelMorphBoneCreate(morph, status);
                if (nanoem_is_not_null(item)) {
                    item->bone_index = nanoemBufferReadIntegerNullable(buffer, bone_index_size, status);
                    nanoemBufferReadFloat32x3LittleEndian(buffer, &item->translation, status);
                    nanoemBufferReadFloat32x4LittleEndian(buffer, &item->orientation, status);
                    morph->u.bones[i] = item;
                }
                else {
                    morph->num_objects = i;
                    break;
                }
            }
            if (i == num_objects) {
                nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_MORPH_CORRUPTED);
            }
        }
    }
}

static void
nanoemModelMorphDestroyBone(nanoem_model_morph_t *morph)
{
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(morph->u.bones)) {
        num_objects = morph->num_objects;
        for (i = 0; i < num_objects; i++) {
            nanoemModelMorphBoneDestroy(morph->u.bones[num_objects - i - 1]);
        }
        nanoem_free(morph->u.bones);
    }
}

static void
nanoemModelMorphParseFlipPMX(nanoem_model_morph_t *morph, nanoem_buffer_t *buffer, nanoem_rsize_t num_objects, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_flip_t *item;
    nanoem_rsize_t morph_index_size, i;
    if (num_objects > 0) {
        morph->u.flips = (nanoem_model_morph_flip_t **) nanoem_calloc(num_objects, sizeof(*morph->u.flips), status);
        if (nanoem_is_not_null(morph->u.flips)) {
            parent_model = nanoemModelMorphGetParentModel(morph);
            if (nanoem_is_null(parent_model)) {
                nanoem_status_ptr_assign_null_object(status);
                return;
            }
            morph_index_size = parent_model->info.morph_index_size;
            morph->num_objects = num_objects;
            for (i = 0; i < num_objects; i++) {
                item = nanoemModelMorphFlipCreate(morph, status);
                if (nanoem_is_not_null(item)) {
                    item->morph_index = nanoemBufferReadIntegerNullable(buffer, morph_index_size, status);
                    item->weight = nanoemBufferReadFloat32LittleEndian(buffer, status);
                    morph->u.flips[i] = item;
                }
                else {
                    morph->num_objects = i;
                    break;
                }
            }
            if (i == num_objects) {
                nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_MORPH_CORRUPTED);
            }
        }
    }
}

static void
nanoemModelMorphDestroyFlip(nanoem_model_morph_t *morph)
{
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(morph->u.flips)) {
        num_objects = morph->num_objects;
        for (i = 0; i < num_objects; i++) {
            nanoemModelMorphFlipDestroy(morph->u.flips[num_objects - i - 1]);
        }
        nanoem_free(morph->u.flips);
    }
}

static void
nanoemModelMorphParseGroupPMX(nanoem_model_morph_t *morph, nanoem_buffer_t *buffer, nanoem_rsize_t num_objects, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_group_t *item;
    nanoem_rsize_t morph_index_size, i;
    if (num_objects > 0) {
        morph->u.groups = (nanoem_model_morph_group_t **) nanoem_calloc(num_objects, sizeof(*morph->u.groups), status);
        if (nanoem_is_not_null(morph->u.groups)) {
            parent_model = nanoemModelMorphGetParentModel(morph);
            if (nanoem_is_null(parent_model)) {
                nanoem_status_ptr_assign_null_object(status);
                return;
            }
            morph_index_size = parent_model->info.morph_index_size;
            morph->num_objects = num_objects;
            for (i = 0; i < num_objects; i++) {
                item = nanoemModelMorphGroupCreate(morph, status);
                if (nanoem_is_not_null(item)) {
                    item->morph_index = nanoemBufferReadIntegerNullable(buffer, morph_index_size, status);
                    item->weight = nanoemBufferReadFloat32LittleEndian(buffer, status);
                    morph->u.groups[i] = item;
                }
                else {
                    morph->num_objects = i;
                    break;
                }
            }
            if (i == num_objects) {
                nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_MORPH_CORRUPTED);
            }
        }
    }
}

static void
nanoemModelMorphDestroyGroup(nanoem_model_morph_t *morph)
{
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(morph->u.groups)) {
        num_objects = morph->num_objects;
        for (i = 0; i < num_objects; i++) {
            nanoemModelMorphGroupDestroy(morph->u.groups[num_objects - i - 1]);
        }
        nanoem_free(morph->u.groups);
    }
}

static void
nanoemModelMorphParseImpulsePMX(nanoem_model_morph_t *morph, nanoem_buffer_t *buffer, nanoem_rsize_t num_objects, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_impulse_t *item;
    nanoem_rsize_t rigid_body_index_size, i;
    if (num_objects > 0) {
        morph->u.impulses = (nanoem_model_morph_impulse_t **) nanoem_calloc(num_objects, sizeof(*morph->u.impulses), status);
        if (nanoem_is_not_null(morph->u.impulses)) {
            parent_model = nanoemModelMorphGetParentModel(morph);
            if (nanoem_is_null(parent_model)) {
                nanoem_status_ptr_assign_null_object(status);
                return;
            }
            rigid_body_index_size = parent_model->info.rigid_body_index_size;
            morph->num_objects = num_objects;
            for (i = 0; i < num_objects; i++) {
                item = nanoemModelMorphImpulseCreate(morph, status);
                if (nanoem_is_not_null(item)) {
                    item->rigid_body_index = nanoemBufferReadIntegerNullable(buffer, rigid_body_index_size, status);
                    item->is_local = nanoemBufferReadByte(buffer, status);
                    nanoemBufferReadFloat32x3LittleEndian(buffer, &item->velocity, status);
                    nanoemBufferReadFloat32x3LittleEndian(buffer, &item->torque, status);
                    morph->u.impulses[i] = item;
                }
                else {
                    morph->num_objects = i;
                    break;
                }
            }
            if (i == num_objects) {
                nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_MORPH_CORRUPTED);
            }
        }
    }
}

static void
nanoemModelMorphDestroyImpulse(nanoem_model_morph_t *morph)
{
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(morph->u.impulses)) {
        num_objects = morph->num_objects;
        for (i = 0; i < num_objects; i++) {
            nanoemModelMorphImpulseDestroy(morph->u.impulses[num_objects - i - 1]);
        }
        nanoem_free(morph->u.impulses);
    }
}

static void
nanoemModelMorphParseMaterialPMX(nanoem_model_morph_t *morph, nanoem_buffer_t *buffer, nanoem_rsize_t num_objects, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_material_t *item;
    nanoem_rsize_t material_index_size, operation, i;
    if (num_objects > 0) {
        morph->u.materials = (nanoem_model_morph_material_t **) nanoem_calloc(num_objects, sizeof(*morph->u.materials), status);
        if (nanoem_is_not_null(morph->u.materials)) {
            parent_model = nanoemModelMorphGetParentModel(morph);
            if (nanoem_is_null(parent_model)) {
                nanoem_status_ptr_assign_null_object(status);
                return;
            }
            material_index_size = parent_model->info.material_index_size;
            morph->num_objects = num_objects;
            for (i = 0; i < num_objects; i++) {
                item = nanoemModelMorphMaterialCreate(morph, status);
                if (nanoem_is_not_null(item)) {
                    item->material_index = nanoemBufferReadIntegerNullable(buffer, material_index_size, status);
                    operation = nanoemBufferReadByte(buffer, status);
                    item->operation = (nanoem_model_morph_material_operation_type_t) operation;
                    nanoemBufferReadFloat32x3LittleEndian(buffer, &item->diffuse_color, status);
                    item->diffuse_opacity = nanoemBufferReadFloat32LittleEndian(buffer, status);
                    nanoemBufferReadFloat32x3LittleEndian(buffer, &item->specular_color, status);
                    item->specular_power = nanoemBufferReadFloat32LittleEndian(buffer, status);
                    nanoemBufferReadFloat32x3LittleEndian(buffer, &item->ambient_color, status);
                    nanoemBufferReadFloat32x3LittleEndian(buffer, &item->edge_color, status);
                    item->edge_opacity = nanoemBufferReadFloat32LittleEndian(buffer, status);
                    item->edge_size = nanoemBufferReadFloat32LittleEndian(buffer, status);
                    nanoemBufferReadFloat32x4LittleEndian(buffer, &item->diffuse_texture_blend, status);
                    nanoemBufferReadFloat32x4LittleEndian(buffer, &item->sphere_map_texture_blend, status);
                    nanoemBufferReadFloat32x4LittleEndian(buffer, &item->toon_texture_blend, status);
                    morph->u.materials[i] = item;
                }
                else {
                    morph->num_objects = i;
                    break;
                }
            }
            if (i == num_objects) {
                nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_MORPH_CORRUPTED);
            }
        }
    }
}

static void
nanoemModelMorphDestroyMaterial(nanoem_model_morph_t *morph)
{
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(morph->u.materials)) {
        num_objects = morph->num_objects;
        for (i = 0; i < num_objects; i++) {
            nanoemModelMorphMaterialDestroy(morph->u.materials[num_objects - i - 1]);
        }
        nanoem_free(morph->u.materials);
    }
}

static void
nanoemModelMorphParseUVPMX(nanoem_model_morph_t *morph, nanoem_buffer_t *buffer, nanoem_rsize_t num_objects, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_uv_t *item;
    nanoem_rsize_t vertex_index_size, i;
    if (num_objects > 0) {
        morph->u.uvs = (nanoem_model_morph_uv_t **) nanoem_calloc(num_objects, sizeof(*morph->u.uvs), status);
        if (nanoem_is_not_null(morph->u.uvs)) {
            parent_model = nanoemModelMorphGetParentModel(morph);
            if (nanoem_is_null(parent_model)) {
                nanoem_status_ptr_assign_null_object(status);
                return;
            }
            vertex_index_size = parent_model->info.vertex_index_size;
            morph->num_objects = num_objects;
            for (i = 0; i < num_objects; i++) {
                item = nanoemModelMorphUVCreate(morph, status);
                if (nanoem_is_not_null(item)) {
                    item->vertex_index = nanoemBufferReadInteger(buffer, vertex_index_size, status);
                    nanoemBufferReadFloat32x4LittleEndian(buffer, &item->position, status);
                    morph->u.uvs[i] = item;
                }
                else {
                    morph->num_objects = i;
                    break;
                }
            }
            if (i == num_objects) {
                nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_MORPH_CORRUPTED);
            }
        }
    }
}

static void
nanoemModelMorphDestroyUV(nanoem_model_morph_t *morph)
{
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(morph->u.uvs)) {
        num_objects = morph->num_objects;
        for (i = 0; i < num_objects; i++) {
            nanoemModelMorphUVDestroy(morph->u.uvs[num_objects - i - 1]);
        }
        nanoem_free(morph->u.uvs);
    }
}

static void
nanoemModelMorphParseVertexPMX(nanoem_model_morph_t *morph, nanoem_buffer_t *buffer, nanoem_rsize_t num_objects, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_vertex_t *item;
    nanoem_rsize_t vertex_index_size, i;
    if (num_objects > 0) {
        morph->u.vertices = (nanoem_model_morph_vertex_t **) nanoem_calloc(num_objects, sizeof(*morph->u.vertices), status);
        if (nanoem_is_not_null(morph->u.vertices)) {
            parent_model = nanoemModelMorphGetParentModel(morph);
            if (nanoem_is_null(parent_model)) {
                nanoem_status_ptr_assign_null_object(status);
                return;
            }
            vertex_index_size = parent_model->info.vertex_index_size;
            morph->num_objects = num_objects;
            for (i = 0; i < num_objects; i++) {
                item = nanoemModelMorphVertexCreate(morph, status);
                if (nanoem_is_not_null(item)) {
                    item->vertex_index = nanoemBufferReadInteger(buffer, vertex_index_size, status);
                    nanoemBufferReadFloat32x3LittleEndian(buffer, &item->position, status);
                    morph->u.vertices[i] = item;
                }
                else {
                    morph->num_objects = i;
                    break;
                }
            }
            if (i == num_objects) {
                nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_MORPH_CORRUPTED);
            }
        }
    }
}

static void
nanoemModelMorphDestroyVertex(nanoem_model_morph_t *morph)
{
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(morph->u.vertices)) {
        num_objects = morph->num_objects;
        for (i = 0; i < num_objects; i++) {
            nanoemModelMorphVertexDestroy(morph->u.vertices[num_objects - i - 1]);
        }
        nanoem_free(morph->u.vertices);
    }
}

void
nanoemModelMorphParsePMX(nanoem_model_morph_t *morph, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_rsize_t num_objects;
    int category, type;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(buffer)) {
        parent_model = nanoemModelMorphGetParentModel(morph);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
            return;
        }
        morph->name_ja = nanoemModelGetStringPMX(parent_model, buffer, status);
        morph->name_en = nanoemModelGetStringPMX(parent_model, buffer, status);
        category = nanoemBufferReadByte(buffer, status);
        switch (category) {
        case NANOEM_MODEL_MORPH_CATEGORY_BASE:
        case NANOEM_MODEL_MORPH_CATEGORY_EYE:
        case NANOEM_MODEL_MORPH_CATEGORY_EYEBROW:
        case NANOEM_MODEL_MORPH_CATEGORY_LIP:
        case NANOEM_MODEL_MORPH_CATEGORY_OTHER:
            morph->category = (nanoem_model_morph_category_t) category;
            break;
        default:
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_MORPH_CORRUPTED);
            break;
        }
        type = nanoemBufferReadByte(buffer, status);
        num_objects = nanoemBufferReadLength(buffer, status);
        if (!nanoem_status_ptr_has_error(status)) {
            switch (type) {
            case NANOEM_MODEL_MORPH_TYPE_BONE:
                nanoemModelMorphParseBonePMX(morph, buffer, num_objects, status);
                morph->type = (nanoem_model_morph_type_t) type;
                break;
            case NANOEM_MODEL_MORPH_TYPE_FLIP:
                nanoemModelMorphParseFlipPMX(morph, buffer, num_objects, status);
                morph->type = (nanoem_model_morph_type_t) type;
                break;
            case NANOEM_MODEL_MORPH_TYPE_GROUP:
                nanoemModelMorphParseGroupPMX(morph, buffer, num_objects, status);
                morph->type = (nanoem_model_morph_type_t) type;
                break;
            case NANOEM_MODEL_MORPH_TYPE_IMPULUSE:
                nanoemModelMorphParseImpulsePMX(morph, buffer, num_objects, status);
                morph->type = (nanoem_model_morph_type_t) type;
                break;
            case NANOEM_MODEL_MORPH_TYPE_MATERIAL:
                nanoemModelMorphParseMaterialPMX(morph, buffer, num_objects, status);
                morph->type = (nanoem_model_morph_type_t) type;
                break;
            case NANOEM_MODEL_MORPH_TYPE_TEXTURE:
            case NANOEM_MODEL_MORPH_TYPE_UVA1:
            case NANOEM_MODEL_MORPH_TYPE_UVA2:
            case NANOEM_MODEL_MORPH_TYPE_UVA3:
            case NANOEM_MODEL_MORPH_TYPE_UVA4:
                nanoemModelMorphParseUVPMX(morph, buffer, num_objects, status);
                morph->type = (nanoem_model_morph_type_t) type;
                break;
            case NANOEM_MODEL_MORPH_TYPE_VERTEX:
                nanoemModelMorphParseVertexPMX(morph, buffer, num_objects, status);
                morph->type = (nanoem_model_morph_type_t) type;
                break;
            case NANOEM_MODEL_MORPH_TYPE_MAX_ENUM:
            case NANOEM_MODEL_MORPH_TYPE_UNKNOWN:
            default:
                nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_MORPH_CORRUPTED);
                break;
            }
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_MORPH_CORRUPTED);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemModelMorphDestroy(nanoem_model_morph_t *morph)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(morph)) {
        switch (morph->type) {
        case NANOEM_MODEL_MORPH_TYPE_BONE:
            nanoemModelMorphDestroyBone(morph);
            break;
        case NANOEM_MODEL_MORPH_TYPE_FLIP:
            nanoemModelMorphDestroyFlip(morph);
            break;
        case NANOEM_MODEL_MORPH_TYPE_GROUP:
            nanoemModelMorphDestroyGroup(morph);
            break;
        case NANOEM_MODEL_MORPH_TYPE_IMPULUSE:
            nanoemModelMorphDestroyImpulse(morph);
            break;
        case NANOEM_MODEL_MORPH_TYPE_MATERIAL:
            nanoemModelMorphDestroyMaterial(morph);
            break;
        case NANOEM_MODEL_MORPH_TYPE_TEXTURE:
        case NANOEM_MODEL_MORPH_TYPE_UVA1:
        case NANOEM_MODEL_MORPH_TYPE_UVA2:
        case NANOEM_MODEL_MORPH_TYPE_UVA3:
        case NANOEM_MODEL_MORPH_TYPE_UVA4:
            nanoemModelMorphDestroyUV(morph);
            break;
        case NANOEM_MODEL_MORPH_TYPE_VERTEX:
            nanoemModelMorphDestroyVertex(morph);
            break;
        case NANOEM_MODEL_MORPH_TYPE_MAX_ENUM:
        case NANOEM_MODEL_MORPH_TYPE_UNKNOWN:
        default:
            break;
        }
        parent_model = morph->base.parent.model;
        if (nanoem_is_not_null(parent_model)) {
            factory = morph->base.parent.model->factory;
            nanoemUtilDestroyString(morph->name_ja, factory);
            nanoemUtilDestroyString(morph->name_en, factory);
        }
        nanoemModelObjectDestroy(&morph->base);
        nanoem_free(morph);
    }
}

nanoem_model_label_item_t *
nanoemModelLabelItemCreate(const nanoem_model_label_t *parent, nanoem_status_t *status)
{
    nanoem_model_label_item_t *item = NULL;
    if (nanoem_is_not_null(parent)) {
        item = (nanoem_model_label_item_t *) nanoem_calloc(1, sizeof(*item), status);
        if (nanoem_is_not_null(item)) {
            item->base.parent.label = parent;
            item->type = NANOEM_MODEL_LABEL_ITEM_TYPE_UNKNOWN;
        }
    }
    return item;
}

void
nanoemModelLabelItemDestroy(nanoem_model_label_item_t *item)
{
    if (nanoem_is_not_null(item)) {
        nanoem_free(item);
    }
}

nanoem_model_label_t *
nanoemModelLabelCreate(const nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_label_t *label;
    label = (nanoem_model_label_t *) nanoem_calloc(1, sizeof(*label), status);
    if (nanoem_is_not_null(label)) {
        nanoemModelObjectInitialize(&label->base, model);
    }
    return label;
}

void
nanoemModelLabelParsePMX(nanoem_model_label_t *label, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_label_item_t *item, **items;
    nanoem_rsize_t bone_index_size, morph_index_size, num_items, i;
    int item_type;
    if (nanoem_is_not_null(label) && nanoem_is_not_null(buffer)) {
        parent_model = nanoemModelLabelGetParentModel(label);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
            return;
        }
        bone_index_size = parent_model->info.bone_index_size;
        morph_index_size = parent_model->info.morph_index_size;
        label->name_ja = nanoemModelGetStringPMX(parent_model, buffer, status);
        label->name_en = nanoemModelGetStringPMX(parent_model, buffer, status);
        label->is_special = nanoemBufferReadByte(buffer, status) != 0;
        num_items = nanoemBufferReadLength(buffer, status);
        if (num_items > 0) {
            label->items = items = (nanoem_model_label_item_t **) nanoem_calloc(num_items, sizeof(*label->items), status);
            if (nanoem_is_not_null(items)) {
                for (i = 0; i < num_items; i++) {
                    item = items[i] = nanoemModelLabelItemCreate(label, status);
                    if (nanoem_is_not_null(item)) {
                        item_type = nanoemBufferReadByte(buffer, status);
                        switch (item_type) {
                        case NANOEM_MODEL_LABEL_ITEM_TYPE_BONE:
                            item->u.bone = nanoemModelGetOneBoneObject(parent_model, nanoemBufferReadIntegerNullable(buffer, bone_index_size, status));
                            item->type = (nanoem_model_label_item_type_t) item_type;
                            break;
                        case NANOEM_MODEL_LABEL_ITEM_TYPE_MORPH:
                            item->u.morph = nanoemModelGetOneMorphObject(parent_model, nanoemBufferReadIntegerNullable(buffer, morph_index_size, status));
                            item->type = (nanoem_model_label_item_type_t) item_type;
                            break;
                        case NANOEM_MODEL_LABEL_ITEM_TYPE_MAX_ENUM:
                        case NANOEM_MODEL_LABEL_ITEM_TYPE_UNKNOWN:
                        default:
                            num_items = i + 1;
                            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_LABEL_CORRUPTED);
                            break;
                        }
                    }
                    else {
                        num_items = i;
                        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_LABEL_CORRUPTED);
                        break;
                    }
                }
                label->num_items = num_items;
                nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_LABEL_CORRUPTED);
            }
        }
        else {
            nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_LABEL_CORRUPTED);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemModelLabelDestroy(nanoem_model_label_t *label)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(label)) {
        parent_model = label->base.parent.model;
        if (nanoem_is_not_null(parent_model)) {
            factory = parent_model->factory;
            nanoemUtilDestroyString(label->name_ja, factory);
            nanoemUtilDestroyString(label->name_en, factory);
        }
        nanoemModelObjectDestroy(&label->base);
        if (nanoem_is_not_null(label->items)) {
            num_objects = label->num_items;
            for (i = 0; i < num_objects; i++) {
                nanoemModelLabelItemDestroy(label->items[num_objects - i - 1]);
            }
            nanoem_free(label->items);
        }
        nanoem_free(label);
    }
}

nanoem_model_rigid_body_t *
nanoemModelRigidBodyCreate(const nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_rigid_body_t *rigid_body;
    rigid_body = (nanoem_model_rigid_body_t *) nanoem_calloc(1, sizeof(*rigid_body), status);
    if (nanoem_is_not_null(rigid_body)) {
        nanoemModelObjectInitialize(&rigid_body->base, model);
        rigid_body->shape_type = NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_UNKNOWN;
        rigid_body->transform_type = NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_UNKNOWN;
    }
    return rigid_body;
}

static void
nanoemModelRigidBodyParseCommon(nanoem_model_rigid_body_t *rigid_body, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    int shape_type, transform_type;
    rigid_body->collision_group_id = nanoemBufferReadByte(buffer, status);
    rigid_body->collision_mask = nanoemBufferReadInt16LittleEndian(buffer, status);
    shape_type = nanoemBufferReadByte(buffer, status);
    switch (shape_type) {
    case NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_BOX:
    case NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_CAPSULE:
    case NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_SPHERE:
        rigid_body->shape_type = (nanoem_model_rigid_body_shape_type_t) shape_type;
        break;
    default:
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_CORRUPTED);
        break;
    }
    nanoemBufferReadFloat32x3LittleEndian(buffer, &rigid_body->size, status);
    nanoemBufferReadFloat32x3LittleEndian(buffer, &rigid_body->origin, status);
    nanoemBufferReadFloat32x3LittleEndian(buffer, &rigid_body->orientation, status);
    rigid_body->mass = nanoemBufferReadFloat32LittleEndian(buffer, status);
    rigid_body->linear_damping = nanoemBufferReadFloat32LittleEndian(buffer, status);
    rigid_body->angular_damping = nanoemBufferReadFloat32LittleEndian(buffer, status);
    rigid_body->restitution = nanoemBufferReadFloat32LittleEndian(buffer, status);
    rigid_body->friction = nanoemBufferReadFloat32LittleEndian(buffer, status);
    transform_type = nanoemBufferReadByte(buffer, status);
    switch (transform_type) {
    case NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_ORIENTATION_AND_SIMULATION_TO_BONE:
    case NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_TO_SIMULATION:
    case NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_SIMULATION_TO_BONE:
        rigid_body->transform_type = (nanoem_model_rigid_body_transform_type_t) transform_type;
        break;
    default:
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_CORRUPTED);
        break;
    }
    nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_CORRUPTED);
}

void
nanoemModelRigidBodyParsePMD(nanoem_model_rigid_body_t *rigid_body, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(rigid_body) && nanoem_is_not_null(buffer)) {
        parent_model = nanoemModelRigidBodyGetParentModel(rigid_body);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
            return;
        }
        factory = parent_model->factory;
        rigid_body->name_ja = nanoemBufferGetStringFromCp932(buffer, PMD_RIGID_BODY_NAME_LENGTH, factory, status);
        rigid_body->bone_index = nanoemBufferReadInt16LittleEndian(buffer, status);
        rigid_body->is_bone_relative = nanoem_true;
        nanoemModelRigidBodyParseCommon(rigid_body, buffer, status);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemModelRigidBodyParsePMX(nanoem_model_rigid_body_t *rigid_body, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    if (nanoem_is_not_null(rigid_body) && nanoem_is_not_null(buffer)) {
        parent_model = nanoemModelRigidBodyGetParentModel(rigid_body);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
            return;
        }
        rigid_body->name_ja = nanoemModelGetStringPMX(parent_model, buffer, status);
        rigid_body->name_en = nanoemModelGetStringPMX(parent_model, buffer, status);
        rigid_body->bone_index = nanoemBufferReadIntegerNullable(buffer, parent_model->info.bone_index_size, status);
        nanoemModelRigidBodyParseCommon(rigid_body, buffer, status);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemModelRigidBodyDestroy(nanoem_model_rigid_body_t *rigid_body)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(rigid_body)) {
        parent_model = rigid_body->base.parent.model;
        if (nanoem_is_not_null(parent_model)) {
            factory = parent_model->factory;
            nanoemUtilDestroyString(rigid_body->name_ja, factory);
            nanoemUtilDestroyString(rigid_body->name_en, factory);
        }
        nanoemModelObjectDestroy(&rigid_body->base);
        nanoem_free(rigid_body);
    }
}

nanoem_model_joint_t *
nanoemModelJointCreate(const nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_joint_t *joint;
    joint = (nanoem_model_joint_t *) nanoem_calloc(1, sizeof(*joint), status);
    if (nanoem_is_not_null(joint)) {
        nanoemModelObjectInitialize(&joint->base, model);
        joint->type = NANOEM_MODEL_JOINT_TYPE_UNKNOWN;
    }
    return joint;
}

static void
nanoemModelJointParseCommon(nanoem_model_joint_t *joint, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemBufferReadFloat32x3LittleEndian(buffer, &joint->origin, status);
    nanoemBufferReadFloat32x3LittleEndian(buffer, &joint->orientation, status);
    nanoemBufferReadFloat32x3LittleEndian(buffer, &joint->linear_lower_limit, status);
    nanoemBufferReadFloat32x3LittleEndian(buffer, &joint->linear_upper_limit, status);
    nanoemBufferReadFloat32x3LittleEndian(buffer, &joint->angular_lower_limit, status);
    nanoemBufferReadFloat32x3LittleEndian(buffer, &joint->angular_upper_limit, status);
    nanoemBufferReadFloat32x3LittleEndian(buffer, &joint->linear_stiffness, status);
    nanoemBufferReadFloat32x3LittleEndian(buffer, &joint->angular_stiffness, status);
    nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MODEL_JOINT_CORRUPTED);
}

void
nanoemModelJointParsePMD(nanoem_model_joint_t *joint, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(joint) && nanoem_is_not_null(buffer)) {
        parent_model = nanoemModelJointGetParentModel(joint);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
            return;
        }
        factory = parent_model->factory;
        joint->name_ja = nanoemBufferGetStringFromCp932(buffer, PMD_JOINT_NAME_LENGTH, factory, status);
        joint->rigid_body_a_index = nanoemBufferReadInt32LittleEndian(buffer, status);
        joint->rigid_body_b_index = nanoemBufferReadInt32LittleEndian(buffer, status);
        joint->type = NANOEM_MODEL_JOINT_TYPE_GENERIC_6DOF_SPRING_CONSTRAINT;
        nanoemModelJointParseCommon(joint, buffer, status);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemModelJointParsePMX(nanoem_model_joint_t *joint, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_rsize_t rigid_body_index_size;
    int joint_type;
    if (nanoem_is_not_null(joint) && nanoem_is_not_null(buffer)) {
        parent_model = nanoemModelJointGetParentModel(joint);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
            return;
        }
        rigid_body_index_size = parent_model->info.rigid_body_index_size;
        joint->name_ja = nanoemModelGetStringPMX(parent_model, buffer, status);
        joint->name_en = nanoemModelGetStringPMX(parent_model, buffer, status);
        joint_type = nanoemBufferReadByte(buffer, status);
        switch (joint_type) {
        case NANOEM_MODEL_JOINT_TYPE_CONE_TWIST_CONSTRAINT:
        case NANOEM_MODEL_JOINT_TYPE_GENERIC_6DOF_CONSTRAINT:
        case NANOEM_MODEL_JOINT_TYPE_GENERIC_6DOF_SPRING_CONSTRAINT:
        case NANOEM_MODEL_JOINT_TYPE_HINGE_CONSTRAINT:
        case NANOEM_MODEL_JOINT_TYPE_POINT2POINT_CONSTRAINT:
        case NANOEM_MODEL_JOINT_TYPE_SLIDER_CONSTRAINT:
            joint->type = (nanoem_model_joint_type_t) joint_type;
            break;
        default:
            break;
        }
        joint->rigid_body_a_index = nanoemBufferReadIntegerNullable(buffer, rigid_body_index_size, status);
        joint->rigid_body_b_index = nanoemBufferReadIntegerNullable(buffer, rigid_body_index_size, status);
        nanoemModelJointParseCommon(joint, buffer, status);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemModelJointDestroy(nanoem_model_joint_t *joint)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(joint)) {
        parent_model = joint->base.parent.model;
        if (nanoem_is_not_null(parent_model)) {
            factory = parent_model->factory;
            nanoemUtilDestroyString(joint->name_ja, factory);
            nanoemUtilDestroyString(joint->name_en, factory);
        }
        nanoemModelObjectDestroy(&joint->base);
        nanoem_free(joint);
    }
}

nanoem_model_soft_body_anchor_t *
nanoemModelSoftBodyAnchorCreate(const nanoem_model_soft_body_t *parent, nanoem_status_t *status)
{
    nanoem_model_soft_body_anchor_t *item = NULL;
    if (nanoem_is_not_null(parent)) {
        item = (nanoem_model_soft_body_anchor_t *) nanoem_calloc(1, sizeof(*item), status);
        if (nanoem_is_not_null(item)) {
            item->base.parent.soft_body = parent;
        }
    }
    return item;
}

nanoem_model_soft_body_t *
nanoemModelSoftBodyCreate(const nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_soft_body_t *soft_body;
    soft_body = (nanoem_model_soft_body_t *) nanoem_calloc(1, sizeof(*soft_body), status);
    if (nanoem_is_not_null(soft_body)) {
        nanoemModelObjectInitialize(&soft_body->base, model);
    }
    return soft_body;
}

void
nanoemModelSoftBodyParsePMX(nanoem_model_soft_body_t *soft_body, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_soft_body_anchor_t *anchor;
    nanoem_rsize_t material_index_size, rigid_body_index_size, vertex_index_size, num_anchors, num_pin_vertex_indices, i;
    int shape, aero_model;
    if (nanoem_is_not_null(soft_body) && nanoem_is_not_null(buffer)) {
        parent_model = nanoemModelSoftBodyGetParentModel(soft_body);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
            return;
        }
        material_index_size = parent_model->info.material_index_size;
        rigid_body_index_size = parent_model->info.rigid_body_index_size;
        vertex_index_size = parent_model->info.vertex_index_size;
        soft_body->name_ja = nanoemModelGetStringPMX(parent_model, buffer, status);
        soft_body->name_en = nanoemModelGetStringPMX(parent_model, buffer, status);
        shape = nanoemBufferReadByte(buffer, status);
        switch (shape) {
        case NANOEM_MODEL_SOFT_BODY_SHAPE_TYPE_TRI_MESH:
        case NANOEM_MODEL_SOFT_BODY_SHAPE_TYPE_ROPE:
            soft_body->shape_type = (nanoem_model_soft_body_shape_type_t) shape;
            break;
        default:
            soft_body->shape_type = NANOEM_MODEL_SOFT_BODY_SHAPE_TYPE_UNKNOWN;
            break;
        }
        soft_body->material_index = nanoemBufferReadIntegerNullable(buffer, material_index_size, status);
        soft_body->collision_group_id = nanoemBufferReadByte(buffer, status);
        soft_body->collision_mask = nanoemBufferReadInt16LittleEndianUnsigned(buffer, status);
        soft_body->flags = nanoemBufferReadByte(buffer, status);
        soft_body->bending_constraints_distance = nanoemBufferReadInt32LittleEndian(buffer, status);
        soft_body->cluster_count = nanoemBufferReadInt32LittleEndian(buffer, status);
        soft_body->total_mass = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->collision_margin = nanoemBufferReadFloat32LittleEndian(buffer, status);
        aero_model = nanoemBufferReadInt32LittleEndian(buffer, status);
        switch (aero_model) {
        case NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_VERTEX_POINT:
        case NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_VERTEX_TWO_SIDED:
        case NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_VERTEX_ONE_SIDED:
        case NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_FACE_TWO_SIDED:
        case NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_FACE_ONE_SIDED:
            soft_body->aero_model = (nanoem_model_soft_body_aero_model_type_t) aero_model;
            break;
        default:
            soft_body->aero_model = NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_UNKNOWN;
            break;
        }
        soft_body->velocity_correction_factor = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->damping_coefficient = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->drag_coefficient = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->lift_coefficient = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->pressure_coefficient = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->volume_conversation_coefficient = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->dynamic_friction_coefficient = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->pose_matching_coefficient = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->rigid_contact_hardness = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->kinetic_contact_hardness = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->soft_contact_hardness = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->anchor_hardness = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->soft_vs_rigid_hardness = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->soft_vs_kinetic_hardness = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->soft_vs_soft_hardness = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->soft_vs_rigid_impulse_split = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->soft_vs_kinetic_impulse_split = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->soft_vs_soft_impulse_split = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->velocity_solver_iterations = nanoemBufferReadInt32LittleEndian(buffer, status);
        soft_body->positions_solver_iterations = nanoemBufferReadInt32LittleEndian(buffer, status);
        soft_body->drift_solver_iterations = nanoemBufferReadInt32LittleEndian(buffer, status);
        soft_body->cluster_solver_iterations = nanoemBufferReadInt32LittleEndian(buffer, status);
        soft_body->linear_stiffness_coefficient = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->angular_stiffness_coefficient = nanoemBufferReadFloat32LittleEndian(buffer, status);
        soft_body->volume_stiffness_coefficient = nanoemBufferReadFloat32LittleEndian(buffer, status);
        num_anchors = nanoemBufferReadLength(buffer, status);
        if (num_anchors > 0) {
            soft_body->num_anchors = num_anchors;
            soft_body->anchors = (nanoem_model_soft_body_anchor_t **) nanoem_calloc(num_anchors, sizeof(*soft_body->anchors), status);
            if (nanoem_is_not_null(soft_body->anchors)) {
                for (i = 0; i < num_anchors; i++) {
                    anchor = nanoemModelSoftBodyAnchorCreate(soft_body, status);
                    if (nanoem_is_not_null(anchor)) {
                        anchor->rigid_body_index = nanoemBufferReadIntegerNullable(buffer, rigid_body_index_size, status);
                        anchor->vertex_index = nanoemBufferReadIntegerNullable(buffer, vertex_index_size, status);
                        anchor->is_near_enabled = nanoemBufferReadByte(buffer, status);
                        soft_body->anchors[i] = anchor;
                    }
                    else {
                        soft_body->num_anchors = i;
                        break;
                    }
                }
            }
        }
        num_pin_vertex_indices = nanoemBufferReadLength(buffer, status);
        if (num_pin_vertex_indices > 0) {
            soft_body->num_pinned_vertex_indices = num_pin_vertex_indices;
            soft_body->pinned_vertex_indices = (nanoem_u32_t *) nanoem_calloc(num_pin_vertex_indices, sizeof(*soft_body->pinned_vertex_indices), status);
            if (nanoem_is_not_null(soft_body->pinned_vertex_indices)) {
                for (i = 0; i < num_pin_vertex_indices; i++) {
                    soft_body->pinned_vertex_indices[i] = (nanoem_u32_t) nanoemBufferReadInteger(buffer, vertex_index_size, status);
                }
            }
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemModelSoftBodyAnchorDestroy(nanoem_model_soft_body_anchor_t *anchor)
{
    if (nanoem_is_not_null(anchor)) {
        nanoem_free(anchor);
    }
}

void
nanoemModelSoftBodyDestroy(nanoem_model_soft_body_t *soft_body)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(soft_body)) {
        parent_model = soft_body->base.parent.model;
        if (nanoem_is_not_null(parent_model)) {
            factory = parent_model->factory;
            nanoemUtilDestroyString(soft_body->name_ja, factory);
            nanoemUtilDestroyString(soft_body->name_en, factory);
        }
        if (nanoem_is_not_null(soft_body->anchors)) {
            num_objects = soft_body->num_anchors;
            for (i = 0; i < num_objects; i++) {
                nanoemModelSoftBodyAnchorDestroy(soft_body->anchors[num_objects - i - 1]);
            }
            nanoem_free(soft_body->anchors);
        }
        nanoemModelObjectDestroy(&soft_body->base);
        nanoem_free(soft_body->pinned_vertex_indices);
        nanoem_free(soft_body);
    }
}

const nanoem_model_t *APIENTRY
nanoemModelObjectGetParentModel(const nanoem_model_object_t *object)
{
    return nanoem_is_not_null(object) ? object->parent.model : NULL;
}

int APIENTRY
nanoemModelObjectGetIndex(const nanoem_model_object_t *object)
{
    return nanoem_is_not_null(object) ? object->index : NANOEM_MODEL_OBJECT_NOT_FOUND;
}

const nanoem_user_data_t *APIENTRY
nanoemModelObjectGetUserData(const nanoem_model_object_t *object)
{
    return nanoem_is_not_null(object) ? object->user_data : NULL;
}

void APIENTRY
nanoemModelObjectSetUserData(nanoem_model_object_t *object, nanoem_user_data_t *user_data)
{
    if (nanoem_is_not_null(object)) {
        object->user_data = user_data;
    }
}

const nanoem_f32_t *APIENTRY
nanoemModelVertexGetOrigin(const nanoem_model_vertex_t *vertex)
{
    return nanoem_is_not_null(vertex) ? vertex->origin.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelVertexGetNormal(const nanoem_model_vertex_t *vertex)
{
    return nanoem_is_not_null(vertex) ? vertex->normal.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelVertexGetTexCoord(const nanoem_model_vertex_t *vertex)
{
    return nanoem_is_not_null(vertex) ? vertex->uv.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelVertexGetAdditionalUV(const nanoem_model_vertex_t *vertex, nanoem_rsize_t index)
{
    if (nanoem_is_not_null(vertex) && index < 4) {
        return vertex->additional_uv[index].values;
    }
    return __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelVertexGetSdefC(const nanoem_model_vertex_t *vertex)
{
    return nanoem_is_not_null(vertex) ? vertex->sdef_c.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelVertexGetSdefR0(const nanoem_model_vertex_t *vertex)
{
    return nanoem_is_not_null(vertex) ? vertex->sdef_r0.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelVertexGetSdefR1(const nanoem_model_vertex_t *vertex)
{
    return nanoem_is_not_null(vertex) ? vertex->sdef_r1.values : __nanoem_null_vector4;
}

const nanoem_model_bone_t *APIENTRY
nanoemModelVertexGetBoneObject(const nanoem_model_vertex_t *vertex, nanoem_rsize_t index)
{
    nanoem_model_bone_t *bone = NULL;
    if (nanoem_is_not_null(vertex) && index < vertex->num_bone_indices) {
        bone = nanoemModelGetOneBoneObject(nanoemModelVertexGetParentModel(vertex), vertex->bone_indices[index]);
    }
    return bone;
}

nanoem_f32_t APIENTRY
nanoemModelVertexGetBoneWeight(const nanoem_model_vertex_t *vertex, nanoem_rsize_t index)
{
    nanoem_f32_t weight = 0;
    if (nanoem_is_not_null(vertex) && index < vertex->num_bone_weights) {
        weight = vertex->bone_weights.values[index];
    }
    return weight;
}

nanoem_f32_t APIENTRY
nanoemModelVertexGetEdgeSize(const nanoem_model_vertex_t *vertex)
{
    return nanoem_is_not_null(vertex) ? vertex->edge_size : 0;
}

nanoem_model_vertex_type_t APIENTRY
nanoemModelVertexGetType(const nanoem_model_vertex_t *vertex)
{
    return nanoem_is_not_null(vertex) ? vertex->type : NANOEM_MODEL_VERTEX_TYPE_UNKNOWN;
}

const nanoem_model_object_t *APIENTRY
nanoemModelVertexGetModelObject(const nanoem_model_vertex_t *vertex)
{
    return nanoem_is_not_null(vertex) ? &vertex->base : NULL;
}

nanoem_model_object_t *APIENTRY
nanoemModelVertexGetModelObjectMutable(nanoem_model_vertex_t *vertex)
{
    return nanoem_is_not_null(vertex) ? &vertex->base : NULL;
}

const nanoem_unicode_string_t *APIENTRY
nanoemModelMaterialGetName(const nanoem_model_material_t *material, nanoem_language_type_t language)
{
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(material)) {
        switch (language) {
        case NANOEM_LANGUAGE_TYPE_JAPANESE:
            name = material->name_ja;
            break;
        case NANOEM_LANGUAGE_TYPE_ENGLISH:
            name = material->name_en;
            break;
        case NANOEM_LANGUAGE_TYPE_MAX_ENUM:
        case NANOEM_LANGUAGE_TYPE_UNKNOWN:
        default:
            break;
        }
    }
    return name;
}

const nanoem_model_texture_t *APIENTRY
nanoemModelMaterialGetDiffuseTextureObject(const nanoem_model_material_t *material)
{
    const nanoem_model_texture_t *texture = NULL;
    int diffuse_texture_index;
    if (nanoem_is_not_null(material)) {
        diffuse_texture_index = material->diffuse_texture_index;
        if (diffuse_texture_index > NANOEM_MODEL_OBJECT_NOT_FOUND) {
            texture = nanoemModelGetOneTextureObject(nanoemModelMaterialGetParentModel(material), diffuse_texture_index);
        }
        else if (material->diffuse_texture) {
            texture = material->diffuse_texture;
        }
    }
    return texture;
}

const nanoem_model_texture_t *APIENTRY
nanoemModelMaterialGetSphereMapTextureObject(const nanoem_model_material_t *material)
{
    const nanoem_model_texture_t *texture = NULL;
    int sphere_texture_index;
    if (nanoem_is_not_null(material)) {
        sphere_texture_index = material->sphere_map_texture_index;
        if (sphere_texture_index > NANOEM_MODEL_OBJECT_NOT_FOUND) {
            texture = nanoemModelGetOneTextureObject(nanoemModelMaterialGetParentModel(material), sphere_texture_index);
        }
        else if (material->sphere_map_texture_spa) {
            texture = material->sphere_map_texture_spa;
        }
        else if (material->sphere_map_texture_sph) {
            texture = material->sphere_map_texture_sph;
        }
    }
    return texture;
}

const nanoem_model_texture_t *APIENTRY
nanoemModelMaterialGetToonTextureObject(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? nanoemModelGetOneTextureObject(nanoemModelMaterialGetParentModel(material), material->toon_texture_index) : NULL;
}

const nanoem_unicode_string_t *APIENTRY
nanoemModelMaterialGetClob(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->clob : NULL;
}

const nanoem_f32_t *APIENTRY
nanoemModelMaterialGetAmbientColor(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->ambient_color.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelMaterialGetDiffuseColor(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->diffuse_color.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelMaterialGetSpecularColor(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->specular_color.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelMaterialGetEdgeColor(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->edge_color.values : __nanoem_null_vector4;
}

nanoem_f32_t APIENTRY
nanoemModelMaterialGetDiffuseOpacity(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->diffuse_opacity : 0;
}

nanoem_f32_t APIENTRY
nanoemModelMaterialGetEdgeOpacity(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->edge_opacity : 0;
}

nanoem_f32_t APIENTRY
nanoemModelMaterialGetEdgeSize(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->edge_size : 0;
}

nanoem_f32_t APIENTRY
nanoemModelMaterialGetSpecularPower(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->specular_power : 0;
}

nanoem_model_material_sphere_map_texture_type_t APIENTRY
nanoemModelMaterialGetSphereMapTextureType(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->sphere_map_texture_type : NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE;
}

nanoem_rsize_t APIENTRY
nanoemModelMaterialGetNumVertexIndices(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->num_vertex_indices : 0;
}

int APIENTRY
nanoemModelMaterialGetToonTextureIndex(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->toon_texture_index : NANOEM_MODEL_OBJECT_NOT_FOUND;
}

nanoem_bool_t APIENTRY
nanoemModelMaterialIsEdgeEnabled(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->u.flags.is_edge_enabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelMaterialIsToonShared(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->is_toon_shared : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelMaterialIsCullingDisabled(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->u.flags.is_culling_disabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelMaterialIsCastingShadowEnabled(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->u.flags.is_casting_shadow_enabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelMaterialIsCastingShadowMapEnabled(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->u.flags.is_casting_shadow_map_enabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelMaterialIsShadowMapEnabled(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->u.flags.is_shadow_map_enabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelMaterialIsVertexColorEnabled(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->u.flags.is_vertex_color_enabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelMaterialIsPointDrawEnabled(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->u.flags.is_point_draw_enabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelMaterialIsLineDrawEnabled(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->u.flags.is_line_draw_enabled : nanoem_false;
}

const nanoem_model_object_t *APIENTRY
nanoemModelMaterialGetModelObject(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? &material->base : NULL;
}

nanoem_model_object_t *APIENTRY
nanoemModelMaterialGetModelObjectMutable(nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? &material->base : NULL;
}

const nanoem_unicode_string_t *APIENTRY
nanoemModelBoneGetName(const nanoem_model_bone_t *bone, nanoem_language_type_t language)
{
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(bone)) {
        switch (language) {
        case NANOEM_LANGUAGE_TYPE_JAPANESE:
            name = bone->name_ja;
            break;
        case NANOEM_LANGUAGE_TYPE_ENGLISH:
            name = bone->name_en;
            break;
        case NANOEM_LANGUAGE_TYPE_MAX_ENUM:
        case NANOEM_LANGUAGE_TYPE_UNKNOWN:
        default:
            break;
        }
    }
    return name;
}

const nanoem_model_bone_t *APIENTRY
nanoemModelBoneGetParentBoneObject(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? nanoemModelGetOneBoneObject(nanoemModelBoneGetParentModel(bone), bone->parent_bone_index) : NULL;
}

const nanoem_model_bone_t *APIENTRY
nanoemModelBoneGetInherentParentBoneObject(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? nanoemModelGetOneBoneObject(nanoemModelBoneGetParentModel(bone), bone->parent_inherent_bone_index) : NULL;
}

const nanoem_model_bone_t *APIENTRY
nanoemModelBoneGetEffectorBoneObject(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? nanoemModelGetOneBoneObject(nanoemModelBoneGetParentModel(bone), bone->effector_bone_index) : NULL;
}

const nanoem_model_bone_t *APIENTRY
nanoemModelBoneGetTargetBoneObject(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? nanoemModelGetOneBoneObject(nanoemModelBoneGetParentModel(bone), bone->target_bone_index) : NULL;
}

const nanoem_model_constraint_t *APIENTRY
nanoemModelBoneGetConstraintObject(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->constraint : NULL;
}

nanoem_model_constraint_t *APIENTRY
nanoemModelBoneGetConstraintObjectMutable(nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->constraint : NULL;
}

const nanoem_f32_t *APIENTRY
nanoemModelBoneGetOrigin(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->origin.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelBoneGetDestinationOrigin(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->destination_origin.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelBoneGetFixedAxis(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->fixed_axis.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelBoneGetLocalXAxis(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->local_x_axis.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelBoneGetLocalZAxis(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->local_z_axis.values : __nanoem_null_vector4;
}

nanoem_f32_t APIENTRY
nanoemModelBoneGetInherentCoefficient(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->inherent_coefficient : 0;
}

int APIENTRY
nanoemModelBoneGetStageIndex(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->stage_index : 0;
}

nanoem_bool_t APIENTRY
nanoemModelBoneHasDestinationBone(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->u.flags.has_destination_bone_index : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelBoneIsRotateable(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->u.flags.is_rotateable : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelBoneIsMovable(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->u.flags.is_movable : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelBoneIsVisible(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->u.flags.is_visible : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelBoneIsUserHandleable(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->u.flags.is_user_handleable : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelBoneHasConstraint(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->u.flags.has_constraint : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelBoneHasLocalInherent(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->u.flags.has_local_inherent : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelBoneHasInherentTranslation(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->u.flags.has_inherent_translation : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelBoneHasInherentOrientation(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->u.flags.has_inherent_orientation : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelBoneHasFixedAxis(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->u.flags.has_fixed_axis : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelBoneHasLocalAxes(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->u.flags.has_local_axes : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelBoneIsAffectedByPhysicsSimulation(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->u.flags.is_affected_by_physics_simulation : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelBoneHasExternalParentBone(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->u.flags.has_external_parent_bone : nanoem_false;
}

const nanoem_model_object_t *APIENTRY
nanoemModelBoneGetModelObject(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? &bone->base : NULL;
}

nanoem_model_object_t *APIENTRY
nanoemModelBoneGetModelObjectMutable(nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? &bone->base : NULL;
}

const nanoem_model_bone_t *APIENTRY
nanoemModelConstraintGetEffectorBoneObject(const nanoem_model_constraint_t *constraint)
{
    return nanoem_is_not_null(constraint) ? nanoemModelGetOneBoneObject(nanoemModelConstraintGetParentModel(constraint), constraint->effector_bone_index) : NULL;
}

const nanoem_model_bone_t *APIENTRY
nanoemModelConstraintGetTargetBoneObject(const nanoem_model_constraint_t *constraint)
{
    return nanoem_is_not_null(constraint) ? nanoemModelGetOneBoneObject(nanoemModelConstraintGetParentModel(constraint), constraint->target_bone_index) : NULL;
}

nanoem_f32_t APIENTRY
nanoemModelConstraintGetAngleLimit(const nanoem_model_constraint_t *constraint)
{
    return nanoem_is_not_null(constraint) ? constraint->angle_limit : 0;
}

int APIENTRY
nanoemModelConstraintGetNumIterations(const nanoem_model_constraint_t *constraint)
{
    return nanoem_is_not_null(constraint) ? constraint->num_iterations : 0;
}

nanoem_model_constraint_joint_t *const *APIENTRY
nanoemModelConstraintGetAllJointObjects(const nanoem_model_constraint_t *constraint, nanoem_rsize_t *num_joints)
{
    nanoem_model_constraint_joint_t *const *joints = NULL;
    if (nanoem_is_not_null(constraint) && nanoem_is_not_null(num_joints)) {
        *num_joints = constraint->num_joints;
        joints = constraint->joints;
    }
    else if (nanoem_is_not_null(num_joints)) {
        *num_joints = 0;
    }
    return joints;
}

const nanoem_model_object_t *APIENTRY
nanoemModelConstraintGetModelObject(const nanoem_model_constraint_t *constraint)
{
    return nanoem_is_not_null(constraint) ? &constraint->base : NULL;
}

nanoem_model_object_t *APIENTRY
nanoemModelConstraintGetModelObjectMutable(nanoem_model_constraint_t *constraint)
{
    return nanoem_is_not_null(constraint) ? &constraint->base : NULL;
}

const nanoem_model_bone_t *APIENTRY
nanoemModelConstraintJointGetBoneObject(const nanoem_model_constraint_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? nanoemModelGetOneBoneObject(nanoemModelConstraintGetParentModel(nanoemModelConstraintJointGetParentConstraintObject(joint)), joint->bone_index) : NULL;
}

const nanoem_f32_t *APIENTRY
nanoemModelConstraintJointGetUpperLimit(const nanoem_model_constraint_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? joint->upper_limit.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelConstraintJointGetLowerLimit(const nanoem_model_constraint_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? joint->lower_limit.values : __nanoem_null_vector4;
}

nanoem_bool_t APIENTRY
nanoemModelConstraintJointHasAngleLimit(const nanoem_model_constraint_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? joint->has_angle_limit : nanoem_false;
}

const nanoem_unicode_string_t *APIENTRY
nanoemModelTextureGetPath(const nanoem_model_texture_t *texture)
{
    return nanoem_is_not_null(texture) ? texture->path : NULL;
}

const nanoem_model_object_t *APIENTRY
nanoemModelTextureGetModelObject(const nanoem_model_texture_t *texture)
{
    return nanoem_is_not_null(texture) ? &texture->base : NULL;
}

nanoem_model_object_t *APIENTRY
nanoemModelTextureGetModelObjectMutable(nanoem_model_texture_t *texture)
{
    return nanoem_is_not_null(texture) ? &texture->base : NULL;
}

const nanoem_model_bone_t *APIENTRY
nanoemModelMorphBoneGetBoneObject(const nanoem_model_morph_bone_t *morph)
{
    return nanoem_is_not_null(morph) ? nanoemModelGetOneBoneObject(nanoemModelMorphGetParentModel(nanoemModelMorphBoneGetParentMorph(morph)), morph->bone_index) : NULL;
}

const nanoem_f32_t *APIENTRY
nanoemModelMorphBoneGetTranslation(const nanoem_model_morph_bone_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->translation.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelMorphBoneGetOrientation(const nanoem_model_morph_bone_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->orientation.values : __nanoem_null_vector4;
}

const nanoem_model_morph_t *APIENTRY
nanoemModelMorphFlipGetMorphObject(const nanoem_model_morph_flip_t *morph)
{
    return nanoem_is_not_null(morph) ? nanoemModelGetOneMorphObject(nanoemModelMorphGetParentModel(nanoemModelMorphFlipGetParentMorph(morph)), morph->morph_index) : NULL;
}

nanoem_f32_t APIENTRY
nanoemModelMorphFlipGetWeight(const nanoem_model_morph_flip_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->weight : 0;
}

const nanoem_model_morph_t *APIENTRY
nanoemModelMorphGroupGetMorphObject(const nanoem_model_morph_group_t *morph)
{
    return nanoem_is_not_null(morph) ? nanoemModelGetOneMorphObject(nanoemModelMorphGetParentModel(nanoemModelMorphGroupGetParentMorph(morph)), morph->morph_index) : NULL;
}

nanoem_f32_t APIENTRY
nanoemModelMorphGroupGetWeight(const nanoem_model_morph_group_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->weight : 0;
}

const nanoem_model_rigid_body_t *APIENTRY
nanoemModelMorphImpulseGetRigidBodyObject(const nanoem_model_morph_impulse_t *morph)
{
    return nanoem_is_not_null(morph) ? nanoemModelGetOneRigidBodyObject(nanoemModelMorphGetParentModel(nanoemModelMorphImpulseGetParentMorph(morph)), morph->rigid_body_index) : NULL;
}

const nanoem_f32_t *APIENTRY
nanoemModelMorphImpulseGetTorque(const nanoem_model_morph_impulse_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->torque.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelMorphImpulseGetVelocity(const nanoem_model_morph_impulse_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->velocity.values : __nanoem_null_vector4;
}

nanoem_bool_t APIENTRY
nanoemModelMorphImpulseIsLocal(const nanoem_model_morph_impulse_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->is_local : nanoem_false;
}

const nanoem_model_material_t *APIENTRY
nanoemModelMorphMaterialGetMaterialObject(const nanoem_model_morph_material_t *morph)
{
    return nanoem_is_not_null(morph) ? nanoemModelGetOneMaterialObject(nanoemModelMorphGetParentModel(nanoemModelMorphMaterialGetParentMorph(morph)), morph->material_index) : NULL;
}

const nanoem_f32_t *APIENTRY
nanoemModelMorphMaterialGetAmbientColor(const nanoem_model_morph_material_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->ambient_color.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelMorphMaterialGetDiffuseColor(const nanoem_model_morph_material_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->diffuse_color.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelMorphMaterialGetSpecularColor(const nanoem_model_morph_material_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->specular_color.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelMorphMaterialGetEdgeColor(const nanoem_model_morph_material_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->edge_color.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelMorphMaterialGetDiffuseTextureBlend(const nanoem_model_morph_material_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->diffuse_texture_blend.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelMorphMaterialGetSphereMapTextureBlend(const nanoem_model_morph_material_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->sphere_map_texture_blend.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelMorphMaterialGetToonTextureBlend(const nanoem_model_morph_material_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->toon_texture_blend.values : __nanoem_null_vector4;
}

nanoem_f32_t APIENTRY
nanoemModelMorphMaterialGetDiffuseOpacity(const nanoem_model_morph_material_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->diffuse_opacity : 0;
}

nanoem_f32_t APIENTRY
nanoemModelMorphMaterialGetEdgeOpacity(const nanoem_model_morph_material_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->edge_opacity : 0;
}

nanoem_f32_t APIENTRY
nanoemModelMorphMaterialGetSpecularPower(const nanoem_model_morph_material_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->specular_power : 0;
}

nanoem_f32_t APIENTRY
nanoemModelMorphMaterialGetEdgeSize(const nanoem_model_morph_material_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->edge_size : 0;
}

nanoem_model_morph_material_operation_type_t APIENTRY
nanoemModelMorphMaterialGetOperationType(const nanoem_model_morph_material_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->operation : NANOEM_MODEL_MORPH_MATERIAL_OPERATION_TYPE_UNKNOWN;
}

const nanoem_model_vertex_t *APIENTRY
nanoemModelMorphUVGetVertexObject(const nanoem_model_morph_uv_t *morph)
{
    return nanoem_is_not_null(morph) ? nanoemModelGetOneVertexObject(nanoemModelMorphGetParentModel(nanoemModelMorphUVGetParentMorph(morph)), morph->vertex_index) : NULL;
}

const nanoem_f32_t *APIENTRY
nanoemModelMorphUVGetPosition(const nanoem_model_morph_uv_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->position.values : __nanoem_null_vector4;
}

const nanoem_model_vertex_t *APIENTRY
nanoemModelMorphVertexGetVertexObject(const nanoem_model_morph_vertex_t *morph)
{
    return nanoem_is_not_null(morph) ? nanoemModelGetOneVertexObject(nanoemModelMorphGetParentModel(nanoemModelMorphVertexGetParentMorph(morph)), morph->vertex_index) : NULL;
}

const nanoem_f32_t *APIENTRY
nanoemModelMorphVertexGetPosition(const nanoem_model_morph_vertex_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->position.values : __nanoem_null_vector4;
}

const nanoem_unicode_string_t *APIENTRY
nanoemModelMorphGetName(const nanoem_model_morph_t *morph, nanoem_language_type_t language)
{
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(morph)) {
        switch (language) {
        case NANOEM_LANGUAGE_TYPE_JAPANESE:
            name = morph->name_ja;
            break;
        case NANOEM_LANGUAGE_TYPE_ENGLISH:
            name = morph->name_en;
            break;
        case NANOEM_LANGUAGE_TYPE_MAX_ENUM:
        case NANOEM_LANGUAGE_TYPE_UNKNOWN:
        default:
            break;
        }
    }
    return name;
}

nanoem_model_morph_category_t APIENTRY
nanoemModelMorphGetCategory(const nanoem_model_morph_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->category : NANOEM_MODEL_MORPH_CATEGORY_UNKNOWN;
}

nanoem_model_morph_type_t APIENTRY
nanoemModelMorphGetType(const nanoem_model_morph_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->type : NANOEM_MODEL_MORPH_TYPE_UNKNOWN;
}

nanoem_model_morph_bone_t *const *APIENTRY
nanoemModelMorphGetAllBoneMorphObjects(const nanoem_model_morph_t *morph, nanoem_rsize_t *num_objects)
{
    nanoem_model_morph_bone_t *const *items = NULL;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(num_objects) && morph->type == NANOEM_MODEL_MORPH_TYPE_BONE) {
        *num_objects = morph->num_objects;
        items = morph->u.bones;
    }
    else if (nanoem_is_not_null(num_objects))  {
        *num_objects = 0;
    }
    return items;
}

nanoem_model_morph_flip_t *const *APIENTRY
nanoemModelMorphGetAllFlipMorphObjects(const nanoem_model_morph_t *morph, nanoem_rsize_t *num_objects)
{
    nanoem_model_morph_flip_t *const *items = NULL;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(num_objects) && morph->type == NANOEM_MODEL_MORPH_TYPE_FLIP) {
        *num_objects = morph->num_objects;
        items = morph->u.flips;
    }
    else if (nanoem_is_not_null(num_objects))  {
        *num_objects = 0;
    }
    return items;
}

nanoem_model_morph_group_t *const *APIENTRY
nanoemModelMorphGetAllGroupMorphObjects(const nanoem_model_morph_t *morph, nanoem_rsize_t *num_objects)
{
    nanoem_model_morph_group_t *const *items = NULL;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(num_objects) && morph->type == NANOEM_MODEL_MORPH_TYPE_GROUP) {
        *num_objects = morph->num_objects;
        items = morph->u.groups;
    }
    else if (nanoem_is_not_null(num_objects))  {
        *num_objects = 0;
    }
    return items;
}

nanoem_model_morph_impulse_t *const *APIENTRY
nanoemModelMorphGetAllImpulseMorphObjects(const nanoem_model_morph_t *morph, nanoem_rsize_t *num_objects)
{
    nanoem_model_morph_impulse_t *const *items = NULL;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(num_objects) && morph->type == NANOEM_MODEL_MORPH_TYPE_IMPULUSE) {
        *num_objects = morph->num_objects;
        items = morph->u.impulses;
    }
    else if (nanoem_is_not_null(num_objects)) {
        *num_objects = 0;
    }
    return items;
}

nanoem_model_morph_material_t *const *APIENTRY
nanoemModelMorphGetAllMaterialMorphObjects(const nanoem_model_morph_t *morph, nanoem_rsize_t *num_objects)
{
    nanoem_model_morph_material_t *const *items = NULL;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(num_objects) && morph->type == NANOEM_MODEL_MORPH_TYPE_MATERIAL) {
        *num_objects = morph->num_objects;
        items = morph->u.materials;
    }
    else if (nanoem_is_not_null(num_objects))  {
        *num_objects = 0;
    }
    return items;
}

nanoem_model_morph_uv_t *const *APIENTRY
nanoemModelMorphGetAllUVMorphObjects(const nanoem_model_morph_t *morph, nanoem_rsize_t *num_objects)
{
    nanoem_model_morph_uv_t *const *items = NULL;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(num_objects) && morph->type >= NANOEM_MODEL_MORPH_TYPE_TEXTURE && morph->type <= NANOEM_MODEL_MORPH_TYPE_UVA4) {
        *num_objects = morph->num_objects;
        items = morph->u.uvs;
    }
    else if (nanoem_is_not_null(num_objects))  {
        *num_objects = 0;
    }
    return items;
}

nanoem_model_morph_vertex_t *const *APIENTRY
nanoemModelMorphGetAllVertexMorphObjects(const nanoem_model_morph_t *morph, nanoem_rsize_t *num_objects)
{
    nanoem_model_morph_vertex_t *const *items = NULL;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(num_objects) && morph->type == NANOEM_MODEL_MORPH_TYPE_VERTEX) {
        *num_objects = morph->num_objects;
        items = morph->u.vertices;
    }
    else if (nanoem_is_not_null(num_objects))  {
        *num_objects = 0;
    }
    return items;
}

const nanoem_model_object_t *APIENTRY
nanoemModelMorphGetModelObject(const nanoem_model_morph_t *morph)
{
    return nanoem_is_not_null(morph) ? &morph->base : NULL;
}

nanoem_model_object_t *APIENTRY
nanoemModelMorphGetModelObjectMutable(nanoem_model_morph_t *morph)
{
    return nanoem_is_not_null(morph) ? &morph->base : NULL;
}

const nanoem_unicode_string_t *APIENTRY
nanoemModelLabelGetName(const nanoem_model_label_t *label, nanoem_language_type_t language)
{
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(label)) {
        switch (language) {
        case NANOEM_LANGUAGE_TYPE_JAPANESE:
            name = label->name_ja;
            break;
        case NANOEM_LANGUAGE_TYPE_ENGLISH:
            name = label->name_en;
            break;
        case NANOEM_LANGUAGE_TYPE_MAX_ENUM:
        case NANOEM_LANGUAGE_TYPE_UNKNOWN:
        default:
            break;
        }
    }
    return name;
}

nanoem_bool_t APIENTRY
nanoemModelLabelIsSpecial(const nanoem_model_label_t *label)
{
    return nanoem_is_not_null(label) ? label->is_special : nanoem_false;
}

nanoem_model_label_item_t *const *APIENTRY
nanoemModelLabelGetAllItemObjects(const nanoem_model_label_t *label, nanoem_rsize_t *num_objects)
{
    nanoem_model_label_item_t *const *items = NULL;
    if (nanoem_is_not_null(label) && nanoem_is_not_null(num_objects)) {
        *num_objects = label->num_items;
        items = label->items;
    }
    else if (nanoem_is_not_null(num_objects))  {
        *num_objects = 0;
    }
    return items;
}

nanoem_model_label_item_type_t APIENTRY
nanoemModelLabelItemGetType(const nanoem_model_label_item_t *label_item)
{
    return nanoem_is_not_null(label_item) ? label_item->type : NANOEM_MODEL_LABEL_ITEM_TYPE_UNKNOWN;
}

const nanoem_model_bone_t *APIENTRY
nanoemModelLabelItemGetBoneObject(const nanoem_model_label_item_t *label_item)
{
    return nanoem_is_not_null(label_item) && label_item->type == NANOEM_MODEL_LABEL_ITEM_TYPE_BONE ? label_item->u.bone : 0;
}

const nanoem_model_morph_t *APIENTRY
nanoemModelLabelItemGetMorphObject(const nanoem_model_label_item_t *label_item)
{
    return nanoem_is_not_null(label_item) && label_item->type == NANOEM_MODEL_LABEL_ITEM_TYPE_MORPH ? label_item->u.morph : 0;
}

const nanoem_model_object_t *APIENTRY
nanoemModelLabelGetModelObject(const nanoem_model_label_t *label)
{
    return nanoem_is_not_null(label) ? &label->base : NULL;
}

nanoem_model_object_t *APIENTRY
nanoemModelLabelGetModelObjectMutable(nanoem_model_label_t *label)
{
    return nanoem_is_not_null(label) ? &label->base : NULL;
}

const nanoem_unicode_string_t *APIENTRY
nanoemModelRigidBodyGetName(const nanoem_model_rigid_body_t *rigid_body, nanoem_language_type_t language)
{
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(rigid_body)) {
        switch (language) {
        case NANOEM_LANGUAGE_TYPE_JAPANESE:
            name = rigid_body->name_ja;
            break;
        case NANOEM_LANGUAGE_TYPE_ENGLISH:
            name = rigid_body->name_en;
            break;
        case NANOEM_LANGUAGE_TYPE_MAX_ENUM:
        case NANOEM_LANGUAGE_TYPE_UNKNOWN:
        default:
            break;
        }
    }
    return name;
}

const nanoem_model_bone_t *APIENTRY
nanoemModelRigidBodyGetBoneObject(const nanoem_model_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? nanoemModelGetOneBoneObject(nanoemModelRigidBodyGetParentModel(rigid_body), rigid_body->bone_index) : NULL;
}

const nanoem_f32_t *APIENTRY
nanoemModelRigidBodyGetOrigin(const nanoem_model_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? rigid_body->origin.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelRigidBodyGetOrientation(const nanoem_model_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? rigid_body->orientation.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelRigidBodyGetShapeSize(const nanoem_model_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? rigid_body->size.values : __nanoem_null_vector4;
}

nanoem_f32_t APIENTRY
nanoemModelRigidBodyGetMass(const nanoem_model_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? rigid_body->mass : 0;
}

nanoem_f32_t APIENTRY
nanoemModelRigidBodyGetLinearDamping(const nanoem_model_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? rigid_body->linear_damping : 0;
}

nanoem_f32_t APIENTRY
nanoemModelRigidBodyGetAngularDamping(const nanoem_model_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? rigid_body->angular_damping : 0;
}

nanoem_f32_t APIENTRY
nanoemModelRigidBodyGetFriction(const nanoem_model_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? rigid_body->friction : 0;
}

nanoem_f32_t APIENTRY
nanoemModelRigidBodyGetRestitution(const nanoem_model_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? rigid_body->restitution : 0;
}

nanoem_model_rigid_body_shape_type_t APIENTRY
nanoemModelRigidBodyGetShapeType(const nanoem_model_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? rigid_body->shape_type : NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_UNKNOWN;
}

nanoem_model_rigid_body_transform_type_t APIENTRY
nanoemModelRigidBodyGetTransformType(const nanoem_model_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? rigid_body->transform_type : NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_UNKNOWN;
}

int APIENTRY
nanoemModelRigidBodyGetCollisionGroupId(const nanoem_model_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? rigid_body->collision_group_id : 0;
}

int APIENTRY
nanoemModelRigidBodyGetCollisionMask(const nanoem_model_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? rigid_body->collision_mask : 0;
}

nanoem_bool_t APIENTRY
nanoemModelRigidBodyIsBoneRelativePosition(const nanoem_model_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? rigid_body->is_bone_relative : nanoem_false;
}

const nanoem_model_object_t *APIENTRY
nanoemModelRigidBodyGetModelObject(const nanoem_model_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? &rigid_body->base : NULL;
}

nanoem_model_object_t *APIENTRY
nanoemModelRigidBodyGetModelObjectMutable(nanoem_model_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? &rigid_body->base : NULL;
}

const nanoem_unicode_string_t *APIENTRY
nanoemModelJointGetName(const nanoem_model_joint_t *joint, nanoem_language_type_t language)
{
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(joint)) {
        switch (language) {
        case NANOEM_LANGUAGE_TYPE_JAPANESE:
            name = joint->name_ja;
            break;
        case NANOEM_LANGUAGE_TYPE_ENGLISH:
            name = joint->name_en;
            break;
        case NANOEM_LANGUAGE_TYPE_MAX_ENUM:
        case NANOEM_LANGUAGE_TYPE_UNKNOWN:
        default:
            break;
        }
    }
    return name;
}

const nanoem_model_rigid_body_t *APIENTRY
nanoemModelJointGetRigidBodyAObject(const nanoem_model_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? nanoemModelGetOneRigidBodyObject(nanoemModelJointGetParentModel(joint), joint->rigid_body_a_index) : NULL;
}

const nanoem_model_rigid_body_t *APIENTRY
nanoemModelJointGetRigidBodyBObject(const nanoem_model_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? nanoemModelGetOneRigidBodyObject(nanoemModelJointGetParentModel(joint), joint->rigid_body_b_index) : NULL;
}

const nanoem_f32_t *APIENTRY
nanoemModelJointGetOrigin(const nanoem_model_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? joint->origin.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelJointGetOrientation(const nanoem_model_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? joint->orientation.values : __nanoem_null_vector4;
}

nanoem_model_joint_type_t APIENTRY
nanoemModelJointGetType(const nanoem_model_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? joint->type : NANOEM_MODEL_JOINT_TYPE_UNKNOWN;
}

const nanoem_f32_t *APIENTRY
nanoemModelJointGetLinearUpperLimit(const nanoem_model_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? joint->linear_upper_limit.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelJointGetLinearLowerLimit(const nanoem_model_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? joint->linear_lower_limit.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelJointGetLinearStiffness(const nanoem_model_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? joint->linear_stiffness.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelJointGetAngularUpperLimit(const nanoem_model_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? joint->angular_upper_limit.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelJointGetAngularLowerLimit(const nanoem_model_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? joint->angular_lower_limit.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemModelJointGetAngularStiffness(const nanoem_model_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? joint->angular_stiffness.values : __nanoem_null_vector4;
}

const nanoem_model_object_t *APIENTRY
nanoemModelJointGetModelObject(const nanoem_model_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? &joint->base : NULL;
}

nanoem_model_object_t *APIENTRY
nanoemModelJointGetModelObjectMutable(nanoem_model_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? &joint->base : NULL;
}

const nanoem_unicode_string_t *APIENTRY
nanoemModelSoftBodyGetName(const nanoem_model_soft_body_t *soft_body, nanoem_language_type_t language)
{
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(soft_body)) {
        switch (language) {
        case NANOEM_LANGUAGE_TYPE_JAPANESE:
            name = soft_body->name_ja;
            break;
        case NANOEM_LANGUAGE_TYPE_ENGLISH:
            name = soft_body->name_en;
            break;
        case NANOEM_LANGUAGE_TYPE_MAX_ENUM:
        case NANOEM_LANGUAGE_TYPE_UNKNOWN:
        default:
            break;
        }
    }
    return name;
}

nanoem_model_soft_body_anchor_t *const * APIENTRY
nanoemModelSoftBodyGetAllAnchorObjects(const nanoem_model_soft_body_t *soft_body, nanoem_rsize_t *num_objects)
{
    nanoem_model_soft_body_anchor_t *const *objects = NULL;
    if (nanoem_is_not_null(soft_body) && nanoem_is_not_null(num_objects)) {
        *num_objects = soft_body->num_anchors;
        objects = soft_body->anchors;
    }
    else if (nanoem_is_not_null(num_objects))  {
        *num_objects = 0;
    }
    return objects;
}

const nanoem_u32_t * APIENTRY
nanoemModelSoftBodyGetAllPinnedVertexIndices(const nanoem_model_soft_body_t *soft_body, nanoem_rsize_t *num_objects)
{
    const nanoem_u32_t *objects = NULL;
    if (nanoem_is_not_null(soft_body) && nanoem_is_not_null(num_objects)) {
        *num_objects = soft_body->num_pinned_vertex_indices;
        objects = soft_body->pinned_vertex_indices;
    }
    else if (nanoem_is_not_null(num_objects))  {
        *num_objects = 0;
    }
    return objects;
}

const nanoem_model_material_t *APIENTRY
nanoemModelSoftBodyGetMaterialObject(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? nanoemModelGetOneMaterialObject(nanoemModelSoftBodyGetParentModel(soft_body), soft_body->material_index) : NULL;
}

nanoem_model_soft_body_shape_type_t APIENTRY
nanoemModelSoftBodyGetShapeType(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->shape_type : NANOEM_MODEL_SOFT_BODY_SHAPE_TYPE_UNKNOWN;
}

nanoem_model_soft_body_aero_model_type_t APIENTRY
nanoemModelSoftBodyGetAeroModel(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->aero_model : NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_UNKNOWN;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetTotalMass(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->total_mass : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetCollisionMargin(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->collision_margin : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetVelocityCorrectionFactor(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->velocity_correction_factor : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetDampingCoefficient(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->damping_coefficient : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetDragCoefficient(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->drag_coefficient : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetLiftCoefficient(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->lift_coefficient : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetPressureCoefficient(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->pressure_coefficient : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetVolumeConversationCoefficient(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->volume_conversation_coefficient : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetDynamicFrictionCoefficient(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->dynamic_friction_coefficient : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetPoseMatchingCoefficient(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->pose_matching_coefficient : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetRigidContactHardness(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->rigid_contact_hardness : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetKineticContactHardness(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->kinetic_contact_hardness : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetSoftContactHardness(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->soft_contact_hardness : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetAnchorHardness(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->anchor_hardness : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetSoftVSRigidHardness(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->soft_vs_rigid_hardness : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetSoftVSKineticHardness(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->soft_vs_kinetic_hardness : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetSoftVSSoftHardness(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->soft_vs_soft_hardness : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetSoftVSRigidImpulseSplit(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->soft_vs_rigid_impulse_split : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetSoftVSKineticImpulseSplit(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->soft_vs_kinetic_impulse_split : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetSoftVSSoftImpulseSplit(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->soft_vs_soft_impulse_split : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetLinearStiffnessCoefficient(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->linear_stiffness_coefficient : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetAngularStiffnessCoefficient(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->angular_stiffness_coefficient : 0;
}

nanoem_f32_t APIENTRY
nanoemModelSoftBodyGetVolumeStiffnessCoefficient(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->volume_stiffness_coefficient : 0;
}

int APIENTRY
nanoemModelSoftBodyGetCollisionGroupId(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->collision_group_id : 0;
}

int APIENTRY
nanoemModelSoftBodyGetCollisionMask(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->collision_mask : 0;
}

int APIENTRY
nanoemModelSoftBodyGetBendingConstraintsDistance(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->bending_constraints_distance : 0;
}

int APIENTRY
nanoemModelSoftBodyGetClusterCount(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->cluster_count : 0;
}

int APIENTRY
nanoemModelSoftBodyGetVelocitySolverIterations(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->velocity_solver_iterations : 0;
}

int APIENTRY
nanoemModelSoftBodyGetPositionsSolverIterations(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->positions_solver_iterations : 0;
}

int APIENTRY
nanoemModelSoftBodyGetDriftSolverIterations(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->drift_solver_iterations : 0;
}

int APIENTRY
nanoemModelSoftBodyGetClusterSolverIterations(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->cluster_solver_iterations : 0;
}

nanoem_bool_t APIENTRY
nanoemModelSoftBodyIsBendingConstraintsEnabled(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? (soft_body->flags & 0x1) != 0 : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelSoftBodyIsClustersEnabled(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? (soft_body->flags & 0x2) != 0 : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemModelSoftBodyIsRandomizeConstraintsNeeded(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? (soft_body->flags & 0x4) != 0 : nanoem_false;
}

const nanoem_model_rigid_body_t *APIENTRY
nanoemModelSoftBodyAnchorGetRigidBodyObject(const nanoem_model_soft_body_anchor_t *anchor)
{
    const nanoem_model_rigid_body_t *body = NULL;
    const nanoem_model_soft_body_t *parent_soft_body = NULL;
    const nanoem_model_t *parent_model = NULL;
    if (nanoem_is_not_null(anchor)) {
        parent_soft_body = nanoemModelSoftBodyAnchorGetParentSoftBody(anchor);
        parent_model = nanoemModelSoftBodyGetParentModel(parent_soft_body);
        body = nanoemModelGetOneRigidBodyObject(parent_model, anchor->rigid_body_index);
    }
    return body;
}

const nanoem_model_vertex_t *APIENTRY
nanoemModelSoftBodyAnchorGetVertexObject(const nanoem_model_soft_body_anchor_t *anchor)
{
    const nanoem_model_vertex_t *vertex = NULL;
    const nanoem_model_soft_body_t *parent_soft_body = NULL;
    const nanoem_model_t *parent_model = NULL;
    if (nanoem_is_not_null(anchor)) {
        parent_soft_body = nanoemModelSoftBodyAnchorGetParentSoftBody(anchor);
        parent_model = nanoemModelSoftBodyGetParentModel(parent_soft_body);
        vertex = nanoemModelGetOneVertexObject(parent_model, anchor->rigid_body_index);
    }
    return vertex;
}

nanoem_bool_t APIENTRY
nanoemModelSoftBodyAnchorIsNearEnabled(const nanoem_model_soft_body_anchor_t *anchor)
{
    return nanoem_is_not_null(anchor) ? anchor->is_near_enabled : nanoem_false;
}

const nanoem_model_object_t *APIENTRY
nanoemModelSoftBodyGetModelObject(const nanoem_model_soft_body_t *body)
{
    return nanoem_is_not_null(body) ? &body->base : NULL;
}

nanoem_model_object_t *APIENTRY
nanoemModelSoftBodyGetModelObjectMutable(nanoem_model_soft_body_t *body)
{
    return nanoem_is_not_null(body) ? &body->base : NULL;
}

nanoem_model_t *APIENTRY
nanoemModelCreate(nanoem_unicode_string_factory_t *factory, nanoem_status_t *status)
{
    nanoem_model_t *model = NULL;
    if (nanoem_is_not_null(factory)) {
        model = (nanoem_model_t *) nanoem_calloc(1, sizeof(*model), status);
        if (nanoem_is_not_null(model)) {
            model->factory = factory;
        }
    }
    return model;
}

nanoem_bool_t APIENTRY
nanoemModelLoadFromBufferPMD(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_u8_t *ptr = nanoemBufferGetDataPtr(buffer);
    nanoem_unicode_string_factory_t *factory;
    if (nanoemBufferCanReadLength(buffer, 3) && ptr && nanoem_crt_memcmp(ptr, __nanoem_pmd_signature, sizeof(__nanoem_pmd_signature) - 1) == 0) {
        nanoem_status_ptr_assign_succeeded(status);
        factory = model->factory;
        nanoemBufferSkip(buffer, sizeof(__nanoem_pmd_signature) - 1, status);
        model->version = nanoemBufferReadFloat32LittleEndian(buffer, status);
        model->name_ja = nanoemBufferGetStringFromCp932(buffer, PMD_MODEL_NAME_LENGTH, factory, status);
        model->comment_ja = nanoemBufferGetStringFromCp932(buffer, PMD_MODEL_COMMENT_LENGTH, factory, status);
        nanoemModelParsePMD(model, buffer, status);
    }
    else {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_INVALID_SIGNATURE);
    }
    return !nanoem_status_ptr_has_error(status);
}

nanoem_bool_t APIENTRY
nanoemModelLoadFromBufferPMX(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_u32_t signature = (nanoem_u32_t) nanoemBufferReadInt32LittleEndian(buffer, status);
    if (signature == nanoem_fourcc('P', 'M', 'X', ' ') || signature == nanoem_fourcc('P', 'M', 'X', (nanoem_u32_t) 0xA0)) {
        nanoem_status_ptr_assign_succeeded(status);
        model->version = nanoemBufferReadFloat32LittleEndian(buffer, status);
        model->info_length = nanoemBufferReadByte(buffer, status);
        if (model->info_length == 8) {
            model->info.codec_type = nanoemBufferReadByte(buffer, status);
            model->info.additional_uv_size = nanoemBufferReadByte(buffer, status);
            model->info.vertex_index_size = nanoemBufferReadByte(buffer, status);
            model->info.texture_index_size = nanoemBufferReadByte(buffer, status);
            model->info.material_index_size = nanoemBufferReadByte(buffer, status);
            model->info.bone_index_size = nanoemBufferReadByte(buffer, status);
            model->info.morph_index_size = nanoemBufferReadByte(buffer, status);
            model->info.rigid_body_index_size = nanoemBufferReadByte(buffer, status);
            model->name_ja = nanoemModelGetStringPMX(model, buffer, status);
            model->name_en = nanoemModelGetStringPMX(model, buffer, status);
            model->comment_ja = nanoemModelGetStringPMX(model, buffer, status);
            model->comment_en = nanoemModelGetStringPMX(model, buffer, status);
            nanoemModelParsePMX(model, buffer, status);
        }
    }
    else {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_INVALID_SIGNATURE);
    }
    return !nanoem_status_ptr_has_error(status);
}

nanoem_bool_t APIENTRY
nanoemModelLoadFromBuffer(nanoem_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    if (nanoem_is_not_null(model) && nanoem_is_not_null(buffer)) {
        nanoem_status_ptr_assign_succeeded(status);
        if (!nanoemModelLoadFromBufferPMX(model, buffer, status) && (status && *status == NANOEM_STATUS_ERROR_INVALID_SIGNATURE)) {
            nanoem_status_ptr_assign_succeeded(status);
            nanoemBufferSeek(buffer, 0, status);
            nanoemModelLoadFromBufferPMD(model, buffer, status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
    return !nanoem_status_ptr_has_error(status);
}

nanoem_model_format_type_t APIENTRY
nanoemModelGetFormatType(const nanoem_model_t *model)
{
    int version;
    if (nanoem_is_not_null(model)) {
        version = (int) (model->version * 10);
        switch (version) {
        case 10:
            return NANOEM_MODEL_FORMAT_TYPE_PMD_1_0;
        case 20:
            return NANOEM_MODEL_FORMAT_TYPE_PMX_2_0;
        case 21:
            return NANOEM_MODEL_FORMAT_TYPE_PMX_2_1;
        }
    }
    return NANOEM_MODEL_FORMAT_TYPE_UNKNOWN;
}

nanoem_codec_type_t APIENTRY
nanoemModelGetCodecType(const nanoem_model_t *model)
{
    int major_version;
    if (nanoem_is_not_null(model)) {
        major_version = (int) model->version;
        if (major_version >= 2) {
            return model->info.codec_type != 0 ? NANOEM_CODEC_TYPE_UTF8 : NANOEM_CODEC_TYPE_UTF16;
        }
        else if (major_version == 1) {
            return NANOEM_CODEC_TYPE_SJIS;
        }
    }
    return NANOEM_CODEC_TYPE_UNKNOWN;
}

nanoem_rsize_t APIENTRY
nanoemModelGetAdditionalUVSize(const nanoem_model_t *model)
{
    return nanoem_is_not_null(model) ? model->info.additional_uv_size : 0;
}

const nanoem_unicode_string_t *APIENTRY
nanoemModelGetName(const nanoem_model_t *model, nanoem_language_type_t language)
{
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(model)) {
        switch (language) {
        case NANOEM_LANGUAGE_TYPE_JAPANESE:
            name = model->name_ja;
            break;
        case NANOEM_LANGUAGE_TYPE_ENGLISH:
            name = model->name_en;
            break;
        case NANOEM_LANGUAGE_TYPE_MAX_ENUM:
        case NANOEM_LANGUAGE_TYPE_UNKNOWN:
        default:
            break;
        }
    }
    return name;
}

const nanoem_unicode_string_t *APIENTRY
nanoemModelGetComment(const nanoem_model_t *model, nanoem_language_type_t language)
{
    const nanoem_unicode_string_t *comment = NULL;
    if (nanoem_is_not_null(model)) {
        switch (language) {
        case NANOEM_LANGUAGE_TYPE_JAPANESE:
            comment = model->comment_ja;
            break;
        case NANOEM_LANGUAGE_TYPE_ENGLISH:
            comment = model->comment_en;
            break;
        case NANOEM_LANGUAGE_TYPE_MAX_ENUM:
        case NANOEM_LANGUAGE_TYPE_UNKNOWN:
        default:
            break;
        }
    }
    return comment;
}

nanoem_model_vertex_t *const *APIENTRY
nanoemModelGetAllVertexObjects(const nanoem_model_t *model, nanoem_rsize_t *num_vertices)
{
    nanoem_model_vertex_t *const *objects = NULL;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(num_vertices)) {
        *num_vertices = model->num_vertices;
        objects = model->vertices;
    }
    else if (nanoem_is_not_null(num_vertices))  {
        *num_vertices = 0;
    }
    return objects;
}

const nanoem_u32_t *APIENTRY
nanoemModelGetAllVertexIndices(const nanoem_model_t *model, nanoem_rsize_t *num_indices)
{
    const nanoem_u32_t *indices = NULL;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(num_indices)) {
        *num_indices = model->num_vertex_indices;
        indices = model->vertex_indices;
    }
    else if (nanoem_is_not_null(num_indices))  {
        *num_indices = 0;
    }
    return indices;
}

nanoem_model_material_t *const *APIENTRY
nanoemModelGetAllMaterialObjects(const nanoem_model_t *model, nanoem_rsize_t *num_materials)
{
    nanoem_model_material_t *const *objects = NULL;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(num_materials)) {
        *num_materials = model->num_materials;
        objects = model->materials;
    }
    else if (nanoem_is_not_null(num_materials))  {
        *num_materials = 0;
    }
    return objects;
}

nanoem_model_bone_t *const *APIENTRY
nanoemModelGetAllBoneObjects(const nanoem_model_t *model, nanoem_rsize_t *num_bones)
{
    nanoem_model_bone_t *const *objects = NULL;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(num_bones)) {
        *num_bones = model->num_bones;
        objects = model->bones;
    }
    else if (nanoem_is_not_null(num_bones))  {
        *num_bones = 0;
    }
    return objects;
}

nanoem_model_bone_t *const *APIENTRY
nanoemModelGetAllOrderedBoneObjects(const nanoem_model_t *model, nanoem_rsize_t *num_bones)
{
    nanoem_model_bone_t *const *objects = NULL;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(num_bones)) {
        *num_bones = model->num_bones;
        objects = model->ordered_bones;
    }
    else if (nanoem_is_not_null(num_bones))  {
        *num_bones = 0;
    }
    return objects;
}

nanoem_model_constraint_t *const *APIENTRY
nanoemModelGetAllConstraintObjects(const nanoem_model_t *model, nanoem_rsize_t *num_constraints)
{
    nanoem_model_constraint_t *const *objects = NULL;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(num_constraints)) {
        *num_constraints = model->num_constraints;
        objects = model->constraints;
    }
    else if (nanoem_is_not_null(num_constraints))  {
        *num_constraints = 0;
    }
    return objects;
}

nanoem_model_texture_t *const *APIENTRY
nanoemModelGetAllTextureObjects(const nanoem_model_t *model, nanoem_rsize_t *num_textures)
{
    nanoem_model_texture_t *const *objects = NULL;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(num_textures)) {
        *num_textures = model->num_textures;
        objects = model->textures;
    }
    else if (nanoem_is_not_null(num_textures))  {
        *num_textures = 0;
    }
    return objects;
}

nanoem_model_morph_t *const *APIENTRY
nanoemModelGetAllMorphObjects(const nanoem_model_t *model, nanoem_rsize_t *num_morphs)
{
    nanoem_model_morph_t *const *objects = NULL;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(num_morphs)) {
        *num_morphs = model->num_morphs;
        objects = model->morphs;
    }
    else if (nanoem_is_not_null(num_morphs))  {
        *num_morphs = 0;
    }
    return objects;
}

nanoem_model_label_t *const *APIENTRY
nanoemModelGetAllLabelObjects(const nanoem_model_t *model, nanoem_rsize_t *num_labels)
{
    nanoem_model_label_t *const *objects = NULL;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(num_labels)) {
        *num_labels = model->num_labels;
        objects = model->labels;
    }
    else if (nanoem_is_not_null(num_labels))  {
        *num_labels = 0;
    }
    return objects;
}

nanoem_model_rigid_body_t *const *APIENTRY
nanoemModelGetAllRigidBodyObjects(const nanoem_model_t *model, nanoem_rsize_t *num_rigid_bodies)
{
    nanoem_model_rigid_body_t *const *objects = NULL;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(num_rigid_bodies)) {
        *num_rigid_bodies = model->num_rigid_bodies;
        objects = model->rigid_bodies;
    }
    else if (nanoem_is_not_null(num_rigid_bodies))  {
        *num_rigid_bodies = 0;
    }
    return objects;
}

nanoem_model_joint_t *const *APIENTRY
nanoemModelGetAllJointObjects(const nanoem_model_t *model, nanoem_rsize_t *num_joints)
{
    nanoem_model_joint_t *const *objects = NULL;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(num_joints)) {
        *num_joints = model->num_joints;
        objects = model->joints;
    }
    else if (nanoem_is_not_null(num_joints))  {
        *num_joints = 0;
    }
    return objects;
}

nanoem_model_soft_body_t *const *APIENTRY
nanoemModelGetAllSoftBodyObjects(const nanoem_model_t *model, nanoem_rsize_t *num_soft_bodies)
{
    nanoem_model_soft_body_t *const *objects = NULL;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(num_soft_bodies)) {
        *num_soft_bodies = model->num_soft_bodies;
        objects = model->soft_bodies;
    }
    else if (nanoem_is_not_null(num_soft_bodies))  {
        *num_soft_bodies = 0;
    }
    return objects;
}

nanoem_user_data_t *APIENTRY
nanoemModelGetUserData(const nanoem_model_t *model)
{
    return nanoem_is_not_null(model) ? model->user_data : NULL;
}

void APIENTRY
nanoemModelSetUserData(nanoem_model_t *model, nanoem_user_data_t *user_data)
{
    if (nanoem_is_not_null(model)) {
        model->user_data = user_data;
    }
}

void APIENTRY
nanoemModelDestroy(nanoem_model_t *model)
{
    nanoem_unicode_string_factory_t *factory;
    nanoem_user_data_t *user_data;
    nanoem_rsize_t i, num_objects;
    if (nanoem_is_not_null(model)) {
        user_data = model->user_data;
        if (user_data && user_data->type == NANOEM_USER_DATA_DESTROY_CALLBACK_MODEL) {
            user_data->destroy.model(user_data->opaque, model);
        }
        nanoemUserDataDestroy(user_data);
        factory = model->factory;
        nanoemUtilDestroyString(model->name_ja, factory);
        nanoemUtilDestroyString(model->name_en, factory);
        nanoemUtilDestroyString(model->comment_ja, factory);
        nanoemUtilDestroyString(model->comment_en, factory);
        if (nanoem_is_not_null(model->vertices)) {
            num_objects = model->num_vertices;
            for (i = 0; i < num_objects; i++) {
                nanoemModelVertexDestroy(model->vertices[num_objects - i - 1]);
            }
            nanoem_free(model->vertices);
        }
        if (nanoem_is_not_null(model->vertex_indices)) {
            nanoem_free(model->vertex_indices);
        }
        if (nanoem_is_not_null(model->materials)) {
            num_objects = model->num_materials;
            for (i = 0; i < num_objects; i++) {
                nanoemModelMaterialDestroy(model->materials[num_objects - i - 1]);
            }
            nanoem_free(model->materials);
        }
        if (nanoem_is_not_null(model->bones)) {
            num_objects = model->num_bones;
            for (i = 0; i < num_objects; i++) {
                nanoemModelBoneDestroy(model->bones[num_objects - i - 1]);
            }
            nanoem_free(model->ordered_bones);
            nanoem_free(model->bones);
        }
        if (nanoem_is_not_null(model->morphs)) {
            num_objects = model->num_morphs;
            for (i = 0; i < num_objects; i++) {
                nanoemModelMorphDestroy(model->morphs[num_objects - i - 1]);
            }
            nanoem_free(model->morphs);
        }
        if (nanoem_is_not_null(model->labels)) {
            num_objects = model->num_labels;
            for (i = 0; i < num_objects; i++) {
                nanoemModelLabelDestroy(model->labels[num_objects - i - 1]);
            }
            nanoem_free(model->labels);
        }
        if (nanoem_is_not_null(model->constraints)) {
            num_objects = model->num_constraints;
            for (i = 0; i < num_objects; i++) {
                nanoemModelConstraintDestroy(model->constraints[num_objects - i - 1]);
            }
            nanoem_free(model->constraints);
        }
        if (nanoem_is_not_null(model->textures)) {
            num_objects = model->num_textures;
            for (i = 0; i < num_objects; i++) {
                nanoemModelTextureDestroy(model->textures[num_objects - i - 1]);
            }
            nanoem_free(model->textures);
        }
        /* due to dependency of rigid body, destroy joints first */
        if (nanoem_is_not_null(model->joints)) {
            num_objects = model->num_joints;
            for (i = 0; i < num_objects; i++) {
                nanoemModelJointDestroy(model->joints[num_objects - i - 1]);
            }
            nanoem_free(model->joints);
        }
        if (nanoem_is_not_null(model->rigid_bodies)) {
            num_objects = model->num_rigid_bodies;
            for (i = 0; i < num_objects; i++) {
                nanoemModelRigidBodyDestroy(model->rigid_bodies[num_objects - i - 1]);
            }
            nanoem_free(model->rigid_bodies);
        }
        if (nanoem_is_not_null(model->soft_bodies)) {
            num_objects = model->num_soft_bodies;
            for (i = 0; i < num_objects; i++) {
                nanoemModelSoftBodyDestroy(model->soft_bodies[num_objects - i - 1]);
            }
            nanoem_free(model->soft_bodies);
        }
        nanoem_free(model);
    }
}

void
nanoemMotionTrackBundleAddKeyframe(kh_motion_track_bundle_t *bundle, nanoem_motion_keyframe_object_t *keyframe, nanoem_frame_index_t frame_index, const nanoem_unicode_string_t *name, nanoem_unicode_string_factory_t *factory, int *ret)
{
    union nanoem_const_to_mutable_unicode_string_cast_t {
        const nanoem_unicode_string_t *s;
        nanoem_unicode_string_t *m;
    } u;
    kh_keyframe_map_t *keyframes;
    nanoem_motion_track_t track;
    khiter_t it, it2;
    if (nanoem_is_not_null(bundle) && nanoem_is_not_null(name) && nanoem_is_not_null(factory)) {
        u.s = name;
        track.factory = factory;
        track.id = 0;
        track.keyframes = NULL;
        track.name = u.m;
        it = kh_get_motion_track_bundle(bundle, track);
        if (it != kh_end(bundle)) {
            keyframes = kh_key(bundle, it).keyframes;
            it2 = kh_put_keyframe_map(keyframes, frame_index, ret);
            kh_val(keyframes, it2) = keyframe;
        }
    }
}

void
nanoemMotionTrackBundleDestroy(kh_motion_track_bundle_t *bundle, nanoem_unicode_string_factory_t *factory)
{
    khiter_t it, end;
    nanoem_unicode_string_t *name;
    if (nanoem_is_not_null(bundle)) {
        end = kh_end(bundle);
        for (it = kh_begin(bundle); it != end; it++) {
            if (kh_exist(bundle, it)) {
                name = kh_key(bundle, it).name;
                nanoemUtilDestroyString(name, factory);
                kh_destroy_keyframe_map(kh_key(bundle, it).keyframes);
            }
        }
        kh_destroy_motion_track_bundle(bundle);
    }
}

void
nanoemStringCacheDestroy(kh_string_cache_t *cache)
{
    khiter_t it, end;
    for (it = kh_begin(cache), end = kh_end(cache); it != end; it++) {
        if (kh_exist(cache, it)) {
            nanoem_free(kh_key(cache, it));
        }
    }
    kh_destroy_string_cache(cache);
}

static void
nanoemMotionParseBoneKeyframeBlockVMD(nanoem_motion_t *motion, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    kh_string_cache_t *cache;
    nanoem_motion_bone_keyframe_t *keyframe;
    nanoem_rsize_t num_bone_keyframes, i;
    num_bone_keyframes = nanoemBufferReadLength(buffer, status);
    if (num_bone_keyframes > 0) {
        cache = kh_init_string_cache();
        motion->bone_keyframes = (nanoem_motion_bone_keyframe_t **) nanoem_calloc(num_bone_keyframes, sizeof(*motion->bone_keyframes), status);
        if (nanoem_is_not_null(motion->bone_keyframes)) {
            motion->num_bone_keyframes = num_bone_keyframes;
            for (i = 0; i < num_bone_keyframes; i++) {
                keyframe = nanoemMotionBoneKeyframeCreate(motion, status);
                nanoemMotionBoneKeyframeParseVMD(keyframe, buffer, cache, offset, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemMotionBoneKeyframeDestroy(keyframe);
                    num_bone_keyframes = motion->num_bone_keyframes = i;
                    break;
                }
                nanoemMotionSetMaxFrameIndex(motion, &keyframe->base);
                keyframe->base.index = (int) i;
                motion->bone_keyframes[i] = keyframe;
            }
            if (!nanoem_status_ptr_has_error(status)) {
                nanoem_crt_qsort(motion->bone_keyframes, num_bone_keyframes, sizeof(*motion->bone_keyframes), nanoemMotionCompareKeyframe);
            }
        }
        nanoemStringCacheDestroy(cache);
    }
}

static void
nanoemMotionParseMorphKeyframeBlockVMD(nanoem_motion_t *motion, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    kh_string_cache_t *cache;
    nanoem_motion_morph_keyframe_t *keyframe;
    nanoem_rsize_t num_morph_keyframes, i;
    num_morph_keyframes = nanoemBufferReadLength(buffer, status);
    if (num_morph_keyframes > 0) {
        motion->morph_keyframes = (nanoem_motion_morph_keyframe_t **) nanoem_calloc(num_morph_keyframes, sizeof(*motion->morph_keyframes), status);
        cache = kh_init_string_cache();
        if (nanoem_is_not_null(motion->morph_keyframes)) {
            motion->num_morph_keyframes = num_morph_keyframes;
            for (i = 0; i < num_morph_keyframes; i++) {
                keyframe = nanoemMotionMorphKeyframeCreate(motion, status);
                nanoemMotionMorphKeyframeParseVMD(keyframe, buffer, cache, offset, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemMotionMorphKeyframeDestroy(keyframe);
                    num_morph_keyframes = motion->num_morph_keyframes = i;
                    break;
                }
                nanoemMotionSetMaxFrameIndex(motion, &keyframe->base);
                keyframe->base.index = (int) i;
                motion->morph_keyframes[i] = keyframe;
            }
            if (!nanoem_status_ptr_has_error(status)) {
                nanoem_crt_qsort(motion->morph_keyframes, num_morph_keyframes, sizeof(*motion->morph_keyframes), nanoemMotionCompareKeyframe);
            }
        }
        nanoemStringCacheDestroy(cache);
    }
}

static void
nanoemMotionParseCameraKeyframeBlockVMD(nanoem_motion_t *motion, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    nanoem_motion_camera_keyframe_t *keyframe;
    nanoem_rsize_t num_camera_keyframes, i;
    num_camera_keyframes = nanoemBufferReadLength(buffer, status);
    if (num_camera_keyframes > 0) {
        motion->camera_keyframes = (nanoem_motion_camera_keyframe_t **) nanoem_calloc(num_camera_keyframes, sizeof(*motion->camera_keyframes), status);
        if (nanoem_is_not_null(motion->camera_keyframes)) {
            motion->num_camera_keyframes = num_camera_keyframes;
            for (i = 0; i < num_camera_keyframes; i++) {
                keyframe = nanoemMotionCameraKeyframeCreate(motion, status);
                nanoemMotionCameraKeyframeParseVMD(keyframe, buffer, offset, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemMotionCameraKeyframeDestroy(keyframe);
                    num_camera_keyframes = motion->num_camera_keyframes = i;
                    break;
                }
                nanoemMotionSetMaxFrameIndex(motion, &keyframe->base);
                keyframe->base.index = (int) i;
                motion->camera_keyframes[i] = keyframe;
            }
            if (!nanoem_status_ptr_has_error(status)) {
                nanoem_crt_qsort(motion->camera_keyframes, num_camera_keyframes, sizeof(*motion->camera_keyframes), nanoemMotionCompareKeyframe);
            }
        }
    }
    else if (!nanoem_status_ptr_has_error(status) && nanoemBufferGetLength(buffer) - nanoemBufferGetOffset(buffer) == VMD_CAMERA_KEYFRAME_TWEAK_LENGTH) {
        nanoemBufferSkip(buffer, VMD_CAMERA_KEYFRAME_TWEAK_LENGTH, status);
    }
}

static void
nanoemMotionParseLightKeyframeBlockVMD(nanoem_motion_t *motion, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    nanoem_motion_light_keyframe_t *keyframe;
    nanoem_rsize_t num_light_keyframes, i;
    num_light_keyframes = nanoemBufferReadLength(buffer, status);
    if (num_light_keyframes > 0) {
        motion->light_keyframes = (nanoem_motion_light_keyframe_t **) nanoem_calloc(num_light_keyframes, sizeof(*motion->light_keyframes), status);
        if (nanoem_is_not_null(motion->light_keyframes)) {
            motion->num_light_keyframes = num_light_keyframes;
            for (i = 0; i < num_light_keyframes; i++) {
                keyframe = nanoemMotionLightKeyframeCreate(motion, status);
                nanoemMotionLightKeyframeParseVMD(keyframe, buffer, offset, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemMotionLightKeyframeDestroy(keyframe);
                    num_light_keyframes = motion->num_light_keyframes = i;
                    break;
                }
                nanoemMotionSetMaxFrameIndex(motion, &keyframe->base);
                keyframe->base.index = (int) i;
                motion->light_keyframes[i] = keyframe;
            }
            if (!nanoem_status_ptr_has_error(status)) {
                nanoem_crt_qsort(motion->light_keyframes, num_light_keyframes, sizeof(*motion->light_keyframes), nanoemMotionCompareKeyframe);
            }
        }
    }
    else if (!nanoem_status_ptr_has_error(status) && nanoemBufferGetLength(buffer) - nanoemBufferGetOffset(buffer) == VMD_LIGHT_KEYFRAME_TWEAK_LENGTH) {
        nanoemBufferSkip(buffer, VMD_LIGHT_KEYFRAME_TWEAK_LENGTH, status);
    }
}

static void
nanoemMotionParseSelfShadowKeyframeBlockVMD(nanoem_motion_t *motion, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    nanoem_motion_self_shadow_keyframe_t *keyframe;
    nanoem_rsize_t num_self_shadow_keyframes, i;
    num_self_shadow_keyframes = nanoemBufferReadLength(buffer, status);
    if (num_self_shadow_keyframes > 0) {
        motion->self_shadow_keyframes = (nanoem_motion_self_shadow_keyframe_t **) nanoem_calloc(num_self_shadow_keyframes, sizeof(*motion->self_shadow_keyframes), status);
        if (nanoem_is_not_null(motion->self_shadow_keyframes)) {
            motion->num_self_shadow_keyframes = num_self_shadow_keyframes;
            for (i = 0; i < num_self_shadow_keyframes; i++) {
                keyframe = nanoemMotionSelfShadowKeyframeCreate(motion, status);
                nanoemMotionSelfShadowKeyframeParseVMD(keyframe, buffer, offset, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemMotionSelfShadowKeyframeDestroy(keyframe);
                    num_self_shadow_keyframes = motion->num_self_shadow_keyframes = i;
                    break;
                }
                nanoemMotionSetMaxFrameIndex(motion, &keyframe->base);
                keyframe->base.index = (int) i;
                motion->self_shadow_keyframes[i] = keyframe;
            }
            if (!nanoem_status_ptr_has_error(status)) {
                nanoem_crt_qsort(motion->self_shadow_keyframes, num_self_shadow_keyframes, sizeof(*motion->self_shadow_keyframes), nanoemMotionCompareKeyframe);
            }
        }
    }
    else if (nanoem_status_ptr_has_error(status) && nanoemBufferGetLength(buffer) - nanoemBufferGetOffset(buffer) == VMD_SELF_SHADOW_KEYFRAME_TWEAK_LENGTH) {
        nanoemBufferSkip(buffer, VMD_SELF_SHADOW_KEYFRAME_TWEAK_LENGTH, status);
    }
}

static void
nanoemMotionParseModelKeyframeBlockVMD(nanoem_motion_t *motion, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    nanoem_motion_model_keyframe_t *keyframe;
    nanoem_rsize_t num_model_keyframes, i;
    num_model_keyframes = nanoemBufferReadLength(buffer, status);
    if (num_model_keyframes > 0) {
        motion->model_keyframes = (nanoem_motion_model_keyframe_t **) nanoem_calloc(num_model_keyframes, sizeof(*motion->model_keyframes), status);
        if (nanoem_is_not_null(motion->model_keyframes)) {
            motion->num_model_keyframes = num_model_keyframes;
            for (i = 0; i < num_model_keyframes; i++) {
                keyframe = nanoemMotionModelKeyframeCreate(motion, status);
                nanoemMotionModelKeyframeParseVMD(keyframe, buffer, offset, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemMotionModelKeyframeDestroy(keyframe);
                    num_model_keyframes = motion->num_model_keyframes = i;
                    break;
                }
                nanoemMotionSetMaxFrameIndex(motion, &keyframe->base);
                keyframe->base.index = (int) i;
                motion->model_keyframes[i] = keyframe;
            }
            if (!nanoem_status_ptr_has_error(status)) {
                nanoem_crt_qsort(motion->model_keyframes, num_model_keyframes, sizeof(*motion->model_keyframes), nanoemMotionCompareKeyframe);
            }
        }
    }
}

static nanoem_bool_t
nanoemMotionParseVMD(nanoem_motion_t *motion, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    nanoemMotionParseBoneKeyframeBlockVMD(motion, buffer, offset, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    nanoemMotionParseMorphKeyframeBlockVMD(motion, buffer, offset, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    else if (nanoemBufferIsEnd(buffer)) {
        return nanoem_true;
    }
    nanoemMotionParseCameraKeyframeBlockVMD(motion, buffer, offset, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    else if (nanoemBufferIsEnd(buffer)) {
        return nanoem_true;
    }
    nanoemMotionParseLightKeyframeBlockVMD(motion, buffer, offset, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    else if (nanoemBufferIsEnd(buffer)) {
        return nanoem_true;
    }
    nanoemMotionParseSelfShadowKeyframeBlockVMD(motion, buffer, offset, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    else if (nanoemBufferIsEnd(buffer)) {
        return nanoem_true;
    }
    nanoemMotionParseModelKeyframeBlockVMD(motion, buffer, offset, status);
    if (nanoem_status_ptr_has_error(status)) {
        return nanoem_false;
    }
    return nanoemBufferIsEnd(buffer);
}

static void
nanoemMotionKeyframeObjectDestroy(nanoem_motion_keyframe_object_t *object)
{
    nanoem_user_data_t *user_data = object->user_data;
    if (user_data && user_data->type == NANOEM_USER_DATA_DESTROY_CALLBACK_KEYFRAME_OBJECT) {
        user_data->destroy.keyframe_object(user_data->opaque, object);
    }
    nanoemUserDataDestroy(user_data);
    object->frame_index = 0;
    object->index = NANOEM_MOTION_OBJECT_NOT_FOUND;
    object->parent_motion = NULL;
}

nanoem_motion_effect_parameter_t *
nanoemMotionEffectParameterCreateFromAccessoryKeyframe(nanoem_motion_accessory_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_motion_effect_parameter_t *parameter;
    parameter = (nanoem_motion_effect_parameter_t *) nanoem_calloc(1, sizeof(*parameter), status);
    if (nanoem_is_not_null(parameter)) {
        parameter->value_type = NANOEM_MOTION_EFFECT_PARAMETER_TYPE_UNKNOWN;
        parameter->keyframe_type = NANOEM_PARENT_KEYFRAME_TYPE_ACCESSORY;
        parameter->keyframe.accessory = keyframe;
    }
    return parameter;
}

nanoem_motion_effect_parameter_t *
nanoemMotionEffectParameterCreateFromModelKeyframe(nanoem_motion_model_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_motion_effect_parameter_t *parameter;
    parameter = (nanoem_motion_effect_parameter_t *) nanoem_calloc(1, sizeof(*parameter), status);
    if (nanoem_is_not_null(parameter)) {
        parameter->value_type = NANOEM_MOTION_EFFECT_PARAMETER_TYPE_UNKNOWN;
        parameter->keyframe_type = NANOEM_PARENT_KEYFRAME_TYPE_MODEL;
        parameter->keyframe.model = keyframe;
    }
    return parameter;
}

void
nanoemMotionEffectParameterDestroy(nanoem_motion_effect_parameter_t *parameter)
{
    if (nanoem_is_not_null(parameter)) {
        nanoem_free(parameter);
    }
}

nanoem_motion_accessory_keyframe_t *
nanoemMotionAccessoryKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status)
{
    nanoem_motion_accessory_keyframe_t *keyframe = NULL;
    if (nanoem_is_not_null(motion)) {
        keyframe = (nanoem_motion_accessory_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
        if (nanoem_is_not_null(keyframe)) {
            keyframe->base.parent_motion = motion;
            keyframe->is_shadow_enabled = nanoem_true;
            keyframe->visible = nanoem_true;
        }
    }
    return keyframe;
}

nanoem_motion_outside_parent_t *
nanoemMotionOutsideParentCreateFromAccessoryKeyframe(nanoem_motion_accessory_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_motion_outside_parent_t *op;
    op = (nanoem_motion_outside_parent_t *) nanoem_calloc(1, sizeof(*op), status);
    if (nanoem_is_not_null(op)) {
        op->keyframe.accessory = keyframe;
        op->keyframe_type = NANOEM_PARENT_KEYFRAME_TYPE_ACCESSORY;
    }
    return op;
}

nanoem_motion_outside_parent_t *
nanoemMotionOutsideParentCreateFromCameraKeyframe(nanoem_motion_camera_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_motion_outside_parent_t *op;
    op = (nanoem_motion_outside_parent_t *) nanoem_calloc(1, sizeof(*op), status);
    if (nanoem_is_not_null(op)) {
        op->keyframe.camera = keyframe;
        op->keyframe_type = NANOEM_PARENT_KEYFRAME_TYPE_CAMERA;
    }
    return op;
}

nanoem_motion_outside_parent_t *
nanoemMotionOutsideParentCreateFromModelKeyframe(nanoem_motion_model_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_motion_outside_parent_t *op;
    op = (nanoem_motion_outside_parent_t *) nanoem_calloc(1, sizeof(*op), status);
    if (nanoem_is_not_null(op)) {
        op->keyframe.model = keyframe;
        op->keyframe_type = NANOEM_PARENT_KEYFRAME_TYPE_MODEL;
    }
    return op;
}

void
nanoemMotionOutsideParentDestroy(nanoem_motion_outside_parent_t *op)
{
    if (nanoem_is_not_null(op)) {
        nanoem_free(op);
    }
}

void
nanoemMotionAccessoryKeyframeDestroy(nanoem_motion_accessory_keyframe_t *keyframe)
{
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(keyframe)) {
        nanoemMotionOutsideParentDestroy(keyframe->outside_parent);
        num_objects = keyframe->num_effect_parameters;
        for (i = 0; i < num_objects; i++) {
            nanoem_free(keyframe->effect_parameters[num_objects - i - 1]);
        }
        nanoem_free(keyframe->effect_parameters);
        nanoemMotionKeyframeObjectDestroy(&keyframe->base);
        nanoem_free(keyframe);
    }
}

nanoem_motion_bone_keyframe_t *
nanoemMotionBoneKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status)
{
    nanoem_motion_bone_keyframe_t *keyframe = NULL;
    nanoem_rsize_t j;
    int i;
    if (nanoem_is_not_null(motion)) {
        keyframe = (nanoem_motion_bone_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
        if (nanoem_is_not_null(keyframe)) {
            keyframe->base.parent_motion = motion;
            keyframe->is_physics_simulation_enabled = nanoem_true;
            for (i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM; i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
                for (j = 0; j < sizeof(__nanoem_default_interpolation) / sizeof(__nanoem_default_interpolation[0]); j++) {
                    keyframe->interplation[i].u.values[j] = __nanoem_default_interpolation[j];
                }
            }
        }
    }
    return keyframe;
}

void
nanoemMotionBoneKeyframeParseVMD(nanoem_motion_bone_keyframe_t *keyframe, nanoem_buffer_t *buffer, kh_string_cache_t *cache, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    const nanoem_motion_t *motion;
    nanoem_unicode_string_factory_t *factory;
    nanoem_unicode_string_t *name, *found_name;
    nanoem_rsize_t i, length;
    khiter_t it;
    int j, ret = 0;
    const char *buffer_ptr;
    char str[VMD_BONE_KEYFRAME_NAME_LENGTH + 1];
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(buffer)) {
        motion = keyframe->base.parent_motion;
        factory = motion->factory;
        buffer_ptr = (const char *) nanoemBufferGetDataPtr(buffer);
        if (buffer_ptr) {
            length = nanoemBufferGetLength(buffer) - nanoemBufferGetOffset(buffer);
            nanoemUtilCopyString(str, sizeof(str), buffer_ptr, length >= VMD_BONE_KEYFRAME_NAME_LENGTH ? VMD_BONE_KEYFRAME_NAME_LENGTH : length);
            if (*str) {
                it = kh_get_string_cache(cache, str);
                if (it != kh_end(cache)) {
                    nanoemBufferSkip(buffer, VMD_BONE_KEYFRAME_NAME_LENGTH, status);
                    name = nanoemMotionTrackBundleResolveName(keyframe->base.parent_motion->local_bone_motion_track_bundle, kh_val(cache, it));
                    keyframe->bone_id = kh_val(cache, it);
                }
                else {
                    name = nanoemBufferGetStringFromCp932(buffer, VMD_BONE_KEYFRAME_NAME_LENGTH, factory, status);
                    keyframe->bone_id = nanoemMotionResolveLocalBoneTrackId(keyframe->base.parent_motion, name, &found_name, &ret);
                    if (nanoem_unlikely(found_name)) {
                        nanoemUtilDestroyString(name, factory);
                        name = found_name;
                    }
                    if (ret < 0) {
                        return;
                    }
                    it = kh_put_string_cache(cache, nanoemUtilCloneString(str, status), &ret);
                    if (ret < 0) {
                        return;
                    }
                    kh_val(cache, it) = keyframe->bone_id;
                }
            }
            else {
                nanoemBufferSkip(buffer, VMD_BONE_KEYFRAME_NAME_LENGTH, status);
                name = NULL;
            }
            keyframe->base.frame_index = nanoemBufferReadInt32LittleEndian(buffer, status) + offset;
            nanoemBufferReadFloat32x3LittleEndian(buffer, &keyframe->translation, status);
            nanoemBufferReadFloat32x4LittleEndian(buffer, &keyframe->orientation, status);
            for (i = 0; i < 4; i++) {
                for (j = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM; j < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; j++) {
                    keyframe->interplation[j].u.values[i] = nanoemBufferReadByte(buffer, status);
                }
            }
            nanoemBufferSkip(buffer, 48, status);
            if (!nanoem_status_ptr_has_error(status)) {
                nanoemMotionTrackBundleAddKeyframe(motion->local_bone_motion_track_bundle, (nanoem_motion_keyframe_object_t *) keyframe, keyframe->base.frame_index, name, factory, &ret);
                nanoem_status_ptr_assign(status, ret >= 0 ? NANOEM_STATUS_SUCCESS : NANOEM_STATUS_ERROR_MALLOC_FAILED);
            }
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_CORRUPTED);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemMotionBoneKeyframeDestroy(nanoem_motion_bone_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        nanoemMotionKeyframeObjectDestroy(&keyframe->base);
        nanoem_free(keyframe);
    }
}

nanoem_motion_camera_keyframe_t *
nanoemMotionCameraKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status)
{
    nanoem_motion_camera_keyframe_t *keyframe = NULL;
    nanoem_rsize_t j;
    int i;
    if (nanoem_is_not_null(motion)) {
        keyframe = (nanoem_motion_camera_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
        if (nanoem_is_not_null(keyframe)) {
            keyframe->base.parent_motion = motion;
            for (i = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM; i < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
                for (j = 0; j < sizeof(__nanoem_default_interpolation) / sizeof(__nanoem_default_interpolation[0]); j++) {
                    keyframe->interplation[i].u.values[j] = __nanoem_default_interpolation[j];
                }
            }
        }
    }
    return keyframe;
}

void
nanoemMotionCameraKeyframeParseVMD(nanoem_motion_camera_keyframe_t *keyframe, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    nanoem_rsize_t i;
    int j;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(buffer)) {
        keyframe->base.frame_index = nanoemBufferReadInt32LittleEndian(buffer, status) + offset;
        keyframe->distance = nanoemBufferReadFloat32LittleEndian(buffer, status);
        nanoemBufferReadFloat32x3LittleEndian(buffer, &keyframe->look_at, status);
        nanoemBufferReadFloat32x3LittleEndian(buffer, &keyframe->angle, status);
        for (i = 0; i < 4; i++) {
            for (j = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM; j < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; j++) {
                keyframe->interplation[j].u.values[i] = nanoemBufferReadByte(buffer, status);
            }
        }
        keyframe->fov = nanoemBufferReadInt32LittleEndian(buffer, status);
        keyframe->is_perspective_view = nanoemBufferReadByte(buffer, status) == 0;
        nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_CORRUPTED);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemMotionCameraKeyframeDestroy(nanoem_motion_camera_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        nanoemMotionOutsideParentDestroy(keyframe->outside_parent);
        nanoemMotionKeyframeObjectDestroy(&keyframe->base);
        nanoem_free(keyframe);
    }
}

nanoem_motion_light_keyframe_t *
nanoemMotionLightKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status)
{
    nanoem_motion_light_keyframe_t *keyframe = NULL;
    if (nanoem_is_not_null(motion)) {
        keyframe = (nanoem_motion_light_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
        if (nanoem_is_not_null(keyframe)) {
            keyframe->base.parent_motion = motion;
        }
    }
    return keyframe;
}

void
nanoemMotionLightKeyframeParseVMD(nanoem_motion_light_keyframe_t *keyframe, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(buffer)) {
        keyframe->base.frame_index = nanoemBufferReadInt32LittleEndian(buffer, status) + offset;
        nanoemBufferReadFloat32x3LittleEndian(buffer, &keyframe->color, status);
        nanoemBufferReadFloat32x3LittleEndian(buffer, &keyframe->direction, status);
        nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_CORRUPTED);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemMotionLightKeyframeDestroy(nanoem_motion_light_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        nanoemMotionKeyframeObjectDestroy(&keyframe->base);
        nanoem_free(keyframe);
    }
}

nanoem_motion_model_keyframe_t *
nanoemMotionModelKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status)
{
    nanoem_motion_model_keyframe_t *keyframe = NULL;
    if (nanoem_is_not_null(motion)) {
        keyframe = (nanoem_motion_model_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
        if (nanoem_is_not_null(keyframe)) {
            keyframe->base.parent_motion = motion;
            keyframe->is_physics_simulation_enabled = nanoem_true;
        }
    }
    return keyframe;
}

void
nanoemMotionModelKeyframeParseVMD(nanoem_motion_model_keyframe_t *keyframe, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    nanoem_motion_t *motion;
    nanoem_unicode_string_factory_t *factory;
    nanoem_motion_model_keyframe_constraint_state_t *state;
    nanoem_unicode_string_t *name, *found_name;
    nanoem_rsize_t num_constraint_states, i;
    int bone_id, ret = 0;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(buffer)) {
        motion = keyframe->base.parent_motion;
        keyframe->base.frame_index = nanoemBufferReadInt32LittleEndian(buffer, status) + offset;
        keyframe->visible = nanoemBufferReadByte(buffer, status) != 0;
        num_constraint_states = nanoemBufferReadLength(buffer, status);
        if (num_constraint_states > 0) {
            keyframe->constraint_states = (nanoem_motion_model_keyframe_constraint_state_t **) nanoem_calloc(num_constraint_states, sizeof(*keyframe->constraint_states), status);
            if (nanoem_is_not_null(keyframe->constraint_states)) {
                factory = keyframe->base.parent_motion->factory;
                keyframe->num_constraint_states = num_constraint_states;
                for (i = 0; i < num_constraint_states; i++) {
                    state = (nanoem_motion_model_keyframe_constraint_state_t *) nanoem_calloc(1, sizeof(*state), status);
                    if (nanoem_is_not_null(state)) {
                        name = nanoemBufferGetStringFromCp932(buffer, PMD_BONE_NAME_LENGTH, factory, status);
                        bone_id = nanoemMotionResolveLocalBoneTrackId(motion, name, &found_name, &ret);
                        if (found_name) {
                            nanoemUtilDestroyString(name, factory);
                        }
                        if (ret < 0) {
                            nanoem_free(state);
                            keyframe->num_constraint_states = i;
                            break;
                        }
                        state->parent_keyframe = keyframe;
                        state->bone_id = bone_id;
                        state->enabled = nanoemBufferReadByte(buffer, status);
                        keyframe->constraint_states[i] = state;
                    }
                    else {
                        keyframe->num_constraint_states = i;
                        break;
                    }
                }
                if (i == num_constraint_states) {
                    nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_CORRUPTED);
                }
            }
        }
        else {
            nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_CORRUPTED);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemMotionModelKeyframeDestroy(nanoem_motion_model_keyframe_t *keyframe)
{
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(keyframe)) {
        if (nanoem_is_not_null(keyframe->constraint_states)) {
            num_objects = keyframe->num_constraint_states;
            for (i = 0; i < num_objects; i++) {
                nanoem_free(keyframe->constraint_states[num_objects - i - 1]);
            }
            nanoem_free(keyframe->constraint_states);
        }
        if (nanoem_is_not_null(keyframe->outside_parents)) {
            num_objects = keyframe->num_outside_parents;
            for (i = 0; i < num_objects; i++) {
                nanoemMotionOutsideParentDestroy(keyframe->outside_parents[num_objects - i - 1]);
            }
            nanoem_free(keyframe->outside_parents);
        }
        if (nanoem_is_not_null(keyframe->effect_parameters)) {
            num_objects = keyframe->num_effect_parameters;
            for (i = 0; i < num_objects; i++) {
                nanoemMotionEffectParameterDestroy(keyframe->effect_parameters[num_objects - i - 1]);
            }
            nanoem_free(keyframe->effect_parameters);
        }
        nanoemMotionKeyframeObjectDestroy(&keyframe->base);
        nanoem_free(keyframe);
    }
}

nanoem_motion_morph_keyframe_t *
nanoemMotionMorphKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status)
{
    nanoem_motion_morph_keyframe_t *keyframe = NULL;
    if (nanoem_is_not_null(motion)) {
        keyframe = (nanoem_motion_morph_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
        if (nanoem_is_not_null(keyframe)) {
            keyframe->base.parent_motion = motion;
        }
    }
    return keyframe;
}

void
nanoemMotionMorphKeyframeParseVMD(nanoem_motion_morph_keyframe_t *keyframe, nanoem_buffer_t *buffer, kh_string_cache_t *cache, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    nanoem_motion_t *motion;
    nanoem_unicode_string_factory_t *factory;
    nanoem_unicode_string_t *name, *found_name;
    nanoem_rsize_t length;
    khiter_t it;
    const char *buffer_ptr;
    char str[VMD_MORPH_KEYFRAME_NAME_LENGTH + 1];
    int ret = 0;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(buffer)) {
        motion = keyframe->base.parent_motion;
        factory = motion->factory;
        buffer_ptr = (const char *) nanoemBufferGetDataPtr(buffer);
        if (buffer_ptr) {
            length = nanoemBufferGetLength(buffer) - nanoemBufferGetOffset(buffer);
            nanoemUtilCopyString(str, sizeof(str), buffer_ptr, length >= VMD_MORPH_KEYFRAME_NAME_LENGTH ? VMD_MORPH_KEYFRAME_NAME_LENGTH : length);
            if (*str) {
                it = kh_get_string_cache(cache, str);
                if (it != kh_end(cache)) {
                    nanoemBufferSkip(buffer, VMD_MORPH_KEYFRAME_NAME_LENGTH, status);
                    name = nanoemMotionTrackBundleResolveName(motion->local_morph_motion_track_bundle, kh_val(cache, it));
                    keyframe->morph_id = kh_val(cache, it);
                }
                else {
                    name = nanoemBufferGetStringFromCp932(buffer, VMD_MORPH_KEYFRAME_NAME_LENGTH, factory, status);
                    keyframe->morph_id = nanoemMotionResolveLocalMorphTrackId(motion, name, &found_name, &ret);
                    if (nanoem_unlikely(found_name)) {
                        nanoemUtilDestroyString(name, factory);
                        name = found_name;
                    }
                    if (ret < 0) {
                        return;
                    }
                    it = kh_put_string_cache(cache, nanoemUtilCloneString(str, status), &ret);
                    if (ret < 0) {
                        return;
                    }
                    kh_val(cache, it) = keyframe->morph_id;
                }
            }
            else {
                nanoemBufferSkip(buffer, VMD_MORPH_KEYFRAME_NAME_LENGTH, status);
                name = NULL;
            }
            keyframe->base.frame_index = nanoemBufferReadInt32LittleEndian(buffer, status) + offset;
            keyframe->weight = nanoemBufferReadFloat32LittleEndian(buffer, status);
            if (!nanoem_status_ptr_has_error(status)) {
                nanoemMotionTrackBundleAddKeyframe(motion->local_morph_motion_track_bundle, (nanoem_motion_keyframe_object_t *) keyframe, keyframe->base.frame_index, name, factory, &ret);
                nanoem_status_ptr_assign(status, ret >= 0 ? NANOEM_STATUS_SUCCESS : NANOEM_STATUS_ERROR_MALLOC_FAILED);
            }
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_CORRUPTED);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemMotionMorphKeyframeDestroy(nanoem_motion_morph_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        nanoemMotionKeyframeObjectDestroy(&keyframe->base);
        nanoem_free(keyframe);
    }
}

nanoem_motion_self_shadow_keyframe_t *
nanoemMotionSelfShadowKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status)
{
    nanoem_motion_self_shadow_keyframe_t *keyframe = NULL;
    if (nanoem_is_not_null(motion)) {
        keyframe = (nanoem_motion_self_shadow_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
        if (nanoem_is_not_null(keyframe)) {
            keyframe->base.parent_motion = motion;
        }
    }
    return keyframe;
}

void
nanoemMotionSelfShadowKeyframeParseVMD(nanoem_motion_self_shadow_keyframe_t *keyframe, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(buffer)) {
        keyframe->base.frame_index = nanoemBufferReadInt32LittleEndian(buffer, status) + offset;
        keyframe->mode = nanoemBufferReadByte(buffer, status);
        keyframe->distance = nanoemBufferReadFloat32LittleEndian(buffer, status);
        nanoem_status_ptr_assign_select(status, NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_CORRUPTED);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void
nanoemMotionSelfShadowKeyframeDestroy(nanoem_motion_self_shadow_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        nanoemMotionKeyframeObjectDestroy(&keyframe->base);
        nanoem_free(keyframe);
    }
}

const nanoem_motion_t *APIENTRY
nanoemMotionKeyframeObjectGetParentMotion(const nanoem_motion_keyframe_object_t *object)
{
    return nanoem_is_not_null(object) ? object->parent_motion : NULL;
}

int APIENTRY
nanoemMotionKeyframeObjectGetIndex(const nanoem_motion_keyframe_object_t *object)
{
    return nanoem_is_not_null(object) ? object->index : NANOEM_MOTION_OBJECT_NOT_FOUND;
}

nanoem_frame_index_t APIENTRY
nanoemMotionKeyframeObjectGetFrameIndex(const nanoem_motion_keyframe_object_t *object)
{
    return nanoem_is_not_null(object) ? object->frame_index : 0;
}

nanoem_frame_index_t APIENTRY
nanoemMotionKeyframeObjectGetFrameIndexWithOffset(const nanoem_motion_keyframe_object_t *object, int offset)
{
    nanoem_frame_index_t frame_index = nanoemMotionKeyframeObjectGetFrameIndex(object);
    if (offset > 0 && frame_index + offset < frame_index) {
        frame_index = ~((nanoem_frame_index_t) 0);
    }
    else if (offset < 0 && frame_index + offset > frame_index) {
        frame_index = 0;
    }
    else {
        frame_index += offset;
    }
    return frame_index;
}

nanoem_bool_t APIENTRY
nanoemMotionKeyframeObjectIsSelected(const nanoem_motion_keyframe_object_t *object)
{
    return nanoem_is_not_null(object) ? object->is_selected : nanoem_false;
}

void APIENTRY
nanoemMotionKeyframeObjectSetSelected(nanoem_motion_keyframe_object_t *object, nanoem_bool_t value)
{
    if (nanoem_is_not_null(object)) {
        object->is_selected = value ? nanoem_true : nanoem_false;
    }
}

nanoem_user_data_t *APIENTRY
nanoemMotionKeyframeObjectGetUserDataObject(const nanoem_motion_keyframe_object_t *object)
{
    return nanoem_is_not_null(object) ? object->user_data : NULL;
}

void APIENTRY
nanoemMotionKeyframeObjectSetUserDataObject(nanoem_motion_keyframe_object_t *object, nanoem_user_data_t *user_data)
{
    if (nanoem_is_not_null(object)) {
        object->user_data = user_data;
    }
}

const char *APIENTRY
nanoemMotionKeyframeObjectGetAnnotation(const nanoem_motion_keyframe_object_t *object, const char *key)
{
    return nanoem_is_not_null(object) ? nanoemAnnotationGet(object->annotations, key) : NULL;
}

void APIENTRY
nanoemMotionKeyframeObjectSetAnnotation(nanoem_motion_keyframe_object_t *object, const char *key, const char *value, nanoem_status_t *status)
{
    int ret;
    if (nanoem_is_not_null(object)) {
        nanoemAnnotationSet(&object->annotations, key, value, &ret, status);
    }
}

const nanoem_motion_t *APIENTRY
nanoemMotionEffectParameterGetParentMotion(const nanoem_motion_effect_parameter_t *parameter)
{
    const nanoem_motion_t *parent_motion = NULL;
    if (nanoem_is_not_null(parameter)) {
        switch (parameter->keyframe_type) {
        case NANOEM_PARENT_KEYFRAME_TYPE_ACCESSORY:
            parent_motion = parameter->keyframe.accessory->base.parent_motion;
            break;
        case NANOEM_PARENT_KEYFRAME_TYPE_MODEL:
            parent_motion = parameter->keyframe.model->base.parent_motion;
            break;
        default:
            break;
        }
    }
    return parent_motion;
}

const nanoem_unicode_string_t *APIENTRY
nanoemMotionEffectParameterGetName(const nanoem_motion_effect_parameter_t *parameter)
{
    const nanoem_motion_t *motion;
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(parameter)) {
        motion = nanoemMotionEffectParameterGetParentMotion(parameter);
        name = nanoemMotionTrackBundleResolveName(motion->global_motion_track_bundle, parameter->parameter_id);
    }
    return name;
}

nanoem_motion_effect_parameter_type_t APIENTRY
nanoemMotionEffectParameterGetType(const nanoem_motion_effect_parameter_t *parameter)
{
    return nanoem_is_not_null(parameter) ? parameter->value_type : NANOEM_MOTION_EFFECT_PARAMETER_TYPE_UNKNOWN;
}

const void *APIENTRY
nanoemMotionEffectParameterGetValue(const nanoem_motion_effect_parameter_t *parameter)
{
    switch (parameter->value_type) {
    case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_INT:
    case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_BOOL:
        return &parameter->value.i;
        break;
    case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_FLOAT:
        return &parameter->value.f;
    case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_VECTOR4:
        return parameter->value.v.values;
    default:
        return NULL;
    }
}

const nanoem_motion_t *APIENTRY
nanoemMotionOutsideParentGetParentMotion(const nanoem_motion_outside_parent_t *op)
{
    const nanoem_motion_t *parent_motion = NULL;
    if (nanoem_is_not_null(op)) {
        switch (op->keyframe_type) {
        case NANOEM_PARENT_KEYFRAME_TYPE_ACCESSORY:
            parent_motion = op->keyframe.accessory->base.parent_motion;
            break;
        case NANOEM_PARENT_KEYFRAME_TYPE_MODEL:
            parent_motion = op->keyframe.model->base.parent_motion;
            break;
        default:
            break;
        }
    }
    return parent_motion;
}

const nanoem_unicode_string_t *APIENTRY
nanoemMotionOutsideParentGetTargetBoneName(const nanoem_motion_outside_parent_t *op)
{
    const nanoem_motion_t *parent_motion = nanoemMotionOutsideParentGetParentMotion(op);
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(parent_motion)) {
        name = nanoemMotionTrackBundleResolveName(parent_motion->global_motion_track_bundle, op->global_bone_track_index);
    }
    return name;
}

const nanoem_unicode_string_t *APIENTRY
nanoemMotionOutsideParentGetTargetObjectName(const nanoem_motion_outside_parent_t *op)
{
    const nanoem_motion_t *parent_motion = nanoemMotionOutsideParentGetParentMotion(op);
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(parent_motion)) {
        name = nanoemMotionTrackBundleResolveName(parent_motion->global_motion_track_bundle, op->global_model_track_index);
    }
    return name;
}

const nanoem_unicode_string_t *APIENTRY
nanoemMotionOutsideParentGetSubjectBoneName(const nanoem_motion_outside_parent_t *op)
{
    const nanoem_motion_t *parent_motion = nanoemMotionOutsideParentGetParentMotion(op);
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(parent_motion)) {
        name = nanoemMotionTrackBundleResolveName(parent_motion->global_motion_track_bundle, op->local_bone_track_index);
    }
    return name;
}

const nanoem_f32_t *APIENTRY
nanoemMotionAccessoryKeyframeGetTranslation(const nanoem_motion_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->translation.values : __nanoem_null_vector3;
}

const nanoem_f32_t *APIENTRY
nanoemMotionAccessoryKeyframeGetOrientation(const nanoem_motion_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->orientation.values : __nanoem_null_vector4;
}

nanoem_f32_t APIENTRY
nanoemMotionAccessoryKeyframeGetScaleFactor(const nanoem_motion_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->scale_factor : 0.0f;
}

nanoem_f32_t APIENTRY
nanoemMotionAccessoryKeyframeGetOpacity(const nanoem_motion_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->opacity : 0.0f;
}

nanoem_bool_t APIENTRY
nanoemMotionAccessoryKeyframeIsVisible(const nanoem_motion_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->visible : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemMotionAccessoryKeyframeIsAddBlendEnabled(const nanoem_motion_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->is_add_blending_enabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemMotionAccessoryKeyframeIsShadowEnabled(const nanoem_motion_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->is_shadow_enabled : nanoem_false;
}

const nanoem_motion_outside_parent_t *APIENTRY
nanoemMotionAccessoryKeyframeGetOutsideParent(const nanoem_motion_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->outside_parent : NULL;
}

nanoem_motion_outside_parent_t *APIENTRY
nanoemMotionAccessoryKeyframeGetOutsideParentMutable(nanoem_motion_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->outside_parent : NULL;
}

nanoem_motion_effect_parameter_t *const *APIENTRY
nanoemMotionAccessoryKeyframeGetAllEffectParameterObjects(const nanoem_motion_accessory_keyframe_t *keyframe, nanoem_rsize_t *num_objects)
{
    nanoem_motion_effect_parameter_t *const *objects = NULL;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(num_objects)) {
        *num_objects = keyframe->num_effect_parameters;
        objects = keyframe->effect_parameters;
    }
    else if (nanoem_is_not_null(num_objects))  {
        *num_objects = 0;
    }
    return objects;
}

const nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionAccessoryKeyframeGetKeyframeObject(const nanoem_motion_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionAccessoryKeyframeGetKeyframeObjectMutable(nanoem_motion_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

const nanoem_unicode_string_t *APIENTRY
nanoemMotionBoneKeyframeGetName(const nanoem_motion_bone_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? nanoemMotionTrackBundleResolveName(keyframe->base.parent_motion->local_bone_motion_track_bundle, keyframe->bone_id) : NULL;
}

const nanoem_f32_t *APIENTRY
nanoemMotionBoneKeyframeGetTranslation(const nanoem_motion_bone_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->translation.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemMotionBoneKeyframeGetOrientation(const nanoem_motion_bone_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->orientation.values : __nanoem_null_vector4;
}

int APIENTRY
nanoemMotionBoneKeyframeGetId(const nanoem_motion_bone_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->bone_id : 0;
}

const nanoem_u8_t *APIENTRY
nanoemMotionBoneKeyframeGetInterpolation(const nanoem_motion_bone_keyframe_t *keyframe, nanoem_motion_bone_keyframe_interpolation_type_t index)
{
    const nanoem_u8_t *parameters = __nanoem_default_interpolation;
    if (nanoem_is_not_null(keyframe) && index > NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_UNKNOWN && index < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM) {
        parameters = keyframe->interplation[index].u.values;
    }
    return parameters;
}

nanoem_bool_t APIENTRY
nanoemMotionBoneKeyframeIsLinearInterpolation(const nanoem_motion_bone_keyframe_t *keyframe, nanoem_motion_bone_keyframe_interpolation_type_t index)
{
    nanoem_bool_t result = nanoem_true;
    if (nanoem_is_not_null(keyframe) && index > NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_UNKNOWN && index < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM) {
        const nanoem_u8_t *v = keyframe->interplation[index].u.values;
        result = v[0] == v[1] && v[2] == v[3] && v[0] + v[2] == v[1] + v[3];
    }
    return result;
}

nanoem_u32_t APIENTRY
nanoemMotionBoneKeyframeGetStageIndex(const nanoem_motion_bone_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->stage_index : 0;
}

nanoem_bool_t APIENTRY
nanoemMotionBoneKeyframeIsPhysicsSimulationEnabled(const nanoem_motion_bone_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->is_physics_simulation_enabled : nanoem_false;
}

const nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionBoneKeyframeGetKeyframeObject(const nanoem_motion_bone_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionBoneKeyframeGetKeyframeObjectMutable(nanoem_motion_bone_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

const nanoem_f32_t *APIENTRY
nanoemMotionCameraKeyframeGetLookAt(const nanoem_motion_camera_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->look_at.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemMotionCameraKeyframeGetAngle(const nanoem_motion_camera_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->angle.values : __nanoem_null_vector4;
}

nanoem_f32_t APIENTRY
nanoemMotionCameraKeyframeGetDistance(const nanoem_motion_camera_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->distance : 0;
}

int APIENTRY
nanoemMotionCameraKeyframeGetFov(const nanoem_motion_camera_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->fov : 0;
}

nanoem_bool_t APIENTRY
nanoemMotionCameraKeyframeIsPerspectiveView(const nanoem_motion_camera_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->is_perspective_view : nanoem_false;
}

const nanoem_u8_t *APIENTRY
nanoemMotionCameraKeyframeGetInterpolation(const nanoem_motion_camera_keyframe_t *keyframe, nanoem_motion_camera_keyframe_interpolation_type_t index)
{
    const nanoem_u8_t *parameters = __nanoem_default_interpolation;
    if (nanoem_is_not_null(keyframe) && index > NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_UNKNOWN && index < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM) {
        parameters = keyframe->interplation[index].u.values;
    }
    return parameters;
}

nanoem_bool_t APIENTRY
nanoemMotionCameraKeyframeIsLinearInterpolation(const nanoem_motion_camera_keyframe_t *keyframe, nanoem_motion_camera_keyframe_interpolation_type_t index)
{
    nanoem_bool_t result = nanoem_true;
    if (nanoem_is_not_null(keyframe) && index > NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_UNKNOWN && index < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM) {
        const nanoem_u8_t *v = keyframe->interplation[index].u.values;
        result = v[0] == v[1] && v[2] == v[3] && v[0] + v[2] == v[1] + v[3];
    }
    return result;
}

nanoem_u32_t APIENTRY
nanoemMotionCameraKeyframeGetStageIndex(const nanoem_motion_camera_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->stage_index : 0;
}

const nanoem_motion_outside_parent_t *APIENTRY
nanoemMotionCameraKeyframeGetOutsideParent(const nanoem_motion_camera_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->outside_parent : NULL;
}

nanoem_motion_outside_parent_t *APIENTRY
nanoemMotionCameraKeyframeGetOutsideParentMutable(nanoem_motion_camera_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->outside_parent : NULL;
}

const nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionCameraKeyframeGetKeyframeObject(const nanoem_motion_camera_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionCameraKeyframeGetKeyframeObjectMutable(nanoem_motion_camera_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

const nanoem_f32_t *APIENTRY
nanoemMotionLightKeyframeGetColor(const nanoem_motion_light_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->color.values : __nanoem_null_vector4;
}

const nanoem_f32_t *APIENTRY
nanoemMotionLightKeyframeGetDirection(const nanoem_motion_light_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->direction.values : __nanoem_null_vector4;
}

const nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionLightKeyframeGetKeyframeObject(const nanoem_motion_light_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionLightKeyframeGetKeyframeObjectMutable(nanoem_motion_light_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_bool_t APIENTRY
nanoemMotionModelKeyframeIsVisible(const nanoem_motion_model_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->visible : nanoem_false;
}

nanoem_f32_t APIENTRY
nanoemMotionModelKeyframeGetEdgeScaleFactor(const nanoem_motion_model_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->edge_scale_factor : 0.0f;
}

const nanoem_f32_t *APIENTRY
nanoemMotionModelKeyframeGetEdgeColor(const nanoem_motion_model_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->edge_color.values : __nanoem_null_vector4;
}

nanoem_bool_t APIENTRY
nanoemMotionModelKeyframeIsAddBlendEnabled(const nanoem_motion_model_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->is_add_blending_enabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemMotionModelKeyframeIsPhysicsSimulationEnabled(const nanoem_motion_model_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->is_physics_simulation_enabled : nanoem_false;
}

nanoem_motion_model_keyframe_constraint_state_t *const *APIENTRY
nanoemMotionModelKeyframeGetAllConstraintStateObjects(const nanoem_motion_model_keyframe_t *keyframe, nanoem_rsize_t *num_objects)
{
    nanoem_motion_model_keyframe_constraint_state_t *const *objects = NULL;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(num_objects)) {
        *num_objects = keyframe->num_constraint_states;
        objects = keyframe->constraint_states;
    }
    else if (nanoem_is_not_null(num_objects))  {
        *num_objects = 0;
    }
    return objects;
}

nanoem_motion_effect_parameter_t *const *APIENTRY
nanoemMotionModelKeyframeGetAllEffectParameterObjects(const nanoem_motion_model_keyframe_t *keyframe, nanoem_rsize_t *num_objects)
{
    nanoem_motion_effect_parameter_t *const *objects = NULL;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(num_objects)) {
        *num_objects = keyframe->num_effect_parameters;
        objects = keyframe->effect_parameters;
    }
    else if (nanoem_is_not_null(num_objects))  {
        *num_objects = 0;
    }
    return objects;
}

nanoem_motion_outside_parent_t *const *APIENTRY
nanoemMotionModelKeyframeGetAllOutsideParentObjects(const nanoem_motion_model_keyframe_t *keyframe, nanoem_rsize_t *num_objects)
{
    nanoem_motion_outside_parent_t *const *objects = NULL;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(num_objects)) {
        *num_objects = keyframe->num_outside_parents;
        objects = keyframe->outside_parents;
    }
    else if (nanoem_is_not_null(num_objects))  {
        *num_objects = 0;
    }
    return objects;
}

const nanoem_unicode_string_t *APIENTRY
nanoemMotionModelKeyframeConstraintStateGetBoneName(const nanoem_motion_model_keyframe_constraint_state_t *state)
{
    return nanoem_is_not_null(state) ? nanoemMotionTrackBundleResolveName(state->parent_keyframe->base.parent_motion->local_bone_motion_track_bundle, state->bone_id) : NULL;
}

int APIENTRY
nanoemMotionModelKeyframeConstraintStateGetBoneId(const nanoem_motion_model_keyframe_constraint_state_t *state)
{
    return nanoem_is_not_null(state) ? state->bone_id : 0;
}

nanoem_bool_t APIENTRY
nanoemMotionModelKeyframeConstraintStateIsEnabled(const nanoem_motion_model_keyframe_constraint_state_t *state)
{
    return nanoem_is_not_null(state) ? state->enabled : nanoem_false;
}

const nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionModelKeyframeGetKeyframeObject(const nanoem_motion_model_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionModelKeyframeGetKeyframeObjectMutable(nanoem_motion_model_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

const nanoem_unicode_string_t *APIENTRY
nanoemMotionMorphKeyframeGetName(const nanoem_motion_morph_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? nanoemMotionTrackBundleResolveName(keyframe->base.parent_motion->local_morph_motion_track_bundle, keyframe->morph_id) : NULL;
}

nanoem_f32_t APIENTRY
nanoemMotionMorphKeyframeGetWeight(const nanoem_motion_morph_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->weight : 0;
}

int APIENTRY
nanoemMotionMorphKeyframeGetId(const nanoem_motion_morph_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->morph_id : 0;
}

const nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionMorphKeyframeGetKeyframeObject(const nanoem_motion_morph_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionMorphKeyframeGetKeyframeObjectMutable(nanoem_motion_morph_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_f32_t APIENTRY
nanoemMotionSelfShadowKeyframeGetDistance(const nanoem_motion_self_shadow_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->distance : 0.0f;
}

int APIENTRY
nanoemMotionSelfShadowKeyframeGetMode(const nanoem_motion_self_shadow_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->mode : 0;
}

const nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionSelfShadowKeyframeGetKeyframeObject(const nanoem_motion_self_shadow_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_motion_keyframe_object_t *APIENTRY
nanoemMotionSelfShadowKeyframeGetKeyframeObjectMutable(nanoem_motion_self_shadow_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_motion_t *APIENTRY
nanoemMotionCreate(nanoem_unicode_string_factory_t *factory, nanoem_status_t *status)
{
    nanoem_motion_t *motion = NULL;
    if (nanoem_is_not_null(factory)) {
        motion = (nanoem_motion_t *) nanoem_calloc(1, sizeof(*motion), status);
        if (nanoem_is_not_null(motion)) {
            motion->factory = factory;
            motion->local_bone_motion_track_bundle = kh_init_motion_track_bundle();
            motion->local_morph_motion_track_bundle = kh_init_motion_track_bundle();
            motion->preferred_fps = 30;
            motion->type = NANOEM_MOTION_FORMAT_TYPE_UNKNOWN;
        }
    }
    return motion;
}

nanoem_bool_t APIENTRY
nanoemMotionLoadFromBufferVMD(nanoem_motion_t *motion, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    const nanoem_u8_t *ptr;
    nanoem_unicode_string_factory_t *factory;
    nanoem_status_ptr_assign_succeeded(status);
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(buffer)) {
        ptr = nanoemBufferGetDataPtr(buffer);
        if (nanoemBufferCanReadLength(buffer, VMD_SIGNATURE_SIZE)) {
            nanoemBufferSkip(buffer, VMD_SIGNATURE_SIZE, status);
            factory = motion->factory;
            if (nanoem_crt_memcmp(ptr, __nanoem_vmd_signature_type2, sizeof(__nanoem_vmd_signature_type2) - 1) == 0) {
                motion->target_model_name = nanoemBufferGetStringFromCp932(buffer, VMD_TARGET_MODEL_NAME_LENGTH_V2, factory, status);
                nanoemMotionParseVMD(motion, buffer, offset, status);
            }
            else if (nanoem_crt_memcmp(ptr, __nanoem_vmd_signature_type1, sizeof(__nanoem_vmd_signature_type1) - 1) == 0) {
                motion->target_model_name = nanoemBufferGetStringFromCp932(buffer, VMD_TARGET_MODEL_NAME_LENGTH_V1, factory, status);
                nanoemMotionParseVMD(motion, buffer, offset, status);
            }
            else {
                nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_INVALID_SIGNATURE);
            }
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_INVALID_SIGNATURE);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
    return !nanoem_status_ptr_has_error(status);
}

nanoem_bool_t APIENTRY
nanoemMotionLoadFromBuffer(nanoem_motion_t *motion, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    return nanoemMotionLoadFromBufferVMD(motion, buffer, offset, status);
}

nanoem_motion_format_type_t APIENTRY
nanoemMotionGetFormatType(const nanoem_motion_t *motion)
{
    return nanoem_is_not_null(motion) ? motion->type : NANOEM_MOTION_FORMAT_TYPE_UNKNOWN;
}

const nanoem_unicode_string_t *APIENTRY
nanoemMotionGetTargetModelName(const nanoem_motion_t *motion)
{
    return nanoem_is_not_null(motion) ? motion->target_model_name : NULL;
}

nanoem_frame_index_t APIENTRY
nanoemMotionGetMaxFrameIndex(const nanoem_motion_t *motion)
{
    return nanoem_is_not_null(motion) ? motion->max_frame_index : 0;
}

nanoem_f32_t APIENTRY
nanoemMotionGetPreferredFPS(const nanoem_motion_t *motion)
{
    return nanoem_is_not_null(motion) ? motion->preferred_fps : 0.0f;
}

const char *APIENTRY
nanoemMotionGetAnnotation(const nanoem_motion_t *motion, const char *key)
{
    return nanoem_is_not_null(motion) ? nanoemAnnotationGet(motion->annotations, key) : NULL;
}

nanoem_motion_accessory_keyframe_t *const *APIENTRY
nanoemMotionGetAllAccessoryKeyframeObjects(const nanoem_motion_t *motion, nanoem_rsize_t *num_keyframes)
{
    nanoem_motion_accessory_keyframe_t *const *objects = NULL;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(num_keyframes)) {
        *num_keyframes = motion->num_accessory_keyframes;
        objects = motion->accessory_keyframes;
    }
    else if (nanoem_is_not_null(num_keyframes)) {
        *num_keyframes = 0;
    }
    return objects;
}

nanoem_motion_bone_keyframe_t *const *APIENTRY
nanoemMotionGetAllBoneKeyframeObjects(const nanoem_motion_t *motion, nanoem_rsize_t *num_keyframes)
{
    nanoem_motion_bone_keyframe_t *const *objects = NULL;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(num_keyframes)) {
        *num_keyframes = motion->num_bone_keyframes;
        objects = motion->bone_keyframes;
    }
    else if (nanoem_is_not_null(num_keyframes)) {
        *num_keyframes = 0;
    }
    return objects;
}

nanoem_motion_camera_keyframe_t *const *APIENTRY
nanoemMotionGetAllCameraKeyframeObjects(const nanoem_motion_t *motion, nanoem_rsize_t *num_keyframes)
{
    nanoem_motion_camera_keyframe_t *const *objects = NULL;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(num_keyframes)) {
        *num_keyframes = motion->num_camera_keyframes;
        objects = motion->camera_keyframes;
    }
    else if (nanoem_is_not_null(num_keyframes)) {
        *num_keyframes = 0;
    }
    return objects;
}

nanoem_motion_light_keyframe_t *const *APIENTRY
nanoemMotionGetAllLightKeyframeObjects(const nanoem_motion_t *motion, nanoem_rsize_t *num_keyframes)
{
    nanoem_motion_light_keyframe_t *const *objects = NULL;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(num_keyframes)) {
        *num_keyframes = motion->num_light_keyframes;
        objects = motion->light_keyframes;
    }
    else if (nanoem_is_not_null(num_keyframes)) {
        *num_keyframes = 0;
    }
    return objects;
}

nanoem_motion_model_keyframe_t *const *APIENTRY
nanoemMotionGetAllModelKeyframeObjects(const nanoem_motion_t *motion, nanoem_rsize_t *num_keyframes)
{
    nanoem_motion_model_keyframe_t *const *objects = NULL;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(num_keyframes)) {
        *num_keyframes = motion->num_model_keyframes;
        objects = motion->model_keyframes;
    }
    else if (nanoem_is_not_null(num_keyframes)) {
        *num_keyframes = 0;
    }
    return objects;
}

nanoem_motion_morph_keyframe_t *const *APIENTRY
nanoemMotionGetAllMorphKeyframeObjects(const nanoem_motion_t *motion, nanoem_rsize_t *num_keyframes)
{
    nanoem_motion_morph_keyframe_t *const *objects = NULL;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(num_keyframes)) {
        *num_keyframes = motion->num_morph_keyframes;
        objects = motion->morph_keyframes;
    }
    else if (nanoem_is_not_null(num_keyframes)) {
        *num_keyframes = 0;
    }
    return objects;
}

nanoem_motion_self_shadow_keyframe_t *const *APIENTRY
nanoemMotionGetAllSelfShadowKeyframeObjects(const nanoem_motion_t *motion, nanoem_rsize_t *num_keyframes)
{
    nanoem_motion_self_shadow_keyframe_t *const *objects = NULL;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(num_keyframes)) {
        *num_keyframes = motion->num_self_shadow_keyframes;
        objects = motion->self_shadow_keyframes;
    }
    else if (nanoem_is_not_null(num_keyframes)) {
        *num_keyframes = 0;
    }
    return objects;
}

nanoem_motion_bone_keyframe_t *const *APIENTRY
nanoemMotionExtractBoneTrackKeyframes(const nanoem_motion_t *motion, const nanoem_unicode_string_t *name, nanoem_rsize_t *num_keyframes, nanoem_status_t *status)
{
    nanoem_motion_bone_keyframe_t **keyframes = NULL;
    kh_keyframe_map_t *keyframes_map;
    khiter_t it, end;
    nanoem_rsize_t i = 0;
    if (nanoem_is_not_null(motion)) {
        keyframes_map = nanoemMotionFindKeyframesMap(motion->local_bone_motion_track_bundle, name, motion->factory);
        if (keyframes_map) {
            keyframes = (nanoem_motion_bone_keyframe_t **) nanoem_calloc(kh_size(keyframes_map), sizeof(*keyframes), status);
            if (nanoem_is_not_null(keyframes)) {
                for (it = kh_begin(keyframes_map), end = kh_end(keyframes_map); it != end; it++) {
                    if (kh_exist(keyframes_map, it)) {
                        keyframes[i++] = (nanoem_motion_bone_keyframe_t *) kh_val(keyframes_map, it);
                    }
                }
                *num_keyframes = kh_size(keyframes_map);
                return keyframes;
            }
        }
    }
    else if (nanoem_is_not_null(num_keyframes)) {
        *num_keyframes = 0;
    }
    return NULL;
}

nanoem_motion_morph_keyframe_t *const *APIENTRY
nanoemMotionExtractMorphTrackKeyframes(const nanoem_motion_t *motion, const nanoem_unicode_string_t *name, nanoem_rsize_t *num_keyframes, nanoem_status_t *status)
{
    nanoem_motion_morph_keyframe_t **keyframes = NULL;
    kh_keyframe_map_t *keyframes_map;
    khiter_t it, end;
    nanoem_rsize_t i = 0;
    if (nanoem_is_not_null(motion)) {
        keyframes_map = nanoemMotionFindKeyframesMap(motion->local_morph_motion_track_bundle, name, motion->factory);
        if (keyframes_map) {
            keyframes = (nanoem_motion_morph_keyframe_t **) nanoem_calloc(kh_size(keyframes_map), sizeof(*keyframes), status);
            if (nanoem_is_not_null(keyframes)) {
                for (it = kh_begin(keyframes_map), end = kh_end(keyframes_map); it != end; it++) {
                    if (kh_exist(keyframes_map, it)) {
                        keyframes[i++] = (nanoem_motion_morph_keyframe_t *) kh_val(keyframes_map, it);
                    }
                }
                *num_keyframes = kh_size(keyframes_map);
                return keyframes;
            }
        }
    }
    else if (nanoem_is_not_null(num_keyframes)) {
        *num_keyframes = 0;
    }
    return NULL;
}

const nanoem_motion_accessory_keyframe_t *APIENTRY
nanoemMotionFindAccessoryKeyframeObject(const nanoem_motion_t *motion, nanoem_frame_index_t index)
{
    nanoem_motion_accessory_keyframe_t *keyframe = NULL;
    nanoem_motion_keyframe_object_t find_key, *find_key_ptr = &find_key;
    void *found_keyframe;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(motion->accessory_keyframes)) {
        find_key.frame_index = index;
        found_keyframe = nanoem_crt_bsearch(&find_key_ptr, motion->accessory_keyframes, motion->num_accessory_keyframes, sizeof(*motion->accessory_keyframes), nanoemMotionCompareKeyframe);
        if (found_keyframe != NULL) {
            keyframe = *(nanoem_motion_accessory_keyframe_t **) found_keyframe;
        }
    }
    return keyframe;
}

const nanoem_motion_bone_keyframe_t *APIENTRY
nanoemMotionFindBoneKeyframeObject(const nanoem_motion_t *motion, const nanoem_unicode_string_t *name, nanoem_frame_index_t index)
{
    nanoem_motion_bone_keyframe_t *keyframe = NULL;
    kh_keyframe_map_t *keyframes_map;
    khiter_t it;
    if (nanoem_is_not_null(motion)) {
        keyframes_map = nanoemMotionFindKeyframesMap(motion->local_bone_motion_track_bundle, name, motion->factory);
        if (keyframes_map) {
            it = kh_get_keyframe_map(keyframes_map, index);
            if (it != kh_end(keyframes_map)) {
                keyframe = (nanoem_motion_bone_keyframe_t *) kh_val(keyframes_map, it);
            }
        }
    }
    return keyframe;
}

const nanoem_motion_camera_keyframe_t *APIENTRY
nanoemMotionFindCameraKeyframeObject(const nanoem_motion_t *motion, nanoem_frame_index_t index)
{
    nanoem_motion_camera_keyframe_t *keyframe = NULL;
    nanoem_motion_keyframe_object_t find_key, *find_key_ptr = &find_key;
    void *found_keyframe;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(motion->camera_keyframes)) {
        find_key.frame_index = index;
        found_keyframe = nanoem_crt_bsearch(&find_key_ptr, motion->camera_keyframes, motion->num_camera_keyframes, sizeof(*motion->camera_keyframes), nanoemMotionCompareKeyframe);
        if (found_keyframe != NULL) {
            keyframe = *(nanoem_motion_camera_keyframe_t **) found_keyframe;
        }
    }
    return keyframe;
}

const nanoem_motion_light_keyframe_t *APIENTRY
nanoemMotionFindLightKeyframeObject(const nanoem_motion_t *motion, nanoem_frame_index_t index)
{
    nanoem_motion_light_keyframe_t *keyframe = NULL;
    nanoem_motion_keyframe_object_t find_key, *find_key_ptr = &find_key;
    void *found_keyframe;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(motion->light_keyframes)) {
        find_key.frame_index = index;
        found_keyframe = nanoem_crt_bsearch(&find_key_ptr, motion->light_keyframes, motion->num_light_keyframes, sizeof(*motion->light_keyframes), nanoemMotionCompareKeyframe);
        if (found_keyframe != NULL) {
            keyframe = *(nanoem_motion_light_keyframe_t **) found_keyframe;
        }
    }
    return keyframe;
}

const nanoem_motion_model_keyframe_t *APIENTRY
nanoemMotionFindModelKeyframeObject(const nanoem_motion_t *motion, nanoem_frame_index_t index)
{
    nanoem_motion_model_keyframe_t *keyframe = NULL;
    nanoem_motion_keyframe_object_t find_key, *find_key_ptr = &find_key;
    void *found_keyframe;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(motion->model_keyframes)) {
        find_key.frame_index = index;
        found_keyframe = nanoem_crt_bsearch(&find_key_ptr, motion->model_keyframes, motion->num_model_keyframes, sizeof(*motion->model_keyframes), nanoemMotionCompareKeyframe);
        if (found_keyframe != NULL) {
            keyframe = *(nanoem_motion_model_keyframe_t **) found_keyframe;
        }
    }
    return keyframe;
}

const nanoem_motion_morph_keyframe_t *APIENTRY
nanoemMotionFindMorphKeyframeObject(const nanoem_motion_t *motion, const nanoem_unicode_string_t *name, nanoem_frame_index_t index)
{
    nanoem_motion_morph_keyframe_t *keyframe = NULL;
    kh_keyframe_map_t *keyframes_map;
    khiter_t it;
    if (nanoem_is_not_null(motion)) {
        keyframes_map = nanoemMotionFindKeyframesMap(motion->local_morph_motion_track_bundle, name, motion->factory);
        if (keyframes_map) {
            it = kh_get_keyframe_map(keyframes_map, index);
            if (it != kh_end(keyframes_map)) {
                keyframe = (nanoem_motion_morph_keyframe_t *) kh_val(keyframes_map, it);
            }
        }
    }
    return keyframe;
}

const nanoem_motion_self_shadow_keyframe_t *APIENTRY
nanoemMotionFindSelfShadowKeyframeObject(const nanoem_motion_t *motion, nanoem_frame_index_t index)
{
    nanoem_motion_self_shadow_keyframe_t *keyframe = NULL;
    nanoem_motion_keyframe_object_t find_key, *find_key_ptr = &find_key;
    void *found_keyframe;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(motion->self_shadow_keyframes)) {
        find_key.frame_index = index;
        found_keyframe = nanoem_crt_bsearch(&find_key_ptr, motion->self_shadow_keyframes, motion->num_self_shadow_keyframes, sizeof(*motion->self_shadow_keyframes), nanoemMotionCompareKeyframe);
        if (found_keyframe != NULL) {
            keyframe = *(nanoem_motion_self_shadow_keyframe_t **) found_keyframe;
        }
    }
    return keyframe;
}

void APIENTRY
nanoemMotionSearchClosestAccessoryKeyframes(const nanoem_motion_t *motion, nanoem_frame_index_t base_index, nanoem_motion_accessory_keyframe_t **prev_keyframe, nanoem_motion_accessory_keyframe_t **next_keyframe)
{
    nanoem_motion_accessory_keyframe_t *current_keyframe, *last_keyframe = NULL;
    nanoem_frame_index_t prev_nearest = NANOEM_FRAME_INDEX_MAX_SIZE, next_nearest = NANOEM_FRAME_INDEX_MAX_SIZE;
    nanoem_motion_accessory_keyframe_t *const *accessory_keyframes;
    nanoem_rsize_t num_accessory_keyframes, i;
    if (nanoem_is_not_null(prev_keyframe)) {
        *prev_keyframe = NULL;
    }
    if (nanoem_is_not_null(next_keyframe)) {
        *next_keyframe = NULL;
    }
    if (nanoem_is_not_null(motion)) {
        accessory_keyframes = motion->accessory_keyframes;
        num_accessory_keyframes = motion->num_accessory_keyframes;
        for (i = 0; i < num_accessory_keyframes; i++) {
            current_keyframe = accessory_keyframes[i];
            nanoemMotionGetNearestKeyframes(&current_keyframe->base,
                base_index,
                &prev_nearest,
                &next_nearest,
                (nanoem_motion_keyframe_object_t **) prev_keyframe,
                (nanoem_motion_keyframe_object_t **) next_keyframe,
                (nanoem_motion_keyframe_object_t **) &last_keyframe);
        }
        if (nanoem_is_not_null(next_keyframe) && !*next_keyframe) {
            *next_keyframe = last_keyframe;
        }
    }
}

void APIENTRY
nanoemMotionSearchClosestBoneKeyframes(const nanoem_motion_t *motion, const nanoem_unicode_string_t *name, nanoem_frame_index_t base_index, nanoem_motion_bone_keyframe_t **prev_keyframe, nanoem_motion_bone_keyframe_t **next_keyframe)
{
    nanoem_motion_bone_keyframe_t *current_keyframe, *last_keyframe = NULL;
    nanoem_frame_index_t prev_nearest = NANOEM_FRAME_INDEX_MAX_SIZE, next_nearest = NANOEM_FRAME_INDEX_MAX_SIZE;
    kh_keyframe_map_t *keyframes_map;
    khiter_t it, end;
    if (nanoem_is_not_null(prev_keyframe)) {
        *prev_keyframe = NULL;
    }
    if (nanoem_is_not_null(next_keyframe)) {
        *next_keyframe = NULL;
    }
    if (nanoem_is_not_null(motion)) {
        keyframes_map = nanoemMotionFindKeyframesMap(motion->local_bone_motion_track_bundle, name, motion->factory);
        if (keyframes_map) {
            for (it = kh_begin(keyframes_map), end = kh_end(keyframes_map); it != end; it++) {
                if (kh_exist(keyframes_map, it)) {
                    current_keyframe = (nanoem_motion_bone_keyframe_t *) kh_val(keyframes_map, it);
                    nanoemMotionGetNearestKeyframes(&current_keyframe->base,
                        base_index,
                        &prev_nearest,
                        &next_nearest,
                        (nanoem_motion_keyframe_object_t **) prev_keyframe,
                        (nanoem_motion_keyframe_object_t **) next_keyframe,
                        (nanoem_motion_keyframe_object_t **) &last_keyframe);
                }
            }
            if (nanoem_is_not_null(next_keyframe) && !*next_keyframe) {
                *next_keyframe = last_keyframe;
            }
        }
    }
}

void APIENTRY
nanoemMotionSearchClosestCameraKeyframes(const nanoem_motion_t *motion, nanoem_frame_index_t base_index, nanoem_motion_camera_keyframe_t **prev_keyframe, nanoem_motion_camera_keyframe_t **next_keyframe)
{
    nanoem_motion_camera_keyframe_t *current_keyframe, *last_keyframe = NULL;
    nanoem_frame_index_t prev_nearest = NANOEM_FRAME_INDEX_MAX_SIZE, next_nearest = NANOEM_FRAME_INDEX_MAX_SIZE;
    nanoem_motion_camera_keyframe_t *const *camera_keyframes;
    nanoem_rsize_t num_camera_keyframes, i;
    if (nanoem_is_not_null(prev_keyframe)) {
        *prev_keyframe = NULL;
    }
    if (nanoem_is_not_null(next_keyframe)) {
        *next_keyframe = NULL;
    }
    if (nanoem_is_not_null(motion)) {
        camera_keyframes = motion->camera_keyframes;
        num_camera_keyframes = motion->num_camera_keyframes;
        for (i = 0; i < num_camera_keyframes; i++) {
            current_keyframe = camera_keyframes[i];
            nanoemMotionGetNearestKeyframes(&current_keyframe->base,
                base_index,
                &prev_nearest,
                &next_nearest,
                (nanoem_motion_keyframe_object_t **) prev_keyframe,
                (nanoem_motion_keyframe_object_t **) next_keyframe,
                (nanoem_motion_keyframe_object_t **) &last_keyframe);
        }
        if (nanoem_is_not_null(next_keyframe) && !*next_keyframe) {
            *next_keyframe = last_keyframe;
        }
    }
}

void APIENTRY
nanoemMotionSearchClosestLightKeyframes(const nanoem_motion_t *motion, nanoem_frame_index_t base_index, nanoem_motion_light_keyframe_t **prev_keyframe, nanoem_motion_light_keyframe_t **next_keyframe)
{
    nanoem_motion_light_keyframe_t *current_keyframe, *last_keyframe = NULL;
    nanoem_frame_index_t prev_nearest = NANOEM_FRAME_INDEX_MAX_SIZE, next_nearest = NANOEM_FRAME_INDEX_MAX_SIZE;
    nanoem_motion_light_keyframe_t *const *light_keyframes;
    nanoem_rsize_t num_light_keyframes, i;
    if (nanoem_is_not_null(prev_keyframe)) {
        *prev_keyframe = NULL;
    }
    if (nanoem_is_not_null(next_keyframe)) {
        *next_keyframe = NULL;
    }
    if (nanoem_is_not_null(motion)) {
        light_keyframes = motion->light_keyframes;
        num_light_keyframes = motion->num_light_keyframes;
        for (i = 0; i < num_light_keyframes; i++) {
            current_keyframe = light_keyframes[i];
            nanoemMotionGetNearestKeyframes(&current_keyframe->base,
                base_index,
                &prev_nearest,
                &next_nearest,
                (nanoem_motion_keyframe_object_t **) prev_keyframe,
                (nanoem_motion_keyframe_object_t **) next_keyframe,
                (nanoem_motion_keyframe_object_t **) &last_keyframe);
        }
        if (nanoem_is_not_null(next_keyframe) && !*next_keyframe) {
            *next_keyframe = last_keyframe;
        }
    }
}

void APIENTRY
nanoemMotionSearchClosestModelKeyframes(const nanoem_motion_t *motion, nanoem_frame_index_t base_index, nanoem_motion_model_keyframe_t **prev_keyframe, nanoem_motion_model_keyframe_t **next_keyframe)
{
    nanoem_motion_model_keyframe_t *current_keyframe, *last_keyframe = NULL;
    nanoem_frame_index_t prev_nearest = NANOEM_FRAME_INDEX_MAX_SIZE, next_nearest = NANOEM_FRAME_INDEX_MAX_SIZE;
    nanoem_motion_model_keyframe_t *const *model_keyframes;
    nanoem_rsize_t num_model_keyframes, i;
    if (nanoem_is_not_null(prev_keyframe)) {
        *prev_keyframe = NULL;
    }
    if (nanoem_is_not_null(next_keyframe)) {
        *next_keyframe = NULL;
    }
    if (nanoem_is_not_null(motion)) {
        model_keyframes = motion->model_keyframes;
        num_model_keyframes = motion->num_model_keyframes;
        for (i = 0; i < num_model_keyframes; i++) {
            current_keyframe = model_keyframes[i];
            nanoemMotionGetNearestKeyframes(&current_keyframe->base,
                base_index,
                &prev_nearest,
                &next_nearest,
                (nanoem_motion_keyframe_object_t **) prev_keyframe,
                (nanoem_motion_keyframe_object_t **) next_keyframe,
                (nanoem_motion_keyframe_object_t **) &last_keyframe);
        }
        if (nanoem_is_not_null(next_keyframe) && !*next_keyframe) {
            *next_keyframe = last_keyframe;
        }
    }
}

void APIENTRY
nanoemMotionSearchClosestMorphKeyframes(const nanoem_motion_t *motion, const nanoem_unicode_string_t *name, nanoem_frame_index_t base_index, nanoem_motion_morph_keyframe_t **prev_keyframe, nanoem_motion_morph_keyframe_t **next_keyframe)
{
    nanoem_motion_morph_keyframe_t *current_keyframe, *last_keyframe = NULL;
    nanoem_frame_index_t prev_nearest = NANOEM_FRAME_INDEX_MAX_SIZE, next_nearest = NANOEM_FRAME_INDEX_MAX_SIZE;
    kh_keyframe_map_t *keyframes_map;
    khiter_t it, end;
    if (nanoem_is_not_null(prev_keyframe)) {
        *prev_keyframe = NULL;
    }
    if (nanoem_is_not_null(next_keyframe)) {
        *next_keyframe = NULL;
    }
    if (nanoem_is_not_null(motion)) {
        keyframes_map = nanoemMotionFindKeyframesMap(motion->local_morph_motion_track_bundle, name, motion->factory);
        if (keyframes_map) {
            for (it = kh_begin(keyframes_map), end = kh_end(keyframes_map); it != end; it++) {
                if (kh_exist(keyframes_map, it)) {
                    current_keyframe = (nanoem_motion_morph_keyframe_t *) kh_val(keyframes_map, it);
                    nanoemMotionGetNearestKeyframes(&current_keyframe->base,
                        base_index,
                        &prev_nearest,
                        &next_nearest,
                        (nanoem_motion_keyframe_object_t **) prev_keyframe,
                        (nanoem_motion_keyframe_object_t **) next_keyframe,
                        (nanoem_motion_keyframe_object_t **) &last_keyframe);
                }
            }
            if (nanoem_is_not_null(next_keyframe) && !*next_keyframe) {
                *next_keyframe = last_keyframe;
            }
        }
    }
}

void APIENTRY
nanoemMotionSearchClosestSelfShadowKeyframes(const nanoem_motion_t *motion, nanoem_frame_index_t base_index, nanoem_motion_self_shadow_keyframe_t **prev_keyframe, nanoem_motion_self_shadow_keyframe_t **next_keyframe)
{
    nanoem_motion_self_shadow_keyframe_t *current_keyframe, *last_keyframe = NULL;
    nanoem_frame_index_t prev_nearest = NANOEM_FRAME_INDEX_MAX_SIZE, next_nearest = NANOEM_FRAME_INDEX_MAX_SIZE;
    nanoem_motion_self_shadow_keyframe_t *const *self_shadow_keyframes;
    nanoem_rsize_t num_self_shadow_keyframes, i;
    if (nanoem_is_not_null(prev_keyframe)) {
        *prev_keyframe = NULL;
    }
    if (nanoem_is_not_null(next_keyframe)) {
        *next_keyframe = NULL;
    }
    if (nanoem_is_not_null(motion)) {
        self_shadow_keyframes = motion->self_shadow_keyframes;
        num_self_shadow_keyframes = motion->num_self_shadow_keyframes;
        for (i = 0; i < num_self_shadow_keyframes; i++) {
            current_keyframe = self_shadow_keyframes[i];
            nanoemMotionGetNearestKeyframes(&current_keyframe->base,
                base_index,
                &prev_nearest,
                &next_nearest,
                (nanoem_motion_keyframe_object_t **) prev_keyframe,
                (nanoem_motion_keyframe_object_t **) next_keyframe,
                (nanoem_motion_keyframe_object_t **) &last_keyframe);
        }
        if (nanoem_is_not_null(next_keyframe) && !*next_keyframe) {
            *next_keyframe = last_keyframe;
        }
    }
}

nanoem_user_data_t *APIENTRY
nanoemMotionGetUserData(const nanoem_motion_t *motion)
{
    return nanoem_is_not_null(motion) ? motion->user_data : NULL;
}

void APIENTRY
nanoemMotionSetUserData(nanoem_motion_t *motion, nanoem_user_data_t *user_data)
{
    if (nanoem_is_not_null(motion)) {
        motion->user_data = user_data;
    }
}

void APIENTRY
nanoemMotionDestroy(nanoem_motion_t *motion)
{
    nanoem_unicode_string_factory_t *factory;
    nanoem_user_data_t *user_data;
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(motion)) {
        user_data = motion->user_data;
        if (user_data && user_data->type == NANOEM_USER_DATA_DESTROY_CALLBACK_MOTION) {
            user_data->destroy.motion(user_data->opaque, motion);
        }
        nanoemUserDataDestroy(user_data);
        factory = motion->factory;
        nanoemMotionTrackBundleDestroy(motion->local_bone_motion_track_bundle, factory);
        nanoemMotionTrackBundleDestroy(motion->local_morph_motion_track_bundle, factory);
        nanoemMotionTrackBundleDestroy(motion->global_motion_track_bundle, factory);
        nanoemAnnotationDestroy(motion->annotations);
        if (nanoem_is_not_null(motion->target_model_name)) {
            nanoemUtilDestroyString(motion->target_model_name, factory);
        }
        if (nanoem_is_not_null(motion->accessory_keyframes)) {
            num_objects = motion->num_accessory_keyframes;
            for (i = 0; i < num_objects; i++) {
                nanoemMotionAccessoryKeyframeDestroy(motion->accessory_keyframes[num_objects - i - 1]);
            }
            nanoem_free(motion->accessory_keyframes);
        }
        if (nanoem_is_not_null(motion->bone_keyframes)) {
            num_objects = motion->num_bone_keyframes;
            for (i = 0; i < num_objects; i++) {
                nanoemMotionBoneKeyframeDestroy(motion->bone_keyframes[num_objects - i - 1]);
            }
            nanoem_free(motion->bone_keyframes);
        }
        if (nanoem_is_not_null(motion->camera_keyframes)) {
            num_objects = motion->num_camera_keyframes;
            for (i = 0; i < num_objects; i++) {
                nanoemMotionCameraKeyframeDestroy(motion->camera_keyframes[num_objects - i - 1]);
            }
            nanoem_free(motion->camera_keyframes);
        }
        if (nanoem_is_not_null(motion->light_keyframes)) {
            num_objects = motion->num_light_keyframes;
            for (i = 0; i < num_objects; i++) {
                nanoemMotionLightKeyframeDestroy(motion->light_keyframes[num_objects - i - 1]);
            }
            nanoem_free(motion->light_keyframes);
        }
        if (nanoem_is_not_null(motion->model_keyframes)) {
            num_objects = motion->num_model_keyframes;
            for (i = 0; i < num_objects; i++) {
                nanoemMotionModelKeyframeDestroy(motion->model_keyframes[num_objects - i - 1]);
            }
            nanoem_free(motion->model_keyframes);
        }
        if (nanoem_is_not_null(motion->morph_keyframes)) {
            num_objects = motion->num_morph_keyframes;
            for (i = 0; i < num_objects; i++) {
                nanoemMotionMorphKeyframeDestroy(motion->morph_keyframes[num_objects - i - 1]);
            }
            nanoem_free(motion->morph_keyframes);
        }
        if (nanoem_is_not_null(motion->self_shadow_keyframes)) {
            num_objects = motion->num_self_shadow_keyframes;
            for (i = 0; i < num_objects; i++) {
                nanoemMotionSelfShadowKeyframeDestroy(motion->self_shadow_keyframes[num_objects - i - 1]);
            }
            nanoem_free(motion->self_shadow_keyframes);
        }
        nanoem_free(motion);
    }
}

nanoem_user_data_t *APIENTRY
nanoemUserDataCreate(nanoem_status_t *status)
{
    nanoem_user_data_t *user_data;
    user_data = (nanoem_user_data_t *) nanoem_calloc(1, sizeof(*user_data), status);
    return user_data;
}

void *APIENTRY
nanoemUserDataGetOpaqueData(const nanoem_user_data_t *user_data)
{
    return nanoem_is_not_null(user_data) ? user_data->opaque : NULL;
}

void APIENTRY
nanoemUserDataSetOpaqueData(nanoem_user_data_t *user_data, void *opaque)
{
    if (nanoem_is_not_null(user_data)) {
        user_data->opaque = opaque;
    }
}

void APIENTRY
nanoemUserDataSetOnDestroyModelCallback(nanoem_user_data_t *user_data, nanoem_user_data_on_destroy_model_t value)
{
    if (nanoem_is_not_null(user_data)) {
        user_data->type = NANOEM_USER_DATA_DESTROY_CALLBACK_MODEL;
        user_data->destroy.model = value;
    }
}

void APIENTRY
nanoemUserDataSetOnDestroyMotionCallback(nanoem_user_data_t *user_data, nanoem_user_data_on_destroy_motion_t value)
{
    if (nanoem_is_not_null(user_data)) {
        user_data->type = NANOEM_USER_DATA_DESTROY_CALLBACK_MOTION;
        user_data->destroy.motion = value;
    }
}

void APIENTRY
nanoemUserDataSetOnDestroyModelObjectCallback(nanoem_user_data_t *user_data, nanoem_user_data_on_destroy_model_object_t value)
{
    if (nanoem_is_not_null(user_data)) {
        user_data->type = NANOEM_USER_DATA_DESTROY_CALLBACK_MODEL_OBJECT;
        user_data->destroy.model_object = value;
    }
}

void APIENTRY
nanoemUserDataSetOnDestroyKeyframeObjectCallback(nanoem_user_data_t *user_data, nanoem_user_data_on_destroy_keyframe_object_t value)
{
    if (nanoem_is_not_null(user_data)) {
        user_data->type = NANOEM_USER_DATA_DESTROY_CALLBACK_KEYFRAME_OBJECT;
        user_data->destroy.keyframe_object = value;
    }
}

void APIENTRY
nanoemUserDataDestroy(nanoem_user_data_t *user_data)
{
    if (nanoem_is_not_null(user_data)) {
        nanoem_free(user_data);
    }
}
