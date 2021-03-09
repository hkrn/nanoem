/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/LazySetActiveModelCommand.h"

namespace nanoem {
namespace internal {
namespace imgui {

LazySetActiveModelCommand::LazySetActiveModelCommand(Model *model, ImGuiWindow *parent)
    : m_parent(parent)
    , m_value(model)
{
}

void
LazySetActiveModelCommand::execute(Project *project)
{
    m_parent->cancelAllDraggingStates();
    project->setActiveModel(m_value);
    m_parent->resetSelectionIndex();
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
