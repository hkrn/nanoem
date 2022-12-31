/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/CommandRegistrator.h"

#include "emapp/Accessory.h"
#include "emapp/EnumUtils.h"
#include "emapp/Error.h"
#include "emapp/ICamera.h"
#include "emapp/ILight.h"
#include "emapp/IModelObjectSelection.h"
#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/Model.h"
#include "emapp/Progress.h"
#include "emapp/Project.h"
#include "emapp/ShadowCamera.h"
#include "emapp/StringUtils.h"
#include "emapp/command/AddAccessoryKeyframeCommand.h"
#include "emapp/command/AddBoneKeyframeCommand.h"
#include "emapp/command/AddCameraKeyframeCommand.h"
#include "emapp/command/AddLightKeyframeCommand.h"
#include "emapp/command/AddModelKeyframeCommand.h"
#include "emapp/command/AddMorphKeyframeCommand.h"
#include "emapp/command/AddSelfShadowKeyframeCommand.h"
#include "emapp/command/BatchUndoCommandListCommand.h"
#include "emapp/command/InsertEmptyTimelineFrameCommand.h"
#include "emapp/command/MotionSnapshotCommand.h"
#include "emapp/command/RemoveAccessoryKeyframeCommand.h"
#include "emapp/command/RemoveBoneKeyframeCommand.h"
#include "emapp/command/RemoveCameraKeyframeCommand.h"
#include "emapp/command/RemoveLightKeyframeCommand.h"
#include "emapp/command/RemoveModelKeyframeCommand.h"
#include "emapp/command/RemoveMorphKeyframeCommand.h"
#include "emapp/command/RemoveSelfShadowKeyframeCommand.h"
#include "emapp/command/RemoveTimelineFrameCommand.h"
#include "emapp/command/TransformBoneCommand.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {

CommandRegistrator::CommandRegistrator(Project *project)
    : m_project(project)
{
}

CommandRegistrator::~CommandRegistrator() NANOEM_DECL_NOEXCEPT
{
}

void
CommandRegistrator::registerAddAccessoryKeyframesCommand(
    const Motion::FrameIndexList &frameIndices, Accessory *accessory, Motion *motion)
{
    nanoem_parameter_assert(accessory, "must not be nullptr");
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    if (accessory && canRegisterMotionCommand()) {
        m_project->pushUndo(command::AddAccessoryKeyframeCommand::create(accessory, frameIndices, motion));
    }
}

void
CommandRegistrator::registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(Accessory *accessory)
{
    nanoem_assert(accessory, "must not be nullptr");
    Motion::FrameIndexList frameIndices;
    frameIndices.push_back(m_project->currentLocalFrameIndex());
    if (Motion *motion = m_project->resolveMotion(accessory)) {
        registerAddAccessoryKeyframesCommand(frameIndices, accessory, motion);
    }
}

void
CommandRegistrator::registerAddBoneKeyframesCommand(
    const Motion::BoneFrameIndexSetMap &bones, Model *model, Motion *motion)
{
    nanoem_parameter_assert(model, "must not be nullptr");
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    if (model && !bones.empty() && canRegisterMotionCommand()) {
        model->pushUndo(command::AddBoneKeyframeCommand::create(bones, model, motion));
    }
}

void
CommandRegistrator::registerAddBoneKeyframesCommandBySelectedBoneSet(Model *model)
{
    nanoem_parameter_assert(model, "must not be nullptr");
    const model::Bone::Set *selectedBoneSet = model->selection()->allBoneSet();
    const nanoem_frame_index_t frameIndex = m_project->currentLocalFrameIndex();
    Motion::BoneFrameIndexSetMap boneFrameIndexListMap;
    if (!selectedBoneSet->empty()) {
        for (model::Bone::Set::const_iterator it = selectedBoneSet->begin(), end = selectedBoneSet->end(); it != end;
             ++it) {
            const nanoem_model_bone_t *bone = *it;
            boneFrameIndexListMap[bone].insert(frameIndex);
        }
    }
    else if (const nanoem_model_bone_t *activeBone = model->activeBone()) {
        boneFrameIndexListMap[activeBone].insert(frameIndex);
    }
    Motion *motion = m_project->resolveMotion(model);
    if (motion && !boneFrameIndexListMap.empty()) {
        registerAddBoneKeyframesCommand(boneFrameIndexListMap, model, motion);
    }
}

void
CommandRegistrator::registerAddCameraKeyframesCommand(
    const Motion::FrameIndexList &frameIndices, ICamera *camera, Motion *motion)
{
    nanoem_parameter_assert(camera, "must not be nullptr");
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    if (canRegisterMotionCommand()) {
        m_project->pushUndo(command::AddCameraKeyframeCommand::create(camera, frameIndices, motion));
    }
}

void
CommandRegistrator::registerAddCameraKeyframesCommandByCurrentLocalFrameIndex()
{
    nanoem_assert(m_project->cameraMotion(), "must not be nullptr");
    Motion::FrameIndexList frameIndices;
    frameIndices.push_back(m_project->currentLocalFrameIndex());
    registerAddCameraKeyframesCommand(frameIndices, m_project->globalCamera(), m_project->cameraMotion());
}

void
CommandRegistrator::registerAddLightKeyframesCommand(
    const Motion::FrameIndexList &frameIndices, ILight *light, Motion *motion)
{
    nanoem_parameter_assert(light, "must not be nullptr");
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    if (canRegisterMotionCommand()) {
        m_project->pushUndo(command::AddLightKeyframeCommand::create(light, frameIndices, motion));
    }
}

void
CommandRegistrator::registerAddLightKeyframesCommandByCurrentLocalFrameIndex()
{
    Motion::FrameIndexList frameIndices;
    frameIndices.push_back(m_project->currentLocalFrameIndex());
    registerAddLightKeyframesCommand(frameIndices, m_project->globalLight(), m_project->lightMotion());
}

void
CommandRegistrator::registerAddModelKeyframesCommand(
    const Motion::FrameIndexList &frameIndices, Model *model, Motion *motion)
{
    nanoem_parameter_assert(model, "must not be nullptr");
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    if (model && !frameIndices.empty() && canRegisterMotionCommand()) {
        model->pushUndo(command::AddModelKeyframeCommand::create(model, frameIndices, motion));
    }
}

void
CommandRegistrator::registerAddModelKeyframesCommandByCurrentLocalFrameIndex()
{
    registerAddModelKeyframesCommandByCurrentLocalFrameIndex(m_project->activeModel());
}

void
CommandRegistrator::registerAddModelKeyframesCommandByCurrentLocalFrameIndex(Model *model)
{
    if (Motion *motion = m_project->resolveMotion(model)) {
        Motion::FrameIndexList frameIndices;
        frameIndices.push_back(m_project->currentLocalFrameIndex());
        registerAddModelKeyframesCommand(frameIndices, model, motion);
    }
}

void
CommandRegistrator::registerAddMorphKeyframesCommand(
    const Motion::MorphFrameIndexSetMap &morphs, Model *model, Motion *motion)
{
    nanoem_parameter_assert(model, "must not be nullptr");
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    if (model && !morphs.empty() && canRegisterMotionCommand()) {
        model->pushUndo(command::AddMorphKeyframeCommand::create(morphs, model, motion));
    }
}

void
CommandRegistrator::registerAddMorphKeyframesCommandByAllMorphs()
{
    registerAddMorphKeyframesCommandByAllMorphs(m_project->activeModel());
}

void
CommandRegistrator::registerAddMorphKeyframesCommandByAllMorphs(Model *model)
{
    nanoem_parameter_assert(model, "must not be nullptr");
    Motion *motion = m_project->resolveMotion(model);
    if (model && motion) {
        nanoem_rsize_t numMorphs;
        nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numMorphs);
        Motion::MorphFrameIndexSetMap morphFrameIndexListMap;
        const nanoem_frame_index_t frameIndex = m_project->currentLocalFrameIndex();
        for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
            const nanoem_model_morph_t *morph = morphs[i];
            const nanoem_model_morph_category_t category = nanoemModelMorphGetCategory(morph);
            if (category > NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM && category < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM) {
                morphFrameIndexListMap[morph].insert(frameIndex);
            }
        }
        registerAddMorphKeyframesCommand(morphFrameIndexListMap, model, motion);
    }
}

void
CommandRegistrator::registerAddMorphKeyframesCommandByActiveMorph(Model *model, nanoem_model_morph_category_t category)
{
    nanoem_parameter_assert(model, "must not be nullptr");
    Motion *motion = m_project->resolveMotion(model);
    if (model && motion) {
        const nanoem_model_morph_t *morphPtr = model->activeMorph(category);
        Motion::MorphFrameIndexSetMap morphFrameIndexListMap;
        Motion::FrameIndexSet frameIndices;
        frameIndices.insert(m_project->currentLocalFrameIndex());
        morphFrameIndexListMap.insert(tinystl::make_pair(morphPtr, frameIndices));
        registerAddMorphKeyframesCommand(morphFrameIndexListMap, model, motion);
    }
}

void
CommandRegistrator::registerAddSelfShadowKeyframesCommand(
    const Motion::FrameIndexList &frameIndices, ShadowCamera *shadowCamera, Motion *motion)
{
    nanoem_parameter_assert(m_project->selfShadowMotion(), "must not be nullptr");
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    if (canRegisterMotionCommand()) {
        m_project->pushUndo(command::AddSelfShadowKeyframeCommand::create(shadowCamera, frameIndices, motion));
    }
}

void
CommandRegistrator::registerAddSelfShadowKeyframesCommandByCurrentLocalFrameIndex()
{
    nanoem_assert(m_project->selfShadowMotion(), "must not be nullptr");
    Motion::FrameIndexList frameIndices;
    frameIndices.push_back(m_project->currentLocalFrameIndex());
    registerAddSelfShadowKeyframesCommand(frameIndices, m_project->shadowCamera(), m_project->selfShadowMotion());
}

void
CommandRegistrator::registerRemoveAccessoryKeyframesCommand(
    const Motion::FrameIndexList &frameIndices, Accessory *accessory, Motion *motion)
{
    nanoem_assert(!m_project->isPlaying(), "must not be called while playing");
    if (motion) {
        Motion::AccessoryKeyframeList keyframes;
        for (Motion::FrameIndexList::const_iterator it = frameIndices.begin(), end = frameIndices.end(); it != end;
             ++it) {
            const nanoem_frame_index_t frameIndex = *it;
            if (const nanoem_motion_accessory_keyframe_t *keyframe = motion->findAccessoryKeyframe(frameIndex)) {
                keyframes.push_back(keyframe);
            }
        }
        if (accessory && !keyframes.empty()) {
            m_project->pushUndo(command::RemoveAccessoryKeyframeCommand::create(keyframes, accessory, motion));
        }
    }
}

void
CommandRegistrator::registerRemoveAccessoryKeyframesCommandByCurrentLocalFrameIndex(Accessory *accessory)
{
    nanoem_assert(accessory, "must not be nullptr");
    nanoem_assert(!m_project->isPlaying(), "must not be called while playing");
    Motion::FrameIndexList frameIndices;
    frameIndices.push_back(m_project->currentLocalFrameIndex());
    if (Motion *motion = m_project->resolveMotion(accessory)) {
        registerRemoveAccessoryKeyframesCommand(frameIndices, accessory, motion);
    }
}

void
CommandRegistrator::registerRemoveBoneKeyframesCommand(
    const Motion::BoneFrameIndexSetMap &bones, Model *model, Motion *motion)
{
    nanoem_assert(!m_project->isPlaying(), "must not be called while playing");
    if (motion) {
        Motion::BoneKeyframeList keyframes;
        for (Motion::BoneFrameIndexSetMap::const_iterator it = bones.begin(), end = bones.end(); it != end; ++it) {
            const nanoem_model_bone_t *bone = it->first;
            const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
            const Motion::FrameIndexSet &frameIndices = it->second;
            for (Motion::FrameIndexSet::const_iterator it2 = frameIndices.begin(), end2 = frameIndices.end();
                 it2 != end2; ++it2) {
                const nanoem_frame_index_t frameIndex = *it2;
                if (const nanoem_motion_bone_keyframe_t *keyframe = motion->findBoneKeyframe(name, frameIndex)) {
                    keyframes.push_back(keyframe);
                }
            }
        }
        if (model && !keyframes.empty()) {
            model->pushUndo(command::RemoveBoneKeyframeCommand::create(keyframes, model, motion));
        }
    }
}

void
CommandRegistrator::registerRemoveBoneKeyframesCommandByCurrentLocalFrameIndex(Model *model)
{
    nanoem_parameter_assert(model, "must not be nullptr");
    nanoem_assert(!m_project->isPlaying(), "must not be called while playing");
    Motion *motion = m_project->resolveMotion(model);
    if (model && motion) {
        const model::Bone::Set *boneSet = model->selection()->allBoneSet();
        const nanoem_frame_index_t frameIndex = m_project->currentLocalFrameIndex();
        Motion::BoneFrameIndexSetMap boneFrameIndexListMap;
        for (model::Bone::Set::const_iterator it = boneSet->begin(), end = boneSet->end(); it != end; ++it) {
            const nanoem_model_bone_t *bone = *it;
            boneFrameIndexListMap[bone].insert(frameIndex);
        }
        if (!boneFrameIndexListMap.empty()) {
            registerRemoveBoneKeyframesCommand(boneFrameIndexListMap, model, motion);
        }
    }
}

void
CommandRegistrator::registerRemoveCameraKeyframesCommand(
    const Motion::FrameIndexList &frameIndices, ICamera *camera, Motion *motion)
{
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    if (motion && canRegisterMotionCommand()) {
        Motion::CameraKeyframeList keyframes;
        for (Motion::FrameIndexList::const_iterator it = frameIndices.begin(), end = frameIndices.end(); it != end;
             ++it) {
            const nanoem_frame_index_t frameIndex = *it;
            if (const nanoem_motion_camera_keyframe_t *keyframe = motion->findCameraKeyframe(frameIndex)) {
                keyframes.push_back(keyframe);
            }
        }
        if (camera && !keyframes.empty()) {
            m_project->pushUndo(command::RemoveCameraKeyframeCommand::create(keyframes, camera, motion));
        }
    }
}

void
CommandRegistrator::registerRemoveCameraKeyframesCommandByCurrentLocalFrameIndex()
{
    nanoem_assert(m_project->cameraMotion(), "must not be nullptr");
    Motion::FrameIndexList frameIndices;
    frameIndices.push_back(m_project->currentLocalFrameIndex());
    registerRemoveCameraKeyframesCommand(frameIndices, m_project->globalCamera(), m_project->cameraMotion());
}

void
CommandRegistrator::registerRemoveLightKeyframesCommand(
    const Motion::FrameIndexList &frameIndices, ILight *light, Motion *motion)
{
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    if (motion && canRegisterMotionCommand()) {
        Motion::LightKeyframeList keyframes;
        for (Motion::FrameIndexList::const_iterator it = frameIndices.begin(), end = frameIndices.end(); it != end;
             ++it) {
            const nanoem_frame_index_t frameIndex = *it;
            if (const nanoem_motion_light_keyframe_t *keyframe = motion->findLightKeyframe(frameIndex)) {
                keyframes.push_back(keyframe);
            }
        }
        if (light && !keyframes.empty()) {
            m_project->pushUndo(command::RemoveLightKeyframeCommand::create(keyframes, light, motion));
        }
    }
}

void
CommandRegistrator::registerRemoveLightKeyframesCommandByCurrentLocalFrameIndex()
{
    Motion::FrameIndexList frameIndices;
    frameIndices.push_back(m_project->currentLocalFrameIndex());
    registerRemoveLightKeyframesCommand(frameIndices, m_project->globalLight(), m_project->lightMotion());
}

void
CommandRegistrator::registerRemoveModelKeyframesCommand(
    const Motion::FrameIndexList &frameIndices, Model *model, Motion *motion)
{
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    if (model && motion && canRegisterMotionCommand()) {
        Motion::ModelKeyframeList keyframes;
        for (Motion::FrameIndexList::const_iterator it = frameIndices.begin(), end = frameIndices.end(); it != end;
             ++it) {
            const nanoem_frame_index_t frameIndex = *it;
            if (const nanoem_motion_model_keyframe_t *keyframe = motion->findModelKeyframe(frameIndex)) {
                keyframes.push_back(keyframe);
            }
        }
        if (!keyframes.empty()) {
            model->pushUndo(command::RemoveModelKeyframeCommand::create(keyframes, model, motion));
        }
    }
}

void
CommandRegistrator::registerRemoveModelKeyframesCommandByCurrentLocalFrameIndex()
{
    registerRemoveModelKeyframesCommandByCurrentLocalFrameIndex(m_project->activeModel());
}

void
CommandRegistrator::registerRemoveModelKeyframesCommandByCurrentLocalFrameIndex(Model *model)
{
    if (Motion *motion = m_project->resolveMotion(model)) {
        Motion::FrameIndexList frameIndices;
        frameIndices.push_back(m_project->currentLocalFrameIndex());
        registerRemoveModelKeyframesCommand(frameIndices, model, motion);
    }
}

void
CommandRegistrator::registerRemoveMorphKeyframesCommand(
    const Motion::MorphFrameIndexSetMap &morphs, Model *model, Motion *motion)
{
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    if (model && motion && canRegisterMotionCommand()) {
        Motion::MorphKeyframeList keyframes;
        for (Motion::MorphFrameIndexSetMap::const_iterator it = morphs.begin(), end = morphs.end(); it != end; ++it) {
            const nanoem_model_morph_t *morph = it->first;
            const nanoem_unicode_string_t *name = nanoemModelMorphGetName(morph, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
            const Motion::FrameIndexSet &frameIndices = it->second;
            for (Motion::FrameIndexSet::const_iterator it2 = frameIndices.begin(), end2 = frameIndices.end();
                 it2 != end2; ++it2) {
                const nanoem_frame_index_t frameIndex = *it2;
                if (const nanoem_motion_morph_keyframe_t *keyframe = motion->findMorphKeyframe(name, frameIndex)) {
                    keyframes.push_back(keyframe);
                }
            }
        }
        if (!keyframes.empty()) {
            model->pushUndo(command::RemoveMorphKeyframeCommand::create(keyframes, model, motion));
        }
    }
}

void
CommandRegistrator::registerRemoveMorphKeyframesCommandByCurrentLocalFrameIndex(Model *model)
{
    nanoem_parameter_assert(model, "must not be nullptr");
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    Motion *motion = m_project->resolveMotion(model);
    if (model && motion && canRegisterMotionCommand()) {
        nanoem_rsize_t numMorphs;
        nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numMorphs);
        Motion::MorphFrameIndexSetMap morphFrameIndexListMap;
        const nanoem_frame_index_t frameIndex = m_project->currentLocalFrameIndex();
        for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
            const nanoem_model_morph_t *morph = morphs[i];
            const nanoem_model_morph_category_t category = nanoemModelMorphGetCategory(morph);
            if (category > NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM && category < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM) {
                morphFrameIndexListMap[morph].insert(frameIndex);
            }
        }
        if (!morphFrameIndexListMap.empty()) {
            registerRemoveMorphKeyframesCommand(morphFrameIndexListMap, model, motion);
        }
    }
}

void
CommandRegistrator::registerRemoveSelfShadowKeyframesCommand(
    const Motion::FrameIndexList &frameIndices, ShadowCamera *shadowCamera, Motion *motion)
{
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    if (shadowCamera && motion && canRegisterMotionCommand()) {
        Motion::SelfShadowKeyframeList keyframes;
        for (Motion::FrameIndexList::const_iterator it = frameIndices.begin(), end = frameIndices.end(); it != end;
             ++it) {
            const nanoem_frame_index_t frameIndex = *it;
            if (const nanoem_motion_self_shadow_keyframe_t *keyframe = motion->findSelfShadowKeyframe(frameIndex)) {
                keyframes.push_back(keyframe);
            }
        }
        if (!keyframes.empty()) {
            m_project->pushUndo(command::RemoveSelfShadowKeyframeCommand::create(
                keyframes, shadowCamera, m_project->selfShadowMotion()));
        }
    }
}

void
CommandRegistrator::registerRemoveSelfShadowKeyframesCommandByCurrentLocalFrameIndex()
{
    Motion::FrameIndexList frameIndices;
    frameIndices.push_back(m_project->currentLocalFrameIndex());
    registerRemoveSelfShadowKeyframesCommand(frameIndices, m_project->shadowCamera(), m_project->selfShadowMotion());
}

void
CommandRegistrator::registerInsertEmptyTimelineFrameCommand()
{
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    nanoem_frame_index_t frameIndex = m_project->currentLocalFrameIndex();
    if (frameIndex > 0 && canRegisterMotionCommand()) {
        command::BatchUndoCommandListCommand::UndoCommandList commands;
        if (Model *model = m_project->activeModel()) {
            if (Motion *motion = m_project->resolveMotion(model)) {
                commands.push_back(command::InsertEmptyTimelineFrameCommand::create(
                    motion, frameIndex, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE));
                commands.push_back(command::InsertEmptyTimelineFrameCommand::create(
                    motion, frameIndex, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL));
                commands.push_back(command::InsertEmptyTimelineFrameCommand::create(
                    motion, frameIndex, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH));
                model->pushUndo(command::BatchUndoCommandListCommand::create(commands, model, m_project));
            }
        }
        else {
            const Project::AccessoryList *accessories = m_project->allAccessories();
            for (Project::AccessoryList::const_iterator it = accessories->begin(), end = accessories->end(); it != end;
                 ++it) {
                if (Motion *motion = m_project->resolveMotion(*it)) {
                    commands.push_back(command::InsertEmptyTimelineFrameCommand::create(
                        motion, frameIndex, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY));
                }
            }
            commands.push_back(command::InsertEmptyTimelineFrameCommand::create(
                m_project->cameraMotion(), frameIndex, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA));
            commands.push_back(command::InsertEmptyTimelineFrameCommand::create(
                m_project->lightMotion(), frameIndex, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT));
            commands.push_back(command::InsertEmptyTimelineFrameCommand::create(
                m_project->selfShadowMotion(), frameIndex, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW));
            m_project->pushUndo(command::BatchUndoCommandListCommand::create(commands, nullptr, m_project));
        }
    }
}

void
CommandRegistrator::registerMoveAllSelectedKeyframesCommand(int delta, Error &error)
{
    nanoem_parameter_assert(delta != 0, "must not be zero");
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    command::BatchUndoCommandListCommand::UndoCommandList commands;
    if (delta != 0 && canRegisterMotionCommand() && internalMoveAllSelectedKeyframes(delta, commands, error) &&
        !commands.empty()) {
        m_project->pushUndo(command::BatchUndoCommandListCommand::create(commands, nullptr, m_project));
    }
}

void
CommandRegistrator::registerRemoveTimelineFrameCommand()
{
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    if (canRegisterMotionCommand()) {
        nanoem_frame_index_t frameIndex = m_project->currentLocalFrameIndex();
        command::BatchUndoCommandListCommand::UndoCommandList commands;
        if (Model *model = m_project->activeModel()) {
            if (Motion *motion = m_project->resolveMotion(model)) {
                commands.push_back(command::RemoveTimelineFrameCommand::create(
                    motion, frameIndex, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE));
                commands.push_back(command::RemoveTimelineFrameCommand::create(
                    motion, frameIndex, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL));
                commands.push_back(command::RemoveTimelineFrameCommand::create(
                    motion, frameIndex, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH));
                model->pushUndo(command::BatchUndoCommandListCommand::create(commands, model, m_project));
            }
        }
        else {
            const Project::AccessoryList *accessories = m_project->allAccessories();
            for (Project::AccessoryList::const_iterator it = accessories->begin(), end = accessories->end(); it != end;
                 ++it) {
                if (Motion *motion = m_project->resolveMotion(*it)) {
                    commands.push_back(command::RemoveTimelineFrameCommand::create(
                        motion, frameIndex, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY));
                }
            }
            commands.push_back(command::RemoveTimelineFrameCommand::create(
                m_project->cameraMotion(), frameIndex, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA));
            commands.push_back(command::RemoveTimelineFrameCommand::create(
                m_project->lightMotion(), frameIndex, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT));
            commands.push_back(command::RemoveTimelineFrameCommand::create(
                m_project->selfShadowMotion(), frameIndex, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW));
            m_project->pushUndo(command::BatchUndoCommandListCommand::create(commands, nullptr, m_project));
        }
    }
}

void
CommandRegistrator::registerCorrectAllSelectedBoneKeyframesCommand(Model *model,
    const Motion::CorrectionVectorFactor &translation, const Motion::CorrectionVectorFactor &orientation, Error &error)
{
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    if (canRegisterMotionCommand()) {
        if (Motion *motion = m_project->resolveMotion(model)) {
            ByteArray snapshot;
            motion->save(snapshot, model, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
            motion->correctAllSelectedBoneKeyframes(translation, orientation);
            model->pushUndo(command::MotionSnapshotCommand::create(
                motion, model, snapshot, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE));
        }
    }
}

void
CommandRegistrator::registerCorrectAllSelectedCameraKeyframesCommand(const Motion::CorrectionVectorFactor &lookAt,
    const Motion::CorrectionVectorFactor &angle, const Motion::CorrectionScalarFactor &distance, Error &error)
{
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    if (canRegisterMotionCommand()) {
        Motion *motion = m_project->cameraMotion();
        ByteArray snapshot;
        motion->save(snapshot, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
        motion->correctAllSelectedCameraKeyframes(lookAt, angle, distance);
        m_project->pushUndo(command::MotionSnapshotCommand::create(
            motion, nullptr, snapshot, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA));
    }
}

void
CommandRegistrator::registerCorrectAllSelectedMorphKeyframesCommand(
    Model *model, const Motion::CorrectionScalarFactor &weight, Error &error)
{
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    Motion *motion = m_project->resolveMotion(model);
    if (canRegisterMotionCommand() && motion) {
        ByteArray snapshot;
        motion->save(snapshot, model, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
        motion->correctAllSelectedMorphKeyframes(weight);
        model->pushUndo(
            command::MotionSnapshotCommand::create(motion, model, snapshot, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH));
    }
}

void
CommandRegistrator::registerScaleAllMotionKeyframesInCommand(
    const TimelineSegment &range, nanoem_f32_t scaleFactor, nanoem_u32_t flags, Error &error)
{
    command::BatchUndoCommandListCommand::UndoCommandList commands;
    Motion::SelfShadowKeyframeList lastSelectedSelfShadowKeyframes;
    Model *model = m_project->activeModel();
    if (!canRegisterMotionCommand() || range.m_from == range.m_to) {
        return;
    }
    if (scaleFactor < 1.0f || internalMoveAllSelectedKeyframes(range.m_to - range.m_from, commands, error)) {
        ByteArray snapshot;
        if (Motion *modelMotionPtr = m_project->resolveMotion(model)) {
            modelMotionPtr->save(snapshot, model, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
            if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE)) {
                modelMotionPtr->scaleAllBoneKeyframesIn(model, range.m_from, range.m_to, scaleFactor);
            }
            if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL)) {
                modelMotionPtr->scaleAllModelKeyframesIn(range.m_from, range.m_to, scaleFactor);
            }
            if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH)) {
                modelMotionPtr->scaleAllMorphKeyframesIn(model, range.m_from, range.m_to, scaleFactor);
            }
            commands.push_back(command::MotionSnapshotCommand::create(modelMotionPtr, model, snapshot, flags));
        }
        else {
            if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT)) {
                Motion *lightMotionPtr = m_project->lightMotion();
                lightMotionPtr->save(snapshot, 0, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
                lightMotionPtr->scaleAllLightKeyframesIn(range.m_from, range.m_to, scaleFactor);
                commands.push_back(command::MotionSnapshotCommand::create(
                    lightMotionPtr, 0, snapshot, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT));
            }
            if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW)) {
                Motion *selfShadowMotionPtr = m_project->selfShadowMotion();
                selfShadowMotionPtr->save(snapshot, 0, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
                selfShadowMotionPtr->scaleAllSelfShadowKeyframesIn(range.m_from, range.m_to, scaleFactor);
                commands.push_back(command::MotionSnapshotCommand::create(
                    selfShadowMotionPtr, 0, snapshot, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW));
            }
            if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY)) {
                Accessory *accessory = m_project->activeAccessory();
                if (Motion *accessoryMotionPtr = m_project->resolveMotion(accessory)) {
                    accessoryMotionPtr->save(snapshot, 0, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
                    accessoryMotionPtr->scaleAllAccessoryKeyframesIn(range.m_from, range.m_to, scaleFactor);
                    commands.push_back(command::MotionSnapshotCommand::create(
                        accessoryMotionPtr, 0, snapshot, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY));
                }
            }
        }
        if ((scaleFactor > 1.0f || internalMoveAllSelectedKeyframes(range.m_to - range.m_from, commands, error)) &&
            !commands.empty()) {
            m_project->pushUndo(command::BatchUndoCommandListCommand::create(commands, nullptr, m_project));
        }
    }
}

void
CommandRegistrator::registerRemoveAllSelectedKeyframesCommand()
{
    registerRemoveAllSelectedKeyframesCommand(m_project->activeModel());
}

void
CommandRegistrator::registerRemoveAllSelectedKeyframesCommand(Model *model)
{
    command::BatchUndoCommandListCommand::UndoCommandList commands;
    if (Motion *motion = m_project->resolveMotion(model)) {
        Motion::BoneKeyframeList boneKeyframes;
        Motion::ModelKeyframeList modelKeyframes;
        Motion::MorphKeyframeList morphKeyframes;
        IMotionKeyframeSelection *selection = motion->selection();
        selection->getAll(boneKeyframes, nullptr);
        commands.push_back(command::RemoveBoneKeyframeCommand::create(boneKeyframes, model, motion));
        selection->getAll(modelKeyframes, nullptr);
        commands.push_back(command::RemoveModelKeyframeCommand::create(modelKeyframes, model, motion));
        selection->getAll(morphKeyframes, nullptr);
        commands.push_back(command::RemoveMorphKeyframeCommand::create(morphKeyframes, model, motion));
        selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
        model->pushUndo(command::BatchUndoCommandListCommand::create(commands, model, m_project));
    }
    else {
        {
            Motion::CameraKeyframeList cameraKeyframes;
            Motion *motion = m_project->cameraMotion();
            IMotionKeyframeSelection *selection = motion->selection();
            selection->getAll(cameraKeyframes, nullptr);
            commands.push_back(
                command::RemoveCameraKeyframeCommand::create(cameraKeyframes, m_project->globalCamera(), motion));
            selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
        }
        {
            Motion::LightKeyframeList lightKeyframes;
            Motion *motion = m_project->lightMotion();
            IMotionKeyframeSelection *selection = motion->selection();
            selection->getAll(lightKeyframes, nullptr);
            commands.push_back(
                command::RemoveLightKeyframeCommand::create(lightKeyframes, m_project->globalLight(), motion));
            selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
        }
        {
            Motion::SelfShadowKeyframeList selfShadowKeyframes;
            Motion *motion = m_project->selfShadowMotion();
            IMotionKeyframeSelection *selection = motion->selection();
            selection->getAll(selfShadowKeyframes, nullptr);
            commands.push_back(command::RemoveSelfShadowKeyframeCommand::create(
                selfShadowKeyframes, m_project->shadowCamera(), motion));
            selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
        }
        const Project::AccessoryList *accessories = m_project->allAccessories();
        for (Project::AccessoryList::const_iterator it = accessories->begin(), end = accessories->end(); it != end;
             ++it) {
            Accessory *accessory = *it;
            if (Motion *motion = m_project->resolveMotion(accessory)) {
                IMotionKeyframeSelection *selection = motion->selection();
                Motion::AccessoryKeyframeList accessoryKeyframes;
                selection->getAll(accessoryKeyframes, nullptr);
                commands.push_back(
                    command::RemoveAccessoryKeyframeCommand::create(accessoryKeyframes, accessory, motion));
                selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
            }
        }
        m_project->pushUndo(command::BatchUndoCommandListCommand::create(commands, nullptr, m_project));
    }
}

void
CommandRegistrator::registerRemoveAllLipMorphKeyframesCommand()
{
    registerRemoveAllLipMorphKeyframesCommand(m_project->activeModel());
}

void
CommandRegistrator::registerRemoveAllLipMorphKeyframesCommand(Model *model)
{
    internalRegisterRemoveAllMorphKeyframesCommand(model, NANOEM_MODEL_MORPH_CATEGORY_LIP);
}
void
CommandRegistrator::registerRemoveAllEyeMorphKeyframesCommand()
{
    registerRemoveAllEyeMorphKeyframesCommand(m_project->activeModel());
}

void
CommandRegistrator::registerRemoveAllEyeMorphKeyframesCommand(Model *model)
{
    internalRegisterRemoveAllMorphKeyframesCommand(model, NANOEM_MODEL_MORPH_CATEGORY_EYE);
}

void
CommandRegistrator::registerRemoveAllEyebrowMorphKeyframesCommand()
{
    registerRemoveAllEyebrowMorphKeyframesCommand(m_project->activeModel());
}

void
CommandRegistrator::registerRemoveAllEyebrowMorphKeyframesCommand(Model *model)
{
    internalRegisterRemoveAllMorphKeyframesCommand(model, NANOEM_MODEL_MORPH_CATEGORY_EYEBROW);
}

void
CommandRegistrator::registerInitializeMotionCommand(Error &error)
{
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    ByteArray snapshot;
    if (!canRegisterMotionCommand()) {
        return;
    }
    if (Model *model = m_project->activeModel()) {
        Motion *motion = m_project->resolveMotion(model);
        if (motion && motion->save(snapshot, model, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error)) {
            model->reset();
            motion->clearAllKeyframes();
            motion->initialize(model);
            model->pushUndo(command::MotionSnapshotCommand::create(
                motion, model, snapshot, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL));
            m_project->seek(m_project->currentLocalFrameIndex(), true);
        }
    }
    else if (Accessory *accessory = m_project->activeAccessory()) {
        Motion *motion = m_project->resolveMotion(accessory);
        if (motion && motion->save(snapshot, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error)) {
            accessory->reset();
            motion->clearAllKeyframes();
            motion->initialize(accessory);
            m_project->pushUndo(command::MotionSnapshotCommand::create(
                motion, model, snapshot, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL));
            m_project->seek(m_project->currentLocalFrameIndex(), true);
        }
    }
    else {
        command::BatchUndoCommandListCommand::UndoCommandList commands;
        ICamera *globalCamera = m_project->globalCamera();
        Motion *cameraMotion = m_project->cameraMotion();
        if (cameraMotion->save(snapshot, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error)) {
            globalCamera->reset();
            cameraMotion->clearAllKeyframes();
            cameraMotion->initialize(globalCamera);
            commands.push_back(command::MotionSnapshotCommand::create(
                cameraMotion, nullptr, snapshot, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL));
        }
        ILight *globalLight = m_project->globalLight();
        Motion *lightMotion = m_project->lightMotion();
        if (lightMotion->save(snapshot, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error)) {
            globalLight->reset();
            lightMotion->clearAllKeyframes();
            lightMotion->initialize(globalLight);
            commands.push_back(command::MotionSnapshotCommand::create(
                lightMotion, nullptr, snapshot, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL));
        }
        ShadowCamera *shadowCamera = m_project->shadowCamera();
        Motion *selfShadowMotion = m_project->selfShadowMotion();
        if (selfShadowMotion->save(snapshot, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error)) {
            shadowCamera->reset();
            selfShadowMotion->clearAllKeyframes();
            selfShadowMotion->initialize(shadowCamera);
            commands.push_back(command::MotionSnapshotCommand::create(
                selfShadowMotion, nullptr, snapshot, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL));
        }
        m_project->pushUndo(command::BatchUndoCommandListCommand::create(commands, nullptr, m_project));
        m_project->seek(m_project->currentLocalFrameIndex(), 0);
    }
}

void
CommandRegistrator::registerBakeAllModelMotionsCommand(bool enableBakingConstraint, Error &error)
{
    typedef tinystl::unordered_map<const Model *, model::Bone::Set, TinySTLAllocator> AllModelConstraintBoneSet;
    typedef tinystl::unordered_map<const nanoem_model_bone_t *, const nanoem_model_bone_t *, TinySTLAllocator>
        ResolveInherentParentBoneMap;
    const nanoem_frame_index_t duration = m_project->duration(),
                               lastLocalFrameIndex = m_project->currentLocalFrameIndex();
    const PhysicsEngine::SimulationModeType lastSimulationMode = m_project->physicsEngine()->simulationMode();
    const ITranslator *translator = m_project->translator();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    AllModelConstraintBoneSet allConstraintBoneSets;
    const char *message = translator->translate("nanoem.dialog.progress.bake.message");
    char progressText[Inline::kLongNameStackBufferSize];
    Progress progress(m_project, translator->translate("nanoem.dialog.progress.bake.title"), message, duration);
    Model *lastActiveModel = m_project->activeModel();
    m_project->seek(0, true);
    m_project->setPhysicsSimulationMode(PhysicsEngine::kSimulationModeEnableTracing);
    const Project::ModelList *models = m_project->allModels();
    Project::MotionList motions;
    motions.reserve(models->size());
    for (Project::ModelList::const_iterator it = models->begin(), end = models->end(); it != end; ++it) {
        Model *model = *it;
        Motion *motion = m_project->createMotion();
        motion->initialize(model);
        motions.push_back(motion);
    }
    if (enableBakingConstraint) {
        ResolveInherentParentBoneMap resolveInherentParentBones;
        for (Project::ModelList::const_iterator it = models->begin(), end = models->end(); it != end; ++it) {
            Model *model = *it;
            nanoem_rsize_t numBones;
            nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
            model::Bone::Set &constraintBoneSet = allConstraintBoneSets[model];
            resolveInherentParentBones.clear();
            for (nanoem_rsize_t i = 0; i < numBones; i++) {
                const nanoem_model_bone_t *bonePtr = bones[i];
                if (const nanoem_model_bone_t *inherentParentBonePtr =
                        nanoemModelBoneGetInherentParentBoneObject(bonePtr)) {
                    while (const nanoem_model_bone_t *upperInherentParentBonePtr =
                               nanoemModelBoneGetInherentParentBoneObject(inherentParentBonePtr)) {
                        inherentParentBonePtr = upperInherentParentBonePtr;
                    }
                    resolveInherentParentBones.insert(tinystl::make_pair(inherentParentBonePtr, bonePtr));
                }
            }
            for (nanoem_rsize_t i = 0; i < numBones; i++) {
                const nanoem_model_bone_t *bonePtr = bones[i];
                if (nanoemModelBoneHasConstraint(bonePtr)) {
                    const nanoem_model_constraint_t *constraintPtr = nanoemModelBoneGetConstraintObject(bonePtr);
                    nanoem_rsize_t numJoints;
                    nanoem_model_constraint_joint_t *const *joints =
                        nanoemModelConstraintGetAllJointObjects(constraintPtr, &numJoints);
                    for (nanoem_rsize_t j = 0; j < numJoints; j++) {
                        const nanoem_model_constraint_joint_t *jointPtr = joints[j];
                        const nanoem_model_bone_t *jointBonePtr = nanoemModelConstraintJointGetBoneObject(jointPtr);
                        ResolveInherentParentBoneMap::const_iterator it = resolveInherentParentBones.find(jointBonePtr);
                        constraintBoneSet.insert(it != resolveInherentParentBones.end() ? it->second : jointBonePtr);
                    }
                    const nanoem_model_bone_t *effectorBonePtr =
                        nanoemModelConstraintGetEffectorBoneObject(constraintPtr);
                    ResolveInherentParentBoneMap::const_iterator it = resolveInherentParentBones.find(effectorBonePtr);
                    constraintBoneSet.insert(it != resolveInherentParentBones.end() ? it->second : effectorBonePtr);
                    constraintBoneSet.insert(bonePtr);
                }
            }
        }
    }
    model::Bone::Set constraintBoneSet;
    for (nanoem_frame_index_t frameIndex = 0; frameIndex <= duration;) {
        for (Project::ModelList::const_iterator it = models->begin(), end = models->end(); it != end; ++it) {
            Model *model = *it;
            nanoem_rsize_t offset = it - models->begin(), numBones, numConstraints, numMorphs;
            Motion *sourceMotion = m_project->resolveMotion(model), *destMotion = motions[offset];
            nanoem_motion_t *sourceMotionPtr = sourceMotion->data(), *dstMotionPtr = destMotion->data();
            nanoem_mutable_motion_t *m = nanoemMutableMotionCreateAsReference(dstMotionPtr, &status);
            nanoem_mutable_motion_model_keyframe_t *mk = nullptr;
            nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
            if (nanoem_mutable_motion_model_keyframe_t *src =
                    nanoemMutableMotionModelKeyframeCreateByFound(sourceMotionPtr, frameIndex, &status)) {
                nanoem_motion_model_keyframe_t *sk = nanoemMutableMotionModelKeyframeGetOriginObject(src);
                mk = frameIndex == 0 ? nanoemMutableMotionModelKeyframeCreateByFound(dstMotionPtr, 0, &status)
                                     : nanoemMutableMotionModelKeyframeCreate(dstMotionPtr, &status);
                nanoemMutableMotionModelKeyframeSetAddBlendEnabled(mk, nanoemMotionModelKeyframeIsAddBlendEnabled(sk));
                nanoemMutableMotionModelKeyframeSetEdgeColor(mk, nanoemMotionModelKeyframeGetEdgeColor(sk));
                nanoemMutableMotionModelKeyframeSetEdgeScaleFactor(mk, nanoemMotionModelKeyframeGetEdgeScaleFactor(sk));
                nanoemMutableMotionModelKeyframeSetVisible(mk, nanoemMotionModelKeyframeIsVisible(sk));
                nanoemMutableMotionModelKeyframeSetPhysicsSimulationEnabled(mk, 0);
                nanoemMutableMotionModelKeyframeDestroy(src);
            }
            AllModelConstraintBoneSet::const_iterator it2 = allConstraintBoneSets.find(model);
            if (it2 != allConstraintBoneSets.end()) {
                constraintBoneSet = it2->second;
            }
            else {
                constraintBoneSet.clear();
            }
            for (nanoem_rsize_t i = 0; i < numBones; i++) {
                const nanoem_model_bone_t *bonePtr = bones[i];
                int enableConstraint = enableBakingConstraint ? 0 : 1;
                if (const model::Bone *bone = model::Bone::cast(bonePtr)) {
                    const nanoem_unicode_string_t *name =
                        nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                    if (model->isRigidBodyBound(bonePtr)) {
                        const nanoem_unicode_string_t *name =
                            nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                        nanoem_mutable_motion_bone_keyframe_t *dst = frameIndex == 0
                            ? nanoemMutableMotionBoneKeyframeCreateByFound(dstMotionPtr, name, 0, &status)
                            : nanoemMutableMotionBoneKeyframeCreate(dstMotionPtr, &status);
                        const Vector4 translation(bone->localUserTranslation(), 1);
                        const Quaternion orientation(bone->localUserOrientation());
                        nanoemMutableMotionBoneKeyframeSetTranslation(dst, glm::value_ptr(translation));
                        nanoemMutableMotionBoneKeyframeSetOrientation(dst, glm::value_ptr(orientation));
                        nanoemMutableMotionBoneKeyframeSetPhysicsSimulationEnabled(dst, 0);
                        nanoemMutableMotionAddBoneKeyframe(m, dst, name, frameIndex, &status);
                        nanoemMutableMotionBoneKeyframeDestroy(dst);
                    }
                    else if (nanoem_mutable_motion_bone_keyframe_t *src = nanoemMutableMotionBoneKeyframeCreateByFound(
                                 sourceMotionPtr, name, frameIndex, &status)) {
                        nanoem_mutable_motion_bone_keyframe_t *dst = frameIndex == 0
                            ? nanoemMutableMotionBoneKeyframeCreateByFound(dstMotionPtr, name, 0, &status)
                            : nanoemMutableMotionBoneKeyframeCreate(dstMotionPtr, &status);
                        nanoemMutableMotionBoneKeyframeCopy(dst, nanoemMutableMotionBoneKeyframeGetOriginObject(src));
                        if (enableBakingConstraint && constraintBoneSet.find(bonePtr) != constraintBoneSet.end()) {
                            const Matrix4x4 transform(bone->localTransform());
                            nanoemMutableMotionBoneKeyframeSetTranslation(dst, glm::value_ptr(transform[3]));
                            nanoemMutableMotionBoneKeyframeSetOrientation(
                                dst, glm::value_ptr(glm::quat_cast(transform)));
                        }
                        nanoemMutableMotionBoneKeyframeSetPhysicsSimulationEnabled(dst, 0);
                        nanoemMutableMotionAddBoneKeyframe(m, dst, name, frameIndex, &status);
                        nanoemMutableMotionBoneKeyframeDestroy(dst);
                        nanoemMutableMotionBoneKeyframeDestroy(src);
                        if (nanoemModelBoneHasConstraint(bonePtr)) {
                            const nanoem_model_constraint_t *constraintPtr =
                                nanoemModelBoneGetConstraintObject(bonePtr);
                            nanoem_rsize_t numJoints;
                            nanoem_model_constraint_joint_t *const *joints =
                                nanoemModelConstraintGetAllJointObjects(constraintPtr, &numJoints);
                            for (nanoem_rsize_t j = 0; j < numJoints; j++) {
                                const nanoem_model_constraint_joint_t *jointPtr = joints[j];
                                if (model->isRigidBodyBound(nanoemModelConstraintJointGetBoneObject(jointPtr))) {
                                    enableConstraint = 0;
                                    break;
                                }
                            }
                        }
                    }
                    else if (enableBakingConstraint && constraintBoneSet.find(bonePtr) != constraintBoneSet.end()) {
                        const Matrix4x4 transform(bone->localTransform());
                        nanoem_mutable_motion_bone_keyframe_t *dst = frameIndex == 0
                            ? nanoemMutableMotionBoneKeyframeCreateByFound(dstMotionPtr, name, 0, &status)
                            : nanoemMutableMotionBoneKeyframeCreate(dstMotionPtr, &status);
                        nanoemMutableMotionBoneKeyframeSetTranslation(dst, glm::value_ptr(transform[3]));
                        nanoemMutableMotionBoneKeyframeSetOrientation(dst, glm::value_ptr(glm::quat_cast(transform)));
                        nanoemMutableMotionBoneKeyframeSetPhysicsSimulationEnabled(dst, 0);
                        nanoemMutableMotionAddBoneKeyframe(m, dst, name, frameIndex, &status);
                        nanoemMutableMotionBoneKeyframeDestroy(dst);
                    }
                    if (mk && nanoemModelBoneHasConstraint(bonePtr)) {
                        nanoem_mutable_motion_model_keyframe_constraint_state_t *state =
                            nanoemMutableMotionModelKeyframeConstraintStateCreate(
                                nanoemMutableMotionModelKeyframeGetOriginObject(mk), &status);
                        nanoemMutableMotionModelKeyframeConstraintStateSetBoneName(state, name, &status);
                        nanoemMutableMotionModelKeyframeConstraintStateSetEnabled(state, enableConstraint);
                        nanoemMutableMotionModelKeyframeAddConstraintState(mk, state, &status);
                        nanoemMutableMotionModelKeyframeConstraintStateDestroy(state);
                    }
                }
            }
            if (mk) {
                nanoem_model_constraint_t *const *constraints =
                    nanoemModelGetAllConstraintObjects(model->data(), &numConstraints);
                for (nanoem_rsize_t i = 0; i < numConstraints; i++) {
                    const nanoem_model_constraint_t *constraintPtr = constraints[i];
                    const nanoem_model_bone_t *bonePtr = nanoemModelConstraintGetEffectorBoneObject(constraintPtr);
                    const nanoem_unicode_string_t *name =
                        nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                    nanoem_mutable_motion_model_keyframe_constraint_state_t *state =
                        nanoemMutableMotionModelKeyframeConstraintStateCreate(
                            nanoemMutableMotionModelKeyframeGetOriginObject(mk), &status);
                    nanoemMutableMotionModelKeyframeConstraintStateSetBoneName(state, name, &status);
                    nanoemMutableMotionModelKeyframeConstraintStateSetEnabled(state, 1);
                    nanoemMutableMotionModelKeyframeAddConstraintState(mk, state, &status);
                    nanoemMutableMotionModelKeyframeConstraintStateDestroy(state);
                }
                nanoemMutableMotionAddModelKeyframe(m, mk, frameIndex, &status);
                nanoemMutableMotionModelKeyframeDestroy(mk);
            }
            nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numMorphs);
            for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
                const nanoem_model_morph_t *morphPtr = morphs[i];
                if (const model::Morph *morph = model::Morph::cast(morphPtr)) {
                    const nanoem_unicode_string_t *name =
                        nanoemModelMorphGetName(morphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                    if (nanoem_mutable_motion_morph_keyframe_t *src =
                            nanoemMutableMotionMorphKeyframeCreateByFound(sourceMotionPtr, name, frameIndex, &status)) {
                        nanoem_mutable_motion_morph_keyframe_t *dst = frameIndex == 0
                            ? nanoemMutableMotionMorphKeyframeCreateByFound(dstMotionPtr, name, 0, &status)
                            : nanoemMutableMotionMorphKeyframeCreate(dstMotionPtr, &status);
                        nanoemMutableMotionMorphKeyframeCopy(dst, nanoemMutableMotionMorphKeyframeGetOriginObject(src));
                        nanoemMutableMotionAddMorphKeyframe(m, dst, name, frameIndex, &status);
                        nanoemMutableMotionMorphKeyframeDestroy(dst);
                        nanoemMutableMotionMorphKeyframeDestroy(src);
                    }
                }
            }
            nanoemMutableMotionDestroy(m);
        }
        progress.increment();
        StringUtils::format(progressText, sizeof(progressText), "%s (%u/%u)", message, frameIndex, duration);
        progress.setText(progressText);
        m_project->seek(++frameIndex, true);
    }
    command::BatchUndoCommandListCommand::UndoCommandList commands;
    for (Project::ModelList::const_iterator it = models->begin(), end = models->end(); it != end; ++it) {
        Model *model = *it;
        ByteArray snapshot, bytes;
        Motion *newMotion = motions[it - models->begin()];
        nanoem_mutable_motion_t *m = nanoemMutableMotionCreateAsReference(newMotion->data(), &status);
        nanoemMutableMotionSortAllKeyframes(m);
        nanoemMutableMotionDestroy(m);
        newMotion->save(bytes, model, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
        if (Motion *currentMotion = m_project->resolveMotion(model)) {
            currentMotion->setFormat(NANOEM_MOTION_FORMAT_TYPE_NMD);
            currentMotion->save(snapshot, model, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
            currentMotion->clearAllKeyframes();
            currentMotion->load(bytes, 0, error);
            commands.push_back(command::MotionSnapshotCommand::create(
                currentMotion, model, snapshot, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL));
        }
    }
    for (Project::MotionList::const_iterator it = motions.begin(), end = motions.end(); it != end; ++it) {
        Motion *motion = *it;
        m_project->destroyMotion(motion);
    }
    m_project->seek(lastLocalFrameIndex, true);
    m_project->setPhysicsSimulationMode(lastSimulationMode);
    m_project->setFileURI(URI());
    m_project->setActiveModel(lastActiveModel);
    if (!commands.empty()) {
        m_project->pushUndo(command::BatchUndoCommandListCommand::create(commands, nullptr, m_project));
    }
    progress.complete();
}

void
CommandRegistrator::internalRegisterRemoveAllMorphKeyframesCommand(Model *model, nanoem_model_morph_category_t category)
{
    nanoem_assert(canRegisterMotionCommand(), "must not be called while playing");
    Motion *motion = m_project->resolveMotion(model);
    if (motion && canRegisterMotionCommand()) {
        nanoem_rsize_t numKeyframes;
        nanoem_motion_morph_keyframe_t *const *keyframes =
            nanoemMotionGetAllMorphKeyframeObjects(motion->data(), &numKeyframes);
        Motion::MorphKeyframeList removingKeyframes;
        for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
            const nanoem_motion_morph_keyframe_t *keyframe = keyframes[i];
            if (const nanoem_model_morph_t *morph = model->findMorph(nanoemMotionMorphKeyframeGetName(keyframe))) {
                if (nanoemModelMorphGetCategory(morph) == category) {
                    removingKeyframes.push_back(keyframe);
                }
            }
        }
        model->pushUndo(command::RemoveMorphKeyframeCommand::create(removingKeyframes, model, motion));
    }
}

bool
CommandRegistrator::internalMoveAllSelectedKeyframes(int delta, UndoCommandList &commands, Error &error)
{
    Motion::SortDirectionType direction =
        delta > 0 ? Motion::kSortDirectionTypeDescend : Motion::kSortDirectionTypeAscend;
    return internalMoveAllSelectedCameraKeyframes(delta, direction, commands, error) &&
        internalMoveAllSelectedLightKeyframes(delta, direction, commands, error) &&
        internalMoveAllSelectedSelfShadowKeyframes(delta, direction, commands, error) &&
        internalMoveAllSelectedAccessoryKeyframes(delta, direction, commands, error) &&
        internalMoveAllSelectedModelKeyframes(delta, direction, commands, error);
}

bool
CommandRegistrator::internalMoveAllSelectedAccessoryKeyframes(
    int delta, Motion::SortDirectionType direction, UndoCommandList &commands, Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_frame_index_t newFrameIndex;
    ByteArray bytes;
    const Project::AccessoryList *accessories = m_project->allAccessories();
    for (Project::AccessoryList::const_iterator it = accessories->begin(), end = accessories->end(); it != end; ++it) {
        Accessory *accessory = *it;
        if (Motion *motion = m_project->resolveMotion(accessory)) {
            Motion::AccessoryKeyframeList keyframes;
            IMotionKeyframeSelection *selection = motion->selection();
            selection->getAll(keyframes, nullptr);
            if (!keyframes.empty()) {
                Motion::AccessoryKeyframeSet deleted;
                motion->save(bytes, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
                Motion::sortAllKeyframes(keyframes, direction);
                nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion->data(), &status);
                for (Motion::AccessoryKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end();
                     it != end; ++it) {
                    const nanoem_motion_accessory_keyframe_t *keyframe = *it;
                    if (deleted.find(keyframe) != deleted.end()) {
                        continue;
                    }
                    const nanoem_frame_index_t frameIndex = nanoemMotionKeyframeObjectGetFrameIndex(
                        nanoemMotionAccessoryKeyframeGetKeyframeObject(keyframe));
                    if (Motion::addFrameIndexDelta(delta, frameIndex, newFrameIndex)) {
                        nanoem_mutable_motion_accessory_keyframe_t *foundKeyframe =
                            nanoemMutableMotionAccessoryKeyframeCreateByFound(
                                nanoemMutableMotionGetOriginObject(mutableMotion), frameIndex, &status);
                        if (frameIndex > 0) {
                            nanoemMutableMotionRemoveAccessoryKeyframe(mutableMotion, foundKeyframe, &status);
                            if (nanoem_mutable_motion_accessory_keyframe_t *removingKeyframe =
                                    nanoemMutableMotionAccessoryKeyframeCreateByFound(
                                        nanoemMutableMotionGetOriginObject(mutableMotion), newFrameIndex, &status)) {
                                deleted.insert(nanoemMutableMotionAccessoryKeyframeGetOriginObject(removingKeyframe));
                                nanoemMutableMotionRemoveAccessoryKeyframe(mutableMotion, removingKeyframe, &status);
                                nanoemMutableMotionAccessoryKeyframeDestroy(removingKeyframe);
                            }
                            nanoemMutableMotionAddAccessoryKeyframe(
                                mutableMotion, foundKeyframe, newFrameIndex, &status);
                        }
                        else {
                            nanoem_mutable_motion_accessory_keyframe_t *duplicatedKeyframe =
                                nanoemMutableMotionAccessoryKeyframeCreate(motion->data(), &status);
                            nanoemMutableMotionAccessoryKeyframeCopy(duplicatedKeyframe,
                                nanoemMutableMotionAccessoryKeyframeGetOriginObject(foundKeyframe), &status);
                            nanoemMutableMotionAddAccessoryKeyframe(
                                mutableMotion, duplicatedKeyframe, newFrameIndex, &status);
                            nanoemMutableMotionAccessoryKeyframeDestroy(duplicatedKeyframe);
                        }
                        nanoemMutableMotionAccessoryKeyframeDestroy(foundKeyframe);
                    }
                }
                for (Motion::AccessoryKeyframeSet::const_iterator it = deleted.begin(), end = deleted.end(); it != end;
                     ++it) {
                    selection->remove(*it);
                }
                nanoemMutableMotionDestroy(mutableMotion);
                commands.push_back(command::MotionSnapshotCommand::create(
                    motion, nullptr, bytes, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY));
            }
        }
    }
    return status == NANOEM_STATUS_SUCCESS;
}

bool
CommandRegistrator::internalMoveAllSelectedCameraKeyframes(
    int delta, Motion::SortDirectionType direction, UndoCommandList &commands, Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_frame_index_t newFrameIndex;
    ByteArray bytes;
    if (Motion *motion = m_project->cameraMotion()) {
        Motion::CameraKeyframeList keyframes;
        IMotionKeyframeSelection *selection = motion->selection();
        selection->getAll(keyframes, nullptr);
        if (!keyframes.empty()) {
            Motion::CameraKeyframeSet deleted;
            motion->save(bytes, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
            Motion::sortAllKeyframes(keyframes, direction);
            nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion->data(), &status);
            for (Motion::CameraKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end;
                 ++it) {
                const nanoem_motion_camera_keyframe_t *keyframe = *it;
                if (deleted.find(keyframe) != deleted.end()) {
                    continue;
                }
                const nanoem_frame_index_t frameIndex =
                    nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(keyframe));
                if (Motion::addFrameIndexDelta(delta, frameIndex, newFrameIndex)) {
                    nanoem_mutable_motion_camera_keyframe_t *foundKeyframe =
                        nanoemMutableMotionCameraKeyframeCreateByFound(
                            nanoemMutableMotionGetOriginObject(mutableMotion), frameIndex, &status);
                    if (frameIndex > 0) {
                        nanoemMutableMotionRemoveCameraKeyframe(mutableMotion, foundKeyframe, &status);
                        if (nanoem_mutable_motion_camera_keyframe_t *removingKeyframe =
                                nanoemMutableMotionCameraKeyframeCreateByFound(
                                    nanoemMutableMotionGetOriginObject(mutableMotion), newFrameIndex, &status)) {
                            deleted.insert(nanoemMutableMotionCameraKeyframeGetOriginObject(removingKeyframe));
                            nanoemMutableMotionRemoveCameraKeyframe(mutableMotion, removingKeyframe, &status);
                            nanoemMutableMotionCameraKeyframeDestroy(removingKeyframe);
                        }
                        nanoemMutableMotionAddCameraKeyframe(mutableMotion, foundKeyframe, newFrameIndex, &status);
                    }
                    else {
                        nanoem_mutable_motion_camera_keyframe_t *duplicatedKeyframe =
                            nanoemMutableMotionCameraKeyframeCreate(motion->data(), &status);
                        nanoemMutableMotionCameraKeyframeCopy(
                            duplicatedKeyframe, nanoemMutableMotionCameraKeyframeGetOriginObject(foundKeyframe));
                        nanoemMutableMotionAddCameraKeyframe(mutableMotion, duplicatedKeyframe, newFrameIndex, &status);
                        nanoemMutableMotionCameraKeyframeDestroy(duplicatedKeyframe);
                    }
                    nanoemMutableMotionCameraKeyframeDestroy(foundKeyframe);
                }
            }
            for (Motion::CameraKeyframeSet::const_iterator it = deleted.begin(), end = deleted.end(); it != end; ++it) {
                selection->remove(*it);
            }
            nanoemMutableMotionDestroy(mutableMotion);
            commands.push_back(command::MotionSnapshotCommand::create(
                motion, nullptr, bytes, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA));
        }
    }
    return status == NANOEM_STATUS_SUCCESS;
}

bool
CommandRegistrator::internalMoveAllSelectedLightKeyframes(
    int delta, Motion::SortDirectionType direction, UndoCommandList &commands, Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_frame_index_t newFrameIndex;
    ByteArray bytes;
    if (Motion *motion = m_project->lightMotion()) {
        Motion::LightKeyframeList keyframes;
        IMotionKeyframeSelection *selection = motion->selection();
        selection->getAll(keyframes, nullptr);
        if (!keyframes.empty()) {
            Motion::LightKeyframeSet deleted;
            motion->save(bytes, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
            Motion::sortAllKeyframes(keyframes, direction);
            nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion->data(), &status);
            for (Motion::LightKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end;
                 ++it) {
                const nanoem_motion_light_keyframe_t *keyframe = *it;
                if (deleted.find(keyframe) != deleted.end()) {
                    continue;
                }
                const nanoem_frame_index_t frameIndex =
                    nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionLightKeyframeGetKeyframeObject(keyframe));
                if (Motion::addFrameIndexDelta(delta, frameIndex, newFrameIndex)) {
                    nanoem_mutable_motion_light_keyframe_t *foundKeyframe =
                        nanoemMutableMotionLightKeyframeCreateByFound(
                            nanoemMutableMotionGetOriginObject(mutableMotion), frameIndex, &status);
                    if (frameIndex > 0) {
                        nanoemMutableMotionRemoveLightKeyframe(mutableMotion, foundKeyframe, &status);
                        if (nanoem_mutable_motion_light_keyframe_t *removingKeyframe =
                                nanoemMutableMotionLightKeyframeCreateByFound(
                                    nanoemMutableMotionGetOriginObject(mutableMotion), newFrameIndex, &status)) {
                            deleted.insert(nanoemMutableMotionLightKeyframeGetOriginObject(removingKeyframe));
                            nanoemMutableMotionRemoveLightKeyframe(mutableMotion, removingKeyframe, &status);
                            nanoemMutableMotionLightKeyframeDestroy(removingKeyframe);
                        }
                        nanoemMutableMotionAddLightKeyframe(mutableMotion, foundKeyframe, newFrameIndex, &status);
                    }
                    else {
                        nanoem_mutable_motion_light_keyframe_t *duplicatedKeyframe =
                            nanoemMutableMotionLightKeyframeCreate(motion->data(), &status);
                        nanoemMutableMotionLightKeyframeCopy(
                            duplicatedKeyframe, nanoemMutableMotionLightKeyframeGetOriginObject(foundKeyframe));
                        nanoemMutableMotionAddLightKeyframe(mutableMotion, duplicatedKeyframe, newFrameIndex, &status);
                        nanoemMutableMotionLightKeyframeDestroy(duplicatedKeyframe);
                    }
                    nanoemMutableMotionLightKeyframeDestroy(foundKeyframe);
                }
            }
            for (Motion::LightKeyframeSet::const_iterator it = deleted.begin(), end = deleted.end(); it != end; ++it) {
                selection->remove(*it);
            }
            nanoemMutableMotionDestroy(mutableMotion);
            commands.push_back(command::MotionSnapshotCommand::create(
                motion, nullptr, bytes, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT));
        }
    }
    return status == NANOEM_STATUS_SUCCESS;
}

bool
CommandRegistrator::internalMoveAllSelectedModelKeyframes(
    int delta, Motion::SortDirectionType direction, UndoCommandList &commands, Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_frame_index_t newFrameIndex;
    ByteArray bytes;
    Model *model = m_project->activeModel();
    if (Motion *motion = m_project->resolveMotion(model)) {
        IMotionKeyframeSelection *selection = motion->selection();
        motion->save(bytes, model, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
        nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion->data(), &status);
        {
            Motion::ModelKeyframeSet deleted;
            Motion::ModelKeyframeList keyframes;
            selection->getAll(keyframes, nullptr);
            Motion::sortAllKeyframes(keyframes, direction);
            for (Motion::ModelKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end;
                 ++it) {
                const nanoem_motion_model_keyframe_t *keyframe = *it;
                if (deleted.find(keyframe) != deleted.end()) {
                    continue;
                }
                const nanoem_frame_index_t frameIndex =
                    nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionModelKeyframeGetKeyframeObject(keyframe));
                if (Motion::addFrameIndexDelta(delta, frameIndex, newFrameIndex)) {
                    nanoem_mutable_motion_model_keyframe_t *foundKeyframe =
                        nanoemMutableMotionModelKeyframeCreateByFound(
                            nanoemMutableMotionGetOriginObject(mutableMotion), frameIndex, &status);
                    if (frameIndex > 0) {
                        nanoemMutableMotionRemoveModelKeyframe(mutableMotion, foundKeyframe, &status);
                        if (nanoem_mutable_motion_model_keyframe_t *removingKeyframe =
                                nanoemMutableMotionModelKeyframeCreateByFound(
                                    nanoemMutableMotionGetOriginObject(mutableMotion), newFrameIndex, &status)) {
                            deleted.insert(nanoemMutableMotionModelKeyframeGetOriginObject(removingKeyframe));
                            nanoemMutableMotionRemoveModelKeyframe(mutableMotion, removingKeyframe, &status);
                            nanoemMutableMotionModelKeyframeDestroy(removingKeyframe);
                        }
                        nanoemMutableMotionAddModelKeyframe(mutableMotion, foundKeyframe, newFrameIndex, &status);
                    }
                    else {
                        nanoem_mutable_motion_model_keyframe_t *duplicatedKeyframe =
                            nanoemMutableMotionModelKeyframeCreate(motion->data(), &status);
                        nanoemMutableMotionModelKeyframeCopy(duplicatedKeyframe,
                            nanoemMutableMotionModelKeyframeGetOriginObject(foundKeyframe), &status);
                        nanoemMutableMotionAddModelKeyframe(mutableMotion, duplicatedKeyframe, newFrameIndex, &status);
                        nanoemMutableMotionModelKeyframeDestroy(duplicatedKeyframe);
                    }
                    nanoemMutableMotionModelKeyframeDestroy(foundKeyframe);
                }
            }
            for (Motion::ModelKeyframeSet::const_iterator it = deleted.begin(), end = deleted.end(); it != end; ++it) {
                selection->remove(*it);
            }
        }
        {
            Motion::BoneKeyframeSet deleted;
            Motion::BoneKeyframeList keyframes;
            selection->getAll(keyframes, nullptr);
            Motion::sortAllKeyframes(keyframes, direction);
            for (Motion::BoneKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end;
                 ++it) {
                const nanoem_motion_bone_keyframe_t *keyframe = *it;
                if (deleted.find(keyframe) != deleted.end()) {
                    continue;
                }
                const nanoem_unicode_string_t *name = nanoemMotionBoneKeyframeGetName(keyframe);
                const nanoem_frame_index_t frameIndex =
                    nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(keyframe));
                if (Motion::addFrameIndexDelta(delta, frameIndex, newFrameIndex)) {
                    nanoem_mutable_motion_bone_keyframe_t *foundKeyframe = nanoemMutableMotionBoneKeyframeCreateByFound(
                        nanoemMutableMotionGetOriginObject(mutableMotion), name, frameIndex, &status);
                    if (frameIndex > 0) {
                        nanoemMutableMotionRemoveBoneKeyframe(mutableMotion, foundKeyframe, &status);
                        if (nanoem_mutable_motion_bone_keyframe_t *removingKeyframe =
                                nanoemMutableMotionBoneKeyframeCreateByFound(
                                    nanoemMutableMotionGetOriginObject(mutableMotion), name, newFrameIndex, &status)) {
                            deleted.insert(nanoemMutableMotionBoneKeyframeGetOriginObject(removingKeyframe));
                            nanoemMutableMotionRemoveBoneKeyframe(mutableMotion, removingKeyframe, &status);
                            nanoemMutableMotionBoneKeyframeDestroy(removingKeyframe);
                        }
                        nanoemMutableMotionAddBoneKeyframe(mutableMotion, foundKeyframe, name, newFrameIndex, &status);
                    }
                    else {
                        nanoem_mutable_motion_bone_keyframe_t *duplicatedKeyframe =
                            nanoemMutableMotionBoneKeyframeCreate(motion->data(), &status);
                        nanoemMutableMotionBoneKeyframeCopy(
                            duplicatedKeyframe, nanoemMutableMotionBoneKeyframeGetOriginObject(foundKeyframe));
                        nanoemMutableMotionAddBoneKeyframe(
                            mutableMotion, duplicatedKeyframe, name, newFrameIndex, &status);
                        nanoemMutableMotionBoneKeyframeDestroy(duplicatedKeyframe);
                    }
                    nanoemMutableMotionBoneKeyframeDestroy(foundKeyframe);
                }
            }
            for (Motion::BoneKeyframeSet::const_iterator it = deleted.begin(), end = deleted.end(); it != end; ++it) {
                selection->remove(*it);
            }
        }
        {
            Motion::MorphKeyframeSet deleted;
            Motion::MorphKeyframeList keyframes;
            selection->getAll(keyframes, nullptr);
            Motion::sortAllKeyframes(keyframes, direction);
            for (Motion::MorphKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end;
                 ++it) {
                const nanoem_motion_morph_keyframe_t *keyframe = *it;
                if (deleted.find(keyframe) != deleted.end()) {
                    continue;
                }
                const nanoem_unicode_string_t *name = nanoemMotionMorphKeyframeGetName(keyframe);
                const nanoem_frame_index_t frameIndex =
                    nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(keyframe));
                if (Motion::addFrameIndexDelta(delta, frameIndex, newFrameIndex)) {
                    nanoem_mutable_motion_morph_keyframe_t *foundKeyframe =
                        nanoemMutableMotionMorphKeyframeCreateByFound(
                            nanoemMutableMotionGetOriginObject(mutableMotion), name, frameIndex, &status);
                    if (frameIndex > 0) {
                        nanoemMutableMotionRemoveMorphKeyframe(mutableMotion, foundKeyframe, &status);
                        if (nanoem_mutable_motion_morph_keyframe_t *removingKeyframe =
                                nanoemMutableMotionMorphKeyframeCreateByFound(
                                    nanoemMutableMotionGetOriginObject(mutableMotion), name, newFrameIndex, &status)) {
                            deleted.insert(nanoemMutableMotionMorphKeyframeGetOriginObject(removingKeyframe));
                            nanoemMutableMotionRemoveMorphKeyframe(mutableMotion, removingKeyframe, &status);
                            nanoemMutableMotionMorphKeyframeDestroy(removingKeyframe);
                        }
                        nanoemMutableMotionAddMorphKeyframe(mutableMotion, foundKeyframe, name, newFrameIndex, &status);
                    }
                    else {
                        nanoem_mutable_motion_morph_keyframe_t *duplicatedKeyframe =
                            nanoemMutableMotionMorphKeyframeCreate(motion->data(), &status);
                        nanoemMutableMotionMorphKeyframeCopy(
                            duplicatedKeyframe, nanoemMutableMotionMorphKeyframeGetOriginObject(foundKeyframe));
                        nanoemMutableMotionAddMorphKeyframe(
                            mutableMotion, duplicatedKeyframe, name, newFrameIndex, &status);
                        nanoemMutableMotionMorphKeyframeDestroy(duplicatedKeyframe);
                    }
                    nanoemMutableMotionMorphKeyframeDestroy(foundKeyframe);
                }
            }
            for (Motion::MorphKeyframeSet::const_iterator it = deleted.begin(), end = deleted.end(); it != end; ++it) {
                selection->remove(*it);
            }
        }
        nanoemMutableMotionDestroy(mutableMotion);
        commands.push_back(command::MotionSnapshotCommand::create(motion, model, bytes,
            NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE | NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL |
                NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH));
    }
    return status == NANOEM_STATUS_SUCCESS;
}

bool
CommandRegistrator::internalMoveAllSelectedSelfShadowKeyframes(
    int delta, Motion::SortDirectionType direction, UndoCommandList &commands, Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_frame_index_t newFrameIndex;
    ByteArray bytes;
    if (Motion *motion = m_project->selfShadowMotion()) {
        Motion::SelfShadowKeyframeList keyframes;
        IMotionKeyframeSelection *selection = motion->selection();
        selection->getAll(keyframes, nullptr);
        if (!keyframes.empty()) {
            Motion::SelfShadowKeyframeSet deleted;
            motion->save(bytes, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
            Motion::sortAllKeyframes(keyframes, direction);
            nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(motion->data(), &status);
            for (Motion::SelfShadowKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end();
                 it != end; ++it) {
                const nanoem_motion_self_shadow_keyframe_t *keyframe = *it;
                if (deleted.find(keyframe) != deleted.end()) {
                    continue;
                }
                const nanoem_frame_index_t frameIndex =
                    nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionSelfShadowKeyframeGetKeyframeObject(keyframe));
                if (Motion::addFrameIndexDelta(delta, frameIndex, newFrameIndex)) {
                    nanoem_mutable_motion_self_shadow_keyframe_t *foundKeyframe =
                        nanoemMutableMotionSelfShadowKeyframeCreateByFound(
                            nanoemMutableMotionGetOriginObject(mutableMotion), frameIndex, &status);
                    if (frameIndex > 0) {
                        nanoemMutableMotionRemoveSelfShadowKeyframe(mutableMotion, foundKeyframe, &status);
                        if (nanoem_mutable_motion_self_shadow_keyframe_t *removingKeyframe =
                                nanoemMutableMotionSelfShadowKeyframeCreateByFound(
                                    nanoemMutableMotionGetOriginObject(mutableMotion), newFrameIndex, &status)) {
                            deleted.insert(nanoemMutableMotionSelfShadowKeyframeGetOriginObject(removingKeyframe));
                            nanoemMutableMotionRemoveSelfShadowKeyframe(mutableMotion, removingKeyframe, &status);
                            nanoemMutableMotionSelfShadowKeyframeDestroy(removingKeyframe);
                        }
                        nanoemMutableMotionAddSelfShadowKeyframe(mutableMotion, foundKeyframe, newFrameIndex, &status);
                    }
                    else {
                        nanoem_mutable_motion_self_shadow_keyframe_t *duplicatedKeyframe =
                            nanoemMutableMotionSelfShadowKeyframeCreate(motion->data(), &status);
                        nanoemMutableMotionSelfShadowKeyframeCopy(
                            duplicatedKeyframe, nanoemMutableMotionSelfShadowKeyframeGetOriginObject(foundKeyframe));
                        nanoemMutableMotionAddSelfShadowKeyframe(
                            mutableMotion, duplicatedKeyframe, newFrameIndex, &status);
                        nanoemMutableMotionSelfShadowKeyframeDestroy(duplicatedKeyframe);
                    }
                    nanoemMutableMotionSelfShadowKeyframeDestroy(foundKeyframe);
                }
            }
            for (Motion::SelfShadowKeyframeSet::const_iterator it = deleted.begin(), end = deleted.end(); it != end;
                 ++it) {
                selection->remove(*it);
            }
            nanoemMutableMotionDestroy(mutableMotion);
            commands.push_back(command::MotionSnapshotCommand::create(
                motion, nullptr, bytes, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW));
        }
    }
    return status == NANOEM_STATUS_SUCCESS;
}

bool
CommandRegistrator::canRegisterMotionCommand() const NANOEM_DECL_NOEXCEPT
{
    return !(m_project->isPlaying() || m_project->isModelEditingEnabled());
}

} /* namespace nanoem */
