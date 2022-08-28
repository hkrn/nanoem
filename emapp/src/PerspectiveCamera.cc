/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/PerspectiveCamera.h"

#include "emapp/Model.h"
#include "emapp/Motion.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtx/intersect.hpp"
#include "glm/gtx/quaternion.hpp"

#include "undo/undo.h"

namespace nanoem {
namespace {

static Vector3
bonePosition(const nanoem_model_bone_t *bonePtr)
{
    Vector3 at(Constants::kZeroV3);
    if (bonePtr) {
        const model::Bone *bone = model::Bone::cast(bonePtr);
        at = Vector3(bone->skinningTransform() * Vector4(model::Bone::origin(bonePtr), 1));
    }
    return at;
}

} /* namespace anonymous */

const Vector3 PerspectiveCamera::kAngleScaleFactor = Vector3(-1, 1, 1);
const Vector3 PerspectiveCamera::kInitialLookAt = Vector3(0, 10, 0);
const nanoem_f32_t PerspectiveCamera::kInitialDistance = 45.0f;
const nanoem_f32_t PerspectiveCamera::kInitialFovRadian = glm::radians(nanoem_f32_t(PerspectiveCamera::kInitialFov));
const int PerspectiveCamera::kMaxFov = 135;
const int PerspectiveCamera::kMinFov = 1;
const int PerspectiveCamera::kInitialFov = 30;
const Vector4U8 PerspectiveCamera::kDefaultBezierControlPoint = Vector4U8(20, 20, 107, 107);
const Vector4U8 PerspectiveCamera::kDefaultAutomaticBezierControlPoint = Vector4U8(64, 0, 64, 127);

PerspectiveCamera::PerspectiveCamera(Project *project)
    : m_project(project)
    , m_undoStack(nullptr)
    , m_transformCoordinateType(kTransformCoordinateTypeLocal)
    , m_position(Constants::kZeroV3)
    , m_direction(Constants::kUnitZ)
    , m_lookAt(kInitialLookAt)
    , m_distance(kInitialDistance)
    , m_fov(kInitialFov, kInitialFovRadian)
    , m_followingType(kFollowingTypeNone)
    , m_perspective(true)
    , m_locked(false)
    , m_dirty(false)
{
    nanoem_parameter_assert(m_project, "must NOT be nullptr");
    for (size_t i = 0; i < BX_COUNTOF(m_bezierControlPoints); i++) {
        m_bezierControlPoints[i] = kDefaultBezierControlPoint;
        m_isLinearInterpolation[i] = true;
    }
    m_undoStack = undoStackCreateWithSoftLimit(undoStackGetSoftLimit(m_project->undoStack()));
}

PerspectiveCamera::~PerspectiveCamera() NANOEM_DECL_NOEXCEPT
{
    for (BezierCurve::Map::const_iterator it = m_bezierCurvesData.begin(), end = m_bezierCurvesData.end(); it != end;
         ++it) {
        nanoem_delete(it->second);
    }
    m_bezierCurvesData.clear();
    undoStackDestroy(m_undoStack);
    m_undoStack = nullptr;
}

void
PerspectiveCamera::destroy() NANOEM_DECL_NOEXCEPT
{
    undoStackClear(m_undoStack);
}

void
PerspectiveCamera::reset() NANOEM_DECL_NOEXCEPT
{
    m_perspective = true;
    m_angle = Vector3(0);
    m_lookAt = kInitialLookAt;
    m_distance = kInitialDistance;
    m_fov = tinystl::make_pair(kInitialFov, kInitialFovRadian);
    setDirty(true);
    for (size_t i = 0; i < BX_COUNTOF(m_bezierControlPoints); i++) {
        m_bezierControlPoints[i] = kDefaultBezierControlPoint;
        m_isLinearInterpolation[i] = true;
    }
}

void
PerspectiveCamera::update()
{
    const Vector3 lookAt(boundLookAt());
    const Vector3 angle(m_angle * kAngleScaleFactor);
    const Quaternion x(glm::angleAxis(angle.x, Constants::kUnitX));
    const Quaternion y(glm::angleAxis(angle.y, Constants::kUnitY));
    const Quaternion z(glm::angleAxis(angle.z, Constants::kUnitZ));
    const Matrix3x3 viewOrientation(glm::mat3_cast(z * x * y));
    m_viewMatrix = Matrix4x4(viewOrientation) * glm::translate(Constants::kIdentity, -lookAt);
    m_viewMatrix[3] += Vector4(0, 0, m_distance, 0);
    const Vector3 position(glm::affineInverse(m_viewMatrix)[3]);
    if (m_distance > 0) {
        m_direction = glm::normalize(lookAt - position);
    }
    else if (m_distance < 0) {
        m_direction = glm::normalize(position - lookAt);
    }
    m_position = position;
    if (m_perspective) {
        m_fov.second = glm::max(glm::radians(1.0f), m_fov.second);
        m_projectionMatrix = glm::infinitePerspective(m_fov.second, aspectRatio(), 0.5f);
    }
    else {
        const Vector2 viewportImageSize(m_project->viewportImageSize());
        const nanoem_f32_t inverseDistance = 1.0f / m_distance;
        Matrix4x4 projectionMatrix(1);
        projectionMatrix[0][0] = 2.0f * glm::max(viewportImageSize.y / viewportImageSize.x, 1.0f) * inverseDistance;
        projectionMatrix[1][1] = 2.0f * glm::max(viewportImageSize.x / viewportImageSize.y, 1.0f) * inverseDistance;
        projectionMatrix[2][2] = 2.0f / (zfar() - 0.5f);
        m_projectionMatrix = projectionMatrix;
    }
}

void
PerspectiveCamera::synchronizeParameters(const Motion *motion, const nanoem_frame_index_t frameIndex)
{
    static const nanoem_f32_t kDistanceFactor = -1.0f;
    nanoem_parameter_assert(motion, "must not be nullptr");
    m_outsideParent = StringPair();
    if (const nanoem_motion_camera_keyframe_t *keyframe = motion->findCameraKeyframe(frameIndex)) {
        setLookAt(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(keyframe)));
        setAngle(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(keyframe)));
        setFov(nanoemMotionCameraKeyframeGetFov(keyframe));
        setDistance(nanoemMotionCameraKeyframeGetDistance(keyframe) * kDistanceFactor);
        setPerspective(nanoemMotionCameraKeyframeIsPerspectiveView(keyframe) != 0);
        for (int i = 0; i < int(BX_COUNTOF(m_bezierControlPoints)); i++) {
            nanoem_motion_camera_keyframe_interpolation_type_t type =
                nanoem_motion_camera_keyframe_interpolation_type_t(i);
            if (nanoemMotionCameraKeyframeIsLinearInterpolation(keyframe, type)) {
                m_bezierControlPoints[i] = kDefaultBezierControlPoint;
                m_isLinearInterpolation[i] = true;
            }
            else {
                m_bezierControlPoints[i] = glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe, type));
                m_isLinearInterpolation[i] = false;
            }
        }
        synchronizeOutsideParent(keyframe);
    }
    else {
        nanoem_motion_camera_keyframe_t *prevKeyframe, *nextKeyframe;
        nanoemMotionSearchClosestCameraKeyframes(motion->data(), frameIndex, &prevKeyframe, &nextKeyframe);
        if (prevKeyframe && nextKeyframe) {
            const nanoem_motion_camera_keyframe_t *interpolateKeyframe = nextKeyframe;
            const nanoem_f32_t coef = Motion::coefficient(prevKeyframe, nextKeyframe, frameIndex);
            const Vector3 prevLookAt = glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(prevKeyframe));
            const Vector3 nextLookAt = glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(nextKeyframe));
            if (nanoemMotionCameraKeyframeIsLinearInterpolation(
                    interpolateKeyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X) &&
                nanoemMotionCameraKeyframeIsLinearInterpolation(
                    interpolateKeyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y) &&
                nanoemMotionCameraKeyframeIsLinearInterpolation(
                    interpolateKeyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z)) {
                setLookAt(glm::mix(prevLookAt, nextLookAt, coef));
                for (int i = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X;
                     i <= NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z; i++) {
                    m_bezierControlPoints[i] = kDefaultBezierControlPoint;
                    m_isLinearInterpolation[i] = true;
                }
            }
            else {
                Vector3 lookAt;
                for (int i = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X;
                     i <= NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z; i++) {
                    const glm::highp_float32_t v0 = prevLookAt[i], v1 = nextLookAt[i];
                    nanoem_motion_camera_keyframe_interpolation_type_t type =
                        nanoem_motion_camera_keyframe_interpolation_type_t(i);
                    if (nanoemMotionCameraKeyframeIsLinearInterpolation(interpolateKeyframe, type)) {
                        lookAt[i] = glm::mix(v0, v1, coef);
                        m_bezierControlPoints[i] = kDefaultBezierControlPoint;
                        m_isLinearInterpolation[i] = true;
                    }
                    else {
                        const nanoem_f32_t t2 = bezierCurve(prevKeyframe, nextKeyframe, type, coef);
                        lookAt[i] = glm::mix(v0, v1, t2);
                        m_bezierControlPoints[i] =
                            glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(interpolateKeyframe, type));
                        m_isLinearInterpolation[i] = false;
                    }
                }
                setLookAt(lookAt);
            }
            const Vector3 angle0(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(prevKeyframe))),
                angle1(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(nextKeyframe)));
            if (nanoemMotionCameraKeyframeIsLinearInterpolation(
                    interpolateKeyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE)) {
                setAngle(glm::mix(angle0, angle1, coef));
                m_bezierControlPoints[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE] =
                    kDefaultBezierControlPoint;
                m_isLinearInterpolation[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE] = true;
            }
            else {
                const nanoem_f32_t &t2 = bezierCurve(
                    prevKeyframe, nextKeyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE, coef);
                setAngle(glm::mix(angle0, angle1, t2));
                m_bezierControlPoints[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE] =
                    glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                        interpolateKeyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE));
                m_isLinearInterpolation[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE] = false;
            }
            nanoem_f32_t fov0 = nanoem_f32_t(nanoemMotionCameraKeyframeGetFov(prevKeyframe));
            nanoem_f32_t fov1 = nanoem_f32_t(nanoemMotionCameraKeyframeGetFov(nextKeyframe));
            if (nanoemMotionCameraKeyframeIsLinearInterpolation(
                    interpolateKeyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV)) {
                setFovRadians(glm::radians(glm::mix(fov0, fov1, coef)));
                m_bezierControlPoints[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV] =
                    kDefaultBezierControlPoint;
                m_isLinearInterpolation[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV] = true;
            }
            else {
                const nanoem_f32_t &t2 =
                    bezierCurve(prevKeyframe, nextKeyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV, coef);
                setFovRadians(glm::radians(glm::mix(fov0, fov1, t2)));
                m_bezierControlPoints[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV] =
                    glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                        interpolateKeyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV));
                m_isLinearInterpolation[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV] = false;
            }
            const nanoem_f32_t distance0 = nanoemMotionCameraKeyframeGetDistance(prevKeyframe) * kDistanceFactor;
            const nanoem_f32_t distance1 = nanoemMotionCameraKeyframeGetDistance(nextKeyframe) * kDistanceFactor;
            if (nanoemMotionCameraKeyframeIsLinearInterpolation(
                    interpolateKeyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE)) {
                setDistance(glm::mix(distance0, distance1, coef));
                m_bezierControlPoints[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE] =
                    kDefaultBezierControlPoint;
                m_isLinearInterpolation[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE] = true;
            }
            else {
                const nanoem_f32_t &t2 = bezierCurve(
                    prevKeyframe, nextKeyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE, coef);
                setDistance(glm::mix(distance0, distance1, t2));
                m_bezierControlPoints[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE] =
                    glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                        interpolateKeyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE));
                m_isLinearInterpolation[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE] = false;
            }
            setPerspective(nanoemMotionCameraKeyframeIsPerspectiveView(prevKeyframe) != 0);
            synchronizeOutsideParent(prevKeyframe);
        }
    }
}

void
PerspectiveCamera::synchronizeOutsideParent(const nanoem_motion_camera_keyframe_t *keyframe)
{
    if (const nanoem_motion_outside_parent_t *op = nanoemMotionCameraKeyframeGetOutsideParent(keyframe)) {
        String modelName;
        nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
        StringUtils::getUtf8String(nanoemMotionOutsideParentGetTargetObjectName(op), factory, modelName);
        if (const Model *model = m_project->findModelByName(modelName)) {
            m_outsideParent.first = model->name();
        }
        StringUtils::getUtf8String(nanoemMotionOutsideParentGetTargetBoneName(op), factory, m_outsideParent.second);
    }
}

void
PerspectiveCamera::getViewTransform(Matrix4x4 &view, Matrix4x4 &projection) const NANOEM_DECL_NOEXCEPT
{
    view = m_viewMatrix;
    projection = m_projectionMatrix;
}

Vector3
PerspectiveCamera::unprojected(const Vector3 &value) const NANOEM_DECL_NOEXCEPT
{
    const Vector4 viewport(0, 0, m_project->logicalScaleUniformedViewportImageSize());
    const Vector3 coordinate(glm::unProject(value, m_viewMatrix, m_projectionMatrix, viewport));
    return coordinate;
}

Vector2SI32
PerspectiveCamera::toDeviceScreenCoordinateInViewport(const Vector3 &value) const NANOEM_DECL_NOEXCEPT
{
    const Vector4UI16 layoutRect(m_project->deviceScaleUniformedViewportLayoutRect());
    const Vector2 imageSize(m_project->deviceScaleUniformedViewportImageSize());
    const Vector4 viewportRect(0, 0, imageSize.x, imageSize.y);
    const nanoem_f32_t x = (layoutRect.z - imageSize.x) * 0.5f, y = (layoutRect.w - imageSize.y) * 0.5f;
    const Vector2 coordinate(glm::project(value, m_viewMatrix, m_projectionMatrix, viewportRect));
    return Vector2SI32(x + coordinate.x, layoutRect.w - coordinate.y - y);
}

Vector2SI32
PerspectiveCamera::toDeviceScreenCoordinateInWindow(const Vector3 &value) const NANOEM_DECL_NOEXCEPT
{
    const Vector2SI32 layoutRect(m_project->deviceScaleUniformedViewportLayoutRect());
    return layoutRect + toDeviceScreenCoordinateInViewport(value);
}

Ray
PerspectiveCamera::createRay(const Vector2SI32 &value) const NANOEM_DECL_NOEXCEPT
{
    const Vector2SI32 coord(m_project->resolveLogicalCursorPositionInViewport(value));
    Ray ray;
    ray.from = unprojected(Vector3(coord, 0));
    ray.to = unprojected(Vector3(coord, 1 - Constants::kEpsilon));
    ray.direction =
        glm::abs(glm::distance(ray.to, ray.from)) > 0 ? glm::normalize(ray.to - ray.from) : Constants::kUnitZ;
    return ray;
}

bool
PerspectiveCamera::castRay(const Vector2SI32 &position, Vector3 &intersection) const NANOEM_DECL_NOEXCEPT
{
    const Ray ray(createRay(position));
    float distance;
    bool intersected = glm::intersectRayPlane(ray.from, ray.direction, Constants::kZeroV3, -direction(), distance);
    intersection = ray.from + ray.direction * distance;
    return intersected;
}

nanoem_f32_t
PerspectiveCamera::aspectRatio() const NANOEM_DECL_NOEXCEPT
{
    const Vector4 viewport(glm::max(m_project->logicalScaleUniformedViewportImageRect(), Vector4(1)));
    const nanoem_f32_t value = viewport.z / viewport.w;
    return value;
}

nanoem_f32_t
PerspectiveCamera::zfar() const NANOEM_DECL_NOEXCEPT
{
    return 10000.0f;
}

const Project *
PerspectiveCamera::project() const NANOEM_DECL_NOEXCEPT
{
    return m_project;
}

Project *
PerspectiveCamera::project() NANOEM_DECL_NOEXCEPT
{
    return m_project;
}

StringPair
PerspectiveCamera::outsideParent() const
{
    return m_outsideParent;
}

void
PerspectiveCamera::setOutsideParent(const StringPair &value)
{
    m_outsideParent = value;
}

Vector3
PerspectiveCamera::position() const NANOEM_DECL_NOEXCEPT
{
    return m_position;
}

Vector3
PerspectiveCamera::direction() const NANOEM_DECL_NOEXCEPT
{
    return m_direction;
}

Vector3
PerspectiveCamera::lookAt() const NANOEM_DECL_NOEXCEPT
{
    Vector3 at(Constants::kZeroV3);
    switch (m_followingType) {
    case kFollowingTypeBone: {
        if (const Model *model = m_project->activeModel()) {
            at = bonePosition(model->activeBone());
        }
        break;
    }
    case kFollowingTypeModel: {
        if (const Model *model = m_project->activeModel()) {
            if (const nanoem_model_bone_t *bonePtr =
                    model->findBone(reinterpret_cast<const char *>(model::Bone::kNameCenterOfViewportInJapanese))) {
                at = m_lookAt + bonePosition(bonePtr);
            }
            else if (const nanoem_model_bone_t *bonePtr =
                         model->findBone(reinterpret_cast<const char *>(model::Bone::kNameCenterOffsetInJapanese))) {
                at = m_lookAt + bonePosition(nanoemModelBoneGetParentBoneObject(bonePtr));
            }
            else {
                nanoem_rsize_t numBones;
                nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
                at = numBones > 0 ? m_lookAt + bonePosition(bones[0]) : m_lookAt;
            }
        }
        else {
            at = m_lookAt;
        }
        break;
    }
    case kFollowingTypeNone:
    default:
        at = m_lookAt;
        break;
    }
    return at;
}

Vector3
PerspectiveCamera::boundLookAt() const NANOEM_DECL_NOEXCEPT
{
    const model::Bone *bone = model::Bone::cast(m_project->resolveBone(m_outsideParent));
    return (bone ? bone->worldTransformOrigin() : Constants::kZeroV3) + (lookAt() * Constants::kTranslateDirection);
}

void
PerspectiveCamera::setLookAt(const Vector3 &value)
{
    if (!isLocked() && !glm::all(glm::epsilonEqual(value, m_lookAt, Constants::kEpsilon))) {
        m_lookAt = value;
        setDirty(true);
    }
}

Vector3
PerspectiveCamera::angle() const NANOEM_DECL_NOEXCEPT
{
    return m_angle;
}

void
PerspectiveCamera::setAngle(const Vector3 &value)
{
    if (!isLocked() && !glm::all(glm::epsilonEqual(value, m_angle, Constants::kEpsilon))) {
        m_angle = value;
        setDirty(true);
    }
}

nanoem_f32_t
PerspectiveCamera::distance() const NANOEM_DECL_NOEXCEPT
{
    return m_distance;
}

void
PerspectiveCamera::setDistance(nanoem_f32_t value)
{
    if (!isLocked() && !glm::epsilonEqual(m_distance, value, Constants::kEpsilon)) {
        m_distance = value;
        setDirty(true);
    }
}

int
PerspectiveCamera::fov() const NANOEM_DECL_NOEXCEPT
{
    return m_fov.first;
}

void
PerspectiveCamera::setFov(int value)
{
    if (!isLocked() && value != m_fov.first) {
        m_fov.first = value;
        m_fov.second = glm::radians(nanoem_f32_t(value));
        setDirty(true);
    }
}

nanoem_f32_t
PerspectiveCamera::fovRadians() const NANOEM_DECL_NOEXCEPT
{
    return m_fov.second;
}

void
PerspectiveCamera::setFovRadians(nanoem_f32_t value)
{
    if (!isLocked() && !glm::epsilonEqual(m_fov.second, value, Constants::kEpsilon)) {
        m_fov.second = value;
        m_fov.first = int(glm::round(glm::degrees(value)));
        setDirty(true);
    }
}

Vector4U8
PerspectiveCamera::bezierControlPoints(
    nanoem_motion_camera_keyframe_interpolation_type_t index) const NANOEM_DECL_NOEXCEPT
{
    return m_bezierControlPoints[index];
}

void
PerspectiveCamera::setBezierControlPoints(
    nanoem_motion_camera_keyframe_interpolation_type_t index, const Vector4U8 &value)
{
    m_bezierControlPoints[index] = value;
}

bool
PerspectiveCamera::isLinearInterpolation(
    nanoem_motion_camera_keyframe_interpolation_type_t index) const NANOEM_DECL_NOEXCEPT
{
    return m_isLinearInterpolation[index];
}

bool
PerspectiveCamera::isPerspective() const NANOEM_DECL_NOEXCEPT
{
    return m_perspective;
}

void
PerspectiveCamera::setPerspective(bool value)
{
    if (!isLocked() && value != m_perspective) {
        m_perspective = value;
        setDirty(true);
    }
}

bool
PerspectiveCamera::isLocked() const NANOEM_DECL_NOEXCEPT
{
    return m_locked;
}

void
PerspectiveCamera::setLocked(bool value)
{
    m_locked = value;
}

PerspectiveCamera::TransformCoordinateType
PerspectiveCamera::transformCoordinateType() const NANOEM_DECL_NOEXCEPT
{
    return m_transformCoordinateType;
}

void
PerspectiveCamera::setTransformCoordinateType(TransformCoordinateType value)
{
    m_transformCoordinateType = value;
}

void
PerspectiveCamera::toggleTransformCoordinateType()
{
    switch (transformCoordinateType()) {
    case kTransformCoordinateTypeGlobal:
    default:
        setTransformCoordinateType(kTransformCoordinateTypeLocal);
        break;
    case kTransformCoordinateTypeLocal:
        setTransformCoordinateType(kTransformCoordinateTypeGlobal);
        break;
    }
}

ICamera::FollowingType
PerspectiveCamera::followingType() const NANOEM_DECL_NOEXCEPT
{
    return m_followingType;
}

void
PerspectiveCamera::setFollowingType(FollowingType value)
{
    m_followingType = value;
    update();
}

bool
PerspectiveCamera::isDirty() const NANOEM_DECL_NOEXCEPT
{
    return m_dirty;
}

void
PerspectiveCamera::setDirty(bool value)
{
    m_dirty = value;
}

nanoem_f32_t
PerspectiveCamera::bezierCurve(const nanoem_motion_camera_keyframe_t *prev, const nanoem_motion_camera_keyframe_t *next,
    nanoem_motion_camera_keyframe_interpolation_type_t index, nanoem_f32_t value) const
{
    const nanoem_u8_t *parameters = nanoemMotionCameraKeyframeGetInterpolation(next, index);
    BezierCurve *curve;
    KeyframeBezierCurveMap::const_iterator it = m_keyframeBezierCurves.find(next);
    if (it != m_keyframeBezierCurves.end()) {
        curve = it->second;
    }
    else {
        const nanoem_frame_index_t interval =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(next)) -
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(prev));
        const nanoem_u64_t hash = BezierCurve::toHash(parameters, interval);
        BezierCurve::Map::const_iterator it2 = m_bezierCurvesData.find(hash);
        if (it2 != m_bezierCurvesData.end()) {
            curve = it2->second;
            m_keyframeBezierCurves.insert(tinystl::make_pair(next, curve));
        }
        else {
            const Vector2 c0(parameters[0], parameters[1]), c1(parameters[2], parameters[3]);
            curve = nanoem_new(BezierCurve(c0, c1, interval));
            m_bezierCurvesData.insert(tinystl::make_pair(hash, curve));
            m_keyframeBezierCurves.insert(tinystl::make_pair(next, curve));
        }
    }
    return curve->value(value);
}

} /* namespace nanoem */
