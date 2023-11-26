/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/ActiveAccessorySelector.h"

#include "emapp/Accessory.h"
#include "emapp/private/CommonInclude.h"

#include "imgui/imgui.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *
ActiveAccessorySelector::callback(void *userData, int index) NANOEM_DECL_NOEXCEPT
{
    const ActiveAccessorySelector *self = static_cast<const ActiveAccessorySelector *>(userData);
    return self->select(index);
}

ActiveAccessorySelector::ActiveAccessorySelector(const Project *project)
    : m_translator(project->translator())
    , m_accessories(project->isPlaying() ? Project::AccessoryList() : *project->allAccessories())
{
}

bool
ActiveAccessorySelector::combo(int *accessoryIndex)
{
    return ImGui::Combo("##accessory.active", accessoryIndex, callback, this, count());
}

const char *
ActiveAccessorySelector::select(int index) const NANOEM_DECL_NOEXCEPT
{
    return index > 0 ? m_accessories[index - 1]->nameConstString()
                     : m_translator->translate("nanoem.gui.panel.accessory.none");
}

int
ActiveAccessorySelector::index(const Accessory *activeAccessory) const NANOEM_DECL_NOEXCEPT
{
    int value = 0;
    if (!m_accessories.empty() && activeAccessory) {
        value = 1;
        for (Project::AccessoryList::const_iterator it = m_accessories.begin(), end = m_accessories.end(); it != end;
             ++it) {
            if (*it == activeAccessory) {
                break;
            }
            value++;
        }
    }
    return value;
}

int
ActiveAccessorySelector::count() const NANOEM_DECL_NOEXCEPT
{
    return Inline::saturateInt32(m_accessories.size() + 1);
}

Accessory *
ActiveAccessorySelector::resolve(int offset) NANOEM_DECL_NOEXCEPT
{
    return offset > 0 && offset < count() ? m_accessories[offset - 1] : nullptr;
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
