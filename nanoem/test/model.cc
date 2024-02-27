/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_model_basic", "[nanoem]")
{
    nanoem_rsize_t num_objects;
    CHECK_FALSE(nanoemModelCreate(NULL, NULL));
    CHECK_FALSE(nanoemModelGetAllBoneObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemModelGetAllOrderedBoneObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK(nanoemModelGetCodecType(NULL) == NANOEM_CODEC_TYPE_UNKNOWN);
    CHECK_FALSE(nanoemModelGetComment(NULL, NANOEM_LANGUAGE_TYPE_JAPANESE));
    CHECK_FALSE(nanoemModelGetComment(NULL, NANOEM_LANGUAGE_TYPE_ENGLISH));
    CHECK_FALSE(nanoemModelGetComment(NULL, NANOEM_LANGUAGE_TYPE_SIMPLIFIED_CHINESE));
    CHECK_FALSE(nanoemModelGetComment(NULL, NANOEM_LANGUAGE_TYPE_TRADITIONAL_CHINESE));
    CHECK_FALSE(nanoemModelGetAllConstraintObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemModelGetAllJointObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemModelGetAllLabelObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemModelGetAllMaterialObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemModelGetAllMorphObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemModelGetName(NULL, NANOEM_LANGUAGE_TYPE_JAPANESE));
    CHECK_FALSE(nanoemModelGetName(NULL, NANOEM_LANGUAGE_TYPE_ENGLISH));
    CHECK_FALSE(nanoemModelGetAllRigidBodyObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK_FALSE(nanoemModelGetAllTextureObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK(nanoemModelGetFormatType(NULL) == NANOEM_MODEL_FORMAT_TYPE_UNKNOWN);
    CHECK_FALSE(nanoemModelGetAllVertexObjects(NULL, &num_objects));
    CHECK(num_objects == 0);
    CHECK(nanoemModelVertexGetAdditionalUV(NULL, 0));
    CHECK_FALSE(nanoemModelVertexGetBoneObject(NULL, 0));
    CHECK_FALSE(nanoemModelVertexGetModelObject(NULL));
    CHECK_FALSE(nanoemModelVertexGetModelObjectMutable(NULL));
    nanoemModelDestroy(NULL);
}

TEST_CASE("null_model_internal", "[nanoem]")
{
    /* model (internal) */
    ModelScope scope;
    nanoem_model_t *model = nanoemMutableModelGetOriginObject(scope.newModel());
    CHECK_FALSE(nanoemModelGetOneBoneObject(NULL, 0));
    CHECK_FALSE(nanoemModelGetOneBoneObject(model, -1));
    CHECK_FALSE(nanoemModelGetOneBoneObject(model, 0));
    CHECK_FALSE(nanoemModelGetOneBoneObject(model, 1));
    CHECK_FALSE(nanoemModelGetOneRigidBodyObject(NULL, -1));
    CHECK_FALSE(nanoemModelGetOneRigidBodyObject(model, -1));
    CHECK_FALSE(nanoemModelGetOneRigidBodyObject(model, 0));
    CHECK_FALSE(nanoemModelGetOneRigidBodyObject(model, 1));
    CHECK_FALSE(nanoemModelGetOneTextureObject(NULL, -1));
    CHECK_FALSE(nanoemModelGetOneTextureObject(model, -1));
    CHECK_FALSE(nanoemModelGetOneTextureObject(model, 0));
    CHECK_FALSE(nanoemModelGetOneTextureObject(model, 1));
}

TEST_CASE("mutable_model_null", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    CHECK_FALSE(nanoemMutableModelGetOriginObject(NULL));
    CHECK_FALSE(nanoemMutableModelGetOriginObjectReference(NULL));
    CHECK_FALSE(nanoemMutableModelCreateAsReference(NULL, NULL));
    nanoemMutableModelSetComment(NULL, NULL, NANOEM_LANGUAGE_TYPE_FIRST_ENUM, NULL);
    nanoemMutableModelSetName(NULL, NULL, NANOEM_LANGUAGE_TYPE_FIRST_ENUM, NULL);
    nanoemMutableModelSetFormatType(NULL, NANOEM_MODEL_FORMAT_TYPE_FIRST_ENUM);
    nanoemMutableModelSetCodecType(NULL, NANOEM_CODEC_TYPE_FIRST_ENUM);
    nanoemMutableModelSetVertexIndices(NULL, NULL, 0, &status);
    CHECK_FALSE(nanoemMutableModelSaveToBuffer(NULL, NULL, &status));
    nanoemMutableModelDestroy(NULL);
}

TEST_CASE("mutable_model_null_values", "[nanoem]")
{
    ModelScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_t *model = scope.newModel();
    nanoemMutableModelSetComment(model, NULL, NANOEM_LANGUAGE_TYPE_FIRST_ENUM, NULL);
    nanoemMutableModelSetName(model, NULL, NANOEM_LANGUAGE_TYPE_FIRST_ENUM, NULL);
    nanoemMutableModelSetVertexIndices(model, NULL, 0, &status);
    CHECK_FALSE(nanoemMutableModelSaveToBuffer(model, NULL, &status));
}

TEST_CASE("mutable_model_copy_name", "[nanoem]")
{
    ModelScope scope;
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
            nanoem_mutable_model_t *model = scope.newModel();
            nanoemMutableModelSetName(model, s, language, &status);
            scope.copy(NANOEM_MODEL_FORMAT_TYPE_PMD_1_0);
            CHECK_THAT(
                scope.describe(nanoemModelGetName(scope.reference(), language)), Catch::Equals(input[i].expected));
        }
    }
}
