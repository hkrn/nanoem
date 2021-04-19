/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/DraggingCameraState.h"

#include "emapp/ICamera.h"
#include "emapp/IEventPublisher.h"
#include "emapp/Project.h"
#include "emapp/command/UpdateCameraCommand.h"
#include "emapp/internal/IDraggingState.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace internal {

DraggingCameraState::DraggingCameraState(Project *project, ICamera *camera, const Vector2SI32 &pressedCursorPosition)
    : m_project(project)
    , m_activeCamera(camera)
    , m_lookAt(camera->lookAt())
    , m_angle(camera->angle())
    , m_distance(camera->distance())
    , m_fov(camera->fovRadians())
    , m_perspective(camera->isPerspective())
    , m_accumulatedPositionDelta(0)
    , m_lastPressedCursorPosition(pressedCursorPosition)
    , m_scaleFactor(1.0f)
{
    nanoem_parameter_assert(m_project, "must NOT be NULL");
    nanoem_parameter_assert(m_activeCamera, "must NOT be NULL");
    m_project->eventPublisher()->publishDisableCursorEvent(pressedCursorPosition);
}

void
DraggingCameraState::commit(const Vector2SI32 & /* logicalCursorPosition */)
{
    if (m_project->globalCamera() == m_activeCamera) {
        m_project->pushUndo(command::UpdateCameraCommand::create(
            m_project, m_activeCamera, m_lookAt, m_angle, m_distance, m_fov, m_perspective));
    }
    m_project->eventPublisher()->publishEnableCursorEvent(Vector2SI32(0));
    m_accumulatedPositionDelta = Vector3(0);
    m_lastPressedCursorPosition = Vector2(0);
}

nanoem_f32_t
DraggingCameraState::scaleFactor() const NANOEM_DECL_NOEXCEPT
{
    return m_scaleFactor;
}

void
DraggingCameraState::setScaleFactor(nanoem_f32_t value)
{
    m_scaleFactor = value;
}

Vector3
DraggingCameraState::handleDragAxis(int index)
{
    return Matrix3x3(1)[index];
}

ICamera *
DraggingCameraState::activeCamera() NANOEM_DECL_NOEXCEPT
{
    return m_activeCamera;
}

Vector3
DraggingCameraState::angle() const NANOEM_DECL_NOEXCEPT
{
    return m_angle;
}

Vector3
DraggingCameraState::lookAt() const NANOEM_DECL_NOEXCEPT
{
    return m_lookAt;
}

Vector3
DraggingCameraState::accumulatedPositionDelta() const NANOEM_DECL_NOEXCEPT
{
    return m_accumulatedPositionDelta;
}

Vector2
DraggingCameraState::cursorDelta(const Vector2 &logicalCursorPosition) const NANOEM_DECL_NOEXCEPT
{
    return (logicalCursorPosition - m_lastPressedCursorPosition) * m_scaleFactor;
}

nanoem_f32_t
DraggingCameraState::distance() const NANOEM_DECL_NOEXCEPT
{
    return m_distance;
}

void
DraggingCameraState::updateLastCursorPosition(const Vector2 &logicalCursorPosition, const Vector3 &delta)
{
    m_lastPressedCursorPosition = logicalCursorPosition;
    m_accumulatedPositionDelta += delta;
}

void
DraggingCameraState::resetAllModelEdges()
{
    m_project->resetAllModelEdges();
}

AxisAlignedTranslateCameraState::AxisAlignedTranslateCameraState(
    Project *project, ICamera *camera, const Vector2SI32 &pressedCursorPosition, int axisIndex)
    : DraggingCameraState(project, camera, pressedCursorPosition)
    , m_axisIndex(axisIndex)
{
    setScaleFactor(kDefaultScaleFactor);
}

void
AxisAlignedTranslateCameraState::transform(const Vector2SI32 &logicalCursorPosition)
{
    const Vector3 axis(handleDragAxis(m_axisIndex));
    const Vector3 delta(cursorDelta(logicalCursorPosition).y * axis);
    ICamera *camera = activeCamera();
    switch (camera->transformCoordinateType()) {
    case ICamera::kTransformCoordinateTypeGlobal: {
        camera->setLookAt(lookAt() + delta);
        camera->update();
        resetAllModelEdges();
    }
    case ICamera::kTransformCoordinateTypeLocal: {
        Matrix4x4 viewMatrix, projectionMatrix;
        camera->getViewTransform(viewMatrix, projectionMatrix);
        camera->setLookAt(lookAt() + glm::inverse(Matrix3x3(viewMatrix)) * (accumulatedPositionDelta() + delta));
        camera->update();
        resetAllModelEdges();
    }
    case ICamera::kTransformCoordinateTypeMaxEnum:
    default:
        break;
    }
    updateLastCursorPosition(logicalCursorPosition, delta);
}

const char *
AxisAlignedTranslateCameraState::name() const NANOEM_DECL_NOEXCEPT
{
    return "nanoem.gui.viewport.camera.axis-aligned-translate";
}

const nanoem_f32_t AxisAlignedTranslateCameraState::kDefaultScaleFactor = 0.5f;

AxisAlignedOrientateCameraState::AxisAlignedOrientateCameraState(
    Project *project, ICamera *camera, const Vector2SI32 &pressedCursorPosition, int axisIndex)
    : DraggingCameraState(project, camera, pressedCursorPosition)
    , m_axisIndex(axisIndex)
{
    setScaleFactor(kDefaultScaleFactor);
}

void
AxisAlignedOrientateCameraState::transform(const Vector2SI32 &logicalCursorPosition)
{
    const Vector3 axis(handleDragAxis(m_axisIndex));
    const Vector3 delta(cursorDelta(logicalCursorPosition).y * axis);
    ICamera *camera = activeCamera();
    switch (camera->transformCoordinateType()) {
    case ICamera::kTransformCoordinateTypeGlobal:
    case ICamera::kTransformCoordinateTypeLocal: {
        camera->setAngle(glm::radians(glm::degrees(angle()) + accumulatedPositionDelta() + delta));
        camera->update();
        break;
    }
    case ICamera::kTransformCoordinateTypeMaxEnum:
    default:
        break;
    }
    updateLastCursorPosition(logicalCursorPosition, delta);
}

const char *
AxisAlignedOrientateCameraState::name() const NANOEM_DECL_NOEXCEPT
{
    return "nanoem.gui.viewport.camera.axis-aligned-orientate";
}

const nanoem_f32_t AxisAlignedOrientateCameraState::kDefaultScaleFactor = 0.5f;

CameraZoomState::CameraZoomState(Project *project, ICamera *camera, const Vector2SI32 &pressedCursorPosition)
    : DraggingCameraState(project, camera, pressedCursorPosition)
{
    setScaleFactor(kDefaultScaleFactor);
}

void
CameraZoomState::transform(const Vector2SI32 &logicalCursorPosition)
{
    const Vector3 delta(cursorDelta(logicalCursorPosition), 0);
    ICamera *camera = activeCamera();
    camera->setDistance(distance() + accumulatedPositionDelta().y + delta.y);
    camera->update();
    updateLastCursorPosition(logicalCursorPosition, delta);
    resetAllModelEdges();
}

const char *
CameraZoomState::name() const NANOEM_DECL_NOEXCEPT
{
    return "nanoem.gui.viewport.camera.zoom";
}

const nanoem_f32_t CameraZoomState::kDefaultScaleFactor = 0.5f;

CameraLookAtState::CameraLookAtState(Project *project, ICamera *camera, const Vector2 &pressedCursorPosition)
    : DraggingCameraState(project, camera, pressedCursorPosition)
{
    setScaleFactor(kDefaultScaleFactor);
}

void
CameraLookAtState::transform(const Vector2SI32 &logicalCursorPosition)
{
    const Vector3 delta(cursorDelta(logicalCursorPosition) * Vector2(1, -1), 0);
    Matrix4x4 viewMatrix, projectionMatrix;
    ICamera *camera = activeCamera();
    camera->getViewTransform(viewMatrix, projectionMatrix);
    camera->setLookAt(lookAt() + glm::inverse(Matrix3x3(viewMatrix)) * accumulatedPositionDelta() + delta);
    camera->update();
    updateLastCursorPosition(logicalCursorPosition, delta);
    resetAllModelEdges();
}

const char *
CameraLookAtState::name() const NANOEM_DECL_NOEXCEPT
{
    return "nanoem.gui.viewport.camera.look-at";
}

const nanoem_f32_t CameraLookAtState::kDefaultScaleFactor = 0.01f;

} /* namespace internal */
} /* namespace nanoem */
