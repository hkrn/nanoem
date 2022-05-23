/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODEL_SOFTBODY_H_
#define NANOEM_EMAPP_MODEL_SOFTBODY_H_

#include "emapp/Model.h"
#include "emapp/model/RigidBody.h"

namespace nanoem {

class PhysicsEngine;

namespace model {

class Bone;

class SoftBody NANOEM_DECL_SEALED : private NonCopyable {
public:
    typedef tinystl::vector<const nanoem_model_soft_body_t *, TinySTLAllocator> List;
    typedef tinystl::unordered_set<const nanoem_model_soft_body_t *, TinySTLAllocator> Set;
    typedef tinystl::unordered_set<nanoem_model_soft_body_t *, TinySTLAllocator> MutableSet;

    static int index(const nanoem_model_soft_body_t *softBodyPtr) NANOEM_DECL_NOEXCEPT;
    static const char *nameConstString(
        const nanoem_model_soft_body_t *softBodyPtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT;
    static const char *canonicalNameConstString(
        const nanoem_model_soft_body_t *softBodyPtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT;
    static SoftBody *cast(const nanoem_model_soft_body_t *softBodyPtr);
    static SoftBody *create();

    ~SoftBody() NANOEM_DECL_NOEXCEPT;

    void bind(nanoem_model_soft_body_t *softBodyPtr, PhysicsEngine *world, RigidBody::Resolver &resolver);
    void resetLanguage(const nanoem_model_soft_body_t *body, nanoem_unicode_string_factory_t *factory,
        nanoem_language_type_t language);
    void destroy() NANOEM_DECL_NOEXCEPT;

    void initializeTransformFeedback();
    void synchronizeTransformFeedbackFromSimulation(
        Model::VertexUnit *vertexUnits, nanoem_rsize_t numVertices) NANOEM_DECL_NOEXCEPT;
    void synchronizeTransformFeedbackToSimulation(
        const nanoem_model_soft_body_t *softBodyPtr, const Model::VertexUnit *vertexUnits, nanoem_rsize_t numVertices);
    void getVertexPosition(const nanoem_model_vertex_t *vertex, bx::simd128_t *value) const NANOEM_DECL_NOEXCEPT;
    void getVertexNormal(const nanoem_model_vertex_t *vertex, bx::simd128_t *value) const NANOEM_DECL_NOEXCEPT;
    void enable();
    void disable();

    String name() const;
    String canonicalName() const;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT;
    const char *canonicalNameConstString() const NANOEM_DECL_NOEXCEPT;
    PhysicsEngine *physicsEngine() const NANOEM_DECL_NOEXCEPT;
    nanoem_physics_soft_body_t *physicsSoftBody() const NANOEM_DECL_NOEXCEPT;
    bool isEditingMasked() const NANOEM_DECL_NOEXCEPT;
    void setEditingMasked(bool value);

private:
    struct PlaceHolder { };

    static void destroy(void *opaque, nanoem_model_object_t *object) NANOEM_DECL_NOEXCEPT;
    SoftBody(const PlaceHolder &holder) NANOEM_DECL_NOEXCEPT;

    int vertexIndexOf(const nanoem_model_vertex_t *value) const NANOEM_DECL_NOEXCEPT;
    void getVertexPosition(int index, bx::simd128_t *value) const NANOEM_DECL_NOEXCEPT;
    void getVertexNormal(int index, bx::simd128_t *value) const NANOEM_DECL_NOEXCEPT;
    void setVertexPosition(int index, const bx::simd128_t *value);
    void setVertexNormal(int index, const bx::simd128_t *value);

    PhysicsEngine *m_physicsEngine;
    nanoem_physics_soft_body_t *m_physicsSoftBody;
    String m_name;
    String m_canonicalName;
    nanoem_u32_t m_states;
};

} /* namespace model */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODEL_SOFTBODY_H_ */
