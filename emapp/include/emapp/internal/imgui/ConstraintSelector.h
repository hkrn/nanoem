/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_CONSTRAINTSELECTOR_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_CONSTRAINTSELECTOR_H_

#include "emapp/model/Constraint.h"

namespace nanoem {

class Model;
class ITranslator;

namespace internal {
namespace imgui {

struct ConstraintSelector {
    static bool callback(void *userData, int index, const char **out) NANOEM_DECL_NOEXCEPT;
    ConstraintSelector(const Model *model, const ITranslator *translator);
    bool combo(int *constraintIndex);
    bool select(int index, const char **out) const NANOEM_DECL_NOEXCEPT;
    int index(const Model *activeModel) const NANOEM_DECL_NOEXCEPT;
    int count() const NANOEM_DECL_NOEXCEPT;
    const nanoem_model_constraint_t *activeConstraint(int offset) const NANOEM_DECL_NOEXCEPT;

    const ITranslator *m_translator;
    const Model *m_model;
    model::Constraint::List m_constraints;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_CONSTRAINTSELECTOR_H_ */
