/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/model/SoftBody.h"

#include "emapp/EnumUtils.h"
#include "emapp/Model.h"
#include "emapp/PhysicsEngine.h"
#include "emapp/StringUtils.h"
#include "emapp/model/Vertex.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace model {
namespace {

enum PrivateStateFlags {
    kPrivateStateEnabled = 1 << 1,
    kPrivateStateEditingMasked = 1 << 2,
};
static const nanoem_u32_t kPrivateStateInitialValue = 0;

} /* namespace anonymous */

int
SoftBody::index(const nanoem_model_soft_body_t *softBodyPtr) NANOEM_DECL_NOEXCEPT
{
    return nanoemModelObjectGetIndex(nanoemModelSoftBodyGetModelObject(softBodyPtr));
}

const char *
SoftBody::nameConstString(
    const nanoem_model_soft_body_t *softBodyPtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT
{
    const SoftBody *body = cast(softBodyPtr);
    return body ? body->nameConstString() : placeHolder;
}

const char *
SoftBody::canonicalNameConstString(
    const nanoem_model_soft_body_t *softBodyPtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT
{
    const SoftBody *body = cast(softBodyPtr);
    return body ? body->canonicalNameConstString() : placeHolder;
}

SoftBody *
SoftBody::cast(const nanoem_model_soft_body_t *softBodyPtr)
{
    const nanoem_model_object_t *object = nanoemModelSoftBodyGetModelObject(softBodyPtr);
    const nanoem_user_data_t *userData = nanoemModelObjectGetUserData(object);
    return static_cast<SoftBody *>(nanoemUserDataGetOpaqueData(userData));
}

SoftBody *
SoftBody::create()
{
    static const PlaceHolder holder = {};
    return nanoem_new(SoftBody(holder));
}

SoftBody::~SoftBody() NANOEM_DECL_NOEXCEPT
{
}

void
SoftBody::bind(nanoem_model_soft_body_t *softBodyPtr, PhysicsEngine *engine, RigidBody::Resolver & /* resolver */)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_user_data_t *userData = nanoemUserDataCreate(&status);
    nanoemUserDataSetOnDestroyModelObjectCallback(userData, &SoftBody::destroy);
    nanoemUserDataSetOpaqueData(userData, this);
    nanoemModelObjectSetUserData(nanoemModelSoftBodyGetModelObjectMutable(softBodyPtr), userData);
    m_physicsSoftBody = engine->createSoftBody(softBodyPtr, status);
    m_physicsEngine = engine;
    enable();
    int numSoftBodyVertices = engine->numSoftBodyVertices(m_physicsSoftBody);
    for (int i = 0; i < numSoftBodyVertices; i++) {
        const nanoem_model_vertex_t *vertexPtr = engine->resolveSoftBodyVertexObject(m_physicsSoftBody, i);
        if (model::Vertex *vertex = model::Vertex::cast(vertexPtr)) {
            vertex->setSoftBody(softBodyPtr);
        }
    }
    nanoem_rsize_t numIndices;
    const nanoem_u32_t *indices = nanoemModelSoftBodyGetAllPinnedVertexIndices(softBodyPtr, &numIndices);
    for (nanoem_rsize_t i = 0; i < numIndices; i++) {
        const nanoem_u32_t index = indices[i];
        const nanoem_model_vertex_t *vertexPtr = engine->resolveSoftBodyVertexObject(m_physicsSoftBody, index);
        if (model::Vertex *vertex = model::Vertex::cast(vertexPtr)) {
            vertex->setSoftBody(nullptr);
        }
    }
}

void
SoftBody::resetLanguage(
    const nanoem_model_soft_body_t *body, nanoem_unicode_string_factory_t *factory, nanoem_language_type_t language)
{
    StringUtils::getUtf8String(nanoemModelSoftBodyGetName(body, language), factory, m_name);
    if (m_canonicalName.empty()) {
        StringUtils::getUtf8String(
            nanoemModelSoftBodyGetName(body, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), factory, m_canonicalName);
        if (m_canonicalName.empty()) {
            StringUtils::format(m_canonicalName, "SoftBody%d", index(body));
        }
    }
    if (m_name.empty()) {
        m_name = m_canonicalName;
    }
}

void
SoftBody::destroy() NANOEM_DECL_NOEXCEPT
{
    if (m_physicsSoftBody) {
        disable();
        m_physicsEngine->destroySoftBody(m_physicsSoftBody);
        m_physicsSoftBody = nullptr;
    }
}

void
SoftBody::initializeTransformFeedback()
{
    int numSoftBodyVertices = m_physicsEngine->numSoftBodyVertices(m_physicsSoftBody);
    for (int i = 0; i < numSoftBodyVertices; i++) {
        const nanoem_model_vertex_t *vertexPtr = m_physicsEngine->resolveSoftBodyVertexObject(m_physicsSoftBody, i);
        const model::Vertex *vertex = model::Vertex::cast(vertexPtr);
        bx::simd128_t p, n;
        Model::VertexUnit::performSkinningByType(vertex, &p, &n);
        setVertexPosition(i, &p);
        setVertexNormal(i, &n);
    }
}

void
SoftBody::synchronizeTransformFeedbackFromSimulation(
    Model::VertexUnit *vertexUnits, nanoem_rsize_t numVertices) NANOEM_DECL_NOEXCEPT
{
    int numSoftBodyVertices = m_physicsEngine->numSoftBodyVertices(m_physicsSoftBody);
    for (int i = 0; i < numSoftBodyVertices; i++) {
        const nanoem_model_vertex_t *vertexPtr = m_physicsEngine->resolveSoftBodyVertexObject(m_physicsSoftBody, i);
        nanoem_rsize_t vertexIndex = static_cast<nanoem_rsize_t>(model::Vertex::index(vertexPtr));
        if (nanoem_likely(vertexIndex < numVertices)) {
            Model::VertexUnit &vertexUnit = vertexUnits[vertexIndex];
            getVertexPosition(i, &vertexUnit.m_position);
            getVertexNormal(i, &vertexUnit.m_normal);
        }
    }
}

void
SoftBody::synchronizeTransformFeedbackToSimulation(
    const nanoem_model_soft_body_t *softBodyPtr, const Model::VertexUnit *vertexUnits, nanoem_rsize_t numVertices)
{
    nanoem_rsize_t numIndices;
    const nanoem_u32_t *indices = nanoemModelSoftBodyGetAllPinnedVertexIndices(softBodyPtr, &numIndices);
    for (nanoem_rsize_t i = 0; i < numIndices; i++) {
        const nanoem_u32_t index = indices[i];
        const nanoem_model_vertex_t *vertexPtr = m_physicsEngine->resolveSoftBodyVertexObject(m_physicsSoftBody, index);
        nanoem_rsize_t vertexIndex = static_cast<nanoem_rsize_t>(model::Vertex::index(vertexPtr));
        if (nanoem_likely(vertexIndex < numVertices)) {
            const Model::VertexUnit &vertexUnit = vertexUnits[vertexIndex];
            setVertexPosition(index, &vertexUnit.m_position);
            setVertexNormal(index, &vertexUnit.m_normal);
        }
    }
}

void
SoftBody::getVertexPosition(const nanoem_model_vertex_t *vertexPtr, bx::simd128_t *value) const NANOEM_DECL_NOEXCEPT
{
    getVertexPosition(vertexIndexOf(vertexPtr), value);
}

void
SoftBody::getVertexNormal(const nanoem_model_vertex_t *vertexPtr, bx::simd128_t *value) const NANOEM_DECL_NOEXCEPT
{
    getVertexNormal(vertexIndexOf(vertexPtr), value);
}

void
SoftBody::enable()
{
    if (!EnumUtils::isEnabled(kPrivateStateEnabled, m_states)) {
        m_physicsEngine->addSoftBody(m_physicsSoftBody);
        EnumUtils::setEnabled(kPrivateStateEnabled, m_states, true);
    }
}

void
SoftBody::disable()
{
    if (EnumUtils::isEnabled(kPrivateStateEnabled, m_states)) {
        m_physicsEngine->removeSoftBody(m_physicsSoftBody);
        EnumUtils::setEnabled(kPrivateStateEnabled, m_states, false);
    }
}

String
SoftBody::name() const
{
    return m_name;
}

String
SoftBody::canonicalName() const
{
    return m_canonicalName;
}

const char *
SoftBody::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_name.c_str();
}

const char *
SoftBody::canonicalNameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_canonicalName.c_str();
}

PhysicsEngine *
SoftBody::physicsEngine() const NANOEM_DECL_NOEXCEPT
{
    return m_physicsEngine;
}

nanoem_physics_soft_body_t *
SoftBody::physicsSoftBody() const NANOEM_DECL_NOEXCEPT
{
    return m_physicsSoftBody;
}

bool
SoftBody::isEditingMasked() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateEditingMasked, m_states);
}

void
SoftBody::setEditingMasked(bool value)
{
    EnumUtils::setEnabled(kPrivateStateEditingMasked, m_states, value);
}

void
SoftBody::destroy(void *opaque, nanoem_model_object_t * /* object */) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(opaque, "must not be nullptr");
    SoftBody *self = static_cast<SoftBody *>(opaque);
    self->destroy();
    nanoem_delete(self);
}

SoftBody::SoftBody(const PlaceHolder & /* holder */) NANOEM_DECL_NOEXCEPT : m_physicsEngine(nullptr),
                                                                            m_physicsSoftBody(nullptr),
                                                                            m_states(kPrivateStateInitialValue)
{
}

void
SoftBody::getVertexPosition(int index, bx::simd128_t *value) const NANOEM_DECL_NOEXCEPT
{
    m_physicsEngine->getSoftBodyVertexPosition(m_physicsSoftBody, index, reinterpret_cast<nanoem_f32_t *>(value));
}

void
SoftBody::getVertexNormal(int index, bx::simd128_t *value) const NANOEM_DECL_NOEXCEPT
{
    m_physicsEngine->getSoftBodyVertexNormal(m_physicsSoftBody, index, reinterpret_cast<nanoem_f32_t *>(value));
}

void
SoftBody::setVertexPosition(int index, const bx::simd128_t *value)
{
    m_physicsEngine->setSoftBodyVertexPosition(m_physicsSoftBody, index, reinterpret_cast<const nanoem_f32_t *>(value));
}

void
SoftBody::setVertexNormal(int index, const bx::simd128_t *value)
{
    m_physicsEngine->setSoftBodyVertexNormal(m_physicsSoftBody, index, reinterpret_cast<const nanoem_f32_t *>(value));
}

int
SoftBody::vertexIndexOf(const nanoem_model_vertex_t *value) const NANOEM_DECL_NOEXCEPT
{
    int numSoftBodyVertices = m_physicsEngine->numSoftBodyVertices(m_physicsSoftBody);
    for (int i = 0; i < numSoftBodyVertices; i++) {
        const nanoem_model_vertex_t *vertexPtr = m_physicsEngine->resolveSoftBodyVertexObject(m_physicsSoftBody, i);
        if (value == vertexPtr) {
            return i;
        }
    }
    return -1;
}

} /* namespace model */
} /* namespace nanoem */
