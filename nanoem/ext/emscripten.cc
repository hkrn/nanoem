/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./emscripten.h"

#include "../nanoem_p.h" /* for nanoem_calloc/nanoem_free */

#include <stdlib.h>
#include <emscripten/val.h>

using namespace emscripten;

struct nanoem_unicode_factory_opaque_data_emscripten_t {
    struct {
        val utf8;
        val utf16;
        val cp932;
    } encoder;
    struct {
        val utf8;
        val utf16;
        val cp932;
    } decoder;
};

struct nanoem_unicode_string_emscripten_t {
    std::wstring value;
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

static nanoem_unicode_string_emscripten_t *
nanoemUnicodeStringFactoryFromStringEMST(val decoder, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_unicode_string_emscripten_t *s;
    s = (nanoem_unicode_string_emscripten_t *) nanoem_calloc(1, sizeof(*s), status);
    if (nanoem_is_not_null(s)) {
        s->value = decoder.call<val>("decode", typed_memory_view(length, string)).as<std::wstring>();
        nanoem_status_ptr_assign(status, NANOEM_STATUS_SUCCESS);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
    return s;
}

static void
nanoemUnicodeStringFactoryToStringEMST(val encoder, const nanoem_unicode_string_emscripten_t *s, nanoem_rsize_t *length, nanoem_u8_t *buffer, size_t capacity, nanoem_status_t *status)
{
    if (nanoem_is_not_null(buffer) && nanoem_is_not_null(s)) {
        const std::string result = encoder.call<val>("encode", s->value).as<std::string>();
        nanoem_rsize_t size = std::min(capacity, result.size());
        memcpy(buffer, result.data(), size);
        buffer[size] = 0;
        *length = size;
        nanoem_status_ptr_assign(status, NANOEM_STATUS_SUCCESS);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToStringOnHeapEMST(val encoder, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    const nanoem_unicode_string_emscripten_t *s = (const nanoem_unicode_string_emscripten_t *) string;
    nanoem_u8_t *buffer = NULL;
    int capacity;
    if (s) {
        capacity = s->value.size() * sizeof(wchar_t);
        buffer = (nanoem_u8_t *) nanoem_calloc(capacity, sizeof(*buffer), status);
        nanoemUnicodeStringFactoryToStringEMST(encoder, s, length, buffer, capacity, status);
    }
    return buffer;
}

static nanoem_unicode_string_t *
nanoemUnicodeStringFactoryFromCp932CallbackEMST(void *opaque, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_emscripten_t *data = (nanoem_unicode_factory_opaque_data_emscripten_t *) opaque;
    return (nanoem_unicode_string_t *) nanoemUnicodeStringFactoryFromStringEMST(data->decoder.cp932, string, length, status);
}

static nanoem_unicode_string_t *
nanoemUnicodeStringFactoryFromUtf8CallbackEMST(void *opaque, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_emscripten_t *data = (nanoem_unicode_factory_opaque_data_emscripten_t *) opaque;
    return (nanoem_unicode_string_t *) nanoemUnicodeStringFactoryFromStringEMST(data->decoder.utf8, string, length, status);
}

static nanoem_unicode_string_t *
nanoemUnicodeStringFactoryFromUtf16CallbackEMST(void *opaque, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_emscripten_t *data = (nanoem_unicode_factory_opaque_data_emscripten_t *) opaque;
    return (nanoem_unicode_string_t *) nanoemUnicodeStringFactoryFromStringEMST(data->decoder.utf16, string, length, status);
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToCp932CallbackEMST(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_emscripten_t *data = (nanoem_unicode_factory_opaque_data_emscripten_t *) opaque;
    return nanoemUnicodeStringFactoryToStringOnHeapEMST(data->encoder.cp932, string, length, status);
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToUtf8CallbackEMST(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_emscripten_t *data = (nanoem_unicode_factory_opaque_data_emscripten_t *) opaque;
    return nanoemUnicodeStringFactoryToStringOnHeapEMST(data->encoder.utf8, string, length, status);
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToUtf16CallbackEMST(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_emscripten_t *data = (nanoem_unicode_factory_opaque_data_emscripten_t *) opaque;
    return nanoemUnicodeStringFactoryToStringOnHeapEMST(data->encoder.utf16, string, length, status);
}

static nanoem_i32_t
nanoemUnicodeStringFactoryHashCallbackEMST(void *opaque, const nanoem_unicode_string_t *string)
{
    const nanoem_unicode_string_emscripten_t *s = (const nanoem_unicode_string_emscripten_t *) string;
    nanoem_i32_t hash = s ? (nanoem_i32_t) MurmurHash(s->value.c_str(), s->value.size() * sizeof(wchar_t), 0) : -1;
    nanoem_mark_unused(opaque);
    return hash;
}

static int
nanoemUnicodeStringFactoryCompareCallbackEMST(void *opaque, const nanoem_unicode_string_t *left, const nanoem_unicode_string_t *right)
{
    const nanoem_unicode_string_emscripten_t *lvalue = (const nanoem_unicode_string_emscripten_t *) left,
                                      *rvalue = (const nanoem_unicode_string_emscripten_t *) right;
    nanoem_mark_unused(opaque);
    return lvalue->value.compare(rvalue->value);
}

static void
nanoemUnicodeStringFactoryDestroyByteArrayCallbackEMST(void *opaque, nanoem_u8_t *bytes)
{
    nanoem_mark_unused(opaque);
    if (bytes) {
        nanoem_free(bytes);
    }
}

static void
nanoemUnicodeStringFactoryDestroyStringCallbackEMST(void *opaque, nanoem_unicode_string_t *string)
{
    nanoem_unicode_string_emscripten_t *s = (nanoem_unicode_string_emscripten_t *) string;
    nanoem_mark_unused(opaque);
    if (s) {
        nanoem_free(s);
    }
}

nanoem_unicode_string_factory_t *APIENTRY
nanoemUnicodeStringFactoryCreateEMST(nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_emscripten_t *opaque;
    nanoem_unicode_string_factory_t *factory;
    opaque = (nanoem_unicode_factory_opaque_data_emscripten_t *) nanoem_calloc(1, sizeof(*opaque), status);
    if (nanoem_is_not_null(opaque)) {
        val encoder = val::global("TextEncoder");
        val decoder = val::global("TextDecoder");
        /*
        try {
            val options = val::object();
            options.set("NONSTANDARD_allowLegacyEncoding", true);
            opaque->encoder.cp932 = encoder.new_(std::string("cp932"), options);
        } catch (const std::exception e) {
            opaque->encoder.cp932 = encoder.new_(std::string("utf-8"));
        }
        */
        opaque->encoder.utf8 = encoder.new_(std::string("utf-8"));
        opaque->encoder.utf16 = encoder.new_(std::string("utf-16"));
        opaque->decoder.cp932 = decoder.new_(std::string("shift_jis"));
        opaque->decoder.utf8 = decoder.new_(std::string("utf-8"));
        opaque->decoder.utf16 = decoder.new_(std::string("utf-16"));
    }
    factory = nanoemUnicodeStringFactoryCreate(status);
    nanoemUnicodeStringFactorySetCompareCallback(factory, nanoemUnicodeStringFactoryCompareCallbackEMST);
    nanoemUnicodeStringFactorySetConvertFromCp932Callback(factory, nanoemUnicodeStringFactoryFromCp932CallbackEMST);
    nanoemUnicodeStringFactorySetConvertFromUtf8Callback(factory, nanoemUnicodeStringFactoryFromUtf8CallbackEMST);
    nanoemUnicodeStringFactorySetConvertFromUtf16Callback(factory, nanoemUnicodeStringFactoryFromUtf16CallbackEMST);
    nanoemUnicodeStringFactorySetConvertToCp932Callback(factory, nanoemUnicodeStringFactoryToCp932CallbackEMST);
    nanoemUnicodeStringFactorySetConvertToUtf8Callback(factory, nanoemUnicodeStringFactoryToUtf8CallbackEMST);
    nanoemUnicodeStringFactorySetConvertToUtf16Callback(factory, nanoemUnicodeStringFactoryToUtf16CallbackEMST);
    nanoemUnicodeStringFactorySetDestroyByteArrayCallback(factory, nanoemUnicodeStringFactoryDestroyByteArrayCallbackEMST);
    nanoemUnicodeStringFactorySetDestroyStringCallback(factory, nanoemUnicodeStringFactoryDestroyStringCallbackEMST);
    nanoemUnicodeStringFactorySetHashCallback(factory, nanoemUnicodeStringFactoryHashCallbackEMST);
    nanoemUnicodeStringFactorySetOpaqueData(factory, opaque);
    return factory;
}

void APIENTRY
nanoemUnicodeStringFactoryDestroyEMST(nanoem_unicode_string_factory_t *factory)
{
    nanoem_unicode_factory_opaque_data_emscripten_t *opaque;
    opaque = (nanoem_unicode_factory_opaque_data_emscripten_t *) nanoemUnicodeStringFactoryGetOpaqueData(factory);
    if (nanoem_is_not_null(opaque)) {
        nanoem_free(opaque);
    }
    nanoemUnicodeStringFactoryDestroy(factory);
}

void APIENTRY
nanoemUnicodeStringFactoryToUtf8OnStackEMST(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_u8_t *buffer, size_t capacity, nanoem_status_t *status)
{
    void *opaque = nanoemUnicodeStringFactoryGetOpaqueData(factory);
    const nanoem_unicode_string_emscripten_t *s = (const nanoem_unicode_string_emscripten_t *) string;
    nanoem_unicode_factory_opaque_data_emscripten_t *data = (nanoem_unicode_factory_opaque_data_emscripten_t *) opaque;
    nanoemUnicodeStringFactoryToStringEMST(data->encoder.utf8, s, length, buffer, capacity, status);
    buffer[*length >= capacity ? (capacity - 1) : *length] = '\0';
}

const wchar_t *APIENTRY
nanoemUnicodeStringGetData(const nanoem_unicode_string_t *string)
{
    const nanoem_unicode_string_emscripten_t *s = (const nanoem_unicode_string_emscripten_t *) string;
    return nanoem_is_not_null(s) ? s->value.c_str() : NULL;
}

nanoem_rsize_t APIENTRY
nanoemUnicodeStringGetLength(const nanoem_unicode_string_t *string)
{
    const nanoem_unicode_string_emscripten_t *s = (const nanoem_unicode_string_emscripten_t *) string;
    return nanoem_is_not_null(s) ? s->value.size() : 0;
}
