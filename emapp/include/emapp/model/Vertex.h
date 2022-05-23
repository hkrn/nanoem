/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODEL_VERTEX_H_
#define NANOEM_EMAPP_MODEL_VERTEX_H_

#include "emapp/Forward.h"

#include "bx/float4x4_t.h"

namespace nanoem {

class Model;

namespace model {

class Bone;

class Vertex NANOEM_DECL_SEALED : private NonCopyable {
public:
    typedef tinystl::vector<const nanoem_model_vertex_t *, TinySTLAllocator> List;
    typedef tinystl::unordered_set<const nanoem_model_vertex_t *, TinySTLAllocator> Set;
    typedef tinystl::unordered_set<nanoem_model_vertex_t *, TinySTLAllocator> MutableSet;

    ~Vertex() NANOEM_DECL_NOEXCEPT;

    void bind(nanoem_model_vertex_t *vertexPtr);
    void initialize(const nanoem_model_vertex_t *vertexPtr);
    void setupBoneBinding(nanoem_model_vertex_t *vertexPtr, Model *model);

    static int index(const nanoem_model_vertex_t *vertexPtr);
    static Vertex *cast(const nanoem_model_vertex_t *vertexPtr);
    static Vertex *create();

    const nanoem_model_vertex_t *data() const NANOEM_DECL_NOEXCEPT;
    const model::Bone *bone(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT;
    model::Bone *bone(nanoem_rsize_t index) NANOEM_DECL_NOEXCEPT;
    const nanoem_model_material_t *material() const NANOEM_DECL_NOEXCEPT;
    void setMaterial(const nanoem_model_material_t *value);
    const nanoem_model_soft_body_t *softBody() const NANOEM_DECL_NOEXCEPT;
    void setSoftBody(const nanoem_model_soft_body_t *value);
    bool hasSoftBody() const NANOEM_DECL_NOEXCEPT;
    bool isSkinningEnabled() const NANOEM_DECL_NOEXCEPT;
    void setSkinningEnabled(bool value);
    bool isEditingMasked() const NANOEM_DECL_NOEXCEPT;
    void setEditingMasked(bool value);

    void deform(const nanoem_model_morph_vertex_t *morph, nanoem_f32_t weight) NANOEM_DECL_NOEXCEPT;
    void deform(const nanoem_model_morph_uv_t *morph, int index, nanoem_f32_t weight) NANOEM_DECL_NOEXCEPT;
    void reset() NANOEM_DECL_NOEXCEPT;

    BX_ALIGN_DECL_16(struct) SIMD
    {
        bx::simd128_t m_origin;
        bx::simd128_t m_normal;
        bx::simd128_t m_texcoord;
        bx::simd128_t m_info;
        bx::simd128_t m_indices;
        bx::simd128_t m_delta;
        bx::simd128_t m_weights;
        bx::simd128_t m_originUVA[4];
        bx::simd128_t m_deltaUVA[5];
    }
    m_simd;

private:
    struct PlaceHolder { };

    static void destroy(void *opaque, nanoem_model_object_t *object) NANOEM_DECL_NOEXCEPT;
    Vertex(const PlaceHolder &holder) NANOEM_DECL_NOEXCEPT;

    const nanoem_model_material_t *m_materialPtr;
    const nanoem_model_soft_body_t *m_softBodyPtr;
    model::Bone *m_bones[4];
    nanoem_model_vertex_t *m_opaque;
    nanoem_u32_t m_states;
};

} /* namespace model */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODEL_VERTEX_H_ */
