/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_BASENONMODALDIALOGWINDOW_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_BASENONMODALDIALOGWINDOW_H_

#include "emapp/internal/ImGuiWindow.h"

namespace nanoem {
namespace internal {
namespace imgui {

class BaseNonModalDialogWindow : public ImGuiWindow::INoModalDialogWindow {
public:
    enum ResponseType {
        kResponseTypeIndetermine,
        kResponseTypeOK,
        kResponseTypeCancel,
    };
    static ImVec2 calcExpandedImageSize(int width, int height, nanoem_f32_t scaleFactor) NANOEM_DECL_NOEXCEPT;
    static ImVec2 calcExpandedImageSize(const sg_image_desc &desc, nanoem_f32_t scaleFactor) NANOEM_DECL_NOEXCEPT;
    static void detectUpDown(bool &up, bool &down) NANOEM_DECL_NOEXCEPT;
    static void selectIndex(
        bool up, bool down, const nanoem_rsize_t numObjects, nanoem_rsize_t &offset) NANOEM_DECL_NOEXCEPT;
    static void selectIndex(bool up, bool down, const nanoem_rsize_t numObjects, int &offset) NANOEM_DECL_NOEXCEPT;
    static void addSeparator();

    BaseNonModalDialogWindow(BaseApplicationService *applicationPtr);
    ~BaseNonModalDialogWindow() NANOEM_DECL_NOEXCEPT;

    virtual void destroy(Project *project);

    bool open(const char *title, const char *id, bool *visible, const ImVec2 size,
        ImGuiWindowFlags flagss = ImGuiWindowFlags_None, ImGuiSizeCallback sizeCB = nullptr,
        void *sizeCBOpaque = nullptr);
    bool open(const char *title, const char *id, bool *visible, nanoem_f32_t height = 0,
        ImGuiWindowFlags flags = ImGuiWindowFlags_None, ImGuiSizeCallback sizeCB = nullptr,
        void *sizeCBOpaque = nullptr);
    void close();

    ResponseType layoutCommonButtons(bool *visible);
    const BaseApplicationService *application() const NANOEM_DECL_NOEXCEPT;
    BaseApplicationService *application() NANOEM_DECL_NOEXCEPT;
    const char *tr(const char *text) const NANOEM_DECL_NOEXCEPT;

private:
    BaseApplicationService *m_applicationPtr;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_BASENONMODALDIALOGWINDOW_H_ */
