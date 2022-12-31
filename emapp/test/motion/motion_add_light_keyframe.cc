/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/ILight.h"
#include "emapp/IMotionKeyframeSelection.h"

using namespace nanoem;
using namespace test;

TEST_CASE("motion_add_light_keyframe", "[emapp][motion]")
{
    TestScope scope;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ILight *light = project->globalLight();
        {
            project->seek(1337, true);
            light->setColor(Vector3(0.7, 0.5, 0.3));
            light->setDirection(glm::radians(Vector3(-0.4, 0.9, 0.2)));
        }
        CommandRegistrator registrator(project);
        registrator.registerAddLightKeyframesCommandByCurrentLocalFrameIndex();
        SECTION("adding camera keyframe")
        {
            CHECK(project->lightMotion()->countAllKeyframes() == 2);
            const nanoem_motion_light_keyframe_t *keyframe =
                nanoemMotionFindLightKeyframeObject(project->lightMotion()->data(), 1337);
            CHECK(keyframe);
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetColor(keyframe)), Equals(Vector3(0.7, 0.5, 0.3)));
            CHECK(glm::make_vec3(nanoemMotionLightKeyframeGetDirection(keyframe)) ==
                glm::radians(Vector3(-0.4, 0.9, 0.2)));
            CHECK(project->lightMotion()->selection()->contains(keyframe));
            CHECK(project->duration() == 1337);
            CHECK(project->lightMotion()->isDirty());
        }
        project->handleUndoAction();
        SECTION("undo")
        {
            CHECK(project->lightMotion()->countAllKeyframes() == 1);
            CHECK_FALSE(nanoemMotionFindLightKeyframeObject(project->lightMotion()->data(), 1337));
            CHECK(project->duration() == project->baseDuration());
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleRedoAction();
        SECTION("redo")
        {
            CHECK(project->lightMotion()->countAllKeyframes() == 2);
            const nanoem_motion_light_keyframe_t *keyframe =
                nanoemMotionFindLightKeyframeObject(project->lightMotion()->data(), 1337);
            CHECK(keyframe);
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetColor(keyframe)), Equals(Vector3(0.7, 0.5, 0.3)));
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetDirection(keyframe)),
                EqualsRadians(Vector3(-0.4, 0.9, 0.2)));
            CHECK(project->lightMotion()->selection()->contains(keyframe));
            CHECK(project->duration() == 1337);
            CHECK_FALSE(scope.hasAnyError());
        }
        SECTION("recovery")
        {
            ProjectPtr second = scope.createProject();
            Project *project2 = second.get()->m_project;
            scope.recover(project2);
            CHECK(project2->lightMotion()->countAllKeyframes() == 2);
            const nanoem_motion_light_keyframe_t *keyframe =
                nanoemMotionFindLightKeyframeObject(project2->lightMotion()->data(), 1337);
            CHECK(keyframe);
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetColor(keyframe)), Equals(Vector3(0.7, 0.5, 0.3)));
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetDirection(keyframe)),
                EqualsRadians(Vector3(-0.4, 0.9, 0.2)));
            CHECK(project2->duration() == 1337);
            CHECK_FALSE(scope.hasAnyError());
        }
    }
}
