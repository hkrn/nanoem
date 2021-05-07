/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/Accessory.h"
#include "emapp/CommandRegistrator.h"
#include "emapp/IModelObjectSelection.h"
#include "emapp/ITrack.h"
#include "emapp/Model.h"

using namespace nanoem;
using namespace test;

static const int kKeyframeIndexRegistrationOrder[] = { 1336, 1337, 1338 };

static TimelineSegment
createSegment()
{
    TimelineSegment segment;
    segment.m_from = kKeyframeIndexRegistrationOrder[0];
    segment.m_to = kKeyframeIndexRegistrationOrder[2];
    return segment;
}

static ITrack *
findTrack(const void *opaque, const Project *project)
{
    const Project::TrackList *tracks = project->allTracks();
    for (const auto it : *tracks) {
        for (const auto &it2 : it->children()) {
            const ITrack *track = it2;
            if (track->opaque() == opaque) {
                return it2;
            }
        }
    }
    return 0;
}

TEST_CASE("project_copy_paste_motion_keyframes", "[emapp][project]")
{
    TestScope scope;
    Error error;
    ProjectPtr first = scope.createProject();
    Project *project = first->m_project;
    CommandRegistrator registrator(project);
    SECTION("camera")
    {
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            registrator.registerAddCameraKeyframesCommandByCurrentLocalFrameIndex();
        }
        project->setSelectedTrack(project->allTracks()->data()[0]);
        project->selectAllMotionKeyframesIn(createSegment());
        CHECK(project->isMotionClipboardEmpty());
        project->copyAllSelectedKeyframes(error);
        CHECK_FALSE(error.hasReason());
        CHECK_FALSE(project->isMotionClipboardEmpty());
        project->pasteAllSelectedKeyframes(2672, error);
        CHECK_FALSE(error.hasReason());
        Motion *motion = project->cameraMotion();
        CHECK_FALSE(motion->findCameraKeyframe(2671));
        CHECK(motion->findCameraKeyframe(2672));
        CHECK(motion->findCameraKeyframe(2673));
        CHECK(motion->findCameraKeyframe(2674));
        CHECK_FALSE(motion->findCameraKeyframe(2675));
        CHECK(motion->duration() == 2674);
        project->handleUndoAction();
        CHECK_FALSE(motion->findCameraKeyframe(2672));
        CHECK_FALSE(motion->findCameraKeyframe(2673));
        CHECK_FALSE(motion->findCameraKeyframe(2674));
        CHECK(motion->duration() == 1338);
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("light")
    {
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            registrator.registerAddLightKeyframesCommandByCurrentLocalFrameIndex();
        }
        project->setSelectedTrack(project->allTracks()->data()[1]);
        project->selectAllMotionKeyframesIn(createSegment());
        CHECK(project->isMotionClipboardEmpty());
        project->copyAllSelectedKeyframes(error);
        CHECK_FALSE(error.hasReason());
        CHECK_FALSE(project->isMotionClipboardEmpty());
        project->pasteAllSelectedKeyframes(2672, error);
        CHECK_FALSE(error.hasReason());
        Motion *motion = project->lightMotion();
        CHECK_FALSE(motion->findLightKeyframe(2671));
        CHECK(motion->findLightKeyframe(2672));
        CHECK(motion->findLightKeyframe(2673));
        CHECK(motion->findLightKeyframe(2674));
        CHECK_FALSE(motion->findLightKeyframe(2675));
        CHECK(motion->duration() == 2674);
        project->handleUndoAction();
        CHECK_FALSE(motion->findLightKeyframe(2672));
        CHECK_FALSE(motion->findLightKeyframe(2673));
        CHECK_FALSE(motion->findLightKeyframe(2674));
        CHECK(motion->duration() == 1338);
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("selfshadow")
    {
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            registrator.registerAddSelfShadowKeyframesCommandByCurrentLocalFrameIndex();
        }
        project->setSelectedTrack(project->allTracks()->data()[2]);
        project->selectAllMotionKeyframesIn(createSegment());
        CHECK(project->isMotionClipboardEmpty());
        project->copyAllSelectedKeyframes(error);
        CHECK_FALSE(error.hasReason());
        CHECK_FALSE(project->isMotionClipboardEmpty());
        project->pasteAllSelectedKeyframes(2672, error);
        CHECK_FALSE(error.hasReason());
        Motion *motion = project->selfShadowMotion();
        CHECK_FALSE(motion->findSelfShadowKeyframe(2671));
        CHECK(motion->findSelfShadowKeyframe(2672));
        CHECK(motion->findSelfShadowKeyframe(2673));
        CHECK(motion->findSelfShadowKeyframe(2674));
        CHECK_FALSE(motion->findSelfShadowKeyframe(2675));
        CHECK(motion->duration() == 2674);
        project->handleUndoAction();
        CHECK_FALSE(motion->findSelfShadowKeyframe(2672));
        CHECK_FALSE(motion->findSelfShadowKeyframe(2673));
        CHECK_FALSE(motion->findSelfShadowKeyframe(2674));
        CHECK(motion->duration() == 1338);
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("accessory")
    {
        Accessory *activeAccessory = first->createAccessory();
        project->addAccessory(activeAccessory);
        project->setActiveAccessory(activeAccessory);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            registrator.registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(activeAccessory);
        }
        project->setSelectedTrack(project->allTracks()->data()[3]);
        project->selectAllMotionKeyframesIn(createSegment());
        CHECK(project->isMotionClipboardEmpty());
        project->copyAllSelectedKeyframes(error);
        CHECK_FALSE(error.hasReason());
        CHECK_FALSE(project->isMotionClipboardEmpty());
        project->pasteAllSelectedKeyframes(2672, error);
        CHECK_FALSE(error.hasReason());
        Motion *motion = project->resolveMotion(activeAccessory);
        CHECK_FALSE(motion->findAccessoryKeyframe(2671));
        CHECK(motion->findAccessoryKeyframe(2672));
        CHECK(motion->findAccessoryKeyframe(2673));
        CHECK(motion->findAccessoryKeyframe(2674));
        CHECK_FALSE(motion->findAccessoryKeyframe(2675));
        CHECK(motion->duration() == 2674);
        project->handleUndoAction();
        CHECK_FALSE(motion->findAccessoryKeyframe(2672));
        CHECK_FALSE(motion->findAccessoryKeyframe(2673));
        CHECK_FALSE(motion->findAccessoryKeyframe(2674));
        CHECK(motion->duration() == 1338);
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("bone")
    {
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        activeModel->setShowAllBones(true);
        activeModel->selection()->addAllBones();
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
        }
        const nanoem_model_bone_t *bonePtr = activeModel->activeBone();
        project->setSelectedTrack(findTrack(bonePtr, project));
        project->selectAllMotionKeyframesIn(createSegment());
        CHECK(project->isMotionClipboardEmpty());
        project->copyAllSelectedKeyframes(error);
        CHECK_FALSE(error.hasReason());
        CHECK_FALSE(project->isMotionClipboardEmpty());
        project->pasteAllSelectedKeyframes(2672, error);
        CHECK_FALSE(error.hasReason());
        Motion *motion = project->resolveMotion(activeModel);
        const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        CHECK_FALSE(motion->findBoneKeyframe(name, 2671));
        CHECK(motion->findBoneKeyframe(name, 2672));
        CHECK(motion->findBoneKeyframe(name, 2673));
        CHECK(motion->findBoneKeyframe(name, 2674));
        CHECK_FALSE(motion->findBoneKeyframe(name, 2675));
        CHECK(motion->duration() == 2674);
        project->handleUndoAction();
        CHECK_FALSE(motion->findBoneKeyframe(name, 2672));
        CHECK_FALSE(motion->findBoneKeyframe(name, 2673));
        CHECK_FALSE(motion->findBoneKeyframe(name, 2674));
        CHECK(motion->duration() == 1338);
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("model")
    {
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            registrator.registerAddModelKeyframesCommandByCurrentLocalFrameIndex(activeModel);
        }
        project->setSelectedTrack(project->allTracks()->data()[0]);
        project->selectAllMotionKeyframesIn(createSegment());
        CHECK(project->isMotionClipboardEmpty());
        project->copyAllSelectedKeyframes(error);
        CHECK_FALSE(error.hasReason());
        CHECK_FALSE(project->isMotionClipboardEmpty());
        project->pasteAllSelectedKeyframes(2672, error);
        CHECK_FALSE(error.hasReason());
        Motion *motion = project->resolveMotion(activeModel);
        CHECK_FALSE(motion->findModelKeyframe(2671));
        CHECK(motion->findModelKeyframe(2672));
        CHECK(motion->findModelKeyframe(2673));
        CHECK(motion->findModelKeyframe(2674));
        CHECK_FALSE(motion->findModelKeyframe(2675));
        CHECK(motion->duration() == 2674);
        project->handleUndoAction();
        CHECK_FALSE(motion->findModelKeyframe(2672));
        CHECK_FALSE(motion->findModelKeyframe(2673));
        CHECK_FALSE(motion->findModelKeyframe(2674));
        CHECK(motion->duration() == 1338);
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("morph")
    {
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            registrator.registerAddMorphKeyframesCommandByAllMorphs(activeModel);
        }
        const nanoem_model_morph_t *morphPtr = TestScope::findRandomMorph(activeModel);
        project->setSelectedTrack(findTrack(morphPtr, project));
        project->selectAllMotionKeyframesIn(createSegment());
        CHECK(project->isMotionClipboardEmpty());
        project->copyAllSelectedKeyframes(error);
        CHECK_FALSE(error.hasReason());
        CHECK_FALSE(project->isMotionClipboardEmpty());
        project->pasteAllSelectedKeyframes(2672, error);
        CHECK_FALSE(error.hasReason());
        Motion *motion = project->resolveMotion(activeModel);
        const nanoem_unicode_string_t *name = nanoemModelMorphGetName(morphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        CHECK_FALSE(motion->findMorphKeyframe(name, 2671));
        CHECK(motion->findMorphKeyframe(name, 2672));
        CHECK(motion->findMorphKeyframe(name, 2673));
        CHECK(motion->findMorphKeyframe(name, 2674));
        CHECK_FALSE(motion->findMorphKeyframe(name, 2675));
        CHECK(motion->duration() == 2674);
        project->handleUndoAction();
        CHECK_FALSE(motion->findMorphKeyframe(name, 2672));
        CHECK_FALSE(motion->findMorphKeyframe(name, 2673));
        CHECK_FALSE(motion->findMorphKeyframe(name, 2674));
        CHECK(motion->duration() == 1338);
        CHECK_FALSE(scope.hasAnyError());
    }
}

static void
ACTION(Project *project)
{
    Error error;
    project->selectAllMotionKeyframesIn(createSegment());
    project->copyAllSelectedKeyframes(error);
    project->pasteAllSelectedKeyframes(2672, error);
    project->handleUndoAction();
    project->handleRedoAction();
}

TEST_CASE("project_copy_paste_motion_keyframes_redo", "[emapp][project]")
{
    TestScope scope;
    Error error;
    ProjectPtr first = scope.createProject();
    Project *firstProject = first->withRecoverable();
    CommandRegistrator registrator(firstProject);
    SECTION("camera")
    {
        for (auto it : kKeyframeIndexRegistrationOrder) {
            firstProject->seek(it, true);
            registrator.registerAddCameraKeyframesCommandByCurrentLocalFrameIndex();
        }
        firstProject->setSelectedTrack(firstProject->allTracks()->data()[0]);
        ACTION(firstProject);
        ProjectPtr second = scope.createProject();
        Project *secondProject = second->m_project;
        scope.recover(secondProject);
        Motion *motion = secondProject->cameraMotion();
        CHECK_FALSE(motion->findCameraKeyframe(2671));
        CHECK(motion->findCameraKeyframe(2672));
        CHECK(motion->findCameraKeyframe(2673));
        CHECK(motion->findCameraKeyframe(2674));
        CHECK_FALSE(motion->findCameraKeyframe(2675));
        CHECK(motion->duration() == 2674);
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("light")
    {
        for (auto it : kKeyframeIndexRegistrationOrder) {
            firstProject->seek(it, true);
            registrator.registerAddLightKeyframesCommandByCurrentLocalFrameIndex();
        }
        firstProject->setSelectedTrack(firstProject->allTracks()->data()[1]);
        ACTION(firstProject);
        ProjectPtr second = scope.createProject();
        Project *secondProject = second->m_project;
        scope.recover(secondProject);
        Motion *motion = secondProject->lightMotion();
        CHECK_FALSE(motion->findLightKeyframe(2671));
        CHECK(motion->findLightKeyframe(2672));
        CHECK(motion->findLightKeyframe(2673));
        CHECK(motion->findLightKeyframe(2674));
        CHECK_FALSE(motion->findLightKeyframe(2675));
        CHECK(motion->duration() == 2674);
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("selfshadow")
    {
        for (auto it : kKeyframeIndexRegistrationOrder) {
            firstProject->seek(it, true);
            registrator.registerAddSelfShadowKeyframesCommandByCurrentLocalFrameIndex();
        }
        firstProject->setSelectedTrack(firstProject->allTracks()->data()[2]);
        ACTION(firstProject);
        ProjectPtr second = scope.createProject();
        Project *secondProject = second->m_project;
        scope.recover(secondProject);
        Motion *motion = secondProject->selfShadowMotion();
        CHECK_FALSE(motion->findSelfShadowKeyframe(2671));
        CHECK(motion->findSelfShadowKeyframe(2672));
        CHECK(motion->findSelfShadowKeyframe(2673));
        CHECK(motion->findSelfShadowKeyframe(2674));
        CHECK_FALSE(motion->findSelfShadowKeyframe(2675));
        CHECK(motion->duration() == 2674);
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("accessory")
    {
        Accessory *activeAccessory = first->createAccessory();
        firstProject->addAccessory(activeAccessory);
        firstProject->setActiveAccessory(activeAccessory);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            firstProject->seek(it, true);
            registrator.registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(activeAccessory);
        }
        firstProject->setSelectedTrack(firstProject->allTracks()->data()[3]);
        ACTION(firstProject);
        ProjectPtr second = scope.createProject();
        Project *secondProject = second->m_project;
        scope.recover(secondProject);
        Motion *motion = secondProject->resolveMotion(secondProject->allAccessories()->data()[0]);
        CHECK_FALSE(motion->findAccessoryKeyframe(2671));
        CHECK(motion->findAccessoryKeyframe(2672));
        CHECK(motion->findAccessoryKeyframe(2673));
        CHECK(motion->findAccessoryKeyframe(2674));
        CHECK_FALSE(motion->findAccessoryKeyframe(2675));
        CHECK(motion->duration() == 2674);
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("bone")
    {
        Model *activeModel = first->createModel();
        firstProject->addModel(activeModel);
        firstProject->setActiveModel(activeModel);
        activeModel->selection()->addAllBones();
        for (auto it : kKeyframeIndexRegistrationOrder) {
            firstProject->seek(it, true);
            registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
        }
        const nanoem_model_bone_t *bonePtr = activeModel->activeBone();
        firstProject->setSelectedTrack(findTrack(bonePtr, firstProject));
        ACTION(firstProject);
        ProjectPtr second = scope.createProject();
        Project *secondProject = second->m_project;
        scope.recover(secondProject);
        Motion *motion = secondProject->resolveMotion(secondProject->activeModel());
        const nanoem_model_bone_t *bonePtr2 =
            secondProject->activeModel()->findBone(nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM));
        const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bonePtr2, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        CHECK_FALSE(motion->findBoneKeyframe(name, 2671));
        CHECK(motion->findBoneKeyframe(name, 2672));
        CHECK(motion->findBoneKeyframe(name, 2673));
        CHECK(motion->findBoneKeyframe(name, 2674));
        CHECK_FALSE(motion->findBoneKeyframe(name, 2675));
        CHECK(motion->duration() == 2674);
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("model")
    {
        Model *activeModel = first->createModel();
        firstProject->addModel(activeModel);
        firstProject->setActiveModel(activeModel);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            firstProject->seek(it, true);
            registrator.registerAddModelKeyframesCommandByCurrentLocalFrameIndex(activeModel);
        }
        firstProject->setSelectedTrack(firstProject->allTracks()->data()[0]);
        ACTION(firstProject);
        ProjectPtr second = scope.createProject();
        Project *secondProject = second->m_project;
        scope.recover(secondProject);
        Motion *motion = secondProject->resolveMotion(secondProject->activeModel());
        CHECK_FALSE(motion->findModelKeyframe(2671));
        CHECK(motion->findModelKeyframe(2672));
        CHECK(motion->findModelKeyframe(2673));
        CHECK(motion->findModelKeyframe(2674));
        CHECK_FALSE(motion->findModelKeyframe(2675));
        CHECK(motion->duration() == 2674);
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("morph")
    {
        Model *activeModel = first->createModel();
        firstProject->addModel(activeModel);
        firstProject->setActiveModel(activeModel);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            firstProject->seek(it, true);
            registrator.registerAddMorphKeyframesCommandByAllMorphs(activeModel);
        }
        const nanoem_model_morph_t *morphPtr = TestScope::findRandomMorph(activeModel);
        firstProject->setSelectedTrack(findTrack(morphPtr, firstProject));
        ACTION(firstProject);
        ProjectPtr second = scope.createProject();
        Project *secondProject = second->m_project;
        scope.recover(secondProject);
        Motion *motion = secondProject->resolveMotion(secondProject->activeModel());
        const nanoem_model_morph_t *morphPtr2 =
            secondProject->activeModel()->findMorph(nanoemModelMorphGetName(morphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM));
        const nanoem_unicode_string_t *name = nanoemModelMorphGetName(morphPtr2, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        CHECK_FALSE(motion->findMorphKeyframe(name, 2671));
        CHECK(motion->findMorphKeyframe(name, 2672));
        CHECK(motion->findMorphKeyframe(name, 2673));
        CHECK(motion->findMorphKeyframe(name, 2674));
        CHECK_FALSE(motion->findMorphKeyframe(name, 2675));
        CHECK(motion->duration() == 2674);
        CHECK_FALSE(scope.hasAnyError());
    }
}
