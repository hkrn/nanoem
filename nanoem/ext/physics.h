/*
   Copyright (c) 2015-2023 hkrn All rights reserved

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
 * \brief Get whether the physics world extension is available
 *
 * \param opaque The opaque data
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemPhysicsWorldIsAvailable(void *opaque);

/**
 * \brief Create an opaque physics world opaque object
 *
 * \param opaque The opaque data
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \remark alway returns \b NULL when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API nanoem_physics_world_t *APIENTRY
nanoemPhysicsWorldCreate(void *opaque, nanoem_status_t *status);

/**
 * \brief Add the given opaque physics rigid body object to the associated opaque physics world object
 *
 * \param world The opaque physics world object
 * \param rigid_body The opaque physics rigid body object
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldAddRigidBody(nanoem_physics_world_t *world, nanoem_physics_rigid_body_t *rigid_body);

/**
 * \brief Add the given opaque physics soft body object to the associated opaque physics world object
 *
 * \param world The opaque physics world object
 * \param soft_body The opaque physics soft body object
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldAddSoftBody(nanoem_physics_world_t *world, nanoem_physics_soft_body_t *soft_body);

/**
 * \brief Add the given opaque physics joint object to the associated opaque physics world object
 *
 * \param world The opaque physics world object
 * \param joint The opaque physics joint object
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldAddJoint(nanoem_physics_world_t *world, nanoem_physics_joint_t *joint);

/**
 * \brief Remove the given opaque physics rigid body object from the associated opaque physics world object
 *
 * \param world The opaque physics world object
 * \param rigid_body The opaque physics rigid body object
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldRemoveRigidBody(nanoem_physics_world_t *world, nanoem_physics_rigid_body_t *rigid_body);

/**
 * \brief Remove the given opaque physics soft body object from the associated opaque physics world object
 *
 * \param world The opaque physics world object
 * \param soft_body The opaque physics soft body object
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldRemoveSoftBody(nanoem_physics_world_t *world, nanoem_physics_soft_body_t *soft_body);

/**
 * \brief Remove the given opaque physics joint object from the associated opaque physics world object
 *
 * \param world The opaque physics world object
 * \param joint The opaque physics joint object
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldRemoveJoint(nanoem_physics_world_t *world, nanoem_physics_joint_t *joint);

/**
 * \brief Set the FPS value to the given opaque physics world object
 *
 * \param world The opaque physics world object
 * \param value The value to set
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldSetPreferredFPS(nanoem_physics_world_t *world, int value);

/**
 * \brief Step simulation with delta to the opaque physics world object
 *
 * \param world The opaque physics world object
 * \param delta The delta seconds to step simulation
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API int APIENTRY
nanoemPhysicsWorldStepSimulation(nanoem_physics_world_t *world, nanoem_f32_t delta);

/**
 * \brief Reset all physics objects in the opaque physics world object
 *
 * \param world The opaque physics world object
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldReset(nanoem_physics_world_t *world);

/**
 * \brief Get the gravity vector from the given opaque physics world object
 *
 * \param world The opaque physics world object
 * \remark alway returns \b 0.0 when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API const nanoem_f32_t * APIENTRY
nanoemPhysicsWorldGetGravity(const nanoem_physics_world_t *world);

/**
 * \brief Set the gravity vector to the given opaque physics world object
 *
 * \param world The opaque physics world object
 * \param value The value to set
 * \remark \b value must be at least 4 components float array
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldSetGravity(nanoem_physics_world_t *world, const nanoem_f32_t *value);

/**
 * \brief Get the gravity factor from the given opaque physics world object
 *
 * \param world The opaque physics world object
 * \remark alway returns \b 0.0 when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API const nanoem_f32_t * APIENTRY
nanoemPhysicsWorldGetGravityFactor(const nanoem_physics_world_t *world);

/**
 * \brief Set the gravity factor to the given opaque physics world object
 *
 * \param world The opaque physics world object
 * \param value The value to set
 * \remark \b value must be at least 4 components float array
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldSetGravityFactor(nanoem_physics_world_t *world, const nanoem_f32_t *value);

/**
 * \brief Get whether the world is active from the given opaque physics world object
 *
 * \param world The opaque physics world object
 * \remark alway returns \b false when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemPhysicsWorldIsActive(const nanoem_physics_world_t *world);

/**
 * \brief Set whether the world is active to the given opaque physics world object
 *
 * \param world The opaque physics world object
 * \param value The value to set
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldSetActive(nanoem_physics_world_t *world, nanoem_bool_t value);

/**
 * \brief Set the deactivation time threshold to the given opaque physics world object
 *
 * \param world The opaque physics world object
 * \param value The value to set
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldSetDeactivationTimeThreshold(nanoem_physics_world_t *world, nanoem_f32_t value);

/**
 * \brief Get whether the ground is enabled from the given opaque physics world object
 *
 * \param world The opaque physics world object
 * \remark alway returns \b false when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemPhysicsWorldIsGroundEnabled(const nanoem_physics_world_t *world);

/**
 * \brief Set whether the ground is enabled to the given opaque physics world object
 *
 * \param world The opaque physics world object
 * \param value The value to set
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldSetGroundEnabled(nanoem_physics_world_t *world, nanoem_bool_t value);

/**
 * \brief Destroy the given opaque physics world object
 *
 * \param world The opaque physics world object
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldDestroy(nanoem_physics_world_t *world);
/** @} */

/**
 * \defgroup nanoem_physics_rigid_body Physics Rigid Body
 * @{
 */

/**
 * \brief Create an opaque physics rigid body opaque object
 *
 * \param value The value to set
 * \param opaque The opaque data to create an opaque physics rigid body object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \remark alway returns \b NULL when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API nanoem_physics_rigid_body_t *APIENTRY
nanoemPhysicsRigidBodyCreate(const nanoem_model_rigid_body_t *value, void *opaque, nanoem_status_t *status);

/**
 * \brief Get the opaque motion state object from the given opaque physics rigid body object
 *
 * \param rigid_body The opaque physics rigid body object
 * \remark alway returns \b NULL when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API nanoem_physics_motion_state_t *APIENTRY
nanoemPhysicsRigidBodyGetMotionState(const nanoem_physics_rigid_body_t *rigid_body);

/**
 * \brief Get the world transform matrix from the given opaque physics rigid body object
 *
 * \param rigid_body The opaque physics rigid body object
 * \param[out] value The value to set
 * \remark \b value must be at least 16 components float array
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodyGetWorldTransform(const nanoem_physics_rigid_body_t *rigid_body, nanoem_f32_t *value);

/**
 * \brief Set the world transform matrix to the given opaque physics rigid body object
 *
 * \param rigid_body The opaque physics rigid body object
 * \param value The value to set
 * \remark \b value must be at least 16 components float array
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodySetWorldTransform(nanoem_physics_rigid_body_t *rigid_body, const nanoem_f32_t *value);

/**
 * \brief Get whether the visualization is enabled from the given opaque physics rigid body object
 *
 * \param rigid_body The opaque physics rigid body object
 * \remark alway returns \b false when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemPhysicsRigidBodyIsVisualizeEnabled(const nanoem_physics_rigid_body_t *rigid_body);

/**
 * \brief Set whether the visualization is enabled to the given opaque physics rigid body object
 *
 * \param rigid_body The opaque physics rigid body object
 * \param value The value to set
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodySetVisualizeEnabled(nanoem_physics_rigid_body_t *rigid_body, nanoem_bool_t value);

/**
 * \brief Disable deactivation to the given opaque physics rigid body object
 *
 * \param rigid_body The opaque physics rigid body object
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodyDisableDeactivation(nanoem_physics_rigid_body_t *rigid_body);

/**
 * \brief Set whether the rigid body is active to the given opaque physics rigid body object
 *
 * \param rigid_body The opaque physics rigid body object
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodySetActive(nanoem_physics_rigid_body_t *rigid_body);

/**
 * \brief Set whether the rigid body is kinematic to the given opaque physics rigid body object
 *
 * \param rigid_body The opaque physics rigid body object
 * \param value The value to set
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodySetKinematic(nanoem_physics_rigid_body_t* rigid_body, int value);

/**
 * \brief Get whether the rigid body is kinematic from the given opaque physics rigid body object
 *
 * \param rigid_body The opaque physics rigid body object
 * \remark alway returns \b false when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API int APIENTRY
nanoemPhysicsRigidBodyIsKinematic(nanoem_physics_rigid_body_t* rigid_body);

/**
 * \brief Reset linear velocity to the opaque physics rigid body object
 *
 * \param rigid_body The opaque physics rigid body object
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodyResetLinearVelocity(nanoem_physics_rigid_body_t *rigid_body);

/**
 * \brief Reset all rigid body states to the given opaque physics rigid body object
 *
 * \param rigid_body The opaque physics rigid body object
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodyResetStates(nanoem_physics_rigid_body_t *rigid_body);

/**
 * \brief Apply torque impulse vector to the given opaque physics rigid body object
 *
 * \param rigid_body The opaque physics rigid body object
 * \param value The value to set
 * \remark \b value must be at least 4 components float array
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodyApplyTorqueImpulse(nanoem_physics_rigid_body_t *rigid_body, const nanoem_f32_t *value);

/**
 * \brief Apply velocity impulse vector to the given opaque physics rigid body object
 *
 * \param rigid_body The opaque physics rigid body object
 * \param value The value to set
 * \remark \b value must be at least 4 components float array
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodyApplyVelocityImpulse(nanoem_physics_rigid_body_t *rigid_body, const nanoem_f32_t *value);

/**
 * \brief Destroy the given opaque physics rigid body object
 *
 * \param rigid_body The opaque physics rigid body object
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsRigidBodyDestroy(nanoem_physics_rigid_body_t *rigid_body);
/** @} */

/**
 * \defgroup nanoem_physics_motion_state Physics Motion State
 * @{
 */

/**
 * \brief Get the initial world transform matrix from the given opaque physics motion state object
 *
 * \param motion_state The opaque physics motion state object
 * \param[out] value The value to set
 * \remark \b value must be at least 16 components float array
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsMotionStateGetInitialWorldTransform(const nanoem_physics_motion_state_t *motion_state, nanoem_f32_t *value);

/**
 * \brief Get the current world transform matrix from the given opaque physics motion state object
 *
 * \param motion_state The opaque physics motion state object
 * \param[out] value The value to set
 * \remark \b value must be at least 16 components float array
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsMotionStateGetCurrentWorldTransform(const nanoem_physics_motion_state_t *motion_state, nanoem_f32_t *value);

/**
 * \brief Set the current world transform matrix to the given opaque physics motion state object
 *
 * \param motion_state The opaque physics motion state object
 * \param value The value to set
 * \remark \b value must be at least 16 components float array
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsMotionStateSetCurrentWorldTransform(nanoem_physics_motion_state_t *motion_state, const nanoem_f32_t *value);

/**
 * \brief Get the center of mass offset vector from the given opaque physics motion state object
 *
 * \param motion_state The opaque physics motion state object
 * \param[out] value The value to set
 * \remark \b value must be at least 16 components float array
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsMotionStateGetCenterOfMassOffset(const nanoem_physics_motion_state_t *motion_state, nanoem_f32_t *value);

/**
 * \brief Set the center of mass offset vector to the given opaque physics motion state object
 *
 * \param motion_state The opaque physics motion state object
 * \param value The value to set
 * \remark \b value must be at least 16 components float array
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsMotionStateSetCenterOfMassOffset(nanoem_physics_motion_state_t *motion_state, const nanoem_f32_t *value);
/** @} */

/**
 * \defgroup nanoem_physics_joint Physics Joint
 * @{
 */

/**
 * \brief Create an opaque physics joint opaque object
 *
 * \param value The value to set
 * \param opaque The opaque data to create an opaque physics joint object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \remark alway returns \b NULL when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API nanoem_physics_joint_t *APIENTRY
nanoemPhysicsJointCreate(const nanoem_model_joint_t *value, void *opaque, nanoem_status_t *status);

/**
 * \brief Get the calculated transform matrix of the rigid body A from the given opaque physics joint object
 *
 * \param joint The opaque physics joint object
 * \param[out] value The value to set
 * \remark \b value must be at least 16 components float array
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsJointGetCalculatedTransformA(const nanoem_physics_joint_t *joint, nanoem_f32_t *value);

/**
 * \brief Get the calculated transform matrix of the rigid body B from the given opaque physics joint object
 *
 * \param joint The opaque physics joint object
 * \param[out] value The value to set
 * \remark \b value must be at least 16 components float array
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsJointGetCalculatedTransformB(const nanoem_physics_joint_t *joint, nanoem_f32_t *value);

/**
 * \brief Get whether the visualization is enabled from the given opaque physics rigid body object
 *
 * \param joint The opaque physics joint object
 * \remark alway returns \b false when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemPhysicsJointIsVisualizeEnabled(const nanoem_physics_joint_t *joint);

/**
 * \brief Set whether the visualization is enabled to the given opaque physics rigid body object
 *
 * \param joint The opaque physics joint object
 * \param value The value to set
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsJointSetVisualizeEnabled(nanoem_physics_joint_t *joint, nanoem_bool_t value);

/**
 * \brief Destroy the given opaque physics joint object
 *
 * \param joint The opaque physics joint object
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsJointDestroy(nanoem_physics_joint_t *joint);
/** @} */

/**
 * \defgroup nanoem_physics_debug_geometry Physics Debug Geometry
 * @{
 */

/**
 * \brief Get the debug gometry flags from the given opaque physics world object
 *
 * \param world The opaque physics world object
 * \remark alway returns \b 0 when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API nanoem_u32_t APIENTRY
nanoemPhysicsWorldGetDebugGeomtryFlags(const nanoem_physics_world_t *world);

/**
 * \brief Set the debug gometry flags to the given opaque physics world object
 *
 * \param world The opaque physics world object
 * \param value The value to set
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsWorldSetDebugGeomtryFlags(nanoem_physics_world_t *world, nanoem_u32_t value);

/**
 * \brief Get all opaque debug geometry objects from the given opaque physics world object
 *
 * \param world The opaque physics world object
 * \param[out] num_objects Number of all objects in the object
 * \remark alway returns \b NULL when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API nanoem_physics_debug_geometry_t *const *APIENTRY
nanoemPhysicsWorldGetDebugGeomtryObjects(const nanoem_physics_world_t *world, int *num_objects);

/**
 * \brief Get the from-position vector from the given opaque physics debug geometry object
 *
 * \param geometry The opaque physics debug geometry object
 * \remark alway returns null vector when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemPhysicsDebugGeometryGetFromPosition(const nanoem_physics_debug_geometry_t *geometry);

/**
 * \brief Get the to-position vector from the given opaque physics debug geometry object
 *
 * \param geometry The opaque physics debug geometry object
 * \remark alway returns null vector when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemPhysicsDebugGeometryGetToPosition(const nanoem_physics_debug_geometry_t *geometry);

/**
 * \brief Get the color vector from the given opaque physics debug geometry object
 *
 * \param geometry The opaque physics debug geometry object
 * \remark alway returns null vector when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemPhysicsDebugGeometryGetColor(const nanoem_physics_debug_geometry_t *geometry);
/** @} */

/**
 * \defgroup nanoem_physics_rigid_body Physics Rigid Body
 * @{
 */

/**
 * \brief Create an opaque physics soft body opaque object
 *
 * \param value The value to set
 * \param world The opaque physics world object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \remark alway returns \b NULL when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API nanoem_physics_soft_body_t *APIENTRY
nanoemPhysicsSoftBodyCreate(const nanoem_model_soft_body_t *value, nanoem_physics_world_t *world, nanoem_status_t *status);

/**
 * \brief Get number of all opaque model vertex objects from the given opaque physics soft body object
 *
 * \param soft_body The opaque physics soft body object
 * \remark alway returns \b 0 when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API int APIENTRY
nanoemPhysicsSoftBodyGetNumVertexObjects(const nanoem_physics_soft_body_t *soft_body);

/**
 * \brief Get the opaque model vertex object from the given opaque physics soft body object and offset
 *
 * \param soft_body The opaque physics soft body object
 * \param offset The offset to get the opaque model vertex object
 * \remark alway returns \b NULL when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API const nanoem_model_vertex_t * APIENTRY
nanoemPhysicsSoftBodyGetVertexObject(const nanoem_physics_soft_body_t *soft_body, int offset);

/**
 * \brief Get the vertex position vector from the given opaque physics soft body object and offset
 *
 * \param soft_body The opaque physics soft body object
 * \param offset The offset to get the vertex position vector
 * \param value The value to set
 * \remark \b value must be at least 4 components float array
 * \remark alway returns null vector when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsSoftBodyGetVertexPosition(const nanoem_physics_soft_body_t *soft_body, int offset, nanoem_f32_t *value);

/**
 * \brief Get the vertex normal vector from the given opaque physics soft body object and offset
 *
 * \param soft_body The opaque physics soft body object
 * \param offset The offset to get the vertex normal vector
 * \param value The value to set
 * \remark \b value must be at least 4 components float array
 * \remark alway returns null vector when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsSoftBodyGetVertexNormal(const nanoem_physics_soft_body_t *soft_body, int offset, nanoem_f32_t *value);

/**
 * \brief Set the vertex position vector to the given opaque physics soft body object and offset
 *
 * \param soft_body The opaque physics soft body object
 * \param offset The offset to set the vertex position vector
 * \param value The value to set
 * \remark \b value must be at least 4 components float array
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsSoftBodySetVertexPosition(nanoem_physics_soft_body_t *soft_body, int offset, const nanoem_f32_t *value);

/**
 * \brief Set the vertex normal vector to the given opaque physics soft body object and offset
 *
 * \param soft_body The opaque physics soft body object
 * \param offset The offset to set the vertex position vector
 * \param value The value to set
 * \remark \b value must be at least 4 components float array
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsSoftBodySetVertexNormal(nanoem_physics_soft_body_t *soft_body, int offset, const nanoem_f32_t *value);

/**
 * \brief Get whether the visualization is enabled from the given opaque physics soft body object
 *
 * \param soft_body The opaque physics soft body object
 * \remark alway returns \b false when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemPhysicsSoftBodyIsVisualizeEnabled(const nanoem_physics_soft_body_t *soft_body);

/**
 * \brief Set whether the visualization is enabled to the given opaque physics soft body object
 *
 * \param soft_body The opaque physics soft body object
 * \param value The value to set
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsSoftBodySetVisualizeEnabled(nanoem_physics_soft_body_t *soft_body, nanoem_bool_t value);

/**
 * \brief Destroy the given opaque physics soft body object
 *
 * \param soft_body The opaque physics soft body object
 * \remark Do nothing when ::nanoemPhysicsWorldIsAvailable is \b false
 */
NANOEM_DECL_API void APIENTRY
nanoemPhysicsSoftBodyDestroy(nanoem_physics_soft_body_t *soft_body);
/** @} */

/** @} */

/** @} */

#endif /* NANOEM_EXT_PHYSICS_H_ */
