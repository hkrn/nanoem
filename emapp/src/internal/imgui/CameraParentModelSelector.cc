/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/CameraParentModelSelector.h"

#include "emapp/Model.h"
#include "emapp/private/CommonInclude.h"

#include "imgui/imgui.h"

namespace nanoem {
namespace internal {
namespace imgui {

bool
CameraParentModelSelector::callback(void *userData, int index, const char **out) NANOEM_DECL_NOEXCEPT
{
    const CameraParentModelSelector *self = static_cast<const CameraParentModelSelector *>(userData);
    return self->select(index, out);
}

CameraParentModelSelector::CameraParentModelSelector(const Project *project)
    : m_translator(project->translator())
    , m_models(project->allModels())
{
}

bool
CameraParentModelSelector::combo(int *modelIndex)
{
    return ImGui::Combo("##camera.model", modelIndex, callback, this, count());
}

bool
CameraParentModelSelector::select(int index, const char **out) const NANOEM_DECL_NOEXCEPT
{
    *out = index > 0 ? m_models[index - 1]->nameConstString()
                     : m_translator->translate("nanoem.gui.panel.camera.model.none");
    return true;
}

int
CameraParentModelSelector::index(const StringPair &pair) const
{
    int value = 0;
    if (!m_models.empty() && !pair.first.empty()) {
        value = 1;
        for (Project::ModelList::const_iterator it = m_models.begin(), end = m_models.end(); it != end; ++it) {
            const Model *model = *it;
            if (model->canonicalName() == pair.first) {
                break;
            }
            value++;
        }
    }
    return value;
}

int
CameraParentModelSelector::count() const NANOEM_DECL_NOEXCEPT
{
    return Inline::saturateInt32(m_models.size() + 1);
}

const Model *
CameraParentModelSelector::resolveModel(int index) const NANOEM_DECL_NOEXCEPT
{
    return index > 0 && index < count() ? m_models[index - 1] : nullptr;
}

String
CameraParentModelSelector::resolveName(int index) const
{
    const Model *model = resolveModel(index);
    return model ? model->canonicalName() : String();
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
