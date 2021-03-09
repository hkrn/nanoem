/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/LazySetActiveModelBoneCommand.h"

#include "emapp/Model.h"

namespace nanoem {
namespace internal {
namespace imgui {

LazySetActiveModelBoneCommand::LazySetActiveModelBoneCommand(const nanoem_model_bone_t *bone)
    : m_value(bone)
{
}

void
LazySetActiveModelBoneCommand::execute(Project *project)
{
    if (Model *activeModel = project->activeModel()) {
        activeModel->setActiveBone(m_value);
    }
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
