/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/LazySetFrameIndexCommand.h"

namespace nanoem {
namespace internal {
namespace imgui {

LazySetFrameIndexCommand::LazySetFrameIndexCommand(const nanoem_frame_index_t value)
    : m_value(value)
{
}

void
LazySetFrameIndexCommand::execute(Project *project)
{
    project->seek(m_value, false);
}

void
LazySetFrameIndexCommand::destroy(Project *project)
{
    BX_UNUSED_1(project);
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
