/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/LightValueState.h"

#include "emapp/ILight.h"
#include "emapp/IVectorValueState.h"
#include "emapp/Project.h"
#include "emapp/command/UpdateLightCommand.h"

#include "glm/gtc/type_ptr.hpp"

namespace nanoem {
namespace internal {

BaseLightVectorValueState::BaseLightVectorValueState(Project *project, ILight *light)
    : m_project(project)
    , m_light(light)
{
}

BaseLightVectorValueState::~BaseLightVectorValueState()
{
    m_project = 0;
    m_light = 0;
}

LightColorVectorValueState::LightColorVectorValueState(Project *project, ILight *light)
    : BaseLightVectorValueState(project, light)
    , m_currentColor(light->color())
    , m_initialColor(light->color())
{
}

LightColorVectorValueState::~LightColorVectorValueState()
{
}

nanoem_rsize_t
LightColorVectorValueState::numComponents() const
{
    return 3;
}

const nanoem_f32_t *
LightColorVectorValueState::initialValue() const
{
    return glm::value_ptr(m_initialColor);
}

const nanoem_f32_t *
LightColorVectorValueState::currentValue() const
{
    return glm::value_ptr(m_currentColor);
}

void
LightColorVectorValueState::setValue(const nanoem_f32_t *value)
{
    m_currentColor = glm::make_vec3(value);
    m_light->setColor(m_currentColor);
}

void
LightColorVectorValueState::commit()
{
    if (m_project->globalLight() == m_light) {
        m_project->pushUndo(
            command::UpdateLightCommand::create(m_project, m_light, m_initialColor, m_light->direction()));
    }
}

void
LightColorVectorValueState::rollback()
{
    m_light->setColor(m_initialColor);
}

LightDirectionVectorValueState::LightDirectionVectorValueState(Project *project, ILight *light)
    : BaseLightVectorValueState(project, light)
    , m_currentDirection(light->direction())
    , m_initialDirection(light->direction())
{
}

LightDirectionVectorValueState::~LightDirectionVectorValueState()
{
}

nanoem_rsize_t
LightDirectionVectorValueState::numComponents() const
{
    return 3;
}

const nanoem_f32_t *
LightDirectionVectorValueState::initialValue() const
{
    return glm::value_ptr(m_initialDirection);
}

const nanoem_f32_t *
LightDirectionVectorValueState::currentValue() const
{
    return glm::value_ptr(m_currentDirection);
}

void
LightDirectionVectorValueState::setValue(const nanoem_f32_t *value)
{
    m_currentDirection = glm::make_vec3(value);
    m_light->setDirection(m_currentDirection);
}

void
LightDirectionVectorValueState::commit()
{
    if (m_project->globalLight() == m_light) {
        m_project->pushUndo(
            command::UpdateLightCommand::create(m_project, m_light, m_light->color(), m_initialDirection));
    }
}

void
LightDirectionVectorValueState::rollback()
{
    m_light->setDirection(m_initialDirection);
}

} /* namespace internal */
} /* namespace nanoem */
