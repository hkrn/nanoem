/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/ModelParameterDialog.h"

#include "emapp/Error.h"
#include "emapp/Grid.h"
#include "emapp/ICamera.h"
#include "emapp/IEventPublisher.h"
#include "emapp/IFileManager.h"
#include "emapp/IImageView.h"
#include "emapp/ILight.h"
#include "emapp/Progress.h"
#include "emapp/command/ModelObjectCommand.h"
#include "emapp/internal/imgui/ModelEditCommandDialog.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace internal {
namespace imgui {
namespace {

struct UndoCommand : ImGuiWindow::ILazyExecutionCommand {
    UndoCommand(undo_command_t *command)
        : m_command(command)
    {
    }
    ~UndoCommand() NANOEM_DECL_NOEXCEPT
    {
        m_command = nullptr;
    }
    void
    execute(Project *project)
    {
        project->activeModel()->pushUndo(m_command);
    }
    undo_command_t *m_command;
};

} /* namespace anonymous */

const char *const ModelParameterDialog::kIdentifier = "dialog.project.model";
const nanoem_f32_t ModelParameterDialog::kMinimumWindowWidth = 600;
const int ModelParameterDialog::kWindowFrameHeightRowCount = 17;
const int ModelParameterDialog::kInnerItemListFrameHeightRowCount = 10;

IModelObjectSelection::EditingType
ModelParameterDialog::convertToEditingType(TabType tab) NANOEM_DECL_NOEXCEPT
{
    switch (tab) {
    case kTabTypeBone:
        return IModelObjectSelection::kEditingTypeBone;
    case kTabTypeFace:
        return IModelObjectSelection::kEditingTypeFace;
    case kTabTypeJoint:
        return IModelObjectSelection::kEditingTypeJoint;
    case kTabTypeLabel:
        return IModelObjectSelection::kEditingTypeLabel;
    case kTabTypeMaterial:
        return IModelObjectSelection::kEditingTypeMaterial;
    case kTabTypeMorph:
        return IModelObjectSelection::kEditingTypeMorph;
    case kTabTypeRigidBody:
        return IModelObjectSelection::kEditingTypeRigidBody;
    case kTabTypeSoftBody:
        return IModelObjectSelection::kEditingTypeSoftBody;
    case kTabTypeVertex:
        return IModelObjectSelection::kEditingTypeVertex;
    default:
        return IModelObjectSelection::kEditingTypeNone;
    }
}

void
ModelParameterDialog::formatVertexText(char *buffer, nanoem_rsize_t size, const nanoem_model_vertex_t *vertexPtr)
{
    const nanoem_f32_t *origin = nanoemModelVertexGetOrigin(vertexPtr);
    int offset = model::Vertex::index(vertexPtr) + 1;
    StringUtils::format(buffer, size, "%d (%.2f, %.2f, %.2f)", offset, origin[0], origin[1], origin[2]);
}

ModelParameterDialog::ModelParameterDialog(
    Model *model, Project *project, BaseApplicationService *applicationPtr, ImGuiWindow *parent)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_parent(parent)
    , m_project(project)
    , m_activeModel(model)
    , m_saveState(nullptr)
    , m_language(project->castLanguage())
    , m_tabType(kTabTypeFirstEnum)
    , m_explicitTabType(kTabTypeMaxEnum)
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
    , m_showAllRigidBodies(model->isShowAllRigidBodyShapes())
    , m_showAllJoints(model->isShowAllJointShapes())
    , m_showAllSoftBodies(false)
{
    ModelEditCommandDialog::afterToggleEditingMode(IModelObjectSelection::kEditingTypeNone, m_activeModel, project);
    project->saveState(m_saveState);
    project->activeCamera()->reset();
    project->activeLight()->reset();
    project->grid()->setVisible(true);
    setActiveModel(model);
}

ModelParameterDialog::~ModelParameterDialog()
{
    const Project::DrawableList drawables(m_project->drawableOrderList());
    Error error;
    for (SavedModelMotionMap::const_iterator it = m_savedModelMotions.begin(), end = m_savedModelMotions.end(); it != end; ++it) {
        Model *model = it->first;
        if (Motion *motion = m_project->resolveMotion(model)) {
            motion->load(it->second, 0, error);
        }
    }
    for (Project::DrawableList::const_iterator it = drawables.begin(), end = drawables.end(); it != end; ++it) {
        IDrawable *drawable = *it;
        if (drawable != m_activeModel) {
            drawable->setVisible(true);
        }
    }
    m_project->restoreState(m_saveState, true);
    m_activeModel->restoreBindPose(m_bindPose);
    m_saveState = nullptr;
}

bool
ModelParameterDialog::draw(Project *project)
{
    String windowTitle;
    nanoem_f32_t width = kMinimumWindowWidth * project->windowDevicePixelRatio();
    bool visible = true;
    Model *currentActiveModel = project->activeModel();
    if (currentActiveModel != m_activeModel) {
        if (currentActiveModel) {
            setActiveModel(currentActiveModel);
        }
        m_activeModel = currentActiveModel;
        visible = currentActiveModel != nullptr;
    }
    if (m_activeModel) {
        StringUtils::format(
            windowTitle, "%s - %s", tr("nanoem.gui.window.model.title"), m_activeModel->nameConstString());
    }
    if (open(windowTitle.c_str(), kIdentifier, &visible,
            ImVec2(width, ImGui::GetFrameHeightWithSpacing() * kWindowFrameHeightRowCount), ImGuiWindowFlags_None)) {
        const TabType explicitTabType = m_explicitTabType;
        if (explicitTabType != kTabTypeMaxEnum) {
            m_explicitTabType = kTabTypeMaxEnum;
        }
        ImGui::BeginTabBar("tabbar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton);
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.info"), nullptr,
                explicitTabType == kTabTypeInfo ? ImGuiTabItemFlags_SetSelected : 0)) {
            layoutInformation(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeInfo, project);
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.vertex"), nullptr,
                explicitTabType == kTabTypeVertex ? ImGuiTabItemFlags_SetSelected : 0)) {
            layoutAllVertices(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeVertex, project);
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.face"), nullptr,
                explicitTabType == kTabTypeFace ? ImGuiTabItemFlags_SetSelected : 0)) {
            layoutAllFaces(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeFace, project);
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.material"), nullptr,
                explicitTabType == kTabTypeMaterial ? ImGuiTabItemFlags_SetSelected : 0)) {
            layoutAllMaterials(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeMaterial, project);
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.bone"), nullptr,
                explicitTabType == kTabTypeBone ? ImGuiTabItemFlags_SetSelected : 0)) {
            layoutAllBones(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeBone, project);
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.morph"), nullptr,
                explicitTabType == kTabTypeMorph ? ImGuiTabItemFlags_SetSelected : 0)) {
            layoutAllMorphs(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeMorph, project);
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.label"), nullptr,
                explicitTabType == kTabTypeLabel ? ImGuiTabItemFlags_SetSelected : 0)) {
            layoutAllLabels(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeLabel, project);
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.rigid-body"), nullptr,
                explicitTabType == kTabTypeRigidBody ? ImGuiTabItemFlags_SetSelected : 0)) {
            layoutAllRigidBodies(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeRigidBody, project);
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.joint"), nullptr,
                explicitTabType == kTabTypeJoint ? ImGuiTabItemFlags_SetSelected : 0)) {
            layoutAllJoints(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeJoint, project);
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.model.tab.soft-body"), nullptr,
                explicitTabType == kTabTypeSoftBody ? ImGuiTabItemFlags_SetSelected : 0)) {
            layoutAllSoftBodies(project);
            ImGui::EndTabItem();
            toggleTab(kTabTypeSoftBody, project);
        }
        ImGui::EndTabBar();
    }
    else if (!visible && m_activeModel) {
        project->setEditingMode(m_editingMode);
        m_activeModel->setShowAllVertexPoints(m_showAllVertexPoints);
        m_activeModel->setShowAllVertexFaces(m_showAllVertexFaces);
        m_activeModel->setShowAllBones(m_showAllBones);
        m_activeModel->setShowAllRigidBodyShapes(m_showAllRigidBodies);
        m_activeModel->setShowAllJointShapes(m_showAllJoints);
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
            command::ScopedMutableModel scoped(m_activeModel);
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
            command::ScopedMutableModel scoped(m_activeModel);
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
                command::ScopedMutableModel scoped(m_activeModel);
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
                command::ScopedMutableVertex scoped(vertexPtr);
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
        const nanoem_model_bone_t *bonePtr = nanoemModelVertexGetBoneObject(vertexPtr, i);
        StringUtils::format(label, sizeof(label), "%s##toggle%jd", ImGuiWindow::kFALink, i);
        if (ImGuiWindow::handleButton(label, 0, bonePtr != nullptr)) {
            m_explicitTabType = kTabTypeBone;
            m_boneIndex = nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(bonePtr));
        }
        ImGui::SameLine();
        StringUtils::format(label, sizeof(label), "##bone%jd", i);
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.75f);
        layoutVertexBoneSelection(label, vertexPtr, i, bones, numBones);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        nanoem_f32_t weight = nanoemModelVertexGetBoneWeight(vertexPtr, i);
        StringUtils::format(label, sizeof(label), "##weight%jd", i);
        if (ImGui::SliderFloat(label, &weight, 0, 1)) {
            command::ScopedMutableVertex scoped(vertexPtr);
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
    IModelObjectSelection *selection = m_activeModel->selection();
    if (ImGui::BeginPopup("vertex-op-menu")) {
        if (ImGui::BeginMenu("Selection")) {
            if (ImGui::MenuItem("Select All Vertices")) {
                for (nanoem_rsize_t i = 0; i < numVertices; i++) {
                    const nanoem_model_vertex_t *vertexPtr = vertices[i];
                    selection->addVertex(vertexPtr);
                }
            }
            if (ImGui::MenuItem("Deselect All Vertices")) {
                selection->removeAllVertices();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Masking")) {
            if (ImGui::MenuItem("Mask All Selected Vertices")) {
                const model::Vertex::Set vertices(selection->allVertexSet());
                for (model::Vertex::Set::const_iterator it = vertices.begin(), end = vertices.end(); it != end; ++it) {
                    const nanoem_model_vertex_t *vertexPtr = *it;
                    if (model::Vertex *vertex = model::Vertex::cast(vertexPtr)) {
                        vertex->setEditingMasked(true);
                    }
                }
            }
            if (ImGui::MenuItem("Unmask All Selected Vertices")) {
                const model::Vertex::Set vertices(selection->allVertexSet());
                for (model::Vertex::Set::const_iterator it = vertices.begin(), end = vertices.end(); it != end; ++it) {
                    const nanoem_model_vertex_t *vertexPtr = *it;
                    if (model::Vertex *vertex = model::Vertex::cast(vertexPtr)) {
                        vertex->setEditingMasked(false);
                    }
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Invert All Mask/Unmask Vertices")) {
                for (nanoem_rsize_t i = 0; i < numVertices; i++) {
                    const nanoem_model_vertex_t *vertexPtr = vertices[i];
                    if (model::Vertex *vertex = model::Vertex::cast(vertexPtr)) {
                        vertex->setEditingMasked(!vertex->isEditingMasked());
                    }
                }
            }
            ImGui::EndMenu();
        }
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
    clipper.Begin(Inline::saturateInt32(numVertices));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            char buffer[Inline::kNameStackBufferSize];
            nanoem_model_vertex_t *vertexPtr = vertices[i];
            const bool selected = selection->containsVertex(vertexPtr);
            if (selected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWindow::kColorSelectedModelObject);
            }
            model::Vertex *vertex = model::Vertex::cast(vertexPtr);
            bool visible = vertex->isEditingMasked() ? false : true;
            StringUtils::format(buffer, sizeof(buffer), "##vertex[%d].visible", i);
            if (ImGui::Checkbox(buffer, &visible)) {
                vertex->setEditingMasked(visible ? false : true);
            }
            ImGui::SameLine();
            formatVertexText(buffer, sizeof(buffer), vertexPtr);
            if (ImGui::Selectable(buffer, selected)) {
                const ImGuiIO &io = ImGui::GetIO();
                if (io.KeyCtrl) {
                    selected ? selection->removeVertex(vertexPtr) : selection->addVertex(vertexPtr);
                }
                else {
                    if (io.KeyShift) {
                        const nanoem_rsize_t offset = i, from = glm::min(offset, m_vertexIndex),
                                             to = glm::max(offset, m_vertexIndex);
                        for (nanoem_rsize_t j = from; j < to; j++) {
                            selection->addVertex(vertices[j]);
                        }
                    }
                    else {
                        selection->removeAllVertices();
                    }
                    m_vertexIndex = i;
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
            if (selected) {
                ImGui::PopStyleColor();
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
            command::ScopedMutableVertex scoped(vertexPtr);
            nanoemMutableModelVertexSetOrigin(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.vertex.normal"));
        Vector4 value(glm::make_vec4(nanoemModelVertexGetNormal(vertexPtr)));
        if (ImGui::InputFloat3("##normal", glm::value_ptr(value))) {
            command::ScopedMutableVertex scoped(vertexPtr);
            nanoemMutableModelVertexSetNormal(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.vertex.texcoord"));
        Vector4 value(glm::make_vec4(nanoemModelVertexGetTexCoord(vertexPtr)));
        if (ImGui::InputFloat2("##texcoord", glm::value_ptr(value))) {
            command::ScopedMutableVertex scoped(vertexPtr);
            nanoemMutableModelVertexSetTexCoord(scoped, glm::value_ptr(value));
        }
    }
    {
        nanoem_rsize_t offset = 0;
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.2f);
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
        ImGui::PopItemWidth();
        ImGui::SameLine();
        Vector4 value(glm::make_vec4(nanoemModelVertexGetAdditionalUV(vertexPtr, offset)));
        if (ImGui::InputFloat4("##uva-value", glm::value_ptr(value))) {
            command::ScopedMutableVertex scoped(vertexPtr);
            nanoemMutableModelVertexSetAdditionalUV(scoped, glm::value_ptr(value), offset);
        }
    }
    addSeparator();
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.vertex.material"));
        char label[Inline::kNameStackBufferSize];
        const model::Vertex *vertex = model::Vertex::cast(vertexPtr);
        const nanoem_model_material_t *materialPtr = vertex->material();
        StringUtils::format(label, sizeof(label), "%s##material", ImGuiWindow::kFALink);
        if (ImGuiWindow::handleButton(label, 0, materialPtr != nullptr)) {
            m_explicitTabType = kTabTypeMaterial;
            m_materialIndex = nanoemModelObjectGetIndex(nanoemModelMaterialGetModelObject(materialPtr));
        }
        ImGui::SameLine();
        const model::Material *material = model::Material::cast(materialPtr);
        ImGui::TextUnformatted(material ? material->nameConstString() : "(none)");
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.vertex.edge"));
        nanoem_f32_t width = nanoemModelVertexGetEdgeSize(vertexPtr);
        if (ImGui::InputFloat("##edge", &width)) {
            command::ScopedMutableVertex scoped(vertexPtr);
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
                    command::ScopedMutableVertex scoped(vertexPtr);
                    nanoemMutableModelVertexSetType(scoped, type);
                }
            }
            ImGui::EndCombo();
        }
        switch (nanoemModelVertexGetType(vertexPtr)) {
        case NANOEM_MODEL_VERTEX_TYPE_BDEF1: {
            char label[Inline::kNameStackBufferSize];
            nanoem_rsize_t numBones;
            nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
            const nanoem_model_bone_t *bonePtr = nanoemModelVertexGetBoneObject(vertexPtr, 0);
            StringUtils::format(label, sizeof(label), "%s##toggle0", ImGuiWindow::kFALink);
            if (ImGuiWindow::handleButton(label, 0, bonePtr != nullptr)) {
                m_explicitTabType = kTabTypeBone;
                m_boneIndex = nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(bonePtr));
            }
            ImGui::SameLine();
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
                    command::ScopedMutableVertex scoped(vertexPtr);
                    nanoemMutableModelVertexSetSdefR0(scoped, glm::value_ptr(value));
                }
            }
            {
                ImGui::TextUnformatted(tr("nanoem.gui.model.edit.vertex.sdef.r1"));
                Vector4 value(glm::make_vec4(nanoemModelVertexGetSdefR1(vertexPtr)));
                if (ImGui::InputFloat3("##sdef.r1", glm::value_ptr(value))) {
                    command::ScopedMutableVertex scoped(vertexPtr);
                    nanoemMutableModelVertexSetSdefR1(scoped, glm::value_ptr(value));
                }
            }
            {
                ImGui::TextUnformatted(tr("nanoem.gui.model.edit.vertex.sdef.c"));
                Vector4 value(glm::make_vec4(nanoemModelVertexGetSdefC(vertexPtr)));
                if (ImGui::InputFloat3("##sdef.c", glm::value_ptr(value))) {
                    command::ScopedMutableVertex scoped(vertexPtr);
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
    IModelObjectSelection *selection = m_activeModel->selection();
    if (ImGui::BeginPopup("face-op-menu")) {
        if (ImGui::BeginMenu("Selection")) {
            if (ImGui::MenuItem("Select All Faces")) {
                for (nanoem_rsize_t i = 0; i < numFaces; i++) {
                    const nanoem_u32_t *facesPtr = &vertexIndices[i * 3];
                    const Vector4UI32 face(i, facesPtr[0], facesPtr[1], facesPtr[2]);
                    selection->addFace(face);
                }
            }
            if (ImGui::MenuItem("Deselect All Faces")) {
                selection->removeAllFaces();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Masking")) {
            if (ImGui::MenuItem("Mask All Selected Faces")) {
                const IModelObjectSelection::FaceList faces(selection->allFaces());
                for (IModelObjectSelection::FaceList::const_iterator it = faces.begin(), end = faces.end(); it != end;
                     ++it) {
                    const Vector4UI32 &face = *it;
                    m_activeModel->setFaceEditingMasked(face.x, true);
                }
            }
            if (ImGui::MenuItem("Unmask All Selected Faces")) {
                const IModelObjectSelection::FaceList faces(selection->allFaces());
                for (IModelObjectSelection::FaceList::const_iterator it = faces.begin(), end = faces.end(); it != end;
                     ++it) {
                    const Vector4UI32 &face = *it;
                    m_activeModel->setFaceEditingMasked(face.x, false);
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Invert All Mask/Unmask Faces")) {
                nanoem_rsize_t numVertices;
                nanoem_model_vertex_t *const *vertices =
                    nanoemModelGetAllVertexObjects(m_activeModel->data(), &numVertices);
                for (nanoem_rsize_t i = 0; i < numFaces; i++) {
                    const nanoem_u32_t *facesPtr = &vertexIndices[i * 3];
                    for (nanoem_rsize_t j = 0; j < 3; j++) {
                        const nanoem_model_vertex_t *vertexPtr = vertices[facesPtr[j]];
                        if (model::Vertex *vertex = model::Vertex::cast(vertexPtr)) {
                            vertex->setEditingMasked(!vertex->isEditingMasked());
                        }
                    }
                }
            }
            ImGui::EndMenu();
        }
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
            const nanoem_rsize_t offset = nanoem_rsize_t(i) * 3;
            const nanoem_u32_t vertexIndex0 = vertexIndices[offset + 0], vertexIndex1 = vertexIndices[offset + 1],
                               vertexIndex2 = vertexIndices[offset + 2];
            const Vector4UI32 face(i, vertexIndex0, vertexIndex1, vertexIndex2);
            bool selected = selection->containsFace(face);
            if (selected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWindow::kColorSelectedModelObject);
            }
            bool visible = m_activeModel->isFaceEditingMasked(i) ? false : true;
            StringUtils::format(buffer, sizeof(buffer), "##face[%d].visible", i);
            if (ImGui::Checkbox(buffer, &visible)) {
                m_activeModel->setFaceEditingMasked(i, visible ? false : true);
            }
            ImGui::SameLine();
            StringUtils::format(buffer, sizeof(buffer), "%d (%d, %d, %d)##face[%d].name", i + 1, vertexIndex0,
                vertexIndex1, vertexIndex2, i);
            if (ImGui::Selectable(buffer, selected)) {
                const ImGuiIO &io = ImGui::GetIO();
                if (io.KeyCtrl) {
                    selected ? selection->removeFace(face) : selection->addFace(face);
                }
                else {
                    if (io.KeyShift) {
                        const nanoem_rsize_t offset = i, from = glm::min(offset, m_faceIndex),
                                             to = glm::max(offset, m_faceIndex);
                        for (nanoem_rsize_t j = from; j < to; j++) {
                            const nanoem_rsize_t offset = nanoem_rsize_t(j) * 3;
                            const nanoem_u32_t childVertexIndex0 = vertexIndices[offset + 0],
                                               childVertexIndex1 = vertexIndices[offset + 1],
                                               childVertexIndex2 = vertexIndices[offset + 2];
                            const Vector4UI32 childFace(j, childVertexIndex0, childVertexIndex1, childVertexIndex2);
                            selection->addFace(childFace);
                        }
                    }
                    else {
                        selection->removeAllFaces();
                    }
                    m_faceIndex = i;
                    selection->addFace(face);
                }
            }
            else if ((up || down) && m_faceIndex == static_cast<nanoem_rsize_t>(i)) {
                ImGui::SetScrollHereY();
                m_faceIndex = i;
            }
            if (selected) {
                ImGui::PopStyleColor();
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
    if (numFaces > 0 && m_faceIndex < numFaces) {
        const nanoem_rsize_t offset = m_faceIndex * 3;
        const nanoem_u32_t vertexIndex0 = vertexIndices[offset + 0], vertexIndex1 = vertexIndices[offset + 1],
                           vertexIndex2 = vertexIndices[offset + 2];
        const Vector3UI32 face(vertexIndex0, vertexIndex1, vertexIndex2);
        layoutFacePropertyPane(face);
    }
    ImGui::EndChild();
}

void
ModelParameterDialog::layoutFacePropertyPane(const Vector3UI32 &face)
{
    char label[Inline::kNameStackBufferSize];
    nanoem_rsize_t numVertices;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(m_activeModel->data(), &numVertices);
    nanoem_u32_t vertexIndex = face[0];
    if (vertexIndex < numVertices) {
        const nanoem_model_vertex_t *vertexPtr = vertices[vertexIndex];
        const model::Vertex *vertex = model::Vertex::cast(vertexPtr);
        const nanoem_model_material_t *materialPtr = vertex->material();
        StringUtils::format(label, sizeof(label), "%s##material", ImGuiWindow::kFALink);
        if (ImGuiWindow::handleButton(label, 0, materialPtr != nullptr)) {
            m_explicitTabType = kTabTypeMaterial;
            m_materialIndex = nanoemModelObjectGetIndex(nanoemModelMaterialGetModelObject(materialPtr));
        }
        ImGui::SameLine();
        const model::Material *material = model::Material::cast(materialPtr);
        ImGui::TextUnformatted(material ? material->nameConstString() : "(none)");
    }
    addSeparator();
    for (int i = 0; i < 3; i++) {
        nanoem_u32_t vertexIndex = face[i];
        if (vertexIndex < numVertices) {
            const nanoem_model_vertex_t *vertexPtr = vertices[vertexIndex];
            StringUtils::format(label, sizeof(label), "%s##vertex[%d]", ImGuiWindow::kFALink, i);
            int vertexIndex = nanoemModelObjectGetIndex(nanoemModelVertexGetModelObject(vertexPtr));
            if (ImGuiWindow::handleButton(label, 0, vertexPtr != nullptr)) {
                m_explicitTabType = kTabTypeVertex;
                m_vertexIndex = nanoem_rsize_t(vertexIndex);
            }
            ImGui::SameLine();
            StringUtils::format(label, sizeof(label), "Vertex%d", vertexIndex);
            ImGui::TextUnformatted(label);
        }
    }
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
    IModelObjectSelection *selection = m_activeModel->selection();
    if (ImGui::BeginPopup("material-op-menu")) {
        if (ImGui::BeginMenu("Selection")) {
            if (ImGui::MenuItem("Select All Materials")) {
                for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
                    const nanoem_model_material_t *materialPtr = materials[i];
                    selection->addMaterial(materialPtr);
                }
            }
            if (ImGui::MenuItem("Deselect All Materials")) {
                selection->removeAllMaterials();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Select All Bones from All Selected Materials")) {
                nanoem_rsize_t numVertexIndices, numVertices;
                const nanoem_u32_t *vertexIndices =
                    nanoemModelGetAllVertexIndices(m_activeModel->data(), &numVertexIndices);
                nanoem_model_vertex_t *const *vertices =
                    nanoemModelGetAllVertexObjects(m_activeModel->data(), &numVertices);
                typedef tinystl::unordered_map<const nanoem_model_material_t *, nanoem_rsize_t, TinySTLAllocator>
                    IndicesMap;
                IndicesMap indices;
                for (nanoem_rsize_t i = 0, offset = 0; i < numMaterials; i++) {
                    const nanoem_model_material_t *materialPtr = materials[i];
                    const nanoem_rsize_t numIndices = nanoemModelMaterialGetNumVertexIndices(materialPtr);
                    indices.insert(tinystl::make_pair(materialPtr, offset));
                    offset += numIndices;
                }
                const model::Material::Set materials(selection->allMaterialSet());
                for (model::Material::Set::const_iterator it = materials.begin(), end = materials.end(); it != end;
                     ++it) {
                    const nanoem_model_material_t *materialPtr = *it;
                    IndicesMap::const_iterator it2 = indices.find(materialPtr);
                    if (it2 != indices.end()) {
                        for (nanoem_rsize_t i = it2->second,
                                            to = i + nanoemModelMaterialGetNumVertexIndices(materialPtr);
                             i < to; i++) {
                            const nanoem_model_vertex_t *vertexPtr = vertices[vertexIndices[i]];
                            selection->addBone(nanoemModelVertexGetBoneObject(vertexPtr, 0));
                            selection->addBone(nanoemModelVertexGetBoneObject(vertexPtr, 1));
                            selection->addBone(nanoemModelVertexGetBoneObject(vertexPtr, 2));
                            selection->addBone(nanoemModelVertexGetBoneObject(vertexPtr, 3));
                        }
                    }
                }
            }
            if (ImGui::MenuItem("Select All Faces from All Selected Materials")) {
                nanoem_rsize_t numVertexIndices;
                const nanoem_u32_t *vertexIndices =
                    nanoemModelGetAllVertexIndices(m_activeModel->data(), &numVertexIndices);
                typedef tinystl::unordered_map<const nanoem_model_material_t *, nanoem_rsize_t, TinySTLAllocator>
                    IndicesMap;
                IndicesMap indices;
                for (nanoem_rsize_t i = 0, offset = 0; i < numMaterials; i++) {
                    const nanoem_model_material_t *materialPtr = materials[i];
                    const nanoem_rsize_t numIndices = nanoemModelMaterialGetNumVertexIndices(materialPtr);
                    indices.insert(tinystl::make_pair(materialPtr, offset));
                    offset += numIndices;
                }
                const model::Material::Set materials(selection->allMaterialSet());
                for (model::Material::Set::const_iterator it = materials.begin(), end = materials.end(); it != end;
                     ++it) {
                    const nanoem_model_material_t *materialPtr = *it;
                    IndicesMap::const_iterator it2 = indices.find(materialPtr);
                    if (it2 != indices.end()) {
                        for (nanoem_rsize_t i = it2->second,
                                            to = i + nanoemModelMaterialGetNumVertexIndices(materialPtr);
                             i < to; i += 3) {
                            const Vector4UI32 face(i, vertexIndices[i], vertexIndices[i + 1], vertexIndices[i + 2]);
                            selection->addFace(face);
                        }
                    }
                }
            }
            if (ImGui::MenuItem("Select All Vertices from All Selected Materials")) {
                nanoem_rsize_t numVertexIndices, numVertices;
                const nanoem_u32_t *vertexIndices =
                    nanoemModelGetAllVertexIndices(m_activeModel->data(), &numVertexIndices);
                nanoem_model_vertex_t *const *vertices =
                    nanoemModelGetAllVertexObjects(m_activeModel->data(), &numVertices);
                typedef tinystl::unordered_map<const nanoem_model_material_t *, nanoem_rsize_t, TinySTLAllocator>
                    IndicesMap;
                IndicesMap indices;
                for (nanoem_rsize_t i = 0, offset = 0; i < numMaterials; i++) {
                    const nanoem_model_material_t *materialPtr = materials[i];
                    const nanoem_rsize_t numIndices = nanoemModelMaterialGetNumVertexIndices(materialPtr);
                    indices.insert(tinystl::make_pair(materialPtr, offset));
                    offset += numIndices;
                }
                const model::Material::Set materials(selection->allMaterialSet());
                for (model::Material::Set::const_iterator it = materials.begin(), end = materials.end(); it != end;
                     ++it) {
                    const nanoem_model_material_t *materialPtr = *it;
                    IndicesMap::const_iterator it2 = indices.find(materialPtr);
                    if (it2 != indices.end()) {
                        for (nanoem_rsize_t i = it2->second,
                                            to = i + nanoemModelMaterialGetNumVertexIndices(materialPtr);
                             i < to; i++) {
                            const nanoem_model_vertex_t *vertexPtr = vertices[vertexIndices[i]];
                            selection->addVertex(vertexPtr);
                        }
                    }
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Masking")) {
            if (ImGui::MenuItem("Mask All Selected Materials")) {
                const model::Material::Set materials(selection->allMaterialSet());
                for (model::Material::Set::const_iterator it = materials.begin(), end = materials.end(); it != end;
                     ++it) {
                    const nanoem_model_material_t *materialPtr = *it;
                    if (model::Material *material = model::Material::cast(materialPtr)) {
                        material->setVisible(true);
                    }
                }
            }
            if (ImGui::MenuItem("Unmask All Selected Materials")) {
                const model::Material::Set materials(selection->allMaterialSet());
                for (model::Material::Set::const_iterator it = materials.begin(), end = materials.end(); it != end;
                     ++it) {
                    const nanoem_model_material_t *materialPtr = *it;
                    if (model::Material *material = model::Material::cast(materialPtr)) {
                        material->setVisible(false);
                    }
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Invert All Mask/Unmask Materials")) {
                for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
                    const nanoem_model_material_t *materialPtr = materials[i];
                    if (model::Material *material = model::Material::cast(materialPtr)) {
                        material->setVisible(!material->isVisible());
                    }
                }
            }
            ImGui::EndMenu();
        }
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
            if (selected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWindow::kColorSelectedModelObject);
            }
            if (ImGui::Selectable(buffer, selected)) {
                ImGui::SetScrollHereY();
                const ImGuiIO &io = ImGui::GetIO();
                if (io.KeyCtrl) {
                    selected ? selection->removeMaterial(materialPtr) : selection->addMaterial(materialPtr);
                }
                else {
                    if (io.KeyShift) {
                        const nanoem_rsize_t offset = i, from = glm::min(offset, m_materialIndex),
                                             to = glm::max(offset, m_materialIndex);
                        for (nanoem_rsize_t j = from; j < to; j++) {
                            selection->addMaterial(materials[j]);
                        }
                    }
                    else {
                        selection->removeAllMaterials();
                    }
                    m_materialIndex = i;
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
            if (selected) {
                ImGui::PopStyleColor();
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
        undo_command_t *command = command::DeleteMaterialCommand::create(project, materials, m_materialIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowUp), 0, m_materialIndex > 0)) {
        undo_command_t *command = command::MoveMaterialUpCommand::create(project, materials, m_materialIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowDown), 0,
            numMaterials > 0 && m_materialIndex < numMaterials)) {
        undo_command_t *command = command::MoveMaterialDownCommand::create(project, materials, m_materialIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
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
        command::ScopedMutableMaterial scoped(materialPtr);
        nanoemMutableModelMaterialSetName(scoped, scope.value(), language, &status);
    }
    addSeparator();
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.material.ambient.color"));
        Vector4 value(glm::make_vec4(nanoemModelMaterialGetAmbientColor(materialPtr)));
        if (ImGui::ColorEdit3("##ambient", glm::value_ptr(value))) {
            command::ScopedMutableMaterial scoped(materialPtr);
            nanoemMutableModelMaterialSetAmbientColor(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.material.diffuse.color"));
        Vector4 value(glm::make_vec3(nanoemModelMaterialGetDiffuseColor(materialPtr)),
            nanoemModelMaterialGetDiffuseOpacity(materialPtr));
        if (ImGui::ColorEdit4("##diffuse", glm::value_ptr(value))) {
            command::ScopedMutableMaterial scoped(materialPtr);
            nanoemMutableModelMaterialSetDiffuseColor(scoped, glm::value_ptr(value));
            nanoemMutableModelMaterialSetDiffuseOpacity(scoped, value.w);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.material.specular.color"));
        Vector4 value(glm::make_vec4(nanoemModelMaterialGetSpecularColor(materialPtr)));
        if (ImGui::ColorEdit3("##specular", glm::value_ptr(value))) {
            command::ScopedMutableMaterial scoped(materialPtr);
            nanoemMutableModelMaterialSetSpecularColor(scoped, glm::value_ptr(value));
        }
    }
    addSeparator();
    {
        ImGui::TextUnformatted("Edge");
        Vector4 value(glm::make_vec3(nanoemModelMaterialGetEdgeColor(materialPtr)),
            nanoemModelMaterialGetEdgeOpacity(materialPtr));
        if (ImGui::ColorEdit4("##edge", glm::value_ptr(value))) {
            command::ScopedMutableMaterial scoped(materialPtr);
            nanoemMutableModelMaterialSetEdgeColor(scoped, glm::value_ptr(value));
            nanoemMutableModelMaterialSetEdgeOpacity(scoped, value.w);
        }
    }
    {
        ImGui::TextUnformatted("Primitive Type");
        if (ImGui::BeginCombo("##primitive", selectedMaterialPrimitiveType(materialPtr))) {
            if (ImGui::Selectable(tr("nanoem.gui.model.edit.material.primitive.triangle"))) {
                command::ScopedMutableMaterial scoped(materialPtr);
                nanoemMutableModelMaterialSetLineDrawEnabled(scoped, false);
                nanoemMutableModelMaterialSetPointDrawEnabled(scoped, false);
            }
            if (ImGui::Selectable(tr("nanoem.gui.model.edit.material.primitive.line"))) {
                command::ScopedMutableMaterial scoped(materialPtr);
                nanoemMutableModelMaterialSetLineDrawEnabled(scoped, true);
                nanoemMutableModelMaterialSetPointDrawEnabled(scoped, false);
            }
            if (ImGui::Selectable(tr("nanoem.gui.model.edit.material.primitive.point"))) {
                command::ScopedMutableMaterial scoped(materialPtr);
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
                    command::ScopedMutableMaterial scoped(materialPtr);
                    nanoemMutableModelMaterialSetSphereMapTextureType(scoped, index);
                }
            }
            ImGui::EndCombo();
        }
    }
    addSeparator();
    char buffer[Inline::kNameStackBufferSize];
    StringUtils::format(buffer, sizeof(buffer), "%s##properties", tr("nanoem.gui.model.edit.material.properties"));
    if (ImGui::CollapsingHeader(buffer)) {
        {
            bool value = nanoemModelMaterialIsToonShared(materialPtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.material.property.shared-toon"), &value)) {
                command::ScopedMutableMaterial scoped(materialPtr);
                nanoemMutableModelMaterialSetToonShared(scoped, value);
            }
        }
        {
            bool value = nanoemModelMaterialIsEdgeEnabled(materialPtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.material.property.edge"), &value)) {
                command::ScopedMutableMaterial scoped(materialPtr);
                nanoemMutableModelMaterialSetEdgeEnabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelMaterialIsCullingDisabled(materialPtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.material.property.disable-culling"), &value)) {
                command::ScopedMutableMaterial scoped(materialPtr);
                nanoemMutableModelMaterialSetCullingDisabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelMaterialIsShadowMapEnabled(materialPtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.material.property.self-shadow"), &value)) {
                command::ScopedMutableMaterial scoped(materialPtr);
                nanoemMutableModelMaterialSetShadowMapEnabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelMaterialIsCastingShadowEnabled(materialPtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.material.property.casting-shadow"), &value)) {
                command::ScopedMutableMaterial scoped(materialPtr);
                nanoemMutableModelMaterialSetCastingShadowEnabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelMaterialIsCastingShadowMapEnabled(materialPtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.material.property.casting-self-shadow"), &value)) {
                command::ScopedMutableMaterial scoped(materialPtr);
                nanoemMutableModelMaterialSetCastingShadowMapEnabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelMaterialIsVertexColorEnabled(materialPtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.material.property.vertex-color"), &value)) {
                command::ScopedMutableMaterial scoped(materialPtr);
                nanoemMutableModelMaterialSetVertexColorEnabled(scoped, value);
            }
        }
    }
    model::Material *material = model::Material::cast(materialPtr);
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    const IImageView *diffuseImage = material->diffuseImage(), *sphereMapImage = material->sphereMapImage(),
                     *toonImage = material->toonImage();
    bool hasOwnToonTexture = toonImage && !nanoemModelMaterialIsToonShared(materialPtr);
    if (ImGui::Button(reinterpret_cast<const char *>(ImGuiWindow::kFACogs))) {
        ImGui::OpenPopup("material.texture.menu");
    }
    if (ImGui::BeginPopupContextItem("material.texture.menu")) {
        IEventPublisher *eventPublisher = project->eventPublisher();
        IFileManager *fileManager = m_applicationPtr->fileManager();
        struct BaseSetTextureCallback : IFileManager::QueryFileDialogCallbacks {
            struct BaseUserData {
                BaseUserData(nanoem_model_material_t *material, Model *model)
                    : m_model(model)
                    , m_material(material)
                {
                }
                virtual void upload(const URI &fileURI, BaseUserData *userData,
                    nanoem_mutable_model_material_t *material, nanoem_mutable_model_texture_t *texture,
                    Error &error) = 0;
                Model *m_model;
                nanoem_model_material_t *m_material;
            };
            static void
            accept(const URI &fileURI, Project * /* project */, void *opaque)
            {
                BaseUserData *userData = static_cast<BaseUserData *>(opaque);
                Model *model = userData->m_model;
                const String filePath(FileUtils::relativePath(fileURI.absolutePath(), model->fileURI().absolutePath()));
                nanoem_unicode_string_factory_t *factory = model->project()->unicodeStringFactory();
                StringUtils::UnicodeStringScope scope(factory);
                if (StringUtils::tryGetString(factory, filePath, scope)) {
                    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                    nanoem_mutable_model_material_t *material =
                        nanoemMutableModelMaterialCreateAsReference(userData->m_material, &status);
                    nanoem_mutable_model_texture_t *texture = nanoemMutableModelTextureCreate(model->data(), &status);
                    nanoemMutableModelTextureSetPath(texture, scope.value(), &status);
                    Error error;
                    userData->upload(fileURI, userData, material, texture, error);
                    nanoemMutableModelTextureDestroy(texture);
                    nanoemMutableModelMaterialDestroy(material);
                }
            }
            static void
            destroy(void *opaque)
            {
                BaseUserData *userData = static_cast<BaseUserData *>(opaque);
                nanoem_delete(userData);
            }
            BaseSetTextureCallback()
            {
                m_accept = accept;
                m_destory = destroy;
                m_cancel = nullptr;
            }
            void
            open(IFileManager *fileManager, IEventPublisher *eventPublisher, nanoem_model_material_t *material,
                Model *model)
            {
                StringList extensions;
                extensions.push_back("png");
                extensions.push_back("jpg");
                extensions.push_back("bmp");
                extensions.push_back("tga");
                m_opaque = createUserData(material, model);
                fileManager->setTransientQueryFileDialogCallback(*this);
                eventPublisher->publishQueryOpenSingleFileDialogEvent(
                    IFileManager::kDialogTypeUserCallback, extensions);
            }
            virtual BaseUserData *createUserData(nanoem_model_material_t *material, Model *model) = 0;
        };
        if (ImGui::MenuItem("Set Diffuse Texture") && !fileManager->hasTransientQueryFileDialogCallback()) {
            struct SetDiffuseTextureCallback : BaseSetTextureCallback {
                struct PrivateUserData : BaseUserData {
                    PrivateUserData(nanoem_model_material_t *material, Model *model)
                        : BaseUserData(material, model)
                    {
                    }
                    void
                    upload(const URI &fileURI, BaseUserData *userData, nanoem_mutable_model_material_t *material,
                        nanoem_mutable_model_texture_t *texture, Error &error) NANOEM_DECL_OVERRIDE
                    {
                        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                        nanoemMutableModelMaterialSetDiffuseTextureObject(material, texture, &status);
                        userData->m_model->uploadDiffuseImage(userData->m_material, fileURI, error);
                    }
                };
                BaseUserData *
                createUserData(nanoem_model_material_t *material, Model *model) NANOEM_DECL_OVERRIDE
                {
                    return nanoem_new(PrivateUserData(material, model));
                }
            };
            SetDiffuseTextureCallback callbacks;
            callbacks.open(fileManager, eventPublisher, materialPtr, m_activeModel);
        }
        if (ImGui::MenuItem("Set SphereMap Texture") && !fileManager->hasTransientQueryFileDialogCallback()) {
            struct SetSphereMapTextureCallback : BaseSetTextureCallback {
                struct PrivateUserData : BaseUserData {
                    PrivateUserData(nanoem_model_material_t *material, Model *model)
                        : BaseUserData(material, model)
                    {
                    }
                    void
                    upload(const URI &fileURI, BaseUserData *userData, nanoem_mutable_model_material_t *material,
                        nanoem_mutable_model_texture_t *texture, Error &error) NANOEM_DECL_OVERRIDE
                    {
                        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                        nanoemMutableModelMaterialSetSphereMapTextureType(
                            material, NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_MULTIPLY);
                        nanoemMutableModelMaterialSetSphereMapTextureObject(material, texture, &status);
                        userData->m_model->uploadSphereMapImage(userData->m_material, fileURI, error);
                    }
                };
                BaseUserData *
                createUserData(nanoem_model_material_t *material, Model *model) NANOEM_DECL_OVERRIDE
                {
                    return nanoem_new(PrivateUserData(material, model));
                }
            };
            SetSphereMapTextureCallback callbacks;
            callbacks.open(fileManager, eventPublisher, materialPtr, m_activeModel);
        }
        if (ImGui::MenuItem("Set Toon Texture") && !fileManager->hasTransientQueryFileDialogCallback()) {
            struct SetToonTextureCallback : BaseSetTextureCallback {
                struct PrivateUserData : BaseUserData {
                    PrivateUserData(nanoem_model_material_t *material, Model *model)
                        : BaseUserData(material, model)
                    {
                    }
                    void
                    upload(const URI &fileURI, BaseUserData *userData, nanoem_mutable_model_material_t *material,
                        nanoem_mutable_model_texture_t *texture, Error &error) NANOEM_DECL_OVERRIDE
                    {
                        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                        nanoemMutableModelMaterialSetToonShared(material, false);
                        nanoemMutableModelMaterialSetToonTextureObject(material, texture, &status);
                        userData->m_model->uploadToonImage(userData->m_material, fileURI, error);
                    }
                };
                BaseUserData *
                createUserData(nanoem_model_material_t *material, Model *model) NANOEM_DECL_OVERRIDE
                {
                    return nanoem_new(PrivateUserData(material, model));
                }
            };
            SetToonTextureCallback callbacks;
            callbacks.open(fileManager, eventPublisher, materialPtr, m_activeModel);
        }
        if (diffuseImage || sphereMapImage || hasOwnToonTexture) {
            ImGui::Separator();
        }
        if (diffuseImage && ImGui::MenuItem("Clear Diffuse Texture")) {
            // TODO: implement
        }
        if (sphereMapImage && ImGui::MenuItem("Clear SphereMap Texture")) {
            // TODO: implement
        }
        if (hasOwnToonTexture && ImGui::MenuItem("Clear Toon Texture")) {
            // TODO: implement
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    ImGui::TextUnformatted("Textures");
    if (diffuseImage || hasOwnToonTexture || sphereMapImage) {
        String filename;
        if (diffuseImage) {
            StringUtils::getUtf8String(
                nanoemModelTextureGetPath(nanoemModelMaterialGetDiffuseTextureObject(materialPtr)), factory, filename);
            layoutMaterialDiffuseImage(diffuseImage, filename, materialPtr);
        }
        if (sphereMapImage) {
            StringUtils::getUtf8String(
                nanoemModelTextureGetPath(nanoemModelMaterialGetSphereMapTextureObject(materialPtr)), factory,
                filename);
            layoutMaterialSphereMapImage(sphereMapImage, filename, materialPtr);
        }
        if (hasOwnToonTexture) {
            StringUtils::getUtf8String(
                nanoemModelTextureGetPath(nanoemModelMaterialGetToonTextureObject(materialPtr)), factory, filename);
            layoutMaterialToonImage(toonImage, filename);
        }
    }
    addSeparator();
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
                command::ScopedMutableMaterial scoped(materialPtr);
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableModelMaterialSetClob(scoped, s.value(), &status);
            }
        }
    }
    ImGui::PopItemWidth();
}

void
ModelParameterDialog::layoutMaterialDiffuseImage(
    const IImageView *image, const String &filename, const nanoem_model_material_t *activeMaterialPtr)
{
    static const char id[] = "nanoem.gui.model.edit.material.texture.diffuse";
    char label[Inline::kLongNameStackBufferSize];
    StringUtils::format(label, sizeof(label), "%s (%s)##%s/%s", tr(id), filename.c_str(), id, filename.c_str());
    bool imageNotFound = !image->isFileExist();
    if (imageNotFound) {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0xff, 0xff, 0, 0xff));
    }
    if (ImGui::CollapsingHeader(label)) {
        model::Material *material = model::Material::cast(activeMaterialPtr);
        bool displayUVMeshEnabled = material->isDisplayDiffuseTextureUVMeshEnabled();
        if (ImGui::Button(reinterpret_cast<const char *>(ImGuiWindow::kFAExpand))) {
            m_parent->openUVEditDialog(activeMaterialPtr, m_activeModel);
        }
        ImGui::SameLine();
        if (ImGui::Checkbox("Display UV Mesh", &displayUVMeshEnabled)) {
            material->setDisplayDiffuseTextureUVMeshEnabled(displayUVMeshEnabled);
        }
        const ImVec2 offset(ImGui::GetCursorScreenPos());
        UVEditDialog::drawImage(image, 1.0f);
        if (displayUVMeshEnabled) {
            const ImVec2 size(ImGui::GetItemRectSize());
            ImDrawList *drawList = ImGui::GetWindowDrawList();
            drawList->PushClipRect(offset, ImVec2(offset.x + size.x, offset.y + size.y), true);
            UVEditDialog::drawDiffuseImageUVMesh(
                drawList, offset, size, activeMaterialPtr, m_activeModel, nullptr, false);
            drawList->PopClipRect();
        }
    }
    if (imageNotFound) {
        ImGui::PopStyleColor();
    }
}

void
ModelParameterDialog::layoutMaterialSphereMapImage(
    const IImageView *image, const String &filename, const nanoem_model_material_t *activeMaterialPtr)
{
    static const char id[] = "nanoem.gui.model.edit.material.texture.sphere";
    char label[Inline::kLongNameStackBufferSize];
    StringUtils::format(label, sizeof(label), "%s (%s)##%s/%s", tr(id), filename.c_str(), id, filename.c_str());
    bool imageNotFound = !image->isFileExist();
    if (imageNotFound) {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0xff, 0xff, 0, 0xff));
    }
    if (ImGui::CollapsingHeader(label)) {
        model::Material *material = model::Material::cast(activeMaterialPtr);
        bool displayUVMeshEnabled = material->isDisplaySphereMapTextureUVMeshEnabled();
        if (ImGui::Checkbox("Display UV Mesh", &displayUVMeshEnabled)) {
            material->setDisplaySphereMapTextureUVMeshEnabled(displayUVMeshEnabled);
        }
        const ImVec2 offset(ImGui::GetCursorScreenPos());
        UVEditDialog::drawImage(image, 1.0f);
        if (displayUVMeshEnabled) {
            UVEditDialog::drawSphereMapImageUVMesh(offset, activeMaterialPtr, m_activeModel);
        }
    }
    if (imageNotFound) {
        ImGui::PopStyleColor();
    }
}

void
ModelParameterDialog::layoutMaterialToonImage(const IImageView *image, const String &filename)
{
    static const char id[] = "nanoem.gui.model.edit.material.texture.toon";
    char label[Inline::kLongNameStackBufferSize];
    StringUtils::format(label, sizeof(label), "%s (%s)##%s/%s", tr(id), filename.c_str(), id, filename.c_str());
    bool imageNotFound = !image->isFileExist();
    if (imageNotFound) {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0xff, 0xff, 0, 0xff));
    }
    if (ImGui::CollapsingHeader(label)) {
        const ImTextureID textureID = reinterpret_cast<ImTextureID>(image->handle().id);
        ImGui::Image(textureID, calcExpandedImageSize(image->description(), 1.0f), ImVec2(0, 0), ImVec2(1, 1),
            ImVec4(1, 1, 1, 1), ImGui::ColorConvertU32ToFloat4(ImGuiWindow::kColorBorder));
    }
    if (imageNotFound) {
        ImGui::PopStyleColor();
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
#if 0
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
#endif
    nanoem_model_bone_t *hoveredBonePtr = nullptr;
    nanoem_f32_t width = ImGuiWindow::kLeftPaneWidth * project->windowDevicePixelRatio();
    ImGui::BeginChild("left-pane", ImVec2(width, 0), false);
    IModelObjectSelection *selection = m_activeModel->selection();
    if (ImGui::Button(reinterpret_cast<const char *>(ImGuiWindow::kFACogs))) {
        ImGui::OpenPopup("bone-op-menu");
    }
    if (ImGui::BeginPopup("bone-op-menu")) {
        if (ImGui::BeginMenu("Selection")) {
            if (ImGui::MenuItem("Select All Bones")) {
                for (nanoem_rsize_t i = 0; i < numBones; i++) {
                    const nanoem_model_bone_t *bonePtr = bones[i];
                    selection->addBone(bonePtr);
                }
            }
            if (ImGui::MenuItem("Deselect All Bones")) {
                selection->removeAllBones();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Masking")) {
            if (ImGui::MenuItem("Mask All Selected Bones")) {
                const model::Bone::Set bones(selection->allBoneSet());
                for (model::Bone::Set::const_iterator it = bones.begin(), end = bones.end(); it != end; ++it) {
                    const nanoem_model_bone_t *bonePtr = *it;
                    if (model::Bone *bone = model::Bone::cast(bonePtr)) {
                        bone->setEditingMasked(true);
                    }
                }
            }
            if (ImGui::MenuItem("Unmask All Selected Bones")) {
                const model::Bone::Set bones(selection->allBoneSet());
                for (model::Bone::Set::const_iterator it = bones.begin(), end = bones.end(); it != end; ++it) {
                    const nanoem_model_bone_t *bonePtr = *it;
                    if (model::Bone *bone = model::Bone::cast(bonePtr)) {
                        bone->setEditingMasked(false);
                    }
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Mask All Invisible Bones")) {
                for (nanoem_rsize_t i = 0; i < numBones; i++) {
                    const nanoem_model_bone_t *bonePtr = bones[i];
                    model::Bone *bone = model::Bone::cast(bonePtr);
                    if (bone &&
                        !(m_activeModel->isBoneSelectable(bonePtr) && m_activeModel->isRigidBodyBound(bonePtr))) {
                        bone->setEditingMasked(true);
                    }
                }
            }
            if (ImGui::MenuItem("Invert All Mask/Unmask Bones")) {
                for (nanoem_rsize_t i = 0; i < numBones; i++) {
                    const nanoem_model_bone_t *bonePtr = bones[i];
                    if (model::Bone *bone = model::Bone::cast(bonePtr)) {
                        bone->setEditingMasked(!bone->isEditingMasked());
                    }
                }
            }
            ImGui::EndMenu();
        }
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
    clipper.Begin(Inline::saturateInt32(numBones));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            const nanoem_model_bone_t *bonePtr = bones[i];
            model::Bone *bone = model::Bone::cast(bonePtr);
            StringUtils::format(buffer, sizeof(buffer), "##bone[%d].visible", i);
            bool visible = bone->isEditingMasked() ? false : true;
            if (ImGui::Checkbox(buffer, &visible)) {
                bone->setEditingMasked(visible ? false : true);
            }
            ImGui::SameLine();
            const bool selected = selection->containsBone(bonePtr);
            StringUtils::format(buffer, sizeof(buffer), "%s##bone[%d].name", bone->nameConstString(), i);
            if (selected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWindow::kColorSelectedModelObject);
            }
            if (ImGui::Selectable(buffer, selected)) {
                ImGui::SetScrollHereY();
                const ImGuiIO &io = ImGui::GetIO();
                if (io.KeyCtrl) {
                    selected ? selection->removeBone(bonePtr) : selection->addBone(bonePtr);
                }
                else {
                    if (io.KeyShift) {
                        const nanoem_rsize_t offset = i, from = glm::min(offset, m_boneIndex),
                                             to = glm::max(offset, m_boneIndex);
                        for (nanoem_rsize_t j = from; j < to; j++) {
                            selection->addBone(bones[j]);
                        }
                    }
                    else {
                        selection->removeAllBones();
                    }
                    m_boneIndex = i;
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
            if (selected) {
                ImGui::PopStyleColor();
            }
        }
    }
    selection->setHoveredBone(hoveredBonePtr);
    ImGui::EndChild();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAPlus), 0, true)) {
        ImGui::OpenPopup("bone-create-menu");
    }
    if (ImGui::BeginPopup("bone-create-menu")) {
        const nanoem_model_bone_t *selectedBone = numBones > 0 ? bones[m_boneIndex] : nullptr;
        int selectedBoneIndex = model::Bone::index(selectedBone);
        if (ImGui::BeginMenu("Insert New")) {
            if (ImGui::MenuItem("at Last")) {
                undo_command_t *command = command::CreateBoneCommand::create(project, numBones, -1, nullptr);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            if (ImGui::MenuItem("after Selected", nullptr, nullptr, m_boneIndex < numBones)) {
                undo_command_t *command = command::CreateBoneCommand::create(project, numBones, selectedBoneIndex + 1, nullptr);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Insert Copy", m_boneIndex < numBones)) {
            if (ImGui::MenuItem("at Last")) {
                undo_command_t *command = command::CreateBoneCommand::create(project, numBones, -1, selectedBone);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            if (ImGui::MenuItem("at Next")) {
                undo_command_t *command = command::CreateBoneCommand::create(project, numBones, selectedBoneIndex + 1, selectedBone);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAMinus), 0, m_boneIndex < numBones)) {
        undo_command_t *command = command::DeleteBoneCommand::create(project, bones, m_boneIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowUp), 0, m_boneIndex > 0)) {
        undo_command_t *command = command::MoveBoneUpCommand::create(project, bones, m_boneIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(
            reinterpret_cast<const char *>(ImGuiWindow::kFAArrowDown), 0, numBones > 0 && m_boneIndex < numBones - 1)) {
        undo_command_t *command = command::MoveBoneDownCommand::create(project, bones, m_boneIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
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
        command::ScopedMutableBone scoped(bonePtr);
        nanoemMutableModelBoneSetName(scoped, scope.value(), language, &status);
    }
    addSeparator();
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.bone.origin"));
        Vector4 value(glm::make_vec4(nanoemModelBoneGetOrigin(bonePtr)));
        if (ImGui::InputFloat3("##origin", glm::value_ptr(value))) {
            command::ScopedMutableBone scoped(bonePtr);
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
                    command::ScopedMutableBone scoped(bonePtr);
                    nanoemMutableModelBoneSetParentBoneObject(scoped, candidateBonePtr);
                }
            }
            ImGui::EndCombo();
        }
    }
    {
        bool value = nanoemModelBoneHasDestinationBone(bonePtr) != 0;
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.bone.target"), value)) {
            command::ScopedMutableBone scoped(bonePtr);
            nanoemMutableModelBoneSetTargetBoneObject(scoped, nanoemModelBoneGetTargetBoneObject(bonePtr));
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.bone.destination-origin"), value ? false : true)) {
            command::ScopedMutableBone scoped(bonePtr);
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
                        command::ScopedMutableBone scoped(bonePtr);
                        nanoemMutableModelBoneSetTargetBoneObject(scoped, candidateBonePtr);
                    }
                }
                ImGui::EndCombo();
            }
        }
        else {
            Vector3 origin(glm::make_vec3(nanoemModelBoneGetDestinationOrigin(bonePtr)));
            if (ImGui::DragFloat3("##destination.origin", glm::value_ptr(origin))) {
                command::ScopedMutableBone scoped(bonePtr);
                nanoemMutableModelBoneSetDestinationOrigin(scoped, glm::value_ptr(origin));
            }
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.bone.stage"));
        int stageIndex = nanoemModelBoneGetStageIndex(bonePtr);
        if (ImGui::InputInt("##stage", &stageIndex)) {
            command::ScopedMutableBone scoped(bonePtr);
            nanoemMutableModelBoneSetStageIndex(scoped, stageIndex);
        }
    }
    addSeparator();
    char buffer[Inline::kNameStackBufferSize];
    StringUtils::format(buffer, sizeof(buffer), "%s##properties", tr("nanoem.gui.model.edit.bone.properties"));
    if (ImGui::CollapsingHeader(buffer)) {
        {
            bool value = nanoemModelBoneIsVisible(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.visible"), &value)) {
                command::ScopedMutableBone scoped(bonePtr);
                nanoemMutableModelBoneSetVisible(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneIsMovable(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.movable"), &value)) {
                command::ScopedMutableBone scoped(bonePtr);
                nanoemMutableModelBoneSetMovable(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneIsRotateable(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.rotateable"), &value)) {
                command::ScopedMutableBone scoped(bonePtr);
                nanoemMutableModelBoneSetRotateable(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneIsUserHandleable(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.user-handleable"), &value)) {
                command::ScopedMutableBone scoped(bonePtr);
                nanoemMutableModelBoneSetUserHandleable(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneHasLocalInherent(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.inherent.local"), &value)) {
                command::ScopedMutableBone scoped(bonePtr);
                nanoemMutableModelBoneSetLocalInherentEnabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneIsAffectedByPhysicsSimulation(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.affected-by-physics-simulation"), &value)) {
                command::ScopedMutableBone scoped(bonePtr);
                nanoemMutableModelBoneSetAffectedByPhysicsSimulation(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneHasInherentTranslation(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.inherent.translation"), &value)) {
                command::ScopedMutableBone scoped(bonePtr);
                nanoemMutableModelBoneSetInherentTranslationEnabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneHasInherentOrientation(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.inherent.orientation"), &value)) {
                command::ScopedMutableBone scoped(bonePtr);
                nanoemMutableModelBoneSetInherentOrientationEnabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneHasFixedAxis(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.has-fixed-axis"), &value)) {
                command::ScopedMutableBone scoped(bonePtr);
                nanoemMutableModelBoneSetFixedAxisEnabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneHasLocalAxes(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.has-local-axes"), &value)) {
                command::ScopedMutableBone scoped(bonePtr);
                nanoemMutableModelBoneSetLocalAxesEnabled(scoped, value);
            }
        }
        {
            bool value = nanoemModelBoneHasConstraint(bonePtr) != 0;
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.inverse-kinematics"), &value)) {
                command::ScopedMutableBone scoped(bonePtr);
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
                            command::ScopedMutableBone scoped(bonePtr);
                            nanoemMutableModelBoneSetInherentParentBoneObject(scoped, candidateBonePtr);
                        }
                    }
                    ImGui::EndCombo();
                }
                nanoem_f32_t coefficient = nanoemModelBoneGetInherentCoefficient(bonePtr);
                ImGui::TextUnformatted(tr("nanoem.gui.model.edit.bone.inherent.coefficient"));
                if (ImGui::DragFloat("##coefficient", &coefficient)) {
                    command::ScopedMutableBone scoped(bonePtr);
                    nanoemMutableModelBoneSetInherentCoefficient(scoped, coefficient);
                }
            }
        }
        {
            if (nanoemModelBoneHasFixedAxis(bonePtr)) {
                Vector3 axis(glm::make_vec3(nanoemModelBoneGetFixedAxis(bonePtr)));
                ImGui::TextUnformatted("Fixed Axis");
                if (ImGui::DragFloat3("##axis", glm::value_ptr(axis), 1.0f, 0.0f, 1.0f)) {
                    command::ScopedMutableBone scoped(bonePtr);
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
                    command::ScopedMutableBone scoped(bonePtr);
                    nanoemMutableModelBoneSetLocalXAxis(scoped, glm::value_ptr(axisX));
                }
                ImGui::TextUnformatted(tr("nanoem.gui.model.edit.bone.local-axis.z"));
                Vector3 axisZ(glm::make_vec3(nanoemModelBoneGetLocalZAxis(bonePtr)));
                if (ImGui::DragFloat3("##axis.z", glm::value_ptr(axisZ), 1.0f, 0.0f, 1.0f)) {
                    command::ScopedMutableBone scoped(bonePtr);
                    nanoemMutableModelBoneSetLocalZAxis(scoped, glm::value_ptr(axisZ));
                }
            }
        }
    }
    layoutBoneConstraintPanel(bonePtr, project);
    if (ImGui::CollapsingHeader("Internal Parameters##properties.internal")) {
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
                            command::ScopedMutableConstraint scoped(constraintPtr);
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
                        command::ScopedMutableConstraint scoped(constraintPtr);
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
                command::ScopedMutableConstraint scoped(constraintPtr);
                nanoemMutableModelConstraintSetNumIterations(scoped, value);
            }
        }
        {
            nanoem_f32_t value = glm::degrees(nanoemModelConstraintGetAngleLimit(constraintPtr));
            StringUtils::format(buffer, sizeof(buffer), "%s: %%.3f", tr("nanoem.gui.model.edit.bone.constraint.angle"));
            if (ImGui::SliderFloat("##constraint.angle", &value, -90, 90, buffer)) {
                command::ScopedMutableConstraint scoped(constraintPtr);
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
                    manipulatable = true;
                }
            }
            if (ImGuiWindow::handleButton(tr("nanoem.gui.model.edit.bone.constraint.joints.add"),
                    ImGui::GetContentRegionAvail().x * 0.5f, manipulatable)) {
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                command::ScopedMutableConstraint scoped(constraintPtr);
                nanoem_mutable_model_constraint_joint_t *joint =
                    nanoemMutableModelConstraintJointCreate(scoped, &status);
                nanoemMutableModelConstraintJointSetBoneObject(joint, bones[m_constraintJointCandidateIndex]);
                nanoemMutableModelConstraintInsertJointObject(scoped, joint, -1, &status);
                nanoemMutableModelConstraintJointDestroy(joint);
                joints = nanoemModelConstraintGetAllJointObjects(constraintPtr, &numJoints);
                m_constraintJointCandidateIndex = NANOEM_RSIZE_MAX;
            }
            ImGui::SameLine();
            if (ImGuiWindow::handleButton(tr("nanoem.gui.model.edit.bone.constraint.joints.remove"),
                    ImGui::GetContentRegionAvail().x, m_constraintJointIndex < numJoints)) {
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                command::ScopedMutableConstraint scoped(constraintPtr);
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
            if (ImGui::Checkbox(tr("nanoem.gui.model.edit.bone.constraint.joint.angle-limit"), &value)) {
                command::ScopedMutableConstraintJoint scoped(jointPtr);
                nanoemMutableModelConstraintJointSetAngleLimitEnabled(scoped, value ? 1 : 0);
            }
        }
        {
            Vector3 lowerLimit(glm::degrees(glm::make_vec3(nanoemModelConstraintJointGetLowerLimit(jointPtr))));
            ImGui::TextUnformatted(tr("nanoem.gui.model.edit.bone.constraint.joint.angle-limit.lower"));
            if (ImGui::DragFloat3("##joint.upper", glm::value_ptr(lowerLimit), 1.0f, 0.0f, 180.0f)) {
                command::ScopedMutableConstraintJoint scoped(jointPtr);
                nanoemMutableModelConstraintJointSetLowerLimit(scoped, glm::value_ptr(glm::radians(lowerLimit)));
            }
        }
        {
            Vector3 upperLimit(glm::degrees(glm::make_vec3(nanoemModelConstraintJointGetUpperLimit(jointPtr))));
            ImGui::TextUnformatted(tr("nanoem.gui.model.edit.bone.constraint.joint.angle-limit.upper"));
            if (ImGui::DragFloat3("##joint.lower", glm::value_ptr(upperLimit), 1.0f, 0.0f, 180.0f)) {
                command::ScopedMutableConstraintJoint scoped(jointPtr);
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
    IModelObjectSelection *selection = m_activeModel->selection();
    if (ImGui::BeginPopup("morph-op-menu")) {
        if (ImGui::BeginMenu("Selection")) {
            if (ImGui::MenuItem("Select All Morphs")) {
                for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
                    const nanoem_model_morph_t *morphPtr = morphs[i];
                    selection->addMorph(morphPtr);
                }
            }
            if (ImGui::MenuItem("Deselect All Morphs")) {
                selection->removeAllMorphs();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Select All Model Objects from All Selected Morphs")) {
                const model::Morph::Set morphs(selection->allMorphSet());
                for (model::Morph::Set::const_iterator it = morphs.begin(), end = morphs.end(); it != end; ++it) {
                    const nanoem_model_morph_t *morphPtr = *it;
                    nanoem_rsize_t numItems;
                    switch (nanoemModelMorphGetType(morphPtr)) {
                    case NANOEM_MODEL_MORPH_TYPE_BONE: {
                        nanoem_model_morph_bone_t *const *items =
                            nanoemModelMorphGetAllBoneMorphObjects(morphPtr, &numItems);
                        for (nanoem_rsize_t i = 0; i < numItems; i++) {
                            selection->addBone(nanoemModelMorphBoneGetBoneObject(items[i]));
                        }
                        break;
                    }
                    case NANOEM_MODEL_MORPH_TYPE_FLIP: {
                        nanoem_model_morph_flip_t *const *items =
                            nanoemModelMorphGetAllFlipMorphObjects(morphPtr, &numItems);
                        for (nanoem_rsize_t i = 0; i < numItems; i++) {
                            selection->addMorph(nanoemModelMorphFlipGetMorphObject(items[i]));
                        }
                        break;
                    }
                    case NANOEM_MODEL_MORPH_TYPE_GROUP: {
                        nanoem_model_morph_group_t *const *items =
                            nanoemModelMorphGetAllGroupMorphObjects(morphPtr, &numItems);
                        for (nanoem_rsize_t i = 0; i < numItems; i++) {
                            selection->addMorph(nanoemModelMorphGroupGetMorphObject(items[i]));
                        }
                        break;
                    }
                    case NANOEM_MODEL_MORPH_TYPE_IMPULUSE: {
                        nanoem_model_morph_impulse_t *const *items =
                            nanoemModelMorphGetAllImpulseMorphObjects(morphPtr, &numItems);
                        for (nanoem_rsize_t i = 0; i < numItems; i++) {
                            selection->addRigidBody(nanoemModelMorphImpulseGetRigidBodyObject(items[i]));
                        }
                        break;
                    }
                    case NANOEM_MODEL_MORPH_TYPE_MATERIAL: {
                        nanoem_model_morph_material_t *const *items =
                            nanoemModelMorphGetAllMaterialMorphObjects(morphPtr, &numItems);
                        for (nanoem_rsize_t i = 0; i < numItems; i++) {
                            selection->addMaterial(nanoemModelMorphMaterialGetMaterialObject(items[i]));
                        }
                        break;
                    }
                    case NANOEM_MODEL_MORPH_TYPE_TEXTURE:
                    case NANOEM_MODEL_MORPH_TYPE_UVA1:
                    case NANOEM_MODEL_MORPH_TYPE_UVA2:
                    case NANOEM_MODEL_MORPH_TYPE_UVA3:
                    case NANOEM_MODEL_MORPH_TYPE_UVA4: {
                        nanoem_model_morph_uv_t *const *items =
                            nanoemModelMorphGetAllUVMorphObjects(morphPtr, &numItems);
                        for (nanoem_rsize_t i = 0; i < numItems; i++) {
                            selection->addVertex(nanoemModelMorphUVGetVertexObject(items[i]));
                        }
                        break;
                    }
                    case NANOEM_MODEL_MORPH_TYPE_VERTEX: {
                        nanoem_model_morph_vertex_t *const *items =
                            nanoemModelMorphGetAllVertexMorphObjects(morphPtr, &numItems);
                        for (nanoem_rsize_t i = 0; i < numItems; i++) {
                            selection->addVertex(nanoemModelMorphVertexGetVertexObject(items[i]));
                        }
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
            ImGui::EndMenu();
        }
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
    clipper.Begin(Inline::saturateInt32(numMorphs));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            const nanoem_model_morph_t *morphPtr = morphs[i];
            model::Morph *morph = model::Morph::cast(morphPtr);
            const bool selected = selection->containsMorph(morphPtr);
            StringUtils::format(buffer, sizeof(buffer), "%s##morph[%d].name", morph->nameConstString(), i);
            if (selected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWindow::kColorSelectedModelObject);
            }
            if (ImGui::Selectable(buffer, selected)) {
                ImGui::SetScrollHereY();
                const ImGuiIO &io = ImGui::GetIO();
                if (io.KeyCtrl) {
                    selected ? selection->removeMorph(morphPtr) : selection->addMorph(morphPtr);
                }
                else {
                    if (io.KeyShift) {
                        const nanoem_rsize_t offset = i, from = glm::min(offset, m_morphIndex),
                                             to = glm::max(offset, m_morphIndex);
                        for (nanoem_rsize_t j = from; j < to; j++) {
                            selection->addMorph(morphs[j]);
                        }
                    }
                    else {
                        selection->removeAllMorphs();
                    }
                    m_morphIndex = i;
                    m_morphItemIndex = 0;
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
            if (selected) {
                ImGui::PopStyleColor();
            }
        }
    }
    selection->setHoveredMorph(hoveredMorphPtr);
    ImGui::EndChild();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAPlus), 0, true)) {
        ImGui::OpenPopup("morph-create-menu");
    }
    if (ImGui::BeginPopup("morph-create-menu")) {
        const nanoem_model_morph_t *selectedMorph = numMorphs > 0 ? morphs[m_morphIndex] : nullptr;
        int selectedMorphIndex = model::Morph::index(selectedMorph);
        if (ImGui::BeginMenu("Insert New")) {
            if (ImGui::BeginMenu("Bone Morph")) {
                const nanoem_model_morph_type_t type = NANOEM_MODEL_MORPH_TYPE_BONE;
                if (ImGui::MenuItem("at Last")) {
                    undo_command_t *command = command::CreateMorphCommand::create(project, numMorphs, -1, nullptr, type);
                    m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
                }
                if (ImGui::MenuItem("at Next")) {
                    undo_command_t *command =
                        command::CreateMorphCommand::create(project, numMorphs, selectedMorphIndex + 1, nullptr, type);
                    m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Group Morph")) {
                const nanoem_model_morph_type_t type = NANOEM_MODEL_MORPH_TYPE_GROUP;
                if (ImGui::MenuItem("at Last")) {
                    undo_command_t *command = command::CreateMorphCommand::create(project, numMorphs, -1, nullptr, type);
                    m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
                }
                if (ImGui::MenuItem("at Next")) {
                    undo_command_t *command =
                        command::CreateMorphCommand::create(project, numMorphs, selectedMorphIndex + 1, nullptr, type);
                    m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Material Morph")) {
                const nanoem_model_morph_type_t type = NANOEM_MODEL_MORPH_TYPE_MATERIAL;
                if (ImGui::MenuItem("at Last")) {
                    undo_command_t *command = command::CreateMorphCommand::create(project, numMorphs, -1, nullptr, type);
                    m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
                }
                if (ImGui::MenuItem("at Next")) {
                    undo_command_t *command =
                        command::CreateMorphCommand::create(project, numMorphs, selectedMorphIndex + 1, nullptr, type);
                    m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("UV Morph")) {
                const nanoem_model_morph_type_t type = NANOEM_MODEL_MORPH_TYPE_TEXTURE;
                if (ImGui::MenuItem("at Last")) {
                    undo_command_t *command = command::CreateMorphCommand::create(project, numMorphs, -1, nullptr, type);
                    m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
                }
                if (ImGui::MenuItem("at Next")) {
                    undo_command_t *command =
                        command::CreateMorphCommand::create(project, numMorphs, selectedMorphIndex + 1, nullptr, type);
                    m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Vertex Morph")) {
                const nanoem_model_morph_type_t type = NANOEM_MODEL_MORPH_TYPE_VERTEX;
                if (ImGui::MenuItem("at Last")) {
                    undo_command_t *command = command::CreateMorphCommand::create(project, numMorphs, -1, nullptr, type);
                    m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
                }
                if (ImGui::MenuItem("at Next")) {
                    undo_command_t *command =
                        command::CreateMorphCommand::create(project, numMorphs, selectedMorphIndex + 1, nullptr, type);
                    m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Flip Morph")) {
                const nanoem_model_morph_type_t type = NANOEM_MODEL_MORPH_TYPE_FLIP;
                if (ImGui::MenuItem("at Last")) {
                    undo_command_t *command = command::CreateMorphCommand::create(project, numMorphs, -1, nullptr, type);
                    m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
                }
                if (ImGui::MenuItem("at Next")) {
                    undo_command_t *command =
                        command::CreateMorphCommand::create(project, numMorphs, selectedMorphIndex + 1, nullptr, type);
                    m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Impulse Morph")) {
                const nanoem_model_morph_type_t type = NANOEM_MODEL_MORPH_TYPE_IMPULUSE;
                if (ImGui::MenuItem("at Last")) {
                    undo_command_t *command = command::CreateMorphCommand::create(project, numMorphs, -1, nullptr, type);
                    m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
                }
                if (ImGui::MenuItem("at Next")) {
                    undo_command_t *command =
                        command::CreateMorphCommand::create(project, numMorphs, selectedMorphIndex + 1, nullptr, type);
                    m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Insert Copy")) {
            if (ImGui::MenuItem("at Last")) {
                undo_command_t *command =
                    command::CreateMorphCommand::create(project, numMorphs, -1, selectedMorph, NANOEM_MODEL_MORPH_TYPE_UNKNOWN);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            if (ImGui::MenuItem("at Next")) {
                undo_command_t *command = command::CreateMorphCommand::create(project,
                    numMorphs, selectedMorphIndex + 1, selectedMorph, NANOEM_MODEL_MORPH_TYPE_UNKNOWN);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAMinus), 0, m_morphIndex < numMorphs)) {
        undo_command_t *command = command::DeleteMorphCommand::create(project, morphs, m_morphIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowUp), 0, m_morphIndex > 0)) {
        undo_command_t *command = command::MoveMorphUpCommand::create(project, morphs, m_morphIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowDown), 0,
            numMorphs > 0 && m_morphIndex < numMorphs - 1)) {
        undo_command_t *command = command::MoveMorphDownCommand::create(project, morphs, m_morphIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
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
    nanoem_language_type_t language = static_cast<nanoem_language_type_t>(m_language);
    StringUtils::UnicodeStringScope scope(project->unicodeStringFactory());
    ImGui::PushItemWidth(-1);
    if (layoutName(nanoemModelMorphGetName(morphPtr, language), project, scope)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        command::ScopedMutableMorph scoped(morphPtr);
        nanoemMutableModelMorphSetName(scoped, scope.value(), language, &status);
    }
    addSeparator();
    {
        nanoem_model_morph_category_t value = nanoemModelMorphGetCategory(morphPtr);
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.category"));
        if (ImGui::BeginCombo("##category", selectedMorphCategory(value))) {
            /* skip base */
            for (int i = NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM + 1; i < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM; i++) {
                nanoem_model_morph_category_t type = static_cast<nanoem_model_morph_category_t>(i);
                if (ImGui::Selectable(selectedMorphCategory(type))) {
                    command::ScopedMutableMorph scoped(morphPtr);
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
                    command::ScopedMutableMorph scoped(morphPtr);
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
        switch (nanoemModelMorphGetType(morphPtr)) {
        case NANOEM_MODEL_MORPH_TYPE_BONE: {
            layoutMorphBonePropertyPane(propertyWindowSize, morphPtr, project);
            break;
        }
        case NANOEM_MODEL_MORPH_TYPE_FLIP: {
            layoutMorphFlipPropertyPane(propertyWindowSize, morphPtr, project);
            break;
        }
        case NANOEM_MODEL_MORPH_TYPE_GROUP: {
            layoutMorphGroupPropertyPane(propertyWindowSize, morphPtr, project);
            break;
        }
        case NANOEM_MODEL_MORPH_TYPE_IMPULUSE: {
            layoutMorphImpulsePropertyPane(propertyWindowSize, morphPtr, project);
            break;
        }
        case NANOEM_MODEL_MORPH_TYPE_MATERIAL: {
            layoutMorphMaterialPropertyPane(propertyWindowSize, morphPtr, project);
            break;
        }
        case NANOEM_MODEL_MORPH_TYPE_TEXTURE:
        case NANOEM_MODEL_MORPH_TYPE_UVA1:
        case NANOEM_MODEL_MORPH_TYPE_UVA2:
        case NANOEM_MODEL_MORPH_TYPE_UVA3:
        case NANOEM_MODEL_MORPH_TYPE_UVA4: {
            layoutMorphUVPropertyPane(propertyWindowSize, morphPtr, project);
            break;
        }
        case NANOEM_MODEL_MORPH_TYPE_VERTEX: {
            layoutMorphVertexPropertyPane(propertyWindowSize, morphPtr, project);
            break;
        }
        default:
            break;
        }
        ImGui::EndChild();
    }
    ImGui::PopItemWidth();
}

void
ModelParameterDialog::layoutMorphBonePropertyPane(
    const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr, Project *project)
{
    char buffer[Inline::kNameStackBufferSize];
    bool itemUp, itemDown;
    detectUpDown(itemUp, itemDown);
    model::Bone::Set reservedBoneSet;
    nanoem_rsize_t numItems;
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
                StringUtils::format(buffer, sizeof(buffer), "%s##item[%lu].name", bone->nameConstString(), i);
                if (ImGui::Selectable(buffer, selected, flags)) {
                    m_morphItemCandidateBoneIndex = i;
                }
                reservedBoneSet.insert(bonePtr);
            }
        }
        ImGui::EndCombo();
    }
    if (ImGuiWindow::handleButton(tr("nanoem.gui.model.edit.morph.bone.add"), ImGui::GetContentRegionAvail().x * 0.5f,
            m_morphItemCandidateBoneIndex < numBones)) {
        nanoem_status_t status;
        command::ScopedMutableMorph scoped(morphPtr);
        nanoem_mutable_model_morph_bone_t *item = nanoemMutableModelMorphBoneCreate(scoped, &status);
        nanoemMutableModelMorphBoneSetBoneObject(item, bones[m_morphItemCandidateBoneIndex]);
        nanoemMutableModelMorphInsertBoneMorphObject(scoped, item, -1, &status);
        nanoemMutableModelMorphBoneDestroy(item);
        items = nanoemModelMorphGetAllBoneMorphObjects(morphPtr, &numItems);
        m_morphItemCandidateBoneIndex = NANOEM_RSIZE_MAX;
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(tr("nanoem.gui.model.edit.morph.bone.remove"), ImGui::GetContentRegionAvail().x,
            m_morphItemIndex < numItems)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        command::ScopedMutableMorph scoped(morphPtr);
        nanoem_mutable_model_morph_bone_t *item =
            nanoemMutableModelMorphBoneCreateAsReference(items[m_morphItemIndex], &status);
        nanoemMutableModelMorphRemoveBoneMorphObject(scoped, item, &status);
        nanoemMutableModelMorphBoneDestroy(item);
    }
    addSeparator();
    nanoem_model_morph_bone_t *morphBonePtr = m_morphItemIndex < numItems ? items[m_morphItemIndex] : nullptr;
    {
        StringUtils::format(buffer, sizeof(buffer), "%s##bone.toggle", ImGuiWindow::kFALink);
        const nanoem_model_bone_t *bonePtr = nanoemModelMorphBoneGetBoneObject(morphBonePtr);
        if (ImGuiWindow::handleButton(buffer, 0, morphBonePtr != nullptr)) {
            m_explicitTabType = kTabTypeBone;
            m_morphIndex = nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(bonePtr));
        }
        ImGui::SameLine();
        const model::Bone *bone = model::Bone::cast(bonePtr);
        ImGui::TextUnformatted(bone ? bone->nameConstString() : "(none)");
    }
    addSeparator();
    {
        Vector4 translation(glm::make_vec4(nanoemModelMorphBoneGetTranslation(morphBonePtr)));
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.bone.translation"));
        if (ImGui::DragFloat3("##bone.translation", glm::value_ptr(translation), 1.0f, 0.0f, 0.0f)) {
            command::ScopedMutableMorphBone scoped(morphBonePtr);
            nanoemMutableModelMorphBoneSetTranslation(scoped, glm::value_ptr(translation));
        }
    }
    {
        const Quaternion orientation(glm::make_quat(nanoemModelMorphBoneGetOrientation(morphBonePtr)));
        Vector3 angles(glm::degrees(glm::eulerAngles(orientation)));
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.bone.orientation"));
        if (ImGui::DragFloat3("##bone.orientation", glm::value_ptr(angles), 1.0f, 0.0f, 0.0f)) {
            command::ScopedMutableMorphBone scoped(morphBonePtr);
            nanoemMutableModelMorphBoneSetOrientation(scoped, glm::value_ptr(glm::quat(glm::radians(angles))));
        }
    }
    ImGui::PopItemWidth();
}

void
ModelParameterDialog::layoutMorphFlipPropertyPane(
    const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr, Project *project)
{
    char buffer[Inline::kNameStackBufferSize];
    bool itemUp, itemDown;
    detectUpDown(itemUp, itemDown);
    model::Morph::Set reservedMorphSet;
    nanoem_rsize_t numItems;
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
                StringUtils::format(buffer, sizeof(buffer), "%s##item[%lu].name", morph->nameConstString(), i);
                if (ImGui::Selectable(buffer, selected, flags)) {
                    m_morphItemCandidateMorphIndex = i;
                }
                reservedMorphSet.insert(morphPtr);
            }
        }
        ImGui::EndCombo();
    }
    if (ImGuiWindow::handleButton(tr("nanoem.gui.model.edit.morph.flip.add"), ImGui::GetContentRegionAvail().x * 0.5f,
            m_morphItemCandidateMorphIndex < numMorphs)) {
        nanoem_status_t status;
        command::ScopedMutableMorph scoped(morphPtr);
        nanoem_mutable_model_morph_flip_t *item = nanoemMutableModelMorphFlipCreate(scoped, &status);
        nanoemMutableModelMorphFlipSetMorphObject(item, morphs[m_morphItemCandidateMorphIndex]);
        nanoemMutableModelMorphInsertFlipMorphObject(scoped, item, -1, &status);
        nanoemMutableModelMorphFlipDestroy(item);
        items = nanoemModelMorphGetAllFlipMorphObjects(morphPtr, &numItems);
        m_morphItemCandidateMorphIndex = NANOEM_RSIZE_MAX;
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(tr("nanoem.gui.model.edit.morph.flip.remove"), ImGui::GetContentRegionAvail().x,
            m_morphItemIndex < numItems)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        command::ScopedMutableMorph scoped(morphPtr);
        nanoem_mutable_model_morph_flip_t *item =
            nanoemMutableModelMorphFlipCreateAsReference(items[m_morphItemIndex], &status);
        nanoemMutableModelMorphRemoveFlipMorphObject(scoped, item, &status);
        nanoemMutableModelMorphFlipDestroy(item);
    }
    addSeparator();
    nanoem_model_morph_flip_t *morphFlipPtr = m_morphItemIndex < numItems ? items[m_morphItemIndex] : nullptr;
    {
        StringUtils::format(buffer, sizeof(buffer), "%s##group.toggle", ImGuiWindow::kFALink);
        const nanoem_model_morph_t *morphPtr = nanoemModelMorphFlipGetMorphObject(morphFlipPtr);
        if (ImGuiWindow::handleButton(buffer, 0, morphFlipPtr != nullptr)) {
            m_morphIndex = nanoemModelObjectGetIndex(nanoemModelMorphGetModelObject(morphPtr));
        }
        ImGui::SameLine();
        const model::Morph *morph = model::Morph::cast(morphPtr);
        ImGui::TextUnformatted(morph ? morph->nameConstString() : "(none)");
    }
    addSeparator();
    {
        nanoem_f32_t weight = nanoemModelMorphFlipGetWeight(morphFlipPtr);
        StringUtils::format(buffer, sizeof(buffer), "%s %%.3f", tr("nanoem.gui.model.edit.morph.flip.weight"));
        if (ImGui::SliderFloat("##flip.weight", &weight, 0.0f, 0.0f, buffer)) {
            command::ScopedMutableMorphFlip scoped(morphFlipPtr);
            nanoemMutableModelMorphFlipSetWeight(scoped, weight);
        }
    }
    ImGui::PopItemWidth();
}

void
ModelParameterDialog::layoutMorphGroupPropertyPane(
    const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr, Project *project)
{
    char buffer[Inline::kNameStackBufferSize];
    bool itemUp, itemDown;
    detectUpDown(itemUp, itemDown);
    model::Morph::Set reservedMorphSet;
    nanoem_rsize_t numItems;
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
                StringUtils::format(buffer, sizeof(buffer), "%s##item[%lu].name", morph->nameConstString(), i);
                if (ImGui::Selectable(buffer, selected, flags)) {
                    m_morphItemCandidateMorphIndex = i;
                }
                reservedMorphSet.insert(morphPtr);
            }
        }
        ImGui::EndCombo();
    }
    if (ImGuiWindow::handleButton(
            tr("nanoem.gui.model.edit.morph.group.add"), ImGui::GetContentRegionAvail().x * 0.5f, true)) {
        nanoem_status_t status;
        command::ScopedMutableMorph scoped(morphPtr);
        nanoem_mutable_model_morph_group_t *item = nanoemMutableModelMorphGroupCreate(scoped, &status);
        nanoemMutableModelMorphGroupSetMorphObject(item, morphs[m_morphItemCandidateMorphIndex]);
        nanoemMutableModelMorphInsertGroupMorphObject(scoped, item, -1, &status);
        nanoemMutableModelMorphGroupDestroy(item);
        items = nanoemModelMorphGetAllGroupMorphObjects(morphPtr, &numItems);
        m_morphItemCandidateMorphIndex = NANOEM_RSIZE_MAX;
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(tr("nanoem.gui.model.edit.morph.group.remove"), ImGui::GetContentRegionAvail().x,
            m_morphItemIndex < numItems)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        command::ScopedMutableMorph scoped(morphPtr);
        nanoem_mutable_model_morph_group_t *item =
            nanoemMutableModelMorphGroupCreateAsReference(items[m_morphItemIndex], &status);
        nanoemMutableModelMorphRemoveGroupMorphObject(scoped, item, &status);
        nanoemMutableModelMorphGroupDestroy(item);
    }
    addSeparator();
    nanoem_model_morph_group_t *morphGroupPtr = m_morphItemIndex < numItems ? items[m_morphItemIndex] : nullptr;
    {
        StringUtils::format(buffer, sizeof(buffer), "%s##group.toggle", ImGuiWindow::kFALink);
        const nanoem_model_morph_t *morphPtr = nanoemModelMorphGroupGetMorphObject(morphGroupPtr);
        if (ImGuiWindow::handleButton(buffer, 0, morphGroupPtr != nullptr)) {
            m_morphIndex = nanoemModelObjectGetIndex(nanoemModelMorphGetModelObject(morphPtr));
        }
        ImGui::SameLine();
        const model::Morph *morph = model::Morph::cast(morphPtr);
        ImGui::TextUnformatted(morph ? morph->nameConstString() : "(none)");
    }
    addSeparator();
    {
        nanoem_f32_t weight = nanoemModelMorphGroupGetWeight(morphGroupPtr);
        StringUtils::format(buffer, sizeof(buffer), "%s %%.3f", tr("nanoem.gui.model.edit.morph.group.weight"));
        if (ImGui::SliderFloat("##group.weight", &weight, 0.0f, 0.0f)) {
            command::ScopedMutableMorphGroup scoped(morphGroupPtr);
            nanoemMutableModelMorphGroupSetWeight(scoped, weight);
        }
    }
    ImGui::PopItemWidth();
}

void
ModelParameterDialog::layoutMorphImpulsePropertyPane(
    const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr, Project *project)
{
    char buffer[Inline::kNameStackBufferSize];
    bool itemUp, itemDown;
    detectUpDown(itemUp, itemDown);
    model::RigidBody::Set reservedRigidBodySet;
    nanoem_rsize_t numItems;
    nanoem_model_morph_impulse_t *const *items = nanoemModelMorphGetAllImpulseMorphObjects(morphPtr, &numItems);
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
        if (model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodies[m_morphItemCandidateRigidBodyIndex])) {
            name = rigidBody->nameConstString();
        }
    }
    if (ImGui::BeginCombo("##candidate", name)) {
        for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
            const nanoem_model_rigid_body_t *rigidBodyPtr = rigidBodies[i];
            if (model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr)) {
                const bool selected = static_cast<nanoem_rsize_t>(i) == m_morphItemCandidateRigidBodyIndex;
                const ImGuiSelectableFlags flags = reservedRigidBodySet.find(rigidBodyPtr) != reservedRigidBodySet.end()
                    ? ImGuiSelectableFlags_Disabled
                    : ImGuiSelectableFlags_None;
                StringUtils::format(buffer, sizeof(buffer), "%s##item[%lu].name", rigidBody->nameConstString(), i);
                if (ImGui::Selectable(buffer, selected, flags)) {
                    m_morphItemCandidateRigidBodyIndex = i;
                }
                reservedRigidBodySet.insert(rigidBodyPtr);
            }
        }
        ImGui::EndCombo();
    }
    if (ImGuiWindow::handleButton(tr("nanoem.gui.model.edit.morph.impulse.add"),
            ImGui::GetContentRegionAvail().x * 0.5f, m_morphItemCandidateRigidBodyIndex < numRigidBodies)) {
        nanoem_status_t status;
        command::ScopedMutableMorph scoped(morphPtr);
        nanoem_mutable_model_morph_impulse_t *item = nanoemMutableModelMorphImpulseCreate(scoped, &status);
        nanoemMutableModelMorphImpulseSetRigidBodyObject(item, rigidBodies[m_morphItemCandidateRigidBodyIndex]);
        nanoemMutableModelMorphInsertImpulseMorphObject(scoped, item, -1, &status);
        nanoemMutableModelMorphImpulseDestroy(item);
        items = nanoemModelMorphGetAllImpulseMorphObjects(morphPtr, &numItems);
        m_morphItemCandidateRigidBodyIndex = NANOEM_RSIZE_MAX;
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(tr("nanoem.gui.model.edit.morph.impulse.remove"), ImGui::GetContentRegionAvail().x,
            m_morphItemIndex < numItems)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        command::ScopedMutableMorph scoped(morphPtr);
        nanoem_mutable_model_morph_impulse_t *item =
            nanoemMutableModelMorphImpulseCreateAsReference(items[m_morphItemIndex], &status);
        nanoemMutableModelMorphRemoveImpulseMorphObject(scoped, item, &status);
        nanoemMutableModelMorphImpulseDestroy(item);
    }
    addSeparator();
    nanoem_model_morph_impulse_t *morphImpulsePtr = m_morphItemIndex < numItems ? items[m_morphItemIndex] : nullptr;
    {
        StringUtils::format(buffer, sizeof(buffer), "%s##impulse.toggle", ImGuiWindow::kFALink);
        const nanoem_model_rigid_body_t *rigidBodyPtr = nanoemModelMorphImpulseGetRigidBodyObject(morphImpulsePtr);
        if (ImGuiWindow::handleButton(buffer, 0, morphImpulsePtr != nullptr)) {
            m_explicitTabType = kTabTypeRigidBody;
            m_rigidBodyIndex = nanoemModelObjectGetIndex(nanoemModelRigidBodyGetModelObject(rigidBodyPtr));
        }
        ImGui::SameLine();
        const model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr);
        ImGui::TextUnformatted(rigidBody ? rigidBody->nameConstString() : "(none)");
    }
    addSeparator();
    {
        Vector4 torque(glm::make_vec4(nanoemModelMorphImpulseGetTorque(morphImpulsePtr)));
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.impulse.torque"));
        if (ImGui::DragFloat3("##impulse.torque", glm::value_ptr(torque), 1.0f, 0.0f, 0.0f)) {
            command::ScopedMutableMorphImpulse scoped(morphImpulsePtr);
            nanoemMutableModelMorphImpulseSetTorque(scoped, glm::value_ptr(torque));
        }
    }
    {
        Vector4 torque(glm::make_vec4(nanoemModelMorphImpulseGetVelocity(morphImpulsePtr)));
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.impulse.velocity"));
        if (ImGui::DragFloat3("##impulse.velocity", glm::value_ptr(torque), 1.0f, 0.0f, 0.0f)) {
            command::ScopedMutableMorphImpulse scoped(morphImpulsePtr);
            nanoemMutableModelMorphImpulseSetVelocity(scoped, glm::value_ptr(torque));
        }
    }
    {
        bool isLocal = nanoemModelMorphImpulseIsLocal(morphImpulsePtr) != 0;
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.impulse.local"));
        if (ImGui::Checkbox("##impuluse.local", &isLocal)) {
            command::ScopedMutableMorphImpulse scoped(morphImpulsePtr);
            nanoemMutableModelMorphImpulseSetLocal(scoped, isLocal ? 1 : 0);
        }
    }
    ImGui::PopItemWidth();
}

void
ModelParameterDialog::layoutMorphMaterialPropertyPane(
    const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr, Project *project)
{
    char buffer[Inline::kNameStackBufferSize];
    bool itemUp, itemDown;
    detectUpDown(itemUp, itemDown);
    model::Material::Set reservedMaterialSet;
    nanoem_rsize_t numItems;
    nanoem_model_morph_material_t *const *items = nanoemModelMorphGetAllMaterialMorphObjects(morphPtr, &numItems);
    selectIndex(itemUp, itemDown, numItems, m_morphItemIndex);
    for (nanoem_rsize_t i = 0; i < numItems; i++) {
        const nanoem_model_morph_material_t *item = items[i];
        const nanoem_model_material_t *materialPtr = nanoemModelMorphMaterialGetMaterialObject(item);
        if (const model::Material *material = model::Material::cast(materialPtr)) {
            const bool selected = static_cast<nanoem_rsize_t>(i) == m_morphItemIndex;
            StringUtils::format(buffer, sizeof(buffer), "%s##item[%lu].name", material->nameConstString(), i);
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
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_activeModel->data(), &numMaterials);
    const char *name = "(select)";
    if (m_morphItemCandidateMaterialIndex < numMaterials) {
        if (model::Material *material = model::Material::cast(materials[m_morphItemCandidateMaterialIndex])) {
            name = material->nameConstString();
        }
    }
    if (ImGui::BeginCombo("##candidate", name)) {
        for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
            const nanoem_model_material_t *materialPtr = materials[i];
            if (model::Material *material = model::Material::cast(materialPtr)) {
                const bool selected = static_cast<nanoem_rsize_t>(i) == m_morphItemCandidateMaterialIndex;
                const ImGuiSelectableFlags flags = reservedMaterialSet.find(materialPtr) != reservedMaterialSet.end()
                    ? ImGuiSelectableFlags_Disabled
                    : ImGuiSelectableFlags_None;
                StringUtils::format(buffer, sizeof(buffer), "%s##item[%lu].name", material->nameConstString(), i);
                if (ImGui::Selectable(buffer, selected, flags)) {
                    m_morphItemCandidateMaterialIndex = i;
                }
            }
        }
        ImGui::EndCombo();
    }
    if (ImGuiWindow::handleButton(tr("nanoem.gui.model.edit.morph.material.add"),
            ImGui::GetContentRegionAvail().x * 0.5f, m_morphItemCandidateMaterialIndex < numMaterials)) {
        nanoem_status_t status;
        command::ScopedMutableMorph scoped(morphPtr);
        nanoem_mutable_model_morph_material_t *item = nanoemMutableModelMorphMaterialCreate(scoped, &status);
        nanoemMutableModelMorphMaterialSetMaterialObject(item, materials[m_morphItemCandidateMaterialIndex]);
        nanoemMutableModelMorphInsertMaterialMorphObject(scoped, item, -1, &status);
        nanoemMutableModelMorphMaterialDestroy(item);
        items = nanoemModelMorphGetAllMaterialMorphObjects(morphPtr, &numItems);
        m_morphItemCandidateMaterialIndex = NANOEM_RSIZE_MAX;
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(tr("nanoem.gui.model.edit.morph.material.remove"), ImGui::GetContentRegionAvail().x,
            m_morphItemIndex < numItems)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        command::ScopedMutableMorph scoped(morphPtr);
        nanoem_mutable_model_morph_material_t *item =
            nanoemMutableModelMorphMaterialCreateAsReference(items[m_morphItemIndex], &status);
        nanoemMutableModelMorphRemoveMaterialMorphObject(scoped, item, &status);
        nanoemMutableModelMorphMaterialDestroy(item);
    }
    addSeparator();
    nanoem_model_morph_material_t *morphMaterialPtr = m_morphItemIndex < numItems ? items[m_morphItemIndex] : nullptr;
    {
        StringUtils::format(buffer, sizeof(buffer), "%s##material.toggle", ImGuiWindow::kFALink);
        const nanoem_model_material_t *materialPtr = nanoemModelMorphMaterialGetMaterialObject(morphMaterialPtr);
        if (ImGuiWindow::handleButton(buffer, 0, morphMaterialPtr != nullptr)) {
            m_explicitTabType = kTabTypeMaterial;
            m_materialIndex = nanoemModelObjectGetIndex(nanoemModelMaterialGetModelObject(materialPtr));
        }
        ImGui::SameLine();
        const model::Material *material = model::Material::cast(materialPtr);
        ImGui::TextUnformatted(material ? material->nameConstString() : "(none)");
    }
    addSeparator();
    {
        nanoem_model_morph_material_operation_type_t value = nanoemModelMorphMaterialGetOperationType(morphMaterialPtr);
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.material.operation"));
        if (ImGui::BeginCombo("##operation", selectedMorphMaterialOperationType(value))) {
            for (int i = NANOEM_MODEL_MORPH_MATERIAL_OPERATION_TYPE_FIRST_ENUM;
                 i < NANOEM_MODEL_MORPH_MATERIAL_OPERATION_TYPE_MAX_ENUM; i++) {
                nanoem_model_morph_material_operation_type_t type =
                    static_cast<nanoem_model_morph_material_operation_type_t>(i);
                if (ImGui::Selectable(selectedMorphMaterialOperationType(type))) {
                    command::ScopedMutableMorphMaterial scoped(morphMaterialPtr);
                    nanoemMutableModelMorphMaterialSetOperationType(scoped, type);
                }
            }
            ImGui::EndCombo();
        }
    }
    {
        Vector4 ambientColor(glm::make_vec4(nanoemModelMorphMaterialGetAmbientColor(morphMaterialPtr)));
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.material.ambient.color"));
        if (ImGui::ColorEdit3("##material.ambient.color", glm::value_ptr(ambientColor))) {
            command::ScopedMutableMorphMaterial scoped(morphMaterialPtr);
            nanoemMutableModelMorphMaterialSetAmbientColor(scoped, glm::value_ptr(ambientColor));
        }
    }
    {
        Vector4 diffuseColor(glm::make_vec3(nanoemModelMorphMaterialGetDiffuseColor(morphMaterialPtr)),
            nanoemModelMorphMaterialGetDiffuseOpacity(morphMaterialPtr));
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.material.diffuse.color"));
        if (ImGui::ColorEdit4("##material.diffuse.color", glm::value_ptr(diffuseColor))) {
            command::ScopedMutableMorphMaterial scoped(morphMaterialPtr);
            nanoemMutableModelMorphMaterialSetDiffuseColor(scoped, glm::value_ptr(diffuseColor));
            nanoemMutableModelMorphMaterialSetDiffuseOpacity(scoped, diffuseColor.w);
        }
    }
    {
        Vector4 specularColor(glm::make_vec4(nanoemModelMorphMaterialGetSpecularColor(morphMaterialPtr)));
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.material.specular.color"));
        if (ImGui::ColorEdit3("##material.specular.color", glm::value_ptr(specularColor))) {
            command::ScopedMutableMorphMaterial scoped(morphMaterialPtr);
            nanoemMutableModelMorphMaterialSetSpecularColor(scoped, glm::value_ptr(specularColor));
        }
    }
    {
        nanoem_f32_t specularPower = nanoemModelMorphMaterialGetSpecularPower(morphMaterialPtr);
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.material.specular.power"));
        if (ImGui::DragFloat("##material.specular.power", &specularPower)) {
            command::ScopedMutableMorphMaterial scoped(morphMaterialPtr);
            nanoemMutableModelMorphMaterialSetSpecularPower(scoped, specularPower);
        }
    }
    addSeparator();
    {
        Vector4 edgeColor(glm::make_vec3(nanoemModelMorphMaterialGetEdgeColor(morphMaterialPtr)),
            nanoemModelMorphMaterialGetEdgeOpacity(morphMaterialPtr));
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.material.edge.color"));
        if (ImGui::ColorEdit4("##material.edge.color", glm::value_ptr(edgeColor))) {
            command::ScopedMutableMorphMaterial scoped(morphMaterialPtr);
            nanoemMutableModelMorphMaterialSetEdgeColor(scoped, glm::value_ptr(edgeColor));
            nanoemMutableModelMorphMaterialSetEdgeOpacity(scoped, edgeColor.w);
        }
    }
    {
        nanoem_f32_t edgeSize = nanoemModelMorphMaterialGetEdgeSize(morphMaterialPtr);
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.material.edge.size"));
        if (ImGui::DragFloat("##material.edge.size", &edgeSize)) {
            command::ScopedMutableMorphMaterial scoped(morphMaterialPtr);
            nanoemMutableModelMorphMaterialSetEdgeSize(scoped, edgeSize);
        }
    }
    addSeparator();
    {
        Vector4 blendFactor(glm::make_vec4(nanoemModelMorphMaterialGetDiffuseTextureBlend(morphMaterialPtr)));
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.material.blend.diffuse"));
        if (ImGui::ColorEdit4("##material.blend.diffuse", glm::value_ptr(blendFactor))) {
            command::ScopedMutableMorphMaterial scoped(morphMaterialPtr);
            nanoemMutableModelMorphMaterialSetDiffuseTextureBlend(scoped, glm::value_ptr(blendFactor));
        }
    }
    {
        Vector4 blendFactor(glm::make_vec4(nanoemModelMorphMaterialGetSphereMapTextureBlend(morphMaterialPtr)));
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.material.blend.sphere-map"));
        if (ImGui::ColorEdit4("##material.blend.sphere-map", glm::value_ptr(blendFactor))) {
            command::ScopedMutableMorphMaterial scoped(morphMaterialPtr);
            nanoemMutableModelMorphMaterialSetSphereMapTextureBlend(scoped, glm::value_ptr(blendFactor));
        }
    }
    {
        Vector4 blendFactor(glm::make_vec4(nanoemModelMorphMaterialGetToonTextureBlend(morphMaterialPtr)));
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.material.blend.toon"));
        if (ImGui::ColorEdit4("##material.blend.toon", glm::value_ptr(blendFactor))) {
            command::ScopedMutableMorphMaterial scoped(morphMaterialPtr);
            nanoemMutableModelMorphMaterialSetToonTextureBlend(scoped, glm::value_ptr(blendFactor));
        }
    }
    ImGui::PopItemWidth();
}

void
ModelParameterDialog::layoutMorphUVPropertyPane(
    const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr, Project *project)
{
    char buffer[Inline::kNameStackBufferSize];
    bool itemUp, itemDown;
    detectUpDown(itemUp, itemDown);
    nanoem_rsize_t numItems;
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
    nanoem_model_morph_uv_t *morphUVPtr = m_morphItemIndex < numItems ? items[m_morphItemIndex] : nullptr;
    {
        StringUtils::format(buffer, sizeof(buffer), "%s##uv.toggle", ImGuiWindow::kFALink);
        const nanoem_model_vertex_t *vertexPtr = nanoemModelMorphUVGetVertexObject(morphUVPtr);
        if (ImGuiWindow::handleButton(buffer, 0, morphUVPtr != nullptr)) {
            m_explicitTabType = kTabTypeVertex;
            m_vertexIndex = nanoemModelObjectGetIndex(nanoemModelVertexGetModelObject(vertexPtr));
        }
        ImGui::SameLine();
        formatVertexText(buffer, sizeof(buffer), vertexPtr);
        ImGui::TextUnformatted(buffer);
    }
    addSeparator();
    {
        Vector4 position(glm::make_vec4(nanoemModelMorphUVGetPosition(morphUVPtr)));
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.uv.position"));
        if (ImGui::DragFloat4("##uv.position", glm::value_ptr(position), 1.0f, 0.0f, 0.0f)) {
            command::ScopedMutableMorphUV scoped(morphUVPtr);
            nanoemMutableModelMorphUVSetPosition(scoped, glm::value_ptr(position));
        }
    }
    ImGui::PopItemWidth();
}

void
ModelParameterDialog::layoutMorphVertexPropertyPane(
    const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr, Project *project)
{
    char buffer[Inline::kNameStackBufferSize];
    bool itemUp, itemDown;
    detectUpDown(itemUp, itemDown);
    nanoem_rsize_t numItems;
    nanoem_model_morph_vertex_t *const *items = nanoemModelMorphGetAllVertexMorphObjects(morphPtr, &numItems);
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
    nanoem_model_morph_vertex_t *morphVertexPtr = m_morphItemIndex < numItems ? items[m_morphItemIndex] : nullptr;
    {
        StringUtils::format(buffer, sizeof(buffer), "%s##vertex.toggle", ImGuiWindow::kFALink);
        const nanoem_model_vertex_t *vertexPtr = nanoemModelMorphVertexGetVertexObject(morphVertexPtr);
        if (ImGuiWindow::handleButton(buffer, 0, morphVertexPtr != nullptr)) {
            m_explicitTabType = kTabTypeVertex;
            m_vertexIndex = nanoemModelObjectGetIndex(nanoemModelVertexGetModelObject(vertexPtr));
        }
        ImGui::SameLine();
        formatVertexText(buffer, sizeof(buffer), vertexPtr);
        ImGui::TextUnformatted(buffer);
    }
    addSeparator();
    {
        Vector4 position(glm::make_vec4(nanoemModelMorphVertexGetPosition(morphVertexPtr)));
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.morph.vertex.position"));
        if (ImGui::DragFloat3("##vertex.position", glm::value_ptr(position), 1.0f, 0.0f, 0.0f)) {
            command::ScopedMutableMorphVertex scoped(morphVertexPtr);
            nanoemMutableModelMorphVertexSetPosition(scoped, glm::value_ptr(position));
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
    IModelObjectSelection *selection = m_activeModel->selection();
    if (ImGui::BeginPopup("label-op-menu")) {
        if (ImGui::BeginMenu("Selection")) {
            if (ImGui::MenuItem("Select All Labels")) {
                for (nanoem_rsize_t i = 0; i < numLabels; i++) {
                    const nanoem_model_label_t *labelPtr = labels[i];
                    selection->addLabel(labelPtr);
                }
            }
            if (ImGui::MenuItem("Deselect All Labels")) {
                selection->removeAllLabels();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Select All Bones/Morphs from All Selected Labels")) {
                const model::Label::Set labels(selection->allLabelSet());
                for (model::Label::Set::const_iterator it = labels.begin(), end = labels.end(); it != end; ++it) {
                    const nanoem_model_label_t *labelPtr = *it;
                    nanoem_rsize_t numItems;
                    nanoem_model_label_item_t *const *items = nanoemModelLabelGetAllItemObjects(labelPtr, &numItems);
                    for (nanoem_rsize_t i = 0; i < numItems; i++) {
                        const nanoem_model_label_item_t *item = items[i];
                        switch (nanoemModelLabelItemGetType(item)) {
                        case NANOEM_MODEL_LABEL_ITEM_TYPE_BONE: {
                            selection->addBone(nanoemModelLabelItemGetBoneObject(item));
                            break;
                        }
                        case NANOEM_MODEL_LABEL_ITEM_TYPE_MORPH: {
                            selection->addMorph(nanoemModelLabelItemGetMorphObject(item));
                            break;
                        }
                        default:
                            break;
                        }
                    }
                }
            }
            ImGui::EndMenu();
        }
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
    clipper.Begin(Inline::saturateInt32(numLabels));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            const nanoem_model_label_t *labelPtr = labels[i];
            const model::Label *label = model::Label::cast(labelPtr);
            const bool selected = selection->containsLabel(labelPtr);
            StringUtils::format(buffer, sizeof(buffer), "%s##label[%d].name", label->nameConstString(), i);
            if (selected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWindow::kColorSelectedModelObject);
            }
            if (ImGui::Selectable(buffer, selected)) {
                ImGui::SetScrollHereY();
                const ImGuiIO &io = ImGui::GetIO();
                if (io.KeyCtrl) {
                    selected ? selection->removeLabel(labelPtr) : selection->addLabel(labelPtr);
                }
                else {
                    if (io.KeyShift) {
                        const nanoem_rsize_t offset = i, from = glm::min(offset, m_labelIndex),
                                             to = glm::max(offset, m_labelIndex);
                        for (nanoem_rsize_t j = from; j < to; j++) {
                            selection->addLabel(labels[j]);
                        }
                    }
                    else {
                        selection->removeAllLabels();
                    }
                    m_labelIndex = i;
                    m_labelItemIndex = 0;
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
            if (selected) {
                ImGui::PopStyleColor();
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
        int selectedLabelIndex = model::Label::index(selectedLabel);
        if (ImGui::BeginMenu("Insert New")) {
            if (ImGui::MenuItem("at Last")) {
                undo_command_t *command = command::CreateLabelCommand::create(project, numLabels, -1, nullptr);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            if (ImGui::MenuItem("after Selected", nullptr, nullptr, m_labelIndex < numLabels)) {
                undo_command_t *command = command::CreateLabelCommand::create(project, numLabels, selectedLabelIndex + 1, nullptr);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Insert Copy", m_labelIndex < numLabels)) {
            if (ImGui::MenuItem("at Last")) {
                undo_command_t *command = command::CreateLabelCommand::create(project, numLabels, -1, selectedLabel);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            if (ImGui::MenuItem("at Next")) {
                undo_command_t *command = command::CreateLabelCommand::create(project, numLabels, selectedLabelIndex + 1, selectedLabel);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(
            reinterpret_cast<const char *>(ImGuiWindow::kFAMinus), 0, isEditable && m_labelIndex < numLabels)) {
       undo_command_t *command = command::DeleteLabelCommand::create(project, labels, m_labelIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowUp), 0, isEditable)) {
        undo_command_t *command = command::MoveLabelUpCommand::create(project, labels, m_labelIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowDown), 0,
            isEditable && numLabels > 0 && m_labelIndex < numLabels - 1)) {
        undo_command_t *command = command::MoveLabelDownCommand::create(project, labels, m_labelIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
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
        command::ScopedMutableLabel scoped(labelPtr);
        nanoemMutableModelLabelSetName(scoped, scope.value(), language, &status);
    }
    addSeparator();
    {
        bool value = nanoemModelLabelIsSpecial(labelPtr) != 0;
        if (ImGuiWindow::handleCheckBox(tr("nanoem.gui.model.edit.label.special"), &value, m_labelIndex > 0)) {
            command::ScopedMutableLabel scoped(labelPtr);
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
            command::ScopedMutableLabel scoped(labelPtr);
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
            command::ScopedMutableLabel scoped(labelPtr);
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
    IModelObjectSelection *selection = m_activeModel->selection();
    if (ImGui::BeginPopup("rigid-body-op-menu")) {
        if (ImGui::BeginMenu("Selection")) {
            if (ImGui::MenuItem("Select All Rigid Bodies")) {
                for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
                    const nanoem_model_rigid_body_t *rigidBodyPtr = rigidBodies[i];
                    selection->addRigidBody(rigidBodyPtr);
                }
            }
            if (ImGui::MenuItem("Deselect All Rigid Bodies")) {
                selection->removeAllRigidBodies();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Select All Bones from All Selected Rigid Bodies")) {
                const model::RigidBody::Set rigidBodies(selection->allRigidBodySet());
                for (model::RigidBody::Set::const_iterator it = rigidBodies.begin(), end = rigidBodies.end(); it != end;
                     ++it) {
                    const nanoem_model_rigid_body_t *rigidBodyPtr = *it;
                    const nanoem_model_bone_t *bonePtr = nanoemModelRigidBodyGetBoneObject(rigidBodyPtr);
                    selection->addBone(bonePtr);
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Masking")) {
            if (ImGui::MenuItem("Mask All Selected Rigid Bodies")) {
                const model::RigidBody::Set rigidBodies(selection->allRigidBodySet());
                for (model::RigidBody::Set::const_iterator it = rigidBodies.begin(), end = rigidBodies.end(); it != end;
                     ++it) {
                    const nanoem_model_rigid_body_t *rigidBodyPtr = *it;
                    if (model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr)) {
                        rigidBody->setEditingMasked(true);
                    }
                }
            }
            if (ImGui::MenuItem("Unmask All Selected Rigid Bodies")) {
                const model::RigidBody::Set rigidBodies(selection->allRigidBodySet());
                for (model::RigidBody::Set::const_iterator it = rigidBodies.begin(), end = rigidBodies.end(); it != end;
                     ++it) {
                    const nanoem_model_rigid_body_t *rigidBodyPtr = *it;
                    if (model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr)) {
                        rigidBody->setEditingMasked(false);
                    }
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Invert All Mask/Unmask Rigid Bodies")) {
                for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
                    const nanoem_model_rigid_body_t *rigidBodyPtr = rigidBodies[i];
                    if (model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr)) {
                        rigidBody->setEditingMasked(!rigidBody->isEditingMasked());
                    }
                }
            }
            ImGui::EndMenu();
        }
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
    clipper.Begin(Inline::saturateInt32(numRigidBodies));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            const nanoem_model_rigid_body_t *rigidBodyPtr = rigidBodies[i];
            model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr);
            StringUtils::format(buffer, sizeof(buffer), "##rigidBody[%d].visible", i);
            bool visible = rigidBody->isEditingMasked() ? false : true;
            if (ImGui::Checkbox(buffer, &visible)) {
                rigidBody->setEditingMasked(visible ? false : true);
            }
            else if (ImGui::IsItemHovered()) {
                hoveredRigidBodyPtr = rigidBodies[i];
            }
            ImGui::SameLine();
            const bool selected = selection->containsRigidBody(rigidBodyPtr);
            StringUtils::format(buffer, sizeof(buffer), "%s##rigidBody[%d].name", rigidBody->nameConstString(), i);
            if (selected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWindow::kColorSelectedModelObject);
            }
            if (ImGui::Selectable(buffer, selected)) {
                ImGui::SetScrollHereY();
                const ImGuiIO &io = ImGui::GetIO();
                if (io.KeyCtrl) {
                    selected ? selection->removeRigidBody(rigidBodyPtr) : selection->addRigidBody(rigidBodyPtr);
                }
                else {
                    if (io.KeyShift) {
                        const nanoem_rsize_t offset = i, from = glm::min(offset, m_rigidBodyIndex),
                                             to = glm::max(offset, m_rigidBodyIndex);
                        for (nanoem_rsize_t j = from; j < to; j++) {
                            selection->addRigidBody(rigidBodies[j]);
                        }
                    }
                    else {
                        selection->removeAllRigidBodies();
                    }
                    m_rigidBodyIndex = i;
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
            if (selected) {
                ImGui::PopStyleColor();
            }
        }
    }
    ImGui::EndChild();
    bool hovered = ImGui::IsItemHovered();
    if (hovered && hoveredRigidBodyPtr) {
        selection->setHoveredRigidBody(hoveredRigidBodyPtr);
    }
    else if (!hovered && !hoveredRigidBodyPtr) {
        selection->setHoveredRigidBody(nullptr);
    }
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAPlus), 0, true)) {
        ImGui::OpenPopup("rigid-body-create-menu");
    }
    if (ImGui::BeginPopup("rigid-body-create-menu")) {
        const nanoem_model_rigid_body_t *selectedRigidBody =
            numRigidBodies > 0 ? rigidBodies[m_rigidBodyIndex] : nullptr;
        int selectedRigidBodyIndex = model::RigidBody::index(selectedRigidBody);
        if (ImGui::BeginMenu("Insert New")) {
            if (ImGui::MenuItem("at Last")) {
                undo_command_t *command = command::CreateRigidBodyCommand::create(project, numRigidBodies, -1, nullptr);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            if (ImGui::MenuItem("after Selected", nullptr, nullptr, m_rigidBodyIndex < numRigidBodies)) {
                undo_command_t *command = command::CreateRigidBodyCommand::create(project, numRigidBodies, selectedRigidBodyIndex + 1, nullptr);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Insert Copy", m_rigidBodyIndex < numRigidBodies)) {
            if (ImGui::MenuItem("at Last")) {
                undo_command_t *command = command::CreateRigidBodyCommand::create(project, numRigidBodies, -1, selectedRigidBody);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            if (ImGui::MenuItem("at Next")) {
                undo_command_t *command = command::CreateRigidBodyCommand::create(project, numRigidBodies, selectedRigidBodyIndex + 1, selectedRigidBody);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(
            reinterpret_cast<const char *>(ImGuiWindow::kFAMinus), 0, m_rigidBodyIndex < numRigidBodies)) {
        undo_command_t *command = command::DeleteRigidBodyCommand::create(project, rigidBodies, m_rigidBodyIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowUp), 0, m_rigidBodyIndex > 0)) {
        undo_command_t *command = command::MoveRigidBodyUpCommand::create(project, rigidBodies, m_rigidBodyIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowDown), 0,
            numRigidBodies > 0 && m_rigidBodyIndex < numRigidBodies - 1)) {
        undo_command_t *command = command::MoveRigidBodyDownCommand::create(project, rigidBodies, m_rigidBodyIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
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
        command::ScopedMutableRigidBody scoped(rigidBodyPtr);
        nanoemMutableModelRigidBodySetName(scoped, scope.value(), language, &status);
    }
    addSeparator();
    {
        char label[Inline::kNameStackBufferSize];
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.rigid-body.bone"));
        const nanoem_model_bone_t *bonePtr = nanoemModelRigidBodyGetBoneObject(rigidBodyPtr);
        StringUtils::format(label, sizeof(label), "%s##toggle", ImGuiWindow::kFALink);
        if (ImGuiWindow::handleButton(label, 0, bonePtr != nullptr)) {
            m_explicitTabType = kTabTypeBone;
            m_boneIndex = nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(bonePtr));
        }
        ImGui::SameLine();
        const model::Bone *bone = model::Bone::cast(bonePtr);
        nanoem_rsize_t numBones;
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
        if (ImGui::BeginCombo("##bone", bone ? bone->nameConstString() : "(none)")) {
            for (nanoem_rsize_t i = 0; i < numBones; i++) {
                const nanoem_model_bone_t *candidateBonePtr = bones[i];
                if (const model::Bone *candidateBone = model::Bone::cast(candidateBonePtr)) {
                    StringUtils::format(buffer, sizeof(buffer), "%s##item[%lu].name", bone->nameConstString(), i);
                    if (ImGui::Selectable(candidateBone->nameConstString(), candidateBone == bone)) {
                        command::ScopedMutableRigidBody scoped(rigidBodyPtr);
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
            command::ScopedMutableRigidBody scoped(rigidBodyPtr);
            nanoemMutableModelRigidBodySetOrigin(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.rigid-body.orientation"));
        Vector4 value(glm::make_vec4(nanoemModelRigidBodyGetOrientation(rigidBodyPtr)));
        if (ImGui::InputFloat3("##orientation", glm::value_ptr(value))) {
            command::ScopedMutableRigidBody scoped(rigidBodyPtr);
            nanoemMutableModelRigidBodySetOrientation(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.rigid-body.shape.size"));
        Vector4 value(glm::make_vec4(nanoemModelRigidBodyGetShapeSize(rigidBodyPtr)));
        if (ImGui::InputFloat3("##size", glm::value_ptr(value))) {
            command::ScopedMutableRigidBody scoped(rigidBodyPtr);
            nanoemMutableModelRigidBodySetShapeSize(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.rigid-body.shape-type"));
        nanoem_model_rigid_body_shape_type_t value = nanoemModelRigidBodyGetShapeType(rigidBodyPtr);
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.rigid-body.shape-type.sphere"),
                value == NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_SPHERE)) {
            command::ScopedMutableRigidBody scoped(rigidBodyPtr);
            nanoemMutableModelRigidBodySetShapeType(scoped, NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_SPHERE);
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.rigid-body.shape-type.box"),
                value == NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_BOX)) {
            command::ScopedMutableRigidBody scoped(rigidBodyPtr);
            nanoemMutableModelRigidBodySetShapeType(scoped, NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_BOX);
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.rigid-body.shape-type.capsule"),
                value == NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_CAPSULE)) {
            command::ScopedMutableRigidBody scoped(rigidBodyPtr);
            nanoemMutableModelRigidBodySetShapeType(scoped, NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_CAPSULE);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.rigid-body.object-type"));
        nanoem_model_rigid_body_transform_type_t value = nanoemModelRigidBodyGetTransformType(rigidBodyPtr);
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.rigid-body.object-type.static"),
                value == NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_TO_SIMULATION)) {
            command::ScopedMutableRigidBody scoped(rigidBodyPtr);
            nanoemMutableModelRigidBodySetTransformType(
                scoped, NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_TO_SIMULATION);
        }
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.rigid-body.object-type.dynamic"),
                value == NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_SIMULATION_TO_BONE)) {
            command::ScopedMutableRigidBody scoped(rigidBodyPtr);
            nanoemMutableModelRigidBodySetTransformType(
                scoped, NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_SIMULATION_TO_BONE);
        }
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.rigid-body.object-type.kinematic"),
                value == NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_ORIENTATION_AND_SIMULATION_TO_BONE)) {
            command::ScopedMutableRigidBody scoped(rigidBodyPtr);
            nanoemMutableModelRigidBodySetTransformType(
                scoped, NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_ORIENTATION_AND_SIMULATION_TO_BONE);
        }
    }
    addSeparator();
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.rigid-body.mass"));
        nanoem_f32_t value = nanoemModelRigidBodyGetMass(rigidBodyPtr);
        if (ImGui::InputFloat("##mass", &value)) {
            command::ScopedMutableRigidBody scoped(rigidBodyPtr);
            nanoemMutableModelRigidBodySetMass(scoped, value);
        }
    }
    {
        StringUtils::format(buffer, sizeof(buffer), "%s: %%.3f", tr("nanoem.gui.model.edit.rigid-body.linear-damping"));
        nanoem_f32_t value = nanoemModelRigidBodyGetLinearDamping(rigidBodyPtr);
        if (ImGui::SliderFloat("##damping.linear", &value, 0.0f, 1.0f, buffer)) {
            command::ScopedMutableRigidBody scoped(rigidBodyPtr);
            nanoemMutableModelRigidBodySetLinearDamping(scoped, value);
        }
    }
    {
        StringUtils::format(
            buffer, sizeof(buffer), "%s: %%.3f", tr("nanoem.gui.model.edit.rigid-body.angular-damping"));
        nanoem_f32_t value = nanoemModelRigidBodyGetAngularDamping(rigidBodyPtr);
        if (ImGui::SliderFloat("##damping.angular", &value, 0.0f, 1.0f, buffer)) {
            command::ScopedMutableRigidBody scoped(rigidBodyPtr);
            nanoemMutableModelRigidBodySetAngularDamping(scoped, value);
        }
    }
    {
        StringUtils::format(buffer, sizeof(buffer), "%s: %%.3f", tr("nanoem.gui.model.edit.rigid-body.friction"));
        nanoem_f32_t value = nanoemModelRigidBodyGetFriction(rigidBodyPtr);
        if (ImGui::SliderFloat("##friction", &value, 0.0f, 1.0f, buffer)) {
            command::ScopedMutableRigidBody scoped(rigidBodyPtr);
            nanoemMutableModelRigidBodySetFriction(scoped, value);
        }
    }
    {
        StringUtils::format(buffer, sizeof(buffer), "%s: %%.3f", tr("nanoem.gui.model.edit.rigid-body.restitution"));
        nanoem_f32_t value = nanoemModelRigidBodyGetRestitution(rigidBodyPtr);
        if (ImGui::SliderFloat("##restitution", &value, 0.0f, 1.0f, buffer)) {
            command::ScopedMutableRigidBody scoped(rigidBodyPtr);
            nanoemMutableModelRigidBodySetRestitution(scoped, value);
        }
    }
    addSeparator();
    {
        ImGui::TextUnformatted("Collision Group");
        int value = nanoemModelRigidBodyGetCollisionGroupId(rigidBodyPtr);
        if (ImGui::DragInt("##collision.group", &value, 0.05f, 0, 15)) {
            command::ScopedMutableRigidBody scoped(rigidBodyPtr);
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
                command::ScopedMutableRigidBody scoped(rigidBodyPtr);
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
    IModelObjectSelection *selection = m_activeModel->selection();
    if (ImGui::BeginPopup("joint-op-menu")) {
        if (ImGui::BeginMenu("Selection")) {
            if (ImGui::MenuItem("Select All Joints")) {
                for (nanoem_rsize_t i = 0; i < numJoints; i++) {
                    const nanoem_model_joint_t *jointPtr = joints[i];
                    selection->addJoint(jointPtr);
                }
            }
            if (ImGui::MenuItem("Deselect All Joints")) {
                selection->removeAllJoints();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Select All Bones from All Selected Joints")) {
                const model::Joint::Set joints(selection->allJointSet());
                for (model::Joint::Set::const_iterator it = joints.begin(), end = joints.end(); it != end; ++it) {
                    const nanoem_model_joint_t *jointPtr = *it;
                    const nanoem_model_rigid_body_t *bodyAPtr = nanoemModelJointGetRigidBodyAObject(jointPtr),
                                                    *bodyBPtr = nanoemModelJointGetRigidBodyBObject(jointPtr);
                    const nanoem_model_bone_t *boneAPtr = nanoemModelRigidBodyGetBoneObject(bodyAPtr),
                                              *boneBPtr = nanoemModelRigidBodyGetBoneObject(bodyBPtr);
                    selection->addBone(boneAPtr);
                    selection->addBone(boneBPtr);
                }
            }
            if (ImGui::MenuItem("Select All Rigid Bodies from All Selected Joints")) {
                const model::Joint::Set joints(selection->allJointSet());
                for (model::Joint::Set::const_iterator it = joints.begin(), end = joints.end(); it != end; ++it) {
                    const nanoem_model_joint_t *jointPtr = *it;
                    const nanoem_model_rigid_body_t *bodyAPtr = nanoemModelJointGetRigidBodyAObject(jointPtr),
                                                    *bodyBPtr = nanoemModelJointGetRigidBodyBObject(jointPtr);
                    selection->addRigidBody(bodyAPtr);
                    selection->addRigidBody(bodyBPtr);
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Masking")) {
            if (ImGui::MenuItem("Mask All Selected Joints")) {
                const model::Joint::Set joints(selection->allJointSet());
                for (model::Joint::Set::const_iterator it = joints.begin(), end = joints.end(); it != end; ++it) {
                    const nanoem_model_joint_t *jointPtr = *it;
                    if (model::Joint *joint = model::Joint::cast(jointPtr)) {
                        joint->setEditingMasked(true);
                    }
                }
            }
            if (ImGui::MenuItem("Unmask All Selected Joints")) {
                const model::Joint::Set joints(selection->allJointSet());
                for (model::Joint::Set::const_iterator it = joints.begin(), end = joints.end(); it != end; ++it) {
                    const nanoem_model_joint_t *jointPtr = *it;
                    if (model::Joint *joint = model::Joint::cast(jointPtr)) {
                        joint->setEditingMasked(false);
                    }
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Invert All Mask/Unmask Joints")) {
                for (nanoem_rsize_t i = 0; i < numJoints; i++) {
                    const nanoem_model_joint_t *jointPtr = joints[i];
                    if (model::Joint *joint = model::Joint::cast(jointPtr)) {
                        joint->setEditingMasked(!joint->isEditingMasked());
                    }
                }
            }
            ImGui::EndMenu();
        }
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
    clipper.Begin(Inline::saturateInt32(numJoints));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            const nanoem_model_joint_t *jointPtr = joints[i];
            model::Joint *joint = model::Joint::cast(jointPtr);
            StringUtils::format(buffer, sizeof(buffer), "##joint[%d].visible", i);
            bool visible = joint->isEditingMasked() ? false : true;
            if (ImGui::Checkbox(buffer, &visible)) {
                joint->setEditingMasked(visible ? false : true);
            }
            else if (ImGui::IsItemHovered()) {
                hoveredJointPtr = joints[i];
            }
            ImGui::SameLine();
            const bool selected = selection->containsJoint(jointPtr);
            StringUtils::format(buffer, sizeof(buffer), "%s##joint[%d].name", joint->nameConstString(), i);
            if (selected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWindow::kColorSelectedModelObject);
            }
            if (ImGui::Selectable(buffer, selected)) {
                ImGui::SetScrollHereY();
                const ImGuiIO &io = ImGui::GetIO();
                if (io.KeyCtrl) {
                    selected ? selection->removeJoint(jointPtr) : selection->addJoint(jointPtr);
                }
                else {
                    if (io.KeyShift) {
                        const nanoem_rsize_t offset = i, from = glm::min(offset, m_jointIndex),
                                             to = glm::max(offset, m_jointIndex);
                        for (nanoem_rsize_t j = from; j < to; j++) {
                            selection->addJoint(joints[j]);
                        }
                    }
                    else {
                        selection->removeAllJoints();
                    }
                    m_jointIndex = i;
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
            if (selected) {
                ImGui::PopStyleColor();
            }
        }
    }
    bool hovered = ImGui::IsItemHovered();
    if (hovered && hoveredJointPtr) {
        selection->setHoveredJoint(hoveredJointPtr);
    }
    else if (!hovered && !hoveredJointPtr) {
        selection->setHoveredJoint(nullptr);
    }
    ImGui::EndChild();
    joints = nanoemModelGetAllJointObjects(m_activeModel->data(), &numJoints);
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAPlus), 0, true)) {
        ImGui::OpenPopup("rigid-body-create-menu");
    }
    if (ImGui::BeginPopup("rigid-body-create-menu")) {
        const nanoem_model_joint_t *selectedJoint = numJoints > 0 ? joints[m_jointIndex] : nullptr;
        int selectedJointIndex = model::Joint::index(selectedJoint);
        if (ImGui::BeginMenu("Insert New")) {
            if (ImGui::MenuItem("at Last")) {
                undo_command_t *command = command::CreateJointCommand::create(project, numJoints, -1, nullptr);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            if (ImGui::MenuItem("after Selected", nullptr, nullptr, m_jointIndex < numJoints)) {
                undo_command_t *command = command::CreateJointCommand::create(project, numJoints, selectedJointIndex + 1, nullptr);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Insert Copy", m_jointIndex < numJoints)) {
            if (ImGui::MenuItem("at Last")) {
                undo_command_t *command = command::CreateJointCommand::create(project, numJoints, -1, selectedJoint);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            if (ImGui::MenuItem("at Next")) {
                undo_command_t *command = command::CreateJointCommand::create(project, numJoints, selectedJointIndex + 1, selectedJoint);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAMinus), 0, m_jointIndex < numJoints)) {
        undo_command_t *command = command::DeleteJointCommand::create(project, joints, m_jointIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowUp), 0, m_jointIndex > 0)) {
        undo_command_t *command = command::MoveJointUpCommand::create(project, joints, m_jointIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowDown), 0,
            numJoints > 0 && m_jointIndex < numJoints - 1)) {
        undo_command_t *command = command::MoveJointDownCommand::create(project, joints, m_jointIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
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
        command::ScopedMutableJoint scoped(jointPtr);
        nanoemMutableModelJointSetName(scoped, scope.value(), language, &status);
    }
    addSeparator();
    {
        char label[Inline::kNameStackBufferSize];
        nanoem_rsize_t numRigidBodies;
        nanoem_model_rigid_body_t *const *bodies =
            nanoemModelGetAllRigidBodyObjects(m_activeModel->data(), &numRigidBodies);
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.rigid-body.a"));
        const nanoem_model_rigid_body_t *rigidBodyAPtr = nanoemModelJointGetRigidBodyAObject(jointPtr);
        StringUtils::format(label, sizeof(label), "%s##toggle-a", ImGuiWindow::kFALink);
        if (ImGuiWindow::handleButton(label, 0, rigidBodyAPtr != nullptr)) {
            m_explicitTabType = kTabTypeRigidBody;
            m_rigidBodyIndex = nanoemModelObjectGetIndex(nanoemModelRigidBodyGetModelObject(rigidBodyAPtr));
        }
        ImGui::SameLine();
        const model::RigidBody *rigidBodyA = model::RigidBody::cast(rigidBodyAPtr);
        if (ImGui::BeginCombo("##rigid-body.a", rigidBodyA ? rigidBodyA->nameConstString() : "(none)")) {
            for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
                const nanoem_model_rigid_body_t *candidateBodyPtr = bodies[i];
                if (const model::RigidBody *candidateBody = model::RigidBody::cast(candidateBodyPtr)) {
                    StringUtils::format(
                        buffer, sizeof(buffer), "%s##item[%lu].name", candidateBody->nameConstString(), i);
                    if (ImGui::Selectable(candidateBody->nameConstString(), candidateBody == rigidBodyA)) {
                        command::ScopedMutableJoint scoped(jointPtr);
                        nanoemMutableModelJointSetRigidBodyAObject(scoped, candidateBodyPtr);
                    }
                }
            }
            ImGui::EndCombo();
        }
        const nanoem_model_rigid_body_t *rigidBodyBPtr = nanoemModelJointGetRigidBodyBObject(jointPtr);
        StringUtils::format(label, sizeof(label), "%s##toggle-b", ImGuiWindow::kFALink);
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.rigid-body.b"));
        if (ImGuiWindow::handleButton(label, 0, rigidBodyBPtr != nullptr)) {
            m_explicitTabType = kTabTypeRigidBody;
            m_rigidBodyIndex = nanoemModelObjectGetIndex(nanoemModelRigidBodyGetModelObject(rigidBodyBPtr));
        }
        ImGui::SameLine();
        const model::RigidBody *rigidBodyB = model::RigidBody::cast(rigidBodyBPtr);
        if (ImGui::BeginCombo("##rigid-body.b", rigidBodyB ? rigidBodyB->nameConstString() : "(none)")) {
            for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
                const nanoem_model_rigid_body_t *candidateBodyPtr = bodies[i];
                model::RigidBody *candidateBody = model::RigidBody::cast(candidateBodyPtr);
                if (candidateBody && ImGui::Selectable(candidateBody->nameConstString(), candidateBody == rigidBodyB)) {
                    command::ScopedMutableJoint scoped(jointPtr);
                    nanoemMutableModelJointSetRigidBodyAObject(scoped, candidateBodyPtr);
                }
            }
            ImGui::EndCombo();
        }
    }
    addSeparator();
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.origin"));
        Vector4 value(glm::make_vec4(nanoemModelJointGetOrigin(jointPtr)));
        if (ImGui::InputFloat3("##origin", glm::value_ptr(value))) {
            command::ScopedMutableJoint scoped(jointPtr);
            nanoemMutableModelJointSetOrigin(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.orientation"));
        Vector4 value(glm::make_vec4(nanoemModelJointGetOrientation(jointPtr)));
        if (ImGui::InputFloat3("##orientation", glm::value_ptr(value))) {
            command::ScopedMutableJoint scoped(jointPtr);
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
                    command::ScopedMutableJoint scoped(jointPtr);
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
            command::ScopedMutableJoint scoped(jointPtr);
            nanoemMutableModelJointSetLinearStiffness(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.linear.upper"));
        Vector4 value(glm::make_vec4(nanoemModelJointGetLinearUpperLimit(jointPtr)));
        if (ImGui::InputFloat3("##linear.upper", glm::value_ptr(value))) {
            command::ScopedMutableJoint scoped(jointPtr);
            nanoemMutableModelJointSetLinearUpperLimit(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.linear.lower"));
        Vector4 value(glm::make_vec4(nanoemModelJointGetLinearLowerLimit(jointPtr)));
        if (ImGui::InputFloat3("##linear.lower", glm::value_ptr(value))) {
            command::ScopedMutableJoint scoped(jointPtr);
            nanoemMutableModelJointSetLinearLowerLimit(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.angular.stiffness"));
        Vector4 value(glm::make_vec4(nanoemModelJointGetAngularStiffness(jointPtr)));
        if (ImGui::InputFloat3("##angular.stiffness", glm::value_ptr(value))) {
            command::ScopedMutableJoint scoped(jointPtr);
            nanoemMutableModelJointSetAngularStiffness(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.angular.upper"));
        Vector4 value(glm::make_vec4(nanoemModelJointGetAngularUpperLimit(jointPtr)));
        if (ImGui::InputFloat3("##angular.upper", glm::value_ptr(value))) {
            command::ScopedMutableJoint scoped(jointPtr);
            nanoemMutableModelJointSetAngularUpperLimit(scoped, glm::value_ptr(value));
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.joint.angular.lower"));
        Vector4 value(glm::make_vec4(nanoemModelJointGetAngularLowerLimit(jointPtr)));
        if (ImGui::InputFloat3("##angular.lower", glm::value_ptr(value))) {
            command::ScopedMutableJoint scoped(jointPtr);
            nanoemMutableModelJointSetAngularLowerLimit(scoped, glm::value_ptr(value));
        }
    }
    ImGui::PopItemWidth();
}

void
ModelParameterDialog::layoutAllSoftBodies(Project *project)
{
    nanoem_rsize_t numSoftBodies;
    nanoem_model_soft_body_t *const *softBodies =
        nanoemModelGetAllSoftBodyObjects(m_activeModel->data(), &numSoftBodies);
    nanoem_f32_t width = ImGuiWindow::kLeftPaneWidth * project->windowDevicePixelRatio();
    ImGui::BeginChild("left-pane", ImVec2(width, 0), false);
    if (ImGui::Button(reinterpret_cast<const char *>(ImGuiWindow::kFACogs))) {
        ImGui::OpenPopup("soft-body-op-menu");
    }
    IModelObjectSelection *selection = m_activeModel->selection();
    if (ImGui::BeginPopup("soft-body-op-menu")) {
        if (ImGui::BeginMenu("Selection")) {
            if (ImGui::MenuItem("Select All Soft Bodies")) {
                for (nanoem_rsize_t i = 0; i < numSoftBodies; i++) {
                    const nanoem_model_soft_body_t *softBodyPtr = softBodies[i];
                    selection->addSoftBody(softBodyPtr);
                }
            }
            if (ImGui::MenuItem("Deselect All Soft Bodies")) {
                selection->removeAllSoftBodies();
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    ImGui::Text("%lu / %lu", m_softBodyIndex + 1, numSoftBodies);
    ImGui::BeginChild(
        "left-pane-inner", ImVec2(width, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing()), true);
    ImGuiListClipper clipper;
    nanoem_model_soft_body_t *hoveredSoftBodyPtr = nullptr;
    char buffer[Inline::kNameStackBufferSize];
    bool up, down;
    detectUpDown(up, down);
    selectIndex(up, down, numSoftBodies, m_softBodyIndex);
    clipper.Begin(Inline::saturateInt32(numSoftBodies));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            const nanoem_model_soft_body_t *softBodyPtr = softBodies[i];
            const model::SoftBody *softBody = model::SoftBody::cast(softBodyPtr);
            const bool selected = selection->containsSoftBody(softBodyPtr);
            StringUtils::format(buffer, sizeof(buffer), "%s##softBody[%d].name", softBody->nameConstString(), i);
            if (selected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWindow::kColorSelectedModelObject);
            }
            if (ImGui::Selectable(buffer, selected)) {
                ImGui::SetScrollHereY();
                const ImGuiIO &io = ImGui::GetIO();
                if (io.KeyCtrl) {
                    selected ? selection->removeSoftBody(softBodyPtr) : selection->addSoftBody(softBodyPtr);
                }
                else {
                    if (io.KeyShift) {
                        const nanoem_rsize_t offset = i, from = glm::min(offset, m_softBodyIndex),
                                             to = glm::max(offset, m_softBodyIndex);
                        for (nanoem_rsize_t j = from; j < to; j++) {
                            selection->addSoftBody(softBodies[j]);
                        }
                    }
                    else {
                        selection->removeAllSoftBodies();
                    }
                    m_softBodyIndex = i;
                    selection->addSoftBody(softBodyPtr);
                }
                hoveredSoftBodyPtr = softBodies[i];
            }
            else if ((up || down) && m_softBodyIndex == static_cast<nanoem_rsize_t>(i)) {
                ImGui::SetScrollHereY();
                m_softBodyIndex = i;
            }
            else if (ImGui::IsItemHovered()) {
                hoveredSoftBodyPtr = softBodies[i];
            }
            if (selected) {
                ImGui::PopStyleColor();
            }
        }
    }
    selection->setHoveredSoftBody(hoveredSoftBodyPtr);
    ImGui::EndChild();
    softBodies = nanoemModelGetAllSoftBodyObjects(m_activeModel->data(), &numSoftBodies);
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAPlus), 0, true)) {
        ImGui::OpenPopup("rigid-body-create-menu");
    }
    if (ImGui::BeginPopup("rigid-body-create-menu")) {
        const nanoem_model_soft_body_t *selectedSoftBody = numSoftBodies > 0 ? softBodies[m_softBodyIndex] : nullptr;
        int selectedSoftBodyIndex = model::SoftBody::index(selectedSoftBody);
        if (ImGui::BeginMenu("Insert New")) {
            if (ImGui::MenuItem("at Last")) {
                undo_command_t *command = command::CreateSoftBodyCommand::create(project, numSoftBodies, -1, nullptr);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            if (ImGui::MenuItem("after Selected", nullptr, nullptr, m_softBodyIndex < numSoftBodies)) {
                undo_command_t *command = command::CreateSoftBodyCommand::create(project, numSoftBodies, selectedSoftBodyIndex + 1, nullptr);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Insert Copy", m_softBodyIndex < numSoftBodies)) {
            if (ImGui::MenuItem("at Last")) {
                undo_command_t *command = command::CreateSoftBodyCommand::create(project, numSoftBodies, -1, selectedSoftBody);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            if (ImGui::MenuItem("at Next")) {
                undo_command_t *command = command::CreateSoftBodyCommand::create(project, numSoftBodies, selectedSoftBodyIndex + 1, selectedSoftBody);
                m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(
            reinterpret_cast<const char *>(ImGuiWindow::kFAMinus), 0, m_softBodyIndex < numSoftBodies)) {
        undo_command_t *command = command::DeleteSoftBodyCommand::create(project, softBodies, m_softBodyIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowUp), 0, m_softBodyIndex > 0)) {
        undo_command_t *command = command::MoveSoftBodyUpCommand::create(project, softBodies, m_softBodyIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(reinterpret_cast<const char *>(ImGuiWindow::kFAArrowDown), 0,
            numSoftBodies > 0 && m_softBodyIndex < numSoftBodies - 1)) {
        undo_command_t *command = command::MoveSoftBodyDownCommand::create(project, softBodies, m_softBodyIndex);
        m_parent->addLazyExecutionCommand(nanoem_new(UndoCommand(command)));
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("right-pane", ImGui::GetContentRegionAvail());
    if (numSoftBodies > 0 && m_softBodyIndex < numSoftBodies) {
        nanoem_model_soft_body_t *softBodyPtr = softBodies[m_softBodyIndex];
        layoutSoftBodyPropertyPane(softBodyPtr, project);
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
        command::ScopedMutableSoftBody scoped(softBodyPtr);
        nanoemMutableModelSoftBodySetName(scoped, scope.value(), language, &status);
    }
    addSeparator();
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
                        command::ScopedMutableSoftBody scoped(softBodyPtr);
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
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetShapeType(scoped, NANOEM_MODEL_SOFT_BODY_SHAPE_TYPE_TRI_MESH);
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(tr("nanoem.gui.model.edit.soft-body.shape-type.rope"),
                value == NANOEM_MODEL_SOFT_BODY_SHAPE_TYPE_ROPE)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
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
                    command::ScopedMutableSoftBody scoped(softBodyPtr);
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
            command::ScopedMutableSoftBody scoped(softBodyPtr);
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
                command::ScopedMutableSoftBody scoped(softBodyPtr);
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
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetTotalMass(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.collision-margin"));
        nanoem_f32_t value = nanoemModelSoftBodyGetCollisionMargin(softBodyPtr);
        if (ImGui::InputFloat("##collision-margin", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetCollisionMargin(scoped, value);
        }
    }
    addSeparator();
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.velocity-correction-factor"));
        nanoem_f32_t value = nanoemModelSoftBodyGetVelocityCorrectionFactor(softBodyPtr);
        if (ImGui::InputFloat("##velocity-correction-factor", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetVelocityCorrectionFactor(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.damping-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetDampingCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##damping-coefficient", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetDampingCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.drag-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetDragCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##drag-coefficient", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetDragCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.lift-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetLiftCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##lift-coefficient", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetLiftCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.pressure-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetPressureCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##pressure-coefficient", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetPressureCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.volume-conversation-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetVolumeConversationCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##volume-conversation-coefficient", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetVolumeConversationCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.dynamic-friction-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetDynamicFrictionCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##dynamic-friction-coefficient", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetDynamicFrictionCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.pose-matching-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetPoseMatchingCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##pose-matching-coefficient", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetPoseMatchingCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.rigid-contact-hardness"));
        nanoem_f32_t value = nanoemModelSoftBodyGetRigidContactHardness(softBodyPtr);
        if (ImGui::InputFloat("##rigid-contact-hardness", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetRigidContactHardness(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.kinetic-contact-hardness"));
        nanoem_f32_t value = nanoemModelSoftBodyGetKineticContactHardness(softBodyPtr);
        if (ImGui::InputFloat("##kinetic-contact-hardness", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetKineticContactHardness(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.soft-contact-hardness"));
        nanoem_f32_t value = nanoemModelSoftBodyGetSoftContactHardness(softBodyPtr);
        if (ImGui::InputFloat("##soft-contact-hardness", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetSoftContactHardness(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.anchor-hardness"));
        nanoem_f32_t value = nanoemModelSoftBodyGetAnchorHardness(softBodyPtr);
        if (ImGui::InputFloat("##anchor-hardness", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetAnchorHardness(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.soft-vs-kinetic-hardness"));
        nanoem_f32_t value = nanoemModelSoftBodyGetSoftVSKineticHardness(softBodyPtr);
        if (ImGui::InputFloat("##soft-vs-kinetic-hardness", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetSoftVSKineticHardness(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.soft-vs-rigid-hardness"));
        nanoem_f32_t value = nanoemModelSoftBodyGetSoftVSRigidHardness(softBodyPtr);
        if (ImGui::InputFloat("##soft-vs-rigid-hardness", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetSoftVSRigidHardness(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.soft-vs-soft-hardness"));
        nanoem_f32_t value = nanoemModelSoftBodyGetSoftVSSoftHardness(softBodyPtr);
        if (ImGui::InputFloat("##soft-vs-soft-hardness", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetSoftVSSoftHardness(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.soft-vs-kinetic-impulse-split"));
        nanoem_f32_t value = nanoemModelSoftBodyGetSoftVSKineticImpulseSplit(softBodyPtr);
        if (ImGui::InputFloat("##soft-vs-kinetic-impulse-split", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetSoftVSKineticImpulseSplit(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.soft-vs-rigid-impulse-split"));
        nanoem_f32_t value = nanoemModelSoftBodyGetSoftVSRigidImpulseSplit(softBodyPtr);
        if (ImGui::InputFloat("##soft-vs-rigid-impulse-split", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetSoftVSRigidImpulseSplit(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.soft-vs-soft-impulse-split"));
        nanoem_f32_t value = nanoemModelSoftBodyGetSoftVSSoftImpulseSplit(softBodyPtr);
        if (ImGui::InputFloat("##soft-vs-kinetic-soft-split", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetSoftVSSoftImpulseSplit(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.linear-stiffness-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetLinearStiffnessCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##linear-stiffness-coefficient", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetLinearStiffnessCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.angular-stiffness-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetAngularStiffnessCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##angular-stiffness-coefficient", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetAngularStiffnessCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.volume-stiffness-coefficient"));
        nanoem_f32_t value = nanoemModelSoftBodyGetVolumeStiffnessCoefficient(softBodyPtr);
        if (ImGui::InputFloat("##volume-stiffness-coefficient", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetVolumeStiffnessCoefficient(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.bending-constraints-distance"));
        int value = nanoemModelSoftBodyGetBendingConstraintsDistance(softBodyPtr);
        if (ImGui::InputInt("##bending-constraints-distance", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetBendingConstraintsDistance(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.cluster-count"));
        int value = nanoemModelSoftBodyGetClusterCount(softBodyPtr);
        if (ImGui::InputInt("##cluster-count", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetClusterCount(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.velocity-solver-iterations"));
        int value = nanoemModelSoftBodyGetVelocitySolverIterations(softBodyPtr);
        if (ImGui::InputInt("##velocity-solver-iterations", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetVelocitySolverIterations(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.position-solver-iterations"));
        int value = nanoemModelSoftBodyGetVelocitySolverIterations(softBodyPtr);
        if (ImGui::InputInt("##position-solver-iterations", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetPositionsSolverIterations(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.drift-solver-iterations"));
        int value = nanoemModelSoftBodyGetDriftSolverIterations(softBodyPtr);
        if (ImGui::InputInt("##drift-solver-iterations", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetDriftSolverIterations(scoped, value);
        }
    }
    {
        ImGui::TextUnformatted(tr("nanoem.gui.model.edit.soft-body.cluster-solver-iterations"));
        int value = nanoemModelSoftBodyGetClusterSolverIterations(softBodyPtr);
        if (ImGui::InputInt("##cluster-solver-iterations", &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetClusterSolverIterations(scoped, value);
        }
    }
    {
        bool value = nanoemModelSoftBodyIsBendingConstraintsEnabled(softBodyPtr) != 0;
        if (ImGui::Checkbox(tr("nanoem.gui.model.edit.soft-body.bending-constraints-enabled"), &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetBendingConstraintsEnabled(scoped, value);
        }
    }
    {
        bool value = nanoemModelSoftBodyIsClustersEnabled(softBodyPtr) != 0;
        if (ImGui::Checkbox(tr("nanoem.gui.model.edit.soft-body.clusters-enabled"), &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
            nanoemMutableModelSoftBodySetClustersEnabled(scoped, value);
        }
    }
    {
        bool value = nanoemModelSoftBodyIsRandomizeConstraintsNeeded(softBodyPtr) != 0;
        if (ImGui::Checkbox(tr("nanoem.gui.model.edit.soft-body.randomize-constraints-enabled"), &value)) {
            command::ScopedMutableSoftBody scoped(softBodyPtr);
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
        ModelEditCommandDialog::beforeToggleEditingMode(convertToEditingType(m_tabType), m_activeModel, project);
        m_tabType = value;
        ModelEditCommandDialog::afterToggleEditingMode(convertToEditingType(value), m_activeModel, project);
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

void
ModelParameterDialog::setActiveModel(Model *model)
{
    const Project::DrawableList drawables(m_project->drawableOrderList());
    for (Project::DrawableList::const_iterator it = drawables.begin(), end = drawables.end(); it != end; ++it) {
        IDrawable *drawable = *it;
        drawable->setVisible(false);
    }
    if (Motion *motion = m_project->resolveMotion(model)) {
        Error error;
        SavedModelMotionMap::const_iterator it = m_savedModelMotions.find(model);
        if (it == m_savedModelMotions.end()) {
            ByteArray bytes;
            if (motion->save(bytes, model, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error)) {
                motion->clearAllKeyframes();
                m_savedModelMotions.insert(tinystl::make_pair(model, bytes));
            }
        }
    }
    model->setVisible(true);
    model->resetAllBoneTransforms();
    model->resetAllMaterials();
    model->resetAllMorphs();
    model->resetAllVertices();
    model->performAllBonesTransform();;
    model->updateStagingVertexBuffer();
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
