/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/MorphSelector.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/Model.h"
#include "emapp/internal/ImGuiWindow.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *
MorphSelector::callback(void *userData, int index) NANOEM_DECL_NOEXCEPT
{
    const MorphSelector *self = static_cast<const MorphSelector *>(userData);
    return self->select(index);
}

MorphSelector::MorphSelector(const Model *model, const ITranslator *translator, nanoem_model_morph_category_t category)
    : m_translator(translator)
    , m_model(model)
    , m_category(category)
{
    nanoem_rsize_t numMorphs;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numMorphs);
    if (morphs && numMorphs > 0) {
        m_morphs.reserve(numMorphs);
        for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
            const nanoem_model_morph_t *morphPtr = morphs[i];
            if (nanoemModelMorphGetCategory(morphPtr) == category) {
                m_morphs.push_back(morphPtr);
            }
        }
    }
}

bool
MorphSelector::combo(int *morphIndex)
{
    const char *label = nullptr;
    switch (m_category) {
    case NANOEM_MODEL_MORPH_CATEGORY_EYE: {
        label = "##eye.combo";
        break;
    }
    case NANOEM_MODEL_MORPH_CATEGORY_LIP: {
        label = "##lip.combo";
        break;
    }
    case NANOEM_MODEL_MORPH_CATEGORY_OTHER: {
        label = "##other.combo";
        break;
    }
    case NANOEM_MODEL_MORPH_CATEGORY_EYEBROW: {
        label = "##eyebrow.combo";
        break;
    }
    default:
        break;
    }
    return ImGui::Combo(label, morphIndex, callback, this, count());
}

bool
MorphSelector::slider(nanoem_f32_t *weight, const Model *activeModel)
{
    const char *label = nullptr;
    switch (m_category) {
    case NANOEM_MODEL_MORPH_CATEGORY_EYE: {
        label = "##eye.weight";
        break;
    }
    case NANOEM_MODEL_MORPH_CATEGORY_LIP: {
        label = "##lip.weight";
        break;
    }
    case NANOEM_MODEL_MORPH_CATEGORY_OTHER: {
        label = "##other.weight";
        break;
    }
    case NANOEM_MODEL_MORPH_CATEGORY_EYEBROW: {
        label = "##eyebrow.weight";
        break;
    }
    default:
        break;
    }
    const char *format = m_translator->translate("nanoem.gui.panel.morph.weight.format");
    bool reacted = false;
    if (canRegister(activeModel)) {
        reacted = ImGui::SliderFloat(label, weight, 0, 1, format);
    }
    else {
        nanoem_f32_t fakeValue(*weight);
        ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWindow::kColorTextDisabled);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImGuiWindow::kColorFrameBg);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImGuiWindow::kColorFrameBg);
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImGuiWindow::kColorSliderGrab);
        ImGui::SliderFloat(label, &fakeValue, fakeValue, fakeValue, format);
        ImGui::PopStyleColor(4);
    }
    return reacted;
}

bool
MorphSelector::canRegister(const Model *activeModel) const
{
    return activeModel && !activeModel->project()->isPlaying() && activeModel->activeMorph(m_category) != nullptr;
}

void
MorphSelector::handleRegisterButton(Model *model, Project *project)
{
    CommandRegistrator registrator(project);
    registrator.registerAddMorphKeyframesCommandByActiveMorph(model, m_category);
}

const char *
MorphSelector::select(int index) const NANOEM_DECL_NOEXCEPT
{
    const char *name = nullptr;
    if (index > 0 && index < count()) {
        if (const model::Morph *morph = model::Morph::cast(m_morphs[index - 1])) {
            name = morph->nameConstString();
        }
    }
    if (!name) {
        name = m_translator->translate("nanoem.gui.panel.morph.none");
    }
    return name;
}

int
MorphSelector::index() const NANOEM_DECL_NOEXCEPT
{
    int value = 0;
    if (!m_morphs.empty()) {
        if (const nanoem_model_morph_t *activeMorphPtr = m_model->activeMorph(m_category)) {
            value = 1;
            for (model::Morph::List::const_iterator it = m_morphs.begin(), end = m_morphs.end(); it != end; ++it) {
                if (*it == activeMorphPtr) {
                    break;
                }
                value++;
            }
        }
    }
    return value;
}

int
MorphSelector::count() const NANOEM_DECL_NOEXCEPT
{
    return Inline::saturateInt32(m_morphs.size() + 1);
}

const nanoem_model_morph_t *
MorphSelector::activeMorph(int offset) const NANOEM_DECL_NOEXCEPT
{
    return offset > 0 ? m_morphs[offset - 1] : nullptr;
}

nanoem_model_morph_category_t
MorphSelector::category() const NANOEM_DECL_NOEXCEPT
{
    return m_category;
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
