/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/ModelEdgeDialog.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/Model.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const ModelEdgeDialog::kIdentifier = "dialog.model.edge";

ModelEdgeDialog::ModelEdgeDialog(Model *model, Project *project, BaseApplicationService *applicationPtr)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_activeModel(model)
    , m_color(model->edgeColor())
    , m_scaleFactor(model->edgeSizeScaleFactor())
    , m_editingMode(project->editingMode())
{
    project->setEditingMode(Project::kEditingModeNone);
}

bool
ModelEdgeDialog::draw(Project *project)
{
    bool visible = true;
    Model *currentActiveModel = project->activeModel();
    if (currentActiveModel != m_activeModel) {
        reset(project);
        project->setEditingMode(Project::kEditingModeNone);
        m_activeModel = currentActiveModel;
        visible = currentActiveModel != nullptr;
    }
    if (open(tr("nanoem.gui.window.model.edge.title"), kIdentifier, &visible)) {
        ImGui::PushItemWidth(-1);
        ImGui::TextUnformatted(tr("nanoem.gui.window.model.edge.color"));
        Vector4 color(m_activeModel->edgeColor());
        if (ImGui::ColorEdit4("##color", glm::value_ptr(color))) {
            m_activeModel->setEdgeColor(color);
            m_activeModel->markStagingVertexBufferDirty();
        }
        ImGui::TextUnformatted(tr("nanoem.gui.window.model.edge.size"));
        nanoem_f32_t scaleFactor = m_activeModel->edgeSizeScaleFactor();
        if (ImGui::SliderFloat("##size", &scaleFactor, 0.0f, 2.0f)) {
            m_activeModel->setEdgeSizeScaleFactor(scaleFactor);
            m_activeModel->markStagingVertexBufferDirty();
        }
        ImGui::Spacing();
        if (ImGuiWindow::handleButton(
                tr("nanoem.gui.window.model.edge.register"), ImGui::GetContentRegionAvail().x, true)) {
            CommandRegistrator registrator(project);
            registrator.registerAddModelKeyframesCommandByCurrentLocalFrameIndex(m_activeModel);
        }
        ImGui::Spacing();
        switch (layoutCommonButtons(&visible)) {
        case kResponseTypeOK: {
            project->setEditingMode(m_editingMode);
            break;
        }
        case kResponseTypeCancel: {
            reset(project);
            break;
        }
        default:
            break;
        }
        ImGui::PopItemWidth();
    }
    else if (!visible) {
        reset(project);
    }
    close();
    return visible;
}

void
ModelEdgeDialog::reset(Project *project)
{
    if (m_activeModel) {
        m_activeModel->setEdgeColor(m_color);
        m_activeModel->setEdgeSizeScaleFactor(m_scaleFactor);
    }
    project->setEditingMode(m_editingMode);
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
