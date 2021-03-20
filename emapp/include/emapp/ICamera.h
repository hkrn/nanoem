/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_ICAMERA_H_
#define NANOEM_EMAPP_ICAMERA_H_

#include "emapp/Forward.h"

#include "emapp/Ray.h"

namespace nanoem {

class Motion;
class Project;

class ICamera {
public:
    enum FollowingType {
        kFollowingTypeFirstEnum,
        kFollowingTypeNone,
        kFollowingTypeModel,
        kFollowingTypeBone,
        kFollowingTypeMaxEnum
    };
    enum TransformCoordinateType {
        kTransformCoordinateTypeFirstEnum,
        kTransformCoordinateTypeGlobal = kTransformCoordinateTypeFirstEnum,
        kTransformCoordinateTypeLocal,
        kTransformCoordinateTypeMaxEnum
    };

    virtual ~ICamera() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual void destroy() NANOEM_DECL_NOEXCEPT = 0;
    virtual void reset() NANOEM_DECL_NOEXCEPT = 0;
    virtual void update() = 0;
    virtual void synchronizeParameters(const Motion *motion, const nanoem_frame_index_t frameIndex) = 0;
    virtual void getViewTransform(Matrix4x4 &view, Matrix4x4 &projection) const NANOEM_DECL_NOEXCEPT = 0;
    virtual Vector2 toScreenCoordinate(const Vector3 &value) const NANOEM_DECL_NOEXCEPT = 0;
    virtual Vector2 toDeviceScreenCoordinateInViewport(const Vector3 &value) const NANOEM_DECL_NOEXCEPT = 0;
    virtual Vector2 toDeviceScreenCoordinateInWindow(const Vector3 &value) const NANOEM_DECL_NOEXCEPT = 0;
    virtual Ray createRay(const Vector2 &cursor, nanoem_f32_t zfar = FLT_MAX) const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool intersectsRay(const Ray &ray, nanoem_f32_t intersectDistance) const NANOEM_DECL_NOEXCEPT = 0;
    virtual nanoem_f32_t aspectRatio() const NANOEM_DECL_NOEXCEPT = 0;
    virtual nanoem_f32_t zfar() const NANOEM_DECL_NOEXCEPT = 0;

    virtual const Project *project() const NANOEM_DECL_NOEXCEPT = 0;
    virtual Project *project() NANOEM_DECL_NOEXCEPT = 0;
    virtual StringPair outsideParent() const = 0;
    virtual void setOutsideParent(const StringPair &value) = 0;
    virtual Vector3 position() const NANOEM_DECL_NOEXCEPT = 0;
    virtual Vector3 direction() const NANOEM_DECL_NOEXCEPT = 0;
    virtual Vector3 lookAt() const NANOEM_DECL_NOEXCEPT = 0;
    virtual Vector3 boundLookAt() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setLookAt(const Vector3 &value) = 0;
    virtual Vector3 angle() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setAngle(const Vector3 &value) = 0;
    virtual nanoem_f32_t distance() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setDistance(nanoem_f32_t value) = 0;
    virtual int fov() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setFov(int value) = 0;
    virtual nanoem_f32_t fovRadians() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setFovRadians(nanoem_f32_t value) = 0;
    virtual Vector4U8 automaticBezierControlPoint() const NANOEM_DECL_NOEXCEPT = 0;
    virtual Vector4U8 bezierControlPoints(
        nanoem_motion_camera_keyframe_interpolation_type_t index) const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setBezierControlPoints(
        nanoem_motion_camera_keyframe_interpolation_type_t index, const Vector4U8 &value) = 0;
    virtual bool isLinearInterpolation(
        nanoem_motion_camera_keyframe_interpolation_type_t index) const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool isPerspective() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setPerspective(bool value) = 0;
    virtual bool isLocked() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setLocked(bool value) = 0;

    virtual TransformCoordinateType transformCoordinateType() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setTransformCoordinateType(TransformCoordinateType value) = 0;
    virtual void toggleTransformCoordinateType() = 0;
    virtual FollowingType followingType() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setFollowingType(FollowingType value) = 0;
    virtual bool isDirty() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setDirty(bool value) = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_PERSPECTIVECAMERA_H_ */
