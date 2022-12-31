/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_CAMERAPARENTMODELSELECTOR_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_CAMERAPARENTMODELSELECTOR_H_

#include "emapp/Project.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct CameraParentModelSelector {
    static bool callback(void *userData, int index, const char **out) NANOEM_DECL_NOEXCEPT;
    CameraParentModelSelector(const Project *project);
    bool combo(int *modelIndex);
    bool select(int index, const char **out) const NANOEM_DECL_NOEXCEPT;
    int index(const StringPair &pair) const;
    int count() const NANOEM_DECL_NOEXCEPT;
    const Model *resolveModel(int index) const NANOEM_DECL_NOEXCEPT;
    String resolveName(int index) const;

    const ITranslator *m_translator;
    const Project::ModelList m_models;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_CAMERAPARENTMODELSELECTOR_H_ */
