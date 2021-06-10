/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/Constants.h"
#include "emapp/ICamera.h"

using namespace nanoem;
using namespace test;

TEST_CASE("motion_remove_camera_keyframe", "[emapp][motion]")
{
    TestScope scope;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ICamera *camera = project->globalCamera();
        {
            project->seek(1337, true);
            camera->setLookAt(Vector3(84, 126, 42));
            camera->setAngle(glm::radians(Vector3(30, 45, 15)));
            camera->setFovRadians(glm::radians(32.0f));
            camera->setDistance(767);
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
        CommandRegistrator registrator(project);
        registrator.registerAddCameraKeyframesCommandByCurrentLocalFrameIndex();
        registrator.registerRemoveCameraKeyframesCommandByCurrentLocalFrameIndex();
        SECTION("removing camera keyframe")
        {
            CHECK(project->cameraMotion()->countAllKeyframes() == 1);
            CHECK_FALSE(nanoemMotionFindCameraKeyframeObject(project->cameraMotion()->data(), 1337));
            CHECK(project->duration() == project->baseDuration());
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
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(keyframe)), Equals(Vector3(84, 126, 42)));
            CHECK_THAT(
                glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(keyframe)), EqualsRadians(Vector3(30, 45, 15)));
            CHECK(nanoemMotionCameraKeyframeGetFov(keyframe) == 32);
            CHECK(nanoemMotionCameraKeyframeGetDistance(keyframe) == Approx(767));
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
            CHECK(project->duration() == 1337);
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleRedoAction();
        SECTION("redo")
        {
            CHECK(project->cameraMotion()->countAllKeyframes() == 1);
            CHECK_FALSE(nanoemMotionFindCameraKeyframeObject(project->cameraMotion()->data(), 1337));
            CHECK(project->duration() == project->baseDuration());
            CHECK_FALSE(scope.hasAnyError());
        }
        SECTION("reconver")
        {
            ProjectPtr second = scope.createProject();
            Project *project2 = second.get()->m_project;
            scope.recover(project2);
            CHECK(project2->cameraMotion()->countAllKeyframes() == 1);
            CHECK_FALSE(nanoemMotionFindCameraKeyframeObject(project2->cameraMotion()->data(), 1337));
            CHECK(project2->duration() == project2->baseDuration());
            CHECK_FALSE(scope.hasAnyError());
        }
    }
}

TEST_CASE("motion_remove_camera_keyframe_at_zero", "[emapp][motion]")
{
    TestScope scope;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ICamera *camera = project->globalCamera();
        {
            camera->setLookAt(Vector3(84, 126, 42));
            camera->setAngle(glm::radians(Vector3(30, 45, 15)));
            camera->setFovRadians(glm::radians(32.0f));
            camera->setDistance(767);
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
        CommandRegistrator registrator(project);
        registrator.registerAddCameraKeyframesCommandByCurrentLocalFrameIndex();
        registrator.registerRemoveCameraKeyframesCommandByCurrentLocalFrameIndex();
        SECTION("removing camera keyframe")
        {
            CHECK(project->cameraMotion()->countAllKeyframes() == 1);
            const nanoem_motion_camera_keyframe_t *keyframe =
                nanoemMotionFindCameraKeyframeObject(project->cameraMotion()->data(), 0);
            CHECK(keyframe);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(keyframe)), Equals(Constants::kZeroV3));
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(keyframe)), Equals(Constants::kZeroV3));
            CHECK(nanoemMotionCameraKeyframeGetFov(keyframe) == 30);
            CHECK(nanoemMotionCameraKeyframeGetDistance(keyframe) == Approx(-45));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE)) == Vector4U8(20, 20, 107, 107));
            CHECK(project->duration() == project->baseDuration());
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleUndoAction();
        SECTION("undo")
        {
            CHECK(project->cameraMotion()->countAllKeyframes() == 1);
            const nanoem_motion_camera_keyframe_t *keyframe =
                nanoemMotionFindCameraKeyframeObject(project->cameraMotion()->data(), 0);
            CHECK(keyframe);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(keyframe)), Equals(Vector3(84, 126, 42)));
            CHECK_THAT(
                glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(keyframe)), EqualsRadians(Vector3(30, 45, 15)));
            CHECK(nanoemMotionCameraKeyframeGetFov(keyframe) == 32);
            CHECK(nanoemMotionCameraKeyframeGetDistance(keyframe) == Approx(767));
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
            CHECK(project->duration() == project->baseDuration());
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleRedoAction();
        SECTION("redo")
        {
            CHECK(project->cameraMotion()->countAllKeyframes() == 1);
            const nanoem_motion_camera_keyframe_t *keyframe =
                nanoemMotionFindCameraKeyframeObject(project->cameraMotion()->data(), 0);
            CHECK(keyframe);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(keyframe)), Equals(Constants::kZeroV3));
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(keyframe)), Equals(Constants::kZeroV3));
            CHECK(nanoemMotionCameraKeyframeGetFov(keyframe) == 30);
            CHECK(nanoemMotionCameraKeyframeGetDistance(keyframe) == Approx(-45));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE)) == Vector4U8(20, 20, 107, 107));
            CHECK(project->duration() == project->baseDuration());
            CHECK_FALSE(scope.hasAnyError());
        }
        SECTION("recover")
        {
            ProjectPtr second = scope.createProject();
            Project *project2 = second.get()->m_project;
            scope.recover(project2);
            CHECK(project2->cameraMotion()->countAllKeyframes() == 1);
            const nanoem_motion_camera_keyframe_t *keyframe =
                nanoemMotionFindCameraKeyframeObject(project2->cameraMotion()->data(), 0);
            CHECK(keyframe);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(keyframe)), Equals(Constants::kZeroV3));
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(keyframe)), Equals(Constants::kZeroV3));
            CHECK(nanoemMotionCameraKeyframeGetFov(keyframe) == 30);
            CHECK(nanoemMotionCameraKeyframeGetDistance(keyframe) == Approx(-45));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(
                      keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE)) == Vector4U8(20, 20, 107, 107));
            CHECK(project2->duration() == project->baseDuration());
            CHECK_FALSE(scope.hasAnyError());
        }
    }
}
