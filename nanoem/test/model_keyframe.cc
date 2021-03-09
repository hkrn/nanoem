/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_model_keyframe_basic", "[nanoem]")
{
    nanoem_rsize_t num_objects;
    CHECK(nanoemMotionModelKeyframeGetEdgeColor(NULL));
    CHECK(nanoemMotionModelKeyframeGetEdgeScaleFactor(NULL) == 0.0f);
    CHECK_FALSE(nanoemMotionModelKeyframeIsAddBlendEnabled(NULL));
    CHECK_FALSE(nanoemMotionModelKeyframeIsVisible(NULL));
    CHECK_FALSE(nanoemMotionModelKeyframeIsPhysicsSimulationEnabled(NULL));
    CHECK_FALSE(nanoemMotionModelKeyframeGetAllConstraintStateObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK(nanoemMotionModelKeyframeConstraintStateGetBoneId(NULL) == 0);
    CHECK_FALSE(nanoemMotionModelKeyframeConstraintStateGetBoneName(NULL));
    CHECK_FALSE(nanoemMotionModelKeyframeConstraintStateIsEnabled(NULL));
    CHECK_FALSE(nanoemMotionModelKeyframeGetKeyframeObject(NULL));
    CHECK_FALSE(nanoemMotionModelKeyframeGetKeyframeObjectMutable(NULL));
    CHECK_FALSE(nanoemMotionModelKeyframeGetAllOutsideParentObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemMotionModelKeyframeGetAllEffectParameterObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    nanoemMotionModelKeyframeDestroy(NULL);
}

TEST_CASE("model_keyframe_default_value", "[nanoem]")
{
    MotionScope scope;
    nanoem_motion_model_keyframe_t *keyframe = nanoemMutableMotionModelKeyframeGetOriginObject(scope.newModelKeyframe());
    CHECK(nanoemMotionModelKeyframeIsPhysicsSimulationEnabled(keyframe));
}

TEST_CASE("mutable_model_keyframe_null", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    CHECK_FALSE(nanoemMutableMotionModelKeyframeCreate(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionModelKeyframeCreateAsReference(NULL, NULL));
    CHECK_FALSE(nanoemMutableMotionModelKeyframeCreateByFound(NULL, 0, NULL));
    CHECK_FALSE(nanoemMutableMotionModelKeyframeGetOriginObject(NULL));
    CHECK_FALSE(nanoemMutableMotionModelKeyframeGetOriginObject(NULL));
    nanoemMutableMotionModelKeyframeSetAddBlendEnabled(NULL, nanoem_false);
    nanoemMutableMotionModelKeyframeSetEdgeColor(NULL, NULL);
    nanoemMutableMotionModelKeyframeSetEdgeScaleFactor(NULL, 0);
    nanoemMutableMotionModelKeyframeSetVisible(NULL, nanoem_false);
    nanoemMutableMotionModelKeyframeSetPhysicsSimulationEnabled(NULL, nanoem_false);
    nanoemMutableMotionModelKeyframeAddConstraintState(NULL, NULL, &status);
    nanoemMutableMotionModelKeyframeRemoveConstraintState(NULL, NULL, &status);
    nanoemMutableMotionModelKeyframeAddEffectParameter(NULL, NULL, &status);
    nanoemMutableMotionModelKeyframeRemoveEffectParameter(NULL, NULL, &status);
    nanoemMutableMotionModelKeyframeAddOutsideParent(NULL, NULL, &status);
    nanoemMutableMotionModelKeyframeRemoveOutsideParent(NULL, NULL, &status);
    nanoemMutableMotionModelKeyframeSaveToBuffer(NULL, NULL, &status);
    nanoemMutableMotionModelKeyframeDestroy(NULL);
}

TEST_CASE("mutable_model_keyframe_null_value", "[nanoem]")
{
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_model_keyframe_t *keyframe = scope.newModelKeyframe();
    nanoemMutableMotionModelKeyframeSetAddBlendEnabled(keyframe, nanoem_false);
    nanoemMutableMotionModelKeyframeSetEdgeColor(keyframe, NULL);
    nanoemMutableMotionModelKeyframeSetEdgeScaleFactor(keyframe, 0);
    nanoemMutableMotionModelKeyframeSetVisible(keyframe, nanoem_false);
    nanoemMutableMotionModelKeyframeSetPhysicsSimulationEnabled(keyframe, nanoem_false);
    nanoemMutableMotionModelKeyframeAddConstraintState(keyframe, NULL, &status);
    nanoemMutableMotionModelKeyframeRemoveConstraintState(keyframe, NULL, &status);
    nanoemMutableMotionModelKeyframeAddEffectParameter(keyframe, NULL, &status);
    nanoemMutableMotionModelKeyframeRemoveEffectParameter(keyframe, NULL, &status);
    nanoemMutableMotionModelKeyframeAddOutsideParent(keyframe, NULL, &status);
    nanoemMutableMotionModelKeyframeRemoveOutsideParent(keyframe, NULL, &status);
    nanoemMutableMotionModelKeyframeSaveToBuffer(keyframe, NULL, &status);
    nanoem_mutable_motion_model_keyframe_constraint_state_t *state =
        nanoemMutableMotionModelKeyframeConstraintStateCreate(nanoemMutableMotionModelKeyframeGetOriginObject(keyframe), &status);
    nanoemMutableMotionModelKeyframeAddConstraintState(keyframe, state, NULL);
    nanoemMutableMotionModelKeyframeRemoveConstraintState(keyframe, state, NULL);
    nanoem_mutable_motion_outside_parent_t *op =
        nanoemMutableMotionOutsideParentCreateFromModelKeyframe(nanoemMutableMotionModelKeyframeGetOriginObject(keyframe), &status);
    nanoemMutableMotionModelKeyframeAddOutsideParent(keyframe, op, NULL);
    nanoemMutableMotionModelKeyframeRemoveOutsideParent(keyframe, op, NULL);
    nanoem_mutable_motion_effect_parameter_t *parameter = nanoemMutableMotionEffectParameterCreateFromModelKeyframe(
        nanoemMutableMotionModelKeyframeGetOriginObject(keyframe), &status);
    nanoemMutableMotionModelKeyframeAddEffectParameter(keyframe, parameter, NULL);
    nanoemMutableMotionModelKeyframeRemoveEffectParameter(keyframe, parameter, NULL);
    nanoemMutableMotionModelKeyframeConstraintStateDestroy(state);
    nanoemMutableMotionOutsideParentDestroy(op);
    nanoemMutableMotionEffectParameterDestroy(parameter);
}

TEST_CASE("mutable_model_keyframe_manipulate", "[nanoem]")
{
    MotionScope scope;
    nanoem_mutable_motion_t *motion = scope.newMotion();
    nanoem_mutable_motion_model_keyframe_t *keyframe = scope.newModelKeyframe();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableMotionAddModelKeyframe(NULL, NULL, 0, &status);
    nanoemMutableMotionAddModelKeyframe(motion, NULL, 0, &status);
    nanoemMutableMotionRemoveModelKeyframe(NULL, NULL, &status);
    nanoemMutableMotionRemoveModelKeyframe(motion, NULL, &status);
    nanoemMutableMotionRemoveModelKeyframe(motion, keyframe, &status);
    CHECK(status == NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_NOT_FOUND);
}

TEST_CASE("mutable_model_keyframe_generate_vmd", "[nanoem]")
{
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_t *mutable_motion = scope.newMotion();
    nanoem_mutable_motion_model_keyframe_t *mutable_keyframe = scope.newModelKeyframe();
    /* create */
    nanoemMutableMotionModelKeyframeSetVisible(mutable_keyframe, nanoem_true);
    nanoemMutableMotionAddModelKeyframe(mutable_motion, mutable_keyframe, 42, &status);
    CHECK(status == NANOEM_STATUS_SUCCESS);
    SECTION("duplication should reject")
    {
        nanoemMutableMotionAddModelKeyframe(mutable_motion, mutable_keyframe, 42, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_ALREADY_EXISTS);
    }
    SECTION("all keyframe properties should get")
    {
        nanoem_motion_model_keyframe_t *origin_keyframe = nanoemMutableMotionModelKeyframeGetOriginObject(mutable_keyframe);
        CHECK(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionModelKeyframeGetKeyframeObject(origin_keyframe)) == 42);
        CHECK(nanoemMotionModelKeyframeIsVisible(origin_keyframe));
    }
    SECTION("saved keyframe data should be able to parse")
    {
        nanoem_mutable_buffer_t *mutable_buffer = scope.newBuffer();
        nanoemMutableMotionModelKeyframeSaveToBuffer(mutable_keyframe, mutable_buffer, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoem_buffer_t *buffer = scope.newBuffer(mutable_buffer);
        nanoem_motion_t *motion = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_motion_model_keyframe_t *generated_keyframe = nanoemMotionModelKeyframeCreate(motion, &status);
        nanoemMotionModelKeyframeParseVMD(generated_keyframe, buffer, 3, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        CHECK(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionModelKeyframeGetKeyframeObject(generated_keyframe)) == 45);
        CHECK(nanoemMotionModelKeyframeIsVisible(generated_keyframe));
        nanoemMotionModelKeyframeDestroy(generated_keyframe);
    }
    SECTION("duplicated remove should reject")
    {
        nanoem_rsize_t num_keyframes;
        nanoem_motion_t *motion = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoemMutableMotionRemoveModelKeyframe(mutable_motion, mutable_keyframe, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMotionGetAllBoneKeyframeObjects(motion, &num_keyframes);
        CHECK(num_keyframes == 0);
        nanoemMutableMotionRemoveModelKeyframe(mutable_motion, mutable_keyframe, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_NOT_FOUND);
    }
}

TEST_CASE("mutable_model_keyframe_generate_nmd", "[nanoem]")
{
    MotionScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_buffer_t *mutable_buffer = scope.newBuffer();
    {
        nanoem_mutable_motion_t *mutable_motion = scope.newMotion();
        nanoem_motion_t *motion = nanoemMutableMotionGetOriginObject(mutable_motion);
        nanoem_mutable_motion_model_keyframe_t *mutable_keyframe = nanoemMutableMotionModelKeyframeCreate(motion, &status);
        nanoemMutableMotionModelKeyframeSetAddBlendEnabled(mutable_keyframe, nanoem_true);
        nanoemMutableMotionModelKeyframeSetEdgeColor(mutable_keyframe, 0);
        nanoemMutableMotionModelKeyframeSetEdgeScaleFactor(mutable_keyframe, 0.42f);
        nanoemMutableMotionModelKeyframeSetVisible(mutable_keyframe, nanoem_true);
        nanoemMutableMotionModelKeyframeSetPhysicsSimulationEnabled(mutable_keyframe, nanoem_false);
        nanoem_mutable_motion_model_keyframe_constraint_state_t *state = nanoemMutableMotionModelKeyframeConstraintStateCreate(
            nanoemMutableMotionModelKeyframeGetOriginObject(mutable_keyframe), &status);
        nanoem_unicode_string_t *name = scope.newString("model_constraint_state_disabled");
        nanoemMutableMotionModelKeyframeConstraintStateSetBoneName(state, name, &status);
        {
            nanoem_mutable_motion_bone_keyframe_t *bone_keyframe = nanoemMutableMotionBoneKeyframeCreate(motion, &status);
            nanoemMutableMotionAddBoneKeyframe(mutable_motion, bone_keyframe, name, 0, &status);
            nanoemMutableMotionBoneKeyframeDestroy(bone_keyframe);
        }
        nanoemMutableMotionModelKeyframeConstraintStateSetEnabled(state, nanoem_false);
        nanoemMutableMotionModelKeyframeAddConstraintState(mutable_keyframe, state, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableMotionModelKeyframeConstraintStateDestroy(state);
        {
            {
                nanoem_mutable_motion_model_keyframe_constraint_state_t *state =
                    nanoemMutableMotionModelKeyframeConstraintStateCreate(
                        nanoemMutableMotionModelKeyframeGetOriginObject(mutable_keyframe), &status);
                nanoem_unicode_string_t *name = scope.newString("model_constraint_state_enabled");
                nanoemMutableMotionModelKeyframeConstraintStateSetBoneName(state, name, &status);
                {
                    nanoem_mutable_motion_bone_keyframe_t *bone_keyframe =
                        nanoemMutableMotionBoneKeyframeCreate(motion, &status);
                    nanoemMutableMotionAddBoneKeyframe(mutable_motion, bone_keyframe, name, 0, &status);
                    nanoemMutableMotionBoneKeyframeDestroy(bone_keyframe);
                }
                nanoemMutableMotionModelKeyframeConstraintStateSetEnabled(state, nanoem_true);
                nanoemMutableMotionModelKeyframeAddConstraintState(mutable_keyframe, state, &status);
                CHECK(status == NANOEM_STATUS_SUCCESS);
                nanoemMutableMotionModelKeyframeConstraintStateDestroy(state);
            }
            {
                nanoem_mutable_motion_outside_parent_t *op = nanoemMutableMotionOutsideParentCreateFromModelKeyframe(
                    nanoemMutableMotionModelKeyframeGetOriginObject(mutable_keyframe), &status);
                nanoem_unicode_string_t *base_bone_name = scope.newString("outside_parent_base_bone_name_first");
                nanoemMutableMotionOutsideParentSetSubjectBoneName(op, base_bone_name, &status);
                nanoem_unicode_string_t *bone_name = scope.newString("outside_parent_parent_bone_name_first");
                nanoemMutableMotionOutsideParentSetTargetBoneName(op, bone_name, &status);
                nanoem_unicode_string_t *object_name = scope.newString("outside_parent_parent_object_name_first");
                nanoemMutableMotionOutsideParentSetTargetObjectName(op, object_name, &status);
                nanoemMutableMotionModelKeyframeAddOutsideParent(mutable_keyframe, op, &status);
                CHECK(status == NANOEM_STATUS_SUCCESS);
                nanoemMutableMotionOutsideParentDestroy(op);
            }
            {
                nanoem_mutable_motion_outside_parent_t *op = nanoemMutableMotionOutsideParentCreateFromModelKeyframe(
                    nanoemMutableMotionModelKeyframeGetOriginObject(mutable_keyframe), &status);
                nanoem_unicode_string_t *base_bone_name = scope.newString("outside_parent_base_bone_name_second");
                nanoemMutableMotionOutsideParentSetSubjectBoneName(op, base_bone_name, &status);
                nanoem_unicode_string_t *bone_name = scope.newString("outside_parent_parent_bone_name_second");
                nanoemMutableMotionOutsideParentSetTargetBoneName(op, bone_name, &status);
                nanoem_unicode_string_t *object_name = scope.newString("outside_parent_parent_object_name_second");
                nanoemMutableMotionOutsideParentSetTargetObjectName(op, object_name, &status);
                nanoemMutableMotionModelKeyframeAddOutsideParent(mutable_keyframe, op, &status);
                CHECK(status == NANOEM_STATUS_SUCCESS);
                nanoemMutableMotionOutsideParentDestroy(op);
            }
            {
                nanoem_mutable_motion_effect_parameter_t *parameter = nanoemMutableMotionEffectParameterCreateFromModelKeyframe(
                    nanoemMutableMotionModelKeyframeGetOriginObject(mutable_keyframe), &status);
                nanoem_unicode_string_t *boolean_parameter_name = scope.newString("boolean_parameter");
                nanoemMutableMotionEffectParameterSetName(parameter, boolean_parameter_name, &status);
                nanoemMutableMotionEffectParameterSetType(parameter, NANOEM_MOTION_EFFECT_PARAMETER_TYPE_BOOL);
                nanoem_i32_t boolean_value = 1;
                nanoemMutableMotionEffectParameterSetValue(parameter, &boolean_value);
                nanoemMutableMotionModelKeyframeAddEffectParameter(mutable_keyframe, parameter, &status);
                CHECK(status == NANOEM_STATUS_SUCCESS);
                nanoemMutableMotionEffectParameterDestroy(parameter);
            }
            {
                nanoem_mutable_motion_effect_parameter_t *parameter = nanoemMutableMotionEffectParameterCreateFromModelKeyframe(
                    nanoemMutableMotionModelKeyframeGetOriginObject(mutable_keyframe), &status);
                nanoem_unicode_string_t *integer_parameter_name = scope.newString("integer_parameter");
                nanoemMutableMotionEffectParameterSetName(parameter, integer_parameter_name, &status);
                nanoemMutableMotionEffectParameterSetType(parameter, NANOEM_MOTION_EFFECT_PARAMETER_TYPE_INT);
                nanoem_i32_t integer_value = 42;
                nanoemMutableMotionEffectParameterSetValue(parameter, &integer_value);
                nanoemMutableMotionModelKeyframeAddEffectParameter(mutable_keyframe, parameter, &status);
                CHECK(status == NANOEM_STATUS_SUCCESS);
                nanoemMutableMotionEffectParameterDestroy(parameter);
            }
            {
                nanoem_mutable_motion_effect_parameter_t *parameter = nanoemMutableMotionEffectParameterCreateFromModelKeyframe(
                    nanoemMutableMotionModelKeyframeGetOriginObject(mutable_keyframe), &status);
                nanoem_unicode_string_t *float_parameter_name = scope.newString("float_parameter");
                nanoemMutableMotionEffectParameterSetName(parameter, float_parameter_name, &status);
                nanoemMutableMotionEffectParameterSetType(parameter, NANOEM_MOTION_EFFECT_PARAMETER_TYPE_FLOAT);
                nanoem_f32_t float_value = 42.0f;
                nanoemMutableMotionEffectParameterSetValue(parameter, &float_value);
                nanoemMutableMotionModelKeyframeAddEffectParameter(mutable_keyframe, parameter, &status);
                CHECK(status == NANOEM_STATUS_SUCCESS);
                nanoemMutableMotionEffectParameterDestroy(parameter);
            }
            {
                nanoem_mutable_motion_effect_parameter_t *parameter = nanoemMutableMotionEffectParameterCreateFromModelKeyframe(
                    nanoemMutableMotionModelKeyframeGetOriginObject(mutable_keyframe), &status);
                nanoem_unicode_string_t *vector4_parameter_name = scope.newString("vector4_parameter");
                nanoemMutableMotionEffectParameterSetName(parameter, vector4_parameter_name, &status);
                nanoemMutableMotionEffectParameterSetType(parameter, NANOEM_MOTION_EFFECT_PARAMETER_TYPE_VECTOR4);
                nanoem_f32_t vector4_value[] = { 0.1f, 0.2f, 0.3f, 0.4f };
                nanoemMutableMotionEffectParameterSetValue(parameter, vector4_value);
                nanoemMutableMotionModelKeyframeAddEffectParameter(mutable_keyframe, parameter, &status);
                CHECK(status == NANOEM_STATUS_SUCCESS);
                nanoemMutableMotionEffectParameterDestroy(parameter);
            }
        }
        nanoemMutableMotionAddModelKeyframe(mutable_motion, mutable_keyframe, 42, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableMotionModelKeyframeDestroy(mutable_keyframe);
        nanoemMutableMotionSaveToBufferNMD(mutable_motion, mutable_buffer, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
    }
    {
        nanoem_motion_t *new_motion = nanoemMutableMotionGetOriginObject(scope.newMotion());
        nanoem_buffer_t *buffer = scope.newBuffer(mutable_buffer);
        nanoemMotionLoadFromBufferNMD(new_motion, buffer, 1, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        const nanoem_motion_model_keyframe_t *generated_keyframe = nanoemMotionFindModelKeyframeObject(new_motion, 43);
        CHECK(generated_keyframe);
        CHECK(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionModelKeyframeGetKeyframeObject(generated_keyframe)) == 43);
        CHECK(nanoemMotionModelKeyframeIsAddBlendEnabled(generated_keyframe));
        CHECK(nanoemMotionModelKeyframeGetEdgeScaleFactor(generated_keyframe) == Approx(0.42f));
        CHECK(nanoemMotionModelKeyframeIsVisible(generated_keyframe));
        CHECK(nanoemMotionModelKeyframeIsPhysicsSimulationEnabled(generated_keyframe) == nanoem_false);
        {
            nanoem_rsize_t num_states;
            nanoem_motion_model_keyframe_constraint_state_t *const *states =
                nanoemMotionModelKeyframeGetAllConstraintStateObjects(generated_keyframe, &num_states);
            CHECK(states);
            CHECK(num_states == 2);
            {
                const std::string &name = scope.describe(nanoemMotionModelKeyframeConstraintStateGetBoneName(states[0]));
                CHECK_THAT(name, Catch::Equals("model_constraint_state_disabled"));
                CHECK_FALSE(nanoemMotionModelKeyframeConstraintStateIsEnabled(states[0]));
            }
            {
                const std::string &name = scope.describe(nanoemMotionModelKeyframeConstraintStateGetBoneName(states[1]));
                CHECK_THAT(name, Catch::Equals("model_constraint_state_enabled"));
                CHECK(nanoemMotionModelKeyframeConstraintStateIsEnabled(states[1]));
            }
        }
        {
            nanoem_rsize_t num_ops;
            nanoem_motion_outside_parent_t *const *ops =
                nanoemMotionModelKeyframeGetAllOutsideParentObjects(generated_keyframe, &num_ops);
            CHECK(ops);
            CHECK(num_ops == 2);
            {
                const std::string &base_bone_name = scope.describe(nanoemMotionOutsideParentGetTargetBoneName(ops[0]));
                CHECK_THAT(base_bone_name, Catch::Equals("outside_parent_parent_bone_name_first"));
                const std::string &bone_name = scope.describe(nanoemMotionOutsideParentGetTargetBoneName(ops[0]));
                CHECK_THAT(bone_name, Catch::Equals("outside_parent_parent_bone_name_first"));
                const std::string &object_name = scope.describe(nanoemMotionOutsideParentGetTargetObjectName(ops[0]));
                CHECK_THAT(object_name, Catch::Equals("outside_parent_parent_object_name_first"));
            }
            {
                const std::string &base_bone_name = scope.describe(nanoemMotionOutsideParentGetTargetBoneName(ops[1]));
                CHECK_THAT(base_bone_name, Catch::Equals("outside_parent_parent_bone_name_second"));
                const std::string &bone_name = scope.describe(nanoemMotionOutsideParentGetTargetBoneName(ops[1]));
                CHECK_THAT(bone_name, Catch::Equals("outside_parent_parent_bone_name_second"));
                const std::string &object_name = scope.describe(nanoemMotionOutsideParentGetTargetObjectName(ops[1]));
                CHECK_THAT(object_name, Catch::Equals("outside_parent_parent_object_name_second"));
            }
        }
        {
            nanoem_rsize_t num_parameters;
            nanoem_motion_effect_parameter_t *const *parameters =
                nanoemMotionModelKeyframeGetAllEffectParameterObjects(generated_keyframe, &num_parameters);
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
