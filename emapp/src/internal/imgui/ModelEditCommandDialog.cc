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
#include "imguizmo/ImGuizmo.h"

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
const nanoem_f32_t UVEditDialog::kMinimumHeight = 350;

UVEditDialog::Selector::Selector(
    const Vector4SI32 &region, const ImVec2 &itemOffset, const ImVec2 &itemSize, VertexSet *vertexSet, bool modifiable)
    : m_vertexSet(vertexSet)
    , m_region(region)
    , m_offset(itemOffset)
    , m_size(itemSize)
    , m_modifiable(modifiable)
{
}

bool
UVEditDialog::Selector::select(const nanoem_model_vertex_t *vertex, ImVec2 &pos)
{
    const nanoem_f32_t *t0 = nanoemModelVertexGetTexCoord(vertex);
    bool selected = false;
    pos = ImVec2(m_offset.x + m_size.x * t0[0], m_offset.y + m_size.y * t0[1]);
    if (m_vertexSet->find(vertex) != m_vertexSet->end()) {
        selected = true;
    }
    else if (Inline::intersectsRectPoint(m_region, Vector2SI32(pos.x, pos.y))) {
        if (m_modifiable) {
            m_vertexSet->insert(vertex);
        }
        selected = true;
    }
    return selected;
}

UVEditDialog::State::State()
    : m_matrix(Constants::kIdentity)
    , m_operation(kOperationTypeSelect)
    , m_dragging(false)
{
}

void
UVEditDialog::drawImage(const IImageView *image, nanoem_f32_t scaleFactor)
{
    const ImTextureID textureID = reinterpret_cast<ImTextureID>(image->handle().id);
    const ImVec2 itemSize(calcExpandedImageSize(image->description(), scaleFactor));
    ImGui::ImageButton(textureID, itemSize, ImVec2(0, 0), ImVec2(1, 1), 0);
}

void
UVEditDialog::drawDiffuseImageUVMesh(ImDrawList *drawList, const ImVec2 &itemOffset, const ImVec2 &itemSize,
    const nanoem_model_material_t *activeMaterialPtr, Model *activeModel, State *state, bool hovered)
{
    Vector4SI32 region;
    VertexSet emptySet, *vertexSetPtr;
    bool modifiable = false;
    if (state) {
        const ImVec4 &rect = state->m_rect;
        region.x = rect.z > rect.x ? rect.x : rect.z;
        region.y = rect.w > rect.y ? rect.y : rect.w;
        region.z = rect.z > rect.x ? rect.z - rect.x : rect.x - rect.z;
        region.w = rect.w > rect.y ? rect.w - rect.y : rect.y - rect.w;
        modifiable = hovered;
        vertexSetPtr = &state->m_vertexSet;
    }
    else {
        region = Vector4SI32(0);
        vertexSetPtr = &emptySet;
    }
    Selector selector(region, itemOffset, itemSize, vertexSetPtr, modifiable);
    static ImU32 kColorRed(IM_COL32(0xff, 0, 0, 0xff)), kColorYellow(IM_COL32(0xff, 0xff, 0, 0xff));
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
    const nanoem_rsize_t offsetTo = offset + nanoemModelMaterialGetNumVertexIndices(activeMaterialPtr);
    for (nanoem_rsize_t i = offset; i < offsetTo; i += 3) {
        const nanoem_model_vertex_t *v0 = vertices[vertexIndices[i]], *v1 = vertices[vertexIndices[i + 1]],
                                    *v2 = vertices[vertexIndices[i + 2]];
        ImVec2 v0Pos, v1Pos, v2Pos;
        const ImU32 v0Color = selector.select(v0, v0Pos) ? kColorYellow : kColorRed,
                    v1Color = selector.select(v1, v1Pos) ? kColorYellow : kColorRed,
                    v2Color = selector.select(v2, v2Pos) ? kColorYellow : kColorRed;
        drawList->AddLine(v0Pos, v1Pos, v0Color);
        drawList->AddLine(v1Pos, v2Pos, v1Color);
        drawList->AddLine(v2Pos, v0Pos, v2Color);
    }
}

void
UVEditDialog::drawSphereMapImageUVMesh(
    const ImVec2 &itemOffset, const nanoem_model_material_t *activeMaterialPtr, Model *activeModel)
{
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

void
UVEditDialog::squareSizeConstraint(ImGuiSizeCallbackData *data) NANOEM_DECL_NOEXCEPT
{
    data->DesiredSize.x = data->DesiredSize.y = glm::max(data->DesiredSize.x, data->DesiredSize.y);
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
    const nanoem_f32_t height = kMinimumHeight * project->windowDevicePixelRatio();
    if (open(buffer, kIdentifier, &visible, height, ImGuiWindowFlags_None, squareSizeConstraint)) {
        ImGui::PushItemWidth(-1);
        ImGui::SliderFloat("##scale", &m_scaleFactor, 1.0f, 32.0f, "Scale Factor: %.1f", ImGuiSliderFlags_None);
        ImGui::Spacing();
        if (ImGui::RadioButton("Select", m_selectionState.m_operation == kOperationTypeSelect)) {
            m_selectionState.m_operation = kOperationTypeSelect;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Translate", m_selectionState.m_operation == kOperationTypeTranslate)) {
            m_selectionState.m_operation = kOperationTypeTranslate;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", m_selectionState.m_operation == kOperationTypeRotate)) {
            m_selectionState.m_operation = kOperationTypeRotate;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", m_selectionState.m_operation == kOperationTypeScale)) {
            m_selectionState.m_operation = kOperationTypeScale;
        }
        ImGui::PopItemWidth();
        ImGui::Separator();
        ImVec2 size(ImGui::GetContentRegionAvail());
        ImGui::BeginChild("image", size, true, ImGuiWindowFlags_HorizontalScrollbar);
        const ImVec2 itemOffset(ImGui::GetCursorScreenPos());
        if (m_selectionState.m_operation == kOperationTypeSelect) {
            drawImage(diffuseImage, m_scaleFactor);
            bool hovered = ImGui::IsItemHovered();
            const ImVec2 itemSize(ImGui::GetItemRectSize());
            ImDrawList *drawList = ImGui::GetWindowDrawList();
            drawList->PushClipRect(itemOffset, ImVec2(itemOffset.x + itemSize.x, itemOffset.y + itemSize.y), true);
            drawDiffuseImageUVMesh(drawList, itemOffset, itemSize, hovered);
            if (hovered) {
                drawSelectRegion(drawList);
            }
            drawList->PopClipRect();
        }
        else {
            const ImTextureID textureID = reinterpret_cast<ImTextureID>(diffuseImage->handle().id);
            const ImVec2 itemSize(calcExpandedImageSize(diffuseImage->description(), m_scaleFactor));
            ImGui::Image(textureID, itemSize);
            ImDrawList *drawList = ImGui::GetWindowDrawList();
            drawList->PushClipRect(itemOffset, ImVec2(itemOffset.x + itemSize.x, itemOffset.y + itemSize.y), true);
            drawDiffuseImageUVMesh(drawList, itemOffset, itemSize, ImGui::IsItemHovered());
            const OperationType operation = m_selectionState.m_operation;
            ImGuizmo::OPERATION op;
            switch (operation) {
            case kOperationTypeTranslate: {
                op = ImGuizmo::TRANSLATE_X | ImGuizmo::TRANSLATE_Y;
                break;
            }
            case kOperationTypeRotate: {
                op = ImGuizmo::ROTATE_X | ImGuizmo::ROTATE_Y | ImGuizmo::ROTATE_SCREEN;
                break;
            }
            case kOperationTypeScale: {
                op = ImGuizmo::SCALE_X | ImGuizmo::SCALE_Y;
                break;
            }
            default:
                op = ImGuizmo::ROTATE_SCREEN;
                break;
            }
            ImGuizmo::SetDrawlist(drawList);
            ImGuizmo::SetRect(itemOffset.x, itemOffset.y, itemSize.x, itemSize.y);
            ImGuizmo::SetOrthographic(true);
            Matrix4x4 view(Constants::kIdentity), projection(glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f));
            if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), op, ImGuizmo::WORLD,
                    glm::value_ptr(m_selectionState.m_matrix))) {
                m_selectionState.m_dragging = true;
            }
            else if (!ImGuizmo::IsUsing()) {
                m_selectionState.m_dragging = false;
            }
        }
        ImGui::EndChild();
    }
    close();
    return visible;
}

void
UVEditDialog::drawDiffuseImageUVMesh(
    ImDrawList *drawList, const ImVec2 &itemOffset, const ImVec2 &itemSize, bool hovered)
{
    drawDiffuseImageUVMesh(
        drawList, itemOffset, itemSize, m_activeMaterialPtr, m_activeModel, &m_selectionState, hovered);
}

void
UVEditDialog::drawSelectRegion(ImDrawList *drawList)
{
    ImVec4 &rect = m_selectionState.m_rect;
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        rect = ImVec4();
    }
    else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        const ImVec2 pos(ImGui::GetMousePos());
        rect.z = pos.x;
        rect.w = pos.y;
        if (!ImGui::GetIO().KeyCtrl) {
            m_selectionState.m_vertexSet.clear();
        }
    }
    else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        const ImVec2 pos(ImGui::GetMousePos());
        rect.x = rect.z = pos.x;
        rect.y = rect.w = pos.y;
        if (!ImGui::GetIO().KeyCtrl) {
            m_selectionState.m_vertexSet.clear();
        }
    }
    drawList->AddRectFilled(ImVec2(rect.x, rect.y), ImVec2(rect.z, rect.w), IM_COL32(0xff, 0, 0, 0x7f));
    drawList->AddRect(ImVec2(rect.x, rect.y), ImVec2(rect.z, rect.w), IM_COL32(0xff, 0, 0, 0xff));
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
