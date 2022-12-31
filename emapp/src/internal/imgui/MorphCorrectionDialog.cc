/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/MorphCorrectionDialog.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/Error.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const MorphCorrectionDialog::kIdentifier = "dialog.morph.correction";

MorphCorrectionDialog::MorphCorrectionDialog(Model *model, BaseApplicationService *applicationPtr)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_activeModel(model)
{
}

bool
MorphCorrectionDialog::draw(Project *project)
{
    const nanoem_f32_t height = ImGui::GetFrameHeightWithSpacing() * 3;
    bool visible = true;
    Model *currentActiveModel = project->activeModel();
    if (currentActiveModel != m_activeModel) {
        m_activeModel = currentActiveModel;
        visible = currentActiveModel != nullptr;
    }
    if (open(tr("nanoem.gui.window.morph.correction.title"), kIdentifier, &visible, height)) {
        ImGui::PushItemWidth(-1);
        ImGui::TextUnformatted(tr("nanoem.gui.window.morph.correction.weight"));
        ImGui::DragFloat("##weight.mul", &m_weight.m_mul, 1, 0, 0, " * %.2f");
        ImGui::SameLine();
        ImGui::DragFloat("##weight.add", &m_weight.m_add, 1, 0, 0, " + %.2f");
        ImGui::PopItemWidth();
        if (ImGuiWindow::handleButton("OK", -1, true)) {
            Error error;
            CommandRegistrator registrator(project);
            registrator.registerCorrectAllSelectedMorphKeyframesCommand(m_activeModel, m_weight, error);
            error.notify(project->eventPublisher());
            visible = false;
        }
        ImGui::SameLine();
        if (ImGuiWindow::handleButton("Cancel", -1, true)) {
            visible = false;
        }
    }
    close();
    return visible;
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
