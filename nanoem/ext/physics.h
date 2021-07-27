/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EXT_PHYSICS_H_
#define NANOEM_EXT_PHYSICS_H_

#include "../nanoem.h"

/**
 * \defgroup nanoem nanoem
 * @{
 */

/**
 * \defgroup nanoem_physics Physics Interface
 * @{
 */
NANOEM_DECL_OPAQUE(nanoem_physics_world_t);
NANOEM_DECL_OPAQUE(nanoem_physics_rigid_body_t);
NANOEM_DECL_OPAQUE(nanoem_physics_joint_t);
NANOEM_DECL_OPAQUE(nanoem_physics_soft_body_t);
NANOEM_DECL_OPAQUE(nanoem_physics_debug_geometry_t);
NANOEM_DECL_OPAQUE(nanoem_physics_motion_state_t);

/**
 * \defgroup nanoem_physics_world Physics World
 * @{
 */

/**
 * \brief 
 * 
 * \param opaque 
 * \return NANOEM_DECL_API nanoem_bool_t APIENTRY 
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemPhysicsWorldIsAvailable(void *opaque);

/**
 * \brief 
 * 
 * \param opaque 
 * \param status 
 * \return NANOEM_DECL_API nanoem_physics_world_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_physics_world_t *APIENTRY
nanoemPhysicsWorldCreate(void *opaque, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param world 
 * \param rigid_body 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldAddRigidBody(nanoem_physics_world_t *world, nanoem_physics_rigid_body_t *rigid_body);

/**
 * \brief 
 * 
 * \param world 
 * \param soft_body 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldAddSoftBody(nanoem_physics_world_t *world, nanoem_physics_soft_body_t *soft_body);

/**
 * \brief 
 * 
 * \param world 
 * \param joint 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldAddJoint(nanoem_physics_world_t *world, nanoem_physics_joint_t *joint);

/**
 * \brief 
 * 
 * \param world 
 * \param rigid_body 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldRemoveRigidBody(nanoem_physics_world_t *world, nanoem_physics_rigid_body_t *rigid_body);

/**
 * \brief 
 * 
 * \param world 
 * \param soft_body 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldRemoveSoftBody(nanoem_physics_world_t *world, nanoem_physics_soft_body_t *soft_body);

/**
 * \brief 
 * 
 * \param world 
 * \param joint 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldRemoveJoint(nanoem_physics_world_t *world, nanoem_physics_joint_t *joint);

/**
 * \brief 
 * 
 * \param world 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldSetPreferredFPS(nanoem_physics_world_t *world, int value);

/**
 * \brief 
 * 
 * \param world 
 * \param delta 
 * \return NANOEM_DECL_API int APIENTRY 
 */
NANOEM_DECL_API int APIENTRY
nanoemPhysicsWorldStepSimulation(nanoem_physics_world_t *world, nanoem_f32_t delta);

/**
 * \brief 
 * 
 * \param world 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldReset(nanoem_physics_world_t *world);

/**
 * \brief 
 * 
 * \param world 
 * \return NANOEM_DECL_API const nanoem_f32_t* APIENTRY 
 */
NANOEM_DECL_API const nanoem_f32_t * APIENTRY
nanoemPhysicsWorldGetGravity(const nanoem_physics_world_t *world);

/**
 * \brief 
 * 
 * \param world 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldSetGravity(nanoem_physics_world_t *world, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param world 
 * \return NANOEM_DECL_API const nanoem_f32_t* APIENTRY 
 */
NANOEM_DECL_API const nanoem_f32_t * APIENTRY
nanoemPhysicsWorldGetGravityFactor(const nanoem_physics_world_t *world);

/**
 * \brief 
 * 
 * \param world 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldSetGravityFactor(nanoem_physics_world_t *world, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param world 
 * \return NANOEM_DECL_API nanoem_bool_t APIENTRY 
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemPhysicsWorldIsActive(const nanoem_physics_world_t *world);

/**
 * \brief 
 * 
 * \param world 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldSetActive(nanoem_physics_world_t *world, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param world 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldSetDeactivationTimeThreshold(nanoem_physics_world_t *world, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param world 
 * \return NANOEM_DECL_API nanoem_bool_t APIENTRY 
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemPhysicsWorldIsGroundEnabled(const nanoem_physics_world_t *world);

/**
 * \brief 
 * 
 * \param world 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldSetGroundEnabled(nanoem_physics_world_t *world, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param world 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldDestroy(nanoem_physics_world_t *world);
/** @} */

/**
 * \defgroup nanoem_physics_rigid_body Physics Rigid Body
 * @{
 */

/**
 * \brief 
 * 
 * \param value 
 * \param opaque 
 * \param status 
 * \return NANOEM_DECL_API nanoem_physics_rigid_body_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_physics_rigid_body_t *APIENTRY
nanoemPhysicsRigidBodyCreate(const nanoem_model_rigid_body_t *value, void *opaque, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \return NANOEM_DECL_API nanoem_physics_motion_state_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_physics_motion_state_t *APIENTRY
nanoemPhysicsRigidBodyGetMotionState(const nanoem_physics_rigid_body_t *rigid_body);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodyGetWorldTransform(const nanoem_physics_rigid_body_t *rigid_body, nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodySetWorldTransform(nanoem_physics_rigid_body_t *rigid_body, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \return NANOEM_DECL_API nanoem_bool_t APIENTRY 
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemPhysicsRigidBodyIsVisualizeEnabled(const nanoem_physics_rigid_body_t *rigid_body);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodySetVisualizeEnabled(nanoem_physics_rigid_body_t *rigid_body, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param rigid_body 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodyDisableDeactivation(nanoem_physics_rigid_body_t *rigid_body);

/**
 * \brief 
 * 
 * \param rigid_body 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodySetActive(nanoem_physics_rigid_body_t *rigid_body);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodySetKinematic(nanoem_physics_rigid_body_t* rigid_body, int value);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \return NANOEM_DECL_API int APIENTRY 
 */
NANOEM_DECL_API int APIENTRY
nanoemPhysicsRigidBodyIsKinematic(nanoem_physics_rigid_body_t* rigid_body);

/**
 * \brief 
 * 
 * \param rigid_body 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodyResetLinearVelocity(nanoem_physics_rigid_body_t *rigid_body);

/**
 * \brief 
 * 
 * \param rigid_body 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodyResetStates(nanoem_physics_rigid_body_t *rigid_body);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodyApplyTorqueImpulse(nanoem_physics_rigid_body_t *rigid_body, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodyApplyVelocityImpulse(nanoem_physics_rigid_body_t *rigid_body, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param rigid_body 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodyDestroy(nanoem_physics_rigid_body_t *rigid_body);
/** @} */

/**
 * \defgroup nanoem_physics_motion_state Physics Motion State
 * @{
 */

/**
 * \brief 
 * 
 * \param motion_state 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsMotionStateGetInitialWorldTransform(const nanoem_physics_motion_state_t *motion_state, nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param motion_state 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsMotionStateGetCurrentWorldTransform(const nanoem_physics_motion_state_t *motion_state, nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param motion_state 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsMotionStateSetCurrentWorldTransform(nanoem_physics_motion_state_t *motion_state, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param motion_state 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsMotionStateGetCenterOfMassOffset(const nanoem_physics_motion_state_t *motion_state, nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param motion_state 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsMotionStateSetCenterOfMassOffset(nanoem_physics_motion_state_t *motion_state, const nanoem_f32_t *value);
/** @} */

/**
 * \defgroup nanoem_physics_joint Physics Joint
 * @{
 */

/**
 * \brief 
 * 
 * \param value 
 * \param opaque 
 * \param status 
 * \return NANOEM_DECL_API nanoem_physics_joint_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_physics_joint_t *APIENTRY
nanoemPhysicsJointCreate(const nanoem_model_joint_t *value, void *opaque, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsJointGetCalculatedTransformA(const nanoem_physics_joint_t *joint, nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsJointGetCalculatedTransformB(const nanoem_physics_joint_t *joint, nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param joint 
 * \return NANOEM_DECL_API nanoem_bool_t APIENTRY 
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemPhysicsJointIsVisualizeEnabled(const nanoem_physics_joint_t *joint);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsJointSetVisualizeEnabled(nanoem_physics_joint_t *joint, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param joint 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsJointDestroy(nanoem_physics_joint_t *joint);
/** @} */

/**
 * \defgroup nanoem_physics_debug_geometry Physics Debug Geometry
 * @{
 */

/**
 * \brief 
 * 
 * \param world 
 * \return NANOEM_DECL_API nanoem_u32_t APIENTRY 
 */
NANOEM_DECL_API nanoem_u32_t APIENTRY
nanoemPhysicsWorldGetDebugGeomtryFlags(const nanoem_physics_world_t *world);

/**
 * \brief 
 * 
 * \param world 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldSetDebugGeomtryFlags(nanoem_physics_world_t *world, nanoem_u32_t value);

/**
 * \brief 
 * 
 * \param world 
 * \param num_objects 
 * \return NANOEM_DECL_API nanoem_physics_debug_geometry_t* const* APIENTRY 
 */
NANOEM_DECL_API nanoem_physics_debug_geometry_t *const *APIENTRY
nanoemPhysicsWorldGetDebugGeomtryObjects(const nanoem_physics_world_t *world, int *num_objects);

/**
 * \brief 
 * 
 * \param geometry 
 * \return NANOEM_DECL_API const nanoem_f32_t* APIENTRY 
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemPhysicsDebugGeometryGetFromPosition(const nanoem_physics_debug_geometry_t *geometry);

/**
 * \brief 
 * 
 * \param geometry 
 * \return NANOEM_DECL_API const nanoem_f32_t* APIENTRY 
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemPhysicsDebugGeometryGetToPosition(const nanoem_physics_debug_geometry_t *geometry);

/**
 * \brief 
 * 
 * \param geometry 
 * \return NANOEM_DECL_API const nanoem_f32_t* APIENTRY 
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemPhysicsDebugGeometryGetColor(const nanoem_physics_debug_geometry_t *geometry);
/** @} */

/**
 * \defgroup nanoem_physics_rigid_body Physics Rigid Body
 * @{
 */

/**
 * \brief 
 * 
 * \param value 
 * \param world 
 * \param status 
 * \return NANOEM_DECL_API nanoem_physics_soft_body_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_physics_soft_body_t *APIENTRY
nanoemPhysicsSoftBodyCreate(const nanoem_model_soft_body_t *value, nanoem_physics_world_t *world, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param soft_body 
 * \return NANOEM_DECL_API int APIENTRY 
 */
NANOEM_DECL_API int APIENTRY
nanoemPhysicsSoftBodyGetNumVertexObjects(const nanoem_physics_soft_body_t *soft_body);

/**
 * \brief 
 * 
 * \param soft_body 
 * \param offset 
 * \return NANOEM_DECL_API const nanoem_model_vertex_t* APIENTRY 
 */
NANOEM_DECL_API const nanoem_model_vertex_t * APIENTRY
nanoemPhysicsSoftBodyGetVertexObject(const nanoem_physics_soft_body_t *soft_body, int offset);

/**
 * \brief 
 * 
 * \param soft_body 
 * \param offset 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsSoftBodyGetVertexPosition(const nanoem_physics_soft_body_t *soft_body, int offset, nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param soft_body 
 * \param offset 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsSoftBodyGetVertexNormal(const nanoem_physics_soft_body_t *soft_body, int offset, nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param soft_body 
 * \param offset 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsSoftBodySetVertexPosition(nanoem_physics_soft_body_t *soft_body, int offset, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param soft_body 
 * \param offset 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsSoftBodySetVertexNormal(nanoem_physics_soft_body_t *soft_body, int offset, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param soft_body 
 * \return NANOEM_DECL_API nanoem_bool_t APIENTRY 
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemPhysicsSoftBodyIsVisualizeEnabled(const nanoem_physics_soft_body_t *soft_body);

/**
 * \brief 
 * 
 * \param soft_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsSoftBodySetVisualizeEnabled(nanoem_physics_soft_body_t *soft_body, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param soft_body 
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsSoftBodyDestroy(nanoem_physics_soft_body_t *soft_body);
/** @} */

/** @} */

/** @} */

#endif /* NANOEM_EXT_PHYSICS_H_ */
