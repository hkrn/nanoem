/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/model/Vertex.h"

#include "emapp/Constants.h"
#include "emapp/EnumUtils.h"
#include "emapp/Model.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace model {
namespace {

enum PrivateStateFlags {
    kPrivateStateSkinningEnabled = 1 << 1,
    kPrivateStateEditingMasked = 1 << 2,
    kPrivateStateReserved = 1 << 31,
};
static const nanoem_u32_t kPrivateStateInitialValue = 0;

} /* namespace anonymous */

static BX_FORCE_INLINE void
assignBone(Model *model, const nanoem_model_vertex_t *vertexPtr, nanoem_rsize_t i, model::Bone **bones)
{
    model::Bone *&bone = bones[i];
    bone = model::Bone::cast(nanoemModelVertexGetBoneObject(vertexPtr, i));
    if (!bone) {
        bone = model->sharedFallbackBone();
    }
}

Vertex::~Vertex() NANOEM_DECL_NOEXCEPT
{
}

void
Vertex::bind(nanoem_model_vertex_t *vertexPtr)
{
    nanoem_parameter_assert(vertexPtr, "must not be nullptr");
    initialize(vertexPtr);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_user_data_t *userData = nanoemUserDataCreate(&status);
    nanoemUserDataSetOnDestroyModelObjectCallback(userData, &Vertex::destroy);
    nanoemUserDataSetOpaqueData(userData, this);
    nanoemModelObjectSetUserData(nanoemModelVertexGetModelObjectMutable(vertexPtr), userData);
}

void
Vertex::initialize(const nanoem_model_vertex_t *vertexPtr)
{
    const bx::simd128_t direction = bx::simd_ld(glm::value_ptr(Vector4(Constants::kTranslateDirection, 1)));
    m_simd.m_origin = bx::simd_mul(bx::simd_ld(nanoemModelVertexGetOrigin(vertexPtr)), direction);
    m_simd.m_normal = bx::simd_mul(bx::simd_ld(nanoemModelVertexGetNormal(vertexPtr)), direction);
    const nanoem_f32_t *texcoord = nanoemModelVertexGetTexCoord(vertexPtr);
    m_simd.m_texcoord = bx::simd_ld(Inline::fract(texcoord[0]), Inline::fract(texcoord[1]), texcoord[2], texcoord[3]);
    m_simd.m_info = bx::simd_ld(nanoemModelVertexGetEdgeSize(vertexPtr),
        nanoem_f32_t(nanoemModelVertexGetType(vertexPtr)), nanoem_f32_t(index(vertexPtr)), /* needTransform */ 1);
    m_simd.m_indices = bx::simd_ld(nanoem_f32_t(model::Bone::index(nanoemModelVertexGetBoneObject(vertexPtr, 0))),
        nanoem_f32_t(model::Bone::index(nanoemModelVertexGetBoneObject(vertexPtr, 1))),
        nanoem_f32_t(model::Bone::index(nanoemModelVertexGetBoneObject(vertexPtr, 2))),
        nanoem_f32_t(model::Bone::index(nanoemModelVertexGetBoneObject(vertexPtr, 3))));
    m_simd.m_weights =
        bx::simd_ld(nanoemModelVertexGetBoneWeight(vertexPtr, 0), nanoemModelVertexGetBoneWeight(vertexPtr, 1),
            nanoemModelVertexGetBoneWeight(vertexPtr, 2), nanoemModelVertexGetBoneWeight(vertexPtr, 3));
    m_simd.m_originUVA[0] = bx::simd_ld(nanoemModelVertexGetAdditionalUV(vertexPtr, 0));
    m_simd.m_originUVA[1] = bx::simd_ld(nanoemModelVertexGetAdditionalUV(vertexPtr, 1));
    m_simd.m_originUVA[2] = bx::simd_ld(nanoemModelVertexGetAdditionalUV(vertexPtr, 2));
    m_simd.m_originUVA[3] = bx::simd_ld(nanoemModelVertexGetAdditionalUV(vertexPtr, 3));
    m_simd.m_deltaUVA[0] = bx::simd_zero();
    m_simd.m_deltaUVA[1] = bx::simd_zero();
    m_simd.m_deltaUVA[2] = bx::simd_zero();
    m_simd.m_deltaUVA[3] = bx::simd_zero();
    m_simd.m_deltaUVA[4] = bx::simd_zero();
}

void
Vertex::setupBoneBinding(nanoem_model_vertex_t *vertexPtr, Model *model)
{
    m_opaque = vertexPtr;
    switch (nanoemModelVertexGetType(vertexPtr)) {
    case NANOEM_MODEL_VERTEX_TYPE_BDEF1: {
        assignBone(model, vertexPtr, 0, m_bones);
        break;
    }
    case NANOEM_MODEL_VERTEX_TYPE_BDEF2:
    case NANOEM_MODEL_VERTEX_TYPE_SDEF: {
        assignBone(model, vertexPtr, 0, m_bones);
        assignBone(model, vertexPtr, 1, m_bones);
        break;
    }
    case NANOEM_MODEL_VERTEX_TYPE_BDEF4:
    case NANOEM_MODEL_VERTEX_TYPE_QDEF: {
        assignBone(model, vertexPtr, 0, m_bones);
        assignBone(model, vertexPtr, 1, m_bones);
        assignBone(model, vertexPtr, 2, m_bones);
        assignBone(model, vertexPtr, 3, m_bones);
        break;
    }
    default:
        break;
    }
}

int
Vertex::index(const nanoem_model_vertex_t *vertexPtr)
{
    return nanoemModelObjectGetIndex(nanoemModelVertexGetModelObject(vertexPtr));
}

Vertex *
Vertex::cast(const nanoem_model_vertex_t *vertexPtr)
{
    const nanoem_model_object_t *object = nanoemModelVertexGetModelObject(vertexPtr);
    const nanoem_user_data_t *userData = nanoemModelObjectGetUserData(object);
    return static_cast<Vertex *>(nanoemUserDataGetOpaqueData(userData));
}

Vertex *
Vertex::create()
{
    static const PlaceHolder holder = {};
    return nanoem_new(Vertex(holder));
}

const nanoem_model_vertex_t *
Vertex::data() const NANOEM_DECL_NOEXCEPT
{
    return m_opaque;
}

const model::Bone *
Vertex::bone(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(index < BX_COUNTOF(m_bones), "must be less than 4");
    return m_bones[index];
}

model::Bone *
Vertex::bone(nanoem_rsize_t index) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(index < BX_COUNTOF(m_bones), "must be less than 4");
    return m_bones[index];
}

const nanoem_model_material_t *
Vertex::material() const NANOEM_DECL_NOEXCEPT
{
    return m_materialPtr;
}

void
Vertex::setMaterial(const nanoem_model_material_t *value)
{
    m_materialPtr = value;
}

const nanoem_model_soft_body_t *
Vertex::softBody() const NANOEM_DECL_NOEXCEPT
{
    return m_softBodyPtr;
}

void
Vertex::setSoftBody(const nanoem_model_soft_body_t *value)
{
    m_softBodyPtr = value;
}

bool
Vertex::hasSoftBody() const NANOEM_DECL_NOEXCEPT
{
    return m_softBodyPtr != nullptr;
}

bool
Vertex::isSkinningEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateSkinningEnabled, m_states);
}

void
Vertex::setSkinningEnabled(bool value)
{
    EnumUtils::setEnabled(kPrivateStateSkinningEnabled, m_states, value);
}

bool
Vertex::isEditingMasked() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateEditingMasked, m_states);
}

void
Vertex::setEditingMasked(bool value)
{
    EnumUtils::setEnabled(kPrivateStateEditingMasked, m_states, value);
}

void
Vertex::deform(const nanoem_model_morph_vertex_t *morph, nanoem_f32_t weight) NANOEM_DECL_NOEXCEPT
{
    bx::simd128_t a = m_simd.m_delta;
    bx::simd128_t b = bx::simd_ld(nanoemModelMorphVertexGetPosition(morph));
    m_simd.m_delta = bx::simd_add(a, bx::simd_mul(b, bx::simd_splat(weight)));
}

void
Vertex::deform(const nanoem_model_morph_uv_t *morph, int index, nanoem_f32_t weight) NANOEM_DECL_NOEXCEPT
{
    m_simd.m_deltaUVA[index] = bx::simd_add(
        m_simd.m_delta, bx::simd_mul(bx::simd_ld(nanoemModelMorphUVGetPosition(morph)), bx::simd_splat(weight)));
}

void
Vertex::reset() NANOEM_DECL_NOEXCEPT
{
    m_simd.m_delta = bx::simd_zero();
    m_simd.m_deltaUVA[0] = bx::simd_zero();
    m_simd.m_deltaUVA[1] = bx::simd_zero();
    m_simd.m_deltaUVA[2] = bx::simd_zero();
    m_simd.m_deltaUVA[3] = bx::simd_zero();
    m_simd.m_deltaUVA[4] = bx::simd_zero();
}

void
Vertex::destroy(void *opaque, nanoem_model_object_t * /* vertex */) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(opaque, "must not be nullptr");
    Vertex *self = static_cast<Vertex *>(opaque);
    nanoem_delete(self);
}

Vertex::Vertex(const PlaceHolder & /* holder */) NANOEM_DECL_NOEXCEPT : m_materialPtr(nullptr),
                                                                        m_softBodyPtr(nullptr),
                                                                        m_opaque(nullptr),
                                                                        m_states(kPrivateStateInitialValue)
{
    m_bones[0] = m_bones[1] = m_bones[2] = m_bones[3] = nullptr;
}

} /* namespace model */
} /* namespace nanoem */
