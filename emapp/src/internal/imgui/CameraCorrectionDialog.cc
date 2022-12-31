/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/CameraCorrectionDialog.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/Error.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const CameraCorrectionDialog::kIdentifier = "dialog.camera.correction";

CameraCorrectionDialog::CameraCorrectionDialog(BaseApplicationService *applicationPtr)
    : BaseNonModalDialogWindow(applicationPtr)
{
}

bool
CameraCorrectionDialog::draw(Project *project)
{
    bool visible = true;
    if (open(tr("nanoem.gui.window.camera.correction.title"), kIdentifier, &visible)) {
        ImGui::TextUnformatted(tr("nanoem.gui.window.camera.correction.look-at"));
        ImGui::Columns(4, nullptr, false);
        {
            ImGui::Spacing();
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##look.x.mul", &m_lookAt.m_mul.x, 1, 0, 0, " * %.2f");
            ImGui::PopItemWidth();
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##look.x.add", &m_lookAt.m_add.x, 1, 0, 0, " + %.2f");
            ImGui::PopItemWidth();
            ImGui::NextColumn();
            ImGui::Spacing();
            ImGui::NextColumn();
        }
        {
            ImGui::Spacing();
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##look.y.mul", &m_lookAt.m_mul.y, 1, 0, 0, " * %.2f");
            ImGui::PopItemWidth();
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##look.y.add", &m_lookAt.m_add.y, 1, 0, 0, " + %.2f");
            ImGui::PopItemWidth();
            ImGui::NextColumn();
            ImGui::Spacing();
            ImGui::NextColumn();
        }
        {
            ImGui::Spacing();
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##look.z.mul", &m_lookAt.m_mul.z, 1, 0, 0, " * %.2f");
            ImGui::PopItemWidth();
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##look.z.add", &m_lookAt.m_add.z, 1, 0, 0, " + %.2f");
            ImGui::PopItemWidth();
            ImGui::NextColumn();
            ImGui::Spacing();
            ImGui::NextColumn();
        }
        ImGui::Columns(1, nullptr, false);
        ImGui::TextUnformatted(tr("nanoem.gui.window.camera.correction.angle"));
        ImGui::Columns(4, nullptr, false);
        {
            ImGui::Spacing();
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##angle.x.mul", &m_angle.m_mul.x, 1, 0, 0, " * %.2f");
            ImGui::PopItemWidth();
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##angle.x.add", &m_angle.m_add.x, 1, 0, 0, " + %.2f");
            ImGui::PopItemWidth();
            ImGui::NextColumn();
            ImGui::Spacing();
            ImGui::NextColumn();
        }
        {
            ImGui::Spacing();
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##angle.y.mul", &m_angle.m_mul.y, 1, 0, 0, " * %.2f");
            ImGui::PopItemWidth();
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##angle.y.add", &m_angle.m_add.y, 1, 0, 0, " + %.2f");
            ImGui::PopItemWidth();
            ImGui::NextColumn();
            ImGui::Spacing();
            ImGui::NextColumn();
        }
        {
            ImGui::Spacing();
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##angle.z.mul", &m_angle.m_mul.z, 1, 0, 0, " * %.2f");
            ImGui::PopItemWidth();
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##angle.z.add", &m_angle.m_add.z, 1, 0, 0, " + %.2f");
            ImGui::PopItemWidth();
            ImGui::NextColumn();
            ImGui::Spacing();
            ImGui::NextColumn();
        }
        ImGui::Columns(1, nullptr, false);
        ImGui::TextUnformatted(tr("nanoem.gui.window.camera.correction.distance"));
        ImGui::Columns(4, nullptr, false);
        {
            ImGui::Spacing();
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##distance.mul", &m_distance.m_mul, 1, 0, 0, " * %.2f");
            ImGui::PopItemWidth();
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##distance.z.add", &m_distance.m_add, 1, 0, 0, " + %.2f");
            ImGui::PopItemWidth();
            ImGui::NextColumn();
            ImGui::Spacing();
            ImGui::NextColumn();
        }
        ImGui::Spacing();
        ImGui::NextColumn();
        if (ImGuiWindow::handleButton("OK", -1, true)) {
            Error error;
            CommandRegistrator registrator(project);
            registrator.registerCorrectAllSelectedCameraKeyframesCommand(m_lookAt, m_angle, m_distance, error);
            error.notify(project->eventPublisher());
            visible = false;
        }
        ImGui::NextColumn();
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
