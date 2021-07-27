/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EXT_MOTION_H_
#define NANOEM_EXT_MOTION_H_

#include "../nanoem.h"
#include "./mutable.h"

/**
 * \brief 
 * 
 * \param motion 
 * \param buffer 
 * \param offset 
 * \param status 
 * \return NANOEM_DECL_API nanoem_bool_t APIENTRY 
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMotionLoadFromBufferNMD(nanoem_motion_t *motion, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param buffer 
 * \param status 
 * \return NANOEM_DECL_API nanoem_bool_t APIENTRY 
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMutableMotionSaveToBufferNMD(nanoem_mutable_motion_t *motion, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

#endif /* NANOEM_EXT_MOTION_H_ */
