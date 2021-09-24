/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/PixelFormat.h"

#include "emapp/private/CommonInclude.h"

namespace nanoem {

PixelFormat::PixelFormat() NANOEM_DECL_NOEXCEPT
    : m_depthPixelFormat(_SG_PIXELFORMAT_DEFAULT)
    , m_numColorAttachments(1)
    , m_numSamples(1)
{
    Inline::clearZeroMemory(m_colorPixelFormats);
}

PixelFormat::PixelFormat(const PixelFormat &value) NANOEM_DECL_NOEXCEPT
    : m_depthPixelFormat(value.m_depthPixelFormat)
    , m_numColorAttachments(value.m_numColorAttachments)
    , m_numSamples(value.m_numSamples)
{
    memcpy(m_colorPixelFormats, value.m_colorPixelFormats, sizeof(m_colorPixelFormats));
}

PixelFormat::~PixelFormat() NANOEM_DECL_NOEXCEPT
{
}

nanoem_u32_t
PixelFormat::hash() const NANOEM_DECL_NOEXCEPT
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
PixelFormat::reset(int numSamples)
{
    for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
        m_colorPixelFormats[i] = _SG_PIXELFORMAT_DEFAULT;
    }
    m_depthPixelFormat = _SG_PIXELFORMAT_DEFAULT;
    m_numColorAttachments = 1;
    m_numSamples = numSamples;
}

sg_pixel_format
PixelFormat::colorPixelFormat(nanoem_rsize_t offset) const NANOEM_DECL_NOEXCEPT
{
    return offset < SG_MAX_COLOR_ATTACHMENTS ? m_colorPixelFormats[offset] : SG_PIXELFORMAT_NONE;
}

void
PixelFormat::setColorPixelFormat(sg_pixel_format value, nanoem_rsize_t offset)
{
    if (offset < SG_MAX_COLOR_ATTACHMENTS) {
        m_colorPixelFormats[offset] = value;
    }
}

sg_pixel_format
PixelFormat::depthPixelFormat() const NANOEM_DECL_NOEXCEPT
{
    return m_depthPixelFormat;
}

void
PixelFormat::setDepthPixelFormat(sg_pixel_format value)
{
    m_depthPixelFormat = value;
}

int
PixelFormat::numColorAttachments() const NANOEM_DECL_NOEXCEPT
{
    return m_numColorAttachments;
}

void
PixelFormat::setNumColorAttachemnts(int value)
{
    m_numColorAttachments = glm::clamp(value, 1, int(SG_MAX_COLOR_ATTACHMENTS));
}

int
PixelFormat::numSamples() const NANOEM_DECL_NOEXCEPT
{
    return m_numSamples;
}

void
PixelFormat::setNumSamples(int value)
{
    m_numSamples = glm::max(value, 1);
}

void
PixelFormat::operator=(const PixelFormat &value) NANOEM_DECL_NOEXCEPT
{
    memcpy(m_colorPixelFormats, value.m_colorPixelFormats, sizeof(m_colorPixelFormats));
    m_depthPixelFormat = value.m_depthPixelFormat;
    m_numColorAttachments = value.m_numColorAttachments;
    m_numSamples = value.m_numSamples;
}

} /* namespace nanoem */
