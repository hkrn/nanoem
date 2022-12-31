/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_bone_basic", "[nanoem]")
{
    CHECK_FALSE(nanoemModelBoneGetEffectorBoneObject(NULL));
    CHECK(nanoemModelBoneGetDestinationOrigin(NULL));
    CHECK(nanoemModelBoneGetFixedAxis(NULL));
    CHECK(nanoemModelBoneGetStageIndex(NULL) == 0);
    CHECK(nanoemModelBoneGetLocalXAxis(NULL));
    CHECK(nanoemModelBoneGetLocalZAxis(NULL));
    CHECK_FALSE(nanoemModelBoneGetName(NULL, NANOEM_LANGUAGE_TYPE_JAPANESE));
    CHECK_FALSE(nanoemModelBoneGetName(NULL, NANOEM_LANGUAGE_TYPE_ENGLISH));
    CHECK(nanoemModelBoneGetOrigin(NULL));
    CHECK_FALSE(nanoemModelBoneGetParentBoneObject(NULL));
    CHECK_FALSE(nanoemModelBoneGetInherentParentBoneObject(NULL));
    CHECK_FALSE(nanoemModelBoneGetTargetBoneObject(NULL));
    CHECK(nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(NULL)) == -1);
    CHECK(nanoemModelBoneGetInherentCoefficient(NULL) == Approx(0.0f));
    CHECK_FALSE(nanoemModelBoneIsUserHandleable(NULL));
    CHECK_FALSE(nanoemModelBoneIsMovable(NULL));
    CHECK_FALSE(nanoemModelBoneIsRotateable(NULL));
    CHECK_FALSE(nanoemModelBoneIsVisible(NULL));
    CHECK_FALSE(nanoemModelBoneHasFixedAxis(NULL));
    CHECK_FALSE(nanoemModelBoneHasLocalAxes(NULL));
    CHECK_FALSE(nanoemModelBoneIsAffectedByPhysicsSimulation(NULL));
    CHECK_FALSE(nanoemModelBoneHasExternalParentBone(NULL));
    CHECK_FALSE(nanoemModelBoneGetModelObject(NULL));
    CHECK_FALSE(nanoemModelBoneGetModelObjectMutable(NULL));
    nanoemModelBoneDestroy(NULL);
}

TEST_CASE("mutable_bone_null", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    CHECK_FALSE(nanoemMutableModelBoneGetOriginObject(NULL));
    nanoemMutableModelBoneSetAffectedByPhysicsSimulation(NULL, nanoem_false);
    nanoemMutableModelBoneSetConstraintObject(NULL, NULL);
    nanoemMutableModelBoneSetDestinationOrigin(NULL, NULL);
    nanoemMutableModelBoneSetEffectorBoneObject(NULL, NULL);
    nanoemMutableModelBoneSetFixedAxis(NULL, NULL);
    nanoemMutableModelBoneSetInherentCoefficient(NULL, 0);
    nanoemMutableModelBoneSetInherentParentBoneObject(NULL, NULL);
    nanoemMutableModelBoneSetLocalXAxis(NULL, NULL);
    nanoemMutableModelBoneSetLocalZAxis(NULL, NULL);
    nanoemMutableModelBoneSetMovable(NULL, nanoem_false);
    nanoemMutableModelBoneSetName(NULL, NULL, NANOEM_LANGUAGE_TYPE_JAPANESE, NULL);
    nanoemMutableModelBoneSetOrigin(NULL, NULL);
    nanoemMutableModelBoneSetParentBoneObject(NULL, NULL);
    nanoemMutableModelBoneSetRotateable(NULL, nanoem_false);
    nanoemMutableModelBoneSetStageIndex(NULL, 0);
    nanoemMutableModelBoneSetTargetBoneObject(NULL, NULL);
    nanoemMutableModelBoneSetUserHandleable(NULL, nanoem_false);
    nanoemMutableModelBoneSetVisible(NULL, nanoem_false);
    nanoemMutableModelBoneSetOffsetRelative(NULL, nanoem_false);
    nanoemMutableModelBoneSetConstraintEnabled(NULL, nanoem_false);
    nanoemMutableModelBoneSetLocalInherentEnabled(NULL, nanoem_false);
    nanoemMutableModelBoneSetInherentTranslationEnabled(NULL, nanoem_false);
    nanoemMutableModelBoneSetInherentOrientationEnabled(NULL, nanoem_false);
    nanoemMutableModelBoneSetFixedAxisEnabled(NULL, nanoem_false);
    nanoemMutableModelBoneSetLocalAxesEnabled(NULL, nanoem_false);
    nanoemMutableModelBoneSaveToBuffer(NULL, NULL, &status);
    nanoemMutableModelBoneDestroy(NULL);
}

TEST_CASE("mutable_bone_null_values", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_bone_t *bone = scope.newBone();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelBoneSetConstraintObject(bone, NULL);
    nanoemMutableModelBoneSetDestinationOrigin(bone, NULL);
    nanoemMutableModelBoneSetEffectorBoneObject(bone, NULL);
    nanoemMutableModelBoneSetFixedAxis(bone, NULL);
    nanoemMutableModelBoneSetInherentParentBoneObject(bone, NULL);
    nanoemMutableModelBoneSetLocalXAxis(bone, NULL);
    nanoemMutableModelBoneSetLocalZAxis(bone, NULL);
    nanoemMutableModelBoneSetName(bone, NULL, NANOEM_LANGUAGE_TYPE_JAPANESE, NULL);
    nanoemMutableModelBoneSetOrigin(bone, NULL);
    nanoemMutableModelBoneSetParentBoneObject(bone, NULL);
    nanoemMutableModelBoneSetTargetBoneObject(bone, NULL);
    nanoemMutableModelBoneSaveToBuffer(bone, NULL, &status);
}

TEST_CASE("mutable_bone_manipulate", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_t *model = scope.newModel();
    nanoem_mutable_model_bone_t *first_bone = scope.newBone();
    nanoem_mutable_model_bone_t *second_bone = scope.newBone();
    nanoem_mutable_model_bone_t *third_bone = scope.newBone();
    nanoem_model_bone_t *const *bones;
    nanoem_rsize_t num_bones;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelInsertBoneObject(model, first_bone, -1, &status);
    nanoemMutableModelInsertBoneObject(model, second_bone, 1, &status);
    nanoemMutableModelInsertBoneObject(model, third_bone, 0, &status);
    CHECK(status == NANOEM_STATUS_SUCCESS);
    SECTION("duplication should reject")
    {
        nanoemMutableModelInsertBoneObject(model, first_bone, -1, &status);
        REQUIRE(status == NANOEM_STATUS_ERROR_MODEL_BONE_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        bones = nanoemModelGetAllBoneObjects(nanoemMutableModelGetOriginObject(model), &num_bones);
        REQUIRE(num_bones == 3);
        CHECK(bones[0] == nanoemMutableModelBoneGetOriginObject(third_bone));
        CHECK(bones[1] == nanoemMutableModelBoneGetOriginObject(first_bone));
        CHECK(bones[2] == nanoemMutableModelBoneGetOriginObject(second_bone));
    }
    SECTION("remove")
    {
        nanoemMutableModelRemoveBoneObject(model, first_bone, &status);
        REQUIRE(status == NANOEM_STATUS_SUCCESS);
        bones = nanoemModelGetAllBoneObjects(nanoemMutableModelGetOriginObject(model), &num_bones);
        REQUIRE(num_bones == 2);
        CHECK(bones[0] == nanoemMutableModelBoneGetOriginObject(third_bone));
        CHECK(bones[2] == nanoemMutableModelBoneGetOriginObject(second_bone));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableModelRemoveBoneObject(model, first_bone, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableModelRemoveBoneObject(model, first_bone, &status);
        REQUIRE(status == NANOEM_STATUS_ERROR_MODEL_BONE_NOT_FOUND);
    }
}

TEST_CASE("mutable_bone_copy_name", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_bone_t *first_bone = scope.appendedBone();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    struct input_t {
        const char *input;
        const char *expected;
    };
    input_t input[] = { { "0123456789012345678", "0123456789012345678" /* 19bytes */ },
        { "01234567890123456789", "01234567890123456789" /* 20bytes */ },
        { "012345678901234567890", "01234567890123456789" /* 21bytes -> 20bytes */ } };
    for (size_t j = NANOEM_LANGUAGE_TYPE_FIRST_ENUM; j < NANOEM_LANGUAGE_TYPE_MAX_ENUM; j++) {
        nanoem_language_type_t language = (nanoem_language_type_t) j;
        for (size_t i = 0; i < sizeof(input) / sizeof(input[0]); i++) {
            const char *name = input[i].input;
            nanoem_unicode_string_t *s = scope.newString(name);
            nanoemMutableModelBoneSetName(first_bone, s, language, &status);
            CHECK_THAT(
                scope.describe(nanoemModelBoneGetName(scope.copyBone(NANOEM_MODEL_FORMAT_TYPE_PMD_1_0), language)),
                Catch::Equals(input[i].expected));
        }
    }
}

TEST_CASE("mutable_bone_copy_origin", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_bone_t *first_bone = scope.appendedBone();
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelBoneSetOrigin(first_bone, value);
    CHECK_THAT(nanoemModelBoneGetOrigin(scope.copyBone()), Equals(value));
}

TEST_CASE("mutable_bone_copy_stage_index", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_bone_t *first_bone = scope.appendedBone();
    nanoemMutableModelBoneSetStageIndex(first_bone, 42);
    CHECK(nanoemModelBoneGetStageIndex(scope.copyBone()) == 42);
}

TEST_CASE("mutable_bone_copy_parent_bone", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_bone_t *first_bone = scope.appendedBone();
    nanoem_mutable_model_bone_t *second_bone = scope.appendedBone("bone0");
    nanoemMutableModelBoneSetParentBoneObject(first_bone, nanoemMutableModelBoneGetOriginObject(second_bone));
    CHECK_THAT(nanoemModelBoneGetParentBoneObject(scope.copyBone()), EqualsBoneName("bone0"));
    nanoemMutableModelBoneSetParentBoneObject(first_bone, nullptr);
    CHECK(nanoemModelBoneGetParentBoneObject(scope.copyBone()) == nullptr);
}

TEST_CASE("mutable_bone_copy_constraint_bone", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_bone_t *first_bone = scope.appendedBone();
    nanoem_mutable_model_constraint_t *constraint = scope.newConstraint();
    nanoemMutableModelBoneSetConstraintObject(first_bone, constraint);
    nanoemMutableModelBoneSetConstraintEnabled(first_bone, 1);
    CHECK(nanoemModelBoneGetConstraintObject(scope.copyBone()) != nullptr);
    nanoemMutableModelBoneSetConstraintEnabled(first_bone, 0);
    CHECK(nanoemModelBoneGetConstraintObject(scope.copyBone()) == nullptr);
}

TEST_CASE("mutable_bone_copy_inherent_translation_bone", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_bone_t *first_bone = scope.appendedBone();
    nanoem_mutable_model_bone_t *second_bone = scope.appendedBone("bone0");
    nanoemMutableModelBoneSetInherentParentBoneObject(first_bone, nanoemMutableModelBoneGetOriginObject(second_bone));
    nanoemMutableModelBoneSetInherentCoefficient(first_bone, 0.42f);
    nanoemMutableModelBoneSetInherentTranslationEnabled(first_bone, 1);
    const nanoem_model_bone_t *enabled_bone = scope.copyBone();
    CHECK(nanoemModelBoneHasInherentTranslation(enabled_bone) == 1);
    CHECK(nanoemModelBoneHasInherentOrientation(enabled_bone) == 0);
    CHECK_THAT(nanoemModelBoneGetInherentParentBoneObject(enabled_bone), EqualsBoneName("bone0"));
    CHECK(nanoemModelBoneGetInherentCoefficient(enabled_bone) == Approx(0.42f));
    nanoemMutableModelBoneSetInherentTranslationEnabled(first_bone, 0);
    const nanoem_model_bone_t *disabled_bone = scope.copyBone();
    CHECK(nanoemModelBoneHasInherentTranslation(disabled_bone) == 0);
    CHECK(nanoemModelBoneHasInherentOrientation(disabled_bone) == 0);
    CHECK(nanoemModelBoneGetInherentParentBoneObject(disabled_bone) == nullptr);
    CHECK(nanoemModelBoneGetInherentCoefficient(disabled_bone) == Approx(1.0f));
}

TEST_CASE("mutable_bone_copy_inherent_orientation_bone", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_bone_t *first_bone = scope.appendedBone();
    nanoem_mutable_model_bone_t *second_bone = scope.appendedBone("bone0");
    nanoemMutableModelBoneSetInherentParentBoneObject(first_bone, nanoemMutableModelBoneGetOriginObject(second_bone));
    nanoemMutableModelBoneSetInherentCoefficient(first_bone, -0.42f);
    nanoemMutableModelBoneSetInherentOrientationEnabled(first_bone, 1);
    const nanoem_model_bone_t *enabled_bone = scope.copyBone();
    CHECK(nanoemModelBoneHasInherentTranslation(enabled_bone) == 0);
    CHECK(nanoemModelBoneHasInherentOrientation(enabled_bone) == 1);
    CHECK_THAT(nanoemModelBoneGetInherentParentBoneObject(enabled_bone), EqualsBoneName("bone0"));
    CHECK(nanoemModelBoneGetInherentCoefficient(enabled_bone) == Approx(-0.42f));
    nanoemMutableModelBoneSetInherentOrientationEnabled(first_bone, 0);
    const nanoem_model_bone_t *disabled_bone = scope.copyBone();
    CHECK(nanoemModelBoneHasInherentTranslation(disabled_bone) == 0);
    CHECK(nanoemModelBoneHasInherentOrientation(disabled_bone) == 0);
    CHECK(nanoemModelBoneGetInherentParentBoneObject(disabled_bone) == nullptr);
    CHECK(nanoemModelBoneGetInherentCoefficient(disabled_bone) == Approx(1.0f));
}

TEST_CASE("mutable_bone_copy_destination_origin", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_bone_t *first_bone = scope.appendedBone();
    nanoem_mutable_model_bone_t *second_bone = scope.appendedBone("bone0");
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelBoneSetDestinationOrigin(first_bone, value);
    const nanoem_model_bone_t *enabled_bone = scope.copyBone();
    CHECK_THAT(nanoemModelBoneGetDestinationOrigin(enabled_bone), Equals(value));
    CHECK(nanoemModelBoneGetTargetBoneObject(enabled_bone) == nullptr);
    nanoemMutableModelBoneSetTargetBoneObject(first_bone, nanoemMutableModelBoneGetOriginObject(second_bone));
    const nanoem_model_bone_t *disabled_bone = scope.copyBone();
    CHECK_THAT(nanoemModelBoneGetDestinationOrigin(disabled_bone), EqualsOne(0));
    CHECK_THAT(nanoemModelBoneGetTargetBoneObject(disabled_bone), EqualsBoneName("bone0"));
}

TEST_CASE("mutable_bone_copy_fixed_axis", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_bone_t *first_bone = scope.appendedBone();
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelBoneSetFixedAxis(first_bone, value);
    nanoemMutableModelBoneSetFixedAxisEnabled(first_bone, 1);
    const nanoem_model_bone_t *enabled_bone = scope.copyBone();
    CHECK(nanoemModelBoneHasFixedAxis(enabled_bone));
    CHECK_THAT(nanoemModelBoneGetFixedAxis(enabled_bone), Equals(value));
    nanoemMutableModelBoneSetFixedAxisEnabled(first_bone, 0);
    const nanoem_model_bone_t *disabled_bone = scope.copyBone();
    CHECK_FALSE(nanoemModelBoneHasFixedAxis(disabled_bone));
    CHECK_THAT(nanoemModelBoneGetFixedAxis(disabled_bone), EqualsOne(0));
}

TEST_CASE("mutable_bone_copy_local_axes", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_bone_t *first_bone = scope.appendedBone();
    float value_x[] = { 1, 2, 3, 0 }, value_z[] = { 3, 2, 1, 0 };
    nanoemMutableModelBoneSetLocalXAxis(first_bone, value_x);
    nanoemMutableModelBoneSetLocalZAxis(first_bone, value_z);
    nanoemMutableModelBoneSetLocalAxesEnabled(first_bone, 1);
    const nanoem_model_bone_t *enabled_bone = scope.copyBone();
    CHECK(nanoemModelBoneHasLocalAxes(enabled_bone));
    CHECK_THAT(nanoemModelBoneGetLocalXAxis(enabled_bone), Equals(value_x));
    CHECK_THAT(nanoemModelBoneGetLocalZAxis(enabled_bone), Equals(value_z));
    nanoemMutableModelBoneSetLocalAxesEnabled(first_bone, 0);
    const nanoem_model_bone_t *disabled_bone = scope.copyBone();
    CHECK_FALSE(nanoemModelBoneHasLocalAxes(disabled_bone));
    CHECK_THAT(nanoemModelBoneGetLocalXAxis(disabled_bone), Equals(1, 0, 0, 0));
    CHECK_THAT(nanoemModelBoneGetLocalZAxis(disabled_bone), Equals(0, 0, 1, 0));
}

TEST_CASE("mutable_bone_copy_is_movable", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_bone_t *first_bone = scope.appendedBone();
    nanoemMutableModelBoneSetMovable(first_bone, 1);
    CHECK(nanoemModelBoneIsMovable(scope.copyBone()));
}

TEST_CASE("mutable_bone_copy_is_rotateable", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_bone_t *first_bone = scope.appendedBone();
    nanoemMutableModelBoneSetRotateable(first_bone, 1);
    CHECK(nanoemModelBoneIsRotateable(scope.copyBone()));
}

TEST_CASE("mutable_bone_copy_is_visible", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_bone_t *first_bone = scope.appendedBone();
    nanoemMutableModelBoneSetVisible(first_bone, 1);
    CHECK(nanoemModelBoneIsVisible(scope.copyBone()));
}

TEST_CASE("mutable_bone_copy_is_user_handleable", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_bone_t *first_bone = scope.appendedBone();
    nanoemMutableModelBoneSetUserHandleable(first_bone, 1);
    CHECK(nanoemModelBoneIsUserHandleable(scope.copyBone()));
}

TEST_CASE("mutable_bone_copy_is_affected_by_physics_simulation", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_bone_t *first_bone = scope.appendedBone();
    nanoemMutableModelBoneSetAffectedByPhysicsSimulation(first_bone, 1);
    CHECK(nanoemModelBoneIsAffectedByPhysicsSimulation(scope.copyBone()));
}
