/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_vertex_basic", "[nanoem]")
{
    CHECK_FALSE(nanoemModelVertexGetBoneObject(NULL, 0));
    CHECK(nanoemModelVertexGetBoneWeight(NULL, 0) == Approx(0));
    CHECK(nanoemModelVertexGetEdgeSize(NULL) == Approx(0));
    CHECK(nanoemModelVertexGetNormal(NULL));
    CHECK(nanoemModelVertexGetOrigin(NULL));
    CHECK(nanoemModelVertexGetSdefC(NULL));
    CHECK(nanoemModelVertexGetSdefR0(NULL));
    CHECK(nanoemModelVertexGetSdefR1(NULL));
    CHECK(nanoemModelVertexGetTexCoord(NULL));
    CHECK(nanoemModelVertexGetType(NULL) == NANOEM_MODEL_VERTEX_TYPE_UNKNOWN);
    CHECK(nanoemModelObjectGetIndex(nanoemModelVertexGetModelObject(NULL)) == -1);
    nanoemModelVertexDestroy(NULL);
}

TEST_CASE("mutable_vertex_null", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    CHECK_FALSE(nanoemMutableModelVertexGetOriginObject(NULL));
    nanoemMutableModelVertexSetAdditionalUV(NULL, NULL, 0);
    nanoemMutableModelVertexSetBoneObject(NULL, NULL, 0);
    nanoemMutableModelVertexSetBoneWeight(NULL, 0, 0);
    nanoemMutableModelVertexSetEdgeSize(NULL, 0);
    nanoemMutableModelVertexSetNormal(NULL, 0);
    nanoemMutableModelVertexSetOrigin(NULL, NULL);
    nanoemMutableModelVertexSetSdefC(NULL, NULL);
    nanoemMutableModelVertexSetSdefR0(NULL, NULL);
    nanoemMutableModelVertexSetSdefR1(NULL, NULL);
    nanoemMutableModelVertexSetTexCoord(NULL, NULL);
    nanoemMutableModelVertexSetType(NULL, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
    nanoemMutableModelVertexSaveToBuffer(NULL, NULL, &status);
    nanoemMutableModelVertexDestroy(NULL);
}

TEST_CASE("mutable_vertex_null_values", "[nanoem]")
{
    ModelScope scope;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_vertex_t *vertex = scope.newVertex();
    nanoem_model_bone_t *bone = nanoemModelBoneCreate(scope.origin(), &status);
    nanoemMutableModelVertexSetAdditionalUV(vertex, NULL, 0);
    nanoemMutableModelVertexSetAdditionalUV(vertex, __nanoem_null_vector3, -1);
    nanoemMutableModelVertexSetAdditionalUV(vertex, __nanoem_null_vector3, 4);
    nanoemMutableModelVertexSetBoneObject(vertex, NULL, 0);
    nanoemMutableModelVertexSetBoneObject(vertex, bone, -1);
    nanoemMutableModelVertexSetBoneObject(vertex, bone, 4);
    nanoemMutableModelVertexSetOrigin(vertex, NULL);
    nanoemMutableModelVertexSetSdefC(vertex, NULL);
    nanoemMutableModelVertexSetSdefR0(vertex, NULL);
    nanoemMutableModelVertexSetSdefR1(vertex, NULL);
    nanoemMutableModelVertexSetTexCoord(vertex, NULL);
    nanoemMutableModelVertexSaveToBuffer(vertex, NULL, &status);
    nanoemModelBoneDestroy(bone);
}

TEST_CASE("mutable_vertex_manipulate", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_t *model = scope.newModel();
    nanoem_mutable_model_vertex_t *first_vertex = scope.newVertex();
    nanoem_mutable_model_vertex_t *second_vertex = scope.newVertex();
    nanoem_mutable_model_vertex_t *third_vertex = scope.newVertex();
    nanoem_model_vertex_t *const *vertices;
    nanoem_rsize_t num_vertices;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    /* add */
    nanoemMutableModelInsertVertexObject(model, first_vertex, -1, &status);
    nanoemMutableModelInsertVertexObject(model, second_vertex, 1, &status);
    nanoemMutableModelInsertVertexObject(model, third_vertex, 0, &status);
    SECTION("duplication should reject")
    {
        nanoemMutableModelInsertVertexObject(model, first_vertex, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_VERTEX_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        vertices = nanoemModelGetAllVertexObjects(nanoemMutableModelGetOriginObject(model), &num_vertices);
        CHECK(num_vertices == 3);
        CHECK(vertices[0] == nanoemMutableModelVertexGetOriginObject(third_vertex));
        CHECK(vertices[1] == nanoemMutableModelVertexGetOriginObject(first_vertex));
        CHECK(vertices[2] == nanoemMutableModelVertexGetOriginObject(second_vertex));
    }
    SECTION("remove")
    {
        /* remove */
        nanoemMutableModelRemoveVertexObject(model, first_vertex, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        vertices = nanoemModelGetAllVertexObjects(nanoemMutableModelGetOriginObject(model), &num_vertices);
        CHECK(num_vertices == 2);
        CHECK(vertices[0] == nanoemMutableModelVertexGetOriginObject(third_vertex));
        CHECK(vertices[1] == nanoemMutableModelVertexGetOriginObject(second_vertex));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableModelRemoveVertexObject(model, first_vertex, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableModelRemoveVertexObject(model, first_vertex, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_VERTEX_NOT_FOUND);
    }
}

TEST_CASE("mutable_vertex_copy_origin", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_vertex_t *first_vertex = scope.appendedVertex();
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelVertexSetType(first_vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
    nanoemMutableModelVertexSetOrigin(first_vertex, value);
    CHECK_THAT(nanoemModelVertexGetOrigin(scope.copyVertex()), Equals(value));
}

TEST_CASE("mutable_vertex_copy_normal", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_vertex_t *first_vertex = scope.appendedVertex();
    float value[] = { 0.1f, 0.2f, 0.3f, 0 };
    nanoemMutableModelVertexSetType(first_vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
    nanoemMutableModelVertexSetNormal(first_vertex, value);
    CHECK_THAT(nanoemModelVertexGetNormal(scope.copyVertex()), Equals(value));
}

TEST_CASE("mutable_vertex_copy_texcoord", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_vertex_t *first_vertex = scope.appendedVertex();
    float value[] = { 0.1f, 0.2f, 0, 0 };
    nanoemMutableModelVertexSetType(first_vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
    nanoemMutableModelVertexSetTexCoord(first_vertex, value);
    CHECK_THAT(nanoemModelVertexGetTexCoord(scope.copyVertex()), Equals(value));
}

TEST_CASE("mutable_vertex_copy_edge", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_vertex_t *first_vertex = scope.appendedVertex();
    nanoemMutableModelVertexSetType(first_vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
    nanoemMutableModelVertexSetEdgeSize(first_vertex, 0.42f);
    CHECK(nanoemModelVertexGetEdgeSize(scope.copyVertex()) == Approx(0.42f));
}

TEST_CASE("mutable_vertex_copy_bdef1", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_vertex_t *first_vertex = scope.appendedVertex();
    nanoem_model_bone_t *bone0 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone0"));
    nanoem_model_bone_t *bone1 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone1"));
    nanoem_model_bone_t *bone2 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone2"));
    nanoem_model_bone_t *bone3 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone3"));
    nanoemMutableModelVertexSetType(first_vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone0, 0);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone1, 1);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone2, 2);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone3, 3);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.4f, 0);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.3f, 1);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.2f, 2);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.1f, 3);
    const nanoem_model_vertex_t *copy = scope.copyVertex();
    CHECK(nanoemModelVertexGetType(copy) == NANOEM_MODEL_VERTEX_TYPE_BDEF1);
    CHECK_THAT(nanoemModelVertexGetBoneObject(copy, 0), EqualsBoneName("bone0"));
    CHECK(nanoemModelVertexGetBoneObject(copy, 1) == NULL);
    CHECK(nanoemModelVertexGetBoneObject(copy, 2) == NULL);
    CHECK(nanoemModelVertexGetBoneObject(copy, 3) == NULL);
    CHECK(nanoemModelVertexGetBoneWeight(copy, 0) == Approx(1.0f));
    CHECK(nanoemModelVertexGetBoneWeight(copy, 1) == Approx(0));
    CHECK(nanoemModelVertexGetBoneWeight(copy, 2) == Approx(0));
    CHECK(nanoemModelVertexGetBoneWeight(copy, 3) == Approx(0));
}

TEST_CASE("mutable_vertex_copy_bdef2", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_vertex_t *first_vertex = scope.appendedVertex();
    nanoem_model_bone_t *bone0 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone0"));
    nanoem_model_bone_t *bone1 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone1"));
    nanoem_model_bone_t *bone2 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone2"));
    nanoem_model_bone_t *bone3 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone3"));
    nanoemMutableModelVertexSetType(first_vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF2);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone0, 0);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone1, 1);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone2, 2);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone3, 3);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.4f, 0);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.3f, 1);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.2f, 2);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.1f, 3);
    const nanoem_model_vertex_t *copy = scope.copyVertex();
    CHECK(nanoemModelVertexGetType(copy) == NANOEM_MODEL_VERTEX_TYPE_BDEF2);
    CHECK_THAT(nanoemModelVertexGetBoneObject(copy, 0), EqualsBoneName("bone0"));
    CHECK_THAT(nanoemModelVertexGetBoneObject(copy, 1), EqualsBoneName("bone1"));
    CHECK(nanoemModelVertexGetBoneObject(copy, 2) == NULL);
    CHECK(nanoemModelVertexGetBoneObject(copy, 3) == NULL);
    CHECK(nanoemModelVertexGetBoneWeight(copy, 0) == Approx(0.4f));
    CHECK(nanoemModelVertexGetBoneWeight(copy, 1) == Approx(0.6f));
    CHECK(nanoemModelVertexGetBoneWeight(copy, 2) == Approx(0));
    CHECK(nanoemModelVertexGetBoneWeight(copy, 3) == Approx(0));
}

TEST_CASE("mutable_vertex_copy_bdef4", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_vertex_t *first_vertex = scope.appendedVertex();
    nanoem_model_bone_t *bone0 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone0"));
    nanoem_model_bone_t *bone1 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone1"));
    nanoem_model_bone_t *bone2 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone2"));
    nanoem_model_bone_t *bone3 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone3"));
    nanoemMutableModelVertexSetType(first_vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF4);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone0, 0);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone1, 1);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone2, 2);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone3, 3);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.4f, 0);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.3f, 1);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.2f, 2);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.1f, 3);
    const nanoem_model_vertex_t *copy = scope.copyVertex();
    CHECK(nanoemModelVertexGetType(copy) == NANOEM_MODEL_VERTEX_TYPE_BDEF4);
    CHECK_THAT(nanoemModelVertexGetBoneObject(copy, 0), EqualsBoneName("bone0"));
    CHECK_THAT(nanoemModelVertexGetBoneObject(copy, 1), EqualsBoneName("bone1"));
    CHECK_THAT(nanoemModelVertexGetBoneObject(copy, 2), EqualsBoneName("bone2"));
    CHECK_THAT(nanoemModelVertexGetBoneObject(copy, 3), EqualsBoneName("bone3"));
    CHECK(nanoemModelVertexGetBoneWeight(copy, 0) == Approx(0.4f));
    CHECK(nanoemModelVertexGetBoneWeight(copy, 1) == Approx(0.3f));
    CHECK(nanoemModelVertexGetBoneWeight(copy, 2) == Approx(0.2f));
    CHECK(nanoemModelVertexGetBoneWeight(copy, 3) == Approx(0.1f));
}

TEST_CASE("mutable_vertex_copy_sdef", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_vertex_t *first_vertex = scope.appendedVertex();
    nanoem_model_bone_t *bone0 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone0"));
    nanoem_model_bone_t *bone1 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone1"));
    nanoem_model_bone_t *bone2 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone2"));
    nanoem_model_bone_t *bone3 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone3"));
    nanoemMutableModelVertexSetType(first_vertex, NANOEM_MODEL_VERTEX_TYPE_SDEF);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone0, 0);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone1, 1);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone2, 2);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone3, 3);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.4f, 0);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.3f, 1);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.2f, 2);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.1f, 3);
    const nanoem_model_vertex_t *copy = scope.copyVertex();
    CHECK(nanoemModelVertexGetType(copy) == NANOEM_MODEL_VERTEX_TYPE_SDEF);
    CHECK_THAT(nanoemModelVertexGetBoneObject(copy, 0), EqualsBoneName("bone0"));
    CHECK_THAT(nanoemModelVertexGetBoneObject(copy, 1), EqualsBoneName("bone1"));
    CHECK(nanoemModelVertexGetBoneObject(copy, 2) == NULL);
    CHECK(nanoemModelVertexGetBoneObject(copy, 3) == NULL);
    CHECK(nanoemModelVertexGetBoneWeight(copy, 0) == Approx(0.4f));
    CHECK(nanoemModelVertexGetBoneWeight(copy, 1) == Approx(0.6f));
    CHECK(nanoemModelVertexGetBoneWeight(copy, 2) == Approx(0));
    CHECK(nanoemModelVertexGetBoneWeight(copy, 3) == Approx(0));
}

TEST_CASE("mutable_vertex_copy_qdef", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_vertex_t *first_vertex = scope.appendedVertex();
    nanoem_model_bone_t *bone0 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone0"));
    nanoem_model_bone_t *bone1 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone1"));
    nanoem_model_bone_t *bone2 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone2"));
    nanoem_model_bone_t *bone3 = nanoemMutableModelBoneGetOriginObject(scope.appendedBone("bone3"));
    nanoemMutableModelVertexSetType(first_vertex, NANOEM_MODEL_VERTEX_TYPE_QDEF);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone0, 0);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone1, 1);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone2, 2);
    nanoemMutableModelVertexSetBoneObject(first_vertex, bone3, 3);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.4f, 0);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.3f, 1);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.2f, 2);
    nanoemMutableModelVertexSetBoneWeight(first_vertex, 0.1f, 3);
    const nanoem_model_vertex_t *copy = scope.copyVertex(NANOEM_MODEL_FORMAT_TYPE_PMX_2_1);
    CHECK(nanoemModelVertexGetType(copy) == NANOEM_MODEL_VERTEX_TYPE_QDEF);
    CHECK_THAT(nanoemModelVertexGetBoneObject(copy, 0), EqualsBoneName("bone0"));
    CHECK_THAT(nanoemModelVertexGetBoneObject(copy, 1), EqualsBoneName("bone1"));
    CHECK_THAT(nanoemModelVertexGetBoneObject(copy, 2), EqualsBoneName("bone2"));
    CHECK_THAT(nanoemModelVertexGetBoneObject(copy, 3), EqualsBoneName("bone3"));
    CHECK(nanoemModelVertexGetBoneWeight(copy, 0) == Approx(0.4f));
    CHECK(nanoemModelVertexGetBoneWeight(copy, 1) == Approx(0.3f));
    CHECK(nanoemModelVertexGetBoneWeight(copy, 2) == Approx(0.2f));
    CHECK(nanoemModelVertexGetBoneWeight(copy, 3) == Approx(0.1f));
}

TEST_CASE("mutable_vertex_copy_sdef_c", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_vertex_t *first_vertex = scope.appendedVertex();
    const float value[] = { 0.1f, 0.2f, 0.3f, 1.0f };
    SECTION("NG")
    {
        nanoemMutableModelVertexSetType(first_vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
        nanoemMutableModelVertexSetSdefC(first_vertex, value);
        CHECK_THAT(nanoemModelVertexGetSdefC(scope.copyVertex()), EqualsOne(0));
    }
    SECTION("OK")
    {
        nanoemMutableModelVertexSetType(first_vertex, NANOEM_MODEL_VERTEX_TYPE_SDEF);
        nanoemMutableModelVertexSetSdefC(first_vertex, value);
        CHECK_THAT(nanoemModelVertexGetSdefC(scope.copyVertex()), Equals(value));
    }
}

TEST_CASE("mutable_vertex_copy_sdef_r0", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_vertex_t *first_vertex = scope.appendedVertex();
    const float value[] = { 0.1f, 0.2f, 0.3f, 1.0f };
    SECTION("NG")
    {
        nanoemMutableModelVertexSetType(first_vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
        nanoemMutableModelVertexSetSdefR0(first_vertex, value);
        CHECK_THAT(nanoemModelVertexGetSdefR0(scope.copyVertex()), EqualsOne(0));
    }
    SECTION("OK")
    {
        nanoemMutableModelVertexSetType(first_vertex, NANOEM_MODEL_VERTEX_TYPE_SDEF);
        nanoemMutableModelVertexSetSdefR0(first_vertex, value);
        CHECK_THAT(nanoemModelVertexGetSdefR0(scope.copyVertex()), Equals(value));
    }
}

TEST_CASE("mutable_vertex_copy_sdef_r1", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_vertex_t *first_vertex = scope.appendedVertex();
    const float value[] = { 0.1f, 0.2f, 0.3f, 1.0f };
    SECTION("NG")
    {
        nanoemMutableModelVertexSetType(first_vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
        nanoemMutableModelVertexSetSdefR1(first_vertex, value);
        CHECK_THAT(nanoemModelVertexGetSdefR1(scope.copyVertex()), EqualsOne(0));
    }
    SECTION("OK")
    {
        nanoemMutableModelVertexSetType(first_vertex, NANOEM_MODEL_VERTEX_TYPE_SDEF);
        nanoemMutableModelVertexSetSdefR1(first_vertex, value);
        CHECK_THAT(nanoemModelVertexGetSdefR1(scope.copyVertex()), Equals(value));
    }
}

#if 0
TEST_CASE("mutable_vertex_copy_additional_uv", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_vertex_t *first_vertex = scope.appendedVertex();
    const float value[] = { 0.1f, 0.2f, 0.3f, 0.4f };
    nanoemMutableModelVertexSetAdditionalUV(first_vertex, value, 0);
    nanoemMutableModelVertexSetAdditionalUV(first_vertex, value, 1);
    nanoemMutableModelVertexSetAdditionalUV(first_vertex, value, 2);
    nanoemMutableModelVertexSetAdditionalUV(first_vertex, value, 3);
    nanoem_model_vertex_t *copy = scope.copyVertex(NANOEM_MODEL_FORMAT_TYPE_PMX_2_1);
    CHECK_THAT(nanoemModelVertexGetAdditionalUV(copy, 0), Equals(value));
    CHECK_THAT(nanoemModelVertexGetAdditionalUV(copy, 1), Equals(value));
    CHECK_THAT(nanoemModelVertexGetAdditionalUV(copy, 2), Equals(value));
    CHECK_THAT(nanoemModelVertexGetAdditionalUV(copy, 3), Equals(value));
}
#endif
