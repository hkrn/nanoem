/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./mbwc.h"

#include "../nanoem_p.h" /* for nanoem_calloc/nanoem_free */

#include <stdlib.h>
#include <wchar.h>

static const UINT NANOEM_MBSC_CP_SJIS = 932;
static const UINT NANOEM_MBSC_CP_UTF8 = CP_UTF8;

struct nanoem_unicode_string_mbwc_t {
    wchar_t *data;
    int length;
    struct nanoem_unicode_string_mbwc_cache_t {
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

    const unsigned char *data = (const unsigned char *) key;

    while (len >= 4) {
        h += *(unsigned int *) data;
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

static nanoem_unicode_string_mbwc_t *
nanoemUnicodeStringFactoryFromStringMBWC(const nanoem_u8_t *string, nanoem_rsize_t length, UINT codepage, nanoem_status_t *status)
{
    nanoem_unicode_string_mbwc_t *s;
    int capacity = MultiByteToWideChar(codepage, 0, (const char *) string, (int) length, NULL, 0);
    s = (nanoem_unicode_string_mbwc_t *) nanoem_calloc(1, sizeof(*s), status);
    if (nanoem_is_not_null(s)) {
        s->data = (wchar_t *) nanoem_calloc(capacity + 1, sizeof(*s->data), status);
        s->length = MultiByteToWideChar(codepage, 0, (const char *) string, (int) length, s->data, capacity);
        nanoem_status_ptr_assign(status, s->length != 0 || GetLastError() != 0 ? NANOEM_STATUS_SUCCESS : NANOEM_STATUS_ERROR_DECODE_UNICODE_STRING_FAILED);
    }
    return s;
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToStringMBWC(const nanoem_unicode_string_t *string, UINT codepage, nanoem_rsize_t *length, nanoem_status_t *status)
{
    const nanoem_unicode_string_mbwc_t *s = (const nanoem_unicode_string_mbwc_t *) string;
    int capacity = 0;
    nanoem_u8_t *buffer = NULL;
    if (s) {
        capacity = WideCharToMultiByte(codepage, 0, s->data, s->length, NULL, 0, NULL, NULL);
        buffer = (nanoem_u8_t *) nanoem_calloc(capacity + 1, sizeof(*buffer), status);
        if (nanoem_is_not_null(buffer)) {
            *length = WideCharToMultiByte(codepage, 0, s->data, s->length, (char *) buffer, capacity, NULL, NULL);
            nanoem_status_ptr_assign(status, *length != 0 || GetLastError() != 0 ? NANOEM_STATUS_SUCCESS : NANOEM_STATUS_ERROR_ENCODE_UNICODE_STRING_FAILED);
        }
        else {
            *length = 0;
        }
    }
    else {
        *length = 0;
        nanoem_status_ptr_assign_null_object(status);
    }
    return buffer;
}

static nanoem_unicode_string_t *
nanoemUnicodeStringFactoryFromCp932CallbackMBWC(void *opaque, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_mark_unused(opaque);
    return (nanoem_unicode_string_t *) nanoemUnicodeStringFactoryFromStringMBWC(string, length, NANOEM_MBSC_CP_SJIS, status);
}

static nanoem_unicode_string_t *
nanoemUnicodeStringFactoryFromUtf8CallbackMBWC(void *opaque, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_mark_unused(opaque);
    return (nanoem_unicode_string_t *) nanoemUnicodeStringFactoryFromStringMBWC(string, length, NANOEM_MBSC_CP_UTF8, status);
}

static nanoem_unicode_string_t *
nanoemUnicodeStringFactoryFromUtf16CallbackMBWC(void *opaque, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_unicode_string_mbwc_t *s;
    nanoem_mark_unused(opaque);
    s = (nanoem_unicode_string_mbwc_t *) nanoem_calloc(1, sizeof(*s), status);
    if (nanoem_is_not_null(s)) {
        s->data = (wchar_t *) nanoem_calloc(length, sizeof(*s->data), status);
        s->length = (int) length / sizeof(*s->data);
        memcpy(s->data, string, length);
        nanoem_status_ptr_assign_succeeded(status);
    }
    return (nanoem_unicode_string_t *) s;
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToCp932CallbackMBWC(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    nanoem_mark_unused(opaque);
    return nanoemUnicodeStringFactoryToStringMBWC(string, NANOEM_MBSC_CP_SJIS, length, status);
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToUtf8CallbackMBWC(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    nanoem_mark_unused(opaque);
    return nanoemUnicodeStringFactoryToStringMBWC(string, NANOEM_MBSC_CP_UTF8, length, status);
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToUtf16CallbackMBWC(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    const nanoem_unicode_string_mbwc_t *s = (const nanoem_unicode_string_mbwc_t *) string;
    int capacity = 0;
    nanoem_u8_t *buffer = NULL;
    nanoem_mark_unused(opaque);
    if (s) {
        capacity = s->length * sizeof(*s->data);
        buffer = (nanoem_u8_t *) nanoem_calloc(capacity + 1, sizeof(*buffer), status);
        if (nanoem_is_not_null(buffer)) {
            memcpy(buffer, s->data, capacity);
            buffer[capacity] = '\0';
            *length = capacity;
            nanoem_status_ptr_assign_succeeded(status);
        }
        else {
            *length = 0;
        }
    }
    else {
        *length = 0;
        nanoem_status_ptr_assign_null_object(status);
    }
    return buffer;
}

static nanoem_i32_t
nanoemUnicodeStringFactoryHashCallbackMBWC(void *opaque, const nanoem_unicode_string_t *string)
{
    const nanoem_unicode_string_mbwc_t *s = (const nanoem_unicode_string_mbwc_t *) string;
    nanoem_i32_t hash = s ? (nanoem_i32_t) MurmurHash(s->data, s->length * sizeof(*s->data), 0) : -1;
    nanoem_mark_unused(opaque);
    return hash;
}

static int
nanoemUnicodeStringFactoryCompareCallbackMBWC(void *opaque, const nanoem_unicode_string_t *left, const nanoem_unicode_string_t *right)
{
    const nanoem_unicode_string_mbwc_t *lvalue = (const nanoem_unicode_string_mbwc_t *) left,
                                       *rvalue = (const nanoem_unicode_string_mbwc_t *) right;
    nanoem_mark_unused(opaque);
    return (nanoem_is_not_null(rvalue) && nanoem_is_not_null(lvalue)) && nanoem_is_not_null(rvalue->data) && nanoem_is_not_null(lvalue->data) ? wcscmp(lvalue->data, rvalue->data) : -1;
}

static const nanoem_u8_t *
nanoemUnicodeStringFactoryGetCacheCallbackMBWC(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_codec_type_t codec, nanoem_status_t *status)
{
    const nanoem_unicode_string_mbwc_t *s = (const nanoem_unicode_string_mbwc_t *) string;
    nanoem_mark_unused(opaque);
    nanoem_mark_unused(codec);
    if (s->cache.data) {
        *length = s->cache.length;
        nanoem_status_ptr_assign_succeeded(status);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
    return s->cache.data;
}

static void
nanoemUnicodeStringFactorySetCacheCallbackMBWC(void *opaque, nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_codec_type_t codec, nanoem_status_t *status)
{
    nanoem_unicode_string_mbwc_t *s = (nanoem_unicode_string_mbwc_t *) string;
    nanoem_mark_unused(opaque);
    if (s->cache.data) {
        nanoem_free(s->cache.data);
        s->cache.data = 0;
        *length = 0;
    }
    switch (codec) {
    case NANOEM_CODEC_TYPE_SJIS:
        s->cache.data = nanoemUnicodeStringFactoryToCp932CallbackMBWC(opaque, string, length, status);
        break;
    case NANOEM_CODEC_TYPE_UTF8:
        s->cache.data = nanoemUnicodeStringFactoryToUtf8CallbackMBWC(opaque, string, length, status);
        break;
    case NANOEM_CODEC_TYPE_UTF16:
        s->cache.data = nanoemUnicodeStringFactoryToUtf16CallbackMBWC(opaque, string, length, status);
        break;
    default:
        break;
    }
    s->cache.length = *length;
}

static void
nanoemUnicodeStringFactoryDestroyStringCallbackMBWC(void *opaque, nanoem_unicode_string_t *string)
{
    nanoem_unicode_string_mbwc_t *s = (nanoem_unicode_string_mbwc_t *) string;
    nanoem_mark_unused(opaque);
    if (s) {
        nanoem_free(s->data);
        nanoem_free(s);
    }
}

static void
nanoemUnicodeStringFactoryDestroyByteArrayCallbackMBWC(void *opaque, nanoem_u8_t *string)
{
    nanoem_mark_unused(opaque);
    if (string) {
        nanoem_free(string);
    }
}

nanoem_unicode_string_factory_t *APIENTRY
nanoemUnicodeStringFactoryCreateMBWC(nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    factory = nanoemUnicodeStringFactoryCreate(status);
    nanoemUnicodeStringFactorySetGetCacheCallback(factory, nanoemUnicodeStringFactoryGetCacheCallbackMBWC);
    nanoemUnicodeStringFactorySetSetCacheCallback(factory, nanoemUnicodeStringFactorySetCacheCallbackMBWC);
    nanoemUnicodeStringFactorySetCompareCallback(factory, nanoemUnicodeStringFactoryCompareCallbackMBWC);
    nanoemUnicodeStringFactorySetConvertFromCp932Callback(factory, nanoemUnicodeStringFactoryFromCp932CallbackMBWC);
    nanoemUnicodeStringFactorySetConvertFromUtf8Callback(factory, nanoemUnicodeStringFactoryFromUtf8CallbackMBWC);
    nanoemUnicodeStringFactorySetConvertFromUtf16Callback(factory, nanoemUnicodeStringFactoryFromUtf16CallbackMBWC);
    nanoemUnicodeStringFactorySetConvertToCp932Callback(factory, nanoemUnicodeStringFactoryToCp932CallbackMBWC);
    nanoemUnicodeStringFactorySetConvertToUtf8Callback(factory, nanoemUnicodeStringFactoryToUtf8CallbackMBWC);
    nanoemUnicodeStringFactorySetConvertToUtf16Callback(factory, nanoemUnicodeStringFactoryToUtf16CallbackMBWC);
    nanoemUnicodeStringFactorySetDestroyStringCallback(factory, nanoemUnicodeStringFactoryDestroyStringCallbackMBWC);
    nanoemUnicodeStringFactorySetDestroyByteArrayCallback(factory, nanoemUnicodeStringFactoryDestroyByteArrayCallbackMBWC);
    nanoemUnicodeStringFactorySetHashCallback(factory, nanoemUnicodeStringFactoryHashCallbackMBWC);
    return factory;
}

void APIENTRY
nanoemUnicodeStringFactoryDestroyMBWC(nanoem_unicode_string_factory_t *factory)
{
    nanoem_unicode_factory_opaque_data_mbwc_t *opaque;
    opaque = (nanoem_unicode_factory_opaque_data_mbwc_t *) nanoemUnicodeStringFactoryGetOpaqueData(factory);
    if (opaque) {
        nanoem_free(opaque);
    }
    nanoemUnicodeStringFactoryDestroy(factory);
}

void APIENTRY
nanoemUnicodeStringFactoryToUtf8OnStackMBWC(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_u8_t *buffer, nanoem_rsize_t capacity, nanoem_status_t *status)
{
    const nanoem_unicode_string_mbwc_t *s = (const nanoem_unicode_string_mbwc_t *) string;
    nanoem_mark_unused(factory);
    if (s) {
        *length = WideCharToMultiByte(NANOEM_MBSC_CP_UTF8, 0, s->data, (int) s->length, (char *) buffer, (int) capacity, NULL, NULL);
        nanoem_status_ptr_assign(status, *length != 0 || GetLastError() == 0 ? NANOEM_STATUS_SUCCESS : NANOEM_STATUS_ERROR_DECODE_UNICODE_STRING_FAILED);
    }
    else {
        *length = 0;
        nanoem_status_ptr_assign_null_object(status);
    }
    buffer[*length >= capacity ? (capacity - 1) : *length] = '\0';
}

const wchar_t *APIENTRY
nanoemUnicodeStringGetData(const nanoem_unicode_string_t *string)
{
    const nanoem_unicode_string_mbwc_t *s = (const nanoem_unicode_string_mbwc_t *) string;
    return nanoem_is_not_null(s) ? s->data : NULL;
}

nanoem_rsize_t APIENTRY
nanoemUnicodeStringGetLength(const nanoem_unicode_string_t *string)
{
    const nanoem_unicode_string_mbwc_t *s = (const nanoem_unicode_string_mbwc_t *) string;
    return nanoem_is_not_null(s) ? s->length : 0;
}

nanoem_unicode_string_factory_t * APIENTRY
nanoemUnicodeStringFactoryCreateEXT(nanoem_status_t *status)
{
    return nanoemUnicodeStringFactoryCreateMBWC(status);
}

void APIENTRY
nanoemUnicodeStringFactoryDestroyEXT(nanoem_unicode_string_factory_t *factory)
{
    nanoemUnicodeStringFactoryDestroyMBWC(factory);
}

void APIENTRY
nanoemUnicodeStringFactoryToUtf8OnStackEXT(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_u8_t *buffer, nanoem_rsize_t capacity, nanoem_status_t *status)
{
    nanoemUnicodeStringFactoryToUtf8OnStackMBWC(factory, string, length, buffer, capacity, status);
}
