/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#pragma once
#ifndef NANOEM_EMAPP_EFFECT_RENDERTARGETCOLORIMAGECONTAINER_H_
#define NANOEM_EMAPP_EFFECT_RENDERTARGETCOLORIMAGECONTAINER_H_

#include "emapp/Forward.h"

namespace nanoem {

class Effect;
class Project;

namespace effect {

class RenderTargetMipmapGenerator;

class RenderTargetColorImageContainer : private NonCopyable {
public:
    static const bool kAntialiasEnabled;
    static void setDescription(
        const Vector2SI32 &size, int numMipLevels, int sampleCount, sg_pixel_format format, sg_image_desc &desc);

    RenderTargetColorImageContainer(const String &name);
    RenderTargetColorImageContainer(const RenderTargetColorImageContainer &value);

    void create(Effect *effect);
    void create(Effect *effect, const Vector2UI16 &size, const Vector2 &scaleFactor, int numMipLevels, int sampleCount,
        sg_pixel_format format);
    void resizeWithScale(const Vector2UI16 &size);
    void setSampleCount(int value);
    void invalidate(Effect *effect);
    void share(const RenderTargetColorImageContainer *value);
    void inherit(const RenderTargetColorImageContainer *shared);
    void setMipmapGenerator(RenderTargetMipmapGenerator *value);
    void setColorImageDescription(const sg_image_desc &value);
    void setColorImageHandle(sg_image value);
    void setScaleFactor(const Vector2 &value);
    void destroy(Effect *effect) NANOEM_DECL_NOEXCEPT;

    RenderTargetMipmapGenerator *mipmapGenerator();
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT;
    const sg_image_desc &colorImageDescription() const NANOEM_DECL_NOEXCEPT;
    const Vector2 scaleFactor() const NANOEM_DECL_NOEXCEPT;
    sg_image colorImageHandle() const NANOEM_DECL_NOEXCEPT;
    bool isSharedTexture() const NANOEM_DECL_NOEXCEPT;

protected:
    void setColorImageDescription(const Vector2SI32 &size, int numMipLevels, int sampleCount, sg_pixel_format format);
    void resizeColorImageDescription(int width, int height);

private:
    const String m_name;
    RenderTargetMipmapGenerator *m_mipmapGenerator;
    Vector2 m_scaleFactor;
    sg_image m_colorImage;
    sg_image_desc m_colorImageDescription;
    bool m_sharedTexture;
    bool m_dirty;
};

} /* namespace effect */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_EFFECT_RENDERTARGETCOLORIMAGECONTAINER_H_ */
