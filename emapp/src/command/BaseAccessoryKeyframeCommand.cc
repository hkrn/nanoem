/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/BaseAccessoryKeyframeCommand.h"

#include "emapp/Accessory.h"
#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"

namespace nanoem {
namespace command {

BaseAccessoryKeyframeCommand::~BaseAccessoryKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

BaseAccessoryKeyframeCommand::AccessoryKeyframe::State::State()
    : m_translation(Constants::kZeroV4)
    , m_orientation(Constants::kZeroQ)
    , m_scaleFactor(1.0f)
    , m_opacity(1.0f)
    , m_shadow(true)
    , m_blend(false)
    , m_visible(true)
{
}

BaseAccessoryKeyframeCommand::AccessoryKeyframe::State::~State()
{
}

void
BaseAccessoryKeyframeCommand::AccessoryKeyframe::State::assign(const Accessory *accessory)
{
    m_translation = Vector4(accessory->translation(), 1);
    m_orientation = accessory->orientationQuaternion();
    m_scaleFactor = accessory->scaleFactor();
    m_opacity = accessory->opacity();
    m_outsideParent = accessory->outsideParent();
    m_blend = accessory->isAddBlendEnabled();
    m_shadow = accessory->isShadowMapEnabled();
    m_visible = accessory->isVisible();
}

void
BaseAccessoryKeyframeCommand::AccessoryKeyframe::State::assign(
    const nanoem_motion_accessory_keyframe_t *keyframe, const Accessory *accessory)
{
    m_translation = glm::make_vec4(nanoemMotionAccessoryKeyframeGetTranslation(keyframe));
    m_orientation = glm::make_quat(nanoemMotionAccessoryKeyframeGetOrientation(keyframe));
    m_scaleFactor = nanoemMotionAccessoryKeyframeGetScaleFactor(keyframe);
    m_opacity = nanoemMotionAccessoryKeyframeGetOpacity(keyframe);
    m_shadow = nanoemMotionAccessoryKeyframeIsShadowEnabled(keyframe) != 0;
    m_blend = nanoemMotionAccessoryKeyframeIsAddBlendEnabled(keyframe) != 0;
    m_visible = nanoemMotionAccessoryKeyframeIsVisible(keyframe) != 0;
    const nanoem_motion_outside_parent_t *op = nanoemMotionAccessoryKeyframeGetOutsideParent(keyframe);
    const Project *project = accessory->project();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    nanoem_rsize_t length;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_outsideParent.first = m_outsideParent.second = String();
    if (nanoem_u8_t *objectName = nanoemUnicodeStringFactoryGetByteArrayEncoding(
            factory, nanoemMotionOutsideParentGetTargetObjectName(op), &length, NANOEM_CODEC_TYPE_UTF8, &status)) {
        if (const Model *model = project->findModelByName(reinterpret_cast<const char *>(objectName))) {
            m_outsideParent.first = model->canonicalName();
            if (model::Bone *bone =
                    model::Bone::cast(model->findBone(nanoemMotionOutsideParentGetTargetBoneName(op)))) {
                m_outsideParent.second = bone->canonicalName();
            }
        }
        nanoemUnicodeStringFactoryDestroyByteArray(factory, objectName);
    }
}

BaseAccessoryKeyframeCommand::BaseAccessoryKeyframeCommand(
    const AccessoryKeyframeList &keyframes, const Accessory *accessory, Motion *motion)
    : BaseKeyframeCommand(motion)
    , m_accessory(accessory)
    , m_keyframes(keyframes)
{
    nanoem_assert(m_accessory, "must not be nullptr");
}

void
BaseAccessoryKeyframeCommand::restoreKeyframeState(nanoem_mutable_motion_accessory_keyframe_t *keyframe,
    const AccessoryKeyframe::State &state, nanoem_status_t &status)
{
    nanoem_assert(keyframe, "must NOT be nullptr");
    nanoemMutableMotionAccessoryKeyframeSetTranslation(keyframe, glm::value_ptr(state.m_translation));
    nanoemMutableMotionAccessoryKeyframeSetOrientation(keyframe, glm::value_ptr(state.m_orientation));
    nanoemMutableMotionAccessoryKeyframeSetScaleFactor(keyframe, state.m_scaleFactor);
    nanoemMutableMotionAccessoryKeyframeSetOpacity(keyframe, state.m_opacity);
    nanoemMutableMotionAccessoryKeyframeSetShadowEnabled(keyframe, state.m_shadow);
    nanoemMutableMotionAccessoryKeyframeSetAddBlendEnabled(keyframe, state.m_blend);
    nanoemMutableMotionAccessoryKeyframeSetVisible(keyframe, state.m_visible);
    nanoem_mutable_motion_outside_parent_t *outsideParent = nanoemMutableMotionOutsideParentCreateFromAccessoryKeyframe(
        nanoemMutableMotionAccessoryKeyframeGetOriginObject(keyframe), &status);
    const Project *project = m_accessory->project();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    if (const Model *model = project->findModelByName(state.m_outsideParent.first)) {
        if (StringUtils::tryGetString(factory, model->canonicalName(), scope)) {
            nanoemMutableMotionOutsideParentSetTargetObjectName(outsideParent, scope.value(), &status);
        }
        const model::Bone *bone = model::Bone::cast(model->findBone(state.m_outsideParent.second));
        if (bone && StringUtils::tryGetString(factory, bone->canonicalName(), scope)) {
            nanoemMutableMotionOutsideParentSetTargetBoneName(outsideParent, scope.value(), &status);
        }
    }
    nanoemMutableMotionAccessoryKeyframeSetOutsideParent(keyframe, outsideParent, &status);
    nanoemMutableMotionOutsideParentDestroy(outsideParent);
}

void
BaseAccessoryKeyframeCommand::addKeyframe(Error &error)
{
    if (currentProject()->containsMotion(m_motion)) {
        IMotionKeyframeSelection *selection = m_motion->selection();
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_motion_t *motion = m_motion->data();
        nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion, &status);
        resetTransformPerformedAt();
        for (AccessoryKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
            AccessoryKeyframe &keyframe = *it;
            nanoem_mutable_motion_accessory_keyframe_t *mutableKeyframe;
            if (keyframe.m_updated) {
                mutableKeyframe =
                    nanoemMutableMotionAccessoryKeyframeCreateByFound(motion, keyframe.m_frameIndex, &status);
            }
            else {
                mutableKeyframe = nanoemMutableMotionAccessoryKeyframeCreate(motion, &status);
                nanoemMutableMotionAddAccessoryKeyframe(mutableMotion, mutableKeyframe, keyframe.m_frameIndex, &status);
                if (keyframe.m_selected) {
                    selection->add(nanoemMutableMotionAccessoryKeyframeGetOriginObject(mutableKeyframe));
                }
            }
            restoreKeyframeState(mutableKeyframe, keyframe.m_state.first, status);
            nanoemMutableMotionAccessoryKeyframeDestroy(mutableKeyframe);
            if (status != NANOEM_STATUS_SUCCESS &&
                status != NANOEM_STATUS_ERROR_MOTION_ACCESSORY_KEYFRAME_ALREADY_EXISTS) {
                break;
            }
        }
        commit(mutableMotion);
        nanoemMutableMotionDestroy(mutableMotion);
        assignError(status, error);
    }
}

void
BaseAccessoryKeyframeCommand::removeKeyframe(Error &error)
{
    if (currentProject()->containsMotion(m_motion)) {
        IMotionKeyframeSelection *selection = m_motion->selection();
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_motion_t *motion = m_motion->data();
        nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion, &status);
        resetTransformPerformedAt();
        for (AccessoryKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
            AccessoryKeyframe &keyframe = *it;
            nanoem_mutable_motion_accessory_keyframe_t *mutableKeyframe =
                nanoemMutableMotionAccessoryKeyframeCreateByFound(motion, keyframe.m_frameIndex, &status);
            if (keyframe.m_updated) {
                restoreKeyframeState(mutableKeyframe, keyframe.m_state.second, status);
            }
            else {
                selection->remove(nanoemMutableMotionAccessoryKeyframeGetOriginObject(mutableKeyframe));
                nanoemMutableMotionRemoveAccessoryKeyframe(mutableMotion, mutableKeyframe, &status);
            }
            nanoemMutableMotionAccessoryKeyframeDestroy(mutableKeyframe);
            if (status != NANOEM_STATUS_SUCCESS && status != NANOEM_STATUS_ERROR_MOTION_ACCESSORY_KEYFRAME_NOT_FOUND) {
                break;
            }
        }
        commit(mutableMotion);
        nanoemMutableMotionDestroy(mutableMotion);
        assignError(status, error);
    }
}

void
BaseAccessoryKeyframeCommand::readMessage(const void *messagePtr, bool removing)
{
    const Nanoem__Application__RedoAccessoryKeyframeCommand *commandPtr =
        static_cast<const Nanoem__Application__RedoAccessoryKeyframeCommand *>(messagePtr);
    Accessory *accessory = currentProject()->findAccessoryByHandle(commandPtr->accessory_handle);
    if (accessory) {
        m_accessory = accessory;
        nanoem_rsize_t numKeyframes = commandPtr->n_keyframes;
        m_keyframes.resize(numKeyframes);
        for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
            const Nanoem__Application__RedoAccessoryKeyframeCommand__Keyframe *k = commandPtr->keyframes[i];
            AccessoryKeyframe &keyframe = m_keyframes[i];
            readStateMessage(k->current_state, keyframe.m_state.first);
            readStateMessage(k->last_state, keyframe.m_state.second);
            keyframe.setFrameIndex(m_motion->data(), CommandMessageUtil::saturatedFrameIndex(k->frame_index), removing);
        }
    }
}

void
BaseAccessoryKeyframeCommand::writeMessage(void *messagPtr, nanoem_u32_t type)
{
    Nanoem__Application__RedoAccessoryKeyframeCommand__Keyframe **keyframes =
        new Nanoem__Application__RedoAccessoryKeyframeCommand__Keyframe *[m_keyframes.size()];
    nanoem_rsize_t offset = 0;
    for (AccessoryKeyframeList::const_iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
        const AccessoryKeyframe &keyframe = *it;
        Nanoem__Application__RedoAccessoryKeyframeCommand__Keyframe *k =
            nanoem_new(Nanoem__Application__RedoAccessoryKeyframeCommand__Keyframe);
        nanoem__application__redo_accessory_keyframe_command__keyframe__init(k);
        k->frame_index = keyframe.m_frameIndex;
        writeStateMessage(keyframe.m_state.first, &k->current_state);
        writeStateMessage(keyframe.m_state.second, &k->last_state);
        keyframes[offset++] = k;
    }
    Nanoem__Application__RedoAccessoryKeyframeCommand *command =
        nanoem_new(Nanoem__Application__RedoAccessoryKeyframeCommand);
    nanoem__application__redo_accessory_keyframe_command__init(command);
    command->accessory_handle = m_accessory->handle();
    command->n_keyframes = m_keyframes.size();
    command->keyframes = keyframes;
    writeCommandMessage(command, type, messagPtr);
}

void
BaseAccessoryKeyframeCommand::releaseMessage(void *messagePtr)
{
    Nanoem__Application__RedoAccessoryKeyframeCommand *command =
        static_cast<Nanoem__Application__RedoAccessoryKeyframeCommand *>(messagePtr);
    nanoem_rsize_t numKeyframes = command->n_keyframes;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        Nanoem__Application__RedoAccessoryKeyframeCommand__Keyframe *k = command->keyframes[i];
        releaseStateMessage(k->current_state);
        releaseStateMessage(k->last_state);
        nanoem_delete(k);
    }
    delete[] command->keyframes;
    nanoem_delete(command);
}

void
BaseAccessoryKeyframeCommand::readStateMessage(const void *opaque, AccessoryKeyframe::State &s)
{
    const Nanoem__Application__RedoAccessoryKeyframeCommand__State *state =
        static_cast<const Nanoem__Application__RedoAccessoryKeyframeCommand__State *>(opaque);
    CommandMessageUtil::getVector(state->translation, s.m_translation);
    CommandMessageUtil::getQuaternion(state->orientation, s.m_orientation);
    const Nanoem__Application__OutsideParent *op = state->outside_parent;
    if (op->has_parent_model_handle) {
        if (const Model *model = currentProject()->resolveRedoModel(op->parent_model_handle)) {
            s.m_outsideParent.first = model->canonicalName();
            if (op->has_parent_model_bone_index) {
                if (const nanoem_model_bone_t *bonePtr = model->findRedoBone(op->parent_model_bone_index)) {
                    const model::Bone *bone = model::Bone::cast(bonePtr);
                    s.m_outsideParent.second = bone->canonicalName();
                }
            }
        }
    }
    s.m_opacity = state->opacity;
    s.m_scaleFactor = state->scale_factor;
    s.m_blend = state->is_blend_enabled != 0;
    s.m_shadow = state->is_shadow_enabled != 0;
    s.m_visible = state->is_visible != 0;
}

void
BaseAccessoryKeyframeCommand::writeStateMessage(const AccessoryKeyframe::State &s, void *opaque)
{
    Nanoem__Application__RedoAccessoryKeyframeCommand__State *
        *statePtr = static_cast<Nanoem__Application__RedoAccessoryKeyframeCommand__State **>(opaque),
       *state = nanoem_new(Nanoem__Application__RedoAccessoryKeyframeCommand__State);
    nanoem__application__redo_accessory_keyframe_command__state__init(state);
    CommandMessageUtil::setVector(s.m_translation, state->translation);
    CommandMessageUtil::setQuaternion(s.m_orientation, state->orientation);
    CommandMessageUtil::setOutsideParent(s.m_outsideParent, currentProject(), state->outside_parent);
    state->opacity = s.m_opacity;
    state->scale_factor = s.m_scaleFactor;
    state->is_blend_enabled = s.m_blend;
    state->is_shadow_enabled = s.m_shadow;
    state->is_visible = s.m_visible;
    *statePtr = state;
}

void
BaseAccessoryKeyframeCommand::releaseStateMessage(void *opaque)
{
    Nanoem__Application__RedoAccessoryKeyframeCommand__State *statePtr =
        static_cast<Nanoem__Application__RedoAccessoryKeyframeCommand__State *>(opaque);
    CommandMessageUtil::releaseVector(statePtr->translation);
    CommandMessageUtil::releaseQuaternion(statePtr->orientation);
    CommandMessageUtil::releaseOutsideParent(statePtr->outside_parent);
    nanoem_delete(statePtr);
}

} /* namespace command */
} /* namespace nanoem */
