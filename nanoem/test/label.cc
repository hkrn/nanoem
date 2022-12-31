/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_label_basic", "[nanoem]")
{
    nanoem_rsize_t num_objects;
    CHECK_FALSE(nanoemModelLabelGetName(NULL, NANOEM_LANGUAGE_TYPE_JAPANESE));
    CHECK_FALSE(nanoemModelLabelGetName(NULL, NANOEM_LANGUAGE_TYPE_ENGLISH));
    CHECK_FALSE(nanoemModelLabelIsSpecial(NULL));
    CHECK_FALSE(nanoemModelLabelGetAllItemObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemModelLabelItemGetBoneObject(NULL));
    CHECK_FALSE(nanoemModelLabelItemGetMorphObject(NULL));
    CHECK(nanoemModelLabelItemGetType(NULL) == NANOEM_MODEL_LABEL_ITEM_TYPE_UNKNOWN);
    CHECK(nanoemModelObjectGetIndex(nanoemModelLabelGetModelObject(NULL)) == -1);
    nanoemModelLabelDestroy(NULL);
}

TEST_CASE("mutable_label_null", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    CHECK_FALSE(nanoemMutableModelLabelItemGetOriginObject(NULL));
    CHECK_FALSE(nanoemMutableModelLabelGetOriginObject(NULL));
    CHECK_FALSE(nanoemMutableModelLabelItemCreateFromBoneObject(NULL, NULL, &status));
    CHECK_FALSE(nanoemMutableModelLabelItemCreateFromMorphObject(NULL, NULL, &status));
    nanoemMutableModelLabelSetName(NULL, NULL, NANOEM_LANGUAGE_TYPE_JAPANESE, NULL);
    nanoemMutableModelLabelSetSpecial(NULL, nanoem_false);
    nanoemMutableModelLabelSaveToBuffer(NULL, NULL, &status);
    nanoemMutableModelLabelDestroy(NULL);
}

TEST_CASE("mutable_label_null_values", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_label_t *label = scope.newLabel();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelLabelSetName(label, NULL, NANOEM_LANGUAGE_TYPE_JAPANESE, NULL);
    nanoemMutableModelLabelSaveToBuffer(label, NULL, &status);
}

TEST_CASE("mutable_label_manipulate", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_t *model = scope.newModel();
    nanoem_mutable_model_label_t *first_label = scope.newLabel();
    nanoem_mutable_model_label_t *second_label = scope.newLabel();
    nanoem_mutable_model_label_t *third_label = scope.newLabel();
    nanoem_model_label_t *const *labels;
    nanoem_rsize_t num_labels;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelInsertLabelObject(model, first_label, -1, &status);
    nanoemMutableModelInsertLabelObject(model, second_label, 1, &status);
    nanoemMutableModelInsertLabelObject(model, third_label, 0, &status);
    SECTION("duplication should reject")
    {
        nanoemMutableModelInsertLabelObject(model, first_label, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_LABEL_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        labels = nanoemModelGetAllLabelObjects(nanoemMutableModelGetOriginObject(model), &num_labels);
        CHECK(num_labels == 3);
        CHECK(labels[0] == nanoemMutableModelLabelGetOriginObject(third_label));
        CHECK(labels[1] == nanoemMutableModelLabelGetOriginObject(first_label));
        CHECK(labels[2] == nanoemMutableModelLabelGetOriginObject(second_label));
    }
    SECTION("remove")
    {
        nanoemMutableModelRemoveLabelObject(model, first_label, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        labels = nanoemModelGetAllLabelObjects(nanoemMutableModelGetOriginObject(model), &num_labels);
        CHECK(num_labels == 2);
        CHECK(labels[0] == nanoemMutableModelLabelGetOriginObject(third_label));
        CHECK(labels[1] == nanoemMutableModelLabelGetOriginObject(second_label));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableModelRemoveLabelObject(model, first_label, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableModelRemoveLabelObject(model, first_label, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_LABEL_NOT_FOUND);
    }
}

TEST_CASE("mutable_label_manipulate_item", "[nanoem]")
{
    ModelScope scope;
    scope.newModel();
    nanoem_mutable_model_label_t *label = scope.newLabel();
    nanoem_model_bone_t *bone = nanoemMutableModelBoneGetOriginObject(scope.newBone());
    nanoem_model_morph_t *morph = nanoemMutableModelMorphGetOriginObject(scope.newMorph());
    nanoem_mutable_model_label_item_t *first_label = scope.newLabelItem(bone);
    nanoem_mutable_model_label_item_t *second_label = scope.newLabelItem(morph);
    nanoem_mutable_model_label_item_t *third_label = scope.newLabelItem(bone);
    nanoem_model_label_item_t *const *labels;
    nanoem_rsize_t num_labels;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelLabelInsertItemObject(label, first_label, -1, &status);
    nanoemMutableModelLabelInsertItemObject(label, second_label, 1, &status);
    nanoemMutableModelLabelInsertItemObject(label, third_label, 0, &status);
    SECTION("duplication should reject")
    {
        nanoemMutableModelLabelInsertItemObject(label, first_label, 0, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_LABEL_ALREADY_EXISTS);
    }
    SECTION("duplication should reject")
    {
        nanoemMutableModelLabelInsertItemObject(label, second_label, 0, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_LABEL_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        labels = nanoemModelLabelGetAllItemObjects(nanoemMutableModelLabelGetOriginObject(label), &num_labels);
        CHECK(num_labels == 3);
        CHECK(labels[0] == nanoemMutableModelLabelItemGetOriginObject(third_label));
        CHECK(labels[1] == nanoemMutableModelLabelItemGetOriginObject(first_label));
        CHECK(labels[2] == nanoemMutableModelLabelItemGetOriginObject(second_label));
    }
    SECTION("remove")
    {
        nanoemMutableModelLabelRemoveItemObject(label, first_label, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        labels = nanoemModelLabelGetAllItemObjects(nanoemMutableModelLabelGetOriginObject(label), &num_labels);
        CHECK(num_labels == 2);
        CHECK(labels[0] == nanoemMutableModelLabelItemGetOriginObject(third_label));
        CHECK(labels[1] == nanoemMutableModelLabelItemGetOriginObject(second_label));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableModelLabelRemoveItemObject(label, first_label, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableModelLabelRemoveItemObject(label, first_label, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_LABEL_ITEM_NOT_FOUND);
    }
}

TEST_CASE("mutable_label_copy_name", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_label_t *first_label = scope.appendedLabel();
    nanoem_mutable_model_label_t *second_label = scope.appendedLabel();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    struct input_t {
        const char *input;
        const char *expected;
    };
    input_t input[] = { { "0123456789012345678901234567890123456789012345678",
                            "0123456789012345678901234567890123456789012345678" /* 49bytes */ },
        { "01234567890123456789012345678901234567890123456789",
            "01234567890123456789012345678901234567890123456789" /* 50bytes */ },
        { "012345678901234567890123456789012345678901234567890",
            "01234567890123456789012345678901234567890123456789" /* 51bytes -> 50bytes */ } };
    nanoemMutableModelLabelSetSpecial(first_label, 1);
    for (size_t j = NANOEM_LANGUAGE_TYPE_FIRST_ENUM; j < NANOEM_LANGUAGE_TYPE_MAX_ENUM; j++) {
        nanoem_language_type_t language = (nanoem_language_type_t) j;
        for (size_t i = 0; i < sizeof(input) / sizeof(input[0]); i++) {
            const char *name = input[i].input;
            nanoem_unicode_string_t *s = scope.newString(name);
            nanoemMutableModelLabelSetName(second_label, s, language, &status);
            CHECK_THAT(
                scope.describe(nanoemModelLabelGetName(scope.copyLabel(NANOEM_MODEL_FORMAT_TYPE_PMD_1_0, 1), language)),
                Catch::Equals(input[i].expected));
        }
    }
}
