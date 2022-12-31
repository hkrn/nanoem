/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_PREFERENCEDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_PREFERENCEDIALOG_H_

#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

#include "emapp/ApplicationMenuBuilder.h"
#include "emapp/ApplicationPreference.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct PreferenceDialog : BaseNonModalDialogWindow {
    static const char *const kIdentifier;
    PreferenceDialog(BaseApplicationService *applicationPtr, ImGuiWindow *parent);
    bool draw(Project *project);
    const char *selectedMenuItemString(Project *project, const ApplicationMenuBuilder::MenuItemType *items,
        nanoem_rsize_t numItems) const NANOEM_DECL_NOEXCEPT;
    void layoutPreferenceMenuItemCombo(
        Project *project, const ApplicationMenuBuilder::MenuItemType *items, nanoem_rsize_t numItems);
    const char *selectedPixelFormatString(sg_pixel_format value) const NANOEM_DECL_NOEXCEPT;
    const char *selectedLanguageString(ITranslator::LanguageType value) const NANOEM_DECL_NOEXCEPT;
    const char *selectedFPSMenuItemString(nanoem_u32_t value) const NANOEM_DECL_NOEXCEPT;
    const char *selectedHighDPIViewportModeString(
        ApplicationPreference::HighDPIViewportModeType value) const NANOEM_DECL_NOEXCEPT;
    const char *selectedFilePathModeMenuItemString(Project::FilePathMode value) const NANOEM_DECL_NOEXCEPT;

    ImGuiWindow *m_parent;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_PREFERENCEDIALOG_H_ */
