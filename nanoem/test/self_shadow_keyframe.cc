/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_self_shadow_keyframe_basic", "[nanoem]")
{
    CHECK(nanoemMotionSelfShadowKeyframeGetMode(NULL) == 0);
    CHECK(nanoemMotionSelfShadowKeyframeGetDistance(NULL) == Approx(0));
    CHECK_FALSE(nanoemMotionSelfShadowKeyframeGetKeyframeObject(NULL));
    CHECK_FALSE(nanoemMotionSelfShadowKeyframeGetKeyframeObjectMutable(NULL));
    nanoemMotionSelfShadowKeyframeDestroy(NULL);
}

TEST_CASE("mutable_self_shadow_keyframe_null", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    CHECK_FALSE(nanoemMutableMotionSelfShadowKeyframeCreate(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionSelfShadowKeyframeCreateAsReference(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionSelfShadowKeyframeCreateByFound(NULL, 0, NULL));
    CHECK_FALSE(nanoemMutableMotionSelfShadowKeyframeGetOriginObject(NULL));
    CHECK_FALSE(nanoemMutableMotionSelfShadowKeyframeGetOriginObject(NULL));
    nanoemMutableMotionSelfShadowKeyframeSetDistance(NULL, 0);
    nanoemMutableMotionSelfShadowKeyframeSetMode(NULL, 0);
    nanoemMutableMotionSelfShadowKeyframeSaveToBuffer(NULL, NULL, &status);
    nanoemMutableMotionSelfShadowKeyframeDestroy(NULL);
}

TEST_CASE("mutable_self_shadow_keyframe_null_value", "[nanoem]")
{
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_self_shadow_keyframe_t *keyframe = scope.newSelfShadowKeyframe();
    nanoemMutableMotionSelfShadowKeyframeSetDistance(keyframe, 0);
    nanoemMutableMotionSelfShadowKeyframeSetMode(keyframe, 0);
    nanoemMutableMotionSelfShadowKeyframeSaveToBuffer(keyframe, NULL, &status);
}

TEST_CASE("mutable_self_shadow_keyframe_manipulate", "[nanoem]")
{
    MotionScope scope;
    nanoem_mutable_motion_t *motion = scope.newMotion();
    nanoem_mutable_motion_self_shadow_keyframe_t *keyframe = scope.newSelfShadowKeyframe();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableMotionAddSelfShadowKeyframe(NULL, NULL, 0, &status);
    nanoemMutableMotionAddSelfShadowKeyframe(motion, NULL, 0, &status);
    nanoemMutableMotionRemoveSelfShadowKeyframe(NULL, NULL, &status);
    nanoemMutableMotionRemoveSelfShadowKeyframe(motion, NULL, &status);
    nanoemMutableMotionRemoveSelfShadowKeyframe(motion, keyframe, &status);
    CHECK(status == NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_NOT_FOUND);
}

TEST_CASE("mutable_self_shadow_keyframe_generate_vmd", "[nanoem]")
{
    MotionScope scope;
    nanoem_mutable_motion_t *mutable_motion = scope.newMotion();
    nanoem_mutable_motion_self_shadow_keyframe_t *mutable_keyframe = scope.newSelfShadowKeyframe();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableMotionSelfShadowKeyframeSetDistance(mutable_keyframe, 8875.0f);
    nanoemMutableMotionSelfShadowKeyframeSetMode(mutable_keyframe, 1);
    /* add */
    nanoemMutableMotionAddSelfShadowKeyframe(mutable_motion, mutable_keyframe, 42, &status);
    CHECK(status == NANOEM_STATUS_SUCCESS);
    SECTION("duplication should reject")
    {
        nanoemMutableMotionAddSelfShadowKeyframe(mutable_motion, mutable_keyframe, 42, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_ALREADY_EXISTS);
    }
    SECTION("all keyframe properties should get")
    {
        const nanoem_motion_self_shadow_keyframe_t *origin_keyframe =
            nanoemMutableMotionSelfShadowKeyframeGetOriginObject(mutable_keyframe);
        CHECK(
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionSelfShadowKeyframeGetKeyframeObject(origin_keyframe)) == 42);
        CHECK(nanoemMotionSelfShadowKeyframeGetDistance(origin_keyframe) == Approx(8875.0f));
        CHECK(nanoemMotionSelfShadowKeyframeGetMode(origin_keyframe) == 1);
    }
    SECTION("saved keyframe data should be able to parse")
    {
        nanoem_mutable_buffer_t *mutable_buffer = scope.newBuffer();
        nanoemMutableMotionSelfShadowKeyframeSaveToBuffer(mutable_keyframe, mutable_buffer, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoem_buffer_t *buffer = scope.newBuffer(mutable_buffer);
        nanoem_motion_t *motion = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoem_motion_self_shadow_keyframe_t *generated_keyframe = nanoemMotionSelfShadowKeyframeCreate(motion, &status);
        nanoemMotionSelfShadowKeyframeParseVMD(generated_keyframe, buffer, 3, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        CHECK(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionSelfShadowKeyframeGetKeyframeObject(generated_keyframe)) ==
            45);
        CHECK(nanoemMotionSelfShadowKeyframeGetDistance(generated_keyframe) == Approx(8875.0f));
        CHECK(nanoemMotionSelfShadowKeyframeGetMode(generated_keyframe) == 1);
        nanoemMotionSelfShadowKeyframeDestroy(generated_keyframe);
    }
    SECTION("duplicated remove should reject")
    {
        nanoem_rsize_t num_keyframes;
        nanoem_motion_t *motion = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoemMutableMotionRemoveSelfShadowKeyframe(mutable_motion, mutable_keyframe, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMotionGetAllSelfShadowKeyframeObjects(motion, &num_keyframes);
        CHECK(num_keyframes == 0);
        nanoemMutableMotionRemoveSelfShadowKeyframe(mutable_motion, mutable_keyframe, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_NOT_FOUND);
    }
}

TEST_CASE("mutable_self_shadow_keyframe_generate_nmd", "[nanoem]")
{
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_buffer_t *mutable_buffer = scope.newBuffer();
    {
        nanoem_mutable_motion_t *mutable_motion = scope.newMotion();
        nanoem_motion_t *motion = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoem_mutable_motion_self_shadow_keyframe_t *mutable_keyframe =
            nanoemMutableMotionSelfShadowKeyframeCreate(motion, &status);
        nanoemMutableMotionSelfShadowKeyframeSetDistance(mutable_keyframe, 8875.0f);
        nanoemMutableMotionSelfShadowKeyframeSetMode(mutable_keyframe, 1);
        nanoemMutableMotionAddSelfShadowKeyframe(mutable_motion, mutable_keyframe, 42, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableMotionSelfShadowKeyframeDestroy(mutable_keyframe);
        nanoemMutableMotionSaveToBufferNMD(mutable_motion, mutable_buffer, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
    }
    {
        nanoem_motion_t *new_motion = nanoemMutableMotionGetOriginObject(scope.newMotion());
        nanoem_buffer_t *buffer = scope.newBuffer(mutable_buffer);
        nanoemMotionLoadFromBufferNMD(new_motion, buffer, 1, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        const nanoem_motion_self_shadow_keyframe_t *generated_keyframe =
            nanoemMotionFindSelfShadowKeyframeObject(new_motion, 43);
        CHECK(generated_keyframe);
        CHECK(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionSelfShadowKeyframeGetKeyframeObject(generated_keyframe)) ==
            43);
        CHECK(nanoemMotionSelfShadowKeyframeGetDistance(generated_keyframe) == Approx(8875.0f));
        CHECK(nanoemMotionSelfShadowKeyframeGetMode(generated_keyframe) == 1);
    }
}
