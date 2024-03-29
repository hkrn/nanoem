/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#include "nanoem/ext/mutable.h"
#include "nanoem/nanoem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

NANOEM_DECL_API nanoem_unicode_string_factory_t *APIENTRY nanoemUnicodeStringFactoryCreateEXT(nanoem_status_t *status);
NANOEM_DECL_API void APIENTRY nanoemUnicodeStringFactoryDestroyEXT(nanoem_unicode_string_factory_t *factory);

namespace {

static void
generateRainbowModel(nanoem_unicode_string_factory_t *factory)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_t *model = nanoemMutableModelCreate(factory, &status);
    nanoem_model_t *origin = nanoemMutableModelGetOriginObject(model);
    nanoem_mutable_buffer_t *buffer = nanoemMutableBufferCreate(&status);
    nanoem_u8_t name_buffer[1024];
    char *name_ptr = reinterpret_cast<char *>(name_buffer);
    int step = 5, total = step * step * step * step;
    for (int i = 0; i <= total; i++) {
        snprintf(name_ptr, sizeof(name_buffer), "Material%d", i);
        nanoem_mutable_model_material_t *material = nanoemMutableModelMaterialCreate(origin, &status);
        const nanoem_f32_t a = nanoem_f32_t((i / (step * step * step)) % step) / step,
                           b = nanoem_f32_t((i / (step * step)) % step) / step,
                           g = nanoem_f32_t((i / step) % step) / step, r = nanoem_f32_t(i % step) / step;
        const nanoem_f32_t full[] = { r, g, b, a };
        const nanoem_f32_t half[] = { r * 0.5f, g * 0.5f, b * 0.5f, a * 0.5f };
        const nanoem_f32_t zero[] = { 0, 0, 0, 0 };
        nanoemMutableModelMaterialSetAmbientColor(material, half);
        nanoemMutableModelMaterialSetDiffuseColor(material, zero);
        nanoemMutableModelMaterialSetSpecularColor(material, full);
        nanoemMutableModelMaterialSetSpecularPower(material, 10.0f);
        nanoemMutableModelMaterialSetNumVertexIndices(material, 3);
        nanoem_unicode_string_t *s =
            nanoemUnicodeStringFactoryCreateString(factory, name_buffer, strlen(name_ptr), &status);
        nanoemMutableModelMaterialSetName(material, s, NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
        nanoemUnicodeStringFactoryDestroyString(factory, s);
        nanoemMutableModelInsertMaterialObject(model, material, -1, &status);
        nanoemMutableModelMaterialDestroy(material);
        nanoem_mutable_model_vertex_t *vertex = nanoemMutableModelVertexCreate(origin, &status);
        const nanoem_f32_t origin[] = { r, g, b, a };
        nanoemMutableModelVertexSetOrigin(vertex, origin);
        nanoemMutableModelInsertVertexObject(model, vertex, -1, &status);
        nanoemMutableModelVertexDestroy(vertex);
    }
    nanoem_mutable_model_bone_t *bone = nanoemMutableModelBoneCreate(origin, &status);
    nanoemMutableModelInsertBoneObject(model, bone, -1, &status);
    nanoemMutableModelBoneDestroy(bone);
    nanoem_rsize_t length = 3 * total;
    nanoem_u32_t *indices = new nanoem_u32_t[length];
    for (nanoem_rsize_t i = 0; i < length; i++) {
        indices[i] = nanoem_u32_t(i / 3);
    }
    nanoemMutableModelSetVertexIndices(model, indices, length, &status);
    snprintf(reinterpret_cast<char *>(name_buffer), sizeof(name_buffer), "Test Model generated by %s", __FILE__);
    nanoem_unicode_string_t *s =
        nanoemUnicodeStringFactoryCreateString(factory, name_buffer, strlen(name_ptr), &status);
    nanoemMutableModelSetName(model, s, NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    nanoemMutableModelSetComment(model, s, NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    nanoemUnicodeStringFactoryDestroyString(factory, s);
    nanoemMutableModelSaveToBuffer(model, buffer, &status);
    FILE *fp = fopen("model.pmx", "wb");
    nanoem_buffer_t *output = nanoemMutableBufferCreateBufferObject(buffer, &status);
    fwrite(nanoemBufferGetDataPtr(output), nanoemBufferGetLength(output), 1, fp);
    fclose(fp);
    nanoemMutableModelDestroy(model);
    nanoemMutableBufferDestroy(buffer);
    nanoemBufferDestroy(output);
}

static void
loadModel(nanoem_unicode_string_factory_t *factory, nanoem_u8_t *data, size_t size, nanoem_status_t *status)
{
    nanoem_model_t *model = nanoemModelCreate(factory, status);
    nanoem_buffer_t *buffer = nanoemBufferCreate(data, size, status);
    nanoemModelLoadFromBuffer(model, buffer, status);
    {
        nanoem_mutable_model_t *new_model = nanoemMutableModelCreateAsReference(model, status);
        nanoem_mutable_buffer_t *mutable_buffer = nanoemMutableBufferCreate(status);
#if 0
        nanoem_global_allocator_t allocator = {};
        allocator.malloc = [](void *, nanoem_rsize_t size, const char *file, int line) {
            fprintf(stdout, "[malloc] %ld bytes at %s:%d\n", size, file, line);
            return ::malloc(size);
        };
        allocator.calloc = [](void *, nanoem_rsize_t count, nanoem_rsize_t size, const char *file, int line) {
            fprintf(stdout, "[calloc] %ld bytes at %s:%d\n", size * count, file, line);
            return ::calloc(count, size);
        };
        allocator.realloc = [](void *, void *ptr, nanoem_rsize_t size, const char *file, int line) {
            fprintf(stdout, "[realloc] %ld bytes at %s:%d\n", size, file, line);
            return ::realloc(ptr, size);
        };
        allocator.free = [](void *, void *ptr, const char *file, int line) {
            fprintf(stdout, "[free] at %s:%d\n", file, line);
            ::free(ptr);
        };
        nanoemGlobalSetCustomAllocator(&allocator);
        nanoemMutableModelSaveToBuffer(new_model, mutable_buffer, &status);
        nanoemGlobalSetCustomAllocator(0);
#else
        nanoemMutableModelSaveToBuffer(new_model, mutable_buffer, status);
#endif
        nanoemMutableBufferDestroy(mutable_buffer);
        nanoemMutableModelDestroy(new_model);
    }
    nanoemBufferDestroy(buffer);
    nanoemModelDestroy(model);
}

static void
loadModelFromFile(nanoem_unicode_string_factory_t *factory, const char *input_path)
{
    nanoem_status_t status;
    if (FILE *fp = fopen(input_path, "rb")) {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        nanoem_u8_t *data = new nanoem_u8_t[size];
        fread(data, size, 1, fp);
        loadModel(factory, data, size, &status);
        delete[] data;
        fclose(fp);
        fprintf(stderr, "%s: %d\n", input_path, status);
    }
}

} /* namespace anonymous */

int
main(int argc, char **argv)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = nanoemUnicodeStringFactoryCreateEXT(&status);
    if (argc > 1) {
        const char *input_path = argv[1];
        if (strstr(input_path, ".txt")) {
            if (FILE *fp = fopen(input_path, "r")) {
                char buffer[1024];
                while (fgets(buffer, sizeof(buffer), fp)) {
                    buffer[strlen(buffer) - 1] = 0;
                    loadModelFromFile(factory, buffer);
                }
                fclose(fp);
            }
        }
        else {
            loadModelFromFile(factory, input_path);
        }
    }
    else {
        generateRainbowModel(factory);
    }
    nanoemUnicodeStringFactoryDestroyEXT(factory);
    return 0;
}
