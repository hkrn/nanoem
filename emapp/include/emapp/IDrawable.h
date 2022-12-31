/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_IDRAWABLE_H_
#define NANOEM_EMAPP_IDRAWABLE_H_

#include "emapp/URI.h"

namespace nanoem {

class IEffect;
class IImageView;
class Motion;
class Project;

class IDrawable {
public:
    enum DrawType {
        kDrawTypeFirstEnum,
        kDrawTypeColor = kDrawTypeFirstEnum,
        kDrawTypeEdge,
        kDrawTypeGroundShadow,
        kDrawTypeShadowMap,
        kDrawTypeScriptExternalColor,
        kDrawTypeMaxEnum
    };
    typedef tinystl::unordered_map<String, IImageView *, TinySTLAllocator> ImageViewMap;

    virtual ~IDrawable() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual void getAllImageViews(ImageViewMap &value) const = 0;
    virtual IImageView *uploadImage(const String &filename, const sg_image_desc &desc) = 0;
    virtual const IEffect *findOffscreenPassiveRenderTargetEffect(
        const String &ownerName) const NANOEM_DECL_NOEXCEPT = 0;
    virtual IEffect *findOffscreenPassiveRenderTargetEffect(const String &ownerName) NANOEM_DECL_NOEXCEPT = 0;
    virtual void setOffscreenDefaultRenderTargetEffect(const String &ownerName) = 0;
    virtual void setOffscreenPassiveRenderTargetEffect(const String &ownerName, IEffect *value) = 0;
    virtual void removeOffscreenPassiveRenderTargetEffect(const String &ownerName) = 0;
    virtual bool isOffscreenPassiveRenderTargetEffectEnabled(const String &ownerName) const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setOffscreenPassiveRenderTargetEffectEnabled(const String &ownerName, bool value) = 0;
    virtual void draw(DrawType type) = 0;
    virtual void reset() = 0;
    virtual const Project *project() const NANOEM_DECL_NOEXCEPT = 0;
    virtual nanoem_u16_t handle() const NANOEM_DECL_NOEXCEPT = 0;
    virtual String name() const = 0;
    virtual const char *nameConstString() const NANOEM_DECL_NOEXCEPT = 0;
    virtual String canonicalName() const = 0;
    virtual const char *canonicalNameConstString() const NANOEM_DECL_NOEXCEPT = 0;
    virtual URI fileURI() const = 0;
    virtual const IEffect *activeEffect() const NANOEM_DECL_NOEXCEPT = 0;
    virtual IEffect *activeEffect() NANOEM_DECL_NOEXCEPT = 0;
    virtual void setActiveEffect(IEffect *value) = 0;
    virtual const IEffect *passiveEffect() const NANOEM_DECL_NOEXCEPT = 0;
    virtual IEffect *passiveEffect() NANOEM_DECL_NOEXCEPT = 0;
    virtual void setPassiveEffect(IEffect *value) = 0;
    virtual bool isVisible() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setVisible(bool value) = 0;
    virtual bool isAddBlendEnabled() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setAddBlendEnabled(bool value) = 0;
    virtual bool isDirty() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool isTranslucent() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool isUploaded() const NANOEM_DECL_NOEXCEPT = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_IDRAWABLE_H_ */
