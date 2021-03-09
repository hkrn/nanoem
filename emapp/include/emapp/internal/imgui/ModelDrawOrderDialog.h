/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_MODELDRAWORDERDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_MODELDRAWORDERDIALOG_H_

#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"
#include "emapp/internal/imgui/DrawOrderState.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct ModelDrawOrderDialog : BaseNonModalDialogWindow {
    static const char *const kIdentifier;

    ModelDrawOrderDialog(Project *project, BaseApplicationService *applicationPtr);
    bool draw(Project *project);

    Project::DrawableList m_lastDrawableOrderList;
    DrawOrderState m_orderState;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_MODELDRAWORDERDIALOG_H_ */
