/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/ModelEditCommandDialog.h"

#include "emapp/Model.h"
#include "emapp/Project.h"

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

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
