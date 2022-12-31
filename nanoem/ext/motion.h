/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EXT_MOTION_H_
#define NANOEM_EXT_MOTION_H_

#include "../nanoem.h"
#include "./mutable.h"

/**
 * \defgroup nanoem nanoem
 * @{
 */

/**
 * \defgroup nanoem_motion Motion
 * @{
 */

/**
 * \brief Load data as NMD from the given opaque motion object associated with the opaque buffer object
 *
 * The buffer data must be serialized with \b nanoem.motion.Motion defined in \e nanoem/proto/motion.proto
 *
 * \param motion The opaque motion object
 * \param buffer The opaque buffer object
 * \param offset The start frame index offset to load
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \return 1 if succeeded, 0 if any error occured
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMotionLoadFromBufferNMD(nanoem_motion_t *motion, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status);

/** @} */

/**
 * \defgroup nanoem_mutable_motion Mutable Motion
 * @{
 */

/**
 * \brief Save data as NMD to the opaque buffer object from the given opaque motion object
 *
 * The buffer data will be serialized with \b nanoem.motion.Motion defined in \e nanoem/proto/motion.proto
 *
 * \param motion The opaque motion object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \return 1 if succeeded, 0 if any error occured
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMutableMotionSaveToBufferNMD(nanoem_mutable_motion_t *motion, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/** @} */

/** @} */

#endif /* NANOEM_EXT_MOTION_H_ */
