/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_LAZYUNDOCOMMAND_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_LAZYUNDOCOMMAND_H_

#include "emapp/internal/ImGuiWindow.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct LazyPushUndoCommand : ImGuiWindow::ILazyExecutionCommand {
    LazyPushUndoCommand(undo_command_t *command);
    ~LazyPushUndoCommand() NANOEM_DECL_NOEXCEPT;

    void execute(Project *project) NANOEM_DECL_OVERRIDE;
    void destroy(Project *project) NANOEM_DECL_OVERRIDE;

    undo_command_t *m_command;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_LAZYUNDOCOMMAND_H_ */
