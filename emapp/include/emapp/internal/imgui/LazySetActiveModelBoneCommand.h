/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_LAZYSETACTIVEMODELBONECOMMAND_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_LAZYSETACTIVEMODELBONECOMMAND_H_

#include "emapp/internal/ImGuiWindow.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct LazySetActiveModelBoneCommand : ImGuiWindow::ILazyExecutionCommand {
    LazySetActiveModelBoneCommand(const nanoem_model_bone_t *bone);

    void execute(Project *project) NANOEM_DECL_OVERRIDE;
    void destroy(Project *project) NANOEM_DECL_OVERRIDE;

    const nanoem_model_bone_t *m_value;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_LAZYSETACTIVEMODELBONECOMMAND_H_ */
