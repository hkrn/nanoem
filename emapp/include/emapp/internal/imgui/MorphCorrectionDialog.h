/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_MORPHCORRECTIONDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_MORPHCORRECTIONDIALOG_H_

#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct MorphCorrectionDialog : BaseNonModalDialogWindow {
    static const char *const kIdentifier;

    MorphCorrectionDialog(Model *model, BaseApplicationService *applicationPtr);
    bool draw(Project *project);

    Model *m_activeModel;
    Motion::CorrectionScalarFactor m_weight;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_MORPHCORRECTIONDIALOG_H_ */
