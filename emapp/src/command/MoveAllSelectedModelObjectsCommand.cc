/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/MoveAllSelectedModelObjectsCommand.h"

#include "emapp/private/CommonInclude.h"

#include "nanoem/ext/mutable.h"

namespace nanoem {
namespace command {
namespace {

typedef tinystl::unordered_map<const nanoem_model_material_t *, nanoem_rsize_t, TinySTLAllocator> MaterialOffsetMap;
typedef tinystl::unordered_set<nanoem_model_vertex_t *, TinySTLAllocator> MutableVertexSet;

} /* namespace anonymous */

MoveAllSelectedModelObjectsCommand::State::State(Model *activeModel)
    : m_activeModel(activeModel)
    , m_editingType(activeModel->selection()->editingType())
{
    const IModelObjectSelection *selection = activeModel->selection();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    switch (selection->editingType()) {
    case IModelObjectSelection::kEditingTypeBone: {
        const model::Bone::Set selectedBoneSet(selection->allBoneSet());
        nanoem_rsize_t numBones;
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(activeModel->data(), &numBones);
        for (model::Bone::Set::const_iterator it = selectedBoneSet.begin(), end = selectedBoneSet.end(); it != end;
             ++it) {
            const nanoem_model_bone_t *bonePtr = *it;
            nanoem_model_bone_t *mutableBonePtr = bones[model::Bone::index(bonePtr)];
            nanoem_mutable_model_bone_t *item = nanoemMutableModelBoneCreateAsReference(mutableBonePtr, &status);
            const Vector4 origin(glm::make_vec4(nanoemModelBoneGetOrigin(bonePtr)));
            const MutableOffsetBone bone = { item, origin };
            m_bones.push_back(bone);
        }
        activeModel->performAllBonesTransform();
        break;
    }
    case IModelObjectSelection::kEditingTypeFace: {
        const IModelObjectSelection::FaceList selectedFaces(selection->allFaces());
        MutableVertexSet vertexSet;
        nanoem_rsize_t numVertices;
        nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(activeModel->data(), &numVertices);
        for (IModelObjectSelection::FaceList::const_iterator it = selectedFaces.begin(), end = selectedFaces.end();
             it != end; ++it) {
            const Vector4UI32 &face = *it;
            for (nanoem_rsize_t i = 1; i < 4; i++) {
                nanoem_model_vertex_t *mutableVertexPtr = vertices[face[i]];
                vertexSet.insert(mutableVertexPtr);
            }
        }
        for (MutableVertexSet::const_iterator it = vertexSet.begin(), end = vertexSet.end(); it != end; ++it) {
            nanoem_model_vertex_t *mutableVertexPtr = *it;
            nanoem_mutable_model_vertex_t *item = nanoemMutableModelVertexCreateAsReference(mutableVertexPtr, &status);
            const Vector4 origin(glm::make_vec4(nanoemModelVertexGetOrigin(mutableVertexPtr)));
            const MutableOffsetVertex vertex = { item, origin };
            m_vertices.push_back(vertex);
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeJoint: {
        const model::Joint::Set selectedJointSet(selection->allJointSet());
        nanoem_rsize_t numJoints;
        nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(activeModel->data(), &numJoints);
        for (model::Joint::Set::const_iterator it = selectedJointSet.begin(), end = selectedJointSet.end(); it != end;
             ++it) {
            const nanoem_model_joint_t *jointPtr = *it;
            nanoem_model_joint_t *mutableJointPtr = joints[model::Joint::index(jointPtr)];
            nanoem_mutable_model_joint_t *item = nanoemMutableModelJointCreateAsReference(mutableJointPtr, &status);
            const Vector4 origin(glm::make_vec4(nanoemModelJointGetOrigin(jointPtr)));
            const MutableOffsetJoint joint = { item, origin };
            m_joints.push_back(joint);
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeMaterial: {
        const model::Material::Set selectedMaterialSet(selection->allMaterialSet());
        MutableVertexSet vertexSet;
        nanoem_rsize_t numMaterials, numVertices, numVertexIndices;
        const nanoem_model_t *opaque = activeModel->data();
        nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(opaque, &numMaterials);
        nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(opaque, &numVertices);
        const nanoem_u32_t *vertexIndices = nanoemModelGetAllVertexIndices(opaque, &numVertexIndices);
        MaterialOffsetMap materialOffsets;
        for (nanoem_rsize_t i = 0, offset = 0; i < numMaterials; i++) {
            const nanoem_model_material_t *material = materials[i];
            materialOffsets.insert(tinystl::make_pair(material, offset));
            offset += nanoemModelMaterialGetNumVertexIndices(material);
        }
        for (model::Material::Set::const_iterator it = selectedMaterialSet.begin(), end = selectedMaterialSet.end();
             it != end; ++it) {
            const nanoem_model_material_t *materialPtr = *it;
            MaterialOffsetMap::const_iterator it2 = materialOffsets.find(materialPtr);
            if (it2 != materialOffsets.end()) {
                nanoem_rsize_t offset = it2->second, indices = nanoemModelMaterialGetNumVertexIndices(materialPtr);
                for (nanoem_rsize_t i = 0; i < indices; i++) {
                    const nanoem_u32_t vertexIndex = vertexIndices[offset + i];
                    nanoem_model_vertex_t *mutableVertexPtr = vertices[vertexIndex];
                    vertexSet.insert(mutableVertexPtr);
                }
            }
        }
        for (MutableVertexSet::const_iterator it = vertexSet.begin(), end = vertexSet.end(); it != end; ++it) {
            nanoem_model_vertex_t *mutableVertexPtr = *it;
            nanoem_mutable_model_vertex_t *item = nanoemMutableModelVertexCreateAsReference(mutableVertexPtr, &status);
            const Vector4 origin(glm::make_vec4(nanoemModelVertexGetOrigin(mutableVertexPtr)));
            const MutableOffsetVertex vertex = { item, origin };
            m_vertices.push_back(vertex);
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeRigidBody: {
        const model::RigidBody::Set selectedRigidBodySet(selection->allRigidBodySet());
        nanoem_rsize_t numRigidBodies;
        nanoem_model_rigid_body_t *const *rigidBodies =
            nanoemModelGetAllRigidBodyObjects(activeModel->data(), &numRigidBodies);
        for (model::RigidBody::Set::const_iterator it = selectedRigidBodySet.begin(), end = selectedRigidBodySet.end();
             it != end; ++it) {
            const nanoem_model_rigid_body_t *rigidBodyPtr = *it;
            nanoem_model_rigid_body_t *mutableRigidBodyPtr = rigidBodies[model::RigidBody::index(rigidBodyPtr)];
            nanoem_mutable_model_rigid_body_t *item =
                nanoemMutableModelRigidBodyCreateAsReference(mutableRigidBodyPtr, &status);
            const Vector4 origin(glm::make_vec4(nanoemModelRigidBodyGetOrigin(mutableRigidBodyPtr)));
            const MutableOffsetRigidBody rigidBody = { item, rigidBodyWorldTransform(rigidBodyPtr), origin };
            m_rigidBodies.push_back(rigidBody);
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeSoftBody: {
        const model::SoftBody::Set selectedSoftBodySet(selection->allSoftBodySet());
        MutableVertexSet vertexSet;
        nanoem_rsize_t numMaterials, numVertices, numVertexIndices;
        const nanoem_model_t *opaque = activeModel->data();
        nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(opaque, &numMaterials);
        nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(opaque, &numVertices);
        const nanoem_u32_t *vertexIndices = nanoemModelGetAllVertexIndices(opaque, &numVertexIndices);
        MaterialOffsetMap materialOffsets;
        for (nanoem_rsize_t i = 0, offset = 0; i < numMaterials; i++) {
            const nanoem_model_material_t *material = materials[i];
            materialOffsets.insert(tinystl::make_pair(material, offset));
            offset += nanoemModelMaterialGetNumVertexIndices(material);
        }
        for (model::SoftBody::Set::const_iterator it = selectedSoftBodySet.begin(), end = selectedSoftBodySet.end();
             it != end; ++it) {
            const nanoem_model_soft_body_t *softBodyPtr = *it;
            const nanoem_model_material_t *materialPtr = nanoemModelSoftBodyGetMaterialObject(softBodyPtr);
            MaterialOffsetMap::const_iterator it2 = materialOffsets.find(materialPtr);
            if (it2 != materialOffsets.end()) {
                nanoem_rsize_t offset = it2->second, indices = nanoemModelMaterialGetNumVertexIndices(materialPtr);
                for (nanoem_rsize_t i = 0; i < indices; i++) {
                    const nanoem_u32_t vertexIndex = vertexIndices[offset + i];
                    nanoem_model_vertex_t *mutableVertexPtr = vertices[vertexIndex];
                    vertexSet.insert(mutableVertexPtr);
                }
            }
        }
        for (MutableVertexSet::const_iterator it = vertexSet.begin(), end = vertexSet.end(); it != end; ++it) {
            nanoem_model_vertex_t *mutableVertexPtr = *it;
            nanoem_mutable_model_vertex_t *item = nanoemMutableModelVertexCreateAsReference(mutableVertexPtr, &status);
            const Vector4 origin(glm::make_vec4(nanoemModelVertexGetOrigin(mutableVertexPtr)));
            const MutableOffsetVertex vertex = { item, origin };
            m_vertices.push_back(vertex);
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeVertex: {
        const model::Vertex::Set selectedVertexSet(selection->allVertexSet());
        nanoem_rsize_t numVertices;
        nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(activeModel->data(), &numVertices);
        for (model::Vertex::Set::const_iterator it = selectedVertexSet.begin(), end = selectedVertexSet.end();
             it != end; ++it) {
            const nanoem_model_vertex_t *vertexPtr = *it;
            nanoem_model_vertex_t *mutableVertexPtr = vertices[model::Vertex::index(vertexPtr)];
            nanoem_mutable_model_vertex_t *item = nanoemMutableModelVertexCreateAsReference(mutableVertexPtr, &status);
            const Vector4 origin(glm::make_vec4(nanoemModelVertexGetOrigin(mutableVertexPtr)));
            const MutableOffsetVertex vertex = { item, origin };
            m_vertices.push_back(vertex);
        }
        break;
    }
    default:
        break;
    }
}

MoveAllSelectedModelObjectsCommand::State::~State() NANOEM_DECL_NOEXCEPT
{
    for (MutableOffsetBoneList::const_iterator it = m_bones.begin(), end = m_bones.end(); it != end; ++it) {
        nanoemMutableModelBoneDestroy(it->m_opaque);
    }
    m_bones.clear();
    for (MutableOffsetJointList::const_iterator it = m_joints.begin(), end = m_joints.end(); it != end; ++it) {
        nanoemMutableModelJointDestroy(it->m_opaque);
    }
    m_joints.clear();
    for (MutableOffsetRigidBodyList::const_iterator it = m_rigidBodies.begin(), end = m_rigidBodies.end(); it != end;
         ++it) {
        nanoemMutableModelRigidBodyDestroy(it->m_opaque);
    }
    m_rigidBodies.clear();
    for (MutableOffsetVertexList::const_iterator it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
        nanoemMutableModelVertexDestroy(it->m_opaque);
    }
    m_vertices.clear();
    m_activeModel = nullptr;
}

void
MoveAllSelectedModelObjectsCommand::State::transform(const Matrix4x4 &delta)
{
    switch (m_editingType) {
    case IModelObjectSelection::kEditingTypeBone: {
        for (MutableOffsetBoneList::const_iterator it = m_bones.begin(), end = m_bones.end(); it != end; ++it) {
            const nanoem_model_bone_t *bonePtr = nanoemMutableModelBoneGetOriginObject(it->m_opaque);
            const Vector4 newOrigin(delta * Vector4(glm::make_vec3(nanoemModelBoneGetOrigin(bonePtr)), 1));
            nanoemMutableModelBoneSetOrigin(it->m_opaque, glm::value_ptr(newOrigin));
        }
        m_activeModel->performAllBonesTransform();
        break;
    }
    case IModelObjectSelection::kEditingTypeJoint: {
        for (MutableOffsetJointList::const_iterator it = m_joints.begin(), end = m_joints.end(); it != end; ++it) {
            const nanoem_model_joint_t *jointPtr = nanoemMutableModelJointGetOriginObject(it->m_opaque);
            const Vector4 newOrigin(delta * Vector4(glm::make_vec3(nanoemModelJointGetOrigin(jointPtr)), 1));
            nanoemMutableModelJointSetOrigin(it->m_opaque, glm::value_ptr(newOrigin));
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeRigidBody: {
        for (MutableOffsetRigidBodyList::const_iterator it = m_rigidBodies.begin(), end = m_rigidBodies.end();
             it != end; ++it) {
            const nanoem_model_rigid_body_t *rigidBodyPtr = nanoemMutableModelRigidBodyGetOriginObject(it->m_opaque);
            const Vector4 newOrigin(delta * Vector4(glm::make_vec3(nanoemModelRigidBodyGetOrigin(rigidBodyPtr)), 1));
            nanoemMutableModelRigidBodySetOrigin(it->m_opaque, glm::value_ptr(newOrigin));
            updateRigidBody(delta, rigidBodyWorldTransform(rigidBodyPtr), rigidBodyPtr);
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeFace:
    case IModelObjectSelection::kEditingTypeMaterial:
    case IModelObjectSelection::kEditingTypeSoftBody:
    case IModelObjectSelection::kEditingTypeVertex: {
        for (MutableOffsetVertexList::const_iterator it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
            const nanoem_model_vertex_t *vertexPtr = nanoemMutableModelVertexGetOriginObject(it->m_opaque);
            const Vector4 newOrigin(delta * Vector4(glm::make_vec3(nanoemModelVertexGetOrigin(vertexPtr)), 1));
            nanoemMutableModelVertexSetOrigin(it->m_opaque, glm::value_ptr(newOrigin));
            model::Vertex *vertex = model::Vertex::cast(vertexPtr);
            vertex->m_simd.m_origin = bx::simd_ld(glm::value_ptr(newOrigin));
        }
        m_activeModel->markStagingVertexBufferDirty();
        m_activeModel->updateStagingVertexBuffer();
        break;
    }
    default:
        break;
    }
}

void
MoveAllSelectedModelObjectsCommand::State::commit(const Matrix4x4 &delta)
{
    switch (m_editingType) {
    case IModelObjectSelection::kEditingTypeBone: {
        for (MutableOffsetBoneList::const_iterator it = m_bones.begin(), end = m_bones.end(); it != end; ++it) {
            const Vector4 newOrigin(delta * Vector4(Vector3(it->m_origin), 1));
            nanoemMutableModelBoneSetOrigin(it->m_opaque, glm::value_ptr(newOrigin));
        }
        m_activeModel->performAllBonesTransform();
        break;
    }
    case IModelObjectSelection::kEditingTypeJoint: {
        for (MutableOffsetJointList::const_iterator it = m_joints.begin(), end = m_joints.end(); it != end; ++it) {
            const Vector4 newOrigin(delta * Vector4(Vector3(it->m_origin), 1));
            nanoemMutableModelJointSetOrigin(it->m_opaque, glm::value_ptr(newOrigin));
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeRigidBody: {
        for (MutableOffsetRigidBodyList::const_iterator it = m_rigidBodies.begin(), end = m_rigidBodies.end();
             it != end; ++it) {
            const Vector4 newOrigin(delta * Vector4(Vector3(it->m_origin), 1));
            nanoemMutableModelRigidBodySetOrigin(it->m_opaque, glm::value_ptr(newOrigin));
            const nanoem_model_rigid_body_t *rigidBodyPtr = nanoemMutableModelRigidBodyGetOriginObject(it->m_opaque);
            updateRigidBody(delta, it->m_worldTransform, rigidBodyPtr);
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeFace:
    case IModelObjectSelection::kEditingTypeMaterial:
    case IModelObjectSelection::kEditingTypeSoftBody:
    case IModelObjectSelection::kEditingTypeVertex: {
        for (MutableOffsetVertexList::const_iterator it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
            const Vector4 newOrigin(delta * Vector4(Vector3(it->m_origin), 1));
            nanoemMutableModelVertexSetOrigin(it->m_opaque, glm::value_ptr(newOrigin));
            const nanoem_model_vertex_t *vertexPtr = nanoemMutableModelVertexGetOriginObject(it->m_opaque);
            model::Vertex *vertex = model::Vertex::cast(vertexPtr);
            vertex->m_simd.m_origin = bx::simd_ld(glm::value_ptr(newOrigin));
        }
        m_activeModel->markStagingVertexBufferDirty();
        m_activeModel->updateStagingVertexBuffer();
        break;
    }
    default:
        break;
    }
}

void
MoveAllSelectedModelObjectsCommand::State::reset()
{
    switch (m_editingType) {
    case IModelObjectSelection::kEditingTypeBone: {
        for (MutableOffsetBoneList::const_iterator it = m_bones.begin(), end = m_bones.end(); it != end; ++it) {
            nanoemMutableModelBoneSetOrigin(it->m_opaque, glm::value_ptr(it->m_origin));
        }
        m_activeModel->performAllBonesTransform();
        break;
    }
    case IModelObjectSelection::kEditingTypeJoint: {
        for (MutableOffsetJointList::const_iterator it = m_joints.begin(), end = m_joints.end(); it != end; ++it) {
            nanoemMutableModelJointSetOrigin(it->m_opaque, glm::value_ptr(it->m_origin));
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeRigidBody: {
        for (MutableOffsetRigidBodyList::const_iterator it = m_rigidBodies.begin(), end = m_rigidBodies.end();
             it != end; ++it) {
            nanoemMutableModelRigidBodySetOrigin(it->m_opaque, glm::value_ptr(it->m_origin));
            const nanoem_model_rigid_body_t *rigidBodyPtr = nanoemMutableModelRigidBodyGetOriginObject(it->m_opaque);
            updateRigidBody(Constants::kIdentity, it->m_worldTransform, rigidBodyPtr);
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeFace:
    case IModelObjectSelection::kEditingTypeMaterial:
    case IModelObjectSelection::kEditingTypeSoftBody:
    case IModelObjectSelection::kEditingTypeVertex: {
        for (MutableOffsetVertexList::const_iterator it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
            nanoemMutableModelVertexSetOrigin(it->m_opaque, glm::value_ptr(it->m_origin));
            const nanoem_model_vertex_t *vertexPtr = nanoemMutableModelVertexGetOriginObject(it->m_opaque);
            model::Vertex *vertex = model::Vertex::cast(vertexPtr);
            vertex->m_simd.m_origin = bx::simd_ld(glm::value_ptr(it->m_origin));
        }
        m_activeModel->markStagingVertexBufferDirty();
        m_activeModel->updateStagingVertexBuffer();
        break;
    }
    default:
        break;
    }
}

void
MoveAllSelectedModelObjectsCommand::State::setPivotMatrix(const Matrix4x4 &value)
{
    m_activeModel->setPivotMatrix(value);
}

Matrix4x4
MoveAllSelectedModelObjectsCommand::State::rigidBodyWorldTransform(const nanoem_model_rigid_body_t *rigidBodyPtr) const
{
    Matrix4x4 worldTransform(Constants::kIdentity);
    if (const model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr)) {
        PhysicsEngine *engine = rigidBody->physicsEngine();
        const nanoem_physics_motion_state_t *state = engine->motionState(rigidBody->physicsRigidBody());
        engine->getWorldTransform(state, glm::value_ptr(worldTransform));
    }
    return worldTransform;
}

void
MoveAllSelectedModelObjectsCommand::State::updateRigidBody(
    const Matrix4x4 &delta, const Matrix4x4 &worldTransform, const nanoem_model_rigid_body_t *rigidBodyPtr)
{
    if (model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr)) {
        PhysicsEngine *engine = rigidBody->physicsEngine();
        nanoem_physics_motion_state_t *state = engine->motionState(rigidBody->physicsRigidBody());
        engine->setWorldTransform(state, glm::value_ptr(delta * worldTransform));
    }
}

undo_command_t *
MoveAllSelectedModelObjectsCommand::create(
    const Matrix4x4 &transformMatrix, const Matrix4x4 &pivotMatrix, Model *activeModel, Project *project)
{
    MoveAllSelectedModelObjectsCommand *command =
        nanoem_new(MoveAllSelectedModelObjectsCommand(transformMatrix, pivotMatrix, activeModel, project));
    return command->createCommand();
}

MoveAllSelectedModelObjectsCommand::~MoveAllSelectedModelObjectsCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveAllSelectedModelObjectsCommand::undo(Error & /* error */)
{
    m_state.transform(glm::inverse(m_transformMatrix));
    m_state.setPivotMatrix(m_lastPivotMatrix);
}

void
MoveAllSelectedModelObjectsCommand::redo(Error & /* error */)
{
    m_state.commit(m_transformMatrix);
    m_state.setPivotMatrix(m_currentPivotMatrix);
}

const char *
MoveAllSelectedModelObjectsCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "MoveAllSelectedModelObjectsCommand";
}

MoveAllSelectedModelObjectsCommand::MoveAllSelectedModelObjectsCommand(
    const Matrix4x4 &transformMatrix, const Matrix4x4 &pivotMatrix, Model *activeModel, Project *project)
    : BaseUndoCommand(project)
    , m_state(activeModel)
    , m_transformMatrix(transformMatrix)
    , m_lastPivotMatrix(pivotMatrix)
    , m_currentPivotMatrix(activeModel->pivotMatrix())
{
}

void
MoveAllSelectedModelObjectsCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveAllSelectedModelObjectsCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveAllSelectedModelObjectsCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

} /* namespace command */
} /* namespace nanoem */
