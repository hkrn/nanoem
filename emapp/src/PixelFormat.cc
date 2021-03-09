/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/PixelFormat.h"

#include "emapp/private/CommonInclude.h"

namespace nanoem {

PixelFormat::PixelFormat()
    : m_depthPixelFormat(SG_PIXELFORMAT_DEPTH_STENCIL)
    , m_numColorAttachments(1)
    , m_numSamples(1)
{
    Inline::clearZeroMemory(m_colorPixelFormats);
}

PixelFormat::PixelFormat(const PixelFormat &value)
    : m_depthPixelFormat(value.m_depthPixelFormat)
    , m_numColorAttachments(value.m_numColorAttachments)
    , m_numSamples(value.m_numSamples)
{
    memcpy(m_colorPixelFormats, value.m_colorPixelFormats, sizeof(m_colorPixelFormats));
}

PixelFormat::~PixelFormat()
{
}

nanoem_u32_t
PixelFormat::hash() const
{
    bx::HashMurmur2A v;
    v.begin();
    addHash(v);
    return v.end();
}

void
PixelFormat::addHash(bx::HashMurmur2A &value) const
{
    value.add(m_colorPixelFormats, sizeof(m_colorPixelFormats));
    value.add(m_depthPixelFormat);
    value.add(m_numColorAttachments);
    value.add(m_numSamples);
}

void
PixelFormat::reset()
{
    for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
        m_colorPixelFormats[i] = _SG_PIXELFORMAT_DEFAULT;
    }
    m_depthPixelFormat = _SG_PIXELFORMAT_DEFAULT;
    m_numColorAttachments = 1;
    m_numSamples = 1;
}

} /* namespace nanoem */
