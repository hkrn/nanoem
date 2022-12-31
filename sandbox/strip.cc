/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#include "nanoem/ext/mutable.h"
#include "nanoem/nanoem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unordered_set>

NANOEM_DECL_API nanoem_unicode_string_factory_t *APIENTRY nanoemUnicodeStringFactoryCreateEXT(nanoem_status_t *status);
NANOEM_DECL_API void APIENTRY nanoemUnicodeStringFactoryDestroyEXT(nanoem_unicode_string_factory_t *factory);

namespace {

static nanoem_mutable_buffer_t *
stripModel(nanoem_unicode_string_factory_t *factory, nanoem_u8_t *data, size_t size, nanoem_status_t *status)
{
    static const unsigned char kJapaneseMaterialName[] = { 0xe6, 0x9d, 0x90, 0xe8, 0xb3, 0xaa, 0x31, 0x0 };
    static const char kEnglishMaterialName[] = "Material1";
    static const float kNormalUpVector[] = { 0, 1, 0, 0 };
    static const float kColorWhite[] = { 1, 1, 1, 1 };
    static const float kColorGreen[] = { 0, 1, 0, 1 };
    static const float kColorRed[] = { 1, 0, 0, 1 };
    nanoem_model_t *input_model = nanoemModelCreate(factory, status);
    nanoem_buffer_t *input_buffer = nanoemBufferCreate(data, size, status);
    nanoem_mutable_buffer_t *output_buffer = nullptr;
    nanoemModelLoadFromBuffer(input_model, input_buffer, status);
    {
        nanoem_mutable_model_t *output_model = nanoemMutableModelCreate(factory, status);
        for (nanoem_rsize_t i = NANOEM_LANGUAGE_TYPE_FIRST_ENUM; i < NANOEM_LANGUAGE_TYPE_MAX_ENUM; i++) {
            nanoem_language_type_t language = static_cast<nanoem_language_type_t>(i);
            nanoemMutableModelSetName(output_model, nanoemModelGetName(input_model, language), language, status);
            nanoemMutableModelSetComment(output_model, nanoemModelGetComment(input_model, language), language, status);
        }
        nanoemMutableModelSetFormatType(output_model, NANOEM_MODEL_FORMAT_TYPE_PMX_2_1);
        nanoemMutableModelSetAdditionalUVSize(output_model, 1);
        nanoem_model_t *origin_model = nanoemMutableModelGetOriginObject(output_model);
        {
            nanoem_rsize_t num_bones;
            nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(input_model, &num_bones);
            for (nanoem_rsize_t i = 0; i < num_bones; i++) {
                const nanoem_model_bone_t *input_bone = bones[i];
                nanoem_mutable_model_bone_t *output_bone = nanoemMutableModelBoneCreate(origin_model, status);
                nanoemMutableModelBoneCopy(output_bone, input_bone, status);
                nanoemMutableModelInsertBoneObject(output_model, output_bone, -1, status);
                nanoemMutableModelBoneDestroy(output_bone);
            }
            nanoem_rsize_t actual_num_vertex_indices = 0, num_constraints, num_joints, new_num_bones;
            nanoem_u32_t *indices = new nanoem_u32_t[num_bones * 3];
            nanoem_model_bone_t *const *new_bones = nanoemModelGetAllBoneObjects(origin_model, &new_num_bones);
            nanoem_model_constraint_t *const *constraints =
                nanoemModelGetAllConstraintObjects(input_model, &num_constraints);
            std::unordered_set<const nanoem_model_bone_t *> bone_set;
            for (nanoem_rsize_t i = 0; i < num_constraints; i++) {
                const nanoem_model_constraint_t *constraint = constraints[i];
                bone_set.insert(nanoemModelConstraintGetEffectorBoneObject(constraint));
                bone_set.insert(nanoemModelConstraintGetTargetBoneObject(constraint));
                nanoem_model_constraint_joint_t *const *joints =
                    nanoemModelConstraintGetAllJointObjects(constraint, &num_joints);
                for (nanoem_rsize_t j = 0; j < num_joints; j++) {
                    bone_set.insert(nanoemModelConstraintJointGetBoneObject(joints[j]));
                }
            }
            bool point = false;
            if (point) {
                for (nanoem_rsize_t i = 0, num_vertices = 0; i < num_bones; i++) {
                    const nanoem_model_bone_t *bone = bones[i];
                    if (bone && nanoemModelBoneIsVisible(bone) && nanoemModelBoneIsUserHandleable(bone) &&
                        bone_set.find(bone) == bone_set.end()) {
                        nanoem_mutable_model_vertex_t *new_vertex =
                            nanoemMutableModelVertexCreate(origin_model, status);
                        nanoemMutableModelVertexSetOrigin(new_vertex, nanoemModelBoneGetOrigin(bone));
                        nanoemMutableModelVertexSetNormal(new_vertex, kNormalUpVector);
                        nanoemMutableModelVertexSetType(new_vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
                        nanoemMutableModelVertexSetBoneObject(new_vertex, new_bones[i], 0);
                        nanoemMutableModelVertexSetAdditionalUV(new_vertex, kColorGreen, 0);
                        nanoemMutableModelInsertVertexObject(output_model, new_vertex, -1, status);
                        nanoemMutableModelVertexDestroy(new_vertex);
                        indices[actual_num_vertex_indices + 0] = num_vertices;
                        indices[actual_num_vertex_indices + 1] = num_vertices;
                        indices[actual_num_vertex_indices + 2] = num_vertices;
                        actual_num_vertex_indices += 3;
                        num_vertices++;
                    }
                }
            }
            else {
                for (nanoem_rsize_t i = 0, num_vertices = 0; i < num_bones; i++) {
                    const nanoem_model_bone_t *bone = bones[i], *parent_bone = nanoemModelBoneGetParentBoneObject(bone);
                    if (bone && parent_bone && nanoemModelBoneIsVisible(bone) &&
                        nanoemModelBoneIsVisible(parent_bone) && nanoemModelBoneIsUserHandleable(bone) &&
                        nanoemModelBoneIsUserHandleable(parent_bone) && bone_set.find(bone) == bone_set.end() &&
                        bone_set.find(parent_bone) == bone_set.end()) {
                        {
                            nanoem_mutable_model_vertex_t *output_vertex =
                                nanoemMutableModelVertexCreate(origin_model, status);
                            nanoemMutableModelVertexSetOrigin(output_vertex, nanoemModelBoneGetOrigin(bone));
                            nanoemMutableModelVertexSetNormal(output_vertex, kNormalUpVector);
                            nanoemMutableModelVertexSetType(output_vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
                            nanoemMutableModelVertexSetBoneObject(output_vertex, new_bones[i], 0);
                            nanoemMutableModelVertexSetAdditionalUV(output_vertex, kColorGreen, 0);
                            nanoemMutableModelInsertVertexObject(output_model, output_vertex, -1, status);
                            nanoemMutableModelVertexDestroy(output_vertex);
                        }
                        {
                            nanoem_mutable_model_vertex_t *output_vertex =
                                nanoemMutableModelVertexCreate(origin_model, status);
                            nanoemMutableModelVertexSetOrigin(output_vertex, nanoemModelBoneGetOrigin(parent_bone));
                            nanoemMutableModelVertexSetNormal(output_vertex, kNormalUpVector);
                            nanoemMutableModelVertexSetType(output_vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
                            nanoemMutableModelVertexSetBoneObject(output_vertex, new_bones[i], 0);
                            nanoemMutableModelVertexSetAdditionalUV(output_vertex, kColorRed, 0);
                            nanoemMutableModelInsertVertexObject(output_model, output_vertex, -1, status);
                            nanoemMutableModelVertexDestroy(output_vertex);
                        }
                        indices[actual_num_vertex_indices + 0] = num_vertices + 0;
                        indices[actual_num_vertex_indices + 1] = num_vertices + 1;
                        indices[actual_num_vertex_indices + 2] = num_vertices + 0;
                        actual_num_vertex_indices += 3;
                        num_vertices += 2;
                    }
                }
            }
            nanoemMutableModelSetVertexIndices(output_model, indices, actual_num_vertex_indices, status);
            delete[] indices;
            {
                nanoem_mutable_model_material_t *output_material =
                    nanoemMutableModelMaterialCreate(origin_model, status);
                if (nanoem_unicode_string_t *s = nanoemUnicodeStringFactoryCreateString(factory, kJapaneseMaterialName,
                        strlen(reinterpret_cast<const char *>(kJapaneseMaterialName)), status)) {
                    nanoemMutableModelMaterialSetName(output_material, s, NANOEM_LANGUAGE_TYPE_JAPANESE, status);
                    nanoemUnicodeStringFactoryDestroyString(factory, s);
                }
                if (nanoem_unicode_string_t *s = nanoemUnicodeStringFactoryCreateString(factory,
                        reinterpret_cast<const nanoem_u8_t *>(kEnglishMaterialName), strlen(kEnglishMaterialName),
                        status)) {
                    nanoemMutableModelMaterialSetName(output_material, s, NANOEM_LANGUAGE_TYPE_ENGLISH, status);
                    nanoemUnicodeStringFactoryDestroyString(factory, s);
                }
                nanoemMutableModelMaterialSetLineDrawEnabled(output_material, point ? 0 : 1);
                nanoemMutableModelMaterialSetPointDrawEnabled(output_material, point ? 1 : 0);
                nanoemMutableModelMaterialSetVertexColorEnabled(output_material, 1);
                nanoemMutableModelMaterialSetAmbientColor(output_material, kColorWhite);
                nanoemMutableModelMaterialSetDiffuseColor(output_material, kColorWhite);
                nanoemMutableModelMaterialSetDiffuseOpacity(output_material, 1);
                nanoemMutableModelMaterialSetNumVertexIndices(output_material, actual_num_vertex_indices);
                nanoemMutableModelInsertMaterialObject(output_model, output_material, -1, status);
                nanoemMutableModelMaterialDestroy(output_material);
            }
        }
        output_buffer = nanoemMutableBufferCreate(status);
        nanoemMutableModelSaveToBuffer(output_model, output_buffer, status);
        nanoemMutableModelDestroy(output_model);
    }
    nanoemBufferDestroy(input_buffer);
    nanoemModelDestroy(input_model);
    return output_buffer;
}

static void
execute(nanoem_unicode_string_factory_t *factory, const char *input_path, const char *output_path)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    if (FILE *rfp = fopen(input_path, "rb")) {
        fseek(rfp, 0, SEEK_END);
        long size = ftell(rfp);
        fseek(rfp, 0, SEEK_SET);
        nanoem_u8_t *data = new nanoem_u8_t[size];
        fread(data, size, 1, rfp);
        nanoem_mutable_buffer_t *mutable_buffer = stripModel(factory, data, size, &status);
        delete[] data;
        fclose(rfp);
        if (FILE *wfp = fopen(output_path, "wb")) {
            nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mutable_buffer, &status);
            fwrite(nanoemBufferGetDataPtr(buffer), nanoemBufferGetLength(buffer), 1, wfp);
            nanoemBufferDestroy(buffer);
            fclose(wfp);
        }
        nanoemMutableBufferDestroy(mutable_buffer);
        fprintf(stderr, "%s: %d\n", input_path, status);
    }
}

} /* namespace anonymous */

int
main(int argc, char **argv)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = nanoemUnicodeStringFactoryCreateEXT(&status);
    if (argc > 2) {
        const char *input_path = argv[1];
        const char *output_path = argv[2];
        execute(factory, input_path, output_path);
    }
    else {
        fprintf(stderr, "Usage: %s [input] [output]\n", argv[0]);
    }
    nanoemUnicodeStringFactoryDestroyEXT(factory);
    return 0;
}
