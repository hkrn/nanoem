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
    if (open(tr("nanoem.gui.window.model.edit.command.title"), kIdentifier, &visible)) {
        if (ImGui::CollapsingHeader("Gizmo")) {
            Model::GizmoOperationType op = m_activeModel->gizmoOperationType();
            if (ImGui::RadioButton("Translate", op == Model::kGizmoOperationTypeTranslate)) {
                m_activeModel->setGizmoOperationType(Model::kGizmoOperationTypeTranslate);
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Rotate", op == Model::kGizmoOperationTypeRotate)) {
                m_activeModel->setGizmoOperationType(Model::kGizmoOperationTypeRotate);
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Scale", op == Model::kGizmoOperationTypeScale)) {
                m_activeModel->setGizmoOperationType(Model::kGizmoOperationTypeScale);
            }
            ImGui::Separator();
            Model::TransformCoordinateType coord = m_activeModel->gizmoTransformCoordinateType();
            if (ImGui::RadioButton("Global", coord == Model::kTransformCoordinateTypeGlobal)) {
                m_activeModel->setGizmoTransformCoordinateType(Model::kTransformCoordinateTypeGlobal);
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Local", coord == Model::kTransformCoordinateTypeLocal)) {
                m_activeModel->setGizmoTransformCoordinateType(Model::kTransformCoordinateTypeLocal);
            }
        }
        if (ImGui::CollapsingHeader("Selection")) {
            IModelObjectSelection *selection = m_activeModel->selection();
            IModelObjectSelection::EditingType editingType = selection->editingType();
            if (ImGui::RadioButton("Vertex", editingType == IModelObjectSelection::kEditingTypeVertex)) {
                beforeToggleEditingMode(editingType, m_activeModel, project);
                afterToggleEditingMode(IModelObjectSelection::kEditingTypeVertex, m_activeModel, project);
            }
            if (ImGui::RadioButton("Face", editingType == IModelObjectSelection::kEditingTypeFace)) {
                beforeToggleEditingMode(editingType, m_activeModel, project);
                afterToggleEditingMode(IModelObjectSelection::kEditingTypeFace, m_activeModel, project);
            }
            if (ImGui::RadioButton("Material", editingType == IModelObjectSelection::kEditingTypeMaterial)) {
                beforeToggleEditingMode(editingType, m_activeModel, project);
                afterToggleEditingMode(IModelObjectSelection::kEditingTypeMaterial, m_activeModel, project);
            }
            if (ImGui::RadioButton("Bone", editingType == IModelObjectSelection::kEditingTypeBone)) {
                beforeToggleEditingMode(editingType, m_activeModel, project);
                afterToggleEditingMode(IModelObjectSelection::kEditingTypeBone, m_activeModel, project);
            }
            if (ImGui::RadioButton("RigidBody", editingType == IModelObjectSelection::kEditingTypeRigidBody)) {
                beforeToggleEditingMode(editingType, m_activeModel, project);
                afterToggleEditingMode(IModelObjectSelection::kEditingTypeRigidBody, m_activeModel, project);
            }
            if (ImGui::RadioButton("Joint", editingType == IModelObjectSelection::kEditingTypeJoint)) {
                beforeToggleEditingMode(editingType, m_activeModel, project);
                afterToggleEditingMode(IModelObjectSelection::kEditingTypeJoint, m_activeModel, project);
            }
        }
    }
    close();
    return visible;
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
