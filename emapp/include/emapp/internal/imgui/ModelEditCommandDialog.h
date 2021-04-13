/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_MODELEDITCOMMANDDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_MODELEDITCOMMANDDIALOG_H_

#include "emapp/Model.h"
#include "emapp/IModelObjectSelection.h"
#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct ModelEditCommandDialog : BaseNonModalDialogWindow {
    static const char *const kIdentifier;
    static void applyDeltaTransform(const Matrix4x4 &delta, Model *activeModel);
    static void beforeToggleEditingMode(
        IModelObjectSelection::EditingType editingType, Model *activeModel, Project *project);
    static void afterToggleEditingMode(
        IModelObjectSelection::EditingType editingType, Model *activeModel, Project *project);

    ModelEditCommandDialog(Model *model, BaseApplicationService *applicationPtr);

    bool draw(Project *project);
    void addGizmoOperationButton(const char *text, Model::GizmoOperationType type);
    void addGizmoCoordinationButton(const char *text, Model::TransformCoordinateType type);
    void addSelectionButton(const char *text, IModelObjectSelection::EditingType type, Project *project);

    Model *m_activeModel;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_MODELEDITCOMMANDDIALOG_H_ */
