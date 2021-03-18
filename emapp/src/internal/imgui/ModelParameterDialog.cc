/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/ModelParameterDialog.h"

#include "emapp/Error.h"
#include "emapp/IImageView.h"
#include "emapp/IModelObjectSelection.h"
#include "emapp/Progress.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace internal {
namespace imgui {

static const nanoem_u8_t kNewObjectPrefixName[] = { 0xe6, 0x96, 0xb0, 0xe8, 0xa6, 0x8f, 0 };

const char *const ModelParameterDialog::kIdentifier = "dialog.project.model";
const nanoem_f32_t ModelParameterDialog::kMinimumWindowWidth = 600;
const int ModelParameterDialog::kWindowFrameHeightRowCount = 17;
const int ModelParameterDialog::kInnerItemListFrameHeightRowCount = 10;

void
ModelParameterDialog::formatVertexText(char *buffer, nanoem_rsize_t size, const nanoem_model_vertex_t *vertexPtr)
{
    const nanoem_f32_t *origin = nanoemModelVertexGetOrigin(vertexPtr);
    int offset = nanoemModelObjectGetIndex(nanoemModelVertexGetModelObject(vertexPtr)) + 1;
    StringUtils::format(buffer, size, "%d (%.2f, %.2f, %.2f)", offset, origin[0], origin[1], origin[2]);
}

ModelParameterDialog::ModelParameterDialog(
    Model *model, Project *project, BaseApplicationService *applicationPtr, ImGuiWindow *parent)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_parent(parent)
    , m_activeModel(model)
    , m_language(project->castLanguage())
    , m_tabType(kTabTypeFirstEnum)
    , m_vertexIndex(0)
    , m_faceIndex(0)
    , m_materialIndex(0)
    , m_boneIndex(0)
    , m_constraintJointIndex(0)
    , m_constraintJointCandidateIndex(NANOEM_RSIZE_MAX)
    , m_morphIndex(0)
    , m_morphItemIndex(0)
    , m_morphItemCandidateBoneIndex(NANOEM_RSIZE_MAX)
    , m_morphItemCandidateMaterialIndex(NANOEM_RSIZE_MAX)
    , m_morphItemCandidateMorphIndex(NANOEM_RSIZE_MAX)
    , m_morphItemCandidateRigidBodyIndex(NANOEM_RSIZE_MAX)
    , m_labelIndex(0)
    , m_labelItemIndex(0)
    , m_labelItemCandidateBoneIndex(NANOEM_RSIZE_MAX)
    , m_labelItemCandidateMorphIndex(NANOEM_RSIZE_MAX)
    , m_rigidBodyIndex(0)
    , m_jointIndex(0)
    , m_softBodyIndex(0)
    , m_editingMode(project->editingMode())
    , m_showAllVertexPoints(model->isShowAllVertexPoints())
    , m_showAllVertexFaces(model->isShowAllVertexFaces())
    , m_showAllBones(model->isShowAllBones())
    , m_showAllRigidBodies(model->isShowAllRigidBodies())
    , m_showAllJoints(model->isShowAllJoints())
    , m_showAllSoftBodies(false)
{
}

bool
ModelParameterDialog::draw(Project *project)
{
    String windowTitle;
    nanoem_f32_t width = kMinimumWindowWidth * project->windowDevicePixelRatio();
    bool visible = true;
    Model *currentActiveModel = project->activeModel();
    if (currentActiveModel != m_activeModel) {
        m_activeModel = currentActiveModel;
        visible = currentActiveModel != nullptr;
    }
    if (m_activeModel) {
        StringUtils::format(
            windowTitle, "%s - %s", tr("nanoem.gui.window.model.title"), m_activeModel->nameConstString());
    }
    if (open(windowTitle.c_str(), kIdentifier, &visible,
            ImVec2(width, ImGui::GetFrameHeightWithSpacing() * kWindowFrameHeightRowCount), 0)) {
        ImGui::BeginTabBar("tabbar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton);
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.info"))) {
            layoutInformation(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeInfo, project);
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.vertex"))) {
            layoutAllVertices(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeVertex, project);
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.face"))) {
            layoutAllFaces(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeFace, project);
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.material"))) {
            layoutAllMaterials(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeMaterial, project);
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.bone"))) {
            layoutAllBones(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeBone, project);
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.morph"))) {
            layoutAllMorphs(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeMorph, project);
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.label"))) {
            layoutAllLabels(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeLabel, project);
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.rigid-body"))) {
            layoutAllRigidBodies(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeRigidBody, project);
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.joint"))) {
            layoutAllJoints(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeJoint, project);
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.soft-body"))) {
            layoutAllSoftBodies(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeSoftBody, project);
        }
        ImGui::EndTabBar();
    }
    else if (m_activeModel) {
        project->setEditingMode(m_editingMode);
        m_activeModel->setShowAllVertexPoints(m_showAllVertexPoints);
        m_activeModel->setShowAllVertexFaces(m_showAllVertexFaces);
        m_activeModel->setShowAllBones(m_showAllBones);
        m_activeModel->setShowAllRigidBodies(m_showAllRigidBodies);
        m_activeModel->setShowAllJoints(m_showAllJoints);
        toggleTab(kTabTypeInfo, project);
    }
    close();
    project->setModelEditingEnabled(visible);
    return visible;
}

void
ModelParameterDialog::layoutInformation(Project *project)
{
    MutableString nameBuffer, commentBuffer;
    switch (nanoemModelGetFormatType(m_activeModel->data())) {
    case NANOEM_MODEL_FORMAT_TYPE_PMD_1_0: {
        ImGui::Value("Version", 1.0f, "%.1f");
        break;
    }
    case NANOEM_MODEL_FORMAT_TYPE_PMX_2_0: {
        ImGui::Value("Version", 2.0f, "%.1f");
        break;
    }
    case NANOEM_MODEL_FORMAT_TYPE_PMX_2_1: {
        ImGui::Value("Version", 2.1f, "%.1f");
        break;
    }
    default:
        ImGui::TextUnformatted("Version: (Unknown)");
        break;
    }
    ImGui::RadioButton(
        tr("nanoem.gui.window.preference.project.language.japanese"), &m_language, NANOEM_LANGUAGE_TYPE_JAPANESE);
    ImGui::SameLine();
    ImGui::RadioButton(
        tr("nanoem.gui.window.preference.project.language.english"), &m_language, NANOEM_LANGUAGE_TYPE_ENGLISH);
    ImGui::PushItemWidth(-1);
    ImGui::TextUnformatted("Name");
    String name, comment;
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    nanoem_language_type_t language = static_cast<nanoem_language_type_t>(m_language);
    StringUtils::getUtf8String(nanoemModelGetName(m_activeModel->data(), language), factory, name);
    nameBuffer.assign(name.c_str(), name.c_str() + name.size());
    nameBuffer.push_back(0);
    if (ImGui::InputText("##name", nameBuffer.data(), nameBuffer.capacity())) {
        StringUtils::UnicodeStringScope s(factory);
        if (StringUtils::tryGetString(factory, nameBuffer.data(), s)) {
            ScopedMutableModel scoped(m_activeModel);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            nanoemMutableModelSetName(scoped, s.value(), language, &status);
        }
    }
    StringUtils::getUtf8String(nanoemModelGetComment(m_activeModel->data(), language), factory, comment);
    commentBuffer.assign(comment.c_str(), comment.c_str() + comment.size());
    commentBuffer.push_back(0);
    ImGui::TextUnformatted("Comment");
    ImVec2 avail(ImGui::GetContentRegionAvail());
    avail.y -= ImGui::GetFrameHeightWithSpacing();
    if (ImGui::InputTextMultiline("##comment", commentBuffer.data(), commentBuffer.capacity(), avail)) {
        StringUtils::UnicodeStringScope s(factory);
        if (StringUtils::tryGetString(factory, commentBuffer.data(), s)) {
            ScopedMutableModel scoped(m_activeModel);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            nanoemMutableModelSetComment(scoped, s.value(), language, &status);
        }
    }
    ImGui::PopItemWidth();
    ImGui::Columns(4, nullptr, false);
    ImGui::TextUnformatted("Encoding");
    ImGui::NextColumn();
    if (ImGui::BeginCombo("##encoding", selectedCodecType(nanoemModelGetCodecType(m_activeModel->data())))) {
        for (int i = NANOEM_CODEC_TYPE_FIRST_ENUM; i < NANOEM_CODEC_TYPE_MAX_ENUM; i++) {
            nanoem_codec_type_t codec = static_cast<nanoem_codec_type_t>(i);
            if (ImGui::Selectable(selectedCodecType(codec))) {
                ScopedMutableModel scoped(m_activeModel);
                nanoemMutableModelSetCodecType(scoped, codec);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::NextColumn();
    ImGui::TextUnformatted("UVA");
    ImGui::NextColumn();
    int uva = 0;
    if (ImGui::DragInt("##uva", &uva, 0.01f, 0, 4)) {
    }
    else if (ImGui::IsItemDeactivatedAfterEdit()) {
    }
    ImGui::Columns(1);
}

void
ModelParameterDialog::layoutVertexBoneSelection(const char *label, nanoem_model_vertex_t *vertexPtr, nanoem_rsize_t i,
    nanoem_model_bone_t *const *bones, nanoem_rsize_t numBones)
{
    const model::Bone *bone = model::Bone::cast(nanoemModelVertexGetBoneObject(vertexPtr, i));
    if (ImGui::BeginCombo(label, bone ? bone->nameConstString() : "(none)")) {
        for (nanoem_rsize_t j = 0; j < numBones; j++) {
            const nanoem_model_bone_t *bonePtr = bones[j];
            bone = model::Bone::cast(bonePtr);
            if (bone && ImGui::Selectable(bone->nameConstString())) {
                ScopedMutableVertex scoped(vertexPtr, m_activeModel);
                nanoemMutableModelVertexSetBoneObject(scoped, bonePtr, i);
            }
        }
        ImGui::EndCombo();
    }
}

void
ModelParameterDialog::layoutVertexBoneWeights(nanoem_model_vertex_t *vertexPtr, nanoem_rsize_t numItems)
{
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
    for (nanoem_rsize_t i = 0; i < numItems; i++) {
        char label[Inline::kNameStackBufferSize];
        StringUtils::format(label, sizeof(label), "##bone%jd", i);
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
        layoutVertexBoneSelection(label, vertexPtr, i, bones, numBones);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        nanoem_f32_t weight = nanoemModelVertexGetBoneWeight(vertexPtr, i);
        StringUtils::format(label, sizeof(label), "##weight%jd", i);
        if (ImGui::SliderFloat(label, &weight, 0, 1)) {
            ScopedMutableVertex scoped(vertexPtr, m_activeModel);
            nanoemMutableModelVertexSetBoneWeight(scoped, weight, i);
        }
        ImGui::PopItemWidth();
    }
}

void
ModelParameterDialog::layoutAllVertices(Project *project)
{
    nanoem_rsize_t numVertices;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(m_activeModel->data(), &numVertices);
    nanoem_f32_t width = ImGuiWindow::kLeftPaneWidth * project->windowDevicePixelRatio();
    ImGui::BeginChild("left-pane", ImVec2(width, 0), false);
    if (ImGui::Button(reinterpret_cast<const char *>(ImGuiWindow::kFACogs))) {
        ImGui::OpenPopup("vertex-op-menu");
    }
    if (ImGui::BeginPopup("vertex-op-menu")) {
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    ImGui::Text("%lu / %lu", m_vertexIndex + 1, numVertices);
    ImGui::BeginChild(
        "left-pane-inner", ImVec2(width, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing()), true);
    ImGuiListClipper clipper;
    nanoem_model_vertex_t *hoveredVertexPtr = nullptr;
    bool up, down;
    detectUpDown(up, down);
    selectIndex(up, down, numVertices, m_vertexIndex);
    IModelObjectSelection *selection = m_activeModel->selection();
    clipper.Begin(Inline::saturateInt32(numVertices));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            char buffer[Inline::kNameStackBufferSize];
            nanoem_model_vertex_t *vertexPtr = vertices[i];
            formatVertexText(buffer, sizeof(buffer), vertexPtr);
            const bool selected = selection->containsVertex(vertexPtr);
            if (ImGui::Selectable(buffer, selected)) {
                if (ImGui::GetIO().KeyCtrl) {
                    selected ? selection->removeVertex(vertexPtr) : selection->addVertex(vertexPtr);
                }
                else {
                    if (!ImGui::GetIO().KeyShift) {
                        selection->removeAllVertices();
                        m_vertexIndex = i;
                    }
                    selection->addVertex(vertexPtr);
                }
                hoveredVertexPtr = vertexPtr;
            }
            else if ((up || down) && m_vertexIndex == static_cast<nanoem_rsize_t>(i)) {
                ImGui::SetScrollHereY();
                m_vertexIndex = i;
            }
            else if (ImGui::IsItemHovered()) {
                hoveredVertexPtr = vertexPtr;
            }
        }
    }
    selection->setHoveredVertex(hoveredVertexPtr);
    ImGui::EndChild();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAPlus), 0, false)) {
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAMinus), 0, false)) {
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowUp), 0, false)) {
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowDown), 0, false)) {
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("right-pane", ImGui::GetContentRegionAvail());
    if (numVertices > 0 && m_vertexIndex < numVertices) {
        nanoem_model_vertex_t *vertexPtr = vertices[m_vertexIndex];
        layoutVertexPropertyPane(vertexPtr);
    }
    ImGui::EndChild();
}

void
ModelParameterDialog::layoutVertexPropertyPane(nanoem_model_vertex_t *vertexPtr)
{
    ImGui::PushItemWidth(-1);
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.vertex.origin"));
        Vector4 value(glm::make_vec4(nanoemModelVertexGetOrigin(vertexPtr)));
        if (ImGui::InputFloat3("##origin", glm::value_ptr(value))) {
            ScopedMutableVertex scoped(vertexPtr, m_activeModel);
            nanoemMutableModelVertexSetOrigin(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.vertex.normal"));
        Vector4 value(glm::make_vec4(nanoemModelVertexGetNormal(vertexPtr)));
        if (ImGui::InputFloat3("##normal", glm::value_ptr(value))) {
            ScopedMutableVertex scoped(vertexPtr, m_activeModel);
            nanoemMutableModelVertexSetNormal(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.vertex.texcoord"));
        Vector4 value(glm::make_vec4(nanoemModelVertexGetTexCoord(vertexPtr)));
        if (ImGui::InputFloat2("##texcoord", glm::value_ptr(value))) {
            ScopedMutableVertex scoped(vertexPtr, m_activeModel);
            nanoemMutableModelVertexSetTexCoord(scoped, glm::value_ptr(value));
        }
    }
    addSeparator();
    {
        nanoem_rsize_t offset = 0;
        if (ImGui::BeginCombo("##uva", "UVA")) {
            if (ImGui::Selectable("UVA1")) {
                offset = 1;
            }
            if (ImGui::Selectable("UVA2")) {
                offset = 2;
            }
            if (ImGui::Selectable("UVA3")) {
                offset = 3;
            }
            if (ImGui::Selectable("UVA4")) {
                offset = 4;
            }
            ImGui::EndCombo();
        }
        Vector4 value(glm::make_vec4(nanoemModelVertexGetAdditionalUV(vertexPtr, offset)));
        if (ImGui::InputFloat4("##uva-value", glm::value_ptr(value))) {
            ScopedMutableVertex scoped(vertexPtr, m_activeModel);
            nanoemMutableModelVertexSetAdditionalUV(scoped, glm::value_ptr(value), offset);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.vertex.edge"));
        nanoem_f32_t width = nanoemModelVertexGetEdgeSize(vertexPtr);
        if (ImGui::InputFloat("##edge", &width)) {
            ScopedMutableVertex scoped(vertexPtr, m_activeModel);
            nanoemMutableModelVertexSetEdgeSize(scoped, width);
        }
    }
    addSeparator();
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.vertex.type"));
        if (ImGui::BeginCombo("##type", selectedVertexType(nanoemModelVertexGetType(vertexPtr)))) {
            for (int i = NANOEM_MODEL_VERTEX_TYPE_FIRST_ENUM; i < NANOEM_MODEL_VERTEX_TYPE_MAX_ENUM; i++) {
                const nanoem_model_vertex_type_t type = static_cast<nanoem_model_vertex_type_t>(i);
                if (ImGui::Selectable(selectedVertexType(type))) {
                    ScopedMutableVertex scoped(vertexPtr, m_activeModel);
                    nanoemMutableModelVertexSetType(scoped, type);
                }
            }
            ImGui::EndCombo();
        }
        switch (nanoemModelVertexGetType(vertexPtr)) {
        case NANOEM_MODEL_VERTEX_TYPE_BDEF1: {
            nanoem_rsize_t numBones;
            nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
            layoutVertexBoneSelection("##bone", vertexPtr, 0, bones, numBones);
            break;
        }
        case NANOEM_MODEL_VERTEX_TYPE_BDEF2: {
            layoutVertexBoneWeights(vertexPtr, 2);
            break;
        }
        case NANOEM_MODEL_VERTEX_TYPE_BDEF4: {
            layoutVertexBoneWeights(vertexPtr, 4);
            break;
        }
        case NANOEM_MODEL_VERTEX_TYPE_SDEF: {
            layoutVertexBoneWeights(vertexPtr, 2);
            {
                ImGui::TextUnformatted(tr("nanoem.gui.model.edit.vertex.sdef.r0"));
                Vector4 value(glm::make_vec4(nanoemModelVertexGetSdefR0(vertexPtr)));
                if (ImGui::InputFloat3("##sdef.r0", glm::value_ptr(value))) {
                    ScopedMutableVertex scoped(vertexPtr, m_activeModel);
                    nanoemMutableModelVertexSetSdefR0(scoped, glm::value_ptr(value));
                }
            }
            {
                ImGui::TextUnformatted(tr("nanoem.gui.model.edit.vertex.sdef.r1"));
                Vector4 value(glm::make_vec4(nanoemModelVertexGetSdefR1(vertexPtr)));
                if (ImGui::InputFloat3("##sdef.r1", glm::value_ptr(value))) {
                    ScopedMutableVertex scoped(vertexPtr, m_activeModel);
                    nanoemMutableModelVertexSetSdefR1(scoped, glm::value_ptr(value));
                }
            }
            {
                ImGui::TextUnformatted(tr("nanoem.gui.model.edit.vertex.sdef.c"));
                Vector4 value(glm::make_vec4(nanoemModelVertexGetSdefC(vertexPtr)));
                if (ImGui::InputFloat3("##sdef.c", glm::value_ptr(value))) {
                    ScopedMutableVertex scoped(vertexPtr, m_activeModel);
                    nanoemMutableModelVertexSetSdefC(scoped, glm::value_ptr(value));
                }
            }
            break;
        }
        case NANOEM_MODEL_VERTEX_TYPE_QDEF: {
            layoutVertexBoneWeights(vertexPtr, 4);
            break;
        }
        default:
            break;
        }
    }
    ImGui::PopItemWidth();
}

void
ModelParameterDialog::layoutAllFaces(Project *project)
{
    nanoem_rsize_t numVertexIndices;
    const nanoem_u32_t *vertexIndices = nanoemModelGetAllVertexIndices(m_activeModel->data(), &numVertexIndices);
    nanoem_rsize_t numFaces = numVertexIndices / 3;
    nanoem_f32_t width = ImGuiWindow::kLeftPaneWidth * project->windowDevicePixelRatio();
    ImGui::BeginChild("left-pane", ImVec2(width, 0), false);
    if (ImGui::Button(reinterpret_cast<const char *>(ImGuiWindow::kFACogs))) {
        ImGui::OpenPopup("face-op-menu");
    }
    if (ImGui::BeginPopup("face-op-menu")) {
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    ImGui::Text("%lu / %lu", m_faceIndex + 1, numFaces);
    ImGui::BeginChild(
        "left-pane-inner", ImVec2(width, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing()), true);
    ImGuiListClipper clipper;
    char buffer[Inline::kNameStackBufferSize];
    bool up, down;
    detectUpDown(up, down);
    selectIndex(up, down, numFaces, m_faceIndex);
    clipper.Begin(Inline::saturateInt32(numFaces));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            nanoem_rsize_t offset = i * 3;
            nanoem_u32_t vertexIndex0 = vertexIndices[offset + 0];
            nanoem_u32_t vertexIndex1 = vertexIndices[offset + 1];
            nanoem_u32_t vertexIndex2 = vertexIndices[offset + 2];
            StringUtils::format(buffer, sizeof(buffer), "%d (%d, %d, %d)##face[%d].name", i + 1, vertexIndex0,
                vertexIndex1, vertexIndex2, i);
            bool selected = false;
            if (ImGui::Selectable(buffer, selected) || ((up || down) && selected)) {
                ImGui::SetScrollHereY();
                m_faceIndex = i;
            }
        }
    }
    ImGui::EndChild();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAPlus), 0, false)) {
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAMinus), 0, false)) {
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowUp), 0, false)) {
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowDown), 0, false)) {
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("right-pane", ImGui::GetContentRegionAvail());
    if (numVertexIndices > 0 && m_faceIndex < numVertexIndices) {
    }
    ImGui::EndChild();
}

void
ModelParameterDialog::layoutAllMaterials(Project *project)
{
    nanoem_rsize_t numMaterials;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_activeModel->data(), &numMaterials);
    nanoem_f32_t width = ImGuiWindow::kLeftPaneWidth * project->windowDevicePixelRatio();
    ImGui::BeginChild("left-pane", ImVec2(width, 0), false);
    if (ImGui::Button(reinterpret_cast<const char *>(ImGuiWindow::kFACogs))) {
        ImGui::OpenPopup("material-op-menu");
    }
    if (ImGui::BeginPopup("material-op-menu")) {
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    ImGui::Text("%lu / %lu", m_materialIndex + 1, numMaterials);
    ImGui::BeginChild(
        "left-pane-inner", ImVec2(width, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing()), true);
    ImGuiListClipper clipper;
    nanoem_model_material_t *hoveredMaterialPtr = nullptr;
    char buffer[Inline::kNameStackBufferSize];
    bool up, down;
    detectUpDown(up, down);
    selectIndex(up, down, numMaterials, m_materialIndex);
    IModelObjectSelection *selection = m_activeModel->selection();
    clipper.Begin(Inline::saturateInt32(numMaterials));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            const nanoem_model_material_t *materialPtr = materials[i];
            model::Material *material = model::Material::cast(materialPtr);
            StringUtils::format(buffer, sizeof(buffer), "##material[%d].visible", i);
            bool visible = material->isVisible();
            if (ImGui::Checkbox(buffer, &visible)) {
                material->setVisible(visible);
            }
            ImGui::SameLine();
            const bool selected = selection->containsMaterial(materialPtr);
            StringUtils::format(buffer, sizeof(buffer), "%s##material[%d].name", material->nameConstString(), i);
            if (ImGui::Selectable(buffer, selected)) {
                ImGui::SetScrollHereY();
                if (ImGui::GetIO().KeyCtrl) {
                    selected ? selection->removeMaterial(materialPtr) : selection->addMaterial(materialPtr);
                }
                else {
                    if (!ImGui::GetIO().KeyShift) {
                        selection->removeAllMaterials();
                        m_materialIndex = i;
                    }
                    selection->addMaterial(materialPtr);
                }
                hoveredMaterialPtr = materials[i];
            }
            else if ((up || down) && m_materialIndex == static_cast<nanoem_rsize_t>(i)) {
                ImGui::SetScrollHereY();
                m_materialIndex = i;
            }
            else if (ImGui::IsItemHovered()) {
                hoveredMaterialPtr = materials[i];
            }
        }
    }
    selection->setHoveredMaterial(hoveredMaterialPtr);
    ImGui::EndChild();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAPlus), 0, false)) {
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(
            reinterpret_cast<const char *>(ImGuiWindow::kFAMinus), 0, m_materialIndex < numMaterials)) {
        struct DeleteMaterialCommand : ImGuiWindow::ILazyExecutionCommand {
            DeleteMaterialCommand(nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex)
                : m_materials(materials)
                , m_materialIndex(materialIndex)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableMaterial material(m_materials[m_materialIndex], activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_rsize_t numMaterials, offset = 0, size = 0;
                nanoem_model_material_t *const *materials =
                    nanoemModelGetAllMaterialObjects(nanoemMutableModelGetOriginObject(model), &numMaterials);
                for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
                    const nanoem_model_material_t *currentMaterialPtr = materials[i];
                    const size_t innerSize = nanoemModelMaterialGetNumVertexIndices(currentMaterialPtr);
                    if (currentMaterialPtr == nanoemMutableModelMaterialGetOriginObject(material)) {
                        size = innerSize;
                        break;
                    }
                    offset += innerSize;
                }
                nanoem_rsize_t numIndices, rest;
                const nanoem_u32_t *indices =
                    nanoemModelGetAllVertexIndices(nanoemMutableModelGetOriginObject(model), &numIndices);
                tinystl::vector<nanoem_u32_t, TinySTLAllocator> workingBuffer(numIndices);
                rest = numIndices - offset - size;
                memcpy(workingBuffer.data(), indices, numIndices * sizeof(workingBuffer[0]));
                memmove(workingBuffer.data() + offset, workingBuffer.data() + offset + size,
                    rest * sizeof(workingBuffer[0]));
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelSetVertexIndices(model, workingBuffer.data(), numIndices - size, &status);
                nanoemMutableModelRemoveMaterialObject(model, material, &status);
                nanoemMutableModelMaterialDestroy(material);
                ByteArray bytes;
                Error error;
                activeModel->save(bytes, error);
                Progress reloadModelProgress(project, 0);
                activeModel->clear();
                activeModel->load(bytes, error);
                activeModel->setupAllBindings();
                activeModel->upload();
                activeModel->loadAllImages(reloadModelProgress, error);
                if (Motion *motion = project->resolveMotion(activeModel)) {
                    motion->initialize(activeModel);
                }
                activeModel->updateStagingVertexBuffer();
                project->rebuildAllTracks();
                project->restart();
                reloadModelProgress.complete();
            }
            nanoem_model_material_t *const *m_materials;
            nanoem_rsize_t m_materialIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(DeleteMaterialCommand(materials, m_materialIndex)));
    }
    ImGui::SameLine();
    struct BaseMoveMaterialCommand : ImGuiWindow::ILazyExecutionCommand {
        struct LayoutPosition {
            size_t m_offset;
            size_t m_size;
        };
        BaseMoveMaterialCommand(nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex)
            : m_materials(materials)
            , m_materialIndex(materialIndex)
        {
        }
        void
        move(int destination, const LayoutPosition &from, const LayoutPosition &to, Model *activeModel)
        {
            ScopedMutableMaterial material(m_materials[m_materialIndex], activeModel);
            ScopedMutableModel model(activeModel);
            nanoem_rsize_t numIndices;
            const nanoem_u32_t *indices =
                nanoemModelGetAllVertexIndices(nanoemMutableModelGetOriginObject(model), &numIndices);
            tinystl::vector<nanoem_u32_t, TinySTLAllocator> tempFromBuffer(from.m_size), tempToBuffer(to.m_size);
            memcpy(tempFromBuffer.data(), indices + from.m_offset, from.m_size * sizeof(tempToBuffer[0]));
            memcpy(tempToBuffer.data(), indices + to.m_offset, to.m_size * sizeof(tempToBuffer[0]));
            tinystl::vector<nanoem_u32_t, TinySTLAllocator> workingBuffer(numIndices);
            memcpy(workingBuffer.data(), indices, numIndices * sizeof(workingBuffer[0]));
            memcpy(
                workingBuffer.data() + from.m_offset, tempFromBuffer.data(), from.m_size * sizeof(tempFromBuffer[0]));
            memcpy(workingBuffer.data() + to.m_offset, tempToBuffer.data(), to.m_size * sizeof(tempToBuffer[0]));
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            nanoemMutableModelSetVertexIndices(model, workingBuffer.data(), numIndices, &status);
            nanoemMutableModelRemoveMaterialObject(model, material, &status);
            nanoemMutableModelInsertMaterialObject(model, material, destination, &status);
        }
        nanoem_model_material_t *const *m_materials;
        nanoem_rsize_t &m_materialIndex;
    };
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowUp), 0, m_materialIndex > 0)) {
        struct MoveMaterialUpCommand : BaseMoveMaterialCommand {
            MoveMaterialUpCommand(nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex)
                : BaseMoveMaterialCommand(materials, materialIndex)
            {
            }
            void
            execute(Project *project)
            {
                const nanoem_model_material_t *activeMaterial = m_materials[m_materialIndex];
                Model *activeModel = project->activeModel();
                nanoem_rsize_t numMaterials, offset = 0;
                nanoem_model_material_t *const *materials =
                    nanoemModelGetAllMaterialObjects(activeModel->data(), &numMaterials);
                LayoutPosition from, to;
                int destination = 0;
                for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
                    const nanoem_model_material_t *currentMaterialPtr = materials[i];
                    size_t size = nanoemModelMaterialGetNumVertexIndices(currentMaterialPtr);
                    if (currentMaterialPtr == activeMaterial) {
                        const nanoem_model_material_t *previousMaterialPtr = materials[i - 1];
                        destination = i - 1;
                        to.m_size = nanoemModelMaterialGetNumVertexIndices(previousMaterialPtr);
                        to.m_offset = offset - to.m_size;
                        from.m_offset = offset;
                        from.m_size = size;
                        break;
                    }
                    offset += size;
                }
                move(destination, from, to, activeModel);
            }
        };
        m_parent->addLazyExecutionCommand(nanoem_new(MoveMaterialUpCommand(materials, m_materialIndex)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(
            reinterpret_cast<const char *>(ImGuiWindow::kFAArrowDown), 0, m_materialIndex < numMaterials)) {
        struct MoveMaterialDownCommand : BaseMoveMaterialCommand {
            MoveMaterialDownCommand(nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex)
                : BaseMoveMaterialCommand(materials, materialIndex)
            {
            }
            void
            execute(Project *project)
            {
                const nanoem_model_material_t *activeMaterial = m_materials[m_materialIndex];
                Model *activeModel = project->activeModel();
                nanoem_rsize_t numMaterials, offset = 0;
                nanoem_model_material_t *const *materials =
                    nanoemModelGetAllMaterialObjects(activeModel->data(), &numMaterials);
                LayoutPosition from, to;
                int destination = 0;
                for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
                    const nanoem_model_material_t *currentMaterialPtr = materials[i];
                    size_t size = nanoemModelMaterialGetNumVertexIndices(currentMaterialPtr);
                    if (currentMaterialPtr == activeMaterial) {
                        const nanoem_model_material_t *nextMaterialPtr = materials[i + 1];
                        destination = i + 1;
                        to.m_size = nanoemModelMaterialGetNumVertexIndices(nextMaterialPtr);
                        to.m_offset = offset + to.m_size;
                        from.m_offset = offset;
                        from.m_size = size;
                        break;
                    }
                    offset += size;
                }
                move(destination, from, to, activeModel);
            }
        };
        m_parent->addLazyExecutionCommand(nanoem_new(MoveMaterialDownCommand(materials, m_materialIndex)));
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("right-pane", ImGui::GetContentRegionAvail());
    if (numMaterials > 0 && m_materialIndex < numMaterials) {
        nanoem_model_material_t *materialPtr = materials[m_materialIndex];
        layoutMaterialPropertyPane(materialPtr, project);
    }
    ImGui::EndChild();
}

void
ModelParameterDialog::layoutMaterialPropertyPane(nanoem_model_material_t *materialPtr, Project *project)
{
    nanoem_language_type_t language = static_cast<nanoem_language_type_t>(m_language);
    StringUtils::UnicodeStringScope scope(project->unicodeStringFactory());
    ImGui::PushItemWidth(-1);
    if (layoutName(nanoemModelMaterialGetName(materialPtr, language), project, scope)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        ScopedMutableMaterial scoped(materialPtr, m_activeModel);
        nanoemMutableModelMaterialSetName(scoped, scope.value(), language, &status);
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.material.ambient.color"));
        Vector4 value(glm::make_vec4(nanoemModelMaterialGetAmbientColor(materialPtr)));
        if (ImGui::ColorEdit3("##ambient", glm::value_ptr(value))) {
            ScopedMutableMaterial scoped(materialPtr, m_activeModel);
            nanoemMutableModelMaterialSetAmbientColor(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.material.diffuse.color"));
        Vector4 value(glm::make_vec3(nanoemModelMaterialGetDiffuseColor(materialPtr)),
            nanoemModelMaterialGetDiffuseOpacity(materialPtr));
        if (ImGui::ColorEdit4("##diffuse", glm::value_ptr(value))) {
            ScopedMutableMaterial scoped(materialPtr, m_activeModel);
            nanoemMutableModelMaterialSetDiffuseColor(scoped, glm::value_ptr(value));
            nanoemMutableModelMaterialSetDiffuseOpacity(scoped, value.w);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.material.specular.color"));
        Vector4 value(glm::make_vec4(nanoemModelMaterialGetSpecularColor(materialPtr)));
        if (ImGui::ColorEdit3("##specular", glm::value_ptr(value))) {
            ScopedMutableMaterial scoped(materialPtr, m_activeModel);
            nanoemMutableModelMaterialSetSpecularColor(scoped, glm::value_ptr(value));
        }
    }
    addSeparator();
    {
        ImGui::TextUnformatted("Edge");
        Vector4 value(glm::make_vec3(nanoemModelMaterialGetEdgeColor(materialPtr)),
            nanoemModelMaterialGetEdgeOpacity(materialPtr));
        if (ImGui::ColorEdit4("##edge", glm::value_ptr(value))) {
            ScopedMutableMaterial scoped(materialPtr, m_activeModel);
            nanoemMutableModelMaterialSetEdgeColor(scoped, glm::value_ptr(value));
            nanoemMutableModelMaterialSetEdgeOpacity(scoped, value.w);
        }
    }
    {
        ImGui::TextUnformatted("Primitive Type");
        if (ImGui::BeginCombo("##primitive", selectedMaterialPrimitiveType(materialPtr))) {
            if (ImGui::Selectable(tr("nanoem.gui.model.edit.material.primitive.triangle"))) {
                ScopedMutableMaterial scoped(materialPtr, m_activeModel);
                nanoemMutableModelMaterialSetLineDrawEnabled(scoped, false);
                nanoemMutableModelMaterialSetPointDrawEnabled(scoped, false);
            }
            if (ImGui::Selectable(tr("nanoem.gui.model.edit.material.primitive.line"))) {
                ScopedMutableMaterial scoped(materialPtr, m_activeModel);
                nanoemMutableModelMaterialSetLineDrawEnabled(scoped, true);
                nanoemMutableModelMaterialSetPointDrawEnabled(scoped, false);
            }
            if (ImGui::Selectable(tr("nanoem.gui.model.edit.material.primitive.point"))) {
                ScopedMutableMaterial scoped(materialPtr, m_activeModel);
                nanoemMutableModelMaterialSetLineDrawEnabled(scoped, false);
                nanoemMutableModelMaterialSetPointDrawEnabled(scoped, true);
            }
            ImGui::EndCombo();
        }
    }
    {
        ImGui::TextUnformatted("SphereMap Type");
        if (ImGui::BeginCombo("##spheremap",
                selectedMaterialSphereMapType(nanoemModelMaterialGetSphereMapTextureType(materialPtr)))) {
            for (nanoem_rsize_t i = NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_FIRST_ENUM;
                 i < NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_MAX_ENUM; i++) {
                const nanoem_model_material_sphere_map_texture_type_t index =
                    static_cast<nanoem_model_material_sphere_map_texture_type_t>(i);
                if (ImGui::Selectable(selectedMaterialSphereMapType(index))) {
                    ScopedMutableMaterial scoped(materialPtr, m_activeModel);
                    nanoemMutableModelMaterialSetSphereMapTextureType(scoped, index);
                }
            }
            ImGui::EndCombo();
        }
    }
    addSeparator();
    if (ImGui::TreeNode("properties", "%s", tr("nanoem.gui.model.edit.bone.properties"))) {
        ImGui::Unindent();
        {
            bool value = nanoemModelMaterialIsToonShared(materialPtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.material.property.shared-toon"), &value)) {
                ScopedMutableMaterial scoped(materialPtr, m_activeModel);
                nanoemMutableModelMaterialSetToonShared(scoped, value);
            }
        }
        {
            bool value = nanoemModelMaterialIsEdgeEnabled(materialPtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.material.property.edge"), &value)) {
                ScopedMutableMaterial scoped(materialPtr, m_activeModel);
                nanoemMutableModelMaterialSetEdgeEnabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelMaterialIsCullingDisabled(materialPtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.material.property.disable-culling"), &value)) {
                ScopedMutableMaterial scoped(materialPtr, m_activeModel);
                nanoemMutableModelMaterialSetCullingDisabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelMaterialIsShadowMapEnabled(materialPtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.material.property.self-shadow"), &value)) {
                ScopedMutableMaterial scoped(materialPtr, m_activeModel);
                nanoemMutableModelMaterialSetShadowMapEnabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelMaterialIsCastingShadowEnabled(materialPtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.material.property.casting-shadow"), &value)) {
                ScopedMutableMaterial scoped(materialPtr, m_activeModel);
                nanoemMutableModelMaterialSetCastingShadowEnabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelMaterialIsCastingShadowMapEnabled(materialPtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.material.property.casting-self-shadow"), &value)) {
                ScopedMutableMaterial scoped(materialPtr, m_activeModel);
                nanoemMutableModelMaterialSetCastingShadowMapEnabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelMaterialIsVertexColorEnabled(materialPtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.material.property.vertex-color"), &value)) {
                ScopedMutableMaterial scoped(materialPtr, m_activeModel);
                nanoemMutableModelMaterialSetVertexColorEnabled(scoped, value);
            }
        }
        ImGui::Indent();
        ImGui::TreePop();
    }
    ImGui::TextUnformatted("Textures");
    const model::Material *material = model::Material::cast(materialPtr);
    String filename;
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::getUtf8String(
        nanoemModelTextureGetPath(nanoemModelMaterialGetDiffuseTextureObject(materialPtr)), factory, filename);
    layoutMaterialImage(material->diffuseImage(), tr("nanoem.gui.model.edit.material.texture.diffuse"), filename);
    StringUtils::getUtf8String(
        nanoemModelTextureGetPath(nanoemModelMaterialGetSphereMapTextureObject(materialPtr)), factory, filename);
    layoutMaterialImage(material->sphereMapImage(), tr("nanoem.gui.model.edit.material.texture.sphere"), filename);
    StringUtils::getUtf8String(
        nanoemModelTextureGetPath(nanoemModelMaterialGetToonTextureObject(materialPtr)), factory, filename);
    layoutMaterialImage(material->toonImage(), tr("nanoem.gui.model.edit.material.texture.toon"), filename);
    {
        ImGui::TextUnformatted("Arbitrary Area");
        String clob;
        MutableString clobBuffer;
        StringUtils::getUtf8String(nanoemModelMaterialGetClob(materialPtr), factory, clob);
        clobBuffer.assign(clob.c_str(), clob.c_str() + clob.size());
        clobBuffer.push_back(0);
        if (ImGui::InputTextMultiline("##clob", clobBuffer.data(), clobBuffer.capacity(),
                ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing() * 5))) {
            StringUtils::UnicodeStringScope s(factory);
            if (StringUtils::tryGetString(factory, clobBuffer.data(), s)) {
                ScopedMutableMaterial scoped(materialPtr, m_activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelMaterialSetClob(scoped, s.value(), &status);
            }
        }
    }
    ImGui::PopItemWidth();
}

void
ModelParameterDialog::layoutMaterialImage(const IImageView *image, const char *label, const String &filename)
{
    if (image && ImGui::TreeNode(label)) {
        const ImTextureID textureID = reinterpret_cast<ImTextureID>(image->handle().id);
        ImGui::TextUnformatted(filename.c_str());
        ImGui::Image(textureID, calcExpandedImageSize(image->description()), ImVec2(0, 0), ImVec2(1, 1),
            ImVec4(1, 1, 1, 1), ImGui::ColorConvertU32ToFloat4(ImGuiWindow::kColorBorder));
        ImGui::TreePop();
    }
}

typedef tinystl::unordered_map<const nanoem_model_bone_t *, model::Bone::Set, TinySTLAllocator> BoneTree;
static void
constructTree(const BoneTree &boneTree, const nanoem_model_bone_t *parentBonePtr)
{
    BoneTree::const_iterator it = boneTree.find(parentBonePtr);
    if (it != boneTree.end()) {
        const model::Bone::Set &boneSet = it->second;
        if (const model::Bone *parentBone = model::Bone::cast(parentBonePtr)) {
            if (ImGui::TreeNode(parentBonePtr, "%s", parentBone->nameConstString())) {
                const model::Bone::Set &boneSet = it->second;
                for (model::Bone::Set::const_iterator it2 = boneSet.begin(), end2 = boneSet.end(); it2 != end2; ++it2) {
                    const nanoem_model_bone_t *bonePtr = *it2;
                    constructTree(boneTree, bonePtr);
                }
                ImGui::TreePop();
            }
        }
        else {
            for (model::Bone::Set::const_iterator it2 = boneSet.begin(), end2 = boneSet.end(); it2 != end2; ++it2) {
                const nanoem_model_bone_t *bonePtr = *it2;
                constructTree(boneTree, bonePtr);
            }
        }
    }
    else if (const model::Bone *parentBone = model::Bone::cast(parentBonePtr)) {
        ImGui::Selectable(parentBone->nameConstString());
    }
}

void
ModelParameterDialog::layoutAllBones(Project *project)
{
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
    {
        ImGui::SetNextWindowSize(
            ImVec2(kMinimumWindowWidth * 0.5f * project->windowDevicePixelRatio(), 0), ImGuiCond_FirstUseEver);
        ImGui::Begin("Tree");
        BoneTree boneTree;
        for (nanoem_rsize_t i = 0; i < numBones; i++) {
            const nanoem_model_bone_t *bonePtr = bones[i], *parentBonePtr = nanoemModelBoneGetParentBoneObject(bonePtr);
            boneTree[parentBonePtr].insert(bonePtr);
            if (const nanoem_model_constraint_t *constraintPtr = nanoemModelBoneGetConstraintObject(bonePtr)) {
                nanoem_rsize_t numJoints;
                nanoem_model_constraint_joint_t *const *joints =
                    nanoemModelConstraintGetAllJointObjects(constraintPtr, &numJoints);
                for (nanoem_rsize_t j = 0; j < numJoints; j++) {
                    const nanoem_model_constraint_joint_t *jointPtr = joints[j];
                    boneTree[parentBonePtr].insert(nanoemModelConstraintJointGetBoneObject(jointPtr));
                }
            }
            if (const nanoem_model_bone_t *inherentParentBonePtr =
                    nanoemModelBoneGetInherentParentBoneObject(bonePtr)) {
                boneTree[inherentParentBonePtr].insert(bonePtr);
            }
        }
        constructTree(boneTree, nullptr);
        ImGui::End();
    }
    nanoem_model_bone_t *hoveredBonePtr = nullptr;
    nanoem_f32_t width = ImGuiWindow::kLeftPaneWidth * project->windowDevicePixelRatio();
    ImGui::BeginChild("left-pane", ImVec2(width, 0), false);
    if (ImGui::Button(reinterpret_cast<const char *>(ImGuiWindow::kFACogs))) {
        ImGui::OpenPopup("bone-op-menu");
    }
    if (ImGui::BeginPopup("bone-op-menu")) {
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    ImGui::Text("%lu / %lu", m_boneIndex + 1, numBones);
    ImGui::BeginChild(
        "left-pane-inner", ImVec2(width, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing()), true);
    ImGuiListClipper clipper;
    char buffer[Inline::kNameStackBufferSize];
    bool up, down;
    detectUpDown(up, down);
    selectIndex(up, down, numBones, m_boneIndex);
    IModelObjectSelection *selection = m_activeModel->selection();
    clipper.Begin(Inline::saturateInt32(numBones));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            const nanoem_model_bone_t *bonePtr = bones[i];
            const model::Bone *bone = model::Bone::cast(bonePtr);
            const bool selected = selection->containsBone(bonePtr);
            StringUtils::format(buffer, sizeof(buffer), "%s##bone[%d].name", bone->nameConstString(), i);
            if (ImGui::Selectable(buffer, selected)) {
                ImGui::SetScrollHereY();
                if (ImGui::GetIO().KeyCtrl) {
                    selected ? selection->removeBone(bonePtr) : selection->addBone(bonePtr);
                }
                else {
                    if (!ImGui::GetIO().KeyShift) {
                        selection->removeAllBones();
                        m_boneIndex = i;
                    }
                    selection->addBone(bonePtr);
                }
                hoveredBonePtr = bones[i];
            }
            else if ((up || down) && m_boneIndex == static_cast<nanoem_rsize_t>(i)) {
                ImGui::SetScrollHereY();
                m_boneIndex = i;
            }
            else if (ImGui::IsItemHovered()) {
                hoveredBonePtr = bones[i];
            }
        }
    }
    selection->setHoveredBone(hoveredBonePtr);
    ImGui::EndChild();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAPlus), 0, true)) {
        ImGui::OpenPopup("bone-create-menu");
    }
    if (ImGui::BeginPopup("bone-create-menu")) {
        struct CreateBoneCommand : ImGuiWindow::ILazyExecutionCommand {
            CreateBoneCommand(nanoem_rsize_t numBones, int offset, const nanoem_model_bone_t *base)
                : m_base(base)
                , m_numBones(numBones)
                , m_offset(offset)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableModel model(activeModel);
                ScopedMutableBone bone(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
                StringUtils::UnicodeStringScope scope(factory);
                char buffer[Inline::kMarkerStringLength];
                if (m_base) {
                    nanoemMutableModelBoneCopy(bone, m_base, &status);
                }
                StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, m_numBones + 1);
                if (StringUtils::tryGetString(factory, buffer, scope)) {
                    nanoemMutableModelBoneSetName(bone, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
                }
                StringUtils::format(buffer, sizeof(buffer), "NewBone%zu", m_numBones + 1);
                if (StringUtils::tryGetString(factory, buffer, scope)) {
                    nanoemMutableModelBoneSetName(bone, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
                }
                nanoemMutableModelInsertBoneObject(model, bone, m_offset, &status);
                model::Bone *newBone = model::Bone::create();
                newBone->bind(nanoemMutableModelBoneGetOriginObject(bone));
                newBone->resetLanguage(nanoemMutableModelBoneGetOriginObject(bone), factory, project->castLanguage());
            }
            const nanoem_model_bone_t *m_base;
            const nanoem_rsize_t m_numBones;
            const int m_offset;
        };
        const nanoem_model_bone_t *selectedBone = numBones > 0 ? bones[m_boneIndex] : nullptr;
        int selectedBoneIndex = nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(selectedBone));
        if (ImGui::BeginMenu("Insert New")) {
            if (ImGui::MenuItem("at Last")) {
                m_parent->addLazyExecutionCommand(nanoem_new(CreateBoneCommand(numBones, -1, nullptr)));
            }
            if (ImGui::MenuItem("after Selected", nullptr, nullptr, m_boneIndex < numBones)) {
                m_parent->addLazyExecutionCommand(
                    nanoem_new(CreateBoneCommand(numBones, selectedBoneIndex + 1, nullptr)));
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Insert Copy", m_boneIndex < numBones)) {
            if (ImGui::MenuItem("at Last")) {
                m_parent->addLazyExecutionCommand(nanoem_new(CreateBoneCommand(numBones, -1, selectedBone)));
            }
            if (ImGui::MenuItem("at Next")) {
                m_parent->addLazyExecutionCommand(
                    nanoem_new(CreateBoneCommand(numBones, selectedBoneIndex + 1, selectedBone)));
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAMinus), 0, m_boneIndex < numBones)) {
        struct DeleteBoneCommand : ImGuiWindow::ILazyExecutionCommand {
            DeleteBoneCommand(nanoem_model_bone_t *const *bones, nanoem_rsize_t &boneIndex)
                : m_bones(bones)
                , m_boneIndex(boneIndex)
            {
            }
            void
            execute(Project *project)
            {
                nanoem_model_bone_t *bonePtr = m_bones[m_boneIndex];
                Model *activeModel = project->activeModel();
                ScopedMutableBone bone(bonePtr, activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                if (activeModel->activeBone() == bonePtr) {
                    activeModel->setActiveBone(nullptr);
                }
                activeModel->removeBone(bonePtr);
                nanoemMutableModelRemoveBoneObject(model, bone, &status);
                project->rebuildAllTracks();
                if (m_boneIndex > 0) {
                    m_boneIndex--;
                }
            }
            nanoem_model_bone_t *const *m_bones;
            nanoem_rsize_t &m_boneIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(DeleteBoneCommand(bones, m_boneIndex)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowUp), 0, m_boneIndex > 0)) {
        struct MoveBoneUpCommand : ImGuiWindow::ILazyExecutionCommand {
            MoveBoneUpCommand(nanoem_model_bone_t *const *bones, nanoem_rsize_t &boneIndex)
                : m_bones(bones)
                , m_boneIndex(boneIndex)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableBone bone(m_bones[m_boneIndex], activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelRemoveBoneObject(model, bone, &status);
                int offset = Inline::saturateInt32(--m_boneIndex);
                nanoemMutableModelInsertBoneObject(model, bone, offset, &status);
            }
            nanoem_model_bone_t *const *m_bones;
            nanoem_rsize_t &m_boneIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(MoveBoneUpCommand(bones, m_boneIndex)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(
            reinterpret_cast<const char *>(ImGuiWindow::kFAArrowDown), 0, m_boneIndex < numBones - 1)) {
        struct MoveBoneDownCommand : ImGuiWindow::ILazyExecutionCommand {
            MoveBoneDownCommand(nanoem_model_bone_t *const *bones, nanoem_rsize_t &boneIndex)
                : m_bones(bones)
                , m_boneIndex(boneIndex)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableBone bone(m_bones[m_boneIndex], activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelRemoveBoneObject(model, bone, &status);
                int offset = Inline::saturateInt32(++m_boneIndex);
                nanoemMutableModelInsertBoneObject(model, bone, offset, &status);
            }
            nanoem_model_bone_t *const *m_bones;
            nanoem_rsize_t &m_boneIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(MoveBoneDownCommand(bones, m_boneIndex)));
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("right-pane", ImGui::GetContentRegionAvail());
    if (numBones > 0 && m_boneIndex < numBones) {
        nanoem_model_bone_t *bonePtr = bones[m_boneIndex];
        layoutBonePropertyPane(bonePtr, project);
    }
    ImGui::EndChild();
}

void
ModelParameterDialog::layoutBonePropertyPane(nanoem_model_bone_t *bonePtr, Project *project)
{
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
    nanoem_language_type_t language = static_cast<nanoem_language_type_t>(m_language);
    StringUtils::UnicodeStringScope scope(project->unicodeStringFactory());
    ImGui::PushItemWidth(-1);
    if (layoutName(nanoemModelBoneGetName(bonePtr, language), project, scope)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        ScopedMutableBone scoped(bonePtr, m_activeModel);
        nanoemMutableModelBoneSetName(scoped, scope.value(), language, &status);
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.bone.origin"));
        Vector4 value(glm::make_vec4(nanoemModelBoneGetOrigin(bonePtr)));
        if (ImGui::InputFloat3("##origin", glm::value_ptr(value))) {
            ScopedMutableBone scoped(bonePtr, m_activeModel);
            nanoemMutableModelBoneSetOrigin(scoped, glm::value_ptr(value));
        }
    }
    {
        const model::Bone *parentBone = model::Bone::cast(nanoemModelBoneGetParentBoneObject(bonePtr));
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.bone.parent"));
        if (ImGui::BeginCombo("##parent", parentBone ? parentBone->nameConstString() : "(none)")) {
            for (nanoem_rsize_t i = 0; i < numBones; i++) {
                const nanoem_model_bone_t *candidateBonePtr = bones[i];
                const model::Bone *candidateBone = model::Bone::cast(candidateBonePtr);
                if (candidateBone && ImGui::Selectable(candidateBone->nameConstString(), parentBone == candidateBone)) {
                    ScopedMutableBone scoped(bonePtr, m_activeModel);
                    nanoemMutableModelBoneSetParentBoneObject(scoped, candidateBonePtr);
                }
            }
            ImGui::EndCombo();
        }
    }
    {
        bool value = nanoemModelBoneHasDestinationBone(bonePtr) != 0;
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.bone.target"), value)) {
            ScopedMutableBone scoped(bonePtr, m_activeModel);
            nanoemMutableModelBoneSetTargetBoneObject(scoped, nanoemModelBoneGetTargetBoneObject(bonePtr));
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.bone.destination-origin"), value ? false : true)) {
            ScopedMutableBone scoped(bonePtr, m_activeModel);
            const Vector4 copy(glm::make_vec4(nanoemModelBoneGetDestinationOrigin(bonePtr)));
            nanoemMutableModelBoneSetDestinationOrigin(scoped, glm::value_ptr(copy));
        }
        if (value) {
            const model::Bone *targetbone = model::Bone::cast(nanoemModelBoneGetTargetBoneObject(bonePtr));
            if (ImGui::BeginCombo("##destination.bone", targetbone ? targetbone->nameConstString() : "(none)")) {
                for (nanoem_rsize_t i = 0; i < numBones; i++) {
                    const nanoem_model_bone_t *candidateBonePtr = bones[i];
                    const model::Bone *candidateBone = model::Bone::cast(candidateBonePtr);
                    if (candidateBone &&
                        ImGui::Selectable(candidateBone->nameConstString(), targetbone == candidateBone)) {
                        ScopedMutableBone scoped(bonePtr, m_activeModel);
                        nanoemMutableModelBoneSetTargetBoneObject(scoped, candidateBonePtr);
                    }
                }
                ImGui::EndCombo();
            }
        }
        else {
            Vector3 origin(glm::make_vec3(nanoemModelBoneGetDestinationOrigin(bonePtr)));
            if (ImGui::DragFloat3("##destination.origin", glm::value_ptr(origin))) {
                ScopedMutableBone scoped(bonePtr, m_activeModel);
                nanoemMutableModelBoneSetDestinationOrigin(scoped, glm::value_ptr(origin));
            }
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.bone.stage"));
        int stageIndex = nanoemModelBoneGetStageIndex(bonePtr);
        if (ImGui::InputInt("##stage", &stageIndex)) {
            ScopedMutableBone scoped(bonePtr, m_activeModel);
            nanoemMutableModelBoneSetStageIndex(scoped, stageIndex);
        }
    }
    addSeparator();
    char buffer[Inline::kNameStackBufferSize];
    StringUtils::format(buffer, sizeof(buffer), "%s##properties", tr("nanoem.gui.model.edit.bone.properties"));
    if (ImGui::TreeNode(buffer)) {
        ImGui::Unindent();
        {
            bool value = nanoemModelBoneIsVisible(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.visible"), &value)) {
                ScopedMutableBone scoped(bonePtr, m_activeModel);
                nanoemMutableModelBoneSetVisible(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneIsMovable(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.movable"), &value)) {
                ScopedMutableBone scoped(bonePtr, m_activeModel);
                nanoemMutableModelBoneSetMovable(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneIsRotateable(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.rotateable"), &value)) {
                ScopedMutableBone scoped(bonePtr, m_activeModel);
                nanoemMutableModelBoneSetRotateable(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneIsUserHandleable(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.user-handleable"), &value)) {
                ScopedMutableBone scoped(bonePtr, m_activeModel);
                nanoemMutableModelBoneSetUserHandleable(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneHasLocalInherent(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.inherent.local"), &value)) {
                ScopedMutableBone scoped(bonePtr, m_activeModel);
                nanoemMutableModelBoneSetLocalInherentEnabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneIsAffectedByPhysicsSimulation(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.affected-by-physics-simulation"), &value)) {
                ScopedMutableBone scoped(bonePtr, m_activeModel);
                nanoemMutableModelBoneSetAffectedByPhysicsSimulation(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneHasInherentTranslation(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.inherent.translation"), &value)) {
                ScopedMutableBone scoped(bonePtr, m_activeModel);
                nanoemMutableModelBoneSetInherentTranslationEnabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneHasInherentOrientation(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.inherent.orientation"), &value)) {
                ScopedMutableBone scoped(bonePtr, m_activeModel);
                nanoemMutableModelBoneSetInherentOrientationEnabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneHasFixedAxis(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.has-fixed-axis"), &value)) {
                ScopedMutableBone scoped(bonePtr, m_activeModel);
                nanoemMutableModelBoneSetFixedAxisEnabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneHasLocalAxes(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.has-local-axes"), &value)) {
                ScopedMutableBone scoped(bonePtr, m_activeModel);
                nanoemMutableModelBoneSetLocalAxesEnabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneHasConstraint(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.inverse-kinematics"), &value)) {
                ScopedMutableBone scoped(bonePtr, m_activeModel);
                nanoemMutableModelBoneSetConstraintEnabled(scoped, value);
            }
        }
        {
            if (nanoemModelBoneHasInherentTranslation(bonePtr) || nanoemModelBoneHasInherentOrientation(bonePtr)) {
                ImGui::Separator();
                const model::Bone *parentBone = model::Bone::cast(nanoemModelBoneGetInherentParentBoneObject(bonePtr));
                ImGui::TextUnformatted(tr("nanoem.gui.model.edit.bone.inherent.parent-bone"));
                if (ImGui::BeginCombo("##parent.inherent", parentBone ? parentBone->nameConstString() : "(none)")) {
                    for (nanoem_rsize_t i = 0; i < numBones; i++) {
                        const nanoem_model_bone_t *candidateBonePtr = bones[i];
                        const model::Bone *candidateBone = model::Bone::cast(candidateBonePtr);
                        if (candidateBone &&
                            ImGui::Selectable(candidateBone->nameConstString(), candidateBone == parentBone)) {
                            ScopedMutableBone scoped(bonePtr, m_activeModel);
                            nanoemMutableModelBoneSetInherentParentBoneObject(scoped, candidateBonePtr);
                        }
                    }
                    ImGui::EndCombo();
                }
                nanoem_f32_t coefficient = nanoemModelBoneGetInherentCoefficient(bonePtr);
                ImGui::TextUnformatted(tr("nanoem.gui.model.edit.bone.inherent.coefficient"));
                if (ImGui::DragFloat("##coefficient", &coefficient)) {
                    ScopedMutableBone scoped(bonePtr, m_activeModel);
                    nanoemMutableModelBoneSetInherentCoefficient(scoped, coefficient);
                }
            }
        }
        {
            if (nanoemModelBoneHasFixedAxis(bonePtr)) {
                Vector3 axis(glm::make_vec3(nanoemModelBoneGetFixedAxis(bonePtr)));
                ImGui::TextUnformatted("Fixed Axis");
                if (ImGui::DragFloat3("##axis", glm::value_ptr(axis), 1.0f, 0.0f, 1.0f)) {
                    ScopedMutableBone scoped(bonePtr, m_activeModel);
                    nanoemMutableModelBoneSetFixedAxis(scoped, glm::value_ptr(axis));
                }
            }
        }
        {
            if (nanoemModelBoneHasLocalAxes(bonePtr)) {
                ImGui::Separator();
                Vector3 axisX(glm::make_vec3(nanoemModelBoneGetLocalXAxis(bonePtr)));
                ImGui::TextUnformatted(tr("nanoem.gui.model.edit.bone.local-axis.x"));
                if (ImGui::DragFloat3("##axis.x", glm::value_ptr(axisX), 1.0f, 0.0f, 1.0f)) {
                    ScopedMutableBone scoped(bonePtr, m_activeModel);
                    nanoemMutableModelBoneSetLocalXAxis(scoped, glm::value_ptr(axisX));
                }
                ImGui::TextUnformatted(tr("nanoem.gui.model.edit.bone.local-axis.z"));
                Vector3 axisZ(glm::make_vec3(nanoemModelBoneGetLocalZAxis(bonePtr)));
                if (ImGui::DragFloat3("##axis.z", glm::value_ptr(axisZ), 1.0f, 0.0f, 1.0f)) {
                    ScopedMutableBone scoped(bonePtr, m_activeModel);
                    nanoemMutableModelBoneSetLocalZAxis(scoped, glm::value_ptr(axisZ));
                }
            }
        }
        ImGui::Indent();
        ImGui::TreePop();
    }
    layoutBoneConstraintPanel(bonePtr, project);
    if (ImGui::TreeNode("Internal Parameters")) {
        ImGui::Unindent();
        model::Bone *bone = model::Bone::cast(bonePtr);
        {
            Vector3 worldTransformOrigin(bone->worldTransformOrigin());
            ImGui::TextUnformatted("Global Transform Origin");
            ImGui::InputFloat3(
                "##tranform.origin.global", glm::value_ptr(worldTransformOrigin), "%.3f", ImGuiInputTextFlags_ReadOnly);
        }
        {
            Vector3 worldTransformOrientation(glm::degrees(glm::eulerAngles(glm::quat_cast(bone->worldTransform()))));
            ImGui::TextUnformatted("Global Transform Orientation");
            ImGui::InputFloat3("##tranform.orientation", glm::value_ptr(worldTransformOrientation), "%.3f",
                ImGuiInputTextFlags_ReadOnly);
        }
        {
            Vector3 localTransformOrigin(bone->localTransformOrigin());
            ImGui::TextUnformatted("Local Transform Origin");
            ImGui::InputFloat3(
                "##tranform.origin.local", glm::value_ptr(localTransformOrigin), "%.3f", ImGuiInputTextFlags_ReadOnly);
        }
        {
            Vector3 localUserTranslation(bone->localUserTranslation());
            ImGui::TextUnformatted("Local User Translation");
            ImGui::InputFloat3(
                "##translation.user", glm::value_ptr(localUserTranslation), "%.3f", ImGuiInputTextFlags_ReadOnly);
        }
        {
            Vector3 localUserOrientation(glm::degrees(glm::eulerAngles(bone->localUserOrientation())));
            ImGui::TextUnformatted("Local User Orientation");
            ImGui::InputFloat3(
                "##orientation.user", glm::value_ptr(localUserOrientation), "%.3f", ImGuiInputTextFlags_ReadOnly);
        }
        {
            Vector3 localTranslation(bone->localTranslation());
            ImGui::TextUnformatted("Local Translation");
            ImGui::InputFloat3(
                "##translation.local", glm::value_ptr(localTranslation), "%.3f", ImGuiInputTextFlags_ReadOnly);
        }
        {
            Vector3 localOrientation(glm::degrees(glm::eulerAngles(bone->localOrientation())));
            ImGui::TextUnformatted("Local Orientation");
            ImGui::InputFloat3(
                "##orientation.local", glm::value_ptr(localOrientation), "%.3f", ImGuiInputTextFlags_ReadOnly);
        }
        {
            Vector3 constraintJointOrientation(glm::degrees(glm::eulerAngles(bone->constraintJointOrientation())));
            ImGui::TextUnformatted("Constraint Joint Orientation");
            ImGui::InputFloat3("##orientation.joint", glm::value_ptr(constraintJointOrientation), "%.3f",
                ImGuiInputTextFlags_ReadOnly);
        }
        {
            Vector3 morphTranslation(bone->localMorphTranslation());
            ImGui::TextUnformatted("Local Morph Translation");
            ImGui::InputFloat3(
                "##translation.morph", glm::value_ptr(morphTranslation), "%.3f", ImGuiInputTextFlags_ReadOnly);
        }
        {
            Vector3 morphOrientation(glm::degrees(glm::eulerAngles(bone->localMorphOrientation())));
            ImGui::TextUnformatted("Local Morph Orientation");
            ImGui::InputFloat3(
                "##orientation.morph", glm::value_ptr(morphOrientation), "%.3f", ImGuiInputTextFlags_ReadOnly);
        }
        {
            Vector3 localInherentTranslation(bone->localInherentTranslation());
            ImGui::TextUnformatted("Inherent Translation");
            ImGui::InputFloat3("##translation.inherent", glm::value_ptr(localInherentTranslation), "%.3f",
                ImGuiInputTextFlags_ReadOnly);
        }
        {
            Vector3 localInherentOrientation(glm::degrees(glm::eulerAngles(bone->localInherentOrientation())));
            ImGui::TextUnformatted("Inherent Orientation");
            ImGui::InputFloat3("##orientation.inherent", glm::value_ptr(localInherentOrientation), "%.3f",
                ImGuiInputTextFlags_ReadOnly);
        }
        ImGui::Indent();
        ImGui::TreePop();
    }
    ImGui::PopItemWidth();
}

void
ModelParameterDialog::layoutBoneConstraintPanel(nanoem_model_bone_t *bonePtr, Project *project)
{
    nanoem_model_constraint_t *constraintPtr =
        const_cast<nanoem_model_constraint_t *>(nanoemModelBoneGetConstraintObject(bonePtr));
    if (constraintPtr && ImGui::TreeNode(constraintPtr, "%s", tr("nanoem.gui.model.edit.bone.inverse-kinematics"))) {
        ImGui::Unindent();
        char buffer[Inline::kNameStackBufferSize];
        nanoem_rsize_t numBones;
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
#if 0 /* same as bone */
            {
                const model::Bone *targetBone =
                    model::Bone::cast(nanoemModelConstraintGetTargetBoneObject(constraintPtr));
                ImGui::TextUnformatted(tr("nanoem.gui.model.edit.bone.constraint.target"));
                if (ImGui::BeginCombo(
                        "##constriant.target", targetBone ? targetBone->nameConstString() : "(none)")) {
                    for (nanoem_rsize_t i = 0; i < numBones; i++) {
                        const nanoem_model_bone_t *targetBonePtr = bones[i];
                        targetBone = model::Bone::cast(targetBonePtr);
                        if (targetBone && ImGui::Selectable(targetBone->nameConstString())) {
                            ScopedMutableConstraint scoped(constraintPtr, m_activeModel);
                            nanoemMutableModelConstraintSetTargetBoneObject(scoped, targetBonePtr);
                        }
                    }
                    ImGui::EndCombo();
                }
            }
#endif
        {
            const model::Bone *effectorBone =
                model::Bone::cast(nanoemModelConstraintGetEffectorBoneObject(constraintPtr));
            ImGui::TextUnformatted(tr("nanoem.gui.model.edit.bone.constraint.effector"));
            if (ImGui::BeginCombo("##constriant.effector", effectorBone ? effectorBone->nameConstString() : "(none)")) {
                for (nanoem_rsize_t i = 0; i < numBones; i++) {
                    const nanoem_model_bone_t *candidateBonePtr = bones[i];
                    const model::Bone *candidateBone = model::Bone::cast(candidateBonePtr);
                    if (candidateBone &&
                        ImGui::Selectable(candidateBone->nameConstString(), candidateBone == effectorBone)) {
                        ScopedMutableConstraint scoped(constraintPtr, m_activeModel);
                        nanoemMutableModelConstraintSetEffectorBoneObject(scoped, candidateBonePtr);
                    }
                }
                ImGui::EndCombo();
            }
        }
        {
            int value = nanoemModelConstraintGetNumIterations(constraintPtr);
            ImGui::TextUnformatted(tr("nanoem.gui.model.edit.bone.constraint.iteration"));
            if (ImGui::DragInt("##constraint.iterations", &value)) {
                ScopedMutableConstraint scoped(constraintPtr, m_activeModel);
                nanoemMutableModelConstraintSetNumIterations(scoped, value);
            }
        }
        {
            nanoem_f32_t value = glm::degrees(nanoemModelConstraintGetAngleLimit(constraintPtr));
            ImGui::TextUnformatted(tr("nanoem.gui.model.edit.bone.constraint.angle"));
            if (ImGui::SliderFloat("##constraint.angle", &value, -90, 90)) {
                ScopedMutableConstraint scoped(constraintPtr, m_activeModel);
                nanoemMutableModelConstraintSetAngleLimit(scoped, glm::radians(value));
            }
        }
        const ImVec2 panelWindowSize(ImGuiWindow::kLeftPaneWidth * project->windowDevicePixelRatio(),
            ImGui::GetTextLineHeightWithSpacing() * kInnerItemListFrameHeightRowCount);
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.bone.constraint.joints"));
        ImGui::BeginChild("##joints", panelWindowSize, true);
        nanoem_rsize_t numJoints;
        nanoem_model_constraint_joint_t *const *joints =
            nanoemModelConstraintGetAllJointObjects(constraintPtr, &numJoints);
        model::Bone::Set reservedConstraintJointBoneSet;
        {
            bool itemUp, itemDown;
            detectUpDown(itemUp, itemDown);
            selectIndex(itemUp, itemDown, numJoints, m_constraintJointIndex);
            for (nanoem_rsize_t i = 0; i < numJoints; i++) {
                const nanoem_model_constraint_joint_t *joint = joints[i];
                const nanoem_model_bone_t *bonePtr = nanoemModelConstraintJointGetBoneObject(joint);
                if (const model::Bone *bone = model::Bone::cast(bonePtr)) {
                    const bool selected = static_cast<nanoem_rsize_t>(i) == m_constraintJointIndex;
                    StringUtils::format(buffer, sizeof(buffer), "%s##joint[%lu].name", bone->nameConstString(), i);
                    if (ImGui::Selectable(buffer, selected) || ((itemUp || itemDown) && selected)) {
                        ImGui::SetScrollHereY();
                        m_constraintJointIndex = i;
                    }
                    reservedConstraintJointBoneSet.insert(bonePtr);
                }
            }
        }
        ImGui::EndChild();
        ImGui::SameLine();
        const ImVec2 propertyWindowSize(0, ImGui::GetTextLineHeightWithSpacing() * kInnerItemListFrameHeightRowCount);
        ImGui::BeginChild("##joint.properties", propertyWindowSize, false);
        {
            const char *name = "(select)";
            bool manipulatable = false;
            ImGui::PushItemWidth(-1);
            if (ImGui::BeginCombo("##candidate", name)) {
                for (nanoem_rsize_t i = 0; i < numBones; i++) {
                    const nanoem_model_bone_t *bonePtr = bones[i];
                    if (model::Bone *bone = model::Bone::cast(bonePtr)) {
                        const bool selected = static_cast<nanoem_rsize_t>(i) == m_constraintJointCandidateIndex;
                        const ImGuiSelectableFlags flags =
                            reservedConstraintJointBoneSet.find(bonePtr) != reservedConstraintJointBoneSet.end()
                            ? ImGuiSelectableFlags_Disabled
                            : ImGuiSelectableFlags_None;
                        StringUtils::format(buffer, sizeof(buffer), "%s##item[%lu].name", bone->nameConstString(), i);
                        if (ImGui::Selectable(buffer, selected, flags)) {
                            m_constraintJointCandidateIndex = i;
                        }
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            if (m_constraintJointCandidateIndex < numBones) {
                if (const model::Bone *bone = model::Bone::cast(bones[m_constraintJointCandidateIndex])) {
                    name = bone->nameConstString();
                    manipulatable = true;
                }
            }
            if (ImGuiWindow::handleButton("Add", ImGui::GetContentRegionAvail().x * 0.5f, manipulatable)) {
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                ScopedMutableConstraint scoped(constraintPtr, m_activeModel);
                nanoem_mutable_model_constraint_joint_t *joint =
                    nanoemMutableModelConstraintJointCreate(scoped, &status);
                nanoemMutableModelConstraintJointSetBoneObject(joint, bones[m_constraintJointCandidateIndex]);
                nanoemMutableModelConstraintInsertJointObject(scoped, joint, -1, &status);
                nanoemMutableModelConstraintJointDestroy(joint);
                joints = nanoemModelConstraintGetAllJointObjects(constraintPtr, &numJoints);
                m_constraintJointCandidateIndex = NANOEM_RSIZE_MAX;
            }
            ImGui::SameLine();
            if (ImGuiWindow::handleButton(
                    "Remove", ImGui::GetContentRegionAvail().x, m_constraintJointIndex < numJoints)) {
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                ScopedMutableConstraint scoped(constraintPtr, m_activeModel);
                nanoem_mutable_model_constraint_joint_t *joint =
                    nanoemMutableModelConstraintJointCreateAsReference(joints[m_constraintJointIndex], &status);
                nanoemMutableModelConstraintRemoveJointObject(scoped, joint, &status);
                nanoemMutableModelConstraintJointDestroy(joint);
            }
        }
        addSeparator();
        ImGui::PushItemWidth(-1);
        nanoem_model_constraint_joint_t *jointPtr =
            m_constraintJointIndex < numJoints ? joints[m_constraintJointIndex] : nullptr;
        {
            bool value = nanoemModelConstraintJointHasAngleLimit(jointPtr);
            if (ImGui::Checkbox("Enable Angle Limit", &value)) {
                ScopedMutableConstraintJoint scoped(jointPtr, m_activeModel);
                nanoemMutableModelConstraintJointSetAngleLimitEnabled(scoped, value ? 1 : 0);
            }
        }
        {
            Vector3 lowerLimit(glm::degrees(glm::make_vec3(nanoemModelConstraintJointGetLowerLimit(jointPtr))));
            ImGui::TextUnformatted("Lower Limit");
            if (ImGui::DragFloat3("##joint.upper", glm::value_ptr(lowerLimit), 1.0f, 0.0f, 180.0f)) {
                ScopedMutableConstraintJoint scoped(jointPtr, m_activeModel);
                nanoemMutableModelConstraintJointSetLowerLimit(scoped, glm::value_ptr(glm::radians(lowerLimit)));
            }
        }
        {
            Vector3 upperLimit(glm::degrees(glm::make_vec3(nanoemModelConstraintJointGetUpperLimit(jointPtr))));
            ImGui::TextUnformatted("Upper Limit");
            if (ImGui::DragFloat3("##joint.lower", glm::value_ptr(upperLimit), 1.0f, 0.0f, 180.0f)) {
                ScopedMutableConstraintJoint scoped(jointPtr, m_activeModel);
                nanoemMutableModelConstraintJointSetUpperLimit(scoped, glm::value_ptr(glm::radians(upperLimit)));
            }
        }
        ImGui::PopItemWidth();
        ImGui::EndChild();
        ImGui::Indent();
        ImGui::TreePop();
    }
}

void
ModelParameterDialog::layoutAllMorphs(Project *project)
{
    nanoem_rsize_t numMorphs;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_activeModel->data(), &numMorphs);
    nanoem_model_morph_t *hoveredMorphPtr = nullptr;
    nanoem_f32_t width = ImGuiWindow::kLeftPaneWidth * project->windowDevicePixelRatio();
    ImGui::BeginChild("left-pane", ImVec2(width, 0), false);
    if (ImGui::Button(reinterpret_cast<const char *>(ImGuiWindow::kFACogs))) {
        ImGui::OpenPopup("morph-op-menu");
    }
    if (ImGui::BeginPopup("morph-op-menu")) {
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    ImGui::Text("%lu / %lu", m_morphIndex + 1, numMorphs);
    ImGui::BeginChild(
        "left-pane-inner", ImVec2(width, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing()), true);
    ImGuiListClipper clipper;
    char buffer[Inline::kNameStackBufferSize];
    bool up, down;
    detectUpDown(up, down);
    selectIndex(up, down, numMorphs, m_morphIndex);
    project->setEditingMode(Project::kEditingModeNone);
    IModelObjectSelection *selection = m_activeModel->selection();
    clipper.Begin(Inline::saturateInt32(numMorphs));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            const nanoem_model_morph_t *morphPtr = morphs[i];
            model::Morph *morph = model::Morph::cast(morphPtr);
            const bool selected = selection->containsMorph(morphPtr);
            StringUtils::format(buffer, sizeof(buffer), "%s##morph[%d].name", morph->nameConstString(), i);
            if (ImGui::Selectable(buffer, selected)) {
                ImGui::SetScrollHereY();
                if (ImGui::GetIO().KeyCtrl) {
                    selected ? selection->removeMorph(morphPtr) : selection->addMorph(morphPtr);
                }
                else {
                    if (!ImGui::GetIO().KeyShift) {
                        selection->removeAllMorphs();
                        forceUpdateMorph(morph, project);
                        m_morphIndex = i;
                        m_morphItemIndex = 0;
                    }
                    selection->addMorph(morphPtr);
                }
                hoveredMorphPtr = morphs[i];
            }
            else if ((up || down) && m_morphIndex == static_cast<nanoem_rsize_t>(i)) {
                ImGui::SetScrollHereY();
                forceUpdateMorph(morph, project);
                m_morphIndex = i;
                m_morphItemIndex = 0;
            }
            else if (ImGui::IsItemHovered()) {
                hoveredMorphPtr = morphs[i];
            }
        }
    }
    selection->setHoveredMorph(hoveredMorphPtr);
    ImGui::EndChild();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAPlus), 0, true)) {
        ImGui::OpenPopup("morph-op-menu");
    }
    if (ImGui::BeginPopup("morph-op-menu")) {
        struct CreateMorphCommand : ImGuiWindow::ILazyExecutionCommand {
            CreateMorphCommand(
                nanoem_rsize_t numMorphs, int offset, const nanoem_model_morph_t *base, nanoem_model_morph_type_t type)
                : m_base(base)
                , m_type(type)
                , m_numMorphs(numMorphs)
                , m_offset(offset)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableMorph morph(activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
                StringUtils::UnicodeStringScope scope(factory);
                char buffer[Inline::kMarkerStringLength];
                if (m_base) {
                    nanoemMutableModelMorphCopy(morph, m_base, &status);
                }
                else {
                    nanoemMutableModelMorphSetType(morph, m_type);
                }
                StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, m_numMorphs + 1);
                if (StringUtils::tryGetString(factory, buffer, scope)) {
                    nanoemMutableModelMorphSetName(morph, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
                }
                StringUtils::format(buffer, sizeof(buffer), "NewMorph%zu", m_numMorphs + 1);
                if (StringUtils::tryGetString(factory, buffer, scope)) {
                    nanoemMutableModelMorphSetName(morph, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
                }
                nanoemMutableModelInsertMorphObject(model, morph, m_offset, &status);
                model::Morph *newMorph = model::Morph::create();
                newMorph->bind(nanoemMutableModelMorphGetOriginObject(morph));
                newMorph->resetLanguage(
                    nanoemMutableModelMorphGetOriginObject(morph), factory, project->castLanguage());
                project->rebuildAllTracks();
            }
            const nanoem_model_morph_t *m_base;
            const nanoem_model_morph_type_t m_type;
            const nanoem_rsize_t m_numMorphs;
            const int m_offset;
        };
        const nanoem_model_morph_t *selectedMorph = numMorphs > 0 ? morphs[m_morphIndex] : nullptr;
        int selectedMorphIndex = nanoemModelObjectGetIndex(nanoemModelMorphGetModelObject(selectedMorph));
        if (ImGui::BeginMenu("Insert New")) {
            if (ImGui::BeginMenu("Bone Morph")) {
                const nanoem_model_morph_type_t type = NANOEM_MODEL_MORPH_TYPE_BONE;
                if (ImGui::MenuItem("at Last")) {
                    m_parent->addLazyExecutionCommand(nanoem_new(CreateMorphCommand(numMorphs, -1, nullptr, type)));
                }
                if (ImGui::MenuItem("at Next")) {
                    m_parent->addLazyExecutionCommand(
                        nanoem_new(CreateMorphCommand(numMorphs, selectedMorphIndex + 1, nullptr, type)));
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Group Morph")) {
                const nanoem_model_morph_type_t type = NANOEM_MODEL_MORPH_TYPE_GROUP;
                if (ImGui::MenuItem("at Last")) {
                    m_parent->addLazyExecutionCommand(nanoem_new(CreateMorphCommand(numMorphs, -1, nullptr, type)));
                }
                if (ImGui::MenuItem("at Next")) {
                    m_parent->addLazyExecutionCommand(
                        nanoem_new(CreateMorphCommand(numMorphs, selectedMorphIndex + 1, nullptr, type)));
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Material Morph")) {
                const nanoem_model_morph_type_t type = NANOEM_MODEL_MORPH_TYPE_MATERIAL;
                if (ImGui::MenuItem("at Last")) {
                    m_parent->addLazyExecutionCommand(nanoem_new(CreateMorphCommand(numMorphs, -1, nullptr, type)));
                }
                if (ImGui::MenuItem("at Next")) {
                    m_parent->addLazyExecutionCommand(
                        nanoem_new(CreateMorphCommand(numMorphs, selectedMorphIndex + 1, nullptr, type)));
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("UV Morph")) {
                const nanoem_model_morph_type_t type = NANOEM_MODEL_MORPH_TYPE_TEXTURE;
                if (ImGui::MenuItem("at Last")) {
                    m_parent->addLazyExecutionCommand(nanoem_new(CreateMorphCommand(numMorphs, -1, nullptr, type)));
                }
                if (ImGui::MenuItem("at Next")) {
                    m_parent->addLazyExecutionCommand(
                        nanoem_new(CreateMorphCommand(numMorphs, selectedMorphIndex + 1, nullptr, type)));
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Vertex Morph")) {
                const nanoem_model_morph_type_t type = NANOEM_MODEL_MORPH_TYPE_VERTEX;
                if (ImGui::MenuItem("at Last")) {
                    m_parent->addLazyExecutionCommand(nanoem_new(CreateMorphCommand(numMorphs, -1, nullptr, type)));
                }
                if (ImGui::MenuItem("at Next")) {
                    m_parent->addLazyExecutionCommand(
                        nanoem_new(CreateMorphCommand(numMorphs, selectedMorphIndex + 1, nullptr, type)));
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Flip Morph")) {
                const nanoem_model_morph_type_t type = NANOEM_MODEL_MORPH_TYPE_FLIP;
                if (ImGui::MenuItem("at Last")) {
                    m_parent->addLazyExecutionCommand(nanoem_new(CreateMorphCommand(numMorphs, -1, nullptr, type)));
                }
                if (ImGui::MenuItem("at Next")) {
                    m_parent->addLazyExecutionCommand(
                        nanoem_new(CreateMorphCommand(numMorphs, selectedMorphIndex + 1, nullptr, type)));
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Impulse Morph")) {
                const nanoem_model_morph_type_t type = NANOEM_MODEL_MORPH_TYPE_IMPULUSE;
                if (ImGui::MenuItem("at Last")) {
                    m_parent->addLazyExecutionCommand(nanoem_new(CreateMorphCommand(numMorphs, -1, nullptr, type)));
                }
                if (ImGui::MenuItem("at Next")) {
                    m_parent->addLazyExecutionCommand(
                        nanoem_new(CreateMorphCommand(numMorphs, selectedMorphIndex + 1, nullptr, type)));
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Insert Copy")) {
            if (ImGui::MenuItem("at Last")) {
                m_parent->addLazyExecutionCommand(
                    nanoem_new(CreateMorphCommand(numMorphs, -1, selectedMorph, NANOEM_MODEL_MORPH_TYPE_UNKNOWN)));
            }
            if (ImGui::MenuItem("at Next")) {
                m_parent->addLazyExecutionCommand(nanoem_new(CreateMorphCommand(
                    numMorphs, selectedMorphIndex + 1, selectedMorph, NANOEM_MODEL_MORPH_TYPE_UNKNOWN)));
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAMinus), 0, m_morphIndex < numMorphs)) {
        struct DeleteMorphCommand : ImGuiWindow::ILazyExecutionCommand {
            DeleteMorphCommand(nanoem_model_morph_t *const *morphs, nanoem_rsize_t &morphIndex)
                : m_morphs(morphs)
                , m_morphIndex(morphIndex)
            {
            }
            void
            execute(Project *project)
            {
                nanoem_model_morph_t *morphPtr = m_morphs[m_morphIndex];
                Model *activeModel = project->activeModel();
                ScopedMutableMorph morph(morphPtr, activeModel);
                ScopedMutableModel model(activeModel);
                for (int i = NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM; i < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM; i++) {
                    nanoem_model_morph_category_t category = static_cast<nanoem_model_morph_category_t>(i);
                    if (activeModel->activeMorph(category) == morphPtr) {
                        activeModel->setActiveMorph(category, nullptr);
                    }
                }
                activeModel->removeMorph(morphPtr);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelRemoveMorphObject(model, morph, &status);
                project->rebuildAllTracks();
                if (m_morphIndex > 0) {
                    m_morphIndex--;
                }
            }
            nanoem_model_morph_t *const *m_morphs;
            nanoem_rsize_t &m_morphIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(DeleteMorphCommand(morphs, m_morphIndex)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowUp), 0, m_morphIndex > 0)) {
        struct MoveMorphUpCommand : ImGuiWindow::ILazyExecutionCommand {
            MoveMorphUpCommand(nanoem_model_morph_t *const *morphs, nanoem_rsize_t &morphIndex)
                : m_morphs(morphs)
                , m_morphIndex(morphIndex)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableMorph morph(m_morphs[m_morphIndex], activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelRemoveMorphObject(model, morph, &status);
                int offset = Inline::saturateInt32(--m_morphIndex);
                nanoemMutableModelInsertMorphObject(model, morph, offset, &status);
            }
            nanoem_model_morph_t *const *m_morphs;
            nanoem_rsize_t &m_morphIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(MoveMorphUpCommand(morphs, m_morphIndex)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(
            reinterpret_cast<const char *>(ImGuiWindow::kFAArrowDown), 0, m_morphIndex < numMorphs - 1)) {
        struct MoveMorphDownCommand : ImGuiWindow::ILazyExecutionCommand {
            MoveMorphDownCommand(nanoem_model_morph_t *const *morphs, nanoem_rsize_t &morphIndex)
                : m_morphs(morphs)
                , m_morphIndex(morphIndex)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableMorph morph(m_morphs[m_morphIndex], activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelRemoveMorphObject(model, morph, &status);
                int offset = Inline::saturateInt32(--m_morphIndex);
                nanoemMutableModelInsertMorphObject(model, morph, offset, &status);
            }
            nanoem_model_morph_t *const *m_morphs;
            nanoem_rsize_t &m_morphIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(MoveMorphDownCommand(morphs, m_morphIndex)));
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("right-pane", ImGui::GetContentRegionAvail());
    if (numMorphs > 0 && m_morphIndex < numMorphs) {
        nanoem_model_morph_t *morphPtr = morphs[m_morphIndex];
        layoutMorphPropertyPane(morphPtr, project);
    }
    ImGui::EndChild();
}

void
ModelParameterDialog::layoutMorphPropertyPane(nanoem_model_morph_t *morphPtr, Project *project)
{
    char buffer[Inline::kNameStackBufferSize];
    nanoem_language_type_t language = static_cast<nanoem_language_type_t>(m_language);
    StringUtils::UnicodeStringScope scope(project->unicodeStringFactory());
    ImGui::PushItemWidth(-1);
    if (layoutName(nanoemModelMorphGetName(morphPtr, language), project, scope)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        ScopedMutableMorph scoped(morphPtr, m_activeModel);
        nanoemMutableModelMorphSetName(scoped, scope.value(), language, &status);
    }
    {
        nanoem_model_morph_category_t value = nanoemModelMorphGetCategory(morphPtr);
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.category"));
        if (ImGui::BeginCombo("##category", selectedMorphCategory(value))) {
            /* skip base */
            for (int i = NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM + 1; i < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM; i++) {
                nanoem_model_morph_category_t type = static_cast<nanoem_model_morph_category_t>(i);
                if (ImGui::Selectable(selectedMorphCategory(type))) {
                    ScopedMutableMorph scoped(morphPtr, m_activeModel);
                    nanoemMutableModelMorphSetCategory(scoped, type);
                }
            }
            ImGui::EndCombo();
        }
    }
    {
        nanoem_model_morph_type_t value = nanoemModelMorphGetType(morphPtr);
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.type"));
        if (ImGui::BeginCombo("##type", selectedMorphType(value))) {
            for (int i = NANOEM_MODEL_MORPH_TYPE_FIRST_ENUM; i < NANOEM_MODEL_MORPH_TYPE_MAX_ENUM; i++) {
                nanoem_model_morph_type_t type = static_cast<nanoem_model_morph_type_t>(i);
                if (ImGui::Selectable(selectedMorphType(type))) {
                    ScopedMutableMorph scoped(morphPtr, m_activeModel);
                    nanoemMutableModelMorphSetType(scoped, type);
                }
            }
            ImGui::EndCombo();
        }
    }
    addSeparator();
    {
        const ImVec2 panelWindowSize(ImGuiWindow::kLeftPaneWidth * project->windowDevicePixelRatio(),
            ImGui::GetTextLineHeightWithSpacing() * kInnerItemListFrameHeightRowCount),
            propertyWindowSize(0, ImGui::GetTextLineHeightWithSpacing() * kInnerItemListFrameHeightRowCount);
        ImGui::BeginChild("##items", panelWindowSize, true);
        {
            bool itemUp, itemDown;
            detectUpDown(itemUp, itemDown);
            nanoem_rsize_t numItems;
            switch (nanoemModelMorphGetType(morphPtr)) {
            case NANOEM_MODEL_MORPH_TYPE_BONE: {
                model::Bone::Set reservedBoneSet;
                nanoem_model_morph_bone_t *const *items = nanoemModelMorphGetAllBoneMorphObjects(morphPtr, &numItems);
                selectIndex(itemUp, itemDown, numItems, m_morphItemIndex);
                for (nanoem_rsize_t i = 0; i < numItems; i++) {
                    const nanoem_model_morph_bone_t *item = items[i];
                    const nanoem_model_bone_t *bonePtr = nanoemModelMorphBoneGetBoneObject(item);
                    if (const model::Bone *bone = model::Bone::cast(bonePtr)) {
                        const bool selected = static_cast<nanoem_rsize_t>(i) == m_morphItemIndex;
                        StringUtils::format(buffer, sizeof(buffer), "%s##item[%lu].name", bone->nameConstString(), i);
                        if (ImGui::Selectable(buffer, selected) || ((itemUp || itemDown) && selected)) {
                            ImGui::SetScrollHereY();
                            m_morphItemIndex = i;
                        }
                    }
                }
                ImGui::EndChild();
                ImGui::SameLine();
                ImGui::BeginChild("##bone.properties", propertyWindowSize, false);
                ImGui::PushItemWidth(-1);
                nanoem_rsize_t numBones = 0;
                nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
                const char *name = "(select)";
                if (m_morphItemCandidateBoneIndex < numBones) {
                    if (model::Bone *bone = model::Bone::cast(bones[m_morphItemCandidateBoneIndex])) {
                        name = bone->nameConstString();
                    }
                }
                if (ImGui::BeginCombo("##candidate", name)) {
                    for (nanoem_rsize_t i = 0; i < numBones; i++) {
                        const nanoem_model_bone_t *bonePtr = bones[i];
                        if (model::Bone *bone = model::Bone::cast(bonePtr)) {
                            const bool selected = static_cast<nanoem_rsize_t>(i) == m_morphItemCandidateBoneIndex;
                            const ImGuiSelectableFlags flags = reservedBoneSet.find(bonePtr) != reservedBoneSet.end()
                                ? ImGuiSelectableFlags_Disabled
                                : ImGuiSelectableFlags_None;
                            StringUtils::format(
                                buffer, sizeof(buffer), "%s##item[%lu].name", bone->nameConstString(), i);
                            if (ImGui::Selectable(buffer, selected, flags)) {
                                m_morphItemCandidateBoneIndex = i;
                            }
                            reservedBoneSet.insert(bonePtr);
                        }
                    }
                    ImGui::EndCombo();
                }
                if (ImGuiWindow::handleButton(
                        "Add", ImGui::GetContentRegionAvail().x * 0.5f, m_morphItemCandidateBoneIndex < numBones)) {
                    nanoem_status_t status;
                    ScopedMutableMorph scoped(morphPtr, m_activeModel);
                    nanoem_mutable_model_morph_bone_t *item = nanoemMutableModelMorphBoneCreate(scoped, &status);
                    nanoemMutableModelMorphBoneSetBoneObject(item, bones[m_morphItemCandidateBoneIndex]);
                    nanoemMutableModelMorphInsertBoneMorphObject(scoped, item, -1, &status);
                    nanoemMutableModelMorphBoneDestroy(item);
                    items = nanoemModelMorphGetAllBoneMorphObjects(morphPtr, &numItems);
                    m_morphItemCandidateBoneIndex = NANOEM_RSIZE_MAX;
                }
                ImGui::SameLine();
                if (ImGuiWindow::handleButton(
                        "Remove", ImGui::GetContentRegionAvail().x, m_morphItemIndex < numItems)) {
                    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                    ScopedMutableMorph scoped(morphPtr, m_activeModel);
                    nanoem_mutable_model_morph_bone_t *item =
                        nanoemMutableModelMorphBoneCreateAsReference(items[m_morphItemIndex], &status);
                    nanoemMutableModelMorphRemoveBoneMorphObject(scoped, item, &status);
                    nanoemMutableModelMorphBoneDestroy(item);
                }
                addSeparator();
                nanoem_model_morph_bone_t *bonePtr = m_morphItemIndex < numItems ? items[m_morphItemIndex] : nullptr;
                {
                    Vector4 translation(glm::make_vec4(nanoemModelMorphBoneGetTranslation(bonePtr)));
                    ImGui::TextUnformatted("Translation");
                    if (ImGui::DragFloat3("##bone.translation", glm::value_ptr(translation), 1.0f, 0.0f, 0.0f)) {
                        ScopedMutableMorphBone scoped(bonePtr, m_activeModel);
                        nanoemMutableModelMorphBoneSetTranslation(scoped, glm::value_ptr(translation));
                    }
                }
                {
                    const Quaternion orientation(glm::make_quat(nanoemModelMorphBoneGetOrientation(bonePtr)));
                    Vector3 angles(glm::degrees(glm::eulerAngles(orientation)));
                    ImGui::TextUnformatted("Orientation");
                    if (ImGui::DragFloat3("##bone.orientation", glm::value_ptr(angles), 1.0f, 0.0f, 0.0f)) {
                        ScopedMutableMorphBone scoped(bonePtr, m_activeModel);
                        nanoemMutableModelMorphBoneSetOrientation(
                            scoped, glm::value_ptr(glm::quat(glm::radians(angles))));
                    }
                }
                ImGui::PopItemWidth();
                break;
            }
            case NANOEM_MODEL_MORPH_TYPE_FLIP: {
                model::Morph::Set reservedMorphSet;
                nanoem_model_morph_flip_t *const *items = nanoemModelMorphGetAllFlipMorphObjects(morphPtr, &numItems);
                selectIndex(itemUp, itemDown, numItems, m_morphItemIndex);
                for (nanoem_rsize_t i = 0; i < numItems; i++) {
                    const nanoem_model_morph_flip_t *item = items[i];
                    const nanoem_model_morph_t *morphPtr = nanoemModelMorphFlipGetMorphObject(item);
                    if (const model::Morph *morph = model::Morph::cast(morphPtr)) {
                        const bool selected = static_cast<nanoem_rsize_t>(i) == m_morphItemIndex;
                        StringUtils::format(buffer, sizeof(buffer), "%s##item[%lu].name", morph->nameConstString(), i);
                        if (ImGui::Selectable(buffer, selected) || ((itemUp || itemDown) && selected)) {
                            ImGui::SetScrollHereY();
                            m_morphItemIndex = i;
                        }
                        reservedMorphSet.insert(morphPtr);
                    }
                }
                ImGui::EndChild();
                ImGui::SameLine();
                ImGui::BeginChild("##flip.properties", propertyWindowSize, false);
                ImGui::PushItemWidth(-1);
                nanoem_rsize_t numMorphs;
                nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_activeModel->data(), &numMorphs);
                const char *name = "(select)";
                if (m_morphItemCandidateMorphIndex < numMorphs) {
                    if (model::Morph *morph = model::Morph::cast(morphs[m_morphItemCandidateMorphIndex])) {
                        name = morph->nameConstString();
                    }
                }
                if (ImGui::BeginCombo("##candidate", name)) {
                    for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
                        const nanoem_model_morph_t *morphPtr = morphs[i];
                        if (model::Morph *morph = model::Morph::cast(morphPtr)) {
                            const bool selected = static_cast<nanoem_rsize_t>(i) == m_morphItemCandidateMorphIndex;
                            const ImGuiSelectableFlags flags = reservedMorphSet.find(morphPtr) != reservedMorphSet.end()
                                ? ImGuiSelectableFlags_Disabled
                                : ImGuiSelectableFlags_None;
                            StringUtils::format(
                                buffer, sizeof(buffer), "%s##item[%lu].name", morph->nameConstString(), i);
                            if (ImGui::Selectable(buffer, selected, flags)) {
                                m_morphItemCandidateMorphIndex = i;
                            }
                            reservedMorphSet.insert(morphPtr);
                        }
                    }
                    ImGui::EndCombo();
                }
                if (ImGuiWindow::handleButton(
                        "Add", ImGui::GetContentRegionAvail().x * 0.5f, m_morphItemCandidateMorphIndex < numMorphs)) {
                    nanoem_status_t status;
                    ScopedMutableMorph scoped(morphPtr, m_activeModel);
                    nanoem_mutable_model_morph_flip_t *item = nanoemMutableModelMorphFlipCreate(scoped, &status);
                    nanoemMutableModelMorphFlipSetMorphObject(item, morphs[m_morphItemCandidateMorphIndex]);
                    nanoemMutableModelMorphInsertFlipMorphObject(scoped, item, -1, &status);
                    nanoemMutableModelMorphFlipDestroy(item);
                    items = nanoemModelMorphGetAllFlipMorphObjects(morphPtr, &numItems);
                    m_morphItemCandidateMorphIndex = NANOEM_RSIZE_MAX;
                }
                ImGui::SameLine();
                if (ImGuiWindow::handleButton(
                        "Remove", ImGui::GetContentRegionAvail().x, m_morphItemIndex < numItems)) {
                    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                    ScopedMutableMorph scoped(morphPtr, m_activeModel);
                    nanoem_mutable_model_morph_flip_t *item =
                        nanoemMutableModelMorphFlipCreateAsReference(items[m_morphItemIndex], &status);
                    nanoemMutableModelMorphRemoveFlipMorphObject(scoped, item, &status);
                    nanoemMutableModelMorphFlipDestroy(item);
                }
                addSeparator();
                nanoem_model_morph_flip_t *flipPtr = m_morphItemIndex < numItems ? items[m_morphItemIndex] : nullptr;
                {
                    nanoem_f32_t weight = nanoemModelMorphFlipGetWeight(flipPtr);
                    ImGui::TextUnformatted("Weight");
                    if (ImGui::SliderFloat("##flip.weight", &weight, 0.0f, 0.0f)) {
                        ScopedMutableMorphFlip scoped(flipPtr, m_activeModel);
                        nanoemMutableModelMorphFlipSetWeight(scoped, weight);
                    }
                }
                ImGui::PopItemWidth();
                break;
            }
            case NANOEM_MODEL_MORPH_TYPE_GROUP: {
                model::Morph::Set reservedMorphSet;
                nanoem_model_morph_group_t *const *items = nanoemModelMorphGetAllGroupMorphObjects(morphPtr, &numItems);
                selectIndex(itemUp, itemDown, numItems, m_morphItemIndex);
                for (nanoem_rsize_t i = 0; i < numItems; i++) {
                    const nanoem_model_morph_group_t *item = items[i];
                    const nanoem_model_morph_t *morphPtr = nanoemModelMorphGroupGetMorphObject(item);
                    if (const model::Morph *morph = model::Morph::cast(morphPtr)) {
                        const bool selected = static_cast<nanoem_rsize_t>(i) == m_morphItemIndex;
                        StringUtils::format(buffer, sizeof(buffer), "%s##item[%lu].name", morph->nameConstString(), i);
                        if (ImGui::Selectable(buffer, selected) || ((itemUp || itemDown) && selected)) {
                            ImGui::SetScrollHereY();
                            m_morphItemIndex = i;
                        }
                        reservedMorphSet.insert(morphPtr);
                    }
                }
                ImGui::EndChild();
                ImGui::SameLine();
                ImGui::BeginChild("##group.properties", propertyWindowSize, false);
                ImGui::PushItemWidth(-1);
                nanoem_rsize_t numMorphs;
                nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_activeModel->data(), &numMorphs);
                const char *name = "(select)";
                if (m_morphItemCandidateMorphIndex < numMorphs) {
                    if (model::Morph *morph = model::Morph::cast(morphs[m_morphItemCandidateMorphIndex])) {
                        name = morph->nameConstString();
                    }
                }
                if (ImGui::BeginCombo("##candidate", name)) {
                    for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
                        const nanoem_model_morph_t *morphPtr = morphs[i];
                        if (model::Morph *morph = model::Morph::cast(morphPtr)) {
                            const bool selected = static_cast<nanoem_rsize_t>(i) == m_morphItemCandidateMorphIndex;
                            const ImGuiSelectableFlags flags = reservedMorphSet.find(morphPtr) != reservedMorphSet.end()
                                ? ImGuiSelectableFlags_Disabled
                                : ImGuiSelectableFlags_None;
                            StringUtils::format(
                                buffer, sizeof(buffer), "%s##item[%lu].name", morph->nameConstString(), i);
                            if (ImGui::Selectable(buffer, selected, flags)) {
                                m_morphItemCandidateMorphIndex = i;
                            }
                            reservedMorphSet.insert(morphPtr);
                        }
                    }
                    ImGui::EndCombo();
                }
                if (ImGuiWindow::handleButton("Add", ImGui::GetContentRegionAvail().x * 0.5f, true)) {
                    nanoem_status_t status;
                    ScopedMutableMorph scoped(morphPtr, m_activeModel);
                    nanoem_mutable_model_morph_group_t *item = nanoemMutableModelMorphGroupCreate(scoped, &status);
                    nanoemMutableModelMorphGroupSetMorphObject(item, morphs[m_morphItemCandidateMorphIndex]);
                    nanoemMutableModelMorphInsertGroupMorphObject(scoped, item, -1, &status);
                    nanoemMutableModelMorphGroupDestroy(item);
                    items = nanoemModelMorphGetAllGroupMorphObjects(morphPtr, &numItems);
                    m_morphItemCandidateMorphIndex = NANOEM_RSIZE_MAX;
                }
                ImGui::SameLine();
                if (ImGuiWindow::handleButton(
                        "Remove", ImGui::GetContentRegionAvail().x, m_morphItemIndex < numItems)) {
                    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                    ScopedMutableMorph scoped(morphPtr, m_activeModel);
                    nanoem_mutable_model_morph_group_t *item =
                        nanoemMutableModelMorphGroupCreateAsReference(items[m_morphItemIndex], &status);
                    nanoemMutableModelMorphRemoveGroupMorphObject(scoped, item, &status);
                    nanoemMutableModelMorphGroupDestroy(item);
                }
                addSeparator();
                nanoem_model_morph_group_t *groupPtr = m_morphItemIndex < numItems ? items[m_morphItemIndex] : nullptr;
                {
                    nanoem_f32_t weight = nanoemModelMorphGroupGetWeight(groupPtr);
                    ImGui::TextUnformatted("Weight");
                    if (ImGui::SliderFloat("##group.weight", &weight, 0.0f, 0.0f)) {
                        ScopedMutableMorphGroup scoped(groupPtr, m_activeModel);
                        nanoemMutableModelMorphGroupSetWeight(scoped, weight);
                    }
                }
                ImGui::PopItemWidth();
                break;
            }
            case NANOEM_MODEL_MORPH_TYPE_IMPULUSE: {
                model::RigidBody::Set reservedRigidBodySet;
                nanoem_model_morph_impulse_t *const *items =
                    nanoemModelMorphGetAllImpulseMorphObjects(morphPtr, &numItems);
                selectIndex(itemUp, itemDown, numItems, m_morphItemIndex);
                for (nanoem_rsize_t i = 0; i < numItems; i++) {
                    const nanoem_model_morph_impulse_t *item = items[i];
                    const nanoem_model_rigid_body_t *rigidBodyPtr = nanoemModelMorphImpulseGetRigidBodyObject(item);
                    if (const model::RigidBody *body = model::RigidBody::cast(rigidBodyPtr)) {
                        const bool selected = static_cast<nanoem_rsize_t>(i) == m_morphItemIndex;
                        StringUtils::format(buffer, sizeof(buffer), "%s##item[%lu].name", body->nameConstString(), i);
                        if (ImGui::Selectable(buffer, selected) || ((itemUp || itemDown) && selected)) {
                            ImGui::SetScrollHereY();
                            m_morphItemIndex = i;
                        }
                        reservedRigidBodySet.insert(rigidBodyPtr);
                    }
                }
                ImGui::EndChild();
                ImGui::SameLine();
                ImGui::BeginChild("##impulse.properties", propertyWindowSize, false);
                ImGui::PushItemWidth(-1);
                nanoem_rsize_t numRigidBodies;
                nanoem_model_rigid_body_t *const *rigidBodies =
                    nanoemModelGetAllRigidBodyObjects(m_activeModel->data(), &numRigidBodies);
                const char *name = "(select)";
                if (m_morphItemCandidateRigidBodyIndex < numRigidBodies) {
                    if (model::RigidBody *rigidBody =
                            model::RigidBody::cast(rigidBodies[m_morphItemCandidateRigidBodyIndex])) {
                        name = rigidBody->nameConstString();
                    }
                }
                if (ImGui::BeginCombo("##candidate", name)) {
                    for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
                        const nanoem_model_rigid_body_t *rigidBodyPtr = rigidBodies[i];
                        if (model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr)) {
                            const bool selected = static_cast<nanoem_rsize_t>(i) == m_morphItemCandidateRigidBodyIndex;
                            const ImGuiSelectableFlags flags =
                                reservedRigidBodySet.find(rigidBodyPtr) != reservedRigidBodySet.end()
                                ? ImGuiSelectableFlags_Disabled
                                : ImGuiSelectableFlags_None;
                            StringUtils::format(
                                buffer, sizeof(buffer), "%s##item[%lu].name", rigidBody->nameConstString(), i);
                            if (ImGui::Selectable(buffer, selected, flags)) {
                                m_morphItemCandidateRigidBodyIndex = i;
                            }
                            reservedRigidBodySet.insert(rigidBodyPtr);
                        }
                    }
                    ImGui::EndCombo();
                }
                if (ImGuiWindow::handleButton("Add", ImGui::GetContentRegionAvail().x * 0.5f,
                        m_morphItemCandidateRigidBodyIndex < numRigidBodies)) {
                    nanoem_status_t status;
                    ScopedMutableMorph scoped(morphPtr, m_activeModel);
                    nanoem_mutable_model_morph_impulse_t *item = nanoemMutableModelMorphImpulseCreate(scoped, &status);
                    nanoemMutableModelMorphImpulseSetRigidBodyObject(
                        item, rigidBodies[m_morphItemCandidateRigidBodyIndex]);
                    nanoemMutableModelMorphInsertImpulseMorphObject(scoped, item, -1, &status);
                    nanoemMutableModelMorphImpulseDestroy(item);
                    items = nanoemModelMorphGetAllImpulseMorphObjects(morphPtr, &numItems);
                    m_morphItemCandidateRigidBodyIndex = NANOEM_RSIZE_MAX;
                }
                ImGui::SameLine();
                if (ImGuiWindow::handleButton(
                        "Remove", ImGui::GetContentRegionAvail().x, m_morphItemIndex < numItems)) {
                    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                    ScopedMutableMorph scoped(morphPtr, m_activeModel);
                    nanoem_mutable_model_morph_impulse_t *item =
                        nanoemMutableModelMorphImpulseCreateAsReference(items[m_morphItemIndex], &status);
                    nanoemMutableModelMorphRemoveImpulseMorphObject(scoped, item, &status);
                    nanoemMutableModelMorphImpulseDestroy(item);
                }
                addSeparator();
                nanoem_model_morph_impulse_t *impulsePtr =
                    m_morphItemIndex < numItems ? items[m_morphItemIndex] : nullptr;
                {
                    Vector4 torque(glm::make_vec4(nanoemModelMorphImpulseGetTorque(impulsePtr)));
                    ImGui::TextUnformatted("Torque");
                    if (ImGui::DragFloat3("##impulse.torque", glm::value_ptr(torque), 1.0f, 0.0f, 0.0f)) {
                        ScopedMutableMorphImpulse scoped(impulsePtr, m_activeModel);
                        nanoemMutableModelMorphImpulseSetTorque(scoped, glm::value_ptr(torque));
                    }
                }
                {
                    Vector4 torque(glm::make_vec4(nanoemModelMorphImpulseGetVelocity(impulsePtr)));
                    ImGui::TextUnformatted("Velocity");
                    if (ImGui::DragFloat3("##impulse.velocity", glm::value_ptr(torque), 1.0f, 0.0f, 0.0f)) {
                        ScopedMutableMorphImpulse scoped(impulsePtr, m_activeModel);
                        nanoemMutableModelMorphImpulseSetVelocity(scoped, glm::value_ptr(torque));
                    }
                }
                {
                    bool isLocal = nanoemModelMorphImpulseIsLocal(impulsePtr) != 0;
                    ImGui::TextUnformatted("Local");
                    if (ImGui::Checkbox("##impuluse.local", &isLocal)) {
                        ScopedMutableMorphImpulse scoped(impulsePtr, m_activeModel);
                        nanoemMutableModelMorphImpulseSetLocal(scoped, isLocal ? 1 : 0);
                    }
                }
                ImGui::PopItemWidth();
                break;
            }
            case NANOEM_MODEL_MORPH_TYPE_MATERIAL: {
                model::Material::Set reservedMaterialSet;
                nanoem_model_morph_material_t *const *items =
                    nanoemModelMorphGetAllMaterialMorphObjects(morphPtr, &numItems);
                selectIndex(itemUp, itemDown, numItems, m_morphItemIndex);
                for (nanoem_rsize_t i = 0; i < numItems; i++) {
                    const nanoem_model_morph_material_t *item = items[i];
                    const nanoem_model_material_t *materialPtr = nanoemModelMorphMaterialGetMaterialObject(item);
                    if (const model::Material *material = model::Material::cast(materialPtr)) {
                        const bool selected = static_cast<nanoem_rsize_t>(i) == m_morphItemIndex;
                        StringUtils::format(
                            buffer, sizeof(buffer), "%s##item[%lu].name", material->nameConstString(), i);
                        if (ImGui::Selectable(buffer, selected) || ((itemUp || itemDown) && selected)) {
                            ImGui::SetScrollHereY();
                            m_morphItemIndex = i;
                        }
                        reservedMaterialSet.insert(materialPtr);
                    }
                }
                ImGui::EndChild();
                ImGui::SameLine();
                ImGui::BeginChild("##material.properties", propertyWindowSize, false);
                ImGui::PushItemWidth(-1);
                nanoem_rsize_t numMaterials;
                nanoem_model_material_t *const *materials =
                    nanoemModelGetAllMaterialObjects(m_activeModel->data(), &numMaterials);
                const char *name = "(select)";
                if (m_morphItemCandidateMaterialIndex < numMaterials) {
                    if (model::Material *material =
                            model::Material::cast(materials[m_morphItemCandidateMaterialIndex])) {
                        name = material->nameConstString();
                    }
                }
                if (ImGui::BeginCombo("##candidate", name)) {
                    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
                        const nanoem_model_material_t *materialPtr = materials[i];
                        if (model::Material *material = model::Material::cast(materialPtr)) {
                            const bool selected = static_cast<nanoem_rsize_t>(i) == m_morphItemCandidateMaterialIndex;
                            const ImGuiSelectableFlags flags =
                                reservedMaterialSet.find(materialPtr) != reservedMaterialSet.end()
                                ? ImGuiSelectableFlags_Disabled
                                : ImGuiSelectableFlags_None;
                            StringUtils::format(
                                buffer, sizeof(buffer), "%s##item[%lu].name", material->nameConstString(), i);
                            if (ImGui::Selectable(buffer, selected, flags)) {
                                m_morphItemCandidateMaterialIndex = i;
                            }
                        }
                    }
                    ImGui::EndCombo();
                }
                if (ImGuiWindow::handleButton("Add", ImGui::GetContentRegionAvail().x * 0.5f,
                        m_morphItemCandidateMaterialIndex < numMaterials)) {
                    nanoem_status_t status;
                    ScopedMutableMorph scoped(morphPtr, m_activeModel);
                    nanoem_mutable_model_morph_material_t *item =
                        nanoemMutableModelMorphMaterialCreate(scoped, &status);
                    nanoemMutableModelMorphMaterialSetMaterialObject(
                        item, materials[m_morphItemCandidateMaterialIndex]);
                    nanoemMutableModelMorphInsertMaterialMorphObject(scoped, item, -1, &status);
                    nanoemMutableModelMorphMaterialDestroy(item);
                    items = nanoemModelMorphGetAllMaterialMorphObjects(morphPtr, &numItems);
                    m_morphItemCandidateMaterialIndex = NANOEM_RSIZE_MAX;
                }
                ImGui::SameLine();
                if (ImGuiWindow::handleButton(
                        "Remove", ImGui::GetContentRegionAvail().x, m_morphItemIndex < numItems)) {
                    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                    ScopedMutableMorph scoped(morphPtr, m_activeModel);
                    nanoem_mutable_model_morph_material_t *item =
                        nanoemMutableModelMorphMaterialCreateAsReference(items[m_morphItemIndex], &status);
                    nanoemMutableModelMorphRemoveMaterialMorphObject(scoped, item, &status);
                    nanoemMutableModelMorphMaterialDestroy(item);
                }
                addSeparator();
                nanoem_model_morph_material_t *materialPtr =
                    m_morphItemIndex < numItems ? items[m_morphItemIndex] : nullptr;
                {
                    nanoem_model_morph_material_operation_type_t value =
                        nanoemModelMorphMaterialGetOperationType(materialPtr);
                    ImGui::TextUnformatted("Operation Type");
                    if (ImGui::BeginCombo("##operation", selectedMorphMaterialOperationType(value))) {
                        for (int i = NANOEM_MODEL_MORPH_MATERIAL_OPERATION_TYPE_FIRST_ENUM;
                             i < NANOEM_MODEL_MORPH_MATERIAL_OPERATION_TYPE_MAX_ENUM; i++) {
                            nanoem_model_morph_material_operation_type_t type =
                                static_cast<nanoem_model_morph_material_operation_type_t>(i);
                            if (ImGui::Selectable(selectedMorphMaterialOperationType(type))) {
                                ScopedMutableMorphMaterial scoped(materialPtr, m_activeModel);
                                nanoemMutableModelMorphMaterialSetOperationType(scoped, type);
                            }
                        }
                        ImGui::EndCombo();
                    }
                }
                {
                    Vector4 ambientColor(glm::make_vec4(nanoemModelMorphMaterialGetAmbientColor(materialPtr)));
                    ImGui::TextUnformatted("Ambient Color");
                    if (ImGui::ColorEdit3("##material.ambient.color", glm::value_ptr(ambientColor))) {
                        ScopedMutableMorphMaterial scoped(materialPtr, m_activeModel);
                        nanoemMutableModelMorphMaterialSetAmbientColor(scoped, glm::value_ptr(ambientColor));
                    }
                }
                {
                    Vector4 diffuseColor(glm::make_vec3(nanoemModelMorphMaterialGetDiffuseColor(materialPtr)),
                        nanoemModelMorphMaterialGetDiffuseOpacity(materialPtr));
                    ImGui::TextUnformatted("Diffuse Color");
                    if (ImGui::ColorEdit4("##material.diffuse.color", glm::value_ptr(diffuseColor))) {
                        ScopedMutableMorphMaterial scoped(materialPtr, m_activeModel);
                        nanoemMutableModelMorphMaterialSetDiffuseColor(scoped, glm::value_ptr(diffuseColor));
                        nanoemMutableModelMorphMaterialSetDiffuseOpacity(scoped, diffuseColor.w);
                    }
                }
                {
                    Vector4 specularColor(glm::make_vec4(nanoemModelMorphMaterialGetSpecularColor(materialPtr)));
                    ImGui::TextUnformatted("Specular Color");
                    if (ImGui::ColorEdit3("##material.specular.color", glm::value_ptr(specularColor))) {
                        ScopedMutableMorphMaterial scoped(materialPtr, m_activeModel);
                        nanoemMutableModelMorphMaterialSetSpecularColor(scoped, glm::value_ptr(specularColor));
                    }
                }
                {
                    nanoem_f32_t specularPower = nanoemModelMorphMaterialGetSpecularPower(materialPtr);
                    ImGui::TextUnformatted("Specular Color");
                    if (ImGui::DragFloat("##material.specular.power", &specularPower)) {
                        ScopedMutableMorphMaterial scoped(materialPtr, m_activeModel);
                        nanoemMutableModelMorphMaterialSetSpecularPower(scoped, specularPower);
                    }
                }
                addSeparator();
                {
                    Vector4 edgeColor(glm::make_vec3(nanoemModelMorphMaterialGetEdgeColor(materialPtr)),
                        nanoemModelMorphMaterialGetEdgeOpacity(materialPtr));
                    ImGui::TextUnformatted("Edge Color");
                    if (ImGui::ColorEdit4("##material.edge.color", glm::value_ptr(edgeColor))) {
                        ScopedMutableMorphMaterial scoped(materialPtr, m_activeModel);
                        nanoemMutableModelMorphMaterialSetEdgeColor(scoped, glm::value_ptr(edgeColor));
                        nanoemMutableModelMorphMaterialSetEdgeOpacity(scoped, edgeColor.w);
                    }
                }
                {
                    nanoem_f32_t edgeSize = nanoemModelMorphMaterialGetEdgeSize(materialPtr);
                    ImGui::TextUnformatted("Edge Size");
                    if (ImGui::DragFloat("##material.edge.size", &edgeSize)) {
                        ScopedMutableMorphMaterial scoped(materialPtr, m_activeModel);
                        nanoemMutableModelMorphMaterialSetEdgeSize(scoped, edgeSize);
                    }
                }
                addSeparator();
                {
                    Vector4 blendFactor(glm::make_vec4(nanoemModelMorphMaterialGetDiffuseTextureBlend(materialPtr)));
                    ImGui::TextUnformatted("Diffuse Texture Blend");
                    if (ImGui::ColorEdit4("##material.blend.diffuse", glm::value_ptr(blendFactor))) {
                        ScopedMutableMorphMaterial scoped(materialPtr, m_activeModel);
                        nanoemMutableModelMorphMaterialSetDiffuseTextureBlend(scoped, glm::value_ptr(blendFactor));
                    }
                }
                {
                    Vector4 blendFactor(glm::make_vec4(nanoemModelMorphMaterialGetSphereMapTextureBlend(materialPtr)));
                    ImGui::TextUnformatted("SphereMap Texture Blend");
                    if (ImGui::ColorEdit4("##material.blend.sphere-map", glm::value_ptr(blendFactor))) {
                        ScopedMutableMorphMaterial scoped(materialPtr, m_activeModel);
                        nanoemMutableModelMorphMaterialSetSphereMapTextureBlend(scoped, glm::value_ptr(blendFactor));
                    }
                }
                {
                    Vector4 blendFactor(glm::make_vec4(nanoemModelMorphMaterialGetToonTextureBlend(materialPtr)));
                    ImGui::TextUnformatted("Toon Texture Blend");
                    if (ImGui::ColorEdit4("##material.blend.toon", glm::value_ptr(blendFactor))) {
                        ScopedMutableMorphMaterial scoped(materialPtr, m_activeModel);
                        nanoemMutableModelMorphMaterialSetToonTextureBlend(scoped, glm::value_ptr(blendFactor));
                    }
                }
                ImGui::PopItemWidth();
                break;
            }
            case NANOEM_MODEL_MORPH_TYPE_TEXTURE:
            case NANOEM_MODEL_MORPH_TYPE_UVA1:
            case NANOEM_MODEL_MORPH_TYPE_UVA2:
            case NANOEM_MODEL_MORPH_TYPE_UVA3:
            case NANOEM_MODEL_MORPH_TYPE_UVA4: {
                nanoem_model_morph_uv_t *const *items = nanoemModelMorphGetAllUVMorphObjects(morphPtr, &numItems);
                selectIndex(itemUp, itemDown, numItems, m_morphItemIndex);
                for (nanoem_rsize_t i = 0; i < numItems; i++) {
                    const nanoem_model_morph_uv_t *item = items[i];
                    const nanoem_model_vertex_t *vertexPtr = nanoemModelMorphUVGetVertexObject(item);
                    const bool selected = static_cast<nanoem_rsize_t>(i) == m_morphItemIndex;
                    formatVertexText(buffer, sizeof(buffer), vertexPtr);
                    if (ImGui::Selectable(buffer, selected) || ((itemUp || itemDown) && selected)) {
                        ImGui::SetScrollHereY();
                        m_morphItemIndex = i;
                    }
                }
                ImGui::EndChild();
                ImGui::SameLine();
                ImGui::BeginChild("##uv.properties", propertyWindowSize, false);
                ImGui::PushItemWidth(-1);
                nanoem_model_morph_uv_t *vertexPtr = m_morphItemIndex < numItems ? items[m_morphItemIndex] : nullptr;
                {
                    Vector4 position(glm::make_vec4(nanoemModelMorphUVGetPosition(vertexPtr)));
                    ImGui::TextUnformatted("Position");
                    if (ImGui::DragFloat4("##vertex.position", glm::value_ptr(position), 1.0f, 0.0f, 0.0f)) {
                        ScopedMutableMorphUV scoped(vertexPtr, m_activeModel);
                        nanoemMutableModelMorphUVSetPosition(scoped, glm::value_ptr(position));
                    }
                }
                ImGui::PopItemWidth();
                break;
            }
            case NANOEM_MODEL_MORPH_TYPE_VERTEX: {
                nanoem_model_morph_vertex_t *const *items =
                    nanoemModelMorphGetAllVertexMorphObjects(morphPtr, &numItems);
                selectIndex(itemUp, itemDown, numItems, m_morphItemIndex);
                for (nanoem_rsize_t i = 0; i < numItems; i++) {
                    const nanoem_model_morph_vertex_t *item = items[i];
                    const nanoem_model_vertex_t *vertexPtr = nanoemModelMorphVertexGetVertexObject(item);
                    const bool selected = static_cast<nanoem_rsize_t>(i) == m_morphItemIndex;
                    formatVertexText(buffer, sizeof(buffer), vertexPtr);
                    if (ImGui::Selectable(buffer, selected) || ((itemUp || itemDown) && selected)) {
                        ImGui::SetScrollHereY();
                        m_morphItemIndex = i;
                    }
                }
                ImGui::EndChild();
                ImGui::SameLine();
                ImGui::BeginChild("##vertex.properties", propertyWindowSize, false);
                ImGui::PushItemWidth(-1);
                nanoem_model_morph_vertex_t *vertexPtr =
                    m_morphItemIndex < numItems ? items[m_morphItemIndex] : nullptr;
                {
                    Vector4 position(glm::make_vec4(nanoemModelMorphVertexGetPosition(vertexPtr)));
                    ImGui::TextUnformatted("Position");
                    if (ImGui::DragFloat3("##vertex.position", glm::value_ptr(position), 1.0f, 0.0f, 0.0f)) {
                        ScopedMutableMorphVertex scoped(vertexPtr, m_activeModel);
                        nanoemMutableModelMorphVertexSetPosition(scoped, glm::value_ptr(position));
                    }
                }
                ImGui::PopItemWidth();
                break;
            }
            default:
                numItems = 0;
                break;
            }
            ImGui::EndChild();
        }
    }
    ImGui::PopItemWidth();
}

void
ModelParameterDialog::layoutAllLabels(Project *project)
{
    nanoem_rsize_t numLabels;
    nanoem_model_label_t *const *labels = nanoemModelGetAllLabelObjects(m_activeModel->data(), &numLabels);
    nanoem_model_label_t *hoveredLabelPtr = nullptr;
    nanoem_f32_t width = ImGuiWindow::kLeftPaneWidth * project->windowDevicePixelRatio();
    ImGui::BeginChild("left-pane", ImVec2(width, 0), false);
    if (ImGui::Button(reinterpret_cast<const char *>(ImGuiWindow::kFACogs))) {
        ImGui::OpenPopup("label-op-menu");
    }
    if (ImGui::BeginPopup("label-op-menu")) {
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    ImGui::Text("%lu / %lu", m_labelIndex + 1, numLabels);
    ImGui::BeginChild(
        "left-pane-inner", ImVec2(width, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing()), true);
    ImGuiListClipper clipper;
    char buffer[Inline::kNameStackBufferSize];
    bool up, down;
    detectUpDown(up, down);
    selectIndex(up, down, numLabels, m_labelIndex);
    IModelObjectSelection *selection = m_activeModel->selection();
    clipper.Begin(Inline::saturateInt32(numLabels));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            const nanoem_model_label_t *labelPtr = labels[i];
            const model::Label *label = model::Label::cast(labelPtr);
            const bool selected = selection->containsLabel(labelPtr);
            StringUtils::format(buffer, sizeof(buffer), "%s##label[%d].name", label->nameConstString(), i);
            if (ImGui::Selectable(buffer, selected)) {
                ImGui::SetScrollHereY();
                if (ImGui::GetIO().KeyCtrl) {
                    selected ? selection->removeLabel(labelPtr) : selection->addLabel(labelPtr);
                }
                else {
                    if (!ImGui::GetIO().KeyShift) {
                        selection->removeAllLabels();
                        m_labelIndex = i;
                        m_labelItemIndex = 0;
                    }
                    selection->addLabel(labelPtr);
                }
                hoveredLabelPtr = labels[i];
            }
            else if ((up || down) && m_labelIndex == static_cast<nanoem_rsize_t>(i)) {
                ImGui::SetScrollHereY();
                m_labelIndex = i;
                m_labelItemIndex = 0;
            }
            else if (ImGui::IsItemHovered()) {
                hoveredLabelPtr = labels[i];
            }
        }
    }
    selection->setHoveredLabel(hoveredLabelPtr);
    ImGui::EndChild();
    const nanoem_model_label_t *selectedLabel = numLabels > 0 ? labels[m_labelIndex] : nullptr;
    bool isEditable = !(nanoemModelLabelIsSpecial(selectedLabel) &&
        StringUtils::equals(model::Label::cast(selectedLabel)->nameConstString(), "Root"));
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAPlus), 0, isEditable)) {
        ImGui::OpenPopup("label-create-menu");
    }
    if (ImGui::BeginPopup("label-create-menu")) {
        struct CreateLabelCommand : ImGuiWindow::ILazyExecutionCommand {
            CreateLabelCommand(nanoem_rsize_t numLabels, int offset, const nanoem_model_label_t *base)
                : m_base(base)
                , m_numLabels(numLabels)
                , m_offset(offset)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableLabel label(activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
                StringUtils::UnicodeStringScope scope(factory);
                char buffer[Inline::kMarkerStringLength];
                if (m_base) {
                    nanoemMutableModelLabelCopy(label, m_base, &status);
                }
                StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, m_numLabels + 1);
                if (StringUtils::tryGetString(factory, buffer, scope)) {
                    nanoemMutableModelLabelSetName(label, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
                }
                StringUtils::format(buffer, sizeof(buffer), "NewLabel%zu", m_numLabels + 1);
                if (StringUtils::tryGetString(factory, buffer, scope)) {
                    nanoemMutableModelLabelSetName(label, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
                }
                nanoemMutableModelInsertLabelObject(model, label, m_offset, &status);
                model::Label *newLabel = model::Label::create();
                newLabel->bind(nanoemMutableModelLabelGetOriginObject(label));
                newLabel->resetLanguage(
                    nanoemMutableModelLabelGetOriginObject(label), factory, project->castLanguage());
                project->rebuildAllTracks();
            }
            const nanoem_model_label_t *m_base;
            const nanoem_rsize_t m_numLabels;
            const int m_offset;
        };
        int selectedLabelIndex = nanoemModelObjectGetIndex(nanoemModelLabelGetModelObject(selectedLabel));
        if (ImGui::BeginMenu("Insert New")) {
            if (ImGui::MenuItem("at Last")) {
                m_parent->addLazyExecutionCommand(nanoem_new(CreateLabelCommand(numLabels, -1, nullptr)));
            }
            if (ImGui::MenuItem("after Selected", nullptr, nullptr, m_labelIndex < numLabels)) {
                m_parent->addLazyExecutionCommand(
                    nanoem_new(CreateLabelCommand(numLabels, selectedLabelIndex + 1, nullptr)));
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Insert Copy", m_labelIndex < numLabels)) {
            if (ImGui::MenuItem("at Last")) {
                m_parent->addLazyExecutionCommand(nanoem_new(CreateLabelCommand(numLabels, -1, selectedLabel)));
            }
            if (ImGui::MenuItem("at Next")) {
                m_parent->addLazyExecutionCommand(
                    nanoem_new(CreateLabelCommand(numLabels, selectedLabelIndex + 1, selectedLabel)));
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(
            reinterpret_cast<const char *>(ImGuiWindow::kFAMinus), 0, isEditable && m_labelIndex < numLabels)) {
        struct DeleteLabelCommand : ImGuiWindow::ILazyExecutionCommand {
            DeleteLabelCommand(nanoem_model_label_t *const *labels, nanoem_rsize_t &labelIndex)
                : m_labels(labels)
                , m_labelIndex(labelIndex)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableLabel label(m_labels[m_labelIndex], activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelRemoveLabelObject(model, label, &status);
                project->rebuildAllTracks();
                if (m_labelIndex > 0) {
                    m_labelIndex--;
                }
            }
            nanoem_model_label_t *const *m_labels;
            nanoem_rsize_t m_labelIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(DeleteLabelCommand(labels, m_labelIndex)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowUp), 0, isEditable)) {
        struct MoveLabelUpCommand : ImGuiWindow::ILazyExecutionCommand {
            MoveLabelUpCommand(nanoem_model_label_t *const *labels, nanoem_rsize_t &labelIndex)
                : m_labels(labels)
                , m_labelIndex(labelIndex)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableLabel label(m_labels[m_labelIndex], activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelRemoveLabelObject(model, label, &status);
                int offset = Inline::saturateInt32(--m_labelIndex);
                nanoemMutableModelInsertLabelObject(model, label, offset, &status);
                project->rebuildAllTracks();
            }
            nanoem_model_label_t *const *m_labels;
            nanoem_rsize_t &m_labelIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(MoveLabelUpCommand(labels, m_labelIndex)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(
            reinterpret_cast<const char *>(ImGuiWindow::kFAArrowDown), 0, isEditable && m_labelIndex < numLabels - 1)) {
        struct MoveLabelDownCommand : ImGuiWindow::ILazyExecutionCommand {
            MoveLabelDownCommand(nanoem_model_label_t *const *labels, nanoem_rsize_t &labelIndex)
                : m_labels(labels)
                , m_labelIndex(labelIndex)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableLabel label(m_labels[m_labelIndex], activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelRemoveLabelObject(model, label, &status);
                int offset = Inline::saturateInt32(++m_labelIndex);
                nanoemMutableModelInsertLabelObject(model, label, offset, &status);
                project->rebuildAllTracks();
            }
            nanoem_model_label_t *const *m_labels;
            nanoem_rsize_t &m_labelIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(MoveLabelDownCommand(labels, m_labelIndex)));
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("right-pane", ImGui::GetContentRegionAvail());
    if (numLabels > 0 && m_labelIndex < numLabels) {
        nanoem_model_label_t *labelPtr = labels[m_labelIndex];
        layoutLabelPropertyPane(labelPtr, project);
    }
    ImGui::EndChild();
}

void
ModelParameterDialog::layoutLabelPropertyPane(nanoem_model_label_t *labelPtr, Project *project)
{
    char buffer[Inline::kNameStackBufferSize];
    nanoem_language_type_t language = static_cast<nanoem_language_type_t>(m_language);
    StringUtils::UnicodeStringScope scope(project->unicodeStringFactory());
    ImGui::PushItemWidth(-1);
    if (layoutName(nanoemModelLabelGetName(labelPtr, language), project, scope)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        ScopedMutableLabel scoped(labelPtr, m_activeModel);
        nanoemMutableModelLabelSetName(scoped, scope.value(), language, &status);
    }
    {
        bool value = nanoemModelLabelIsSpecial(labelPtr) != 0;
        if (ImGuiWindow::handleCheckBox(tr("nanoem.gui.model.edit.label.special"), &value, m_labelIndex > 0)) {
            ScopedMutableLabel scoped(labelPtr, m_activeModel);
            nanoemMutableModelLabelSetSpecial(scoped, value);
        }
    }
    {
        ImVec2 avail(ImGui::GetContentRegionAvail());
        avail.y -= ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("##items", avail, true);
        model::Bone::Set reservedBoneSet;
        model::Morph::Set reservedMorphSet;
        nanoem_rsize_t numItems;
        nanoem_model_label_item_t *const *items = nanoemModelLabelGetAllItemObjects(labelPtr, &numItems);
        {
            bool itemUp, itemDown;
            detectUpDown(itemUp, itemDown);
            selectIndex(itemUp, itemDown, numItems, m_labelItemIndex);
            for (nanoem_rsize_t i = 0; i < numItems; i++) {
                const nanoem_model_label_item_t *item = items[i];
                const bool selected = static_cast<nanoem_rsize_t>(i) == m_labelItemIndex;
                const nanoem_model_bone_t *bonePtr = nanoemModelLabelItemGetBoneObject(item);
                const nanoem_model_morph_t *morphPtr = nanoemModelLabelItemGetMorphObject(item);
                if (const model::Bone *bone = model::Bone::cast(bonePtr)) {
                    StringUtils::format(buffer, sizeof(buffer), "%s##item[%lu].name", bone->nameConstString(), i);
                    if (ImGui::Selectable(buffer, selected) || ((itemUp || itemDown) && selected)) {
                        ImGui::SetScrollHereY();
                        m_labelItemIndex = i;
                    }
                    reservedBoneSet.insert(bonePtr);
                }
                else if (const model::Morph *morph = model::Morph::cast(morphPtr)) {
                    StringUtils::format(buffer, sizeof(buffer), "%s##item[%lu].name", morph->nameConstString(), i);
                    if (ImGui::Selectable(buffer, selected) || ((itemUp || itemDown) && selected)) {
                        ImGui::SetScrollHereY();
                        m_labelItemIndex = i;
                    }
                    reservedMorphSet.insert(morphPtr);
                }
            }
        }
        ImGui::EndChild();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
        nanoem_rsize_t numBones;
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
        nanoem_rsize_t numMorphs;
        nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_activeModel->data(), &numMorphs);
        if (ImGui::BeginCombo("##candidate", "(select)")) {
            for (nanoem_rsize_t i = 0; i < numBones; i++) {
                const nanoem_model_bone_t *bonePtr = bones[i];
                if (model::Bone *bone = model::Bone::cast(bonePtr)) {
                    const bool selected = static_cast<nanoem_rsize_t>(i) == m_labelItemCandidateBoneIndex;
                    const ImGuiSelectableFlags flags = reservedBoneSet.find(bonePtr) != reservedBoneSet.end()
                        ? ImGuiSelectableFlags_Disabled
                        : ImGuiSelectableFlags_None;
                    if (ImGui::Selectable(bone->nameConstString(), selected, flags)) {
                        m_labelItemCandidateBoneIndex = i;
                        m_labelItemCandidateMorphIndex = NANOEM_RSIZE_MAX;
                    }
                }
            }
            ImGui::Separator();
            for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
                const nanoem_model_morph_t *morphPtr = morphs[i];
                if (model::Morph *morph = model::Morph::cast(morphPtr)) {
                    const bool selected = static_cast<nanoem_rsize_t>(i) == m_labelItemCandidateMorphIndex;
                    const ImGuiSelectableFlags flags = reservedMorphSet.find(morphPtr) != reservedMorphSet.end()
                        ? ImGuiSelectableFlags_Disabled
                        : ImGuiSelectableFlags_None;
                    if (ImGui::Selectable(morph->nameConstString(), selected, flags)) {
                        m_labelItemCandidateMorphIndex = i;
                        m_labelItemCandidateBoneIndex = NANOEM_RSIZE_MAX;
                    }
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGuiWindow::handleButton("Add", ImGui::GetContentRegionAvail().x * 0.5f,
                m_labelIndex > 0 &&
                    (m_labelItemCandidateBoneIndex < numBones || m_labelItemCandidateMorphIndex < numMorphs))) {
            nanoem_status_t status;
            ScopedMutableLabel scoped(labelPtr, m_activeModel);
            nanoem_mutable_model_label_item_t *item = nullptr;
            if (m_labelItemCandidateBoneIndex < numBones) {
                item = nanoemMutableModelLabelItemCreateFromBoneObject(
                    scoped, bones[m_labelItemCandidateBoneIndex], &status);
            }
            else if (m_labelItemCandidateMorphIndex < numMorphs) {
                item = nanoemMutableModelLabelItemCreateFromMorphObject(
                    scoped, morphs[m_labelItemCandidateMorphIndex], &status);
            }
            if (item) {
                nanoemMutableModelLabelInsertItemObject(scoped, item, -1, &status);
                nanoemMutableModelLabelItemDestroy(item);
                items = nanoemModelLabelGetAllItemObjects(labelPtr, &numItems);
                m_labelItemCandidateBoneIndex = m_labelItemCandidateMorphIndex = NANOEM_RSIZE_MAX;
            }
        }
        ImGui::SameLine();
        if (ImGuiWindow::handleButton(
                "Remove", ImGui::GetContentRegionAvail().x, m_labelIndex > 0 && m_labelItemIndex < numItems)) {
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            ScopedMutableLabel scoped(labelPtr, m_activeModel);
            nanoem_mutable_model_label_item_t *item =
                nanoemMutableModelLabelItemCreateAsReference(items[m_labelItemIndex], &status);
            nanoemMutableModelLabelRemoveItemObject(scoped, item, &status);
            nanoemMutableModelLabelItemDestroy(item);
        }
    }
    ImGui::PopItemWidth();
}

void
ModelParameterDialog::layoutAllRigidBodies(Project *project)
{
    nanoem_rsize_t numRigidBodies;
    nanoem_model_rigid_body_t *const *rigidBodies =
        nanoemModelGetAllRigidBodyObjects(m_activeModel->data(), &numRigidBodies);
    nanoem_f32_t width = ImGuiWindow::kLeftPaneWidth * project->windowDevicePixelRatio();
    ImGui::BeginChild("left-pane", ImVec2(width, 0), false);
    if (ImGui::Button(reinterpret_cast<const char *>(ImGuiWindow::kFACogs))) {
        ImGui::OpenPopup("rigid-body-op-menu");
    }
    if (ImGui::BeginPopup("rigid-body-op-menu")) {
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    ImGui::Text("%lu / %lu", m_rigidBodyIndex + 1, numRigidBodies);
    ImGui::BeginChild(
        "left-pane-inner", ImVec2(width, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing()), true);
    ImGuiListClipper clipper;
    nanoem_model_rigid_body_t *hoveredRigidBodyPtr = nullptr;
    char buffer[Inline::kNameStackBufferSize];
    bool up, down;
    detectUpDown(up, down);
    selectIndex(up, down, numRigidBodies, m_rigidBodyIndex);
    IModelObjectSelection *selection = m_activeModel->selection();
    clipper.Begin(Inline::saturateInt32(numRigidBodies));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            const nanoem_model_rigid_body_t *rigidBodyPtr = rigidBodies[i];
            const model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr);
            const bool selected = selection->containsRigidBody(rigidBodyPtr);
            StringUtils::format(buffer, sizeof(buffer), "%s##body[%d].name", rigidBody->nameConstString(), i);
            if (ImGui::Selectable(buffer, selected)) {
                ImGui::SetScrollHereY();
                if (ImGui::GetIO().KeyCtrl) {
                    selected ? selection->removeRigidBody(rigidBodyPtr) : selection->addRigidBody(rigidBodyPtr);
                }
                else {
                    if (!ImGui::GetIO().KeyShift) {
                        selection->removeAllRigidBodies();
                        m_rigidBodyIndex = i;
                    }
                    selection->addRigidBody(rigidBodyPtr);
                }
                hoveredRigidBodyPtr = rigidBodies[i];
            }
            else if ((up || down) && m_rigidBodyIndex == static_cast<nanoem_rsize_t>(i)) {
                ImGui::SetScrollHereY();
                m_rigidBodyIndex = i;
            }
            else if (ImGui::IsItemHovered()) {
                hoveredRigidBodyPtr = rigidBodies[i];
            }
        }
    }
    selection->setHoveredRigidBody(hoveredRigidBodyPtr);
    ImGui::EndChild();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAPlus), 0, true)) {
        ImGui::OpenPopup("rigid-body-create-menu");
    }
    if (ImGui::BeginPopup("rigid-body-create-menu")) {
        struct CreateRigidBodyCommand : ImGuiWindow::ILazyExecutionCommand {
            CreateRigidBodyCommand(nanoem_rsize_t numRigidBodies, int offset, const nanoem_model_rigid_body_t *base)
                : m_base(base)
                , m_numRigidBodies(numRigidBodies)
                , m_offset(offset)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableModel model(activeModel);
                ScopedMutableRigidBody body(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
                StringUtils::UnicodeStringScope scope(factory);
                char buffer[Inline::kMarkerStringLength];
                if (m_base) {
                    nanoemMutableModelRigidBodyCopy(body, m_base, &status);
                }
                StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, m_numRigidBodies + 1);
                if (StringUtils::tryGetString(factory, buffer, scope)) {
                    nanoemMutableModelRigidBodySetName(body, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
                }
                StringUtils::format(buffer, sizeof(buffer), "NewRigidBody%zu", m_numRigidBodies + 1);
                if (StringUtils::tryGetString(factory, buffer, scope)) {
                    nanoemMutableModelRigidBodySetName(body, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
                }
                nanoemMutableModelInsertRigidBodyObject(model, body, m_offset, &status);
                model::RigidBody *newBody = model::RigidBody::create();
                model::RigidBody::Resolver resolver;
                newBody->bind(nanoemMutableModelRigidBodyGetOriginObject(body), nullptr, false, resolver);
                newBody->resetLanguage(
                    nanoemMutableModelRigidBodyGetOriginObject(body), factory, project->castLanguage());
            }
            const nanoem_model_rigid_body_t *m_base;
            const nanoem_rsize_t m_numRigidBodies;
            const int m_offset;
        };
        const nanoem_model_rigid_body_t *selectedRigidBody =
            numRigidBodies > 0 ? rigidBodies[m_rigidBodyIndex] : nullptr;
        int selectedRigidBodyIndex = nanoemModelObjectGetIndex(nanoemModelRigidBodyGetModelObject(selectedRigidBody));
        if (ImGui::BeginMenu("Insert New")) {
            if (ImGui::MenuItem("at Last")) {
                m_parent->addLazyExecutionCommand(nanoem_new(CreateRigidBodyCommand(numRigidBodies, -1, nullptr)));
            }
            if (ImGui::MenuItem("after Selected", nullptr, nullptr, m_rigidBodyIndex < numRigidBodies)) {
                m_parent->addLazyExecutionCommand(
                    nanoem_new(CreateRigidBodyCommand(numRigidBodies, selectedRigidBodyIndex + 1, nullptr)));
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Insert Copy", m_rigidBodyIndex < numRigidBodies)) {
            if (ImGui::MenuItem("at Last")) {
                m_parent->addLazyExecutionCommand(
                    nanoem_new(CreateRigidBodyCommand(numRigidBodies, -1, selectedRigidBody)));
            }
            if (ImGui::MenuItem("at Next")) {
                m_parent->addLazyExecutionCommand(
                    nanoem_new(CreateRigidBodyCommand(numRigidBodies, selectedRigidBodyIndex + 1, selectedRigidBody)));
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(
            reinterpret_cast<const char *>(ImGuiWindow::kFAMinus), 0, m_rigidBodyIndex < numRigidBodies)) {
        struct DeleteRigidBodyCommand : ImGuiWindow::ILazyExecutionCommand {
            DeleteRigidBodyCommand(nanoem_model_rigid_body_t *const *rigidBodies, nanoem_rsize_t &rigidBodyIndex)
                : m_rigidBodies(rigidBodies)
                , m_rigidBodyIndex(rigidBodyIndex)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableRigidBody rigid_body(m_rigidBodies[m_rigidBodyIndex], activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelRemoveRigidBodyObject(model, rigid_body, &status);
                if (m_rigidBodyIndex > 0) {
                    m_rigidBodyIndex--;
                }
            }
            nanoem_model_rigid_body_t *const *m_rigidBodies;
            nanoem_rsize_t &m_rigidBodyIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(DeleteRigidBodyCommand(rigidBodies, m_rigidBodyIndex)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowUp), 0, m_rigidBodyIndex > 0)) {
        struct MoveRigidBodyUpCommand : ImGuiWindow::ILazyExecutionCommand {
            MoveRigidBodyUpCommand(nanoem_model_rigid_body_t *const *rigidBodies, nanoem_rsize_t &rigidBodyIndex)
                : m_rigidBodies(rigidBodies)
                , m_rigidBodyIndex(rigidBodyIndex)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableRigidBody rigid_body(m_rigidBodies[m_rigidBodyIndex], activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelRemoveRigidBodyObject(model, rigid_body, &status);
                int offset = Inline::saturateInt32(--m_rigidBodyIndex);
                nanoemMutableModelInsertRigidBodyObject(model, rigid_body, offset, &status);
            }
            nanoem_model_rigid_body_t *const *m_rigidBodies;
            nanoem_rsize_t &m_rigidBodyIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(MoveRigidBodyUpCommand(rigidBodies, m_rigidBodyIndex)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(
            reinterpret_cast<const char *>(ImGuiWindow::kFAArrowDown), 0, m_rigidBodyIndex < numRigidBodies - 1)) {
        struct MoveRigidBodyDownCommand : ImGuiWindow::ILazyExecutionCommand {
            MoveRigidBodyDownCommand(nanoem_model_rigid_body_t *const *rigidBodies, nanoem_rsize_t &rigidBodyIndex)
                : m_rigidBodies(rigidBodies)
                , m_rigidBodyIndex(rigidBodyIndex)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableRigidBody rigid_body(m_rigidBodies[m_rigidBodyIndex], activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelRemoveRigidBodyObject(model, rigid_body, &status);
                int offset = Inline::saturateInt32(++m_rigidBodyIndex);
                nanoemMutableModelInsertRigidBodyObject(model, rigid_body, offset, &status);
            }
            nanoem_model_rigid_body_t *const *m_rigidBodies;
            nanoem_rsize_t &m_rigidBodyIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(MoveRigidBodyDownCommand(rigidBodies, m_rigidBodyIndex)));
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("right-pane", ImGui::GetContentRegionAvail());
    if (numRigidBodies > 0 && m_rigidBodyIndex < numRigidBodies) {
        nanoem_model_rigid_body_t *rigidBodyPtr = rigidBodies[m_rigidBodyIndex];
        layoutRigidBodyPropertyPane(rigidBodyPtr, project);
    }
    ImGui::EndChild();
}

void
ModelParameterDialog::layoutRigidBodyPropertyPane(nanoem_model_rigid_body_t *rigidBodyPtr, Project *project)
{
    char buffer[Inline::kNameStackBufferSize];
    nanoem_language_type_t language = static_cast<nanoem_language_type_t>(m_language);
    StringUtils::UnicodeStringScope scope(project->unicodeStringFactory());
    ImGui::PushItemWidth(-1);
    if (layoutName(nanoemModelRigidBodyGetName(rigidBodyPtr, language), project, scope)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        ScopedMutableRigidBody scoped(rigidBodyPtr, m_activeModel);
        nanoemMutableModelRigidBodySetName(scoped, scope.value(), language, &status);
    }
    {
        const model::Bone *bone = model::Bone::cast(nanoemModelRigidBodyGetBoneObject(rigidBodyPtr));
        nanoem_rsize_t numBones;
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.rigid-body.bone"));
        if (ImGui::BeginCombo("##bone", bone ? bone->nameConstString() : "(none)")) {
            for (nanoem_rsize_t i = 0; i < numBones; i++) {
                const nanoem_model_bone_t *candidateBonePtr = bones[i];
                if (const model::Bone *candidateBone = model::Bone::cast(candidateBonePtr)) {
                    StringUtils::format(buffer, sizeof(buffer), "%s##item[%lu].name", bone->nameConstString(), i);
                    if (ImGui::Selectable(candidateBone->nameConstString(), candidateBone == bone)) {
                        ScopedMutableRigidBody scoped(rigidBodyPtr, m_activeModel);
                        nanoemMutableModelRigidBodySetBoneObject(scoped, candidateBonePtr);
                    }
                }
            }
            ImGui::EndCombo();
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.rigid-body.origin"));
        Vector4 value(glm::make_vec4(nanoemModelRigidBodyGetOrigin(rigidBodyPtr)));
        if (ImGui::InputFloat3("##origin", glm::value_ptr(value))) {
            ScopedMutableRigidBody scoped(rigidBodyPtr, m_activeModel);
            nanoemMutableModelRigidBodySetOrigin(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.rigid-body.orientation"));
        Vector4 value(glm::make_vec4(nanoemModelRigidBodyGetOrientation(rigidBodyPtr)));
        if (ImGui::InputFloat3("##orientation", glm::value_ptr(value))) {
            ScopedMutableRigidBody scoped(rigidBodyPtr, m_activeModel);
            nanoemMutableModelRigidBodySetOrientation(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.rigid-body.shape.size"));
        Vector4 value(glm::make_vec4(nanoemModelRigidBodyGetShapeSize(rigidBodyPtr)));
        if (ImGui::InputFloat3("##size", glm::value_ptr(value))) {
            ScopedMutableRigidBody scoped(rigidBodyPtr, m_activeModel);
            nanoemMutableModelRigidBodySetShapeSize(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.rigid-body.shape-type"));
        nanoem_model_rigid_body_shape_type_t value = nanoemModelRigidBodyGetShapeType(rigidBodyPtr);
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.rigid-body.shape-type.sphere"),
                value == NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_SPHERE)) {
            ScopedMutableRigidBody scoped(rigidBodyPtr, m_activeModel);
            nanoemMutableModelRigidBodySetShapeType(scoped, NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_SPHERE);
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.rigid-body.shape-type.box"),
                value == NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_BOX)) {
            ScopedMutableRigidBody scoped(rigidBodyPtr, m_activeModel);
            nanoemMutableModelRigidBodySetShapeType(scoped, NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_BOX);
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.rigid-body.shape-type.capsule"),
                value == NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_CAPSULE)) {
            ScopedMutableRigidBody scoped(rigidBodyPtr, m_activeModel);
            nanoemMutableModelRigidBodySetShapeType(scoped, NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_CAPSULE);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.rigid-body.object-type"));
        nanoem_model_rigid_body_transform_type_t value = nanoemModelRigidBodyGetTransformType(rigidBodyPtr);
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.rigid-body.object-type.static"),
                value == NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_TO_SIMULATION)) {
            ScopedMutableRigidBody scoped(rigidBodyPtr, m_activeModel);
            nanoemMutableModelRigidBodySetTransformType(
                scoped, NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_TO_SIMULATION);
        }
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.rigid-body.object-type.dynamic"),
                value == NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_SIMULATION_TO_BONE)) {
            ScopedMutableRigidBody scoped(rigidBodyPtr, m_activeModel);
            nanoemMutableModelRigidBodySetTransformType(
                scoped, NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_SIMULATION_TO_BONE);
        }
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.rigid-body.object-type.kinematic"),
                value == NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_ORIENTATION_AND_SIMULATION_TO_BONE)) {
            ScopedMutableRigidBody scoped(rigidBodyPtr, m_activeModel);
            nanoemMutableModelRigidBodySetTransformType(
                scoped, NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_ORIENTATION_AND_SIMULATION_TO_BONE);
        }
    }
    addSeparator();
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.rigid-body.mass"));
        nanoem_f32_t value = nanoemModelRigidBodyGetMass(rigidBodyPtr);
        if (ImGui::InputFloat("##mass", &value)) {
            ScopedMutableRigidBody scoped(rigidBodyPtr, m_activeModel);
            nanoemMutableModelRigidBodySetMass(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.rigid-body.linear-damping"));
        nanoem_f32_t value = nanoemModelRigidBodyGetLinearDamping(rigidBodyPtr);
        if (ImGui::InputFloat("##damping.linear", &value)) {
            ScopedMutableRigidBody scoped(rigidBodyPtr, m_activeModel);
            nanoemMutableModelRigidBodySetLinearDamping(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.rigid-body.angular-damping"));
        nanoem_f32_t value = nanoemModelRigidBodyGetAngularDamping(rigidBodyPtr);
        if (ImGui::InputFloat("##damping.angular", &value)) {
            ScopedMutableRigidBody scoped(rigidBodyPtr, m_activeModel);
            nanoemMutableModelRigidBodySetAngularDamping(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.rigid-body.friction"));
        nanoem_f32_t value = nanoemModelRigidBodyGetFriction(rigidBodyPtr);
        if (ImGui::InputFloat("##friction", &value)) {
            ScopedMutableRigidBody scoped(rigidBodyPtr, m_activeModel);
            nanoemMutableModelRigidBodySetFriction(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.rigid-body.restitution"));
        nanoem_f32_t value = nanoemModelRigidBodyGetRestitution(rigidBodyPtr);
        if (ImGui::InputFloat("##restitution", &value)) {
            ScopedMutableRigidBody scoped(rigidBodyPtr, m_activeModel);
            nanoemMutableModelRigidBodySetRestitution(scoped, value);
        }
    }
    addSeparator();
    {
        ImGui::TextUnformatted("Collision Group");
        int value = nanoemModelRigidBodyGetCollisionGroupId(rigidBodyPtr);
        if (ImGui::DragInt("##collision.group", &value, 0.05f, 0, 15)) {
            ScopedMutableRigidBody scoped(rigidBodyPtr, m_activeModel);
            nanoemMutableModelRigidBodySetCollisionGroupId(scoped, value);
        }
    }
    {
        nanoem_u32_t flags = ~nanoemModelRigidBodyGetCollisionMask(rigidBodyPtr);
        ImGui::TextUnformatted("Collision Mask");
        ImGui::Columns(8, nullptr, false);
        for (int i = 0; i < 16; i++) {
            char buffer[16];
            int offset = i + 1;
            StringUtils::format(buffer, sizeof(buffer), "%d##collision.mask.%d", offset, offset);
            if (ImGui::CheckboxFlags(buffer, &flags, 1 << i)) {
                ScopedMutableRigidBody scoped(rigidBodyPtr, m_activeModel);
                nanoemMutableModelRigidBodySetCollisionMask(scoped, ~flags);
            }
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
    }
    ImGui::PopItemWidth();
}

void
ModelParameterDialog::layoutAllJoints(Project *project)
{
    nanoem_rsize_t numJoints;
    nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(m_activeModel->data(), &numJoints);
    nanoem_f32_t width = ImGuiWindow::kLeftPaneWidth * project->windowDevicePixelRatio();
    ImGui::BeginChild("left-pane", ImVec2(width, 0), false);
    if (ImGui::Button(reinterpret_cast<const char *>(ImGuiWindow::kFACogs))) {
        ImGui::OpenPopup("joint-op-menu");
    }
    if (ImGui::BeginPopup("joint-op-menu")) {
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    ImGui::Text("%lu / %lu", m_jointIndex + 1, numJoints);
    ImGui::BeginChild(
        "left-pane-inner", ImVec2(width, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing()), true);
    ImGuiListClipper clipper;
    nanoem_model_joint_t *hoveredJointPtr = nullptr;
    char buffer[Inline::kNameStackBufferSize];
    bool up, down;
    detectUpDown(up, down);
    selectIndex(up, down, numJoints, m_jointIndex);
    IModelObjectSelection *selection = m_activeModel->selection();
    clipper.Begin(Inline::saturateInt32(numJoints));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            const nanoem_model_joint_t *jointPtr = joints[i];
            const model::Joint *joint = model::Joint::cast(jointPtr);
            const bool selected = selection->containsJoint(jointPtr);
            StringUtils::format(buffer, sizeof(buffer), "%s##joint[%d].name", joint->nameConstString(), i);
            if (ImGui::Selectable(buffer, selected)) {
                ImGui::SetScrollHereY();
                if (ImGui::GetIO().KeyCtrl) {
                    selected ? selection->removeJoint(jointPtr) : selection->addJoint(jointPtr);
                }
                else {
                    if (!ImGui::GetIO().KeyShift) {
                        selection->removeAllJoints();
                        m_jointIndex = i;
                    }
                    selection->addJoint(jointPtr);
                }
                hoveredJointPtr = joints[i];
            }
            else if ((up || down) && m_jointIndex == static_cast<nanoem_rsize_t>(i)) {
                ImGui::SetScrollHereY();
                m_jointIndex = i;
            }
            else if (ImGui::IsItemHovered()) {
                hoveredJointPtr = joints[i];
            }
        }
    }
    selection->setHoveredJoint(hoveredJointPtr);
    ImGui::EndChild();
    joints = nanoemModelGetAllJointObjects(m_activeModel->data(), &numJoints);
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAPlus), 0, true)) {
        ImGui::OpenPopup("rigid-body-create-menu");
    }
    if (ImGui::BeginPopup("rigid-body-create-menu")) {
        struct CreateJointCommand : ImGuiWindow::ILazyExecutionCommand {
            CreateJointCommand(nanoem_rsize_t numJoints, int offset, const nanoem_model_joint_t *base)
                : m_base(base)
                , m_numJoints(numJoints)
                , m_offset(offset)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableJoint joint(activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
                StringUtils::UnicodeStringScope scope(factory);
                char buffer[Inline::kMarkerStringLength];
                if (m_base) {
                    nanoemMutableModelJointCopy(joint, m_base, &status);
                }
                StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, m_numJoints + 1);
                if (StringUtils::tryGetString(factory, buffer, scope)) {
                    nanoemMutableModelJointSetName(joint, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
                }
                StringUtils::format(buffer, sizeof(buffer), "NewRigidBody%zu", m_numJoints + 1);
                if (StringUtils::tryGetString(factory, buffer, scope)) {
                    nanoemMutableModelJointSetName(joint, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
                }
                nanoemMutableModelInsertJointObject(model, joint, m_offset, &status);
                model::Joint *newJoint = model::Joint::create();
                model::RigidBody::Resolver resolver;
                newJoint->bind(nanoemMutableModelJointGetOriginObject(joint), nullptr, resolver);
                newJoint->resetLanguage(
                    nanoemMutableModelJointGetOriginObject(joint), factory, project->castLanguage());
            }
            const nanoem_model_joint_t *m_base;
            const nanoem_rsize_t m_numJoints;
            const int m_offset;
        };
        const nanoem_model_joint_t *selectedJoint = numJoints > 0 ? joints[m_jointIndex] : nullptr;
        int selectedJointIndex = nanoemModelObjectGetIndex(nanoemModelJointGetModelObject(selectedJoint));
        if (ImGui::BeginMenu("Insert New")) {
            if (ImGui::MenuItem("at Last")) {
                m_parent->addLazyExecutionCommand(nanoem_new(CreateJointCommand(numJoints, -1, nullptr)));
            }
            if (ImGui::MenuItem("after Selected", nullptr, nullptr, m_jointIndex < numJoints)) {
                m_parent->addLazyExecutionCommand(
                    nanoem_new(CreateJointCommand(numJoints, selectedJointIndex + 1, nullptr)));
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Insert Copy", m_jointIndex < numJoints)) {
            if (ImGui::MenuItem("at Last")) {
                m_parent->addLazyExecutionCommand(nanoem_new(CreateJointCommand(numJoints, -1, selectedJoint)));
            }
            if (ImGui::MenuItem("at Next")) {
                m_parent->addLazyExecutionCommand(
                    nanoem_new(CreateJointCommand(numJoints, selectedJointIndex + 1, selectedJoint)));
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAMinus), 0, m_jointIndex < numJoints)) {
        struct DeleteJointCommand : ImGuiWindow::ILazyExecutionCommand {
            DeleteJointCommand(nanoem_model_joint_t *const *joints, nanoem_rsize_t &jointIndex)
                : m_joints(joints)
                , m_jointIndex(jointIndex)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableJoint joint(m_joints[m_jointIndex], activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelRemoveJointObject(model, joint, &status);
                if (m_jointIndex > 0) {
                    m_jointIndex--;
                }
            }
            nanoem_model_joint_t *const *m_joints;
            nanoem_rsize_t &m_jointIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(DeleteJointCommand(joints, m_jointIndex)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowUp), 0, m_jointIndex > 0)) {
        struct MoveJointUpCommand : ImGuiWindow::ILazyExecutionCommand {
            MoveJointUpCommand(nanoem_model_joint_t *const *joints, nanoem_rsize_t &jointIndex)
                : m_joints(joints)
                , m_jointIndex(jointIndex)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableJoint joint(m_joints[m_jointIndex], activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelRemoveJointObject(model, joint, &status);
                int offset = Inline::saturateInt32(--m_jointIndex);
                nanoemMutableModelInsertJointObject(model, joint, offset, &status);
            }
            nanoem_model_joint_t *const *m_joints;
            nanoem_rsize_t &m_jointIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(MoveJointUpCommand(joints, m_jointIndex)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(
            reinterpret_cast<const char *>(ImGuiWindow::kFAArrowDown), 0, m_jointIndex < numJoints - 1)) {
        struct MoveJointDownCommand : ImGuiWindow::ILazyExecutionCommand {
            MoveJointDownCommand(nanoem_model_joint_t *const *joints, nanoem_rsize_t &jointIndex)
                : m_joints(joints)
                , m_jointIndex(jointIndex)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableJoint joint(m_joints[m_jointIndex], activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelRemoveJointObject(model, joint, &status);
                int offset = Inline::saturateInt32(++m_jointIndex);
                nanoemMutableModelInsertJointObject(model, joint, offset, &status);
            }
            nanoem_model_joint_t *const *m_joints;
            nanoem_rsize_t &m_jointIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(MoveJointDownCommand(joints, m_jointIndex)));
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("right-pane", ImGui::GetContentRegionAvail());
    if (numJoints > 0 && m_jointIndex < numJoints) {
        nanoem_model_joint_t *jointPtr = joints[m_jointIndex];
        layoutJointPropertyPane(jointPtr, project);
    }
    ImGui::EndChild();
}

void
ModelParameterDialog::layoutJointPropertyPane(nanoem_model_joint_t *jointPtr, Project *project)
{
    char buffer[Inline::kNameStackBufferSize];
    nanoem_language_type_t language = static_cast<nanoem_language_type_t>(m_language);
    StringUtils::UnicodeStringScope scope(project->unicodeStringFactory());
    ImGui::PushItemWidth(-1);
    if (layoutName(nanoemModelJointGetName(jointPtr, language), project, scope)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        ScopedMutableJoint scoped(jointPtr, m_activeModel);
        nanoemMutableModelJointSetName(scoped, scope.value(), language, &status);
    }
    {
        const model::RigidBody *rigidBodyA = model::RigidBody::cast(nanoemModelJointGetRigidBodyAObject(jointPtr));
        nanoem_rsize_t numRigidBodies;
        nanoem_model_rigid_body_t *const *bodies =
            nanoemModelGetAllRigidBodyObjects(m_activeModel->data(), &numRigidBodies);
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.rigid-body.a"));
        if (ImGui::BeginCombo("##rigid-body.a", rigidBodyA ? rigidBodyA->nameConstString() : "(none)")) {
            for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
                const nanoem_model_rigid_body_t *candidateBodyPtr = bodies[i];
                if (const model::RigidBody *candidateBody = model::RigidBody::cast(candidateBodyPtr)) {
                    StringUtils::format(
                        buffer, sizeof(buffer), "%s##item[%lu].name", candidateBody->nameConstString(), i);
                    if (ImGui::Selectable(candidateBody->nameConstString(), candidateBody == rigidBodyA)) {
                        ScopedMutableJoint scoped(jointPtr, m_activeModel);
                        nanoemMutableModelJointSetRigidBodyAObject(scoped, candidateBodyPtr);
                    }
                }
            }
            ImGui::EndCombo();
        }
        const model::RigidBody *rigidBodyB = model::RigidBody::cast(nanoemModelJointGetRigidBodyBObject(jointPtr));
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.rigid-body.b"));
        if (ImGui::BeginCombo("##rigid-body.b", rigidBodyB ? rigidBodyB->nameConstString() : "(none)")) {
            for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
                const nanoem_model_rigid_body_t *candidateBodyPtr = bodies[i];
                model::RigidBody *candidateBody = model::RigidBody::cast(candidateBodyPtr);
                if (candidateBody && ImGui::Selectable(candidateBody->nameConstString(), candidateBody == rigidBodyB)) {
                    ScopedMutableJoint scoped(jointPtr, m_activeModel);
                    nanoemMutableModelJointSetRigidBodyAObject(scoped, candidateBodyPtr);
                }
            }
            ImGui::EndCombo();
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.origin"));
        Vector4 value(glm::make_vec4(nanoemModelJointGetOrigin(jointPtr)));
        if (ImGui::InputFloat3("##origin", glm::value_ptr(value))) {
            ScopedMutableJoint scoped(jointPtr, m_activeModel);
            nanoemMutableModelJointSetOrigin(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.orientation"));
        Vector4 value(glm::make_vec4(nanoemModelJointGetOrientation(jointPtr)));
        if (ImGui::InputFloat3("##orientation", glm::value_ptr(value))) {
            ScopedMutableJoint scoped(jointPtr, m_activeModel);
            nanoemMutableModelJointSetOrientation(scoped, glm::value_ptr(value));
        }
    }
    addSeparator();
    {
        nanoem_model_joint_type_t value = nanoemModelJointGetType(jointPtr);
        ImGui::TextUnformatted("Type");
        if (ImGui::BeginCombo("##type", selectedJointType(value))) {
            for (int i = NANOEM_MODEL_JOINT_TYPE_FIRST_ENUM; i < NANOEM_MODEL_JOINT_TYPE_MAX_ENUM; i++) {
                nanoem_model_joint_type_t type = static_cast<nanoem_model_joint_type_t>(i);
                if (ImGui::Selectable(selectedJointType(type))) {
                    ScopedMutableJoint scoped(jointPtr, m_activeModel);
                    nanoemMutableModelJointSetType(scoped, type);
                }
            }
            ImGui::EndCombo();
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.linear.stiffness"));
        Vector4 value(glm::make_vec4(nanoemModelJointGetLinearStiffness(jointPtr)));
        if (ImGui::InputFloat3("##linear.stiffness", glm::value_ptr(value))) {
            ScopedMutableJoint scoped(jointPtr, m_activeModel);
            nanoemMutableModelJointSetLinearStiffness(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.linear.upper"));
        Vector4 value(glm::make_vec4(nanoemModelJointGetLinearUpperLimit(jointPtr)));
        if (ImGui::InputFloat3("##linear.upper", glm::value_ptr(value))) {
            ScopedMutableJoint scoped(jointPtr, m_activeModel);
            nanoemMutableModelJointSetLinearUpperLimit(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.linear.lower"));
        Vector4 value(glm::make_vec4(nanoemModelJointGetLinearLowerLimit(jointPtr)));
        if (ImGui::InputFloat3("##linear.lower", glm::value_ptr(value))) {
            ScopedMutableJoint scoped(jointPtr, m_activeModel);
            nanoemMutableModelJointSetLinearLowerLimit(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.angular.stiffness"));
        Vector4 value(glm::make_vec4(nanoemModelJointGetAngularStiffness(jointPtr)));
        if (ImGui::InputFloat3("##angular.stiffness", glm::value_ptr(value))) {
            ScopedMutableJoint scoped(jointPtr, m_activeModel);
            nanoemMutableModelJointSetAngularStiffness(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.angular.upper"));
        Vector4 value(glm::make_vec4(nanoemModelJointGetAngularUpperLimit(jointPtr)));
        if (ImGui::InputFloat3("##angular.upper", glm::value_ptr(value))) {
            ScopedMutableJoint scoped(jointPtr, m_activeModel);
            nanoemMutableModelJointSetAngularUpperLimit(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.angular.lower"));
        Vector4 value(glm::make_vec4(nanoemModelJointGetAngularLowerLimit(jointPtr)));
        if (ImGui::InputFloat3("##angular.lower", glm::value_ptr(value))) {
            ScopedMutableJoint scoped(jointPtr, m_activeModel);
            nanoemMutableModelJointSetAngularLowerLimit(scoped, glm::value_ptr(value));
        }
    }
    ImGui::PopItemWidth();
}

void
ModelParameterDialog::layoutAllSoftBodies(Project *project)
{
    nanoem_rsize_t numSoftBodys;
    nanoem_model_soft_body_t *const *soft_bodys =
        nanoemModelGetAllSoftBodyObjects(m_activeModel->data(), &numSoftBodys);
    nanoem_f32_t width = ImGuiWindow::kLeftPaneWidth * project->windowDevicePixelRatio();
    ImGui::BeginChild("left-pane", ImVec2(width, 0), false);
    if (ImGui::Button(reinterpret_cast<const char *>(ImGuiWindow::kFACogs))) {
        ImGui::OpenPopup("soft_body-op-menu");
    }
    if (ImGui::BeginPopup("soft_body-op-menu")) {
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    ImGui::Text("%lu / %lu", m_softBodyIndex + 1, numSoftBodys);
    ImGui::BeginChild(
        "left-pane-inner", ImVec2(width, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing()), true);
    ImGuiListClipper clipper;
    nanoem_model_soft_body_t *hoveredSoftBodyPtr = nullptr;
    char buffer[Inline::kNameStackBufferSize];
    bool up, down;
    detectUpDown(up, down);
    selectIndex(up, down, numSoftBodys, m_softBodyIndex);
    IModelObjectSelection *selection = m_activeModel->selection();
    clipper.Begin(Inline::saturateInt32(numSoftBodys));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            const nanoem_model_soft_body_t *soft_bodyPtr = soft_bodys[i];
            const model::SoftBody *soft_body = model::SoftBody::cast(soft_bodyPtr);
            const bool selected = selection->containsSoftBody(soft_bodyPtr);
            StringUtils::format(buffer, sizeof(buffer), "%s##soft_body[%d].name", soft_body->nameConstString(), i);
            if (ImGui::Selectable(buffer, selected)) {
                ImGui::SetScrollHereY();
                if (ImGui::GetIO().KeyCtrl) {
                    selected ? selection->removeSoftBody(soft_bodyPtr) : selection->addSoftBody(soft_bodyPtr);
                }
                else {
                    if (!ImGui::GetIO().KeyShift) {
                        selection->removeAllSoftBodies();
                        m_softBodyIndex = i;
                    }
                    selection->addSoftBody(soft_bodyPtr);
                }
                hoveredSoftBodyPtr = soft_bodys[i];
            }
            else if ((up || down) && m_softBodyIndex == static_cast<nanoem_rsize_t>(i)) {
                ImGui::SetScrollHereY();
                m_softBodyIndex = i;
            }
            else if (ImGui::IsItemHovered()) {
                hoveredSoftBodyPtr = soft_bodys[i];
            }
        }
    }
    selection->setHoveredSoftBody(hoveredSoftBodyPtr);
    ImGui::EndChild();
    soft_bodys = nanoemModelGetAllSoftBodyObjects(m_activeModel->data(), &numSoftBodys);
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAPlus), 0, true)) {
        ImGui::OpenPopup("rigid-body-create-menu");
    }
    if (ImGui::BeginPopup("rigid-body-create-menu")) {
        struct CreateSoftBodyCommand : ImGuiWindow::ILazyExecutionCommand {
            CreateSoftBodyCommand(nanoem_rsize_t numSoftBodys, int offset, const nanoem_model_soft_body_t *base)
                : m_base(base)
                , m_numSoftBodys(numSoftBodys)
                , m_offset(offset)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableSoftBody soft_body(activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
                StringUtils::UnicodeStringScope scope(factory);
                char buffer[Inline::kMarkerStringLength];
                if (m_base) {
                    nanoemMutableModelSoftBodyCopy(soft_body, m_base, &status);
                }
                StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, m_numSoftBodys + 1);
                if (StringUtils::tryGetString(factory, buffer, scope)) {
                    nanoemMutableModelSoftBodySetName(soft_body, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
                }
                StringUtils::format(buffer, sizeof(buffer), "NewRigidBody%zu", m_numSoftBodys + 1);
                if (StringUtils::tryGetString(factory, buffer, scope)) {
                    nanoemMutableModelSoftBodySetName(soft_body, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
                }
                nanoemMutableModelInsertSoftBodyObject(model, soft_body, m_offset, &status);
                model::SoftBody *newSoftBody = model::SoftBody::create();
                model::RigidBody::Resolver resolver;
                newSoftBody->bind(nanoemMutableModelSoftBodyGetOriginObject(soft_body), nullptr, resolver);
                newSoftBody->resetLanguage(
                    nanoemMutableModelSoftBodyGetOriginObject(soft_body), factory, project->castLanguage());
            }
            const nanoem_model_soft_body_t *m_base;
            const nanoem_rsize_t m_numSoftBodys;
            const int m_offset;
        };
        const nanoem_model_soft_body_t *selectedSoftBody = numSoftBodys > 0 ? soft_bodys[m_softBodyIndex] : nullptr;
        int selectedSoftBodyIndex = nanoemModelObjectGetIndex(nanoemModelSoftBodyGetModelObject(selectedSoftBody));
        if (ImGui::BeginMenu("Insert New")) {
            if (ImGui::MenuItem("at Last")) {
                m_parent->addLazyExecutionCommand(nanoem_new(CreateSoftBodyCommand(numSoftBodys, -1, nullptr)));
            }
            if (ImGui::MenuItem("after Selected", nullptr, nullptr, m_softBodyIndex < numSoftBodys)) {
                m_parent->addLazyExecutionCommand(
                    nanoem_new(CreateSoftBodyCommand(numSoftBodys, selectedSoftBodyIndex + 1, nullptr)));
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Insert Copy", m_softBodyIndex < numSoftBodys)) {
            if (ImGui::MenuItem("at Last")) {
                m_parent->addLazyExecutionCommand(
                    nanoem_new(CreateSoftBodyCommand(numSoftBodys, -1, selectedSoftBody)));
            }
            if (ImGui::MenuItem("at Next")) {
                m_parent->addLazyExecutionCommand(
                    nanoem_new(CreateSoftBodyCommand(numSoftBodys, selectedSoftBodyIndex + 1, selectedSoftBody)));
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(
            reinterpret_cast<const char *>(ImGuiWindow::kFAMinus), 0, m_softBodyIndex < numSoftBodys)) {
        struct DeleteSoftBodyCommand : ImGuiWindow::ILazyExecutionCommand {
            DeleteSoftBodyCommand(nanoem_model_soft_body_t *const *soft_bodys, nanoem_rsize_t &soft_bodyIndex)
                : m_soft_bodys(soft_bodys)
                , m_softBodyIndex(soft_bodyIndex)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableSoftBody soft_body(m_soft_bodys[m_softBodyIndex], activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelRemoveSoftBodyObject(model, soft_body, &status);
                if (m_softBodyIndex > 0) {
                    m_softBodyIndex--;
                }
            }
            nanoem_model_soft_body_t *const *m_soft_bodys;
            nanoem_rsize_t &m_softBodyIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(DeleteSoftBodyCommand(soft_bodys, m_softBodyIndex)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowUp), 0, m_softBodyIndex > 0)) {
        struct MoveSoftBodyUpCommand : ImGuiWindow::ILazyExecutionCommand {
            MoveSoftBodyUpCommand(nanoem_model_soft_body_t *const *soft_bodys, nanoem_rsize_t &soft_bodyIndex)
                : m_soft_bodys(soft_bodys)
                , m_softBodyIndex(soft_bodyIndex)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableSoftBody soft_body(m_soft_bodys[m_softBodyIndex], activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelRemoveSoftBodyObject(model, soft_body, &status);
                int offset = Inline::saturateInt32(--m_softBodyIndex);
                nanoemMutableModelInsertSoftBodyObject(model, soft_body, offset, &status);
            }
            nanoem_model_soft_body_t *const *m_soft_bodys;
            nanoem_rsize_t &m_softBodyIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(MoveSoftBodyUpCommand(soft_bodys, m_softBodyIndex)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(
            reinterpret_cast<const char *>(ImGuiWindow::kFAArrowDown), 0, m_softBodyIndex < numSoftBodys - 1)) {
        struct MoveSoftBodyDownCommand : ImGuiWindow::ILazyExecutionCommand {
            MoveSoftBodyDownCommand(nanoem_model_soft_body_t *const *soft_bodys, nanoem_rsize_t &soft_bodyIndex)
                : m_soft_bodys(soft_bodys)
                , m_softBodyIndex(soft_bodyIndex)
            {
            }
            void
            execute(Project *project)
            {
                Model *activeModel = project->activeModel();
                ScopedMutableSoftBody soft_body(m_soft_bodys[m_softBodyIndex], activeModel);
                ScopedMutableModel model(activeModel);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelRemoveSoftBodyObject(model, soft_body, &status);
                int offset = Inline::saturateInt32(++m_softBodyIndex);
                nanoemMutableModelInsertSoftBodyObject(model, soft_body, offset, &status);
            }
            nanoem_model_soft_body_t *const *m_soft_bodys;
            nanoem_rsize_t &m_softBodyIndex;
        };
        m_parent->addLazyExecutionCommand(nanoem_new(MoveSoftBodyDownCommand(soft_bodys, m_softBodyIndex)));
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("right-pane", ImGui::GetContentRegionAvail());
    if (numSoftBodys > 0 && m_softBodyIndex < numSoftBodys) {
        nanoem_model_soft_body_t *soft_bodyPtr = soft_bodys[m_softBodyIndex];
        layoutSoftBodyPropertyPane(soft_bodyPtr, project);
    }
    ImGui::EndChild();
}

void
ModelParameterDialog::layoutSoftBodyPropertyPane(nanoem_model_soft_body_t *softBodyPtr, Project *project)
{
    char buffer[Inline::kNameStackBufferSize];
    nanoem_language_type_t language = static_cast<nanoem_language_type_t>(m_language);
    StringUtils::UnicodeStringScope scope(project->unicodeStringFactory());
    ImGui::PushItemWidth(-1);
    if (layoutName(nanoemModelSoftBodyGetName(softBodyPtr, language), project, scope)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
        nanoemMutableModelSoftBodySetName(scoped, scope.value(), language, &status);
    }
    {
        const model::Material *material = model::Material::cast(nanoemModelSoftBodyGetMaterialObject(softBodyPtr));
        nanoem_rsize_t numMaterials;
        nanoem_model_material_t *const *materials =
            nanoemModelGetAllMaterialObjects(m_activeModel->data(), &numMaterials);
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.material"));
        if (ImGui::BeginCombo("##material", material ? material->nameConstString() : "(none)")) {
            for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
                const nanoem_model_material_t *candidateMaterialPtr = materials[i];
                if (const model::Material *candidateMaterial = model::Material::cast(candidateMaterialPtr)) {
                    StringUtils::format(buffer, sizeof(buffer), "%s##item[%lu].name", material->nameConstString(), i);
                    if (ImGui::Selectable(candidateMaterial->nameConstString(), candidateMaterial == material)) {
                        ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
                        nanoemMutableModelSoftBodySetMaterialObject(scoped, candidateMaterialPtr);
                    }
                }
            }
            ImGui::EndCombo();
        }
    }
    addSeparator();
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.shape-type"));
        nanoem_model_soft_body_shape_type_t value = nanoemModelSoftBodyGetShapeType(softBodyPtr);
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.soft-body.shape-type.tri-mesh"),
                value == NANOEM_MODEL_SOFT_BODY_SHAPE_TYPE_TRI_MESH)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetShapeType(scoped, NANOEM_MODEL_SOFT_BODY_SHAPE_TYPE_TRI_MESH);
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.soft-body.shape-type.rope"),
                value == NANOEM_MODEL_SOFT_BODY_SHAPE_TYPE_ROPE)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetShapeType(scoped, NANOEM_MODEL_SOFT_BODY_SHAPE_TYPE_ROPE);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.aero-model-type"));
        nanoem_model_soft_body_aero_model_type_t value = nanoemModelSoftBodyGetAeroModel(softBodyPtr);
        if (ImGui::BeginCombo("##test", selectedSoftBodyAeroMdoelType(value))) {
            for (int i = NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_FIRST_ENUM;
                 i < NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_MAX_ENUM; i++) {
                nanoem_model_soft_body_aero_model_type_t type =
                    static_cast<nanoem_model_soft_body_aero_model_type_t>(i);
                if (ImGui::Selectable(selectedSoftBodyAeroMdoelType(type))) {
                    ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
                    nanoemMutableModelSoftBodySetAeroModel(scoped, type);
                }
            }
            ImGui::EndCombo();
        }
    }
    addSeparator();
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.collision-group-id"));
        int value = nanoemModelSoftBodyGetCollisionGroupId(softBodyPtr);
        if (ImGui::DragInt("##collision.group", &value, 0.05f, 0, 15)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetCollisionGroupId(scoped, value);
        }
    }
    {
        nanoem_u32_t flags = ~nanoemModelSoftBodyGetCollisionMask(softBodyPtr);
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.collision-group-mask"));
        ImGui::Columns(8, nullptr, false);
        for (int i = 0; i < 16; i++) {
            char buffer[16];
            int offset = i + 1;
            StringUtils::format(buffer, sizeof(buffer), "%d##collision.mask.%d", offset, offset);
            if (ImGui::CheckboxFlags(buffer, &flags, 1 << i)) {
                ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
                nanoemMutableModelSoftBodySetCollisionMask(scoped, ~flags);
            }
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
    }
    addSeparator();
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.total-mass"));
        nanoem_f32_t value = nanoemModelSoftBodyGetTotalMass(softBodyPtr);
        if (ImGui::InputFloat("##total-mass", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetTotalMass(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.collision-margin"));
        nanoem_f32_t value = nanoemModelSoftBodyGetCollisionMargin(softBodyPtr);
        if (ImGui::InputFloat("##collision-margin", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetCollisionMargin(scoped, value);
        }
    }
    addSeparator();
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.velocity-correction-factor"));
        nanoem_f32_t value = nanoemModelSoftBodyGetVelocityCorrectionFactor(softBodyPtr);
        if (ImGui::InputFloat("##velocity-correction-factor", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetVelocityCorrectionFactor(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.damping-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetDampingCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##damping-coefficient", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetDampingCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.drag-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetDragCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##drag-coefficient", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetDragCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.lift-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetLiftCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##lift-coefficient", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetLiftCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.pressure-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetPressureCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##pressure-coefficient", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetPressureCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.volume-conversation-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetVolumeConversationCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##volume-conversation-coefficient", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetVolumeConversationCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.dynamic-friction-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetDynamicFrictionCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##dynamic-friction-coefficient", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetDynamicFrictionCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.pose-matching-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetPoseMatchingCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##pose-matching-coefficient", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetPoseMatchingCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.rigid-contact-hardness"));
        nanoem_f32_t value = nanoemModelSoftBodyGetRigidContactHardness(softBodyPtr);
        if (ImGui::InputFloat("##rigid-contact-hardness", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetRigidContactHardness(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.kinetic-contact-hardness"));
        nanoem_f32_t value = nanoemModelSoftBodyGetKineticContactHardness(softBodyPtr);
        if (ImGui::InputFloat("##kinetic-contact-hardness", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetKineticContactHardness(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.soft-contact-hardness"));
        nanoem_f32_t value = nanoemModelSoftBodyGetSoftContactHardness(softBodyPtr);
        if (ImGui::InputFloat("##soft-contact-hardness", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetSoftContactHardness(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.anchor-hardness"));
        nanoem_f32_t value = nanoemModelSoftBodyGetAnchorHardness(softBodyPtr);
        if (ImGui::InputFloat("##anchor-hardness", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetAnchorHardness(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.soft-vs-kinetic-hardness"));
        nanoem_f32_t value = nanoemModelSoftBodyGetSoftVSKineticHardness(softBodyPtr);
        if (ImGui::InputFloat("##soft-vs-kinetic-hardness", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetSoftVSKineticHardness(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.soft-vs-rigid-hardness"));
        nanoem_f32_t value = nanoemModelSoftBodyGetSoftVSRigidHardness(softBodyPtr);
        if (ImGui::InputFloat("##soft-vs-rigid-hardness", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetSoftVSRigidHardness(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.soft-vs-soft-hardness"));
        nanoem_f32_t value = nanoemModelSoftBodyGetSoftVSSoftHardness(softBodyPtr);
        if (ImGui::InputFloat("##soft-vs-soft-hardness", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetSoftVSSoftHardness(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.soft-vs-kinetic-impulse-split"));
        nanoem_f32_t value = nanoemModelSoftBodyGetSoftVSKineticImpulseSplit(softBodyPtr);
        if (ImGui::InputFloat("##soft-vs-kinetic-impulse-split", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetSoftVSKineticImpulseSplit(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.soft-vs-rigid-impulse-split"));
        nanoem_f32_t value = nanoemModelSoftBodyGetSoftVSRigidImpulseSplit(softBodyPtr);
        if (ImGui::InputFloat("##soft-vs-rigid-impulse-split", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetSoftVSRigidImpulseSplit(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.soft-vs-soft-impulse-split"));
        nanoem_f32_t value = nanoemModelSoftBodyGetSoftVSSoftImpulseSplit(softBodyPtr);
        if (ImGui::InputFloat("##soft-vs-kinetic-soft-split", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetSoftVSSoftImpulseSplit(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.linear-stiffness-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetLinearStiffnessCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##linear-stiffness-coefficient", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetLinearStiffnessCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.angular-stiffness-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetAngularStiffnessCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##angular-stiffness-coefficient", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetAngularStiffnessCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.volume-stiffness-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetVolumeStiffnessCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##volume-stiffness-coefficient", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetVolumeStiffnessCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.bending-constraints-distance"));
        int value = nanoemModelSoftBodyGetBendingConstraintsDistance(softBodyPtr);
        if (ImGui::InputInt("##bending-constraints-distance", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetBendingConstraintsDistance(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.cluster-count"));
        int value = nanoemModelSoftBodyGetClusterCount(softBodyPtr);
        if (ImGui::InputInt("##cluster-count", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetClusterCount(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.velocity-solver-iterations"));
        int value = nanoemModelSoftBodyGetVelocitySolverIterations(softBodyPtr);
        if (ImGui::InputInt("##velocity-solver-iterations", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetVelocitySolverIterations(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.position-solver-iterations"));
        int value = nanoemModelSoftBodyGetVelocitySolverIterations(softBodyPtr);
        if (ImGui::InputInt("##position-solver-iterations", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetPositionsSolverIterations(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.drift-solver-iterations"));
        int value = nanoemModelSoftBodyGetDriftSolverIterations(softBodyPtr);
        if (ImGui::InputInt("##drift-solver-iterations", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetDriftSolverIterations(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.cluster-solver-iterations"));
        int value = nanoemModelSoftBodyGetClusterSolverIterations(softBodyPtr);
        if (ImGui::InputInt("##cluster-solver-iterations", &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetClusterSolverIterations(scoped, value);
        }
    }
    {
        bool value = nanoemModelSoftBodyIsBendingConstraintsEnabled(softBodyPtr) != 0;
        if (ImGui::Checkbox(tr("nanoem.gui.model.edit.soft-body.bending-constraints-enabled"), &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetBendingConstraintsEnabled(scoped, value);
        }
    }
    {
        bool value = nanoemModelSoftBodyIsClustersEnabled(softBodyPtr) != 0;
        if (ImGui::Checkbox(tr("nanoem.gui.model.edit.soft-body.clusters-enabled"), &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetClustersEnabled(scoped, value);
        }
    }
    {
        bool value = nanoemModelSoftBodyIsRandomizeConstraintsNeeded(softBodyPtr) != 0;
        if (ImGui::Checkbox(tr("nanoem.gui.model.edit.soft-body.randomize-constraints-enabled"), &value)) {
            ScopedMutableSoftBody scoped(softBodyPtr, m_activeModel);
            nanoemMutableModelSoftBodySetRandomizeConstraintsNeeded(scoped, value);
        }
    }
}

bool
ModelParameterDialog::layoutName(
    const nanoem_unicode_string_t *namePtr, Project *project, StringUtils::UnicodeStringScope &scope)
{
    String name;
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::getUtf8String(namePtr, factory, name);
    MutableString nameBuffer;
    nameBuffer.assign(name.c_str(), name.c_str() + name.size());
    nameBuffer.push_back(0);
    ImGui::RadioButton(
        tr("nanoem.gui.window.preference.project.language.japanese"), &m_language, NANOEM_LANGUAGE_TYPE_JAPANESE);
    ImGui::SameLine();
    ImGui::RadioButton(
        tr("nanoem.gui.window.preference.project.language.english"), &m_language, NANOEM_LANGUAGE_TYPE_ENGLISH);
    ImGui::TextUnformatted("Name");
    bool changed = false;
    if (ImGui::InputText("##name", nameBuffer.data(), nameBuffer.capacity()) &&
        StringUtils::tryGetString(factory, nameBuffer.data(), scope)) {
        changed = true;
    }
    return changed;
}

void
ModelParameterDialog::toggleTab(TabType value, Project *project)
{
    if (value != m_tabType) {
        beforeToggleTab(project);
        m_tabType = value;
        afterToggleTab(value, project);
    }
}

void
ModelParameterDialog::beforeToggleTab(Project *project)
{
    switch (m_tabType) {
    case kTabTypeVertex: {
        m_activeModel->setShowAllVertexPoints(false);
        break;
    }
    case kTabTypeFace: {
        m_activeModel->setShowAllVertexFaces(false);
        break;
    }
    case kTabTypeBone: {
        m_activeModel->setShowAllBones(false);
        break;
    }
    case kTabTypeMorph: {
        if (Motion *motion = project->resolveMotion(m_activeModel)) {
            m_activeModel->synchronizeMotion(
                motion, project->currentLocalFrameIndex(), 0, PhysicsEngine::kSimulationTimingBefore);
            m_activeModel->resetAllVertices();
            m_activeModel->deformAllMorphs(false);
            m_activeModel->markStagingVertexBufferDirty();
        }
        break;
    }
    case kTabTypeRigidBody: {
        m_activeModel->setShowAllRigidBodies(false);
        break;
    }
    case kTabTypeJoint: {
        m_activeModel->setShowAllJoints(false);
        break;
    }
    case kTabTypeSoftBody: {
        break;
    }
    default:
        break;
    }
}

void
ModelParameterDialog::afterToggleTab(TabType value, Project *project)
{
    switch (value) {
    case kTabTypeVertex: {
        m_activeModel->setShowAllVertexPoints(true);
        m_activeModel->selection()->setEditingType(IModelObjectSelection::kEditingTypeVertex);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case kTabTypeFace: {
        m_activeModel->setShowAllVertexFaces(true);
        m_activeModel->selection()->setEditingType(IModelObjectSelection::kEditingTypeFace);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case kTabTypeMaterial: {
        m_activeModel->selection()->setEditingType(IModelObjectSelection::kEditingTypeMaterial);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case kTabTypeBone: {
        m_activeModel->selection()->setEditingType(IModelObjectSelection::kEditingTypeBone);
        m_activeModel->setShowAllBones(true);
        break;
    }
    case kTabTypeRigidBody: {
        m_activeModel->setShowAllRigidBodies(true);
        m_activeModel->selection()->setEditingType(IModelObjectSelection::kEditingTypeRigidBody);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case kTabTypeJoint: {
        m_activeModel->setShowAllJoints(true);
        m_activeModel->selection()->setEditingType(IModelObjectSelection::kEditingTypeJoint);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case kTabTypeSoftBody: {
        m_activeModel->selection()->setEditingType(IModelObjectSelection::kEditingTypeSoftBody);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    default:
        break;
    }
}

void
ModelParameterDialog::forceUpdateMorph(model::Morph *morph, Project *project)
{
    if (Motion *motion = project->resolveMotion(m_activeModel)) {
        m_activeModel->synchronizeMotion(
            motion, project->currentLocalFrameIndex(), 0, PhysicsEngine::kSimulationTimingBefore);
        morph->setForcedWeight(1.0f);
        m_activeModel->resetAllVertices();
        m_activeModel->deformAllMorphs(false);
        m_activeModel->markStagingVertexBufferDirty();
        project->update();
    }
}

const char *
ModelParameterDialog::selectedCodecType(const nanoem_codec_type_t type) const NANOEM_DECL_NOEXCEPT
{
    switch (type) {
    case NANOEM_CODEC_TYPE_SJIS: {
        return "ShiftJIS";
    }
    case NANOEM_CODEC_TYPE_UTF8: {
        return "UTF-8";
    }
    case NANOEM_CODEC_TYPE_UTF16: {
        return "UTF-16";
    }
    default:
        return "(Unknown)";
    }
}

const char *
ModelParameterDialog::selectedVertexType(const nanoem_model_vertex_type_t type) const NANOEM_DECL_NOEXCEPT
{
    switch (type) {
    case NANOEM_MODEL_VERTEX_TYPE_BDEF1: {
        return "BDEF1";
    }
    case NANOEM_MODEL_VERTEX_TYPE_BDEF2: {
        return "BDEF2";
    }
    case NANOEM_MODEL_VERTEX_TYPE_BDEF4: {
        return "BDEF4";
    }
    case NANOEM_MODEL_VERTEX_TYPE_SDEF: {
        return "SDEF";
    }
    case NANOEM_MODEL_VERTEX_TYPE_QDEF: {
        return "QDEF";
    }
    default:
        return "(Unknown)";
    }
}

const char *
ModelParameterDialog::selectedMaterialPrimitiveType(
    const nanoem_model_material_t *materialPtr) const NANOEM_DECL_NOEXCEPT
{
    const char *typeName;
    if (nanoemModelMaterialIsPointDrawEnabled(materialPtr)) {
        typeName = tr("nanoem.gui.model.edit.material.primitive.point");
    }
    else if (nanoemModelMaterialIsLineDrawEnabled(materialPtr)) {
        typeName = tr("nanoem.gui.model.edit.material.primitive.line");
    }
    else {
        typeName = tr("nanoem.gui.model.edit.material.primitive.triangle");
    }
    return typeName;
}

const char *
ModelParameterDialog::selectedMaterialSphereMapType(
    const nanoem_model_material_sphere_map_texture_type_t value) const NANOEM_DECL_NOEXCEPT
{
    switch (value) {
    case NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE:
        return tr("nanoem.gui.model.edit.material.sphere.type.none");
    case NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_ADD:
        return tr("nanoem.gui.model.edit.material.sphere.type.add");
    case NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_MULTIPLY:
        return tr("nanoem.gui.model.edit.material.sphere.type.multiply");
    case NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_SUB_TEXTURE:
        return tr("nanoem.gui.model.edit.material.sphere.type.sub-texture");
    default:
        return "(Unknown)";
    }
}

const char *
ModelParameterDialog::selectedMorphCategory(nanoem_model_morph_category_t value) const NANOEM_DECL_NOEXCEPT
{
    switch (value) {
    case NANOEM_MODEL_MORPH_CATEGORY_EYE:
        return tr("nanoem.gui.model.edit.morph.category.eye");
    case NANOEM_MODEL_MORPH_CATEGORY_LIP:
        return tr("nanoem.gui.model.edit.morph.category.lip");
    case NANOEM_MODEL_MORPH_CATEGORY_EYEBROW:
        return tr("nanoem.gui.model.edit.morph.category.eyebrow");
    case NANOEM_MODEL_MORPH_CATEGORY_OTHER:
        return tr("nanoem.gui.model.edit.morph.category.other");
    default:
        return "(Unknown)";
    }
}

const char *
ModelParameterDialog::selectedMorphType(nanoem_model_morph_type_t value) const NANOEM_DECL_NOEXCEPT
{
    switch (value) {
    case NANOEM_MODEL_MORPH_TYPE_BONE:
        return tr("nanoem.gui.model.edit.morph.type.bone");
    case NANOEM_MODEL_MORPH_TYPE_FLIP:
        return tr("nanoem.gui.model.edit.morph.type.flip");
    case NANOEM_MODEL_MORPH_TYPE_GROUP:
        return tr("nanoem.gui.model.edit.morph.type.group");
    case NANOEM_MODEL_MORPH_TYPE_IMPULUSE:
        return tr("nanoem.gui.model.edit.morph.type.impulse");
    case NANOEM_MODEL_MORPH_TYPE_MATERIAL:
        return tr("nanoem.gui.model.edit.morph.type.material");
    case NANOEM_MODEL_MORPH_TYPE_TEXTURE:
        return tr("nanoem.gui.model.edit.morph.type.texture");
    case NANOEM_MODEL_MORPH_TYPE_UVA1:
        return tr("nanoem.gui.model.edit.morph.type.uva1");
    case NANOEM_MODEL_MORPH_TYPE_UVA2:
        return tr("nanoem.gui.model.edit.morph.type.uva2");
    case NANOEM_MODEL_MORPH_TYPE_UVA3:
        return tr("nanoem.gui.model.edit.morph.type.uva3");
    case NANOEM_MODEL_MORPH_TYPE_UVA4:
        return tr("nanoem.gui.model.edit.morph.type.uva4");
    case NANOEM_MODEL_MORPH_TYPE_VERTEX:
        return tr("nanoem.gui.model.edit.morph.type.vertex");
    default:
        return "(Unknown)";
    }
}

const char *
ModelParameterDialog::selectedMorphMaterialOperationType(
    nanoem_model_morph_material_operation_type_t value) const NANOEM_DECL_NOEXCEPT
{
    switch (value) {
    case NANOEM_MODEL_MORPH_MATERIAL_OPERATION_TYPE_ADD:
        return "Add";
    case NANOEM_MODEL_MORPH_MATERIAL_OPERATION_TYPE_MULTIPLY:
        return "Multiply";
    default:
        return "(Unknown)";
    }
}

const char *
ModelParameterDialog::selectedJointType(nanoem_model_joint_type_t value) const NANOEM_DECL_NOEXCEPT
{
    switch (value) {
    case NANOEM_MODEL_JOINT_TYPE_HINGE_CONSTRAINT:
        return "Hinge";
    case NANOEM_MODEL_JOINT_TYPE_SLIDER_CONSTRAINT:
        return "Slider";
    case NANOEM_MODEL_JOINT_TYPE_CONE_TWIST_CONSTRAINT:
        return "Cone Twist";
    case NANOEM_MODEL_JOINT_TYPE_POINT2POINT_CONSTRAINT:
        return "Point to Point";
    case NANOEM_MODEL_JOINT_TYPE_GENERIC_6DOF_CONSTRAINT:
        return "6-DOF";
    case NANOEM_MODEL_JOINT_TYPE_GENERIC_6DOF_SPRING_CONSTRAINT:
        return "6-DOF with Spring";
    default:
        return "(Unknown)";
    }
}

const char *
ModelParameterDialog::selectedSoftBodyAeroMdoelType(
    const nanoem_model_soft_body_aero_model_type_t type) const NANOEM_DECL_NOEXCEPT
{
    switch (type) {
    case NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_VERTEX_POINT:
        return tr("nanoem.gui.model.edit.soft-body.aero-model-type.vertex-point");
    case NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_VERTEX_TWO_SIDED:
        return tr("nanoem.gui.model.edit.soft-body.aero-model-type.vertex-two-sided");
    case NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_VERTEX_ONE_SIDED:
        return tr("nanoem.gui.model.edit.soft-body.aero-model-type.vertex-one-sided");
    case NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_FACE_TWO_SIDED:
        return tr("nanoem.gui.model.edit.soft-body.aero-model-type.face-two-sided");
    case NANOEM_MODEL_SOFT_BODY_AERO_MODEL_TYPE_FACE_ONE_SIDED:
        return tr("nanoem.gui.model.edit.soft-body.aero-model-type.face-one-sided");
    default:
        return "(Unknown)";
    }
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
