/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/PhysicsEngine.h"

#include "bx/os.h"
#include "emapp/Constants.h"
#include "emapp/private/CommonInclude.h"

#ifndef DLL
#include "nanoem/ext/physics.h"
#endif

namespace nanoem {

struct PhysicsEngine::PrivateContext {
    typedef nanoem_bool_t(APIENTRY *PFN_nanoemPhysicsWorldIsAvailable)(void *opaque);
    typedef nanoem_physics_world_t *(APIENTRY *PFN_nanoemPhysicsWorldCreate)(void *opaque, nanoem_status_t *status);
    typedef void(APIENTRY *PFN_nanoemPhysicsWorldAddRigidBody)(
        nanoem_physics_world_t *world, nanoem_physics_rigid_body_t *rigid_body);
    typedef void(APIENTRY *PFN_nanoemPhysicsWorldAddJoint)(
        nanoem_physics_world_t *world, nanoem_physics_joint_t *joint);
    typedef void(APIENTRY *PFN_nanoemPhysicsWorldRemoveRigidBody)(
        nanoem_physics_world_t *world, nanoem_physics_rigid_body_t *rigid_body);
    typedef void(APIENTRY *PFN_nanoemPhysicsWorldRemoveJoint)(
        nanoem_physics_world_t *world, nanoem_physics_joint_t *joint);
    typedef void(APIENTRY *PFN_nanoemPhysicsWorldSetPreferredFPS)(nanoem_physics_world_t *world, int value);
    typedef int(APIENTRY *PFN_nanoemPhysicsWorldStepSimulation)(nanoem_physics_world_t *world, nanoem_f32_t delta);
    typedef void(APIENTRY *PFN_nanoemPhysicsWorldReset)(nanoem_physics_world_t *world);
    typedef const nanoem_f32_t *(APIENTRY *PFN_nanoemPhysicsWorldGetGravity)(const nanoem_physics_world_t *world);
    typedef void(APIENTRY *PFN_nanoemPhysicsWorldSetGravity)(nanoem_physics_world_t *world, const nanoem_f32_t *value);
    typedef const nanoem_f32_t *(APIENTRY *PFN_nanoemPhysicsWorldGetGravityFactor)(const nanoem_physics_world_t *world);
    typedef void(APIENTRY *PFN_nanoemPhysicsWorldSetGravityFactor)(
        nanoem_physics_world_t *world, const nanoem_f32_t *value);
    typedef nanoem_bool_t(APIENTRY *PFN_nanoemPhysicsWorldIsActive)(const nanoem_physics_world_t *world);
    typedef void(APIENTRY *PFN_nanoemPhysicsWorldSetActive)(nanoem_physics_world_t *world, nanoem_bool_t value);
    typedef void(APIENTRY *PFN_nanoemPhysicsWorldSetDeactivationTimeThreshold)(
        nanoem_physics_world_t *world, nanoem_f32_t value);
    typedef nanoem_bool_t(APIENTRY *PFN_nanoemPhysicsWorldIsGroundEnabled)(const nanoem_physics_world_t *world);
    typedef void(APIENTRY *PFN_nanoemPhysicsWorldSetGroundEnabled)(nanoem_physics_world_t *world, nanoem_bool_t value);
    typedef void(APIENTRY *PFN_nanoemPhysicsWorldDestroy)(nanoem_physics_world_t *world);
    typedef nanoem_physics_rigid_body_t *(APIENTRY *PFN_nanoemPhysicsRigidBodyCreate)(
        const nanoem_model_rigid_body_t *value, void *opaque, nanoem_status_t *status);
    typedef nanoem_physics_motion_state_t *(APIENTRY *PFN_nanoemPhysicsRigidBodyGetMotionState)(
        const nanoem_physics_rigid_body_t *rigid_body);
    typedef void(APIENTRY *PFN_nanoemPhysicsRigidBodyGetWorldTransform)(
        const nanoem_physics_rigid_body_t *rigid_body, nanoem_f32_t *value);
    typedef void(APIENTRY *PFN_nanoemPhysicsRigidBodySetWorldTransform)(
        nanoem_physics_rigid_body_t *rigid_body, const nanoem_f32_t *value);
    typedef nanoem_bool_t(APIENTRY *PFN_nanoemPhysicsRigidBodyIsVisualizeEnabled)(
        const nanoem_physics_rigid_body_t *rigid_body);
    typedef void(APIENTRY *PFN_nanoemPhysicsRigidBodySetVisualizeEnabled)(
        nanoem_physics_rigid_body_t *rigid_body, nanoem_bool_t value);
    typedef void(APIENTRY *PFN_nanoemPhysicsRigidBodyDisableDeactivation)(nanoem_physics_rigid_body_t *rigid_body);
    typedef void(APIENTRY *PFN_nanoemPhysicsRigidBodySetActive)(nanoem_physics_rigid_body_t *rigid_body);
    typedef void(APIENTRY *PFN_nanoemPhysicsRigidBodySetKinematic)(nanoem_physics_rigid_body_t *rigid_body, int value);
    typedef int(APIENTRY *PFN_nanoemPhysicsRigidBodyIsKinematic)(nanoem_physics_rigid_body_t *rigid_body);
    typedef void(APIENTRY *PFN_nanoemPhysicsRigidBodyResetLinearVelocity)(nanoem_physics_rigid_body_t *rigid_body);
    typedef void(APIENTRY *PFN_nanoemPhysicsRigidBodyResetStates)(nanoem_physics_rigid_body_t *rigid_body);
    typedef void(APIENTRY *PFN_nanoemPhysicsRigidBodyApplyTorqueImpulse)(
        nanoem_physics_rigid_body_t *rigid_body, const nanoem_f32_t *value);
    typedef void(APIENTRY *PFN_nanoemPhysicsRigidBodyApplyVelocityImpulse)(
        nanoem_physics_rigid_body_t *rigid_body, const nanoem_f32_t *value);
    typedef void(APIENTRY *PFN_nanoemPhysicsRigidBodyDestroy)(nanoem_physics_rigid_body_t *rigid_body);
    typedef void(APIENTRY *PFN_nanoemPhysicsMotionStateGetInitialWorldTransform)(
        const nanoem_physics_motion_state_t *motion_state, nanoem_f32_t *value);
    typedef void(APIENTRY *PFN_nanoemPhysicsMotionStateGetCurrentWorldTransform)(
        const nanoem_physics_motion_state_t *motion_state, nanoem_f32_t *value);
    typedef void(APIENTRY *PFN_nanoemPhysicsMotionStateSetCurrentWorldTransform)(
        nanoem_physics_motion_state_t *motion_state, const nanoem_f32_t *value);
    typedef void(APIENTRY *PFN_nanoemPhysicsMotionStateGetCenterOfMassOffset)(
        const nanoem_physics_motion_state_t *motion_state, nanoem_f32_t *value);
    typedef void(APIENTRY *PFN_nanoemPhysicsMotionStateSetCenterOfMassOffset)(
        nanoem_physics_motion_state_t *motion_state, const nanoem_f32_t *value);
    typedef nanoem_physics_joint_t *(APIENTRY *PFN_nanoemPhysicsJointCreate)(
        const nanoem_model_joint_t *value, void *opaque, nanoem_status_t *status);
    typedef void(APIENTRY *PFN_nanoemPhysicsJointGetCalculatedTransformA)(
        const nanoem_physics_joint_t *joint, nanoem_f32_t *value);
    typedef void(APIENTRY *PFN_nanoemPhysicsJointGetCalculatedTransformB)(
        const nanoem_physics_joint_t *joint, nanoem_f32_t *value);
    typedef nanoem_bool_t(APIENTRY *PFN_nanoemPhysicsJointIsVisualizeEnabled)(const nanoem_physics_joint_t *joint);
    typedef void(APIENTRY *PFN_nanoemPhysicsJointSetVisualizeEnabled)(
        nanoem_physics_joint_t *joint, nanoem_bool_t value);
    typedef void(APIENTRY *PFN_nanoemPhysicsJointDestroy)(nanoem_physics_joint_t *joint);
    typedef nanoem_u32_t(APIENTRY *PFN_nanoemPhysicsWorldGetDebugGeomtryFlags)(const nanoem_physics_world_t *world);
    typedef void(APIENTRY *PFN_nanoemPhysicsWorldSetDebugGeomtryFlags)(
        nanoem_physics_world_t *world, nanoem_u32_t value);
    typedef nanoem_physics_debug_geometry_t *const *(APIENTRY *PFN_nanoemPhysicsWorldGetDebugGeomtryObjects)(
        const nanoem_physics_world_t *world, int *num_objects);
    typedef const nanoem_f32_t *(APIENTRY *PFN_nanoemPhysicsDebugGeometryGetFromPosition)(
        const nanoem_physics_debug_geometry_t *geometry);
    typedef const nanoem_f32_t *(APIENTRY *PFN_nanoemPhysicsDebugGeometryGetToPosition)(
        const nanoem_physics_debug_geometry_t *geometry);
    typedef const nanoem_f32_t *(APIENTRY *PFN_nanoemPhysicsDebugGeometryGetColor)(
        const nanoem_physics_debug_geometry_t *geometry);
    typedef nanoem_physics_soft_body_t *(APIENTRY *PFN_nanoemPhysicsSoftBodyCreate)(
        const nanoem_model_soft_body_t *value, nanoem_physics_world_t *world, nanoem_status_t *status);
    typedef void(APIENTRY *PFN_nanoemPhysicsSoftBodyDestroy)(nanoem_physics_soft_body_t *soft_body);
    typedef void(APIENTRY *PFN_nanoemPhysicsWorldAddSoftBody)(
        nanoem_physics_world_t *world, nanoem_physics_soft_body_t *soft_body);
    typedef void(APIENTRY *PFN_nanoemPhysicsWorldRemoveSoftBody)(
        nanoem_physics_world_t *world, nanoem_physics_soft_body_t *soft_body);
    typedef int(APIENTRY *PFN_nanoemPhysicsSoftBodyGetNumVertexObjects)(const nanoem_physics_soft_body_t *soft_body);
    typedef const nanoem_model_vertex_t *(APIENTRY *PFN_nanoemPhysicsSoftBodyGetVertexObject)(
        const nanoem_physics_soft_body_t *soft_body, int offset);
    typedef void(APIENTRY *PFN_nanoemPhysicsSoftBodyGetVertexPosition)(
        const nanoem_physics_soft_body_t *soft_body, int offset, nanoem_f32_t *value);
    typedef void(APIENTRY *PFN_nanoemPhysicsSoftBodyGetVertexNormal)(
        const nanoem_physics_soft_body_t *soft_body, int offset, nanoem_f32_t *value);
    typedef void(APIENTRY *PFN_nanoemPhysicsSoftBodySetVertexPosition)(
        nanoem_physics_soft_body_t *soft_body, int offset, const nanoem_f32_t *value);
    typedef void(APIENTRY *PFN_nanoemPhysicsSoftBodySetVertexNormal)(
        nanoem_physics_soft_body_t *soft_body, int offset, const nanoem_f32_t *value);
    typedef nanoem_bool_t(APIENTRY *PFN_nanoemPhysicsSoftBodyIsVisualizeEnabled)(
        const nanoem_physics_soft_body_t *soft_body);
    typedef void(APIENTRY *PFN_nanoemPhysicsSoftBodySetVisualizeEnabled)(
        nanoem_physics_soft_body_t *soft_body, nanoem_bool_t value);

    PrivateContext()
        : m_opaque(nullptr)
        , m_mode(PhysicsEngine::kSimulationModeDisable)
        , m_direction(-Constants::kUnitY)
        , m_acceleration(9.8f)
        , m_noiseValue(0)
        , m_noiseEnabled(false)
        , worldIsAvailable(nullptr)
        , worldCreate(nullptr)
        , worldAddRigidBody(nullptr)
        , worldAddJoint(nullptr)
        , worldRemoveRigidBody(nullptr)
        , worldRemoveJoint(nullptr)
        , worldSetPreferredFPS(nullptr)
        , worldStepSimulation(nullptr)
        , worldReset(nullptr)
        , worldGetGravity(nullptr)
        , worldSetGravity(nullptr)
        , worldGetGravityFactor(nullptr)
        , worldSetGravityFactor(nullptr)
        , worldIsActive(nullptr)
        , worldSetActive(nullptr)
        , worldSetDeactivationTimeThreshold(nullptr)
        , worldIsGroundEnabled(nullptr)
        , worldSetGroundEnabled(nullptr)
        , worldDestroy(nullptr)
        , rigidBodyCreate(nullptr)
        , rigidBodyGetMotionState(nullptr)
        , rigidBodyGetWorldTransform(nullptr)
        , rigidBodySetWorldTransform(nullptr)
        , rigidBodyIsVisualizeEnabled(nullptr)
        , rigidBodySetVisualizeEnabled(nullptr)
        , rigidBodyDisableDeactivation(nullptr)
        , rigidBodySetActive(nullptr)
        , rigidBodyResetLinearVelocity(nullptr)
        , rigidBodyResetStates(nullptr)
        , rigidBodyApplyTorqueImpulse(nullptr)
        , rigidBodyApplyVelocityImpulse(nullptr)
        , rigidBodyDestroy(nullptr)
        , motionStateGetInitialWorldTransform(nullptr)
        , motionStateGetCurrentWorldTransform(nullptr)
        , motionStateSetCurrentWorldTransform(nullptr)
        , motionStateGetCenterOfMassOffset(nullptr)
        , motionStateSetCenterOfMassOffset(nullptr)
        , jointCreate(nullptr)
        , jointGetCalculatedTransformA(nullptr)
        , jointGetCalculatedTransformB(nullptr)
        , jointIsVisualizeEnabled(nullptr)
        , jointSetVisualizeEnabled(nullptr)
        , jointDestroy(nullptr)
        , worldGetDebugGeomtryFlags(nullptr)
        , worldSetDebugGeomtryFlags(nullptr)
        , worldGetDebugGeomtryObjects(nullptr)
        , debugGeometryGetFromPosition(nullptr)
        , debugGeometryGetToPosition(nullptr)
        , debugGeometryGetColor(nullptr)
        , softBodyCreate(nullptr)
        , softBodyDestroy(nullptr)
        , worldAddSoftBody(nullptr)
        , worldRemoveSoftBody(nullptr)
        , softBodyGetNumVertexObjects(nullptr)
        , softBodyGetVertexObject(nullptr)
        , softBodyGetVertexPosition(nullptr)
        , softBodyGetVertexNormal(nullptr)
        , softBodySetVertexPosition(nullptr)
        , softBodySetVertexNormal(nullptr)
        , softBodyIsVisualizeEnabled(nullptr)
        , softBodySetVisualizeEnabled(nullptr)
    {
    }
    ~PrivateContext() NANOEM_DECL_NOEXCEPT
    {
    }

    bool
    resolve(const char *dllPath)
    {
#ifdef DLL
        bool valid = false;
        void *opaque = bx::dlopen(dllPath);
        if (opaque) {
            valid = true;
            resolveSymbol(opaque, "nanoemPhysicsWorldIsAvailable", worldIsAvailable, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldCreate", worldCreate, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldAddRigidBody", worldAddRigidBody, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldAddJoint", worldAddJoint, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldRemoveRigidBody", worldRemoveRigidBody, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldRemoveJoint", worldRemoveJoint, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldSetPreferredFPS", worldSetPreferredFPS, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldStepSimulation", worldStepSimulation, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldReset", worldReset, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldGetGravity", worldGetGravity, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldSetGravity", worldSetGravity, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldGetGravityFactor", worldGetGravityFactor, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldSetGravityFactor", worldSetGravityFactor, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldDestroy", worldDestroy, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldIsActive", worldIsActive, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldSetActive", worldSetActive, valid);
            resolveSymbol(
                opaque, "nanoemPhysicsWorldSetDeactivationTimeThreshold", worldSetDeactivationTimeThreshold, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldIsGroundEnabled", worldIsGroundEnabled, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldSetGroundEnabled", worldSetGroundEnabled, valid);
            resolveSymbol(opaque, "nanoemPhysicsMotionStateGetWorldTransform", motionStateGetWorldTransform, valid);
            resolveSymbol(opaque, "nanoemPhysicsMotionStateSetWorldTransform", motionStateSetWorldTransform, valid);
            resolveSymbol(opaque, "nanoemPhysicsRigidBodyCreate", rigidBodyCreate, valid);
            resolveSymbol(opaque, "nanoemPhysicsRigidBodyGetMotionState", rigidBodyGetMotionState, valid);
            resolveSymbol(opaque, "nanoemPhysicsRigidBodyGetWorldTransform", rigidBodyGetWorldTransform, valid);
            resolveSymbol(opaque, "nanoemPhysicsRigidBodySetWorldTransform", rigidBodySetWorldTransform, valid);
            resolveSymbol(
                opaque, "nanoemPhysicsMotionStateGetCenterOfMassOffset", motionStateGetCenterOfMassOffset, valid);
            resolveSymbol(
                opaque, "nanoemPhysicsMotionStateSetCenterOfMassOffset", motionStateSetCenterOfMassOffset, valid);
            resolveSymbol(opaque, "nanoemPhysicsRigidBodyIsVisualizeEnabled", rigidBodyIsVisualizeEnabled, valid);
            resolveSymbol(opaque, "nanoemPhysicsRigidBodySetVisualizeEnabled", rigidBodySetVisualizeEnabled, valid);
            resolveSymbol(opaque, "nanoemPhysicsRigidBodyDisableDeactivation", rigidBodyDisableDeactivation, valid);
            resolveSymbol(opaque, "nanoemPhysicsRigidBodySetActive", rigidBodySetActive, valid);
            resolveSymbol(opaque, "nanoemPhysicsRigidBodySetKinematic", rigidBodySetKinematic, valid);
            resolveSymbol(opaque, "nanoemPhysicsRigidBodyIsKinematic", rigidBodyIsKinematic, valid);
            resolveSymbol(opaque, "nanoemPhysicsRigidBodyResetLinearVelocity", rigidBodyResetLinearVelocity, valid);
            resolveSymbol(opaque, "nanoemPhysicsRigidBodyResetStates", rigidBodyResetStates, valid);
            resolveSymbol(opaque, "nanoemPhysicsRigidBodyApplyTorqueImpulse", rigidBodyApplyTorqueImpulse, valid);
            resolveSymbol(opaque, "nanoemPhysicsRigidBodyApplyVelocityImpulse", rigidBodyApplyVelocityImpulse, valid);
            resolveSymbol(opaque, "nanoemPhysicsRigidBodyDestroy", rigidBodyDestroy, valid);
            resolveSymbol(opaque, "nanoemPhysicsJointCreate", jointCreate, valid);
            resolveSymbol(opaque, "nanoemPhysicsJointGetCalculatedTransformA", jointGetCalculatedTransformA, valid);
            resolveSymbol(opaque, "nanoemPhysicsJointGetCalculatedTransformB", jointGetCalculatedTransformB, valid);
            resolveSymbol(opaque, "nanoemPhysicsJointIsVisualizeEnabled", jointIsVisualizeEnabled, valid);
            resolveSymbol(opaque, "nanoemPhysicsJointSetVisualizeEnabled", jointSetVisualizeEnabled, valid);
            resolveSymbol(opaque, "nanoemPhysicsJointDestroy", jointDestroy, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldGetDebugGeomtryFlags", worldGetDebugGeomtryFlags, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldSetDebugGeomtryFlags", worldSetDebugGeomtryFlags, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldGetDebugGeomtryObjects", worldGetDebugGeomtryObjects, valid);
            resolveSymbol(opaque, "nanoemPhysicsDebugGeometryGetFromPosition", debugGeometryGetFromPosition, valid);
            resolveSymbol(opaque, "nanoemPhysicsDebugGeometryGetToPosition", debugGeometryGetToPosition, valid);
            resolveSymbol(opaque, "nanoemPhysicsDebugGeometryGetColor", debugGeometryGetColor, valid);
            resolveSymbol(opaque, "nanoemPhysicsSoftBodyCreate", softBodyCreate, valid);
            resolveSymbol(opaque, "nanoemPhysicsSoftBodyDestroy", softBodyDestroy, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldAddSoftBody", worldAddSoftBody, valid);
            resolveSymbol(opaque, "nanoemPhysicsWorldRemoveSoftBody", worldRemoveSoftBody, valid);
            bx::dlclose(opaque);
        }
        return valid;
#else
        BX_UNUSED_1(dllPath);
        worldIsAvailable = nanoemPhysicsWorldIsAvailable;
        worldCreate = nanoemPhysicsWorldCreate;
        worldAddRigidBody = nanoemPhysicsWorldAddRigidBody;
        worldAddJoint = nanoemPhysicsWorldAddJoint;
        worldRemoveRigidBody = nanoemPhysicsWorldRemoveRigidBody;
        worldRemoveJoint = nanoemPhysicsWorldRemoveJoint;
        worldSetPreferredFPS = nanoemPhysicsWorldSetPreferredFPS;
        worldStepSimulation = nanoemPhysicsWorldStepSimulation;
        worldReset = nanoemPhysicsWorldReset;
        worldGetGravity = nanoemPhysicsWorldGetGravity;
        worldSetGravity = nanoemPhysicsWorldSetGravity;
        worldGetGravityFactor = nanoemPhysicsWorldGetGravityFactor;
        worldSetGravityFactor = nanoemPhysicsWorldSetGravityFactor;
        worldDestroy = nanoemPhysicsWorldDestroy;
        worldIsActive = nanoemPhysicsWorldIsActive;
        worldSetActive = nanoemPhysicsWorldSetActive;
        worldSetDeactivationTimeThreshold = nanoemPhysicsWorldSetDeactivationTimeThreshold;
        worldIsGroundEnabled = nanoemPhysicsWorldIsGroundEnabled;
        worldSetGroundEnabled = nanoemPhysicsWorldSetGroundEnabled;
        motionStateGetInitialWorldTransform = nanoemPhysicsMotionStateGetInitialWorldTransform;
        motionStateGetCurrentWorldTransform = nanoemPhysicsMotionStateGetCurrentWorldTransform;
        motionStateSetCurrentWorldTransform = nanoemPhysicsMotionStateSetCurrentWorldTransform;
        motionStateGetCenterOfMassOffset = nanoemPhysicsMotionStateGetCenterOfMassOffset;
        motionStateSetCenterOfMassOffset = nanoemPhysicsMotionStateSetCenterOfMassOffset;
        rigidBodyCreate = nanoemPhysicsRigidBodyCreate;
        rigidBodyGetMotionState = nanoemPhysicsRigidBodyGetMotionState;
        rigidBodyGetWorldTransform = nanoemPhysicsRigidBodyGetWorldTransform;
        rigidBodySetWorldTransform = nanoemPhysicsRigidBodySetWorldTransform;
        rigidBodyIsVisualizeEnabled = nanoemPhysicsRigidBodyIsVisualizeEnabled;
        rigidBodySetVisualizeEnabled = nanoemPhysicsRigidBodySetVisualizeEnabled;
        rigidBodyDisableDeactivation = nanoemPhysicsRigidBodyDisableDeactivation;
        rigidBodySetActive = nanoemPhysicsRigidBodySetActive;
        rigidBodySetKinematic = nanoemPhysicsRigidBodySetKinematic;
        rigidBodyIsKinematic = nanoemPhysicsRigidBodyIsKinematic;
        rigidBodyResetLinearVelocity = nanoemPhysicsRigidBodyResetLinearVelocity;
        rigidBodyResetStates = nanoemPhysicsRigidBodyResetStates;
        rigidBodyApplyTorqueImpulse = nanoemPhysicsRigidBodyApplyTorqueImpulse;
        rigidBodyApplyVelocityImpulse = nanoemPhysicsRigidBodyApplyVelocityImpulse;
        rigidBodyDestroy = nanoemPhysicsRigidBodyDestroy;
        jointCreate = nanoemPhysicsJointCreate;
        jointGetCalculatedTransformA = nanoemPhysicsJointGetCalculatedTransformA;
        jointGetCalculatedTransformB = nanoemPhysicsJointGetCalculatedTransformB;
        jointIsVisualizeEnabled = nanoemPhysicsJointIsVisualizeEnabled;
        jointSetVisualizeEnabled = nanoemPhysicsJointSetVisualizeEnabled;
        jointDestroy = nanoemPhysicsJointDestroy;
        worldGetDebugGeomtryFlags = nanoemPhysicsWorldGetDebugGeomtryFlags;
        worldSetDebugGeomtryFlags = nanoemPhysicsWorldSetDebugGeomtryFlags;
        worldGetDebugGeomtryObjects = nanoemPhysicsWorldGetDebugGeomtryObjects;
        debugGeometryGetFromPosition = nanoemPhysicsDebugGeometryGetFromPosition;
        debugGeometryGetToPosition = nanoemPhysicsDebugGeometryGetToPosition;
        debugGeometryGetColor = nanoemPhysicsDebugGeometryGetColor;
        softBodyCreate = nanoemPhysicsSoftBodyCreate;
        softBodyDestroy = nanoemPhysicsSoftBodyDestroy;
        worldAddSoftBody = nanoemPhysicsWorldAddSoftBody;
        worldRemoveSoftBody = nanoemPhysicsWorldRemoveSoftBody;
        softBodyGetNumVertexObjects = nanoemPhysicsSoftBodyGetNumVertexObjects;
        softBodyGetVertexObject = nanoemPhysicsSoftBodyGetVertexObject;
        softBodyGetVertexPosition = nanoemPhysicsSoftBodyGetVertexPosition;
        softBodyGetVertexNormal = nanoemPhysicsSoftBodyGetVertexNormal;
        softBodySetVertexPosition = nanoemPhysicsSoftBodySetVertexPosition;
        softBodySetVertexNormal = nanoemPhysicsSoftBodySetVertexNormal;
        softBodyIsVisualizeEnabled = nanoemPhysicsSoftBodyIsVisualizeEnabled;
        softBodySetVisualizeEnabled = nanoemPhysicsSoftBodySetVisualizeEnabled;
        return true;
#endif
    }

    nanoem_physics_world_t *m_opaque;
    PhysicsEngine::SimulationModeType m_mode;
    Vector3 m_direction;
    nanoem_f32_t m_acceleration;
    nanoem_f32_t m_noiseValue;
    bool m_noiseEnabled;

    PFN_nanoemPhysicsWorldIsAvailable worldIsAvailable;
    PFN_nanoemPhysicsWorldCreate worldCreate;
    PFN_nanoemPhysicsWorldAddRigidBody worldAddRigidBody;
    PFN_nanoemPhysicsWorldAddJoint worldAddJoint;
    PFN_nanoemPhysicsWorldRemoveRigidBody worldRemoveRigidBody;
    PFN_nanoemPhysicsWorldRemoveJoint worldRemoveJoint;
    PFN_nanoemPhysicsWorldSetPreferredFPS worldSetPreferredFPS;
    PFN_nanoemPhysicsWorldStepSimulation worldStepSimulation;
    PFN_nanoemPhysicsWorldReset worldReset;
    PFN_nanoemPhysicsWorldGetGravity worldGetGravity;
    PFN_nanoemPhysicsWorldSetGravity worldSetGravity;
    PFN_nanoemPhysicsWorldGetGravityFactor worldGetGravityFactor;
    PFN_nanoemPhysicsWorldSetGravityFactor worldSetGravityFactor;
    PFN_nanoemPhysicsWorldIsActive worldIsActive;
    PFN_nanoemPhysicsWorldSetActive worldSetActive;
    PFN_nanoemPhysicsWorldSetDeactivationTimeThreshold worldSetDeactivationTimeThreshold;
    PFN_nanoemPhysicsWorldIsGroundEnabled worldIsGroundEnabled;
    PFN_nanoemPhysicsWorldSetGroundEnabled worldSetGroundEnabled;
    PFN_nanoemPhysicsWorldDestroy worldDestroy;
    PFN_nanoemPhysicsRigidBodyCreate rigidBodyCreate;
    PFN_nanoemPhysicsRigidBodyGetMotionState rigidBodyGetMotionState;
    PFN_nanoemPhysicsRigidBodyGetWorldTransform rigidBodyGetWorldTransform;
    PFN_nanoemPhysicsRigidBodySetWorldTransform rigidBodySetWorldTransform;
    PFN_nanoemPhysicsRigidBodyIsVisualizeEnabled rigidBodyIsVisualizeEnabled;
    PFN_nanoemPhysicsRigidBodySetVisualizeEnabled rigidBodySetVisualizeEnabled;
    PFN_nanoemPhysicsRigidBodyDisableDeactivation rigidBodyDisableDeactivation;
    PFN_nanoemPhysicsRigidBodySetActive rigidBodySetActive;
    PFN_nanoemPhysicsRigidBodySetKinematic rigidBodySetKinematic;
    PFN_nanoemPhysicsRigidBodyIsKinematic rigidBodyIsKinematic;
    PFN_nanoemPhysicsRigidBodyResetLinearVelocity rigidBodyResetLinearVelocity;
    PFN_nanoemPhysicsRigidBodyResetStates rigidBodyResetStates;
    PFN_nanoemPhysicsRigidBodyApplyTorqueImpulse rigidBodyApplyTorqueImpulse;
    PFN_nanoemPhysicsRigidBodyApplyVelocityImpulse rigidBodyApplyVelocityImpulse;
    PFN_nanoemPhysicsRigidBodyDestroy rigidBodyDestroy;
    PFN_nanoemPhysicsMotionStateGetInitialWorldTransform motionStateGetInitialWorldTransform;
    PFN_nanoemPhysicsMotionStateGetCurrentWorldTransform motionStateGetCurrentWorldTransform;
    PFN_nanoemPhysicsMotionStateSetCurrentWorldTransform motionStateSetCurrentWorldTransform;
    PFN_nanoemPhysicsMotionStateGetCenterOfMassOffset motionStateGetCenterOfMassOffset;
    PFN_nanoemPhysicsMotionStateSetCenterOfMassOffset motionStateSetCenterOfMassOffset;
    PFN_nanoemPhysicsJointCreate jointCreate;
    PFN_nanoemPhysicsJointGetCalculatedTransformA jointGetCalculatedTransformA;
    PFN_nanoemPhysicsJointGetCalculatedTransformB jointGetCalculatedTransformB;
    PFN_nanoemPhysicsJointIsVisualizeEnabled jointIsVisualizeEnabled;
    PFN_nanoemPhysicsJointSetVisualizeEnabled jointSetVisualizeEnabled;
    PFN_nanoemPhysicsJointDestroy jointDestroy;
    PFN_nanoemPhysicsWorldGetDebugGeomtryFlags worldGetDebugGeomtryFlags;
    PFN_nanoemPhysicsWorldSetDebugGeomtryFlags worldSetDebugGeomtryFlags;
    PFN_nanoemPhysicsWorldGetDebugGeomtryObjects worldGetDebugGeomtryObjects;
    PFN_nanoemPhysicsDebugGeometryGetFromPosition debugGeometryGetFromPosition;
    PFN_nanoemPhysicsDebugGeometryGetToPosition debugGeometryGetToPosition;
    PFN_nanoemPhysicsDebugGeometryGetColor debugGeometryGetColor;
    PFN_nanoemPhysicsSoftBodyCreate softBodyCreate;
    PFN_nanoemPhysicsSoftBodyDestroy softBodyDestroy;
    PFN_nanoemPhysicsWorldAddSoftBody worldAddSoftBody;
    PFN_nanoemPhysicsWorldRemoveSoftBody worldRemoveSoftBody;
    PFN_nanoemPhysicsSoftBodyGetNumVertexObjects softBodyGetNumVertexObjects;
    PFN_nanoemPhysicsSoftBodyGetVertexObject softBodyGetVertexObject;
    PFN_nanoemPhysicsSoftBodyGetVertexPosition softBodyGetVertexPosition;
    PFN_nanoemPhysicsSoftBodyGetVertexNormal softBodyGetVertexNormal;
    PFN_nanoemPhysicsSoftBodySetVertexPosition softBodySetVertexPosition;
    PFN_nanoemPhysicsSoftBodySetVertexNormal softBodySetVertexNormal;
    PFN_nanoemPhysicsSoftBodyIsVisualizeEnabled softBodyIsVisualizeEnabled;
    PFN_nanoemPhysicsSoftBodySetVisualizeEnabled softBodySetVisualizeEnabled;
};

PhysicsEngine::PhysicsEngine()
    : m_context(nullptr)
{
    m_context = nanoem_new(PrivateContext);
}

PhysicsEngine::~PhysicsEngine() NANOEM_DECL_NOEXCEPT
{
    nanoem_delete_safe(m_context);
}

bool
PhysicsEngine::isAvailable() const NANOEM_DECL_NOEXCEPT
{
    return !!m_context->worldIsAvailable(m_context->m_opaque);
}

bool
PhysicsEngine::initialize(const char *dllPath)
{
    return m_context->resolve(dllPath);
}

void
PhysicsEngine::create(nanoem_status_t &status)
{
    m_context->m_opaque = m_context->worldCreate(nullptr, &status);
}

void
PhysicsEngine::destroy() NANOEM_DECL_NOEXCEPT
{
    m_context->worldDestroy(m_context->m_opaque);
    m_context->m_opaque = nullptr;
}

void
PhysicsEngine::reset() NANOEM_DECL_NOEXCEPT
{
    m_context->worldReset(m_context->m_opaque);
}

void
PhysicsEngine::stepSimulation(nanoem_f32_t delta)
{
    m_context->worldStepSimulation(m_context->m_opaque, delta);
}

PhysicsEngine::SimulationModeType
PhysicsEngine::simulationMode() const NANOEM_DECL_NOEXCEPT
{
    return m_context->m_mode;
}

void
PhysicsEngine::setSimulationMode(SimulationModeType value)
{
    if (m_context->m_mode != value) {
        setActive(value > PhysicsEngine::kSimulationModeDisable);
        m_context->m_mode = value;
    }
}

const nanoem_physics_world_t *
PhysicsEngine::worldOpaque() const NANOEM_DECL_NOEXCEPT
{
    return m_context->m_opaque;
}

nanoem_physics_world_t *
PhysicsEngine::worldOpaque()
{
    return m_context->m_opaque;
}

nanoem_physics_rigid_body_t *
PhysicsEngine::createRigidBody(const nanoem_model_rigid_body_t *value, nanoem_status_t &status)
{
    return m_context->rigidBodyCreate(value, nullptr, &status);
}

nanoem_physics_motion_state_t *
PhysicsEngine::motionState(const nanoem_physics_rigid_body_t *value)
{
    return m_context->rigidBodyGetMotionState(value);
}

void
PhysicsEngine::getWorldTransform(const nanoem_physics_rigid_body_t *body, nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT
{
    m_context->rigidBodyGetWorldTransform(body, value);
}

void
PhysicsEngine::setWorldTransform(nanoem_physics_rigid_body_t *body, const nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT
{
    m_context->rigidBodySetWorldTransform(body, value);
}

void
PhysicsEngine::addRigidBody(nanoem_physics_rigid_body_t *value)
{
    m_context->worldAddRigidBody(m_context->m_opaque, value);
}

void
PhysicsEngine::removeRigidBody(nanoem_physics_rigid_body_t *value)
{
    m_context->worldRemoveRigidBody(m_context->m_opaque, value);
}

void
PhysicsEngine::destroyRigidBody(nanoem_physics_rigid_body_t *value)
{
    m_context->rigidBodyDestroy(value);
}

void
PhysicsEngine::disableDeactivation(nanoem_physics_rigid_body_t *value)
{
    m_context->rigidBodyDisableDeactivation(value);
}

void
PhysicsEngine::setVisualizedEnabled(nanoem_physics_rigid_body_t *body, bool value) NANOEM_DECL_NOEXCEPT
{
    m_context->rigidBodySetVisualizeEnabled(body, value);
}

void
PhysicsEngine::setVisualizedEnabled(nanoem_physics_joint_t *joint, bool value) NANOEM_DECL_NOEXCEPT
{
    m_context->jointSetVisualizeEnabled(joint, value);
}

void
PhysicsEngine::setVisualizedEnabled(nanoem_physics_soft_body_t *body, bool value) NANOEM_DECL_NOEXCEPT
{
    m_context->softBodySetVisualizeEnabled(body, value);
}

nanoem_physics_joint_t *
PhysicsEngine::createJoint(const nanoem_model_joint_t *value, void *opaque, nanoem_status_t &status)
{
    return m_context->jointCreate(value, opaque, &status);
}

void
PhysicsEngine::addJoint(nanoem_physics_joint_t *value)
{
    m_context->worldAddJoint(m_context->m_opaque, value);
}

void
PhysicsEngine::getCalculatedTransformA(nanoem_physics_joint_t *joint, nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT
{
    m_context->jointGetCalculatedTransformA(joint, value);
}

void
PhysicsEngine::getCalculatedTransformB(nanoem_physics_joint_t *joint, nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT
{
    m_context->jointGetCalculatedTransformB(joint, value);
}

void
PhysicsEngine::removeJoint(nanoem_physics_joint_t *value) NANOEM_DECL_NOEXCEPT
{
    m_context->worldRemoveJoint(m_context->m_opaque, value);
}

void
PhysicsEngine::destroyJoint(nanoem_physics_joint_t *value) NANOEM_DECL_NOEXCEPT
{
    m_context->jointDestroy(value);
}

nanoem_physics_debug_geometry_t *const *
PhysicsEngine::debugGeometryObjects(int *numObjects) const NANOEM_DECL_NOEXCEPT
{
    return m_context->worldGetDebugGeomtryObjects(m_context->m_opaque, numObjects);
}

const nanoem_f32_t *
PhysicsEngine::geometryColor(const nanoem_physics_debug_geometry_t *value) const NANOEM_DECL_NOEXCEPT
{
    return m_context->debugGeometryGetColor(value);
}

const nanoem_f32_t *
PhysicsEngine::geometryFromPosition(const nanoem_physics_debug_geometry_t *value) const NANOEM_DECL_NOEXCEPT
{
    return m_context->debugGeometryGetFromPosition(value);
}

const nanoem_f32_t *
PhysicsEngine::geometryToPosition(const nanoem_physics_debug_geometry_t *value) const NANOEM_DECL_NOEXCEPT
{
    return m_context->debugGeometryGetToPosition(value);
}

nanoem_physics_soft_body_t *
PhysicsEngine::createSoftBody(const nanoem_model_soft_body_t *body, nanoem_status_t &status)
{
    return m_context->softBodyCreate(body, m_context->m_opaque, &status);
}

void
PhysicsEngine::addSoftBody(nanoem_physics_soft_body_t *body)
{
    m_context->worldAddSoftBody(m_context->m_opaque, body);
}

void
PhysicsEngine::removeSoftBody(nanoem_physics_soft_body_t *body)
{
    m_context->worldRemoveSoftBody(m_context->m_opaque, body);
}

void
PhysicsEngine::destroySoftBody(nanoem_physics_soft_body_t *body)
{
    m_context->softBodyDestroy(body);
}

int
PhysicsEngine::numSoftBodyVertices(nanoem_physics_soft_body_t *body) const NANOEM_DECL_NOEXCEPT
{
    return m_context->softBodyGetNumVertexObjects(body);
}

const nanoem_model_vertex_t *
PhysicsEngine::resolveSoftBodyVertexObject(nanoem_physics_soft_body_t *body, int offset) const NANOEM_DECL_NOEXCEPT
{
    return m_context->softBodyGetVertexObject(body, offset);
}

void
PhysicsEngine::getSoftBodyVertexPosition(
    const nanoem_physics_soft_body_t *body, int offset, nanoem_f32_t *value) const NANOEM_DECL_NOEXCEPT
{
    m_context->softBodyGetVertexPosition(body, offset, value);
}

void
PhysicsEngine::getSoftBodyVertexNormal(
    const nanoem_physics_soft_body_t *body, int offset, nanoem_f32_t *value) const NANOEM_DECL_NOEXCEPT
{
    m_context->softBodyGetVertexNormal(body, offset, value);
}

void
PhysicsEngine::setSoftBodyVertexPosition(nanoem_physics_soft_body_t *body, int offset, const nanoem_f32_t *value)
{
    m_context->softBodySetVertexPosition(body, offset, value);
}

void
PhysicsEngine::setSoftBodyVertexNormal(nanoem_physics_soft_body_t *body, int offset, const nanoem_f32_t *value)
{
    m_context->softBodySetVertexNormal(body, offset, value);
}

Vector3
PhysicsEngine::direction() const NANOEM_DECL_NOEXCEPT
{
    return m_context->m_direction;
}

void
PhysicsEngine::setDirection(const Vector3 &value)
{
    m_context->m_direction = glm::length(value) > 0 ? glm::normalize(value) : -Constants::kUnitY;
}

nanoem_f32_t
PhysicsEngine::acceleration() const NANOEM_DECL_NOEXCEPT
{
    return m_context->m_acceleration;
}

void
PhysicsEngine::setAcceleration(nanoem_f32_t value)
{
    m_context->m_acceleration = value;
}

nanoem_f32_t
PhysicsEngine::noise() const NANOEM_DECL_NOEXCEPT
{
    return m_context->m_noiseValue;
}

void
PhysicsEngine::setNoise(nanoem_f32_t value)
{
    m_context->m_noiseValue = value;
}

bool
PhysicsEngine::isNoiseEnabled() const NANOEM_DECL_NOEXCEPT
{
    return m_context->m_noiseEnabled;
}

void
PhysicsEngine::setNoiseEnabled(bool value)
{
    m_context->m_noiseEnabled = value;
}

void
PhysicsEngine::apply()
{
    setGravity(glm::value_ptr(m_context->m_direction * m_context->m_acceleration));
}

const nanoem_f32_t *
PhysicsEngine::gravity() const NANOEM_DECL_NOEXCEPT
{
    return m_context->worldGetGravity(m_context->m_opaque);
}

void
PhysicsEngine::setGravity(const nanoem_f32_t *value)
{
    m_context->worldSetGravity(m_context->m_opaque, value);
}

nanoem_u32_t
PhysicsEngine::debugGeometryFlags() const NANOEM_DECL_NOEXCEPT
{
    return m_context->worldGetDebugGeomtryFlags(m_context->m_opaque);
}

void
PhysicsEngine::setDebugGeometryFlags(nanoem_u32_t value)
{
    m_context->worldSetDebugGeomtryFlags(m_context->m_opaque, value);
}

bool
PhysicsEngine::isActive() const NANOEM_DECL_NOEXCEPT
{
    return !!m_context->worldIsActive(m_context->m_opaque);
}

void
PhysicsEngine::setActive(bool value)
{
    m_context->worldSetActive(m_context->m_opaque, value);
}

void
PhysicsEngine::setActive(nanoem_physics_rigid_body_t *body) NANOEM_DECL_NOEXCEPT
{
    m_context->rigidBodySetActive(body);
}

void
PhysicsEngine::applyTorqueImpulse(nanoem_physics_rigid_body_t *body, const nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT
{
    m_context->rigidBodyApplyTorqueImpulse(body, value);
}

void
PhysicsEngine::applyVelocityImpulse(nanoem_physics_rigid_body_t *body, const nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT
{
    m_context->rigidBodyApplyVelocityImpulse(body, value);
}

void
PhysicsEngine::resetLinearVelocity(nanoem_physics_rigid_body_t *body) NANOEM_DECL_NOEXCEPT
{
    m_context->rigidBodyResetLinearVelocity(body);
}

void
PhysicsEngine::resetStates(nanoem_physics_rigid_body_t *body) NANOEM_DECL_NOEXCEPT
{
    m_context->rigidBodyResetStates(body);
}

void
PhysicsEngine::setKinematic(nanoem_physics_rigid_body_t *body, bool value) NANOEM_DECL_NOEXCEPT
{
    m_context->rigidBodySetKinematic(body, value ? 1 : 0);
}

bool
PhysicsEngine::isKinematic(nanoem_physics_rigid_body_t *body) const NANOEM_DECL_NOEXCEPT
{
    return m_context->rigidBodyIsKinematic(body) != 0;
}

void
PhysicsEngine::getInitialTransform(
    const nanoem_physics_motion_state_t *state, nanoem_f32_t *value) const NANOEM_DECL_NOEXCEPT
{
    m_context->motionStateGetInitialWorldTransform(state, value);
}

void
PhysicsEngine::getWorldTransform(
    const nanoem_physics_motion_state_t *state, nanoem_f32_t *value) const NANOEM_DECL_NOEXCEPT
{
    m_context->motionStateGetCurrentWorldTransform(state, value);
}

void
PhysicsEngine::setWorldTransform(nanoem_physics_motion_state_t *state, const nanoem_f32_t *value)
{
    m_context->motionStateSetCurrentWorldTransform(state, value);
}

void
PhysicsEngine::getCenterOfMassOffset(
    const nanoem_physics_motion_state_t *state, nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT
{
    m_context->motionStateGetCenterOfMassOffset(state, value);
}

void
PhysicsEngine::setCenterOfMassOffset(
    nanoem_physics_motion_state_t *state, const nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT
{
    m_context->motionStateSetCenterOfMassOffset(state, value);
}

bool
PhysicsEngine::isGroundEnabled() const NANOEM_DECL_NOEXCEPT
{
    return !!m_context->worldIsGroundEnabled(m_context->m_opaque);
}

void
PhysicsEngine::setGroundEnabled(bool value)
{
    m_context->worldSetGroundEnabled(m_context->m_opaque, value);
}

} /* namespace nanoem */
