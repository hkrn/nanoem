/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EXT_ICU_H_
#define NANOEM_EXT_ICU_H_

#ifdef NANOEM_ENABLE_ICU
#include <unicode/ustring.h>
#else
typedef unsigned short UChar;
#endif

#include "../nanoem.h"

typedef struct nanoem_unicode_factory_opaque_data_icu_t nanoem_unicode_factory_opaque_data_icu_t;
typedef struct nanoem_unicode_string_icu_t nanoem_unicode_string_icu_t;

NANOEM_DECL_API nanoem_unicode_string_factory_t *APIENTRY
nanoemUnicodeStringFactoryCreateICU(nanoem_status_t *status);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactoryDestroyICU(nanoem_unicode_string_factory_t *factory);

NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactoryToUtf8OnStackICU(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_u8_t *buffer, nanoem_rsize_t capacity, nanoem_status_t *status);
NANOEM_DECL_API const UChar *APIENTRY
nanoemUnicodeStringGetData(const nanoem_unicode_string_t *string);
NANOEM_DECL_API nanoem_rsize_t APIENTRY
nanoemUnicodeStringGetLength(const nanoem_unicode_string_t *string);

#endif /* NANOEM_EXT_ICU_H_ */
