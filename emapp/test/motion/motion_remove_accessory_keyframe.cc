/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/Accessory.h"
#include "emapp/CommandRegistrator.h"

using namespace nanoem;
using namespace test;

TEST_CASE("motion_remove_accessory_keyframe", "[emapp][motion]")
{
    TestScope scope;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Accessory *activeAccessory = first->createAccessory();
        Accessory *other1Accessory = first->createAccessory();
        Accessory *other2Accessory = first->createAccessory();
        Model *bindingModel = first->createModel();
        {
            project->addAccessory(activeAccessory);
            project->addAccessory(other1Accessory);
            project->addAccessory(other2Accessory);
            project->setActiveAccessory(activeAccessory);
            project->addModel(bindingModel);
            project->seek(1337, true);
            activeAccessory->setTranslation(Vector3(84, 126, 42));
            activeAccessory->setOrientation(Vector3(30, 45, 15));
            activeAccessory->setOpacity(0.8f);
            activeAccessory->setScaleFactor(0.4f);
        }
        CommandRegistrator registrator(project);
        registrator.registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(activeAccessory);
        registrator.registerRemoveAccessoryKeyframesCommandByCurrentLocalFrameIndex(activeAccessory);
        SECTION("removing active accessory keyframe")
        {
            CHECK(first->countAllAccessoryKeyframes(activeAccessory) == 1);
            CHECK(first->countAllAccessoryKeyframes(other1Accessory) == 1);
            CHECK(first->countAllAccessoryKeyframes(other2Accessory) == 1);
            CHECK_FALSE(nanoemMotionFindAccessoryKeyframeObject(project->resolveMotion(activeAccessory)->data(), 1337));
            CHECK_FALSE(nanoemMotionFindAccessoryKeyframeObject(project->resolveMotion(other1Accessory)->data(), 1337));
            CHECK_FALSE(nanoemMotionFindAccessoryKeyframeObject(project->resolveMotion(other2Accessory)->data(), 1337));
            CHECK(project->duration() == project->baseDuration());
            CHECK(first->motionDuration(activeAccessory) == 0);
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
            CHECK(first->countAllAccessoryKeyframes(activeAccessory) == 2);
            CHECK(first->countAllAccessoryKeyframes(other1Accessory) == 1);
            CHECK(first->countAllAccessoryKeyframes(other2Accessory) == 1);
            const nanoem_motion_accessory_keyframe_t *keyframe =
                nanoemMotionFindAccessoryKeyframeObject(project->resolveMotion(activeAccessory)->data(), 1337);
            CHECK(keyframe);
            CHECK_FALSE(nanoemMotionFindAccessoryKeyframeObject(project->resolveMotion(other1Accessory)->data(), 1337));
            CHECK_FALSE(nanoemMotionFindAccessoryKeyframeObject(project->resolveMotion(other2Accessory)->data(), 1337));
            CHECK_THAT(
                glm::make_vec3(nanoemMotionAccessoryKeyframeGetTranslation(keyframe)), Equals(Vector3(84, 126, 42)));
            // cr_expect_eq(glm::make_quat(nanoemMotionAccessoryKeyframeGetOrientation(keyframe)),
            // Quaternion(glm::radians(Vector3(30, 45, 15))));
            CHECK(nanoemMotionAccessoryKeyframeGetOpacity(keyframe) == Approx(0.8));
            CHECK(nanoemMotionAccessoryKeyframeGetScaleFactor(keyframe) == Approx(0.4));
            CHECK(project->duration() == 1337);
            CHECK(first->motionDuration(activeAccessory) == 1337);
            CHECK(first->motionDuration(other1Accessory) == 0);
            CHECK(first->motionDuration(other2Accessory) == 0);
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleRedoAction();
        SECTION("redo")
        {
            CHECK(first->countAllAccessoryKeyframes(activeAccessory) == 1);
            CHECK(first->countAllAccessoryKeyframes(other1Accessory) == 1);
            CHECK(first->countAllAccessoryKeyframes(other2Accessory) == 1);
            CHECK_FALSE(nanoemMotionFindAccessoryKeyframeObject(project->resolveMotion(activeAccessory)->data(), 1337));
            CHECK_FALSE(nanoemMotionFindAccessoryKeyframeObject(project->resolveMotion(other1Accessory)->data(), 1337));
            CHECK_FALSE(nanoemMotionFindAccessoryKeyframeObject(project->resolveMotion(other2Accessory)->data(), 1337));
            CHECK(project->duration() == project->baseDuration());
            CHECK(first->motionDuration(activeAccessory) == 0);
            CHECK(first->motionDuration(other1Accessory) == 0);
            CHECK(first->motionDuration(other2Accessory) == 0);
            CHECK_FALSE(scope.hasAnyError());
        }
        if (0) {
            ProjectPtr second = scope.createProject();
            Project *project2 = second.get()->m_project;
            scope.recover(project2);
            Accessory *activeAccessory2 = project2->activeAccessory();
            CHECK(second->countAllAccessoryKeyframes(activeAccessory2) == 1);
            CHECK_FALSE(
                nanoemMotionFindAccessoryKeyframeObject(project2->resolveMotion(activeAccessory2)->data(), 1337));
            CHECK(project2->duration() == project2->baseDuration());
            CHECK(second->motionDuration(activeAccessory2) == 0);
        }
    }
}
