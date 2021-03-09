/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./converter.h"

#include "./mutable_p.h"

#include <stdio.h>
#include <float.h>

#ifdef _WIN32
#define nanoem_snprintf _snprintf
#else
#define nanoem_snprintf snprintf
#endif

#define nanoem_unicode_string_hash_equal(a, b) ((a).factory->compare((a).factory->opaque_data, (a).name, (b).name) == 0)
#define nanoem_unicode_string_hash_func(a) ((a).factory->hash((a).factory->opaque_data, (a).name))
struct nanoem_unicode_string_hash_key_t {
    nanoem_unicode_string_factory_t *factory;
    const nanoem_unicode_string_t *name;
};
typedef struct nanoem_unicode_string_hash_key_t nanoem_unicode_string_hash_key_t;

nanoem_pragma_diagnostics_push();
nanoem_pragma_diagnostics_ignore_clang_gcc("-Wunused-function");
nanoem_pragma_diagnostics_ignore_msvc(4930);
KHASH_INIT(bone_hash, nanoem_unicode_string_hash_key_t, nanoem_model_bone_t *, 1, nanoem_unicode_string_hash_func, nanoem_unicode_string_hash_equal);
KHASH_INIT(morph_hash, nanoem_unicode_string_hash_key_t, nanoem_model_morph_t *, 1, nanoem_unicode_string_hash_func, nanoem_unicode_string_hash_equal);
nanoem_pragma_diagnostics_pop();

struct nanoem_model_converter_t {
    nanoem_model_t *source;
};

nanoem_model_converter_t *APIENTRY
nanoemModelConverterCreate(nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_converter_t *converter;
    converter = (nanoem_model_converter_t *) nanoem_calloc(sizeof(*converter), 1, status);
    if (nanoem_is_not_null(converter)) {
        converter->source = model;
    }
    return converter;
}

static void
nanoemModelConverterCopyConstraint(nanoem_mutable_model_constraint_t *destination_constraint, const nanoem_model_constraint_t *constraint, nanoem_unicode_string_factory_t *factory)
{
    static const nanoem_u8_t kJapaneseLeftKneeName[] = { 0xe5, 0xb7, 0xa6, 0xe3, 0x81, 0xb2, 0xe3, 0x81, 0x96, 0x0 },
                                kJapaneseRightKneeName[] = { 0xe5, 0x8f, 0xb3, 0xe3, 0x81, 0xb2, 0xe3, 0x81, 0x96, 0x0 };
    static const float kUpperLimit[] = { -0.008726646185809f, 0.0f, 0.0f, 0.0f };
    static const float kLowerLimit[] = { -3.141592626891544f, 0.0f, 0.0f, 0.0f };
    nanoem_mutable_model_constraint_joint_t *destination_joint;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_model_constraint_joint_t *const *joints;
    const nanoem_model_constraint_joint_t *joint;
    const nanoem_unicode_string_t *name;
    nanoem_rsize_t length, num_joints, i;
    nanoem_u8_t *buffer;
    nanoemMutableModelConstraintSetAngleLimit(destination_constraint, nanoemModelConstraintGetAngleLimit(constraint) * 4.0f);
    nanoemMutableModelConstraintSetEffectorBoneObject(destination_constraint, nanoemModelConstraintGetEffectorBoneObject(constraint));
    nanoemMutableModelConstraintSetNumIterations(destination_constraint, nanoemModelConstraintGetNumIterations(constraint));
    nanoemMutableModelConstraintSetTargetBoneObject(destination_constraint, nanoemModelConstraintGetTargetBoneObject(constraint));
    joints = nanoemModelConstraintGetAllJointObjects(constraint, &num_joints);
    for (i = 0; i < num_joints; i++) {
        joint = joints[i];
        destination_joint = nanoemMutableModelConstraintJointCreate(destination_constraint, &status);
        nanoemMutableModelConstraintJointSetBoneObject(destination_joint, nanoemModelConstraintJointGetBoneObject(joint));
        name = nanoemModelBoneGetName(nanoemModelConstraintJointGetBoneObject(joint), NANOEM_LANGUAGE_TYPE_JAPANESE);
        buffer = nanoemUnicodeStringFactoryGetByteArray(factory, name, &length, &status);
        if (buffer && (nanoem_crt_strcmp((const char *) buffer, (const char *) kJapaneseLeftKneeName) == 0 ||
                       nanoem_crt_strcmp((const char *) buffer, (const char *) kJapaneseRightKneeName) == 0)) {
            nanoemMutableModelConstraintJointSetLowerLimit(destination_joint, kLowerLimit);
            nanoemMutableModelConstraintJointSetUpperLimit(destination_joint, kUpperLimit);
            nanoemMutableModelConstraintJointSetAngleLimitEnabled(destination_joint, nanoem_true);
        }
        else {
            nanoemMutableModelConstraintJointSetLowerLimit(destination_joint, nanoemModelConstraintJointGetLowerLimit(joint));
            nanoemMutableModelConstraintJointSetUpperLimit(destination_joint, nanoemModelConstraintJointGetUpperLimit(joint));
            nanoemMutableModelConstraintJointSetAngleLimitEnabled(destination_joint, nanoemModelConstraintJointHasAngleLimit(joint));
        }
        nanoemUnicodeStringFactoryDestroyByteArray(factory, buffer);
        nanoemMutableModelConstraintInsertJointObject(destination_constraint, destination_joint, -1, &status);
        nanoemMutableModelConstraintJointDestroy(destination_joint);
    }
}

static void
nanoemModelConverterConvertAllBoneObjects(nanoem_mutable_model_t *mutable_model, const nanoem_model_t *model, nanoem_unicode_string_factory_t *factory, kh_bone_hash_t *bone_hash)
{
    nanoem_rsize_t num_source_bones, num_dest_bones, num_constraints, i;
    const nanoem_unicode_string_t *name;
    nanoem_unicode_string_t *new_name;
    nanoem_model_t *origin_model = nanoemMutableModelGetOriginObject(mutable_model);
    nanoem_model_bone_t *const *all_source_bones = nanoemModelGetAllBoneObjects(model, &num_source_bones);
    nanoem_model_bone_t *const *all_dest_bones;
    nanoem_model_bone_t **ordered_bones;
    nanoem_model_constraint_t *const *constraints = nanoemModelGetAllConstraintObjects(model, &num_constraints);
    nanoem_mutable_model_bone_t *destination_bone;
    nanoem_model_bone_t *bone, *source_bone;
    nanoem_mutable_model_constraint_t *destination_constraint;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_model_constraint_t *constraint;
    nanoem_unicode_string_hash_key_t key;
    khiter_t it;
    nanoem_rsize_t length;
    nanoem_u8_t buffer[64], *name_buffer;
    int ret;
    key.factory = factory;
    for (i = 0; i < num_source_bones; i++) {
        source_bone = all_source_bones[i];
        destination_bone = nanoemMutableModelBoneCreate(origin_model, &status);
        nanoemMutableModelBoneSetName(destination_bone, nanoemModelBoneGetName(source_bone, NANOEM_LANGUAGE_TYPE_JAPANESE), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
        nanoemMutableModelBoneSetName(destination_bone, nanoemModelBoneGetName(source_bone, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
        nanoemMutableModelBoneSetAffectedByPhysicsSimulation(destination_bone, nanoem_false);
        nanoemMutableModelBoneSetFixedAxisEnabled(destination_bone, nanoemModelBoneHasFixedAxis(source_bone));
        nanoemMutableModelBoneSetFixedAxis(destination_bone, nanoemModelBoneGetFixedAxis(source_bone));
        nanoemMutableModelBoneSetLocalAxesEnabled(destination_bone, nanoemModelBoneHasLocalAxes(source_bone));
        nanoemMutableModelBoneSetLocalXAxis(destination_bone, nanoemModelBoneGetLocalXAxis(source_bone));
        nanoemMutableModelBoneSetLocalZAxis(destination_bone, nanoemModelBoneGetLocalZAxis(source_bone));
        nanoemMutableModelBoneSetMovable(destination_bone, nanoemModelBoneIsMovable(source_bone));
        nanoemMutableModelBoneSetOrigin(destination_bone, nanoemModelBoneGetOrigin(source_bone));
        nanoemMutableModelBoneSetRotateable(destination_bone, nanoemModelBoneIsRotateable(source_bone));
        nanoemMutableModelBoneSetTargetBoneObject(destination_bone, nanoemModelBoneGetTargetBoneObject(source_bone));
        nanoemMutableModelBoneSetUserHandleable(destination_bone, nanoemModelBoneIsUserHandleable(source_bone));
        nanoemMutableModelBoneSetVisible(destination_bone, nanoemModelBoneIsVisible(source_bone));
        nanoemMutableModelInsertBoneObject(mutable_model, destination_bone, -1, &status);
        name = key.name = nanoemModelBoneGetName(source_bone, NANOEM_LANGUAGE_TYPE_JAPANESE);
        it = kh_put_bone_hash(bone_hash, key, &ret);
        kh_val(bone_hash, it) = nanoemMutableModelBoneGetOriginObject(destination_bone);
        nanoemMutableModelBoneDestroy(destination_bone);
    }
    constraints = nanoemModelGetAllConstraintObjects(model, &num_constraints);
    for (i = 0; i < num_constraints; i++) {
        constraint = constraints[i];
        key.name = nanoemModelBoneGetName(nanoemModelConstraintGetTargetBoneObject(constraint), NANOEM_LANGUAGE_TYPE_JAPANESE);
        it = kh_get_bone_hash(bone_hash, key);
        if (it != kh_end(bone_hash)) {
            bone = kh_val(bone_hash, it);
            if (bone) {
                destination_constraint = nanoemMutableModelConstraintCreate(origin_model, &status);
                if (!nanoemModelBoneGetConstraintObject(bone)) {
                    nanoemModelConverterCopyConstraint(destination_constraint, constraint, factory);
                    destination_bone = nanoemMutableModelBoneCreateAsReference(bone, &status);
                    nanoemMutableModelBoneSetConstraintObject(destination_bone, destination_constraint);
                }
                else {
                    name = nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_JAPANESE);
                    name_buffer = nanoemUnicodeStringFactoryGetByteArray(factory, name, &length, &status);
                    nanoem_snprintf((char *) buffer, sizeof(buffer), "%s+", name_buffer);
                    nanoemUnicodeStringFactoryDestroyByteArray(factory, name_buffer);
                    new_name = nanoemUnicodeStringFactoryCreateString(factory, buffer, nanoem_crt_strlen((const char *) buffer), &status);
                    nanoemModelConverterCopyConstraint(destination_constraint, constraint, factory);
                    destination_bone = nanoemMutableModelBoneCreate(origin_model, &status);
                    nanoemMutableModelBoneSetName(destination_bone, new_name, NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
                    nanoemUnicodeStringFactoryDestroyString(factory, new_name);
                    nanoemMutableModelBoneSetName(destination_bone, nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
                    nanoemMutableModelBoneSetAffectedByPhysicsSimulation(destination_bone, nanoem_false);
                    nanoemMutableModelBoneSetMovable(destination_bone, nanoem_true);
                    nanoemMutableModelBoneSetOrigin(destination_bone, nanoemModelBoneGetOrigin(bone));
                    nanoemMutableModelBoneSetParentBoneObject(destination_bone, bone);
                    nanoemMutableModelBoneSetRotateable(destination_bone, nanoem_true);
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
    all_dest_bones = nanoemModelGetAllBoneObjects(nanoemMutableModelGetOriginObject(mutable_model), &num_dest_bones);
    for (i = 0; i < num_source_bones; i++) {
        source_bone = all_source_bones[i];
        destination_bone = nanoemMutableModelBoneCreateAsReference(all_dest_bones[i], &status);
        key.name = nanoemModelBoneGetName(nanoemModelBoneGetParentBoneObject(source_bone), NANOEM_LANGUAGE_TYPE_JAPANESE);
        it = kh_get_bone_hash(bone_hash, key);
        if (it != kh_end(bone_hash)) {
            bone = kh_val(bone_hash, it);
            if (bone) {
                nanoemMutableModelBoneSetParentBoneObject(destination_bone, bone);
                int stage_index = nanoemModelBoneGetStageIndex(bone);
                if (stage_index > 0) {
                    nanoemMutableModelBoneSetStageIndex(destination_bone, stage_index);
                }
            }
        }
        if (nanoemModelBoneHasInherentOrientation(source_bone)) {
            key.name = nanoemModelBoneGetName(nanoemModelBoneGetInherentParentBoneObject(source_bone), NANOEM_LANGUAGE_TYPE_JAPANESE);
            it = kh_get_bone_hash(bone_hash, key);
            if (it != kh_end(bone_hash)) {
                bone = kh_val(bone_hash, it);
                if (bone) {
                    nanoemMutableModelBoneSetInherentOrientationEnabled(destination_bone, nanoem_true);
                    nanoemMutableModelBoneSetInherentParentBoneObject(destination_bone, bone);
                    nanoemMutableModelBoneSetStageIndex(destination_bone, nanoemModelBoneHasDestinationBone(source_bone) ? 2 : 0);
                }
            }
            nanoemMutableModelBoneSetInherentCoefficient(destination_bone, nanoemModelBoneGetInherentCoefficient(source_bone));
        }
        if (nanoemModelBoneHasDestinationBone(source_bone)) {
            key.name = nanoemModelBoneGetName(nanoemModelBoneGetTargetBoneObject(source_bone), NANOEM_LANGUAGE_TYPE_JAPANESE);
            it = kh_get_bone_hash(bone_hash, key);
            if (it != kh_end(bone_hash)) {
                bone = kh_val(bone_hash, it);
                if (bone) {
                    nanoemMutableModelBoneSetTargetBoneObject(destination_bone, bone);
                }
            }
        }
        nanoemMutableModelBoneDestroy(destination_bone);
    }
    ordered_bones = (nanoem_model_bone_t **) nanoem_calloc(sizeof(*mutable_model->origin->ordered_bones), num_dest_bones, &status);
    if (nanoem_is_not_null(ordered_bones)) {
        nanoem_free(mutable_model->origin->ordered_bones);
        nanoem_crt_memcpy(ordered_bones, mutable_model->origin->bones, sizeof(*ordered_bones) * num_dest_bones);
        nanoem_crt_qsort(ordered_bones, num_dest_bones, sizeof(*ordered_bones), nanoemModelCompareBonePMX);
        mutable_model->origin->ordered_bones = ordered_bones;
    }
}

static void
nanoemModelConverterSetVertexBoneObjectAndWeight(nanoem_mutable_model_vertex_t *destination, nanoem_model_vertex_t *source, nanoem_rsize_t source_index, nanoem_rsize_t dest_index, nanoem_unicode_string_hash_key_t *key, kh_bone_hash_t *bone_hash)
{
    const nanoem_model_bone_t *source_bone = nanoemModelVertexGetBoneObject(source, source_index), *dest_bone = NULL;
    khiter_t it;
    key->name = nanoemModelBoneGetName(source_bone, NANOEM_LANGUAGE_TYPE_JAPANESE);
    it = kh_get_bone_hash(bone_hash, *key);
    if (it != kh_end(bone_hash)) {
        dest_bone = kh_val(bone_hash, it);
    }
    nanoemMutableModelVertexSetBoneObject(destination, dest_bone, dest_index);
    nanoemMutableModelVertexSetBoneWeight(destination, nanoemModelVertexGetBoneWeight(source, source_index), dest_index);
}

static void
nanoemModelConverterConvertAllVertexObjects(nanoem_mutable_model_t *mutable_model, const nanoem_model_t *model, nanoem_unicode_string_factory_t *factory, kh_bone_hash_t *bone_hash)
{
    nanoem_rsize_t num_vertices, i;
    nanoem_model_t *origin_model = nanoemMutableModelGetOriginObject(mutable_model);
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(model, &num_vertices);
    nanoem_model_vertex_t *source;
    nanoem_mutable_model_vertex_t *destination;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_hash_key_t key;
    nanoem_f32_t weight;
    key.factory = factory;
    for (i = 0; i < num_vertices; i++) {
        source = vertices[i];
        destination = nanoemMutableModelVertexCreate(origin_model, &status);
        nanoemMutableModelVertexSetOrigin(destination, nanoemModelVertexGetOrigin(source));
        nanoemMutableModelVertexSetNormal(destination, nanoemModelVertexGetNormal(source));
        nanoemMutableModelVertexSetTexCoord(destination, nanoemModelVertexGetTexCoord(source));
        nanoemMutableModelVertexSetEdgeSize(destination, nanoemModelVertexGetEdgeSize(source));
        weight = nanoemModelVertexGetBoneWeight(source, 0);
        if (weight <= FLT_EPSILON || 1.0 - weight <= FLT_EPSILON) {
            nanoemMutableModelVertexSetType(destination, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
        }
        else {
            nanoemMutableModelVertexSetType(destination, NANOEM_MODEL_VERTEX_TYPE_BDEF2);
        }
        if (nanoemModelVertexGetBoneWeight(source, 1) > nanoemModelVertexGetBoneWeight(source, 0)) {
            nanoemModelConverterSetVertexBoneObjectAndWeight(destination, source, 1, 0, &key, bone_hash);
            nanoemModelConverterSetVertexBoneObjectAndWeight(destination, source, 0, 1, &key, bone_hash);
        }
        else {
            nanoemModelConverterSetVertexBoneObjectAndWeight(destination, source, 0, 0, &key, bone_hash);
            nanoemModelConverterSetVertexBoneObjectAndWeight(destination, source, 1, 1, &key, bone_hash);
        }
        nanoemMutableModelInsertVertexObject(mutable_model, destination, -1, &status);
        nanoemMutableModelVertexDestroy(destination);
    }
}

static void
nanoemModelConverterConvertAllMaterialObjects(nanoem_mutable_model_t *mutable_model, const nanoem_model_t *model, nanoem_unicode_string_factory_t *factory)
{
    static const unsigned char kJapaneseMaterialNamePrefix[] = { 0xe6, 0x9d, 0x90, 0xe8, 0xb3, 0xaa, 0x0 };
    const nanoem_unicode_string_t *texture_path;
    nanoem_rsize_t num_materials, length, i;
    nanoem_model_material_t *source;
    nanoem_model_t *origin_model = nanoemMutableModelGetOriginObject(mutable_model);
    nanoem_mutable_model_material_t *destination;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(model, &num_materials);
    nanoem_mutable_model_texture_t *diffuse_texture, *sphere_texture, *toon_texture;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_t *name;
    nanoem_u8_t buffer[64], *toon_texture_path;
    int shared_toon_index;
    for (i = 0; i < num_materials; i++) {
        source = materials[i];
        destination = nanoemMutableModelMaterialCreate(origin_model, &status);
        nanoem_snprintf((char *) buffer, sizeof(buffer), "%s%u", kJapaneseMaterialNamePrefix, (nanoem_u32_t) i + 1);
        name = nanoemUnicodeStringFactoryCreateString(factory, buffer, nanoem_crt_strlen((const char *)buffer), &status);
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
        nanoemMutableModelMaterialSetNumVertexIndices(destination, nanoemModelMaterialGetNumVertexIndices(source));
        nanoemMutableModelMaterialSetCastingShadowEnabled(destination, nanoemModelMaterialIsCastingShadowEnabled(source));
        nanoemMutableModelMaterialSetCastingShadowMapEnabled(destination, nanoemModelMaterialIsCastingShadowMapEnabled(source));
        nanoemMutableModelMaterialSetCullingDisabled(destination, nanoemModelMaterialIsCullingDisabled(source));
        nanoemMutableModelMaterialSetEdgeEnabled(destination, nanoemModelMaterialIsEdgeEnabled(source));
        nanoemMutableModelMaterialSetShadowMapEnabled(destination, nanoemModelMaterialIsShadowMapEnabled(source));
        texture_path = nanoemModelTextureGetPath(nanoemModelMaterialGetDiffuseTextureObject(source));
        if (texture_path) {
            diffuse_texture = nanoemMutableModelTextureCreate(origin_model, &status);
            nanoemMutableModelTextureSetPath(diffuse_texture, texture_path, &status);
            nanoemMutableModelMaterialSetDiffuseTextureObject(destination, diffuse_texture, &status);
            nanoemMutableModelTextureDestroy(diffuse_texture);
        }
        texture_path = nanoemModelTextureGetPath(nanoemModelMaterialGetSphereMapTextureObject(source));
        if (texture_path) {
            sphere_texture = nanoemMutableModelTextureCreate(origin_model, &status);
            nanoemMutableModelTextureSetPath(sphere_texture, texture_path, &status);
            nanoemMutableModelMaterialSetSphereMapTextureType(destination, nanoemModelMaterialGetSphereMapTextureType(source));
            nanoemMutableModelMaterialSetSphereMapTextureObject(destination, sphere_texture, &status);
            nanoemMutableModelTextureDestroy(sphere_texture);
        }
        else {
            nanoemMutableModelMaterialSetSphereMapTextureType(destination, NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE);
        }
        texture_path = nanoemModelTextureGetPath(nanoemModelMaterialGetToonTextureObject(source));
        if (texture_path) {
            shared_toon_index = -1;
            toon_texture_path = nanoemUnicodeStringFactoryGetByteArray(factory, texture_path, &length, &status);
            if (toon_texture_path) {
                if (nanoem_crt_strcmp((const char *)toon_texture_path, "toon01.bmp") == 0) {
                    shared_toon_index = 0;
                }
                else if (nanoem_crt_strcmp((const char *)toon_texture_path, "toon02.bmp") == 0) {
                    shared_toon_index = 1;
                }
                else if (nanoem_crt_strcmp((const char *)toon_texture_path, "toon03.bmp") == 0) {
                    shared_toon_index = 2;
                }
                else if (nanoem_crt_strcmp((const char *)toon_texture_path, "toon04.bmp") == 0) {
                    shared_toon_index = 3;
                }
                else if (nanoem_crt_strcmp((const char *)toon_texture_path, "toon05.bmp") == 0) {
                    shared_toon_index = 4;
                }
                else if (nanoem_crt_strcmp((const char *)toon_texture_path, "toon06.bmp") == 0) {
                    shared_toon_index = 5;
                }
                else if (nanoem_crt_strcmp((const char *)toon_texture_path, "toon07.bmp") == 0) {
                    shared_toon_index = 6;
                }
                else if (nanoem_crt_strcmp((const char *)toon_texture_path, "toon08.bmp") == 0) {
                    shared_toon_index = 7;
                }
                else if (nanoem_crt_strcmp((const char *)toon_texture_path, "toon09.bmp") == 0) {
                    shared_toon_index = 8;
                }
                else if (nanoem_crt_strcmp((const char *)toon_texture_path, "toon10.bmp") == 0) {
                    shared_toon_index = 9;
                }
                nanoemUnicodeStringFactoryDestroyByteArray(factory, toon_texture_path);
            }
            if (shared_toon_index > -1) {
                nanoemMutableModelMaterialSetToonShared(destination, nanoem_true);
                nanoemMutableModelMaterialSetToonTextureIndex(destination, shared_toon_index);
            }
            else {
                toon_texture = nanoemMutableModelTextureCreate(origin_model, &status);
                nanoemMutableModelTextureSetPath(toon_texture, texture_path, &status);
                nanoemMutableModelMaterialSetToonTextureObject(destination, toon_texture, &status);
                nanoemMutableModelTextureDestroy(toon_texture);
            }
        }
        nanoemMutableModelInsertMaterialObject(mutable_model, destination, -1, &status);
        nanoemMutableModelMaterialDestroy(destination);
    }
}

void
nanoemModelConverterConvertAllMorphObjects(nanoem_mutable_model_t *mutable_model, const nanoem_model_t *model, nanoem_unicode_string_factory_t *factory, kh_morph_hash_t *morph_hash)
{
    nanoem_rsize_t num_morphs, num_morph_children, i, j;
    nanoem_model_t *origin_model = nanoemMutableModelGetOriginObject(mutable_model);
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model, &num_morphs);
    nanoem_mutable_model_morph_t *destination;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_model_morph_t *source;
    nanoem_model_morph_category_t type;
    nanoem_model_morph_vertex_t *const *children;
    nanoem_model_morph_vertex_t *child;
    nanoem_mutable_model_morph_vertex_t *v;
    nanoem_unicode_string_hash_key_t key;
    khiter_t it;
    int ret;
    key.factory = factory;
    for (i = 0; i < num_morphs; i++) {
        source = morphs[i];
        destination = nanoemMutableModelMorphCreate(origin_model, &status);
        type = nanoemModelMorphGetCategory(source);
        if (type != NANOEM_MODEL_MORPH_CATEGORY_BASE) {
            nanoemMutableModelMorphSetName(destination, nanoemModelMorphGetName(source, NANOEM_LANGUAGE_TYPE_JAPANESE), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
            nanoemMutableModelMorphSetName(destination, nanoemModelMorphGetName(source, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
            nanoemMutableModelMorphSetCategory(destination, type);
            nanoemMutableModelMorphSetType(destination, nanoemModelMorphGetType(source));
            children = nanoemModelMorphGetAllVertexMorphObjects(source, &num_morph_children);
            for (j = 0; j < num_morph_children; j++) {
                child = children[j];
                v = nanoemMutableModelMorphVertexCreate(destination, &status);
                nanoemMutableModelMorphVertexSetPosition(v, nanoemModelMorphVertexGetPosition(child));
                nanoemMutableModelMorphVertexSetVertexObject(v, nanoemModelMorphVertexGetVertexObject(child));
                nanoemMutableModelMorphInsertVertexMorphObject(destination, v, -1, &status);
                nanoemMutableModelMorphVertexDestroy(v);
            }
            nanoemMutableModelInsertMorphObject(mutable_model, destination, -1, &status);
            key.name = nanoemModelMorphGetName(source, NANOEM_LANGUAGE_TYPE_JAPANESE);
            it = kh_put_morph_hash(morph_hash, key, &ret);
            kh_val(morph_hash, it) = nanoemMutableModelMorphGetOriginObject(destination);
        }
        nanoemMutableModelMorphDestroy(destination);
    }
}

static void
nanoemModelConverterConvertAllLabelObjects(nanoem_mutable_model_t *mutable_model, const nanoem_model_t *model, nanoem_unicode_string_factory_t *factory, kh_bone_hash_t *bone_hash, kh_morph_hash_t *morph_hash)
{
    static const nanoem_u8_t kExpressionName[] = { 0xe8, 0xa1, 0xa8, 0xe6, 0x83, 0x85, 0x0 };
    nanoem_rsize_t num_bones, num_labels, num_label_items, i, j;
    nanoem_model_t *origin_model = nanoemMutableModelGetOriginObject(mutable_model);
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model, &num_bones);
    nanoem_model_label_t *const *labels = nanoemModelGetAllLabelObjects(model, &num_labels);
    nanoem_model_label_t *source;
    nanoem_mutable_model_label_t *destination;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    const nanoem_unicode_string_t *name;
    nanoem_unicode_string_t *new_name;
    nanoem_model_label_item_t *const *items;
    nanoem_model_label_item_t *item;
    nanoem_mutable_model_label_item_t *v = NULL;
    const nanoem_model_bone_t *item_bone;
    const nanoem_model_morph_t *item_morph;
    nanoem_model_bone_t *bone;
    nanoem_model_morph_t *morph;
    nanoem_unicode_string_hash_key_t key;
    nanoem_rsize_t length;
    khiter_t it;
    nanoem_u8_t *name_buffer, buffer[64];
    for (i = 0; i < num_labels; i++) {
        source = labels[i];
        destination = nanoemMutableModelLabelCreate(origin_model, &status);
        if (i > 0) {
            name = nanoemModelLabelGetName(source, NANOEM_LANGUAGE_TYPE_JAPANESE);
            name_buffer = nanoemUnicodeStringFactoryGetByteArray(factory, name, &length, &status);
            if (name_buffer[length - 1] == '\n') {
                length--;
            }
            new_name = nanoemUnicodeStringFactoryCreateString(factory, name_buffer, length, &status);
            nanoemUnicodeStringFactoryDestroyByteArray(factory, name_buffer);
            nanoemMutableModelLabelSetName(destination, new_name, NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
            nanoemUnicodeStringFactoryDestroyString(factory, new_name);
            nanoemMutableModelLabelSetName(destination, nanoemModelLabelGetName(source, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
        }
        else if (i == 0) {
            nanoem_crt_strncpy((char *) buffer, (const char *) kExpressionName, sizeof(buffer));
            new_name = nanoemUnicodeStringFactoryCreateString(factory, buffer, nanoem_crt_strlen((const char *)buffer), &status);
            nanoemMutableModelLabelSetName(destination, new_name, NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
            nanoemUnicodeStringFactoryDestroyString(factory, new_name);
            nanoem_crt_strncpy((char *) buffer, "Exp", sizeof(buffer));
            new_name = nanoemUnicodeStringFactoryCreateString(factory, buffer, nanoem_crt_strlen((const char *)buffer), &status);
            nanoemMutableModelLabelSetName(destination, new_name, NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
            nanoemUnicodeStringFactoryDestroyString(factory, new_name);
        }
        nanoemMutableModelLabelSetSpecial(destination, nanoemModelLabelIsSpecial(source));
        items = nanoemModelLabelGetAllItemObjects(source, &num_label_items);
        key.factory = factory;
        for (j = 0; j < num_label_items; j++) {
            item = items[j];
            item_bone = nanoemModelLabelItemGetBoneObject(item);
            item_morph = nanoemModelLabelItemGetMorphObject(item);
            if (item_bone) {
                key.name = nanoemModelBoneGetName(item_bone, NANOEM_LANGUAGE_TYPE_JAPANESE);
                it = kh_get_bone_hash(bone_hash, key);
                if (it != kh_end(bone_hash)) {
                    bone = kh_val(bone_hash, it);
                    if (bone) {
                        v = nanoemMutableModelLabelItemCreateFromBoneObject(destination, bone, &status);
                    }
                }
            }
            else if (item_morph) {
                key.name = nanoemModelMorphGetName(item_morph, NANOEM_LANGUAGE_TYPE_JAPANESE);
                it = kh_get_morph_hash(morph_hash, key);
                if (it != kh_end(morph_hash)) {
                    morph = kh_val(morph_hash, it);
                    if (morph) {
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
        nanoem_crt_strncpy((char *) buffer, "Root", sizeof(buffer));
        destination = nanoemMutableModelLabelCreate(origin_model, &status);
        new_name = nanoemUnicodeStringFactoryCreateString(factory, buffer, nanoem_crt_strlen((const char *) buffer), &status);
        nanoemMutableModelLabelSetName(destination, new_name, NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
        nanoemMutableModelLabelSetName(destination, new_name, NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
        nanoemMutableModelLabelSetSpecial(destination, nanoem_true);
        nanoemUnicodeStringFactoryDestroyString(factory, new_name);
        key.name = nanoemModelBoneGetName(bones[0], NANOEM_LANGUAGE_TYPE_JAPANESE);
        it = kh_get_bone_hash(bone_hash, key);
        if (it != kh_end(bone_hash)) {
            bone = kh_val(bone_hash, it);
            if (bone) {
                v = nanoemMutableModelLabelItemCreateFromBoneObject(destination, bone, &status);
            }
        }
        nanoemMutableModelLabelInsertItemObject(destination, v, -1, &status);
        nanoemMutableModelInsertLabelObject(mutable_model, destination, 0, &status);
        nanoemMutableModelLabelItemDestroy(v);
        nanoemMutableModelLabelDestroy(destination);
    }
}

static void
nanoemModelConverterConvertAllRigidBodyObjects(nanoem_mutable_model_t *mutable_model, const nanoem_model_t *model, nanoem_unicode_string_factory_t *factory, kh_bone_hash_t *bone_hash)
{
    nanoem_rsize_t num_rigid_bodies, num_bones, i;
    nanoem_model_t *origin_model = nanoemMutableModelGetOriginObject(mutable_model);
    nanoem_model_rigid_body_t *const *rigid_bodies = nanoemModelGetAllRigidBodyObjects(model, &num_rigid_bodies);
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model, &num_bones);
    nanoem_mutable_model_rigid_body_t *destination;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_model_rigid_body_t *source;
    const nanoem_model_bone_t *bone, *bone0;
    const nanoem_f32_t *v1, *v2;
    nanoem_f32_t v3[4];
    nanoem_unicode_string_hash_key_t key;
    khiter_t it;
    key.factory = factory;
    for (i = 0; i < num_rigid_bodies; i++) {
        source = rigid_bodies[i];
        destination = nanoemMutableModelRigidBodyCreate(origin_model, &status);
        nanoemMutableModelRigidBodySetName(destination, nanoemModelRigidBodyGetName(source, NANOEM_LANGUAGE_TYPE_JAPANESE), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
        nanoemMutableModelRigidBodySetName(destination, nanoemModelRigidBodyGetName(source, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
        nanoemMutableModelRigidBodySetAngularDamping(destination, nanoemModelRigidBodyGetAngularDamping(source));
        nanoemMutableModelRigidBodySetCollisionGroupId(destination, nanoemModelRigidBodyGetCollisionGroupId(source));
        nanoemMutableModelRigidBodySetCollisionMask(destination, nanoemModelRigidBodyGetCollisionMask(source));
        nanoemMutableModelRigidBodySetFriction(destination, nanoemModelRigidBodyGetFriction(source));
        nanoemMutableModelRigidBodySetLinearDamping(destination, nanoemModelRigidBodyGetLinearDamping(source));
        nanoemMutableModelRigidBodySetMass(destination, nanoemModelRigidBodyGetMass(source));
        nanoemMutableModelRigidBodySetOrientation(destination, nanoemModelRigidBodyGetOrientation(source));
        nanoemMutableModelRigidBodySetRestitution(destination, nanoemModelRigidBodyGetRestitution(source));
        nanoemMutableModelRigidBodySetShapeSize(destination, nanoemModelRigidBodyGetShapeSize(source));
        nanoemMutableModelRigidBodySetShapeType(destination, nanoemModelRigidBodyGetShapeType(source));
        nanoemMutableModelRigidBodySetTransformType(destination, nanoemModelRigidBodyGetTransformType(source));
        bone0 = nanoemModelRigidBodyGetBoneObject(source);
        key.name = nanoemModelBoneGetName(bone0, NANOEM_LANGUAGE_TYPE_JAPANESE);
        it = kh_get_bone_hash(bone_hash, key);
        if (it != kh_end(bone_hash)) {
            bone = kh_val(bone_hash, it);
            if (bone) {
                nanoemMutableModelRigidBodySetBoneObject(destination, bone);
            }
            v1 = nanoemModelBoneGetOrigin(bone0);
        }
        else {
            nanoemMutableModelRigidBodySetBoneObject(destination, NULL);
            v1 = nanoemModelBoneGetOrigin(num_bones > 0 ? bones[0] : NULL);
        }
        v2 = nanoemModelRigidBodyGetOrigin(source);
        v3[0] = v1[0] + v2[0];
        v3[1] = v1[1] + v2[1];
        v3[2] = v1[2] + v2[2];
        nanoemMutableModelRigidBodySetOrigin(destination, v3);
        nanoemMutableModelInsertRigidBodyObject(mutable_model, destination, -1, &status);
        nanoemMutableModelRigidBodyDestroy(destination);
    }
}

static void
nanoemModelConverterConvertAllJointObjects(nanoem_mutable_model_t *mutable_model, const nanoem_model_t *model)
{
    nanoem_rsize_t num_joints, i;
    nanoem_model_joint_t *source;
    nanoem_mutable_model_joint_t *destination;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_model_t *origin_model = nanoemMutableModelGetOriginObject(mutable_model);
    nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(model, &num_joints);
    for (i = 0; i < num_joints; i++) {
        source = joints[i];
        destination = nanoemMutableModelJointCreate(origin_model, &status);
        nanoemMutableModelJointSetName(destination, nanoemModelJointGetName(source, NANOEM_LANGUAGE_TYPE_JAPANESE), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
        nanoemMutableModelJointSetName(destination, nanoemModelJointGetName(source, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
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

nanoem_mutable_model_t *APIENTRY
nanoemModelConverterExecute(nanoem_model_converter_t *converter, nanoem_model_format_type_t target, nanoem_status_t *status)
{
    kh_bone_hash_t *bone_hash;
    kh_bone_hash_t *constraint_hash;
    kh_morph_hash_t *morph_hash;
    nanoem_unicode_string_factory_t *factory;
    nanoem_mutable_model_t *mutable_model = NULL;
    nanoem_model_t *model;
    const nanoem_u32_t *vertex_indices;
    nanoem_rsize_t num_vertex_indices;
    if (nanoem_is_not_null(converter)) {
        nanoem_status_ptr_assign_succeeded(status);
        bone_hash = kh_init_bone_hash();
        constraint_hash = kh_init_bone_hash();
        morph_hash = kh_init_morph_hash();
        factory = converter->source->factory;
        mutable_model = nanoemMutableModelCreate(factory, status);
        model = converter->source;
        nanoemMutableModelSetFormatType(mutable_model, target);
        nanoemMutableModelSetName(mutable_model, nanoemModelGetName(model, NANOEM_LANGUAGE_TYPE_JAPANESE), NANOEM_LANGUAGE_TYPE_JAPANESE, status);
        nanoemMutableModelSetName(mutable_model, nanoemModelGetName(model, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH, status);
        nanoemMutableModelSetComment(mutable_model, nanoemModelGetComment(model, NANOEM_LANGUAGE_TYPE_JAPANESE), NANOEM_LANGUAGE_TYPE_JAPANESE, status);
        nanoemMutableModelSetComment(mutable_model, nanoemModelGetComment(model, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH, status);
        nanoemModelConverterConvertAllBoneObjects(mutable_model, model, factory, bone_hash);
        nanoemModelConverterConvertAllVertexObjects(mutable_model, model, factory, bone_hash);
        vertex_indices = nanoemModelGetAllVertexIndices(model, &num_vertex_indices);
        nanoemMutableModelSetVertexIndices(mutable_model, vertex_indices, num_vertex_indices, status);
        nanoemModelConverterConvertAllMaterialObjects(mutable_model, model, factory);
        nanoemModelConverterConvertAllMorphObjects(mutable_model, model, factory, morph_hash);
        nanoemModelConverterConvertAllLabelObjects(mutable_model, model, factory, bone_hash, morph_hash);
        nanoemModelConverterConvertAllRigidBodyObjects(mutable_model, model, factory, bone_hash);
        nanoemModelConverterConvertAllJointObjects(mutable_model, model);
        kh_destroy_bone_hash(bone_hash);
        kh_destroy_bone_hash(constraint_hash);
        kh_destroy_morph_hash(morph_hash);
    }
    return mutable_model;
}

void APIENTRY
nanoemModelConverterDestroy(nanoem_model_converter_t *converter)
{
    if (nanoem_is_not_null(converter)) {
        nanoem_free(converter);
    }
}
