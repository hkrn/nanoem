/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#pragma once
#ifndef NANOEM_EMAPP_EFFECT_OFFSCREENRENDERTARGETIMAGECONTAINER_H_
#define NANOEM_EMAPP_EFFECT_OFFSCREENRENDERTARGETIMAGECONTAINER_H_

#include "emapp/effect/RenderTargetColorImageContainer.h"

namespace nanoem {
namespace effect {

class OffscreenRenderTargetImageContainer NANOEM_DECL_SEALED : public RenderTargetColorImageContainer {
public:
    typedef tinystl::vector<sg_image, TinySTLAllocator> SGImageList;
    static const bool kAntialiasEnabled;

    OffscreenRenderTargetImageContainer(const String &name);
    OffscreenRenderTargetImageContainer(const OffscreenRenderTargetImageContainer &value);

    void generateDepthStencilMipmapImages(const Effect *effect);
    void create(Effect *effect);
    void create(Effect *effect, const Vector2UI16 &size, const Vector2 &scaleFactor, int numMipLevels, int sampleCount,
        sg_pixel_format format);
    void resizeWithScale(Effect *effect, const Vector2UI16 &size);
    void destroy(Effect *effect) NANOEM_DECL_NOEXCEPT;

    const sg_image_desc &depthStencilImageDescription() const NANOEM_DECL_NOEXCEPT;
    sg_image depthStencilImageHandle() const NANOEM_DECL_NOEXCEPT;
    sg_image mipmapDepthStencilImageHandleAt(int offset) const;

private:
    SGImageList m_depthStencilMipmapImages;
    sg_image m_depthStencilImage;
    sg_image_desc m_depthStencilImageDescription;
};

} /* namespace effect */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_EFFECT_OFFSCREENRENDERTARGETIMAGECONTAINER_H_ */
