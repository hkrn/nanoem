/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_texture_basic", "[nanoem]")
{
    CHECK_FALSE(nanoemModelTextureGetPath(NULL));
    CHECK(nanoemModelObjectGetIndex(nanoemModelTextureGetModelObject(NULL)) == -1);
    CHECK_FALSE(nanoemModelTextureGetModelObject(NULL));
    CHECK_FALSE(nanoemModelTextureGetModelObjectMutable(NULL));
    nanoemModelTextureDestroy(NULL);
}

TEST_CASE("mutable_texture_null", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    CHECK_FALSE(nanoemMutableModelTextureGetOriginObject(NULL));
    nanoemMutableModelTextureSetPath(NULL, NULL, NULL);
    nanoemMutableModelTextureSaveToBuffer(NULL, NULL, &status);
    nanoemMutableModelTextureDestroy(NULL);
}

TEST_CASE("mutable_texture_null_values", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_texture_t *texture = scope.newTexture();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelTextureSetPath(texture, NULL, NULL);
    nanoemMutableModelTextureSaveToBuffer(texture, NULL, &status);
}

TEST_CASE("mutable_texture_manipulate", "[nanoem]")
{
    ModelScope scope;
    nanoem_mutable_model_t *model = scope.newModel();
    nanoem_mutable_model_texture_t *first_texture = scope.newTexture();
    nanoem_mutable_model_texture_t *second_texture = scope.newTexture();
    nanoem_mutable_model_texture_t *third_texture = scope.newTexture();
    nanoem_model_texture_t *const *textures;
    nanoem_rsize_t num_textures;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    /* add */
    nanoemMutableModelInsertTextureObject(model, first_texture, -1, &status);
    nanoemMutableModelInsertTextureObject(model, second_texture, 1, &status);
    nanoemMutableModelInsertTextureObject(model, third_texture, 0, &status);
    SECTION("duplication should reject")
    {
        nanoemMutableModelInsertTextureObject(model, first_texture, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_TEXTURE_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        textures = nanoemModelGetAllTextureObjects(nanoemMutableModelGetOriginObject(model), &num_textures);
        CHECK(num_textures == 3);
        CHECK(textures[0] == nanoemMutableModelTextureGetOriginObject(third_texture));
        CHECK(textures[1] == nanoemMutableModelTextureGetOriginObject(first_texture));
        CHECK(textures[2] == nanoemMutableModelTextureGetOriginObject(second_texture));
    }
    SECTION("remove")
    {
        nanoemMutableModelRemoveTextureObject(model, first_texture, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        textures = nanoemModelGetAllTextureObjects(nanoemMutableModelGetOriginObject(model), &num_textures);
        CHECK(num_textures == 2);
        CHECK(textures[0] == nanoemMutableModelTextureGetOriginObject(third_texture));
        CHECK(textures[1] == nanoemMutableModelTextureGetOriginObject(second_texture));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableModelRemoveTextureObject(model, first_texture, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableModelRemoveTextureObject(model, first_texture, &status);
        CHECK(status == NANOEM_STATUS_ERROR_MODEL_TEXTURE_NOT_FOUND);
    }
}
