/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./qt.h"

#include <QString>
#include <QTextCodec>

#include "../nanoem_p.h" /* for nanoem_calloc/nanoem_free */

struct nanoem_unicode_factory_opaque_data_qt_t {
    QTextCodec *cp932;
    QTextCodec *utf8;
    QTextCodec *utf16;
};

struct nanoem_unicode_string_qt_t {
    QString *value;
};

static nanoem_unicode_string_qt_t *
nanoemUnicodeStringFactoryFromStringQt(QTextCodec *converter, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_unicode_string_qt_t *s;
    s = static_cast<nanoem_unicode_string_qt_t *>(nanoem_calloc(1, sizeof(*s), status));
    if (nanoem_is_not_null(s)) {
        s->value = new QString(converter->toUnicode(reinterpret_cast<const char *>(string), length));
        nanoem_status_ptr_assign(status, NANOEM_STATUS_SUCCESS);
    }
    return s;
}

static void
nanoemUnicodeStringFactoryToStringQt(QTextCodec *converter, const nanoem_unicode_string_qt_t *s, nanoem_rsize_t *length, nanoem_u8_t *buffer, nanoem_rsize_t capacity, nanoem_status_t *status)
{
    if (nanoem_is_not_null(buffer) && nanoem_is_not_null(s)) {
        const QByteArray &bytes = converter->fromUnicode(*s->value);
        *length = bytes.size();
        memcpy(buffer, bytes.constData(), qMin(capacity, static_cast<nanoem_rsize_t>(bytes.size())));
        nanoem_status_ptr_assign(status, NANOEM_STATUS_SUCCESS);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToStringOnHeapQt(QTextCodec *converter, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    const nanoem_unicode_string_qt_t *s = reinterpret_cast<const nanoem_unicode_string_qt_t *>(string);
    nanoem_u8_t *buffer = NULL;
    int capacity;
    if (s) {
        capacity = s->value->size() * sizeof(int) + 1;
        buffer = static_cast<nanoem_u8_t *>(nanoem_calloc(capacity, sizeof(*buffer), status));
        nanoemUnicodeStringFactoryToStringQt(converter, s, length, buffer, capacity, status);
    }
    return buffer;
}

static nanoem_unicode_string_t *
nanoemUnicodeStringFactoryFromCp932CallbackQt(void *opaque, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_qt_t *data = static_cast<nanoem_unicode_factory_opaque_data_qt_t *>(opaque);
    return reinterpret_cast<nanoem_unicode_string_t *>(nanoemUnicodeStringFactoryFromStringQt(data->cp932, string, length, status));
}

static nanoem_unicode_string_t *
nanoemUnicodeStringFactoryFromUtf8CallbackQt(void *opaque, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_qt_t *data = static_cast<nanoem_unicode_factory_opaque_data_qt_t *>(opaque);
    return reinterpret_cast<nanoem_unicode_string_t *>(nanoemUnicodeStringFactoryFromStringQt(data->utf8, string, length, status));
}

static nanoem_unicode_string_t *
nanoemUnicodeStringFactoryFromUtf16CallbackQt(void *opaque, const nanoem_u8_t *string, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_qt_t *data = static_cast<nanoem_unicode_factory_opaque_data_qt_t *>(opaque);
    return reinterpret_cast<nanoem_unicode_string_t *>(nanoemUnicodeStringFactoryFromStringQt(data->utf16, string, length, status));
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToCp932CallbackQt(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_qt_t *data = static_cast<nanoem_unicode_factory_opaque_data_qt_t *>(opaque);
    return nanoemUnicodeStringFactoryToStringOnHeapQt(data->cp932, string, length, status);
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToUtf8CallbackQt(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_qt_t *data = static_cast<nanoem_unicode_factory_opaque_data_qt_t *>(opaque);
    return nanoemUnicodeStringFactoryToStringOnHeapQt(data->utf8, string, length, status);
}

static nanoem_u8_t *
nanoemUnicodeStringFactoryToUtf16CallbackQt(void *opaque, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_qt_t *data = static_cast<nanoem_unicode_factory_opaque_data_qt_t *>(opaque);
    return nanoemUnicodeStringFactoryToStringOnHeapQt(data->utf16, string, length, status);
}

static nanoem_i32_t
nanoemUnicodeStringFactoryHashCallbackQt(void *opaque, const nanoem_unicode_string_t *string)
{
    const nanoem_unicode_string_qt_t *s = reinterpret_cast<const nanoem_unicode_string_qt_t *>(string);
    nanoem_i32_t hash = s ? static_cast<nanoem_i32_t>(qHash(*s->value)) : -1;
    nanoem_mark_unused(opaque);
    return hash;
}

static int
nanoemUnicodeStringFactoryCompareCallbackQt(void *opaque, const nanoem_unicode_string_t *left, const nanoem_unicode_string_t *right)
{
    const nanoem_unicode_string_qt_t *lvalue = reinterpret_cast<const nanoem_unicode_string_qt_t *>(left),
                                      *rvalue = reinterpret_cast<const nanoem_unicode_string_qt_t *>(right);
    nanoem_mark_unused(opaque);
    return nanoem_is_not_null(lvalue) && nanoem_is_not_null(rvalue) ? QString::compare(*lvalue->value, *rvalue->value) : -1;
}

static void
nanoemUnicodeStringFactoryDestroyStringCallbackQt(void *opaque, nanoem_unicode_string_t *string)
{
    nanoem_unicode_string_qt_t *s = reinterpret_cast<nanoem_unicode_string_qt_t *>(string);
    nanoem_mark_unused(opaque);
    if (s) {
        delete s->value;
        nanoem_free(s);
    }
}

static void
nanoemUnicodeStringFactoryDestroyByteArrayCallbackQt(void *opaque, nanoem_u8_t *string)
{
    nanoem_mark_unused(opaque);
    if (string) {
        nanoem_free(string);
    }
}

nanoem_unicode_string_factory_t *APIENTRY
nanoemUnicodeStringFactoryCreateQt(nanoem_status_t *status)
{
    nanoem_unicode_factory_opaque_data_qt_t *opaque;
    nanoem_unicode_string_factory_t *factory;
    opaque = static_cast<nanoem_unicode_factory_opaque_data_qt_t *>(nanoem_calloc(1, sizeof(*opaque), status));
    if (nanoem_is_not_null(opaque)) {
        opaque->cp932 = QTextCodec::codecForName("sjis");
        opaque->utf8 = QTextCodec::codecForName("utf8");
        opaque->utf16 = QTextCodec::codecForName("utf16");
    }
    factory = nanoemUnicodeStringFactoryCreate(status);
    nanoemUnicodeStringFactorySetCompareCallback(factory, nanoemUnicodeStringFactoryCompareCallbackQt);
    nanoemUnicodeStringFactorySetConvertFromCp932Callback(factory, nanoemUnicodeStringFactoryFromCp932CallbackQt);
    nanoemUnicodeStringFactorySetConvertFromUtf8Callback(factory, nanoemUnicodeStringFactoryFromUtf8CallbackQt);
    nanoemUnicodeStringFactorySetConvertFromUtf16Callback(factory, nanoemUnicodeStringFactoryFromUtf16CallbackQt);
    nanoemUnicodeStringFactorySetConvertToCp932Callback(factory, nanoemUnicodeStringFactoryToCp932CallbackQt);
    nanoemUnicodeStringFactorySetConvertToUtf8Callback(factory, nanoemUnicodeStringFactoryToUtf8CallbackQt);
    nanoemUnicodeStringFactorySetConvertToUtf16Callback(factory, nanoemUnicodeStringFactoryToUtf16CallbackQt);
    nanoemUnicodeStringFactorySetDestroyStringCallback(factory, nanoemUnicodeStringFactoryDestroyStringCallbackQt);
    nanoemUnicodeStringFactorySetDestroyByteArrayCallback(factory, nanoemUnicodeStringFactoryDestroyByteArrayCallbackQt);
    nanoemUnicodeStringFactorySetHashCallback(factory, nanoemUnicodeStringFactoryHashCallbackQt);
    nanoemUnicodeStringFactorySetOpaqueData(factory, opaque);
    return factory;
}

void APIENTRY
nanoemUnicodeStringFactoryDestroyQt(nanoem_unicode_string_factory_t *factory)
{
    nanoem_unicode_factory_opaque_data_qt_t *opaque;
    opaque = static_cast<nanoem_unicode_factory_opaque_data_qt_t *>(nanoemUnicodeStringFactoryGetOpaqueData(factory));
    if (nanoem_is_not_null(opaque)) {
        nanoem_free(opaque);
    }
    nanoemUnicodeStringFactoryDestroy(factory);
}

void APIENTRY
nanoemUnicodeStringFactoryToUtf8OnStackQt(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_u8_t *buffer, nanoem_rsize_t capacity, nanoem_status_t *status)
{
    void *opaque = nanoemUnicodeStringFactoryGetOpaqueData(factory);
    const nanoem_unicode_string_qt_t *s = reinterpret_cast<const nanoem_unicode_string_qt_t *>(string);
    nanoem_unicode_factory_opaque_data_qt_t *data = static_cast<nanoem_unicode_factory_opaque_data_qt_t *>(opaque);
    nanoemUnicodeStringFactoryToStringQt(data->utf8, s, length, buffer, capacity, status);
    buffer[*length >= capacity ? (capacity - 1) : *length] = '\0';
}

const QChar *APIENTRY
nanoemUnicodeStringGetData(const nanoem_unicode_string_t *string)
{
    const nanoem_unicode_string_qt_t *s = reinterpret_cast<const nanoem_unicode_string_qt_t *>(string);
    return nanoem_is_not_null(s) ? s->value->constData() : NULL;
}

nanoem_rsize_t APIENTRY
nanoemUnicodeStringGetLength(const nanoem_unicode_string_t *string)
{
    const nanoem_unicode_string_qt_t *s = reinterpret_cast<const nanoem_unicode_string_qt_t *>(string);
    return nanoem_is_not_null(s) ? s->value->length() : 0;
}

NANOEM_DECL_API nanoem_unicode_string_factory_t * APIENTRY
nanoemUnicodeStringFactoryCreateEXT(nanoem_status_t *status)
{
    return nanoemUnicodeStringFactoryCreateQt(status);
}

NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactoryDestroyEXT(nanoem_unicode_string_factory_t *factory)
{
    nanoemUnicodeStringFactoryDestroyQt(factory);
}

NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactoryToUtf8OnStackEXT(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_u8_t *buffer, nanoem_rsize_t capacity, nanoem_status_t *status)
{
    return nanoemUnicodeStringFactoryToUtf8OnStackQt(factory, string, length, buffer, capacity, status);
}
