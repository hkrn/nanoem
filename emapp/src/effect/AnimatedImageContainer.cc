/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/effect/AnimatedImageContainer.h"

#include "emapp/ImageLoader.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace effect {

AnimatedImageContainer::AnimatedImageContainer(
    const String &name, const String &seekVariable, image::APNG *image, nanoem_f32_t offset, nanoem_f32_t speed)
    : m_name(name)
    , m_seekVariable(seekVariable)
    , m_data(image)
    , m_offset(offset)
    , m_speed(speed)
    , m_dirty(false)
{
    Inline::clearZeroMemory(m_colorImageDescription);
    m_colorImage = { SG_INVALID_ID };
}

AnimatedImageContainer::~AnimatedImageContainer() NANOEM_DECL_NOEXCEPT
{
}

void
AnimatedImageContainer::create()
{
    if (!sg::is_valid(m_colorImage)) {
        m_colorImageDescription.width = m_data->width();
        m_colorImageDescription.height = m_data->height();
        m_colorImageDescription.type = SG_IMAGETYPE_2D;
        m_colorImageDescription.usage = SG_USAGE_STREAM;
        m_colorImageDescription.wrap_u = m_colorImageDescription.wrap_v = SG_WRAP_REPEAT;
        m_colorImageDescription.mag_filter = m_colorImageDescription.min_filter = SG_FILTER_LINEAR;
        m_colorImageDescription.pixel_format = SG_PIXELFORMAT_RGBA8;
        if (Inline::isDebugLabelEnabled()) {
            m_colorImageDescription.label = m_name.c_str();
        }
        m_colorImage = sg::make_image(&m_colorImageDescription);
    }
}

void
AnimatedImageContainer::update(nanoem_f32_t seconds)
{
    if (m_dirty) {
        const nanoem_rsize_t index = m_data->findNearestOffset(seconds);
        sg_image_data content;
        Inline::clearZeroMemory(content);
        if (const ByteArray *composition = m_data->compositedFrameImage(index)) {
            sg_range &c = content.subimage[0][0];
            c.ptr = composition->data();
            c.size = composition->size();
            sg::update_image(m_colorImage, &content);
            m_dirty = false;
        }
    }
}

void
AnimatedImageContainer::markDirty()
{
    m_dirty = true;
}

void
AnimatedImageContainer::destroy() NANOEM_DECL_NOEXCEPT
{
    SG_INSERT_MARKERF("effect::AnimatedImageContainer::destroy(id=%d)", m_colorImage.id);
    sg::destroy_image(m_colorImage);
    m_colorImage = { SG_INVALID_ID };
    nanoem_delete_safe(m_data);
}

const char *
AnimatedImageContainer::seekVariableNameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_seekVariable.c_str();
}

sg_image
AnimatedImageContainer::colorImage() const NANOEM_DECL_NOEXCEPT
{
    return m_colorImage;
}

nanoem_f32_t
AnimatedImageContainer::offset() const NANOEM_DECL_NOEXCEPT
{
    return m_offset;
}

nanoem_f32_t
AnimatedImageContainer::speed() const NANOEM_DECL_NOEXCEPT
{
    return m_speed;
}

} /* namespace effect */
} /* namespace nanoem */
