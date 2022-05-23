/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODEL_RIGIDBODY_H_
#define NANOEM_EMAPP_MODEL_RIGIDBODY_H_

#include "emapp/PhysicsEngine.h"

struct par_shapes_mesh_s;
struct nanoem_physics_rigid_body_t;

namespace nanoem {

class PhysicsEngine;

namespace model {

class RigidBody NANOEM_DECL_SEALED : private NonCopyable {
public:
    typedef tinystl::vector<const nanoem_model_rigid_body_t *, TinySTLAllocator> List;
    typedef tinystl::unordered_set<const nanoem_model_rigid_body_t *, TinySTLAllocator> Set;
    typedef tinystl::unordered_map<const nanoem_model_rigid_body_t *, nanoem_physics_rigid_body_t *, TinySTLAllocator>
        Resolver;
    typedef tinystl::vector<nanoem_model_rigid_body_t *, TinySTLAllocator> MutableList;
    typedef tinystl::unordered_set<nanoem_model_rigid_body_t *, TinySTLAllocator> MutableSet;
    struct VisualizationClause {
        VisualizationClause()
            : group(0xffff)
            , shapeType(NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_UNKNOWN)
            , transformType(NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_UNKNOWN)
        {
        }
        nanoem_u32_t group;
        nanoem_model_rigid_body_shape_type_t shapeType;
        nanoem_model_rigid_body_transform_type_t transformType;
    };

    static int index(const nanoem_model_rigid_body_t *rigidBodyPtr) NANOEM_DECL_NOEXCEPT;
    static const char *nameConstString(
        const nanoem_model_rigid_body_t *rigidBodyPtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT;
    static const char *canonicalNameConstString(
        const nanoem_model_rigid_body_t *rigidBodyPtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT;
    static Vector3 colorByShapeType(const nanoem_model_rigid_body_t *rigidBodyPtr) NANOEM_DECL_NOEXCEPT;
    static Vector3 colorByObjectType(const nanoem_model_rigid_body_t *rigidBodyPtr) NANOEM_DECL_NOEXCEPT;
    static RigidBody *cast(const nanoem_model_rigid_body_t *body) NANOEM_DECL_NOEXCEPT;
    static RigidBody *create();
    ~RigidBody() NANOEM_DECL_NOEXCEPT;

    void bind(nanoem_model_rigid_body_t *rigidBodyPtr, PhysicsEngine *engine, bool isMorph, Resolver &resolver);
    void resetLanguage(const nanoem_model_rigid_body_t *rigidBodyPtr, nanoem_unicode_string_factory_t *factory,
        nanoem_language_type_t language);
    void destroy() NANOEM_DECL_NOEXCEPT;

    void getWorldTransform(
        const nanoem_model_rigid_body_t *rigidBodyPtr, nanoem_f32_t *value) const NANOEM_DECL_NOEXCEPT;
    void synchronizeTransformFeedbackFromSimulation(const nanoem_model_rigid_body_t *rigidBodyPtr,
        PhysicsEngine::RigidBodyFollowBoneType followType) NANOEM_DECL_NOEXCEPT;
    void synchronizeTransformFeedbackToSimulation(const nanoem_model_rigid_body_t *body) NANOEM_DECL_NOEXCEPT;
    void applyAllForces(const nanoem_model_rigid_body_t *rigidBodyPtr) NANOEM_DECL_NOEXCEPT;
    void initializeTransformFeedback(const nanoem_model_rigid_body_t *rigidBodyPtr);
    void resetTransformFeedback(const nanoem_model_rigid_body_t *rigidBodyPtr);
    void addGlobalTorqueForce(const Vector3 &value, nanoem_f32_t weight);
    void addGlobalVelocityForce(const Vector3 &value, nanoem_f32_t weight);
    void addLocalTorqueForce(const Vector3 &value, nanoem_f32_t weight);
    void addLocalVelocityForce(const Vector3 &value, nanoem_f32_t weight);
    void markAllForcesReset();
    void enable();
    void disable();
    void enableKinematic();
    void disableKinematic();
    void forceActive();

    String name() const;
    String canonicalName() const;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT;
    const char *canonicalNameConstString() const NANOEM_DECL_NOEXCEPT;
    PhysicsEngine *physicsEngine() const NANOEM_DECL_NOEXCEPT;
    nanoem_physics_rigid_body_t *physicsRigidBody() const NANOEM_DECL_NOEXCEPT;
    Matrix4x4 worldTransform() const NANOEM_DECL_NOEXCEPT;
    Matrix4x4 initialTransform() const NANOEM_DECL_NOEXCEPT;
    bool isKinematic() const NANOEM_DECL_NOEXCEPT;
    bool isEditingMasked() const NANOEM_DECL_NOEXCEPT;
    void setEditingMasked(bool value);

    const par_shapes_mesh_s *sharedShapeMesh(const nanoem_model_rigid_body_t *body);

private:
    struct PlaceHolder { };
    static void destroy(void *opaque, nanoem_model_object_t *object) NANOEM_DECL_NOEXCEPT;
    RigidBody(const PlaceHolder &holder) NANOEM_DECL_NOEXCEPT;

    void getLocalOrientationMatrix(const nanoem_model_rigid_body_t *body, Matrix3x3 &value) const NANOEM_DECL_NOEXCEPT;

    PhysicsEngine *m_physicsEngine;
    nanoem_physics_rigid_body_t *m_physicsRigidBody;
    par_shapes_mesh_s *m_shape;
    tinystl::pair<Vector3, bool> m_globalTorqueForce;
    tinystl::pair<Vector3, bool> m_globalVelocityForce;
    tinystl::pair<Vector3, bool> m_localTorqueForce;
    tinystl::pair<Vector3, bool> m_localVelocityForce;
    String m_name;
    String m_canonicalName;
    nanoem_u32_t m_states;
};

} /* namespace model */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODEL_RIGIDBODY_H_ */
