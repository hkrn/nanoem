/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_EMAPP_INTERNAL_DRAGGINGCAMERASTATE
#define NANOEM_EMAPP_INTERNAL_DRAGGINGCAMERASTATE

#include "emapp/internal/IDraggingState.h"

namespace nanoem {

class ICamera;
class Project;

namespace internal {

class DraggingCameraState : public IDraggingState, private NonCopyable {
public:
    DraggingCameraState(Project *project, ICamera *camera, const Vector2SI32 &pressedCursorPosition);

    void commit(const Vector2SI32 &logicalCursorPosition) NANOEM_DECL_OVERRIDE;

    nanoem_f32_t scaleFactor() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setScaleFactor(nanoem_f32_t value) NANOEM_DECL_OVERRIDE;

protected:
    static Vector3 handleDragAxis(int index);
    ICamera *activeCamera() NANOEM_DECL_NOEXCEPT;
    Vector3 angle() const NANOEM_DECL_NOEXCEPT;
    Vector3 lookAt() const NANOEM_DECL_NOEXCEPT;
    Vector3 accumulatedPositionDelta() const NANOEM_DECL_NOEXCEPT;
    Vector2 cursorDelta(const Vector2 &logicalCursorPosition) const NANOEM_DECL_NOEXCEPT;
    nanoem_f32_t distance() const NANOEM_DECL_NOEXCEPT;
    void updateLastCursorPosition(const Vector2 &logicalCursorPosition, const Vector3 &delta);
    void resetAllModelEdges();

private:
    Project *m_project;
    ICamera *m_activeCamera;
    const Vector3 m_lookAt;
    const Vector3 m_angle;
    const nanoem_f32_t m_distance;
    const nanoem_f32_t m_fov;
    const bool m_perspective;
    Vector3 m_accumulatedPositionDelta;
    Vector2 m_lastPressedCursorPosition;
    nanoem_f32_t m_scaleFactor;
};

class AxisAlignedTranslateCameraState NANOEM_DECL_SEALED : public DraggingCameraState {
public:
    static const nanoem_f32_t kDefaultScaleFactor;

    AxisAlignedTranslateCameraState(
        Project *project, ICamera *camera, const Vector2SI32 &pressedCursorPosition, int axisIndex);

    void transform(const Vector2SI32 &logicalCursorPosition) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

    const int m_axisIndex;
};

class AxisAlignedOrientateCameraState NANOEM_DECL_SEALED : public DraggingCameraState {
public:
    static const nanoem_f32_t kDefaultScaleFactor;

    AxisAlignedOrientateCameraState(
        Project *project, ICamera *camera, const Vector2SI32 &pressedCursorPosition, int axisIndex);

    void transform(const Vector2SI32 &logicalCursorPosition) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

    const int m_axisIndex;
};

class CameraZoomState NANOEM_DECL_SEALED : public DraggingCameraState {
public:
    static const nanoem_f32_t kDefaultScaleFactor;

    CameraZoomState(Project *project, ICamera *camera, const Vector2SI32 &pressedCursorPosition);

    void transform(const Vector2SI32 &logicalCursorPosition) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class CameraLookAtState NANOEM_DECL_SEALED : public DraggingCameraState {
public:
    static const nanoem_f32_t kDefaultScaleFactor;

    CameraLookAtState(Project *project, ICamera *camera, const Vector2 &pressedCursorPosition);

    void transform(const Vector2SI32 &logicalCursorPosition) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_DRAGGINGCAMERASTATE */
