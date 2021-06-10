/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/ICamera.h"

using namespace nanoem;
using namespace test;

TEST_CASE("motion_update_camera_keyframe", "[emapp][motion]")
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
        {
            camera->setLookAt(Vector3(126, 84, 42));
            camera->setAngle(glm::radians(Vector3(45, 30, 15)));
            camera->setFovRadians(glm::radians(36.0f));
            camera->setDistance(512);
            camera->setBezierControlPoints(
                NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X, Vector4U8(8, 6, 4, 2));
            camera->setBezierControlPoints(
                NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y, Vector4U8(9, 7, 5, 3));
            camera->setBezierControlPoints(
                NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z, Vector4U8(17, 15, 13, 11));
            camera->setBezierControlPoints(
                NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE, Vector4U8(18, 16, 14, 12));
            camera->setBezierControlPoints(
                NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV, Vector4U8(30, 28, 26, 24));
            camera->setBezierControlPoints(
                NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE, Vector4U8(31, 29, 27, 25));
        }
        registrator.registerAddCameraKeyframesCommandByCurrentLocalFrameIndex();
        SECTION("updating current camera keyframe")
        {
            CHECK(project->cameraMotion()->countAllKeyframes() == 2);
            const nanoem_motion_camera_keyframe_t *keyframe =
                nanoemMotionFindCameraKeyframeObject(project->cameraMotion()->data(), 1337);
            CHECK(keyframe);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(keyframe)), Equals(Vector3(126, 84, 42)));
            CHECK_THAT(
                glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(keyframe)), EqualsRadians(Vector3(45, 30, 15)));
            CHECK(nanoemMotionCameraKeyframeGetFov(keyframe) == 36);
            CHECK(nanoemMotionCameraKeyframeGetDistance(keyframe) == Approx(-512));
            CHECK(project->duration() == 1337);
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X)) == Vector4U8(8, 6, 4, 2));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y)) == Vector4U8(9, 7, 5, 3));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z)) == Vector4U8(17, 15, 13, 11));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE)) == Vector4U8(18, 16, 14, 12));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV)) == Vector4U8(30, 28, 26, 24));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE)) == Vector4U8(31, 29, 27, 25));
            CHECK(project->cameraMotion()->isDirty());
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleUndoAction();
        SECTION("undo")
        {
            CHECK(project->cameraMotion()->countAllKeyframes() == 2);
            const nanoem_motion_camera_keyframe_t *keyframe =
                nanoemMotionFindCameraKeyframeObject(project->cameraMotion()->data(), 1337);
            CHECK(keyframe);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(keyframe)), Equals(Vector3(42, 84, 126)));
            CHECK_THAT(
                glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(keyframe)), EqualsRadians(Vector3(15, 30, 45)));
            CHECK(nanoemMotionCameraKeyframeGetFov(keyframe) == 35);
            CHECK(nanoemMotionCameraKeyframeGetDistance(keyframe) == Approx(511));
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
        project->handleRedoAction();
        SECTION("redo")
        {
            CHECK(project->cameraMotion()->countAllKeyframes() == 2);
            const nanoem_motion_camera_keyframe_t *keyframe =
                nanoemMotionFindCameraKeyframeObject(project->cameraMotion()->data(), 1337);
            CHECK(keyframe);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(keyframe)), Equals(Vector3(126, 84, 42)));
            CHECK_THAT(
                glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(keyframe)), EqualsRadians(Vector3(45, 30, 15)));
            CHECK(nanoemMotionCameraKeyframeGetFov(keyframe) == 36);
            CHECK(nanoemMotionCameraKeyframeGetDistance(keyframe) == Approx(-512));
            CHECK(project->duration() == 1337);
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X)) == Vector4U8(8, 6, 4, 2));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y)) == Vector4U8(9, 7, 5, 3));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z)) == Vector4U8(17, 15, 13, 11));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE)) == Vector4U8(18, 16, 14, 12));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV)) == Vector4U8(30, 28, 26, 24));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE)) == Vector4U8(31, 29, 27, 25));
            CHECK_FALSE(scope.hasAnyError());
        }
        SECTION("recover")
        {
            ProjectPtr second = scope.createProject();
            Project *project2 = second.get()->m_project;
            scope.recover(project2);
            CHECK(project2->cameraMotion()->countAllKeyframes() == 2);
            const nanoem_motion_camera_keyframe_t *keyframe =
                nanoemMotionFindCameraKeyframeObject(project2->cameraMotion()->data(), 1337);
            CHECK(keyframe);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(keyframe)), Equals(Vector3(126, 84, 42)));
            CHECK_THAT(
                glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(keyframe)), EqualsRadians(Vector3(45, 30, 15)));
            CHECK(nanoemMotionCameraKeyframeGetFov(keyframe) == 36);
            CHECK(nanoemMotionCameraKeyframeGetDistance(keyframe) == Approx(-512));
            CHECK(project2->duration() == 1337);
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X)) == Vector4U8(8, 6, 4, 2));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y)) == Vector4U8(9, 7, 5, 3));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z)) == Vector4U8(17, 15, 13, 11));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE)) == Vector4U8(18, 16, 14, 12));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV)) == Vector4U8(30, 28, 26, 24));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE)) == Vector4U8(31, 29, 27, 25));
            CHECK_FALSE(scope.hasAnyError());
        }
    }
}
