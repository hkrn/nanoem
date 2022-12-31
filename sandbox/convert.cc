/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#include "nanoem/nanoem_p.h"

#include "../dependencies/nanodxm/nanodxm.h"
#include "nanoem/ext/converter.h"
#include "nanoem/ext/mutable.h"
#include "nanoem/nanoem.h"

NANOEM_DECL_API nanoem_unicode_string_factory_t *APIENTRY nanoemUnicodeStringFactoryCreateEXT(nanoem_status_t *status);
NANOEM_DECL_API void APIENTRY nanoemUnicodeStringFactoryDestroyEXT(nanoem_unicode_string_factory_t *factory);

#define nanoem_unicode_string_hash_equal(a, b) ((a).factory->compare((a).factory->opaque_data, (a).name, (b).name) == 0)
#define nanoem_unicode_string_hash_func(a) ((a).factory->hash((a).factory->opaque_data, (a).name))
struct nanoem_unicode_string_hash_key_t {
    nanoem_unicode_string_factory_t *factory;
    const nanoem_unicode_string_t *name;
};

nanoem_pragma_diagnostics_push();
nanoem_pragma_diagnostics_ignore_clang_gcc("-Wunused-function");
nanoem_pragma_diagnostics_ignore_msvc(4930);
KHASH_INIT(bone_hash, nanoem_unicode_string_hash_key_t, nanoem_model_bone_t *, 1, nanoem_unicode_string_hash_func,
    nanoem_unicode_string_hash_equal);
KHASH_INIT(morph_hash, nanoem_unicode_string_hash_key_t, nanoem_model_morph_t *, 1, nanoem_unicode_string_hash_func,
    nanoem_unicode_string_hash_equal);
nanoem_pragma_diagnostics_pop();

namespace {

#if 0
#define STB_LEAKCHECK_IMPLEMENTATION
#include "../stb/stb_leakcheck.h"
#include <assert.h>

static void *
nanoemTestMalloc(void *opaque, size_t size, const char *filename, int line)
{
    nanoem_mark_unused(opaque);
    return stb_leakcheck_malloc(size, (char *) filename, line);
}

static void *
nanoemTestCalloc(void *opaque, size_t length, size_t size, const char *filename, int line)
{
    void *ptr;
    nanoem_mark_unused(opaque);
    ptr = stb_leakcheck_malloc(size * length, (char *) filename, line);
    memset(ptr, 0, size * length);
    return ptr;
}

static void *
nanoemTestRealloc(void *opaque, void *ptr, size_t size, const char *filename, int line)
{
    nanoem_mark_unused(opaque);
    return stb_leakcheck_realloc(ptr, size, (char *) filename, line);
}

static void
nanoemTestFree(void *opaque, void *ptr, const char *filename, int line)
{
    nanoem_mark_unused(opaque);
    nanoem_mark_unused(filename);
    nanoem_mark_unused(line);
    stb_leakcheck_free(ptr);
}

static const
nanoem_global_allocator_t g_allocator = {
    NULL,
    nanoemTestMalloc,
    nanoemTestCalloc,
    nanoemTestRealloc,
    nanoemTestFree
};

static void
initialize()
{
    nanoemGlobalSetCustomAllocator(&g_allocator);
}

static void
finalize()
{
    nanoemGlobalSetCustomAllocator(NULL);
    stb_leakcheck_dumpmem();
}

#else

static void
initialize()
{
}

static void
finalize()
{
}

#endif

static void
convertDXMesh(
    nanodxm_document_t *document, nanoem_mutable_model_t *mutable_model, nanoem_unicode_string_factory_t *factory)
{
    nanoem_model_t *origin = nanoemMutableModelGetOriginObject(mutable_model);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanodxm_rsize_t num_vertices, num_normals, num_texcoords;
    const nanodxm_vector3_t *vertices = nanodxmDocumentGetVertices(document, &num_vertices);
    const nanodxm_vector3_t *normals = nanodxmDocumentGetNormals(document, &num_normals);
    const nanodxm_texcoord_t *texcoords = nanodxmDocumentGetTexCoords(document, &num_texcoords);
    for (nanodxm_rsize_t i = 0; i < num_vertices; i++) {
        const nanodxm_vector3_t &input_vertex = vertices[i];
        nanodxm_vector3_t output_origin = { input_vertex.x * 10, input_vertex.y * 10, input_vertex.z * 10 };
        nanoem_mutable_model_vertex_t *output_vertex = nanoemMutableModelVertexCreate(origin, &status);
        nanoemMutableModelVertexSetOrigin(output_vertex, &output_origin.x);
        if (normals) {
            const nanodxm_vector3_t &input_normal = normals[i];
            nanoemMutableModelVertexSetNormal(output_vertex, &input_normal.x);
        }
        if (texcoords) {
            const nanodxm_texcoord_t &input_texcoord = texcoords[i];
            nanoemMutableModelVertexSetTexCoord(output_vertex, &input_texcoord.u);
        }
        nanoemMutableModelInsertVertexObject(mutable_model, output_vertex, -1, &status);
        nanoemMutableModelVertexDestroy(output_vertex);
    }
    nanodxm_rsize_t num_materials;
    nanodxm_material_t *const *materials = nanodxmDocumentGetMaterials(document, &num_materials);
    for (nanodxm_rsize_t i = 0; i < num_materials; i++) {
        const nanodxm_material_t *input_material = materials[i];
        nanoem_mutable_model_material_t *output_material = nanoemMutableModelMaterialCreate(origin, &status);
        const nanodxm_color_t &emissive = nanodxmMaterialGetEmissive(input_material);
        nanoemMutableModelMaterialSetAmbientColor(output_material, &emissive.r);
        const nanodxm_color_t &diffuse = nanodxmMaterialGetDiffuse(input_material);
        nanoemMutableModelMaterialSetDiffuseColor(output_material, &diffuse.r);
        const nanodxm_color_t &specular = nanodxmMaterialGetSpecular(input_material);
        nanoemMutableModelMaterialSetSpecularColor(output_material, &specular.r);
        nanoemMutableModelMaterialSetSpecularPower(output_material, nanodxmMaterialGetShininess(input_material));
        if (const nanodxm_uint8_t *path = nanodxmMaterialGetTextureFilename(input_material)) {
            const char *pathPtr = reinterpret_cast<const char *>(path);
            nanoem_mutable_model_texture_t *texture = nanoemMutableModelTextureCreate(origin, &status);
            nanoem_unicode_string_t *utf8_path =
                nanoemUnicodeStringFactoryCreateString(factory, path, strlen(pathPtr), &status);
            nanoemMutableModelTextureSetPath(texture, utf8_path, &status);
            nanoemUnicodeStringFactoryDestroyString(factory, utf8_path);
            const char *extension = strchr(pathPtr, '.');
            if (strcmp(extension, ".sph") == 0) {
                nanoemMutableModelMaterialSetSphereMapTextureObject(output_material, texture, &status);
                nanoemMutableModelMaterialSetSphereMapTextureType(
                    output_material, NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_MULTIPLY);
            }
            else if (strcmp(extension, ".spa") == 0) {
                nanoemMutableModelMaterialSetSphereMapTextureObject(output_material, texture, &status);
                nanoemMutableModelMaterialSetSphereMapTextureType(
                    output_material, NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_ADD);
            }
            else {
                nanoemMutableModelMaterialSetDiffuseTextureObject(output_material, texture, &status);
            }
            nanoemMutableModelTextureDestroy(texture);
        }
        nanoem_u8_t name_buffer[64];
        char *name_ptr = reinterpret_cast<char *>(name_buffer);
        snprintf(name_ptr, sizeof(name_buffer), "Material%lu", i + 1);
        nanoem_unicode_string_t *name_utf8 =
            nanoemUnicodeStringFactoryCreateString(factory, name_buffer, strlen(name_ptr), &status);
        nanoemMutableModelMaterialSetName(output_material, name_utf8, NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
        nanoemMutableModelMaterialSetName(output_material, name_utf8, NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
        nanoemUnicodeStringFactoryDestroyString(factory, name_utf8);
        nanoemMutableModelInsertMaterialObject(mutable_model, output_material, -1, &status);
        nanoemMutableModelMaterialDestroy(output_material);
    }
    nanodxm_rsize_t num_vertex_faces, num_material_indices;
    const nanodxm_face_t *vertex_faces = nanodxmDocumentGetVertexFaces(document, &num_vertex_faces);
    const int *material_indices = nanodxmDocumentGetFaceMaterialIndices(document, &num_material_indices);
    std::vector<std::vector<int>> indices_per_material;
    std::vector<nanoem_u32_t> total_indices;
    indices_per_material.resize(num_materials);
    for (nanoem_rsize_t i = 0; i < num_vertex_faces; i++) {
        const nanodxm_face_t &vertex_face = vertex_faces[i];
        const int *vertex_face_indices = vertex_face.indices;
        int material_index = material_indices[i];
        std::vector<int> *indices = &indices_per_material[material_index];
        if (vertex_face.num_indices == 4) {
            int i0 = vertex_face_indices[0], i1 = vertex_face_indices[1], i2 = vertex_face_indices[2],
                i3 = vertex_face_indices[3];
            indices->push_back(i0);
            indices->push_back(i1);
            indices->push_back(i2);
            indices->push_back(i2);
            indices->push_back(i3);
            indices->push_back(i0);
        }
        else if (vertex_face.num_indices == 3) {
            int i0 = vertex_face_indices[0], i1 = vertex_face_indices[1], i2 = vertex_face_indices[2];
            indices->push_back(i0);
            indices->push_back(i1);
            indices->push_back(i2);
        }
    }
    nanoem_rsize_t num_materials_in_model;
    nanoem_model_material_t *const *model_materials =
        nanoemModelGetAllMaterialObjects(nanoemMutableModelGetOriginObject(mutable_model), &num_materials_in_model);
    for (size_t i = 0; i < indices_per_material.size(); i++) {
        const std::vector<int> &indices = indices_per_material[i];
        for (size_t j = 0; j < indices.size(); j++) {
            total_indices.push_back(indices[j]);
        }
        nanoem_mutable_model_material_t *output_material =
            nanoemMutableModelMaterialCreateAsReference(model_materials[i], &status);
        nanoemMutableModelMaterialSetNumVertexIndices(output_material, indices.size());
        nanoemMutableModelMaterialDestroy(output_material);
    }
    nanoemMutableModelSetVertexIndices(mutable_model, total_indices.data(), total_indices.size(), &status);
    nanoemMutableModelSetFormatType(mutable_model, NANOEM_MODEL_FORMAT_TYPE_PMX_2_0);
    static const nanoem_u8_t kName[] = "test.pmx";
    static const nanoem_u8_t kComment[] = "This is generated by " __FILE__;
    nanoem_unicode_string_t *name_utf8 =
        nanoemUnicodeStringFactoryCreateString(factory, kName, sizeof(kName, &status) - 1, &status);
    nanoemMutableModelSetName(mutable_model, name_utf8, NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    nanoemMutableModelSetName(mutable_model, name_utf8, NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    nanoemUnicodeStringFactoryDestroyString(factory, name_utf8);
    nanoem_unicode_string_t *comment_utf8 =
        nanoemUnicodeStringFactoryCreateString(factory, kComment, sizeof(kComment, &status) - 1, &status);
    nanoemMutableModelSetComment(mutable_model, comment_utf8, NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    nanoemMutableModelSetComment(mutable_model, comment_utf8, NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    nanoemUnicodeStringFactoryDestroyString(factory, comment_utf8);
}

static void
copyConstraint(nanoem_mutable_model_constraint_t *destination_constraint, const nanoem_model_constraint_t *constraint,
    nanoem_unicode_string_factory_t *factory)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelConstraintSetAngleLimit(
        destination_constraint, nanoemModelConstraintGetAngleLimit(constraint) * 4.0f);
    nanoemMutableModelConstraintSetEffectorBoneObject(
        destination_constraint, nanoemModelConstraintGetEffectorBoneObject(constraint));
    nanoemMutableModelConstraintSetNumIterations(
        destination_constraint, nanoemModelConstraintGetNumIterations(constraint));
    nanoemMutableModelConstraintSetTargetBoneObject(
        destination_constraint, nanoemModelConstraintGetTargetBoneObject(constraint));
    nanoem_rsize_t num_joints;
    nanoem_model_constraint_joint_t *const *joints = nanoemModelConstraintGetAllJointObjects(constraint, &num_joints);
    for (nanoem_rsize_t i = 0; i < num_joints; i++) {
        const nanoem_model_constraint_joint_t *joint = joints[i];
        nanoem_mutable_model_constraint_joint_t *destination_joint =
            nanoemMutableModelConstraintJointCreate(destination_constraint, &status);
        nanoemMutableModelConstraintJointSetBoneObject(
            destination_joint, nanoemModelConstraintJointGetBoneObject(joint));
        static const nanoem_u8_t kLeftKnee[] = { 0xe5, 0xb7, 0xa6, 0xe3, 0x81, 0xb2, 0xe3, 0x81, 0x96, 0x0 },
                                 kRightKnee[] = { 0xe5, 0x8f, 0xb3, 0xe3, 0x81, 0xb2, 0xe3, 0x81, 0x96, 0x0 };
        const nanoem_unicode_string_t *name =
            nanoemModelBoneGetName(nanoemModelConstraintJointGetBoneObject(joint), NANOEM_LANGUAGE_TYPE_JAPANESE);
        nanoem_rsize_t length;
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_u8_t *buffer = nanoemUnicodeStringFactoryGetByteArray(factory, name, &length, &status);
        const char *bufferPtr = reinterpret_cast<const char *>(buffer);
        if (strcmp(bufferPtr, reinterpret_cast<const char *>(kLeftKnee)) == 0 ||
            strcmp(bufferPtr, reinterpret_cast<const char *>(kRightKnee)) == 0) {
            static const float kUpperLimit[] = { -0.008726646185809f, 0.0f, 0.0f };
            static const float kLowerLimit[] = { -3.141592626891544f, 0.0f, 0.0f };
            nanoemMutableModelConstraintJointSetLowerLimit(destination_joint, kLowerLimit);
            nanoemMutableModelConstraintJointSetUpperLimit(destination_joint, kUpperLimit);
        }
        else {
            nanoemMutableModelConstraintJointSetLowerLimit(
                destination_joint, nanoemModelConstraintJointGetLowerLimit(joint));
            nanoemMutableModelConstraintJointSetUpperLimit(
                destination_joint, nanoemModelConstraintJointGetUpperLimit(joint));
        }
        nanoemUnicodeStringFactoryDestroyByteArray(factory, buffer);
        nanoemMutableModelConstraintInsertJointObject(destination_constraint, destination_joint, -1, &status);
        nanoemMutableModelConstraintJointDestroy(destination_joint);
    }
}

enum {
    CONVERT_BONES = 1 << 0,
    CONVERT_VERTICES = 1 << 1,
    CONVERT_INDICES = 1 << 2,
    CONVERT_MATERIALS = 1 << 3,
    CONVERT_MORPHS = 1 << 4,
    CONVERT_LABELS = 1 << 5,
    CONVERT_RIGID_BODIES = 1 << 6,
    CONVERT_JOINTS = 1 << 7,
    MAX_ENUM = 1 << 8
};

static void
convertPMXFromPMD(nanoem_mutable_model_t *mutable_model, nanoem_model_t *model,
    nanoem_unicode_string_factory_t *factory, nanoem_model_format_type_t target, int flags)
{
    kh_bone_hash_t *bone_hash = kh_init_bone_hash();
    kh_bone_hash_t *constraint_hash = kh_init_bone_hash();
    kh_morph_hash_t *morph_hash = kh_init_morph_hash();
    nanoem_model_t *origin = nanoemMutableModelGetOriginObject(mutable_model);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelSetFormatType(mutable_model, target);
    nanoemMutableModelSetName(mutable_model, nanoemModelGetName(model, NANOEM_LANGUAGE_TYPE_JAPANESE),
        NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    nanoemMutableModelSetName(
        mutable_model, nanoemModelGetName(model, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    nanoemMutableModelSetComment(mutable_model, nanoemModelGetComment(model, NANOEM_LANGUAGE_TYPE_JAPANESE),
        NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    nanoemMutableModelSetComment(mutable_model, nanoemModelGetComment(model, NANOEM_LANGUAGE_TYPE_ENGLISH),
        NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    if ((flags & CONVERT_BONES) != 0) {
        nanoem_rsize_t num_bones;
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model, &num_bones);
        nanoem_unicode_string_hash_key_t key;
        key.factory = factory;
        for (nanoem_rsize_t i = 0; i < num_bones; i++) {
            nanoem_model_bone_t *source = bones[i];
            nanoem_mutable_model_bone_t *destination = nanoemMutableModelBoneCreate(origin, &status);
            nanoemMutableModelBoneSetName(destination, nanoemModelBoneGetName(source, NANOEM_LANGUAGE_TYPE_JAPANESE),
                NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
            nanoemMutableModelBoneSetName(destination, nanoemModelBoneGetName(source, NANOEM_LANGUAGE_TYPE_ENGLISH),
                NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
            nanoemMutableModelBoneSetAffectedByPhysicsSimulation(destination, nanoem_false);
            nanoemMutableModelBoneSetFixedAxis(destination, nanoemModelBoneGetFixedAxis(source));
            nanoemMutableModelBoneSetLocalXAxis(destination, nanoemModelBoneGetLocalXAxis(source));
            nanoemMutableModelBoneSetLocalZAxis(destination, nanoemModelBoneGetLocalZAxis(source));
            nanoemMutableModelBoneSetMovable(destination, nanoemModelBoneIsMovable(source));
            nanoemMutableModelBoneSetOrigin(destination, nanoemModelBoneGetOrigin(source));
            nanoemMutableModelBoneSetRotateable(destination, nanoemModelBoneIsRotateable(source));
            nanoemMutableModelBoneSetTargetBoneObject(destination, nanoemModelBoneGetTargetBoneObject(source));
            nanoemMutableModelBoneSetUserHandleable(destination, nanoemModelBoneIsUserHandleable(source));
            nanoemMutableModelBoneSetVisible(destination, nanoemModelBoneIsVisible(source));
            nanoemMutableModelInsertBoneObject(mutable_model, destination, -1, &status);
            int ret;
            key.name = nanoemModelBoneGetName(source, NANOEM_LANGUAGE_TYPE_JAPANESE);
            khiter_t it = kh_put_bone_hash(bone_hash, key, &ret);
            kh_val(bone_hash, it) = nanoemMutableModelBoneGetOriginObject(destination);
            nanoemMutableModelBoneDestroy(destination);
        }
        nanoem_rsize_t num_constraints;
        nanoem_model_constraint_t *const *constraints = nanoemModelGetAllConstraintObjects(model, &num_constraints);
        for (nanoem_rsize_t i = 0; i < num_constraints; i++) {
            nanoem_model_constraint_t *constraint = constraints[i];
            key.name = nanoemModelBoneGetName(
                nanoemModelConstraintGetTargetBoneObject(constraint), NANOEM_LANGUAGE_TYPE_JAPANESE);
            khiter_t it = kh_get_bone_hash(bone_hash, key);
            if (it != kh_end(bone_hash)) {
                if (nanoem_model_bone_t *bone = kh_val(bone_hash, it)) {
                    nanoem_mutable_model_constraint_t *destination_constraint =
                        nanoemMutableModelConstraintCreate(origin, &status);
                    nanoem_mutable_model_bone_t *destination_bone = NULL;
                    if (!nanoemModelBoneGetConstraintObject(bone)) {
                        copyConstraint(destination_constraint, constraint, factory);
                        destination_bone = nanoemMutableModelBoneCreateAsReference(bone, &status);
                        nanoemMutableModelBoneSetConstraintObject(destination_bone, destination_constraint);
                    }
                    else {
                        nanoem_u8_t buffer[64];
                        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                        nanoem_rsize_t length;
                        const nanoem_unicode_string_t *name =
                            nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_JAPANESE);
                        nanoem_u8_t *name_buffer =
                            nanoemUnicodeStringFactoryGetByteArray(factory, name, &length, &status);
                        char *bufferPtr = reinterpret_cast<char *>(buffer);
                        snprintf(bufferPtr, sizeof(buffer), "%s+", name_buffer);
                        nanoemUnicodeStringFactoryDestroyByteArray(factory, name_buffer);
                        nanoem_unicode_string_t *new_name =
                            nanoemUnicodeStringFactoryCreateString(factory, buffer, strlen(bufferPtr), &status);
                        copyConstraint(destination_constraint, constraint, factory);
                        destination_bone = nanoemMutableModelBoneCreate(origin, &status);
                        nanoemMutableModelBoneSetName(
                            destination_bone, new_name, NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
                        nanoemUnicodeStringFactoryDestroyString(factory, new_name);
                        nanoemMutableModelBoneSetName(destination_bone,
                            nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH,
                            &status);
                        nanoemMutableModelBoneSetAffectedByPhysicsSimulation(destination_bone, nanoem_false);
                        nanoemMutableModelBoneSetMovable(destination_bone, nanoem_true);
                        nanoemMutableModelBoneSetOrigin(destination_bone, nanoemModelBoneGetOrigin(bone));
                        nanoemMutableModelBoneSetParentBoneObject(destination_bone, bone);
                        nanoemMutableModelBoneSetRotateable(destination_bone, nanoem_true);
                        nanoemMutableModelBoneSetTargetBoneObject(
                            destination_bone, nanoemModelBoneGetTargetBoneObject(bone));
                        nanoemMutableModelBoneSetUserHandleable(destination_bone, nanoem_true);
                        nanoemMutableModelBoneSetVisible(destination_bone, nanoem_false);
                        nanoemMutableModelBoneSetConstraintObject(destination_bone, destination_constraint);
                        nanoemMutableModelInsertBoneObject(mutable_model, destination_bone, -1, &status);
                    }
                    nanoemMutableModelBoneSetStageIndex(destination_bone, 1);
                    nanoemMutableModelBoneDestroy(destination_bone);
                    nanoemMutableModelConstraintDestroy(destination_constraint);
                }
            }
        }
        nanoem_model_bone_t *const *dest_bones =
            nanoemModelGetAllBoneObjects(nanoemMutableModelGetOriginObject(mutable_model), &num_bones);
        for (nanoem_rsize_t i = 0; i < num_bones; i++) {
            nanoem_model_bone_t *source = bones[i];
            nanoem_mutable_model_bone_t *destination = nanoemMutableModelBoneCreateAsReference(dest_bones[i], &status);
            key.name =
                nanoemModelBoneGetName(nanoemModelBoneGetParentBoneObject(source), NANOEM_LANGUAGE_TYPE_JAPANESE);
            khiter_t it = kh_get_bone_hash(bone_hash, key);
            if (it != kh_end(bone_hash)) {
                if (const nanoem_model_bone_t *bone = kh_val(bone_hash, it)) {
                    nanoemMutableModelBoneSetParentBoneObject(destination, bone);
                    int stage_index = nanoemModelBoneGetStageIndex(bone);
                    if (stage_index > 0) {
                        nanoemMutableModelBoneSetStageIndex(destination, stage_index);
                    }
                }
            }
            if (nanoemModelBoneHasInherentOrientation(source)) {
                key.name = nanoemModelBoneGetName(
                    nanoemModelBoneGetInherentParentBoneObject(source), NANOEM_LANGUAGE_TYPE_JAPANESE);
                it = kh_get_bone_hash(bone_hash, key);
                if (it != kh_end(bone_hash)) {
                    if (const nanoem_model_bone_t *bone = kh_val(bone_hash, it)) {
                        nanoemMutableModelBoneSetInherentOrientationEnabled(destination, nanoem_true);
                        nanoemMutableModelBoneSetInherentParentBoneObject(destination, bone);
                        nanoemMutableModelBoneSetStageIndex(
                            destination, nanoemModelBoneHasDestinationBone(source) ? 2 : 0);
                    }
                }
                nanoemMutableModelBoneSetInherentCoefficient(
                    destination, nanoemModelBoneGetInherentCoefficient(source));
            }
            if (nanoemModelBoneHasDestinationBone(source)) {
                key.name =
                    nanoemModelBoneGetName(nanoemModelBoneGetTargetBoneObject(source), NANOEM_LANGUAGE_TYPE_JAPANESE);
                it = kh_get_bone_hash(bone_hash, key);
                if (it != kh_end(bone_hash)) {
                    if (const nanoem_model_bone_t *bone = kh_val(bone_hash, it)) {
                        nanoemMutableModelBoneSetTargetBoneObject(destination, bone);
                    }
                }
            }
        }
    }
    if ((flags & CONVERT_VERTICES) != 0) {
        nanoem_rsize_t num_vertices;
        nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(model, &num_vertices);
        nanoem_unicode_string_hash_key_t key;
        key.factory = factory;
        for (nanoem_rsize_t i = 0; i < num_vertices; i++) {
            nanoem_model_vertex_t *source = vertices[i];
            nanoem_mutable_model_vertex_t *destination = nanoemMutableModelVertexCreate(origin, &status);
            nanoemMutableModelVertexSetOrigin(destination, nanoemModelVertexGetOrigin(source));
            nanoemMutableModelVertexSetNormal(destination, nanoemModelVertexGetNormal(source));
            nanoemMutableModelVertexSetTexCoord(destination, nanoemModelVertexGetTexCoord(source));
            nanoemMutableModelVertexSetEdgeSize(destination, nanoemModelVertexGetEdgeSize(source));
            nanoem_f32_t weight = nanoemModelVertexGetBoneWeight(source, 0);
            if (weight <= FLT_EPSILON || 1.0 - weight <= FLT_EPSILON) {
                nanoemMutableModelVertexSetType(destination, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
            }
            else {
                nanoemMutableModelVertexSetType(destination, NANOEM_MODEL_VERTEX_TYPE_BDEF2);
            }
            if (nanoemModelVertexGetBoneWeight(source, 1) > nanoemModelVertexGetBoneWeight(source, 0)) {
                for (int j = 1; j >= 0; j--) {
                    const nanoem_model_bone_t *bone = nanoemModelVertexGetBoneObject(source, j);
                    key.name = nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_JAPANESE);
                    khiter_t it = kh_get_bone_hash(bone_hash, key);
                    if (it != kh_end(bone_hash)) {
                        if (const nanoem_model_bone_t *bone = kh_val(bone_hash, it)) {
                            nanoemMutableModelVertexSetBoneObject(destination, bone, 1 - j);
                        }
                    }
                    nanoemMutableModelVertexSetBoneWeight(
                        destination, nanoemModelVertexGetBoneWeight(source, j), 1 - j);
                }
            }
            else {
                for (nanoem_rsize_t j = 0; j < 2; j++) {
                    const nanoem_model_bone_t *bone = nanoemModelVertexGetBoneObject(source, j);
                    key.name = nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_JAPANESE);
                    khiter_t it = kh_get_bone_hash(bone_hash, key);
                    if (it != kh_end(bone_hash)) {
                        if (const nanoem_model_bone_t *bone = kh_val(bone_hash, it)) {
                            nanoemMutableModelVertexSetBoneObject(destination, bone, j);
                        }
                    }
                    nanoemMutableModelVertexSetBoneWeight(destination, nanoemModelVertexGetBoneWeight(source, j), j);
                }
            }
            nanoemMutableModelInsertVertexObject(mutable_model, destination, -1, &status);
            nanoemMutableModelVertexDestroy(destination);
        }
    }
    if ((flags & CONVERT_INDICES) != 0) {
        nanoem_rsize_t num_vertex_indices;
        const nanoem_u32_t *vertex_indices = nanoemModelGetAllVertexIndices(model, &num_vertex_indices);
        nanoemMutableModelSetVertexIndices(mutable_model, vertex_indices, num_vertex_indices, &status);
    }
    if ((flags & CONVERT_MATERIALS) != 0) {
        nanoem_rsize_t num_materials;
        nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(model, &num_materials);
        for (nanoem_rsize_t i = 0; i < num_materials; i++) {
            nanoem_model_material_t *source = materials[i];
            nanoem_mutable_model_material_t *destination = nanoemMutableModelMaterialCreate(origin, &status);
            static const unsigned char kMaterialNamePrefix[] = { 0xe6, 0x9d, 0x90, 0xe8, 0xb3, 0xaa, 0x0 };
            nanoem_u8_t buffer[64];
            char *bufferPtr = reinterpret_cast<char *>(buffer);
            snprintf(bufferPtr, sizeof(buffer), "%s%u", kMaterialNamePrefix, (nanoem_u32_t) i + 1);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            nanoem_unicode_string_t *name =
                nanoemUnicodeStringFactoryCreateString(factory, buffer, strlen(bufferPtr), &status);
            nanoemMutableModelMaterialSetName(destination, name, NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
            nanoemUnicodeStringFactoryDestroyString(factory, name);
            nanoemMutableModelMaterialSetAmbientColor(destination, nanoemModelMaterialGetAmbientColor(source));
            nanoemMutableModelMaterialSetDiffuseColor(destination, nanoemModelMaterialGetDiffuseColor(source));
            nanoemMutableModelMaterialSetDiffuseOpacity(destination, nanoemModelMaterialGetDiffuseOpacity(source));
            nanoemMutableModelMaterialSetSpecularColor(destination, nanoemModelMaterialGetSpecularColor(source));
            nanoemMutableModelMaterialSetSpecularPower(destination, nanoemModelMaterialGetSpecularPower(source));
            nanoemMutableModelMaterialSetEdgeColor(destination, nanoemModelMaterialGetEdgeColor(source));
            nanoemMutableModelMaterialSetEdgeOpacity(destination, nanoemModelMaterialGetEdgeOpacity(source));
            nanoemMutableModelMaterialSetEdgeSize(destination, nanoemModelMaterialGetEdgeSize(source));
            if ((flags & CONVERT_VERTICES) != 0 && (flags & CONVERT_INDICES)) {
                nanoemMutableModelMaterialSetNumVertexIndices(
                    destination, nanoemModelMaterialGetNumVertexIndices(source));
            }
            nanoemMutableModelMaterialSetCastingShadowEnabled(
                destination, nanoemModelMaterialIsCastingShadowEnabled(source));
            nanoemMutableModelMaterialSetCastingShadowMapEnabled(
                destination, nanoemModelMaterialIsCastingShadowMapEnabled(source));
            nanoemMutableModelMaterialSetCullingDisabled(destination, nanoemModelMaterialIsCullingDisabled(source));
            nanoemMutableModelMaterialSetEdgeEnabled(destination, nanoemModelMaterialIsEdgeEnabled(source));
            nanoemMutableModelMaterialSetShadowMapEnabled(destination, nanoemModelMaterialIsShadowMapEnabled(source));
            nanoem_mutable_model_texture_t *diffuse_texture = nanoemMutableModelTextureCreate(origin, &status);
            nanoemMutableModelTextureSetPath(diffuse_texture,
                nanoemModelTextureGetPath(nanoemModelMaterialGetDiffuseTextureObject(source)), &status);
            nanoemMutableModelMaterialSetDiffuseTextureObject(destination, diffuse_texture, &status);
            nanoemMutableModelTextureDestroy(diffuse_texture);
            nanoem_mutable_model_texture_t *sphere_texture = nanoemMutableModelTextureCreate(origin, &status);
            nanoemMutableModelTextureSetPath(sphere_texture,
                nanoemModelTextureGetPath(nanoemModelMaterialGetSphereMapTextureObject(source)), &status);
            nanoemMutableModelMaterialSetSphereMapTextureObject(destination, sphere_texture, &status);
            nanoemMutableModelMaterialSetSphereMapTextureType(
                destination, nanoemModelMaterialGetSphereMapTextureType(source));
            nanoemMutableModelTextureDestroy(sphere_texture);
            if (nanoemModelMaterialIsToonShared(source)) {
                nanoemMutableModelMaterialSetToonTextureIndex(
                    destination, nanoemModelMaterialGetToonTextureIndex(source));
                nanoemMutableModelMaterialSetToonShared(destination, nanoem_true);
            }
            else {
                nanoem_mutable_model_texture_t *toon_texture = nanoemMutableModelTextureCreate(origin, &status);
                nanoemMutableModelTextureSetPath(
                    toon_texture, nanoemModelTextureGetPath(nanoemModelMaterialGetToonTextureObject(source)), &status);
                nanoemMutableModelMaterialSetToonTextureObject(destination, toon_texture, &status);
                nanoemMutableModelTextureDestroy(toon_texture);
                nanoemMutableModelMaterialSetToonShared(destination, nanoem_false);
            }
            nanoemMutableModelInsertMaterialObject(mutable_model, destination, -1, &status);
            nanoemMutableModelMaterialDestroy(destination);
        }
    }
    if ((flags & CONVERT_MORPHS) != 0) {
        nanoem_rsize_t num_morphs, num_morph_children;
        nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model, &num_morphs);
        for (nanoem_rsize_t i = 0; i < num_morphs; i++) {
            nanoem_model_morph_t *source = morphs[i];
            nanoem_mutable_model_morph_t *destination = nanoemMutableModelMorphCreate(origin, &status);
            nanoem_model_morph_category_t type = nanoemModelMorphGetCategory(source);
            if (type != NANOEM_MODEL_MORPH_CATEGORY_BASE) {
                nanoemMutableModelMorphSetName(destination,
                    nanoemModelMorphGetName(source, NANOEM_LANGUAGE_TYPE_JAPANESE), NANOEM_LANGUAGE_TYPE_JAPANESE,
                    &status);
                nanoemMutableModelMorphSetName(destination,
                    nanoemModelMorphGetName(source, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH,
                    &status);
                nanoemMutableModelMorphSetCategory(destination, type);
                nanoemMutableModelMorphSetType(destination, nanoemModelMorphGetType(source));
                if ((flags & CONVERT_VERTICES) != 0) {
                    nanoem_model_morph_vertex_t *const *children =
                        nanoemModelMorphGetAllVertexMorphObjects(source, &num_morph_children);
                    for (nanoem_rsize_t j = 0; j < num_morph_children; j++) {
                        nanoem_model_morph_vertex_t *child = children[j];
                        nanoem_mutable_model_morph_vertex_t *v =
                            nanoemMutableModelMorphVertexCreate(destination, &status);
                        nanoemMutableModelMorphVertexSetPosition(v, nanoemModelMorphVertexGetPosition(child));
                        nanoemMutableModelMorphVertexSetVertexObject(v, nanoemModelMorphVertexGetVertexObject(child));
                        nanoemMutableModelMorphInsertVertexMorphObject(destination, v, -1, &status);
                        nanoemMutableModelMorphVertexDestroy(v);
                    }
                }
                nanoemMutableModelInsertMorphObject(mutable_model, destination, -1, &status);
                int ret;
                nanoem_unicode_string_hash_key_t key = { factory,
                    nanoemModelMorphGetName(source, NANOEM_LANGUAGE_TYPE_JAPANESE) };
                khiter_t it = kh_put_morph_hash(morph_hash, key, &ret);
                kh_val(morph_hash, it) = nanoemMutableModelMorphGetOriginObject(destination);
            }
            nanoemMutableModelMorphDestroy(destination);
        }
    }
    if ((flags & CONVERT_LABELS) != 0) {
        nanoem_rsize_t num_bones, num_labels, num_label_items;
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model, &num_bones);
        nanoem_model_label_t *const *labels = nanoemModelGetAllLabelObjects(model, &num_labels);
        for (nanoem_rsize_t i = 0; i < num_labels; i++) {
            nanoem_model_label_t *source = labels[i];
            nanoem_mutable_model_label_t *destination = nanoemMutableModelLabelCreate(origin, &status);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            if (i > 0) {
                nanoem_rsize_t length;
                const nanoem_unicode_string_t *name = nanoemModelLabelGetName(source, NANOEM_LANGUAGE_TYPE_JAPANESE);
                nanoem_u8_t *name_buffer = nanoemUnicodeStringFactoryGetByteArray(factory, name, &length, &status);
                if (name_buffer[length - 1] == '\n') {
                    length--;
                }
                nanoem_unicode_string_t *new_name =
                    nanoemUnicodeStringFactoryCreateString(factory, name_buffer, length, &status);
                nanoemUnicodeStringFactoryDestroyByteArray(factory, name_buffer);
                nanoemMutableModelLabelSetName(destination, new_name, NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
                nanoemUnicodeStringFactoryDestroyString(factory, new_name);
                nanoemMutableModelLabelSetName(destination,
                    nanoemModelLabelGetName(source, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH,
                    &status);
            }
            else if (i == 0) {
                static const nanoem_u8_t kExpressionInJapanese[] = { 0x95, 0x5c, 0x8f, 0xee, 0x0 };
                nanoem_unicode_string_t *new_name = nanoemUnicodeStringFactoryCreateString(factory,
                    kExpressionInJapanese, strlen(reinterpret_cast<const char *>(kExpressionInJapanese)), &status);
                nanoemMutableModelLabelSetName(destination, new_name, NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
                nanoemUnicodeStringFactoryDestroyString(factory, new_name);
                static const nanoem_u8_t kExpressionInEnglish[] = "Exp";
                new_name = nanoemUnicodeStringFactoryCreateString(factory, kExpressionInEnglish,
                    strlen(reinterpret_cast<const char *>(kExpressionInEnglish)), &status);
                nanoemMutableModelLabelSetName(destination, new_name, NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
                nanoemUnicodeStringFactoryDestroyString(factory, new_name);
            }
            nanoemMutableModelLabelSetSpecial(destination, nanoemModelLabelIsSpecial(source));
            nanoem_model_label_item_t *const *items = nanoemModelLabelGetAllItemObjects(source, &num_label_items);
            nanoem_unicode_string_hash_key_t key;
            key.factory = factory;
            for (nanoem_rsize_t j = 0; j < num_label_items; j++) {
                nanoem_model_label_item_t *item = items[j];
                nanoem_mutable_model_label_item_t *v = NULL;
                if (const nanoem_model_bone_t *item_bone = nanoemModelLabelItemGetBoneObject(item)) {
                    key.name = nanoemModelBoneGetName(item_bone, NANOEM_LANGUAGE_TYPE_JAPANESE);
                    khiter_t it = kh_get_bone_hash(bone_hash, key);
                    if (it != kh_end(bone_hash)) {
                        if (nanoem_model_bone_t *bone = kh_val(bone_hash, it)) {
                            v = nanoemMutableModelLabelItemCreateFromBoneObject(destination, bone, &status);
                        }
                    }
                }
                else if (const nanoem_model_morph_t *item_morph = nanoemModelLabelItemGetMorphObject(item)) {
                    key.name = nanoemModelMorphGetName(item_morph, NANOEM_LANGUAGE_TYPE_JAPANESE);
                    khiter_t it = kh_get_morph_hash(morph_hash, key);
                    if (it != kh_end(morph_hash)) {
                        if (nanoem_model_morph_t *morph = kh_val(morph_hash, it)) {
                            v = nanoemMutableModelLabelItemCreateFromMorphObject(destination, morph, &status);
                        }
                    }
                }
                nanoemMutableModelLabelInsertItemObject(destination, v, -1, &status);
                nanoemMutableModelLabelItemDestroy(v);
            }
            nanoemMutableModelInsertLabelObject(mutable_model, destination, -1, &status);
            nanoemMutableModelLabelDestroy(destination);
        }
        if (num_bones > 0) {
            nanoem_u8_t buffer[64];
            char *bufferPtr = reinterpret_cast<char *>(buffer);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            strncpy(bufferPtr, "Root", sizeof(buffer));
            nanoem_mutable_model_label_t *destination = nanoemMutableModelLabelCreate(origin, &status);
            nanoem_unicode_string_t *name =
                nanoemUnicodeStringFactoryCreateString(factory, buffer, strlen(bufferPtr), &status);
            nanoemMutableModelLabelSetName(destination, name, NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
            nanoemMutableModelLabelSetName(destination, name, NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
            nanoemUnicodeStringFactoryDestroyString(factory, name);
            nanoem_mutable_model_label_item_t *v =
                nanoemMutableModelLabelItemCreateFromBoneObject(destination, bones[0], &status);
            nanoemMutableModelLabelInsertItemObject(destination, v, -1, &status);
            nanoemMutableModelInsertLabelObject(mutable_model, destination, 0, &status);
            nanoemMutableModelLabelItemDestroy(v);
        }
    }
    if ((flags & CONVERT_RIGID_BODIES) != 0) {
        nanoem_rsize_t num_rigid_bodies;
        nanoem_model_rigid_body_t *const *rigid_bodies = nanoemModelGetAllRigidBodyObjects(model, &num_rigid_bodies);
        nanoem_unicode_string_hash_key_t key;
        key.factory = factory;
        for (nanoem_rsize_t i = 0; i < num_rigid_bodies; i++) {
            nanoem_model_rigid_body_t *source = rigid_bodies[i];
            nanoem_mutable_model_rigid_body_t *destination = nanoemMutableModelRigidBodyCreate(origin, &status);
            nanoemMutableModelRigidBodySetName(destination,
                nanoemModelRigidBodyGetName(source, NANOEM_LANGUAGE_TYPE_JAPANESE), NANOEM_LANGUAGE_TYPE_JAPANESE,
                &status);
            nanoemMutableModelRigidBodySetName(destination,
                nanoemModelRigidBodyGetName(source, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH,
                &status);
            nanoemMutableModelRigidBodySetAngularDamping(destination, nanoemModelRigidBodyGetAngularDamping(source));
            nanoemMutableModelRigidBodySetCollisionGroupId(
                destination, nanoemModelRigidBodyGetCollisionGroupId(source));
            nanoemMutableModelRigidBodySetCollisionMask(destination, nanoemModelRigidBodyGetCollisionMask(source));
            nanoemMutableModelRigidBodySetFriction(destination, nanoemModelRigidBodyGetFriction(source));
            nanoemMutableModelRigidBodySetLinearDamping(destination, nanoemModelRigidBodyGetLinearDamping(source));
            nanoemMutableModelRigidBodySetMass(destination, nanoemModelRigidBodyGetMass(source));
            nanoemMutableModelRigidBodySetTransformType(destination, nanoemModelRigidBodyGetTransformType(source));
            nanoemMutableModelRigidBodySetOrientation(destination, nanoemModelRigidBodyGetOrientation(source));
            nanoemMutableModelRigidBodySetRestitution(destination, nanoemModelRigidBodyGetRestitution(source));
            nanoemMutableModelRigidBodySetShapeSize(destination, nanoemModelRigidBodyGetShapeSize(source));
            nanoemMutableModelRigidBodySetShapeType(destination, nanoemModelRigidBodyGetShapeType(source));
            const nanoem_model_bone_t *bone0 = nanoemModelRigidBodyGetBoneObject(source);
            key.name = nanoemModelBoneGetName(bone0, NANOEM_LANGUAGE_TYPE_JAPANESE);
            khiter_t it = kh_get_bone_hash(bone_hash, key);
            if (it != kh_end(bone_hash)) {
                if (const nanoem_model_bone_t *bone = kh_val(bone_hash, it)) {
                    nanoemMutableModelRigidBodySetBoneObject(destination, bone);
                }
                const nanoem_f32_t *v1 = nanoemModelBoneGetOrigin(bone0);
                const nanoem_f32_t *v2 = nanoemModelRigidBodyGetOrigin(source);
                const nanoem_f32_t v3[] = { v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2] };
                nanoemMutableModelRigidBodySetOrigin(destination, v3);
            }
            else {
                nanoemMutableModelRigidBodySetOrigin(destination, nanoemModelRigidBodyGetOrigin(source));
            }
            nanoemMutableModelInsertRigidBodyObject(mutable_model, destination, -1, &status);
            nanoemMutableModelRigidBodyDestroy(destination);
        }
    }
    if ((flags & CONVERT_JOINTS) != 0) {
        nanoem_rsize_t num_joints;
        nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(model, &num_joints);
        for (nanoem_rsize_t i = 0; i < num_joints; i++) {
            nanoem_model_joint_t *source = joints[i];
            nanoem_mutable_model_joint_t *destination = nanoemMutableModelJointCreate(origin, &status);
            nanoemMutableModelJointSetName(destination, nanoemModelJointGetName(source, NANOEM_LANGUAGE_TYPE_JAPANESE),
                NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
            nanoemMutableModelJointSetName(destination, nanoemModelJointGetName(source, NANOEM_LANGUAGE_TYPE_ENGLISH),
                NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
            nanoemMutableModelJointSetAngularLowerLimit(destination, nanoemModelJointGetAngularLowerLimit(source));
            nanoemMutableModelJointSetAngularStiffness(destination, nanoemModelJointGetAngularStiffness(source));
            nanoemMutableModelJointSetAngularUpperLimit(destination, nanoemModelJointGetAngularUpperLimit(source));
            nanoemMutableModelJointSetLinearLowerLimit(destination, nanoemModelJointGetLinearLowerLimit(source));
            nanoemMutableModelJointSetLinearStiffness(destination, nanoemModelJointGetLinearStiffness(source));
            nanoemMutableModelJointSetLinearUpperLimit(destination, nanoemModelJointGetLinearUpperLimit(source));
            nanoemMutableModelJointSetOrientation(destination, nanoemModelJointGetOrientation(source));
            nanoemMutableModelJointSetOrigin(destination, nanoemModelJointGetOrigin(source));
            nanoemMutableModelJointSetRigidBodyAObject(destination, nanoemModelJointGetRigidBodyAObject(source));
            nanoemMutableModelJointSetRigidBodyBObject(destination, nanoemModelJointGetRigidBodyBObject(source));
            nanoemMutableModelJointSetType(destination, nanoemModelJointGetType(source));
            nanoemMutableModelInsertJointObject(mutable_model, destination, -1, &status);
            nanoemMutableModelJointDestroy(destination);
        }
    }
    kh_destroy_bone_hash(bone_hash);
    kh_destroy_bone_hash(constraint_hash);
    kh_destroy_morph_hash(morph_hash);
}

} /* namespace anonymous */

int
main(int argc, char *argv[])
{
    const int flags = 0 |
        CONVERT_BONES
        // | CONVERT_INDICES
        // | CONVERT_JOINTS
        | CONVERT_LABELS | CONVERT_MATERIALS | CONVERT_MORPHS
        // | CONVERT_RIGID_BODIES
        // | CONVERT_VERTICES
        ;
    const char *input_path = argc > 1 ? argv[1] : "test.pmd";
    const char *output_path = argc > 2 ? argv[2] : "test.pmx";
    initialize();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    if (FILE *reader = fopen(input_path, "rb")) {
        fseek(reader, 0, SEEK_END);
        size_t size = ftell(reader);
        fseek(reader, 0, SEEK_SET);
        nanoem_u8_t *bytes = new nanoem_u8_t[size];
        fread(bytes, size, 1, reader);
        fclose(reader);
        nanoem_unicode_string_factory_t *factory = nanoemUnicodeStringFactoryCreateEXT(&status);
        const char *input_suffix = strrchr(input_path, '.');
        const char *output_suffix = strrchr(output_path, '.');
        if (strcmp(input_suffix, ".x") == 0) {
            nanodxm_document_t *document = nanodxmDocumentCreate();
            nanodxm_buffer_t *input_buffer = nanodxmBufferCreate(bytes, size);
            if (nanodxmDocumentParse(document, input_buffer) == NANODXM_STATUS_SUCCESS) {
                nanoem_mutable_buffer_t *mutable_buffer = nanoemMutableBufferCreate(&status);
                nanoem_mutable_model_t *mutable_model = nanoemMutableModelCreate(factory, &status);
                convertDXMesh(document, mutable_model, factory);
                nanoemMutableModelSaveToBuffer(mutable_model, mutable_buffer, &status);
                if (status == NANOEM_STATUS_SUCCESS) {
                    nanoem_buffer_t *output_buffer = nanoemMutableBufferCreateBufferObject(mutable_buffer, &status);
                    if (FILE *writer = fopen(output_path, "wb")) {
                        fwrite(nanoemBufferGetDataPtr(output_buffer), nanoemBufferGetLength(output_buffer), 1, writer);
                        fclose(writer);
                        fprintf(stderr, "convert %s as %s.\n", input_path, output_path);
                    }
                    nanoemBufferDestroy(output_buffer);
                }
                nanoemMutableModelDestroy(mutable_model);
                nanoemMutableBufferDestroy(mutable_buffer);
            }
            nanodxmBufferDestroy(input_buffer);
            nanodxmDocumentDestroy(document);
        }
        else if (strcmp(input_suffix, ".pmd") == 0 || strcmp(input_suffix, ".pmx") == 0) {
            nanoem_model_t *model = nanoemModelCreate(factory, &status);
            nanoem_buffer_t *input_buffer = nanoemBufferCreate(bytes, size, &status);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            nanoemModelLoadFromBuffer(model, input_buffer, &status);
            if (status == NANOEM_STATUS_SUCCESS) {
                nanoem_mutable_buffer_t *mutable_buffer = nanoemMutableBufferCreate(&status);
                nanoem_mutable_model_t *mutable_model = NULL;
                if (strcmp(input_suffix, output_suffix) == 0) {
                    mutable_model = nanoemMutableModelCreateAsReference(model, &status);
                }
                else if (strcmp(input_suffix, ".pmd") == 0 && strcmp(output_suffix, ".pmx") == 0) {
#if 0
                    mutable_model = nanoemMutableModelCreate(factory, &status);
                    convertPMXFromPMD(mutable_model, model, factory, NANOEM_MODEL_FORMAT_TYPE_PMX_2_0, flags);
#else
                    nanoem_status_t mutableStatus = NANOEM_STATUS_SUCCESS;
                    nanoem_model_converter_t *converter = nanoemModelConverterCreate(model, &status);
                    mutable_model =
                        nanoemModelConverterExecute(converter, NANOEM_MODEL_FORMAT_TYPE_PMX_2_0, &mutableStatus);
                    nanoemModelConverterDestroy(converter);
#endif
                }
                if (mutable_model) {
                    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                    nanoemMutableModelSaveToBuffer(mutable_model, mutable_buffer, &status);
                    if (status == NANOEM_STATUS_SUCCESS) {
                        nanoem_buffer_t *output_buffer = nanoemMutableBufferCreateBufferObject(mutable_buffer, &status);
                        if (FILE *writer = fopen(output_path, "wb")) {
                            fwrite(
                                nanoemBufferGetDataPtr(output_buffer), nanoemBufferGetLength(output_buffer), 1, writer);
                            fclose(writer);
                            fprintf(stderr, "convert %s as %s.\n", input_path, output_path);
                        }
                        nanoemBufferDestroy(output_buffer);
                    }
                    nanoemMutableModelDestroy(mutable_model);
                    nanoemMutableBufferDestroy(mutable_buffer);
                }
            }
            nanoemModelDestroy(model);
        }
        else {
            fprintf(stderr, "no match convertion of %s.\n", input_suffix);
        }
        nanoemUnicodeStringFactoryDestroyEXT(factory);
        delete[] bytes;
    }
    else {
        fprintf(stderr, "%s is not found.\n", input_path);
    }
    finalize();
    return 0;
}
