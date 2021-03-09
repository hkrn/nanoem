/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/BaseSelfShadowKeyframeCommand.h"

#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/ShadowCamera.h"
#include "emapp/private/CommonInclude.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"

namespace nanoem {
namespace command {

BaseSelfShadowKeyframeCommand::~BaseSelfShadowKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

BaseSelfShadowKeyframeCommand::SelfShadowKeyframe::State::State()
    : m_distance(ShadowCamera::kInitialDistance)
    , m_mode(1)
{
}

BaseSelfShadowKeyframeCommand::SelfShadowKeyframe::State::~State() NANOEM_DECL_NOEXCEPT
{
}

void
BaseSelfShadowKeyframeCommand::SelfShadowKeyframe::State::assign(const ShadowCamera *shadow)
{
    m_distance = shadow->distance();
    m_mode = shadow->coverageMode();
}

void
BaseSelfShadowKeyframeCommand::SelfShadowKeyframe::State::assign(const nanoem_motion_self_shadow_keyframe_t *keyframe)
{
    m_distance = nanoemMotionSelfShadowKeyframeGetDistance(keyframe);
    m_mode = nanoemMotionSelfShadowKeyframeGetMode(keyframe);
}

BaseSelfShadowKeyframeCommand::BaseSelfShadowKeyframeCommand(
    const SelfShadowKeyframeList &keyframes, const ShadowCamera *shadow, Motion *motion)
    : BaseKeyframeCommand(motion)
    , m_shadowCameraPtr(shadow)
    , m_keyframes(keyframes)
{
    nanoem_assert(m_shadowCameraPtr, "must not be nullptr");
}

void
BaseSelfShadowKeyframeCommand::restoreKeyframeState(
    nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, const SelfShadowKeyframe::State &state)
{
    nanoem_assert(keyframe, "must NOT be nullptr");
    nanoemMutableMotionSelfShadowKeyframeSetDistance(keyframe, -state.m_distance);
    nanoemMutableMotionSelfShadowKeyframeSetMode(keyframe, state.m_mode);
}

void
BaseSelfShadowKeyframeCommand::addKeyframe(Error &error)
{
    if (currentProject()->containsMotion(m_motion)) {
        IMotionKeyframeSelection *selection = m_motion->selection();
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_motion_t *motion = m_motion->data();
        nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion, &status);
        resetTransformPerformedAt();
        for (SelfShadowKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
            SelfShadowKeyframe &keyframe = *it;
            nanoem_mutable_motion_self_shadow_keyframe_t *mutableKeyframe;
            if (keyframe.m_updated) {
                mutableKeyframe =
                    nanoemMutableMotionSelfShadowKeyframeCreateByFound(motion, keyframe.m_frameIndex, &status);
            }
            else {
                mutableKeyframe = nanoemMutableMotionSelfShadowKeyframeCreate(motion, &status);
                nanoemMutableMotionAddSelfShadowKeyframe(
                    mutableMotion, mutableKeyframe, keyframe.m_frameIndex, &status);
                if (keyframe.m_selected) {
                    selection->add(nanoemMutableMotionSelfShadowKeyframeGetOriginObject(mutableKeyframe));
                }
            }
            restoreKeyframeState(mutableKeyframe, keyframe.m_state.first);
            nanoemMutableMotionSelfShadowKeyframeDestroy(mutableKeyframe);
            if (status != NANOEM_STATUS_SUCCESS &&
                status != NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_ALREADY_EXISTS) {
                break;
            }
        }
        commit(mutableMotion);
        nanoemMutableMotionDestroy(mutableMotion);
        assignError(status, error);
    }
}

void
BaseSelfShadowKeyframeCommand::removeKeyframe(Error &error)
{
    if (currentProject()->containsMotion(m_motion)) {
        IMotionKeyframeSelection *selection = m_motion->selection();
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_motion_t *motion = m_motion->data();
        nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion, &status);
        resetTransformPerformedAt();
        for (SelfShadowKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
            SelfShadowKeyframe &keyframe = *it;
            nanoem_mutable_motion_self_shadow_keyframe_t *mutableKeyframe =
                nanoemMutableMotionSelfShadowKeyframeCreateByFound(motion, keyframe.m_frameIndex, &status);
            if (keyframe.m_updated) {
                restoreKeyframeState(mutableKeyframe, keyframe.m_state.second);
            }
            else {
                selection->remove(nanoemMutableMotionSelfShadowKeyframeGetOriginObject(mutableKeyframe));
                nanoemMutableMotionRemoveSelfShadowKeyframe(mutableMotion, mutableKeyframe, &status);
            }
            nanoemMutableMotionSelfShadowKeyframeDestroy(mutableKeyframe);
            if (status != NANOEM_STATUS_SUCCESS &&
                status != NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_NOT_FOUND) {
                break;
            }
        }
        commit(mutableMotion);
        nanoemMutableMotionDestroy(mutableMotion);
        assignError(status, error);
    }
}

void
BaseSelfShadowKeyframeCommand::readMessage(const void *messagePtr, bool removing)
{
    const Nanoem__Application__RedoSelfShadowKeyframeCommand *commandPtr =
        static_cast<const Nanoem__Application__RedoSelfShadowKeyframeCommand *>(messagePtr);
    nanoem_rsize_t numKeyframes = commandPtr->n_keyframes;
    m_keyframes.resize(numKeyframes);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const Nanoem__Application__RedoSelfShadowKeyframeCommand__Keyframe *k = commandPtr->keyframes[i];
        SelfShadowKeyframe &keyframe = m_keyframes[i];
        readStateMessage(k->current_state, keyframe.m_state.first);
        readStateMessage(k->last_state, keyframe.m_state.second);
        keyframe.setFrameIndex(m_motion->data(), CommandMessageUtil::saturatedFrameIndex(k->frame_index), removing);
    }
}

void
BaseSelfShadowKeyframeCommand::writeMessage(void *messagePtr, nanoem_u32_t type)
{
    Nanoem__Application__RedoSelfShadowKeyframeCommand__Keyframe **keyframes =
        new Nanoem__Application__RedoSelfShadowKeyframeCommand__Keyframe *[m_keyframes.size()];
    nanoem_rsize_t offset = 0;
    for (SelfShadowKeyframeList::const_iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
        const SelfShadowKeyframe &keyframe = *it;
        Nanoem__Application__RedoSelfShadowKeyframeCommand__Keyframe *k =
            nanoem_new(Nanoem__Application__RedoSelfShadowKeyframeCommand__Keyframe);
        nanoem__application__redo_self_shadow_keyframe_command__keyframe__init(k);
        k->frame_index = keyframe.m_frameIndex;
        writeStateMessage(keyframe.m_state.first, &k->current_state);
        writeStateMessage(keyframe.m_state.second, &k->last_state);
        keyframes[offset++] = k;
    }
    Nanoem__Application__RedoSelfShadowKeyframeCommand *command =
        nanoem_new(Nanoem__Application__RedoSelfShadowKeyframeCommand);
    nanoem__application__redo_self_shadow_keyframe_command__init(command);
    command->n_keyframes = m_keyframes.size();
    command->keyframes = keyframes;
    writeCommandMessage(command, type, messagePtr);
}

void
BaseSelfShadowKeyframeCommand::releaseMessage(void *messagePtr)
{
    Nanoem__Application__RedoSelfShadowKeyframeCommand *command =
        static_cast<Nanoem__Application__RedoSelfShadowKeyframeCommand *>(messagePtr);
    nanoem_rsize_t numKeyframes = command->n_keyframes;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        Nanoem__Application__RedoSelfShadowKeyframeCommand__Keyframe *k = command->keyframes[i];
        releaseStateMessage(k->current_state);
        releaseStateMessage(k->last_state);
        nanoem_delete(k);
    }
    delete[] command->keyframes;
    nanoem_delete(command);
}

void
BaseSelfShadowKeyframeCommand::readStateMessage(const void *opaque, SelfShadowKeyframe::State &s)
{
    const Nanoem__Application__RedoSelfShadowKeyframeCommand__State *state =
        static_cast<const Nanoem__Application__RedoSelfShadowKeyframeCommand__State *>(opaque);
    s.m_distance = state->distance;
    s.m_mode = state->mode;
}

void
BaseSelfShadowKeyframeCommand::writeStateMessage(const SelfShadowKeyframe::State &s, void *opaque)
{
    Nanoem__Application__RedoSelfShadowKeyframeCommand__State *
        *statePtr = static_cast<Nanoem__Application__RedoSelfShadowKeyframeCommand__State **>(opaque),
       *state = nanoem_new(Nanoem__Application__RedoSelfShadowKeyframeCommand__State);
    nanoem__application__redo_self_shadow_keyframe_command__state__init(state);
    state->distance = s.m_distance;
    state->mode = s.m_mode;
    *statePtr = state;
}

void
BaseSelfShadowKeyframeCommand::releaseStateMessage(void *opaque)
{
    Nanoem__Application__RedoSelfShadowKeyframeCommand__State *statePtr =
        static_cast<Nanoem__Application__RedoSelfShadowKeyframeCommand__State *>(opaque);
    nanoem_delete(statePtr);
}

} /* namespace command */
} /* namespace nanoem */
