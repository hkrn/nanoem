/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/Accessory.h"
#include "emapp/CommandRegistrator.h"
#include "emapp/IModelObjectSelection.h"
#include "emapp/Model.h"

#include "undo/undo.h"

using namespace nanoem;
using namespace test;

static int kKeyframeIndexRegistrationOrder[] = { 1336, 1337, 1338 };

static void
ACTION(Project *project)
{
    CommandRegistrator registrator(project);
    project->seek(kKeyframeIndexRegistrationOrder[1], true);
    registrator.registerRemoveTimelineFrameCommand();
    project->handleUndoAction();
    project->handleRedoAction();
}

TEST_CASE("project_shift_all_motion_keyframes_forward", "[emapp][project]")
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
        undoStackClear(project->undoStack());
        ACTION(project);
        Motion *motion = project->cameraMotion();
        CHECK_FALSE(motion->findCameraKeyframe(1335));
        CHECK(motion->findCameraKeyframe(1336));
        CHECK(motion->findCameraKeyframe(1337));
        CHECK_FALSE(motion->findCameraKeyframe(1338));
        project->handleUndoAction();
        CHECK_FALSE(motion->findCameraKeyframe(1335));
        CHECK(motion->findCameraKeyframe(1336));
        CHECK(motion->findCameraKeyframe(1337));
        CHECK(motion->findCameraKeyframe(1338));
        CHECK_FALSE(motion->findCameraKeyframe(1339));
    }
    SECTION("light")
    {
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            registrator.registerAddLightKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        ACTION(project);
        Motion *motion = project->lightMotion();
        CHECK_FALSE(motion->findLightKeyframe(1335));
        CHECK(motion->findLightKeyframe(1336));
        CHECK(motion->findLightKeyframe(1337));
        CHECK_FALSE(motion->findLightKeyframe(1338));
        project->handleUndoAction();
        CHECK_FALSE(motion->findLightKeyframe(1335));
        CHECK(motion->findLightKeyframe(1336));
        CHECK(motion->findLightKeyframe(1337));
        CHECK(motion->findLightKeyframe(1338));
        CHECK_FALSE(motion->findLightKeyframe(1339));
    }
    SECTION("selfshadow")
    {
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            registrator.registerAddSelfShadowKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        ACTION(project);
        Motion *motion = project->selfShadowMotion();
        CHECK_FALSE(motion->findSelfShadowKeyframe(1335));
        CHECK(motion->findSelfShadowKeyframe(1336));
        CHECK(motion->findSelfShadowKeyframe(1337));
        CHECK_FALSE(motion->findSelfShadowKeyframe(1338));
        project->handleUndoAction();
        CHECK_FALSE(motion->findSelfShadowKeyframe(1335));
        CHECK(motion->findSelfShadowKeyframe(1336));
        CHECK(motion->findSelfShadowKeyframe(1337));
        CHECK(motion->findSelfShadowKeyframe(1338));
        CHECK_FALSE(motion->findSelfShadowKeyframe(1339));
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
        undoStackClear(project->undoStack());
        ACTION(project);
        Motion *motion = project->resolveMotion(activeAccessory);
        CHECK_FALSE(motion->findAccessoryKeyframe(1335));
        CHECK(motion->findAccessoryKeyframe(1336));
        CHECK(motion->findAccessoryKeyframe(1337));
        CHECK_FALSE(motion->findAccessoryKeyframe(1338));
        project->handleUndoAction();
        CHECK_FALSE(motion->findAccessoryKeyframe(1335));
        CHECK(motion->findAccessoryKeyframe(1336));
        CHECK(motion->findAccessoryKeyframe(1337));
        CHECK(motion->findAccessoryKeyframe(1338));
        CHECK_FALSE(motion->findAccessoryKeyframe(1339));
    }
    SECTION("bone")
    {
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        /* select all "visible and hidden" bones */
        activeModel->setShowAllBones(true);
        activeModel->selection()->addAllBones();
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
        }
        undoStackClear(activeModel->undoStack());
        ACTION(project);
        Motion *motion = project->resolveMotion(activeModel);
        const nanoem_unicode_string_t *name =
            nanoemModelBoneGetName(TestScope::findRandomBone(activeModel), NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        CHECK_FALSE(motion->findBoneKeyframe(name, 1335));
        CHECK(motion->findBoneKeyframe(name, 1336));
        CHECK(motion->findBoneKeyframe(name, 1337));
        CHECK_FALSE(motion->findBoneKeyframe(name, 1338));
        project->handleUndoAction();
        CHECK_FALSE(motion->findBoneKeyframe(name, 1335));
        CHECK(motion->findBoneKeyframe(name, 1336));
        CHECK(motion->findBoneKeyframe(name, 1337));
        CHECK(motion->findBoneKeyframe(name, 1338));
        CHECK_FALSE(motion->findBoneKeyframe(name, 1339));
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
        undoStackClear(activeModel->undoStack());
        ACTION(project);
        Motion *motion = project->resolveMotion(activeModel);
        CHECK_FALSE(motion->findModelKeyframe(1335));
        CHECK(motion->findModelKeyframe(1336));
        CHECK(motion->findModelKeyframe(1337));
        CHECK_FALSE(motion->findModelKeyframe(1338));
        project->handleUndoAction();
        CHECK_FALSE(motion->findModelKeyframe(1335));
        CHECK(motion->findModelKeyframe(1336));
        CHECK(motion->findModelKeyframe(1337));
        CHECK(motion->findModelKeyframe(1338));
        CHECK_FALSE(motion->findModelKeyframe(1339));
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
        undoStackClear(activeModel->undoStack());
        ACTION(project);
        Motion *motion = project->resolveMotion(activeModel);
        const nanoem_unicode_string_t *name =
            nanoemModelMorphGetName(TestScope::findRandomMorph(activeModel), NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        CHECK_FALSE(motion->findMorphKeyframe(name, 1335));
        CHECK(motion->findMorphKeyframe(name, 1336));
        CHECK(motion->findMorphKeyframe(name, 1337));
        CHECK_FALSE(motion->findMorphKeyframe(name, 1338));
        project->handleUndoAction();
        CHECK_FALSE(motion->findMorphKeyframe(name, 1335));
        CHECK(motion->findMorphKeyframe(name, 1336));
        CHECK(motion->findMorphKeyframe(name, 1337));
        CHECK(motion->findMorphKeyframe(name, 1338));
        CHECK_FALSE(motion->findMorphKeyframe(name, 1339));
    }
}

TEST_CASE("project_shift_all_motion_keyframes_forward_redo", "[emapp][project]")
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
        undoStackClear(firstProject->undoStack());
        ACTION(firstProject);
        ProjectPtr second = scope.createProject();
        Project *secondProject = second->m_project;
        scope.recover(secondProject);
        Motion *motion = secondProject->cameraMotion();
        CHECK_FALSE(motion->findCameraKeyframe(1335));
        CHECK(motion->findCameraKeyframe(1336));
        CHECK(motion->findCameraKeyframe(1337));
        CHECK_FALSE(motion->findCameraKeyframe(1338));
    }
    SECTION("light")
    {
        for (auto it : kKeyframeIndexRegistrationOrder) {
            firstProject->seek(it, true);
            registrator.registerAddLightKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(firstProject->undoStack());
        ACTION(firstProject);
        ProjectPtr second = scope.createProject();
        Project *secondProject = second->m_project;
        scope.recover(secondProject);
        Motion *motion = secondProject->lightMotion();
        CHECK_FALSE(motion->findLightKeyframe(1335));
        CHECK(motion->findLightKeyframe(1336));
        CHECK(motion->findLightKeyframe(1337));
        CHECK_FALSE(motion->findLightKeyframe(1338));
    }
    SECTION("selfshadow")
    {
        for (auto it : kKeyframeIndexRegistrationOrder) {
            firstProject->seek(it, true);
            registrator.registerAddSelfShadowKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(firstProject->undoStack());
        ACTION(firstProject);
        ProjectPtr second = scope.createProject();
        Project *secondProject = second->m_project;
        scope.recover(secondProject);
        Motion *motion = secondProject->selfShadowMotion();
        CHECK_FALSE(motion->findSelfShadowKeyframe(1335));
        CHECK(motion->findSelfShadowKeyframe(1336));
        CHECK(motion->findSelfShadowKeyframe(1337));
        CHECK_FALSE(motion->findSelfShadowKeyframe(1338));
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
        undoStackClear(firstProject->undoStack());
        registrator.registerRemoveTimelineFrameCommand();
        firstProject->handleUndoAction();
        firstProject->handleRedoAction();
        ProjectPtr second = scope.createProject();
        Project *secondProject = second->m_project;
        scope.recover(secondProject);
        Motion *motion = secondProject->resolveMotion(secondProject->allAccessories()->data()[0]);
        CHECK_FALSE(motion->findAccessoryKeyframe(1335));
        CHECK(motion->findAccessoryKeyframe(1336));
        CHECK(motion->findAccessoryKeyframe(1337));
        CHECK_FALSE(motion->findAccessoryKeyframe(1338));
    }
    SECTION("bone")
    {
        Model *activeModel = first->createModel();
        firstProject->addModel(activeModel);
        firstProject->setActiveModel(activeModel);
        /* select all "visible and hidden" bones */
        activeModel->setShowAllBones(true);
        activeModel->selection()->addAllBones();
        for (auto it : kKeyframeIndexRegistrationOrder) {
            firstProject->seek(it, true);
            registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
        }
        undoStackClear(activeModel->undoStack());
        ACTION(firstProject);
        ProjectPtr second = scope.createProject();
        Project *secondProject = second->m_project;
        scope.recover(secondProject);
        Model *activeModel2 = secondProject->activeModel();
        Motion *motion = secondProject->resolveMotion(activeModel2);
        const nanoem_unicode_string_t *name =
            nanoemModelBoneGetName(TestScope::findRandomBone(activeModel2), NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        CHECK_FALSE(motion->findBoneKeyframe(name, 1335));
        CHECK(motion->findBoneKeyframe(name, 1336));
        CHECK(motion->findBoneKeyframe(name, 1337));
        CHECK_FALSE(motion->findBoneKeyframe(name, 1338));
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
        undoStackClear(activeModel->undoStack());
        ACTION(firstProject);
        ProjectPtr second = scope.createProject();
        Project *secondProject = second->m_project;
        scope.recover(secondProject);
        Model *activeModel2 = secondProject->activeModel();
        Motion *motion = secondProject->resolveMotion(activeModel2);
        CHECK_FALSE(motion->findModelKeyframe(1335));
        CHECK(motion->findModelKeyframe(1336));
        CHECK(motion->findModelKeyframe(1337));
        CHECK_FALSE(motion->findModelKeyframe(1338));
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
        undoStackClear(activeModel->undoStack());
        ACTION(firstProject);
        ProjectPtr second = scope.createProject();
        Project *secondProject = second->m_project;
        scope.recover(secondProject);
        Model *activeModel2 = secondProject->activeModel();
        Motion *motion = secondProject->resolveMotion(activeModel2);
        const nanoem_unicode_string_t *name =
            nanoemModelMorphGetName(TestScope::findRandomMorph(activeModel2), NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        CHECK_FALSE(motion->findMorphKeyframe(name, 1335));
        CHECK(motion->findMorphKeyframe(name, 1336));
        CHECK(motion->findMorphKeyframe(name, 1337));
        CHECK_FALSE(motion->findMorphKeyframe(name, 1338));
    }
}

TEST_CASE("project_shift_all_motion_keyframes_forward_after_remove", "[emapp][project]")
{
    TestScope scope;
    Error error;
    ProjectPtr first = scope.createProject();
    Project *firstProject = first->withRecoverable();
    CommandRegistrator registrator(firstProject);
    SECTION("accessory")
    {
        Accessory *activeAccessory = first->createAccessory();
        firstProject->addAccessory(activeAccessory);
        firstProject->setActiveAccessory(activeAccessory);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            firstProject->seek(it, true);
            registrator.registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(activeAccessory);
        }
        undoStackClear(firstProject->undoStack());
        ACTION(firstProject);
        firstProject->removeAccessory(activeAccessory);
        firstProject->destroyAccessory(activeAccessory);
        firstProject->handleUndoAction();
        SUCCEED("should not be crashed");
    }
}
