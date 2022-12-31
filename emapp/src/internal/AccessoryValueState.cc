/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/AccessoryValueState.h"

#include "emapp/Accessory.h"
#include "emapp/command/UpdateAccessoryCommand.h"

#include "glm/gtc/type_ptr.hpp"

namespace nanoem {
namespace internal {

BaseAccessoryVectorValueState::BaseAccessoryVectorValueState(Accessory *accessory)
    : m_accessory(accessory)
{
}

BaseAccessoryVectorValueState::~BaseAccessoryVectorValueState()
{
    m_accessory = 0;
}

AccessoryTranslationVectorValueState::AccessoryTranslationVectorValueState(Accessory *accessory)
    : BaseAccessoryVectorValueState(accessory)
    , m_initiallTranslation(accessory->translation())
    , m_currentTranslation(accessory->translation())
{
}

AccessoryTranslationVectorValueState::~AccessoryTranslationVectorValueState()
{
}

nanoem_rsize_t
AccessoryTranslationVectorValueState::numComponents() const
{
    return 3;
}

const nanoem_f32_t *
AccessoryTranslationVectorValueState::initialValue() const
{
    return glm::value_ptr(m_initiallTranslation);
}

const nanoem_f32_t *
AccessoryTranslationVectorValueState::currentValue() const
{
    return glm::value_ptr(m_currentTranslation);
}

void
AccessoryTranslationVectorValueState::setValue(const nanoem_f32_t *value)
{
    m_currentTranslation = glm::make_vec3(value);
    m_accessory->setTranslation(m_currentTranslation);
}

void
AccessoryTranslationVectorValueState::commit()
{
    m_accessory->pushUndo(command::UpdateAccessoryCommand::create(m_accessory, m_initiallTranslation,
        m_accessory->orientation(), m_accessory->opacity(), m_accessory->scaleFactor()));
}

void
AccessoryTranslationVectorValueState::rollback()
{
    m_accessory->setTranslation(m_initiallTranslation);
}

AccessoryOrientationVectorValueState::AccessoryOrientationVectorValueState(Accessory *accessory)
    : BaseAccessoryVectorValueState(accessory)
    , m_initialOrientation(accessory->orientation())
    , m_currentOrientation(accessory->orientation())
{
}

AccessoryOrientationVectorValueState::~AccessoryOrientationVectorValueState()
{
}

nanoem_rsize_t
AccessoryOrientationVectorValueState::numComponents() const
{
    return 3;
}

const nanoem_f32_t *
AccessoryOrientationVectorValueState::initialValue() const
{
    return glm::value_ptr(m_initialOrientation);
}

const nanoem_f32_t *
AccessoryOrientationVectorValueState::currentValue() const
{
    return glm::value_ptr(m_currentOrientation);
}

void
AccessoryOrientationVectorValueState::setValue(const nanoem_f32_t *value)
{
    m_currentOrientation = glm::make_vec3(value);
    m_accessory->setOrientation(m_currentOrientation);
}

void
AccessoryOrientationVectorValueState::commit()
{
    m_accessory->pushUndo(command::UpdateAccessoryCommand::create(m_accessory, m_accessory->translation(),
        m_initialOrientation, m_accessory->opacity(), m_accessory->scaleFactor()));
}

void
AccessoryOrientationVectorValueState::rollback()
{
    m_accessory->setOrientation(m_initialOrientation);
}

AccessoryOpacityValueState::AccessoryOpacityValueState(Accessory *accessory)
    : BaseAccessoryVectorValueState(accessory)
    , m_initialOpacity(accessory->opacity())
    , m_currentOpacity(accessory->opacity())
{
}

AccessoryOpacityValueState::~AccessoryOpacityValueState()
{
}

nanoem_rsize_t
AccessoryOpacityValueState::numComponents() const
{
    return 1;
}

const nanoem_f32_t *
AccessoryOpacityValueState::initialValue() const
{
    return &m_initialOpacity;
}

const nanoem_f32_t *
AccessoryOpacityValueState::currentValue() const
{
    return &m_currentOpacity;
}

void
AccessoryOpacityValueState::setValue(const nanoem_f32_t *value)
{
    m_currentOpacity = *value;
    m_accessory->setOpacity(m_currentOpacity);
}

void
AccessoryOpacityValueState::commit()
{
    m_accessory->pushUndo(command::UpdateAccessoryCommand::create(m_accessory, m_accessory->translation(),
        m_accessory->orientation(), m_initialOpacity, m_accessory->scaleFactor()));
}

void
AccessoryOpacityValueState::rollback()
{
    m_accessory->setOpacity(m_initialOpacity);
}

AccessoryScaleFactorValueState::AccessoryScaleFactorValueState(Accessory *accessory)
    : BaseAccessoryVectorValueState(accessory)
    , m_initialScaleFactor(accessory->scaleFactor())
    , m_currentScaleFactor(accessory->scaleFactor())
{
}

AccessoryScaleFactorValueState::~AccessoryScaleFactorValueState()
{
}

nanoem_rsize_t
AccessoryScaleFactorValueState::numComponents() const
{
    return 1;
}

const nanoem_f32_t *
AccessoryScaleFactorValueState::initialValue() const
{
    return &m_initialScaleFactor;
}

const nanoem_f32_t *
AccessoryScaleFactorValueState::currentValue() const
{
    return &m_currentScaleFactor;
}

void
AccessoryScaleFactorValueState::setValue(const nanoem_f32_t *value)
{
    m_currentScaleFactor = *value;
    m_accessory->setScaleFactor(m_currentScaleFactor);
}

void
AccessoryScaleFactorValueState::commit()
{
    m_accessory->pushUndo(command::UpdateAccessoryCommand::create(m_accessory, m_accessory->translation(),
        m_accessory->orientation(), m_accessory->opacity(), m_initialScaleFactor));
}

void
AccessoryScaleFactorValueState::rollback()
{
    m_accessory->setScaleFactor(m_initialScaleFactor);
}

} /* namespace internal */
} /* namespace nanoem */
