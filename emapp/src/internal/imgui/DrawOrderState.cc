/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/DrawOrderState.h"

namespace nanoem {
namespace internal {
namespace imgui {

DrawOrderState::DrawOrderState(const Project::DrawableList &value)
    : m_currentDrawableList(value)
    , m_checkedDrawable(0)
{
}

void
DrawOrderState::setOrderAt(int index)
{
    setDrawableOrderAt(m_currentDrawableList, m_checkedDrawable, index);
}

bool
DrawOrderState::isOrderBegin() const NANOEM_DECL_NOEXCEPT
{
    return m_currentDrawableList.size() <= 1 || m_checkedDrawable == *m_currentDrawableList.begin();
}

bool
DrawOrderState::isOrderEnd() const NANOEM_DECL_NOEXCEPT
{
    return m_currentDrawableList.size() <= 1 || m_checkedDrawable == *(m_currentDrawableList.end() - 1);
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
