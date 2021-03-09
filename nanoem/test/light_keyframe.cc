/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_light_keyframe_basic", "[nanoem]")
{
    CHECK(nanoemMotionLightKeyframeGetColor(NULL));
    CHECK(nanoemMotionLightKeyframeGetDirection(NULL));
    CHECK_FALSE(nanoemMotionLightKeyframeGetKeyframeObject(NULL));
    CHECK_FALSE(nanoemMotionLightKeyframeGetKeyframeObjectMutable(NULL));
    nanoemMotionLightKeyframeDestroy(NULL);
}

TEST_CASE("mutable_light_keyframe_null", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    CHECK_FALSE(nanoemMutableMotionLightKeyframeCreate(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionLightKeyframeCreateAsReference(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionLightKeyframeCreateByFound(NULL, 0, NULL));
    CHECK_FALSE(nanoemMutableMotionLightKeyframeGetOriginObject(NULL));
    CHECK_FALSE(nanoemMutableMotionLightKeyframeGetOriginObject(NULL));
    nanoemMutableMotionLightKeyframeSetColor(NULL, NULL);
    nanoemMutableMotionLightKeyframeSetDirection(NULL, NULL);
    nanoemMutableMotionLightKeyframeSaveToBuffer(NULL, NULL, &status);
    nanoemMutableMotionLightKeyframeDestroy(NULL);
}

TEST_CASE("mutable_light_keyframe_null_value", "[nanoem]")
{
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_light_keyframe_t *keyframe = scope.newLightKeyframe();
    nanoemMutableMotionLightKeyframeSetColor(keyframe, NULL);
    nanoemMutableMotionLightKeyframeSetDirection(keyframe, NULL);
    nanoemMutableMotionLightKeyframeSaveToBuffer(keyframe, NULL, &status);
}

TEST_CASE("mutable_light_keyframe_manipulate", "[nanoem]")
{
    MotionScope scope;
    nanoem_mutable_motion_t *motion = scope.newMotion();
    nanoem_mutable_motion_light_keyframe_t *keyframe = scope.newLightKeyframe();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableMotionAddLightKeyframe(NULL, NULL, 0, &status);
    nanoemMutableMotionAddLightKeyframe(motion, NULL, 0, &status);
    nanoemMutableMotionRemoveLightKeyframe(NULL, NULL, &status);
    nanoemMutableMotionRemoveLightKeyframe(motion, NULL, &status);
    nanoemMutableMotionRemoveLightKeyframe(motion, keyframe, &status);
    CHECK(status == NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_NOT_FOUND);
}

TEST_CASE("mutable_light_keyframe_generate_vmd", "[nanoem]")
{
    static const nanoem_f32_t expected_color[] = { 1, 2, 3, 0 };
    static const nanoem_f32_t expected_direction[] = { 0.4f, 0.5f, 0.6f, 0 };
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_t *mutable_motion = scope.newMotion();
    nanoem_mutable_motion_light_keyframe_t *mutable_keyframe = scope.newLightKeyframe();
    /* create */
    nanoemMutableMotionLightKeyframeSetColor(mutable_keyframe, expected_color);
    nanoemMutableMotionLightKeyframeSetDirection(mutable_keyframe, expected_direction);
    nanoemMutableMotionAddLightKeyframe(mutable_motion, mutable_keyframe, 42, &status);
    CHECK(status == NANOEM_STATUS_SUCCESS);
    SECTION("duplication should reject")
    {
        nanoemMutableMotionAddLightKeyframe(mutable_motion, mutable_keyframe, 42, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_ALREADY_EXISTS);
    }
    SECTION("all keyframe properties should get")
    {
        nanoem_motion_light_keyframe_t *origin_keyframe = nanoemMutableMotionLightKeyframeGetOriginObject(mutable_keyframe);
        CHECK(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionLightKeyframeGetKeyframeObject(origin_keyframe)) == 42);
        CHECK_THAT(nanoemMotionLightKeyframeGetColor(origin_keyframe), Equals(1, 2, 3, 0));
        CHECK_THAT(nanoemMotionLightKeyframeGetDirection(origin_keyframe), Equals(0.4f, 0.5f, 0.6f, 0.0f));
    }
    SECTION("saved keyframe data should be able to parse")
    {
        nanoem_mutable_buffer_t *mutable_buffer = scope.newBuffer();
        nanoemMutableMotionLightKeyframeSaveToBuffer(mutable_keyframe, mutable_buffer, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoem_buffer_t *buffer = scope.newBuffer(mutable_buffer);
        nanoem_motion_t *motion = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoem_motion_light_keyframe_t *generated_keyframe = nanoemMotionLightKeyframeCreate(motion, &status);
        nanoemMotionLightKeyframeParseVMD(generated_keyframe, buffer, 3, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        CHECK(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionLightKeyframeGetKeyframeObject(generated_keyframe)) == 45);
        CHECK_THAT(nanoemMotionLightKeyframeGetColor(generated_keyframe), Equals(1, 2, 3, 0));
        CHECK_THAT(nanoemMotionLightKeyframeGetDirection(generated_keyframe), Equals(0.4f, 0.5f, 0.6f, 0.0f));
        nanoemMotionLightKeyframeDestroy(generated_keyframe);
    }
    SECTION("duplicated remove should reject")
    {
        nanoem_rsize_t num_keyframes;
        nanoem_motion_t *motion = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoemMutableMotionRemoveLightKeyframe(mutable_motion, mutable_keyframe, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMotionGetAllLightKeyframeObjects(motion, &num_keyframes);
        CHECK(num_keyframes == 0);
        nanoemMutableMotionRemoveLightKeyframe(mutable_motion, mutable_keyframe, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_NOT_FOUND);
    }
}

TEST_CASE("mutable_light_keyframe_generate_nmd", "[nanoem]")
{
    static const nanoem_f32_t expected_color[] = { 0.1f, 0.2f, 0.3f, 0.0f };
    static const nanoem_f32_t expected_direction[] = { 0.4f, 0.5f, 0.6f, 0.0f };
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_buffer_t *mutable_buffer = scope.newBuffer();
    {
        nanoem_mutable_motion_t *mutable_motion = scope.newMotion();
        nanoem_motion_t *motion = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoem_mutable_motion_light_keyframe_t *mutable_keyframe = nanoemMutableMotionLightKeyframeCreate(motion, &status);
        nanoemMutableMotionLightKeyframeSetColor(mutable_keyframe, expected_color);
        nanoemMutableMotionLightKeyframeSetDirection(mutable_keyframe, expected_direction);
        nanoemMutableMotionAddLightKeyframe(mutable_motion, mutable_keyframe, 42, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableMotionLightKeyframeDestroy(mutable_keyframe);
        nanoemMutableMotionSaveToBufferNMD(mutable_motion, mutable_buffer, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
    }
    {
        nanoem_motion_t *new_motion = nanoemMutableMotionGetOriginObject(scope.newMotion());
        nanoem_buffer_t *buffer = scope.newBuffer(mutable_buffer);
        nanoemMotionLoadFromBufferNMD(new_motion, buffer, 1, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        const nanoem_motion_light_keyframe_t *generated_keyframe = nanoemMotionFindLightKeyframeObject(new_motion, 43);
        CHECK(generated_keyframe);
        CHECK(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionLightKeyframeGetKeyframeObject(generated_keyframe)) == 43);
        CHECK_THAT(nanoemMotionLightKeyframeGetColor(generated_keyframe), Equals(0.1f, 0.2f, 0.3f, 0));
        CHECK_THAT(nanoemMotionLightKeyframeGetDirection(generated_keyframe), Equals(0.4f, 0.5f, 0.6f, 0.0f));
    }
}
