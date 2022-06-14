/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/effect/OffscreenRenderTargetImageContainer.h"

#include "emapp/Effect.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace effect {
namespace {

static void
destroyAllImages(Effect *effect, OffscreenRenderTargetImageContainer::SGImageList &images) NANOEM_DECL_NOEXCEPT
{
    for (OffscreenRenderTargetImageContainer::SGImageList::const_iterator it = images.begin(), end = images.end();
         it != end; ++it) {
        effect->removeImageLabel(*it);
        sg::destroy_image(*it);
    }
    images.clear();
}

} /* namespace anonymous */

OffscreenRenderTargetImageContainer::OffscreenRenderTargetImageContainer(const String &name)
    : RenderTargetColorImageContainer(name)
{
    Inline::clearZeroMemory(m_depthStencilImageDescription);
    m_depthStencilImage = { SG_INVALID_ID };
}

OffscreenRenderTargetImageContainer::OffscreenRenderTargetImageContainer(
    const OffscreenRenderTargetImageContainer &value)
    : RenderTargetColorImageContainer(value)
    , m_depthStencilImage(value.m_depthStencilImage)
    , m_depthStencilImageDescription(value.m_depthStencilImageDescription)
{
}

void
OffscreenRenderTargetImageContainer::generateDepthStencilMipmapImages(const Effect *effect)
{
    if (m_depthStencilMipmapImages.empty()) {
        sg_image_desc depthStencilImageDescription(colorImageDescription());
        depthStencilImageDescription.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
        const int numMipmapImages = depthStencilImageDescription.num_mipmaps;
        char label[Inline::kMarkerStringLength];
        for (int i = 1; i < numMipmapImages; i++) {
            sg_image_desc mipmapImageDescription(depthStencilImageDescription);
            mipmapImageDescription.width = glm::max(mipmapImageDescription.width >> i, 1);
            mipmapImageDescription.height = glm::max(mipmapImageDescription.height >> i, 1);
            mipmapImageDescription.num_mipmaps = 1;
            if (Inline::isDebugLabelEnabled()) {
                StringUtils::format(label, sizeof(label), "Effects/%s/%s/DepthStencilImage/Mipmaps/%d",
                    effect->nameConstString(), nameConstString(), i);
                mipmapImageDescription.label = label;
            }
            else {
                *label = 0;
            }
            sg_image image = sg::make_image(&mipmapImageDescription);
            nanoem_assert(sg::query_image_state(image) == SG_RESOURCESTATE_VALID, "image must be valid");
            m_depthStencilMipmapImages.push_back(image);
        }
    }
}

void
OffscreenRenderTargetImageContainer::create(Effect *effect)
{
    SG_PUSH_GROUPF("effect::OffscreenRenderTargetImageContainer::create(name=%s, width=%d, height=%d)",
        nameConstString(), colorImageDescription().width, colorImageDescription().height);
    char label[Inline::kMarkerStringLength];
    RenderTargetColorImageContainer::create(effect);
    if (Inline::isDebugLabelEnabled()) {
        sg_image_desc labeledDepthStencilImageDescription(m_depthStencilImageDescription);
        StringUtils::format(
            label, sizeof(label), "Effects/%s/%s/DepthStencilImage", effect->nameConstString(), nameConstString());
        labeledDepthStencilImageDescription.label = label;
        m_depthStencilImage = sg::make_image(&labeledDepthStencilImageDescription);
    }
    else {
        *label = 0;
        m_depthStencilImage = sg::make_image(&m_depthStencilImageDescription);
    }
    nanoem_assert(sg::query_image_state(m_depthStencilImage) == SG_RESOURCESTATE_VALID, "image must be valid");
    effect->setImageLabel(m_depthStencilImage, nameConstString());
    SG_POP_GROUP();
}

void
OffscreenRenderTargetImageContainer::create(Effect *effect, const Vector2UI16 &size, const Vector2 &scaleFactor,
    int numMipLevels, int sampleCount, sg_pixel_format format)
{
    setColorImageDescription(size, numMipLevels, sampleCount, format);
    m_depthStencilImageDescription = colorImageDescription();
    m_depthStencilImageDescription.num_mipmaps = 1;
    m_depthStencilImageDescription.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
    setScaleFactor(scaleFactor);
    create(effect);
}

void
OffscreenRenderTargetImageContainer::resizeWithScale(Effect *effect, const Vector2UI16 &size)
{
    /* also resize color image to prevent duplicated "leak" create call */
    const Vector2 s(scaleFactor());
    if (s.x > 0 && s.y > 0) {
        const Vector2SI32 newSize(Vector2(size) * s);
        resizeColorImageDescription(newSize.x, newSize.y);
        m_depthStencilImageDescription.width = newSize.x;
        m_depthStencilImageDescription.height = newSize.y;
        destroy(effect);
        create(effect);
    }
}

void
OffscreenRenderTargetImageContainer::destroy(Effect *effect) NANOEM_DECL_NOEXCEPT
{
    SG_PUSH_GROUPF("effect::OffscreenRenderTargetImageContainer::destroy(name=%s)", nameConstString());
    RenderTargetColorImageContainer::destroy(effect);
    destroyAllImages(effect, m_depthStencilMipmapImages);
    m_depthStencilMipmapImages.clear();
    effect->removeImageLabel(m_depthStencilImage);
    sg::destroy_image(m_depthStencilImage);
    m_depthStencilImage = { SG_INVALID_ID };
    SG_POP_GROUP();
}

const sg_image_desc &
OffscreenRenderTargetImageContainer::depthStencilImageDescription() const NANOEM_DECL_NOEXCEPT
{
    return m_depthStencilImageDescription;
}

sg_image
OffscreenRenderTargetImageContainer::depthStencilImageHandle() const NANOEM_DECL_NOEXCEPT
{
    return m_depthStencilImage;
}

sg_image
OffscreenRenderTargetImageContainer::mipmapDepthStencilImageHandleAt(int offset) const
{
    return m_depthStencilMipmapImages[offset];
}

} /* namespace effect */
} /* namespace nanoem */
