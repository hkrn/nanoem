/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/effect/RenderTargetColorImageContainer.h"

#include "emapp/Effect.h"
#include "emapp/StringUtils.h"
#include "emapp/effect/RenderTargetMipmapGenerator.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace effect {

const bool RenderTargetColorImageContainer::kAntialiasEnabled = true;

void
RenderTargetColorImageContainer::setDescription(
    const Vector2SI32 &size, int numMipLevels, int sampleCount, sg_pixel_format format, sg_image_desc &desc)
{
    desc.width = size.x;
    desc.height = size.y;
    desc.num_mipmaps = numMipLevels;
    desc.pixel_format = format;
    desc.sample_count = sampleCount;
    desc.render_target = true;
}

RenderTargetColorImageContainer::RenderTargetColorImageContainer(const String &name)
    : m_name(name)
    , m_mipmapGenerator(nullptr)
    , m_scaleFactor(0)
    , m_sharedTexture(false)
    , m_dirty(false)
{
    Inline::clearZeroMemory(m_colorImageDescription);
    Inline::clearZeroMemory(m_colorSamplerDescription);
    m_colorImage = m_msaaImage = { SG_INVALID_ID };
    m_sampler = { SG_INVALID_ID };
    if (Inline::isDebugLabelEnabled()) {
        m_colorImageDescription.label = m_name.c_str();
    }
}

RenderTargetColorImageContainer::RenderTargetColorImageContainer(const RenderTargetColorImageContainer &value)
    : m_name(value.m_name)
    , m_mipmapGenerator(nullptr)
    , m_scaleFactor(value.m_scaleFactor)
    , m_colorImage(value.m_colorImage)
    , m_msaaImage(value.m_msaaImage)
    , m_colorImageDescription(value.m_colorImageDescription)
    , m_colorSamplerDescription(value.m_colorSamplerDescription)
    , m_sharedTexture(value.m_sharedTexture)
    , m_dirty(false)
{
}

void
RenderTargetColorImageContainer::create(Effect *effect)
{
    SG_PUSH_GROUPF("effect::RenderTargetImageContainer::create(name=%s, width=%d, height=%d)", m_name.c_str(),
        m_colorImageDescription.width, m_colorImageDescription.height);
    char label[Inline::kMarkerStringLength];
    sg_image_desc colorImageDescription(m_colorImageDescription);
    if (Inline::isDebugLabelEnabled()) {
        StringUtils::format(
            label, sizeof(label), "Effects/%s/%s/ColorImage", effect->nameConstString(), m_name.c_str());
        colorImageDescription.label = label;
    }
    else {
        *label = 0;
    }
    if (colorImageDescription.sample_count > 1) {
        sg_image_desc innerColorImageDescription(colorImageDescription);
        innerColorImageDescription.num_mipmaps = 0;
        m_msaaImage = sg::make_image(&innerColorImageDescription);
        colorImageDescription.sample_count = 1;
    }
    m_colorImage = sg::make_image(&colorImageDescription);
    nanoem_assert(sg::query_image_state(m_colorImage) == SG_RESOURCESTATE_VALID, "image must be valid");
    effect->setImageLabel(m_colorImage, m_name);
    m_sampler = sg::make_sampler(&m_colorSamplerDescription);
    effect->setSamplerLabel(m_sampler, m_name.c_str());
    nanoem_assert(sg::query_sampler_state(m_sampler) == SG_RESOURCESTATE_VALID, "sampler must be valid");
    SG_POP_GROUP();
}

void
RenderTargetColorImageContainer::resizeWithScale(const Vector2UI16 &size)
{
    if (m_scaleFactor.x > 0 && m_scaleFactor.y > 0) {
        const Vector2SI32 newSize(Vector2(size) * m_scaleFactor);
        resizeColorImageDescription(newSize.x, newSize.y);
        m_dirty = true;
    }
}

void
RenderTargetColorImageContainer::setSampleCount(int value)
{
    if (m_colorImageDescription.sample_count != value) {
        m_colorImageDescription.sample_count = value;
        m_dirty = true;
    }
}

void
RenderTargetColorImageContainer::invalidate(Effect *effect)
{
    if (m_dirty) {
        destroy(effect);
        create(effect);
        m_dirty = false;
    }
}

void
RenderTargetColorImageContainer::share(const RenderTargetColorImageContainer *value)
{
    m_colorImage = value->m_colorImage;
    m_colorImageDescription = value->m_colorImageDescription;
    m_sharedTexture = true;
}

void
RenderTargetColorImageContainer::inherit(const RenderTargetColorImageContainer *shared)
{
    m_colorImage = shared->m_colorImage;
    m_colorImageDescription = shared->m_colorImageDescription;
}

void
RenderTargetColorImageContainer::getPassDescription(size_t offset, sg_pass_desc &desc) const NANOEM_DECL_NOEXCEPT
{
    if (sg::is_valid(m_msaaImage)) {
        desc.color_attachments[offset].image = m_msaaImage;
        desc.resolve_attachments[offset].image = m_colorImage;
    }
    else {
        desc.color_attachments[offset].image = m_colorImage;
    }
}

void
RenderTargetColorImageContainer::setMipmapGenerator(RenderTargetMipmapGenerator *value)
{
    m_mipmapGenerator = value;
}

void
RenderTargetColorImageContainer::setColorImageDescription(const sg_image_desc &value)
{
    m_colorImageDescription = value;
    m_dirty = true;
}

void
RenderTargetColorImageContainer::setColorSamplerDescription(const sg_sampler_desc &value)
{
    m_colorSamplerDescription = value;
    m_dirty = true;
}

void
RenderTargetColorImageContainer::setColorImageHandle(sg_image value)
{
    m_colorImage = value;
}

void
RenderTargetColorImageContainer::setScaleFactor(const Vector2 &value)
{
    m_scaleFactor = value;
}

void
RenderTargetColorImageContainer::destroy(Effect *effect) NANOEM_DECL_NOEXCEPT
{
    SG_PUSH_GROUPF("effect::RenderTargetColorImageContainer::destroy(name=%s)", m_name.c_str());
    if (!m_sharedTexture) {
        effect->removeImageLabel(m_colorImage);
        sg::destroy_image(m_colorImage);
        sg::destroy_image(m_msaaImage);
    }
    m_colorImage = { SG_INVALID_ID };
    m_msaaImage = { SG_INVALID_ID };
    if (m_mipmapGenerator) {
        m_mipmapGenerator->destroy(effect);
        nanoem_delete_safe(m_mipmapGenerator);
    }
    SG_POP_GROUP();
}

RenderTargetMipmapGenerator *
RenderTargetColorImageContainer::mipmapGenerator()
{
    return m_mipmapGenerator;
}

const char *
RenderTargetColorImageContainer::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_name.c_str();
}

const sg_image_desc &
RenderTargetColorImageContainer::colorImageDescription() const NANOEM_DECL_NOEXCEPT
{
    return m_colorImageDescription;
}

const sg_sampler_desc &
RenderTargetColorImageContainer::samplerImageDescription() const NANOEM_DECL_NOEXCEPT
{
    return m_colorSamplerDescription;
}

const Vector2
RenderTargetColorImageContainer::scaleFactor() const NANOEM_DECL_NOEXCEPT
{
    return m_scaleFactor;
}

sg_image
RenderTargetColorImageContainer::preferredColorImageHandle() const NANOEM_DECL_NOEXCEPT
{
    sg_image handle = { SG_INVALID_ID };
    if (sg::is_valid(m_colorImage)) {
        handle = m_colorImage;
    }
    return handle;
}

sg_image
RenderTargetColorImageContainer::colorImageHandle() const NANOEM_DECL_NOEXCEPT
{
    return m_colorImage;
}

sg_image
RenderTargetColorImageContainer::msaaImageHandle() const NANOEM_DECL_NOEXCEPT
{
    return m_msaaImage;
}

sg_sampler
RenderTargetColorImageContainer::samplerHandle() const NANOEM_DECL_NOEXCEPT
{
    return m_sampler;
}

bool
RenderTargetColorImageContainer::isSharedTexture() const NANOEM_DECL_NOEXCEPT
{
    return m_sharedTexture;
}

void
RenderTargetColorImageContainer::setColorImageDescription(
    const Vector2SI32 &size, int numMipLevels, int sampleCount, sg_pixel_format format)
{
    setDescription(size, numMipLevels, sampleCount, format, m_colorImageDescription);
}

void
RenderTargetColorImageContainer::resizeColorImageDescription(int width, int height)
{
    m_colorImageDescription.width = width;
    m_colorImageDescription.height = height;
    if (m_colorImageDescription.num_mipmaps > 0) {
        const Vector2 sizeF(width, height);
        int maxMipLevels = glm::min(int(glm::log2(glm::max(sizeF.x, sizeF.y))), int(SG_MAX_MIPMAPS));
        m_colorImageDescription.num_mipmaps = maxMipLevels;
    }
}

} /* namespace effect */
} /* namespace nanoem */
