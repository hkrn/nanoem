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

/**
 * \brief Create an opaque model converter object
 *
 * \param model The opaque input model object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_model_converter_t *APIENTRY
nanoemModelConverterCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief Perform converting the opaque model object and creates new opaque converted model object
 *
 * \param converter The opaque model converter object
 * \param target The target model format type
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \return The opaque converted mutable model object
 */
NANOEM_DECL_API nanoem_mutable_model_t *APIENTRY
nanoemModelConverterExecute(nanoem_model_converter_t *converter, nanoem_model_format_type_t target, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque model converter object
 *
 * \param converter The opaque model converter object
 */
NANOEM_DECL_API void APIENTRY
nanoemModelConverterDestroy(nanoem_model_converter_t *converter);
/** @} */

/** @} */

#endif /* NANOEM_EXT_CONVERTER_H_ */
