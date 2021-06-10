/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/LazySetActiveAccessoryCommand.h"

namespace nanoem {
namespace internal {
namespace imgui {

LazySetActiveAccessoryCommand::LazySetActiveAccessoryCommand(Accessory *accessory, ImGuiWindow *parent)
    : m_parent(parent)
    , m_value(accessory)
{
}

void
LazySetActiveAccessoryCommand::execute(Project *project)
{
    m_parent->cancelAllDraggingStates();
    project->setActiveAccessory(m_value);
    m_parent->resetSelectionIndex();
}

void
LazySetActiveAccessoryCommand::destroy(Project *project)
{
    BX_UNUSED_1(project);
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
