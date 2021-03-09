/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/ModelTransformOrderDialog.h"

#include "emapp/Model.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const ModelTransformOrderDialog::kIdentifier = "dialog.model.order.transform";

ModelTransformOrderDialog::ModelTransformOrderDialog(Project *project, BaseApplicationService *applicationPtr)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_lastTransformOrderList(project->transformOrderList())
    , m_orderState(m_lastTransformOrderList)
{
}

bool
ModelTransformOrderDialog::draw(Project *project)
{
    bool visible = true;
    const nanoem_f32_t height = ImGui::GetFrameHeightWithSpacing() * 14;
    if (open(tr("nanoem.gui.window.project.order.transform.title"), kIdentifier, &visible, height)) {
        bool changed = false;
        if (ImGuiWindow::handleButton("Up", ImGui::GetContentRegionAvail().x * 0.5f, !m_orderState.isOrderBegin())) {
            m_orderState.setOrderAt(-1);
            changed = true;
        }
        ImGui::SameLine();
        if (ImGuiWindow::handleButton("Down", ImGui::GetContentRegionAvail().x, !m_orderState.isOrderEnd())) {
            m_orderState.setOrderAt(1);
            changed = true;
        }
        ImGui::BeginChild("##models", ImVec2(0, height - ImGui::GetFrameHeightWithSpacing() * 3.5f), true);
        Project::ModelList models(project->transformOrderList());
        for (Project::ModelList::const_iterator it = models.begin(), end = models.end(); it != end; ++it) {
            Model *model = *it;
            if (ImGui::Selectable(model->nameConstString(), m_orderState.m_checkedTransformModel == model)) {
                m_orderState.m_checkedTransformModel = model;
            }
        }
        ImGui::EndChild();
        if (changed) {
            project->setTransformOrderList(m_orderState.m_currentTransformModelList);
        }
        switch (layoutCommonButtons(&visible)) {
        case kResponseTypeOK: {
            project->setTransformOrderList(m_orderState.m_currentTransformModelList);
            break;
        }
        case kResponseTypeCancel: {
            project->setTransformOrderList(m_lastTransformOrderList);
            break;
        }
        default:
            break;
        }
    }
    else if (!visible) {
        project->setTransformOrderList(m_lastTransformOrderList);
    }
    close();
    return visible;
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
