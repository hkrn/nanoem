/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/model/Joint.h"

#include "emapp/Constants.h"
#include "emapp/PhysicsEngine.h"
#include "emapp/StringUtils.h"
#include "emapp/model/Bone.h"
#include "emapp/private/CommonInclude.h"

#include "par/par_shapes.h"

namespace nanoem {
namespace model {

Joint *
Joint::cast(const nanoem_model_joint_t *joint) NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_object_t *object = nanoemModelJointGetModelObject(joint);
    const nanoem_user_data_t *userData = nanoemModelObjectGetUserData(object);
    return static_cast<Joint *>(nanoemUserDataGetOpaqueData(userData));
}

Joint *
Joint::create()
{
    static const PlaceHolder holder = {};
    return nanoem_new(Joint(holder));
}

Joint::~Joint() NANOEM_DECL_NOEXCEPT
{
}

void
Joint::bind(nanoem_model_joint_t *joint, PhysicsEngine *engine, const RigidBody::Resolver &resolver)
{
    nanoem_parameter_assert(joint, "must not be nullptr");
    nanoem_parameter_assert(engine, "must not be nullptr");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_user_data_t *userData = nanoemUserDataCreate(&status);
    nanoemUserDataSetOnDestroyModelObjectCallback(userData, &Joint::destroy);
    nanoemUserDataSetOpaqueData(userData, this);
    nanoemModelObjectSetUserData(nanoemModelJointGetModelObjectMutable(joint), userData);
    struct Opaque {
        nanoem_physics_world_t *world;
        nanoem_physics_rigid_body_t *physics_body_a;
        nanoem_physics_rigid_body_t *physics_body_b;
        nanoem_f32_t transform_a[16];
        nanoem_f32_t transform_b[16];
    } opaque;
    Inline::clearZeroMemory(opaque);
    const nanoem_model_rigid_body_t *bodyA = nanoemModelJointGetRigidBodyAObject(joint);
    RigidBody::Resolver::const_iterator it = resolver.find(bodyA);
    if (it != resolver.end()) {
        opaque.physics_body_a = it->second;
        if (const Bone *bone = Bone::cast(nanoemModelRigidBodyGetBoneObject(bodyA))) {
            const bx::float4x4_t m = bone->skinningTransformMatrix();
            memcpy(opaque.transform_a, &m, sizeof(opaque.transform_a));
        }
        else {
            memcpy(opaque.transform_a, glm::value_ptr(Constants::kIdentity), sizeof(opaque.transform_a));
        }
    }
    const nanoem_model_rigid_body_t *bodyB = nanoemModelJointGetRigidBodyBObject(joint);
    RigidBody::Resolver::const_iterator it2 = resolver.find(bodyB);
    if (it2 != resolver.end()) {
        opaque.physics_body_b = it2->second;
        if (const Bone *bone = Bone::cast(nanoemModelRigidBodyGetBoneObject(bodyB))) {
            const bx::float4x4_t m = bone->skinningTransformMatrix();
            memcpy(opaque.transform_b, &m, sizeof(opaque.transform_b));
        }
        else {
            memcpy(opaque.transform_b, glm::value_ptr(Constants::kIdentity), sizeof(opaque.transform_b));
        }
    }
    opaque.world = engine->worldOpaque();
    m_physicsJoint = engine->createJoint(joint, &opaque, status);
    m_physicsEngine = engine;
    engine->addJoint(m_physicsJoint);
}

void
Joint::resetLanguage(
    const nanoem_model_joint_t *joint, nanoem_unicode_string_factory_t *factory, nanoem_language_type_t language)
{
    StringUtils::getUtf8String(nanoemModelJointGetName(joint, language), factory, m_name);
    if (m_canonicalName.empty()) {
        StringUtils::getUtf8String(
            nanoemModelJointGetName(joint, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), factory, m_canonicalName);
        if (m_canonicalName.empty()) {
            StringUtils::format(
                m_canonicalName, "Joint%d", nanoemModelObjectGetIndex(nanoemModelJointGetModelObject(joint)));
        }
    }
    if (m_name.empty()) {
        m_name = m_canonicalName;
    }
}

void
Joint::destroy() NANOEM_DECL_NOEXCEPT
{
    if (m_physicsJoint) {
        disable();
        m_physicsEngine->destroyJoint(m_physicsJoint);
        m_physicsJoint = nullptr;
    }
    if (m_shape) {
        par_shapes_free_mesh(m_shape);
        m_shape = nullptr;
    }
}

String
Joint::name() const
{
    return m_name;
}

String
Joint::canonicalName() const
{
    return m_canonicalName;
}

const char *
Joint::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_name.c_str();
}

const char *
Joint::canonicalNameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_canonicalName.c_str();
}

PhysicsEngine *
Joint::physicsEngine() const NANOEM_DECL_NOEXCEPT
{
    return m_physicsEngine;
}

nanoem_physics_joint_t *
Joint::physicsJoint() const NANOEM_DECL_NOEXCEPT
{
    return m_physicsJoint;
}

void
Joint::getWorldTransformA(nanoem_f32_t *value) const NANOEM_DECL_NOEXCEPT
{
    m_physicsEngine->getCalculatedTransformA(m_physicsJoint, value);
}

void
Joint::getWorldTransformB(nanoem_f32_t *value) const NANOEM_DECL_NOEXCEPT
{
    m_physicsEngine->getCalculatedTransformB(m_physicsJoint, value);
}

const par_shapes_mesh_s *
Joint::generateShapeMesh(const nanoem_model_joint_t * /* joint */)
{
    if (!m_shape) {
        m_shape = par_shapes_create_cube();
        par_shapes_translate(m_shape, -0.5f, -0.5f, -0.5f);
        par_shapes_scale(m_shape, 0.25f, 0.25f, 0.25f);
    }
    return m_shape;
}

void
Joint::enable()
{
    m_physicsEngine->addJoint(m_physicsJoint);
}

void
Joint::disable()
{
    m_physicsEngine->removeJoint(m_physicsJoint);
}

void
Joint::destroy(void *opaque, nanoem_model_object_t * /* joint */) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(opaque, "must not be nullptr");
    Joint *self = static_cast<Joint *>(opaque);
    self->destroy();
    nanoem_delete(self);
}

Joint::Joint(const PlaceHolder & /* holder */) NANOEM_DECL_NOEXCEPT : m_physicsEngine(nullptr),
                                                                      m_physicsJoint(nullptr),
                                                                      m_shape(nullptr)
{
}

} /* namespace model */
} /* namespace nanoem */
