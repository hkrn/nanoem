/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#ifdef NANOEM_ENABLE_BULLET

#include "./physics.h"

#include "../nanoem_p.h"

#include <stdlib.h>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_XYZW_ONLY
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/euler_angles.hpp"

nanoem_pragma_diagnostics_push();
nanoem_pragma_diagnostics_ignore_clang_gcc("-Wunused-parameter");
nanoem_pragma_diagnostics_ignore_clang_gcc("-Wunused-private-field");
nanoem_pragma_diagnostics_ignore_clang_gcc("-Wunused-variable");
nanoem_pragma_diagnostics_ignore_msvc(4100);
nanoem_pragma_diagnostics_ignore_msvc(4101);
#include "BulletCollision/BroadphaseCollision/btDbvtBroadphase.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "BulletCollision/CollisionShapes/btSphereShape.h"
#include "BulletCollision/CollisionShapes/btStaticPlaneShape.h"
#include "BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h"
#include "BulletDynamics/ConstraintSolver/btConeTwistConstraint.h"
#include "BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h"
#include "BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h"
#include "BulletDynamics/ConstraintSolver/btHingeConstraint.h"
#include "BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h"
#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h"
#include "BulletDynamics/ConstraintSolver/btSliderConstraint.h"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#include "BulletDynamics/Dynamics/btDynamicsWorld.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "BulletSoftBody/btSoftBody.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "LinearMath/btDefaultMotionState.h"
nanoem_pragma_diagnostics_pop();

#ifndef NDEBUG
#include "glm/gtx/string_cast.hpp"
#include <stdio.h>
namespace {
static inline glm::vec3
to_vec3(const btVector3 &value)
{
    return glm::make_vec3(static_cast<const btScalar *>(value));
}
static inline glm::vec4
to_vec4(const btVector3 &value)
{
    return glm::make_vec4(static_cast<const btScalar *>(value));
}
static inline glm::mat4
to_mat4(const btTransform &value)
{
    float m[16];
    value.getOpenGLMatrix(m);
    return glm::make_mat4(m);
}
nanoem_pragma_diagnostics_push();
nanoem_pragma_diagnostics_ignore_clang_gcc("-Wunused-function");
nanoem_pragma_diagnostics_ignore_msvc(4930);
static inline std::string
to_vec3_s(const btVector3 &value)
{
    return glm::to_string(to_vec3(value));
}
static inline std::string
to_vec4_s(const btVector3 &value)
{
    return glm::to_string(to_vec4(value));
}
static inline std::string
to_mat4_s(const btTransform &value)
{
    return glm::to_string(to_mat4(value));
}
nanoem_pragma_diagnostics_pop();
}
#endif /* NDEBUG */

struct nanoem_physics_debug_geometry_t {
    btVector3 from;
    btVector3 to;
    btVector3 color;
};

namespace {

nanoem_pragma_diagnostics_push();
nanoem_pragma_diagnostics_ignore_clang_gcc("-Wunused-function");
nanoem_pragma_diagnostics_ignore_msvc(4930);
KHASH_MAP_INIT_INT64(vertices, int)
KHASH_MAP_INIT_INT64(rigid_bodies, btRigidBody *)
nanoem_pragma_diagnostics_pop();

static const btVector3 kZeroUnit(0, 0, 0);

static inline bool
isNaN(const float v)
{
    return v != v;
}

static inline int
clamped(const int a, const int b, const int c)
{
    return a < b ? b : (a > c ? c : a);
}

static inline btVector3
toVector3(const nanoem_f32_t *v)
{
    return btVector3(v[0], v[1], v[2]);
}

static inline btVector3
convertVector3FromOrigin(const nanoem_f32_t *v)
{
    return btVector3(v[0], v[1], v[2]);
}

static inline btMatrix3x3
convertMatrixFromEulerAngles(const nanoem_f32_t *v)
{
    btScalar x, y, z;
    x = isNaN(v[0]) ? btRadians(90) : v[0];
    y = isNaN(v[1]) ? btRadians(180) : v[1];
    z = isNaN(v[2]) ? btRadians(180) : v[2];
    btMatrix3x3 m;
    m.setIdentity();
    m.setFromOpenGLSubMatrix(glm::value_ptr(glm::eulerAngleYXZ(y, x, z)));
    return m;
}

static inline void
resetRigidBody(btRigidBody *body)
{
    body->setAngularVelocity(kZeroUnit);
    body->setLinearVelocity(kZeroUnit);
    body->setInterpolationAngularVelocity(kZeroUnit);
    body->setInterpolationLinearVelocity(kZeroUnit);
    body->clearForces();
    body->updateInertiaTensor();
}

struct nanoem_physics_joint_opaque_t {
    nanoem_physics_world_t *m_world;
    nanoem_physics_rigid_body_t *m_rigidBodyA;
    nanoem_physics_rigid_body_t *m_rigidBodyB;
    nanoem_f32_t m_transformA[16];
    nanoem_f32_t m_transformB[16];
};

class Debugger : public btIDebugDraw {
public:
    Debugger()
        : m_flags(0)
    {
    }
    ~Debugger()
    {
        clearGeometryData();
    }

    void
    drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
    {
        nanoem_physics_debug_geometry_t *geometry = new nanoem_physics_debug_geometry_t();
        geometry->from = from;
        geometry->to = to;
        geometry->color = color;
        m_geometryData.push_back(geometry);
    }
    void
    drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int /* lifeTime */,
        const btVector3 &color)
    {
        nanoem_physics_debug_geometry_t *geometry = new nanoem_physics_debug_geometry_t();
        geometry->from = PointOnB;
        geometry->to = PointOnB + normalOnB * distance;
        geometry->color = color;
        m_geometryData.push_back(geometry);
    }
    void
    reportErrorWarning(const char *warningString)
    {
        reserveText(warningString);
        m_textLocation = kZeroUnit;
    }
    void
    draw3dText(const btVector3 &location, const char *textString)
    {
        reserveText(textString);
        m_textLocation = location;
    }
    void
    setDebugMode(int debugMode)
    {
        m_flags = nanoem_u32_t(debugMode);
    }
    int
    getDebugMode() const
    {
        return int(m_flags);
    }

    void
    reserveText(const char *text)
    {
        int length = int(strlen(text));
        m_textData.reserve(int(m_textData.size() + length));
        for (int i = 0; i < length; i++) {
            m_textData.push_back(text[i]);
        }
    }
    void
    clearGeometryData()
    {
        for (int i = 0, nobjects = m_geometryData.size(); i < nobjects; i++) {
            delete m_geometryData[i];
        }
        m_geometryData.clear();
        m_textData.clear();
    }

    btAlignedObjectArray<nanoem_physics_debug_geometry_t *> m_geometryData;
    btAlignedObjectArray<char> m_textData;
    btVector3 m_textLocation;
    nanoem_u32_t m_flags;
};

} /* namespace anonymous */

struct nanoem_physics_world_t {
    void
    create()
    {
        m_config = new btSoftBodyRigidBodyCollisionConfiguration();
        m_dispatcher = new btCollisionDispatcher(m_config);
        m_broadphase = new btDbvtBroadphase();
        m_solver = new btSequentialImpulseConstraintSolver();
        m_world = new btSoftRigidDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_config);
        m_groundPlaneShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
        m_groundBoxShape = new btBoxShape(btVector3(50, 50, 50));
        m_groundBoxShape->setMargin(0.5f);
        // m_defaultGroundShapePtr = m_groundPlaneShape;
        m_defaultGroundShapePtr = m_groundBoxShape;
        m_groundCollisionObject = new btCollisionObject();
        if (m_defaultGroundShapePtr == m_groundBoxShape) {
            btTransform transform;
            transform.getBasis().setIdentity();
            transform.setOrigin(btVector3(0, -m_groundBoxShape->getHalfExtentsWithoutMargin().y(), 0));
            m_groundCollisionObject->setWorldTransform(transform);
            m_groundCollisionObject->setInterpolationWorldTransform(transform);
        }
        m_groundCollisionObject->setCollisionShape(m_defaultGroundShapePtr);
        btTransform initialWorldTransform(btTransform::getIdentity());
        m_nullMotionState = new btDefaultMotionState(initialWorldTransform);
        btRigidBody::btRigidBodyConstructionInfo info(0.0f, m_nullMotionState, m_groundPlaneShape);
        m_nullRigidBody = new btRigidBody(info);
        m_gravity = new btVector3(0, -9.8f, 0);
        m_gravityFactor = new btVector3(5, 5, 5);
        m_debugger = new Debugger();
        m_world->setDebugDrawer(m_debugger);
        m_world->setGravity(*m_gravity * *m_gravityFactor);
        m_worldInfo = new btSoftBodyWorldInfo();
        m_worldInfo->air_density = 1.2f;
        m_worldInfo->water_density = 0;
        m_worldInfo->water_offset = 0;
        m_worldInfo->water_normal = kZeroUnit;
        m_worldInfo->m_gravity = m_world->getGravity();
        m_worldInfo->m_broadphase = m_broadphase;
        m_worldInfo->m_dispatcher = m_dispatcher;
        m_worldInfo->m_sparsesdf.Initialize();
        m_fixedTimeStep = 1.0f / 60.0f;
        m_maxSubSteps = INT_MAX;
        m_active = nanoem_false;
        setGroundEnable(nanoem_true);
    }
    int
    stepSimulation(nanoem_f32_t delta)
    {
        int num_steps = 0;
        if (m_active) {
            m_world->stepSimulation(delta, m_maxSubSteps, m_fixedTimeStep);
            if (m_debugger->m_flags != 0) {
                m_debugger->clearGeometryData();
                m_world->debugDrawWorld();
            }
            m_worldInfo->m_sparsesdf.GarbageCollect();
        }
        return num_steps;
    }
    void
    reset()
    {
        const btCollisionObjectArray &objects = m_world->getCollisionObjectArray();
        btOverlappingPairCache *cache = m_world->getPairCache();
        btDispatcher *dispatcher = m_world->getDispatcher();
        for (int i = m_world->getNumCollisionObjects() - 1; i >= 0; i--) {
            btCollisionObject *object = objects[i];
            if (btSoftBody *body = btSoftBody::upcast(object)) {
                for (int i = 0, numNodes = body->m_nodes.size(); i < numNodes; i++) {
                    btSoftBody::Node &node = body->m_nodes[i];
                    const nanoem_model_vertex_t *vertex = static_cast<const nanoem_model_vertex_t *>(node.m_tag);
                    const nanoem_f32_t *origin = nanoemModelVertexGetOrigin(vertex),
                            *normal = nanoemModelVertexGetNormal(vertex);
                    node.m_x = node.m_q = btVector3(origin[0], origin[1], origin[2]);
                    node.m_n = btVector3(normal[0], normal[1], normal[2]);
                }
                cache->cleanProxyFromPairs(body->getBroadphaseHandle(), dispatcher);
            }
            else if (btRigidBody *body = btRigidBody::upcast(object)) {
                resetRigidBody(body);
                cache->cleanProxyFromPairs(body->getBroadphaseHandle(), dispatcher);
            }
        }
        m_world->getBroadphase()->resetPool(dispatcher);
        m_world->getConstraintSolver()->reset();
        m_world->clearForces();
        m_worldInfo->m_sparsesdf.Reset();
    }
    void
    setGravity(const nanoem_f32_t *value)
    {
        const btVector3 v(value[0], value[1], value[2]);
        *m_gravity = v;
        m_world->setGravity(v * *m_gravityFactor);
        m_worldInfo->m_gravity = m_world->getGravity();
    }
    void
    setGravityFactor(const nanoem_f32_t *value)
    {
        const btVector3 v(value[0], value[1], value[2]);
        *m_gravityFactor = v;
    }
    void
    setGroundEnable(nanoem_bool_t value)
    {
        m_world->removeCollisionObject(m_groundCollisionObject);
        if (value) {
            m_world->addCollisionObject(m_groundCollisionObject, static_cast<short>(0x8000), static_cast<short>(0xffff));
        }
        m_groundEnabled = value;
    }
    void
    destroy()
    {
        for (int i = m_world->getNumConstraints() - 1; i >= 0; i--) {
            m_world->removeConstraint(m_world->getConstraint(i));
        }
        for (int i = m_world->getNumCollisionObjects() - 1; i >= 0; i--) {
            m_world->removeCollisionObject(m_world->getCollisionObjectArray()[i]);
        }
        m_worldInfo->m_sparsesdf.Reset();
        m_defaultGroundShapePtr = 0;
        delete m_config;
        m_config = 0;
        delete m_dispatcher;
        m_dispatcher = 0;
        delete m_broadphase;
        m_broadphase = 0;
        delete m_solver;
        m_solver = 0;
        delete m_groundCollisionObject;
        m_groundCollisionObject = 0;
        delete m_groundBoxShape;
        m_groundBoxShape = 0;
        delete m_groundPlaneShape;
        m_groundPlaneShape = 0;
        delete m_world;
        m_world = 0;
        delete m_worldInfo;
        m_worldInfo = 0;
        delete m_nullRigidBody;
        m_nullRigidBody = 0;
        delete m_gravity;
        m_gravity = 0;
        delete m_gravityFactor;
        m_gravityFactor = 0;
        delete m_debugger;
        m_debugger = 0;
    }
    btSoftBodyRigidBodyCollisionConfiguration *m_config;
    btCollisionDispatcher *m_dispatcher;
    btDbvtBroadphase *m_broadphase;
    btSequentialImpulseConstraintSolver *m_solver;
    btSoftRigidDynamicsWorld *m_world;
    btSoftBodyWorldInfo *m_worldInfo;
    btStaticPlaneShape *m_groundPlaneShape;
    btBoxShape *m_groundBoxShape;
    btCollisionShape* m_defaultGroundShapePtr;
    btCollisionObject *m_groundCollisionObject;
    btDefaultMotionState *m_nullMotionState;
    btRigidBody *m_nullRigidBody;
    btVector3 *m_gravity;
    btVector3 *m_gravityFactor;
    Debugger *m_debugger;
    nanoem_f32_t m_fixedTimeStep;
    nanoem_bool_t m_active;
    nanoem_bool_t m_groundEnabled;
    int m_maxSubSteps;
};

struct nanoem_physics_rigid_body_t {
    static btCollisionShape *
    createShape(const nanoem_model_rigid_body_t *value, btVector3 &localInertia, btScalar &mass)
    {
        const btVector3 size(toVector3(nanoemModelRigidBodyGetShapeSize(value)));
        btCollisionShape *shape = 0;
        switch (nanoemModelRigidBodyGetShapeType(value)) {
        case NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_BOX: {
            shape = new btBoxShape(size);
            break;
        }
        case NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_CAPSULE: {
            shape = new btCapsuleShape(size.x(), size.y());
            break;
        }
        case NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_SPHERE: {
            shape = new btSphereShape(size.x());
            break;
        }
        case NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_MAX_ENUM:
        case NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_UNKNOWN:
        default:
            break;
        }
        if (nanoemModelRigidBodyGetTransformType(value) != NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_TO_SIMULATION) {
            mass = nanoemModelRigidBodyGetMass(value);
            if (shape && mass > 0) {
                shape->calculateLocalInertia(mass, localInertia);
            }
        }
        else {
            mass = 0;
            localInertia.setZero();
        }
        return shape;
    }
    static void
    getMotionStateTransform(const nanoem_model_rigid_body_t *value, btTransform &transform)
    {
        const nanoem_f32_t *q = nanoemModelRigidBodyGetOrientation(value);
        transform.setBasis(convertMatrixFromEulerAngles(q));
        const nanoem_f32_t *p = nanoemModelRigidBodyGetOrigin(value);
        transform.setOrigin(convertVector3FromOrigin(p));
        const nanoem_model_bone_t *bone = nanoemModelRigidBodyGetBoneObject(value);
        if (bone && nanoemModelRigidBodyIsBoneRelativePosition(value)) {
            btTransform offset(btMatrix3x3::getIdentity(), convertVector3FromOrigin(nanoemModelBoneGetOrigin(bone)));
            transform = offset * transform;
        }
    }
    void
    create(const nanoem_model_rigid_body_t *value)
    {
        btTransform initialWorldTransform;
        btVector3 localInertia;
        btScalar mass;
        getMotionStateTransform(value, initialWorldTransform);
        m_shape = createShape(value, localInertia, mass);
        m_motionState = new btDefaultMotionState(initialWorldTransform);
        m_motionState->m_userPointer = this;
        btRigidBody::btRigidBodyConstructionInfo info(mass, m_motionState, m_shape, localInertia);
        info.m_angularDamping = nanoemModelRigidBodyGetAngularDamping(value);
        info.m_friction = nanoemModelRigidBodyGetFriction(value);
        info.m_linearDamping = nanoemModelRigidBodyGetLinearDamping(value);
        info.m_restitution = nanoemModelRigidBodyGetRestitution(value);
        info.m_additionalDamping = true;
        m_group = nanoem_u16_t(0x1 << clamped(nanoemModelRigidBodyGetCollisionGroupId(value), 0, 15));
        m_mask = nanoemModelRigidBodyGetCollisionMask(value) & 0xffff;
        m_internalRigidBody = new btRigidBody(info);
        m_internalRigidBody->setUserPointer(this);
        m_internalRigidBody->setSleepingThresholds(0, 0);
        if (nanoemModelRigidBodyGetTransformType(value) == NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_TO_SIMULATION) {
            m_internalRigidBody->setCollisionFlags(m_internalRigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            m_internalRigidBody->setActivationState(DISABLE_DEACTIVATION);
        }
        else {
            m_internalRigidBody->setActivationState(ACTIVE_TAG);
        }
        m_parentRigidBody = value;
        m_parentWorld = NULL;
    }
    nanoem_bool_t
    isVisualizeEnabled() const
    {
#if BT_BULLET_VERSION > 275
        int result = m_internalRigidBody->getCollisionFlags() & btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT;
        return result == 0 ? nanoem_true : nanoem_false;
#else
        return nanoem_false;
#endif
    }
    void
    setVisualizeEnabled(nanoem_bool_t value)
    {
#if BT_BULLET_VERSION > 275
        int flags = m_internalRigidBody->getCollisionFlags();
        if (value) {
            flags &= ~btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT;
        }
        else {
            flags |= btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT;
        }
        m_internalRigidBody->setCollisionFlags(flags);
#else
        (void) value;
#endif
    }
    void
    destroy()
    {
        m_group = 0;
        m_mask = 0;
        m_parentWorld = NULL;
        m_parentRigidBody = NULL;
        delete m_shape;
        m_shape = 0;
        delete m_internalRigidBody;
        m_internalRigidBody = 0;
        delete m_motionState;
        m_motionState = 0;
    }
    const nanoem_model_rigid_body_t *m_parentRigidBody;
    nanoem_physics_world_t *m_parentWorld;
    btRigidBody *m_internalRigidBody;
    btCollisionShape *m_shape;
    btDefaultMotionState *m_motionState;
    nanoem_u16_t m_group;
    nanoem_u16_t m_mask;
};

struct nanoem_physics_joint_t {
    static void
    getTransform(const nanoem_model_joint_t *value, btTransform &transform)
    {
        const nanoem_f32_t *p = nanoemModelJointGetOrigin(value);
        transform.setOrigin(convertVector3FromOrigin(p));
        const nanoem_f32_t *q = nanoemModelJointGetOrientation(value);
        transform.setBasis(convertMatrixFromEulerAngles(q));
    }
    void
    create(const nanoem_model_joint_t *value, nanoem_physics_joint_opaque_t *data)
    {
        btTransform transformA, transformB, skinningTransformA, skinningTransformB, inverseTransformA, inverseTransformB;
        btRigidBody *bodyA = NULL, *bodyB = NULL;
        if (nanoem_physics_rigid_body_t *rigid_body_a = data->m_rigidBodyA) {
            bodyA = rigid_body_a->m_internalRigidBody;
            if (bodyA) {
                inverseTransformA = rigid_body_a->m_motionState->m_startWorldTrans.inverse();
            }
        }
        if (nanoem_physics_rigid_body_t *rigid_body_b = data->m_rigidBodyB) {
            bodyB = rigid_body_b->m_internalRigidBody;
            if (bodyB) {
                inverseTransformB = rigid_body_b->m_motionState->m_startWorldTrans.inverse();
            }
        }
        if (!bodyA) {
            bodyA = data->m_world->m_nullRigidBody;
        }
        if (!bodyB) {
            bodyB = data->m_world->m_nullRigidBody;
        }
        getTransform(value, transformA);
        getTransform(value, transformB);
        skinningTransformA.setFromOpenGLMatrix(data->m_transformA);
        skinningTransformB.setFromOpenGLMatrix(data->m_transformB);
        const btTransform localFrameInA(inverseTransformA * skinningTransformA * transformA);
        const btTransform localFrameInB(inverseTransformB * skinningTransformB * transformB);
        switch (nanoemModelJointGetType(value)) {
        case NANOEM_MODEL_JOINT_TYPE_CONE_TWIST_CONSTRAINT: {
            const nanoem_f32_t *angularLowerLimit = nanoemModelJointGetAngularLowerLimit(value);
            const nanoem_f32_t *linearUpperLimit = nanoemModelJointGetLinearUpperLimit(value);
            const nanoem_f32_t *linearLowerLimit = nanoemModelJointGetLinearLowerLimit(value);
            const nanoem_f32_t *angularStiffness = nanoemModelJointGetAngularStiffness(value);
            const nanoem_f32_t *linearStiffness = nanoemModelJointGetLinearStiffness(value);
            btConeTwistConstraint *coneTwist = new btConeTwistConstraint(*bodyA, *bodyB, localFrameInA, localFrameInB);
            btQuaternion orientation;
            orientation.setEulerZYX(angularStiffness[0], angularStiffness[1], angularStiffness[2]);
            coneTwist->setLimit(angularLowerLimit[2], angularLowerLimit[1], angularLowerLimit[0], linearStiffness[0], linearStiffness[1], linearStiffness[2]);
            coneTwist->setDamping(linearLowerLimit[0]);
            coneTwist->setFixThresh(linearUpperLimit[0]);
            coneTwist->setMaxMotorImpulse(linearUpperLimit[2]);
            coneTwist->setMotorTarget(orientation);
            m_internalConstraint = coneTwist;
            break;
        }
        case NANOEM_MODEL_JOINT_TYPE_GENERIC_6DOF_CONSTRAINT: {
            btGeneric6DofConstraint *dof = new btGeneric6DofConstraint(*bodyA, *bodyB, localFrameInA, localFrameInB, true);
            setupSixDOFConstraint(value, dof);
            m_internalConstraint = dof;
            break;
        }
        case NANOEM_MODEL_JOINT_TYPE_GENERIC_6DOF_SPRING_CONSTRAINT: {
            const nanoem_f32_t *angularStiffness = nanoemModelJointGetAngularStiffness(value);
            const nanoem_f32_t *linearStiffness = nanoemModelJointGetLinearStiffness(value);
            btGeneric6DofSpringConstraint *spring =
                new btGeneric6DofSpringConstraint(*bodyA, *bodyB, localFrameInA, localFrameInB, true);
            setupSixDOFConstraint(value, spring);
            for (int i = 0; i < 3; i++) {
                const nanoem_f32_t value = linearStiffness[i];
                spring->setStiffness(i, value);
                spring->enableSpring(i, !btFuzzyZero(value));
            }
            for (int i = 3; i < 6; i++) {
                const nanoem_f32_t value = angularStiffness[i - 3];
                spring->setStiffness(i, value);
                spring->enableSpring(i, true);
            }
            m_internalConstraint = spring;
            break;
        }
        case NANOEM_MODEL_JOINT_TYPE_HINGE_CONSTRAINT: {
            const nanoem_f32_t *angularUpperLimit = nanoemModelJointGetAngularUpperLimit(value);
            const nanoem_f32_t *angularLowerLimit = nanoemModelJointGetAngularLowerLimit(value);
            const nanoem_f32_t *angularStiffness = nanoemModelJointGetAngularStiffness(value);
            const nanoem_f32_t *linearStiffness = nanoemModelJointGetLinearStiffness(value);
            btHingeConstraint *hinge = new btHingeConstraint(*bodyA, *bodyB, localFrameInA, localFrameInB, true);
            hinge->setLimit(angularLowerLimit[0], angularUpperLimit[0], linearStiffness[0], linearStiffness[1], linearStiffness[2]);
            hinge->enableMotor(angularStiffness[0] > 0);
            hinge->enableAngularMotor(angularStiffness[0] > 0, angularStiffness[1], angularStiffness[2]);
            m_internalConstraint = hinge;
            break;
        }
        case NANOEM_MODEL_JOINT_TYPE_POINT2POINT_CONSTRAINT: {
            btPoint2PointConstraint *p2p = new btPoint2PointConstraint(*bodyA, *bodyB, localFrameInA.getOrigin(), localFrameInB.getOrigin());
            m_internalConstraint = p2p;
            break;
        }
        case NANOEM_MODEL_JOINT_TYPE_SLIDER_CONSTRAINT: {
            const nanoem_f32_t *angularUpperLimit = nanoemModelJointGetAngularUpperLimit(value);
            const nanoem_f32_t *angularLowerLimit = nanoemModelJointGetAngularLowerLimit(value);
            const nanoem_f32_t *linearUpperLimit = nanoemModelJointGetLinearUpperLimit(value);
            const nanoem_f32_t *linearLowerLimit = nanoemModelJointGetLinearLowerLimit(value);
            const nanoem_f32_t *angularStiffness = nanoemModelJointGetAngularStiffness(value);
            const nanoem_f32_t *linearStiffness = nanoemModelJointGetLinearStiffness(value);
            btSliderConstraint *slider = new btSliderConstraint(*bodyA, *bodyB, localFrameInA, localFrameInB, true);
            slider->setLowerLinLimit(linearLowerLimit[0]);
            slider->setUpperLinLimit(linearUpperLimit[0]);
            slider->setLowerAngLimit(angularLowerLimit[0]);
            slider->setUpperAngLimit(angularUpperLimit[0]);
            slider->setPoweredLinMotor(linearStiffness[0] > 0);
            slider->setTargetLinMotorVelocity(linearStiffness[1]);
            slider->setMaxLinMotorForce(linearStiffness[2]);
            slider->setPoweredAngMotor(angularStiffness[0] > 0);
            slider->setTargetAngMotorVelocity(angularStiffness[1]);
            slider->setMaxAngMotorForce(angularStiffness[2]);
            m_internalConstraint = slider;
            break;
        }
        case NANOEM_MODEL_JOINT_TYPE_MAX_ENUM:
        case NANOEM_MODEL_JOINT_TYPE_UNKNOWN:
        default:
            break;
        }
        m_parentJoint = value;
        m_visualized = true;
    }
    void
    getCalculatedTranformA(nanoem_f32_t *value) const
    {
        switch (m_internalConstraint->getConstraintType()) {
        case D6_CONSTRAINT_TYPE: {
            static_cast<btGeneric6DofConstraint *>(m_internalConstraint)->getCalculatedTransformA().getOpenGLMatrix(value);
            break;
        }
        case SLIDER_CONSTRAINT_TYPE: {
            static_cast<btSliderConstraint *>(m_internalConstraint)->getCalculatedTransformA().getOpenGLMatrix(value);
            break;
        }
        default:
            btTransform::getIdentity().getOpenGLMatrix(value);
            break;
        }
    }
    void
    getCalculatedTranformB(nanoem_f32_t *value) const
    {
        switch (m_internalConstraint->getConstraintType()) {
        case D6_CONSTRAINT_TYPE: {
            static_cast<btGeneric6DofConstraint *>(m_internalConstraint)->getCalculatedTransformB().getOpenGLMatrix(value);
            break;
        }
        case SLIDER_CONSTRAINT_TYPE: {
            static_cast<btSliderConstraint *>(m_internalConstraint)->getCalculatedTransformB().getOpenGLMatrix(value);
            break;
        }
        default:
            btTransform::getIdentity().getOpenGLMatrix(value);
            break;
        }
    }
    nanoem_bool_t
    isVisualizeEnabled() const
    {
        return m_visualized ? nanoem_true : nanoem_false;
    }
    void
    setVisualizeEnabled(nanoem_bool_t value)
    {
        m_internalConstraint->setDbgDrawSize(value ? 1.0f : 0.0f);
        m_visualized = value ? true : false;
    }
    void
    destroy()
    {
        m_visualized = false;
        m_parentJoint = NULL;
        delete m_internalConstraint;
        m_internalConstraint = 0;
    }
    void
    setupSixDOFConstraint(const nanoem_model_joint_t *value, btGeneric6DofConstraint *spring)
    {
        const nanoem_f32_t *angularUpperLimit = nanoemModelJointGetAngularUpperLimit(value);
        const nanoem_f32_t *angularLowerLimit = nanoemModelJointGetAngularLowerLimit(value);
        const nanoem_f32_t *linearUpperLimit = nanoemModelJointGetLinearUpperLimit(value);
        const nanoem_f32_t *linearLowerLimit = nanoemModelJointGetLinearLowerLimit(value);
        spring->setAngularUpperLimit(
            btVector3(angularUpperLimit[0], angularUpperLimit[1], angularUpperLimit[2]));
        spring->setAngularLowerLimit(
            btVector3(angularLowerLimit[0], angularLowerLimit[1], angularLowerLimit[2]));
        spring->setLinearUpperLimit(btVector3(linearUpperLimit[0], linearUpperLimit[1], linearUpperLimit[2]));
        spring->setLinearLowerLimit(btVector3(linearLowerLimit[0], linearLowerLimit[1], linearLowerLimit[2]));
    }
    const nanoem_model_joint_t *m_parentJoint;
    btTypedConstraint *m_internalConstraint;
    bool m_visualized;
};

struct nanoem_physics_soft_body_t {
    void
    create(const nanoem_model_soft_body_t *value, nanoem_physics_world_t *world)
    {
        m_group = nanoem_u16_t(0x1 << clamped(nanoemModelSoftBodyGetCollisionGroupId(value), 0, 15));
        m_mask = nanoemModelSoftBodyGetCollisionMask(value) & 0xffff;
        kh_vertices_t *verticesMap = kh_init_vertices();
        nanoem_rsize_t numVertices, numMaterials, numIndices;
        const nanoem_model_t *model = nanoemModelSoftBodyGetParentModel(value);
        nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(model, &numVertices);
        nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(model, &numMaterials);
        const nanoem_u32_t *indices = nanoemModelGetAllVertexIndices(model, &numIndices);
        const nanoem_model_material_t *material = nanoemModelSoftBodyGetMaterialObject(value);
        int offsetVertexIndices = 0, numVertexIndices = nanoemModelMaterialGetNumVertexIndices(material);
        switch (nanoemModelSoftBodyGetShapeType(value)) {
        case NANOEM_MODEL_SOFT_BODY_SHAPE_TYPE_TRI_MESH: {
            m_points = new btAlignedObjectArray<btScalar>;
            m_indices = new btAlignedObjectArray<int>;
            m_points->reserve(numVertexIndices * 3);
            m_indices->reserve(numVertexIndices);
            for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
                const nanoem_model_material_t *item = materials[i];
                if (item == material) {
                    break;
                }
                offsetVertexIndices += nanoemModelMaterialGetNumVertexIndices(item);
            }
            for (int i = 0, offset = 0; i < numVertexIndices; i++) {
                const nanoem_u32_t vertexIndex = indices[i + offsetVertexIndices];
                const nanoem_model_vertex_t *vertex = vertices[vertexIndex];
                khiter_t it;
                int ret;
                it = kh_get_vertices(verticesMap, reinterpret_cast<khint64_t>(vertex));
                if (it != kh_end(verticesMap)) {
                    int index = kh_value(verticesMap, it);
                    m_indices->push_back(index);
                }
                else {
                    it = kh_put_vertices(verticesMap, reinterpret_cast<khint64_t>(vertex), &ret);
                    if (ret >= 0) {
                        const nanoem_f32_t *origin = nanoemModelVertexGetOrigin(vertex);
                        m_indices->push_back(offset);
                        m_points->push_back(origin[0]);
                        m_points->push_back(origin[1]);
                        m_points->push_back(origin[2]);
                        kh_value(verticesMap, it) = offset;
                        offset++;
                    }
                }
            }
            m_internalSoftBody = btSoftBodyHelpers::CreateFromTriMesh(*world->m_worldInfo,
                &m_points->at(0),
                &m_indices->at(0),
                m_indices->size() / 3, false);
            for (khiter_t it = kh_begin(verticesMap), end = kh_end(verticesMap); it != end; ++it) {
                if (kh_exist(verticesMap, it)) {
                    nanoem_model_vertex_t *vertex = reinterpret_cast<nanoem_model_vertex_t *>(kh_key(verticesMap, it));
                    int value = kh_value(verticesMap, it);
                    m_internalSoftBody->m_nodes[value].m_tag = vertex;
                }
            }
            break;
        }
        case NANOEM_MODEL_SOFT_BODY_SHAPE_TYPE_ROPE: {
            m_points = new btAlignedObjectArray<btScalar>;
            m_points->reserve(numVertexIndices * 3);
            for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
                const nanoem_model_material_t *item = materials[i];
                if (item == material) {
                    break;
                }
                offsetVertexIndices += nanoemModelMaterialGetNumVertexIndices(item);
            }
            for (int i = 0, offset = 0; i < numVertexIndices; i++) {
                const nanoem_u32_t vertexIndex = indices[i + offsetVertexIndices];
                const nanoem_model_vertex_t *vertex = vertices[vertexIndex];
                khiter_t it;
                int ret;
                it = kh_get_vertices(verticesMap, reinterpret_cast<khint64_t>(vertex));
                if (it == kh_end(verticesMap)) {
                    it = kh_put_vertices(verticesMap, reinterpret_cast<khint64_t>(vertex), &ret);
                    if (ret >= 0) {
                        const nanoem_f32_t *origin = nanoemModelVertexGetOrigin(vertex);
                        m_points->push_back(origin[0]);
                        m_points->push_back(origin[1]);
                        m_points->push_back(origin[2]);
                        kh_value(verticesMap, it) = offset;
                        offset++;
                    }
                }
            }
            const nanoem_f32_t *fromOrigin = nanoemModelVertexGetOrigin(vertices[0]),
                    *toOrigin = nanoemModelVertexGetOrigin(vertices[numVertices - 1]);
            m_internalSoftBody = btSoftBodyHelpers::CreateRope(*world->m_worldInfo, btVector3(fromOrigin[0], fromOrigin[1], fromOrigin[2]), btVector3(toOrigin[0], toOrigin[1], toOrigin[2]), numVertices - 2, 0);
            for (khiter_t it = kh_begin(verticesMap), end = kh_end(verticesMap); it != end; ++it) {
                if (kh_exist(verticesMap, it)) {
                    nanoem_model_vertex_t *vertex = reinterpret_cast<nanoem_model_vertex_t *>(kh_key(verticesMap, it));
                    int value = kh_value(verticesMap, it), offset = value * 3;
                    btSoftBody::Node &node = m_internalSoftBody->m_nodes[value];
                    node.m_x = btVector3(m_points->at(offset), m_points->at(offset + 1), m_points->at(offset + 2));
                    node.m_tag = vertex;
                }
            }
            break;
        }
        default:
            m_internalSoftBody = 0;
            m_points = 0;
            m_indices = 0;
            break;
        }
        if (m_internalSoftBody) {
            kh_rigid_bodies_t *rigidBodiesMap = kh_init_rigid_bodies();
            btDiscreteDynamicsWorld *physicsWorld = world->m_world;
            const btCollisionObjectArray &objects = physicsWorld->getCollisionObjectArray();
            for (int i = 0, numObjects = physicsWorld->getNumCollisionObjects(); i < numObjects; i++) {
                if (btRigidBody *body = btRigidBody::upcast(objects[i])) {
                    if (nanoem_physics_rigid_body_t *rb = static_cast<nanoem_physics_rigid_body_t *>(body->getUserPointer())) {
                        int ret;
                        khiter_t it = kh_put_rigid_bodies(rigidBodiesMap, reinterpret_cast<khint64_t>(rb->m_parentRigidBody), &ret);
                        if (ret >= 0) {
                            kh_value(rigidBodiesMap, it) = body;
                        }
                    }
                }
            }
            m_internalSoftBody->setTotalMass(nanoemModelSoftBodyGetTotalMass(value));
            m_internalSoftBody->getCollisionShape()->setMargin(nanoemModelSoftBodyGetCollisionMargin(value));
            m_internalSoftBody->setCollisionFlags(m_internalSoftBody->getCollisionFlags() | btSoftBody::fCollision::VF_SS);
            switch (nanoemModelSoftBodyGetAeroModel(value)) {
            case NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_VERTEX_POINT: {
                m_internalSoftBody->m_cfg.aeromodel = btSoftBody::eAeroModel::V_Point;
                break;
            }
            case NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_VERTEX_TWO_SIDED: {
                m_internalSoftBody->m_cfg.aeromodel = btSoftBody::eAeroModel::V_TwoSided;
                break;
            }
            case NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_VERTEX_ONE_SIDED: {
                m_internalSoftBody->m_cfg.aeromodel = btSoftBody::eAeroModel::V_OneSided;
                break;
            }
            case NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_FACE_TWO_SIDED: {
                m_internalSoftBody->m_cfg.aeromodel = btSoftBody::eAeroModel::F_TwoSided;
                break;
            }
            case NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_FACE_ONE_SIDED: {
                m_internalSoftBody->m_cfg.aeromodel = btSoftBody::eAeroModel::F_OneSided;
                break;
            }
            default:
                break;
            }
            btSoftBody::Config &config = m_internalSoftBody->m_cfg;
            config.kVCF = nanoemModelSoftBodyGetVelocityCorrectionFactor(value);
            config.kDP = glm::clamp(nanoemModelSoftBodyGetDampingCoefficient(value), 0.0f, 1.0f);
            config.kDG = glm::max(nanoemModelSoftBodyGetDragCoefficient(value), 0.0f);
            config.kLF = glm::max(nanoemModelSoftBodyGetLiftCoefficient(value), 0.0f);
            config.kPR = nanoemModelSoftBodyGetPressureCoefficient(value);
            config.kVC = glm::max(nanoemModelSoftBodyGetVolumeConversationCoefficient(value), 0.0f);
            config.kDF = glm::clamp(nanoemModelSoftBodyGetDynamicFrictionCoefficient(value), 0.0f, 1.0f);
            config.kMT = glm::clamp(nanoemModelSoftBodyGetPoseMatchingCoefficient(value), 0.0f, 1.0f);
            config.kCHR = glm::clamp(nanoemModelSoftBodyGetRigidContactHardness(value), 0.0f, 1.0f);
            config.kKHR = glm::clamp(nanoemModelSoftBodyGetKineticContactHardness(value), 0.0f, 1.0f);
            config.kAHR = glm::clamp(nanoemModelSoftBodyGetAnchorHardness(value), 0.0f, 1.0f);
            config.kSRHR_CL = glm::clamp(nanoemModelSoftBodyGetSoftVSRigidHardness(value), 0.0f, 1.0f);
            config.kSKHR_CL = glm::clamp(nanoemModelSoftBodyGetSoftVSKineticHardness(value), 0.0f, 1.0f);
            config.kSSHR_CL = glm::clamp(nanoemModelSoftBodyGetSoftVSSoftHardness(value), 0.0f, 1.0f);
            config.kSR_SPLT_CL = glm::clamp(nanoemModelSoftBodyGetSoftVSRigidImpulseSplit(value), 0.0f, 1.0f);
            config.kSK_SPLT_CL = glm::clamp(nanoemModelSoftBodyGetSoftVSKineticImpulseSplit(value), 0.0f, 1.0f);
            config.kSS_SPLT_CL = glm::clamp(nanoemModelSoftBodyGetSoftVSSoftImpulseSplit(value), 0.0f, 1.0f);
            config.viterations = nanoemModelSoftBodyGetVelocitySolverIterations(value);
            config.piterations = nanoemModelSoftBodyGetPositionsSolverIterations(value);
            config.diterations = nanoemModelSoftBodyGetDriftSolverIterations(value);
            config.citerations = nanoemModelSoftBodyGetClusterSolverIterations(value);
            btSoftBody::Material *material = m_internalSoftBody->appendMaterial();
            if (nanoemModelSoftBodyIsBendingConstraintsEnabled(value)) {
                m_internalSoftBody->generateBendingConstraints(nanoemModelSoftBodyGetBendingConstraintsDistance(value), material);
            }
            material->m_kAST = glm::clamp(nanoemModelSoftBodyGetAngularStiffnessCoefficient(value), 0.0f, 1.0f);
            material->m_kLST = glm::clamp(nanoemModelSoftBodyGetLinearStiffnessCoefficient(value), 0.0f, 1.0f);
            material->m_kVST = glm::clamp(nanoemModelSoftBodyGetVolumeStiffnessCoefficient(value), 0.0f, 1.0f);
            if (nanoemModelSoftBodyIsClustersEnabled(value)) {
                m_internalSoftBody->generateClusters(nanoemModelSoftBodyGetClusterCount(value));
                m_internalSoftBody->setCollisionFlags(btSoftBody::fCollision::CL_SS | btSoftBody::fCollision::CL_RS);
            }
            if (nanoemModelSoftBodyIsRandomizeConstraintsNeeded(value)) {
                m_internalSoftBody->randomizeConstraints();
            }
            nanoem_rsize_t numAnchors;
            nanoem_model_soft_body_anchor_t *const *anchors = nanoemModelSoftBodyGetAllAnchorObjects(value, &numAnchors);
            for (nanoem_rsize_t i = 0; i < numAnchors; i++) {
                const nanoem_model_soft_body_anchor_t *anchor = anchors[i];
                const nanoem_model_rigid_body_t *body = nanoemModelSoftBodyAnchorGetRigidBodyObject(anchor);
                const nanoem_model_vertex_t *vertex = nanoemModelSoftBodyAnchorGetVertexObject(anchor);
                khiter_t it = kh_get_vertices(verticesMap, reinterpret_cast<khint64_t>(vertex));
                int nodeIndex = kh_exist(verticesMap, it) ? kh_value(verticesMap, it) : -1;
                khiter_t it2 = kh_get_rigid_bodies(rigidBodiesMap, reinterpret_cast<khint64_t>(body));
                btRigidBody *rigidBody = kh_exist(rigidBodiesMap, it2) ? kh_value(rigidBodiesMap, it2) : 0;
                if (nodeIndex != -1 && rigidBody) {
                    m_internalSoftBody->appendAnchor(nodeIndex, rigidBody);
                }
            }
            nanoem_rsize_t numPinnedVertices, numNodes = m_internalSoftBody->m_nodes.size();
            const nanoem_u32_t *pinnedVertexIndices = nanoemModelSoftBodyGetAllPinnedVertexIndices(value, &numPinnedVertices);
            for (nanoem_rsize_t i = 0; i < numPinnedVertices; i++) {
                const nanoem_u32_t vertexIndex = pinnedVertexIndices[i];
                if (vertexIndex < numNodes) {
                    m_internalSoftBody->setMass(vertexIndex, 0);
                }
            }
            m_internalSoftBody->setUserPointer(this);
            kh_destroy_rigid_bodies(rigidBodiesMap);
        }
        kh_destroy_vertices(verticesMap);
        m_parentSoftBody = value;
    }
    int
    verticesLength() const
    {
        return nanoem_likely(m_internalSoftBody) ? m_internalSoftBody->m_nodes.size() : 0;
    }
    const nanoem_model_vertex_t *
    vertexObject(int offset) const
    {
        return nanoem_likely(offset < verticesLength()) ? static_cast<const nanoem_model_vertex_t *>(m_internalSoftBody->m_nodes[offset].m_tag) : NULL;
    }
    void
    getVertexPosition(int offset, nanoem_f32_t *value) const
    {
        if (nanoem_likely(offset >= 0 && offset < m_internalSoftBody->m_nodes.size())) {
            memcpy(value, m_internalSoftBody->m_nodes[offset].m_x, sizeof(btVector3));
        }
    }
    void
    getVertexNormal(int offset, nanoem_f32_t *value) const
    {
        if (nanoem_likely(offset >= 0 && offset < m_internalSoftBody->m_nodes.size())) {
            memcpy(value, m_internalSoftBody->m_nodes[offset].m_n, sizeof(btVector3));
        }
    }
    void
    setVertexPosition(int offset, const nanoem_f32_t *value) const
    {
        if (nanoem_likely(offset >= 0 && offset < m_internalSoftBody->m_nodes.size())) {
            memcpy(m_internalSoftBody->m_nodes[offset].m_x, value, sizeof(btVector3));
        }
    }
    void
    setVertexNormal(int offset, const nanoem_f32_t *value) const
    {
        if (nanoem_likely(offset >= 0 && offset < m_internalSoftBody->m_nodes.size())) {
            memcpy(m_internalSoftBody->m_nodes[offset].m_n, value, sizeof(btVector3));
        }
    }
    nanoem_bool_t
    isVisualizeEnabled() const
    {
#if BT_BULLET_VERSION > 275
        int result = m_internalSoftBody->getCollisionFlags() & btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT;
        return result == 0 ? nanoem_true : nanoem_false;
#else
        return nanoem_true;
#endif
    }
    void
    setVisualizeEnabled(nanoem_bool_t value)
    {
#if BT_BULLET_VERSION > 275
        int flags = m_internalSoftBody->getCollisionFlags();
        if (value) {
            flags &= ~btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT;
        }
        else {
            flags |= btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT;
        }
        m_internalSoftBody->setCollisionFlags(flags);
#else
        (void) value;
#endif
    }
    void
    destroy()
    {
        delete m_points;
        m_points = 0;
        delete m_indices;
        m_indices = 0;
        delete m_internalSoftBody;
        m_internalSoftBody = 0;
        m_parentSoftBody = NULL;
    }
    const nanoem_model_soft_body_t *m_parentSoftBody;
    btSoftBody *m_internalSoftBody;
    btAlignedObjectArray<btScalar> *m_points;
    btAlignedObjectArray<int> *m_indices;
    nanoem_u16_t m_group;
    nanoem_u16_t m_mask;
};

nanoem_bool_t APIENTRY
nanoemPhysicsWorldIsAvailable(void * /* opaque */)
{
    return nanoem_true;
}

nanoem_physics_world_t *APIENTRY
nanoemPhysicsWorldCreate(void * /* opaque */, nanoem_status_t *status)
{
    nanoem_physics_world_t *world;
    world = static_cast<nanoem_physics_world_t *>(nanoem_calloc(1, sizeof(*world), status));
    if (nanoem_is_not_null(world)) {
        nanoemPhysicsWorldSetDeactivationTimeThreshold(world, 30.0f);
        world->create();
    }
    return world;
}

void APIENTRY
nanoemPhysicsWorldAddRigidBody(nanoem_physics_world_t *world, nanoem_physics_rigid_body_t *rigid_body)
{
    if (nanoem_is_not_null(world) && nanoem_is_not_null(rigid_body)) {
        world->m_world->addRigidBody(
            rigid_body->m_internalRigidBody, static_cast<short>(rigid_body->m_group), static_cast<short>(rigid_body->m_mask));
        rigid_body->m_parentWorld = world;
    }
}

void APIENTRY
nanoemPhysicsWorldAddSoftBody(nanoem_physics_world_t *world, nanoem_physics_soft_body_t *soft_body)
{
    if (nanoem_is_not_null(world) && nanoem_is_not_null(soft_body)) {
        world->m_world->addSoftBody(
            soft_body->m_internalSoftBody, static_cast<short>(soft_body->m_group), static_cast<short>(soft_body->m_mask));
    }
}

void APIENTRY
nanoemPhysicsWorldAddJoint(nanoem_physics_world_t *world, nanoem_physics_joint_t *joint)
{
    if (nanoem_is_not_null(world) && nanoem_is_not_null(joint) && nanoem_is_not_null(joint->m_internalConstraint)) {
        world->m_world->addConstraint(joint->m_internalConstraint);
    }
}

void APIENTRY
nanoemPhysicsWorldRemoveRigidBody(nanoem_physics_world_t *world, nanoem_physics_rigid_body_t *rigid_body)
{
    if (nanoem_is_not_null(world) && nanoem_is_not_null(rigid_body)) {
        world->m_world->removeRigidBody(rigid_body->m_internalRigidBody);
        rigid_body->m_parentWorld = 0;
    }
}

void APIENTRY
nanoemPhysicsWorldRemoveSoftBody(nanoem_physics_world_t *world, nanoem_physics_soft_body_t *soft_body)
{
    if (nanoem_is_not_null(world) && nanoem_is_not_null(soft_body)) {
        world->m_world->removeSoftBody(soft_body->m_internalSoftBody);
    }
}

void APIENTRY
nanoemPhysicsWorldRemoveJoint(nanoem_physics_world_t *world, nanoem_physics_joint_t *joint)
{
    if (nanoem_is_not_null(world) && nanoem_is_not_null(joint) && nanoem_is_not_null(joint->m_internalConstraint)) {
        world->m_world->removeConstraint(joint->m_internalConstraint);
    }
}

void APIENTRY
nanoemPhysicsWorldSetPreferredFPS(nanoem_physics_world_t *world, int value)
{
    world->m_fixedTimeStep = 1.0f / value;
}

int APIENTRY
nanoemPhysicsWorldStepSimulation(nanoem_physics_world_t *world, nanoem_f32_t delta)
{
    int num_steps = 0;
    if (nanoem_is_not_null(world)) {
        num_steps = world->stepSimulation(delta);
    }
    return num_steps;
}

void APIENTRY
nanoemPhysicsWorldReset(nanoem_physics_world_t *world)
{
    if (nanoem_is_not_null(world)) {
        world->reset();
    }
}

const nanoem_f32_t *APIENTRY
nanoemPhysicsWorldGetGravity(const nanoem_physics_world_t *world)
{
    return static_cast<const nanoem_f32_t *>(nanoem_is_not_null(world) ? *world->m_gravity : kZeroUnit);
}

void APIENTRY
nanoemPhysicsWorldSetGravity(nanoem_physics_world_t *world, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(world) && nanoem_is_not_null(value)) {
        world->setGravity(value);
    }
}

const nanoem_f32_t *APIENTRY
nanoemPhysicsWorldGetGravityFactor(const nanoem_physics_world_t *world)
{
    return static_cast<const nanoem_f32_t *>(nanoem_is_not_null(world) ? *world->m_gravityFactor : kZeroUnit);
}

void APIENTRY
nanoemPhysicsWorldSetGravityFactor(nanoem_physics_world_t *world, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(world) && nanoem_is_not_null(value)) {
        world->setGravityFactor(value);
    }
}

nanoem_bool_t APIENTRY
nanoemPhysicsWorldIsActive(const nanoem_physics_world_t *world)
{
    return nanoem_is_not_null(world) ? world->m_active : nanoem_false;
}

void APIENTRY
nanoemPhysicsWorldSetActive(nanoem_physics_world_t *world, nanoem_bool_t value)
{
    if (nanoem_is_not_null(world)) {
        nanoemPhysicsWorldReset(world);
        world->m_active = value;
    }
}

void APIENTRY
nanoemPhysicsWorldSetDeactivationTimeThreshold(nanoem_physics_world_t * /* world */, nanoem_f32_t value)
{
    gDeactivationTime = value;
}

nanoem_bool_t APIENTRY
nanoemPhysicsWorldIsGroundEnabled(const nanoem_physics_world_t *world)
{
    return nanoem_is_not_null(world) ? world->m_groundEnabled : nanoem_false;
}

void APIENTRY
nanoemPhysicsWorldSetGroundEnabled(nanoem_physics_world_t *world, nanoem_bool_t value)
{
    if (nanoem_is_not_null(world)) {
        world->setGroundEnable(value);
    }
}

void APIENTRY
nanoemPhysicsWorldDestroy(nanoem_physics_world_t *world)
{
    if (nanoem_is_not_null(world)) {
        world->destroy();
        nanoem_free(world);
    }
}

nanoem_physics_rigid_body_t *APIENTRY
nanoemPhysicsRigidBodyCreate(const nanoem_model_rigid_body_t *value, void * /* opaque */, nanoem_status_t *status)
{
    nanoem_physics_rigid_body_t *rigid_body;
    rigid_body = static_cast<nanoem_physics_rigid_body_t *>(nanoem_calloc(1, sizeof(*rigid_body), status));
    if (nanoem_is_not_null(rigid_body)) {
        rigid_body->create(value);
    }
    return rigid_body;
}

nanoem_physics_motion_state_t *APIENTRY
nanoemPhysicsRigidBodyGetMotionState(const nanoem_physics_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? reinterpret_cast<nanoem_physics_motion_state_t *>(rigid_body->m_motionState)
                                          : NULL;
}

void APIENTRY
nanoemPhysicsRigidBodyGetWorldTransform(const nanoem_physics_rigid_body_t *rigid_body, nanoem_f32_t *value)
{
    if (nanoem_is_not_null(rigid_body) && nanoem_is_not_null(value)) {
        rigid_body->m_internalRigidBody->getWorldTransform().getOpenGLMatrix(value);
    }
}

void APIENTRY
nanoemPhysicsRigidBodySetWorldTransform(nanoem_physics_rigid_body_t *rigid_body, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(rigid_body) && nanoem_is_not_null(value)) {
        btTransform transform;
        transform.setFromOpenGLMatrix(value);
        btRigidBody *body = rigid_body->m_internalRigidBody;
        body->setWorldTransform(transform);
        body->setInterpolationWorldTransform(transform);
        rigid_body->m_motionState->setWorldTransform(transform);
    }
}

nanoem_bool_t APIENTRY
nanoemPhysicsRigidBodyIsVisualizeEnabled(const nanoem_physics_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? rigid_body->isVisualizeEnabled() : nanoem_false;
}

void APIENTRY
nanoemPhysicsRigidBodySetVisualizeEnabled(nanoem_physics_rigid_body_t *rigid_body, nanoem_bool_t value)
{
    if (nanoem_is_not_null(rigid_body)) {
        rigid_body->setVisualizeEnabled(value);
    }
}

void APIENTRY
nanoemPhysicsRigidBodyDisableDeactivation(nanoem_physics_rigid_body_t *rigid_body)
{
    if (nanoem_is_not_null(rigid_body)) {
        rigid_body->m_internalRigidBody->forceActivationState(DISABLE_DEACTIVATION);
    }
}

void APIENTRY
nanoemPhysicsRigidBodySetActive(nanoem_physics_rigid_body_t *rigid_body)
{
    if (nanoem_is_not_null(rigid_body)) {
        nanoem_model_rigid_body_transform_type_t type = nanoemModelRigidBodyGetTransformType(rigid_body->m_parentRigidBody);
        if (type != NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_TO_SIMULATION) {
            rigid_body->m_internalRigidBody->activate();
        }
    }
}

void APIENTRY
nanoemPhysicsRigidBodySetKinematic(nanoem_physics_rigid_body_t* rigid_body, int value)
{
    if (nanoem_is_not_null(rigid_body)) {
        nanoem_model_rigid_body_transform_type_t type = nanoemModelRigidBodyGetTransformType(rigid_body->m_parentRigidBody);
        if (type != NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_TO_SIMULATION) {
            btRigidBody *body = rigid_body->m_internalRigidBody;
            if (value) {
                body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
                body->setActivationState(DISABLE_DEACTIVATION);
                resetRigidBody(body);
            }
            else {
                body->setCollisionFlags(body->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
                body->setActivationState(ACTIVE_TAG);
            }
        }
    }
}

int APIENTRY
nanoemPhysicsRigidBodyIsKinematic(nanoem_physics_rigid_body_t* rigid_body)
{
    int result = 1;
    if (nanoem_is_not_null(rigid_body)) {
        nanoem_model_rigid_body_transform_type_t type = nanoemModelRigidBodyGetTransformType(rigid_body->m_parentRigidBody);
        if (type != NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_TO_SIMULATION) {
            btRigidBody* body = rigid_body->m_internalRigidBody;
            result = (body->getCollisionFlags() & btCollisionObject::CF_KINEMATIC_OBJECT) != 0;
        }
    }
    return result;
}

void APIENTRY
nanoemPhysicsRigidBodyResetLinearVelocity(nanoem_physics_rigid_body_t *rigid_body)
{
    if (nanoem_is_not_null(rigid_body)) {
        btRigidBody *body = rigid_body->m_internalRigidBody;
        body->setLinearVelocity(kZeroUnit);
        body->setInterpolationLinearVelocity(kZeroUnit);
    }
}

void APIENTRY
nanoemPhysicsRigidBodyResetStates(nanoem_physics_rigid_body_t *rigid_body)
{
    if (nanoem_is_not_null(rigid_body)) {
        resetRigidBody(rigid_body->m_internalRigidBody);
    }
}

void APIENTRY
nanoemPhysicsRigidBodyApplyTorqueImpulse(nanoem_physics_rigid_body_t *rigid_body, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(rigid_body)) {
        rigid_body->m_internalRigidBody->applyTorqueImpulse(toVector3(value));
    }
}

void APIENTRY
nanoemPhysicsRigidBodyApplyVelocityImpulse(nanoem_physics_rigid_body_t *rigid_body, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(rigid_body)) {
        rigid_body->m_internalRigidBody->applyCentralForce(toVector3(value));
    }
}

void APIENTRY
nanoemPhysicsRigidBodyDestroy(nanoem_physics_rigid_body_t *rigid_body)
{
    if (nanoem_is_not_null(rigid_body)) {
        rigid_body->destroy();
        nanoem_free(rigid_body);
    }
}

void APIENTRY
nanoemPhysicsMotionStateGetInitialWorldTransform(const nanoem_physics_motion_state_t *motion_state, nanoem_f32_t *value)
{
    if (nanoem_is_not_null(motion_state) && nanoem_is_not_null(value)) {
        const btDefaultMotionState *state = reinterpret_cast<const btDefaultMotionState *>(motion_state);
        state->m_startWorldTrans.getOpenGLMatrix(value);
    }
}

void APIENTRY
nanoemPhysicsMotionStateGetCurrentWorldTransform(const nanoem_physics_motion_state_t *motion_state, nanoem_f32_t *value)
{
    if (nanoem_is_not_null(motion_state) && nanoem_is_not_null(value)) {
        const btDefaultMotionState *state = reinterpret_cast<const btDefaultMotionState *>(motion_state);
        state->m_graphicsWorldTrans.getOpenGLMatrix(value);
    }
}

void APIENTRY
nanoemPhysicsMotionStateSetCurrentWorldTransform(nanoem_physics_motion_state_t *motion_state, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(motion_state) && nanoem_is_not_null(value)) {
        btDefaultMotionState *state = reinterpret_cast<btDefaultMotionState *>(motion_state);
        state->m_graphicsWorldTrans.setFromOpenGLMatrix(value);
    }
}

void APIENTRY
nanoemPhysicsMotionStateGetCenterOfMassOffset(const nanoem_physics_motion_state_t *motion_state, nanoem_f32_t *value)
{
    if (nanoem_is_not_null(motion_state) && nanoem_is_not_null(value)) {
        const btDefaultMotionState *state = reinterpret_cast<const btDefaultMotionState *>(motion_state);
        state->m_centerOfMassOffset.getOpenGLMatrix(value);
    }
}

void APIENTRY
nanoemPhysicsMotionStateSetCenterOfMassOffset(nanoem_physics_motion_state_t *motion_state, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(motion_state) && nanoem_is_not_null(value)) {
        btDefaultMotionState *state = reinterpret_cast<btDefaultMotionState *>(motion_state);
        state->m_centerOfMassOffset.setFromOpenGLMatrix(value);
    }
}

nanoem_physics_joint_t *APIENTRY
nanoemPhysicsJointCreate(const nanoem_model_joint_t *value, void *opaque, nanoem_status_t *status)
{
    nanoem_physics_joint_t *joint;
    joint = static_cast<nanoem_physics_joint_t *>(nanoem_calloc(1, sizeof(*joint), status));
    if (nanoem_is_not_null(joint)) {
        nanoem_physics_joint_opaque_t *data = static_cast<nanoem_physics_joint_opaque_t *>(opaque);
        joint->create(value, data);
    }
    return joint;
}

void APIENTRY
nanoemPhysicsJointGetCalculatedTransformA(const nanoem_physics_joint_t *joint, nanoem_f32_t *value)
{
    if (nanoem_is_not_null(joint)) {
        joint->getCalculatedTranformA(value);
    }
}

void APIENTRY
nanoemPhysicsJointGetCalculatedTransformB(const nanoem_physics_joint_t *joint, nanoem_f32_t *value)
{
    if (nanoem_is_not_null(joint)) {
        joint->getCalculatedTranformB(value);
    }
}

nanoem_bool_t APIENTRY
nanoemPhysicsJointIsVisualizeEnabled(const nanoem_physics_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? joint->isVisualizeEnabled() : nanoem_false;
}

void APIENTRY
nanoemPhysicsJointSetVisualizeEnabled(nanoem_physics_joint_t *joint, nanoem_bool_t value)
{
    if (nanoem_is_not_null(joint)) {
        joint->setVisualizeEnabled(value);
    }
}

void APIENTRY
nanoemPhysicsJointDestroy(nanoem_physics_joint_t *joint)
{
    if (nanoem_is_not_null(joint)) {
        joint->destroy();
        nanoem_free(joint);
    }
}

nanoem_u32_t APIENTRY
nanoemPhysicsWorldGetDebugGeomtryFlags(const nanoem_physics_world_t *world)
{
    return nanoem_is_not_null(world) ? world->m_debugger->getDebugMode() : 0;
}

void APIENTRY
nanoemPhysicsWorldSetDebugGeomtryFlags(nanoem_physics_world_t *world, nanoem_u32_t value)
{
    if (nanoem_is_not_null(world)) {
        world->m_debugger->clearGeometryData();
        world->m_debugger->setDebugMode(value);
    }
}

nanoem_physics_debug_geometry_t *const *APIENTRY
nanoemPhysicsWorldGetDebugGeomtryObjects(const nanoem_physics_world_t *world, int *num_objects)
{
    nanoem_physics_debug_geometry_t *const *geometries = NULL;
    if (nanoem_is_not_null(world)) {
        *num_objects = world->m_debugger->m_geometryData.size();
        geometries = *num_objects > 0 ? &world->m_debugger->m_geometryData[0] : NULL;
    }
    else {
        *num_objects = 0;
    }
    return geometries;
}

const nanoem_f32_t *APIENTRY
nanoemPhysicsDebugGeometryGetFromPosition(const nanoem_physics_debug_geometry_t *geometry)
{
    return nanoem_is_not_null(geometry) ? static_cast<const nanoem_f32_t *>(geometry->from) : NULL;
}

const nanoem_f32_t *APIENTRY
nanoemPhysicsDebugGeometryGetToPosition(const nanoem_physics_debug_geometry_t *geometry)
{
    return nanoem_is_not_null(geometry) ? static_cast<const nanoem_f32_t *>(geometry->to) : NULL;
}

const nanoem_f32_t *APIENTRY
nanoemPhysicsDebugGeometryGetColor(const nanoem_physics_debug_geometry_t *geometry)
{
    return nanoem_is_not_null(geometry) ? static_cast<const nanoem_f32_t *>(geometry->color) : NULL;
}

nanoem_physics_soft_body_t *APIENTRY
nanoemPhysicsSoftBodyCreate(const nanoem_model_soft_body_t *value, nanoem_physics_world_t *world, nanoem_status_t *status)
{
    nanoem_physics_soft_body_t *soft_body;
    soft_body = static_cast<nanoem_physics_soft_body_t *>(nanoem_calloc(1, sizeof(*soft_body), status));
    if (nanoem_is_not_null(soft_body)) {
        soft_body->create(value, world);
    }
    return soft_body;
}

int APIENTRY
nanoemPhysicsSoftBodyGetNumVertexObjects(const nanoem_physics_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->verticesLength() : 0;
}

const nanoem_model_vertex_t * APIENTRY
nanoemPhysicsSoftBodyGetVertexObject(const nanoem_physics_soft_body_t *soft_body, int offset)
{
    return nanoem_is_not_null(soft_body) ? soft_body->vertexObject(offset) : NULL;
}

void APIENTRY
nanoemPhysicsSoftBodyGetVertexPosition(const nanoem_physics_soft_body_t *soft_body, int offset, nanoem_f32_t *value)
{
    if (nanoem_is_not_null(soft_body)) {
        soft_body->getVertexPosition(offset, value);
    }
}

void APIENTRY
nanoemPhysicsSoftBodyGetVertexNormal(const nanoem_physics_soft_body_t *soft_body, int offset, nanoem_f32_t *value)
{
    if (nanoem_is_not_null(soft_body)) {
        soft_body->getVertexNormal(offset, value);
    }
}

void APIENTRY
nanoemPhysicsSoftBodySetVertexPosition(nanoem_physics_soft_body_t *soft_body, int offset, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(soft_body)) {
        soft_body->setVertexPosition(offset, value);
    }
}

void APIENTRY
nanoemPhysicsSoftBodySetVertexNormal(nanoem_physics_soft_body_t *soft_body, int offset, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(soft_body)) {
        soft_body->setVertexNormal(offset, value);
    }
}

nanoem_bool_t APIENTRY
nanoemPhysicsSoftBodyIsVisualizeEnabled(const nanoem_physics_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->isVisualizeEnabled() : nanoem_false;
}

void APIENTRY
nanoemPhysicsSoftBodySetVisualizeEnabled(nanoem_physics_soft_body_t *soft_body, nanoem_bool_t value)
{
    if (nanoem_is_not_null(soft_body)) {
        soft_body->setVisualizeEnabled(value);
    }
}

void APIENTRY
nanoemPhysicsSoftBodyDestroy(nanoem_physics_soft_body_t *soft_body)
{
    if (nanoem_is_not_null(soft_body)) {
        soft_body->destroy();
        nanoem_free(soft_body);
    }
}

#else

#include "./physics.h"

nanoem_bool_t APIENTRY
nanoemPhysicsWorldIsAvailable(void * /* opaque */)
{
    return 0;
}

nanoem_physics_world_t *APIENTRY
nanoemPhysicsWorldCreate(void * /* opaque */, nanoem_status_t * /* status */)
{
    return NULL;
}

void APIENTRY
nanoemPhysicsWorldAddRigidBody(nanoem_physics_world_t * /* world */, nanoem_physics_rigid_body_t * /* rigid_body */)
{
}

void APIENTRY
nanoemPhysicsWorldAddJoint(nanoem_physics_world_t * /* world */, nanoem_physics_joint_t * /* joint */)
{
}

void APIENTRY
nanoemPhysicsWorldRemoveRigidBody(nanoem_physics_world_t * /* world */, nanoem_physics_rigid_body_t * /* rigid_body */)
{
}

void APIENTRY
nanoemPhysicsWorldRemoveJoint(nanoem_physics_world_t * /* world */, nanoem_physics_joint_t * /* joint */)
{
}

void APIENTRY
nanoemPhysicsWorldSetPreferredFPS(nanoem_physics_world_t * /* world */, int /* value */)
{
}

int APIENTRY
nanoemPhysicsWorldStepSimulation(nanoem_physics_world_t * /* world */, nanoem_f32_t /* delta */)
{
    return 0;
}

void APIENTRY
nanoemPhysicsWorldReset(nanoem_physics_world_t * /* world */)
{
}

const nanoem_f32_t *APIENTRY
nanoemPhysicsWorldGetGravity(const nanoem_physics_world_t * /* world */)
{
    static const nanoem_f32_t default_gravity_value[] = { 0, -9.8f, 0 };
    return default_gravity_value;
}

void APIENTRY
nanoemPhysicsWorldSetGravity(nanoem_physics_world_t * /* world */, const nanoem_f32_t * /* value */)
{
}

const nanoem_f32_t *APIENTRY
nanoemPhysicsWorldGetGravityFactor(const nanoem_physics_world_t * /* world */)
{
    static const nanoem_f32_t default_gravity_factor_value[] = { 1, 1, 1 };
    return default_gravity_factor_value;
}

void APIENTRY
nanoemPhysicsWorldSetGravityFactor(nanoem_physics_world_t * /* world */, const nanoem_f32_t * /* value */)
{
}

nanoem_bool_t APIENTRY
nanoemPhysicsWorldIsActive(const nanoem_physics_world_t * /* world */)
{
    return 0;
}

void APIENTRY
nanoemPhysicsWorldSetActive(nanoem_physics_world_t * /* world */, nanoem_bool_t /* value */)
{
}

nanoem_bool_t APIENTRY
nanoemPhysicsWorldIsGroundEnabled(const nanoem_physics_world_t * /* world */)
{
    return 0;
}

void APIENTRY
nanoemPhysicsWorldSetGroundEnabled(nanoem_physics_world_t * /* world */, nanoem_bool_t /* value */)
{
}

void APIENTRY
nanoemPhysicsWorldSetDeactivationTimeThreshold(nanoem_physics_world_t * /* world */, nanoem_f32_t /* value */)
{
}

void APIENTRY
nanoemPhysicsWorldDestroy(nanoem_physics_world_t * /* world */)
{
}

nanoem_physics_rigid_body_t *APIENTRY
nanoemPhysicsRigidBodyCreate(
    const nanoem_model_rigid_body_t * /* value */, void * /* opaque */, nanoem_status_t * /* status */)
{
    return NULL;
}

nanoem_physics_motion_state_t *APIENTRY
nanoemPhysicsRigidBodyGetMotionState(const nanoem_physics_rigid_body_t * /* rigid_body */)
{
    return NULL;
}

void APIENTRY
nanoemPhysicsRigidBodyGetWorldTransform(
    const nanoem_physics_rigid_body_t * /* rigid_body */, nanoem_f32_t * /* value */)
{
}

void APIENTRY
nanoemPhysicsRigidBodySetWorldTransform(
    nanoem_physics_rigid_body_t * /* rigid_body */, const nanoem_f32_t * /* value */)
{
}

nanoem_bool_t APIENTRY
nanoemPhysicsRigidBodyIsVisualizeEnabled(const nanoem_physics_rigid_body_t * /* rigid_body */)
{
    return 0;
}

void APIENTRY
nanoemPhysicsRigidBodySetVisualizeEnabled(nanoem_physics_rigid_body_t * /* rigid_body */, nanoem_bool_t /* value */)
{
}

void APIENTRY
nanoemPhysicsRigidBodyDisableDeactivation(nanoem_physics_rigid_body_t * /* rigid_body */)
{
}

void APIENTRY
nanoemPhysicsRigidBodySetActive(nanoem_physics_rigid_body_t * /* rigid_body */)
{
}

void APIENTRY
nanoemPhysicsRigidBodySetKinematic(nanoem_physics_rigid_body_t * /* rigid_body */, int /* value */)
{
}

int APIENTRY
nanoemPhysicsRigidBodyIsKinematic(nanoem_physics_rigid_body_t* /* rigid_body */)
{
    return 0;
}

void APIENTRY
nanoemPhysicsRigidBodyResetLinearVelocity(nanoem_physics_rigid_body_t * /* rigid_body */)
{
}

void APIENTRY
nanoemPhysicsRigidBodyResetStates(nanoem_physics_rigid_body_t * /* rigid_body */)
{
}

void APIENTRY
nanoemPhysicsRigidBodyApplyTorqueImpulse(nanoem_physics_rigid_body_t * /* rigid_body */, const nanoem_f32_t * /* value */)
{
}

void APIENTRY
nanoemPhysicsRigidBodyApplyVelocityImpulse(nanoem_physics_rigid_body_t * /* rigid_body */, const nanoem_f32_t * /* value */)
{
}

void APIENTRY
nanoemPhysicsRigidBodyDestroy(nanoem_physics_rigid_body_t * /* rigid_body */)
{
}

void APIENTRY
nanoemPhysicsMotionStateGetInitialWorldTransform(
    const nanoem_physics_motion_state_t * /* motion_state */, nanoem_f32_t * /* value */)
{
}

void APIENTRY
nanoemPhysicsMotionStateGetCurrentWorldTransform(
    const nanoem_physics_motion_state_t * /* motion_state */, nanoem_f32_t * /* value */)
{
}

void APIENTRY
nanoemPhysicsMotionStateSetCurrentWorldTransform(
    nanoem_physics_motion_state_t * /* motion_state */, const nanoem_f32_t * /* value */)
{
}

void APIENTRY
nanoemPhysicsMotionStateGetCenterOfMassOffset(
    const nanoem_physics_motion_state_t * /* motion_state */, nanoem_f32_t * /* value */)
{
}


void APIENTRY
nanoemPhysicsMotionStateSetCenterOfMassOffset(
    nanoem_physics_motion_state_t * /* motion_state */, const nanoem_f32_t * /* value */)
{
}

nanoem_physics_joint_t *APIENTRY
nanoemPhysicsJointCreate(const nanoem_model_joint_t * /* value */, void * /* opaque */, nanoem_status_t * /* status */)
{
    return NULL;
}

void APIENTRY
nanoemPhysicsJointGetCalculatedTransformA(const nanoem_physics_joint_t * /* join t */, nanoem_f32_t * /* value */)
{
}

void APIENTRY
nanoemPhysicsJointGetCalculatedTransformB(const nanoem_physics_joint_t * /* joint */, nanoem_f32_t * /* value */)
{
}

nanoem_bool_t APIENTRY
nanoemPhysicsJointIsVisualizeEnabled(const nanoem_physics_joint_t * /* joint */)
{
    return 0;
}

void APIENTRY
nanoemPhysicsJointSetVisualizeEnabled(nanoem_physics_joint_t * /* joint */, nanoem_bool_t /* value */)
{
}

void APIENTRY
nanoemPhysicsJointDestroy(nanoem_physics_joint_t * /* joint */)
{
}

nanoem_u32_t APIENTRY
nanoemPhysicsWorldGetDebugGeomtryFlags(const nanoem_physics_world_t * /* world */)
{
    return 0;
}

void APIENTRY
nanoemPhysicsWorldSetDebugGeomtryFlags(nanoem_physics_world_t * /* world */, nanoem_u32_t /* value */)
{
}

nanoem_physics_debug_geometry_t *const *APIENTRY
nanoemPhysicsWorldGetDebugGeomtryObjects(const nanoem_physics_world_t * /* world */, int *num_objects)
{
    *num_objects = 0;
    return NULL;
}

const nanoem_f32_t *APIENTRY
nanoemPhysicsDebugGeometryGetFromPosition(const nanoem_physics_debug_geometry_t * /* geometry */)
{
    static const nanoem_f32_t null_vector[] = { 0, 0, 0 };
    return null_vector;
}

const nanoem_f32_t *APIENTRY
nanoemPhysicsDebugGeometryGetToPosition(const nanoem_physics_debug_geometry_t * /* geometry */)
{
    static const nanoem_f32_t null_vector[] = { 0, 0, 0 };
    return null_vector;
}

const nanoem_f32_t *APIENTRY
nanoemPhysicsDebugGeometryGetColor(const nanoem_physics_debug_geometry_t * /* geometry */)
{
    static const nanoem_f32_t null_color[] = { 0, 0, 0, 0 };
    return null_color;
}

#endif /* NANOEM_ENABLE_BULLET */
