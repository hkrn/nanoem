/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/BoneBiasDialog.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const BoneBiasDialog::kIdentifier = "dialog.bone.bias";

BoneBiasDialog::BoneBiasDialog(Model *model, BaseApplicationService *applicationPtr)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_activeModel(model)
{
}

bool
BoneBiasDialog::draw(Project *project)
{
    bool visible = true;
    Model *currentActiveModel = project->activeModel();
    if (currentActiveModel != m_activeModel) {
        m_activeModel = currentActiveModel;
        visible = currentActiveModel != nullptr;
    }
    if (open(tr("nanoem.gui.window.bone.correction.title"), kIdentifier, &visible)) {
    }
    close();
    return visible;
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
