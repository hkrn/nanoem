/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODEL_MATERIAL_H_
#define NANOEM_EMAPP_MODEL_MATERIAL_H_

#include "emapp/Forward.h"
#include "emapp/IDrawable.h"

namespace nanoem {

class Effect;
class Image;

namespace model {

class Material NANOEM_DECL_SEALED : private NonCopyable {
public:
    typedef tinystl::vector<const nanoem_model_material_t *, TinySTLAllocator> List;
    typedef tinystl::unordered_set<const nanoem_model_material_t *, TinySTLAllocator> Set;
    typedef tinystl::unordered_map<const nanoem_model_material_t *, UInt32HashMap, TinySTLAllocator> IndexHashMap;
    typedef tinystl::unordered_map<const nanoem_model_material_t *, Int32HashMap, TinySTLAllocator> BoneIndexHashMap;
    typedef tinystl::vector<nanoem_model_material_t *, TinySTLAllocator> MutableList;
    typedef tinystl::unordered_set<nanoem_model_material_t *, TinySTLAllocator> MutableSet;
    struct Color {
        Vector3 m_ambient;
        Vector3 m_diffuse;
        Vector3 m_specular;
        nanoem_f32_t m_diffuseOpacity;
        nanoem_f32_t m_specularPower;
        Vector4 m_diffuseTextureBlendFactor;
        Vector4 m_sphereTextureBlendFactor;
        Vector4 m_toonTextureBlendFactor;
        void reset(nanoem_f32_t v);
    };
    struct Edge {
        Vector3 m_color;
        nanoem_f32_t m_opacity;
        nanoem_f32_t m_size;
        void reset(nanoem_f32_t v);
    };

    ~Material() NANOEM_DECL_NOEXCEPT;

    void bind(nanoem_model_material_t *material);
    void resetLanguage(const nanoem_model_material_t *material, nanoem_unicode_string_factory_t *factory,
        nanoem_language_type_t language);
    void destroy() NANOEM_DECL_NOEXCEPT;
    void reset(const nanoem_model_material_t *material) NANOEM_DECL_NOEXCEPT;
    void update(const nanoem_model_morph_material_t *morph, nanoem_f32_t weight) NANOEM_DECL_NOEXCEPT;
    String name() const;
    String canonicalName() const;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT;
    const char *canonicalNameConstString() const NANOEM_DECL_NOEXCEPT;
    Color base() const NANOEM_DECL_NOEXCEPT;
    Color mul() const NANOEM_DECL_NOEXCEPT;
    Color add() const NANOEM_DECL_NOEXCEPT;
    Color color() const NANOEM_DECL_NOEXCEPT;
    Edge edge() const NANOEM_DECL_NOEXCEPT;

    static int index(const nanoem_model_material_t *materialPtr) NANOEM_DECL_NOEXCEPT;
    static Material *cast(const nanoem_model_material_t *materialPtr) NANOEM_DECL_NOEXCEPT;
    static Material *create(sg_image fallbackTexture);

    const IImageView *diffuseImage() const NANOEM_DECL_NOEXCEPT;
    void setDiffuseImage(const IImageView *value);
    const IImageView *sphereMapImage() const NANOEM_DECL_NOEXCEPT;
    void setSphereMapImage(const IImageView *value);
    const IImageView *toonImage() const NANOEM_DECL_NOEXCEPT;
    void setToonImage(const IImageView *value);
    const Effect *effect() const NANOEM_DECL_NOEXCEPT;
    Effect *effect();
    void setEffect(Effect *value);
    UInt32HashMap indexHash() const;
    void setIndexHash(const UInt32HashMap &value);
    Vector4 toonColor() const NANOEM_DECL_NOEXCEPT;
    void setToonColor(const Vector4 &value);
    bool isDisplayDiffuseTextureUVMeshEnabled() const NANOEM_DECL_NOEXCEPT;
    void setDisplayDiffuseTextureUVMeshEnabled(bool value);
    bool isDisplaySphereMapTextureUVMeshEnabled() const NANOEM_DECL_NOEXCEPT;
    void setDisplaySphereMapTextureUVMeshEnabled(bool value);
    bool isVisible() const NANOEM_DECL_NOEXCEPT;
    void setVisible(bool value);

private:
    struct {
        Color base;
        Color add;
        Color mul;
    } m_color;
    struct {
        Edge base;
        Edge add;
        Edge mul;
    } m_edge;
    static void destroy(void *opaque, nanoem_model_object_t *object) NANOEM_DECL_NOEXCEPT;
    Material(sg_image fallbackImage) NANOEM_DECL_NOEXCEPT;

    Effect *m_effect;
    const IImageView *m_diffuseImagePtr;
    const IImageView *m_sphereMapImagePtr;
    const IImageView *m_toonImagePtr;
    String m_name;
    String m_canonicalName;
    sg_image m_fallbackImage;
    UInt32HashMap m_indexHash;
    Vector4 m_toonColor;
    uint32_t m_states;
};

} /* namespace model */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODEL_MATERIAL_H_ */
