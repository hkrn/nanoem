/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/GizmoController.h"

#include "emapp/ICamera.h"
#include "emapp/IModelObjectSelection.h"
#include "emapp/Model.h"
#include "emapp/Project.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/matrix_query.hpp"
#include "imgui/imgui.h"
#include "imguizmo/ImGuizmo.h"

namespace nanoem {
namespace internal {
namespace imgui {
namespace {

struct GizmoUtils : private NonCopyable {
    static ImGuizmo::OPERATION operation(const Model *activeModel) NANOEM_DECL_NOEXCEPT;
    static ImGuizmo::MODE mode(const Model *activeModel) NANOEM_DECL_NOEXCEPT;
    static Vector4 toVector4(const Matrix4x4 &delta, const nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT;
};

ImGuizmo::OPERATION
GizmoUtils::operation(const Model *activeModel) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(activeModel, "model must NOT be null");
    ImGuizmo::OPERATION op;
    switch (activeModel->gizmoOperationType()) {
    case Model::kGizmoOperationTypeTranslate:
    default:
        op = ImGuizmo::TRANSLATE;
        break;
    case Model::kGizmoOperationTypeRotate:
        op = ImGuizmo::ROTATE;
        break;
    case Model::kGizmoOperationTypeScale:
        op = ImGuizmo::SCALE;
        break;
    }
    return op;
}

ImGuizmo::MODE
GizmoUtils::mode(const Model *activeModel) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(activeModel, "model must NOT be null");
    ImGuizmo::MODE mode;
    switch (activeModel->gizmoTransformCoordinateType()) {
    case Model::kTransformCoordinateTypeGlobal:
        mode = ImGuizmo::WORLD;
        break;
    case Model::kTransformCoordinateTypeLocal:
    default:
        mode = ImGuizmo::LOCAL;
        break;
    }
    return mode;
}

Vector4
GizmoUtils::toVector4(const Matrix4x4 &delta, const nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT
{
    return delta * Vector4(glm::make_vec3(value), 1);
}

} /* namespace anonymous */

GizmoController::GizmoController()
{
}

GizmoController::~GizmoController()
{
}

void
GizmoController::begin()
{
    ImGuizmo::BeginFrame();
}

bool
GizmoController::draw(ImDrawList *drawList, const ImVec2 &offset, const ImVec2 &size, Project *project)
{
    Matrix4x4 view, projection, delta;
    bool hovered = true;
    ImGuizmo::SetDrawlist(drawList);
    ImGuizmo::SetRect(offset.x, offset.y, size.x, size.y);
    project->activeCamera()->getViewTransform(view, projection);
    Model *activeModel = project->activeModel();
    Matrix4x4 pivotMatrix(activeModel->pivotMatrix());
    if (!glm::isNull(pivotMatrix, Constants::kEpsilon)) {
        ImGuizmo::DrawCubes(glm::value_ptr(view), glm::value_ptr(projection), glm::value_ptr(pivotMatrix), 1);
        if (ImGuizmo::IsOver()) {
            hovered = false;
        }
        if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), GizmoUtils::operation(activeModel),
                GizmoUtils::mode(activeModel), glm::value_ptr(pivotMatrix), glm::value_ptr(delta))) {
            activeModel->setPivotMatrix(pivotMatrix);
            applyDeltaTransform(delta, activeModel);
        }
    }
    return hovered;
}

void
GizmoController::applyDeltaTransform(const Matrix4x4 &delta, Model *activeModel)
{
    typedef tinystl::unordered_map<const nanoem_model_material_t *, nanoem_rsize_t, TinySTLAllocator> MaterialOffsetMap;
    const IModelObjectSelection *selection = activeModel->selection();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    bool markStagingVertexBufferDirty = false;
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
            const Vector4 newOrigin(GizmoUtils::toVector4(delta, nanoemModelBoneGetOrigin(bonePtr)));
            nanoemMutableModelBoneSetOrigin(item, glm::value_ptr(newOrigin));
            nanoemMutableModelBoneDestroy(item);
        }
        activeModel->performAllBonesTransform();
        break;
    }
    case IModelObjectSelection::kEditingTypeFace: {
        const IModelObjectSelection::VertexIndexSet selectedBoneSet(selection->allVertexIndexSet());
        nanoem_rsize_t numVertices;
        nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(activeModel->data(), &numVertices);
        for (IModelObjectSelection::VertexIndexSet::const_iterator it = selectedBoneSet.begin(),
                                                                   end = selectedBoneSet.end();
             it != end; ++it) {
            const nanoem_u32_t vertexIndex = *it;
            nanoem_model_vertex_t *mutableVertexPtr = vertices[vertexIndex];
            nanoem_mutable_model_vertex_t *item = nanoemMutableModelVertexCreateAsReference(mutableVertexPtr, &status);
            const Vector4 newOrigin(GizmoUtils::toVector4(delta, nanoemModelVertexGetOrigin(mutableVertexPtr)));
            nanoemMutableModelVertexSetOrigin(item, glm::value_ptr(newOrigin));
            nanoemMutableModelVertexDestroy(item);
            model::Vertex *vertex = model::Vertex::cast(mutableVertexPtr);
            vertex->m_simd.m_origin = bx::simd_ld(glm::value_ptr(newOrigin));
        }
        markStagingVertexBufferDirty = true;
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
            const Vector4 newOrigin(GizmoUtils::toVector4(delta, nanoemModelJointGetOrigin(jointPtr)));
            nanoemMutableModelJointSetOrigin(item, glm::value_ptr(newOrigin));
            nanoemMutableModelJointDestroy(item);
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
                    const Vector4 newOrigin(GizmoUtils::toVector4(delta, nanoemModelVertexGetOrigin(mutableVertexPtr)));
                    nanoemMutableModelVertexSetOrigin(item, glm::value_ptr(newOrigin));
                    nanoemMutableModelVertexDestroy(item);
                    model::Vertex *vertex = model::Vertex::cast(mutableVertexPtr);
                    vertex->m_simd.m_origin = bx::simd_ld(glm::value_ptr(newOrigin));
                }
            }
        }
        markStagingVertexBufferDirty = true;
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
            const Vector4 newOrigin(GizmoUtils::toVector4(delta, nanoemModelRigidBodyGetOrigin(rigidBodyPtr)));
            nanoemMutableModelRigidBodySetOrigin(item, glm::value_ptr(newOrigin));
            nanoemMutableModelRigidBodyDestroy(item);
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
                    const Vector4 newOrigin(GizmoUtils::toVector4(delta, nanoemModelVertexGetOrigin(mutableVertexPtr)));
                    nanoemMutableModelVertexSetOrigin(item, glm::value_ptr(newOrigin));
                    nanoemMutableModelVertexDestroy(item);
                    model::Vertex *vertex = model::Vertex::cast(mutableVertexPtr);
                    vertex->m_simd.m_origin = bx::simd_ld(glm::value_ptr(newOrigin));
                }
            }
        }
        markStagingVertexBufferDirty = true;
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
            const Vector4 newOrigin(GizmoUtils::toVector4(delta, nanoemModelVertexGetOrigin(vertexPtr)));
            nanoemMutableModelVertexSetOrigin(item, glm::value_ptr(newOrigin));
            nanoemMutableModelVertexDestroy(item);
            model::Vertex *vertex = model::Vertex::cast(vertexPtr);
            vertex->m_simd.m_origin = bx::simd_ld(glm::value_ptr(newOrigin));
        }
        markStagingVertexBufferDirty = true;
        break;
    }
    default:
        break;
    }
    if (markStagingVertexBufferDirty) {
        activeModel->markStagingVertexBufferDirty();
        activeModel->updateStagingVertexBuffer();
    }
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
