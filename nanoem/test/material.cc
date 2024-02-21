/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_material_basic", "[nanoem]")
{
    CHECK(nanoemModelMaterialGetAmbientColor(NULL));
    CHECK(nanoemModelMaterialGetDiffuseColor(NULL));
    CHECK(nanoemModelMaterialGetDiffuseTextureObject(NULL) == NULL);
    CHECK(nanoemModelMaterialGetDiffuseOpacity(NULL) == Approx(0));
    CHECK(nanoemModelMaterialGetEdgeColor(NULL));
    CHECK(nanoemModelMaterialGetEdgeOpacity(NULL) == Approx(0));
    CHECK(nanoemModelMaterialGetEdgeSize(NULL) == Approx(0));
    CHECK(nanoemModelMaterialGetName(NULL, NANOEM_LANGUAGE_TYPE_JAPANESE) == NULL);
    CHECK(nanoemModelMaterialGetName(NULL, NANOEM_LANGUAGE_TYPE_ENGLISH) == NULL);
    CHECK(nanoemModelMaterialGetName(NULL, NANOEM_LANGUAGE_TYPE_SIMPLIFIED_CHINESE) == NULL);
    CHECK(nanoemModelMaterialGetName(NULL, NANOEM_LANGUAGE_TYPE_TRADITIONAL_CHINESE) == NULL);
    CHECK(nanoemModelMaterialGetNumVertexIndices(NULL) == 0);
    CHECK(nanoemModelMaterialGetClob(NULL) == NULL);
    CHECK(nanoemModelMaterialGetSpecularPower(NULL) == Approx(0));
    CHECK(nanoemModelMaterialGetSpecularColor(NULL));
    CHECK(nanoemModelMaterialGetSphereMapTextureObject(NULL) == NULL);
    CHECK(nanoemModelMaterialGetSphereMapTextureType(NULL) == NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE);
    CHECK(nanoemModelMaterialGetToonTextureObject(NULL) == NULL);
    CHECK_FALSE(nanoemModelMaterialIsEdgeEnabled(NULL));
    CHECK_FALSE(nanoemModelMaterialIsToonShared(NULL));
    CHECK_FALSE(nanoemModelMaterialIsCastingShadowEnabled(NULL));
    CHECK_FALSE(nanoemModelMaterialIsCastingShadowMapEnabled(NULL));
    CHECK_FALSE(nanoemModelMaterialIsLineDrawEnabled(NULL));
    CHECK_FALSE(nanoemModelMaterialIsPointDrawEnabled(NULL));
    CHECK_FALSE(nanoemModelMaterialIsShadowMapEnabled(NULL));
    CHECK_FALSE(nanoemModelMaterialIsVertexColorEnabled(NULL));
    CHECK(nanoemModelObjectGetIndex(nanoemModelMaterialGetModelObject(NULL)) == -1);
    CHECK(nanoemModelMaterialGetModelObject(NULL) == NULL);
    CHECK(nanoemModelMaterialGetModelObjectMutable(NULL) == NULL);
    nanoemModelMaterialDestroy(NULL);
}

TEST_CASE("mutable_material_null", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    CHECK_FALSE(nanoemMutableModelMaterialGetOriginObject(NULL));
    nanoemMutableModelMaterialSetAmbientColor(NULL, NULL);
    nanoemMutableModelMaterialSetCastingShadowEnabled(NULL, nanoem_false);
    nanoemMutableModelMaterialSetCastingShadowMapEnabled(NULL, nanoem_false);
    nanoemMutableModelMaterialSetCullingDisabled(NULL, nanoem_false);
    nanoemMutableModelMaterialSetDiffuseColor(NULL, NULL);
    nanoemMutableModelMaterialSetDiffuseOpacity(NULL, 0);
    nanoemMutableModelMaterialSetDiffuseTextureObject(NULL, NULL, &status);
    nanoemMutableModelMaterialSetEdgeColor(NULL, NULL);
    nanoemMutableModelMaterialSetEdgeEnabled(NULL, nanoem_false);
    nanoemMutableModelMaterialSetEdgeOpacity(NULL, 0);
    nanoemMutableModelMaterialSetEdgeSize(NULL, 0);
    nanoemMutableModelMaterialSetLineDrawEnabled(NULL, nanoem_false);
    nanoemMutableModelMaterialSetName(NULL, NULL, NANOEM_LANGUAGE_TYPE_JAPANESE, NULL);
    nanoemMutableModelMaterialSetNumVertexIndices(NULL, 0);
    nanoemMutableModelMaterialSetClob(NULL, NULL, NULL);
    nanoemMutableModelMaterialSetPointDrawEnabled(NULL, nanoem_false);
    nanoemMutableModelMaterialSetShadowMapEnabled(NULL, nanoem_false);
    nanoemMutableModelMaterialSetSpecularPower(NULL, 0);
    nanoemMutableModelMaterialSetSpecularColor(NULL, NULL);
    nanoemMutableModelMaterialSetSphereMapTextureObject(NULL, NULL, &status);
    nanoemMutableModelMaterialSetSphereMapTextureType(NULL, NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_ADD);
    nanoemMutableModelMaterialSetToonShared(NULL, nanoem_false);
    nanoemMutableModelMaterialSetToonTextureIndex(NULL, 0);
    nanoemMutableModelMaterialSetToonTextureObject(NULL, NULL, &status);
    nanoemMutableModelMaterialSetVertexColorEnabled(NULL, nanoem_false);
    nanoemMutableModelMaterialSaveToBuffer(NULL, NULL, &status);
    nanoemMutableModelMaterialDestroy(NULL);
}

TEST_CASE("mutable_material_null_values", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *material = scope.newMaterial();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelMaterialSetAmbientColor(material, NULL);
    nanoemMutableModelMaterialSetDiffuseColor(material, NULL);
    nanoemMutableModelMaterialSetDiffuseTextureObject(material, NULL, &status);
    nanoemMutableModelMaterialSetEdgeColor(material, NULL);
    nanoemMutableModelMaterialSetName(material, NULL, NANOEM_LANGUAGE_TYPE_JAPANESE, NULL);
    nanoemMutableModelMaterialSetClob(material, NULL, NULL);
    nanoemMutableModelMaterialSetSpecularColor(material, NULL);
    nanoemMutableModelMaterialSetSphereMapTextureObject(material, NULL, &status);
    nanoemMutableModelMaterialSetToonTextureObject(material, NULL, &status);
    nanoemMutableModelMaterialSaveToBuffer(material, NULL, &status);
}

TEST_CASE("mutable_material_manipulate", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_t *model = scope.newModel();
    nanoem_mutable_model_material_t *first_material = scope.newMaterial();
    nanoem_mutable_model_material_t *second_material = scope.newMaterial();
    nanoem_mutable_model_material_t *third_material = scope.newMaterial();
    nanoem_model_material_t *const *materials;
    nanoem_rsize_t num_materials;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelInsertMaterialObject(model, first_material, -1, &status);
    nanoemMutableModelInsertMaterialObject(model, second_material, 1, &status);
    nanoemMutableModelInsertMaterialObject(model, third_material, 0, &status);
    SECTION("duplication should reject")
    {
        nanoemMutableModelInsertMaterialObject(model, first_material, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MATERIAL_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        materials = nanoemModelGetAllMaterialObjects(nanoemMutableModelGetOriginObject(model), &num_materials);
        CHECK(num_materials == 3);
        CHECK(materials[0] == nanoemMutableModelMaterialGetOriginObject(third_material));
        CHECK(materials[1] == nanoemMutableModelMaterialGetOriginObject(first_material));
        CHECK(materials[2] == nanoemMutableModelMaterialGetOriginObject(second_material));
    }
    SECTION("remove")
    {
        nanoemMutableModelRemoveMaterialObject(model, first_material, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        materials = nanoemModelGetAllMaterialObjects(nanoemMutableModelGetOriginObject(model), &num_materials);
        CHECK(num_materials == 2);
        CHECK(materials[0] == nanoemMutableModelMaterialGetOriginObject(third_material));
        CHECK(materials[2] == nanoemMutableModelMaterialGetOriginObject(second_material));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableModelRemoveMaterialObject(model, first_material, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableModelRemoveMaterialObject(model, first_material, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_MATERIAL_NOT_FOUND);
    }
}

TEST_CASE("mutable_material_copy_diffuse_texture", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    nanoem_mutable_model_texture_t *first_texture = scope.appendedTexture("texture0");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelMaterialSetDiffuseTextureObject(first_material, first_texture, &status);
    CHECK(status == NANOEM_STATUS_SUCCESS);
    CHECK_THAT(nanoemModelMaterialGetDiffuseTextureObject(scope.copyMaterial()), EqualsTexturePath("texture0"));
    nanoemMutableModelMaterialSetDiffuseTextureObject(first_material, nullptr, &status);
    CHECK(status == NANOEM_STATUS_SUCCESS);
    CHECK(nanoemModelMaterialGetDiffuseTextureObject(scope.copyMaterial()) == nullptr);
}

TEST_CASE("mutable_material_copy_sphere_map_texture", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    nanoem_mutable_model_texture_t *first_texture = scope.appendedTexture("texture0");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (nanoem_rsize_t i = NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_MULTIPLY;
         i < NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_MAX_ENUM; i++) {
        nanoem_model_material_sphere_map_texture_type_t type = (nanoem_model_material_sphere_map_texture_type_t) i;
        nanoemMutableModelMaterialSetSphereMapTextureType(first_material, type);
        nanoemMutableModelMaterialSetSphereMapTextureObject(first_material, first_texture, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        CHECK_THAT(nanoemModelMaterialGetSphereMapTextureObject(scope.copyMaterial()), EqualsTexturePath("texture0"));
        nanoemMutableModelMaterialSetSphereMapTextureObject(first_material, nullptr, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        CHECK(nanoemModelMaterialGetSphereMapTextureObject(scope.copyMaterial()) == nullptr);
    }
    nanoemMutableModelMaterialSetSphereMapTextureType(
        first_material, NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE);
    nanoemMutableModelMaterialSetSphereMapTextureObject(first_material, first_texture, &status);
    CHECK(status == NANOEM_STATUS_SUCCESS);
    CHECK(nanoemModelMaterialGetSphereMapTextureObject(scope.copyMaterial()) == nullptr);
}

TEST_CASE("mutable_material_copy_toon_texture", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    nanoem_mutable_model_texture_t *first_texture = scope.appendedTexture("texture0");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelMaterialSetToonTextureObject(first_material, first_texture, &status);
    CHECK(status == NANOEM_STATUS_SUCCESS);
    CHECK_THAT(nanoemModelMaterialGetToonTextureObject(scope.copyMaterial()), EqualsTexturePath("texture0"));
    nanoemMutableModelMaterialSetToonTextureObject(first_material, nullptr, &status);
    CHECK(status == NANOEM_STATUS_SUCCESS);
    CHECK(nanoemModelMaterialGetToonTextureObject(scope.copyMaterial()) == nullptr);
}

TEST_CASE("mutable_material_copy_shared_toon_texture", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    nanoem_mutable_model_texture_t *first_texture = scope.appendedTexture("texture0");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelMaterialSetToonTextureObject(first_material, first_texture, &status);
    nanoemMutableModelMaterialSetToonShared(first_material, nanoem_true);
    nanoemMutableModelMaterialSetToonTextureIndex(first_material, 8);
    const nanoem_model_material_t *enabled_material = scope.copyMaterial();
    CHECK(nanoemModelMaterialIsToonShared(enabled_material));
    CHECK(nanoemModelMaterialGetToonTextureIndex(enabled_material) == 8);
    CHECK(nanoemModelMaterialGetToonTextureObject(enabled_material) == nullptr);
    nanoemMutableModelMaterialSetToonShared(first_material, nanoem_false);
    const nanoem_model_material_t *disabled_material = scope.copyMaterial();
    CHECK_FALSE(nanoemModelMaterialIsToonShared(disabled_material));
    CHECK(nanoemModelMaterialGetToonTextureIndex(disabled_material) == 8);
    CHECK(nanoemModelMaterialGetToonTextureObject(disabled_material) == nullptr);
}

TEST_CASE("mutable_material_copy_ambient", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelMaterialSetAmbientColor(first_material, value);
    CHECK_THAT(nanoemModelMaterialGetAmbientColor(scope.copyMaterial()), Equals(value));
}

TEST_CASE("mutable_material_copy_diffuse", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelMaterialSetDiffuseColor(first_material, value);
    CHECK_THAT(nanoemModelMaterialGetDiffuseColor(scope.copyMaterial()), Equals(value));
}

TEST_CASE("mutable_material_copy_specular", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelMaterialSetSpecularColor(first_material, value);
    CHECK_THAT(nanoemModelMaterialGetSpecularColor(scope.copyMaterial()), Equals(value));
}

TEST_CASE("mutable_material_copy_edge_color", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    float value[] = { 1, 2, 3, 0 };
    nanoemMutableModelMaterialSetEdgeColor(first_material, value);
    CHECK_THAT(nanoemModelMaterialGetEdgeColor(scope.copyMaterial()), Equals(value));
}

TEST_CASE("mutable_material_copy_diffuse_opacity", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    nanoemMutableModelMaterialSetDiffuseOpacity(first_material, 0.42f);
    CHECK(nanoemModelMaterialGetDiffuseOpacity(scope.copyMaterial()) == Approx(0.42f));
}

TEST_CASE("mutable_material_copy_specular_power", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    nanoemMutableModelMaterialSetSpecularPower(first_material, 0.42f);
    CHECK(nanoemModelMaterialGetSpecularPower(scope.copyMaterial()) == Approx(0.42f));
}

TEST_CASE("mutable_material_copy_edge_opacity", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    nanoemMutableModelMaterialSetEdgeOpacity(first_material, 0.42f);
    CHECK(nanoemModelMaterialGetEdgeOpacity(scope.copyMaterial()) == Approx(0.42f));
}

TEST_CASE("mutable_material_copy_edge_size", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    nanoemMutableModelMaterialSetEdgeSize(first_material, 0.42f);
    CHECK(nanoemModelMaterialGetEdgeSize(scope.copyMaterial()) == Approx(0.42f));
}

TEST_CASE("mutable_material_copy_num_vertex_indices", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    nanoemMutableModelMaterialSetNumVertexIndices(first_material, 42);
    CHECK(nanoemModelMaterialGetNumVertexIndices(scope.copyMaterial()) == 42);
}

TEST_CASE("mutable_material_copy_is_casting_shadow_enabled", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    nanoemMutableModelMaterialSetCastingShadowEnabled(first_material, 1);
    CHECK(nanoemModelMaterialIsCastingShadowEnabled(scope.copyMaterial()));
    nanoemMutableModelMaterialSetCastingShadowEnabled(first_material, 0);
    CHECK_FALSE(nanoemModelMaterialIsCastingShadowEnabled(scope.copyMaterial()));
}

TEST_CASE("mutable_material_copy_is_culling_disabled", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    nanoemMutableModelMaterialSetCullingDisabled(first_material, 1);
    CHECK(nanoemModelMaterialIsCullingDisabled(scope.copyMaterial()));
    nanoemMutableModelMaterialSetCullingDisabled(first_material, 0);
    CHECK_FALSE(nanoemModelMaterialIsCullingDisabled(scope.copyMaterial()));
}

TEST_CASE("mutable_material_copy_is_edge_enabled", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    nanoemMutableModelMaterialSetEdgeEnabled(first_material, 1);
    CHECK(nanoemModelMaterialIsEdgeEnabled(scope.copyMaterial()));
    nanoemMutableModelMaterialSetEdgeEnabled(first_material, 0);
    CHECK_FALSE(nanoemModelMaterialIsEdgeEnabled(scope.copyMaterial()));
}

TEST_CASE("mutable_material_copy_is_line_draw_enabled", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    nanoemMutableModelMaterialSetLineDrawEnabled(first_material, 1);
    CHECK(nanoemModelMaterialIsLineDrawEnabled(scope.copyMaterial()));
    nanoemMutableModelMaterialSetLineDrawEnabled(first_material, 0);
    CHECK_FALSE(nanoemModelMaterialIsLineDrawEnabled(scope.copyMaterial()));
}

TEST_CASE("mutable_material_copy_is_point_draw_enabled", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    nanoemMutableModelMaterialSetPointDrawEnabled(first_material, 1);
    CHECK(nanoemModelMaterialIsPointDrawEnabled(scope.copyMaterial()));
    nanoemMutableModelMaterialSetPointDrawEnabled(first_material, 0);
    CHECK_FALSE(nanoemModelMaterialIsPointDrawEnabled(scope.copyMaterial()));
}

TEST_CASE("mutable_material_copy_is_shadow_map_enabled", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    nanoemMutableModelMaterialSetShadowMapEnabled(first_material, 1);
    CHECK(nanoemModelMaterialIsShadowMapEnabled(scope.copyMaterial()));
    nanoemMutableModelMaterialSetShadowMapEnabled(first_material, 0);
    CHECK_FALSE(nanoemModelMaterialIsShadowMapEnabled(scope.copyMaterial()));
}

TEST_CASE("mutable_material_copy_is_vertex_color_enabled", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_material_t *first_material = scope.appendedMaterial();
    nanoemMutableModelMaterialSetVertexColorEnabled(first_material, 1);
    CHECK(nanoemModelMaterialIsVertexColorEnabled(scope.copyMaterial()));
    nanoemMutableModelMaterialSetVertexColorEnabled(first_material, 0);
    CHECK_FALSE(nanoemModelMaterialIsVertexColorEnabled(scope.copyMaterial()));
}
