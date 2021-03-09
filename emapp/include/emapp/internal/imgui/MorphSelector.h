/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_MORPHSELECTOR_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_MORPHSELECTOR_H_

#include "emapp/model/Morph.h"

namespace nanoem {

class ITranslator;
class Model;
class Project;

namespace internal {
namespace imgui {

struct MorphSelector {
    static bool callback(void *userData, int index, const char **out) NANOEM_DECL_NOEXCEPT;
    MorphSelector(const Model *model, const ITranslator *translator, nanoem_model_morph_category_t category);
    bool combo(int *morphIndex);
    bool slider(nanoem_f32_t *weight, const Model *activeModel);
    bool canRegister(const Model *activeModel) const;
    void handleRegisterButton(Model *model, Project *project);
    bool select(int index, const char **out) const NANOEM_DECL_NOEXCEPT;
    int index() const NANOEM_DECL_NOEXCEPT;
    int count() const NANOEM_DECL_NOEXCEPT;
    const nanoem_model_morph_t *activeMorph(int offset) const NANOEM_DECL_NOEXCEPT;
    nanoem_model_morph_category_t category() const NANOEM_DECL_NOEXCEPT;

    const ITranslator *m_translator;
    const Model *m_model;
    model::Morph::List m_morphs;
    nanoem_model_morph_category_t m_category;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_MORPHSELECTOR_H_ */
