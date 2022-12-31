/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_constraint_basic", "[nanoem]")
{
    nanoem_rsize_t num_joints;
    CHECK(nanoemModelConstraintGetAngleLimit(NULL) == Approx(0));
    CHECK(nanoemModelConstraintGetNumIterations(NULL) == 0);
    CHECK_FALSE(nanoemModelConstraintGetTargetBoneObject(NULL));
    CHECK_FALSE(nanoemModelConstraintGetAllJointObjects(NULL, &num_joints));
    CHECK(num_joints == 0);
    CHECK_FALSE(nanoemModelConstraintJointGetBoneObject(NULL));
    CHECK(nanoemModelConstraintJointGetLowerLimit(NULL));
    CHECK(nanoemModelConstraintJointGetUpperLimit(NULL));
    CHECK_FALSE(nanoemModelConstraintJointHasAngleLimit(NULL));
    CHECK(nanoemModelObjectGetIndex(nanoemModelConstraintGetModelObject(NULL)) == -1);
    CHECK_FALSE(nanoemModelConstraintGetModelObject(NULL));
    CHECK_FALSE(nanoemModelConstraintGetModelObjectMutable(NULL));
    nanoemModelConstraintDestroy(NULL);
}

TEST_CASE("mutable_constraint_null", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    CHECK_FALSE(nanoemMutableModelConstraintGetOriginObject(NULL));
    nanoemMutableModelConstraintSetAngleLimit(NULL, 0);
    nanoemMutableModelConstraintSetEffectorBoneObject(NULL, NULL);
    nanoemMutableModelConstraintSetNumIterations(NULL, 0);
    nanoemMutableModelConstraintSetTargetBoneObject(NULL, NULL);
    nanoemMutableModelConstraintSaveToBuffer(NULL, NULL, &status);
    nanoemMutableModelConstraintDestroy(NULL);
}

TEST_CASE("mutable_constraint_null_values", "[nanoem]")
{
    ModelScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_constraint_t *constraint = scope.newConstraint();
    nanoemMutableModelConstraintSetEffectorBoneObject(constraint, NULL);
    nanoemMutableModelConstraintSetTargetBoneObject(constraint, NULL);
    nanoemMutableModelConstraintSaveToBuffer(constraint, NULL, &status);
}

TEST_CASE("mutable_constraint_manipulate", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_t *model = scope.newModel();
    nanoem_mutable_model_constraint_t *first_constraint = scope.newConstraint();
    nanoem_mutable_model_constraint_t *second_constraint = scope.newConstraint();
    nanoem_mutable_model_constraint_t *third_constraint = scope.newConstraint();
    nanoem_model_constraint_t *const *constraints;
    nanoem_rsize_t num_constraints;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelInsertConstraintObject(model, first_constraint, -1, &status);
    nanoemMutableModelInsertConstraintObject(model, second_constraint, 1, &status);
    nanoemMutableModelInsertConstraintObject(model, third_constraint, 0, &status);
    SECTION("duplication should reject")
    {
        nanoemMutableModelInsertConstraintObject(model, first_constraint, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        constraints = nanoemModelGetAllConstraintObjects(nanoemMutableModelGetOriginObject(model), &num_constraints);
        CHECK(num_constraints == 3);
        CHECK(constraints[0] == nanoemMutableModelConstraintGetOriginObject(third_constraint));
        CHECK(constraints[1] == nanoemMutableModelConstraintGetOriginObject(first_constraint));
        CHECK(constraints[2] == nanoemMutableModelConstraintGetOriginObject(second_constraint));
    }
    SECTION("remove")
    {
        nanoemMutableModelRemoveConstraintObject(model, first_constraint, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        constraints = nanoemModelGetAllConstraintObjects(nanoemMutableModelGetOriginObject(model), &num_constraints);
        CHECK(num_constraints == 2);
        CHECK(constraints[0] == nanoemMutableModelConstraintGetOriginObject(third_constraint));
        CHECK(constraints[1] == nanoemMutableModelConstraintGetOriginObject(second_constraint));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableModelRemoveConstraintObject(model, first_constraint, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableModelRemoveConstraintObject(model, first_constraint, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_NOT_FOUND);
    }
}

TEST_CASE("mutable_constraint_copy_angle_limit", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_constraint_t *first_constraint = scope.appendedConstraint();
    nanoemMutableModelConstraintSetAngleLimit(first_constraint, 0.42f);
    CHECK(nanoemModelConstraintGetAngleLimit(scope.copyConstraint()) == Approx(0.42f));
}

TEST_CASE("mutable_constraint_copy_num_iteration", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_constraint_t *first_constraint = scope.appendedConstraint();
    nanoemMutableModelConstraintSetNumIterations(first_constraint, 42);
    CHECK(nanoemModelConstraintGetNumIterations(scope.copyConstraint()) == 42);
}

TEST_CASE("mutable_constraint_copy_target_bone", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_constraint_t *first_constraint = scope.appendedConstraint();
    nanoem_mutable_model_bone_t *first_bone = scope.appendedBone("target0");
    nanoemMutableModelConstraintSetTargetBoneObject(
        first_constraint, nanoemMutableModelBoneGetOriginObject(first_bone));
    CHECK_THAT(nanoemModelConstraintGetTargetBoneObject(scope.copyConstraint()), EqualsBoneName("target0"));
    nanoemMutableModelConstraintSetTargetBoneObject(first_constraint, nullptr);
    CHECK(nanoemModelConstraintGetTargetBoneObject(scope.copyConstraint()) == nullptr);
}

TEST_CASE("mutable_constraint_copy_effector_bone", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_constraint_t *first_constraint = scope.appendedConstraint();
    nanoem_mutable_model_bone_t *first_bone = scope.appendedBone("effector0");
    nanoemMutableModelConstraintSetEffectorBoneObject(
        first_constraint, nanoemMutableModelBoneGetOriginObject(first_bone));
    CHECK_THAT(nanoemModelConstraintGetEffectorBoneObject(scope.copyConstraint()), EqualsBoneName("effector0"));
    nanoemMutableModelConstraintSetEffectorBoneObject(first_constraint, nullptr);
    CHECK(nanoemModelConstraintGetEffectorBoneObject(scope.copyConstraint()) == nullptr);
}
