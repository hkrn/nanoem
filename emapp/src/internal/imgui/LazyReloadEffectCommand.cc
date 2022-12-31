/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/LazyReloadEffectCommand.h"

#include "emapp/Error.h"
#include "emapp/Progress.h"

namespace nanoem {
namespace internal {
namespace imgui {

LazyReloadEffectCommand::LazyReloadEffectCommand(IDrawable *value)
    : m_value(value)
{
}

void
LazyReloadEffectCommand::execute(Project *project)
{
    Progress progress(project, 0);
    Error error;
    project->reloadDrawableEffect(m_value, progress, error);
}

void
LazyReloadEffectCommand::destroy(Project *project)
{
    BX_UNUSED_1(project);
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
