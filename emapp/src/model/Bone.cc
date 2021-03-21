/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/model/Bone.h"

#include "emapp/Model.h"
#include "emapp/Motion.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#include "glm/gtx/vector_query.hpp"

namespace nanoem {
namespace model {
namespace {

static inline void
identify(bx::float4x4_t *o) NANOEM_DECL_NOEXCEPT
{
    o->col[0] = bx::simd_ld(1, 0, 0, 0);
    o->col[1] = bx::simd_ld(0, 1, 0, 0);
    o->col[2] = bx::simd_ld(0, 0, 1, 0);
    o->col[3] = bx::simd_ld(0, 0, 0, 1);
}

static inline void
translate(const Vector3 &v, const bx::float4x4_t *m, bx::float4x4_t *o) NANOEM_DECL_NOEXCEPT
{
    const bx::float4x4_t base = *m;
    *o = base;
    o->col[3] = bx::simd_mul_xyz1(bx::simd_ld(v.x, v.y, v.z, 0), &base);
}

static inline void
shrink3x3(const bx::float4x4_t *m, bx::float4x4_t *o) NANOEM_DECL_NOEXCEPT
{
    *o = *m;
    o->col[3] = bx::simd_ld(0, 0, 0, 1);
}

} /* anonymous */

const Vector4U8 Bone::kDefaultBezierControlPoint = Vector4U8(20, 20, 107, 107);
const Vector4U8 Bone::kDefaultAutomaticBezierControlPoint = Vector4U8(64, 0, 64, 127);

const Bone::FrameTransform Bone::FrameTransform::kInitialFrameTransform = FrameTransform();

Bone::FrameTransform::FrameTransform()
    : m_translation(Constants::kZeroV3)
    , m_orientation(Constants::kZeroQ)
{
    Inline::clearZeroMemory(m_bezierControlPoints);
    Inline::clearZeroMemory(m_enableLinearInterpolation);
    for (int i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
         i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
        m_bezierControlPoints[i] = kDefaultBezierControlPoint;
        m_enableLinearInterpolation[i] = true;
    }
}

Bone::FrameTransform::~FrameTransform() NANOEM_DECL_NOEXCEPT
{
}

Bone::~Bone() NANOEM_DECL_NOEXCEPT
{
}

void
Bone::bind(nanoem_model_bone_t *bone)
{
    nanoem_parameter_assert(bone, "must not be nullptr");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_user_data_t *userData = nanoemUserDataCreate(&status);
    nanoemUserDataSetOnDestroyModelObjectCallback(userData, &Bone::destroy);
    nanoemUserDataSetOpaqueData(userData, this);
    nanoemModelObjectSetUserData(nanoemModelBoneGetModelObjectMutable(bone), userData);
}

void
Bone::resetLanguage(
    const nanoem_model_bone_t *bone, nanoem_unicode_string_factory_t *factory, nanoem_language_type_t language)
{
    StringUtils::getUtf8String(nanoemModelBoneGetName(bone, language), factory, m_name);
    if (m_canonicalName.empty()) {
        StringUtils::getUtf8String(
            nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), factory, m_canonicalName);
        if (m_canonicalName.empty()) {
            StringUtils::format(m_canonicalName, "Bone%d", index(bone));
        }
    }
    if (m_name.empty()) {
        m_name = m_canonicalName;
    }
}

void
Bone::resetLocalTransform() NANOEM_DECL_NOEXCEPT
{
    m_localOrientation = m_localInherentOrientation = Constants::kZeroQ;
    m_localTranslation = m_localInherentTranslation = Constants::kZeroV3;
    for (size_t i = 0; i < BX_COUNTOF(m_bezierControlPoints); i++) {
        m_bezierControlPoints[i] = kDefaultBezierControlPoint;
    }
}

void
Bone::resetUserTransform() NANOEM_DECL_NOEXCEPT
{
    m_localUserOrientation = Constants::kZeroQ;
    m_localUserTranslation = Constants::kZeroV3;
    m_dirty = false;
}

void
Bone::resetMorphTransform() NANOEM_DECL_NOEXCEPT
{
    m_localMorphOrientation = Constants::kZeroQ;
    m_localMorphTranslation = Constants::kZeroV3;
}

void
Bone::synchronizeMotion(const Motion *motion, const nanoem_model_bone_t *bone,
    const nanoem_model_rigid_body_t *rigidBodyPtr, nanoem_frame_index_t frameIndex, nanoem_f32_t amount)
{
    nanoem_parameter_assert(bone, "must not be nullptr");
    FrameTransform t0(FrameTransform::kInitialFrameTransform), t1(FrameTransform::kInitialFrameTransform);
    synchronizeTransform(motion, bone, rigidBodyPtr, frameIndex, t0);
    if (amount > 0) {
        synchronizeTransform(motion, bone, nullptr, frameIndex + 1, t1);
        setLocalUserTranslation(glm::mix(t0.m_translation, t1.m_translation, amount));
        setLocalUserOrientation(glm::slerp(t0.m_orientation, t1.m_orientation, amount));
        for (size_t i = 0; i < BX_COUNTOF(m_bezierControlPoints); i++) {
            m_bezierControlPoints[i] = glm::mix(t0.m_bezierControlPoints[i], t1.m_bezierControlPoints[i], amount);
            m_isLinearInterpolation[i] = t0.m_enableLinearInterpolation[i];
        }
    }
    else {
        setLocalUserTranslation(t0.m_translation);
        setLocalUserOrientation(t0.m_orientation);
        for (size_t i = 0; i < BX_COUNTOF(m_bezierControlPoints); i++) {
            m_bezierControlPoints[i] = t0.m_bezierControlPoints[i];
            m_isLinearInterpolation[i] = t0.m_enableLinearInterpolation[i];
        }
    }
}

void
Bone::updateLocalOrientation(const nanoem_model_bone_t *bone, const Model *model) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(bone, "must not be nullptr");
    if (nanoemModelBoneHasInherentOrientation(bone)) {
        const nanoem_model_bone_t *parentBonePtr = nanoemModelBoneGetInherentParentBoneObject(bone);
        Quaternion orientation(Constants::kZeroQ);
        if (const Bone *parentBone = cast(parentBonePtr)) {
            if (nanoemModelBoneHasLocalInherent(parentBonePtr)) {
                orientation = glm::quat_cast(parentBone->localTransform()) * orientation;
            }
            else if (model && model->isConstraintJointBoneActive(parentBonePtr)) {
                orientation = parentBone->constraintJointOrientation() * orientation;
            }
            else {
                if (nanoemModelBoneHasInherentOrientation(parentBonePtr)) {
                    orientation = parentBone->localInherentOrientation() * orientation;
                }
                else {
                    orientation = parentBone->localUserOrientation() * orientation;
                }
            }
        }
        nanoem_f32_t coefficient = nanoemModelBoneGetInherentCoefficient(bone);
        if (glm::abs(coefficient - 1.0f) > 0.0f) {
            const Bone *targetUserData = cast(nanoemModelBoneGetEffectorBoneObject(bone));
            if (targetUserData) {
                orientation = glm::slerp(orientation, targetUserData->localUserOrientation(), coefficient);
            }
            else {
                orientation = glm::slerp(Constants::kZeroQ, orientation, coefficient);
            }
        }
        if (model && model->isConstraintJointBoneActive(bone)) {
            m_localOrientation = glm::normalize(m_constraintJointOrientation * m_localMorphOrientation * orientation);
        }
        else {
            m_localOrientation = glm::normalize(m_localMorphOrientation * m_localUserOrientation * orientation);
        }
        m_localInherentOrientation = orientation;
    }
    else if (model && model->isConstraintJointBoneActive(bone)) {
        m_localOrientation = glm::normalize(m_constraintJointOrientation * m_localMorphOrientation);
    }
    else {
        m_localOrientation = glm::normalize(m_localMorphOrientation * m_localUserOrientation);
    }
}

void
Bone::updateLocalTranslation(const nanoem_model_bone_t *bone) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(bone, "must not be nullptr");
    Vector3 translation(m_localUserTranslation);
    if (nanoemModelBoneHasInherentTranslation(bone)) {
        const nanoem_model_bone_t *parentBonePtr = nanoemModelBoneGetInherentParentBoneObject(bone);
        if (const Bone *parentBone = cast(nanoemModelBoneGetInherentParentBoneObject(bone))) {
            if (nanoemModelBoneHasLocalInherent(parentBonePtr)) {
                translation += parentBone->localTransformOrigin();
            }
            else if (nanoemModelBoneHasInherentTranslation(parentBonePtr)) {
                translation += parentBone->localInherentTranslation();
            }
            else {
                translation += parentBone->localTranslation() * parentBone->localMorphTranslation();
            }
        }
        nanoem_f32_t coefficient = nanoemModelBoneGetInherentCoefficient(bone);
        if (glm::abs(coefficient - 1.0f) > 0.0f) {
            translation *= coefficient;
        }
        m_localInherentTranslation = translation;
    }
    translation += m_localMorphTranslation;
    m_localTranslation = translation;
}

void
Bone::updateLocalMorphTransform(const nanoem_model_morph_bone_t *morph, nanoem_f32_t weight) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(morph, "must not be nullptr");
    const Vector3 translation(toVector3(nanoemModelMorphBoneGetTranslation(morph)));
    const Quaternion orientation(toQuaternion(nanoemModelMorphBoneGetOrientation(morph)));
    m_localMorphTranslation = glm::mix(Constants::kZeroV3, translation, weight);
    m_localMorphOrientation = glm::slerp(Constants::kZeroQ, orientation, weight);
}

void
Bone::updateLocalTransform(
    const nanoem_model_bone_t *bone, const Vector3 &translation, const Quaternion &orientation) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(bone, "must not be nullptr");
    static const bx::simd128_t kUnitX = bx::simd_ld(1, 0, 0, 0);
    static const bx::simd128_t kUnitY = bx::simd_ld(0, 1, 0, 0);
    static const bx::simd128_t kUnitZ = bx::simd_ld(0, 0, 1, 0);
    const nanoem_model_bone_t *parentBone = nanoemModelBoneGetParentBoneObject(bone);
    const bx::float4x4_t translationMatrix = {
        { kUnitX, kUnitY, kUnitZ, bx::simd_ld(translation.x, translation.y, translation.z, 1) },
    };
    bx::float4x4_t localTransform;
    bx::float4x4_mul(&localTransform,
        reinterpret_cast<const bx::float4x4_t *>(glm::value_ptr(glm::mat4_cast(orientation))), &translationMatrix);
    const Vector3 boneOrigin(origin(bone));
    if (const Bone *parentUserData = cast(parentBone)) {
        const Vector3 offset(boneOrigin - origin(parentBone));
        const bx::float4x4_t offsetMatrix = {
            { kUnitX, kUnitY, kUnitZ, bx::simd_ld(offset.x, offset.y, offset.z, 1) },
        };
        const bx::float4x4_t parentWorldTransform = parentUserData->worldTransformMatrix();
        bx::float4x4_t localTranfromWithOffset;
        bx::float4x4_mul(&localTranfromWithOffset, &localTransform, &offsetMatrix);
        bx::float4x4_mul(&m_matrices.m_worldTransform, &localTranfromWithOffset, &parentWorldTransform);
    }
    else {
        const bx::float4x4_t offsetMatrix = {
            { kUnitX, kUnitY, kUnitZ, bx::simd_ld(boneOrigin.x, boneOrigin.y, boneOrigin.z, 1) },
        };
        bx::float4x4_mul(&m_matrices.m_worldTransform, &localTransform, &offsetMatrix);
    }
    m_matrices.m_localTransform = localTransform;
    translate(-boneOrigin, &m_matrices.m_worldTransform, &m_matrices.m_skinningTransform);
    shrink3x3(&m_matrices.m_worldTransform, &m_matrices.m_normalTransform);
}

void
Bone::applyAllLocalTransform(const nanoem_model_bone_t *bone, const Model *model)
{
    updateLocalOrientation(bone, model);
    updateLocalTranslation(bone);
    updateLocalTransform(bone);
    solveConstraint(bone);
}

void
Bone::applyOutsideParentTransform(const nanoem_model_bone_t *bone, const Model *model)
{
    if (model->hasOutsideParent(bone)) {
        const StringPair &pair = model->findOutsideParent(bone);
        if (const Model *parentModel = model->project()->findModelByName(pair.first)) {
            const nanoem_model_bone_t *parentBonePtr = parentModel->findBone(pair.second);
            if (const Bone *parentBone = Bone::cast(parentBonePtr)) {
                const Vector3 inverseOrigin(-origin(bone));
                bx::float4x4_t *worldTransform = &m_matrices.m_worldTransform, out;
                translate(inverseOrigin, worldTransform, &out);
                bx::float4x4_mul(worldTransform, &parentBone->m_matrices.m_worldTransform, &out);
                translate(inverseOrigin, worldTransform, &out);
                m_matrices.m_localTransform = out;
                m_matrices.m_skinningTransform = out;
                shrink3x3(&m_matrices.m_worldTransform, &m_matrices.m_normalTransform);
            }
        }
    }
}

void
Bone::updateLocalTransform(const nanoem_model_bone_t *bone) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(bone, "must not be nullptr");
    if (glm::isNull(m_localTranslation, Constants::kEpsilon) &&
        glm::all(glm::equal(m_localOrientation, Constants::kZeroQ))) {
        updateLocalTransform(bone, Constants::kZeroV3, Constants::kZeroQ);
    }
    else {
        updateLocalTransform(bone, m_localTranslation, m_localOrientation);
    }
}

void
Bone::updateSkinningTransform(const nanoem_model_bone_t *bone, const Matrix4x4 value) NANOEM_DECL_NOEXCEPT
{
    updateSkinningTransform(bone, reinterpret_cast<const bx::float4x4_t *>(glm::value_ptr(value)));
}

void
Bone::updateSkinningTransform(const nanoem_model_bone_t *bone, const bx::float4x4_t *value) NANOEM_DECL_NOEXCEPT
{
    m_matrices.m_skinningTransform = *value;
    translate(origin(bone), value, &m_matrices.m_worldTransform);
    shrink3x3(&m_matrices.m_worldTransform, &m_matrices.m_normalTransform);
}

String
Bone::name() const
{
    return m_name;
}

String
Bone::canonicalName() const
{
    return m_canonicalName;
}

const char *
Bone::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_name.c_str();
}

const char *
Bone::canonicalNameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_canonicalName.c_str();
}

bool
Bone::isDirty() const NANOEM_DECL_NOEXCEPT
{
    return m_dirty;
}

void
Bone::setDirty(bool value)
{
    m_dirty = value;
}

void
Bone::constrainOrientation(
    const Vector3 &upperLimit, const Vector3 &lowerLimit, Quaternion &orientation) NANOEM_DECL_NOEXCEPT
{
    static nanoem_f32_t kRadians90Degree(glm::radians(90.0f));
    const Matrix3x3 matrix(glm::mat3_cast(orientation));
    Quaternion x, y, z;
    Vector3 radians;
    if (lowerLimit.x > -kRadians90Degree && upperLimit.x < kRadians90Degree) {
        radians.x = glm::asin(matrix[1][2]);
        radians.y = glm::atan(-matrix[0][2], matrix[2][2]);
        radians.z = glm::atan(-matrix[1][0], matrix[1][1]);
        createConstraintUnitAxes(radians, lowerLimit, upperLimit, x, y, z);
        orientation = z * x * y;
    }
    else if (lowerLimit.y > -kRadians90Degree && upperLimit.y < kRadians90Degree) {
        radians.x = glm::atan(-matrix[2][1], matrix[2][2]);
        radians.y = glm::asin(matrix[2][0]);
        radians.z = glm::atan(-matrix[1][0], matrix[0][0]);
        createConstraintUnitAxes(radians, lowerLimit, upperLimit, x, y, z);
        orientation = x * y * z;
    }
    else {
        radians.x = glm::atan(-matrix[2][1], matrix[1][1]);
        radians.y = glm::atan(-matrix[0][2], matrix[0][0]);
        radians.z = glm::asin(matrix[0][1]);
        createConstraintUnitAxes(radians, lowerLimit, upperLimit, x, y, z);
        orientation = y * z * x;
    }
}

bool
Bone::isSelectable(const nanoem_model_bone_t *bonePtr) NANOEM_DECL_NOEXCEPT
{
    return nanoemModelBoneIsVisible(bonePtr) && nanoemModelBoneIsUserHandleable(bonePtr) &&
        (nanoemModelBoneIsMovable(bonePtr) || nanoemModelBoneIsRotateable(bonePtr));
}

bool
Bone::isMovable(const nanoem_model_bone_t *bonePtr) NANOEM_DECL_NOEXCEPT
{
    return nanoemModelBoneIsUserHandleable(bonePtr) && nanoemModelBoneIsMovable(bonePtr);
}

bool
Bone::isRotateable(const nanoem_model_bone_t *bonePtr) NANOEM_DECL_NOEXCEPT
{
    return nanoemModelBoneIsUserHandleable(bonePtr) && nanoemModelBoneIsRotateable(bonePtr);
}

int
Bone::index(const nanoem_model_bone_t *bonePtr) NANOEM_DECL_NOEXCEPT
{
    return nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(bonePtr));
}

Matrix3x3
Bone::localAxes(const nanoem_model_bone_t *bonePtr) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(bone, "must not be nullptr");
    if (nanoemModelBoneHasLocalAxes(bonePtr)) {
        const Vector3 axisX(glm::make_vec3(nanoemModelBoneGetLocalXAxis(bonePtr)));
        const Vector3 axisZ(glm::make_vec3(nanoemModelBoneGetLocalZAxis(bonePtr)));
        const Vector3 axisY(glm::cross(axisZ, axisX));
        const Vector3 newAxisZ(glm::cross(axisX, axisY));
        const Matrix3x3 matrix(axisX, axisY, newAxisZ);
        return matrix;
    }
    else {
        static const Matrix3x3 kIdentityMatrix(Constants::kUnitX, Constants::kUnitY, Constants::kUnitZ);
        return kIdentityMatrix;
    }
}

Vector3
Bone::origin(const nanoem_model_bone_t *bonePtr) NANOEM_DECL_NOEXCEPT
{
    return glm::make_vec3(nanoemModelBoneGetOrigin(bonePtr)) * Constants::kTranslateDirection;
}

Vector3
Bone::toVector3(const nanoem_motion_bone_keyframe_t *keyframe) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(keyframe, "must not be nullptr");
    return toVector3(nanoemMotionBoneKeyframeGetTranslation(keyframe));
}

Quaternion
Bone::toQuaternion(const nanoem_motion_bone_keyframe_t *keyframe) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(keyframe, "must not be nullptr");
    return toQuaternion(nanoemMotionBoneKeyframeGetOrientation(keyframe));
}

Bone *
Bone::cast(const nanoem_model_bone_t *bonePtr) NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_object_t *object = nanoemModelBoneGetModelObject(bonePtr);
    const nanoem_user_data_t *userData = nanoemModelObjectGetUserData(object);
    return static_cast<Bone *>(nanoemUserDataGetOpaqueData(userData));
}

Bone *
Bone::create()
{
    static const PlaceHolder holder = {};
    return nanoem_new(Bone(holder));
}

const bx::float4x4_t
Bone::worldTransformMatrix() const NANOEM_DECL_NOEXCEPT
{
    return m_matrices.m_worldTransform;
}

const bx::float4x4_t
Bone::localTransformMatrix() const NANOEM_DECL_NOEXCEPT
{
    return m_matrices.m_localTransform;
}

const bx::float4x4_t
Bone::normalTransformMatrix() const NANOEM_DECL_NOEXCEPT
{
    return m_matrices.m_normalTransform;
}

const bx::float4x4_t
Bone::skinningTransformMatrix() const NANOEM_DECL_NOEXCEPT
{
    return m_matrices.m_skinningTransform;
}

Matrix4x4
Bone::worldTransform() const NANOEM_DECL_NOEXCEPT
{
    return glm::make_mat4(reinterpret_cast<const nanoem_f32_t *>(m_matrices.m_worldTransform.col));
}

Matrix4x4
Bone::localTransform() const NANOEM_DECL_NOEXCEPT
{
    return glm::make_mat4(reinterpret_cast<const nanoem_f32_t *>(m_matrices.m_localTransform.col));
}

Matrix4x4
Bone::skinningTransform() const NANOEM_DECL_NOEXCEPT
{
    return glm::make_mat4(reinterpret_cast<const nanoem_f32_t *>(m_matrices.m_skinningTransform.col));
}

Vector3
Bone::worldTransformOrigin() const NANOEM_DECL_NOEXCEPT
{
    return glm::make_vec3(reinterpret_cast<const nanoem_f32_t *>(&m_matrices.m_worldTransform.col[3]));
}

Vector3
Bone::localTransformOrigin() const NANOEM_DECL_NOEXCEPT
{
    return glm::make_vec3(reinterpret_cast<const nanoem_f32_t *>(&m_matrices.m_worldTransform.col[3]));
}

Quaternion
Bone::localOrientation() const NANOEM_DECL_NOEXCEPT
{
    return m_localOrientation;
}

Quaternion
Bone::localInherentOrientation() const NANOEM_DECL_NOEXCEPT
{
    return m_localInherentOrientation;
}

Quaternion
Bone::localMorphOrientation() const NANOEM_DECL_NOEXCEPT
{
    return m_localMorphOrientation;
}

Quaternion
Bone::localUserOrientation() const NANOEM_DECL_NOEXCEPT
{
    return m_localUserOrientation;
}

Quaternion
Bone::constraintJointOrientation() const NANOEM_DECL_NOEXCEPT
{
    return m_constraintJointOrientation;
}

void
Bone::setConstraintJointOrientation(const Quaternion &value)
{
    m_constraintJointOrientation = value;
}

Vector3
Bone::localTranslation() const NANOEM_DECL_NOEXCEPT
{
    return m_localTranslation;
}

Vector3
Bone::localInherentTranslation() const NANOEM_DECL_NOEXCEPT
{
    return m_localInherentTranslation;
}

Vector3
Bone::localMorphTranslation() const NANOEM_DECL_NOEXCEPT
{
    return m_localMorphTranslation;
}

Vector3
Bone::localUserTranslation() const NANOEM_DECL_NOEXCEPT
{
    return m_localUserTranslation;
}

void
Bone::setLocalUserOrientation(const Quaternion &value)
{
    m_localUserOrientation = value;
}

void
Bone::setLocalUserTranslation(const Vector3 &value)
{
    m_localUserTranslation = value;
}

void
Bone::setLocalMorphOrientation(const Quaternion &value)
{
    m_localMorphOrientation = value;
}

void
Bone::setLocalMorphTranslation(const Vector3 &value)
{
    m_localMorphTranslation = value;
}

Vector4U8
Bone::bezierControlPoints(nanoem_motion_bone_keyframe_interpolation_type_t index) const NANOEM_DECL_NOEXCEPT
{
    return m_bezierControlPoints[index];
}

void
Bone::setBezierControlPoints(nanoem_motion_bone_keyframe_interpolation_type_t index, const Vector4U8 &value)
{
    m_bezierControlPoints[index] = value;
}

bool
Bone::isLinearInterpolation(nanoem_motion_bone_keyframe_interpolation_type_t index) const NANOEM_DECL_NOEXCEPT
{
    return m_isLinearInterpolation[index];
}

void
Bone::setLinearInterpolation(nanoem_motion_bone_keyframe_interpolation_type_t index, bool value)
{
    m_isLinearInterpolation[index] = value;
}

void
Bone::destroy(void *opaque, nanoem_model_object_t * /* object */) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(opaque, "must not be nullptr");
    Bone *self = static_cast<Bone *>(opaque);
    nanoem_delete(self);
}

void
Bone::synchronizeTransform(const Motion *motion, const nanoem_model_bone_t *bone,
    const nanoem_model_rigid_body_t *rigidBodyPtr, nanoem_frame_index_t frameIndex, FrameTransform &transform)
{
    nanoem_parameter_assert(bone, "must not be nullptr");
    const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
    if (const nanoem_motion_bone_keyframe_t *keyframe = motion->findBoneKeyframe(name, frameIndex)) {
        transform.m_translation = toVector3(keyframe);
        transform.m_orientation = toQuaternion(keyframe);
        for (int i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
             i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
            if (!nanoemMotionBoneKeyframeIsLinearInterpolation(
                    keyframe, nanoem_motion_bone_keyframe_interpolation_type_t(i))) {
                transform.m_bezierControlPoints[i] = glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(
                    keyframe, nanoem_motion_bone_keyframe_interpolation_type_t(i)));
                transform.m_enableLinearInterpolation[i] = false;
            }
        }
    }
    else {
        nanoem_motion_bone_keyframe_t *prevKeyframe, *nextKeyframe;
        nanoemMotionSearchClosestBoneKeyframes(motion->data(), name, frameIndex, &prevKeyframe, &nextKeyframe);
        if (prevKeyframe && nextKeyframe) {
            const nanoem_motion_bone_keyframe_t *interpolateKeyframe = nextKeyframe;
            const Vector3 translation0(toVector3(prevKeyframe)), translation1(toVector3(nextKeyframe));
            const nanoem_f32_t coef = Motion::coefficient(prevKeyframe, nextKeyframe, frameIndex);
            const bool prevEnabled = nanoemMotionBoneKeyframeIsPhysicsSimulationEnabled(prevKeyframe),
                       nextEnabled = nanoemMotionBoneKeyframeIsPhysicsSimulationEnabled(nextKeyframe);
            if (prevEnabled && !nextEnabled && rigidBodyPtr) {
                transform.m_translation = glm::mix(cast(bone)->localUserTranslation(), translation1, coef);
            }
            else if (nanoemMotionBoneKeyframeIsLinearInterpolation(
                         interpolateKeyframe, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X) &&
                nanoemMotionBoneKeyframeIsLinearInterpolation(
                    interpolateKeyframe, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y) &&
                nanoemMotionBoneKeyframeIsLinearInterpolation(
                    interpolateKeyframe, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z)) {
                transform.m_translation = glm::mix(translation0, translation1, coef);
            }
            else {
                for (int i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
                     i <= NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z; i++) {
                    const nanoem_f32_t v0 = translation0[i], v1 = translation1[i];
                    const nanoem_motion_bone_keyframe_interpolation_type_t type =
                        nanoem_motion_bone_keyframe_interpolation_type_t(i);
                    if (nanoemMotionBoneKeyframeIsLinearInterpolation(interpolateKeyframe, type)) {
                        transform.m_translation[i] = glm::mix(v0, v1, coef);
                    }
                    else if (motion) {
                        nanoem_f32_t t2 = motion->bezierCurve(prevKeyframe, nextKeyframe, type, coef);
                        transform.m_translation[i] = glm::mix(v0, v1, t2);
                        transform.m_bezierControlPoints[i] =
                            glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(interpolateKeyframe, type));
                        transform.m_enableLinearInterpolation[i] = false;
                    }
                }
            }
            const Quaternion orientation0(toQuaternion(prevKeyframe)), orientation1(toQuaternion(nextKeyframe));
            if (prevEnabled && !nextEnabled && rigidBodyPtr) {
                transform.m_orientation = glm::slerp(cast(bone)->localUserOrientation(), orientation1, coef);
            }
            else if (nanoemMotionBoneKeyframeIsLinearInterpolation(
                         interpolateKeyframe, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION)) {
                transform.m_orientation = glm::slerp(orientation0, orientation1, coef);
            }
            else if (motion) {
                nanoem_f32_t t2 = motion->bezierCurve(
                    prevKeyframe, nextKeyframe, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION, coef);
                transform.m_orientation = glm::slerp(orientation0, orientation1, t2);
                transform.m_bezierControlPoints[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION] =
                    glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(
                        interpolateKeyframe, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION));
                transform.m_enableLinearInterpolation[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION] =
                    false;
            }
            if (prevEnabled && nextEnabled && rigidBodyPtr) {
                RigidBody *body = RigidBody::cast(rigidBodyPtr);
                body->disableKinematic();
            }
        }
    }
}

void
Bone::createConstraintUnitAxes(const Vector3 &radians, const Vector3 &lowerLimit, const Vector3 &upperLimit,
    Quaternion &x, Quaternion &y, Quaternion &z) NANOEM_DECL_NOEXCEPT
{
    const Vector3 r(glm::clamp(radians, lowerLimit, upperLimit));
    x = glm::angleAxis(r.x, Constants::kUnitX);
    y = glm::angleAxis(r.y, Constants::kUnitY);
    z = glm::angleAxis(r.z, Constants::kUnitZ);
}

void
Bone::constrainOrientation(const nanoem_model_constraint_joint_t *joint, Quaternion &orientation) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(joint, "must not be nullptr");
    if (nanoemModelConstraintJointHasAngleLimit(joint)) {
        const Vector3 upperLimit(glm::make_vec3(nanoemModelConstraintJointGetUpperLimit(joint)));
        const Vector3 lowerLimit(glm::make_vec3(nanoemModelConstraintJointGetLowerLimit(joint)));
        constrainOrientation(upperLimit, lowerLimit, orientation);
    }
}

Vector3
Bone::toVector3(const nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT
{
    return glm::make_vec3(value) * Constants::kTranslateDirection;
}

Quaternion
Bone::toQuaternion(const nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT
{
    return glm::make_quat(glm::value_ptr(glm::make_vec4(value) * Constants::kOrientateDirection));
}

void
Bone::solveConstraint(const nanoem_model_constraint_t *constraintPtr, int numIterations)
{
    nanoem_parameter_assert(constraintPtr, "must not be nullptr");
    nanoem_rsize_t numJoints;
    nanoem_model_constraint_joint_t *const *joints = nanoemModelConstraintGetAllJointObjects(constraintPtr, &numJoints);
    const nanoem_model_bone_t *targetBonePtr = nanoemModelConstraintGetTargetBoneObject(constraintPtr);
    const nanoem_model_bone_t *effectorBonePtr = nanoemModelConstraintGetEffectorBoneObject(constraintPtr);
    const nanoem_f32_t angleLimit = nanoemModelConstraintGetAngleLimit(constraintPtr);
    const Bone *targetBone = Bone::cast(targetBonePtr);
    Bone *effectorBone = Bone::cast(effectorBonePtr);
    Constraint *constraintUserData = Constraint::cast(constraintPtr);
    if (!targetBone || !effectorBone || !constraintUserData) {
        return;
    }
    const Vector4 targetBonePosition(targetBone->worldTransformOrigin(), 1);
    for (int i = 0; i < numIterations; i++) {
        const bool firstIteration = i == 0;
        for (nanoem_rsize_t j = 0; j < numJoints; j++) {
            const nanoem_model_constraint_joint_t *joint = joints[j];
            Bone *jointBone = Bone::cast(nanoemModelConstraintJointGetBoneObject(joint));
            Constraint::Joint *jointResult = constraintUserData->jointIterationResult(joint, i);
            const Vector4 effectorBonePosition(effectorBone->worldTransformOrigin(), 1);
            if (jointBone &&
                !Constraint::solveAxisAngle(
                    jointBone->worldTransform(), effectorBonePosition, targetBonePosition, jointResult)) {
                const nanoem_model_bone_t *bone = nanoemModelConstraintJointGetBoneObject(joint);
                if (nanoemModelBoneHasFixedAxis(bone)) {
                    const Vector3 axis(glm::make_vec3(nanoemModelBoneGetFixedAxis(bone)));
                    if (!glm::isNull(axis, Constants::kEpsilon)) {
                        jointResult->setAxis(glm::normalize(axis));
                    }
                }
                else if (firstIteration && nanoemModelConstraintJointHasAngleLimit(joint)) {
                    static const Vector3 kEpsilon(Constants::kEpsilonVec3);
                    const glm::bvec3 hasUpperLimit(glm::lessThanEqual(
                        glm::abs(glm::make_vec3(nanoemModelConstraintJointGetUpperLimit(joint))), kEpsilon));
                    const glm::bvec3 hasLowerLimit(glm::lessThanEqual(
                        glm::abs(glm::make_vec3(nanoemModelConstraintJointGetLowerLimit(joint))), kEpsilon));
                    if (hasLowerLimit.y && hasUpperLimit.y && hasLowerLimit.z && hasUpperLimit.z) {
                        jointResult->setAxis(Constants::kUnitX);
                    }
                    else if (hasLowerLimit.x && hasUpperLimit.x && hasLowerLimit.z && hasUpperLimit.z) {
                        jointResult->setAxis(Constants::kUnitY);
                    }
                    else if (hasLowerLimit.x && hasUpperLimit.x && hasLowerLimit.y && hasUpperLimit.y) {
                        jointResult->setAxis(Constants::kUnitZ);
                    }
                }
                nanoem_f32_t newAngleLimit = angleLimit * (j + 1);
                const Quaternion orientation(
                    glm::angleAxis(glm::min(jointResult->m_angle, newAngleLimit), jointResult->m_axis));
                Quaternion mixedOrientation;
                if (firstIteration) {
                    mixedOrientation = orientation * jointBone->localUserOrientation();
                }
                else {
                    mixedOrientation = jointBone->constraintJointOrientation() * orientation;
                }
                constrainOrientation(joint, mixedOrientation);
                jointBone->setConstraintJointOrientation(glm::normalize(mixedOrientation));
                for (int k = Inline::saturateInt32(j); k >= 0; k--) {
                    const nanoem_model_constraint_joint_t *upperJoint = joints[k];
                    const nanoem_model_bone_t *upperJointBonePtr = nanoemModelConstraintJointGetBoneObject(upperJoint);
                    Bone *upperJointBone = Bone::cast(upperJointBonePtr);
                    upperJointBone->updateLocalTransform(upperJointBonePtr, upperJointBone->localTranslation(),
                        upperJointBone->constraintJointOrientation());
                }
                jointResult->setTransform(jointBone->worldTransform());
                effectorBone->updateLocalTransform(effectorBonePtr);
                Constraint::Joint *effectorResult = constraintUserData->effectorIterationResult(joint, i);
                effectorResult->setTransform(effectorBone->worldTransform());
            }
        }
    }
}

void
Bone::solveConstraint(const nanoem_model_bone_t *bone)
{
    nanoem_parameter_assert(bone, "must not be nullptr");
    const nanoem_model_constraint_t *constraintPtr = nanoemModelBoneGetConstraintObject(bone);
    if (const Constraint *constraint = Constraint::cast(constraintPtr)) {
        if (constraint->isEnabled()) {
            const int numIterations = nanoemModelConstraintGetNumIterations(constraintPtr);
            solveConstraint(constraintPtr, numIterations);
        }
        else {
            nanoem_rsize_t numJoints;
            nanoem_model_constraint_joint_t *const *joints =
                nanoemModelConstraintGetAllJointObjects(constraintPtr, &numJoints);
            for (nanoem_rsize_t i = 0; i < numJoints; i++) {
                if (Bone *jointBone = Bone::cast(nanoemModelConstraintJointGetBoneObject(joints[i]))) {
                    jointBone->setConstraintJointOrientation(Constants::kZeroQ);
                }
            }
        }
    }
}

Bone::Bone(const PlaceHolder & /* holder */) NANOEM_DECL_NOEXCEPT : m_localOrientation(Constants::kZeroQ),
                                                                    m_localInherentOrientation(Constants::kZeroQ),
                                                                    m_localMorphOrientation(Constants::kZeroQ),
                                                                    m_localUserOrientation(Constants::kZeroQ),
                                                                    m_constraintJointOrientation(Constants::kZeroQ),
                                                                    m_localTranslation(Constants::kZeroV3),
                                                                    m_localInherentTranslation(Constants::kZeroV3),
                                                                    m_localMorphTranslation(Constants::kZeroV3),
                                                                    m_localUserTranslation(Constants::kZeroV3),
                                                                    m_dirty(false)
{
    Inline::clearZeroMemory(m_bezierControlPoints);
    Inline::clearZeroMemory(m_isLinearInterpolation);
    identify(&m_matrices.m_localTransform);
    identify(&m_matrices.m_normalTransform);
    identify(&m_matrices.m_skinningTransform);
    identify(&m_matrices.m_worldTransform);
    for (size_t i = 0; i < BX_COUNTOF(m_isLinearInterpolation); i++) {
        m_bezierControlPoints[i] = Vector4U8(0);
        m_isLinearInterpolation[i] = true;
    }
}

} /* namespace model */
} /* namespace nanoem */
