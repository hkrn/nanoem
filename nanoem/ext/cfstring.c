/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./cfstring.h"

#include "../nanoem_p.h" /* for nanoem_calloc/nanoem_free */

#include <stdlib.h>
#include <CoreFoundation/CoreFoundation.h>

static CFStringRef
nanoemUnicodeStringFactoryFromStringCF(const nanoem_u8_t *string, nanoem_rsize_t length, CFStringEncoding encoding, nanoem_status_t *status)
{
    CFStringRef s = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *) string, length, encoding, FALSE);
    nanoem_status_ptr_assign(status, nanoem_is_not_null(s) ? NANOEM_STATUS_SUCCESS : NANOEM_STATUS_ERROR_DECODE_UNICODE_STRING_FAILED);
    return s;
}

static void
nanoemUnicodeStringFactoryToStringCF(CFStringRef s, nanoem_rsize_t *length, CFStringEncoding encoding, UInt8 *buffer, nanoem_rsize_t capacity, nanoem_status_t *status)
{
    CFIndex used_buffer_length, string_length = CFStringGetLength(s);
    CFRange range = CFRangeMake(0, string_length);
    CFStringGetBytes(s, range, encoding, 0, FALSE, 0, 0, &used_buffer_length);
    nanoem_rsize_t estimated_length = (nanoem_rsize_t) used_buffer_length;
    if (estimated_length < capacity) {
        CFStringGetBytes(s, range, encoding, 0, FALSE, buffer, capacity, NULL);
        *length = estimated_length;
        nanoem_status_ptr_assign_succeeded(status);
    }
    else {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_ENCODE_UNICODE_STRING_FAILED);
    }
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToStringOnHeapCF(const nanoem_unicode_string_t *string, nanoem_rsize_t *length, CFStringEncoding encoding, nanoem_status_t *status)
{
    CFStringRef s = (CFStringRef) string;
    CFIndex string_length, capacity;
    UInt8 *buffer = NULL;
    if (nanoem_is_not_null(s)) {
        string_length = CFStringGetLength(s);
        capacity = CFStringGetMaximumSizeForEncoding(string_length, encoding) + 1;
        buffer = (UInt8 *) nanoem_calloc(capacity, sizeof(*buffer), status);
        if (nanoem_is_not_null(buffer)) {
            nanoemUnicodeStringFactoryToStringCF(s, length, encoding, buffer, capacity, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
    return (nanoem_u8_t *) buffer;
}

static nanoem_unicode_string_t *
nanoemUnicodeStringFactoryFromCp932CallbackCF(void *opaque, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_mark_unused(opaque);
    return (nanoem_unicode_string_t *) nanoemUnicodeStringFactoryFromStringCF(string, length, kCFStringEncodingDOSJapanese, status);
}

static nanoem_unicode_string_t *
nanoemUnicodeStringFactoryFromUtf8CallbackCF(void *opaque, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_mark_unused(opaque);
    return (nanoem_unicode_string_t *) nanoemUnicodeStringFactoryFromStringCF(string, length, kCFStringEncodingUTF8, status);
}

static nanoem_unicode_string_t *
nanoemUnicodeStringFactoryFromUtf16CallbackCF(void *opaque, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_mark_unused(opaque);
    return (nanoem_unicode_string_t *) nanoemUnicodeStringFactoryFromStringCF(string, length, kCFStringEncodingUTF16LE, status);
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToCp932CallbackCF(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    nanoem_mark_unused(opaque);
    return nanoemUnicodeStringFactoryToStringOnHeapCF(string, length, kCFStringEncodingDOSJapanese, status);
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToUtf8CallbackCF(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    nanoem_mark_unused(opaque);
    return nanoemUnicodeStringFactoryToStringOnHeapCF(string, length, kCFStringEncodingUTF8, status);
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToUtf16CallbackCF(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    nanoem_mark_unused(opaque);
    return nanoemUnicodeStringFactoryToStringOnHeapCF(string, length, kCFStringEncodingUTF16LE, status);
}

static nanoem_i32_t
nanoemUnicodeStringFactoryHashCallbackCF(void *opaque, const nanoem_unicode_string_t *string)
{
    nanoem_mark_unused(opaque);
    return nanoem_is_not_null(string) ? (nanoem_i32_t) CFHash(string) : 0;
}

static int
nanoemUnicodeStringFactoryCompareCallbackCF(void *opaque, const nanoem_unicode_string_t *left, const nanoem_unicode_string_t *right)
{
    CFStringRef lvalue = (CFStringRef) left, rvalue = (CFStringRef) right;
    nanoem_mark_unused(opaque);
    return nanoem_is_not_null(left) && nanoem_is_not_null(right) ? (int) CFStringCompare(lvalue, rvalue, 0) : (int) (left - right);
}

static void
nanoemUnicodeStringFactoryDestroyStringCallbackCF(void *opaque, nanoem_unicode_string_t *string)
{
    CFStringRef s = (CFStringRef) string;
    nanoem_mark_unused(opaque);
    if (nanoem_is_not_null(s)) {
        CFRelease(s);
    }
}

static void
nanoemUnicodeStringFactoryDestroyByteArrayCallbackCF(void *opaque, nanoem_u8_t *string)
{
    nanoem_mark_unused(opaque);
    if (nanoem_is_not_null(string)) {
        nanoem_free(string);
    }
}

nanoem_unicode_string_factory_t * APIENTRY
nanoemUnicodeStringFactoryCreateCF(nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    factory = nanoemUnicodeStringFactoryCreate(status);
    nanoemUnicodeStringFactorySetCompareCallback(factory, nanoemUnicodeStringFactoryCompareCallbackCF);
    nanoemUnicodeStringFactorySetConvertFromCp932Callback(factory, nanoemUnicodeStringFactoryFromCp932CallbackCF);
    nanoemUnicodeStringFactorySetConvertFromUtf8Callback(factory, nanoemUnicodeStringFactoryFromUtf8CallbackCF);
    nanoemUnicodeStringFactorySetConvertFromUtf16Callback(factory, nanoemUnicodeStringFactoryFromUtf16CallbackCF);
    nanoemUnicodeStringFactorySetConvertToCp932Callback(factory, nanoemUnicodeStringFactoryToCp932CallbackCF);
    nanoemUnicodeStringFactorySetConvertToUtf8Callback(factory, nanoemUnicodeStringFactoryToUtf8CallbackCF);
    nanoemUnicodeStringFactorySetConvertToUtf16Callback(factory, nanoemUnicodeStringFactoryToUtf16CallbackCF);
    nanoemUnicodeStringFactorySetDestroyStringCallback(factory, nanoemUnicodeStringFactoryDestroyStringCallbackCF);
    nanoemUnicodeStringFactorySetDestroyByteArrayCallback(factory, nanoemUnicodeStringFactoryDestroyByteArrayCallbackCF);
    nanoemUnicodeStringFactorySetHashCallback(factory, nanoemUnicodeStringFactoryHashCallbackCF);
    return factory;
}

void APIENTRY
nanoemUnicodeStringFactoryDestroyCF(nanoem_unicode_string_factory_t *factory)
{
    nanoemUnicodeStringFactoryDestroy(factory);
}

nanoem_unicode_string_t *APIENTRY
nanoemUnicodeStringFactoryCreateStringCF(nanoem_unicode_string_factory_t *factory, const char *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    void *opaque = nanoemUnicodeStringFactoryGetOpaqueData(factory);
    return nanoemUnicodeStringFactoryFromUtf8CallbackCF(opaque, (const nanoem_u8_t *) string, length, status);
}

nanoem_u8_t *APIENTRY
nanoemUnicodeStringFactoryToUtf8OnHeapCF(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    void *opaque = nanoemUnicodeStringFactoryGetOpaqueData(factory);
    return nanoemUnicodeStringFactoryToUtf8CallbackCF(opaque, string, length, status);
}

void APIENTRY
nanoemUnicodeStringFactoryToUtf8OnStackCF(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_u8_t *buffer, nanoem_rsize_t capacity, nanoem_status_t *status)
{
    CFStringRef s = (CFStringRef) string;
    nanoem_mark_unused(factory);
    if (nanoem_is_not_null(s)) {
        nanoemUnicodeStringFactoryToStringCF(s, length, kCFStringEncodingUTF8, (UInt8 *) buffer, capacity, status);
        buffer[*length >= capacity ? (capacity - 1) : *length] = '\0';
    }
    else {
        *buffer = '\0';
    }
}

const UniChar *APIENTRY
nanoemUnicodeStringGetData(const nanoem_unicode_string_t *string)
{
    CFStringRef s = (CFStringRef) string;
    return nanoem_is_not_null(s) ? CFStringGetCharactersPtr(s) : NULL;
}

nanoem_rsize_t APIENTRY
nanoemUnicodeStringGetLength(const nanoem_unicode_string_t *string)
{
    CFStringRef s = (CFStringRef) string;
    return nanoem_is_not_null(s) ? CFStringGetLength(s) : 0;
}

nanoem_unicode_string_factory_t * APIENTRY
nanoemUnicodeStringFactoryCreateEXT(nanoem_status_t *status)
{
    return nanoemUnicodeStringFactoryCreateCF(status);
}

void APIENTRY
nanoemUnicodeStringFactoryDestroyEXT(nanoem_unicode_string_factory_t *factory)
{
    nanoemUnicodeStringFactoryDestroyCF(factory);
}

void APIENTRY
nanoemUnicodeStringFactoryToUtf8OnStackEXT(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_u8_t *buffer, nanoem_rsize_t capacity, nanoem_status_t *status)
{
    return nanoemUnicodeStringFactoryToUtf8OnStackCF(factory, string, length, buffer, capacity, status);
}
