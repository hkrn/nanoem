/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/ModelDrawOrderDialog.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const ModelDrawOrderDialog::kIdentifier = "dialog.model.order.draw";

ModelDrawOrderDialog::ModelDrawOrderDialog(Project *project, BaseApplicationService *applicationPtr)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_lastDrawableOrderList(*project->drawableOrderList())
    , m_orderState(*project->drawableOrderList())
{
}

bool
ModelDrawOrderDialog::draw(Project *project)
{
    bool visible = true;
    const nanoem_f32_t height = ImGui::GetFrameHeightWithSpacing() * 14;
    if (open(tr("nanoem.gui.window.project.order.drawable.title"), kIdentifier, &visible, height)) {
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
        ImGui::BeginChild("##drawables", ImVec2(0, height - ImGui::GetFrameHeightWithSpacing() * 3.5f), true);
        const Project::DrawableList *drawables = project->drawableOrderList();
        for (Project::DrawableList::const_iterator it = drawables->begin(), end = drawables->end(); it != end; ++it) {
            IDrawable *drawable = *it;
            if (ImGui::Selectable(drawable->nameConstString(), drawable == m_orderState.m_checkedDrawable)) {
                m_orderState.m_checkedDrawable = drawable;
            }
        }
        ImGui::EndChild();
        if (changed) {
            project->setDrawableOrderList(m_orderState.m_currentDrawableList);
        }
        switch (layoutCommonButtons(&visible)) {
        case kResponseTypeOK: {
            project->setDrawableOrderList(m_orderState.m_currentDrawableList);
            break;
        }
        case kResponseTypeCancel: {
            project->setDrawableOrderList(m_lastDrawableOrderList);
            break;
        }
        default:
            break;
        }
    }
    else if (!visible) {
        project->setDrawableOrderList(m_lastDrawableOrderList);
    }
    close();
    return visible;
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
