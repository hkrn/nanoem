/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_ACTIVEACCESSORYSELECTOR_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_ACTIVEACCESSORYSELECTOR_H_

#include "emapp/Project.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct ActiveAccessorySelector {
    static const char *callback(void *userData, int index) NANOEM_DECL_NOEXCEPT;
    ActiveAccessorySelector(const Project *project);
    bool combo(int *accessoryIndex);
    const char *select(int index) const NANOEM_DECL_NOEXCEPT;
    int index(const Accessory *activeAccessory) const NANOEM_DECL_NOEXCEPT;
    int count() const NANOEM_DECL_NOEXCEPT;
    Accessory *resolve(int offset) NANOEM_DECL_NOEXCEPT;

    const ITranslator *m_translator;
    const Project::AccessoryList m_accessories;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_ACTIVEACCESSORYSELECTOR_H_ */
