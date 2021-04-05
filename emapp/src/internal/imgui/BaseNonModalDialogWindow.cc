/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace internal {
namespace imgui {

ImVec2
BaseNonModalDialogWindow::calcExpandedImageSize(int width, int height) NANOEM_DECL_NOEXCEPT
{
    const ImGuiStyle &style = ImGui::GetStyle();
    const nanoem_f32_t avail = ImGui::GetContentRegionAvail().x - (style.IndentSpacing + style.FramePadding.x);
    ImVec2 size;
    if (width > avail) {
        size = height > avail ? ImVec2(avail, avail) : ImVec2(avail, height * 1.0f);
    }
    else {
        size = ImVec2(width * 1.0f, height * 1.0f);
    }
    return size;
}

ImVec2
BaseNonModalDialogWindow::calcExpandedImageSize(const sg_image_desc &desc) NANOEM_DECL_NOEXCEPT
{
    return calcExpandedImageSize(desc.width, desc.height);
}

void
BaseNonModalDialogWindow::detectUpDown(bool &up, bool &down) NANOEM_DECL_NOEXCEPT
{
    const ImGuiIO &io = ImGui::GetIO();
    const int keyUpIndex = ImGui::GetKeyIndex(ImGuiKey_UpArrow), keyDownIndex = ImGui::GetKeyIndex(ImGuiKey_DownArrow);
    const bool focused = ImGui::IsWindowFocused();
    up = focused && ImGui::GetKeyPressedAmount(keyUpIndex, io.KeyRepeatDelay, 0.02f) > 0,
    down = focused && ImGui::GetKeyPressedAmount(keyDownIndex, io.KeyRepeatDelay, 0.02f) > 0;
}

void
BaseNonModalDialogWindow::selectIndex(
    bool up, bool down, const nanoem_rsize_t numObjects, nanoem_rsize_t &offset) NANOEM_DECL_NOEXCEPT
{
    if (up) {
        offset = offset > 0 ? offset - 1 : 0;
    }
    else if (down) {
        const nanoem_rsize_t maxOffset = numObjects - 1;
        offset = offset < maxOffset ? offset + 1 : maxOffset;
    }
}

void
BaseNonModalDialogWindow::selectIndex(
    bool up, bool down, const nanoem_rsize_t numObjects, int &offset) NANOEM_DECL_NOEXCEPT
{
    if (up) {
        offset = offset > 0 ? offset - 1 : 0;
    }
    else if (down) {
        const int maxOffset = Inline::saturateInt32(numObjects) - 1;
        offset = offset >= 0 ? (offset < maxOffset ? offset + 1 : maxOffset) : 0;
    }
}

void
BaseNonModalDialogWindow::addSeparator()
{
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

BaseNonModalDialogWindow::BaseNonModalDialogWindow(BaseApplicationService *applicationPtr)
    : m_applicationPtr(applicationPtr)
{
}

BaseNonModalDialogWindow::~BaseNonModalDialogWindow() NANOEM_DECL_NOEXCEPT
{
}

bool
BaseNonModalDialogWindow::open(
    const char *title, const char *id, bool *visible, const ImVec2 size, ImGuiWindowFlags flags)
{
    char buffer[Inline::kNameStackBufferSize];
    StringUtils::format(buffer, sizeof(buffer), "%s##%s", title, id);
    ImGui::SetNextWindowSizeConstraints(size, ImVec2(FLT_MAX, FLT_MAX));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, ImGuiWindow::kWindowRounding);
    return ImGui::Begin(buffer, visible, flags) && *visible;
}

bool
BaseNonModalDialogWindow::open(
    const char *title, const char *id, bool *visible, nanoem_f32_t height, ImGuiWindowFlags flags)
{
    char buffer[Inline::kNameStackBufferSize];
    StringUtils::format(buffer, sizeof(buffer), "%s##%s", title, id);
    const ImGuiStyle &style = ImGui::GetStyle();
    const nanoem_f32_t textWidth = ImGui::CalcTextSize(title).x + (style.WindowPadding.x + style.WindowRounding) * 2;
    const nanoem_f32_t width = glm::max(textWidth, 250.0f * ImGui::GetIO().DisplayFramebufferScale.x);
    ImGui::SetNextWindowSizeConstraints(ImVec2(width, height), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, style.WindowRounding);
    return ImGui::Begin(buffer, visible, flags) && *visible;
}

void
BaseNonModalDialogWindow::close()
{
    ImGui::End();
    ImGui::PopStyleVar();
}

BaseNonModalDialogWindow::ResponseType
BaseNonModalDialogWindow::layoutCommonButtons(bool *visible)
{
    ResponseType type = kResponseTypeIndetermine;
    nanoem_f32_t width = ImGui::GetContentRegionAvail().x;
    ImGui::Indent(width * 0.2f);
    if (ImGuiWindow::handleButton("OK", width * 0.3f, true)) {
        type = kResponseTypeOK;
        *visible = false;
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton("Cancel", width * 0.3f, true)) {
        type = kResponseTypeCancel;
        *visible = false;
    }
    ImGui::Unindent();
    return type;
}

const char *
BaseNonModalDialogWindow::tr(const char *text) const NANOEM_DECL_NOEXCEPT
{
    return m_applicationPtr->translator()->translate(text);
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
