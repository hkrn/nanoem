/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "nanoem/ext/mutable.h"
#include "nanoem/nanoem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unordered_set>
#include <vector>

NANOEM_DECL_API nanoem_unicode_string_factory_t *APIENTRY nanoemUnicodeStringFactoryCreateEXT(nanoem_status_t *status);
NANOEM_DECL_API void APIENTRY nanoemUnicodeStringFactoryDestroyEXT(nanoem_unicode_string_factory_t *factory);

namespace {

typedef float btScalar;
typedef glm::vec3 Vector3;

#include "../dependencies/bullet3/Demos/GimpactTestDemo/BunnyMesh.h"
#include "../dependencies/bullet3/Demos/GimpactTestDemo/TorusMesh.h"

struct UnicodeString {
    UnicodeString(nanoem_unicode_string_factory_t *factory, const char *value, nanoem_status_t *status)
        : m_factory(factory)
        , m_value(nanoemUnicodeStringFactoryCreateString(
              factory, reinterpret_cast<const nanoem_u8_t *>(value), strlen(value), status))
    {
    }
    ~UnicodeString()
    {
        nanoemUnicodeStringFactoryDestroyString(m_factory, m_value);
        m_factory = 0;
        m_value = 0;
    }
    nanoem_unicode_string_factory_t *m_factory;
    nanoem_unicode_string_t *m_value;
};

using Callback = void (*)(nanoem_mutable_model_soft_body_t *body);

static nanoem_mutable_buffer_t *
generateBunnyModel(nanoem_unicode_string_factory_t *factory, nanoem_status_t *status, Callback callback)
{
    nanoem_mutable_buffer_t *buffer = nanoemMutableBufferCreate(status);
    nanoem_mutable_model_t *model = nanoemMutableModelCreate(factory, status);
    nanoem_model_t *origin = nanoemMutableModelGetOriginObject(model);
    nanoem_rsize_t num_vertices = BUNNY_NUM_VERTICES;
    nanoemMutableModelSetFormatType(model, NANOEM_MODEL_FORMAT_TYPE_PMX_2_1);
    const nanoem_u32_t *indices = reinterpret_cast<nanoem_u32_t *>(gIndicesBunny);
    nanoem_rsize_t num_indices = BUNNY_NUM_INDICES;
    const nanoem_f32_t translation_offset[] = { 0, 6, 0 };
    const nanoem_f32_t scale_factor = 6;
    std::vector<std::vector<Vector3>> faceNormalList;
    faceNormalList.resize(BUNNY_NUM_VERTICES);
    for (nanoem_rsize_t i = 0; i < BUNNY_NUM_TRIANGLES; i++) {
        int index0 = gIndicesBunny[i][0], index1 = gIndicesBunny[i][1], index2 = gIndicesBunny[i][2];
        const Vector3 o0(glm::make_vec3(&gVerticesBunny[index0 * 3])), o1(glm::make_vec3(&gVerticesBunny[index1 * 3])),
            o2(glm::make_vec3(&gVerticesBunny[index2 * 3]));
        const Vector3 normal(glm::normalize(glm::cross(o1 - o0, o2 - o1)));
        faceNormalList[index0].push_back(normal);
        faceNormalList[index1].push_back(normal);
        faceNormalList[index2].push_back(normal);
    }
    nanoem_mutable_model_bone_t *bone = nanoemMutableModelBoneCreate(origin, status);
    {
        nanoemMutableModelInsertBoneObject(model, bone, -1, status);
        const char bone_name[] = "Bone1";
        for (int i = NANOEM_LANGUAGE_TYPE_FIRST_ENUM; i < NANOEM_LANGUAGE_TYPE_MAX_ENUM; i++) {
            UnicodeString us(factory, bone_name, status);
            nanoemMutableModelBoneSetName(bone, us.m_value, static_cast<nanoem_language_type_t>(i), status);
        }
    }
    for (nanoem_rsize_t i = 0; i < num_vertices; i++) {
        nanoem_rsize_t offset = i * 3;
        const nanoem_f32_t pos[] = {
            gVerticesBunny[offset + 0] * scale_factor + translation_offset[0],
            gVerticesBunny[offset + 1] * scale_factor + translation_offset[1],
            gVerticesBunny[offset + 2] * scale_factor + translation_offset[2],
            1,
        };
        Vector3 sum(0);
        for (const Vector3 &normal : faceNormalList[i]) {
            sum += normal;
        }
        nanoem_mutable_model_vertex_t *vertex = nanoemMutableModelVertexCreate(origin, status);
        nanoemMutableModelVertexSetOrigin(vertex, pos);
        nanoemMutableModelVertexSetNormal(vertex, glm::value_ptr(glm::vec4(glm::normalize(sum), 0)));
        nanoemMutableModelVertexSetType(vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
        nanoemMutableModelVertexSetBoneObject(vertex, nanoemMutableModelBoneGetOriginObject(bone), 0);
        nanoemMutableModelInsertVertexObject(model, vertex, -1, status);
        nanoemMutableModelVertexDestroy(vertex);
    }
    const char model_name[] = "SoftBodyDemo";
    for (int i = NANOEM_LANGUAGE_TYPE_FIRST_ENUM; i < NANOEM_LANGUAGE_TYPE_MAX_ENUM; i++) {
        UnicodeString us(factory, model_name, status);
        nanoemMutableModelSetName(model, us.m_value, static_cast<nanoem_language_type_t>(i), status);
    }
    const char model_comment_name[] = "SoftBody Demo Model with PMX 2.1";
    for (int i = NANOEM_LANGUAGE_TYPE_FIRST_ENUM; i < NANOEM_LANGUAGE_TYPE_MAX_ENUM; i++) {
        UnicodeString us(factory, model_comment_name, status);
        nanoemMutableModelSetComment(model, us.m_value, static_cast<nanoem_language_type_t>(i), status);
    }
    nanoemMutableModelSetVertexIndices(model, indices, num_indices, status);
    nanoem_mutable_model_material_t *material = nanoemMutableModelMaterialCreate(origin, status);
    {
        nanoemMutableModelInsertMaterialObject(model, material, -1, status);
        const char material_name[] = "Material1";
        for (int i = NANOEM_LANGUAGE_TYPE_FIRST_ENUM; i < NANOEM_LANGUAGE_TYPE_MAX_ENUM; i++) {
            UnicodeString us(factory, material_name, status);
            nanoemMutableModelMaterialSetName(material, us.m_value, static_cast<nanoem_language_type_t>(i), status);
        }
        const nanoem_f32_t ambient_color[] = { 0.25, 0.25, 0.25, 1 };
        nanoemMutableModelMaterialSetAmbientColor(material, ambient_color);
        const nanoem_f32_t diffuse_color[] = { 0.25, 0.25, 0.5, 1 };
        nanoemMutableModelMaterialSetDiffuseColor(material, diffuse_color);
        nanoemMutableModelMaterialSetDiffuseOpacity(material, 1);
        nanoemMutableModelMaterialSetNumVertexIndices(material, num_indices);
    }
    nanoem_mutable_model_soft_body_t *soft_body = nanoemMutableModelSoftBodyCreate(origin, status);
    nanoemMutableModelInsertSoftBodyObject(model, soft_body, -1, status);
    nanoemMutableModelSoftBodySetMaterialObject(soft_body, nanoemMutableModelMaterialGetOriginObject(material));
    nanoemMutableModelSoftBodySetCollisionMask(soft_body, 0xffff);
    const char soft_body_name[] = "SoftBody1";
    for (int i = NANOEM_LANGUAGE_TYPE_FIRST_ENUM; i < NANOEM_LANGUAGE_TYPE_MAX_ENUM; i++) {
        UnicodeString us(factory, soft_body_name, status);
        nanoemMutableModelSoftBodySetName(soft_body, us.m_value, static_cast<nanoem_language_type_t>(i), status);
    }
    callback(soft_body);
    nanoemMutableModelSoftBodyDestroy(soft_body);
    nanoemMutableModelMaterialDestroy(material);
    nanoemMutableModelBoneDestroy(bone);
    nanoemMutableModelSaveToBuffer(model, buffer, status);
    nanoemMutableModelDestroy(model);
    return buffer;
}

static void
generateAllBunnyModels(nanoem_unicode_string_factory_t *factory, const char *output_directory)
{
    char output_path[256];
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    auto write_file = [&status](const char *output_path, nanoem_mutable_buffer_t *mutable_buffer) {
        if (FILE *fp = fopen(output_path, "wb")) {
            nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mutable_buffer, &status);
            fwrite(nanoemBufferGetDataPtr(buffer), nanoemBufferGetLength(buffer), 1, fp);
            nanoemBufferDestroy(buffer);
            fclose(fp);
        }
        nanoemMutableBufferDestroy(mutable_buffer);
    };
    {
        snprintf(output_path, sizeof(output_path), "%s/bunny.pmx", output_directory);
        nanoem_mutable_buffer_t *mutable_buffer =
            generateBunnyModel(factory, &status, [](nanoem_mutable_model_soft_body_t *body) {
                nanoemMutableModelSoftBodySetLinearStiffnessCoefficient(body, 0.5f); // kLST
                nanoemMutableModelSoftBodySetBendingConstraintsEnabled(body, true);
                nanoemMutableModelSoftBodySetBendingConstraintsDistance(body, 2);
                nanoemMutableModelSoftBodySetPositionsSolverIterations(body, 2); // piteration
                nanoemMutableModelSoftBodySetDynamicFrictionCoefficient(body, 0.5f); // kDF
                nanoemMutableModelSoftBodySetRandomizeConstraintsNeeded(body, true);
                nanoemMutableModelSoftBodySetTotalMass(body, 100);
            });
        write_file(output_path, mutable_buffer);
    }
    {
        snprintf(output_path, sizeof(output_path), "%s/bunny_pm.pmx", output_directory);
        nanoem_mutable_buffer_t *mutable_buffer =
            generateBunnyModel(factory, &status, [](nanoem_mutable_model_soft_body_t *body) {
                nanoemMutableModelSoftBodySetDynamicFrictionCoefficient(body, 0.5f); // kDF
                nanoemMutableModelSoftBodySetPoseMatchingCoefficient(body, 0.05f); // kMT
                nanoemMutableModelSoftBodySetPositionsSolverIterations(body, 5); // piteration
                nanoemMutableModelSoftBodySetRandomizeConstraintsNeeded(body, true);
                nanoemMutableModelSoftBodySetTotalMass(body, 100);
            });
        write_file(output_path, mutable_buffer);
    }
    {
        snprintf(output_path, sizeof(output_path), "%s/bunny_cluster.pmx", output_directory);
        nanoem_mutable_buffer_t *mutable_buffer =
            generateBunnyModel(factory, &status, [](nanoem_mutable_model_soft_body_t *body) {
                nanoemMutableModelSoftBodySetLinearStiffnessCoefficient(body, 1); // kLST
                nanoemMutableModelSoftBodySetBendingConstraintsEnabled(body, true);
                nanoemMutableModelSoftBodySetBendingConstraintsDistance(body, 2);
                nanoemMutableModelSoftBodySetPositionsSolverIterations(body, 2); // piteration
                nanoemMutableModelSoftBodySetDynamicFrictionCoefficient(body, 1); // kDF
                nanoemMutableModelSoftBodySetRandomizeConstraintsNeeded(body, true);
                nanoemMutableModelSoftBodySetClustersEnabled(body, true);
                nanoemMutableModelSoftBodySetClusterCount(body, 1);
                nanoemMutableModelSoftBodySetTotalMass(body, 150);
            });
        write_file(output_path, mutable_buffer);
    }
    fprintf(stderr, "%s: %d\n", output_path, status);
}

} /* namespace anonymous */

int
main(int argc, char **argv)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = nanoemUnicodeStringFactoryCreateEXT(&status);
    if (argc > 1) {
        const char *output_directory = argv[1];
        generateAllBunnyModels(factory, output_directory);
    }
    else {
        fprintf(stderr, "Usage: %s [output]\n", argv[0]);
    }
    nanoemUnicodeStringFactoryDestroyEXT(factory);
    return 0;
}
