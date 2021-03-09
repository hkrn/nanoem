/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_ACTIVEMODELSELECTOR_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_ACTIVEMODELSELECTOR_H_

#include "emapp/Project.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct ActiveModelSelector {
    static bool callback(void *userData, int index, const char **out) NANOEM_DECL_NOEXCEPT;
    ActiveModelSelector(const Project *project);
    bool combo(int *modelIndex);
    bool select(int index, const char **out) const NANOEM_DECL_NOEXCEPT;
    int index(const Model *activeModel) const NANOEM_DECL_NOEXCEPT;
    int count() const NANOEM_DECL_NOEXCEPT;
    Model *resolve(int index) const;

    const ITranslator *m_translator;
    const Project::ModelList m_models;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_ACTIVEMODELSELECTOR_H_ */
