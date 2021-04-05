/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/ModelEditCommandDialog.h"

#include "emapp/Model.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const ModelEditCommandDialog::kIdentifier = "dialog.model.edit";

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
    }
    close();
    return visible;
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
