/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/BaseLightKeyframeCommand.h"

#include "emapp/DirectionalLight.h"
#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/private/CommonInclude.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"

namespace nanoem {
namespace command {

BaseLightKeyframeCommand::~BaseLightKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

BaseLightKeyframeCommand::LightKeyframe::State::State()
    : m_color(DirectionalLight::kInitialColor, 1)
    , m_direction(DirectionalLight::kInitialDirection, 0)
{
}

BaseLightKeyframeCommand::LightKeyframe::State::~State() NANOEM_DECL_NOEXCEPT
{
}

void
BaseLightKeyframeCommand::LightKeyframe::State::assign(const ILight *light)
{
    m_color = Vector4(light->color(), 1);
    m_direction = Vector4(light->direction(), 0);
}

void
BaseLightKeyframeCommand::LightKeyframe::State::assign(const nanoem_motion_light_keyframe_t *keyframe)
{
    m_color = glm::make_vec4(nanoemMotionLightKeyframeGetColor(keyframe));
    m_direction = glm::make_vec4(nanoemMotionLightKeyframeGetDirection(keyframe));
}

BaseLightKeyframeCommand::BaseLightKeyframeCommand(
    const LightKeyframeList &keyframes, const ILight *light, Motion *motion)
    : BaseKeyframeCommand(motion)
    , m_light(light)
    , m_keyframes(keyframes)
{
    nanoem_assert(m_light, "must not be nullptr");
}

void
BaseLightKeyframeCommand::restoreKeyframeState(
    nanoem_mutable_motion_light_keyframe_t *keyframe, const LightKeyframe::State &state)
{
    nanoem_assert(keyframe, "must NOT be nullptr");
    nanoemMutableMotionLightKeyframeSetColor(keyframe, glm::value_ptr(state.m_color));
    nanoemMutableMotionLightKeyframeSetDirection(keyframe, glm::value_ptr(state.m_direction));
}

void
BaseLightKeyframeCommand::addKeyframe(Error &error)
{
    if (currentProject()->containsMotion(m_motion)) {
        IMotionKeyframeSelection *selection = m_motion->selection();
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_motion_t *motion = m_motion->data();
        nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion, &status);
        resetTransformPerformedAt();
        for (LightKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
            LightKeyframe &keyframe = *it;
            nanoem_mutable_motion_light_keyframe_t *mutableKeyframe;
            if (keyframe.m_updated) {
                mutableKeyframe = nanoemMutableMotionLightKeyframeCreateByFound(motion, keyframe.m_frameIndex, &status);
            }
            else {
                mutableKeyframe = nanoemMutableMotionLightKeyframeCreate(motion, &status);
                nanoemMutableMotionAddLightKeyframe(mutableMotion, mutableKeyframe, keyframe.m_frameIndex, &status);
                if (keyframe.m_selected) {
                    selection->add(nanoemMutableMotionLightKeyframeGetOriginObject(mutableKeyframe));
                }
            }
            restoreKeyframeState(mutableKeyframe, keyframe.m_state.first);
            nanoemMutableMotionLightKeyframeDestroy(mutableKeyframe);
            if (status != NANOEM_STATUS_SUCCESS && status != NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_ALREADY_EXISTS) {
                break;
            }
        }
        commit(mutableMotion);
        nanoemMutableMotionDestroy(mutableMotion);
        assignError(status, error);
    }
}

void
BaseLightKeyframeCommand::removeKeyframe(Error &error)
{
    IMotionKeyframeSelection *selection = m_motion->selection();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_motion_t *motion = m_motion ? m_motion->data() : nullptr;
    nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion, &status);
    resetTransformPerformedAt();
    for (LightKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
        LightKeyframe &keyframe = *it;
        nanoem_mutable_motion_light_keyframe_t *mutableKeyframe =
            nanoemMutableMotionLightKeyframeCreateByFound(motion, keyframe.m_frameIndex, &status);
        if (keyframe.m_updated) {
            restoreKeyframeState(mutableKeyframe, keyframe.m_state.second);
        }
        else {
            selection->remove(nanoemMutableMotionLightKeyframeGetOriginObject(mutableKeyframe));
            nanoemMutableMotionRemoveLightKeyframe(mutableMotion, mutableKeyframe, &status);
        }
        nanoemMutableMotionLightKeyframeDestroy(mutableKeyframe);
        if (status != NANOEM_STATUS_SUCCESS && status != NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_NOT_FOUND) {
            break;
        }
    }
    commit(mutableMotion);
    nanoemMutableMotionDestroy(mutableMotion);
    assignError(status, error);
}

void
BaseLightKeyframeCommand::readMessage(const void *messagePtr, bool removing)
{
    const Nanoem__Application__RedoLightKeyframeCommand *commandPtr =
        static_cast<const Nanoem__Application__RedoLightKeyframeCommand *>(messagePtr);
    nanoem_rsize_t numKeyframes = commandPtr->n_keyframes;
    m_keyframes.resize(numKeyframes);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const Nanoem__Application__RedoLightKeyframeCommand__Keyframe *k = commandPtr->keyframes[i];
        LightKeyframe &keyframe = m_keyframes[i];
        readStateMessage(k->current_state, keyframe.m_state.first);
        readStateMessage(k->last_state, keyframe.m_state.second);
        keyframe.setFrameIndex(m_motion->data(), CommandMessageUtil::saturatedFrameIndex(k->frame_index), removing);
    }
}

void
BaseLightKeyframeCommand::writeMessage(void *messagePtr, nanoem_u32_t type)
{
    Nanoem__Application__RedoLightKeyframeCommand__Keyframe **keyframes =
        new Nanoem__Application__RedoLightKeyframeCommand__Keyframe *[m_keyframes.size()];
    nanoem_rsize_t offset = 0;
    for (LightKeyframeList::const_iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
        const LightKeyframe &keyframe = *it;
        Nanoem__Application__RedoLightKeyframeCommand__Keyframe *k =
            nanoem_new(Nanoem__Application__RedoLightKeyframeCommand__Keyframe);
        nanoem__application__redo_light_keyframe_command__keyframe__init(k);
        k->frame_index = keyframe.m_frameIndex;
        writeStateMessage(keyframe.m_state.first, &k->current_state);
        writeStateMessage(keyframe.m_state.second, &k->last_state);
        keyframes[offset++] = k;
    }
    Nanoem__Application__RedoLightKeyframeCommand *command = nanoem_new(Nanoem__Application__RedoLightKeyframeCommand);
    nanoem__application__redo_light_keyframe_command__init(command);
    command->n_keyframes = m_keyframes.size();
    command->keyframes = keyframes;
    writeCommandMessage(command, type, messagePtr);
}

void
BaseLightKeyframeCommand::releaseMessage(void *messagePtr)
{
    Nanoem__Application__RedoLightKeyframeCommand *command =
        static_cast<Nanoem__Application__RedoLightKeyframeCommand *>(messagePtr);
    nanoem_rsize_t numKeyframes = command->n_keyframes;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        Nanoem__Application__RedoLightKeyframeCommand__Keyframe *k = command->keyframes[i];
        releaseStateMessage(k->current_state);
        releaseStateMessage(k->last_state);
        nanoem_delete(k);
    }
    delete[] command->keyframes;
    nanoem_delete(command);
}

void
BaseLightKeyframeCommand::readStateMessage(const void *opaque, LightKeyframe::State &s)
{
    const Nanoem__Application__RedoLightKeyframeCommand__State *state =
        static_cast<const Nanoem__Application__RedoLightKeyframeCommand__State *>(opaque);
    CommandMessageUtil::getColor(state->color, s.m_color);
    CommandMessageUtil::getVector(state->direction, s.m_direction);
}

void
BaseLightKeyframeCommand::writeStateMessage(const LightKeyframe::State &s, void *opaque)
{
    Nanoem__Application__RedoLightKeyframeCommand__State *
        *statePtr = static_cast<Nanoem__Application__RedoLightKeyframeCommand__State **>(opaque),
       *state = nanoem_new(Nanoem__Application__RedoLightKeyframeCommand__State);
    nanoem__application__redo_light_keyframe_command__state__init(state);
    CommandMessageUtil::setColor(s.m_color, state->color);
    CommandMessageUtil::setVector(s.m_direction, state->direction);
    *statePtr = state;
}

void
BaseLightKeyframeCommand::releaseStateMessage(void *opaque)
{
    Nanoem__Application__RedoLightKeyframeCommand__State *statePtr =
        static_cast<Nanoem__Application__RedoLightKeyframeCommand__State *>(opaque);
    CommandMessageUtil::releaseVector(statePtr->color);
    CommandMessageUtil::releaseVector(statePtr->direction);
    nanoem_delete(statePtr);
}

} /* namespace command */
} /* namespace nanoem */
