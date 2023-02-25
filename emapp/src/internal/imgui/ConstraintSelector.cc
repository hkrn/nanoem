/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/ConstraintSelector.h"

#include "emapp/ITranslator.h"
#include "emapp/Model.h"
#include "emapp/private/CommonInclude.h"

#include "imgui/imgui.h"

namespace nanoem {
namespace internal {
namespace imgui {

bool
ConstraintSelector::callback(void *userData, int index, const char **out) NANOEM_DECL_NOEXCEPT
{
    const ConstraintSelector *self = static_cast<const ConstraintSelector *>(userData);
    return self->select(index, out);
}

ConstraintSelector::ConstraintSelector(const Model *model, const ITranslator *translator)
    : m_translator(translator)
    , m_model(model)
{
    if (model) {
        nanoem_rsize_t numConstraints;
        nanoem_model_constraint_t *const *constraints =
            nanoemModelGetAllConstraintObjects(model->data(), &numConstraints);
        if (constraints && numConstraints > 0) {
            m_constraints.reserve(numConstraints);
            for (nanoem_rsize_t i = 0; i < numConstraints; i++) {
                const nanoem_model_constraint_t *constraintPtr = constraints[i];
                m_constraints.push_back(constraintPtr);
            }
        }
        else {
            nanoem_rsize_t numBones;
            nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
            if (bones && numBones > 0) {
                m_constraints.reserve(numBones);
                for (nanoem_rsize_t i = 0; i < numBones; i++) {
                    const nanoem_model_bone_t *bonePtr = bones[i];
                    if (const nanoem_model_constraint_t *constraintPtr = nanoemModelBoneGetConstraintObject(bonePtr)) {
                        m_constraints.push_back(constraintPtr);
                    }
                }
            }
        }
    }
}

bool
ConstraintSelector::combo(int *constraintIndex)
{
    return ImGui::Combo("##model.constraint", constraintIndex, callback, this, count());
}

bool
ConstraintSelector::select(int index, const char **out) const NANOEM_DECL_NOEXCEPT
{
    const char *name = nullptr;
    if (index > 0 && index < count()) {
        if (const model::Constraint *constraint = model::Constraint::cast(m_constraints[index - 1])) {
            name = constraint->nameConstString();
        }
    }
    if (!name) {
        name = m_translator->translate("nanoem.gui.panel.model.constraint.none");
    }
    *out = name;
    return true;
}

int
ConstraintSelector::index(const Model *activeModel) const NANOEM_DECL_NOEXCEPT
{
    int value = 0;
    if (!m_constraints.empty() && activeModel) {
        if (const nanoem_model_constraint_t *activeConstraintPtr = activeModel->activeConstraint()) {
            value = 1;
            for (model::Constraint::List::const_iterator it = m_constraints.begin(), end = m_constraints.end();
                 it != end; ++it) {
                if (*it == activeConstraintPtr) {
                    break;
                }
                value++;
            }
        }
    }
    return value;
}

int
ConstraintSelector::count() const NANOEM_DECL_NOEXCEPT
{
    return Inline::saturateInt32(m_constraints.size() + 1);
}

const nanoem_model_constraint_t *
ConstraintSelector::activeConstraint(int offset) const NANOEM_DECL_NOEXCEPT
{
    return offset > 0 && offset < count() ? m_constraints[offset - 1] : nullptr;
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
