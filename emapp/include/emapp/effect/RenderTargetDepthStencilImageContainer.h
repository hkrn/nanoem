/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#pragma once
#ifndef NANOEM_EMAPP_EFFECT_RENDERTARGETDEPTHSTENCILIMAGECONTAINER_H_
#define NANOEM_EMAPP_EFFECT_RENDERTARGETDEPTHSTENCILIMAGECONTAINER_H_

#include "emapp/Forward.h"

namespace nanoem {

class Effect;

namespace effect {

class RenderTargetDepthStencilImageContainer NANOEM_DECL_SEALED : private NonCopyable {
public:
    typedef tinystl::vector<sg_image, TinySTLAllocator> SGImageList;
    typedef tinystl::unordered_map<nanoem_u32_t, sg_image, TinySTLAllocator> DepthStencilImageMap;
    typedef tinystl::unordered_map<nanoem_u32_t, SGImageList, TinySTLAllocator> DepthStencilImageListMap;

    RenderTargetDepthStencilImageContainer(const String &name);
    RenderTargetDepthStencilImageContainer(const RenderTargetDepthStencilImageContainer &value);

    sg_image findImage(const Effect *effect, const sg_image_desc &colorImageDescription);
    const SGImageList *findMipmapImages(const Effect *effect, const sg_image_desc &colorImageDescription);
    void create(Effect *effect);
    void create(Effect *effect, const Vector2UI16 &size, const Vector2 &scaleFactor, int numMipLevels, int sampleCount,
        sg_pixel_format format);
    void resizeWithScale(const Vector2UI16 &size);
    void setSampleCount(int value);
    void invalidate(Effect *effect);
    void setImageDescription(const sg_image_desc &value);
    void destroy(Effect *effect) NANOEM_DECL_NOEXCEPT;

    const char *nameConstString() const NANOEM_DECL_NOEXCEPT;
    const sg_image_desc &depthStencilImageDescription() const NANOEM_DECL_NOEXCEPT;
    const sg_sampler_desc &depthStencilSamplerDescription() const NANOEM_DECL_NOEXCEPT;

private:
    const String m_name;
    DepthStencilImageListMap m_allDepthStencilMipmapImages;
    DepthStencilImageMap m_allDepthStencilImages;
    Vector2 m_scaleFactor;
    sg_image_desc m_depthStencilImageDescription;
    sg_sampler_desc m_depthStencilSamplerDescription;
    bool m_dirty;
};

} /* namespace effect */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_EFFECT_RENDERTARGETDEPTHSTENCILIMAGECONTAINER_H_ */
