/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_bone_keyframe_basic", "[nanoem]")
{
    CHECK(nanoemMotionBoneKeyframeGetId(NULL) == 0);
    CHECK_FALSE(nanoemMotionBoneKeyframeGetName(NULL));
    CHECK(nanoemMotionBoneKeyframeGetOrientation(NULL));
    CHECK(nanoemMotionBoneKeyframeGetTranslation(NULL));
    CHECK(nanoemMotionBoneKeyframeGetStageIndex(NULL) == 0);
    CHECK(nanoemMotionBoneKeyframeIsPhysicsSimulationEnabled(NULL) == nanoem_false);
    CHECK_FALSE(nanoemMotionBoneKeyframeGetKeyframeObject(NULL));
    CHECK_FALSE(nanoemMotionBoneKeyframeGetKeyframeObjectMutable(NULL));
    nanoemMotionBoneKeyframeDestroy(NULL);
}

TEST_CASE("mutable_bone_keyframe_null", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    CHECK_FALSE(nanoemMutableMotionBoneKeyframeCreate(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionBoneKeyframeCreateAsReference(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionBoneKeyframeCreateByFound(NULL, NULL, 0, NULL));
    CHECK_FALSE(nanoemMutableMotionBoneKeyframeGetOriginObject(NULL));
    CHECK_FALSE(nanoemMutableMotionBoneKeyframeGetOriginObject(NULL));
    nanoemMutableMotionBoneKeyframeSetInterpolation(
        NULL, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM, NULL);
    nanoemMutableMotionBoneKeyframeSetOrientation(NULL, NULL);
    nanoemMutableMotionBoneKeyframeSetStageIndex(NULL, 0);
    nanoemMutableMotionBoneKeyframeSetPhysicsSimulationEnabled(NULL, nanoem_false);
    nanoemMutableMotionBoneKeyframeSetTranslation(NULL, NULL);
    nanoemMutableMotionBoneKeyframeSaveToBuffer(NULL, NULL, &status);
    nanoemMutableMotionBoneKeyframeDestroy(NULL);
}

TEST_CASE("mutable_bone_keyframe_null_value", "[nanoem]")
{
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_bone_keyframe_t *keyframe = scope.newBoneKeyframe();
    CHECK_FALSE(nanoemMutableMotionBoneKeyframeCreateByFound(scope.origin(), NULL, 0, NULL));
    nanoemMutableMotionBoneKeyframeSetTranslation(keyframe, NULL);
    nanoemMutableMotionBoneKeyframeSetInterpolation(
        keyframe, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM, NULL);
    nanoemMutableMotionBoneKeyframeSetOrientation(keyframe, NULL);
    nanoemMutableMotionBoneKeyframeSetStageIndex(keyframe, 0);
    nanoemMutableMotionBoneKeyframeSetPhysicsSimulationEnabled(keyframe, nanoem_false);
    nanoemMutableMotionBoneKeyframeSetTranslation(keyframe, NULL);
    nanoemMutableMotionBoneKeyframeSaveToBuffer(keyframe, NULL, &status);
}

TEST_CASE("mutable_bone_keyframe_manipulate", "[nanoem]")
{
    MotionScope scope;
    nanoem_mutable_motion_t *motion = scope.newMotion();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_bone_keyframe_t *keyframe = scope.newBoneKeyframe();
    nanoemMutableMotionAddBoneKeyframe(NULL, NULL, NULL, 0, &status);
    nanoemMutableMotionAddBoneKeyframe(motion, NULL, NULL, 0, &status);
    nanoemMutableMotionAddBoneKeyframe(motion, keyframe, NULL, 0, &status);
    nanoemMutableMotionRemoveBoneKeyframe(NULL, NULL, &status);
    nanoemMutableMotionRemoveBoneKeyframe(motion, NULL, &status);
    nanoemMutableMotionRemoveBoneKeyframe(motion, keyframe, &status);
    CHECK(status == NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_NOT_FOUND);
}

TEST_CASE("mutable_bone_keyframe_generate_vmd", "[nanoem]")
{
    static const nanoem_u8_t expected_interpolation[] = { 12, 24, 36, 48 };
    static const nanoem_f32_t expected_translation[] = { 1, 2, 3, 0 };
    static const nanoem_f32_t expected_orientation[] = { 0.1f, 0.2f, 0.3f, 0.4f };
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_t *mutable_motion = scope.newMotion();
    nanoem_mutable_motion_bone_keyframe_t *mutable_keyframe = scope.newBoneKeyframe();
    /* create */
    nanoem_unicode_string_t *name = scope.newString("bone_keyframe");
    nanoemMutableMotionBoneKeyframeSetTranslation(mutable_keyframe, expected_translation);
    nanoemMutableMotionBoneKeyframeSetOrientation(mutable_keyframe, expected_orientation);
    for (int i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
         i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
        nanoemMutableMotionBoneKeyframeSetInterpolation(
            mutable_keyframe, (nanoem_motion_bone_keyframe_interpolation_type_t) i, expected_interpolation);
    }
    /* add */
    nanoemMutableMotionAddBoneKeyframe(mutable_motion, mutable_keyframe, name, 42, &status);
    CHECK(status == NANOEM_STATUS_SUCCESS);
    SECTION("duplication should reject")
    {
        nanoemMutableMotionAddBoneKeyframe(mutable_motion, mutable_keyframe, name, 42, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_ALREADY_EXISTS);
    }
    SECTION("all keyframe properties should get")
    {
        nanoem_motion_bone_keyframe_t *origin_keyframe = nanoemMutableMotionBoneKeyframeGetOriginObject(mutable_keyframe);
        const std::string &actual_name = scope.describe(nanoemMotionBoneKeyframeGetName(origin_keyframe));
        CHECK_THAT(actual_name, Catch::Equals("bone_keyframe"));
        CHECK(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(origin_keyframe)) == 42);
        CHECK_THAT(nanoemMotionBoneKeyframeGetTranslation(origin_keyframe), Equals(1, 2, 3, 0));
        CHECK_THAT(nanoemMotionBoneKeyframeGetOrientation(origin_keyframe), Equals(0.1f, 0.2f, 0.3f, 0.4f));
        for (int i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
             i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
            const nanoem_u8_t *actual_interpolation = nanoemMotionBoneKeyframeGetInterpolation(
                origin_keyframe, (nanoem_motion_bone_keyframe_interpolation_type_t) i);
            CHECK_THAT(actual_interpolation, EqualsU8(12, 24, 36, 48));
        }
    }
    SECTION("saved keyframe data should be able to parse")
    {
        nanoem_mutable_buffer_t *mutable_buffer = scope.newBuffer();
        nanoemMutableMotionBoneKeyframeSaveToBuffer(mutable_keyframe, mutable_buffer, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoem_buffer_t *buffer = scope.newBuffer(mutable_buffer);
        nanoem_motion_t *origin = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_motion_bone_keyframe_t *generated_keyframe = nanoemMotionBoneKeyframeCreate(origin, &status);
        kh_string_cache_t *cache = kh_init_string_cache();
        nanoemMotionBoneKeyframeParseVMD(generated_keyframe, buffer, cache, 1, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        CHECK(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(generated_keyframe)) ==
            43);
        const std::string &actual_name = scope.describe(nanoemMotionBoneKeyframeGetName(generated_keyframe));
        CHECK_THAT(actual_name, Catch::Equals("bone_keyframe"));
        CHECK_THAT(nanoemMotionBoneKeyframeGetTranslation(generated_keyframe), Equals(1, 2, 3, 0));
        CHECK_THAT(nanoemMotionBoneKeyframeGetOrientation(generated_keyframe), Equals(0.1f, 0.2f, 0.3f, 0.4f));
        for (int i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
             i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
            const nanoem_u8_t *actual_interpolation = nanoemMotionBoneKeyframeGetInterpolation(
                generated_keyframe, (nanoem_motion_bone_keyframe_interpolation_type_t) i);
            CHECK_THAT(actual_interpolation, EqualsU8(12, 24, 36, 48));
        }
        nanoemMotionBoneKeyframeDestroy(generated_keyframe);
        nanoemStringCacheDestroy(cache);
    }
    SECTION("duplicated remove should reject")
    {
        nanoem_rsize_t num_keyframes;
        nanoem_motion_t *origin = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoemMutableMotionRemoveBoneKeyframe(mutable_motion, mutable_keyframe, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMotionGetAllBoneKeyframeObjects(origin, &num_keyframes);
        CHECK(num_keyframes == 0);
        nanoemMutableMotionRemoveBoneKeyframe(mutable_motion, mutable_keyframe, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_NOT_FOUND);
    }
}

TEST_CASE("mutable_bone_keyframe_generate_nmd", "[nanoem]")
{
    static const nanoem_u8_t expected_interpolation[] = { 12, 24, 36, 48 };
    static const nanoem_f32_t expected_translation[] = { 1, 2, 3, 0 };
    static const nanoem_f32_t expected_orientation[] = { 0.1f, 0.2f, 0.3f, 0.4f };
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_buffer_t *mutable_buffer = scope.newBuffer();
    nanoem_unicode_string_t *name = scope.newString("this_is_a_long_bone_keyframe_name");
    {
        nanoem_mutable_motion_t *mutable_motion = scope.newMotion();
        nanoem_motion_t *motion = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoem_mutable_motion_bone_keyframe_t *mutable_keyframe = nanoemMutableMotionBoneKeyframeCreate(motion, &status);
        nanoemMutableMotionBoneKeyframeSetTranslation(mutable_keyframe, expected_translation);
        nanoemMutableMotionBoneKeyframeSetOrientation(mutable_keyframe, expected_orientation);
        for (int i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
             i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
            nanoemMutableMotionBoneKeyframeSetInterpolation(
                mutable_keyframe, (nanoem_motion_bone_keyframe_interpolation_type_t) i, expected_interpolation);
        }
        nanoemMutableMotionBoneKeyframeSetStageIndex(mutable_keyframe, 7);
        nanoemMutableMotionBoneKeyframeSetPhysicsSimulationEnabled(mutable_keyframe, nanoem_false);
        nanoemMutableMotionAddBoneKeyframe(mutable_motion, mutable_keyframe, name, 42, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableMotionBoneKeyframeDestroy(mutable_keyframe);
        nanoemMutableMotionSaveToBufferNMD(mutable_motion, mutable_buffer, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
    }
    {
        nanoem_motion_t *new_motion = nanoemMutableMotionGetOriginObject(scope.newMotion());
        nanoem_buffer_t *buffer = scope.newBuffer(mutable_buffer);
        nanoemMotionLoadFromBufferNMD(new_motion, buffer, 1, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        const nanoem_motion_bone_keyframe_t *generated_keyframe =
            nanoemMotionFindBoneKeyframeObject(new_motion, name, 43);
        CHECK(generated_keyframe);
        CHECK(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(generated_keyframe)) ==
            43);
        const std::string &actual_name = scope.describe(nanoemMotionBoneKeyframeGetName(generated_keyframe));
        CHECK_THAT(actual_name, Catch::Equals("this_is_a_long_bone_keyframe_name"));
        CHECK_THAT(nanoemMotionBoneKeyframeGetTranslation(generated_keyframe), Equals(1, 2, 3, 0));
        CHECK_THAT(nanoemMotionBoneKeyframeGetOrientation(generated_keyframe), Equals(0.1f, 0.2f, 0.3f, 0.4f));
        for (int i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
             i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
            CHECK_THAT(nanoemMotionBoneKeyframeGetInterpolation(
                           generated_keyframe, (nanoem_motion_bone_keyframe_interpolation_type_t) i),
                EqualsU8(12, 24, 36, 48));
        }
        CHECK(nanoemMotionBoneKeyframeGetStageIndex(generated_keyframe) == 7);
        CHECK(nanoemMotionBoneKeyframeIsPhysicsSimulationEnabled(generated_keyframe) == nanoem_false);
    }
}
