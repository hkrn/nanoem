/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/TransformModelCommand.h"

#include "emapp/Model.h"
#include "emapp/Project.h"
#include "emapp/private/CommonInclude.h"

#include "nanoem/ext/mutable.h"

namespace nanoem {
namespace command {

undo_command_t *
TransformModelCommand::create(Model *activeModel, const Matrix4x4 &transform)
{
    TransformModelCommand *command = nanoem_new(TransformModelCommand(activeModel, transform));
    return command->createCommand();
}

TransformModelCommand::TransformModelCommand(Model *activeModel, const Matrix4x4 &transform)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_transform(transform)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t numVertices;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(m_activeModel->data(), &numVertices);
    for (nanoem_rsize_t i = 0; i < numVertices; i++) {
        nanoem_model_vertex_t *vertexPtr = vertices[i];
        Vertex vertex = { nanoemMutableModelVertexCreateAsReference(vertexPtr, &status),
            glm::make_vec3(nanoemModelVertexGetOrigin(vertexPtr)) };
        m_vertices.push_back(vertex);
    }
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        nanoem_model_bone_t *bonePtr = bones[i];
        Bone bone = { nanoemMutableModelBoneCreateAsReference(bonePtr, &status),
            glm::make_vec3(nanoemModelBoneGetOrigin(bonePtr)),
            glm::make_vec3(nanoemModelBoneGetDestinationOrigin(bonePtr)) };
        m_bones.push_back(bone);
    }
    nanoem_rsize_t numMorphs;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_activeModel->data(), &numMorphs);
    for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
        nanoem_model_morph_t *morphPtr = morphs[i];
        if (nanoemModelMorphGetType(morphPtr) == NANOEM_MODEL_MORPH_TYPE_VERTEX) {
            nanoem_rsize_t numItems;
            nanoem_model_morph_vertex_t *const *items = nanoemModelMorphGetAllVertexMorphObjects(morphPtr, &numItems);
            for (nanoem_rsize_t j = 0; j < numItems; j++) {
                nanoem_model_morph_vertex_t *itemPtr = items[j];
                VertexMorph morph = { nanoemMutableModelMorphVertexCreateAsReference(itemPtr, &status),
                    glm::make_vec3(nanoemModelMorphVertexGetPosition(itemPtr)) };
                m_vertexMorphs.push_back(morph);
            }
        }
        else if (nanoemModelMorphGetType(morphPtr) == NANOEM_MODEL_MORPH_TYPE_BONE) {
            nanoem_rsize_t numItems;
            nanoem_model_morph_bone_t *const *items = nanoemModelMorphGetAllBoneMorphObjects(morphPtr, &numItems);
            for (nanoem_rsize_t j = 0; j < numItems; j++) {
                nanoem_model_morph_bone_t *itemPtr = items[j];
                BoneMorph morph = { nanoemMutableModelMorphBoneCreateAsReference(itemPtr, &status),
                    glm::make_vec3(nanoemModelMorphBoneGetTranslation(itemPtr)) };
                m_boneMorphs.push_back(morph);
            }
        }
    }
    nanoem_rsize_t numRigidBodies;
    nanoem_model_rigid_body_t *const *rigidBodies =
        nanoemModelGetAllRigidBodyObjects(m_activeModel->data(), &numRigidBodies);
    for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
        nanoem_model_rigid_body_t *rigidBodyPtr = rigidBodies[i];
        RigidBody rigidBody = { nanoemMutableModelRigidBodyCreateAsReference(rigidBodyPtr, &status),
            glm::make_vec3(nanoemModelRigidBodyGetOrigin(rigidBodyPtr)),
            glm::make_vec3(nanoemModelRigidBodyGetShapeSize(rigidBodyPtr)) };
        m_rigidBodies.push_back(rigidBody);
    }
    nanoem_rsize_t numJoints;
    nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(m_activeModel->data(), &numJoints);
    for (nanoem_rsize_t i = 0; i < numJoints; i++) {
        nanoem_model_joint_t *jointPtr = joints[i];
        Joint joint = { nanoemMutableModelJointCreateAsReference(jointPtr, &status),
            glm::make_vec3(nanoemModelJointGetOrigin(jointPtr)) };
        m_joints.push_back(joint);
    }
}

TransformModelCommand::~TransformModelCommand() NANOEM_DECL_NOEXCEPT
{
    for (VertexList::const_iterator it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
        nanoemMutableModelVertexDestroy(it->m_opaque);
    }
    m_vertices.clear();
    for (BoneList::const_iterator it = m_bones.begin(), end = m_bones.end(); it != end; ++it) {
        nanoemMutableModelBoneDestroy(it->m_opaque);
    }
    m_bones.clear();
    for (VertexMorphList::const_iterator it = m_vertexMorphs.begin(), end = m_vertexMorphs.end(); it != end; ++it) {
        nanoemMutableModelMorphVertexDestroy(it->m_opaque);
    }
    m_vertexMorphs.clear();
    for (BoneMorphList::const_iterator it = m_boneMorphs.begin(), end = m_boneMorphs.end(); it != end; ++it) {
        nanoemMutableModelMorphBoneDestroy(it->m_opaque);
    }
    m_boneMorphs.clear();
    for (RigidBodyList::const_iterator it = m_rigidBodies.begin(), end = m_rigidBodies.end(); it != end; ++it) {
        nanoemMutableModelRigidBodyDestroy(it->m_opaque);
    }
    m_rigidBodies.clear();
    for (JointList::const_iterator it = m_joints.begin(), end = m_joints.end(); it != end; ++it) {
        nanoemMutableModelJointDestroy(it->m_opaque);
    }
    m_joints.clear();
}

void
TransformModelCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (VertexList::const_iterator it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
        const Vector4 origin(it->m_origin, 1);
        nanoemMutableModelVertexSetOrigin(it->m_opaque, glm::value_ptr(origin));
        if (model::Vertex *vertex = model::Vertex::cast(nanoemMutableModelVertexGetOriginObject(it->m_opaque))) {
            vertex->m_simd.m_origin = bx::simd_ld(glm::value_ptr(origin));
        }
    }
    for (BoneList::const_iterator it = m_bones.begin(), end = m_bones.end(); it != end; ++it) {
        const Vector4 origin(it->m_origin, 1);
        nanoemMutableModelBoneSetOrigin(it->m_opaque, glm::value_ptr(origin));
        if (!nanoemModelBoneHasDestinationBone(nanoemMutableModelBoneGetOriginObject(it->m_opaque))) {
            const Vector4 destination(it->m_destination, 1);
            nanoemMutableModelBoneSetDestinationOrigin(it->m_opaque, glm::value_ptr(destination));
        }
    }
    for (VertexMorphList::const_iterator it = m_vertexMorphs.begin(), end = m_vertexMorphs.end(); it != end; ++it) {
        const Vector4 origin(it->m_origin, 1);
        nanoemMutableModelMorphVertexSetPosition(it->m_opaque, glm::value_ptr(origin));
    }
    for (BoneMorphList::const_iterator it = m_boneMorphs.begin(), end = m_boneMorphs.end(); it != end; ++it) {
        const Vector4 translation(it->m_translation, 0);
        nanoemMutableModelMorphBoneSetTranslation(it->m_opaque, glm::value_ptr(translation));
    }
    const Matrix4x4 inverseTransform(glm::inverse(m_transform));
    for (RigidBodyList::const_iterator it = m_rigidBodies.begin(), end = m_rigidBodies.end(); it != end; ++it) {
        const Vector4 origin(it->m_origin, 1), size(it->m_size, 0);
        nanoemMutableModelRigidBodySetOrigin(it->m_opaque, glm::value_ptr(origin));
        nanoemMutableModelRigidBodySetShapeSize(it->m_opaque, glm::value_ptr(size));
        scaleRigidBody(*it, inverseTransform);
    }
    for (JointList::const_iterator it = m_joints.begin(), end = m_joints.end(); it != end; ++it) {
        const Vector4 origin(it->m_origin, 1);
        nanoemMutableModelJointSetOrigin(it->m_opaque, glm::value_ptr(origin));
    }
    m_activeModel->markStagingVertexBufferDirty();
    m_activeModel->updateStagingVertexBuffer();
    assignError(status, error);
}

void
TransformModelCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (VertexList::const_iterator it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
        const Vector4 origin(it->m_origin, 1), newOrigin(m_transform * origin);
        nanoemMutableModelVertexSetOrigin(it->m_opaque, glm::value_ptr(newOrigin));
        if (model::Vertex *vertex = model::Vertex::cast(nanoemMutableModelVertexGetOriginObject(it->m_opaque))) {
            vertex->m_simd.m_origin = bx::simd_ld(glm::value_ptr(newOrigin));
        }
    }
    for (BoneList::const_iterator it = m_bones.begin(), end = m_bones.end(); it != end; ++it) {
        const Vector4 origin(it->m_origin, 1), newOrigin(m_transform * origin);
        nanoemMutableModelBoneSetOrigin(it->m_opaque, glm::value_ptr(newOrigin));
        if (!nanoemModelBoneHasDestinationBone(nanoemMutableModelBoneGetOriginObject(it->m_opaque))) {
            const Vector4 destination(it->m_destination, 1), newDestination(m_transform * destination);
            nanoemMutableModelBoneSetDestinationOrigin(it->m_opaque, glm::value_ptr(newDestination));
        }
    }
    for (VertexMorphList::const_iterator it = m_vertexMorphs.begin(), end = m_vertexMorphs.end(); it != end; ++it) {
        const Vector4 origin(it->m_origin, 1), newOrigin(m_transform * origin);
        nanoemMutableModelMorphVertexSetPosition(it->m_opaque, glm::value_ptr(newOrigin));
    }
    for (BoneMorphList::const_iterator it = m_boneMorphs.begin(), end = m_boneMorphs.end(); it != end; ++it) {
        const Vector4 translation(it->m_translation, 1), newTranslation(m_transform * translation);
        nanoemMutableModelMorphBoneSetTranslation(it->m_opaque, glm::value_ptr(newTranslation));
    }
    for (RigidBodyList::const_iterator it = m_rigidBodies.begin(), end = m_rigidBodies.end(); it != end; ++it) {
        const Vector4 origin(it->m_origin, 1), newOrigin(m_transform * origin), size(it->m_size, 1),
            newSize(m_transform * size);
        nanoemMutableModelRigidBodySetOrigin(it->m_opaque, glm::value_ptr(newOrigin));
        nanoemMutableModelRigidBodySetShapeSize(it->m_opaque, glm::value_ptr(newSize));
        scaleRigidBody(*it, m_transform);
    }
    for (JointList::const_iterator it = m_joints.begin(), end = m_joints.end(); it != end; ++it) {
        const Vector4 origin(it->m_origin, 1), newOrigin(m_transform * origin);
        nanoemMutableModelJointSetOrigin(it->m_opaque, glm::value_ptr(newOrigin));
    }
    m_activeModel->markStagingVertexBufferDirty();
    m_activeModel->updateStagingVertexBuffer();
    assignError(status, error);
}

void
TransformModelCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
TransformModelCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
TransformModelCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
TransformModelCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "TransformModelCommand";
}

void
TransformModelCommand::scaleRigidBody(const RigidBody &value, const Matrix4x4 &transform)
{
    if (model::RigidBody *rigidBody =
            model::RigidBody::cast(nanoemMutableModelRigidBodyGetOriginObject(value.m_opaque))) {
        PhysicsEngine *engine = rigidBody->physicsEngine();
        nanoem_physics_rigid_body_t *physicsRigidBodyPtr = rigidBody->physicsRigidBody();
        nanoem_physics_motion_state_t *state = engine->motionState(physicsRigidBodyPtr);
        Matrix4x4 worldTransform;
        engine->getWorldTransform(state, glm::value_ptr(worldTransform));
        engine->setWorldTransform(state, glm::value_ptr(transform * worldTransform));
        engine->getCenterOfMassOffset(state, glm::value_ptr(worldTransform));
        engine->setCenterOfMassOffset(state, glm::value_ptr(transform * worldTransform));
        engine->getWorldTransform(physicsRigidBodyPtr, glm::value_ptr(worldTransform));
        engine->setWorldTransform(physicsRigidBodyPtr, glm::value_ptr(transform * worldTransform));
    }
}

} /* namespace command */
} /* namespace nanoem */
