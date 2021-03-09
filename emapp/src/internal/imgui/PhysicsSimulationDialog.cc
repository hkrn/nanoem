/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/PhysicsSimulationDialog.h"

#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const PhysicsEngineDialog::kIdentifier = "dialog.project.physics";

PhysicsEngineDialog::PhysicsEngineDialog(Project *project, BaseApplicationService *applicationPtr)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_direction(0)
    , m_acceleration(0)
    , m_noise(0)
    , m_timeStepFactor(0)
    , m_enableNoise(false)
{
    const PhysicsEngine *engine = project->physicsEngine();
    m_direction = engine->direction();
    m_acceleration = engine->acceleration();
    m_noise = engine->noise();
    m_timeStepFactor = project->timeStepFactor();
    m_enableNoise = engine->isNoiseEnabled();
}

bool
PhysicsEngineDialog::draw(Project *project)
{
    bool visible = true;
    if (open(tr("nanoem.gui.window.project.physics-engine.title"), kIdentifier, &visible)) {
        ImGui::PushItemWidth(-1);
        ImGui::TextUnformatted(tr("nanoem.gui.window.project.physics-engine.acceleration"));
        PhysicsEngine *engine = project->physicsEngine();
        nanoem_f32_t acceleration = engine->acceleration();
        if (ImGui::DragFloat("##acceleration", &acceleration)) {
            engine->setAcceleration(acceleration);
        }
        Vector3 direction(engine->direction());
        ImGui::TextUnformatted(tr("nanoem.gui.window.project.physics-engine.direction"));
        if (ImGui::DragFloat3("##direction", glm::value_ptr(direction), 0.01f, -1.0f, 1.0f)) {
            engine->setDirection(direction);
        }
        nanoem_f32_t timeStepFactor = project->timeStepFactor();
        ImGui::TextUnformatted(tr("nanoem.gui.window.project.physics-engine.time-step-factor"));
        if (ImGui::DragFloat("##time-step-factor", &timeStepFactor, 0.05f, 0.01f, 2.0f)) {
            project->setTimeStepFactor(timeStepFactor);
        }
#if 0
        ImGui::TextUnformatted(tr("nanoem.gui.window.project.physics-engine.noise"));
        bool enableNoise = engine->isNoiseEnabled();
        if (ImGui::Checkbox(tr("nanoem.gui.window.project.physics-engine.noise.enabled"), &enableNoise)) {
            engine->setNoiseEnabled(enableNoise);
        }
        nanoem_f32_t noise = engine->noise();
        if (ImGui::DragFloat("##noise", &noise)) {
            engine->setNoise(noise);
        }
#endif
        addSeparator();
        switch (layoutCommonButtons(&visible)) {
        case kResponseTypeOK: {
            engine->apply();
            break;
        }
        case kResponseTypeCancel: {
            project->setTimeStepFactor(m_timeStepFactor);
            engine->setDirection(m_direction);
            engine->setAcceleration(m_acceleration);
            engine->setNoise(m_noise);
            engine->setNoiseEnabled(m_enableNoise);
            engine->apply();
            break;
        }
        default:
            break;
        }
        ImGui::PopItemWidth();
    }
    close();
    return visible;
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
