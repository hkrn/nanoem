/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/MoveAllSelectedModelObjectsCommand.h"

#include "emapp/private/CommonInclude.h"

#include "nanoem/ext/mutable.h"

namespace nanoem {
namespace command {

MoveAllSelectedModelObjectsCommand::State::State(Model *activeModel)
    : m_activeModel(activeModel)
    , m_editingType(activeModel->selection()->editingType())
{
    typedef tinystl::unordered_map<const nanoem_model_material_t *, nanoem_rsize_t, TinySTLAllocator> MaterialOffsetMap;
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
            m_bones.push_back(tinystl::make_pair(item, origin));
        }
        activeModel->performAllBonesTransform();
        break;
    }
    case IModelObjectSelection::kEditingTypeFace: {
        const IModelObjectSelection::FaceList selectedFaces(selection->allFaces());
        nanoem_rsize_t numVertices;
        nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(activeModel->data(), &numVertices);
        for (IModelObjectSelection::FaceList::const_iterator it = selectedFaces.begin(), end = selectedFaces.end();
             it != end; ++it) {
            const Vector4UI32 &face = *it;
            for (nanoem_rsize_t i = 1; i < 4; i++) {
                nanoem_model_vertex_t *mutableVertexPtr = vertices[face[i]];
                nanoem_mutable_model_vertex_t *item =
                    nanoemMutableModelVertexCreateAsReference(mutableVertexPtr, &status);
                const Vector4 origin(glm::make_vec4(nanoemModelVertexGetOrigin(mutableVertexPtr)));
                m_vertices.push_back(tinystl::make_pair(item, origin));
            }
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
            m_joints.push_back(tinystl::make_pair(item, origin));
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeMaterial: {
        const model::Material::Set selectedMaterialSet(selection->allMaterialSet());
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
                    nanoem_mutable_model_vertex_t *item =
                        nanoemMutableModelVertexCreateAsReference(mutableVertexPtr, &status);
                    const Vector4 origin(glm::make_vec4(nanoemModelVertexGetOrigin(mutableVertexPtr)));
                    m_vertices.push_back(tinystl::make_pair(item, origin));
                }
            }
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
            m_rigidBodies.push_back(tinystl::make_pair(item, origin));
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeSoftBody: {
        const model::SoftBody::Set selectedSoftBodySet(selection->allSoftBodySet());
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
                    nanoem_mutable_model_vertex_t *item =
                        nanoemMutableModelVertexCreateAsReference(mutableVertexPtr, &status);
                    const Vector4 origin(glm::make_vec4(nanoemModelVertexGetOrigin(mutableVertexPtr)));
                    m_vertices.push_back(tinystl::make_pair(item, origin));
                }
            }
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
            m_vertices.push_back(tinystl::make_pair(item, origin));
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
        nanoemMutableModelBoneDestroy(it->first);
    }
    m_bones.clear();
    for (MutableOffsetJointList::const_iterator it = m_joints.begin(), end = m_joints.end(); it != end; ++it) {
        nanoemMutableModelJointDestroy(it->first);
    }
    m_joints.clear();
    for (MutableOffsetRigidBodyList::const_iterator it = m_rigidBodies.begin(), end = m_rigidBodies.end(); it != end;
         ++it) {
        nanoemMutableModelRigidBodyDestroy(it->first);
    }
    m_rigidBodies.clear();
    for (MutableOffsetVertexList::const_iterator it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
        nanoemMutableModelVertexDestroy(it->first);
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
            const nanoem_model_bone_t *bonePtr = nanoemMutableModelBoneGetOriginObject(it->first);
            const Vector4 newOrigin(delta * Vector4(glm::make_vec3(nanoemModelBoneGetOrigin(bonePtr)), 1));
            nanoemMutableModelBoneSetOrigin(it->first, glm::value_ptr(newOrigin));
        }
        m_activeModel->performAllBonesTransform();
        break;
    }
    case IModelObjectSelection::kEditingTypeJoint: {
        for (MutableOffsetJointList::const_iterator it = m_joints.begin(), end = m_joints.end(); it != end; ++it) {
            const nanoem_model_joint_t *jointPtr = nanoemMutableModelJointGetOriginObject(it->first);
            const Vector4 newOrigin(delta * Vector4(glm::make_vec3(nanoemModelJointGetOrigin(jointPtr)), 1));
            nanoemMutableModelJointSetOrigin(it->first, glm::value_ptr(newOrigin));
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeRigidBody: {
        for (MutableOffsetRigidBodyList::const_iterator it = m_rigidBodies.begin(), end = m_rigidBodies.end();
             it != end; ++it) {
            const nanoem_model_rigid_body_t *rigidBodyPtr = nanoemMutableModelRigidBodyGetOriginObject(it->first);
            const Vector4 newOrigin(delta * Vector4(glm::make_vec3(nanoemModelRigidBodyGetOrigin(rigidBodyPtr)), 1));
            nanoemMutableModelRigidBodySetOrigin(it->first, glm::value_ptr(newOrigin));
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeFace:
    case IModelObjectSelection::kEditingTypeMaterial:
    case IModelObjectSelection::kEditingTypeSoftBody:
    case IModelObjectSelection::kEditingTypeVertex: {
        for (MutableOffsetVertexList::const_iterator it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
            const nanoem_model_vertex_t *vertexPtr = nanoemMutableModelVertexGetOriginObject(it->first);
            const Vector4 newOrigin(delta * Vector4(glm::make_vec3(nanoemModelVertexGetOrigin(vertexPtr)), 1));
            nanoemMutableModelVertexSetOrigin(it->first, glm::value_ptr(newOrigin));
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
            const Vector4 newOrigin(delta * Vector4(Vector3(it->second), 1));
            nanoemMutableModelBoneSetOrigin(it->first, glm::value_ptr(newOrigin));
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeJoint: {
        for (MutableOffsetJointList::const_iterator it = m_joints.begin(), end = m_joints.end(); it != end; ++it) {
            const Vector4 newOrigin(delta * Vector4(Vector3(it->second), 1));
            nanoemMutableModelJointSetOrigin(it->first, glm::value_ptr(newOrigin));
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeRigidBody: {
        for (MutableOffsetRigidBodyList::const_iterator it = m_rigidBodies.begin(), end = m_rigidBodies.end();
             it != end; ++it) {
            const Vector4 newOrigin(delta * Vector4(Vector3(it->second), 1));
            nanoemMutableModelRigidBodySetOrigin(it->first, glm::value_ptr(newOrigin));
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeFace:
    case IModelObjectSelection::kEditingTypeMaterial:
    case IModelObjectSelection::kEditingTypeSoftBody:
    case IModelObjectSelection::kEditingTypeVertex: {
        for (MutableOffsetVertexList::const_iterator it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
            const Vector4 newOrigin(delta * Vector4(Vector3(it->second), 1));
            nanoemMutableModelVertexSetOrigin(it->first, glm::value_ptr(newOrigin));
        }
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
            nanoemMutableModelBoneSetOrigin(it->first, glm::value_ptr(it->second));
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeJoint: {
        for (MutableOffsetJointList::const_iterator it = m_joints.begin(), end = m_joints.end(); it != end; ++it) {
            nanoemMutableModelJointSetOrigin(it->first, glm::value_ptr(it->second));
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeRigidBody: {
        for (MutableOffsetRigidBodyList::const_iterator it = m_rigidBodies.begin(), end = m_rigidBodies.end();
             it != end; ++it) {
            nanoemMutableModelRigidBodySetOrigin(it->first, glm::value_ptr(it->second));
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeFace:
    case IModelObjectSelection::kEditingTypeMaterial:
    case IModelObjectSelection::kEditingTypeSoftBody:
    case IModelObjectSelection::kEditingTypeVertex: {
        for (MutableOffsetVertexList::const_iterator it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
            nanoemMutableModelVertexSetOrigin(it->first, glm::value_ptr(it->second));
        }
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
