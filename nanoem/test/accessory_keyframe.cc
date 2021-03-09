/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_accessory_keyframe_basic", "[nanoem]")
{
    nanoem_rsize_t num_objects;
    CHECK_FALSE(nanoemMotionAccessoryKeyframeGetKeyframeObject(NULL));
    CHECK_FALSE(nanoemMotionAccessoryKeyframeGetKeyframeObjectMutable(NULL));
    CHECK(nanoemMotionAccessoryKeyframeGetOpacity(NULL) == Approx(0));
    CHECK(nanoemMotionAccessoryKeyframeGetOrientation(NULL));
    CHECK(nanoemMotionAccessoryKeyframeGetScaleFactor(NULL) == Approx(0));
    CHECK(nanoemMotionAccessoryKeyframeGetTranslation(NULL));
    CHECK_FALSE(nanoemMotionAccessoryKeyframeGetOutsideParent(NULL));
    CHECK_FALSE(nanoemMotionAccessoryKeyframeGetOutsideParentMutable(NULL));
    CHECK_FALSE(nanoemMotionAccessoryKeyframeGetAllEffectParameterObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    nanoemMotionAccessoryKeyframeDestroy(NULL);
}

TEST_CASE("mutable_accessory_keyframe_null", "[nanoem]")
{
    CHECK_FALSE(nanoemMutableMotionAccessoryKeyframeCreate(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionAccessoryKeyframeCreateAsReference(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionAccessoryKeyframeCreateByFound(NULL, 0, NULL));
    CHECK_FALSE(nanoemMutableMotionAccessoryKeyframeGetOriginObject(NULL));
    CHECK_FALSE(nanoemMutableMotionAccessoryKeyframeGetOriginObject(NULL));
    nanoemMutableMotionAccessoryKeyframeSetOutsideParent(NULL, NULL, NULL);
    nanoemMutableMotionAccessoryKeyframeSetOpacity(NULL, 0);
    nanoemMutableMotionAccessoryKeyframeSetScaleFactor(NULL, 0);
    nanoemMutableMotionAccessoryKeyframeSetTranslation(NULL, NULL);
    nanoemMutableMotionAccessoryKeyframeAddEffectParameter(NULL, NULL, NULL);
    nanoemMutableMotionAccessoryKeyframeRemoveEffectParameter(NULL, NULL, NULL);
    nanoemMutableMotionAccessoryKeyframeDestroy(NULL);
}

TEST_CASE("mutable_accessory_keyframe_null_value", "[nanoem]")
{
    MotionScope scope;
    nanoem_mutable_motion_accessory_keyframe_t *keyframe = scope.newAccessoryKeyframe();
    nanoemMutableMotionAccessoryKeyframeSetOutsideParent(keyframe, NULL, NULL);
    nanoemMutableMotionAccessoryKeyframeSetTranslation(keyframe, NULL);
    nanoemMutableMotionAccessoryKeyframeAddEffectParameter(keyframe, NULL, NULL);
    nanoemMutableMotionAccessoryKeyframeRemoveEffectParameter(keyframe, NULL, NULL);
    nanoem_mutable_motion_effect_parameter_t *parameter = nanoemMutableMotionEffectParameterCreateFromAccessoryKeyframe(
        nanoemMutableMotionAccessoryKeyframeGetOriginObject(keyframe), NULL);
    nanoemMutableMotionAccessoryKeyframeAddEffectParameter(keyframe, parameter, NULL);
    nanoemMutableMotionAccessoryKeyframeRemoveEffectParameter(keyframe, parameter, NULL);
    nanoemMutableMotionEffectParameterDestroy(parameter);
}

TEST_CASE("mutable_accessory_keyframe_generate_nmd", "[nanoem]")
{
    static const nanoem_f32_t expected_translation[] = { 1, 2, 3, 0 };
    static const nanoem_f32_t expected_orientation[] = { 4, 5, 6, 7 };
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_buffer_t *mutable_buffer = scope.newBuffer();
    {
        nanoem_mutable_motion_t *mutable_motion = scope.newMotion();
        nanoem_motion_t *motion = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoem_mutable_motion_accessory_keyframe_t *mutable_keyframe = nanoemMutableMotionAccessoryKeyframeCreate(motion, &status);
        nanoemMutableMotionAccessoryKeyframeSetTranslation(mutable_keyframe, expected_translation);
        nanoemMutableMotionAccessoryKeyframeSetOrientation(mutable_keyframe, expected_orientation);
        nanoemMutableMotionAccessoryKeyframeSetOpacity(mutable_keyframe, 0.7f);
        nanoemMutableMotionAccessoryKeyframeSetScaleFactor(mutable_keyframe, 0.8f);
        nanoem_mutable_motion_outside_parent_t *op = nanoemMutableMotionOutsideParentCreateFromAccessoryKeyframe(
            nanoemMutableMotionAccessoryKeyframeGetOriginObject(mutable_keyframe), &status);
        nanoem_unicode_string_t *bone_name = scope.newString("accessory_outside_parent_parent_bone_name");
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableMotionOutsideParentSetTargetBoneName(op, bone_name, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoem_unicode_string_t *object_name = scope.newString("accessory_outside_parent_parent_object_name");
        nanoemMutableMotionOutsideParentSetTargetObjectName(op, object_name, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableMotionAccessoryKeyframeSetOutsideParent(mutable_keyframe, op, &status);
        nanoemMutableMotionOutsideParentDestroy(op);
        {
            {
                nanoem_mutable_motion_effect_parameter_t *parameter = nanoemMutableMotionEffectParameterCreateFromAccessoryKeyframe(
                    nanoemMutableMotionAccessoryKeyframeGetOriginObject(mutable_keyframe), &status);
                nanoem_unicode_string_t *boolean_parameter_name = scope.newString("boolean_parameter");
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableMotionEffectParameterSetName(parameter, boolean_parameter_name, &status);
                nanoemMutableMotionEffectParameterSetType(parameter, NANOEM_MOTION_EFFECT_PARAMETER_TYPE_BOOL);
                nanoem_i32_t boolean_value = 1;
                nanoemMutableMotionEffectParameterSetValue(parameter, &boolean_value);
                nanoemMutableMotionAccessoryKeyframeAddEffectParameter(mutable_keyframe, parameter, &status);
                CHECK(status == NANOEM_STATUS_SUCCESS);
                nanoemMutableMotionEffectParameterDestroy(parameter);
            }
            {
                nanoem_mutable_motion_effect_parameter_t *parameter = nanoemMutableMotionEffectParameterCreateFromAccessoryKeyframe(
                    nanoemMutableMotionAccessoryKeyframeGetOriginObject(mutable_keyframe), &status);
                nanoem_unicode_string_t *integer_parameter_name = scope.newString("integer_parameter");
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableMotionEffectParameterSetName(parameter, integer_parameter_name, &status);
                CHECK(status == NANOEM_STATUS_SUCCESS);
                nanoemMutableMotionEffectParameterSetType(parameter, NANOEM_MOTION_EFFECT_PARAMETER_TYPE_INT);
                nanoem_i32_t integer_value = 42;
                nanoemMutableMotionEffectParameterSetValue(parameter, &integer_value);
                nanoemMutableMotionAccessoryKeyframeAddEffectParameter(mutable_keyframe, parameter, &status);
                CHECK(status == NANOEM_STATUS_SUCCESS);
                nanoemMutableMotionEffectParameterDestroy(parameter);
            }
            {
                nanoem_mutable_motion_effect_parameter_t *parameter = nanoemMutableMotionEffectParameterCreateFromAccessoryKeyframe(
                    nanoemMutableMotionAccessoryKeyframeGetOriginObject(mutable_keyframe), &status);
                nanoem_unicode_string_t *float_parameter_name = scope.newString("float_parameter");
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableMotionEffectParameterSetName(parameter, float_parameter_name, &status);
                CHECK(status == NANOEM_STATUS_SUCCESS);
                nanoemMutableMotionEffectParameterSetType(parameter, NANOEM_MOTION_EFFECT_PARAMETER_TYPE_FLOAT);
                nanoem_f32_t float_value = 42.0f;
                nanoemMutableMotionEffectParameterSetValue(parameter, &float_value);
                nanoemMutableMotionAccessoryKeyframeAddEffectParameter(mutable_keyframe, parameter, &status);
                CHECK(status == NANOEM_STATUS_SUCCESS);
                nanoemMutableMotionEffectParameterDestroy(parameter);
            }
            {
                nanoem_mutable_motion_effect_parameter_t *parameter = nanoemMutableMotionEffectParameterCreateFromAccessoryKeyframe(
                    nanoemMutableMotionAccessoryKeyframeGetOriginObject(mutable_keyframe), &status);
                nanoem_unicode_string_t *vector4_parameter_name = scope.newString("vector4_parameter");
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoemMutableMotionEffectParameterSetName(parameter, vector4_parameter_name, &status);
                CHECK(status == NANOEM_STATUS_SUCCESS);
                nanoemMutableMotionEffectParameterSetType(parameter, NANOEM_MOTION_EFFECT_PARAMETER_TYPE_VECTOR4);
                nanoem_f32_t vector4_value[] = { 0.1f, 0.2f, 0.3f, 0.4f };
                nanoemMutableMotionEffectParameterSetValue(parameter, vector4_value);
                nanoemMutableMotionAccessoryKeyframeAddEffectParameter(mutable_keyframe, parameter, &status);
                CHECK(status == NANOEM_STATUS_SUCCESS);
                nanoemMutableMotionEffectParameterDestroy(parameter);
            }
        }
        nanoemMutableMotionAddAccessoryKeyframe(mutable_motion, mutable_keyframe, 42, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableMotionAccessoryKeyframeDestroy(mutable_keyframe);
        nanoemMutableMotionSaveToBufferNMD(mutable_motion, mutable_buffer, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
    }
    {
        nanoem_motion_t *new_motion = nanoemMutableMotionGetOriginObject(scope.newMotion());
        nanoem_buffer_t *buffer = scope.newBuffer(mutable_buffer);
        nanoemMotionLoadFromBufferNMD(new_motion, buffer, 1, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        const nanoem_motion_accessory_keyframe_t *generated_keyframe = nanoemMotionFindAccessoryKeyframeObject(new_motion, 43);
        CHECK(generated_keyframe);
        CHECK(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionAccessoryKeyframeGetKeyframeObject(generated_keyframe)) ==
            43);
        CHECK_THAT(nanoemMotionAccessoryKeyframeGetTranslation(generated_keyframe), Equals(1, 2, 3, 0));
        CHECK_THAT(nanoemMotionAccessoryKeyframeGetOrientation(generated_keyframe), Equals(4, 5, 6, 7));
        CHECK(nanoemMotionAccessoryKeyframeGetOpacity(generated_keyframe) == Approx(0.7f));
        CHECK(nanoemMotionAccessoryKeyframeGetScaleFactor(generated_keyframe) == Approx(0.8f));
        const nanoem_motion_outside_parent_t *op = nanoemMotionAccessoryKeyframeGetOutsideParent(generated_keyframe);
        const std::string &bone_name = scope.describe(nanoemMotionOutsideParentGetTargetBoneName(op));
        CHECK_THAT(bone_name, Catch::Equals("accessory_outside_parent_parent_bone_name"));
        const std::string &object_name = scope.describe(nanoemMotionOutsideParentGetTargetObjectName(op));
        CHECK_THAT(object_name, Catch::Equals("accessory_outside_parent_parent_object_name"));
        {
            nanoem_rsize_t num_parameters;
            nanoem_motion_effect_parameter_t *const *parameters =
                nanoemMotionAccessoryKeyframeGetAllEffectParameterObjects(generated_keyframe, &num_parameters);
            CHECK(parameters);
            CHECK(num_parameters == 4);
            CHECK(nanoemMotionEffectParameterGetType(parameters[0]) == NANOEM_MOTION_EFFECT_PARAMETER_TYPE_BOOL);
            const std::string &boolean_parameter_name = scope.describe(nanoemMotionEffectParameterGetName(parameters[0]));
            CHECK_THAT(boolean_parameter_name, Catch::Equals("boolean_parameter"));
            CHECK(*((const nanoem_i32_t *) nanoemMotionEffectParameterGetValue(parameters[0])) == 1);
            CHECK(nanoemMotionEffectParameterGetType(parameters[1]) == NANOEM_MOTION_EFFECT_PARAMETER_TYPE_INT);
            const std::string &integer_parameter_name = scope.describe(nanoemMotionEffectParameterGetName(parameters[1]));
            CHECK_THAT(integer_parameter_name, Catch::Equals("integer_parameter"));
            CHECK(*((const nanoem_i32_t *) nanoemMotionEffectParameterGetValue(parameters[1])) == 42);
            CHECK(nanoemMotionEffectParameterGetType(parameters[2]) == NANOEM_MOTION_EFFECT_PARAMETER_TYPE_FLOAT);
            const std::string &float_parameter_name = scope.describe(nanoemMotionEffectParameterGetName(parameters[2]));
            CHECK_THAT(float_parameter_name, Catch::Equals("float_parameter"));
            CHECK(*((const nanoem_f32_t *) nanoemMotionEffectParameterGetValue(parameters[2])) == Approx(42.0f));
            CHECK(nanoemMotionEffectParameterGetType(parameters[3]) == NANOEM_MOTION_EFFECT_PARAMETER_TYPE_VECTOR4);
            const std::string &vector4_parameter_name = scope.describe(nanoemMotionEffectParameterGetName(parameters[3]));
            CHECK_THAT(vector4_parameter_name, Catch::Equals("vector4_parameter"));
            CHECK_THAT((const nanoem_f32_t *) nanoemMotionEffectParameterGetValue(parameters[3]),
                Equals(0.1f, 0.2f, 0.3f, 0.4f));
        }
    }
}
