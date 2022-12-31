/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#include "nanoem/ext/converter.h"
#include "nanoem/ext/document.h"
#include "nanoem/nanoem.h"

#include "nanodxm/nanodxm.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

NANOEM_DECL_API nanoem_unicode_string_factory_t *APIENTRY nanoemUnicodeStringFactoryCreateEXT(nanoem_status_t *status);
NANOEM_DECL_API void APIENTRY nanoemUnicodeStringFactoryDestroyEXT(nanoem_unicode_string_factory_t *factory);

static const char *
stringify_nanoem_status(nanoem_status_t code)
{
    switch (code) {
    case NANOEM_STATUS_SUCCESS:
        return "NANOEM_STATUS_SUCCESS";
    case NANOEM_STATUS_ERROR_MALLOC_FAILED:
        return "NANOEM_STATUS_ERROR_MALLOC_FAILED";
    case NANOEM_STATUS_ERROR_REALLOC_FAILED:
        return "NANOEM_STATUS_ERROR_REALLOC_FAILED";
    case NANOEM_STATUS_ERROR_NULL_OBJECT:
        return "NANOEM_STATUS_ERROR_NULL_OBJECT";
    case NANOEM_STATUS_ERROR_BUFFER_END:
        return "NANOEM_STATUS_ERROR_BUFFER_END";
    case NANOEM_STATUS_ERROR_DECODE_UNICODE_STRING_FAILED:
        return "NANOEM_STATUS_ERROR_DECODE_UNICODE_STRING_FAILED";
    case NANOEM_STATUS_ERROR_ENCODE_UNICODE_STRING_FAILED:
        return "NANOEM_STATUS_ERROR_ENCODE_UNICODE_STRING_FAILED";
    case NANOEM_STATUS_ERROR_INVALID_SIGNATURE:
        return "NANOEM_STATUS_ERROR_INVALID_SIGNATURE";
    case NANOEM_STATUS_ERROR_MODEL_VERTEX_CORRUPTED:
        return "NANOEM_STATUS_ERROR_MODEL_VERTEX_CORRUPTED";
    case NANOEM_STATUS_ERROR_MODEL_FACE_CORRUPTED:
        return "NANOEM_STATUS_ERROR_FACE_CORRUPTED";
    case NANOEM_STATUS_ERROR_MODEL_MATERIAL_CORRUPTED:
        return "NANOEM_STATUS_ERROR_MODEL_MATERIAL_CORRUPTED";
    case NANOEM_STATUS_ERROR_MODEL_BONE_CORRUPTED:
        return "NANOEM_STATUS_ERROR_MODEL_BONE_CORRUPTED";
    case NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_CORRUPTED:
        return "NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_CORRUPTED";
    case NANOEM_STATUS_ERROR_MODEL_TEXTURE_CORRUPTED:
        return "NANOEM_STATUS_ERROR_MODEL_TEXTURE_CORRUPTED";
    case NANOEM_STATUS_ERROR_MODEL_MORPH_CORRUPTED:
        return "NANOEM_STATUS_ERROR_MODEL_MORPH_CORRUPTED";
    case NANOEM_STATUS_ERROR_MODEL_LABEL_CORRUPTED:
        return "NANOEM_STATUS_ERROR_MODEL_LABEL_CORRUPTED";
    case NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_CORRUPTED:
        return "NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_CORRUPTED";
    case NANOEM_STATUS_ERROR_MODEL_JOINT_CORRUPTED:
        return "NANOEM_STATUS_ERROR_MODEL_JOINT_CORRUPTED";
    case NANOEM_STATUS_ERROR_PMD_ENGLISH_CORRUPTED:
        return "NANOEM_STATUS_ERROR_PMD_ENGLISH_CORRUPTED";
    case NANOEM_STATUS_ERROR_PMX_INFO_CORRUPUTED:
        return "NANOEM_STATUS_ERROR_PMX_INFO_CORRUPUTED";
    case NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_CORRUPTED:
        return "NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_CORRUPTED";
    case NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_CORRUPTED:
        return "NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_CORRUPTED";
    case NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_CORRUPTED:
        return "NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_CORRUPTED";
    case NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_CORRUPTED:
        return "NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_CORRUPTED";
    case NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_CORRUPTED:
        return "NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_CORRUPTED";
    case NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_CORRUPTED:
        return "NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_CORRUPTED";
    default:
        return "(unknown)";
    }
}

static const char *
stringify_nanoem_mutable_status(nanoem_status_t code)
{
    switch (code) {
    case NANOEM_STATUS_UNKNOWN:
        return "NANOEM_STATUS_UNKNOWN";
    case NANOEM_STATUS_SUCCESS:
        return "NANOEM_STATUS_SUCCESS";
    case NANOEM_STATUS_ERROR_MALLOC_FAILED:
        return "NANOEM_STATUS_ERROR_MALLOC_FAILED";
    case NANOEM_STATUS_ERROR_REALLOC_FAILED:
        return "NANOEM_STATUS_ERROR_REALLOC_FAILED";
    case NANOEM_STATUS_ERROR_NULL_OBJECT:
        return "NANOEM_STATUS_ERROR_NULL_OBJECT";
    case NANOEM_STATUS_ERROR_BUFFER_END:
        return "NANOEM_STATUS_ERROR_BUFFER_END";
    case NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_REFERENCE:
        return "NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_REFERENCE";
    case NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_REFERENCE:
        return "NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_REFERENCE";
    case NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_REFERENCE:
        return "NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_REFERENCE";
    case NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_REFERENCE:
        return "NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_REFERENCE";
    case NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_REFERENCE:
        return "NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_REFERENCE";
    case NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_REFERENCE:
        return "NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_REFERENCE";
    case NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MOTION_ACCESSORY_KEYFRAME_REFERENCE:
        return "NANOEM_STATUS_ERROR_MOTION_ACCESSORY_KEYFRAME_REFERENCE";
    case NANOEM_STATUS_ERROR_MOTION_ACCESSORY_KEYFRAME_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_MOTION_ACCESSORY_KEYFRAME_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_MOTION_ACCESSORY_KEYFRAME_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MOTION_ACCESSORY_KEYFRAME_NOT_FOUND";
    case NANOEM_STATUS_ERROR_EFFECT_PARAMETER_REFERENCE:
        return "NANOEM_STATUS_ERROR_EFFECT_PARAMETER_REFERENCE";
    case NANOEM_STATUS_ERROR_EFFECT_PARAMETER_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_EFFECT_PARAMETER_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_EFFECT_PARAMETER_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_EFFECT_PARAMETER_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_STATE_REFERENCE:
        return "NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_STATE_REFERENCE";
    case NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_STATE_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_STATE_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_STATE_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_STATE_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_BINDING_REFERENCE:
        return "NANOEM_STATUS_ERROR_MODEL_BINDING_REFERENCE";
    case NANOEM_STATUS_ERROR_MODEL_BINDING_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_MODEL_BINDING_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_MODEL_BINDING_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_BINDING_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_VERTEX_REFERENCE:
        return "NANOEM_STATUS_ERROR_MODEL_VERTEX_REFERENCE";
    case NANOEM_STATUS_ERROR_MODEL_VERTEX_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_MODEL_VERTEX_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_MODEL_VERTEX_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_VERTEX_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_MATERIAL_REFERENCE:
        return "NANOEM_STATUS_ERROR_MODEL_MATERIAL_REFERENCE";
    case NANOEM_STATUS_ERROR_MODEL_MATERIAL_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_MODEL_MATERIAL_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_MODEL_MATERIAL_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_MATERIAL_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_BONE_REFERENCE:
        return "NANOEM_STATUS_ERROR_MODEL_BONE_REFERENCE";
    case NANOEM_STATUS_ERROR_MODEL_BONE_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_MODEL_BONE_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_MODEL_BONE_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_BONE_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_REFERENCE:
        return "NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_REFERENCE";
    case NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_JOINT_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_JOINT_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_TEXTURE_REFERENCE:
        return "NANOEM_STATUS_ERROR_MODEL_TEXTURE_REFERENCE";
    case NANOEM_STATUS_ERROR_MODEL_TEXTURE_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_MODEL_TEXTURE_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_MODEL_TEXTURE_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_TEXTURE_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_MORPH_REFERENCE:
        return "NANOEM_STATUS_ERROR_MODEL_MORPH_REFERENCE";
    case NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_MODEL_MORPH_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_MORPH_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_MORPH_TYPE_MISMATCH:
        return "NANOEM_STATUS_ERROR_MODEL_MORPH_TYPE_MISMATCH";
    case NANOEM_STATUS_ERROR_MODEL_MORPH_BONE_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_MORPH_BONE_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_MORPH_FLIP_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_MORPH_FLIP_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_MORPH_GROUP_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_MORPH_GROUP_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_MORPH_IMPULSE_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_MORPH_IMPULSE_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_MORPH_MATERIAL_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_MORPH_MATERIAL_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_MORPH_UV_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_MORPH_UV_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_MORPH_VERTEX_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_MORPH_VERTEX_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_LABEL_REFERENCE:
        return "NANOEM_STATUS_ERROR_MODEL_LABEL_REFERENCE";
    case NANOEM_STATUS_ERROR_MODEL_LABEL_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_MODEL_LABEL_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_MODEL_LABEL_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_LABEL_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_LABEL_ITEM_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_LABEL_ITEM_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_REFERENCE:
        return "NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_REFERENCE";
    case NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_JOINT_REFERENCE:
        return "NANOEM_STATUS_ERROR_MODEL_JOINT_REFERENCE";
    case NANOEM_STATUS_ERROR_MODEL_JOINT_ALREADY_EXISTS:
        return "NANOEM_STATUS_ERROR_MODEL_JOINT_ALREADY_EXISTS";
    case NANOEM_STATUS_ERROR_MODEL_JOINT_NOT_FOUND:
        return "NANOEM_STATUS_ERROR_MODEL_JOINT_NOT_FOUND";
    case NANOEM_STATUS_ERROR_MODEL_VERSION_INCOMPATIBLE:
        return "NANOEM_STATUS_ERROR_MODEL_VERSION_INCOMPATIBLE";
    case NANOEM_STATUS_MAX_ENUM:
        return "NANOEM_STATUS_ERROR_MAX_ENUM";
    default:
        return "(unknown)";
    }
}

static const char *
stringify_nanodxm_status(nanodxm_status_t code)
{
    switch (code) {
    case NANODXM_STATUS_SUCCESS:
        return "NANODXM_STATUS_SUCCESS";
    case NANODXM_STATUS_ERROR_NULL_BUFFER:
        return "NANODXM_STATUS_ERROR_NULL_BUFFER";
    case NANODXM_STATUS_ERROR_NULL_DOCUMENT:
        return "NANODXM_STATUS_ERROR_NULL_DOCUMENT";
    case NANODXM_STATUS_ERROR_NULL_SCENE:
        return "NANODXM_STATUS_ERROR_NULL_SCENE";
    case NANODXM_STATUS_ERROR_NULL_MATERIAL:
        return "NANODXM_STATUS_ERROR_NULL_MATERIAL";
    case NANODXM_STATUS_ERROR_NULL_OBJECT:
        return "NANODXM_STATUS_ERROR_NULL_OBJECT";
    case NANODXM_STATUS_ERROR_NULL_VERTEX:
        return "NANODXM_STATUS_ERROR_NULL_VERTEX";
    case NANODXM_STATUS_ERROR_NULL_FACE:
        return "NANODXM_STATUS_ERROR_NULL_FACE";
    case NANODXM_STATUS_ERROR_INVALID_SIGNATURE:
        return "NANODXM_STATUS_ERROR_INVALID_SIGNATURE";
    case NANODXM_STATUS_ERROR_INVALID_VERSION:
        return "NANODXM_STATUS_ERROR_INVALID_VERSION";
    case NANODXM_STATUS_ERROR_INVALID_DATA_TYPE:
        return "NANODXM_STATUS_ERROR_INVALID_DATA_TYPE";
    case NANODXM_STATUS_ERROR_INVALID_FLOAT_SIZE:
        return "NANODXM_STATUS_ERROR_INVALID_FLOAT_SIZE";
    case NANODXM_STATUS_ERROR_INVALID_TOKEN:
        return "NANODXM_STATUS_ERROR_INVALID_TOKEN";
    case NANODXM_STATUS_ERROR_INVALID_EOF:
        return "NANODXM_STATUS_ERROR_INVALID_EOF";
    case NANODXM_STATUS_ERROR_NOT_SUPPORTED_DATA_TYPE:
        return "NANODXM_STATUS_ERROR_NOT_SUPPORTED_DATA_TYPE";
    case NANODXM_STATUS_ERROR_NOT_SUPPORTED_TOKEN:
        return "NANODXM_STATUS_ERROR_NOT_SUPPORTED_TOKEN";
    case NANODXM_STATUS_ERROR_UNKNOWN_CHUNK_TYPE:
        return "NANODXM_STATUS_ERROR_UNKNOWN_CHUNK_TYPE";
    default:
        return "(unknown)";
    }
}

static bool
slurpFile(const char *path, nanoem_u8_t *&data, size_t &size)
{
    bool result = false;
    if (FILE *fp = fopen(path, "rb")) {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = new nanoem_u8_t[size];
        fread(data, 1, size, fp);
        fclose(fp);
        result = true;
    }
    return result;
}

static void
validateModel(nanoem_unicode_string_factory_t *factory, const char *path)
{
    nanoem_u8_t *data = 0;
    size_t size = 0;
    if (slurpFile(path, data, size)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_model_t *model = nanoemModelCreate(factory, &status);
        nanoem_buffer_t *buffer = nanoemBufferCreate(data, size, &status);
        if (nanoemModelLoadFromBuffer(model, buffer, &status)) {
            fprintf(stdout, "[OK] %s loaded successfully\n", path);
        }
        else {
            fprintf(stdout, "[NG] %s cannot be loaded: %s\n", path, stringify_nanoem_status(status));
        }
        if (const char *p = strrchr(path, '.')) {
            if (strcmp(p, ".pmd") == 0) {
                nanoem_model_converter_t *converter = nanoemModelConverterCreate(model, &status);
                nanoem_status_t mutable_status;
                nanoem_mutable_model_t *converted =
                    nanoemModelConverterExecute(converter, NANOEM_MODEL_FORMAT_TYPE_PMX_2_0, &mutable_status);
                if (mutable_status == NANOEM_STATUS_SUCCESS) {
                    fprintf(stdout, "[OK] %s loaded successfully\n", path);
                }
                else {
                    fprintf(stdout, "[NG] %s cannot be converted: %s\n", path,
                        stringify_nanoem_mutable_status(mutable_status));
                }
                nanoemMutableModelDestroy(converted);
                nanoemModelConverterDestroy(converter);
            }
        }
        nanoemBufferDestroy(buffer);
        nanoemModelDestroy(model);
        delete[] data;
    }
    else {
        fprintf(stdout, "[NG] %s cannot be opened: %s\n", path, strerror(errno));
    }
}

static void
validateMotion(nanoem_unicode_string_factory_t *factory, const char *path)
{
    nanoem_u8_t *data = 0;
    size_t size = 0;
    if (slurpFile(path, data, size)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_motion_t *motion = nanoemMotionCreate(factory, &status);
        nanoem_buffer_t *buffer = nanoemBufferCreate(data, size, &status);
        if (nanoemMotionLoadFromBuffer(motion, buffer, 0, &status)) {
            fprintf(stdout, "[OK] %s loaded successfully\n", path);
        }
        else {
            fprintf(stdout, "[NG] %s cannot be loaded: %s\n", path, stringify_nanoem_status(status));
        }
        nanoemBufferDestroy(buffer);
        nanoemMotionDestroy(motion);
        delete[] data;
    }
    else {
        fprintf(stdout, "[NG] %s cannot be opened: %s\n", path, strerror(errno));
    }
}

static void
validateDocument(nanoem_unicode_string_factory_t *factory, const char *path)
{
    nanoem_u8_t *data = 0;
    size_t size = 0;
    if (slurpFile(path, data, size)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_document_t *document = nanoemDocumentCreate(factory, &status);
        nanoem_buffer_t *buffer = nanoemBufferCreate(data, size, &status);
        if (nanoemDocumentLoadFromBuffer(document, buffer, &status)) {
            fprintf(stdout, "[OK] %s loaded successfully\n", path);
        }
        else {
            fprintf(stdout, "[NG] %s cannot be loaded: %s\n", path, stringify_nanoem_status(status));
        }
        nanoemBufferDestroy(buffer);
        nanoemDocumentDestroy(document);
        delete[] data;
    }
    else {
        fprintf(stdout, "[NG] %s cannot be opened: %s\n", path, strerror(errno));
    }
}

static void
validateAccessory(const char *path)
{
    nanoem_u8_t *data = 0;
    size_t size = 0;
    if (slurpFile(path, data, size)) {
        nanodxm_document_t *document = nanodxmDocumentCreate();
        nanodxm_buffer_t *buffer = nanodxmBufferCreate(data, size);
        nanodxm_status_t status = nanodxmDocumentParse(document, buffer);
        if (status == NANODXM_STATUS_SUCCESS) {
            fprintf(stdout, "[OK] %s loaded successfully\n", path);
        }
        else {
            fprintf(stdout, "[NG] %s cannot be loaded: %s\n", path, stringify_nanodxm_status(status));
        }
        nanodxmBufferDestroy(buffer);
        nanodxmDocumentDestroy(document);
        delete[] data;
    }
    else {
        fprintf(stdout, "[NG] %s cannot be opened: %s\n", path, strerror(errno));
    }
}

static void
processFile(nanoem_unicode_string_factory_t *factory, const char *path)
{
    if (const char *p = strrchr(path, '.')) {
        if (strcmp(p, ".pmd") == 0 || strcmp(p, ".pmx") == 0) {
            validateModel(factory, path);
        }
        else if (strcmp(p, ".vmd") == 0) {
            validateMotion(factory, path);
        }
        else if (strcmp(p, ".pmm") == 0) {
            validateDocument(factory, path);
        }
        else if (strcmp(p, ".x") == 0) {
            validateAccessory(path);
        }
    }
}

static void
validateAllFiles(nanoem_unicode_string_factory_t *factory, const char *path)
{
    char buffer[1024];
    if (FILE *fp = fopen(path, "rb")) {
        while (!feof(fp)) {
            fgets(buffer, sizeof(buffer), fp);
            buffer[strcspn(buffer, "\r\n")] = 0;
            processFile(factory, buffer);
        }
        fclose(fp);
    }
}

static void
parsePortableFloatMap(const char *path)
{
    nanoem_u8_t *data = 0;
    size_t size = 0;
    if (slurpFile(path, data, size)) {
        int width, height, offset, count;
        float scaleFactor;
        if ((count = sscanf(reinterpret_cast<const char *>(data), "PF\n%d %d\n%f\n%n", &width, &height, &scaleFactor,
                 &offset)) == 3 &&
            size_t(width * height * sizeof(float) * 3) == size - offset) {
            fprintf(stderr, "OK:PFM\n");
        }
        else if ((count = sscanf(reinterpret_cast<const char *>(data), "Pf\n%d %d\n%f\n%n", &width, &height,
                      &scaleFactor, &offset)) == 3 &&
            size_t(width * height * sizeof(float)) == size - offset) {
            fprintf(stderr, "OK:Pfm\n");
        }
        delete[] data;
    }
}

int
main(int argc, char *argv[])
{
    if (argc > 1) {
        const char *path = argv[1];
        if (const char *p = strrchr(path, '.')) {
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            nanoem_unicode_string_factory_t *factory = nanoemUnicodeStringFactoryCreateEXT(&status);
            if (strcmp(p, ".txt") == 0) {
                validateAllFiles(factory, path);
            }
            else if (strcmp(p, ".pfm") == 0) {
                parsePortableFloatMap(path);
            }
            else {
                processFile(factory, path);
            }
            nanoemUnicodeStringFactoryDestroyEXT(factory);
        }
    }
    else {
        fprintf(stdout, "usage: %s [model_list_path] [motion_list_path]\n", argv[0]);
    }
    return 0;
}
