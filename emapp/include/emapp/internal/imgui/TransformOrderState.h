/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_TRANSFORMORDERSTATE_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_TRANSFORMORDERSTATE_H_

#include "emapp/internal/imgui/BaseOrderState.h"

#include "emapp/Project.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct TransformOrderState : BaseOrderState {
    Project::ModelList m_currentTransformModelList;
    Model *m_checkedTransformModel;
    TransformOrderState(const Project::ModelList &value);
    void setOrderAt(int index) NANOEM_DECL_OVERRIDE;
    bool isOrderBegin() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isOrderEnd() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_BASEORDERSTATE_H_ */
