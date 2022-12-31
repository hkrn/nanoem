/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EXT_QT_H_
#define NANOEM_EXT_QT_H_

#include "../nanoem.h"

#if defined(NANOEM_ENABLE_QT)

#include <QChar>

NANOEM_DECL_API nanoem_unicode_string_factory_t *
nanoemUnicodeStringFactoryCreateQt(nanoem_status_t *status);
NANOEM_DECL_API void
nanoemUnicodeStringFactoryDestroyQt(nanoem_unicode_string_factory_t *factory);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactoryToUtf8OnStackQt(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_u8_t *buffer, nanoem_rsize_t capacity, nanoem_status_t *status);
NANOEM_DECL_API const QChar *APIENTRY
nanoemUnicodeStringGetData(const nanoem_unicode_string_t *string);
NANOEM_DECL_API nanoem_rsize_t APIENTRY
nanoemUnicodeStringGetLength(const nanoem_unicode_string_t *string);

#endif

#endif /* NANOEM_EXT_QT_H_ */
