/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./icu.h"

#include "../nanoem_p.h" /* for nanoem_calloc/nanoem_free */

#include <unicode/ucnv.h>
#include <unicode/ustring.h>

struct nanoem_unicode_factory_opaque_data_icu_t {
    UConverter *cp932;
    UConverter *utf8;
    UConverter *utf16;
};

struct nanoem_unicode_string_icu_t {
    UChar *data;
    int length;
    struct nanoem_unicode_string_icu_cache_t {
        nanoem_u8_t *data;
        nanoem_rsize_t length;
    } cache;
};

//-----------------------------------------------------------------------------
// MurmurHash, by Austin Appleby

// Note - This code makes a few assumptions about how your machine behaves -

// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4

// And it has a few limitations -

// 1. It will not work incrementally.
// 2. It will not produce the same results on little-endian and big-endian
//    machines.

// Changing the seed value will totally change the output of the hash.
// If you don't have a preference, use a seed of 0.

static unsigned int
MurmurHash(const void *key, int len, unsigned int seed)
{
    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.

    const unsigned int m = 0xc6a4a793;
    const int r = 16;

    // Initialize the hash to a 'random' value

    unsigned int h = seed ^ (len * m);

    // Mix 4 bytes at a time into the hash

    union char_to_int {
        const unsigned char *c;
        const unsigned int *i;
    } p;

    const unsigned char *data = p.c = (const unsigned char *) key;

    while (len >= 4) {
        h += *p.i;
        h *= m;
        h ^= h >> r;

        data += 4;
        len -= 4;
    }
    // Handle the last few bytes of the input array

    switch (len) {
    case 3:
        h += data[2] << 16;
    case 2:
        h += data[1] << 8;
    case 1:
        h += data[0];
        h *= m;
        h ^= h >> r;
    default:
        break;
    };

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.

    h *= m;
    h ^= h >> 10;
    h *= m;
    h ^= h >> 17;

    return h;
}

static nanoem_unicode_string_icu_t *
nanoemUnicodeStringFactoryFromStringICU(UConverter *converter, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_unicode_string_icu_t *s;
    UErrorCode code = U_ZERO_ERROR;
    int capacity = length * ucnv_getMinCharSize(converter) + 1;
    s = (nanoem_unicode_string_icu_t *) nanoem_calloc(1, sizeof(*s), status);
    if (nanoem_is_not_null(s)) {
        s->data = (UChar *) nanoem_calloc(capacity, sizeof(*s->data), status);
        s->length = ucnv_toUChars(converter, s->data, capacity, (const char *) string, length, &code);
        nanoem_status_ptr_assign(status, U_SUCCESS(code) ? NANOEM_STATUS_SUCCESS : NANOEM_STATUS_ERROR_DECODE_UNICODE_STRING_FAILED);
    }
    return s;
}

static void
nanoemUnicodeStringFactoryToStringICU(UConverter *converter, const nanoem_unicode_string_icu_t *s, nanoem_rsize_t *length, nanoem_u8_t *buffer, nanoem_rsize_t capacity, nanoem_status_t *status)
{
    UErrorCode code = U_ZERO_ERROR;
    if (nanoem_is_not_null(buffer) && nanoem_is_not_null(s)) {
        *length = ucnv_fromUChars(converter, (char *) buffer, capacity, s->data, s->length, &code);
        nanoem_status_ptr_assign(status, U_SUCCESS(code) ? NANOEM_STATUS_SUCCESS : NANOEM_STATUS_ERROR_ENCODE_UNICODE_STRING_FAILED);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToStringOnHeapICU(UConverter *converter, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    const nanoem_unicode_string_icu_t *s = (const nanoem_unicode_string_icu_t *) string;
    nanoem_u8_t *buffer = NULL;
    int capacity;
    if (s) {
        capacity = s->length * ucnv_getMaxCharSize(converter) + 1;
        buffer = (nanoem_u8_t *) nanoem_calloc(capacity, sizeof(*buffer), status);
        nanoemUnicodeStringFactoryToStringICU(converter, s, length, buffer, capacity, status);
    }
    return buffer;
}

static nanoem_unicode_string_t *
nanoemUnicodeStringFactoryFromCp932CallbackICU(void *opaque, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_icu_t *data = (nanoem_unicode_factory_opaque_data_icu_t *) opaque;
    return (nanoem_unicode_string_t *) nanoemUnicodeStringFactoryFromStringICU(data->cp932, string, length, status);
}

static nanoem_unicode_string_t *
nanoemUnicodeStringFactoryFromUtf8CallbackICU(void *opaque, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_icu_t *data = (nanoem_unicode_factory_opaque_data_icu_t *) opaque;
    return (nanoem_unicode_string_t *) nanoemUnicodeStringFactoryFromStringICU(data->utf8, string, length, status);
}

static nanoem_unicode_string_t *
nanoemUnicodeStringFactoryFromUtf16CallbackICU(void *opaque, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_icu_t *data = (nanoem_unicode_factory_opaque_data_icu_t *) opaque;
    return (nanoem_unicode_string_t *) nanoemUnicodeStringFactoryFromStringICU(data->utf16, string, length, status);
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToCp932CallbackICU(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_icu_t *data = (nanoem_unicode_factory_opaque_data_icu_t *) opaque;
    return nanoemUnicodeStringFactoryToStringOnHeapICU(data->cp932, string, length, status);
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToUtf8CallbackICU(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_icu_t *data = (nanoem_unicode_factory_opaque_data_icu_t *) opaque;
    return nanoemUnicodeStringFactoryToStringOnHeapICU(data->utf8, string, length, status);
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToUtf16CallbackICU(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_icu_t *data = (nanoem_unicode_factory_opaque_data_icu_t *) opaque;
    return nanoemUnicodeStringFactoryToStringOnHeapICU(data->utf16, string, length, status);
}

static nanoem_i32_t
nanoemUnicodeStringFactoryHashCallbackICU(void *opaque, const nanoem_unicode_string_t *string)
{
    const nanoem_unicode_string_icu_t *s = (const nanoem_unicode_string_icu_t *) string;
    nanoem_i32_t hash = s ? (nanoem_i32_t) MurmurHash(s->data, u_strlen(s->data) * sizeof(*s->data), 0) : -1;
    nanoem_mark_unused(opaque);
    return hash;
}

static int
nanoemUnicodeStringFactoryCompareCallbackICU(void *opaque, const nanoem_unicode_string_t *left, const nanoem_unicode_string_t *right)
{
    const nanoem_unicode_string_icu_t *lvalue = (const nanoem_unicode_string_icu_t *) left,
                                      *rvalue = (const nanoem_unicode_string_icu_t *) right;
    nanoem_mark_unused(opaque);
    return nanoem_is_not_null(lvalue) && nanoem_is_not_null(rvalue) ? u_strcmp(lvalue->data, rvalue->data) : -1;
}

static const nanoem_u8_t *
nanoemUnicodeStringFactoryGetCacheCallbackICU(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_codec_type_t codec, nanoem_status_t *status)
{
    const nanoem_unicode_string_icu_t *s = (const nanoem_unicode_string_icu_t *) string;
    nanoem_mark_unused(opaque);
    nanoem_mark_unused(codec);
    if (s->cache.data) {
        *length = s->cache.length;
        nanoem_status_ptr_assign(status, NANOEM_STATUS_SUCCESS);
    }
    else {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_NULL_OBJECT);
    }
    return s->cache.data;
}

static void
nanoemUnicodeStringFactorySetCacheCallbackICU(void *opaque, nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_codec_type_t codec, nanoem_status_t *status)
{
    nanoem_unicode_string_icu_t *s = (nanoem_unicode_string_icu_t *) string;
    nanoem_mark_unused(opaque);
    if (s->cache.data) {
        nanoem_free(s->cache.data);
        s->cache.data = NULL;
        s->cache.length = 0;
    }
    switch (codec) {
    case NANOEM_CODEC_TYPE_SJIS:
        s->cache.data = nanoemUnicodeStringFactoryToCp932CallbackICU(opaque, string, &s->cache.length, status);
        break;
    case NANOEM_CODEC_TYPE_UTF8:
        s->cache.data = nanoemUnicodeStringFactoryToUtf8CallbackICU(opaque, string, &s->cache.length, status);
        break;
    case NANOEM_CODEC_TYPE_UTF16:
        s->cache.data = nanoemUnicodeStringFactoryToUtf16CallbackICU(opaque, string, &s->cache.length, status);
        break;
    default:
        break;
    }
    *length = s->cache.length;
}

static void
nanoemUnicodeStringFactoryDestroyStringCallbackICU(void *opaque, nanoem_unicode_string_t *string)
{
    nanoem_unicode_string_icu_t *s = (nanoem_unicode_string_icu_t *) string;
    nanoem_mark_unused(opaque);
    if (s) {
        nanoem_free(s->data);
        nanoem_free(s->cache.data);
        nanoem_free(s);
    }
}

static void
nanoemUnicodeStringFactoryDestroyByteArrayCallbackICU(void *opaque, nanoem_u8_t *string)
{
    nanoem_mark_unused(opaque);
    if (string) {
        nanoem_free(string);
    }
}

nanoem_unicode_string_factory_t *APIENTRY
nanoemUnicodeStringFactoryCreateICU(nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_icu_t *opaque;
    nanoem_unicode_string_factory_t *factory = NULL;
    UErrorCode code = U_ZERO_ERROR;
    opaque = (nanoem_unicode_factory_opaque_data_icu_t *) nanoem_calloc(1, sizeof(*opaque), status);
    if (nanoem_is_not_null(opaque)) {
        opaque->cp932 = ucnv_open("ibm-943_P15A-2003", &code);
        opaque->utf8 = ucnv_open("utf8", &code);
        opaque->utf16 = ucnv_open("utf16le", &code);
        if (U_SUCCESS(code)) {
            factory = nanoemUnicodeStringFactoryCreate(status);
            nanoemUnicodeStringFactorySetGetCacheCallback(factory, nanoemUnicodeStringFactoryGetCacheCallbackICU);
            nanoemUnicodeStringFactorySetSetCacheCallback(factory, nanoemUnicodeStringFactorySetCacheCallbackICU);
            nanoemUnicodeStringFactorySetCompareCallback(factory, nanoemUnicodeStringFactoryCompareCallbackICU);
            nanoemUnicodeStringFactorySetConvertFromCp932Callback(factory, nanoemUnicodeStringFactoryFromCp932CallbackICU);
            nanoemUnicodeStringFactorySetConvertFromUtf8Callback(factory, nanoemUnicodeStringFactoryFromUtf8CallbackICU);
            nanoemUnicodeStringFactorySetConvertFromUtf16Callback(factory, nanoemUnicodeStringFactoryFromUtf16CallbackICU);
            nanoemUnicodeStringFactorySetConvertToCp932Callback(factory, nanoemUnicodeStringFactoryToCp932CallbackICU);
            nanoemUnicodeStringFactorySetConvertToUtf8Callback(factory, nanoemUnicodeStringFactoryToUtf8CallbackICU);
            nanoemUnicodeStringFactorySetConvertToUtf16Callback(factory, nanoemUnicodeStringFactoryToUtf16CallbackICU);
            nanoemUnicodeStringFactorySetDestroyStringCallback(factory, nanoemUnicodeStringFactoryDestroyStringCallbackICU);
            nanoemUnicodeStringFactorySetDestroyByteArrayCallback(factory, nanoemUnicodeStringFactoryDestroyByteArrayCallbackICU);
            nanoemUnicodeStringFactorySetHashCallback(factory, nanoemUnicodeStringFactoryHashCallbackICU);
            nanoemUnicodeStringFactorySetOpaqueData(factory, opaque);
        }
        else {
            nanoem_free(opaque);
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return factory;
}

void APIENTRY
nanoemUnicodeStringFactoryDestroyICU(nanoem_unicode_string_factory_t *factory)
{
    nanoem_unicode_factory_opaque_data_icu_t *opaque;
    opaque = (nanoem_unicode_factory_opaque_data_icu_t *) nanoemUnicodeStringFactoryGetOpaqueData(factory);
    if (nanoem_is_not_null(opaque)) {
        ucnv_close(opaque->cp932);
        ucnv_close(opaque->utf8);
        ucnv_close(opaque->utf16);
        ucnv_flushCache();
        nanoem_free(opaque);
    }
    nanoemUnicodeStringFactoryDestroy(factory);
}

void APIENTRY
nanoemUnicodeStringFactoryToUtf8OnStackICU(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_u8_t *buffer, nanoem_rsize_t capacity, nanoem_status_t *status)
{
    void *opaque = nanoemUnicodeStringFactoryGetOpaqueData(factory);
    const nanoem_unicode_string_icu_t *s = (const nanoem_unicode_string_icu_t *) string;
    nanoem_unicode_factory_opaque_data_icu_t *data = (nanoem_unicode_factory_opaque_data_icu_t *) opaque;
    nanoemUnicodeStringFactoryToStringICU(data->utf8, s, length, buffer, capacity, status);
    buffer[*length >= capacity ? (capacity - 1) : *length] = '\0';
}

const UChar *APIENTRY
nanoemUnicodeStringGetData(const nanoem_unicode_string_t *string)
{
    const nanoem_unicode_string_icu_t *s = (const nanoem_unicode_string_icu_t *) string;
    return nanoem_is_not_null(s) ? s->data : NULL;
}

nanoem_rsize_t APIENTRY
nanoemUnicodeStringGetLength(const nanoem_unicode_string_t *string)
{
    const nanoem_unicode_string_icu_t *s = (const nanoem_unicode_string_icu_t *) string;
    return nanoem_is_not_null(s) ? s->length : 0;
}

nanoem_unicode_string_factory_t * APIENTRY
nanoemUnicodeStringFactoryCreateEXT(nanoem_status_t *status)
{
    return nanoemUnicodeStringFactoryCreateICU(status);
}

void APIENTRY
nanoemUnicodeStringFactoryDestroyEXT(nanoem_unicode_string_factory_t *factory)
{
    nanoemUnicodeStringFactoryDestroyICU(factory);
}

void APIENTRY
nanoemUnicodeStringFactoryToUtf8OnStackEXT(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_u8_t *buffer, nanoem_rsize_t capacity, nanoem_status_t *status)
{
    return nanoemUnicodeStringFactoryToUtf8OnStackICU(factory, string, length, buffer, capacity, status);
}
