/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/CameraValueState.h"

#include "emapp/ICamera.h"
#include "emapp/IVectorValueState.h"
#include "emapp/Project.h"
#include "emapp/command/UpdateCameraCommand.h"

#include "glm/gtc/type_ptr.hpp"

namespace nanoem {
namespace internal {

BaseCameraVectorValueState::BaseCameraVectorValueState(Project *project, ICamera *camera)
    : m_project(project)
    , m_camera(camera)
{
}

BaseCameraVectorValueState::~BaseCameraVectorValueState()
{
    m_project = 0;
    m_camera = 0;
}

CameraLookAtVectorValueState::CameraLookAtVectorValueState(Project *project, ICamera *camera)
    : BaseCameraVectorValueState(project, camera)
    , m_initialLookAt(camera->lookAt())
    , m_currentLookAt(camera->lookAt())
{
}

CameraLookAtVectorValueState::~CameraLookAtVectorValueState()
{
}

nanoem_rsize_t
CameraLookAtVectorValueState::numComponents() const
{
    return 3;
}

const nanoem_f32_t *
CameraLookAtVectorValueState::initialValue() const
{
    return glm::value_ptr(m_initialLookAt);
}

const nanoem_f32_t *
CameraLookAtVectorValueState::currentValue() const
{
    return glm::value_ptr(m_currentLookAt);
}

void
CameraLookAtVectorValueState::setValue(const nanoem_f32_t *value)
{
    m_currentLookAt = glm::make_vec3(value);
    m_camera->setLookAt(m_currentLookAt);
    m_camera->update();
}

void
CameraLookAtVectorValueState::commit()
{
    if (m_project->globalCamera() == m_camera) {
        m_project->pushUndo(command::UpdateCameraCommand::create(
            m_project, m_camera, m_initialLookAt, m_camera->angle(), m_camera->distance(), m_camera->fovRadians()));
    }
}

void
CameraLookAtVectorValueState::rollback()
{
    m_camera->setLookAt(m_initialLookAt);
    m_camera->update();
}

CameraAngleVectorValueState::CameraAngleVectorValueState(Project *project, ICamera *camera)
    : BaseCameraVectorValueState(project, camera)
    , m_initialAngle(camera->angle())
    , m_currentAngle(camera->angle())
{
}

CameraAngleVectorValueState::~CameraAngleVectorValueState()
{
}

nanoem_rsize_t
CameraAngleVectorValueState::numComponents() const
{
    return 3;
}

const nanoem_f32_t *
CameraAngleVectorValueState::initialValue() const
{
    return glm::value_ptr(m_initialAngle);
}

const nanoem_f32_t *
CameraAngleVectorValueState::currentValue() const
{
    return glm::value_ptr(m_currentAngle);
}

void
CameraAngleVectorValueState::setValue(const nanoem_f32_t *value)
{
    m_currentAngle = glm::make_vec3(value);
    m_camera->setAngle(m_currentAngle);
    m_camera->update();
}

void
CameraAngleVectorValueState::commit()
{
    if (m_project->globalCamera() == m_camera) {
        m_project->pushUndo(command::UpdateCameraCommand::create(
            m_project, m_camera, m_camera->lookAt(), m_initialAngle, m_camera->distance(), m_camera->fovRadians()));
    }
}

void
CameraAngleVectorValueState::rollback()
{
    m_camera->setAngle(m_initialAngle);
    m_camera->update();
}

CameraFovVectorValueState::CameraFovVectorValueState(Project *project, ICamera *camera)
    : BaseCameraVectorValueState(project, camera)
    , m_initialFov(camera->fovRadians())
    , m_currentFov(camera->fovRadians())
{
}

CameraFovVectorValueState::~CameraFovVectorValueState()
{
}

nanoem_rsize_t
CameraFovVectorValueState::numComponents() const
{
    return 1;
}

const nanoem_f32_t *
CameraFovVectorValueState::initialValue() const
{
    return &m_initialFov;
}

const nanoem_f32_t *
CameraFovVectorValueState::currentValue() const
{
    return &m_currentFov;
}

void
CameraFovVectorValueState::setValue(const nanoem_f32_t *value)
{
    m_currentFov = *value;
    m_camera->setFov(int(m_currentFov));
    m_camera->update();
}

void
CameraFovVectorValueState::commit()
{
    if (m_project->globalCamera() == m_camera) {
        m_project->pushUndo(command::UpdateCameraCommand::create(
            m_project, m_camera, m_camera->lookAt(), m_camera->angle(), m_camera->distance(), m_initialFov));
    }
}

void
CameraFovVectorValueState::rollback()
{
    m_camera->setFovRadians(m_initialFov);
    m_camera->update();
}

CameraDistanceVectorValueState::CameraDistanceVectorValueState(Project *project, ICamera *camera)
    : BaseCameraVectorValueState(project, camera)
    , m_initialDistance(camera->distance())
    , m_currentDistance(camera->distance())
{
}

CameraDistanceVectorValueState::~CameraDistanceVectorValueState()
{
}

nanoem_rsize_t
CameraDistanceVectorValueState::numComponents() const
{
    return 1;
}

const nanoem_f32_t *
CameraDistanceVectorValueState::initialValue() const
{
    return &m_initialDistance;
}

const nanoem_f32_t *
CameraDistanceVectorValueState::currentValue() const
{
    return &m_currentDistance;
}

void
CameraDistanceVectorValueState::setValue(const nanoem_f32_t *value)
{
    m_currentDistance = *value;
    m_camera->setDistance(m_currentDistance);
    m_camera->update();
}

void
CameraDistanceVectorValueState::commit()
{
    if (m_project->globalCamera() == m_camera) {
        m_project->pushUndo(command::UpdateCameraCommand::create(
            m_project, m_camera, m_camera->lookAt(), m_camera->angle(), m_initialDistance, m_camera->fovRadians()));
    }
}

void
CameraDistanceVectorValueState::rollback()
{
    m_camera->setDistance(m_initialDistance);
    m_camera->update();
}

} /* namespace internal */
} /* namespace nanoem */
