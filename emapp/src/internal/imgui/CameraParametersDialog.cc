/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/CameraParametersDialog.h"

#include "emapp/ICamera.h"
#include "emapp/command/UpdateCameraCommand.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const CameraParametersDialog::kIdentifier = "dialog.camera.parameters";

CameraParametersDialog::CameraParametersDialog(Project *project, BaseApplicationService *applicationPtr)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_camera(project->globalCamera())
    , m_lookAt(m_camera->lookAt())
    , m_angle(m_camera->angle())
    , m_distance(m_camera->distance())
{
}

bool
CameraParametersDialog::draw(Project *project)
{
    const nanoem_f32_t height = ImGui::GetFrameHeightWithSpacing() * 11;
    bool visible = true;
    if (open(tr("nanoem.gui.window.camera.parameter.title"), kIdentifier, &visible, height)) {
        Vector3 lookAt(m_camera->lookAt()), angle(glm::degrees(m_camera->angle()));
        ImGui::PushItemWidth(-1);
        ImGui::TextUnformatted(tr("nanoem.gui.window.camera.parameter.look-at"));
        bool lookAtChanged = false;
        lookAtChanged |= ImGui::DragFloat("##look.x", &lookAt.x, ImGuiWindow::kTranslationStepFactor, 0, 0, "X: %.3f");
        lookAtChanged |= ImGui::DragFloat("##look.y", &lookAt.y, ImGuiWindow::kTranslationStepFactor, 0, 0, "Y: %.3f");
        lookAtChanged |= ImGui::DragFloat("##look.z", &lookAt.z, ImGuiWindow::kTranslationStepFactor, 0, 0, "Z: %.3f");
        if (lookAtChanged) {
            m_camera->setLookAt(lookAt);
            m_camera->update();
            project->resetAllModelEdges();
        }
        ImGui::TextUnformatted(tr("nanoem.gui.window.camera.parameter.angle"));
        bool angleChanged = false;
        angleChanged |=
            ImGui::DragFloat("##angle.x", &angle.x, ImGuiWindow::kOrientationStepFactor, -180, 180, "X: %.1f");
        angleChanged |=
            ImGui::DragFloat("##angle.y", &angle.y, ImGuiWindow::kOrientationStepFactor, -90, 90, "Y: %.1f");
        angleChanged |=
            ImGui::DragFloat("##angle.z", &angle.z, ImGuiWindow::kOrientationStepFactor, -180, 180, "Z: %.1f");
        if (angleChanged) {
            m_camera->setAngle(glm::radians(angle));
            m_camera->update();
        }
        ImGui::TextUnformatted(tr("nanoem.gui.window.camera.parameter.distance"));
        nanoem_f32_t distance = m_camera->distance();
        if (ImGui::DragFloat("##distance", &distance, ImGuiWindow::kTranslationStepFactor)) {
            m_camera->setDistance(distance);
            m_camera->update();
            project->resetAllModelEdges();
        }
        ImGui::PopItemWidth();
        ImGui::Spacing();
        switch (layoutCommonButtons(&visible)) {
        case kResponseTypeOK: {
            project->pushUndo(command::UpdateCameraCommand::create(
                project, m_camera, lookAt, angle, distance, m_camera->fovRadians(), m_camera->isPerspective()));
            break;
        }
        case kResponseTypeCancel: {
            reset(project);
            break;
        }
        default:
            break;
        }
    }
    else if (!visible) {
        reset(project);
    }
    close();
    return visible;
}

void
CameraParametersDialog::reset(Project *project)
{
    m_camera->setLookAt(m_lookAt);
    m_camera->setAngle(m_angle);
    m_camera->setDistance(m_distance);
    m_camera->update();
    project->resetAllModelEdges();
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
