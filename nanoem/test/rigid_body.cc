/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_rigid_body_basic", "[nanoem]")
{
    CHECK(nanoemModelRigidBodyGetAngularDamping(NULL) == Approx(0));
    CHECK_FALSE(nanoemModelRigidBodyGetBoneObject(NULL));
    CHECK(nanoemModelRigidBodyGetCollisionGroupId(NULL) == 0);
    CHECK(nanoemModelRigidBodyGetCollisionMask(NULL) == 0);
    CHECK(nanoemModelRigidBodyGetFriction(NULL) == Approx(0));
    CHECK(nanoemModelRigidBodyGetLinearDamping(NULL) == Approx(0));
    CHECK(nanoemModelRigidBodyGetMass(NULL) == Approx(0));
    CHECK_FALSE(nanoemModelRigidBodyGetName(NULL, NANOEM_LANGUAGE_TYPE_JAPANESE));
    CHECK_FALSE(nanoemModelRigidBodyGetName(NULL, NANOEM_LANGUAGE_TYPE_ENGLISH));
    CHECK(nanoemModelRigidBodyGetTransformType(NULL) == NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_UNKNOWN);
    CHECK(nanoemModelRigidBodyGetOrigin(NULL));
    CHECK(nanoemModelRigidBodyGetRestitution(NULL) == Approx(0));
    CHECK(nanoemModelRigidBodyGetOrientation(NULL));
    CHECK(nanoemModelRigidBodyGetShapeType(NULL) == NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_UNKNOWN);
    CHECK(nanoemModelObjectGetIndex(nanoemModelRigidBodyGetModelObject(NULL)) == -1);
    CHECK_FALSE(nanoemModelRigidBodyIsBoneRelativePosition(NULL));
    CHECK_FALSE(nanoemModelRigidBodyGetModelObject(NULL));
    CHECK_FALSE(nanoemModelRigidBodyGetModelObjectMutable(NULL));
    nanoemModelRigidBodyDestroy(NULL);
}

TEST_CASE("mutable_rigid_body_null", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    CHECK_FALSE(nanoemMutableModelRigidBodyGetOriginObject(NULL));
    nanoemMutableModelRigidBodySetAngularDamping(NULL, 0);
    nanoemMutableModelRigidBodySetBoneObject(NULL, NULL);
    nanoemMutableModelRigidBodySetCollisionGroupId(NULL, 0);
    nanoemMutableModelRigidBodySetCollisionMask(NULL, 0);
    nanoemMutableModelRigidBodySetFriction(NULL, 0);
    nanoemMutableModelRigidBodySetLinearDamping(NULL, 0);
    nanoemMutableModelRigidBodySetMass(NULL, 0);
    nanoemMutableModelRigidBodySetName(NULL, NULL, NANOEM_LANGUAGE_TYPE_JAPANESE, NULL);
    nanoemMutableModelRigidBodySetOrientation(NULL, NULL);
    nanoemMutableModelRigidBodySetOrigin(NULL, NULL);
    nanoemMutableModelRigidBodySetRestitution(NULL, 0);
    nanoemMutableModelRigidBodySetShapeSize(NULL, NULL);
    nanoemMutableModelRigidBodySetShapeType(NULL, NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_BOX);
    nanoemMutableModelRigidBodySetTransformType(NULL, NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_TO_SIMULATION);
    nanoemMutableModelRigidBodySaveToBuffer(NULL, NULL, &status);
    nanoemMutableModelRigidBodyDestroy(NULL);
}

TEST_CASE("mutable_rigid_body_null_values", "[nanoem]")
{
    ModelScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_rigid_body_t *rigid_body = scope.newRigidBody();
    nanoemMutableModelRigidBodySetBoneObject(rigid_body, NULL);
    nanoemMutableModelRigidBodySetName(rigid_body, NULL, NANOEM_LANGUAGE_TYPE_JAPANESE, NULL);
    nanoemMutableModelRigidBodySetOrientation(rigid_body, NULL);
    nanoemMutableModelRigidBodySetOrigin(rigid_body, NULL);
    nanoemMutableModelRigidBodySetShapeSize(rigid_body, NULL);
    nanoemMutableModelRigidBodySaveToBuffer(rigid_body, NULL, &status);
}

TEST_CASE("mutable_rigid_body_manipulate", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_t *model = scope.newModel();
    nanoem_mutable_model_rigid_body_t *first_rigid_body = scope.newRigidBody();
    nanoem_mutable_model_rigid_body_t *second_rigid_body = scope.newRigidBody();
    nanoem_mutable_model_rigid_body_t *third_rigid_body = scope.newRigidBody();
    nanoem_model_rigid_body_t *const *rigid_bodys;
    nanoem_rsize_t num_rigid_bodys;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    /* add */
    nanoemMutableModelInsertRigidBodyObject(model, first_rigid_body, -1, &status);
    nanoemMutableModelInsertRigidBodyObject(model, second_rigid_body, 1, &status);
    nanoemMutableModelInsertRigidBodyObject(model, third_rigid_body, 0, &status);
    SECTION("duplication should reject")
    {
        nanoemMutableModelInsertRigidBodyObject(model, first_rigid_body, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        rigid_bodys = nanoemModelGetAllRigidBodyObjects(nanoemMutableModelGetOriginObject(model), &num_rigid_bodys);
        CHECK(num_rigid_bodys == 3);
        CHECK(rigid_bodys[0] == nanoemMutableModelRigidBodyGetOriginObject(third_rigid_body));
        CHECK(rigid_bodys[1] == nanoemMutableModelRigidBodyGetOriginObject(first_rigid_body));
        CHECK(rigid_bodys[2] == nanoemMutableModelRigidBodyGetOriginObject(second_rigid_body));
    }
    SECTION("remove")
    {
        nanoemMutableModelRemoveRigidBodyObject(model, first_rigid_body, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        rigid_bodys = nanoemModelGetAllRigidBodyObjects(nanoemMutableModelGetOriginObject(model), &num_rigid_bodys);
        CHECK(num_rigid_bodys == 2);
        CHECK(rigid_bodys[0] == nanoemMutableModelRigidBodyGetOriginObject(third_rigid_body));
        CHECK(rigid_bodys[1] == nanoemMutableModelRigidBodyGetOriginObject(second_rigid_body));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableModelRemoveRigidBodyObject(model, first_rigid_body, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableModelRemoveRigidBodyObject(model, first_rigid_body, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_NOT_FOUND);
    }
}

TEST_CASE("mutable_rigid_body_copy_name", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_rigid_body_t *first_rigid_body = scope.appendedRigidBody();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    struct input_t {
        const char *input;
        const char *expected;
    };
    input_t input[] = { { "0123456789012345678", "0123456789012345678" /* 19bytes */ },
        { "01234567890123456789", "01234567890123456789" /* 20bytes */ },
        { "012345678901234567890", "01234567890123456789" /* 21bytes -> 20bytes */ } };
    for (size_t i = 0; i < sizeof(input) / sizeof(input[0]); i++) {
        const char *name = input[i].input;
        nanoem_unicode_string_t *s = scope.newString(name);
        nanoemMutableModelRigidBodySetName(first_rigid_body, s, NANOEM_LANGUAGE_TYPE_FIRST_ENUM, &status);
        CHECK_THAT(scope.describe(nanoemModelRigidBodyGetName(
                       scope.copyRigidBody(NANOEM_MODEL_FORMAT_TYPE_PMD_1_0), NANOEM_LANGUAGE_TYPE_FIRST_ENUM)),
            Catch::Equals(input[i].expected));
    }
}

TEST_CASE("mutable_rigid_body_copy_origin", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_rigid_body_t *first_rigid_body = scope.appendedRigidBody();
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelRigidBodySetOrigin(first_rigid_body, value);
    CHECK_THAT(nanoemModelRigidBodyGetOrigin(scope.copyRigidBody()), Equals(value));
}

TEST_CASE("mutable_rigid_body_copy_orientation", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_rigid_body_t *first_rigid_body = scope.appendedRigidBody();
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelRigidBodySetOrientation(first_rigid_body, value);
    CHECK_THAT(nanoemModelRigidBodyGetOrientation(scope.copyRigidBody()), Equals(value));
}

TEST_CASE("mutable_rigid_body_copy_shape_size", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_rigid_body_t *first_rigid_body = scope.appendedRigidBody();
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelRigidBodySetShapeSize(first_rigid_body, value);
    CHECK_THAT(nanoemModelRigidBodyGetShapeSize(scope.copyRigidBody()), Equals(value));
}

TEST_CASE("mutable_rigid_body_copy_mass", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_rigid_body_t *first_rigid_body = scope.appendedRigidBody();
    nanoemMutableModelRigidBodySetMass(first_rigid_body, 42.0f);
    CHECK(nanoemModelRigidBodyGetMass(scope.copyRigidBody()) == Approx(42.0f));
}

TEST_CASE("mutable_rigid_body_copy_angular_damping", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_rigid_body_t *first_rigid_body = scope.appendedRigidBody();
    nanoemMutableModelRigidBodySetAngularDamping(first_rigid_body, 42.0f);
    CHECK(nanoemModelRigidBodyGetAngularDamping(scope.copyRigidBody()) == Approx(42.0f));
}

TEST_CASE("mutable_rigid_body_copy_friction", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_rigid_body_t *first_rigid_body = scope.appendedRigidBody();
    nanoemMutableModelRigidBodySetFriction(first_rigid_body, 42.0f);
    CHECK(nanoemModelRigidBodyGetFriction(scope.copyRigidBody()) == Approx(42.0f));
}

TEST_CASE("mutable_rigid_body_copy_restitution", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_rigid_body_t *first_rigid_body = scope.appendedRigidBody();
    nanoemMutableModelRigidBodySetRestitution(first_rigid_body, 42.0f);
    CHECK(nanoemModelRigidBodyGetRestitution(scope.copyRigidBody()) == Approx(42.0f));
}

TEST_CASE("mutable_rigid_body_copy_collision_id", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_rigid_body_t *first_rigid_body = scope.appendedRigidBody();
    nanoemMutableModelRigidBodySetCollisionGroupId(first_rigid_body, 8);
    CHECK(nanoemModelRigidBodyGetCollisionGroupId(scope.copyRigidBody()) == 8);
}

TEST_CASE("mutable_rigid_body_copy_collision_mask", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_rigid_body_t *first_rigid_body = scope.appendedRigidBody();
    nanoemMutableModelRigidBodySetCollisionMask(first_rigid_body, 0x7fff);
    CHECK(nanoemModelRigidBodyGetCollisionMask(scope.copyRigidBody()) == 0x7fff);
}

TEST_CASE("mutable_rigid_body_copy_shape_type", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_rigid_body_t *first_rigid_body = scope.appendedRigidBody();
    for (nanoem_rsize_t i = NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_FIRST_ENUM;
         i < NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_MAX_ENUM; i++) {
        nanoem_model_rigid_body_shape_type_t type = (nanoem_model_rigid_body_shape_type_t) i;
        nanoemMutableModelRigidBodySetShapeType(first_rigid_body, type);
        CHECK(nanoemModelRigidBodyGetShapeType(scope.copyRigidBody()) == type);
    }
}

TEST_CASE("mutable_rigid_body_copy_transform_type", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_rigid_body_t *first_rigid_body = scope.appendedRigidBody();
    for (nanoem_rsize_t i = NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FIRST_ENUM;
         i < NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_MAX_ENUM; i++) {
        nanoem_model_rigid_body_transform_type_t type = (nanoem_model_rigid_body_transform_type_t) i;
        nanoemMutableModelRigidBodySetTransformType(first_rigid_body, type);
        CHECK(nanoemModelRigidBodyGetTransformType(scope.copyRigidBody()) == type);
    }
}
