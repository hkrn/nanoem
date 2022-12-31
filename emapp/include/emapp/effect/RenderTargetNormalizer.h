/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#pragma once
#ifndef NANOEM_EMAPP_EFFECT_RENDERTARGETNORMALIZER_H_
#define NANOEM_EMAPP_EFFECT_RENDERTARGETNORMALIZER_H_

#include "emapp/PixelFormat.h"

namespace nanoem {

class Effect;
class IDrawable;

namespace effect {

class RenderTargetColorImageContainer;
class Pass;

class RenderTargetNormalizer NANOEM_DECL_SEALED : private NonCopyable {
public:
    RenderTargetNormalizer(Effect *effect);
    ~RenderTargetNormalizer() NANOEM_DECL_NOEXCEPT;

    void destroy();
    void normalize(const IDrawable *drawable, const effect::Pass *passPtr, const PixelFormat &originFormat,
        sg_pixel_format normalizedColorFormat, sg_pass_desc &currentPassDescriptionRef);
    void read();
    void apply(const sg_bindings &bindings, const IDrawable *drawable, effect::Pass *pass, sg::PassBlock &pb);
    void write();
    void normalizePrimaryViewportImage(const PixelFormat &originFormat, const sg_pass_desc &pd, nanoem_rsize_t index,
        sg_pixel_format normalizedColorFormat, sg_pass_desc &currentPassDescriptionRef);
    void normalizeOffscreenRenderTargetColorImage(const sg_image_desc &originImageDescription,
        const PixelFormat &originFormat, const sg_pass_desc &originPassDescription, nanoem_rsize_t index,
        sg_pixel_format normalizedColorFormat, sg_pass_desc &currentPassDescriptionRef);
    void normalizeRenderTargetColorImage(const RenderTargetColorImageContainer *containerPtr,
        const PixelFormat &originFormat, const sg_pass_desc &originPassDescription, nanoem_rsize_t index,
        sg_pixel_format normalizedColorFormat, sg_pass_desc &currentPassDescriptionRef);
    void resolveRenderTargetIOConfliction(const effect::Pass *passPtr,
        const RenderTargetColorImageContainer *containerPtr, const PixelFormat &originFormat,
        const sg_pass_desc &originPassDescription, nanoem_rsize_t index, sg_pixel_format normalizedColorFormat,
        sg_pass_desc &currentPassDescriptionRef);
    void createNormalizePass(const char *name, const PixelFormat &originFormat,
        const sg_image_desc &normalizedImageDescription, const sg_pass_desc &originPassDescription,
        nanoem_rsize_t index);

    PixelFormat normalizedColorImagePixelFormat() const NANOEM_DECL_NOEXCEPT;
    void setNormalizedColorImagePixelFormat(const PixelFormat &value);

private:
    Effect *m_effect;
    PixelFormat m_normalizedColorImageFormat;
    PixelFormat m_originColorImageFormats[SG_MAX_COLOR_ATTACHMENTS];
    String m_normalizedColorImageNames[SG_MAX_COLOR_ATTACHMENTS];
    String m_originColorImageNames[SG_MAX_COLOR_ATTACHMENTS];
    sg::NamedImage m_originColorImageRefs[SG_MAX_COLOR_ATTACHMENTS];
    sg::NamedImage m_normalizedColorImages[SG_MAX_COLOR_ATTACHMENTS];
    sg::NamedPass m_originPasses[SG_MAX_COLOR_ATTACHMENTS];
    sg::NamedPass m_normalizedPasses[SG_MAX_COLOR_ATTACHMENTS];
};

} /* namespace effect */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_EFFECT_RENDERTARGETNORMALIZER_H_ */
