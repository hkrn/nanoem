/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/ViewportSettingDialog.h"

#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const ViewportSettingDialog::kIdentifier = "dialog.project.viewport.size";

ViewportSettingDialog::ViewportSettingDialog(Project *project, BaseApplicationService *applicationPtr)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_currentViewportColor(project->viewportBackgroundColor())
    , m_lastViewportColor(m_currentViewportColor)
    , m_viewportImageSize(project->viewportImageSize())
    , m_viewportWithTransparent(project->isViewportWithTransparentEnabled())
{
}

bool
ViewportSettingDialog::draw(Project *project)
{
    bool visible = true;
    const nanoem_f32_t height = ImGui::GetFrameHeightWithSpacing() * 8;
    if (open(tr("nanoem.gui.window.project.viewport.title"), kIdentifier, &visible, height)) {
        ImGui::PushItemWidth(-1);
        ImGui::TextUnformatted(tr("nanoem.gui.window.project.viewport.size.title"));
        ImGui::DragInt2("##size", glm::value_ptr(m_viewportImageSize), 1.0f, 1, UINT16_MAX);
        ImGui::TextUnformatted(tr("nanoem.gui.window.project.viewport.size.preset"));
        if (ImGui::BeginCombo("##preset", tr("nanoem.gui.window.project.viewport.size.preset.from"))) {
            static const struct ViewportSizePreset {
                const char *m_name;
                Vector2SI32 m_size;
            } s_viewportSizePresets[] = { { "Default", Vector2SI32(640, 360) }, { "720p", Vector2SI32(1280, 720) },
                { "1080p", Vector2SI32(1920, 1080) }, { "4K", Vector2SI32(3840, 2160) } };
            for (nanoem_rsize_t i = 0; i < BX_COUNTOF(s_viewportSizePresets); i++) {
                const ViewportSizePreset &item = s_viewportSizePresets[i];
                if (ImGui::Selectable(item.m_name)) {
                    m_viewportImageSize = item.m_size;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::Spacing();
        ImGui::TextUnformatted(tr("nanoem.gui.window.project.viewport.color.title"));
        bool viewportWithTransparent = project->isViewportWithTransparentEnabled();
        const ImGuiColorEditFlags flags =
            viewportWithTransparent ? ImGuiColorEditFlags_AlphaBar : ImGuiColorEditFlags_NoAlpha;
        if (ImGui::ColorEdit4("##color", glm::value_ptr(m_currentViewportColor), flags)) {
            project->setViewportBackgroundColor(m_currentViewportColor);
        }
        if (ImGuiWindow::handleCheckBox(
                tr("nanoem.gui.window.project.viewport.transparent.title"), &viewportWithTransparent, true)) {
            project->setViewportWithTransparentEnabled(viewportWithTransparent);
        }
        ImGui::Spacing();
        switch (layoutCommonButtons(&visible)) {
        case kResponseTypeOK: {
            project->setViewportBackgroundColor(m_currentViewportColor);
            project->setViewportImageSize(m_viewportImageSize);
            break;
        }
        case kResponseTypeCancel: {
            project->setViewportBackgroundColor(m_lastViewportColor);
            project->setViewportWithTransparentEnabled(m_viewportWithTransparent);
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
