/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/TransformOrderState.h"

namespace nanoem {
namespace internal {
namespace imgui {

TransformOrderState::TransformOrderState(const Project::ModelList &value)
    : m_currentTransformModelList(value)
    , m_checkedTransformModel(0)
{
}

void
TransformOrderState::setOrderAt(int index)
{
    setDrawableOrderAt(m_currentTransformModelList, m_checkedTransformModel, index);
}

bool
TransformOrderState::isOrderBegin() const NANOEM_DECL_NOEXCEPT
{
    return m_currentTransformModelList.size() <= 1 || m_checkedTransformModel == *m_currentTransformModelList.begin();
}

bool
TransformOrderState::isOrderEnd() const NANOEM_DECL_NOEXCEPT
{
    return m_currentTransformModelList.size() <= 1 ||
        m_checkedTransformModel == *(m_currentTransformModelList.end() - 1);
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
