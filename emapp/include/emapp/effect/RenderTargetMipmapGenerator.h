/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#pragma once
#ifndef NANOEM_EMAPP_EFFECT_RENDERTARGETMIPMAPGENERATOR_H_
#define NANOEM_EMAPP_EFFECT_RENDERTARGETMIPMAPGENERATOR_H_

#include "emapp/Forward.h"
#include "emapp/PixelFormat.h"

namespace nanoem {

class Effect;
class Project;

namespace internal {
class BlitPass;
} /* internal */

namespace effect {

class RenderTargetColorImageContainer;
class RenderTargetDepthStencilImageContainer;
class OffscreenRenderTargetImageContainer;

class RenderTargetMipmapGenerator NANOEM_DECL_SEALED : private NonCopyable {
public:
    typedef tinystl::vector<sg_image, TinySTLAllocator> SGImageList;
    typedef tinystl::vector<sg_pass, TinySTLAllocator> SGPassList;

    RenderTargetMipmapGenerator(Effect *effect, const char *name, const sg_image_desc &desc);
    ~RenderTargetMipmapGenerator();

    void blitSourceImage(const Effect *effect, sg_image color, const PixelFormat &format, const char *name);
    void generateAllMipmapImages(const Effect *effect, const PixelFormat &format,
        RenderTargetColorImageContainer *colorImageContainer,
        RenderTargetDepthStencilImageContainer *depthStencilImageContainer, const char *name);
    void generateAllMipmapImages(const Effect *effect, const PixelFormat &format,
        OffscreenRenderTargetImageContainer *offscreenImageContainer, const char *name);
    void generateAllMipmapImages(
        const Effect *effect, const PixelFormat &format, const sg_image_desc &colorImageDesc, const char *name);
    void destroy(Effect *effect) NANOEM_DECL_NOEXCEPT;

private:
    Project *m_project;
    internal::BlitPass *m_blitter;
    SGPassList m_destPasses;
    SGPassList m_sourcePasses;
    SGImageList m_sourceColorImages;
    SGImageList m_sourceDepthImages;
};

} /* namespace effect */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_EFFECT_RENDERTARGETMIPMAPGENERATOR_H_ */
