/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_BONECORRECTIONDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_BONECORRECTIONDIALOG_H_

#include "emapp/Motion.h"
#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

namespace nanoem {

class Model;

namespace internal {
namespace imgui {

struct BoneCorrectionDialog : BaseNonModalDialogWindow {
    static const char *const kIdentifier;

    BoneCorrectionDialog(Model *model, BaseApplicationService *applicationPtr);
    bool draw(Project *project);

    Model *m_activeModel;
    Motion::CorrectionVectorFactor m_translation;
    Motion::CorrectionVectorFactor m_orientation;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_BONECORRECTIONDIALOG_H_ */
