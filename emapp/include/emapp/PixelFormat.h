/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_PIXELFORMAT_H_
#define NANOEM_EMAPP_PIXELFORMAT_H_

#include "emapp/Forward.h"

#include "bx/hash.h"

namespace nanoem {

class PixelFormat NANOEM_DECL_SEALED : private NonCopyable {
public:
    static sg_pixel_format depthStencilPixelFormat() NANOEM_DECL_NOEXCEPT;

    PixelFormat() NANOEM_DECL_NOEXCEPT;
    PixelFormat(const PixelFormat &value) NANOEM_DECL_NOEXCEPT;
    ~PixelFormat() NANOEM_DECL_NOEXCEPT;

    nanoem_u32_t hash() const NANOEM_DECL_NOEXCEPT;
    void addHash(bx::HashMurmur2A &value) const;
    void reset(int numSamples);

    sg_pixel_format colorPixelFormat(nanoem_rsize_t offset) const NANOEM_DECL_NOEXCEPT;
    void setColorPixelFormat(sg_pixel_format value, nanoem_rsize_t offset);
    sg_pixel_format depthPixelFormat() const NANOEM_DECL_NOEXCEPT;
    void setDepthPixelFormat(sg_pixel_format value);
    int numColorAttachments() const NANOEM_DECL_NOEXCEPT;
    void setNumColorAttachemnts(int value);
    int numSamples() const NANOEM_DECL_NOEXCEPT;
    void setNumSamples(int value);

    void operator=(const PixelFormat &value) NANOEM_DECL_NOEXCEPT;

private:
    sg_pixel_format m_colorPixelFormats[SG_MAX_COLOR_ATTACHMENTS];
    sg_pixel_format m_depthPixelFormat;
    int m_numColorAttachments;
    int m_numSamples;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_PIXELFORMAT_H_ */
