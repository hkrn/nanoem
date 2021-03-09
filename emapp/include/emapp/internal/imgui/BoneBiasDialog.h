/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_BONEBIASDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_BONEBIASDIALOG_H_

#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

namespace nanoem {

class Model;

namespace internal {
namespace imgui {

struct BoneBiasDialog : BaseNonModalDialogWindow {
    static const char *const kIdentifier;

    BoneBiasDialog(Model *model, BaseApplicationService *applicationPtr);
    bool draw(Project *project);

    Model *m_activeModel;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_BONEBIASDIALOG_H_ */
