/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_motion_basic", "[nanoem]")
{
    nanoem_motion_accessory_keyframe_t *prev_accessory_keyframe, *next_accessory_keyframe;
    nanoem_motion_bone_keyframe_t *prev_bone_keyframe, *next_bone_keyframe;
    nanoem_motion_camera_keyframe_t *prev_camera_keyframe, *next_camera_keyframe;
    nanoem_motion_light_keyframe_t *prev_light_keyframe, *next_light_keyframe;
    nanoem_motion_model_keyframe_t *prev_model_keyframe, *next_model_keyframe;
    nanoem_motion_morph_keyframe_t *prev_morph_keyframe, *next_morph_keyframe;
    nanoem_motion_self_shadow_keyframe_t *prev_self_shadow_keyframe, *next_self_shadow_keyframe;
    nanoem_rsize_t num_objects;
    CHECK(nanoemMotionGetFormatType(NULL) == NANOEM_MOTION_FORMAT_TYPE_UNKNOWN);
    CHECK_FALSE(nanoemMotionGetTargetModelName(NULL));
    CHECK(nanoemMotionGetMaxFrameIndex(NULL) == 0);
    CHECK_FALSE(nanoemMotionGetAllAccessoryKeyframeObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemMotionGetAllBoneKeyframeObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemMotionGetAllCameraKeyframeObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemMotionGetAllLightKeyframeObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemMotionGetAllModelKeyframeObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemMotionGetAllMorphKeyframeObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemMotionFindBoneKeyframeObject(NULL, NULL, 0));
    CHECK_FALSE(nanoemMotionFindMorphKeyframeObject(NULL, NULL, 0));
    nanoemMotionSearchClosestAccessoryKeyframes(NULL, 0, NULL, NULL);
    nanoemMotionSearchClosestBoneKeyframes(NULL, NULL, 0, NULL, NULL);
    nanoemMotionSearchClosestCameraKeyframes(NULL, 0, NULL, NULL);
    nanoemMotionSearchClosestLightKeyframes(NULL, 0, NULL, NULL);
    nanoemMotionSearchClosestModelKeyframes(NULL, 0, NULL, NULL);
    nanoemMotionSearchClosestMorphKeyframes(NULL, NULL, 0, NULL, NULL);
    nanoemMotionSearchClosestAccessoryKeyframes(NULL, 0, &prev_accessory_keyframe, &next_accessory_keyframe);
    CHECK_FALSE(prev_accessory_keyframe);
    CHECK_FALSE(next_accessory_keyframe);
    nanoemMotionSearchClosestBoneKeyframes(NULL, NULL, 0, &prev_bone_keyframe, &next_bone_keyframe);
    CHECK_FALSE(prev_bone_keyframe);
    CHECK_FALSE(next_bone_keyframe);
    nanoemMotionSearchClosestCameraKeyframes(NULL, 0, &prev_camera_keyframe, &next_camera_keyframe);
    CHECK_FALSE(prev_camera_keyframe);
    CHECK_FALSE(next_camera_keyframe);
    nanoemMotionSearchClosestLightKeyframes(NULL, 0, &prev_light_keyframe, &next_light_keyframe);
    CHECK_FALSE(prev_light_keyframe);
    CHECK_FALSE(next_light_keyframe);
    nanoemMotionSearchClosestModelKeyframes(NULL, 0, &prev_model_keyframe, &next_model_keyframe);
    CHECK_FALSE(prev_model_keyframe);
    CHECK_FALSE(next_model_keyframe);
    nanoemMotionSearchClosestMorphKeyframes(NULL, NULL, 0, &prev_morph_keyframe, &next_morph_keyframe);
    CHECK_FALSE(prev_morph_keyframe);
    CHECK_FALSE(next_morph_keyframe);
    nanoemMotionSearchClosestSelfShadowKeyframes(NULL, 0, &prev_self_shadow_keyframe, &next_self_shadow_keyframe);
    CHECK_FALSE(prev_self_shadow_keyframe);
    CHECK_FALSE(next_self_shadow_keyframe);
    CHECK_FALSE(nanoemMotionCreate(NULL, NULL));
    nanoemMotionDestroy(NULL);
}

TEST_CASE("mutable_motion_null", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    CHECK_FALSE(nanoemMutableMotionBoneKeyframeCreateAsReference(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionBoneKeyframeCreateByFound(NULL, NULL, 0, NULL));
    CHECK_FALSE(nanoemMutableMotionCameraKeyframeCreateAsReference(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionCameraKeyframeCreateByFound(NULL, 0, NULL));
    CHECK_FALSE(nanoemMutableMotionLightKeyframeCreateAsReference(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionLightKeyframeCreateByFound(NULL, 0, NULL));
    CHECK_FALSE(nanoemMutableMotionModelKeyframeCreateAsReference(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionModelKeyframeCreateByFound(NULL, 0, NULL));
    CHECK_FALSE(nanoemMutableMotionMorphKeyframeCreateAsReference(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionMorphKeyframeCreateByFound(NULL, NULL, 0, NULL));
    nanoemMutableMotionAddBoneKeyframe(NULL, NULL, NULL, 0, &status);
    nanoemMutableMotionAddCameraKeyframe(NULL, NULL, 0, &status);
    nanoemMutableMotionAddLightKeyframe(NULL, NULL, 0, &status);
    nanoemMutableMotionAddModelKeyframe(NULL, NULL, 0, &status);
    nanoemMutableMotionAddMorphKeyframe(NULL, NULL, NULL, 0, &status);
    nanoemMutableMotionRemoveBoneKeyframe(NULL, NULL, &status);
    nanoemMutableMotionRemoveCameraKeyframe(NULL, NULL, &status);
    nanoemMutableMotionRemoveLightKeyframe(NULL, NULL, &status);
    nanoemMutableMotionRemoveModelKeyframe(NULL, NULL, &status);
    nanoemMutableMotionRemoveMorphKeyframe(NULL, NULL, &status);
    CHECK_FALSE(nanoemMutableMotionSaveToBuffer(NULL, NULL, &status));
    CHECK_FALSE(nanoemMutableMotionSaveToBufferNMD(NULL, NULL, &status));
}

TEST_CASE("mutable_motion_null_values", "[nanoem]")
{
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_t *mutable_motion = scope.newMotion();
    nanoem_motion_t *origin = nanoemMutableMotionGetOriginObject(mutable_motion);
    CHECK_FALSE(nanoemMutableMotionBoneKeyframeCreateByFound(origin, NULL, 0, NULL));
    CHECK_FALSE(nanoemMutableMotionMorphKeyframeCreateByFound(origin, NULL, 0, NULL));
    nanoemMutableMotionAddBoneKeyframe(mutable_motion, NULL, NULL, 0, &status);
    nanoemMutableMotionAddCameraKeyframe(mutable_motion, NULL, 0, &status);
    nanoemMutableMotionAddLightKeyframe(mutable_motion, NULL, 0, &status);
    nanoemMutableMotionAddModelKeyframe(mutable_motion, NULL, 0, &status);
    nanoemMutableMotionAddMorphKeyframe(mutable_motion, NULL, NULL, 0, &status);
    nanoemMutableMotionRemoveBoneKeyframe(mutable_motion, NULL, &status);
    nanoemMutableMotionRemoveCameraKeyframe(mutable_motion, NULL, &status);
    nanoemMutableMotionRemoveLightKeyframe(mutable_motion, NULL, &status);
    nanoemMutableMotionRemoveModelKeyframe(mutable_motion, NULL, &status);
    nanoemMutableMotionRemoveMorphKeyframe(mutable_motion, NULL, &status);
    nanoem_mutable_motion_bone_keyframe_t *bone_keyframe =
        nanoemMutableMotionBoneKeyframeCreate(nanoemMutableMotionGetOriginObject(mutable_motion), &status);
    nanoemMutableMotionAddBoneKeyframe(NULL, bone_keyframe, NULL, 0, &status);
    nanoemMutableMotionBoneKeyframeDestroy(bone_keyframe);
    nanoem_mutable_motion_camera_keyframe_t *camera_keyframe =
        nanoemMutableMotionCameraKeyframeCreate(nanoemMutableMotionGetOriginObject(mutable_motion), &status);
    nanoemMutableMotionAddCameraKeyframe(NULL, camera_keyframe, 0, &status);
    nanoemMutableMotionCameraKeyframeDestroy(camera_keyframe);
    nanoem_mutable_motion_light_keyframe_t *light_keyframe =
        nanoemMutableMotionLightKeyframeCreate(nanoemMutableMotionGetOriginObject(mutable_motion), &status);
    nanoemMutableMotionAddLightKeyframe(NULL, light_keyframe, 0, &status);
    nanoemMutableMotionLightKeyframeDestroy(light_keyframe);
    nanoem_mutable_motion_model_keyframe_t *model_keyframe =
        nanoemMutableMotionModelKeyframeCreate(nanoemMutableMotionGetOriginObject(mutable_motion), &status);
    nanoemMutableMotionAddModelKeyframe(NULL, model_keyframe, 0, &status);
    nanoemMutableMotionModelKeyframeDestroy(model_keyframe);
    nanoem_mutable_motion_morph_keyframe_t *morph_keyframe =
        nanoemMutableMotionMorphKeyframeCreate(nanoemMutableMotionGetOriginObject(mutable_motion), &status);
    nanoemMutableMotionAddMorphKeyframe(NULL, morph_keyframe, NULL, 0, &status);
    nanoemMutableMotionMorphKeyframeDestroy(morph_keyframe);
    CHECK_FALSE(nanoemMutableMotionSaveToBuffer(mutable_motion, NULL, &status));
    CHECK_FALSE(nanoemMutableMotionSaveToBufferNMD(mutable_motion, NULL, &status));
}
