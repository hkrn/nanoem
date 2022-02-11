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
BaseNonModalDialogWindow::calcExpandedImageSize(int width, int height, nanoem_f32_t scaleFactor) NANOEM_DECL_NOEXCEPT
{
    const ImGuiStyle &style = ImGui::GetStyle();
    const nanoem_f32_t avail = ImGui::GetContentRegionAvail().x - (style.IndentSpacing + style.FramePadding.x),
                       inverseAspectRatio = (height * 1.0f) / (width * 1.0f);
    ImVec2 size;
    if (width > avail) {
        size = height > avail ? ImVec2(avail * scaleFactor, avail * inverseAspectRatio * scaleFactor)
                              : ImVec2(avail * scaleFactor, height * inverseAspectRatio * scaleFactor);
    }
    else {
        size = ImVec2(width * scaleFactor, height * inverseAspectRatio * scaleFactor);
    }
    return size;
}

ImVec2
BaseNonModalDialogWindow::calcExpandedImageSize(
    const sg_image_desc &desc, nanoem_f32_t scaleFactor) NANOEM_DECL_NOEXCEPT
{
    return calcExpandedImageSize(desc.width, desc.height, scaleFactor);
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

void
BaseNonModalDialogWindow::destroy(Project *project)
{
    BX_UNUSED_1(project);
}

bool
BaseNonModalDialogWindow::open(const char *title, const char *id, bool *visible, const ImVec2 size,
    ImGuiWindowFlags flags, ImGuiSizeCallback sizeCB, void *sizeCBOpaque)
{
    char buffer[Inline::kNameStackBufferSize];
    const float scaleFactor = ImGui::GetIO().DisplayFramebufferScale.x;
    StringUtils::format(buffer, sizeof(buffer), "%s##%s", title, id);
    ImGui::SetNextWindowSizeConstraints(size, ImVec2(FLT_MAX, FLT_MAX), sizeCB, sizeCBOpaque);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, ImGuiWindow::kWindowRounding * scaleFactor);
    return ImGui::Begin(buffer, visible, flags) && *visible;
}

bool
BaseNonModalDialogWindow::open(const char *title, const char *id, bool *visible, nanoem_f32_t height,
    ImGuiWindowFlags flags, ImGuiSizeCallback sizeCB, void *sizeCBOpaque)
{
    char buffer[Inline::kNameStackBufferSize];
    StringUtils::format(buffer, sizeof(buffer), "%s##%s", title, id);
    const ImGuiStyle &style = ImGui::GetStyle();
    const nanoem_f32_t scaleFactor = ImGui::GetIO().DisplayFramebufferScale.x,
                       windowRounding = ImGuiWindow::kWindowRounding * scaleFactor,
                       textWidth = ImGui::CalcTextSize(title).x + (style.WindowPadding.x + windowRounding) * 2,
                       width = glm::max(textWidth, 250.0f * scaleFactor),
                       minHeight = height + style.WindowPadding.y * 2;
    ImGui::SetNextWindowSizeConstraints(ImVec2(width, minHeight), ImVec2(FLT_MAX, FLT_MAX), sizeCB, sizeCBOpaque);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, windowRounding);
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

const BaseApplicationService *
BaseNonModalDialogWindow::application() const NANOEM_DECL_NOEXCEPT
{
    return m_applicationPtr;
}

BaseApplicationService *
BaseNonModalDialogWindow::application() NANOEM_DECL_NOEXCEPT
{
    return m_applicationPtr;
}

const char *
BaseNonModalDialogWindow::tr(const char *text) const NANOEM_DECL_NOEXCEPT
{
    return m_applicationPtr->translator()->translate(text);
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
