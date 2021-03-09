/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_ACCESSORYOUTSIDEPARENTDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_ACCESSORYOUTSIDEPARENTDIALOG_H_

#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

namespace nanoem {

class Accessory;

namespace internal {
namespace imgui {

struct AccessoryOutsideParentDialog : BaseNonModalDialogWindow {
    static const char *const kIdentifier;

    AccessoryOutsideParentDialog(Accessory *accessory, BaseApplicationService *applicationPtr, ImGuiWindow *parent);
    bool draw(Project *project);
    void reset();

    ImGuiWindow *m_parent;
    Accessory *m_activeAccessory;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_ACCESSORYOUTSIDEPARENTDIALOG_H_ */
