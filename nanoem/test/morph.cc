/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_morph_basic", "[nanoem]")
{
    nanoem_rsize_t num_objects;
    CHECK(nanoemModelMorphGetCategory(NULL) == NANOEM_MODEL_MORPH_CATEGORY_UNKNOWN);
    CHECK_FALSE(nanoemModelMorphGetName(NULL, NANOEM_LANGUAGE_TYPE_JAPANESE));
    CHECK_FALSE(nanoemModelMorphGetName(NULL, NANOEM_LANGUAGE_TYPE_ENGLISH));
    CHECK(nanoemModelMorphGetType(NULL) == NANOEM_MODEL_MORPH_TYPE_UNKNOWN);
    CHECK_FALSE(nanoemModelMorphGetAllBoneMorphObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemModelMorphGetAllFlipMorphObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemModelMorphGetAllGroupMorphObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemModelMorphGetAllImpulseMorphObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemModelMorphGetAllMaterialMorphObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemModelMorphGetAllUVMorphObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemModelMorphGetAllVertexMorphObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemModelMorphBoneGetBoneObject(NULL));
    CHECK(nanoemModelMorphBoneGetOrientation(NULL));
    CHECK(nanoemModelMorphBoneGetTranslation(NULL));
    CHECK_FALSE(nanoemModelMorphFlipGetMorphObject(NULL));
    CHECK(nanoemModelMorphFlipGetWeight(NULL) == Approx(0));
    CHECK_FALSE(nanoemModelMorphGroupGetMorphObject(NULL));
    CHECK(nanoemModelMorphGroupGetWeight(NULL) == Approx(0));
    CHECK_FALSE(nanoemModelMorphImpulseGetRigidBodyObject(NULL));
    CHECK(nanoemModelMorphImpulseGetTorque(NULL));
    CHECK(nanoemModelMorphImpulseGetVelocity(NULL));
    CHECK_FALSE(nanoemModelMorphImpulseIsLocal(NULL));
    CHECK(nanoemModelMorphMaterialGetAmbientColor(NULL));
    CHECK(nanoemModelMorphMaterialGetDiffuseColor(NULL));
    CHECK(nanoemModelMorphMaterialGetDiffuseOpacity(NULL) == Approx(0));
    CHECK(nanoemModelMorphMaterialGetDiffuseTextureBlend(NULL));
    CHECK(nanoemModelMorphMaterialGetEdgeColor(NULL));
    CHECK(nanoemModelMorphMaterialGetEdgeOpacity(NULL) == Approx(0));
    CHECK(nanoemModelMorphMaterialGetEdgeSize(NULL) == Approx(0));
    CHECK_FALSE(nanoemModelMorphMaterialGetMaterialObject(NULL));
    CHECK(nanoemModelMorphMaterialGetOperationType(NULL) == NANOEM_MODEL_MORPH_MATERIAL_OPERATION_TYPE_UNKNOWN);
    CHECK(nanoemModelMorphMaterialGetSpecularPower(NULL) == Approx(0));
    CHECK(nanoemModelMorphMaterialGetSpecularColor(NULL));
    CHECK(nanoemModelMorphMaterialGetSphereMapTextureBlend(NULL));
    CHECK(nanoemModelMorphMaterialGetToonTextureBlend(NULL));
    CHECK(nanoemModelMorphUVGetPosition(NULL));
    CHECK_FALSE(nanoemModelMorphUVGetVertexObject(NULL));
    CHECK(nanoemModelMorphVertexGetPosition(NULL));
    CHECK_FALSE(nanoemModelMorphVertexGetVertexObject(NULL));
    CHECK(nanoemModelObjectGetIndex(nanoemModelMorphGetModelObject(NULL)) == -1);
    CHECK_FALSE(nanoemModelMorphGetModelObject(NULL));
    CHECK_FALSE(nanoemModelMorphGetModelObjectMutable(NULL));
    nanoemModelMorphDestroy(NULL);
}

TEST_CASE("mutable_morph_null", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    CHECK_FALSE(nanoemMutableModelMorphBoneGetOriginObject(NULL));
    CHECK_FALSE(nanoemMutableModelMorphFlipGetOriginObject(NULL));
    CHECK_FALSE(nanoemMutableModelMorphGroupGetOriginObject(NULL));
    CHECK_FALSE(nanoemMutableModelMorphImpulseGetOriginObject(NULL));
    CHECK_FALSE(nanoemMutableModelMorphMaterialGetOriginObject(NULL));
    CHECK_FALSE(nanoemMutableModelMorphUVGetOriginObject(NULL));
    CHECK_FALSE(nanoemMutableModelMorphVertexGetOriginObject(NULL));
    CHECK_FALSE(nanoemMutableModelMorphGetOriginObject(NULL));
    nanoemMutableModelMorphSetCategory(NULL, NANOEM_MODEL_MORPH_CATEGORY_BASE);
    nanoemMutableModelMorphSetName(NULL, NULL, NANOEM_LANGUAGE_TYPE_JAPANESE, NULL);
    nanoemMutableModelMorphSetType(NULL, NANOEM_MODEL_MORPH_TYPE_BONE);
    nanoemMutableModelMorphDestroy(NULL);
    nanoemMutableModelMorphInsertBoneMorphObject(NULL, NULL, -1, &status);
    nanoemMutableModelMorphInsertFlipMorphObject(NULL, NULL, -1, &status);
    nanoemMutableModelMorphInsertGroupMorphObject(NULL, NULL, -1, &status);
    nanoemMutableModelMorphInsertImpulseMorphObject(NULL, NULL, -1, &status);
    nanoemMutableModelMorphInsertMaterialMorphObject(NULL, NULL, -1, &status);
    nanoemMutableModelMorphInsertUVMorphObject(NULL, NULL, -1, &status);
    nanoemMutableModelMorphInsertVertexMorphObject(NULL, NULL, -1, &status);
    nanoemMutableModelMorphRemoveBoneMorphObject(NULL, NULL, &status);
    nanoemMutableModelMorphRemoveFlipMorphObject(NULL, NULL, &status);
    nanoemMutableModelMorphRemoveGroupMorphObject(NULL, NULL, &status);
    nanoemMutableModelMorphRemoveImpulseMorphObject(NULL, NULL, &status);
    nanoemMutableModelMorphRemoveMaterialMorphObject(NULL, NULL, &status);
    nanoemMutableModelMorphRemoveUVMorphObject(NULL, NULL, &status);
    nanoemMutableModelMorphRemoveVertexMorphObject(NULL, NULL, &status);
    nanoemMutableModelMorphBoneSaveToBuffer(NULL, NULL, &status);
    nanoemMutableModelMorphFlipSaveToBuffer(NULL, NULL, &status);
    nanoemMutableModelMorphGroupSaveToBuffer(NULL, NULL, &status);
    nanoemMutableModelMorphImpulseSaveToBuffer(NULL, NULL, &status);
    nanoemMutableModelMorphMaterialSaveToBuffer(NULL, NULL, &status);
    nanoemMutableModelMorphUVSaveToBuffer(NULL, NULL, &status);
    nanoemMutableModelMorphVertexSaveToBuffer(NULL, NULL, &status);
}

TEST_CASE("mutable_morph_null_values", "[nanoem]")
{
    ModelScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_morph_t *morph = scope.newMorph();
    nanoemMutableModelMorphSetName(morph, NULL, NANOEM_LANGUAGE_TYPE_JAPANESE, NULL);
    nanoemMutableModelMorphInsertBoneMorphObject(morph, NULL, -1, &status);
    nanoemMutableModelMorphInsertFlipMorphObject(morph, NULL, -1, &status);
    nanoemMutableModelMorphInsertGroupMorphObject(morph, NULL, -1, &status);
    nanoemMutableModelMorphInsertImpulseMorphObject(morph, NULL, -1, &status);
    nanoemMutableModelMorphInsertMaterialMorphObject(morph, NULL, -1, &status);
    nanoemMutableModelMorphInsertUVMorphObject(morph, NULL, -1, &status);
    nanoemMutableModelMorphInsertVertexMorphObject(morph, NULL, -1, &status);
    nanoemMutableModelMorphRemoveBoneMorphObject(morph, NULL, &status);
    nanoemMutableModelMorphRemoveFlipMorphObject(morph, NULL, &status);
    nanoemMutableModelMorphRemoveGroupMorphObject(morph, NULL, &status);
    nanoemMutableModelMorphRemoveImpulseMorphObject(morph, NULL, &status);
    nanoemMutableModelMorphRemoveMaterialMorphObject(morph, NULL, &status);
    nanoemMutableModelMorphRemoveUVMorphObject(morph, NULL, &status);
    nanoemMutableModelMorphRemoveVertexMorphObject(morph, NULL, &status);
}

TEST_CASE("mutable_morph_manipulate", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_t *model = scope.newModel();
    nanoem_mutable_model_morph_t *first_morph = scope.newMorph();
    nanoem_mutable_model_morph_t *second_morph = scope.newMorph();
    nanoem_mutable_model_morph_t *third_morph = scope.newMorph();
    nanoem_model_morph_t *const *morphs;
    nanoem_rsize_t num_morphs;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    /* add */
    nanoemMutableModelInsertMorphObject(model, first_morph, -1, &status);
    nanoemMutableModelInsertMorphObject(model, second_morph, 1, &status);
    nanoemMutableModelInsertMorphObject(model, third_morph, 0, &status);
    SECTION("duplication should reject")
    {
        nanoemMutableModelInsertMorphObject(model, first_morph, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        morphs = nanoemModelGetAllMorphObjects(nanoemMutableModelGetOriginObject(model), &num_morphs);
        CHECK(num_morphs == 3);
        CHECK(morphs[0] == nanoemMutableModelMorphGetOriginObject(third_morph));
        CHECK(morphs[1] == nanoemMutableModelMorphGetOriginObject(first_morph));
        CHECK(morphs[2] == nanoemMutableModelMorphGetOriginObject(second_morph));
    }
    SECTION("remove")
    {
        nanoemMutableModelRemoveMorphObject(model, first_morph, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        morphs = nanoemModelGetAllMorphObjects(nanoemMutableModelGetOriginObject(model), &num_morphs);
        CHECK(num_morphs == 2);
        CHECK(morphs[0] == nanoemMutableModelMorphGetOriginObject(third_morph));
        CHECK(morphs[1] == nanoemMutableModelMorphGetOriginObject(second_morph));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableModelRemoveMorphObject(model, first_morph, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableModelRemoveMorphObject(model, first_morph, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_NOT_FOUND);
    }
}

TEST_CASE("mutable_morph_manipulate_bone", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_t *model = scope.newModel();
    nanoem_mutable_model_morph_t *parent_morph = scope.newMorph();
    nanoem_mutable_model_morph_bone_t *first_morph = scope.newBoneMorph();
    nanoem_mutable_model_morph_bone_t *second_morph = scope.newBoneMorph();
    nanoem_mutable_model_morph_bone_t *third_morph = scope.newBoneMorph();
    nanoem_model_morph_bone_t *const *bone_morphs;
    nanoem_rsize_t num_bone_morphs;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    SECTION("type mismatch should reject")
    {
        nanoemMutableModelMorphInsertBoneMorphObject(parent_morph, first_morph, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_TYPE_MISMATCH);
    }
    nanoemMutableModelMorphSetType(parent_morph, NANOEM_MODEL_MORPH_TYPE_BONE);
    SECTION("PMD should reject")
    {
        nanoemMutableModelMorphInsertBoneMorphObject(parent_morph, first_morph, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_VERSION_INCOMPATIBLE);
    }
    nanoemMutableModelSetFormatType(model, NANOEM_MODEL_FORMAT_TYPE_PMX_2_0);
    /* add */
    nanoemMutableModelMorphInsertBoneMorphObject(parent_morph, first_morph, -1, &status);
    nanoemMutableModelMorphInsertBoneMorphObject(parent_morph, second_morph, 1, &status);
    nanoemMutableModelMorphInsertBoneMorphObject(parent_morph, third_morph, 0, &status);
    SECTION("duplication should reject")
    {
        nanoemMutableModelMorphInsertBoneMorphObject(parent_morph, third_morph, 0, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        bone_morphs = nanoemModelMorphGetAllBoneMorphObjects(
            nanoemMutableModelMorphGetOriginObject(parent_morph), &num_bone_morphs);
        CHECK(num_bone_morphs == 3);
        CHECK(bone_morphs[0] == nanoemMutableModelMorphBoneGetOriginObject(third_morph));
        CHECK(bone_morphs[1] == nanoemMutableModelMorphBoneGetOriginObject(first_morph));
        CHECK(bone_morphs[2] == nanoemMutableModelMorphBoneGetOriginObject(second_morph));
    }
    SECTION("remove")
    {
        nanoemMutableModelMorphRemoveBoneMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        bone_morphs = nanoemModelMorphGetAllBoneMorphObjects(
            nanoemMutableModelMorphGetOriginObject(parent_morph), &num_bone_morphs);
        CHECK(num_bone_morphs == 2);
        CHECK(bone_morphs[0] == nanoemMutableModelMorphBoneGetOriginObject(third_morph));
        CHECK(bone_morphs[1] == nanoemMutableModelMorphBoneGetOriginObject(second_morph));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableModelMorphRemoveBoneMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableModelMorphRemoveBoneMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_BONE_NOT_FOUND);
    }
}

TEST_CASE("mutable_morph_manipulate_flip", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_t *model = scope.newModel();
    nanoem_mutable_model_morph_t *parent_morph = scope.newMorph();
    nanoem_mutable_model_morph_flip_t *first_morph = scope.newFlipMorph();
    nanoem_mutable_model_morph_flip_t *second_morph = scope.newFlipMorph();
    nanoem_mutable_model_morph_flip_t *third_morph = scope.newFlipMorph();
    nanoem_model_morph_flip_t *const *flip_morphs;
    nanoem_rsize_t num_flip_morphs;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    SECTION("type mismatch should reject")
    {
        nanoemMutableModelMorphInsertFlipMorphObject(parent_morph, first_morph, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_TYPE_MISMATCH);
    }
    nanoemMutableModelMorphSetType(parent_morph, NANOEM_MODEL_MORPH_TYPE_FLIP);
    SECTION("PMD/PMX 2.0 should reject")
    {
        nanoemMutableModelMorphInsertFlipMorphObject(parent_morph, first_morph, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_VERSION_INCOMPATIBLE);
    }
    nanoemMutableModelSetFormatType(model, NANOEM_MODEL_FORMAT_TYPE_PMX_2_1);
    /* add */
    nanoemMutableModelMorphInsertFlipMorphObject(parent_morph, first_morph, -1, &status);
    nanoemMutableModelMorphInsertFlipMorphObject(parent_morph, second_morph, 1, &status);
    nanoemMutableModelMorphInsertFlipMorphObject(parent_morph, third_morph, 0, &status);
    SECTION("duplication should reject")
    {
        nanoemMutableModelMorphInsertFlipMorphObject(parent_morph, third_morph, 0, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        flip_morphs = nanoemModelMorphGetAllFlipMorphObjects(
            nanoemMutableModelMorphGetOriginObject(parent_morph), &num_flip_morphs);
        CHECK(num_flip_morphs == 3);
        CHECK(flip_morphs[0] == nanoemMutableModelMorphFlipGetOriginObject(third_morph));
        CHECK(flip_morphs[1] == nanoemMutableModelMorphFlipGetOriginObject(first_morph));
        CHECK(flip_morphs[2] == nanoemMutableModelMorphFlipGetOriginObject(second_morph));
    }
    SECTION("remove")
    {
        nanoemMutableModelMorphRemoveFlipMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        flip_morphs = nanoemModelMorphGetAllFlipMorphObjects(
            nanoemMutableModelMorphGetOriginObject(parent_morph), &num_flip_morphs);
        CHECK(num_flip_morphs == 2);
        CHECK(flip_morphs[0] == nanoemMutableModelMorphFlipGetOriginObject(third_morph));
        CHECK(flip_morphs[1] == nanoemMutableModelMorphFlipGetOriginObject(second_morph));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableModelMorphRemoveFlipMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableModelMorphRemoveFlipMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_FLIP_NOT_FOUND);
    }
}

TEST_CASE("mutable_morph_manipulate_group", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_t *model = scope.newModel();
    nanoem_mutable_model_morph_t *parent_morph = scope.newMorph();
    nanoem_mutable_model_morph_group_t *first_morph = scope.newGroupMorph();
    nanoem_mutable_model_morph_group_t *second_morph = scope.newGroupMorph();
    nanoem_mutable_model_morph_group_t *third_morph = scope.newGroupMorph();
    nanoem_model_morph_group_t *const *group_morphs;
    nanoem_rsize_t num_group_morphs;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    /* add */
    SECTION("type mismatch should reject")
    {
        nanoemMutableModelMorphInsertGroupMorphObject(parent_morph, first_morph, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_TYPE_MISMATCH);
    }
    nanoemMutableModelMorphSetType(parent_morph, NANOEM_MODEL_MORPH_TYPE_GROUP);
    SECTION("PMD should reject")
    {
        nanoemMutableModelMorphInsertGroupMorphObject(parent_morph, first_morph, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_VERSION_INCOMPATIBLE);
    }
    nanoemMutableModelSetFormatType(model, NANOEM_MODEL_FORMAT_TYPE_PMX_2_0);
    nanoemMutableModelMorphInsertGroupMorphObject(parent_morph, first_morph, -1, &status);
    nanoemMutableModelMorphInsertGroupMorphObject(parent_morph, second_morph, 1, &status);
    nanoemMutableModelMorphInsertGroupMorphObject(parent_morph, third_morph, 0, &status);
    SECTION("duplication should reject")
    {
        nanoemMutableModelMorphInsertGroupMorphObject(parent_morph, third_morph, 0, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        group_morphs = nanoemModelMorphGetAllGroupMorphObjects(
            nanoemMutableModelMorphGetOriginObject(parent_morph), &num_group_morphs);
        CHECK(num_group_morphs == 3);
        CHECK(group_morphs[0] == nanoemMutableModelMorphGroupGetOriginObject(third_morph));
        CHECK(group_morphs[1] == nanoemMutableModelMorphGroupGetOriginObject(first_morph));
        CHECK(group_morphs[2] == nanoemMutableModelMorphGroupGetOriginObject(second_morph));
    }
    SECTION("remove")
    {
        nanoemMutableModelMorphRemoveGroupMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        group_morphs = nanoemModelMorphGetAllGroupMorphObjects(
            nanoemMutableModelMorphGetOriginObject(parent_morph), &num_group_morphs);
        CHECK(num_group_morphs == 2);
        CHECK(group_morphs[0] == nanoemMutableModelMorphGroupGetOriginObject(third_morph));
        CHECK(group_morphs[1] == nanoemMutableModelMorphGroupGetOriginObject(second_morph));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableModelMorphRemoveGroupMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableModelMorphRemoveGroupMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_GROUP_NOT_FOUND);
    }
}

TEST_CASE("mutable_morph_manipulate_impulse", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_t *model = scope.newModel();
    nanoem_mutable_model_morph_t *parent_morph = scope.newMorph();
    nanoem_mutable_model_morph_impulse_t *first_morph = scope.newImpulseMorph();
    nanoem_mutable_model_morph_impulse_t *second_morph = scope.newImpulseMorph();
    nanoem_mutable_model_morph_impulse_t *third_morph = scope.newImpulseMorph();
    nanoem_model_morph_impulse_t *const *impulse_morphs;
    nanoem_rsize_t num_impulse_morphs;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    SECTION("type mismatch should reject")
    {
        nanoemMutableModelMorphInsertImpulseMorphObject(parent_morph, first_morph, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_TYPE_MISMATCH);
    }
    nanoemMutableModelMorphSetType(parent_morph, NANOEM_MODEL_MORPH_TYPE_IMPULUSE);
    SECTION("PMD/PMX 2.0 should reject")
    {
        nanoemMutableModelMorphInsertImpulseMorphObject(parent_morph, first_morph, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_VERSION_INCOMPATIBLE);
    }
    nanoemMutableModelSetFormatType(model, NANOEM_MODEL_FORMAT_TYPE_PMX_2_1);
    /* add */
    nanoemMutableModelMorphInsertImpulseMorphObject(parent_morph, first_morph, -1, &status);
    nanoemMutableModelMorphInsertImpulseMorphObject(parent_morph, second_morph, 1, &status);
    nanoemMutableModelMorphInsertImpulseMorphObject(parent_morph, third_morph, 0, &status);
    SECTION("duplication should reject")
    {
        nanoemMutableModelMorphInsertImpulseMorphObject(parent_morph, third_morph, 0, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        impulse_morphs = nanoemModelMorphGetAllImpulseMorphObjects(
            nanoemMutableModelMorphGetOriginObject(parent_morph), &num_impulse_morphs);
        CHECK(num_impulse_morphs == 3);
        CHECK(impulse_morphs[0] == nanoemMutableModelMorphImpulseGetOriginObject(third_morph));
        CHECK(impulse_morphs[1] == nanoemMutableModelMorphImpulseGetOriginObject(first_morph));
        CHECK(impulse_morphs[2] == nanoemMutableModelMorphImpulseGetOriginObject(second_morph));
    }
    SECTION("remove")
    {
        nanoemMutableModelMorphRemoveImpulseMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        impulse_morphs = nanoemModelMorphGetAllImpulseMorphObjects(
            nanoemMutableModelMorphGetOriginObject(parent_morph), &num_impulse_morphs);
        CHECK(num_impulse_morphs == 2);
        CHECK(impulse_morphs[0] == nanoemMutableModelMorphImpulseGetOriginObject(third_morph));
        CHECK(impulse_morphs[1] == nanoemMutableModelMorphImpulseGetOriginObject(second_morph));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableModelMorphRemoveImpulseMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableModelMorphRemoveImpulseMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_IMPULSE_NOT_FOUND);
    }
}

TEST_CASE("mutable_morph_manipulate_material", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_t *model = scope.newModel();
    nanoem_mutable_model_morph_t *parent_morph = scope.newMorph();
    nanoem_mutable_model_morph_material_t *first_morph = scope.newMaterialMorph();
    nanoem_mutable_model_morph_material_t *second_morph = scope.newMaterialMorph();
    nanoem_mutable_model_morph_material_t *third_morph = scope.newMaterialMorph();
    nanoem_model_morph_material_t *const *material_morphs;
    nanoem_rsize_t num_material_morphs;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    SECTION("type mismatch should reject")
    {
        nanoemMutableModelMorphInsertMaterialMorphObject(parent_morph, first_morph, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_TYPE_MISMATCH);
    }
    nanoemMutableModelMorphSetType(parent_morph, NANOEM_MODEL_MORPH_TYPE_MATERIAL);
    SECTION("PMD should reject")
    {
        nanoemMutableModelMorphInsertMaterialMorphObject(parent_morph, first_morph, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_VERSION_INCOMPATIBLE);
    }
    nanoemMutableModelSetFormatType(model, NANOEM_MODEL_FORMAT_TYPE_PMX_2_0);
    /* add */
    nanoemMutableModelMorphInsertMaterialMorphObject(parent_morph, first_morph, -1, &status);
    nanoemMutableModelMorphInsertMaterialMorphObject(parent_morph, second_morph, 1, &status);
    nanoemMutableModelMorphInsertMaterialMorphObject(parent_morph, third_morph, 0, &status);
    SECTION("duplication should reject")
    {
        nanoemMutableModelMorphInsertMaterialMorphObject(parent_morph, third_morph, 0, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        material_morphs = nanoemModelMorphGetAllMaterialMorphObjects(
            nanoemMutableModelMorphGetOriginObject(parent_morph), &num_material_morphs);
        CHECK(num_material_morphs == 3);
        CHECK(material_morphs[0] == nanoemMutableModelMorphMaterialGetOriginObject(third_morph));
        CHECK(material_morphs[1] == nanoemMutableModelMorphMaterialGetOriginObject(first_morph));
        CHECK(material_morphs[2] == nanoemMutableModelMorphMaterialGetOriginObject(second_morph));
    }
    SECTION("remove")
    {
        nanoemMutableModelMorphRemoveMaterialMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        material_morphs = nanoemModelMorphGetAllMaterialMorphObjects(
            nanoemMutableModelMorphGetOriginObject(parent_morph), &num_material_morphs);
        CHECK(num_material_morphs == 2);
        CHECK(material_morphs[0] == nanoemMutableModelMorphMaterialGetOriginObject(third_morph));
        CHECK(material_morphs[1] == nanoemMutableModelMorphMaterialGetOriginObject(second_morph));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableModelMorphRemoveMaterialMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableModelMorphRemoveMaterialMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_MATERIAL_NOT_FOUND);
    }
}

TEST_CASE("mutable_morph_manipulate_uv", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_t *model = scope.newModel();
    nanoem_mutable_model_morph_t *parent_morph = scope.newMorph();
    nanoem_mutable_model_morph_uv_t *first_morph = scope.newUVMorph();
    nanoem_mutable_model_morph_uv_t *second_morph = scope.newUVMorph();
    nanoem_mutable_model_morph_uv_t *third_morph = scope.newUVMorph();
    nanoem_model_morph_uv_t *const *uv_morphs;
    nanoem_rsize_t num_uv_morphs;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    SECTION("type mismatch should reject")
    {
        nanoemMutableModelMorphInsertUVMorphObject(parent_morph, first_morph, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_TYPE_MISMATCH);
    }
    nanoemMutableModelMorphSetType(parent_morph, NANOEM_MODEL_MORPH_TYPE_UVA1);
    SECTION("PMD should reject")
    {
        nanoemMutableModelMorphInsertUVMorphObject(parent_morph, first_morph, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_VERSION_INCOMPATIBLE);
    }
    nanoemMutableModelSetFormatType(model, NANOEM_MODEL_FORMAT_TYPE_PMX_2_0);
    /* add */
    nanoemMutableModelMorphInsertUVMorphObject(parent_morph, first_morph, -1, &status);
    nanoemMutableModelMorphInsertUVMorphObject(parent_morph, second_morph, 1, &status);
    nanoemMutableModelMorphInsertUVMorphObject(parent_morph, third_morph, 0, &status);
    SECTION("duplication should reject")
    {
        nanoemMutableModelMorphInsertUVMorphObject(parent_morph, third_morph, 0, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        uv_morphs =
            nanoemModelMorphGetAllUVMorphObjects(nanoemMutableModelMorphGetOriginObject(parent_morph), &num_uv_morphs);
        CHECK(num_uv_morphs == 3);
        CHECK(uv_morphs[0] == nanoemMutableModelMorphUVGetOriginObject(third_morph));
        CHECK(uv_morphs[1] == nanoemMutableModelMorphUVGetOriginObject(first_morph));
        CHECK(uv_morphs[2] == nanoemMutableModelMorphUVGetOriginObject(second_morph));
    }
    SECTION("remove")
    {
        nanoemMutableModelMorphRemoveUVMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        uv_morphs =
            nanoemModelMorphGetAllUVMorphObjects(nanoemMutableModelMorphGetOriginObject(parent_morph), &num_uv_morphs);
        CHECK(num_uv_morphs == 2);
        CHECK(uv_morphs[0] == nanoemMutableModelMorphUVGetOriginObject(third_morph));
        CHECK(uv_morphs[1] == nanoemMutableModelMorphUVGetOriginObject(second_morph));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableModelMorphRemoveUVMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableModelMorphRemoveUVMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_UV_NOT_FOUND);
    }
}

TEST_CASE("mutable_morph_manipulate_vertex", "[nanoem]")
{
    ModelScope scope;
    scope.newModel();
    nanoem_mutable_model_morph_t *parent_morph = scope.newMorph();
    nanoem_mutable_model_morph_vertex_t *first_morph = scope.newVertexMorph();
    nanoem_mutable_model_morph_vertex_t *second_morph = scope.newVertexMorph();
    nanoem_mutable_model_morph_vertex_t *third_morph = scope.newVertexMorph();
    nanoem_model_morph_vertex_t *const *vertex_morphs;
    nanoem_rsize_t num_vertex_morphs;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelMorphSetType(parent_morph, NANOEM_MODEL_MORPH_TYPE_UNKNOWN);
    SECTION("type mismatch should reject")
    {
        nanoemMutableModelMorphInsertVertexMorphObject(parent_morph, first_morph, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_TYPE_MISMATCH);
    }
    nanoemMutableModelMorphSetType(parent_morph, NANOEM_MODEL_MORPH_TYPE_VERTEX);
    /* add */
    nanoemMutableModelMorphInsertVertexMorphObject(parent_morph, first_morph, -1, &status);
    nanoemMutableModelMorphInsertVertexMorphObject(parent_morph, second_morph, 1, &status);
    nanoemMutableModelMorphInsertVertexMorphObject(parent_morph, third_morph, 0, &status);
    SECTION("duplication should reject")
    {
        nanoemMutableModelMorphInsertVertexMorphObject(parent_morph, third_morph, 0, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        vertex_morphs = nanoemModelMorphGetAllVertexMorphObjects(
            nanoemMutableModelMorphGetOriginObject(parent_morph), &num_vertex_morphs);
        CHECK(num_vertex_morphs == 3);
        CHECK(vertex_morphs[0] == nanoemMutableModelMorphVertexGetOriginObject(third_morph));
        CHECK(vertex_morphs[1] == nanoemMutableModelMorphVertexGetOriginObject(first_morph));
        CHECK(vertex_morphs[2] == nanoemMutableModelMorphVertexGetOriginObject(second_morph));
    }
    SECTION("remove")
    {
        nanoemMutableModelMorphRemoveVertexMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        vertex_morphs = nanoemModelMorphGetAllVertexMorphObjects(
            nanoemMutableModelMorphGetOriginObject(parent_morph), &num_vertex_morphs);
        CHECK(num_vertex_morphs == 2);
        CHECK(vertex_morphs[0] == nanoemMutableModelMorphVertexGetOriginObject(third_morph));
        CHECK(vertex_morphs[1] == nanoemMutableModelMorphVertexGetOriginObject(second_morph));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableModelMorphRemoveVertexMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableModelMorphRemoveVertexMorphObject(parent_morph, first_morph, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MORPH_VERTEX_NOT_FOUND);
    }
}

TEST_CASE("mutable_morph_copy_name", "[nanoem]")
{
    ModelScope scope;
    scope.appendedMorph();
    nanoem_mutable_model_morph_t *second_morph = scope.appendedMorph();
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
            nanoemMutableModelMorphSetName(second_morph, s, language, &status);
            CHECK_THAT(
                scope.describe(nanoemModelMorphGetName(scope.copyMorph(NANOEM_MODEL_FORMAT_TYPE_PMD_1_0, 1), language)),
                Catch::Equals(input[i].expected));
        }
    }
}

TEST_CASE("mutable_material_copy_category", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_morph_t *first_morph = scope.appendedMorph();
    nanoemMutableModelMorphSetType(first_morph, NANOEM_MODEL_MORPH_TYPE_VERTEX);
    for (nanoem_rsize_t i = NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM; i < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM; i++) {
        nanoem_model_morph_category_t category = (nanoem_model_morph_category_t) i;
        nanoemMutableModelMorphSetCategory(first_morph, category);
        CHECK(nanoemModelMorphGetCategory(scope.copyMorph()) == category);
    }
}

TEST_CASE("mutable_material_copy_type", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_morph_t *first_morph = scope.appendedMorph();
    for (nanoem_rsize_t i = NANOEM_MODEL_MORPH_TYPE_FIRST_ENUM; i < NANOEM_MODEL_MORPH_TYPE_MAX_ENUM; i++) {
        nanoem_model_morph_type_t type = (nanoem_model_morph_type_t) i;
        nanoemMutableModelMorphSetType(first_morph, type);
        CHECK(nanoemModelMorphGetType(scope.copyMorph()) == type);
    }
}
