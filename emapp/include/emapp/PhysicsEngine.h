/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_PHYSICS_H_
#define NANOEM_EMAPP_PHYSICS_H_

#include "emapp/Forward.h"

struct nanoem_physics_world_t;
struct nanoem_physics_debug_geometry_t;
struct nanoem_physics_rigid_body_t;
struct nanoem_physics_joint_t;
struct nanoem_physics_motion_state_t;
struct nanoem_physics_soft_body_t;
struct nanoem_model_rigid_body_t;
struct nanoem_model_soft_body_t;
struct nanoem_model_joint_t;

namespace nanoem {

class Error;

class PhysicsEngine NANOEM_DECL_SEALED : private NonCopyable {
public:
    enum RigidBodyFollowBoneType {
        kRigidBodyFollowBoneFirstEnum,
        kRigidBodyFollowBoneSkip = kRigidBodyFollowBoneFirstEnum,
        kRigidBodyFollowBonePerform,
        kRigidBodyFollowBoneMaxEnum,
    };
    enum SimulationTimingType {
        kSimulationTimingFirstEnum,
        kSimulationTimingBefore = kSimulationTimingFirstEnum,
        kSimulationTimingAfter,
        kSimulationTimingMaxEnum,
    };
    enum SimulationModeType {
        kSimulationModeFirstEnum,
        kSimulationModeDisable = kSimulationModeFirstEnum,
        kSimulationModeEnableAnytime,
        kSimulationModeEnablePlaying,
        kSimulationModeEnableTracing,
        kSimulationModeMaxEnum
    };
    enum DebugDrawType {
        kDebugDrawWireframe = 1 << 0,
        kDebugDrawAabb = 1 << 1,
        kDebugDrawContactPoints = 1 << 3,
        kDebugDrawConstraints = 1 << 11,
        kDebugDrawConstraintLimits = 1 << 12
    };

    PhysicsEngine();
    ~PhysicsEngine() NANOEM_DECL_NOEXCEPT;

    bool isAvailable() const NANOEM_DECL_NOEXCEPT;
    bool initialize(const char *dllPath);
    void create(nanoem_status_t &status);
    void destroy() NANOEM_DECL_NOEXCEPT;
    void reset() NANOEM_DECL_NOEXCEPT;
    void stepSimulation(nanoem_f32_t delta);
    SimulationModeType mode() const NANOEM_DECL_NOEXCEPT;
    void setMode(SimulationModeType value);

    const nanoem_physics_world_t *worldOpaque() const NANOEM_DECL_NOEXCEPT;
    nanoem_physics_world_t *worldOpaque();

    nanoem_physics_rigid_body_t *createRigidBody(const nanoem_model_rigid_body_t *value, nanoem_status_t &status);
    nanoem_physics_motion_state_t *motionState(const nanoem_physics_rigid_body_t *value);
    void getWorldTransform(const nanoem_physics_rigid_body_t *body, nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT;
    void setWorldTransform(nanoem_physics_rigid_body_t *body, const nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT;
    void addRigidBody(nanoem_physics_rigid_body_t *value);
    void removeRigidBody(nanoem_physics_rigid_body_t *value);
    void destroyRigidBody(nanoem_physics_rigid_body_t *value);
    void disableDeactivation(nanoem_physics_rigid_body_t *value);
    void setVisualizedEnabled(nanoem_physics_rigid_body_t *body, bool value) NANOEM_DECL_NOEXCEPT;
    void setVisualizedEnabled(nanoem_physics_joint_t *joint, bool value) NANOEM_DECL_NOEXCEPT;
    void setVisualizedEnabled(nanoem_physics_soft_body_t *body, bool value) NANOEM_DECL_NOEXCEPT;
    void setActive(nanoem_physics_rigid_body_t *body) NANOEM_DECL_NOEXCEPT;
    void applyTorqueImpulse(nanoem_physics_rigid_body_t *body, const nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT;
    void applyVelocityImpulse(nanoem_physics_rigid_body_t *body, const nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT;
    void resetLinearVelocity(nanoem_physics_rigid_body_t *body) NANOEM_DECL_NOEXCEPT;
    void resetStates(nanoem_physics_rigid_body_t *body) NANOEM_DECL_NOEXCEPT;
    void setKinematic(nanoem_physics_rigid_body_t *body, bool value) NANOEM_DECL_NOEXCEPT;
    bool isKinematic(nanoem_physics_rigid_body_t *body) const NANOEM_DECL_NOEXCEPT;

    void getInitialTransform(
        const nanoem_physics_motion_state_t *state, nanoem_f32_t *value) const NANOEM_DECL_NOEXCEPT;
    void getWorldTransform(const nanoem_physics_motion_state_t *state, nanoem_f32_t *value) const NANOEM_DECL_NOEXCEPT;
    void setWorldTransform(nanoem_physics_motion_state_t *state, const nanoem_f32_t *value);
    void getCenterOfMassOffset(const nanoem_physics_motion_state_t *state, nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT;
    void setCenterOfMassOffset(nanoem_physics_motion_state_t *state, const nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT;

    nanoem_physics_joint_t *createJoint(const nanoem_model_joint_t *value, void *worldOpaque, nanoem_status_t &status);
    void addJoint(nanoem_physics_joint_t *value);
    void getCalculatedTransformA(nanoem_physics_joint_t *joint, nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT;
    void getCalculatedTransformB(nanoem_physics_joint_t *joint, nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT;
    void removeJoint(nanoem_physics_joint_t *value) NANOEM_DECL_NOEXCEPT;
    void destroyJoint(nanoem_physics_joint_t *value) NANOEM_DECL_NOEXCEPT;

    nanoem_physics_debug_geometry_t *const *debugGeometryObjects(int *numObjects) const NANOEM_DECL_NOEXCEPT;
    const nanoem_f32_t *geometryColor(const nanoem_physics_debug_geometry_t *value) const NANOEM_DECL_NOEXCEPT;
    const nanoem_f32_t *geometryFromPosition(const nanoem_physics_debug_geometry_t *value) const NANOEM_DECL_NOEXCEPT;
    const nanoem_f32_t *geometryToPosition(const nanoem_physics_debug_geometry_t *value) const NANOEM_DECL_NOEXCEPT;

    nanoem_physics_soft_body_t *createSoftBody(const nanoem_model_soft_body_t *body, nanoem_status_t &status);
    void addSoftBody(nanoem_physics_soft_body_t *body);
    void removeSoftBody(nanoem_physics_soft_body_t *body);
    void destroySoftBody(nanoem_physics_soft_body_t *body);
    int numSoftBodyVertices(nanoem_physics_soft_body_t *body) const NANOEM_DECL_NOEXCEPT;
    const nanoem_model_vertex_t *resolveSoftBodyVertexObject(
        nanoem_physics_soft_body_t *body, int offset) const NANOEM_DECL_NOEXCEPT;
    void getSoftBodyVertexPosition(
        const nanoem_physics_soft_body_t *body, int offset, nanoem_f32_t *value) const NANOEM_DECL_NOEXCEPT;
    void getSoftBodyVertexNormal(
        const nanoem_physics_soft_body_t *body, int offset, nanoem_f32_t *value) const NANOEM_DECL_NOEXCEPT;
    void setSoftBodyVertexPosition(nanoem_physics_soft_body_t *body, int offset, const nanoem_f32_t *value);
    void setSoftBodyVertexNormal(nanoem_physics_soft_body_t *body, int offset, const nanoem_f32_t *value);

    Vector3 direction() const NANOEM_DECL_NOEXCEPT;
    void setDirection(const Vector3 &value);
    nanoem_f32_t acceleration() const NANOEM_DECL_NOEXCEPT;
    void setAcceleration(nanoem_f32_t value);
    nanoem_f32_t noise() const NANOEM_DECL_NOEXCEPT;
    void setNoise(nanoem_f32_t value);
    bool isNoiseEnabled() const NANOEM_DECL_NOEXCEPT;
    void setNoiseEnabled(bool value);
    void apply();

    const nanoem_f32_t *gravity() const NANOEM_DECL_NOEXCEPT;
    void setGravity(const nanoem_f32_t *value);
    nanoem_u32_t debugGeometryFlags() const NANOEM_DECL_NOEXCEPT;
    void setDebugGeometryFlags(nanoem_u32_t value);
    bool isActive() const NANOEM_DECL_NOEXCEPT;
    void setActive(bool value);
    bool isGroundEnabled() const NANOEM_DECL_NOEXCEPT;
    void setGroundEnabled(bool value);

private:
    struct PrivateContext;
    PrivateContext *m_context;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_PHYSICS_H_ */
