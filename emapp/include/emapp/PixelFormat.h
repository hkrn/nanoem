/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_PIXELFORMAT_H_
#define NANOEM_EMAPP_PIXELFORMAT_H_

#include "emapp/Forward.h"

#include "bx/hash.h"

namespace nanoem {

struct PixelFormat {
    PixelFormat();
    PixelFormat(const PixelFormat &value);
    ~PixelFormat();

    nanoem_u32_t hash() const;
    void addHash(bx::HashMurmur2A &value) const;
    void reset();

    sg_pixel_format m_colorPixelFormats[SG_MAX_COLOR_ATTACHMENTS];
    sg_pixel_format m_depthPixelFormat;
    int m_numColorAttachments;
    int m_numSamples;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_PIXELFORMAT_H_ */
