/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/model/Constraint.h"

#include "emapp/Constants.h"
#include "emapp/EnumUtils.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtx/vector_query.hpp"

namespace nanoem {
namespace model {
namespace {

enum PrivateStateFlags {
    kPrivateStateEnabled = 1 << 1,
    kPrivateStateReserved = 1 << 31,
};
static const nanoem_u32_t kPrivateStateInitialValue = kPrivateStateEnabled;

} /* namespade anonymous */

Constraint::Joint::Joint()
    : m_orientation(Constants::kZeroQ)
    , m_translation(Constants::kZeroV3)
    , m_targetDirection(Constants::kZeroV3)
    , m_effectorDirection(Constants::kZeroV3)
    , m_axis(Constants::kZeroV3)
    , m_angle(0.0f)
{
}

Constraint::Joint::~Joint()
{
}

Constraint::~Constraint() NANOEM_DECL_NOEXCEPT
{
    m_jointIterationResult.clear();
    m_effectorIterationResult.clear();
}

int
Constraint::index(const nanoem_model_constraint_t *constraintPtr) NANOEM_DECL_NOEXCEPT
{
    return nanoemModelObjectGetIndex(nanoemModelConstraintGetModelObject(constraintPtr));
}

const char *
Constraint::nameConstString(
    const nanoem_model_constraint_t *constraintPtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT
{
    const Constraint *constraint = cast(constraintPtr);
    return constraint ? constraint->nameConstString() : placeHolder;
}

const char *
Constraint::canonicalNameConstString(
    const nanoem_model_constraint_t *constraintPtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT
{
    const Constraint *constraint = cast(constraintPtr);
    return constraint ? constraint->canonicalNameConstString() : placeHolder;
}

Constraint *
Constraint::cast(const nanoem_model_constraint_t *constraintPtr) NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_object_t *object = nanoemModelConstraintGetModelObject(constraintPtr);
    const nanoem_user_data_t *userData = nanoemModelObjectGetUserData(object);
    return static_cast<Constraint *>(nanoemUserDataGetOpaqueData(userData));
}

Constraint *
Constraint::create()
{
    static const PlaceHolder holder = {};
    return nanoem_new(Constraint(holder));
}

bool
Constraint::solveAxisAngle(const Matrix4x4 &transform, const Vector4 &effectorPosition, const Vector4 &targetPosition,
    Joint *result) NANOEM_DECL_NOEXCEPT
{
    const Matrix4x4 inverseTransform(glm::affineInverse(transform));
    const Vector3 inverseEffectorPosition(inverseTransform * effectorPosition);
    const Vector3 inverseTargetPosition(inverseTransform * targetPosition);
    if (glm::isNull(inverseEffectorPosition, Constants::kEpsilon) ||
        glm::isNull(inverseTargetPosition, Constants::kEpsilon)) {
        return true;
    }
    const Vector3 effectorDirection(glm::normalize(inverseEffectorPosition));
    const Vector3 targetDirection(glm::normalize(inverseTargetPosition));
    const Vector3 axis(glm::cross(effectorDirection, targetDirection));
    result->m_effectorDirection = effectorDirection;
    result->m_targetDirection = targetDirection;
    result->m_axis = axis;
    if (glm::isNull(axis, Constants::kEpsilon)) {
        return true;
    }
    /* must be clamped due to possibility of out of range of rawDotProduct */
    const nanoem_f32_t z = glm::clamp(glm::dot(effectorDirection, targetDirection), -1.0f, 1.0f);
    result->m_axis = glm::normalize(axis);
    if (glm::abs(z) <= Constants::kEpsilon) {
        return true;
    }
    result->m_angle = glm::acos(z);
    return false;
}

bool
Constraint::hasUnitXConstraint(
    const nanoem_model_bone_t *bone, nanoem_unicode_string_factory_t *factory) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(bone, "must not be nullptr");
    nanoem_parameter_assert(factory, "must not be nullptr");
    static const nanoem_u8_t kLeftKnee[] = { 0xe5, 0xb7, 0xa6, 0xe3, 0x81, 0xb2, 0xe3, 0x81, 0x96, 0x0 },
                             kRightKnee[] = { 0xe5, 0x8f, 0xb3, 0xe3, 0x81, 0xb2, 0xe3, 0x81, 0x96, 0x0 };
    const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
    nanoem_u8_t buffer[Inline::kNameStackBufferSize];
    nanoem_rsize_t length;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemUnicodeStringFactoryToUtf8OnStackEXT(factory, name, &length, buffer, sizeof(buffer), &status);
    return StringUtils::equals(reinterpret_cast<const char *>(buffer), reinterpret_cast<const char *>(kLeftKnee)) ||
        StringUtils::equals(reinterpret_cast<const char *>(buffer), reinterpret_cast<const char *>(kRightKnee));
}

void
Constraint::bind(nanoem_model_constraint_t *constraintPtr)
{
    nanoem_parameter_assert(constraintPtr, "must not be nullptr");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_user_data_t *userData = nanoemUserDataCreate(&status);
    nanoemUserDataSetOnDestroyModelObjectCallback(userData, &Constraint::destroy);
    nanoemUserDataSetOpaqueData(userData, this);
    nanoemModelObjectSetUserData(nanoemModelConstraintGetModelObjectMutable(constraintPtr), userData);
}

void
Constraint::resetLanguage(const nanoem_model_constraint_t *constraintPtr, nanoem_unicode_string_factory_t *factory,
    nanoem_language_type_t language)
{
    const nanoem_model_bone_t *targetBone = nanoemModelConstraintGetTargetBoneObject(constraintPtr);
    StringUtils::getUtf8String(nanoemModelBoneGetName(targetBone, language), factory, m_name);
    if (m_canonicalName.empty()) {
        StringUtils::getUtf8String(
            nanoemModelBoneGetName(targetBone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), factory, m_canonicalName);
        if (m_canonicalName.empty()) {
            StringUtils::format(m_canonicalName, "Constraint%d", index(constraintPtr));
        }
    }
    if (m_name.empty()) {
        m_name = m_canonicalName;
    }
}

void
Constraint::initialize(const nanoem_model_constraint_t *constraintPtr)
{
    nanoem_parameter_assert(constraintPtr, "must not be nullptr");
    nanoem_rsize_t numJoints;
    nanoem_model_constraint_joint_t *const *joints = nanoemModelConstraintGetAllJointObjects(constraintPtr, &numJoints);
    nanoem_rsize_t numIterations = nanoem_rsize_t(glm::max(nanoemModelConstraintGetNumIterations(constraintPtr), 0));
    for (nanoem_rsize_t i = 0; i < numJoints; i++) {
        nanoem_model_constraint_joint_t *joint = joints[i];
        m_jointIterationResult[joint].reserve(numIterations);
        m_effectorIterationResult[joint].reserve(numIterations);
    }
}

const Constraint::JointIterationResult *
Constraint::jointIterationResult() const NANOEM_DECL_NOEXCEPT
{
    return &m_jointIterationResult;
}

const Constraint::JointIterationResult *
Constraint::effectorIterationResult() const NANOEM_DECL_NOEXCEPT
{
    return &m_effectorIterationResult;
}

Constraint::Joint *
Constraint::jointIterationResult(const nanoem_model_constraint_joint_t *joint, nanoem_rsize_t offset)
{
    return &m_jointIterationResult[joint][offset];
}

Constraint::Joint *
Constraint::effectorIterationResult(const nanoem_model_constraint_joint_t *joint, nanoem_rsize_t offset)
{
    return &m_effectorIterationResult[joint][offset];
}

String
Constraint::name() const
{
    return m_name;
}

const char *
Constraint::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_name.c_str();
}

String
Constraint::canonicalName() const
{
    return m_canonicalName;
}

const char *
Constraint::canonicalNameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_canonicalName.c_str();
}

bool
Constraint::isEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateEnabled, m_states);
}

void
Constraint::setEnabled(bool value)
{
    EnumUtils::setEnabled(kPrivateStateEnabled, m_states, value);
}

void
Constraint::destroy(void *opaque, nanoem_model_object_t * /* constraintPtr */) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(opaque, "must not be nullptr");
    Constraint *self = static_cast<Constraint *>(opaque);
    nanoem_delete(self);
}

Constraint::Constraint(const PlaceHolder & /* holder */) NANOEM_DECL_NOEXCEPT : m_states(kPrivateStateEnabled)
{
}

} /* namespace model */
} /* namespace nanoem */
