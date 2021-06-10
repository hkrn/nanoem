/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_LAZYSETACTIVEACCESSORYCOMMAND_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_LAZYSETACTIVEACCESSORYCOMMAND_H_

#include "emapp/internal/ImGuiWindow.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct LazySetActiveAccessoryCommand : ImGuiWindow::ILazyExecutionCommand {
    LazySetActiveAccessoryCommand(Accessory *accessory, ImGuiWindow *parent);

    void execute(Project *project) NANOEM_DECL_OVERRIDE;
    void destroy(Project *project) NANOEM_DECL_OVERRIDE;

    ImGuiWindow *m_parent;
    Accessory *m_value;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_LAZYSETACTIVEACCESSORYCOMMAND_H_ */
