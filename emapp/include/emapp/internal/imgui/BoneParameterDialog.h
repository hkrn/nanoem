/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_BONEPARAMETERDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_BONEPARAMETERDIALOG_H_

#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct BoneParameterDialog : BaseNonModalDialogWindow {
    static const char *const kIdentifier;

    BoneParameterDialog(Model *model, BaseApplicationService *applicationPtr);
    bool draw(Project *project);
    void reset();

    Model *m_activeModel;
    model::BindPose m_bindPose;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_BONEPARAMETERDIALOG_H_ */
