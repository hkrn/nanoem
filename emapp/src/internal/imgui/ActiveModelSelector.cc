/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/ActiveModelSelector.h"

#include "emapp/Model.h"
#include "emapp/private/CommonInclude.h"

#include "imgui/imgui.h"

namespace nanoem {
namespace internal {
namespace imgui {

bool
ActiveModelSelector::callback(void *userData, int index, const char **out) NANOEM_DECL_NOEXCEPT
{
    const ActiveModelSelector *self = static_cast<const ActiveModelSelector *>(userData);
    return self->select(index, out);
}

ActiveModelSelector::ActiveModelSelector(const Project *project)
    : m_translator(project->translator())
    , m_models(project->isPlaying() ? Project::ModelList() : *project->allModels())
{
}

bool
ActiveModelSelector::combo(int *modelIndex)
{
    return ImGui::Combo("##model.active", modelIndex, callback, this, count());
}

bool
ActiveModelSelector::select(int index, const char **out) const NANOEM_DECL_NOEXCEPT
{
    *out =
        index > 0 ? m_models[index - 1]->nameConstString() : m_translator->translate("nanoem.gui.panel.model.default");
    return true;
}

int
ActiveModelSelector::index(const Model *activeModel) const NANOEM_DECL_NOEXCEPT
{
    int value = 0;
    if (!m_models.empty() && activeModel) {
        value = 1;
        for (Project::ModelList::const_iterator it = m_models.begin(), end = m_models.end(); it != end; ++it) {
            if (*it == activeModel) {
                break;
            }
            value++;
        }
    }
    return value;
}

int
ActiveModelSelector::count() const NANOEM_DECL_NOEXCEPT
{
    return Inline::saturateInt32(m_models.size() + 1);
}

Model *
ActiveModelSelector::resolve(int index) const
{
    return index > 0 && index < count() ? m_models[index - 1] : nullptr;
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
