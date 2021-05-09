/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/LazySetCameraOutsideParentCommand.h"

#include "emapp/ICamera.h"

namespace nanoem {
namespace internal {
namespace imgui {

LazySetCameraOutsideParenthCommand::LazySetCameraOutsideParenthCommand(const StringPair &value)
    : m_value(value)
{
}

void
LazySetCameraOutsideParenthCommand::execute(Project *project)
{
    project->activeCamera()->setOutsideParent(m_value);
}

void
LazySetCameraOutsideParenthCommand::destroy(Project *project)
{
    BX_UNUSED_1(project);
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
