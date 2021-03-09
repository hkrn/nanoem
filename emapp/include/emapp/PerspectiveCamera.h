/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_PERSPECTIVECAMERA_H_
#define NANOEM_EMAPP_PERSPECTIVECAMERA_H_

#include "emapp/BezierCurve.h"
#include "emapp/ICamera.h"

struct undo_stack_t;

namespace nanoem {

class Motion;
class Project;

class PerspectiveCamera NANOEM_DECL_SEALED : public ICamera, private NonCopyable {
public:
    static const Vector3 kInitialLookAt;
    static const nanoem_f32_t kInitialDistance;
    static const nanoem_f32_t kInitialFovRadian;
    static const int kMaxFov;
    static const int kMinFov;
    static const int kInitialFov;
    static const glm::u8vec4 kDefaultBezierControlPoint;
    static const glm::u8vec4 kDefaultAutomaticBezierControlPoint;

    PerspectiveCamera(Project *project);
    ~PerspectiveCamera() NANOEM_DECL_NOEXCEPT;

    void destroy() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void reset() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void update() NANOEM_DECL_OVERRIDE;
    void synchronizeParameters(const Motion *motion, const nanoem_frame_index_t frameIndex) NANOEM_DECL_OVERRIDE;
    void synchronizeOutsideParent(const nanoem_motion_camera_keyframe_t *keyframe);
    void getViewTransform(Matrix4x4 &view, Matrix4x4 &projection) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    Vector3 unprojected(const Vector3 &value, nanoem_f32_t zfar = FLT_MAX) const NANOEM_DECL_NOEXCEPT;
    Vector2 toScreenCoordinate(const Vector3 &value) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    Vector2 toDeviceScreenCoordinateInViewport(const Vector3 &value) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    Vector2 toDeviceScreenCoordinateInWindow(const Vector3 &value) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    Ray createRay(const Vector2 &cursor, nanoem_f32_t zfar = FLT_MAX) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool intersectsRay(const Ray &ray, nanoem_f32_t intersectDistance) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    nanoem_f32_t aspectRatio() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    nanoem_f32_t zfar() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

    const Project *project() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    Project *project() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    StringPair outsideParent() const NANOEM_DECL_OVERRIDE;
    void setOutsideParent(const StringPair &value) NANOEM_DECL_OVERRIDE;
    Vector3 position() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    Vector3 direction() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    Vector3 lookAt() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    Vector3 boundLookAt() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setLookAt(const Vector3 &value) NANOEM_DECL_OVERRIDE;
    Vector3 angle() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setAngle(const Vector3 &value) NANOEM_DECL_OVERRIDE;
    nanoem_f32_t distance() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setDistance(nanoem_f32_t value) NANOEM_DECL_OVERRIDE;
    int fov() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setFov(int value) NANOEM_DECL_OVERRIDE;
    nanoem_f32_t fovRadians() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setFovRadians(nanoem_f32_t value) NANOEM_DECL_OVERRIDE;
    glm::u8vec4 automaticBezierControlPoint() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    glm::u8vec4 bezierControlPoints(
        nanoem_motion_camera_keyframe_interpolation_type_t index) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setBezierControlPoints(
        nanoem_motion_camera_keyframe_interpolation_type_t index, const glm::u8vec4 &value) NANOEM_DECL_OVERRIDE;
    bool isLinearInterpolation(
        nanoem_motion_camera_keyframe_interpolation_type_t index) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isPerspective() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setPerspective(bool value) NANOEM_DECL_OVERRIDE;
    bool isLocked() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setLocked(bool value) NANOEM_DECL_OVERRIDE;

    TransformCoordinateType transformCoordinateType() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setTransformCoordinateType(TransformCoordinateType value) NANOEM_DECL_OVERRIDE;
    void toggleTransformCoordinateType() NANOEM_DECL_OVERRIDE;
    FollowingType followingType() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setFollowingType(FollowingType value) NANOEM_DECL_OVERRIDE;
    bool isDirty() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setDirty(bool value) NANOEM_DECL_OVERRIDE;

private:
    Matrix4x4 internalPerspective(nanoem_f32_t zfar) const NANOEM_DECL_NOEXCEPT;
    Matrix4x4 internalDevicePerspective(nanoem_f32_t zfar) const NANOEM_DECL_NOEXCEPT;
    nanoem_f32_t bezierCurve(const nanoem_motion_camera_keyframe_t *prev, const nanoem_motion_camera_keyframe_t *next,
        nanoem_motion_camera_keyframe_interpolation_type_t index, nanoem_f32_t value) const;

    typedef tinystl::unordered_map<const nanoem_motion_camera_keyframe_t *, BezierCurve *, TinySTLAllocator>
        KeyframeBezierCurveMap;
    mutable BezierCurve::Map m_bezierCurvesData;
    mutable KeyframeBezierCurveMap m_keyframeBezierCurves;
    Project *m_project;
    undo_stack_t *m_undoStack;
    StringPair m_outsideParent;
    TransformCoordinateType m_transformCoordinateType;
    Matrix4x4 m_viewMatrix;
    Matrix4x4 m_projectionMatrix;
    Vector3 m_position;
    Vector3 m_direction;
    Vector3 m_lookAt;
    Vector3 m_angle;
    nanoem_f32_t m_distance;
    tinystl::pair<int, nanoem_f32_t> m_fov;
    glm::u8vec4 m_bezierControlPoints[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM];
    glm::u8vec4 m_automaticBezierControlPoint;
    FollowingType m_followingType;
    bool m_isLinearInterpolation[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM];
    bool m_perspective;
    bool m_locked;
    bool m_dirty;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_PERSPECTIVECAMERA_H_ */
