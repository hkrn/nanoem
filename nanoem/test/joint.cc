/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_joint_basic", "[nanoem]")
{
    CHECK_FALSE(nanoemModelJointGetName(NULL, NANOEM_LANGUAGE_TYPE_JAPANESE));
    CHECK_FALSE(nanoemModelJointGetName(NULL, NANOEM_LANGUAGE_TYPE_ENGLISH));
    CHECK(nanoemModelJointGetOrigin(NULL));
    CHECK(nanoemModelJointGetLinearLowerLimit(NULL));
    CHECK(nanoemModelJointGetLinearStiffness(NULL));
    CHECK(nanoemModelJointGetLinearUpperLimit(NULL));
    CHECK_FALSE(nanoemModelJointGetRigidBodyAObject(NULL));
    CHECK_FALSE(nanoemModelJointGetRigidBodyBObject(NULL));
    CHECK(nanoemModelJointGetOrientation(NULL));
    CHECK(nanoemModelJointGetType(NULL) == NANOEM_MODEL_JOINT_TYPE_UNKNOWN);
    CHECK(nanoemModelJointGetAngularLowerLimit(NULL));
    CHECK(nanoemModelJointGetAngularStiffness(NULL));
    CHECK(nanoemModelJointGetAngularUpperLimit(NULL));
    CHECK(nanoemModelObjectGetIndex(nanoemModelJointGetModelObject(NULL)) == -1);
    CHECK_FALSE(nanoemModelJointGetModelObject(NULL));
    CHECK_FALSE(nanoemModelJointGetModelObjectMutable(NULL));
    nanoemModelJointDestroy(NULL);
}

TEST_CASE("mutable_joint_null", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    CHECK_FALSE(nanoemMutableModelJointGetOriginObject(NULL));
    nanoemMutableModelJointSetAngularLowerLimit(NULL, NULL);
    nanoemMutableModelJointSetAngularStiffness(NULL, NULL);
    nanoemMutableModelJointSetAngularUpperLimit(NULL, NULL);
    nanoemMutableModelJointSetLinearLowerLimit(NULL, NULL);
    nanoemMutableModelJointSetLinearStiffness(NULL, NULL);
    nanoemMutableModelJointSetLinearUpperLimit(NULL, NULL);
    nanoemMutableModelJointSetName(NULL, NULL, NANOEM_LANGUAGE_TYPE_JAPANESE, NULL);
    nanoemMutableModelJointSetOrientation(NULL, NULL);
    nanoemMutableModelJointSetOrigin(NULL, NULL);
    nanoemMutableModelJointSetRigidBodyAObject(NULL, NULL);
    nanoemMutableModelJointSetRigidBodyBObject(NULL, NULL);
    nanoemMutableModelJointSetType(NULL, NANOEM_MODEL_JOINT_TYPE_UNKNOWN);
    nanoemMutableModelJointSaveToBuffer(NULL, NULL, &status);
    nanoemMutableModelJointDestroy(NULL);
}

TEST_CASE("mutable_joint_null_values", "[nanoem]")
{
    ModelScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_joint_t *joint = scope.newJoint();
    nanoemMutableModelJointSetAngularLowerLimit(joint, NULL);
    nanoemMutableModelJointSetAngularStiffness(joint, NULL);
    nanoemMutableModelJointSetAngularUpperLimit(joint, NULL);
    nanoemMutableModelJointSetLinearLowerLimit(joint, NULL);
    nanoemMutableModelJointSetLinearStiffness(joint, NULL);
    nanoemMutableModelJointSetLinearUpperLimit(joint, NULL);
    nanoemMutableModelJointSetName(joint, NULL, NANOEM_LANGUAGE_TYPE_JAPANESE, NULL);
    nanoemMutableModelJointSetOrientation(joint, NULL);
    nanoemMutableModelJointSetOrigin(joint, NULL);
    nanoemMutableModelJointSetRigidBodyAObject(joint, NULL);
    nanoemMutableModelJointSetRigidBodyBObject(joint, NULL);
    nanoemMutableModelJointSaveToBuffer(joint, NULL, &status);
}

TEST_CASE("mutable_joint_manipulate", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_t *model = scope.newModel();
    nanoem_mutable_model_joint_t *first_joint = scope.newJoint();
    nanoem_mutable_model_joint_t *second_joint = scope.newJoint();
    nanoem_mutable_model_joint_t *third_joint = scope.newJoint();
    nanoem_model_joint_t *const *joints;
    nanoem_rsize_t num_joints;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelInsertJointObject(model, first_joint, -1, &status);
    nanoemMutableModelInsertJointObject(model, second_joint, 1, &status);
    nanoemMutableModelInsertJointObject(model, third_joint, 0, &status);
    SECTION("duplication should reject")
    {
        nanoemMutableModelInsertJointObject(model, first_joint, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_JOINT_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        joints = nanoemModelGetAllJointObjects(nanoemMutableModelGetOriginObject(model), &num_joints);
        CHECK(num_joints == 3);
        CHECK(joints[0] == nanoemMutableModelJointGetOriginObject(third_joint));
        CHECK(joints[1] == nanoemMutableModelJointGetOriginObject(first_joint));
        CHECK(joints[2] == nanoemMutableModelJointGetOriginObject(second_joint));
    }
    SECTION("remove")
    {
        nanoemMutableModelRemoveJointObject(model, first_joint, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        joints = nanoemModelGetAllJointObjects(nanoemMutableModelGetOriginObject(model), &num_joints);
        CHECK(num_joints == 2);
        CHECK(joints[0] == nanoemMutableModelJointGetOriginObject(third_joint));
        CHECK(joints[1] == nanoemMutableModelJointGetOriginObject(second_joint));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableModelRemoveJointObject(model, first_joint, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableModelRemoveJointObject(model, first_joint, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_JOINT_NOT_FOUND);
    }
}

TEST_CASE("mutable_joint_copy_name", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_joint_t *first_joint = scope.appendedJoint();
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
        nanoemMutableModelJointSetName(first_joint, s, NANOEM_LANGUAGE_TYPE_FIRST_ENUM, &status);
        CHECK_THAT(scope.describe(nanoemModelJointGetName(
                       scope.copyJoint(NANOEM_MODEL_FORMAT_TYPE_PMD_1_0), NANOEM_LANGUAGE_TYPE_FIRST_ENUM)),
            Catch::Equals(input[i].expected));
    }
}

TEST_CASE("mutable_joint_copy_origin", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_joint_t *first_joint = scope.appendedJoint();
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelJointSetOrigin(first_joint, value);
    CHECK_THAT(nanoemModelJointGetOrigin(scope.copyJoint()), Equals(value));
}

TEST_CASE("mutable_joint_copy_orientation", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_joint_t *first_joint = scope.appendedJoint();
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelJointSetOrientation(first_joint, value);
    CHECK_THAT(nanoemModelJointGetOrientation(scope.copyJoint()), Equals(value));
}

TEST_CASE("mutable_joint_copy_type", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_joint_t *first_joint = scope.appendedJoint();
    for (nanoem_rsize_t i = NANOEM_MODEL_JOINT_TYPE_FIRST_ENUM; i < NANOEM_MODEL_JOINT_TYPE_MAX_ENUM; i++) {
        nanoem_model_joint_type_t type = (nanoem_model_joint_type_t) i;
        nanoemMutableModelJointSetType(first_joint, type);
        CHECK(nanoemModelJointGetType(scope.copyJoint()) == type);
    }
}

TEST_CASE("mutable_joint_copy_rigid_body_a", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_joint_t *first_joint = scope.appendedJoint();
    nanoem_mutable_model_rigid_body_t *first_rigid_body = scope.appendedRigidBody("body0");
    nanoemMutableModelJointSetRigidBodyAObject(
        first_joint, nanoemMutableModelRigidBodyGetOriginObject(first_rigid_body));
    CHECK_THAT(nanoemModelJointGetRigidBodyAObject(scope.copyJoint()), EqualsRigidBodyName("body0"));
    nanoemMutableModelJointSetRigidBodyAObject(first_joint, nullptr);
    CHECK(nanoemModelJointGetRigidBodyAObject(scope.copyJoint()) == nullptr);
}

TEST_CASE("mutable_joint_copy_rigid_body_b", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_joint_t *first_joint = scope.appendedJoint();
    nanoem_mutable_model_rigid_body_t *first_rigid_body = scope.appendedRigidBody("body0");
    nanoemMutableModelJointSetRigidBodyBObject(
        first_joint, nanoemMutableModelRigidBodyGetOriginObject(first_rigid_body));
    CHECK_THAT(nanoemModelJointGetRigidBodyBObject(scope.copyJoint()), EqualsRigidBodyName("body0"));
    nanoemMutableModelJointSetRigidBodyBObject(first_joint, nullptr);
    CHECK(nanoemModelJointGetRigidBodyBObject(scope.copyJoint()) == nullptr);
}

TEST_CASE("mutable_joint_copy_angular_upper_limit", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_joint_t *first_joint = scope.appendedJoint();
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelJointSetAngularUpperLimit(first_joint, value);
    CHECK_THAT(nanoemModelJointGetAngularUpperLimit(scope.copyJoint()), Equals(value));
}

TEST_CASE("mutable_joint_copy_angular_lower_limit", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_joint_t *first_joint = scope.appendedJoint();
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelJointSetAngularLowerLimit(first_joint, value);
    CHECK_THAT(nanoemModelJointGetAngularLowerLimit(scope.copyJoint()), Equals(value));
}

TEST_CASE("mutable_joint_copy_angular_stiffness", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_joint_t *first_joint = scope.appendedJoint();
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelJointSetAngularStiffness(first_joint, value);
    CHECK_THAT(nanoemModelJointGetAngularStiffness(scope.copyJoint()), Equals(value));
}

TEST_CASE("mutable_joint_copy_linear_upper_limit", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_joint_t *first_joint = scope.appendedJoint();
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelJointSetLinearUpperLimit(first_joint, value);
    CHECK_THAT(nanoemModelJointGetLinearUpperLimit(scope.copyJoint()), Equals(value));
}

TEST_CASE("mutable_joint_copy_linear_lower_limit", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_joint_t *first_joint = scope.appendedJoint();
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelJointSetLinearLowerLimit(first_joint, value);
    CHECK_THAT(nanoemModelJointGetLinearLowerLimit(scope.copyJoint()), Equals(value));
}

TEST_CASE("mutable_joint_copy_linear_stiffness", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_joint_t *first_joint = scope.appendedJoint();
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelJointSetLinearStiffness(first_joint, value);
    CHECK_THAT(nanoemModelJointGetLinearStiffness(scope.copyJoint()), Equals(value));
}
