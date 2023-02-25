/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/BaseModelKeyframeCommand.h"

#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#include "../CommandMessage.inl"

namespace nanoem {
namespace command {

BaseModelKeyframeCommand::~BaseModelKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

BaseModelKeyframeCommand::ModelKeyframe::State::State()
    : m_edge(Vector4(0), 1.0f)
    , m_enablePhysicsSimulation(true)
    , m_visible(true)
{
}

BaseModelKeyframeCommand::ModelKeyframe::State::~State() NANOEM_DECL_NOEXCEPT
{
}

BaseModelKeyframeCommand::ConstraintStateMap
BaseModelKeyframeCommand::ModelKeyframe::State::toCreateConstraintStateMapFromModel(const Model *model)
{
    nanoem_rsize_t numObjects;
    ConstraintStateMap constraintStateMap;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_bone_t *targetBone = bones[i];
        if (const nanoem_model_constraint_t *constraintPtr = model->findConstraint(targetBone)) {
            model::Constraint *constraint = model::Constraint::cast(constraintPtr);
            constraintStateMap.insert(tinystl::make_pair(targetBone, constraint->isEnabled()));
        }
    }
    return constraintStateMap;
}

StringPairMap
BaseModelKeyframeCommand::ModelKeyframe::State::toOutsideParentMapFromModel(const Model *model)
{
    StringPairMap outsideParentMap;
    const model::Bone::OutsideParentMap &allOutsideParents = model->allOutsideParents();
    for (model::Bone::OutsideParentMap::const_iterator it = allOutsideParents.begin(), end = allOutsideParents.end();
         it != end; ++it) {
        if (const model::Bone *bone = model::Bone::cast(it->first)) {
            outsideParentMap.insert(tinystl::make_pair(bone->name(), it->second));
        }
    }
    return outsideParentMap;
}

void
BaseModelKeyframeCommand::ModelKeyframe::State::assign(const Model *model)
{
    m_visible = model->isVisible();
    m_enablePhysicsSimulation = model->isPhysicsSimulationEnabled();
    m_edge.first = model->edgeColor();
    m_edge.second = model->edgeSizeScaleFactor();
    m_constraintStates = toCreateConstraintStateMapFromModel(model);
    m_outsideParents = toOutsideParentMapFromModel(model);
}

BaseModelKeyframeCommand::ConstraintStateMap
BaseModelKeyframeCommand::ModelKeyframe::State::toConstraintStateMapFromKeyframe(
    const nanoem_motion_model_keyframe_t *keyframe, const Model *model)
{
    nanoem_rsize_t numBones, numConstraintStates;
    nanoem_motion_model_keyframe_constraint_state_t *const *states =
        nanoemMotionModelKeyframeGetAllConstraintStateObjects(keyframe, &numConstraintStates);
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
    ConstraintStateMap constraintStateMap;
    for (nanoem_rsize_t i = 0; i < numConstraintStates; i++) {
        const nanoem_motion_model_keyframe_constraint_state_t *state = states[i];
        int boneId = nanoemMotionModelKeyframeConstraintStateGetBoneId(state);
        bool enabled = nanoemMotionModelKeyframeConstraintStateIsEnabled(state) != 0;
        if (boneId >= 0 && static_cast<nanoem_rsize_t>(boneId) < numBones) {
            const nanoem_model_bone_t *bone = bones[boneId];
            constraintStateMap.insert(tinystl::make_pair(bone, enabled));
        }
    }
    return constraintStateMap;
}

StringPairMap
BaseModelKeyframeCommand::ModelKeyframe::State::toOutsideParentMapFromKeyframe(
    const nanoem_motion_model_keyframe_t *keyframe, const Model *model)
{
    nanoem_rsize_t numOutsideParents;
    const Project *project = model->project();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    nanoem_motion_outside_parent_t *const *outsideParents =
        nanoemMotionModelKeyframeGetAllOutsideParentObjects(keyframe, &numOutsideParents);
    StringPairMap outsideParentMap;
    for (nanoem_rsize_t i = 0; i < numOutsideParents; i++) {
        const nanoem_motion_outside_parent_t *outsideParent = outsideParents[i];
        String baseBoneName, objectName, boneName;
        StringUtils::getUtf8String(nanoemMotionOutsideParentGetSubjectBoneName(outsideParent), factory, baseBoneName);
        StringUtils::getUtf8String(nanoemMotionOutsideParentGetTargetObjectName(outsideParent), factory, objectName);
        StringUtils::getUtf8String(nanoemMotionOutsideParentGetTargetBoneName(outsideParent), factory, boneName);
        if (model->findBone(baseBoneName)) {
            if (const Model *parentModel = project->findModelByName(objectName)) {
                if (parentModel->findBone(boneName)) {
                    outsideParentMap.insert(tinystl::make_pair(baseBoneName, tinystl::make_pair(objectName, boneName)));
                }
            }
        }
    }
    return outsideParentMap;
}

void
BaseModelKeyframeCommand::ModelKeyframe::State::assign(
    const nanoem_motion_model_keyframe_t *keyframe, const Model *model)
{
    m_visible = nanoemMotionModelKeyframeIsVisible(keyframe) != 0;
    m_enablePhysicsSimulation = nanoemMotionModelKeyframeIsPhysicsSimulationEnabled(keyframe) != 0;
    m_edge.first = glm::make_vec4(nanoemMotionModelKeyframeGetEdgeColor(keyframe));
    m_edge.second = nanoemMotionModelKeyframeGetEdgeScaleFactor(keyframe);
    m_constraintStates = toConstraintStateMapFromKeyframe(keyframe, model);
    m_outsideParents = toOutsideParentMapFromKeyframe(keyframe, model);
    const StringPairMap &additionalOutsideParents = toOutsideParentMapFromModel(model);
    for (StringPairMap::const_iterator it = additionalOutsideParents.begin(), end = additionalOutsideParents.end();
         it != end; ++it) {
        m_outsideParents[it->first] = it->second;
    }
}

BaseModelKeyframeCommand::BaseModelKeyframeCommand(
    const ModelKeyframeList &keyframes, const Model *model, Motion *motion)
    : BaseKeyframeCommand(motion)
    , m_model(model)
    , m_keyframes(keyframes)
{
    nanoem_assert(m_model, "must not be nullptr");
}

void
BaseModelKeyframeCommand::updateConstraintState(
    nanoem_mutable_motion_model_keyframe_t *mutableKeyframe, const ConstraintStateMap &value, nanoem_status_t &status)
{
    nanoem_rsize_t numItems;
    nanoem_motion_model_keyframe_constraint_state_t *const *items =
        nanoemMotionModelKeyframeGetAllConstraintStateObjects(
            nanoemMutableMotionModelKeyframeGetOriginObject(mutableKeyframe), &numItems);
    for (nanoem_rsize_t i = 0; i < numItems; i++) {
        nanoem_mutable_motion_model_keyframe_constraint_state_t *item =
            nanoemMutableMotionModelKeyframeConstraintStateCreateAsReference(items[i], &status);
        nanoemMutableMotionModelKeyframeRemoveConstraintState(mutableKeyframe, item, &status);
        nanoemMutableMotionModelKeyframeConstraintStateDestroy(item);
    }
    for (ConstraintStateMap::const_iterator it = value.begin(), end = value.end(); it != end; ++it) {
        nanoem_mutable_motion_model_keyframe_constraint_state_t *state =
            nanoemMutableMotionModelKeyframeConstraintStateCreateMutable(mutableKeyframe, &status);
        nanoemMutableMotionModelKeyframeConstraintStateSetBoneName(
            state, nanoemModelBoneGetName(it->first, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), &status);
        nanoemMutableMotionModelKeyframeConstraintStateSetEnabled(state, it->second);
        nanoemMutableMotionModelKeyframeAddConstraintState(mutableKeyframe, state, &status);
        nanoemMutableMotionModelKeyframeConstraintStateDestroy(state);
        if (status != NANOEM_STATUS_SUCCESS && status != NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_STATE_ALREADY_EXISTS) {
            break;
        }
    }
}

void
BaseModelKeyframeCommand::updateOutsideParent(
    nanoem_mutable_motion_model_keyframe_t *mutableKeyframe, const StringPairMap &value, nanoem_status_t &status)
{
    nanoem_unicode_string_factory_t *factory = m_model->project()->unicodeStringFactory();
    nanoem_motion_model_keyframe_t *keyframe = nanoemMutableMotionModelKeyframeGetOriginObject(mutableKeyframe);
    nanoem_rsize_t numOutsideParents;
    nanoem_motion_outside_parent_t *const *outsideParents =
        nanoemMotionModelKeyframeGetAllOutsideParentObjects(keyframe, &numOutsideParents);
    for (nanoem_rsize_t i = 0; i < numOutsideParents; i++) {
        nanoem_motion_outside_parent_t *reference = outsideParents[numOutsideParents - i - 1];
        nanoem_mutable_motion_outside_parent_t *outsideParent =
            nanoemMutableMotionOutsideParentCreateAsReference(reference, &status);
        nanoemMutableMotionModelKeyframeRemoveOutsideParent(mutableKeyframe, outsideParent, &status);
        nanoemMutableMotionOutsideParentDestroy(outsideParent);
    }
    StringUtils::UnicodeStringScope scope(factory);
    for (StringPairMap::const_iterator it = value.begin(), end = value.end(); it != end; ++it) {
        nanoem_mutable_motion_outside_parent_t *outsideParent =
            nanoemMutableMotionOutsideParentCreateFromModelKeyframe(keyframe, &status);
        if (StringUtils::tryGetString(factory, it->first, scope)) {
            nanoemMutableMotionOutsideParentSetSubjectBoneName(outsideParent, scope.value(), &status);
        }
        if (StringUtils::tryGetString(factory, it->second.first, scope)) {
            nanoemMutableMotionOutsideParentSetTargetObjectName(outsideParent, scope.value(), &status);
        }
        if (StringUtils::tryGetString(factory, it->second.second, scope)) {
            nanoemMutableMotionOutsideParentSetTargetBoneName(outsideParent, scope.value(), &status);
        }
        nanoemMutableMotionModelKeyframeAddOutsideParent(mutableKeyframe, outsideParent, &status);
        nanoemMutableMotionOutsideParentDestroy(outsideParent);
        if (status != NANOEM_STATUS_SUCCESS && status != NANOEM_STATUS_ERROR_MODEL_BINDING_ALREADY_EXISTS) {
            break;
        }
    }
}

void
BaseModelKeyframeCommand::restoreKeyframeState(
    nanoem_mutable_motion_model_keyframe_t *keyframe, const ModelKeyframe::State &state)
{
    nanoem_assert(keyframe, "must NOT be nullptr");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableMotionModelKeyframeSetVisible(keyframe, state.m_visible);
    nanoemMutableMotionModelKeyframeSetPhysicsSimulationEnabled(keyframe, state.m_enablePhysicsSimulation);
    nanoemMutableMotionModelKeyframeSetEdgeColor(keyframe, glm::value_ptr(state.m_edge.first));
    nanoemMutableMotionModelKeyframeSetEdgeScaleFactor(keyframe, state.m_edge.second);
    updateConstraintState(keyframe, state.m_constraintStates, status);
    updateOutsideParent(keyframe, state.m_outsideParents, status);
}

void
BaseModelKeyframeCommand::addKeyframe(Error &error)
{
    if (currentProject()->containsMotion(m_motion)) {
        IMotionKeyframeSelection *selection = m_motion->selection();
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_motion_t *motion = m_motion->data();
        nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion, &status);
        resetTransformPerformedAt();
        for (ModelKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
            ModelKeyframe &keyframe = *it;
            nanoem_mutable_motion_model_keyframe_t *mutableKeyframe;
            if (keyframe.m_updated) {
                mutableKeyframe = nanoemMutableMotionModelKeyframeCreateByFound(motion, keyframe.m_frameIndex, &status);
            }
            else {
                mutableKeyframe = nanoemMutableMotionModelKeyframeCreate(motion, &status);
                nanoemMutableMotionAddModelKeyframe(mutableMotion, mutableKeyframe, keyframe.m_frameIndex, &status);
                if (keyframe.m_selected) {
                    selection->add(nanoemMutableMotionModelKeyframeGetOriginObject(mutableKeyframe));
                }
            }
            restoreKeyframeState(mutableKeyframe, keyframe.m_state.first);
            nanoemMutableMotionModelKeyframeDestroy(mutableKeyframe);
            if (status != NANOEM_STATUS_SUCCESS && status != NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_ALREADY_EXISTS) {
                break;
            }
        }
        commit(mutableMotion);
        nanoemMutableMotionDestroy(mutableMotion);
        assignError(status, error);
    }
}

void
BaseModelKeyframeCommand::removeKeyframe(Error &error)
{
    if (currentProject()->containsMotion(m_motion)) {
        IMotionKeyframeSelection *selection = m_motion->selection();
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_motion_t *motion = m_motion->data();
        nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion, &status);
        resetTransformPerformedAt();
        for (ModelKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
            ModelKeyframe &keyframe = *it;
            nanoem_mutable_motion_model_keyframe_t *mutableKeyframe =
                nanoemMutableMotionModelKeyframeCreateByFound(motion, keyframe.m_frameIndex, &status);
            if (keyframe.m_updated) {
                restoreKeyframeState(mutableKeyframe, keyframe.m_state.second);
            }
            else {
                selection->remove(nanoemMutableMotionModelKeyframeGetOriginObject(mutableKeyframe));
                nanoemMutableMotionRemoveModelKeyframe(mutableMotion, mutableKeyframe, &status);
            }
            nanoemMutableMotionModelKeyframeDestroy(mutableKeyframe);
            if (status != NANOEM_STATUS_SUCCESS && status != NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_NOT_FOUND) {
                break;
            }
        }
        commit(mutableMotion);
        nanoemMutableMotionDestroy(mutableMotion);
        assignError(status, error);
    }
}

void
BaseModelKeyframeCommand::readMessage(const void *messagePtr, bool removing)
{
    const Nanoem__Application__RedoModelKeyframeCommand *commandPtr =
        static_cast<const Nanoem__Application__RedoModelKeyframeCommand *>(messagePtr);
    Model *model = currentProject()->resolveRedoModel(commandPtr->model_handle);
    if (model) {
        m_model = model;
        nanoem_rsize_t numKeyframes = commandPtr->n_keyframes;
        m_keyframes.resize(numKeyframes);
        for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
            const Nanoem__Application__RedoModelKeyframeCommand__Keyframe *k = commandPtr->keyframes[i];
            ModelKeyframe &keyframe = m_keyframes[i];
            readStateMessage(k->current_state, keyframe.m_state.first);
            readStateMessage(k->last_state, keyframe.m_state.second);
            keyframe.setFrameIndex(m_motion->data(), CommandMessageUtil::saturatedFrameIndex(k->frame_index), removing);
        }
    }
}

void
BaseModelKeyframeCommand::writeMessage(void *messagePtr, nanoem_u32_t type)
{
    Nanoem__Application__RedoModelKeyframeCommand__Keyframe **keyframes =
        new Nanoem__Application__RedoModelKeyframeCommand__Keyframe *[m_keyframes.size()];
    nanoem_rsize_t offset = 0;
    for (ModelKeyframeList::const_iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
        const ModelKeyframe &keyframe = *it;
        Nanoem__Application__RedoModelKeyframeCommand__Keyframe *k =
            nanoem_new(Nanoem__Application__RedoModelKeyframeCommand__Keyframe);
        nanoem__application__redo_model_keyframe_command__keyframe__init(k);
        k->frame_index = keyframe.m_frameIndex;
        writeStateMessage(keyframe.m_state.first, &k->current_state);
        writeStateMessage(keyframe.m_state.second, &k->last_state);
        keyframes[offset++] = k;
    }
    Nanoem__Application__RedoModelKeyframeCommand *command = nanoem_new(Nanoem__Application__RedoModelKeyframeCommand);
    nanoem__application__redo_model_keyframe_command__init(command);
    command->model_handle = m_model->handle();
    command->n_keyframes = m_keyframes.size();
    command->keyframes = keyframes;
    writeCommandMessage(command, type, messagePtr);
}

void
BaseModelKeyframeCommand::releaseMessage(void *messagePtr)
{
    Nanoem__Application__RedoModelKeyframeCommand *command =
        static_cast<Nanoem__Application__RedoModelKeyframeCommand *>(messagePtr);
    nanoem_rsize_t numKeyframes = command->n_keyframes;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        Nanoem__Application__RedoModelKeyframeCommand__Keyframe *k = command->keyframes[i];
        releaseStateMessage(k->current_state);
        releaseStateMessage(k->last_state);
        nanoem_delete(k);
    }
    delete[] command->keyframes;
    nanoem_delete(command);
}

void
BaseModelKeyframeCommand::readStateMessage(const void *opaque, ModelKeyframe::State &s)
{
    const Nanoem__Application__RedoModelKeyframeCommand__State *state =
        static_cast<const Nanoem__Application__RedoModelKeyframeCommand__State *>(opaque);
    CommandMessageUtil::getColor(state->edge_color, s.m_edge.first);
    s.m_edge.second = state->edge_size;
    s.m_enablePhysicsSimulation = state->is_physics_simulation_enabled != 0;
    s.m_visible = state->is_visible != 0;
    nanoem_rsize_t numConstraints = state->n_constraint_states;
    for (nanoem_rsize_t i = 0; i < numConstraints; i++) {
        const Nanoem__Application__RedoModelKeyframeCommand__ConstraintState *cs = state->constraint_states[i];
        if (const nanoem_model_bone_t *bonePtr = m_model->findRedoBone(cs->bone_index)) {
            s.m_constraintStates.insert(tinystl::make_pair(bonePtr, cs->enabled != 0));
        }
    }
    nanoem_rsize_t numOutsideParent = state->n_outside_parents;
    for (nanoem_rsize_t i = 0; i < numOutsideParent; i++) {
        const Nanoem__Application__OutsideParent *op = state->outside_parents[i];
        const nanoem_model_bone_t *bonePtr = m_model->findRedoBone(op->bone_index);
        const Model *parentModel = currentProject()->resolveRedoModel(op->parent_model_handle);
        if (bonePtr && parentModel) {
            if (const nanoem_model_bone_t *parentBonePtr = parentModel->findRedoBone(op->parent_model_bone_index)) {
                const model::Bone *bone = model::Bone::cast(bonePtr);
                const model::Bone *parentBone = model::Bone::cast(parentBonePtr);
                s.m_outsideParents.insert(
                    tinystl::make_pair(bone->name(), tinystl::make_pair(parentModel->name(), parentBone->name())));
            }
        }
    }
}

void
BaseModelKeyframeCommand::writeStateMessage(const ModelKeyframe::State &s, void *opaque)
{
    Nanoem__Application__RedoModelKeyframeCommand__State *
        *statePtr = static_cast<Nanoem__Application__RedoModelKeyframeCommand__State **>(opaque),
       *state = nanoem_new(Nanoem__Application__RedoModelKeyframeCommand__State);
    nanoem__application__redo_model_keyframe_command__state__init(state);
    CommandMessageUtil::setColor(s.m_edge.first, state->edge_color);
    state->edge_size = s.m_edge.second;
    state->is_physics_simulation_enabled = s.m_enablePhysicsSimulation;
    state->is_visible = s.m_visible;
    nanoem_rsize_t numConstraintStates = s.m_constraintStates.size();
    state->n_constraint_states = numConstraintStates;
    state->constraint_states =
        new Nanoem__Application__RedoModelKeyframeCommand__ConstraintState *[numConstraintStates];
    size_t offset = 0;
    for (ConstraintStateMap::const_iterator it = s.m_constraintStates.begin(), end = s.m_constraintStates.end();
         it != end; ++it) {
        Nanoem__Application__RedoModelKeyframeCommand__ConstraintState *cs = state->constraint_states[offset++] =
            nanoem_new(Nanoem__Application__RedoModelKeyframeCommand__ConstraintState);
        nanoem__application__redo_model_keyframe_command__constraint_state__init(cs);
        cs->bone_index = model::Bone::index(it->first);
        cs->enabled = it->second;
    }
    nanoem_rsize_t numOutsideParents = s.m_outsideParents.size();
    state->n_outside_parents = numOutsideParents;
    state->outside_parents = new Nanoem__Application__OutsideParent *[numOutsideParents];
    offset = 0;
    for (StringPairMap::const_iterator it = s.m_outsideParents.begin(), end = s.m_outsideParents.end(); it != end;
         ++it) {
        if (const nanoem_model_bone_t *bonePtr = m_model->findBone(it->first)) {
            CommandMessageUtil::setOutsideParent(
                bonePtr, it->second, currentProject(), state->outside_parents[offset++]);
        }
    }
    *statePtr = state;
}

void
BaseModelKeyframeCommand::releaseStateMessage(void *opaque)
{
    Nanoem__Application__RedoModelKeyframeCommand__State *statePtr =
        static_cast<Nanoem__Application__RedoModelKeyframeCommand__State *>(opaque);
    for (nanoem_rsize_t i = 0, numItems = statePtr->n_constraint_states; i < numItems; i++) {
        nanoem_delete(statePtr->constraint_states[i]);
    }
    delete[] statePtr->constraint_states;
    for (nanoem_rsize_t i = 0, numItems = statePtr->n_outside_parents; i < numItems; i++) {
        nanoem_delete(statePtr->outside_parents[i]);
    }
    delete[] statePtr->outside_parents;
    CommandMessageUtil::releaseColor(statePtr->edge_color);
    nanoem_delete(statePtr);
}

} /* namespace command */
} /* namespace nanoem */
