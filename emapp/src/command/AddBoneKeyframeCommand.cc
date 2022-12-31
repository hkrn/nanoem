/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/AddBoneKeyframeCommand.h"

#include "../CommandMessage.inl"
#include "../protoc/command.pb-c.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace command {

AddBoneKeyframeCommand::~AddBoneKeyframeCommand() NANOEM_DECL_NOEXCEPT
{
}

undo_command_t *
AddBoneKeyframeCommand::create(const void *messagePtr, const Model *model, Motion *motion)
{
    /* bone keyframe state is read from message so no need to feed bone list (to construct keyframe list) */
    AddBoneKeyframeCommand *command = nanoem_new(AddBoneKeyframeCommand(BoneList(), model, motion));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
AddBoneKeyframeCommand::create(const Motion::BoneFrameIndexSetMap &bones, const Model *model, Motion *motion)
{
    BoneList boneList;
    for (Motion::BoneFrameIndexSetMap::const_iterator it = bones.begin(), end = bones.end(); it != end; ++it) {
        const nanoem_model_bone_t *bone = it->first;
        const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        nanoem_motion_bone_keyframe_t *prevKeyframe = nullptr, *nextKeyframe = nullptr;
        for (Motion::FrameIndexSet::const_iterator it2 = it->second.begin(), end2 = it->second.end(); it2 != end2;
             ++it2) {
            const nanoem_frame_index_t &targetFrameIndex = *it2;
            nanoemMotionSearchClosestBoneKeyframes(
                motion->data(), name, targetFrameIndex, &prevKeyframe, &nextKeyframe);
            const nanoem_frame_index_t prevKeyframeIndex = prevKeyframe
                ? nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(prevKeyframe))
                : Motion::kMaxFrameIndex;
            const nanoem_frame_index_t nextKeyframeIndex = nextKeyframe
                ? nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(nextKeyframe))
                : Motion::kMaxFrameIndex;
            boneList.push_back(tinystl::make_pair(
                bone, Motion::KeyframeBound(prevKeyframeIndex, targetFrameIndex, nextKeyframeIndex)));
        }
    }
    AddBoneKeyframeCommand *command = nanoem_new(AddBoneKeyframeCommand(boneList, model, motion));
    return command->createCommand();
}

void
AddBoneKeyframeCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    removeKeyframe(error);
    nanoem_motion_t *m = m_motion ? m_motion->data() : nullptr;
    for (BoneKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
        BoneKeyframe &keyframe = *it;
        if (keyframe.m_bezierCurveOverrideTargetFrameIndex != Motion::kMaxFrameIndex) {
            nanoem_mutable_motion_bone_keyframe_t *ko = nanoemMutableMotionBoneKeyframeCreateByFound(
                m, keyframe.m_name, keyframe.m_bezierCurveOverrideTargetFrameIndex, &status);
            const BezierControlPointParameterTranslationOnly &p = keyframe.m_translationParameters.second;
            for (int i = 0; i < int(BX_COUNTOF(p.m_value)); i++) {
                nanoemMutableMotionBoneKeyframeSetInterpolation(
                    ko, nanoem_motion_bone_keyframe_interpolation_type_t(i), glm::value_ptr(p.m_value[i]));
            }
            nanoemMutableMotionBoneKeyframeDestroy(ko);
        }
    }
}

void
AddBoneKeyframeCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    addKeyframe(error);
    nanoem_motion_t *m = m_motion ? m_motion->data() : nullptr;
    for (BoneKeyframeList::iterator it = m_keyframes.begin(), end = m_keyframes.end(); it != end; ++it) {
        BoneKeyframe &keyframe = *it;
        if (keyframe.m_bezierCurveOverrideTargetFrameIndex != Motion::kMaxFrameIndex) {
            nanoem_mutable_motion_bone_keyframe_t *ko = nanoemMutableMotionBoneKeyframeCreateByFound(
                m, keyframe.m_name, keyframe.m_bezierCurveOverrideTargetFrameIndex, &status);
            const BezierControlPointParameterTranslationOnly &p = keyframe.m_translationParameters.first;
            for (int i = 0; i < int(BX_COUNTOF(p.m_value)); i++) {
                nanoemMutableMotionBoneKeyframeSetInterpolation(
                    ko, nanoem_motion_bone_keyframe_interpolation_type_t(i), glm::value_ptr(p.m_value[i]));
            }
            nanoemMutableMotionBoneKeyframeDestroy(ko);
        }
    }
}

const char *
AddBoneKeyframeCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "AddBoneKeyframeCommand";
}

BaseBoneKeyframeCommand::BoneKeyframeList
AddBoneKeyframeCommand::toKeyframeList(const BoneList &bones, const Model *model, const Motion *motion)
{
    BoneKeyframeList newKeyframes;
    const BoneList &newBones = motion ? bones : BoneList();
    const Project *project = model->project();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    const bool enableBezierCurveAdjustment = project->isBezierCurveAjustmentEnabled();
    const bool enablePhysicsSimulation = project->isPhysicsSimulationForBoneKeyframeEnabled();
    for (BoneList::const_iterator it = newBones.begin(), end = newBones.end(); it != end; ++it) {
        if (const nanoem_model_bone_t *bonePtr = it->first) {
            const Motion::KeyframeBound &bound = it->second;
            newKeyframes.push_back(
                toKeyframe(bonePtr, bound, motion, factory, enableBezierCurveAdjustment, enablePhysicsSimulation));
        }
    }
    return newKeyframes;
}

BaseBoneKeyframeCommand::BoneKeyframe
AddBoneKeyframeCommand::toKeyframe(const nanoem_model_bone_t *bonePtr, const Motion::KeyframeBound &bound,
    const Motion *motion, nanoem_unicode_string_factory_t *factory, bool enableBezierCurveAdjustment,
    bool enablePhysicsSimulation)
{
    const model::Bone *bone = model::Bone::cast(bonePtr);
    const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
    BoneKeyframe keyframe(bound.m_current);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    keyframe.m_name = nanoemUnicodeStringFactoryCloneString(factory, name, &status);
    BoneKeyframe::State &newState = keyframe.m_state.first;
    newState.assign(bone);
    newState.m_enablePhysicsSimulation = enablePhysicsSimulation;
    if (const nanoem_motion_bone_keyframe_t *ko = motion->findBoneKeyframe(name, keyframe.m_frameIndex)) {
        keyframe.m_state.second.assign(ko);
        keyframe.m_updated = true;
    }
    else {
        const nanoem_frame_index_t prevKeyframeIndex = bound.m_previous;
        if (prevKeyframeIndex != Motion::kMaxFrameIndex) {
            const nanoem_motion_bone_keyframe_t *prevKeyframe = motion->findBoneKeyframe(name, prevKeyframeIndex);
            const nanoem_frame_index_t nextKeyframeIndex = bound.m_next, currentKeyframeIndex = bound.m_current;
            BezierControlPointParameterTranslationOnlyPair &p = keyframe.m_translationParameters;
            bool movable = nanoemModelBoneIsMovable(bonePtr) != 0;
            for (nanoem_rsize_t i = 0; i < BX_COUNTOF(p.first.m_value); i++) {
                nanoem_motion_bone_keyframe_interpolation_type_t type =
                    nanoem_motion_bone_keyframe_interpolation_type_t(i);
                const Vector4U8 &prevKeyframeInterpolationParameter =
                    glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(prevKeyframe, type));
                newState.m_parameter.m_value[i] = bone->bezierControlPoints(type);
                if (enableBezierCurveAdjustment && movable) {
                    if (nanoemMotionBoneKeyframeIsLinearInterpolation(prevKeyframe, type)) {
                        p.first.m_value[i] = model::Bone::kDefaultAutomaticBezierControlPoint;
                    }
                    else if (nextKeyframeIndex != Motion::kMaxFrameIndex && nextKeyframeIndex > prevKeyframeIndex) {
                        const nanoem_frame_index_t interval = nextKeyframeIndex - prevKeyframeIndex;
                        const Vector2 c0(prevKeyframeInterpolationParameter.x, prevKeyframeInterpolationParameter.y),
                            c1(prevKeyframeInterpolationParameter.z, prevKeyframeInterpolationParameter.w);
                        const BezierCurve bezierCurve(c0, c1, interval);
                        BezierCurve::Pair pair(bezierCurve.split(
                            nanoem_f32_t((currentKeyframeIndex - prevKeyframeIndex) / nanoem_f64_t(interval))));
                        p.first.m_value[i] = pair.first->toParameters();
                        newState.m_parameter.m_value[i] = pair.second->toParameters();
                        nanoem_delete(pair.first);
                        nanoem_delete(pair.second);
                    }
                    else {
                        p.first.m_value[i] = prevKeyframeInterpolationParameter;
                    }
                }
                else {
                    p.first.m_value[i] = prevKeyframeInterpolationParameter;
                }
                p.second.m_value[i] = prevKeyframeInterpolationParameter;
            }
            keyframe.m_bezierCurveOverrideTargetFrameIndex = prevKeyframeIndex;
        }
        keyframe.m_selected = true;
    }
    return keyframe;
}

AddBoneKeyframeCommand::AddBoneKeyframeCommand(const BoneList &bones, const Model *model, Motion *motion)
    : BaseBoneKeyframeCommand(toKeyframeList(bones, model, motion), model, motion)
{
}

void
AddBoneKeyframeCommand::read(const void *messagePtr)
{
    readMessage(static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_add_bone_keyframe, false);
}

void
AddBoneKeyframeCommand::write(void *messagePtr)
{
    writeMessage(messagePtr, NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_BONE_KEYFRAME);
}

void
AddBoneKeyframeCommand::release(void *messagePtr)
{
    releaseMessage(static_cast<Nanoem__Application__Command *>(messagePtr)->redo_add_bone_keyframe);
}

} /* namespace command */
} /* namespace nanoem */
