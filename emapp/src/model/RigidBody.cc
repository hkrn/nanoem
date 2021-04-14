/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/model/RigidBody.h"

#include "emapp/Constants.h"
#include "emapp/EnumUtils.h"
#include "emapp/PhysicsEngine.h"
#include "emapp/StringUtils.h"
#include "emapp/model/Bone.h"
#include "emapp/private/CommonInclude.h"

#include "glm/gtc/matrix_inverse.hpp"
#include "par/par_shapes.h"

#include "emapp/DebugUtils.h"

namespace nanoem {
namespace model {
namespace {

enum PrivateStateFlags {
    kPrivateStateAllForcesShouldReset = 1 << 1,
    kPrivateStateEditingMasked = 1 << 2,
};
static const nanoem_u32_t kPrivateStateInitialValue = 0;

} /* namespade anonymous */

RigidBody::~RigidBody() NANOEM_DECL_NOEXCEPT
{
}

void
RigidBody::bind(nanoem_model_rigid_body_t *body, PhysicsEngine *engine, bool isMorph, Resolver &resolver)
{
    nanoem_parameter_assert(body, "must not be nullptr");
    nanoem_parameter_assert(engine, "must not be nullptr");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_user_data_t *userData = nanoemUserDataCreate(&status);
    nanoemUserDataSetOnDestroyModelObjectCallback(userData, &RigidBody::destroy);
    nanoemUserDataSetOpaqueData(userData, this);
    nanoemModelObjectSetUserData(nanoemModelRigidBodyGetModelObjectMutable(body), userData);
    m_physicsRigidBody = engine->createRigidBody(body, status);
    m_physicsEngine = engine;
    const nanoem_model_rigid_body_t *bodyPtr = body;
    resolver.insert(tinystl::make_pair(bodyPtr, m_physicsRigidBody));
    if (isMorph) {
        engine->disableDeactivation(m_physicsRigidBody);
    }
    engine->addRigidBody(m_physicsRigidBody);
}

void
RigidBody::resetLanguage(
    const nanoem_model_rigid_body_t *body, nanoem_unicode_string_factory_t *factory, nanoem_language_type_t language)
{
    StringUtils::getUtf8String(nanoemModelRigidBodyGetName(body, language), factory, m_name);
    if (m_canonicalName.empty()) {
        StringUtils::getUtf8String(
            nanoemModelRigidBodyGetName(body, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), factory, m_canonicalName);
        if (m_canonicalName.empty()) {
            StringUtils::format(
                m_canonicalName, "RigidBody%d", nanoemModelObjectGetIndex(nanoemModelRigidBodyGetModelObject(body)));
        }
    }
    if (m_name.empty()) {
        m_name = m_canonicalName;
    }
}

void
RigidBody::destroy() NANOEM_DECL_NOEXCEPT
{
    if (m_physicsRigidBody) {
        disable();
        m_physicsEngine->destroyRigidBody(m_physicsRigidBody);
        m_physicsRigidBody = nullptr;
    }
    if (m_shape) {
        par_shapes_free_mesh(m_shape);
        m_shape = nullptr;
    }
}

void
RigidBody::getWorldTransform(const nanoem_model_rigid_body_t *body, nanoem_f32_t *value) const NANOEM_DECL_NOEXCEPT
{
    BX_UNUSED_1(body);
    nanoem_parameter_assert(body, "must not be nullptr");
    nanoem_physics_motion_state_t *state = m_physicsEngine->motionState(m_physicsRigidBody);
    m_physicsEngine->getWorldTransform(state, value);
}

void
RigidBody::synchronizeTransformFeedbackFromSimulation(
    const nanoem_model_rigid_body_t *body, PhysicsEngine::RigidBodyFollowBoneType followType) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(body, "must not be nullptr");
    const nanoem_model_rigid_body_transform_type_t transformType = nanoemModelRigidBodyGetTransformType(body);
    if ((transformType == NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_SIMULATION_TO_BONE ||
            transformType == NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_ORIENTATION_AND_SIMULATION_TO_BONE) &&
        !isKinematic()) {
        const nanoem_model_bone_t *bonePtr = nanoemModelRigidBodyGetBoneObject(body);
        if (Bone *bone = Bone::cast(bonePtr)) {
            const nanoem_model_rigid_body_transform_type_t type = nanoemModelRigidBodyGetTransformType(body);
#if 1
            Matrix4x4 worldTransform, initialTransform;
            nanoem_physics_motion_state_t *state = m_physicsEngine->motionState(m_physicsRigidBody);
            m_physicsEngine->getInitialTransform(state, glm::value_ptr(initialTransform));
            m_physicsEngine->getWorldTransform(state, glm::value_ptr(worldTransform));
            if (followType == PhysicsEngine::kRigidBodyFollowBonePerform &&
                type == NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_ORIENTATION_AND_SIMULATION_TO_BONE) {
                const Matrix4x4 localTransform(bone->localTransform());
                worldTransform = glm::translate(Constants::kIdentity, -Vector3(localTransform[3])) * worldTransform;
                m_physicsEngine->setWorldTransform(state, glm::value_ptr(worldTransform));
            }
            const Matrix4x4 skinningTransform(worldTransform * glm::affineInverse(initialTransform));
            bone->updateSkinningTransform(bonePtr, skinningTransform);
            if (const nanoem_model_bone_t *parentBonePtr = nanoemModelBoneGetParentBoneObject(bonePtr)) {
                const model::Bone *parentBone = model::Bone::cast(parentBonePtr);
                const Vector3 offset(Bone::origin(bonePtr) - Bone::origin(parentBonePtr));
                const Matrix4x4 localTransform(
                    glm::affineInverse(parentBone->worldTransform()) * bone->worldTransform());
                bone->setLocalUserTranslation(Vector3(localTransform[3]) - offset);
                bone->setLocalUserOrientation(glm::quat_cast(localTransform));
            }
            else {
                const Matrix4x4 localTransform(bone->worldTransform());
                bone->setLocalUserTranslation(Vector3(localTransform[3]) - Bone::origin(bonePtr));
                bone->setLocalUserOrientation(glm::quat_cast(localTransform));
            }
            m_physicsEngine->setActive(m_physicsRigidBody);
#else /* older behavior compatible (v23.x) */
            if (type == NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_SIMULATION_TO_BONE ||
                type == NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_ORIENTATION_AND_SIMULATION_TO_BONE) {
                Matrix4x4 worldTransform, initialTransform, skinningTransform;
                nanoem_physics_motion_state_t *state = m_worldPtr->motionState(m_physicsRigidBody);
                m_worldPtr->getInitialTransform(state, glm::value_ptr(initialTransform));
                m_worldPtr->getWorldTransform(state, glm::value_ptr(worldTransform));
                if (type == NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_SIMULATION_TO_BONE) {
                    skinningTransform = worldTransform * glm::affineInverse(initialTransform);
                }
                else if (followType == PhysicsEngine::kRigidBodyFollowBonePerform &&
                    type == NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_ORIENTATION_AND_SIMULATION_TO_BONE) {
                    const Vector3 delta(bone->worldTransformOrigin() - Vector3(worldTransform[3]));
                    worldTransform = glm::translate(Constants::kIdentity, delta) * worldTransform;
                    m_worldPtr->setWorldTransform(state, glm::value_ptr(worldTransform));
                    skinningTransform = worldTransform * glm::translate(Constants::kIdentity, -Bone::origin(bonePtr));
                }
                bone->updateSkinningTransform(bonePtr, skinningTransform);
                bone->setLocalUserTranslation(skinningTransform[3]);
                bone->setLocalUserOrientation(glm::quat_cast(skinningTransform));
                m_worldPtr->setActive(m_physicsRigidBody);
            }
#endif
        }
    }
}

void
RigidBody::synchronizeTransformFeedbackToSimulation(const nanoem_model_rigid_body_t *body) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(body, "must not be nullptr");
    const nanoem_model_rigid_body_transform_type_t transformType = nanoemModelRigidBodyGetTransformType(body);
    if (transformType == NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_TO_SIMULATION || isKinematic()) {
        if (const Bone *bone = Bone::cast(nanoemModelRigidBodyGetBoneObject(body))) {
            nanoem_physics_motion_state_t *state = m_physicsEngine->motionState(m_physicsRigidBody);
            bx::float4x4_t initialTransform, worldTransform;
            m_physicsEngine->getInitialTransform(state, reinterpret_cast<nanoem_f32_t *>(&initialTransform));
            const bx::float4x4_t skinningTransformMatrix = bone->skinningTransformMatrix();
            bx::float4x4_mul(&worldTransform, &initialTransform, &skinningTransformMatrix);
            m_physicsEngine->setWorldTransform(state, reinterpret_cast<const nanoem_f32_t *>(&worldTransform));
            m_physicsEngine->resetStates(m_physicsRigidBody);
        }
    }
}

void
RigidBody::applyAllForces(const nanoem_model_rigid_body_t *body) NANOEM_DECL_NOEXCEPT
{
    if (EnumUtils::isEnabled(kPrivateStateAllForcesShouldReset, m_states)) {
        m_physicsEngine->resetStates(m_physicsRigidBody);
    }
    else {
        if (m_globalTorqueForce.second) {
            m_physicsEngine->applyTorqueImpulse(m_physicsRigidBody, glm::value_ptr(m_globalTorqueForce.first));
            m_globalTorqueForce = tinystl::make_pair(Constants::kZeroV3, false);
        }
        if (m_localTorqueForce.second) {
            Matrix3x3 value;
            getLocalOrientationMatrix(body, value);
            m_physicsEngine->applyTorqueImpulse(m_physicsRigidBody, glm::value_ptr(value * m_localTorqueForce.first));
            m_localTorqueForce = tinystl::make_pair(Constants::kZeroV3, false);
        }
        if (m_globalVelocityForce.second) {
            m_physicsEngine->applyVelocityImpulse(m_physicsRigidBody, glm::value_ptr(m_globalVelocityForce.first));
            m_globalVelocityForce = tinystl::make_pair(Constants::kZeroV3, false);
        }
        if (m_localVelocityForce.second) {
            Matrix3x3 value;
            getLocalOrientationMatrix(body, value);
            m_physicsEngine->applyVelocityImpulse(
                m_physicsRigidBody, glm::value_ptr(value * m_localVelocityForce.first));
            m_localVelocityForce = tinystl::make_pair(Constants::kZeroV3, false);
        }
    }
    EnumUtils::setEnabled(kPrivateStateAllForcesShouldReset, m_states, false);
}

void
RigidBody::initializeTransformFeedback(const nanoem_model_rigid_body_t *body)
{
    nanoem_parameter_assert(body, "must not be nullptr");
    if (const Bone *bone = Bone::cast(nanoemModelRigidBodyGetBoneObject(body))) {
        nanoem_physics_motion_state_t *state = m_physicsEngine->motionState(m_physicsRigidBody);
        bx::float4x4_t initialTransform, worldTransform;
        m_physicsEngine->getInitialTransform(state, reinterpret_cast<nanoem_f32_t *>(&initialTransform));
        const bx::float4x4_t skinningTransformMatrix = bone->skinningTransformMatrix();
        bx::float4x4_mul(&worldTransform, &initialTransform, &skinningTransformMatrix);
        m_physicsEngine->setWorldTransform(m_physicsRigidBody, reinterpret_cast<const nanoem_f32_t *>(&worldTransform));
        m_physicsEngine->setWorldTransform(state, reinterpret_cast<const nanoem_f32_t *>(&worldTransform));
        m_physicsEngine->resetStates(m_physicsRigidBody);
    }
}

void
RigidBody::resetTransformFeedback(const nanoem_model_rigid_body_t *body)
{
    if (const model::Bone *bone = model::Bone::cast(nanoemModelRigidBodyGetBoneObject(body))) {
        nanoem_physics_motion_state_t *state = m_physicsEngine->motionState(m_physicsRigidBody);
        bx::float4x4_t initialTransform, worldTransform;
        m_physicsEngine->getInitialTransform(state, reinterpret_cast<nanoem_f32_t *>(&initialTransform));
        const bx::float4x4_t skinningTransformMatrix = bone->skinningTransformMatrix();
        bx::float4x4_mul(&worldTransform, &initialTransform, &skinningTransformMatrix);
        if (nanoemModelRigidBodyGetTransformType(body) ==
            NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_TO_SIMULATION) {
            m_physicsEngine->setWorldTransform(
                m_physicsRigidBody, reinterpret_cast<const nanoem_f32_t *>(&worldTransform));
        }
        else {
            m_physicsEngine->setWorldTransform(state, reinterpret_cast<const nanoem_f32_t *>(&worldTransform));
        }
        m_physicsEngine->resetStates(m_physicsRigidBody);
    }
}

void
RigidBody::addGlobalTorqueForce(const Vector3 &value, nanoem_f32_t weight)
{
    m_globalTorqueForce.first += value * weight;
    m_globalTorqueForce.second = true;
}

void
RigidBody::addGlobalVelocityForce(const Vector3 &value, nanoem_f32_t weight)
{
    m_globalVelocityForce.first += value * weight;
    m_globalVelocityForce.second = true;
}

void
RigidBody::addLocalTorqueForce(const Vector3 &value, nanoem_f32_t weight)
{
    m_localTorqueForce.first += value * weight;
    m_localTorqueForce.second = true;
}

void
RigidBody::addLocalVelocityForce(const Vector3 &value, nanoem_f32_t weight)
{
    m_localVelocityForce.first += value * weight;
    m_localVelocityForce.second = true;
}

void
RigidBody::markAllForcesReset()
{
    EnumUtils::setEnabled(kPrivateStateAllForcesShouldReset, m_states, true);
}

void
RigidBody::enable()
{
    m_physicsEngine->addRigidBody(m_physicsRigidBody);
}

void
RigidBody::disable()
{
    m_physicsEngine->removeRigidBody(m_physicsRigidBody);
}

void
RigidBody::enableKinematic()
{
    if (!isKinematic()) {
        m_physicsEngine->setKinematic(m_physicsRigidBody, true);
    }
}

void
RigidBody::disableKinematic()
{
    if (isKinematic()) {
        m_physicsEngine->setKinematic(m_physicsRigidBody, false);
    }
}

void
RigidBody::forceActive()
{
    m_physicsEngine->setActive(m_physicsRigidBody);
}

int
RigidBody::index(const nanoem_model_rigid_body_t *bodyPtr) NANOEM_DECL_NOEXCEPT
{
    return nanoemModelObjectGetIndex(nanoemModelRigidBodyGetModelObject(bodyPtr));
}

Vector3
RigidBody::colorByShapeType(const nanoem_model_rigid_body_t *bodyPtr) NANOEM_DECL_NOEXCEPT
{
    switch (nanoemModelRigidBodyGetShapeType(bodyPtr)) {
    case NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_BOX: {
        return Vector3(0, 1, 1);
    }
    case NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_CAPSULE: {
        return Vector3(1, 0, 1);
    }
    case NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_SPHERE: {
        return Vector3(1, 1, 0);
    }
    default:
        return Constants::kZeroV3;
    }
}

Vector3
RigidBody::colorByObjectType(const nanoem_model_rigid_body_t *bodyPtr) NANOEM_DECL_NOEXCEPT
{
    switch (nanoemModelRigidBodyGetTransformType(bodyPtr)) {
    case NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_TO_SIMULATION: {
        return Vector3(0, 1, 1);
    }
    case NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_SIMULATION_TO_BONE: {
        return Vector3(1, 1, 0);
    }
    case NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_ORIENTATION_AND_SIMULATION_TO_BONE: {
        return Vector3(1, 0, 1);
    }
    default:
        return Constants::kZeroV3;
    }
}

RigidBody *
RigidBody::cast(const nanoem_model_rigid_body_t *body) NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_object_t *object = nanoemModelRigidBodyGetModelObject(body);
    const nanoem_user_data_t *userData = nanoemModelObjectGetUserData(object);
    return static_cast<RigidBody *>(nanoemUserDataGetOpaqueData(userData));
}

RigidBody *
RigidBody::create()
{
    static const PlaceHolder holder = {};
    return nanoem_new(RigidBody(holder));
}

String
RigidBody::name() const
{
    return m_name;
}

String
RigidBody::canonicalName() const
{
    return m_canonicalName;
}

const char *
RigidBody::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_name.c_str();
}

const char *
RigidBody::canonicalNameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_canonicalName.c_str();
}

PhysicsEngine *
RigidBody::physicsEngine() const NANOEM_DECL_NOEXCEPT
{
    return m_physicsEngine;
}

nanoem_physics_rigid_body_t *
RigidBody::physicsRigidBody() const NANOEM_DECL_NOEXCEPT
{
    return m_physicsRigidBody;
}

Matrix4x4
RigidBody::worldTransform() const NANOEM_DECL_NOEXCEPT
{
    Matrix4x4 value;
    const nanoem_physics_motion_state_t *state = m_physicsEngine->motionState(m_physicsRigidBody);
    m_physicsEngine->getWorldTransform(state, glm::value_ptr(value));
    return value;
}

Matrix4x4
RigidBody::initialTransform() const NANOEM_DECL_NOEXCEPT
{
    Matrix4x4 value;
    const nanoem_physics_motion_state_t *state = m_physicsEngine->motionState(m_physicsRigidBody);
    m_physicsEngine->getInitialTransform(state, glm::value_ptr(value));
    return value;
}

bool
RigidBody::isKinematic() const NANOEM_DECL_NOEXCEPT
{
    return m_physicsEngine->isKinematic(m_physicsRigidBody);
}

bool
RigidBody::isEditingMasked() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateEditingMasked, m_states);
}

void
RigidBody::setEditingMasked(bool value)
{
    EnumUtils::setEnabled(kPrivateStateEditingMasked, m_states, value);
}

const par_shapes_mesh *
RigidBody::sharedShapeMesh(const nanoem_model_rigid_body_t *body)
{
    if (!m_shape) {
        const nanoem_f32_t *size = nanoemModelRigidBodyGetShapeSize(body);
        switch (nanoemModelRigidBodyGetShapeType(body)) {
        case NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_BOX: {
            m_shape = par_shapes_create_cube();
            par_shapes_translate(m_shape, -0.5f, -0.5f, -0.5f);
            const nanoem_f32_t width = size[0] * 2, height = size[1] * 2, depth = size[2] * 2;
            par_shapes_scale(m_shape, width, height, depth);
            break;
        }
        case NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_CAPSULE: {
            m_shape = par_shapes_create_cylinder(16, 1);
            par_shapes_rotate(m_shape, glm::pi<nanoem_f32_t>() / 2.0f, glm::value_ptr(Vector3(1, 0, 0)));
            par_shapes_translate(m_shape, 0.0f, 0.5f, 0.0f);
            par_shapes_scale(m_shape, size[0], size[1], size[0]);
            par_shapes_mesh *s1 = par_shapes_create_hemisphere(16, 8);
            par_shapes_translate(s1, 0.0f, size[1] * 0.5f, 0.0f);
            par_shapes_scale(s1, size[0], 1.0f, size[0]);
            par_shapes_merge_and_free(m_shape, s1);
            par_shapes_mesh *s2 = par_shapes_create_hemisphere(16, 8);
            par_shapes_rotate(s2, -glm::pi<nanoem_f32_t>(), glm::value_ptr(Vector3(1, 0, 0)));
            par_shapes_translate(s2, 0.0f, -size[1] * 0.5f, 0.0f);
            par_shapes_scale(s2, size[0], 1.0f, size[0]);
            par_shapes_merge_and_free(m_shape, s2);
            break;
        }
        case NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_SPHERE: {
            m_shape = par_shapes_create_subdivided_sphere(4);
            const nanoem_f32_t radius = size[0];
            par_shapes_scale(m_shape, radius, radius, radius);
            break;
        }
        default:
            break;
        }
    }
    return m_shape;
}

void
RigidBody::destroy(void *opaque, nanoem_model_object_t * /* object */) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(opaque, "must not be nullptr");
    RigidBody *self = static_cast<RigidBody *>(opaque);
    self->destroy();
    nanoem_delete(self);
}

RigidBody::RigidBody(const PlaceHolder & /* holder */) NANOEM_DECL_NOEXCEPT
    : m_physicsEngine(nullptr),
      m_physicsRigidBody(nullptr),
      m_shape(nullptr),
      m_globalTorqueForce(Constants::kZeroV3, false),
      m_globalVelocityForce(Constants::kZeroV3, false),
      m_localTorqueForce(Constants::kZeroV3, false),
      m_localVelocityForce(Constants::kZeroV3, false),
      m_states(kPrivateStateInitialValue)
{
}

void
RigidBody::getLocalOrientationMatrix(const nanoem_model_rigid_body_t *body, Matrix3x3 &value) const NANOEM_DECL_NOEXCEPT
{
    Matrix4x4 transform;
    m_physicsEngine->getWorldTransform(m_physicsRigidBody, glm::value_ptr(transform));
    if (const model::Bone *bone = model::Bone::cast(nanoemModelRigidBodyGetBoneObject(body))) {
        transform = glm::affineInverse(bone->worldTransform()) * transform;
    }
    value = Matrix3x3(transform);
}

} /* namespace model */
} /* namespace nanoem */
