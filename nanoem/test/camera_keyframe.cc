/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_camera_keyframe_basic", "[nanoem]")
{
    CHECK(nanoemMotionCameraKeyframeGetAngle(NULL));
    CHECK(nanoemMotionCameraKeyframeGetDistance(NULL) == Approx(0));
    CHECK(nanoemMotionCameraKeyframeGetFov(NULL) == 0);
    CHECK(nanoemMotionCameraKeyframeGetLookAt(NULL));
    CHECK_FALSE(nanoemMotionCameraKeyframeGetKeyframeObject(NULL));
    CHECK_FALSE(nanoemMotionCameraKeyframeGetKeyframeObjectMutable(NULL));
    CHECK(nanoemMotionCameraKeyframeGetStageIndex(NULL) == 0);
    CHECK(nanoemMotionCameraKeyframeGetOutsideParent(NULL) == NULL);
    CHECK(nanoemMotionCameraKeyframeGetOutsideParentMutable(NULL) == NULL);
    nanoemMotionCameraKeyframeDestroy(NULL);
}

TEST_CASE("mutable_camera_keyframe_null", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    CHECK_FALSE(nanoemMutableMotionCameraKeyframeCreate(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionCameraKeyframeCreateAsReference(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionCameraKeyframeCreateByFound(NULL, 0, NULL));
    CHECK_FALSE(nanoemMutableMotionCameraKeyframeGetOriginObject(NULL));
    nanoemMutableMotionCameraKeyframeSetAngle(NULL, NULL);
    nanoemMutableMotionCameraKeyframeSetDistance(NULL, 0);
    nanoemMutableMotionCameraKeyframeSetFov(NULL, 0);
    nanoemMutableMotionCameraKeyframeSetInterpolation(NULL, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM, NULL);
    nanoemMutableMotionCameraKeyframeSetLookAt(NULL, NULL);
    nanoemMutableMotionCameraKeyframeSetPerspectiveView(NULL, nanoem_false);
    nanoemMutableMotionCameraKeyframeSetStageIndex(NULL, 0);
    nanoemMutableMotionCameraKeyframeSetOutsideParent(NULL, NULL, NULL);
    nanoemMutableMotionCameraKeyframeSaveToBuffer(NULL, NULL, &status);
    nanoemMutableMotionCameraKeyframeDestroy(NULL);
}

TEST_CASE("mutable_camera_keyframe_null_value", "[nanoem]")
{
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_camera_keyframe_t *keyframe = scope.newCameraKeyframe();
    nanoemMutableMotionCameraKeyframeSetAngle(keyframe, NULL);
    nanoemMutableMotionCameraKeyframeSetDistance(keyframe, 0);
    nanoemMutableMotionCameraKeyframeSetFov(keyframe, 0);
    nanoemMutableMotionCameraKeyframeSetInterpolation(keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM, NULL);
    nanoemMutableMotionCameraKeyframeSetLookAt(keyframe, NULL);
    nanoemMutableMotionCameraKeyframeSetPerspectiveView(keyframe, nanoem_false);
    nanoemMutableMotionCameraKeyframeSetStageIndex(keyframe, 0);
    nanoemMutableMotionCameraKeyframeSetOutsideParent(keyframe, NULL, NULL);
    nanoemMutableMotionCameraKeyframeSaveToBuffer(keyframe, NULL, &status);
}

TEST_CASE("mutable_camera_keyframe_manipulate", "[nanoem]")
{
    MotionScope scope;
    nanoem_mutable_motion_t *motion = scope.newMotion();
    nanoem_mutable_motion_camera_keyframe_t *keyframe = scope.newCameraKeyframe();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableMotionAddCameraKeyframe(NULL, NULL, 0, &status);
    nanoemMutableMotionAddCameraKeyframe(motion, NULL, 0, &status);
    nanoemMutableMotionRemoveCameraKeyframe(NULL, NULL, &status);
    nanoemMutableMotionRemoveCameraKeyframe(motion, NULL, &status);
    nanoemMutableMotionRemoveCameraKeyframe(motion, keyframe, &status);
    CHECK(status == NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_NOT_FOUND);
}

TEST_CASE("mutable_camera_keyframe_generate_vmd", "[nanoem]")
{
    static const nanoem_u8_t expected_interpolation[] = { 12, 24, 36, 48 };
    static const nanoem_f32_t expected_angle[] = { 1, 2, 3, 0 };
    static const nanoem_f32_t expected_lookat[] = { 4, 5, 6, 0 };
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_t *mutable_motion = scope.newMotion();
    /* create */
    nanoem_mutable_motion_camera_keyframe_t *mutable_keyframe = scope.newCameraKeyframe();
    nanoemMutableMotionCameraKeyframeSetLookAt(mutable_keyframe, expected_lookat);
    nanoemMutableMotionCameraKeyframeSetAngle(mutable_keyframe, expected_angle);
    nanoemMutableMotionCameraKeyframeSetDistance(mutable_keyframe, 42);
    nanoemMutableMotionCameraKeyframeSetFov(mutable_keyframe, 24.0f);
    nanoemMutableMotionCameraKeyframeSetPerspectiveView(mutable_keyframe, nanoem_false);
    for (int i = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
         i < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
        nanoemMutableMotionCameraKeyframeSetInterpolation(
            mutable_keyframe, (nanoem_motion_camera_keyframe_interpolation_type_t) i, expected_interpolation);
    }
    /* add */
    nanoemMutableMotionAddCameraKeyframe(mutable_motion, mutable_keyframe, 42, &status);
    CHECK(status == NANOEM_STATUS_SUCCESS);
    SECTION("duplication should reject")
    {
        nanoemMutableMotionAddCameraKeyframe(mutable_motion, mutable_keyframe, 42, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_ALREADY_EXISTS);
    }
    SECTION("all keyframe properties should get")
    {
        nanoem_motion_camera_keyframe_t *origin_keyframe = nanoemMutableMotionCameraKeyframeGetOriginObject(mutable_keyframe);
        CHECK(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(origin_keyframe)) == 42);
        CHECK_THAT(nanoemMotionCameraKeyframeGetAngle(origin_keyframe), Equals(1, 2, 3, 0));
        CHECK_THAT(nanoemMotionCameraKeyframeGetLookAt(origin_keyframe), Equals(4, 5, 6, 0));
        CHECK(nanoemMotionCameraKeyframeGetDistance(origin_keyframe) == Approx(42.0f));
        CHECK(nanoemMotionCameraKeyframeGetFov(origin_keyframe) == 24);
        CHECK_FALSE(nanoemMotionCameraKeyframeIsPerspectiveView(origin_keyframe));
        for (int i = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
             i < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
            CHECK_THAT(
                nanoemMotionCameraKeyframeGetInterpolation(origin_keyframe, (nanoem_motion_camera_keyframe_interpolation_type_t) i),
                EqualsU8(12, 24, 36, 48));
        }
    }
    SECTION("saved keyframe data should be able to parse")
    {
        nanoem_mutable_buffer_t *mutable_buffer = scope.newBuffer();
        nanoemMutableMotionCameraKeyframeSaveToBuffer(mutable_keyframe, mutable_buffer, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoem_buffer_t *buffer = scope.newBuffer(mutable_buffer);
        nanoem_motion_t *origin = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoem_motion_camera_keyframe_t *generated_keyframe = nanoemMotionCameraKeyframeCreate(origin, &status);
        nanoemMotionCameraKeyframeParseVMD(generated_keyframe, buffer, 2, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        CHECK(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(generated_keyframe)) == 44);
        CHECK_THAT(nanoemMotionCameraKeyframeGetAngle(generated_keyframe), Equals(1, 2, 3, 0));
        CHECK_THAT(nanoemMotionCameraKeyframeGetLookAt(generated_keyframe), Equals(4, 5, 6, 0));
        CHECK(nanoemMotionCameraKeyframeGetDistance(generated_keyframe) == Approx(42.0f));
        CHECK(nanoemMotionCameraKeyframeGetFov(generated_keyframe) == 24);
        CHECK_FALSE(nanoemMotionCameraKeyframeIsPerspectiveView(generated_keyframe));
        for (int i = 0; i < 6; i++) {
            CHECK_THAT(nanoemMotionCameraKeyframeGetInterpolation(
                           generated_keyframe, (nanoem_motion_camera_keyframe_interpolation_type_t) i),
                EqualsU8(12, 24, 36, 48));
        }
        nanoemMotionCameraKeyframeDestroy(generated_keyframe);
    }
    SECTION("duplicated remove should reject")
    {
        nanoem_rsize_t num_keyframes;
        nanoem_motion_t *origin = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoemMutableMotionRemoveCameraKeyframe(mutable_motion, mutable_keyframe, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMotionGetAllCameraKeyframeObjects(origin, &num_keyframes);
        CHECK(num_keyframes == 0);
        nanoemMutableMotionRemoveCameraKeyframe(mutable_motion, mutable_keyframe, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_NOT_FOUND);
    }
}

TEST_CASE("mutable_camera_keyframe_generate_nmd", "[nanoem]")
{
    static const nanoem_u8_t expected_interpolation[] = { 12, 24, 36, 48 };
    static const nanoem_f32_t expected_angle[] = { 1, 2, 3, 0 };
    static const nanoem_f32_t expected_lookat[] = { 4, 5, 6, 0 };
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_buffer_t *mutable_buffer = scope.newBuffer();
    {
        nanoem_mutable_motion_t *mutable_motion = scope.newMotion();
        nanoem_motion_t *motion = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoem_mutable_motion_camera_keyframe_t *mutable_keyframe = nanoemMutableMotionCameraKeyframeCreate(motion, &status);
        nanoemMutableMotionCameraKeyframeSetAngle(mutable_keyframe, expected_angle);
        nanoemMutableMotionCameraKeyframeSetLookAt(mutable_keyframe, expected_lookat);
        for (int i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
             i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
            nanoemMutableMotionCameraKeyframeSetInterpolation(
                mutable_keyframe, (nanoem_motion_camera_keyframe_interpolation_type_t) i, expected_interpolation);
        }
        nanoemMutableMotionCameraKeyframeSetDistance(mutable_keyframe, 42);
        nanoemMutableMotionCameraKeyframeSetFov(mutable_keyframe, 21);
        nanoemMutableMotionCameraKeyframeSetPerspectiveView(mutable_keyframe, nanoem_true);
        nanoemMutableMotionCameraKeyframeSetStageIndex(mutable_keyframe, 7);
        nanoemMutableMotionAddCameraKeyframe(mutable_motion, mutable_keyframe, 42, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableMotionCameraKeyframeDestroy(mutable_keyframe);
        nanoemMutableMotionSaveToBufferNMD(mutable_motion, mutable_buffer, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
    }
    {
        nanoem_motion_t *new_motion = nanoemMutableMotionGetOriginObject(scope.newMotion());
        nanoem_buffer_t *buffer = scope.newBuffer(mutable_buffer);
        nanoemMotionLoadFromBufferNMD(new_motion, buffer, 1, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        const nanoem_motion_camera_keyframe_t *generated_keyframe = nanoemMotionFindCameraKeyframeObject(new_motion, 43);
        CHECK(generated_keyframe);
        CHECK(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(generated_keyframe)) == 43);
        CHECK_THAT(nanoemMotionCameraKeyframeGetAngle(generated_keyframe), Equals(1, 2, 3, 0));
        CHECK_THAT(nanoemMotionCameraKeyframeGetLookAt(generated_keyframe), Equals(4, 5, 6, 0));
        for (int i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
             i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
            CHECK_THAT(nanoemMotionCameraKeyframeGetInterpolation(
                           generated_keyframe, (nanoem_motion_camera_keyframe_interpolation_type_t) i),
                EqualsU8(12, 24, 36, 48));
        }
        CHECK(nanoemMotionCameraKeyframeGetDistance(generated_keyframe) == Approx(42));
        CHECK(nanoemMotionCameraKeyframeGetFov(generated_keyframe) == 21);
        CHECK(nanoemMotionCameraKeyframeGetStageIndex(generated_keyframe) == 7);
        CHECK(nanoemMotionCameraKeyframeIsPerspectiveView(generated_keyframe));
    }
}
