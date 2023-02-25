/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/ModelEditCommandDialog.h"

#include "emapp/Model.h"
#include "emapp/Project.h"
#include "emapp/model/IGizmo.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const ModelEditCommandDialog::kIdentifier = "dialog.model.edit";

void
ModelEditCommandDialog::beforeToggleEditingMode(
    IModelObjectSelection::ObjectType editingType, Model *activeModel, Project *project)
{
    activeModel->setShowAllVertexFaces(false);
    activeModel->setShowAllVertexPoints(false);
    activeModel->setShowAllVertexWeights(false);
    activeModel->setShowAllMaterialOverlays(false);
    activeModel->setShowAllBones(false);
    activeModel->setShowAllRigidBodyShapes(false);
    activeModel->setShowAllJointShapes(false);
    switch (editingType) {
    case IModelObjectSelection::kObjectTypeBone: {
        project->setEditingMode(Project::kEditingModeSelect);
        break;
    }
    case IModelObjectSelection::kObjectTypeMorph: {
        if (Motion *motion = project->resolveMotion(activeModel)) {
            activeModel->synchronizeMotion(
                motion, project->currentLocalFrameIndex(), 0, PhysicsEngine::kSimulationTimingBefore);
            activeModel->resetAllVertices();
            activeModel->deformAllMorphs(false);
            activeModel->markStagingVertexBufferDirty();
        }
        break;
    }
    default:
        break;
    }
}

void
ModelEditCommandDialog::afterToggleEditingMode(
    IModelObjectSelection::ObjectType editingType, Model *activeModel, Project *project)
{
    switch (editingType) {
    case IModelObjectSelection::kObjectTypeNull: {
        if (model::IGizmo *gizmo = activeModel->gizmo()) {
            gizmo->setPivotMatrix(Matrix4x4(0));
        }
        activeModel->selection()->setObjectType(editingType);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case IModelObjectSelection::kObjectTypeVertex: {
        activeModel->setShowAllVertexPoints(true);
        activeModel->selection()->setObjectType(editingType);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case IModelObjectSelection::kObjectTypeFace: {
        activeModel->setShowAllVertexFaces(true);
        activeModel->selection()->setObjectType(editingType);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case IModelObjectSelection::kObjectTypeMaterial: {
        activeModel->setShowAllMaterialOverlays(true);
        activeModel->selection()->setObjectType(editingType);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case IModelObjectSelection::kObjectTypeBone: {
        activeModel->selection()->setObjectType(editingType);
        activeModel->setShowAllBones(true);
        project->setEditingMode(Project::kEditingModeSelect);
        break;
    }
    case IModelObjectSelection::kObjectTypeRigidBody: {
        activeModel->setShowAllRigidBodyShapes(true);
        activeModel->selection()->setObjectType(editingType);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case IModelObjectSelection::kObjectTypeJoint: {
        activeModel->setShowAllJointShapes(true);
        activeModel->selection()->setObjectType(editingType);
        project->setEditingMode(Project::kEditingModeNone);
        break;
    }
    case IModelObjectSelection::kObjectTypeSoftBody: {
        activeModel->setShowAllVertexFaces(true);
        activeModel->selection()->setObjectType(editingType);
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

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
