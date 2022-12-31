/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/DraggingMorphState.h"

#include "emapp/Model.h"
#include "emapp/PerspectiveCamera.h"
#include "emapp/Project.h"
#include "emapp/command/TransformMorphCommand.h"
#include "emapp/model/Morph.h"

namespace nanoem {
namespace internal {

DraggingMorphSliderState::DraggingMorphSliderState(
    const nanoem_model_morph_t *morph, Model *model, Project *project, nanoem_f32_t weight)
    : m_lastMorphWeight(weight)
    , m_lastActiveMorph(model->activeMorph())
    , m_targetMorph(morph)
    , m_project(project)
    , m_model(model)
    , m_lastEditingMode(project->editingMode())
{
    model->setActiveMorph(morph);
    model->saveBindPose(m_lastBindPose);
    project->setEditingMode(Project::kEditingModeNone);
    setCameraLocked(true);
}

DraggingMorphSliderState::~DraggingMorphSliderState()
{
    setCameraLocked(false);
    m_lastActiveMorph = nullptr;
    m_targetMorph = nullptr;
    m_project = nullptr;
    m_model = nullptr;
}

void
DraggingMorphSliderState::deform(nanoem_f32_t weight)
{
    model::Morph *morph = model::Morph::cast(m_targetMorph);
    m_model->restoreBindPose(m_lastBindPose);
    morph->setWeight(weight);
    m_model->resetAllMorphDeformStates();
    m_model->deformAllMorphs(true);
    m_model->performAllBonesTransform();
}

void
DraggingMorphSliderState::commit()
{
    const model::Morph *morph = model::Morph::cast(m_targetMorph);
    if (glm::abs(m_lastMorphWeight - morph->weight()) > 0) {
        command::TransformMorphCommand::WeightStateList weights;
        model::BindPose currentBindPose;
        weights.push_back(tinystl::make_pair(m_targetMorph, tinystl::make_pair(morph->weight(), m_lastMorphWeight)));
        m_model->saveBindPose(currentBindPose);
        m_model->pushUndo(command::TransformMorphCommand::create(weights, currentBindPose, m_model, m_project));
    }
    m_model->setActiveMorph(m_lastActiveMorph);
    m_project->setEditingMode(static_cast<Project::EditingMode>(m_lastEditingMode));
    setCameraLocked(false);
}

void
DraggingMorphSliderState::setCameraLocked(bool value)
{
    m_project->globalCamera()->setLocked(value);
    m_model->localCamera()->setLocked(value);
}

} /* namespace internal */
} /* namespace nanoem */
