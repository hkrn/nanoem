/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EXT_CONVERTER_H_
#define NANOEM_EXT_CONVERTER_H_

#include "./mutable.h"

/**
 * \defgroup nanoem nanoem
 * @{
 */

/**
 * \defgroup nanoem_model_converter_t Model Converter
 * @{
 */
NANOEM_DECL_OPAQUE(nanoem_model_converter_t);

NANOEM_DECL_API nanoem_model_converter_t *APIENTRY
nanoemModelConverterCreate(nanoem_model_t *model, nanoem_status_t *status);
NANOEM_DECL_API nanoem_mutable_model_t *APIENTRY
nanoemModelConverterExecute(nanoem_model_converter_t *converter, nanoem_model_format_type_t target, nanoem_status_t *status);
NANOEM_DECL_API void APIENTRY
nanoemModelConverterDestroy(nanoem_model_converter_t *converter);
/** @} */

/** @} */

#endif /* NANOEM_EXT_CONVERTER_H_ */
