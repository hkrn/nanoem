/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_VIEWPORTSETTINGDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_VIEWPORTSETTINGDIALOG_H_

#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct ViewportSettingDialog : BaseNonModalDialogWindow {
    static const char *const kIdentifier;

    ViewportSettingDialog(Project *project, BaseApplicationService *applicationPtr);
    bool draw(Project *project);

    Vector4 m_currentViewportColor;
    Vector4 m_lastViewportColor;
    Vector2SI32 m_viewportImageSize;
    bool m_viewportWithTransparent;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_VIEWPORTSETTINGDIALOG_H_ */
