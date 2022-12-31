/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_CAMERACORRECTIONDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_CAMERACORRECTIONDIALOG_H_

#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct CameraCorrectionDialog : BaseNonModalDialogWindow {
    static const char *const kIdentifier;

    CameraCorrectionDialog(BaseApplicationService *applicationPtr);
    bool draw(Project *project);

    Motion::CorrectionVectorFactor m_lookAt;
    Motion::CorrectionVectorFactor m_angle;
    Motion::CorrectionScalarFactor m_distance;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_CAMERACORRECTIONDIALOG_H_ */
