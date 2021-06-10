/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/LazyPushUndoCommand.h"

#include "emapp/Model.h"

namespace nanoem {
namespace internal {
namespace imgui {

LazyPushUndoCommand::LazyPushUndoCommand(undo_command_t *command)
    : m_command(command)
{
}

LazyPushUndoCommand::~LazyPushUndoCommand() NANOEM_DECL_NOEXCEPT
{
    m_command = nullptr;
}

void
LazyPushUndoCommand::execute(Project *project)
{
    project->activeModel()->pushUndo(m_command);
}

void
LazyPushUndoCommand::destroy(Project *project)
{
    BX_UNUSED_1(project);
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
