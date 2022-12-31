/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_DRAWORDERSTATE_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_DRAWORDERSTATE_H_

#include "emapp/internal/imgui/BaseOrderState.h"

#include "emapp/Project.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct DrawOrderState : BaseOrderState {
    Project::DrawableList m_currentDrawableList;
    IDrawable *m_checkedDrawable;
    DrawOrderState(const Project::DrawableList &value);
    void setOrderAt(int index) NANOEM_DECL_OVERRIDE;
    bool isOrderBegin() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isOrderEnd() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_BASEORDERSTATE_H_ */
