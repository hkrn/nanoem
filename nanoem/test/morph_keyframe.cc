/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_morph_keyframe_basic", "[nanoem]")
{
    CHECK(nanoemMotionMorphKeyframeGetId(NULL) == 0);
    CHECK_FALSE(nanoemMotionMorphKeyframeGetName(NULL));
    CHECK(nanoemMotionMorphKeyframeGetWeight(NULL) == Approx(0));
    CHECK_FALSE(nanoemMotionMorphKeyframeGetKeyframeObject(NULL));
    CHECK_FALSE(nanoemMotionMorphKeyframeGetKeyframeObjectMutable(NULL));
    nanoemMotionMorphKeyframeDestroy(NULL);
}

TEST_CASE("mutable_morph_keyframe_null", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    CHECK_FALSE(nanoemMutableMotionMorphKeyframeCreate(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionMorphKeyframeCreateAsReference(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionMorphKeyframeCreateByFound(NULL, NULL, 0, NULL));
    CHECK_FALSE(nanoemMutableMotionMorphKeyframeGetOriginObject(NULL));
    CHECK_FALSE(nanoemMutableMotionMorphKeyframeGetOriginObject(NULL));
    nanoemMutableMotionMorphKeyframeSetWeight(NULL, 0);
    nanoemMutableMotionMorphKeyframeSaveToBuffer(NULL, NULL, &status);
    nanoemMutableMotionMorphKeyframeDestroy(NULL);
}

TEST_CASE("mutable_morph_keyframe_null_value", "[nanoem]")
{
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_motion_t *origin = nanoemMutableMotionGetOriginObject(scope.newMotion());
    nanoem_mutable_motion_morph_keyframe_t *keyframe = scope.newMorphKeyframe();
    CHECK_FALSE(nanoemMutableMotionMorphKeyframeCreateByFound(origin, NULL, 0, &status));
    nanoemMutableMotionMorphKeyframeSetWeight(keyframe, 0);
    nanoemMutableMotionMorphKeyframeSaveToBuffer(keyframe, NULL, &status);
}

TEST_CASE("mutable_morph_keyframe_manipulate", "[nanoem]")
{
    MotionScope scope;
    nanoem_mutable_motion_t *motion = scope.newMotion();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_morph_keyframe_t *keyframe = scope.newMorphKeyframe();
    nanoemMutableMotionAddBoneKeyframe(NULL, NULL, NULL, 0, &status);
    nanoemMutableMotionAddMorphKeyframe(motion, NULL, NULL, 0, &status);
    nanoemMutableMotionAddMorphKeyframe(motion, keyframe, NULL, 0, &status);
    nanoemMutableMotionRemoveMorphKeyframe(NULL, NULL, &status);
    nanoemMutableMotionRemoveMorphKeyframe(motion, NULL, &status);
    nanoemMutableMotionRemoveMorphKeyframe(motion, keyframe, &status);
    CHECK(status == NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_NOT_FOUND);
}

TEST_CASE("mutable_morph_keyframe_generate_vmd", "[nanoem]")
{
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_t *mutable_motion = scope.newMotion();
    nanoem_mutable_motion_morph_keyframe_t *mutable_keyframe = scope.newMorphKeyframe();
    /* create */
    nanoem_unicode_string_t *name = scope.newString("morph_keyframe");
    nanoemMutableMotionMorphKeyframeSetWeight(mutable_keyframe, 0.42f);
    /* add */
    nanoemMutableMotionAddMorphKeyframe(mutable_motion, mutable_keyframe, name, 42, &status);
    CHECK(status == NANOEM_STATUS_SUCCESS);
    SECTION("duplication should reject")
    {
        nanoemMutableMotionAddMorphKeyframe(mutable_motion, mutable_keyframe, name, 42, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_ALREADY_EXISTS);
    }
    SECTION("all keyframe properties should get")
    {
        nanoem_motion_morph_keyframe_t *origin_keyframe =
            nanoemMutableMotionMorphKeyframeGetOriginObject(mutable_keyframe);
        CHECK(
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(origin_keyframe)) == 42);
        const std::string &actual_name = scope.describe(nanoemMotionMorphKeyframeGetName(origin_keyframe));
        CHECK_THAT(actual_name, Catch::Equals("morph_keyframe"));
        CHECK(nanoemMotionMorphKeyframeGetWeight(origin_keyframe) == Approx(0.42f));
    }
    SECTION("saved keyframe data should be able to parse")
    {
        nanoem_mutable_buffer_t *mutable_buffer = scope.newBuffer();
        nanoemMutableMotionMorphKeyframeSaveToBuffer(mutable_keyframe, mutable_buffer, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoem_buffer_t *buffer = scope.newBuffer(mutable_buffer);
        nanoem_motion_t *motion = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_motion_morph_keyframe_t *generated_keyframe = nanoemMotionMorphKeyframeCreate(motion, &status);
        kh_string_cache_t *cache = kh_init_string_cache();
        nanoemMotionMorphKeyframeParseVMD(generated_keyframe, buffer, cache, 4, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        CHECK(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(generated_keyframe)) ==
            46);
        const std::string &actual_name = scope.describe(nanoemMotionMorphKeyframeGetName(generated_keyframe));
        CHECK_THAT(actual_name, Catch::Equals("morph_keyframe"));
        CHECK(nanoemMotionMorphKeyframeGetWeight(generated_keyframe) == Approx(0.42f));
        nanoemMotionMorphKeyframeDestroy(generated_keyframe);
        nanoemStringCacheDestroy(cache);
    }
    SECTION("duplicated remove should reject")
    {
        nanoem_rsize_t num_keyframes;
        nanoem_motion_t *motion = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoemMutableMotionRemoveMorphKeyframe(mutable_motion, mutable_keyframe, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMotionGetAllMorphKeyframeObjects(motion, &num_keyframes);
        CHECK(num_keyframes == 0);
        nanoemMutableMotionRemoveMorphKeyframe(mutable_motion, mutable_keyframe, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_NOT_FOUND);
    }
}

TEST_CASE("mutable_morph_keyframe_generate_nmd", "[nanoem]")
{
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_buffer_t *mutable_buffer = scope.newBuffer();
    nanoem_unicode_string_t *name = scope.newString("this_is_a_long_morph_keyframe_name");
    {
        nanoem_mutable_motion_t *mutable_motion = scope.newMotion();
        nanoem_motion_t *motion = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoem_mutable_motion_morph_keyframe_t *mutable_keyframe =
            nanoemMutableMotionMorphKeyframeCreate(motion, &status);
        nanoemMutableMotionMorphKeyframeSetWeight(mutable_keyframe, 0.42f);
        nanoemMutableMotionAddMorphKeyframe(mutable_motion, mutable_keyframe, name, 42, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableMotionMorphKeyframeDestroy(mutable_keyframe);
        nanoemMutableMotionSaveToBufferNMD(mutable_motion, mutable_buffer, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
    }
    {
        nanoem_motion_t *new_motion = nanoemMutableMotionGetOriginObject(scope.newMotion());
        nanoem_buffer_t *buffer = scope.newBuffer(mutable_buffer);
        nanoemMotionLoadFromBufferNMD(new_motion, buffer, 1, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        const nanoem_motion_morph_keyframe_t *generated_keyframe =
            nanoemMotionFindMorphKeyframeObject(new_motion, name, 43);
        CHECK(generated_keyframe);
        CHECK(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(generated_keyframe)) ==
            43);
        const std::string &actual_name = scope.describe(nanoemMotionMorphKeyframeGetName(generated_keyframe));
        CHECK_THAT(actual_name, Catch::Equals("this_is_a_long_morph_keyframe_name"));
        CHECK(nanoemMotionMorphKeyframeGetWeight(generated_keyframe) == Approx(0.42f));
    }
}
