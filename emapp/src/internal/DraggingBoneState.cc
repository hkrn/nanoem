/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/DraggingBoneState.h"

#include "emapp/ICamera.h"
#include "emapp/IEventPublisher.h"
#include "emapp/IModelObjectSelection.h"
#include "emapp/ListUtils.h"
#include "emapp/Model.h"
#include "emapp/Project.h"
#include "emapp/Ray.h"
#include "emapp/command/TransformBoneCommand.h"
#include "emapp/internal/IDraggingState.h"
#include "emapp/private/CommonInclude.h"

#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/closest_point.hpp"

namespace nanoem {
namespace internal {

DraggingBoneState::DraggingBoneState(
    Project *project, Model *model, const Vector2SI32 &pressedCursorPosition, const Vector2 &lastBoneCursorPosition)
    : m_project(project)
    , m_model(model)
    , m_pressedCursorPosition(pressedCursorPosition)
    , m_lastBoneCursorPosition(lastBoneCursorPosition)
    , m_accumulatedDeltaPosition(0)
    , m_lastPressedCursorPosition(pressedCursorPosition)
    , m_scaleFactor(1.0f)
{
    nanoem_parameter_assert(m_project, "must NOT be NULL");
    nanoem_parameter_assert(m_model, "must NOT be NULL");
    m_project->eventPublisher()->publishDisableCursorEvent(pressedCursorPosition);
    m_model->saveBindPose(m_lastBindPose);
    setCameraLocked(true);
}

DraggingBoneState::~DraggingBoneState()
{
    setCameraLocked(false);
    m_model = nullptr;
    m_project = nullptr;
}

void
DraggingBoneState::commit(const Vector2SI32 &logicalPosition)
{
    model::BindPose currentBindPose;
    model::Bone::Set boneSet(m_model->selection()->allBoneSet());
    m_model->saveBindPose(currentBindPose);
    const nanoem_model_bone_t *activeBone = m_model->activeBone();
    boneSet.insert(activeBone);
    if (const nanoem_model_constraint_t *constraintPtr = m_model->findConstraint(activeBone)) {
        const model::Constraint *constraint = model::Constraint::cast(constraintPtr);
        if (constraint->isEnabled()) {
            nanoem_rsize_t numJoints;
            nanoem_model_constraint_joint_t *const *joints =
                nanoemModelConstraintGetAllJointObjects(constraintPtr, &numJoints);
            for (nanoem_rsize_t i = 0; i < numJoints; i++) {
                boneSet.insert(nanoemModelConstraintJointGetBoneObject(joints[i]));
            }
            boneSet.insert(nanoemModelConstraintGetEffectorBoneObject(constraintPtr));
        }
    }
    m_model->pushUndo(command::TransformBoneCommand::create(
        m_lastBindPose, currentBindPose, ListUtils::toListFromSet(boneSet), m_model, m_project));
    m_project->eventPublisher()->publishEnableCursorEvent(logicalPosition);
    setCameraLocked(false);
}

nanoem_f32_t
DraggingBoneState::scaleFactor() const NANOEM_DECL_NOEXCEPT
{
    return m_scaleFactor;
}

void
DraggingBoneState::setScaleFactor(nanoem_f32_t value)
{
    m_scaleFactor = value;
}

Vector3
DraggingBoneState::handleDragAxis(const nanoem_model_bone_t *bone, Model::TransformCoordinateType type, int index)
{
    Vector3 axis;
    if (nanoemModelBoneHasFixedAxis(bone)) {
        axis = glm::normalize(glm::make_vec3(nanoemModelBoneGetFixedAxis(bone)));
    }
    else if (type == Model::kTransformCoordinateTypeLocal) {
        axis = model::Bone::localAxes(bone)[glm::clamp(index, 0, 2)];
    }
    else {
        axis = Matrix3x3(1)[glm::clamp(index, 0, 2)];
    }
    return axis;
}

Vector3
DraggingBoneState::handleDragAxis(const nanoem_model_bone_t *bone, int index)
{
    return handleDragAxis(bone, Model::kTransformCoordinateTypeLocal, index);
}

model::Bone::List
DraggingBoneState::createBoneList(const nanoem_model_bone_t *bone)
{
    model::Bone::List bones;
    bones.push_back(bone);
    return bones;
}

void
DraggingBoneState::performTranslationTransform(
    const tinystl::pair<Vector3, const nanoem_model_bone_t *> &translation, const model::Bone::List &bones)
{
    nanoem_parameter_assert(!bones.empty(), "must not be NULL");
    m_model->restoreBindPose(m_lastBindPose);
    const nanoem_rsize_t numBones = bones.size();
    if (numBones > 1) {
        const model::Bone *baseBone = model::Bone::cast(translation.second);
        const Matrix4x4 baseInverseTransform(glm::affineInverse(baseBone->worldTransform()));
        const Vector4 startTranslation(translation.first, 1);
        for (model::Bone::List::const_iterator it = bones.begin(), end = bones.end(); it != end; ++it) {
            const nanoem_model_bone_t *bonePtr = *it;
            model::Bone *bone = model::Bone::cast(bonePtr);
            bone->setLocalUserTranslation(Vector3(startTranslation * baseInverseTransform * bone->worldTransform()));
        }
    }
    else {
        const nanoem_model_bone_t *bonePtr = bones[0];
        model::Bone *bone = model::Bone::cast(bonePtr);
        bone->setLocalUserTranslation(translation.first);
    }
    m_model->performAllBonesTransform();
}

void
DraggingBoneState::performOrientationTransform(
    const tinystl::pair<Quaternion, const nanoem_model_bone_t *> &orientation, const model::Bone::List &bones)
{
    nanoem_parameter_assert(!bones.empty(), "must not be NULL");
    m_model->restoreBindPose(m_lastBindPose);
    const nanoem_rsize_t numBones = bones.size();
    if (numBones > 1) {
        const model::Bone *baseBone = model::Bone::cast(orientation.second);
        const Matrix4x4 baseInverseTransform(glm::affineInverse(baseBone->worldTransform()));
        const Quaternion &startOrientation = orientation.first;
        for (model::Bone::List::const_iterator it = bones.begin(), end = bones.end(); it != end; ++it) {
            const nanoem_model_bone_t *bonePtr = *it;
            model::Bone *bone = model::Bone::cast(bonePtr);
            bone->setLocalUserOrientation(
                startOrientation * glm::quat_cast(baseInverseTransform * bone->worldTransform()));
        }
    }
    else {
        const nanoem_model_bone_t *bonePtr = bones[0];
        model::Bone *bone = model::Bone::cast(bonePtr);
        bone->setLocalUserOrientation(bone->localUserOrientation() * orientation.first);
    }
    m_model->performAllBonesTransform();
}

model::BindPose::Parameter
DraggingBoneState::findBindPoseParameter(const nanoem_model_bone_t *bone, const Model *model) const NANOEM_DECL_NOEXCEPT
{
    nanoem_rsize_t numBones, boneIndex = NANOEM_RSIZE_MAX;
    nanoem_model_bone_t *const *orderedBones = nanoemModelGetAllOrderedBoneObjects(model->data(), &numBones);
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        if (orderedBones[i] == bone) {
            boneIndex = i;
            break;
        }
    }
    return boneIndex != NANOEM_RSIZE_MAX ? m_lastBindPose.m_parameters[boneIndex] : model::BindPose::Parameter();
}

Model *
DraggingBoneState::activeModel() const NANOEM_DECL_NOEXCEPT
{
    return m_model;
}

Vector3
DraggingBoneState::accumulatedDeltaPosition() const NANOEM_DECL_NOEXCEPT
{
    return m_accumulatedDeltaPosition;
}

Vector3
DraggingBoneState::cursorMoveDelta(const Vector2SI32 &value) const NANOEM_DECL_NOEXCEPT
{
    Vector3 intersection, lastPressedPosition;
    const ICamera *camera = m_project->activeCamera();
    camera->castRay(value, intersection);
    camera->castRay(m_pressedCursorPosition, lastPressedPosition);
    return intersection - lastPressedPosition;
}

Vector2
DraggingBoneState::cursorDelta(const Vector2 &logicalPosition) const NANOEM_DECL_NOEXCEPT
{
    return (m_lastPressedCursorPosition - logicalPosition) * m_scaleFactor;
}

Vector2SI32
DraggingBoneState::pressedCursorPosition() const NANOEM_DECL_NOEXCEPT
{
    return m_pressedCursorPosition;
}

Vector2SI32
DraggingBoneState::lastBoneCursorPosition() const NANOEM_DECL_NOEXCEPT
{
    return m_lastBoneCursorPosition;
}

void
DraggingBoneState::updateLastCursorPosition(const Vector2 &logicalPosition, const Vector3 &delta)
{
    m_lastPressedCursorPosition = logicalPosition;
    m_accumulatedDeltaPosition += delta;
}

void
DraggingBoneState::setCameraLocked(bool value)
{
    m_project->globalCamera()->setLocked(value);
    m_model->localCamera()->setLocked(value);
}

TranslateActiveBoneState::TranslateActiveBoneState(
    Project *project, Model *model, const Vector2SI32 &pressedCursorPosition, const Vector2 &lastBoneCursorPosition)
    : DraggingBoneState(project, model, pressedCursorPosition, lastBoneCursorPosition)
    , m_baseLocalTranslation(0)
{
    if (const model::Bone *activeBone = model::Bone::cast(model->activeBone())) {
        m_baseLocalTranslation = activeBone->localUserTranslation();
    }
}

void
TranslateActiveBoneState::transform(const Vector2SI32 &logicalPosition)
{
    const Model *model = activeModel();
    const Model::AxisType axis(model->transformAxisType());
    const glm::bvec2 areAxisTypeSelected(axis == Model::kAxisX, axis == Model::kAxisY);
    Vector3 localTranslation(m_baseLocalTranslation);
    if (glm::any(areAxisTypeSelected)) {
        const Vector2SI32 movingCursorPosition(logicalPosition * Vector2SI32(areAxisTypeSelected)),
            fixedCursorPosition(lastBoneCursorPosition() * Vector2SI32(glm::not_(areAxisTypeSelected)));
        localTranslation += cursorMoveDelta(movingCursorPosition + fixedCursorPosition);
    }
    else {
        localTranslation += cursorMoveDelta(logicalPosition);
    }
    const nanoem_model_bone_t *activeBone = model->activeBone();
    const model::Bone::List &bones =
        ListUtils::toListFromSet<const nanoem_model_bone_t *>(model->selection()->allBoneSet());
    if (!bones.empty()) {
        performTranslationTransform(tinystl::make_pair(localTranslation, activeBone), bones);
    }
}

const char *
TranslateActiveBoneState::name() const NANOEM_DECL_NOEXCEPT
{
    return "nanoem.gui.viewport.bone.translate-active-bone";
}

OrientateActiveBoneState::OrientateActiveBoneState(
    Project *project, Model *model, const Vector2SI32 &pressedCursorPosition, const Vector2 &lastBoneCursorPosition)
    : DraggingBoneState(project, model, pressedCursorPosition, lastBoneCursorPosition)
{
}

void
OrientateActiveBoneState::transform(const Vector2SI32 &logicalPosition)
{
    const Model *model = activeModel();
    const nanoem_model_bone_t *activeBone = model->activeBone();
    Quaternion orientation;
    switch (model->transformAxisType()) {
    case Model::kAxisCenter: {
        /* do nothing */
        break;
    }
    case Model::kAxisX: {
        const Vector2 delta(logicalPosition - pressedCursorPosition());
        const Vector3 axisX(handleDragAxis(activeBone, 0));
        orientation = glm::angleAxis(glm::radians(delta).y, axisX);
        break;
    }
    case Model::kAxisY: {
        const nanoem_f32_t angle(angleCursorFromBone(logicalPosition));
        const Vector3 axisY(handleDragAxis(activeBone, 1));
        orientation = glm::angleAxis(angle, axisY);
        break;
    }
    case Model::kAxisZ: {
        const Vector2 delta(logicalPosition - pressedCursorPosition());
        const Vector3 axisZ(handleDragAxis(activeBone, 2));
        orientation = glm::angleAxis(-glm::radians(delta).x, axisZ);
        break;
    }
    default:
        break;
    }
    const model::Bone::List &bones =
        ListUtils::toListFromSet<const nanoem_model_bone_t *>(model->selection()->allBoneSet());
    if (!bones.empty()) {
        performOrientationTransform(tinystl::make_pair(orientation, activeBone), bones);
    }
}

const char *
OrientateActiveBoneState::name() const NANOEM_DECL_NOEXCEPT
{
    return "nanoem.gui.viewport.bone.orientate-active-bone";
}

nanoem_f32_t
OrientateActiveBoneState::angleCursorFromBone(const Vector2SI32 &value) const
{
    const Vector2 p0(glm::normalize(Vector2(pressedCursorPosition() - lastBoneCursorPosition())));
    const Vector2 p1(glm::normalize(Vector2(value - lastBoneCursorPosition())));
    nanoem_f32_t crossProduct = p1.x * p0.y - p1.y * p0.x;
    nanoem_f32_t angle = glm::sign(crossProduct) * glm::acos(glm::dot(p0, p1));
    return glm::isnan(angle) ? 0 : angle;
}

AxisAlignedTranslateActiveBoneState::AxisAlignedTranslateActiveBoneState(
    Project *project, Model *model, const Vector2 &pressedCursorPosition, int axisIndex)
    : DraggingBoneState(project, model, pressedCursorPosition, Vector2())
    , m_axisIndex(axisIndex)
{
    setScaleFactor(0.05f);
}

void
AxisAlignedTranslateActiveBoneState::transform(const Vector2SI32 &logicalPosition)
{
    const Model *model = activeModel();
    const Model::TransformCoordinateType transformCoordinateType = model->transformCoordinateType();
    const nanoem_model_bone_t *activeBone = model->activeBone();
    const Vector3 axis(handleDragAxis(activeBone, transformCoordinateType, m_axisIndex));
    const model::BindPose::Parameter &parameter = findBindPoseParameter(activeBone, model);
    const Vector3 delta(cursorDelta(logicalPosition).y * axis);
    switch (transformCoordinateType) {
    case Model::kTransformCoordinateTypeGlobal: {
        const Vector3 translation(accumulatedDeltaPosition() + delta + parameter.m_localUserTranslation);
        performTranslationTransform(tinystl::make_pair(translation, activeBone), createBoneList(activeBone));
        break;
    }
    case Model::kTransformCoordinateTypeLocal: {
        const model::Bone *bone = model::Bone::cast(activeBone);
        const Quaternion &orientation = bone->localOrientation();
        const Vector3 translation(
            (orientation * (accumulatedDeltaPosition() + delta)) + parameter.m_localUserTranslation);
        performTranslationTransform(tinystl::make_pair(translation, activeBone), createBoneList(activeBone));
        break;
    }
    case Model::kTransformCoordinateTypeMaxEnum:
    default:
        break;
    }
    updateLastCursorPosition(logicalPosition, delta);
}

const char *
AxisAlignedTranslateActiveBoneState::name() const NANOEM_DECL_NOEXCEPT
{
    return "nanoem.gui.viewport.bone.axis-aligned-translate";
}

AxisAlignedOrientateActiveBoneState::AxisAlignedOrientateActiveBoneState(
    Project *project, Model *model, const Vector2 &pressedCursorPosition, int axisIndex)
    : DraggingBoneState(project, model, pressedCursorPosition, Vector2())
    , m_axisIndex(axisIndex)
{
    setScaleFactor(0.25f);
}

void
AxisAlignedOrientateActiveBoneState::transform(const Vector2SI32 &logicalPosition)
{
    const Model *model = activeModel();
    const Model::TransformCoordinateType transformCoordinateType = model->transformCoordinateType();
    const nanoem_model_bone_t *activeBone = model->activeBone();
    const Vector3 axis(handleDragAxis(activeBone, transformCoordinateType, m_axisIndex) * Vector3(-1, -1, 1));
    const Vector3 delta(cursorDelta(logicalPosition), 0);
    nanoem_f32_t angle = glm::radians(accumulatedDeltaPosition().y + delta.y);
    const Quaternion &orientation = glm::angleAxis(angle, axis);
    performOrientationTransform(tinystl::make_pair(orientation, activeBone), createBoneList(activeBone));
    updateLastCursorPosition(logicalPosition, delta);
}

const char *
AxisAlignedOrientateActiveBoneState::name() const NANOEM_DECL_NOEXCEPT
{
    return "nanoem.gui.viewport.bone.axis-aligned-orientate";
}

} /* namespace internal */
} /* namespace nanoem */
