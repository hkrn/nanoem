/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/BaseCameraKeyframeCommand.h"

#include "emapp/Constants.h"
#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/PerspectiveCamera.h"
#include "emapp/private/CommonInclude.h"

#include "../CommandMessage.inl"

namespace nanoem {
namespace command {

BaseCameraKeyframeCommand::~BaseCameraKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

BaseCameraKeyframeCommand::CameraKeyframe::State::State()
    : m_lookAt(Constants::kZeroV4)
    , m_angle(Constants::kZeroV4)
    , m_distance(PerspectiveCamera::kInitialDistance)
    , m_fov(PerspectiveCamera::kInitialFovRadian)
    , m_stageIndex(0)
    , m_perspective(true)
{
    for (nanoem_rsize_t i = 0; i < BX_COUNTOF(m_parameter.m_value); i++) {
        m_parameter.m_value[i] = PerspectiveCamera::kDefaultBezierControlPoint;
    }
}

BaseCameraKeyframeCommand::CameraKeyframe::State::~State() NANOEM_DECL_NOEXCEPT
{
}

void
BaseCameraKeyframeCommand::CameraKeyframe::State::assign(const ICamera *camera)
{
    m_angle = Vector4(camera->angle() * PerspectiveCamera::kAngleScaleFactor, 0);
    m_lookAt = Vector4(camera->lookAt(), 1);
    m_fov = camera->fovRadians();
    m_distance = camera->distance();
    m_perspective = camera->isPerspective();
    for (size_t j = 0; j < BX_COUNTOF(m_parameter.m_value); j++) {
        m_parameter.m_value[j] =
            camera->bezierControlPoints(static_cast<nanoem_motion_camera_keyframe_interpolation_type_t>(j));
    }
}

void
BaseCameraKeyframeCommand::CameraKeyframe::State::assign(const nanoem_motion_camera_keyframe_t *keyframe)
{
    m_angle = glm::make_vec4(nanoemMotionCameraKeyframeGetAngle(keyframe));
    m_lookAt = glm::make_vec4(nanoemMotionCameraKeyframeGetLookAt(keyframe));
    m_fov = glm::radians(nanoem_f32_t(nanoemMotionCameraKeyframeGetFov(keyframe)));
    m_distance = nanoemMotionCameraKeyframeGetDistance(keyframe);
    m_perspective = nanoemMotionCameraKeyframeIsPerspectiveView(keyframe) != 0;
    for (int i = 0; i < int(BX_COUNTOF(m_parameter.m_value)); i++) {
        m_parameter.m_value[i] = glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
            keyframe, nanoem_motion_camera_keyframe_interpolation_type_t(i)));
    }
}

BaseCameraKeyframeCommand::BaseCameraKeyframeCommand(
    const CameraKeyframeList &keyframes, const ICamera *camera, Motion *motion)
    : BaseKeyframeCommand(motion)
    , m_camera(camera)
    , m_keyframes(keyframes)
{
    nanoem_assert(m_camera, "must not be nullptr");
}

void
BaseCameraKeyframeCommand::restoreKeyframeState(
    nanoem_mutable_motion_camera_keyframe_t *keyframe, const CameraKeyframe::State &state, bool &linear)
{
    nanoem_assert(keyframe, "must NOT be nullptr");
    nanoemMutableMotionCameraKeyframeSetAngle(keyframe, glm::value_ptr(state.m_angle));
    nanoemMutableMotionCameraKeyframeSetLookAt(keyframe, glm::value_ptr(state.m_lookAt));
    nanoemMutableMotionCameraKeyframeSetFov(keyframe, int(glm::round(glm::degrees(state.m_fov))));
    nanoemMutableMotionCameraKeyframeSetDistance(keyframe, -state.m_distance);
    nanoemMutableMotionCameraKeyframeSetPerspectiveView(keyframe, state.m_perspective);
    linear = true;
    const BezierControlPointParameter &p = state.m_parameter;
    for (int i = 0; i < int(BX_COUNTOF(p.m_value)); i++) {
        nanoem_motion_camera_keyframe_interpolation_type_t type = nanoem_motion_camera_keyframe_interpolation_type_t(i);
        nanoemMutableMotionCameraKeyframeSetInterpolation(keyframe, type, glm::value_ptr(p.m_value[i]));
        linear &= (nanoemMotionCameraKeyframeIsLinearInterpolation(
                       nanoemMutableMotionCameraKeyframeGetOriginObject(keyframe), type) != 0);
    }
}

void
BaseCameraKeyframeCommand::addKeyframe(Error &error)
{
    if (currentProject()->containsMotion(m_motion)) {
        IMotionKeyframeSelection *selection = m_motion->selection();
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_motion_t *motion = m_motion->data();
        nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion, &status);
        resetTransformPerformedAt();
        for (CameraKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
            CameraKeyframe &keyframe = *it;
            nanoem_mutable_motion_camera_keyframe_t *mutableKeyframe;
            if (keyframe.m_updated) {
                mutableKeyframe =
                    nanoemMutableMotionCameraKeyframeCreateByFound(motion, keyframe.m_frameIndex, &status);
            }
            else {
                mutableKeyframe = nanoemMutableMotionCameraKeyframeCreate(motion, &status);
                nanoemMutableMotionAddCameraKeyframe(mutableMotion, mutableKeyframe, keyframe.m_frameIndex, &status);
                if (keyframe.m_selected) {
                    selection->add(nanoemMutableMotionCameraKeyframeGetOriginObject(mutableKeyframe));
                }
            }
            bool linear;
            restoreKeyframeState(mutableKeyframe, keyframe.m_state.first, linear);
            nanoemMutableMotionCameraKeyframeDestroy(mutableKeyframe);
            if (status != NANOEM_STATUS_SUCCESS &&
                status != NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_ALREADY_EXISTS) {
                break;
            }
        }
        commit(mutableMotion);
        nanoemMutableMotionDestroy(mutableMotion);
        assignError(status, error);
    }
}

void
BaseCameraKeyframeCommand::removeKeyframe(Error &error)
{
    if (currentProject()->containsMotion(m_motion)) {
        IMotionKeyframeSelection *selection = m_motion->selection();
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_motion_t *motion = m_motion->data();
        nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion, &status);
        resetTransformPerformedAt();
        for (CameraKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
            CameraKeyframe &keyframe = *it;
            nanoem_mutable_motion_camera_keyframe_t *mutableKeyframe =
                nanoemMutableMotionCameraKeyframeCreateByFound(motion, keyframe.m_frameIndex, &status);
            if (keyframe.m_updated) {
                bool linear;
                restoreKeyframeState(mutableKeyframe, keyframe.m_state.second, linear);
            }
            else {
                selection->remove(nanoemMutableMotionCameraKeyframeGetOriginObject(mutableKeyframe));
                nanoemMutableMotionRemoveCameraKeyframe(mutableMotion, mutableKeyframe, &status);
            }
            nanoemMutableMotionCameraKeyframeDestroy(mutableKeyframe);
            if (status != NANOEM_STATUS_SUCCESS && status != NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_NOT_FOUND) {
                break;
            }
        }
        commit(mutableMotion);
        nanoemMutableMotionDestroy(mutableMotion);
        assignError(status, error);
    }
}

void
BaseCameraKeyframeCommand::readMessage(const void *messagePtr, bool removing)
{
    const Nanoem__Application__RedoCameraKeyframeCommand *commandPtr =
        static_cast<const Nanoem__Application__RedoCameraKeyframeCommand *>(messagePtr);
    nanoem_rsize_t numKeyframes = commandPtr->n_keyframes;
    m_keyframes.resize(numKeyframes);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const Nanoem__Application__RedoCameraKeyframeCommand__Keyframe *k = commandPtr->keyframes[i];
        CameraKeyframe &keyframe = m_keyframes[i];
        readStateMessage(k->current_state, keyframe.m_state.first);
        readStateMessage(k->last_state, keyframe.m_state.second);
        keyframe.setFrameIndex(m_motion->data(), CommandMessageUtil::saturatedFrameIndex(k->frame_index), removing);
    }
}

void
BaseCameraKeyframeCommand::writeMessage(void *messagePtr, nanoem_u32_t type)
{
    Nanoem__Application__RedoCameraKeyframeCommand__Keyframe **keyframes =
        new Nanoem__Application__RedoCameraKeyframeCommand__Keyframe *[m_keyframes.size()];
    nanoem_rsize_t offset = 0;
    for (CameraKeyframeList::const_iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
        const CameraKeyframe &keyframe = *it;
        Nanoem__Application__RedoCameraKeyframeCommand__Keyframe *k =
            nanoem_new(Nanoem__Application__RedoCameraKeyframeCommand__Keyframe);
        nanoem__application__redo_camera_keyframe_command__keyframe__init(k);
        k->frame_index = keyframe.m_frameIndex;
        writeStateMessage(keyframe.m_state.first, &k->current_state);
        writeStateMessage(keyframe.m_state.second, &k->last_state);
        keyframes[offset++] = k;
    }
    Nanoem__Application__RedoCameraKeyframeCommand *command =
        nanoem_new(Nanoem__Application__RedoCameraKeyframeCommand);
    nanoem__application__redo_camera_keyframe_command__init(command);
    command->n_keyframes = m_keyframes.size();
    command->keyframes = keyframes;
    writeCommandMessage(command, type, messagePtr);
}

void
BaseCameraKeyframeCommand::releaseMessage(void *messagePtr)
{
    Nanoem__Application__RedoCameraKeyframeCommand *command =
        static_cast<Nanoem__Application__RedoCameraKeyframeCommand *>(messagePtr);
    nanoem_rsize_t numKeyframes = command->n_keyframes;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        Nanoem__Application__RedoCameraKeyframeCommand__Keyframe *k = command->keyframes[i];
        releaseStateMessage(k->current_state);
        releaseStateMessage(k->last_state);
        nanoem_delete(k);
    }
    delete[] command->keyframes;
    nanoem_delete(command);
}

void
BaseCameraKeyframeCommand::readStateMessage(const void *opaque, CameraKeyframe::State &s)
{
    const Nanoem__Application__RedoCameraKeyframeCommand__State *state =
        static_cast<const Nanoem__Application__RedoCameraKeyframeCommand__State *>(opaque);
    CommandMessageUtil::getVector(state->angle, s.m_angle);
    CommandMessageUtil::getVector(state->look_at, s.m_lookAt);
    s.m_distance = state->distance;
    s.m_fov = state->fov;
    s.m_perspective = state->perspective != 0;
    CommandMessageUtil::getInterpolation(
        state->interpolation->x, s.m_parameter.m_value[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X]);
    CommandMessageUtil::getInterpolation(
        state->interpolation->y, s.m_parameter.m_value[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y]);
    CommandMessageUtil::getInterpolation(
        state->interpolation->z, s.m_parameter.m_value[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z]);
    CommandMessageUtil::getInterpolation(
        state->interpolation->angle, s.m_parameter.m_value[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE]);
    CommandMessageUtil::getInterpolation(state->interpolation->distance,
        s.m_parameter.m_value[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE]);
    CommandMessageUtil::getInterpolation(
        state->interpolation->fov, s.m_parameter.m_value[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV]);
}

void
BaseCameraKeyframeCommand::writeStateMessage(const CameraKeyframe::State &s, void *opaque)
{
    Nanoem__Application__RedoCameraKeyframeCommand__State *
        *statePtr = static_cast<Nanoem__Application__RedoCameraKeyframeCommand__State **>(opaque),
       *state = nanoem_new(Nanoem__Application__RedoCameraKeyframeCommand__State);
    nanoem__application__redo_camera_keyframe_command__state__init(state);
    CommandMessageUtil::setVector(s.m_angle, state->angle);
    CommandMessageUtil::setVector(s.m_lookAt, state->look_at);
    state->distance = s.m_distance;
    state->fov = s.m_fov;
    state->perspective = s.m_perspective;
    state->interpolation = nanoem_new(Nanoem__Application__CameraInterpolation);
    nanoem__application__camera_interpolation__init(state->interpolation);
    CommandMessageUtil::setInterpolation(
        s.m_parameter.m_value[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X], state->interpolation->x);
    CommandMessageUtil::setInterpolation(
        s.m_parameter.m_value[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y], state->interpolation->y);
    CommandMessageUtil::setInterpolation(
        s.m_parameter.m_value[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z], state->interpolation->z);
    CommandMessageUtil::setInterpolation(
        s.m_parameter.m_value[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE], state->interpolation->angle);
    CommandMessageUtil::setInterpolation(
        s.m_parameter.m_value[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV], state->interpolation->fov);
    CommandMessageUtil::setInterpolation(
        s.m_parameter.m_value[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE],
        state->interpolation->distance);
    *statePtr = state;
}

void
BaseCameraKeyframeCommand::releaseStateMessage(void *opaque)
{
    Nanoem__Application__RedoCameraKeyframeCommand__State *statePtr =
        static_cast<Nanoem__Application__RedoCameraKeyframeCommand__State *>(opaque);
    CommandMessageUtil::releaseVector(statePtr->look_at);
    CommandMessageUtil::releaseVector(statePtr->angle);
    CommandMessageUtil::releaseInterpolation(statePtr->interpolation->x);
    CommandMessageUtil::releaseInterpolation(statePtr->interpolation->y);
    CommandMessageUtil::releaseInterpolation(statePtr->interpolation->z);
    CommandMessageUtil::releaseInterpolation(statePtr->interpolation->angle);
    CommandMessageUtil::releaseInterpolation(statePtr->interpolation->fov);
    CommandMessageUtil::releaseInterpolation(statePtr->interpolation->distance);
    nanoem_delete(statePtr->interpolation);
    nanoem_delete(statePtr);
}

} /* namespace command */
} /* namespace nanoem */
