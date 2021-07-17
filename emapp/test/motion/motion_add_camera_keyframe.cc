/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/ICamera.h"

using namespace nanoem;
using namespace test;

TEST_CASE("motion_add_camera_keyframe", "[emapp][motion]")
{
    TestScope scope;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ICamera *camera = project->globalCamera();
        {
            project->seek(1337, true);
            camera->setLookAt(Vector3(42, 84, 126));
            camera->setAngle(glm::radians(Vector3(15, 30, 45)));
            camera->setFovRadians(glm::radians(35.0f));
            camera->setDistance(511);
            camera->setBezierControlPoints(
                NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X, Vector4U8(2, 4, 6, 8));
            camera->setBezierControlPoints(
                NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y, Vector4U8(3, 5, 7, 9));
            camera->setBezierControlPoints(
                NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z, Vector4U8(11, 13, 15, 17));
            camera->setBezierControlPoints(
                NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE, Vector4U8(12, 14, 16, 18));
            camera->setBezierControlPoints(
                NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV, Vector4U8(24, 26, 28, 30));
            camera->setBezierControlPoints(
                NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE, Vector4U8(25, 27, 29, 31));
        }
        CommandRegistrator registrator(project);
        registrator.registerAddCameraKeyframesCommandByCurrentLocalFrameIndex();
        SECTION("adding camera keyframe")
        {
            CHECK(project->cameraMotion()->countAllKeyframes() == 2);
            const nanoem_motion_camera_keyframe_t *keyframe =
                nanoemMotionFindCameraKeyframeObject(project->cameraMotion()->data(), 1337);
            CHECK(keyframe);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(keyframe)), Equals(Vector3(42, 84, 126)));
            CHECK_THAT(
                glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(keyframe)), EqualsRadians(Vector3(-15, 30, 45)));
            CHECK(nanoemMotionCameraKeyframeGetFov(keyframe) == 35);
            CHECK(nanoemMotionCameraKeyframeGetDistance(keyframe) == Approx(-511));
            CHECK(project->duration() == 1337);
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X)) == Vector4U8(2, 4, 6, 8));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y)) == Vector4U8(3, 5, 7, 9));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z)) == Vector4U8(11, 13, 15, 17));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE)) == Vector4U8(12, 14, 16, 18));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV)) == Vector4U8(24, 26, 28, 30));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE)) == Vector4U8(25, 27, 29, 31));
            CHECK(project->cameraMotion()->isDirty());
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleUndoAction();
        SECTION("undo")
        {
            CHECK(project->cameraMotion()->countAllKeyframes() == 1);
            CHECK_FALSE(nanoemMotionFindCameraKeyframeObject(project->cameraMotion()->data(), 1337));
            CHECK(project->duration() == project->baseDuration());
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleRedoAction();
        SECTION("redo")
        {
            CHECK(project->cameraMotion()->countAllKeyframes() == 2);
            const nanoem_motion_camera_keyframe_t *keyframe =
                nanoemMotionFindCameraKeyframeObject(project->cameraMotion()->data(), 1337);
            CHECK(keyframe);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(keyframe)), Equals(Vector3(42, 84, 126)));
            CHECK_THAT(
                glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(keyframe)), EqualsRadians(Vector3(-15, 30, 45)));
            CHECK(nanoemMotionCameraKeyframeGetFov(keyframe) == 35);
            CHECK(nanoemMotionCameraKeyframeGetDistance(keyframe) == Approx(-511));
            CHECK(project->duration() == 1337);
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X)) == Vector4U8(2, 4, 6, 8));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y)) == Vector4U8(3, 5, 7, 9));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z)) == Vector4U8(11, 13, 15, 17));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE)) == Vector4U8(12, 14, 16, 18));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV)) == Vector4U8(24, 26, 28, 30));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE)) == Vector4U8(25, 27, 29, 31));
            CHECK_FALSE(scope.hasAnyError());
        }
        SECTION("recovery")
        {
            ProjectPtr second = scope.createProject();
            Project *project2 = second.get()->m_project;
            scope.recover(project2);
            CHECK(project2->cameraMotion()->countAllKeyframes() == 2);
            const nanoem_motion_camera_keyframe_t *keyframe =
                nanoemMotionFindCameraKeyframeObject(project2->cameraMotion()->data(), 1337);
            CHECK(keyframe);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(keyframe)), Equals(Vector3(42, 84, 126)));
            CHECK_THAT(
                glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(keyframe)), EqualsRadians(Vector3(-15, 30, 45)));
            CHECK(nanoemMotionCameraKeyframeGetFov(keyframe) == 35);
            CHECK(nanoemMotionCameraKeyframeGetDistance(keyframe) == Approx(-511));
            CHECK(project2->duration() == 1337);
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X)) == Vector4U8(2, 4, 6, 8));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y)) == Vector4U8(3, 5, 7, 9));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z)) == Vector4U8(11, 13, 15, 17));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE)) == Vector4U8(12, 14, 16, 18));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV)) == Vector4U8(24, 26, 28, 30));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE)) == Vector4U8(25, 27, 29, 31));
            CHECK_FALSE(scope.hasAnyError());
        }
    }
}
