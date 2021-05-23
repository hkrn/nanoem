/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/ModelEditCommandDialog.h"

#include "emapp/ICamera.h"
#include "emapp/ILight.h"
#include "emapp/Model.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#include "glm/gtx/matrix_query.hpp"

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
    case IModelObjectSelection::kEditingTypeMaterial: {
        activeModel->setShowAllMaterialOverlays(false);
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
        activeModel->setShowAllRigidBodyShapes(false);
        break;
    }
    case IModelObjectSelection::kEditingTypeJoint: {
        activeModel->setShowAllJointShapes(false);
        break;
    }
    case IModelObjectSelection::kEditingTypeSoftBody: {
        activeModel->setShowAllVertexFaces(false);
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
        activeModel->setShowAllMaterialOverlays(true);
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
        activeModel->setShowAllRigidBodyShapes(true);
        activeModel->selection()->setEditingType(editingType);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case IModelObjectSelection::kEditingTypeJoint: {
        activeModel->setShowAllJointShapes(true);
        activeModel->selection()->setEditingType(editingType);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case IModelObjectSelection::kEditingTypeSoftBody: {
        activeModel->setShowAllVertexFaces(true);
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
    char buffer[Inline::kNameStackBufferSize];
    bool visible = true;
    Model *currentActiveModel = project->activeModel();
    if (currentActiveModel != m_activeModel) {
        project->setEditingMode(Project::kEditingModeNone);
        m_activeModel = currentActiveModel;
        visible = currentActiveModel != nullptr;
    }
    const nanoem_f32_t height = ImGui::GetFrameHeightWithSpacing() * 3;
    if (open(tr("Model Commands"), kIdentifier, &visible, height)) {
        const bool available = project->isModelEditingEnabled(),
                   enabled = available && !glm::isNull(m_activeModel->pivotMatrix(), Constants::kEpsilon);
        if (ImGui::CollapsingHeader("Gizmo", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Operation Type");
            addGizmoOperationButton("Translate", Model::kGizmoOperationTypeTranslate, enabled);
            ImGui::SameLine();
            addGizmoOperationButton("Rotate", Model::kGizmoOperationTypeRotate, enabled);
            ImGui::SameLine();
            addGizmoOperationButton("Scale", Model::kGizmoOperationTypeScale, enabled);
            addSeparator();
            ImGui::Text("Coordinate Type");
            addGizmoCoordinationButton("Global", Model::kTransformCoordinateTypeGlobal, enabled);
            ImGui::SameLine();
            addGizmoCoordinationButton("Local", Model::kTransformCoordinateTypeLocal, enabled);
        }
        if (ImGui::CollapsingHeader("Selection Mode")) {
            addSelectionTargetButton("Circle", IModelObjectSelection::kTargetModeTypeCircle);
            ImGui::SameLine();
            addSelectionTargetButton("Rectangle", IModelObjectSelection::kTargetModeTypeRectangle);
            ImGui::SameLine();
            addSelectionTargetButton("Point", IModelObjectSelection::kTargetModeTypePoint);
        }
        if (ImGui::CollapsingHeader("Selection Type")) {
            addSelectionTypeButton("(none)", IModelObjectSelection::kEditingTypeNone, project);
            addSelectionTypeButton(
                tr("nanoem.gui.window.model.tab.vertex"), IModelObjectSelection::kEditingTypeVertex, project);
            addSelectionTypeButton(
                tr("nanoem.gui.window.model.tab.face"), IModelObjectSelection::kEditingTypeFace, project);
            addSelectionTypeButton(
                tr("nanoem.gui.window.model.tab.material"), IModelObjectSelection::kEditingTypeMaterial, project);
            addSelectionTypeButton(
                tr("nanoem.gui.window.model.tab.bone"), IModelObjectSelection::kEditingTypeBone, project);
            addSelectionTypeButton(
                tr("nanoem.gui.window.model.tab.rigid-body"), IModelObjectSelection::kEditingTypeRigidBody, project);
            addSelectionTypeButton(
                tr("nanoem.gui.window.model.tab.joint"), IModelObjectSelection::kEditingTypeJoint, project);
        }
        StringUtils::format(buffer, sizeof(buffer), "%s##camera", tr("nanoem.gui.panel.camera"));
        if (ImGui::CollapsingHeader(buffer)) {
            ICamera *camera = project->activeCamera();
            Vector3 lookAt(camera->lookAt()), angle(glm::degrees(camera->angle()));
            nanoem_f32_t distance = camera->distance();
            int fov = camera->fov();
            bool perspective = camera->isPerspective(), changed = false;
            ImGui::PushItemWidth(-1);
            ImGui::TextUnformatted(tr("nanoem.gui.viewport.parameter.camera.look-at"));
            if (ImGui::DragFloat3("##look-at", glm::value_ptr(lookAt))) {
                camera->setLookAt(glm::radians(lookAt));
                changed = true;
            }
            ImGui::TextUnformatted(tr("nanoem.gui.viewport.parameter.camera.angle"));
            if (ImGui::DragFloat3("##angle", glm::value_ptr(angle))) {
                camera->setAngle(glm::radians(angle));
                changed = true;
            }
            StringUtils::format(
                buffer, sizeof(buffer), "%s: %%.1f", tr("nanoem.gui.viewport.parameter.camera.distance"));
            if (ImGui::DragFloat("##distance", &distance, 0.1f, 0.0f, 0.0f, buffer)) {
                camera->setDistance(distance);
                changed = true;
            }
            if (ImGui::DragInt("##fov", &fov, 0.1f, 1, 135, tr("nanoem.gui.panel.camera.fov.format"))) {
                camera->setFov(fov);
                changed = true;
            }
            if (ImGui::Checkbox("Perspective##perspective", &perspective)) {
                camera->setPerspective(perspective);
                changed = true;
            }
            if (ImGui::Button("Initialize##initialize", ImVec2(-1, 0))) {
                camera->reset();
                changed = true;
            }
            ImGui::PopItemWidth();
            if (changed) {
                camera->update();
                project->resetAllModelEdges();
            }
        }
        StringUtils::format(buffer, sizeof(buffer), "%s##light", tr("nanoem.gui.panel.light"));
        if (ImGui::CollapsingHeader(buffer)) {
            ILight *light = project->activeLight();
            Vector3 color(light->color()), direction(light->direction());
            ImGui::PushItemWidth(-1);
            ImGui::TextUnformatted(tr("nanoem.gui.panel.light.color"));
            if (ImGui::ColorEdit3("##color", glm::value_ptr(color))) {
                light->setColor(color);
            }
            ImGui::TextUnformatted(tr("nanoem.gui.panel.light.direction"));
            if (ImGui::DragFloat3("##direction", glm::value_ptr(direction), 0.01f, -1.0f, 1.0f)) {
                light->setDirection(direction);
            }
            StringUtils::format(buffer, sizeof(buffer), "%s##initialize", tr("nanoem.gui.panel.light.reset"));
            if (ImGui::Button(buffer, ImVec2(-1, 0))) {
                light->reset();
            }
            ImGui::PopItemWidth();
        }
    }
    close();
    return visible;
}

void
ModelEditCommandDialog::addGizmoOperationButton(const char *text, Model::GizmoOperationType type, bool enabled)
{
    Model::GizmoOperationType op = m_activeModel->gizmoOperationType();
    if (ImGuiWindow::handleRadioButton(text, op == type, enabled)) {
        m_activeModel->setGizmoOperationType(type);
    }
}

void
ModelEditCommandDialog::addGizmoCoordinationButton(const char *text, Model::TransformCoordinateType type, bool enabled)
{
    Model::TransformCoordinateType coord = m_activeModel->gizmoTransformCoordinateType();
    if (ImGuiWindow::handleRadioButton(text, coord == type, enabled)) {
        m_activeModel->setGizmoTransformCoordinateType(type);
    }
}

void
ModelEditCommandDialog::addSelectionTypeButton(
    const char *text, IModelObjectSelection::EditingType type, Project *project)
{
    const IModelObjectSelection *selection = m_activeModel->selection();
    const IModelObjectSelection::EditingType editingType = selection->editingType();
    if (ImGui::RadioButton(text, editingType == type)) {
        beforeToggleEditingMode(editingType, m_activeModel, project);
        afterToggleEditingMode(type, m_activeModel, project);
    }
}

void
ModelEditCommandDialog::addSelectionTargetButton(const char *text, IModelObjectSelection::TargetModeType type)
{
    IModelObjectSelection *selection = m_activeModel->selection();
    const IModelObjectSelection::TargetModeType targetMode = selection->targetMode();
    if (ImGui::RadioButton(text, targetMode == type)) {
        selection->setTargetMode(type);
    }
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
