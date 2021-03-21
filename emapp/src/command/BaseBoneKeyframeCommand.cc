/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/BaseBoneKeyframeCommand.h"

#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/private/CommonInclude.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"

namespace nanoem {
namespace command {

BaseBoneKeyframeCommand::~BaseBoneKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
    nanoem_unicode_string_factory_t *factory = currentProject()->unicodeStringFactory();
    for (BoneKeyframeList::const_iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
        nanoemUnicodeStringFactoryDestroyString(factory, it->m_name);
    }
}

BaseBoneKeyframeCommand::BoneKeyframe::State::State()
    : m_translation(Constants::kZeroV4)
    , m_orientation(Constants::kZeroQ)
    , m_stageIndex(0)
    , m_enablePhysicsSimulation(true)
{
    for (nanoem_rsize_t i = 0; i < BX_COUNTOF(m_parameter.m_value); i++) {
        m_parameter.m_value[i] = model::Bone::kDefaultBezierControlPoint;
    }
}

BaseBoneKeyframeCommand::BoneKeyframe::State::~State() NANOEM_DECL_NOEXCEPT
{
}

void
BaseBoneKeyframeCommand::BoneKeyframe::State::assign(const model::Bone *bone)
{
    m_translation = Vector4(bone->localUserTranslation(), 1);
    m_orientation = bone->localUserOrientation();
    for (int i = 0; i < int(BX_COUNTOF(m_parameter.m_value)); i++) {
        m_parameter.m_value[i] = bone->bezierControlPoints(nanoem_motion_bone_keyframe_interpolation_type_t(i));
    }
}

void
BaseBoneKeyframeCommand::BoneKeyframe::State::assign(const nanoem_motion_bone_keyframe_t *keyframe)
{
    m_translation = glm::make_vec4(nanoemMotionBoneKeyframeGetTranslation(keyframe));
    m_orientation = glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(keyframe));
    m_stageIndex = nanoemMotionBoneKeyframeGetStageIndex(keyframe);
    m_enablePhysicsSimulation = nanoemMotionBoneKeyframeIsPhysicsSimulationEnabled(keyframe) != 0;
    for (int i = 0; i < int(BX_COUNTOF(m_parameter.m_value)); i++) {
        m_parameter.m_value[i] = glm::make_vec4(
            nanoemMotionBoneKeyframeGetInterpolation(keyframe, nanoem_motion_bone_keyframe_interpolation_type_t(i)));
    }
}

BaseBoneKeyframeCommand::BaseBoneKeyframeCommand(const BoneKeyframeList &keyframes, const Model *model, Motion *motion)
    : BaseKeyframeCommand(motion)
    , m_model(model)
    , m_keyframes(keyframes)
{
    nanoem_assert(m_model, "must not be nullptr");
}

void
BaseBoneKeyframeCommand::restoreKeyframeState(
    nanoem_mutable_motion_bone_keyframe_t *keyframe, const BoneKeyframe::State &state, bool &linear)
{
    nanoem_assert(keyframe, "must NOT be nullptr");
    nanoemMutableMotionBoneKeyframeSetTranslation(keyframe, glm::value_ptr(state.m_translation));
    nanoemMutableMotionBoneKeyframeSetOrientation(keyframe, glm::value_ptr(state.m_orientation));
    nanoemMutableMotionBoneKeyframeSetPhysicsSimulationEnabled(keyframe, state.m_enablePhysicsSimulation);
    nanoemMutableMotionBoneKeyframeSetStageIndex(keyframe, state.m_stageIndex);
    linear = true;
    const BezierControlPointParameter &p = state.m_parameter;
    for (int i = 0; i < int(BX_COUNTOF(p.m_value)); i++) {
        nanoemMutableMotionBoneKeyframeSetInterpolation(
            keyframe, nanoem_motion_bone_keyframe_interpolation_type_t(i), glm::value_ptr(p.m_value[i]));
        linear &=
            (nanoemMotionBoneKeyframeIsLinearInterpolation(nanoemMutableMotionBoneKeyframeGetOriginObject(keyframe),
                 nanoem_motion_bone_keyframe_interpolation_type_t(i)) != 0);
    }
}

void
BaseBoneKeyframeCommand::addKeyframe(Error &error)
{
    if (currentProject()->containsMotion(m_motion)) {
        IMotionKeyframeSelection *selection = m_motion->selection();
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_motion_t *motion = m_motion->data();
        nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion, &status);
        resetTransformPerformedAt();
        for (BoneKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
            BoneKeyframe &keyframe = *it;
            nanoem_mutable_motion_bone_keyframe_t *mutableKeyframe;
            if (keyframe.m_updated) {
                mutableKeyframe = nanoemMutableMotionBoneKeyframeCreateByFound(
                    motion, keyframe.m_name, keyframe.m_frameIndex, &status);
            }
            else {
                mutableKeyframe = nanoemMutableMotionBoneKeyframeCreate(motion, &status);
                nanoemMutableMotionAddBoneKeyframe(
                    mutableMotion, mutableKeyframe, keyframe.m_name, keyframe.m_frameIndex, &status);
                if (keyframe.m_selected) {
                    selection->add(nanoemMutableMotionBoneKeyframeGetOriginObject(mutableKeyframe));
                }
            }
            bool linear;
            restoreKeyframeState(mutableKeyframe, keyframe.m_state.first, linear);
            nanoemMutableMotionBoneKeyframeDestroy(mutableKeyframe);
            if (status != NANOEM_STATUS_SUCCESS && status != NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_ALREADY_EXISTS) {
                break;
            }
            else if (model::Bone *bone = model::Bone::cast(m_model->findBone(keyframe.m_name))) {
                if (bone->isDirty()) {
                    keyframe.wasDirty = true;
                    bone->setDirty(false);
                }
            }
        }
        commit(mutableMotion);
        nanoemMutableMotionDestroy(mutableMotion);
        assignError(status, error);
    }
}

void
BaseBoneKeyframeCommand::removeKeyframe(Error &error)
{
    if (currentProject()->containsMotion(m_motion)) {
        IMotionKeyframeSelection *selection = m_motion->selection();
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_motion_t *motion = m_motion->data();
        nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion, &status);
        resetTransformPerformedAt();
        for (BoneKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
            BoneKeyframe &keyframe = *it;
            nanoem_mutable_motion_bone_keyframe_t *mutableKeyframe =
                nanoemMutableMotionBoneKeyframeCreateByFound(motion, keyframe.m_name, keyframe.m_frameIndex, &status);
            if (keyframe.m_updated) {
                bool linear;
                restoreKeyframeState(mutableKeyframe, keyframe.m_state.second, linear);
            }
            else {
                selection->remove(nanoemMutableMotionBoneKeyframeGetOriginObject(mutableKeyframe));
                nanoemMutableMotionRemoveBoneKeyframe(mutableMotion, mutableKeyframe, &status);
            }
            nanoemMutableMotionBoneKeyframeDestroy(mutableKeyframe);
            if (status != NANOEM_STATUS_SUCCESS && status != NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_NOT_FOUND) {
                break;
            }
            else if (keyframe.wasDirty) {
                if (model::Bone *bone = model::Bone::cast(m_model->findBone(keyframe.m_name))) {
                    bone->setDirty(true);
                }
            }
        }
        commit(mutableMotion);
        nanoemMutableMotionDestroy(mutableMotion);
        assignError(status, error);
    }
}

void
BaseBoneKeyframeCommand::readMessage(const void *messagePtr, bool removing)
{
    const Nanoem__Application__RedoBoneKeyframeCommand *commandPtr =
        static_cast<const Nanoem__Application__RedoBoneKeyframeCommand *>(messagePtr);
    Model *model = currentProject()->resolveRedoModel(commandPtr->model_handle);
    if (model) {
        m_model = model;
        nanoem_rsize_t numKeyframes = commandPtr->n_keyframes;
        m_keyframes.resize(numKeyframes);
        nanoem_unicode_string_factory_t *factory = currentProject()->unicodeStringFactory();
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
            const Nanoem__Application__RedoBoneKeyframeCommand__Keyframe *k = commandPtr->keyframes[i];
            BoneKeyframe &keyframe = m_keyframes[i];
            readStateMessage(k->current_state, keyframe.m_state.first);
            readStateMessage(k->last_state, keyframe.m_state.second);
            keyframe.m_name = nanoemUnicodeStringFactoryCloneString(factory,
                nanoemModelBoneGetName(m_model->findRedoBone(k->bone_index), NANOEM_LANGUAGE_TYPE_FIRST_ENUM), &status);
            keyframe.wasDirty = k->dirty != 0;
            keyframe.setFrameIndex(m_motion->data(), CommandMessageUtil::saturatedFrameIndex(k->frame_index), removing);
        }
    }
}

void
BaseBoneKeyframeCommand::writeMessage(void *messagePtr, nanoem_u32_t type)
{
    Nanoem__Application__RedoBoneKeyframeCommand__Keyframe **keyframes =
        new Nanoem__Application__RedoBoneKeyframeCommand__Keyframe *[m_keyframes.size()];
    nanoem_rsize_t offset = 0;
    for (BoneKeyframeList::const_iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
        const BoneKeyframe &keyframe = *it;
        Nanoem__Application__RedoBoneKeyframeCommand__Keyframe *k =
            nanoem_new(Nanoem__Application__RedoBoneKeyframeCommand__Keyframe);
        nanoem__application__redo_bone_keyframe_command__keyframe__init(k);
        k->bone_index = model::Bone::index(m_model->findBone(keyframe.m_name));
        k->frame_index = keyframe.m_frameIndex;
        k->dirty = keyframe.wasDirty;
        writeStateMessage(keyframe.m_state.first, &k->current_state);
        writeStateMessage(keyframe.m_state.second, &k->last_state);
        keyframes[offset++] = k;
    }
    Nanoem__Application__RedoBoneKeyframeCommand *command = nanoem_new(Nanoem__Application__RedoBoneKeyframeCommand);
    nanoem__application__redo_bone_keyframe_command__init(command);
    command->model_handle = m_model->handle();
    command->n_keyframes = m_keyframes.size();
    command->keyframes = keyframes;
    writeCommandMessage(command, type, messagePtr);
}

void
BaseBoneKeyframeCommand::releaseMessage(void *messagePtr)
{
    Nanoem__Application__RedoBoneKeyframeCommand *command =
        static_cast<Nanoem__Application__RedoBoneKeyframeCommand *>(messagePtr);
    nanoem_rsize_t numKeyframes = command->n_keyframes;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        Nanoem__Application__RedoBoneKeyframeCommand__Keyframe *k = command->keyframes[i];
        releaseStateMessage(k->current_state);
        releaseStateMessage(k->last_state);
        nanoem_delete(k);
    }
    delete[] command->keyframes;
    nanoem_delete(command);
}

void
BaseBoneKeyframeCommand::readStateMessage(const void *opaque, BoneKeyframe::State &s)
{
    const Nanoem__Application__RedoBoneKeyframeCommand__State *state =
        static_cast<const Nanoem__Application__RedoBoneKeyframeCommand__State *>(opaque);
    CommandMessageUtil::getVector(state->translation, s.m_translation);
    CommandMessageUtil::getQuaternion(state->orientation, s.m_orientation);
    CommandMessageUtil::getInterpolation(
        state->interpolation->x, s.m_parameter.m_value[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X]);
    CommandMessageUtil::getInterpolation(
        state->interpolation->y, s.m_parameter.m_value[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y]);
    CommandMessageUtil::getInterpolation(
        state->interpolation->z, s.m_parameter.m_value[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z]);
    CommandMessageUtil::getInterpolation(state->interpolation->orientation,
        s.m_parameter.m_value[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION]);
    s.m_enablePhysicsSimulation = state->is_physics_simulation_enabled != 0;
}

void
BaseBoneKeyframeCommand::writeStateMessage(const BoneKeyframe::State &s, void *opaque)
{
    Nanoem__Application__RedoBoneKeyframeCommand__State *
        *statePtr = static_cast<Nanoem__Application__RedoBoneKeyframeCommand__State **>(opaque),
       *state = nanoem_new(Nanoem__Application__RedoBoneKeyframeCommand__State);
    nanoem__application__redo_bone_keyframe_command__state__init(state);
    CommandMessageUtil::setVector(s.m_translation, state->translation);
    CommandMessageUtil::setQuaternion(s.m_orientation, state->orientation);
    state->interpolation = nanoem_new(Nanoem__Application__BoneInterpolation);
    nanoem__application__bone_interpolation__init(state->interpolation);
    CommandMessageUtil::setInterpolation(
        s.m_parameter.m_value[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X], state->interpolation->x);
    CommandMessageUtil::setInterpolation(
        s.m_parameter.m_value[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y], state->interpolation->y);
    CommandMessageUtil::setInterpolation(
        s.m_parameter.m_value[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z], state->interpolation->z);
    CommandMessageUtil::setInterpolation(
        s.m_parameter.m_value[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION],
        state->interpolation->orientation);
    state->is_physics_simulation_enabled = s.m_enablePhysicsSimulation;
    *statePtr = state;
}

void
BaseBoneKeyframeCommand::releaseStateMessage(void *opaque)
{
    Nanoem__Application__RedoBoneKeyframeCommand__State *statePtr =
        static_cast<Nanoem__Application__RedoBoneKeyframeCommand__State *>(opaque);
    CommandMessageUtil::releaseVector(statePtr->translation);
    CommandMessageUtil::releaseQuaternion(statePtr->orientation);
    CommandMessageUtil::releaseInterpolation(statePtr->interpolation->x);
    CommandMessageUtil::releaseInterpolation(statePtr->interpolation->y);
    CommandMessageUtil::releaseInterpolation(statePtr->interpolation->z);
    CommandMessageUtil::releaseInterpolation(statePtr->interpolation->orientation);
    nanoem_delete(statePtr->interpolation);
    nanoem_delete(statePtr);
}

} /* namespace command */
} /* namespace nanoem */
