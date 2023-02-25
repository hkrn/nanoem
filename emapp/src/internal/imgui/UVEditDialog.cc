/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/UVEditDialog.h"

#include "emapp/Constants.h"
#include "emapp/IImageView.h"
#include "emapp/Model.h"
#include "emapp/StringUtils.h"
#include "emapp/model/Vertex.h"
#include "emapp/private/CommonInclude.h"

#include "glm/gtx/matrix_decompose.hpp"
#include "imguizmo/ImGuizmo.h"

namespace nanoem {
namespace internal {
namespace imgui {

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
UVEditDialog::Selector::select(nanoem_model_vertex_t *vertex, ImVec2 &pos)
{
    const nanoem_f32_t *t0 = nanoemModelVertexGetTexCoord(vertex);
    bool selected = false;
    pos = ImVec2(m_offset.x + m_size.x * Inline::fract(t0[0]), m_offset.y + m_size.y * Inline::fract(t0[1]));
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
    : m_pivotMatrix(glm::translate(Constants::kIdentity, Vector3(0.5f)))
    , m_initialPivotMatrix(Constants::kIdentity)
    , m_transformMatrix(Constants::kIdentity)
    , m_operation(kOperationTypeSelect)
    , m_dragging(false)
{
}

void
UVEditDialog::State::begin()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (VertexSet::const_iterator it = m_vertexSet.begin(), end = m_vertexSet.end(); it != end; ++it) {
        nanoem_model_vertex_t *vertexPtr = *it;
        m_mutableVertices.push_back(nanoemMutableModelVertexCreateAsReference(vertexPtr, &status));
    }
    m_initialPivotMatrix = m_pivotMatrix;
    m_dragging = true;
}

void
UVEditDialog::State::transform(const Matrix4x4 &delta, Model *activeModel)
{
    Vector4 perspective;
    Vector3 scale(1), translation(0), skew;
    Quaternion orientation;
    glm::decompose(delta, scale, orientation, translation, skew, perspective);
    if (m_operation == kOperationTypeTranslate) {
        for (MutableVertexList::const_iterator it = m_mutableVertices.begin(), end = m_mutableVertices.end(); it != end;
             ++it) {
            const nanoem_model_vertex_t *vertexPtr = nanoemMutableModelVertexGetOriginObject(*it);
            Vector4 newTexCoord(glm::make_vec4(nanoemModelVertexGetTexCoord(vertexPtr)));
            newTexCoord.x += translation.x;
            newTexCoord.y -= translation.y;
            nanoemMutableModelVertexSetTexCoord(*it, glm::value_ptr(newTexCoord));
            model::Vertex *vertex = model::Vertex::cast(vertexPtr);
            vertex->m_simd.m_texcoord = bx::simd_ld(glm::value_ptr(newTexCoord));
        }
    }
    else if (m_operation == kOperationTypeRotate) {
        const Vector2 position(m_initialPivotMatrix[3]), offset(position.x, 1 - position.y);
        const nanoem_f32_t angle = glm::angle(orientation) * glm::sign(glm::axis(orientation).z), c = glm::cos(angle),
                           s = glm::sin(angle);
        for (MutableVertexList::const_iterator it = m_mutableVertices.begin(), end = m_mutableVertices.end(); it != end;
             ++it) {
            const nanoem_model_vertex_t *vertexPtr = nanoemMutableModelVertexGetOriginObject(*it);
            Vector4 newTexCoord(glm::make_vec4(nanoemModelVertexGetTexCoord(vertexPtr)));
            newTexCoord.x -= offset.x;
            newTexCoord.y -= offset.y;
            const nanoem_f32_t x = newTexCoord.x, y = newTexCoord.y;
            newTexCoord.x = x * c + y * s;
            newTexCoord.y = x * -s + y * c;
            newTexCoord.x += offset.x;
            newTexCoord.y += offset.y;
            nanoemMutableModelVertexSetTexCoord(*it, glm::value_ptr(newTexCoord));
            model::Vertex *vertex = model::Vertex::cast(vertexPtr);
            vertex->m_simd.m_texcoord = bx::simd_ld(glm::value_ptr(newTexCoord));
        }
    }
    else if (m_operation == kOperationTypeScale) {
        const Vector2 position(m_initialPivotMatrix[3]), offset(position.x, 1 - position.y);
        for (MutableVertexList::const_iterator it = m_mutableVertices.begin(), end = m_mutableVertices.end(); it != end;
             ++it) {
            const nanoem_model_vertex_t *vertexPtr = nanoemMutableModelVertexGetOriginObject(*it);
            Vector4 newTexCoord(glm::make_vec4(nanoemModelVertexGetTexCoord(vertexPtr)));
            newTexCoord.x -= offset.x;
            newTexCoord.y -= offset.y;
            newTexCoord.x *= scale.x;
            newTexCoord.y *= scale.y;
            newTexCoord.x += offset.x;
            newTexCoord.y += offset.y;
            nanoemMutableModelVertexSetTexCoord(*it, glm::value_ptr(newTexCoord));
            model::Vertex *vertex = model::Vertex::cast(vertexPtr);
            vertex->m_simd.m_texcoord = bx::simd_ld(glm::value_ptr(newTexCoord));
        }
    }
    activeModel->markStagingVertexBufferDirty();
    activeModel->updateStagingVertexBuffer();
}

void
UVEditDialog::State::commit()
{
    for (MutableVertexList::const_iterator it = m_mutableVertices.begin(), end = m_mutableVertices.end(); it != end;
         ++it) {
        nanoemMutableModelVertexDestroy(*it);
    }
    m_mutableVertices.clear();
    m_dragging = false;
}

bool
UVEditDialog::State::isDragging() const NANOEM_DECL_NOEXCEPT
{
    return m_dragging;
}

UVEditDialog::OperationType
UVEditDialog::State::operation() const NANOEM_DECL_NOEXCEPT
{
    return m_operation;
}

void
UVEditDialog::State::setOperation(OperationType value)
{
    m_operation = value;
    if (value != kOperationTypeSelect) {
        if (m_vertexSet.empty()) {
            m_pivotMatrix = glm::translate(Constants::kIdentity, Vector3(0.5f));
        }
        else {
            Vector2 min(FLT_MAX), max(-FLT_MAX), pivot;
            for (VertexSet::const_iterator it = m_vertexSet.begin(), end = m_vertexSet.end(); it != end; ++it) {
                const nanoem_model_vertex_t *vertexPtr = *it;
                const Vector2 texCoord(glm::make_vec2(nanoemModelVertexGetTexCoord(vertexPtr)));
                min = glm::min(min, texCoord);
                max = glm::max(max, texCoord);
            }
            pivot = (min + max) * 0.5f;
            pivot.y = 1.0f - pivot.y;
            m_pivotMatrix = glm::translate(Constants::kIdentity, Vector3(pivot, 0));
        }
    }
}

void
UVEditDialog::drawImage(const IImageView *image, nanoem_f32_t scaleFactor)
{
    const ImTextureID textureID = reinterpret_cast<ImTextureID>(image->handle().id);
    const ImVec2 itemSize(calcExpandedImageSize(image->description(), scaleFactor));
    ImGui::ImageButton(nullptr, textureID, itemSize, ImVec2(0, 0), ImVec2(1, 1));
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
        nanoem_model_vertex_t *v0 = vertices[vertexIndices[i]], *v1 = vertices[vertexIndices[i + 1]],
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
    bool visible = project->isModelEditingEnabled();
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
        const OperationType operation(m_selectionState.operation());
        if (ImGui::RadioButton("Select", operation == kOperationTypeSelect)) {
            m_selectionState.setOperation(kOperationTypeSelect);
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Translate", operation == kOperationTypeTranslate)) {
            m_selectionState.setOperation(kOperationTypeTranslate);
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", operation == kOperationTypeRotate)) {
            m_selectionState.setOperation(kOperationTypeRotate);
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", operation == kOperationTypeScale)) {
            m_selectionState.setOperation(kOperationTypeScale);
        }
        ImGui::PopItemWidth();
        ImGui::Separator();
        ImVec2 size(ImGui::GetContentRegionAvail());
        ImGui::BeginChild("image", size, true, ImGuiWindowFlags_HorizontalScrollbar);
        const ImVec2 itemOffset(ImGui::GetCursorScreenPos());
        if (operation == kOperationTypeSelect) {
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
            const Matrix4x4 view(glm::lookAt(-Constants::kUnitZ, Constants::kZeroV3, Constants::kUnitY)),
                projection(glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f));
            Matrix4x4 delta;
            if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), op, ImGuizmo::WORLD,
                    glm::value_ptr(m_selectionState.m_pivotMatrix), glm::value_ptr(delta))) {
                m_selectionState.isDragging() ? m_selectionState.transform(delta, m_activeModel)
                                              : m_selectionState.begin();
                m_selectionState.m_transformMatrix *= delta;
            }
            else if (!ImGuizmo::IsUsing()) {
                m_selectionState.commit();
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
