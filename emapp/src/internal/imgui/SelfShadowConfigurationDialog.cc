/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/SelfShadowConfigurationDialog.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/ShadowCamera.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const SelfShadowConfigurationDialog::kIdentifier = "dialog.project.self-shadow";

SelfShadowConfigurationDialog::SelfShadowConfigurationDialog(
    BaseApplicationService *applicationPtr, ImGuiWindow *parent)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_parent(parent)
{
}

bool
SelfShadowConfigurationDialog::draw(Project *project)
{
    bool visible = true;
    const nanoem_f32_t height = ImGui::GetFrameHeightWithSpacing() * 6;
    if (open(tr("nanoem.gui.window.op.self-shadow"), kIdentifier, &visible, height)) {
        ImGui::PushItemWidth(-1);
        ImGui::TextUnformatted(tr("nanoem.gui.window.op.self-shadow.mode.title"));
        ShadowCamera *shadow = project->shadowCamera();
        const ShadowCamera::CoverageModeType mode = shadow->coverageMode();
        if (ImGui::RadioButton(
                tr("nanoem.gui.window.op.self-shadow.mode.none"), mode == ShadowCamera::kCoverageModeTypeNone)) {
            shadow->setCoverageMode(ShadowCamera::kCoverageModeTypeNone);
        }
        if (ImGui::RadioButton(
                tr("nanoem.gui.window.op.self-shadow.mode.type1"), mode == ShadowCamera::kCoverageModeType1)) {
            shadow->setCoverageMode(ShadowCamera::kCoverageModeType1);
        }
        if (ImGui::RadioButton(
                tr("nanoem.gui.window.op.self-shadow.mode.type2"), mode == ShadowCamera::kCoverageModeType2)) {
            shadow->setCoverageMode(ShadowCamera::kCoverageModeType2);
        }
        ImGui::TextUnformatted(tr("nanoem.gui.window.op.self-shadow.distance.title"));
        nanoem_f32_t distance = shadow->distance();
        if (ImGui::SliderFloat(
                "##distance", &distance, ShadowCamera::kMinimumDistance, ShadowCamera::kMaximumDistance, "%.0f")) {
            shadow->setDistance(distance);
        }
        ImGui::Spacing();
        if (m_parent->handleTranslatedButton("nanoem.gui.window.op.self-shadow.initialize", -1, true)) {
            shadow->reset();
        }
        addSeparator();
        if (m_parent->handleTranslatedButton("nanoem.gui.window.op.self-shadow.register", -1, true)) {
            CommandRegistrator registrator(project);
            registrator.registerAddSelfShadowKeyframesCommandByCurrentLocalFrameIndex();
        }
    }
    close();
    return visible;
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
