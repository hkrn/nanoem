/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_SCALEALLSELECTEDKEYFRAMESDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_SCALEALLSELECTEDKEYFRAMESDIALOG_H_

#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct ScaleAllSelectedKeyframesDialog : BaseNonModalDialogWindow {
    static const char *const kIdentifier;
    ScaleAllSelectedKeyframesDialog(BaseApplicationService *applicationPtr);
    bool draw(Project *project);

    nanoem_f32_t m_scaleFactor;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_SCALEALLSELECTEDKEYFRAMESDIALOG_H_ */
