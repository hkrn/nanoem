/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_MODELOUTSIDEPARENTDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_MODELOUTSIDEPARENTDIALOG_H_

#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct ModelOutsideParentDialog : BaseNonModalDialogWindow {
    static const char *const kIdentifier;

    ModelOutsideParentDialog(Project *project, BaseApplicationService *applicationPtr, ImGuiWindow *parent);
    bool draw(Project *project);
    void reset();

    ImGuiWindow *m_parent;
    Model *m_activeModel;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_MODELOUTSIDEPARENTDIALOG_H_ */
