/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/LazySetActiveModelMorphCommand.h"

#include "emapp/Model.h"

namespace nanoem {
namespace internal {
namespace imgui {

LazySetActiveModelMorphCommand::LazySetActiveModelMorphCommand(const nanoem_model_morph_t *morph)
    : m_value(morph)
    , m_category(nanoemModelMorphGetCategory(morph))
{
}

LazySetActiveModelMorphCommand::LazySetActiveModelMorphCommand(
    const nanoem_model_morph_t *morph, nanoem_model_morph_category_t category)
    : m_value(morph)
    , m_category(category)
{
}

void
LazySetActiveModelMorphCommand::execute(Project *project)
{
    if (Model *activeModel = project->activeModel()) {
        activeModel->setActiveMorph(m_category, m_value);
    }
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
