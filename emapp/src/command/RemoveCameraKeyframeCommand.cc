/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/RemoveCameraKeyframeCommand.h"

#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/private/CommonInclude.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"

namespace nanoem {
namespace command {

RemoveCameraKeyframeCommand::~RemoveCameraKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
RemoveCameraKeyframeCommand::create(const void *messagePtr, const ICamera *camera, Motion *motion)
{
    /* camera keyframe list is read from message so no need to feed keyframe list */
    RemoveCameraKeyframeCommand *command =
        nanoem_new(RemoveCameraKeyframeCommand(Motion::CameraKeyframeList(), camera, motion));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
RemoveCameraKeyframeCommand::create(const Motion::CameraKeyframeList &keyframes, const ICamera *camera, Motion *motion)
{
    RemoveCameraKeyframeCommand *command = nanoem_new(RemoveCameraKeyframeCommand(keyframes, camera, motion));
    return command->createCommand();
}

void
RemoveCameraKeyframeCommand::undo(Error &error)
{
    addKeyframe(error);
}

void
RemoveCameraKeyframeCommand::redo(Error &error)
{
    removeKeyframe(error);
}

const char *
RemoveCameraKeyframeCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "RemoveCameraKeyframeCommand";
}

BaseCameraKeyframeCommand::CameraKeyframeList
RemoveCameraKeyframeCommand::toKeyframeList(const Motion::CameraKeyframeList &keyframes, const Motion *motion)
{
    const IMotionKeyframeSelection *selection = motion->selection();
    CameraKeyframeList newKeyframes;
    for (Motion::CameraKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        const nanoem_motion_camera_keyframe_t *ko = *it;
        const nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(ko));
        CameraKeyframe keyframe(frameIndex);
        keyframe.m_state.first.assign(ko);
        keyframe.m_bezierCurveOverrideTargetFrameIndex =
            Motion::kMaxFrameIndex; /* join bezier curve is not supported */
        keyframe.m_selected = selection->contains(ko);
        newKeyframes.push_back(keyframe);
    }
    return newKeyframes;
}

RemoveCameraKeyframeCommand::RemoveCameraKeyframeCommand(
    const Motion::CameraKeyframeList &keyframes, const ICamera *camera, Motion *motion)
    : BaseCameraKeyframeCommand(toKeyframeList(keyframes, motion), camera, motion)
{
}

void
RemoveCameraKeyframeCommand::read(const void *messagePtr)
{
    readMessage(static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_remove_camera_keyframe, true);
}

void
RemoveCameraKeyframeCommand::write(void *messagePtr)
{
    writeMessage(messagePtr, NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_CAMERA_KEYFRAME);
}

void
RemoveCameraKeyframeCommand::release(void *messagePtr)
{
    releaseMessage(static_cast<Nanoem__Application__Command *>(messagePtr)->redo_remove_camera_keyframe);
}

} /* namespace command */
} /* namespace nanoem */
