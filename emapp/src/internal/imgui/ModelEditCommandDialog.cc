/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/ModelEditCommandDialog.h"

#include "emapp/IImageView.h"
#include "emapp/Model.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#include "glm/gtc/type_ptr.hpp"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const ModelEditCommandDialog::kIdentifier = "dialog.model.edit";

void
ModelEditCommandDialog::beforeToggleEditingMode(
    IModelObjectSelection::EditingType editingType, Model *activeModel, Project *project)
{
    switch (editingType) {
    case IModelObjectSelection::kEditingTypeVertex: {
        activeModel->setShowAllVertexPoints(false);
        break;
    }
    case IModelObjectSelection::kEditingTypeFace: {
        activeModel->setShowAllVertexFaces(false);
        break;
    }
    case IModelObjectSelection::kEditingTypeBone: {
        activeModel->setShowAllBones(false);
        project->setEditingMode(Project::kEditingModeSelect);
        break;
    }
    case IModelObjectSelection::kEditingTypeMorph: {
        if (Motion *motion = project->resolveMotion(activeModel)) {
            activeModel->synchronizeMotion(
                motion, project->currentLocalFrameIndex(), 0, PhysicsEngine::kSimulationTimingBefore);
            activeModel->resetAllVertices();
            activeModel->deformAllMorphs(false);
            activeModel->markStagingVertexBufferDirty();
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeRigidBody: {
        activeModel->setShowAllRigidBodies(false);
        break;
    }
    case IModelObjectSelection::kEditingTypeJoint: {
        activeModel->setShowAllJoints(false);
        break;
    }
    case IModelObjectSelection::kEditingTypeSoftBody: {
        activeModel->setShowAllVertexPoints(false);
        break;
    }
    default:
        break;
    }
}

void
ModelEditCommandDialog::afterToggleEditingMode(
    IModelObjectSelection::EditingType editingType, Model *activeModel, Project *project)
{
    switch (editingType) {
    case IModelObjectSelection::kEditingTypeNone: {
        activeModel->setPivotMatrix(Matrix4x4(0));
        activeModel->selection()->setEditingType(editingType);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case IModelObjectSelection::kEditingTypeVertex: {
        activeModel->setShowAllVertexPoints(true);
        activeModel->selection()->setEditingType(editingType);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case IModelObjectSelection::kEditingTypeFace: {
        activeModel->setShowAllVertexFaces(true);
        activeModel->selection()->setEditingType(editingType);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case IModelObjectSelection::kEditingTypeMaterial: {
        activeModel->selection()->setEditingType(editingType);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case IModelObjectSelection::kEditingTypeBone: {
        activeModel->selection()->setEditingType(editingType);
        activeModel->setShowAllBones(true);
        project->setEditingMode(Project::kEditingModeSelect);
        break;
    }
    case IModelObjectSelection::kEditingTypeRigidBody: {
        activeModel->setShowAllRigidBodies(true);
        activeModel->selection()->setEditingType(editingType);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case IModelObjectSelection::kEditingTypeJoint: {
        activeModel->setShowAllJoints(true);
        activeModel->selection()->setEditingType(editingType);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case IModelObjectSelection::kEditingTypeSoftBody: {
        activeModel->setShowAllVertexPoints(true);
        activeModel->selection()->setEditingType(editingType);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    default:
        break;
    }
}

ModelEditCommandDialog::ModelEditCommandDialog(Model *model, BaseApplicationService *applicationPtr)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_activeModel(model)
{
}

bool
ModelEditCommandDialog::draw(Project *project)
{
    bool visible = true;
    Model *currentActiveModel = project->activeModel();
    if (currentActiveModel != m_activeModel) {
        project->setEditingMode(Project::kEditingModeNone);
        m_activeModel = currentActiveModel;
        visible = currentActiveModel != nullptr;
    }
    const nanoem_f32_t height = ImGui::GetFrameHeightWithSpacing() * 3;
    if (open(tr("Model Commands"), kIdentifier, &visible, height)) {
        if (ImGui::CollapsingHeader("Gizmo")) {
            ImGui::Text("Operation Type");
            addGizmoOperationButton("Translate", Model::kGizmoOperationTypeTranslate);
            ImGui::SameLine();
            addGizmoOperationButton("Rotate", Model::kGizmoOperationTypeRotate);
            ImGui::SameLine();
            addGizmoOperationButton("Scale", Model::kGizmoOperationTypeScale);
            addSeparator();
            ImGui::Text("Coordinate Type");
            addGizmoCoordinationButton("Global", Model::kTransformCoordinateTypeGlobal);
            ImGui::SameLine();
            addGizmoCoordinationButton("Local", Model::kTransformCoordinateTypeLocal);
        }
        if (ImGui::CollapsingHeader("Selection")) {
            addSelectionButton("(none)", IModelObjectSelection::kEditingTypeNone, project);
            addSelectionButton("Vertex", IModelObjectSelection::kEditingTypeVertex, project);
            addSelectionButton("Face", IModelObjectSelection::kEditingTypeFace, project);
            addSelectionButton("Material", IModelObjectSelection::kEditingTypeMaterial, project);
            addSelectionButton("Bone", IModelObjectSelection::kEditingTypeBone, project);
            addSelectionButton("Rigid Body", IModelObjectSelection::kEditingTypeRigidBody, project);
            addSelectionButton("Joint", IModelObjectSelection::kEditingTypeJoint, project);
        }
    }
    close();
    return visible;
}

void
ModelEditCommandDialog::addGizmoOperationButton(const char *text, Model::GizmoOperationType type)
{
    Model::GizmoOperationType op = m_activeModel->gizmoOperationType();
    if (ImGui::RadioButton(text, op == type)) {
        m_activeModel->setGizmoOperationType(type);
    }
}

void
ModelEditCommandDialog::addGizmoCoordinationButton(const char *text, Model::TransformCoordinateType type)
{
    Model::TransformCoordinateType coord = m_activeModel->gizmoTransformCoordinateType();
    if (ImGui::RadioButton(text, coord == type)) {
        m_activeModel->setGizmoTransformCoordinateType(type);
    }
}

void
ModelEditCommandDialog::addSelectionButton(const char *text, IModelObjectSelection::EditingType type, Project *project)
{
    const IModelObjectSelection *selection = m_activeModel->selection();
    const IModelObjectSelection::EditingType editingType = selection->editingType();
    if (ImGui::RadioButton(text, editingType == type)) {
        beforeToggleEditingMode(editingType, m_activeModel, project);
        afterToggleEditingMode(type, m_activeModel, project);
    }
}

const char *const UVEditDialog::kIdentifier = "dialog.uv.edit";

void
UVEditDialog::drawDiffuseImage(const IImageView *image, const nanoem_model_material_t *activeMaterialPtr,
    Model *activeModel, nanoem_f32_t scaleFactor, bool displayUVMeshEnabled)
{
    const ImTextureID textureID = reinterpret_cast<ImTextureID>(image->handle().id);
    const ImVec2 itemOffset(ImGui::GetCursorScreenPos());
    ImGui::Image(textureID, calcExpandedImageSize(image->description(), scaleFactor), ImVec2(0, 0), ImVec2(1, 1),
        ImVec4(1, 1, 1, 1), ImGui::ColorConvertU32ToFloat4(ImGuiWindow::kColorBorder));
    if (displayUVMeshEnabled) {
        nanoem_rsize_t numMaterials, numVertices, numVertexIndices, offset = 0;
        const nanoem_model_t *opaque = activeModel->data();
        nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(opaque, &numMaterials);
        nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(opaque, &numVertices);
        const nanoem_u32_t *vertexIndices = nanoemModelGetAllVertexIndices(opaque, &numVertexIndices);
        for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
            const nanoem_model_material_t *material = materials[i];
            if (material == activeMaterialPtr) {
                break;
            }
            offset += nanoemModelMaterialGetNumVertexIndices(material);
        }
        const ImVec2 itemSize(ImGui::GetItemRectSize());
        const nanoem_rsize_t offsetTo = offset + nanoemModelMaterialGetNumVertexIndices(activeMaterialPtr);
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        drawList->PushClipRect(itemOffset, ImVec2(itemOffset.x + itemSize.x, itemOffset.y + itemSize.y), true);
        for (nanoem_rsize_t i = offset; i < offsetTo; i += 3) {
            const nanoem_f32_t *t0 = nanoemModelVertexGetTexCoord(vertices[vertexIndices[i]]),
                               *t1 = nanoemModelVertexGetTexCoord(vertices[vertexIndices[i + 1]]),
                               *t2 = nanoemModelVertexGetTexCoord(vertices[vertexIndices[i + 2]]);
            const ImVec2 v0Pos(itemOffset.x + itemSize.x * t0[0], itemOffset.y + itemSize.y * t0[1]),
                v1Pos(itemOffset.x + itemSize.x * t1[0], itemOffset.y + itemSize.y * t1[1]),
                v2Pos(itemOffset.x + itemSize.x * t2[0], itemOffset.y + itemSize.y * t2[1]);
            drawList->AddLine(v0Pos, v1Pos, IM_COL32(0xff, 0, 0, 0xff));
            drawList->AddLine(v1Pos, v2Pos, IM_COL32(0xff, 0, 0, 0xff));
            drawList->AddLine(v2Pos, v0Pos, IM_COL32(0xff, 0, 0, 0xff));
        }
        drawList->PopClipRect();
    }
}

void
UVEditDialog::drawSphereMapImage(const IImageView *image, const nanoem_model_material_t *activeMaterialPtr,
    Model *activeModel, nanoem_f32_t scaleFactor, bool displayUVMeshEnabled)
{
    const ImTextureID textureID = reinterpret_cast<ImTextureID>(image->handle().id);
    const ImVec2 itemOffset(ImGui::GetCursorScreenPos());
    ImGui::Image(textureID, calcExpandedImageSize(image->description(), scaleFactor), ImVec2(0, 0), ImVec2(1, 1),
        ImVec4(1, 1, 1, 1), ImGui::ColorConvertU32ToFloat4(ImGuiWindow::kColorBorder));
    if (displayUVMeshEnabled) {
        nanoem_rsize_t numMaterials, numVertices, numVertexIndices, offset = 0;
        const nanoem_model_t *opaque = activeModel->data();
        nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(opaque, &numMaterials);
        nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(opaque, &numVertices);
        const nanoem_u32_t *vertexIndices = nanoemModelGetAllVertexIndices(opaque, &numVertexIndices);
        for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
            const nanoem_model_material_t *material = materials[i];
            if (material == activeMaterialPtr) {
                break;
            }
            offset += nanoemModelMaterialGetNumVertexIndices(material);
        }
        const ImVec2 itemSize(ImGui::GetItemRectSize());
        const nanoem_rsize_t offsetTo = offset + nanoemModelMaterialGetNumVertexIndices(activeMaterialPtr);
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        drawList->PushClipRect(itemOffset, ImVec2(itemOffset.x + itemSize.x, itemOffset.y + itemSize.y), true);
        for (nanoem_rsize_t i = offset; i < offsetTo; i += 3) {
            const Vector3 n0(
                glm::normalize(glm::make_vec3(nanoemModelVertexGetNormal(vertices[vertexIndices[i]]))) * Vector3(0.5f) +
                Vector3(0.5f)),
                n1(glm::normalize(glm::make_vec3(nanoemModelVertexGetNormal(vertices[vertexIndices[i + 1]]))) *
                        Vector3(0.5f) +
                    Vector3(0.5f)),
                n2(glm::normalize(glm::make_vec3(nanoemModelVertexGetNormal(vertices[vertexIndices[i + 2]]))) *
                        Vector3(0.5f) +
                    Vector3(0.5f));
            const ImVec2 n0Pos(itemOffset.x + itemSize.x * n0.x, itemOffset.y + itemSize.y * n0.y),
                n1Pos(itemOffset.x + itemSize.x * n1.x, itemOffset.y + itemSize.y * n1.y),
                n2Pos(itemOffset.x + itemSize.x * n2.x, itemOffset.y + itemSize.y * n2.y);
            drawList->AddLine(n0Pos, n1Pos, IM_COL32(0xff, 0, 0, 0xff));
            drawList->AddLine(n1Pos, n2Pos, IM_COL32(0xff, 0, 0, 0xff));
            drawList->AddLine(n2Pos, n0Pos, IM_COL32(0xff, 0, 0, 0xff));
        }
        drawList->PopClipRect();
    }
}

UVEditDialog::UVEditDialog(
    const nanoem_model_material_t *materialPtr, Model *activeModel, BaseApplicationService *applicationPtr)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_activeModel(activeModel)
    , m_activeMaterialPtr(materialPtr)
    , m_scaleFactor(1.0f)
{
}

bool
UVEditDialog::draw(Project *project)
{
    bool visible = true;
    const nanoem_f32_t height = ImGui::GetFrameHeightWithSpacing() * 3;
    Model *currentActiveModel = project->activeModel();
    const model::Material *material = model::Material::cast(m_activeMaterialPtr);
    const IImageView *diffuseImage = material ? material->diffuseImage() : nullptr;
    if (currentActiveModel != m_activeModel) {
        m_activeModel = currentActiveModel;
        visible = currentActiveModel != nullptr && diffuseImage != nullptr;
    }
    char buffer[Inline::kNameStackBufferSize] = { 0 };
    if (diffuseImage) {
        StringUtils::format(
            buffer, sizeof(buffer), "%s - %s", diffuseImage->filenameConstString(), m_activeModel->nameConstString());
    }
    if (open(buffer, kIdentifier, &visible, ImVec2(500, 500), ImGuiWindowFlags_None)) {
        ImGui::PushItemWidth(-1);
        ImGui::SliderFloat("", &m_scaleFactor, 1.0f, 32.0f, "Scale Factor: %.1f", ImGuiSliderFlags_None);
        ImGui::PopItemWidth();
        ImVec2 size(ImGui::GetContentRegionAvail());
        ImGui::BeginChild("uv.edit.image", size, true, ImGuiWindowFlags_HorizontalScrollbar);
        drawDiffuseImage(diffuseImage, m_activeMaterialPtr, m_activeModel, m_scaleFactor, true);
        ImGui::EndChild();
    }
    close();
    return visible;
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
