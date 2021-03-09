/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/Accessory.h"
#include "emapp/CommandRegistrator.h"

using namespace nanoem;
using namespace test;

TEST_CASE("motion_update_accessory_keyframe", "[emapp][motion]")
{
    TestScope scope;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Accessory *other1Accessory = first->createAccessory();
        Accessory *other2Accessory = first->createAccessory();
        Accessory *activeAccessory = first->createAccessory();
        Model *bindingModel = first->createModel();
        {
            project->addAccessory(other1Accessory);
            project->addAccessory(other2Accessory);
            project->addAccessory(activeAccessory);
            project->setActiveAccessory(activeAccessory);
            project->addModel(bindingModel);
            REQUIRE(project->allAccessories().size() == 3);
            REQUIRE(project->allModels().size() == 1);
            REQUIRE(project->activeAccessory() == activeAccessory);
            project->seek(1337, true);
            activeAccessory->setTranslation(Vector3(42, 84, 126));
            activeAccessory->setOrientation(glm::radians(Vector3(15, 30, 45)));
            activeAccessory->setOpacity(0.4f);
            activeAccessory->setScaleFactor(0.8f);
        }
        CommandRegistrator registrator(project);
        registrator.registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(activeAccessory);
        {
            activeAccessory->setTranslation(Vector3(126, 42, 84));
            activeAccessory->setOrientation(glm::radians(Vector3(45, 15, 30)));
            activeAccessory->setOpacity(0.8f);
            activeAccessory->setScaleFactor(0.4f);
        }
        registrator.registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(activeAccessory);
        SECTION("updating current active accessory keyframe")
        {
            CHECK(first->countAllAccessoryKeyframes(activeAccessory) == 2);
            CHECK(first->countAllAccessoryKeyframes(other1Accessory) == 1);
            CHECK(first->countAllAccessoryKeyframes(other2Accessory) == 1);
            const nanoem_motion_accessory_keyframe_t *keyframe =
                nanoemMotionFindAccessoryKeyframeObject(project->resolveMotion(activeAccessory)->data(), 1337);
            CHECK(keyframe);
            CHECK_FALSE(nanoemMotionFindAccessoryKeyframeObject(project->resolveMotion(other1Accessory)->data(), 1337));
            CHECK_FALSE(nanoemMotionFindAccessoryKeyframeObject(project->resolveMotion(other2Accessory)->data(), 1337));
            CHECK_THAT(
                glm::make_vec3(nanoemMotionAccessoryKeyframeGetTranslation(keyframe)), Equals(Vector3(126, 42, 84)));
            CHECK(glm::make_quat(nanoemMotionAccessoryKeyframeGetOrientation(keyframe)) ==
                Quaternion(glm::radians(Vector3(45, 15, 30))));
            CHECK(nanoemMotionAccessoryKeyframeGetOpacity(keyframe) == Approx(0.8));
            CHECK(nanoemMotionAccessoryKeyframeGetScaleFactor(keyframe) == Approx(0.4));
            CHECK(project->duration() == 1337);
            CHECK(first->motionDuration(activeAccessory) == 1337);
            CHECK(first->motionDuration(other1Accessory) == 0);
            CHECK(first->motionDuration(other2Accessory) == 0);
            CHECK(project->resolveMotion(activeAccessory)->isDirty());
            CHECK_FALSE(project->resolveMotion(other1Accessory)->isDirty());
            CHECK_FALSE(project->resolveMotion(other2Accessory)->isDirty());
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleUndoAction();
        SECTION("undo")
        {
            const nanoem_motion_accessory_keyframe_t *keyframe =
                nanoemMotionFindAccessoryKeyframeObject(project->resolveMotion(activeAccessory)->data(), 1337);
            CHECK(keyframe);
            CHECK_FALSE(nanoemMotionFindAccessoryKeyframeObject(project->resolveMotion(other1Accessory)->data(), 1337));
            CHECK_FALSE(nanoemMotionFindAccessoryKeyframeObject(project->resolveMotion(other2Accessory)->data(), 1337));
            CHECK_THAT(
                glm::make_vec3(nanoemMotionAccessoryKeyframeGetTranslation(keyframe)), Equals(Vector3(42, 84, 126)));
            CHECK(glm::make_quat(nanoemMotionAccessoryKeyframeGetOrientation(keyframe)) ==
                Quaternion(glm::radians(Vector3(15, 30, 45))));
            CHECK(nanoemMotionAccessoryKeyframeGetOpacity(keyframe) == Approx(0.4));
            CHECK(nanoemMotionAccessoryKeyframeGetScaleFactor(keyframe) == Approx(0.8));
            CHECK(project->duration() == 1337);
            CHECK(first->motionDuration(activeAccessory) == 1337);
            CHECK(first->motionDuration(other1Accessory) == 0);
            CHECK(first->motionDuration(other2Accessory) == 0);
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleRedoAction();
        SECTION("redo")
        {
            CHECK(first->countAllAccessoryKeyframes(activeAccessory) == 2);
            CHECK(first->countAllAccessoryKeyframes(other1Accessory) == 1);
            CHECK(first->countAllAccessoryKeyframes(other2Accessory) == 1);
            const nanoem_motion_accessory_keyframe_t *keyframe =
                nanoemMotionFindAccessoryKeyframeObject(project->resolveMotion(activeAccessory)->data(), 1337);
            CHECK(keyframe);
            CHECK_FALSE(nanoemMotionFindAccessoryKeyframeObject(project->resolveMotion(other1Accessory)->data(), 1337));
            CHECK_FALSE(nanoemMotionFindAccessoryKeyframeObject(project->resolveMotion(other2Accessory)->data(), 1337));
            CHECK_THAT(
                glm::make_vec3(nanoemMotionAccessoryKeyframeGetTranslation(keyframe)), Equals(Vector3(126, 42, 84)));
            CHECK(glm::make_quat(nanoemMotionAccessoryKeyframeGetOrientation(keyframe)) ==
                Quaternion(glm::radians(Vector3(45, 15, 30))));
            CHECK(nanoemMotionAccessoryKeyframeGetOpacity(keyframe) == Approx(0.8));
            CHECK(nanoemMotionAccessoryKeyframeGetScaleFactor(keyframe) == Approx(0.4));
            CHECK(project->duration() == 1337);
            CHECK(first->motionDuration(activeAccessory) == 1337);
            CHECK(first->motionDuration(other1Accessory) == 0);
            CHECK(first->motionDuration(other2Accessory) == 0);
            CHECK_FALSE(scope.hasAnyError());
        }
        SECTION("recover")
        {
            ProjectPtr second = scope.createProject();
            Project *project2 = second.get()->m_project;
            scope.recover(project2);
            Accessory *activeAccessory2 = project2->activeAccessory();
            CHECK(second->countAllAccessoryKeyframes(activeAccessory2) == 2);
            const nanoem_motion_accessory_keyframe_t *keyframe2 =
                nanoemMotionFindAccessoryKeyframeObject(project2->resolveMotion(activeAccessory2)->data(), 1337);
            CHECK(keyframe2);
            CHECK_THAT(
                glm::make_vec3(nanoemMotionAccessoryKeyframeGetTranslation(keyframe2)), Equals(Vector3(126, 42, 84)));
            CHECK(glm::make_quat(nanoemMotionAccessoryKeyframeGetOrientation(keyframe2)) ==
                Quaternion(glm::radians(Vector3(45, 15, 30))));
            CHECK(nanoemMotionAccessoryKeyframeGetOpacity(keyframe2) == Approx(0.8));
            CHECK(nanoemMotionAccessoryKeyframeGetScaleFactor(keyframe2) == Approx(0.4));
            CHECK(project2->duration() == 1337);
            CHECK(second->motionDuration(activeAccessory2) == 1337);
        }
    }
}
