/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/ScaleAllSelectedKeyframesDialog.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/Error.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const ScaleAllSelectedKeyframesDialog::kIdentifier = "dialog.project.scale-all-keyframes";

ScaleAllSelectedKeyframesDialog::ScaleAllSelectedKeyframesDialog(BaseApplicationService *applicationPtr)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_scaleFactor(1.0f)
{
}

bool
ScaleAllSelectedKeyframesDialog::draw(Project *project)
{
    BX_UNUSED_1(project);
    bool visible = true;
    if (open("", kIdentifier, &visible)) {
        const TimelineSegment &segment = project->selectionSegment();
        nanoem_frame_index_t from = segment.m_from, to = segment.m_to;
        ImGui::PushItemWidth(-1);
        ImGui::TextUnformatted("Target Range");
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 2.0f);
        ImGui::InputScalar(
            "##from", ImGuiWindow::kFrameIndexDataType, &from, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputScalar(
            "##to", ImGuiWindow::kFrameIndexDataType, &to, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
        ImGui::PopItemWidth();
        ImGui::TextUnformatted("Scale Factor");
        ImGui::DragFloat("##size", &m_scaleFactor, 0.1f, 0.1f, 10.0f, "%.1f");
        ImGui::Spacing();
        ImGui::TextUnformatted("* Camera keyframe cannot be scaled due to scene change by adjacent keyframe");
        ImGui::Spacing();
        switch (layoutCommonButtons(&visible)) {
        case kResponseTypeOK: {
            Error error;
            CommandRegistrator registrator(project);
            registrator.registerScaleAllMotionKeyframesInCommand(
                segment, m_scaleFactor, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
            error.notify(project->eventPublisher());
            break;
        }
        case kResponseTypeCancel: {
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
