/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/AddCameraKeyframeCommand.h"

#include "../CommandMessage.inl"
#include "emapp/ICamera.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace command {

AddCameraKeyframeCommand::~AddCameraKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
AddCameraKeyframeCommand::create(const void *messagePtr, const ICamera *camera, Motion *motion)
{
    /* camera keyframe state is read from message so no need to feed frame indices (to construct keyframe list) */
    AddCameraKeyframeCommand *command = nanoem_new(AddCameraKeyframeCommand(camera, Motion::FrameIndexList(), motion));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
AddCameraKeyframeCommand::create(
    const ICamera *camera, const Motion::FrameIndexList &targetFrameIndices, Motion *motion)
{
    AddCameraKeyframeCommand *command = nanoem_new(AddCameraKeyframeCommand(camera, targetFrameIndices, motion));
    return command->createCommand();
}

void
AddCameraKeyframeCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    removeKeyframe(error);
    nanoem_motion_t *m = m_motion ? m_motion->data() : nullptr;
    for (CameraKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
        CameraKeyframe &keyframe = *it;
        if (keyframe.m_bezierCurveOverrideTargetFrameIndex != Motion::kMaxFrameIndex) {
            nanoem_mutable_motion_camera_keyframe_t *ko = nanoemMutableMotionCameraKeyframeCreateByFound(
                m, keyframe.m_bezierCurveOverrideTargetFrameIndex, &status);
            const BezierControlPointParameterLookAtOnly &p = keyframe.m_lookAtParameters.second;
            for (int i = 0; i < int(BX_COUNTOF(p.m_value)); i++) {
                nanoemMutableMotionCameraKeyframeSetInterpolation(
                    ko, nanoem_motion_camera_keyframe_interpolation_type_t(i), glm::value_ptr(p.m_value[i]));
            }
            nanoemMutableMotionCameraKeyframeDestroy(ko);
        }
    }
}

void
AddCameraKeyframeCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    addKeyframe(error);
    nanoem_motion_t *m = m_motion ? m_motion->data() : nullptr;
    for (CameraKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
        CameraKeyframe &keyframe = *it;
        if (keyframe.m_bezierCurveOverrideTargetFrameIndex != Motion::kMaxFrameIndex) {
            nanoem_mutable_motion_camera_keyframe_t *ko = nanoemMutableMotionCameraKeyframeCreateByFound(
                m, keyframe.m_bezierCurveOverrideTargetFrameIndex, &status);
            const BezierControlPointParameterLookAtOnly &p = keyframe.m_lookAtParameters.first;
            for (int i = 0; i < int(BX_COUNTOF(p.m_value)); i++) {
                nanoemMutableMotionCameraKeyframeSetInterpolation(
                    ko, nanoem_motion_camera_keyframe_interpolation_type_t(i), glm::value_ptr(p.m_value[i]));
            }
            nanoemMutableMotionCameraKeyframeDestroy(ko);
        }
    }
}

const char *
AddCameraKeyframeCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "AddCameraKeyframeCommand";
}

BaseCameraKeyframeCommand::CameraKeyframeList
AddCameraKeyframeCommand::toKeyframeList(
    const ICamera *camera, const Motion::FrameIndexList &frameIndices, const Motion *motion)
{
    CameraKeyframeList newKeyframes;
    const Motion::FrameIndexList &newFrameIndices = motion ? frameIndices : Motion::FrameIndexList();
    const bool enableBezierCurveAdjustment = camera->project()->isBezierCurveAjustmentEnabled();
    for (Motion::FrameIndexList::const_iterator it = newFrameIndices.begin(), end = newFrameIndices.end(); it != end;
         ++it) {
        const nanoem_frame_index_t &frameIndex = *it;
        CameraKeyframe keyframe(frameIndex);
        keyframe.m_state.first.assign(camera);
        if (const nanoem_motion_camera_keyframe_t *ko = motion->findCameraKeyframe(frameIndex)) {
            keyframe.m_state.second.assign(ko);
            keyframe.m_updated = true;
        }
        else {
            nanoem_motion_camera_keyframe_t *prev = nullptr, *next = nullptr;
            nanoemMotionSearchClosestCameraKeyframes(motion->data(), frameIndex, &prev, &next);
            if (prev) {
                const nanoem_frame_index_t prevKeyframeIndex =
                    nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(prev));
                nanoem_frame_index_t nextKeyframeIndex = Motion::kMaxFrameIndex;
                if (next) {
                    nextKeyframeIndex =
                        nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(next));
                }
                keyframe.m_bezierCurveOverrideTargetFrameIndex = prevKeyframeIndex;
                BezierControlPointParameterLookAtOnlyPair &p2 = keyframe.m_lookAtParameters;
                CameraKeyframe::State &newState = keyframe.m_state.first;
                for (int i = 0; i < int(BX_COUNTOF(p2.first.m_value)); i++) {
                    nanoem_motion_camera_keyframe_interpolation_type_t type =
                        nanoem_motion_camera_keyframe_interpolation_type_t(i);
                    const Vector4U8 prevKeyframeInterpolationParameter(
                        glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(prev, type)));
                    if (enableBezierCurveAdjustment && nextKeyframeIndex != Motion::kMaxFrameIndex &&
                        nextKeyframeIndex > prevKeyframeIndex) {
                        const nanoem_frame_index_t interval = nextKeyframeIndex - prevKeyframeIndex;
                        const Vector2 c0(prevKeyframeInterpolationParameter.x, prevKeyframeInterpolationParameter.y),
                            c1(prevKeyframeInterpolationParameter.z, prevKeyframeInterpolationParameter.w);
                        const BezierCurve bezierCurve(c0, c1, interval);
                        BezierCurve::Pair pair(
                            bezierCurve.split(nanoem_f32_t((frameIndex - prevKeyframeIndex) / nanoem_f64_t(interval))));
                        p2.first.m_value[i] = pair.first->toParameters();
                        newState.m_parameter.m_value[i] = pair.second->toParameters();
                        nanoem_delete(pair.first);
                        nanoem_delete(pair.second);
                    }
                    else {
                        p2.first.m_value[i] = prevKeyframeInterpolationParameter;
                    }
                    p2.second.m_value[i] = prevKeyframeInterpolationParameter;
                }
            }
            keyframe.m_selected = true;
        }
        newKeyframes.push_back(keyframe);
    }
    return newKeyframes;
}

AddCameraKeyframeCommand::AddCameraKeyframeCommand(
    const ICamera *camera, const Motion::FrameIndexList &targetFrameIndices, Motion *motion)
    : BaseCameraKeyframeCommand(toKeyframeList(camera, targetFrameIndices, motion), camera, motion)
{
}

void
AddCameraKeyframeCommand::read(const void *messagePtr)
{
    readMessage(static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_add_camera_keyframe, false);
}

void
AddCameraKeyframeCommand::write(void *messagePtr)
{
    writeMessage(messagePtr, NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_CAMERA_KEYFRAME);
}

void
AddCameraKeyframeCommand::release(void *messagePtr)
{
    releaseMessage(static_cast<Nanoem__Application__Command *>(messagePtr)->redo_add_camera_keyframe);
}

} /* namespace command */
} /* namespace nanoem */
