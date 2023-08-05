/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/effect/RenderTargetDepthStencilImageContainer.h"

#include "emapp/Effect.h"
#include "emapp/StringUtils.h"
#include "emapp/effect/RenderTargetColorImageContainer.h"
#include "emapp/private/CommonInclude.h"

#include "bx/hash.h"

namespace nanoem {
namespace effect {
namespace {

static void
destroyAllImages(Effect *effect, RenderTargetDepthStencilImageContainer::SGImageList &images) NANOEM_DECL_NOEXCEPT
{
    for (RenderTargetDepthStencilImageContainer::SGImageList::const_iterator it = images.begin(), end = images.end();
         it != end; ++it) {
        effect->removeImageLabel(*it);
        sg::destroy_image(*it);
    }
    images.clear();
}

} /* namespace anonymous */

RenderTargetDepthStencilImageContainer::RenderTargetDepthStencilImageContainer(const String &name)
    : m_name(name)
    , m_scaleFactor(0)
    , m_dirty(false)
{
    Inline::clearZeroMemory(m_depthStencilImageDescription);
    Inline::clearZeroMemory(m_depthStencilSamplerDescription);
}

RenderTargetDepthStencilImageContainer::RenderTargetDepthStencilImageContainer(
    const RenderTargetDepthStencilImageContainer &value)
    : m_name(value.m_name)
    , m_allDepthStencilImages(value.m_allDepthStencilImages)
    , m_scaleFactor(value.m_scaleFactor)
    , m_depthStencilImageDescription(value.m_depthStencilImageDescription)
    , m_depthStencilSamplerDescription(value.m_depthStencilSamplerDescription)
    , m_dirty(false)
{
}

sg_image
RenderTargetDepthStencilImageContainer::findImage(const Effect *effect, const sg_image_desc &colorImageDescription)
{
    bx::HashMurmur2A hash;
    hash.begin(0);
    hash.add(colorImageDescription.width);
    hash.add(colorImageDescription.height);
    hash.add(colorImageDescription.pixel_format);
    hash.add(colorImageDescription.sample_count);
    hash.add(colorImageDescription.num_mipmaps);
    nanoem_u32_t key = hash.end();
    DepthStencilImageMap::const_iterator it = m_allDepthStencilImages.find(key);
    sg_image image;
    if (it != m_allDepthStencilImages.end()) {
        image = it->second;
    }
    else {
        char label[Inline::kMarkerStringLength];
        sg_image_desc depthStencilImageDescription(colorImageDescription);
        depthStencilImageDescription.pixel_format = PixelFormat::depthStencilPixelFormat();
        depthStencilImageDescription.num_mipmaps = 1;
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(label, sizeof(label), "Effects/%s/%s/DepthStencilImages/%d", effect->nameConstString(),
                m_name.c_str(), Inline::saturateInt32(m_allDepthStencilImages.size()));
            depthStencilImageDescription.label = label;
        }
        else {
            *label = 0;
        }
        image = sg::make_image(&depthStencilImageDescription);
        nanoem_assert(sg::query_image_state(image) == SG_RESOURCESTATE_VALID, "image must be valid");
        SG_LABEL_IMAGE(image, label);
        m_allDepthStencilImages.insert(tinystl::make_pair(key, image));
    }
    return image;
}

const RenderTargetDepthStencilImageContainer::SGImageList *
RenderTargetDepthStencilImageContainer::findMipmapImages(
    const Effect *effect, const sg_image_desc &colorImageDescription)
{
    bx::HashMurmur2A hash;
    hash.begin(0);
    hash.add(colorImageDescription.width);
    hash.add(colorImageDescription.height);
    hash.add(colorImageDescription.pixel_format);
    hash.add(colorImageDescription.sample_count);
    hash.add(colorImageDescription.num_mipmaps);
    nanoem_u32_t key = hash.end();
    DepthStencilImageListMap::const_iterator it = m_allDepthStencilMipmapImages.find(key);
    const SGImageList *images = nullptr;
    if (it != m_allDepthStencilMipmapImages.end()) {
        images = &it->second;
    }
    else {
        sg_image_desc depthStencilImageDescription(colorImageDescription);
        depthStencilImageDescription.pixel_format = PixelFormat::depthStencilPixelFormat();
        depthStencilImageDescription.sample_count = 1;
        const int numMipmapImages = colorImageDescription.num_mipmaps;
        SGImageList mipmapImages;
        char label[Inline::kMarkerStringLength];
        for (int i = 1; i < numMipmapImages; i++) {
            sg_image_desc mipmapImageDescription(depthStencilImageDescription);
            mipmapImageDescription.width = glm::max(mipmapImageDescription.width >> i, 1);
            mipmapImageDescription.height = glm::max(mipmapImageDescription.height >> i, 1);
            mipmapImageDescription.num_mipmaps = 0;
            if (Inline::isDebugLabelEnabled()) {
                StringUtils::format(label, sizeof(label), "Effects/%s/%s/DepthStencilImage/Mipmaps/%d",
                    effect->nameConstString(), mipmapImageDescription.label, i);
                mipmapImageDescription.label = label;
            }
            else {
                *label = 0;
            }
            sg_image image = sg::make_image(&mipmapImageDescription);
            nanoem_assert(sg::query_image_state(image) == SG_RESOURCESTATE_VALID, "image must be valid");
            mipmapImages.push_back(image);
        }
        images = &m_allDepthStencilMipmapImages.insert(tinystl::make_pair(key, mipmapImages)).first->second;
    }
    return images;
}

void
RenderTargetDepthStencilImageContainer::create(Effect *effect)
{
    SG_PUSH_GROUPF("effect::RenderDepthImageContainer::create(name=%s, width=%d, height=%d)", m_name.c_str(),
        m_depthStencilImageDescription.width, m_depthStencilImageDescription.height);
    bx::HashMurmur2A hash;
    hash.begin(0);
    hash.add(m_depthStencilImageDescription.width);
    hash.add(m_depthStencilImageDescription.height);
    hash.add(m_depthStencilImageDescription.sample_count);
    char label[Inline::kMarkerStringLength];
    nanoem_u32_t key = hash.end();
    sg_image image = { SG_INVALID_ID };
    sg_image_desc depthStencilImageDescription(m_depthStencilImageDescription);
    depthStencilImageDescription.num_mipmaps = 1;
    if (Inline::isDebugLabelEnabled()) {
        StringUtils::format(
            label, sizeof(label), "Effects/%s/%s/DepthStencilImage", effect->nameConstString(), m_name.c_str());
        depthStencilImageDescription.label = label;
    }
    else {
        *label = 0;
    }
    image = sg::make_image(&depthStencilImageDescription);
    nanoem_assert(sg::query_image_state(image) == SG_RESOURCESTATE_VALID, "image must be valid");
    effect->setImageLabel(image, m_name);
    m_allDepthStencilImages.insert(tinystl::make_pair(key, image));
    const int numMipmapImages = m_depthStencilImageDescription.num_mipmaps;
    SGImageList mipmapImages;
    for (int i = 1; i < numMipmapImages; i++) {
        sg_image_desc mipmapImageDescription(m_depthStencilImageDescription);
        mipmapImageDescription.width = glm::max(mipmapImageDescription.width >> i, 1);
        mipmapImageDescription.height = glm::max(mipmapImageDescription.height >> i, 1);
        mipmapImageDescription.num_mipmaps = 0;
        mipmapImageDescription.sample_count = 1;
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(label, sizeof(label), "Effects/%s/%s/DepthStencilImage/Mipmaps/%d",
                effect->nameConstString(), m_name.c_str(), i);
            mipmapImageDescription.label = label;
        }
        sg_image image = sg::make_image(&mipmapImageDescription);
        nanoem_assert(sg::query_image_state(image) == SG_RESOURCESTATE_VALID, "image must be valid");
        mipmapImages.push_back(image);
    }
    m_allDepthStencilMipmapImages.insert(tinystl::make_pair(key, mipmapImages));
    SG_POP_GROUP();
}

void
RenderTargetDepthStencilImageContainer::resizeWithScale(const Vector2UI16 &size)
{
    if (m_scaleFactor.x > 0 && m_scaleFactor.y > 0) {
        const Vector2SI32 newSize(Vector2(size) * m_scaleFactor);
        m_depthStencilImageDescription.width = newSize.x;
        m_depthStencilImageDescription.height = newSize.y;
        m_dirty = true;
    }
}

void
RenderTargetDepthStencilImageContainer::setSampleCount(int value)
{
    if (m_depthStencilImageDescription.sample_count != value) {
        m_depthStencilImageDescription.sample_count = value;
        m_dirty = true;
    }
}

void
RenderTargetDepthStencilImageContainer::invalidate(Effect *effect)
{
    if (m_dirty) {
        destroy(effect);
        create(effect);
        m_dirty = false;
    }
}

void
RenderTargetDepthStencilImageContainer::setDepthStencilImageDescription(const sg_image_desc &value)
{
    m_depthStencilImageDescription = value;
    m_dirty = true;
}

void
RenderTargetDepthStencilImageContainer::setScaleFactor(const Vector2 &value)
{
    m_scaleFactor = value;
    m_dirty = true;
}

void
RenderTargetDepthStencilImageContainer::destroy(Effect *effect) NANOEM_DECL_NOEXCEPT
{
    SG_PUSH_GROUPF("effect::RenderDepthImageContainer::destroy(name=%s)", m_name.c_str());
    for (DepthStencilImageMap::const_iterator it = m_allDepthStencilImages.begin(), end = m_allDepthStencilImages.end();
         it != end; ++it) {
        effect->removeImageLabel(it->second);
        sg::destroy_image(it->second);
    }
    m_allDepthStencilImages.clear();
    for (DepthStencilImageListMap::iterator it = m_allDepthStencilMipmapImages.begin(),
                                            end = m_allDepthStencilMipmapImages.end();
         it != end; ++it) {
        destroyAllImages(effect, it->second);
    }
    m_allDepthStencilMipmapImages.clear();
    SG_POP_GROUP();
}

const char *
RenderTargetDepthStencilImageContainer::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_name.c_str();
}

const sg_image_desc &
RenderTargetDepthStencilImageContainer::depthStencilImageDescription() const NANOEM_DECL_NOEXCEPT
{
    return m_depthStencilImageDescription;
}

const sg_sampler_desc &
RenderTargetDepthStencilImageContainer::depthStencilSamplerDescription() const NANOEM_DECL_NOEXCEPT
{
    return m_depthStencilSamplerDescription;
}

} /* namespace effect */
} /* namespace nanoem */
