/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_MODELEDGEDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_MODELEDGEDIALOG_H_

#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct ModelEdgeDialog : BaseNonModalDialogWindow {
    static const char *const kIdentifier;

    ModelEdgeDialog(Model *model, Project *project, BaseApplicationService *applicationPtr);
    bool draw(Project *project);
    void reset(Project *project);

    Model *m_activeModel;
    Vector4 m_color;
    nanoem_f32_t m_scaleFactor;
    Project::EditingMode m_editingMode;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_MODELEDGEDIALOG_H_ */
