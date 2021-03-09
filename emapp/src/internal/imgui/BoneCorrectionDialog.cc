/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/BoneCorrectionDialog.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/Error.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const BoneCorrectionDialog::kIdentifier = "dialog.bone.correction";

BoneCorrectionDialog::BoneCorrectionDialog(Model *model, BaseApplicationService *applicationPtr)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_activeModel(model)
{
}

bool
BoneCorrectionDialog::draw(Project *project)
{
    bool visible = true;
    Model *currentActiveModel = project->activeModel();
    if (currentActiveModel != m_activeModel) {
        m_activeModel = currentActiveModel;
        visible = currentActiveModel != nullptr;
    }
    if (open(tr("nanoem.gui.window.bone.correction.title"), kIdentifier, &visible)) {
        ImGui::PushItemWidth(-1);
        ImGui::TextUnformatted(tr("nanoem.gui.window.bone.correction.translation"));
        ImGui::DragFloat("##translation.x.mul", &m_translation.m_mul.x, 1, 0, 0, " * %.2f");
        ImGui::SameLine();
        ImGui::DragFloat("##translation.x.add", &m_translation.m_add.x, 1, 0, 0, " + %.2f");
        ImGui::DragFloat("##translation.y.mul", &m_translation.m_mul.y, 1, 0, 0, " * %.2f");
        ImGui::SameLine();
        ImGui::DragFloat("##translation.y.add", &m_translation.m_add.y, 1, 0, 0, " + %.2f");
        ImGui::DragFloat("##translation.z.mul", &m_translation.m_mul.z, 1, 0, 0, " * %.2f");
        ImGui::SameLine();
        ImGui::DragFloat("##translation.z.add", &m_translation.m_add.z, 1, 0, 0, " + %.2f");
        ImGui::TextUnformatted(tr("nanoem.gui.window.bone.correction.orientation"));
        ImGui::DragFloat("##orientation.x.mul", &m_orientation.m_mul.x, 1, 0, 0, " * %.2f");
        ImGui::SameLine();
        ImGui::DragFloat("##orientation.x.add", &m_orientation.m_add.x, 1, 0, 0, " + %.2f");
        ImGui::DragFloat("##orientation.y.mul", &m_orientation.m_mul.y, 1, 0, 0, " * %.2f");
        ImGui::SameLine();
        ImGui::DragFloat("##orientation.y.add", &m_orientation.m_add.y, 1, 0, 0, " + %.2f");
        ImGui::DragFloat("##orientation.z.mul", &m_orientation.m_mul.z, 1, 0, 0, " * %.2f");
        ImGui::SameLine();
        ImGui::DragFloat("##orientation.z.add", &m_orientation.m_add.z, 1, 0, 0, " + %.2f");
        ImGui::PopItemWidth();
        switch (layoutCommonButtons(&visible)) {
        case kResponseTypeOK: {
            Error error;
            CommandRegistrator registrator(project);
            registrator.registerCorrectAllSelectedBoneKeyframesCommand(
                m_activeModel, m_translation, m_orientation, error);
            error.notify(project->eventPublisher());
            break;
        }
        default:
            break;
        }
    }
    close();
    return visible;
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
