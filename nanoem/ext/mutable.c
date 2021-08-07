/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./mutable_p.h"

static void
nanoemMutableModelObjectApplyChangeObjectIndex(int *target, int object_index, int delta)
{
    int dest_object_index = *target;
    if (dest_object_index != NANOEM_MODEL_OBJECT_NOT_FOUND) {
        if (delta < 0 && dest_object_index == object_index) {
            *target = NANOEM_MODEL_OBJECT_NOT_FOUND;
        }
        else if (dest_object_index >= object_index) {
            *target = dest_object_index + delta;
        }
    }
}

static void
nanoemMutableModeVertexApplyChangeAllObjectIndices(nanoem_model_t *origin_model, int vertex_index, int delta)
{
    nanoem_model_morph_t *morph;
    nanoem_model_morph_type_t type;
    nanoem_model_morph_vertex_t *morph_vertex;
    nanoem_model_morph_uv_t *morph_uv;
    nanoem_model_soft_body_t *soft_body;
    nanoem_model_soft_body_anchor_t *anchor;
    nanoem_rsize_t num_objects, num_items, i, j;
    num_objects = origin_model->num_morphs;
    for (i = 0; i < num_objects; i++) {
        morph = origin_model->morphs[i];
        type = nanoemModelMorphGetType(morph);
        if (type == NANOEM_MODEL_MORPH_TYPE_VERTEX) {
            num_items = morph->num_objects;
            for (j = 0; j < num_items; j++) {
                morph_vertex = morph->u.vertices[j];
                nanoemMutableModelObjectApplyChangeObjectIndex(&morph_vertex->vertex_index, vertex_index, delta);
            }
        }
        else if (type == NANOEM_MODEL_MORPH_TYPE_UVA1 || type == NANOEM_MODEL_MORPH_TYPE_UVA2 || type == NANOEM_MODEL_MORPH_TYPE_UVA3 || type == NANOEM_MODEL_MORPH_TYPE_UVA4) {
            num_items = morph->num_objects;
            for (j = 0; j < num_items; j++) {
                morph_uv = morph->u.uvs[j];
                nanoemMutableModelObjectApplyChangeObjectIndex(&morph_uv->vertex_index, vertex_index, delta);
            }
        }
    }
    num_objects = origin_model->num_soft_bodies;
    for (i = 0; i < num_objects; i++) {
        soft_body = origin_model->soft_bodies[i];
        num_items = soft_body->num_anchors;
        for (j = 0; j < num_items; j++) {
            anchor = soft_body->anchors[j];
            nanoemMutableModelObjectApplyChangeObjectIndex(&anchor->vertex_index, vertex_index, delta);
        }
    }
}

static void
nanoemMutableModelMaterialApplyChangeAllObjectIndices(nanoem_model_t *origin_model, int material_index, int delta)
{
    nanoem_model_morph_t *morph;
    nanoem_model_morph_material_t *material;
    nanoem_model_soft_body_t *soft_body;
    nanoem_model_morph_type_t type;
    nanoem_rsize_t num_objects, num_items, i, j;
    num_objects = origin_model->num_morphs;
    for (i = 0; i < num_objects; i++) {
        morph = origin_model->morphs[i];
        type = nanoemModelMorphGetType(morph);
        if (type == NANOEM_MODEL_MORPH_TYPE_MATERIAL) {
            num_items = morph->num_objects;
            for (j = 0; j < num_items; j++) {
                material = morph->u.materials[j];
                nanoemMutableModelObjectApplyChangeObjectIndex(&material->material_index, material_index, delta);
            }
        }
    }
    num_objects = origin_model->num_soft_bodies;
    for (i = 0; i < num_objects; i++) {
        soft_body = origin_model->soft_bodies[i];
        nanoemMutableModelObjectApplyChangeObjectIndex(&soft_body->material_index, material_index, delta);
    }
}

static void
nanoemMutableModelBoneApplyChangeAllObjectIndices(nanoem_model_t *origin_model, int bone_index, int delta)
{
    nanoem_model_bone_t *bone;
    nanoem_model_constraint_t *constraint;
    nanoem_model_constraint_joint_t *joint;
    nanoem_model_morph_t *morph;
    nanoem_model_morph_bone_t *morph_bone;
    nanoem_model_rigid_body_t *body;
    nanoem_model_vertex_t *vertex;
    nanoem_rsize_t num_objects, num_items, i, j;
    num_objects = origin_model->num_vertices;
    for (i = 0; i < num_objects; i++) {
        vertex = origin_model->vertices[i];
        for (j = 0; j < 4; j++) {
            nanoemMutableModelObjectApplyChangeObjectIndex(&vertex->bone_indices[j], bone_index, delta);
        }
    }
    num_objects = origin_model->num_constraints;
    for (i = 0; i < num_objects; i++) {
        constraint = origin_model->constraints[i];
        num_items = constraint->num_joints;
        nanoemMutableModelObjectApplyChangeObjectIndex(&constraint->effector_bone_index, bone_index, delta);
        nanoemMutableModelObjectApplyChangeObjectIndex(&constraint->target_bone_index, bone_index, delta);
        for (j = 0; j < num_items; j++) {
            joint = constraint->joints[j];
            nanoemMutableModelObjectApplyChangeObjectIndex(&joint->bone_index, bone_index, delta);
        }
    }
    num_objects = origin_model->num_morphs;
    for (i = 0; i < num_objects; i++) {
        morph = origin_model->morphs[i];
        if (nanoemModelMorphGetType(morph) == NANOEM_MODEL_MORPH_TYPE_BONE) {
            num_items = morph->num_objects;
            for (j = 0; j < num_items; j++) {
                morph_bone = morph->u.bones[j];
                nanoemMutableModelObjectApplyChangeObjectIndex(&morph_bone->bone_index, bone_index, delta);
            }
        }
    }
    num_objects = origin_model->num_bones;
    for (i = 0; i < num_objects; i++) {
        bone = origin_model->bones[i];
        nanoemMutableModelObjectApplyChangeObjectIndex(&bone->parent_bone_index, bone_index, delta);
        nanoemMutableModelObjectApplyChangeObjectIndex(&bone->parent_inherent_bone_index, bone_index, delta);
        nanoemMutableModelObjectApplyChangeObjectIndex(&bone->effector_bone_index, bone_index, delta);
        nanoemMutableModelObjectApplyChangeObjectIndex(&bone->target_bone_index, bone_index, delta);
        constraint = bone->constraint;
        if (constraint) {
            num_items = constraint->num_joints;
            nanoemMutableModelObjectApplyChangeObjectIndex(&constraint->effector_bone_index, bone_index, delta);
            nanoemMutableModelObjectApplyChangeObjectIndex(&constraint->target_bone_index, bone_index, delta);
            for (j = 0; j < num_items; j++) {
                joint = constraint->joints[j];
                nanoemMutableModelObjectApplyChangeObjectIndex(&joint->bone_index, bone_index, delta);
            }
        }
    }
    num_objects = origin_model->num_rigid_bodies;
    for (i = 0; i < num_objects; i++) {
        body = origin_model->rigid_bodies[i];
        nanoemMutableModelObjectApplyChangeObjectIndex(&body->bone_index, bone_index, delta);
    }
}

static void
nanoemMutableModelMorphApplyChangeAllObjectIndices(nanoem_model_t *origin_model, int morph_index, int delta)
{
    nanoem_model_morph_t *morph;
    nanoem_model_morph_flip_t *flip;
    nanoem_model_morph_group_t *group;
    nanoem_model_morph_type_t type;
    nanoem_rsize_t num_objects, num_items, i, j;
    num_objects = origin_model->num_morphs;
    for (i = 0; i < num_objects; i++) {
        morph = origin_model->morphs[i];
        type = nanoemModelMorphGetType(morph);
        if (type == NANOEM_MODEL_MORPH_TYPE_GROUP) {
            num_items = morph->num_objects;
            for (j = 0; j < num_items; j++) {
                group = morph->u.groups[j];
                nanoemMutableModelObjectApplyChangeObjectIndex(&group->morph_index, morph_index, delta);
            }
        }
        else if (type == NANOEM_MODEL_MORPH_TYPE_FLIP) {
            num_items = morph->num_objects;
            for (j = 0; j < num_items; j++) {
                flip = morph->u.flips[j];
                nanoemMutableModelObjectApplyChangeObjectIndex(&flip->morph_index, morph_index, delta);
            }
        }
    }
}

static void
nanoemMutableModelRigidBodyApplyChangeAllObjectIndices(nanoem_model_t *origin_model, int rigid_body_index, int delta)
{
    nanoem_model_morph_t *morph;
    nanoem_model_morph_impulse_t *impulse;
    nanoem_model_joint_t *joint;
    nanoem_model_soft_body_t *soft_body;
    nanoem_model_soft_body_anchor_t *anchor;
    nanoem_model_morph_type_t type;
    nanoem_rsize_t num_objects, num_items, i, j;
    num_objects = origin_model->num_morphs;
    for (i = 0; i < num_objects; i++) {
        morph = origin_model->morphs[i];
        type = nanoemModelMorphGetType(morph);
        if (type == NANOEM_MODEL_MORPH_TYPE_IMPULUSE) {
            num_items = morph->num_objects;
            for (j = 0; j < num_items; j++) {
                impulse = morph->u.impulses[j];
                nanoemMutableModelObjectApplyChangeObjectIndex(&impulse->rigid_body_index, rigid_body_index, delta);
            }
        }
    }
    num_objects = origin_model->num_joints;
    for (i = 0; i < num_objects; i++) {
        joint = origin_model->joints[i];
        nanoemMutableModelObjectApplyChangeObjectIndex(&joint->rigid_body_a_index, rigid_body_index, delta);
        nanoemMutableModelObjectApplyChangeObjectIndex(&joint->rigid_body_b_index, rigid_body_index, delta);
    }
    num_objects = origin_model->num_soft_bodies;
    for (i = 0; i < num_objects; i++) {
        soft_body = origin_model->soft_bodies[i];
        num_items = soft_body->num_anchors;
        for (j = 0; j < num_items; j++) {
            anchor = soft_body->anchors[j];
            nanoemMutableModelObjectApplyChangeObjectIndex(&anchor->rigid_body_index, rigid_body_index, delta);
        }
    }
}

static void
nanoemMutableModelTextureApplyChangeAllObjectIndices(nanoem_model_t *origin_model, int texture_index, int delta)
{
    nanoem_model_material_t *material;
    nanoem_rsize_t num_objects, i;
    num_objects = origin_model->num_materials;
    for (i = 0; i < num_objects; i++) {
        material = origin_model->materials[i];
        nanoemMutableModelObjectApplyChangeObjectIndex(&material->diffuse_texture_index, texture_index, delta);
        nanoemMutableModelObjectApplyChangeObjectIndex(&material->sphere_map_texture_index, texture_index, delta);
        if (!material->is_toon_shared) {
            nanoemMutableModelObjectApplyChangeObjectIndex(&material->toon_texture_index, texture_index, delta);
        }
    }
}

static int
nanoemModelResolveMaterialObject(const nanoem_model_t *model, const nanoem_model_material_t *value)
{
    const nanoem_unicode_string_t *name;
    const nanoem_model_object_t *object;
    nanoem_unicode_string_factory_t *factory;
    nanoem_rsize_t num_materials, i;
    int object_index = NANOEM_MODEL_OBJECT_NOT_FOUND;
    if (nanoem_is_not_null(value)) {
        object = nanoemModelMaterialGetModelObject(value);
        if (nanoemModelObjectGetParentModel(object) == model) {
            object_index = nanoemModelObjectGetIndex(object);
        }
        else if (model) {
            name = value->name_ja;
            factory = model->factory;
            num_materials = model->num_materials;
            for (i = 0; i < num_materials; i++) {
                nanoem_model_material_t *material = model->materials[i];
                if (nanoemUnicodeStringFactoryCompareString(factory, material->name_ja, name) == 0) {
                    object_index = i;
                    break;
                }
            }
        }
    }
    return object_index;
}

static int
nanoemModelResolveBoneObject(const nanoem_model_t *model, const nanoem_model_bone_t *value)
{
    const nanoem_unicode_string_t *name;
    const nanoem_model_object_t *object;
    nanoem_unicode_string_factory_t *factory;
    nanoem_rsize_t num_bones, i;
    int object_index = NANOEM_MODEL_OBJECT_NOT_FOUND;
    if (nanoem_is_not_null(value)) {
        object = nanoemModelBoneGetModelObject(value);
        if (nanoemModelObjectGetParentModel(object) == model) {
            object_index = nanoemModelObjectGetIndex(object);
        }
        else if (nanoem_is_not_null(model)) {
            name = value->name_ja;
            factory = model->factory;
            num_bones = model->num_bones;
            for (i = 0; i < num_bones; i++) {
                nanoem_model_bone_t *bone = model->bones[i];
                if (nanoemUnicodeStringFactoryCompareString(factory, bone->name_ja, name) == 0) {
                    object_index = i;
                    break;
                }
            }
        }
    }
    return object_index;
}

static int
nanoemModelResolveMorphObject(const nanoem_model_t *model, const nanoem_model_morph_t *value)
{
    const nanoem_unicode_string_t *name;
    const nanoem_model_object_t *object;
    nanoem_unicode_string_factory_t *factory;
    nanoem_rsize_t num_morphs, i;
    int object_index = NANOEM_MODEL_OBJECT_NOT_FOUND;
    if (nanoem_is_not_null(value)) {
        object = nanoemModelMorphGetModelObject(value);
        if (nanoemModelObjectGetParentModel(object) == model) {
            object_index = nanoemModelObjectGetIndex(object);
        }
        else if (nanoem_is_not_null(model)) {
            name = value->name_ja;
            factory = model->factory;
            num_morphs = model->num_morphs;
            for (i = 0; i < num_morphs; i++) {
                nanoem_model_morph_t *morph = model->morphs[i];
                if (nanoemUnicodeStringFactoryCompareString(factory, morph->name_ja, name) == 0) {
                    object_index = i;
                    break;
                }
            }
        }
    }
    return object_index;
}

static int
nanoemModelResolveRigidBodyObject(const nanoem_model_t *model, const nanoem_model_rigid_body_t *value)
{
    const nanoem_unicode_string_t *name;
    const nanoem_model_object_t *object;
    nanoem_unicode_string_factory_t *factory;
    nanoem_rsize_t num_rigid_bodys, i;
    int object_index = NANOEM_MODEL_OBJECT_NOT_FOUND;
    if (nanoem_is_not_null(value)) {
        object = nanoemModelRigidBodyGetModelObject(value);
        if (nanoemModelObjectGetParentModel(object) == model) {
            object_index = nanoemModelObjectGetIndex(object);
        }
        else if (nanoem_is_not_null(model)) {
            name = value->name_ja;
            factory = model->factory;
            num_rigid_bodys = model->num_rigid_bodies;
            for (i = 0; i < num_rigid_bodys; i++) {
                nanoem_model_rigid_body_t *rigid_body = model->rigid_bodies[i];
                if (nanoemUnicodeStringFactoryCompareString(factory, rigid_body->name_ja, name) == 0) {
                    object_index = i;
                    break;
                }
            }
        }
    }
    return object_index;
}

static int
nanoemMutableModelBoneResolveBoneObject(nanoem_mutable_model_bone_t *bone, const nanoem_model_bone_t *value)
{
    const nanoem_model_t *parent_model = nanoemModelBoneGetParentModel(bone->origin);
    return nanoem_is_not_null(bone) ? nanoemModelResolveBoneObject(parent_model, value) : NANOEM_MODEL_OBJECT_NOT_FOUND;
}

static int
nanoemMutableModelConstraintResolveBoneObject(nanoem_mutable_model_constraint_t *constraint, const nanoem_model_bone_t *value)
{
    const nanoem_model_t *parent_model = nanoemModelConstraintGetParentModel(constraint->origin);
    return nanoem_is_not_null(constraint) ? nanoemModelResolveBoneObject (parent_model, value) : NANOEM_MODEL_OBJECT_NOT_FOUND;
}

static void
nanoemMutableMotionAssignGlobalTrackId(nanoem_motion_t *motion, const nanoem_unicode_string_t *value, nanoem_motion_track_index_t *output, nanoem_status_t *status, int *ret)
{
    nanoem_unicode_string_factory_t *factory = motion->factory;
    nanoem_unicode_string_t *found_name;
    nanoem_unicode_string_t *new_name = nanoemUnicodeStringFactoryCloneString(factory, value, status);
    if (!motion->global_motion_track_bundle) {
        motion->global_motion_track_bundle = kh_init_motion_track_bundle();
    }
    *output = nanoemMotionResolveGlobalTrackId(motion, new_name, &found_name, ret);
    if (nanoem_unlikely(found_name)) {
        nanoemUtilDestroyString(new_name, factory);
    }
}

static nanoem_bool_t
nanoemMutableBufferEnsureSize(nanoem_mutable_buffer_t *buffer, nanoem_rsize_t required, nanoem_status_t *status)
{
    nanoem_rsize_t allocated_length, actual_length;
    if (nanoem_is_not_null(buffer)) {
        allocated_length = buffer->allocated_length;
        actual_length = buffer->actual_length;
        if (actual_length + required > allocated_length) {
            allocated_length = (allocated_length + required) << 1;
            if (allocated_length > actual_length) {
                buffer->data = (nanoem_u8_t *) nanoem_realloc(buffer->data, allocated_length, status);
                if (nanoem_is_null(buffer->data)) {
                    buffer->actual_length = 0;
                    buffer->offset = 0;
                    return nanoem_false;
                }
            }
            else {
                return nanoem_false;
            }
        }
        buffer->allocated_length = allocated_length;
        buffer->actual_length = actual_length + required;
        return nanoem_true;
    }
    return nanoem_false;
}

void
nanoemMutableMotionBoneKeyframeSetName(nanoem_mutable_motion_bone_keyframe_t *keyframe, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_motion_t *motion;
    nanoem_unicode_string_factory_t *factory;
    nanoem_unicode_string_t *new_name, *found_name;
    int ret;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        motion = keyframe->origin->base.parent_motion;
        factory = motion->factory;
        new_name = nanoemUnicodeStringFactoryCloneString(factory, value, status);
        keyframe->origin->bone_id = nanoemMotionResolveLocalBoneTrackId(motion, new_name, &found_name, &ret);
        if (found_name) {
            nanoemUtilDestroyString(new_name, factory);
        }
    }
}

void
nanoemMutableMotionMorphKeyframeSetName(nanoem_mutable_motion_morph_keyframe_t *keyframe, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_motion_t *motion;
    nanoem_unicode_string_factory_t *factory;
    nanoem_unicode_string_t *new_name, *found_name;
    int ret;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        motion = keyframe->origin->base.parent_motion;
        factory = motion->factory;
        new_name = nanoemUnicodeStringFactoryCloneString(factory, value, status);
        keyframe->origin->morph_id = nanoemMotionResolveLocalMorphTrackId(motion, new_name, &found_name, &ret);
        if (found_name) {
            nanoemUtilDestroyString(new_name, factory);
        }
    }
}

static nanoem_mutable_motion_effect_parameter_t *
nanoemMutableMotionEffectParameterCreateInternal(nanoem_motion_effect_parameter_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_motion, nanoem_status_t *status)
{
    nanoem_mutable_motion_effect_parameter_t *parameter = NULL;
    if (nanoem_is_not_null(origin)) {
        parameter = (nanoem_mutable_motion_effect_parameter_t *) nanoem_calloc(1, sizeof(*parameter), status);
        if (nanoem_is_not_null(parameter)) {
            parameter->origin = origin;
            parameter->base.is_reference = is_reference;
            parameter->base.is_in_motion = is_in_motion;
        }
    }
    return parameter;
}

static nanoem_mutable_motion_outside_parent_t *
nanoemMutableMotionOutsideParentCreateInternal(nanoem_motion_outside_parent_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_motion, nanoem_status_t *status)
{
    nanoem_mutable_motion_outside_parent_t *op = NULL;
    if (nanoem_is_not_null(origin)) {
        op = (nanoem_mutable_motion_outside_parent_t *) nanoem_calloc(1, sizeof(*op), status);
        if (nanoem_is_not_null(op)) {
            op->origin = origin;
            op->base.is_reference = is_reference;
            op->base.is_in_motion = is_in_motion;
        }
    }
    return op;
}

static nanoem_mutable_motion_accessory_keyframe_t *
nanoemMutableMotionAccessoryKeyframeCreateInternal(nanoem_motion_accessory_keyframe_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_motion, nanoem_status_t *status)
{
    nanoem_mutable_motion_accessory_keyframe_t *keyframe = NULL;
    nanoem_rsize_t num_effect_parameters;
    if (nanoem_is_not_null(origin)) {
        keyframe = (nanoem_mutable_motion_accessory_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
        if (nanoem_is_not_null(keyframe)) {
            keyframe->origin = origin;
            keyframe->base.is_reference = is_reference;
            keyframe->base.is_in_motion = is_in_motion;
            num_effect_parameters = origin->num_effect_parameters;
            if (num_effect_parameters > 0) {
                keyframe->num_allocated_effect_parameters = num_effect_parameters;
            }
            else if (!keyframe->origin->effect_parameters) {
                keyframe->origin->effect_parameters = (nanoem_motion_effect_parameter_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*keyframe->origin->effect_parameters), status);
                keyframe->num_allocated_effect_parameters = __nanoem_default_allocation_size;
            }
        }
    }
    return keyframe;
}

static nanoem_mutable_motion_bone_keyframe_t *
nanoemMutableMotionBoneKeyframeCreateInternal(nanoem_motion_bone_keyframe_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_motion, nanoem_status_t *status)
{
    nanoem_mutable_motion_bone_keyframe_t *keyframe = NULL;
    if (nanoem_is_not_null(origin)) {
        keyframe = (nanoem_mutable_motion_bone_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
        if (nanoem_is_not_null(keyframe)) {
            keyframe->origin = origin;
            keyframe->base.is_reference = is_reference;
            keyframe->base.is_in_motion = is_in_motion;
        }
    }
    return keyframe;
}

static void
nanoemMotionBoneKeyframeSaveToBuffer(const nanoem_motion_bone_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_unicode_string_t *name;
    kh_motion_track_bundle_t *track_bundle;
    nanoem_unicode_string_factory_t *factory;
    nanoem_u8_t name_buffer[VMD_BONE_KEYFRAME_NAME_LENGTH], *name_cp932;
    nanoem_rsize_t length, i;
    int j;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(buffer)) {
        factory = keyframe->base.parent_motion->factory;
        track_bundle = keyframe->base.parent_motion->local_bone_motion_track_bundle;
        name = nanoemMotionTrackBundleResolveName(track_bundle, keyframe->bone_id);
        name_cp932 = nanoemUnicodeStringFactoryGetByteArrayEncoding(factory, name, &length, NANOEM_CODEC_TYPE_SJIS, status);
        if (!nanoem_status_ptr_has_error(status) && nanoem_is_not_null(name_cp932)) {
            nanoem_crt_memset(name_buffer, 0, sizeof(name_buffer));
            nanoemUtilCopyString((char *) name_buffer, sizeof(name_buffer), (const char *) name_cp932, length);
        }
        nanoemUnicodeStringFactoryDestroyByteArray(factory, name_cp932);
        nanoemMutableBufferWriteByteArray(buffer, name_buffer, VMD_BONE_KEYFRAME_NAME_LENGTH, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, keyframe->base.frame_index, status);
        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &keyframe->translation, status);
        nanoemMutableBufferWriteFloat32x4LittleEndian(buffer, &keyframe->orientation, status);
        for (i = 0; i < 4; i++) {
            for (j = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM; j < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; j++) {
                nanoemMutableBufferWriteByte(buffer, keyframe->interplation[j].u.values[i], status);
            }
        }
        for (i = 0; i < 48; i++) {
            nanoemMutableBufferWriteByte(buffer, 0, status);
        }
    }
}

static nanoem_mutable_motion_camera_keyframe_t *
nanoemMutableMotionCameraKeyframeCreateInternal(nanoem_motion_camera_keyframe_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_motion, nanoem_status_t *status)
{
    nanoem_mutable_motion_camera_keyframe_t *keyframe = NULL;
    if (nanoem_is_not_null(origin)) {
        keyframe = (nanoem_mutable_motion_camera_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
        if (nanoem_is_not_null(keyframe)) {
            keyframe->origin = origin;
            keyframe->base.is_reference = is_reference;
            keyframe->base.is_in_motion = is_in_motion;
        }
    }
    return keyframe;
}

static void
nanoemMotionCameraKeyframeSaveToBuffer(const nanoem_motion_camera_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_rsize_t i, j;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(buffer)) {
        nanoemMutableBufferWriteInt32LittleEndian(buffer, keyframe->base.frame_index, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, keyframe->distance, status);
        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &keyframe->look_at, status);
        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &keyframe->angle, status);
        for (i = 0; i < 4; i++) {
            for (j = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM; j < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; j++) {
                nanoemMutableBufferWriteByte(buffer, keyframe->interplation[j].u.values[i], status);
            }
        }
        nanoemMutableBufferWriteInt32LittleEndian(buffer, keyframe->fov, status);
        nanoemMutableBufferWriteByte(buffer, keyframe->is_perspective_view == 0, status);
    }
}

static nanoem_mutable_motion_light_keyframe_t *
nanoemMutableMotionLightKeyframeCreateInternal(nanoem_motion_light_keyframe_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_motion, nanoem_status_t *status)
{
    nanoem_mutable_motion_light_keyframe_t *keyframe = NULL;
    if (nanoem_is_not_null(origin)) {
        keyframe = (nanoem_mutable_motion_light_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
        if (nanoem_is_not_null(keyframe)) {
            keyframe->origin = origin;
            keyframe->base.is_reference = is_reference;
            keyframe->base.is_in_motion = is_in_motion;
        }
    }
    return keyframe;
}

static void
nanoemMotionLightKeyframeSaveToBuffer(nanoem_motion_light_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(buffer)) {
        nanoemMutableBufferWriteInt32LittleEndian(buffer, keyframe->base.frame_index, status);
        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &keyframe->color, status);
        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &keyframe->direction, status);
    }
}

static nanoem_mutable_motion_model_keyframe_t *
nanoemMutableMotionModelKeyframeCreateInternal(nanoem_motion_model_keyframe_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_motion, nanoem_status_t *status)
{
    nanoem_mutable_motion_model_keyframe_t *keyframe = NULL;
    nanoem_rsize_t num_constraint_states = 0, num_outside_parents = 0, num_effect_parameters = 0;
    if (nanoem_is_not_null(origin)) {
        keyframe = (nanoem_mutable_motion_model_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
        if (nanoem_is_not_null(keyframe)) {
            keyframe->origin = origin;
            num_constraint_states = origin->num_constraint_states;
            if (num_constraint_states > 0) {
                keyframe->num_allocated_constraint_states = num_constraint_states;
            }
            else if (!origin->constraint_states) {
                origin->constraint_states = (nanoem_motion_model_keyframe_constraint_state_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->constraint_states), status);
                keyframe->num_allocated_constraint_states = __nanoem_default_allocation_size;
            }
            num_outside_parents = origin->num_outside_parents;
            if (num_outside_parents > 0) {
                keyframe->num_allocated_outside_parents = num_outside_parents;
            }
            else if (!origin->outside_parents) {
                origin->outside_parents = (nanoem_motion_outside_parent_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->outside_parents), status);
                keyframe->num_allocated_outside_parents = __nanoem_default_allocation_size;
            }
            num_effect_parameters = origin->num_effect_parameters;
            if (num_effect_parameters > 0) {
                keyframe->num_allocated_effect_parameters = num_effect_parameters;
            }
            else if (!origin->effect_parameters) {
                origin->effect_parameters = (nanoem_motion_effect_parameter_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->effect_parameters), status);
                keyframe->num_allocated_effect_parameters = __nanoem_default_allocation_size;
            }
            keyframe->base.is_reference = is_reference;
            keyframe->base.is_in_motion = is_in_motion;
        }
    }
    return keyframe;
}

static void
nanoemMotionModelKeyframeSaveToBuffer(nanoem_motion_model_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_mutable_motion_model_keyframe_constraint_state_t state;
    nanoem_rsize_t i, num_constraint_states;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(buffer)) {
        nanoemMutableBufferWriteInt32LittleEndian(buffer, keyframe->base.frame_index, status);
        nanoemMutableBufferWriteByte(buffer, keyframe->visible, status);
        if (nanoem_is_not_null(keyframe->constraint_states)) {
            num_constraint_states = keyframe->num_constraint_states;
            nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_constraint_states, status);
            state.base.is_in_motion = nanoem_false;
            state.base.is_reference = nanoem_true;
            for (i = 0; i < num_constraint_states; i++) {
                state.origin = keyframe->constraint_states[i];
                nanoemMutableMotionModelKeyframeConstraintStateSaveToBuffer(&state, buffer, status);
            }
        }
        else {
            nanoemMutableBufferWriteInt32LittleEndian(buffer, 0, status);
        }
    }
}

static nanoem_mutable_motion_model_keyframe_constraint_state_t *
nanoemMutableMotionModelKeyframeConstraintStateCreateInternal(nanoem_motion_model_keyframe_constraint_state_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_motion, nanoem_status_t *status)
{
    nanoem_mutable_motion_model_keyframe_constraint_state_t *state = NULL;
    if (nanoem_is_not_null(origin)) {
        state = (nanoem_mutable_motion_model_keyframe_constraint_state_t *) nanoem_calloc(1, sizeof(*state), status);
        if (nanoem_is_not_null(state)) {
            state->origin = origin;
            state->base.is_reference = is_reference;
            state->base.is_in_motion = is_in_motion;
        }
    }
    return state;
}

static nanoem_mutable_motion_morph_keyframe_t *
nanoemMutableMotionMorphKeyframeCreateInternal(nanoem_motion_morph_keyframe_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_motion, nanoem_status_t *status)
{
    nanoem_mutable_motion_morph_keyframe_t *keyframe = NULL;
    if (nanoem_is_not_null(origin)) {
        keyframe = (nanoem_mutable_motion_morph_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
        if (nanoem_is_not_null(keyframe)) {
            keyframe->origin = origin;
            keyframe->base.is_reference = is_reference;
            keyframe->base.is_in_motion = is_in_motion;
        }
    }
    return keyframe;
}

static void
nanoemMotionMorphKeyframeSaveToBuffer(const nanoem_motion_morph_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_unicode_string_t *name;
    kh_motion_track_bundle_t *track_bundle;
    nanoem_unicode_string_factory_t *factory;
    nanoem_u8_t name_buffer[VMD_BONE_KEYFRAME_NAME_LENGTH], *name_cp932;
    nanoem_rsize_t length;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(buffer)) {
        factory = keyframe->base.parent_motion->factory;
        track_bundle = keyframe->base.parent_motion->local_morph_motion_track_bundle;
        name = nanoemMotionTrackBundleResolveName(track_bundle, keyframe->morph_id);
        name_cp932 = nanoemUnicodeStringFactoryGetByteArrayEncoding(factory, name, &length, NANOEM_CODEC_TYPE_SJIS, status);
        if (!nanoem_status_ptr_has_error(status) && nanoem_is_not_null(name_cp932)) {
            nanoem_crt_memset(name_buffer, 0, sizeof(name_buffer));
            nanoemUtilCopyString((char *) name_buffer, sizeof(name_buffer), (const char *) name_cp932, length);
        }
        nanoemUnicodeStringFactoryDestroyByteArray(factory, name_cp932);
        nanoemMutableBufferWriteByteArray(buffer, name_buffer, VMD_MORPH_KEYFRAME_NAME_LENGTH, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, keyframe->base.frame_index, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, keyframe->weight, status);
    }
}

static nanoem_mutable_motion_self_shadow_keyframe_t *
nanoemMutableMotionSelfShadowKeyframeCreateInternal(nanoem_motion_self_shadow_keyframe_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_motion, nanoem_status_t *status)
{
    nanoem_mutable_motion_self_shadow_keyframe_t *keyframe = NULL;
    if (nanoem_is_not_null(origin)) {
        keyframe = (nanoem_mutable_motion_self_shadow_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
        if (nanoem_is_not_null(keyframe)) {
            keyframe->origin = origin;
            keyframe->base.is_reference = is_reference;
            keyframe->base.is_in_motion = is_in_motion;
        }
    }
    return keyframe;
}

static void
nanoemMotionSelfShadowKeyframeSaveToBuffer(const nanoem_motion_self_shadow_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(buffer)) {
        nanoemMutableBufferWriteInt32LittleEndian(buffer, keyframe->base.frame_index, status);
        nanoemMutableBufferWriteByte(buffer, keyframe->mode, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, keyframe->distance, status);
    }
}

static void
nanoemMotionTrackBundleRemoveKeyframe(kh_motion_track_bundle_t *bundle, nanoem_frame_index_t frame_index, nanoem_unicode_string_t *name, nanoem_unicode_string_factory_t *factory)
{
    kh_keyframe_map_t *keyframes = NULL;
    nanoem_motion_track_t track;
    khiter_t it;
    if (nanoem_is_not_null(bundle) && nanoem_is_not_null(name) && nanoem_is_not_null(factory)) {
        track.factory = factory;
        track.id = 0;
        track.keyframes = NULL;
        track.name = name;
        it = kh_get_motion_track_bundle(bundle, track);
        if (it != kh_end(bundle)) {
            keyframes = kh_key(bundle, it).keyframes;
            kh_del_keyframe_map(keyframes, kh_get_keyframe_map(keyframes, frame_index));
        }
    }
}

static void
nanoemMutableMotionSaveToBufferBoneKeyframeBlockVMD(nanoem_mutable_motion_t *motion, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_motion_t *origin_motion = motion->origin;
    nanoem_motion_bone_keyframe_t *bone_keyframe;
    nanoem_rsize_t num_keyframes = origin_motion->num_bone_keyframes, i;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_keyframes, status);
    for (i = 0; i < num_keyframes; i++) {
        bone_keyframe = origin_motion->bone_keyframes[i];
        nanoemMotionBoneKeyframeSaveToBuffer(bone_keyframe, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            break;
        }
    }
}

static void
nanoemMutableMotionSaveToBufferMorphKeyframeBlockVMD(nanoem_mutable_motion_t *motion, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_motion_t *origin_motion = motion->origin;
    nanoem_motion_morph_keyframe_t *morph_keyframe;
    nanoem_rsize_t num_keyframes = origin_motion->num_morph_keyframes, i;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_keyframes, status);
    for (i = 0; i < num_keyframes; i++) {
        morph_keyframe = origin_motion->morph_keyframes[i];
        nanoemMotionMorphKeyframeSaveToBuffer(morph_keyframe, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            break;
        }
    }
}

static void
nanoemMutableMotionSaveToBufferCameraKeyframeBlockVMD(nanoem_mutable_motion_t *motion, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_motion_t *origin_motion = motion->origin;
    nanoem_motion_camera_keyframe_t *camera_keyframe;
    nanoem_rsize_t num_keyframes = origin_motion->num_camera_keyframes, i;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_keyframes, status);
    for (i = 0; i < num_keyframes; i++) {
        camera_keyframe = origin_motion->camera_keyframes[i];
        nanoemMotionCameraKeyframeSaveToBuffer(camera_keyframe, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            break;
        }
    }
}

static void
nanoemMutableMotionSaveToBufferLightKeyframeBlockVMD(nanoem_mutable_motion_t *motion, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_motion_t *origin_motion = motion->origin;
    nanoem_motion_light_keyframe_t *light_keyframe;
    nanoem_rsize_t num_keyframes = origin_motion->num_light_keyframes, i;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_keyframes, status);
    for (i = 0; i < num_keyframes; i++) {
        light_keyframe = origin_motion->light_keyframes[i];
        nanoemMotionLightKeyframeSaveToBuffer(light_keyframe, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            break;
        }
    }
}

static void
nanoemMutableMotionSaveToBufferSelfShadowKeyframeBlockVMD(nanoem_mutable_motion_t *motion, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_motion_t *origin_motion = motion->origin;
    nanoem_motion_self_shadow_keyframe_t *self_shadow_keyframe;
    nanoem_rsize_t num_keyframes = origin_motion->num_self_shadow_keyframes, i;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_keyframes, status);
    for (i = 0; i < num_keyframes; i++) {
        self_shadow_keyframe = origin_motion->self_shadow_keyframes[i];
        nanoemMotionSelfShadowKeyframeSaveToBuffer(self_shadow_keyframe, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            break;
        }
    }
}

static void
nanoemMutableMotionSaveToBufferModelKeyframeBlockVMD(nanoem_mutable_motion_t *motion, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_motion_t *origin_motion = motion->origin;
    nanoem_motion_model_keyframe_t *model_keyframe;
    nanoem_rsize_t num_keyframes = origin_motion->num_model_keyframes, i;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_keyframes, status);
    for (i = 0; i < num_keyframes; i++) {
        model_keyframe = origin_motion->model_keyframes[i];
        nanoemMotionModelKeyframeSaveToBuffer(model_keyframe, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            break;
        }
    }
}

static void
nanoemMutableBufferWriteByteArrayCallback(void *opaque, nanoem_mutable_buffer_t *buffer, const nanoem_u8_t *data, nanoem_rsize_t len, nanoem_status_t *status)
{
    nanoem_mark_unused(opaque);
    if (nanoemMutableBufferEnsureSize(buffer, len, status)) {
        nanoem_crt_memcpy(buffer->data + buffer->offset, data, len);
        buffer->offset += len;
        nanoem_status_ptr_assign_succeeded(status);
    }
}

nanoem_mutable_buffer_t *APIENTRY
nanoemMutableBufferCreate(nanoem_status_t *status)
{
    return nanoemMutableBufferCreateWithReservedSize(2 << 12, status);
}

nanoem_mutable_buffer_t *APIENTRY
nanoemMutableBufferCreateWithReservedSize(nanoem_rsize_t capacity, nanoem_status_t *status)
{
    nanoem_mutable_buffer_t *buffer;
    buffer = (nanoem_mutable_buffer_t *) nanoem_calloc(1, sizeof(*buffer), status);
    if (nanoem_is_not_null(buffer)) {
        buffer->allocated_length = nanoem_unlikely(capacity < 8) ? 8 : capacity;
        buffer->data = (nanoem_u8_t *) nanoem_malloc(buffer->allocated_length, status);
        buffer->write_byte_array = nanoemMutableBufferWriteByteArrayCallback;
    }
    return buffer;
}

void APIENTRY
nanoemMutableBufferWriteByte(nanoem_mutable_buffer_t *buffer, nanoem_u8_t value, nanoem_status_t *status)
{
    nanoemMutableBufferWriteByteArray(buffer, &value, sizeof(value), status);
}

void APIENTRY
nanoemMutableBufferWriteInt16LittleEndian(nanoem_mutable_buffer_t *buffer, nanoem_i16_t value, nanoem_status_t *status)
{
    nanoem_u8_t bytes[sizeof(value)];
    bytes[0] = ((value & 0x00ff) >> 0) & 0xff;
    bytes[1] = ((value & 0xff00) >> 8) & 0xff;
    nanoemMutableBufferWriteByteArray(buffer, bytes, sizeof(bytes), status);
}

void APIENTRY
nanoemMutableBufferWriteInt16LittleEndianUnsigned(nanoem_mutable_buffer_t *buffer, nanoem_u16_t value, nanoem_status_t *status)
{
    nanoem_u8_t bytes[sizeof(value)];
    bytes[0] = ((value & 0x00ff) >> 0) & 0xff;
    bytes[1] = ((value & 0xff00) >> 8) & 0xff;
    nanoemMutableBufferWriteByteArray(buffer, bytes, sizeof(bytes), status);
}

void APIENTRY
nanoemMutableBufferWriteInt32LittleEndian(nanoem_mutable_buffer_t *buffer, nanoem_i32_t value, nanoem_status_t *status)
{
    nanoem_u8_t bytes[sizeof(value)];
    bytes[0] = ((value & 0x000000ff) >> 0) & 0xff;
    bytes[1] = ((value & 0x0000ff00) >> 8) & 0xff;
    bytes[2] = ((value & 0x00ff0000) >> 16) & 0xff;
    bytes[3] = ((value & 0xff000000) >> 24) & 0xff;
    nanoemMutableBufferWriteByteArray(buffer, bytes, sizeof(value), status);
}

void APIENTRY
nanoemMutableBufferWriteFloat32LittleEndian(nanoem_mutable_buffer_t *buffer, nanoem_f32_t value, nanoem_status_t *status)
{
    union nanoem_float_to_int_cast_t {
        nanoem_i32_t i;
        nanoem_f32_t f;
    } u;
    u.f = value;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, u.i, status);
}

void APIENTRY
nanoemMutableBufferWriteByteArray(nanoem_mutable_buffer_t *buffer, const nanoem_u8_t *data, nanoem_rsize_t len, nanoem_status_t *status)
{
    if (nanoem_is_null(buffer) || nanoem_is_null(data)) {
        nanoem_status_ptr_assign_null_object(status);
    }
    else if (nanoem_likely(len > 0) && !nanoem_status_ptr_has_error(status)) {
        buffer->write_byte_array(buffer->opaque, buffer, data, len, status);
    }
}

void APIENTRY
nanoemMutableBufferWriteUnicodeString(nanoem_mutable_buffer_t *buffer, const nanoem_unicode_string_t *value, nanoem_unicode_string_factory_t *factory, nanoem_codec_type_t codec, nanoem_status_t *status)
{
    nanoem_rsize_t length;
    nanoem_u8_t *bytes = nanoemUnicodeStringFactoryGetByteArrayEncoding(factory, value, &length, codec, status);
    if (bytes) {
        nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) length, status);
        nanoemMutableBufferWriteByteArray(buffer, bytes, length, status);
        nanoemUnicodeStringFactoryDestroyByteArray(factory, bytes);
    }
    else {
        nanoemMutableBufferWriteInt32LittleEndian(buffer, 0, status);
    }
}

NANOEM_DECL_INLINE static void APIENTRY
nanoemMutableBufferWriteCp932String(nanoem_mutable_buffer_t *buffer, const nanoem_unicode_string_t *value, nanoem_unicode_string_factory_t *factory, nanoem_u8_t *ptr, nanoem_rsize_t limit, nanoem_status_t *status)
{
    nanoem_rsize_t length;
    nanoem_u8_t *bytes = nanoemUnicodeStringFactoryGetByteArrayEncoding(factory, value, &length, NANOEM_CODEC_TYPE_SJIS, status);
    if (!nanoem_status_ptr_has_error(status) && nanoem_is_not_null(bytes)) {
        nanoemUtilCopyString((char *) ptr, limit, (const char *) bytes, length);
        nanoemUnicodeStringFactoryDestroyByteArray(factory, bytes);
    }
    nanoemMutableBufferWriteByteArray(buffer, ptr, limit - 1, status);
}

void APIENTRY
nanoemMutableBufferWriteInteger(nanoem_mutable_buffer_t *buffer, int value, nanoem_rsize_t size, nanoem_status_t *status)
{
    switch (size) {
    case 4:
    default:
        nanoemMutableBufferWriteInt32LittleEndian(buffer, value, status);
        break;
    case 2:
        nanoemMutableBufferWriteInt16LittleEndianUnsigned(buffer, value, status);
        break;
    case 1:
        nanoemMutableBufferWriteByte(buffer, value, status);
        break;
    }
}

nanoem_buffer_t *APIENTRY
nanoemMutableBufferCreateBufferObject(nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    return nanoem_is_not_null(buffer) ? nanoemBufferCreate(buffer->data, buffer->actual_length, status) : NULL;
}

void APIENTRY
nanoemMutableBufferDestroy(nanoem_mutable_buffer_t *buffer)
{
    if (nanoem_is_not_null(buffer)) {
        if (nanoem_is_not_null(buffer->data)) {
            nanoem_free(buffer->data);
        }
        nanoem_free(buffer);
    }
}

static nanoem_mutable_motion_effect_parameter_t *
nanoemMutableMotionEffectParameterCreateCommon(nanoem_motion_effect_parameter_t *origin, nanoem_status_t *status)
{
    nanoem_mutable_motion_effect_parameter_t *parameter = NULL;
    if (nanoem_is_not_null(origin)) {
        parameter = nanoemMutableMotionEffectParameterCreateInternal(origin, nanoem_false, nanoem_false, status);
        if (nanoem_is_null(parameter)) {
            nanoem_free(origin);
        }
    }
    return parameter;
}

nanoem_mutable_motion_effect_parameter_t *APIENTRY
nanoemMutableMotionEffectParameterCreateFromAccessoryKeyframe(nanoem_motion_accessory_keyframe_t *keyframe, nanoem_status_t *status)
{
    return nanoemMutableMotionEffectParameterCreateCommon(nanoemMotionEffectParameterCreateFromAccessoryKeyframe(keyframe, status), status);
}

nanoem_mutable_motion_effect_parameter_t *APIENTRY
nanoemMutableMotionEffectParameterCreateFromModelKeyframe(nanoem_motion_model_keyframe_t *keyframe, nanoem_status_t *status)
{
    return nanoemMutableMotionEffectParameterCreateCommon(nanoemMotionEffectParameterCreateFromModelKeyframe(keyframe, status), status);
}

nanoem_mutable_motion_effect_parameter_t *APIENTRY
nanoemMutableMotionEffectParameterCreateAsReference(nanoem_motion_effect_parameter_t *value, nanoem_status_t *status)
{
    return nanoemMutableMotionEffectParameterCreateInternal(value, nanoem_true, nanoem_false, status);
}

void APIENTRY
nanoemMutableMotionEffectParameterSetName(nanoem_mutable_motion_effect_parameter_t *parameter, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_motion_t *parent_motion = NULL;
    nanoem_motion_effect_parameter_t *origin;
    int ret;
    if (nanoem_is_not_null(parameter)) {
        origin = parameter->origin;
        switch (origin->keyframe_type) {
        case NANOEM_PARENT_KEYFRAME_TYPE_ACCESSORY:
            parent_motion = origin->keyframe.accessory->base.parent_motion;
            break;
        case NANOEM_PARENT_KEYFRAME_TYPE_MODEL:
            parent_motion = origin->keyframe.model->base.parent_motion;
            break;
        default:
            break;
        }
        if (nanoem_is_not_null(parent_motion)) {
            nanoemMutableMotionAssignGlobalTrackId(parent_motion, value, &origin->parameter_id, status, &ret);
        }
    }
}

void APIENTRY
nanoemMutableMotionEffectParameterSetType(nanoem_mutable_motion_effect_parameter_t *parameter, nanoem_motion_effect_parameter_type_t value)
{
    if (nanoem_is_not_null(parameter) && value >= NANOEM_MOTION_EFFECT_PARAMETER_TYPE_FIRST_ENUM && value < NANOEM_MOTION_EFFECT_PARAMETER_TYPE_MAX_ENUM_ENUM) {
        parameter->origin->value_type = value;
    }
}

void APIENTRY
nanoemMutableMotionEffectParameterSetValue(nanoem_mutable_motion_effect_parameter_t *parameter, const void *value)
{
    nanoem_motion_effect_parameter_t *origin;
    if (nanoem_is_not_null(parameter) && nanoem_is_not_null(value)) {
        origin = parameter->origin;
        switch (origin->value_type) {
        case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_BOOL:
        case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_INT:
            origin->value.i = *((nanoem_i32_t *) value);
            break;
        case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_FLOAT:
            origin->value.f = *((nanoem_f32_t *) value);
            break;
        case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_VECTOR4:
            if (origin->value.v.values != value) {
                nanoem_crt_memcpy(origin->value.v.values, value, sizeof(origin->value.v));
            }
            break;
        default:
            break;
        }
    }
}

void APIENTRY
nanoemMutableMotionEffectParameterCopy(nanoem_mutable_motion_effect_parameter_t *new_parameter, const nanoem_motion_effect_parameter_t *parameter, nanoem_status_t *status)
{
    nanoemMutableMotionEffectParameterSetName(new_parameter, nanoemMotionEffectParameterGetName(parameter), status);
    nanoemMutableMotionEffectParameterSetType(new_parameter, nanoemMotionEffectParameterGetType(parameter));
    nanoemMutableMotionEffectParameterSetValue(new_parameter, nanoemMotionEffectParameterGetValue(parameter));
}

nanoem_motion_effect_parameter_t *APIENTRY
nanoemMutableMotionEffectParameterGetOriginObject(nanoem_mutable_motion_effect_parameter_t *parameter)
{
    return nanoem_is_not_null(parameter) ? parameter->origin : NULL;
}

void APIENTRY
nanoemMutableMotionEffectParameterDestroy(nanoem_mutable_motion_effect_parameter_t *parameter)
{
    if (nanoem_is_not_null(parameter)) {
        if (nanoemMutableBaseKeyframeCanDelete(&parameter->base)) {
            nanoemMotionEffectParameterDestroy(parameter->origin);
        }
        nanoem_free(parameter);
    }
}

nanoem_mutable_motion_outside_parent_t *APIENTRY
nanoemMutableMotionOutsideParentCreateFromAccessoryKeyframe(nanoem_motion_accessory_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_mutable_motion_outside_parent_t *op = NULL;
    nanoem_motion_outside_parent_t *origin = nanoemMotionOutsideParentCreateFromAccessoryKeyframe(keyframe, status);
    if (nanoem_is_not_null(origin)) {
        op = nanoemMutableMotionOutsideParentCreateInternal(origin, nanoem_false, nanoem_false, status);
        if (nanoem_is_null(op)) {
            nanoem_free(origin);
        }
    }
    return op;
}

nanoem_mutable_motion_outside_parent_t *APIENTRY
nanoemMutableMotionOutsideParentCreateFromCameraKeyframe(nanoem_motion_camera_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_mutable_motion_outside_parent_t *op = NULL;
    nanoem_motion_outside_parent_t *origin = nanoemMotionOutsideParentCreateFromCameraKeyframe(keyframe, status);
    if (nanoem_is_not_null(origin)) {
        op = nanoemMutableMotionOutsideParentCreateInternal(origin, nanoem_false, nanoem_false, status);
        if (nanoem_is_null(op)) {
            nanoem_free(origin);
        }
    }
    return op;
}

nanoem_mutable_motion_outside_parent_t *APIENTRY
nanoemMutableMotionOutsideParentCreateFromModelKeyframe(nanoem_motion_model_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_mutable_motion_outside_parent_t *op = NULL;
    nanoem_motion_outside_parent_t *origin = nanoemMotionOutsideParentCreateFromModelKeyframe(keyframe, status);
    if (nanoem_is_not_null(origin)) {
        op = nanoemMutableMotionOutsideParentCreateInternal(origin, nanoem_false, nanoem_false, status);
        if (nanoem_is_null(op)) {
            nanoem_free(origin);
        }
    }
    return op;
}

nanoem_mutable_motion_outside_parent_t *APIENTRY
nanoemMutableMotionOutsideParentCreateAsReference(nanoem_motion_outside_parent_t *origin, nanoem_status_t *status)
{
    nanoem_mutable_motion_outside_parent_t *op = NULL;
    nanoem_motion_model_keyframe_t *model_keyframe;
    nanoem_bool_t is_in_motion = nanoem_false;
    if (nanoem_is_not_null(origin)) {
        switch (origin->keyframe_type) {
        case NANOEM_PARENT_KEYFRAME_TYPE_ACCESSORY:
            is_in_motion = origin->keyframe.accessory->outside_parent == origin;
            break;
        case NANOEM_PARENT_KEYFRAME_TYPE_MODEL:
            model_keyframe = origin->keyframe.model;
            is_in_motion = nanoemObjectArrayContainsObject((const void *const *)model_keyframe->outside_parents, origin, model_keyframe->num_outside_parents);
            break;
        default:
            break;
        }
        op = nanoemMutableMotionOutsideParentCreateInternal(origin, nanoem_true, is_in_motion, status);
    }
    return op;
}

nanoem_motion_t *APIENTRY
nanoemMutableMotionOutsideParentGetParentMotionMutable(nanoem_mutable_motion_outside_parent_t *op)
{
    nanoem_motion_t *parent_motion = NULL;
    nanoem_motion_outside_parent_t *origin;
    if (nanoem_is_not_null(op)) {
        origin = op->origin;
        switch (origin->keyframe_type) {
        case NANOEM_PARENT_KEYFRAME_TYPE_ACCESSORY:
            parent_motion = origin->keyframe.accessory->base.parent_motion;
            break;
        case NANOEM_PARENT_KEYFRAME_TYPE_MODEL:
            parent_motion = origin->keyframe.model->base.parent_motion;
            break;
        default:
            break;
        }
    }
    return parent_motion;
}

void APIENTRY
nanoemMutableMotionOutsideParentSetTargetBoneName(nanoem_mutable_motion_outside_parent_t *op, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_motion_t *parent_motion = nanoemMutableMotionOutsideParentGetParentMotionMutable(op);
    nanoem_motion_outside_parent_t *origin;
    int ret;
    if (nanoem_is_not_null(parent_motion) && value) {
        origin = op->origin;
        nanoemMutableMotionAssignGlobalTrackId(parent_motion, value, &origin->global_bone_track_index, status, &ret);
    }
}

void APIENTRY
nanoemMutableMotionOutsideParentSetTargetObjectName(nanoem_mutable_motion_outside_parent_t *op, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_motion_t *parent_motion = nanoemMutableMotionOutsideParentGetParentMotionMutable(op);
    nanoem_motion_outside_parent_t *origin;
    int ret;
    if (nanoem_is_not_null(parent_motion) && value) {
        origin = op->origin;
        nanoemMutableMotionAssignGlobalTrackId(parent_motion, value, &origin->global_model_track_index, status, &ret);
    }
}

void APIENTRY
nanoemMutableMotionOutsideParentSetSubjectBoneName(nanoem_mutable_motion_outside_parent_t *op, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_motion_t *parent_motion = nanoemMutableMotionOutsideParentGetParentMotionMutable(op);
    nanoem_motion_outside_parent_t *origin;
    int ret;
    if (nanoem_is_not_null(parent_motion) && value) {
        origin = op->origin;
        nanoemMutableMotionAssignGlobalTrackId(parent_motion, value, &origin->local_bone_track_index, status, &ret);
    }
}

void APIENTRY
nanoemMutableMotionOutsideParentCopy(nanoem_mutable_motion_outside_parent_t *new_op, const nanoem_motion_outside_parent_t *op, nanoem_status_t *status)
{
    nanoemMutableMotionOutsideParentSetSubjectBoneName(new_op, nanoemMotionOutsideParentGetSubjectBoneName(op), status);
    nanoemMutableMotionOutsideParentSetTargetBoneName(new_op, nanoemMotionOutsideParentGetTargetBoneName(op), status);
    nanoemMutableMotionOutsideParentSetTargetObjectName(new_op, nanoemMotionOutsideParentGetTargetObjectName(op), status);
}

nanoem_motion_outside_parent_t *APIENTRY
nanoemMutableMotionOutsideParentGetOriginObject(nanoem_mutable_motion_outside_parent_t *op)
{
    return nanoem_is_not_null(op) ? op->origin : NULL;
}

void APIENTRY
nanoemMutableMotionOutsideParentDestroy(nanoem_mutable_motion_outside_parent_t *op)
{
    if (nanoem_is_not_null(op)) {
        if (nanoemMutableBaseKeyframeCanDelete(&op->base)) {
            nanoemMotionOutsideParentDestroy(op->origin);
        }
        nanoem_free(op);
    }
}

nanoem_mutable_motion_accessory_keyframe_t *APIENTRY
nanoemMutableMotionAccessoryKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status)
{
    nanoem_motion_accessory_keyframe_t *origin = nanoemMotionAccessoryKeyframeCreate(motion, status);
    return nanoemMutableMotionAccessoryKeyframeCreateInternal(origin, nanoem_false, nanoem_false, status);
}

nanoem_mutable_motion_accessory_keyframe_t *APIENTRY
nanoemMutableMotionAccessoryKeyframeCreateAsReference(nanoem_motion_accessory_keyframe_t *origin, nanoem_status_t *status)
{
    const nanoem_motion_t *motion;
    nanoem_bool_t is_in_motion = nanoem_false;
    if (nanoem_is_not_null(origin)) {
        motion = origin->base.parent_motion;
        is_in_motion = nanoemMotionFindAccessoryKeyframeObject(motion, origin->base.frame_index) != NULL;
    }
    return nanoemMutableMotionAccessoryKeyframeCreateInternal(origin, nanoem_true, is_in_motion, status);
}

nanoem_mutable_motion_accessory_keyframe_t *APIENTRY
nanoemMutableMotionAccessoryKeyframeCreateByFound(nanoem_motion_t *motion, nanoem_frame_index_t index, nanoem_status_t *status)
{
    nanoem_mutable_motion_accessory_keyframe_t *keyframe = NULL;
    union nanoem_immutable_accessory_keyframe_to_mutable_accessory_keyframe_cast_t {
        const nanoem_motion_accessory_keyframe_t *p;
        nanoem_motion_accessory_keyframe_t *m;
    } u;
    if (nanoem_is_not_null(motion)) {
        u.p = nanoemMotionFindAccessoryKeyframeObject(motion, index);
        keyframe = nanoemMutableMotionAccessoryKeyframeCreateInternal(u.m, nanoem_true, nanoem_true, status);
    }
    return keyframe;
}

void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetTranslation(nanoem_mutable_motion_accessory_keyframe_t *keyframe, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value) && keyframe->origin->translation.values != value) {
        nanoem_crt_memcpy(keyframe->origin->translation.values, value, sizeof(keyframe->origin->translation));
    }
}

void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetOrientation(nanoem_mutable_motion_accessory_keyframe_t *keyframe, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value) && keyframe->origin->orientation.values != value) {
        nanoem_crt_memcpy(keyframe->origin->orientation.values, value, sizeof(keyframe->origin->orientation));
    }
}

void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetScaleFactor(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_f32_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->scale_factor = value;
    }
}

void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetOpacity(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_f32_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->opacity = value;
    }
}

void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetAddBlendEnabled(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_bool_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->is_add_blending_enabled = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetShadowEnabled(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_bool_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->is_shadow_enabled = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetVisible(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_bool_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->visible = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetOutsideParent(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_mutable_motion_outside_parent_t *value, nanoem_status_t *status)
{
    nanoem_motion_outside_parent_t *origin, *op;
    if (nanoem_is_not_null(keyframe)) {
        op = keyframe->origin->outside_parent;
        if (!op) {
            keyframe->origin->outside_parent = op = nanoemMotionOutsideParentCreateFromAccessoryKeyframe(keyframe->origin, status);
        }
        origin = value ? value->origin : NULL;
        if (origin) {
            op->global_bone_track_index = origin->global_bone_track_index;
            op->global_model_track_index = origin->global_model_track_index;
            op->local_bone_track_index = origin->local_bone_track_index;
        }
        else {
            op->global_bone_track_index = op->global_model_track_index = op->local_bone_track_index = NANOEM_MOTION_OBJECT_NOT_FOUND;
        }
    }
}

void APIENTRY
nanoemMutableMotionAccessoryKeyframeCopy(nanoem_mutable_motion_accessory_keyframe_t *new_keyframe, const nanoem_motion_accessory_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_motion_accessory_keyframe_t *origin_keyframe = nanoemMutableMotionAccessoryKeyframeGetOriginObject(new_keyframe);
    nanoem_motion_effect_parameter_t *const *effect_parameters, *effect_parameter;
    nanoem_mutable_motion_outside_parent_t *mutable_outside_parent;
    nanoem_mutable_motion_effect_parameter_t *mutable_effect_parameter;
    nanoem_rsize_t num_effect_parameters, i;
    nanoemMutableMotionAccessoryKeyframeSetTranslation(new_keyframe, nanoemMotionAccessoryKeyframeGetTranslation(keyframe));
    nanoemMutableMotionAccessoryKeyframeSetOrientation(new_keyframe, nanoemMotionAccessoryKeyframeGetOrientation(keyframe));
    nanoemMutableMotionAccessoryKeyframeSetScaleFactor(new_keyframe, nanoemMotionAccessoryKeyframeGetScaleFactor(keyframe));
    nanoemMutableMotionAccessoryKeyframeSetOpacity(new_keyframe, nanoemMotionAccessoryKeyframeGetOpacity(keyframe));
    nanoemMutableMotionAccessoryKeyframeSetShadowEnabled(new_keyframe, nanoemMotionAccessoryKeyframeIsShadowEnabled(keyframe));
    nanoemMutableMotionAccessoryKeyframeSetAddBlendEnabled(new_keyframe, nanoemMotionAccessoryKeyframeIsAddBlendEnabled(keyframe));
    nanoemMutableMotionAccessoryKeyframeSetVisible(new_keyframe, nanoemMotionAccessoryKeyframeIsVisible(keyframe));
    mutable_outside_parent = nanoemMutableMotionOutsideParentCreateFromAccessoryKeyframe(origin_keyframe, status);
    nanoemMutableMotionOutsideParentCopy(mutable_outside_parent, nanoemMotionAccessoryKeyframeGetOutsideParent(keyframe), status);
    nanoemMutableMotionAccessoryKeyframeSetOutsideParent(new_keyframe, mutable_outside_parent, status);
    nanoemMutableMotionOutsideParentDestroy(mutable_outside_parent);
    effect_parameters = nanoemMotionAccessoryKeyframeGetAllEffectParameterObjects(keyframe, &num_effect_parameters);
    for (i = 0; i < num_effect_parameters; i++) {
        effect_parameter = effect_parameters[i];
        mutable_effect_parameter = nanoemMutableMotionEffectParameterCreateFromAccessoryKeyframe(origin_keyframe, status);
        nanoemMutableMotionEffectParameterCopy(mutable_effect_parameter, effect_parameter, status);
        nanoemMutableMotionAccessoryKeyframeAddEffectParameter(new_keyframe, mutable_effect_parameter, status);
        nanoemMutableMotionEffectParameterDestroy(mutable_effect_parameter);
        if (nanoem_status_ptr_has_error(status)) {
            break;
        }
    }
}

void APIENTRY
nanoemMutableMotionAccessoryKeyframeAddEffectParameter(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_mutable_motion_effect_parameter_t *parameter, nanoem_status_t *status)
{
    nanoem_motion_effect_parameter_t *origin_parameter, **ptr;
    nanoem_motion_accessory_keyframe_t *origin_keyframe;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(parameter)) {
        origin_keyframe = keyframe->origin;
        origin_parameter = parameter->origin;
        if (!parameter->base.is_in_motion) {
            if (origin_parameter->keyframe_type == NANOEM_PARENT_KEYFRAME_TYPE_ACCESSORY) {
                ptr = (nanoem_motion_effect_parameter_t **) nanoemMutableObjectArrayResize(origin_keyframe->effect_parameters,
                    &keyframe->num_allocated_effect_parameters,
                    &origin_keyframe->num_effect_parameters, status);
                if (nanoem_is_not_null(ptr)) {
                    ptr[origin_keyframe->num_effect_parameters - 1] = origin_parameter;
                    origin_keyframe->effect_parameters = ptr;
                    parameter->base.is_in_motion = nanoem_true;
                    nanoem_status_ptr_assign_succeeded(status);
                }
            }
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_EFFECT_PARAMETER_ALREADY_EXISTS);
        }
    }
}

void APIENTRY
nanoemMutableMotionAccessoryKeyframeRemoveEffectParameter(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_mutable_motion_effect_parameter_t *parameter, nanoem_status_t *status)
{
    nanoem_motion_effect_parameter_t *origin_parameter;
    nanoem_motion_accessory_keyframe_t *origin_keyframe;
    nanoem_rsize_t num_effect_parameters, i;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(parameter)) {
        origin_keyframe = keyframe->origin;
        origin_parameter = parameter->origin;
        num_effect_parameters = origin_keyframe->num_effect_parameters;
        for (i = 0; i < num_effect_parameters; i++) {
            if (origin_keyframe->effect_parameters[i] == origin_parameter) {
                nanoem_crt_memmove(&origin_keyframe->effect_parameters[i], &origin_keyframe->effect_parameters[i + 1], (num_effect_parameters - i) * sizeof(origin_keyframe));
                origin_keyframe->num_effect_parameters = num_effect_parameters - 1;
                parameter->base.is_in_motion = nanoem_false;
                nanoem_status_ptr_assign_succeeded(status);
                return;
            }
        }
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_EFFECT_PARAMETER_NOT_FOUND);
    }
}

nanoem_motion_accessory_keyframe_t *APIENTRY
nanoemMutableMotionAccessoryKeyframeGetOriginObject(nanoem_mutable_motion_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->origin : NULL;
}

void APIENTRY
nanoemMutableMotionAccessoryKeyframeDestroy(nanoem_mutable_motion_accessory_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        if (nanoemMutableBaseKeyframeCanDelete(&keyframe->base)) {
            nanoemMotionAccessoryKeyframeDestroy(keyframe->origin);
        }
        nanoem_free(keyframe);
    }
}

nanoem_mutable_motion_bone_keyframe_t *APIENTRY
nanoemMutableMotionBoneKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status)
{
    nanoem_motion_bone_keyframe_t *origin = nanoemMotionBoneKeyframeCreate(motion, status);
    return nanoemMutableMotionBoneKeyframeCreateInternal(origin, nanoem_false, nanoem_false, status);
}

nanoem_mutable_motion_bone_keyframe_t *APIENTRY
nanoemMutableMotionBoneKeyframeCreateAsReference(nanoem_motion_bone_keyframe_t *origin, nanoem_status_t *status)
{
    const nanoem_unicode_string_t *name;
    const nanoem_motion_t *motion;
    nanoem_bool_t is_in_motion = nanoem_false;
    if (nanoem_is_not_null(origin)) {
        motion = origin->base.parent_motion;
        name = nanoemMotionTrackBundleResolveName(motion->local_bone_motion_track_bundle, origin->bone_id);
        is_in_motion = nanoemMotionFindBoneKeyframeObject(motion, name, origin->base.frame_index) != NULL;
    }
    return nanoemMutableMotionBoneKeyframeCreateInternal(origin, nanoem_true, is_in_motion, status);
}

nanoem_mutable_motion_bone_keyframe_t *APIENTRY
nanoemMutableMotionBoneKeyframeCreateByFound(nanoem_motion_t *motion, const nanoem_unicode_string_t *name, nanoem_frame_index_t index, nanoem_status_t *status)
{
    nanoem_mutable_motion_bone_keyframe_t *keyframe = NULL;
    union nanoem_immutable_bone_keyframe_to_mutable_bone_keyframe_cast_t {
        const nanoem_motion_bone_keyframe_t *p;
        nanoem_motion_bone_keyframe_t *m;
    } u;
    if (nanoem_is_not_null(motion)) {
        u.p = nanoemMotionFindBoneKeyframeObject(motion, name, index);
        keyframe = nanoemMutableMotionBoneKeyframeCreateInternal(u.m, nanoem_true, nanoem_true, status);
    }
    return keyframe;
}

void APIENTRY
nanoemMutableMotionBoneKeyframeSetTranslation(nanoem_mutable_motion_bone_keyframe_t *keyframe, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value) && keyframe->origin->translation.values != value) {
        nanoem_crt_memcpy(keyframe->origin->translation.values, value, sizeof(keyframe->origin->translation));
    }
}

void APIENTRY
nanoemMutableMotionBoneKeyframeSetOrientation(nanoem_mutable_motion_bone_keyframe_t *keyframe, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value) && keyframe->origin->orientation.values != value) {
        nanoem_crt_memcpy(keyframe->origin->orientation.values, value, sizeof(keyframe->origin->orientation));
    }
}

void
nanoemMutableMotionAccessoryKeyframeSetFrameIndex(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_frame_index_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->base.frame_index = value;
    }
}

void
nanoemMutableMotionBoneKeyframeSetFrameIndex(nanoem_mutable_motion_bone_keyframe_t *keyframe, nanoem_frame_index_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->base.frame_index = value;
    }
}

void APIENTRY
nanoemMutableMotionBoneKeyframeSetInterpolation(nanoem_mutable_motion_bone_keyframe_t *keyframe, nanoem_motion_bone_keyframe_interpolation_type_t index, const nanoem_u8_t *value)
{
    if (nanoem_is_not_null(keyframe)
            && nanoem_is_not_null(value)
            && index > NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_UNKNOWN
            && index < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM
            && keyframe->origin->interplation[index].u.values != value) {
        nanoem_crt_memcpy(keyframe->origin->interplation[index].u.values, value, sizeof(keyframe->origin->interplation[0]));
    }
}

void APIENTRY
nanoemMutableMotionBoneKeyframeSetStageIndex(nanoem_mutable_motion_bone_keyframe_t *keyframe, nanoem_u32_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->stage_index = value;
    }
}

void APIENTRY
nanoemMutableMotionBoneKeyframeSetPhysicsSimulationEnabled(nanoem_mutable_motion_bone_keyframe_t *keyframe, nanoem_bool_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->is_physics_simulation_enabled = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableMotionBoneKeyframeCopy(nanoem_mutable_motion_bone_keyframe_t *new_keyframe, const nanoem_motion_bone_keyframe_t *keyframe)
{
    int i;
    nanoemMutableMotionBoneKeyframeSetTranslation(new_keyframe, nanoemMotionBoneKeyframeGetTranslation(keyframe));
    nanoemMutableMotionBoneKeyframeSetOrientation(new_keyframe, nanoemMotionBoneKeyframeGetOrientation(keyframe));
    nanoemMutableMotionBoneKeyframeSetStageIndex(new_keyframe, nanoemMotionBoneKeyframeGetStageIndex(keyframe));
    nanoemMutableMotionBoneKeyframeSetPhysicsSimulationEnabled(new_keyframe, nanoemMotionBoneKeyframeIsPhysicsSimulationEnabled(keyframe));
    for (i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM; i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
        nanoemMutableMotionBoneKeyframeSetInterpolation(new_keyframe, (nanoem_motion_bone_keyframe_interpolation_type_t)i, nanoemMotionBoneKeyframeGetInterpolation(keyframe, (nanoem_motion_bone_keyframe_interpolation_type_t)i));
    }
}

nanoem_motion_bone_keyframe_t *APIENTRY
nanoemMutableMotionBoneKeyframeGetOriginObject(nanoem_mutable_motion_bone_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->origin : NULL;
}

void APIENTRY
nanoemMutableMotionBoneKeyframeSaveToBuffer(nanoem_mutable_motion_bone_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemMotionBoneKeyframeSaveToBuffer(nanoemMutableMotionBoneKeyframeGetOriginObject(keyframe), buffer, status);
}

void APIENTRY
nanoemMutableMotionBoneKeyframeDestroy(nanoem_mutable_motion_bone_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        if (nanoemMutableBaseKeyframeCanDelete(&keyframe->base)) {
            nanoemMotionBoneKeyframeDestroy(keyframe->origin);
        }
        nanoem_free(keyframe);
    }
}

nanoem_mutable_motion_camera_keyframe_t *APIENTRY
nanoemMutableMotionCameraKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status)
{
    nanoem_motion_camera_keyframe_t *origin = nanoemMotionCameraKeyframeCreate(motion, status);
    return nanoemMutableMotionCameraKeyframeCreateInternal(origin, nanoem_false, nanoem_false, status);
}

nanoem_mutable_motion_camera_keyframe_t *APIENTRY
nanoemMutableMotionCameraKeyframeCreateAsReference(nanoem_motion_camera_keyframe_t *origin, nanoem_status_t *status)
{
    const nanoem_motion_t *motion;
    nanoem_bool_t is_in_motion = nanoem_false;
    if (nanoem_is_not_null(origin)) {
        motion = origin->base.parent_motion;
        is_in_motion = nanoemMotionFindCameraKeyframeObject(motion, origin->base.frame_index) != NULL;
    }
    return nanoemMutableMotionCameraKeyframeCreateInternal(origin, nanoem_true, is_in_motion, status);
}

nanoem_mutable_motion_camera_keyframe_t *APIENTRY
nanoemMutableMotionCameraKeyframeCreateByFound(nanoem_motion_t *motion, nanoem_frame_index_t index, nanoem_status_t *status)
{
    nanoem_mutable_motion_camera_keyframe_t *keyframe = NULL;
    union nanoem_immutable_camera_keyframe_to_mutable_camera_keyframe_cast_t {
        const nanoem_motion_camera_keyframe_t *p;
        nanoem_motion_camera_keyframe_t *m;
    } u;
    if (nanoem_is_not_null(motion)) {
        u.p = nanoemMotionFindCameraKeyframeObject(motion, index);
        keyframe = nanoemMutableMotionCameraKeyframeCreateInternal(u.m, nanoem_true, nanoem_true, status);
    }
    return keyframe;
}

void APIENTRY
nanoemMutableMotionCameraKeyframeSetOutsideParent(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_mutable_motion_outside_parent_t *value, nanoem_status_t *status)
{
    nanoem_motion_outside_parent_t *origin, *op;
    if (nanoem_is_not_null(keyframe)) {
        op = keyframe->origin->outside_parent;
        if (!op) {
            keyframe->origin->outside_parent = op = nanoemMotionOutsideParentCreateFromCameraKeyframe(keyframe->origin, status);
        }
        origin = value ? value->origin : NULL;
        if (origin) {
            op->global_bone_track_index = origin->global_bone_track_index;
            op->global_model_track_index = origin->global_model_track_index;
            op->local_bone_track_index = origin->local_bone_track_index;
        }
        else {
            op->global_bone_track_index = op->global_model_track_index = op->local_bone_track_index = 0;
        }
    }
}

void APIENTRY
nanoemMutableMotionCameraKeyframeSetLookAt(nanoem_mutable_motion_camera_keyframe_t *keyframe, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value) && keyframe->origin->look_at.values != value) {
        nanoem_crt_memcpy(keyframe->origin->look_at.values, value, sizeof(keyframe->origin->look_at));
    }
}

void APIENTRY
nanoemMutableMotionCameraKeyframeSetAngle(nanoem_mutable_motion_camera_keyframe_t *keyframe, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value) && keyframe->origin->angle.values != value) {
        nanoem_crt_memcpy(keyframe->origin->angle.values, value, sizeof(keyframe->origin->angle));
    }
}

void APIENTRY
nanoemMutableMotionCameraKeyframeSetDistance(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_f32_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->distance = value;
    }
}

void
nanoemMutableMotionCameraKeyframeSetFrameIndex(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_frame_index_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->base.frame_index = value;
    }
}

void APIENTRY
nanoemMutableMotionCameraKeyframeSetFov(nanoem_mutable_motion_camera_keyframe_t *keyframe, int value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->fov = value;
    }
}

void APIENTRY
nanoemMutableMotionCameraKeyframeSetPerspectiveView(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_bool_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->is_perspective_view = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableMotionCameraKeyframeSetInterpolation(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_motion_camera_keyframe_interpolation_type_t index, const nanoem_u8_t *value)
{
    if (nanoem_is_not_null(keyframe)
            && nanoem_is_not_null(value)
            && index > NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_UNKNOWN
            && index < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM
            && keyframe->origin->interplation[index].u.values != value) {
        nanoem_crt_memcpy(keyframe->origin->interplation[index].u.values, value, sizeof(keyframe->origin->interplation[0]));
    }
}

void APIENTRY
nanoemMutableMotionCameraKeyframeSetStageIndex(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_u32_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->stage_index = value;
    }
}

void APIENTRY
nanoemMutableMotionCameraKeyframeCopy(nanoem_mutable_motion_camera_keyframe_t *new_keyframe, const nanoem_motion_camera_keyframe_t *keyframe)
{
    int i;
    nanoemMutableMotionCameraKeyframeSetAngle(new_keyframe, nanoemMotionCameraKeyframeGetAngle(keyframe));
    nanoemMutableMotionCameraKeyframeSetLookAt(new_keyframe, nanoemMotionCameraKeyframeGetLookAt(keyframe));
    nanoemMutableMotionCameraKeyframeSetFov(new_keyframe, nanoemMotionCameraKeyframeGetFov(keyframe));
    nanoemMutableMotionCameraKeyframeSetDistance(new_keyframe, nanoemMotionCameraKeyframeGetDistance(keyframe));
    nanoemMutableMotionCameraKeyframeSetPerspectiveView(new_keyframe, nanoemMotionCameraKeyframeIsPerspectiveView(keyframe));
    for (i = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM; i < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
        nanoemMutableMotionCameraKeyframeSetInterpolation(new_keyframe, (nanoem_motion_camera_keyframe_interpolation_type_t)i, nanoemMotionCameraKeyframeGetInterpolation(keyframe, (nanoem_motion_camera_keyframe_interpolation_type_t)i));
    }
}

nanoem_motion_camera_keyframe_t *APIENTRY
nanoemMutableMotionCameraKeyframeGetOriginObject(nanoem_mutable_motion_camera_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->origin : NULL;
}

void APIENTRY
nanoemMutableMotionCameraKeyframeSaveToBuffer(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemMotionCameraKeyframeSaveToBuffer(nanoemMutableMotionCameraKeyframeGetOriginObject(keyframe), buffer, status);
}

void APIENTRY
nanoemMutableMotionCameraKeyframeDestroy(nanoem_mutable_motion_camera_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        if (nanoemMutableBaseKeyframeCanDelete(&keyframe->base)) {
            nanoemMotionCameraKeyframeDestroy(keyframe->origin);
        }
        nanoem_free(keyframe);
    }
}

nanoem_mutable_motion_light_keyframe_t *APIENTRY
nanoemMutableMotionLightKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status)
{
    nanoem_motion_light_keyframe_t *origin = nanoemMotionLightKeyframeCreate(motion, status);
    return nanoemMutableMotionLightKeyframeCreateInternal(origin, nanoem_false, nanoem_false, status);
}

nanoem_mutable_motion_light_keyframe_t *APIENTRY
nanoemMutableMotionLightKeyframeCreateAsReference(nanoem_motion_light_keyframe_t *origin, nanoem_status_t *status)
{
    const nanoem_motion_t *motion;
    nanoem_bool_t is_in_motion = nanoem_false;
    if (nanoem_is_not_null(origin)) {
        motion = origin->base.parent_motion;
        is_in_motion = nanoemMotionFindLightKeyframeObject(motion, origin->base.frame_index) != NULL;
    }
    return nanoemMutableMotionLightKeyframeCreateInternal(origin, nanoem_true, is_in_motion, status);
}

nanoem_mutable_motion_light_keyframe_t *APIENTRY
nanoemMutableMotionLightKeyframeCreateByFound(nanoem_motion_t *motion, nanoem_frame_index_t index, nanoem_status_t *status)
{
    nanoem_mutable_motion_light_keyframe_t *keyframe = NULL;
    union nanoem_immutable_light_keyframe_to_mutable_light_keyframe_cast_t {
        const nanoem_motion_light_keyframe_t *p;
        nanoem_motion_light_keyframe_t *m;
    } u;
    if (nanoem_is_not_null(motion)) {
        u.p = nanoemMotionFindLightKeyframeObject(motion, index);
        keyframe = nanoemMutableMotionLightKeyframeCreateInternal(u.m, nanoem_true, nanoem_true, status);
    }
    return keyframe;
}

void APIENTRY
nanoemMutableMotionLightKeyframeSetColor(nanoem_mutable_motion_light_keyframe_t *keyframe, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value) && keyframe->origin->color.values != value) {
        nanoem_crt_memcpy(keyframe->origin->color.values, value, sizeof(keyframe->origin->color));
    }
}

void APIENTRY
nanoemMutableMotionLightKeyframeSetDirection(nanoem_mutable_motion_light_keyframe_t *keyframe, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value) && keyframe->origin->direction.values != value) {
        nanoem_crt_memcpy(keyframe->origin->direction.values, value, sizeof(keyframe->origin->direction));
    }
}

void APIENTRY
nanoemMutableMotionLightKeyframeCopy(nanoem_mutable_motion_light_keyframe_t *new_keyframe, const nanoem_motion_light_keyframe_t *keyframe)
{
    nanoemMutableMotionLightKeyframeSetColor(new_keyframe, nanoemMotionLightKeyframeGetColor(keyframe));
    nanoemMutableMotionLightKeyframeSetDirection(new_keyframe, nanoemMotionLightKeyframeGetDirection(keyframe));
}

void
nanoemMutableMotionLightKeyframeSetFrameIndex(nanoem_mutable_motion_light_keyframe_t *keyframe, nanoem_frame_index_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->base.frame_index = value;
    }
}

nanoem_motion_light_keyframe_t *APIENTRY
nanoemMutableMotionLightKeyframeGetOriginObject(nanoem_mutable_motion_light_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->origin : NULL;
}

void APIENTRY
nanoemMutableMotionLightKeyframeSaveToBuffer(nanoem_mutable_motion_light_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemMotionLightKeyframeSaveToBuffer(nanoemMutableMotionLightKeyframeGetOriginObject(keyframe), buffer, status);
}

void APIENTRY
nanoemMutableMotionLightKeyframeDestroy(nanoem_mutable_motion_light_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        if (nanoemMutableBaseKeyframeCanDelete(&keyframe->base)) {
            nanoemMotionLightKeyframeDestroy(keyframe->origin);
        }
        nanoem_free(keyframe);
    }
}

nanoem_mutable_motion_model_keyframe_t *APIENTRY
nanoemMutableMotionModelKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status)
{
    nanoem_motion_model_keyframe_t *origin = nanoemMotionModelKeyframeCreate(motion, status);
    return nanoemMutableMotionModelKeyframeCreateInternal(origin, nanoem_false, nanoem_false, status);
}

nanoem_mutable_motion_model_keyframe_t *APIENTRY
nanoemMutableMotionModelKeyframeCreateAsReference(nanoem_motion_model_keyframe_t *origin, nanoem_status_t *status)
{
    const nanoem_motion_t *motion;
    nanoem_bool_t is_in_motion = nanoem_false;
    if (nanoem_is_not_null(origin)) {
        motion = origin->base.parent_motion;
        is_in_motion = nanoemMotionFindModelKeyframeObject(motion, origin->base.frame_index) != NULL;
    }
    return nanoemMutableMotionModelKeyframeCreateInternal(origin, nanoem_true, is_in_motion, status);
}

nanoem_mutable_motion_model_keyframe_t *APIENTRY
nanoemMutableMotionModelKeyframeCreateByFound(nanoem_motion_t *motion, nanoem_frame_index_t index, nanoem_status_t *status)
{
    nanoem_mutable_motion_model_keyframe_t *keyframe = NULL;
    union nanoem_immutable_model_keyframe_to_mutable_model_keyframe_cast_t {
        const nanoem_motion_model_keyframe_t *p;
        nanoem_motion_model_keyframe_t *m;
    } u;
    if (nanoem_is_not_null(motion)) {
        u.p = nanoemMotionFindModelKeyframeObject(motion, index);
        keyframe = nanoemMutableMotionModelKeyframeCreateInternal(u.m, nanoem_true, nanoem_true, status);
    }
    return keyframe;
}

void APIENTRY
nanoemMutableMotionModelKeyframeSetVisible(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_bool_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->visible = value ? 1 : 0;
    }
}

void
nanoemMutableMotionModelKeyframeSetFrameIndex(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_frame_index_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->base.frame_index = value;
    }
}

void APIENTRY
nanoemMutableMotionModelKeyframeSetEdgeScaleFactor(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_f32_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->edge_scale_factor = value;
    }
}

void APIENTRY
nanoemMutableMotionModelKeyframeSetEdgeColor(nanoem_mutable_motion_model_keyframe_t *keyframe, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value) && keyframe->origin->edge_color.values != value) {
        nanoem_crt_memcpy(keyframe->origin->edge_color.values, value, sizeof(keyframe->origin->edge_color));
    }
}

void APIENTRY
nanoemMutableMotionModelKeyframeSetAddBlendEnabled(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_bool_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->is_add_blending_enabled = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableMotionModelKeyframeSetPhysicsSimulationEnabled(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_bool_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->is_physics_simulation_enabled = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableMotionModelKeyframeCopy(nanoem_mutable_motion_model_keyframe_t *new_keyframe, const nanoem_motion_model_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_motion_model_keyframe_t *origin_keyframe = nanoemMutableMotionModelKeyframeGetOriginObject(new_keyframe);
    nanoem_motion_model_keyframe_constraint_state_t *const *constraint_states, *constraint_state;
    nanoem_motion_outside_parent_t *const *outside_parents, *outside_parent;
    nanoem_motion_effect_parameter_t *const *effect_parameters, *effect_parameter;
    nanoem_mutable_motion_outside_parent_t *mutable_outside_parent;
    nanoem_mutable_motion_model_keyframe_constraint_state_t *mutable_constraint_state;
    nanoem_mutable_motion_effect_parameter_t *mutable_effect_parameter;
    nanoem_rsize_t num_constraint_states, num_outside_parents, num_effect_parameters, i;
    nanoemMutableMotionModelKeyframeSetVisible(new_keyframe, nanoemMotionModelKeyframeIsVisible(keyframe));
    nanoemMutableMotionModelKeyframeSetAddBlendEnabled(new_keyframe, nanoemMotionModelKeyframeIsAddBlendEnabled(keyframe));
    nanoemMutableMotionModelKeyframeSetPhysicsSimulationEnabled(new_keyframe, nanoemMotionModelKeyframeIsPhysicsSimulationEnabled(keyframe));
    nanoemMutableMotionModelKeyframeSetEdgeColor(new_keyframe, nanoemMotionModelKeyframeGetEdgeColor(keyframe));
    nanoemMutableMotionModelKeyframeSetEdgeScaleFactor(new_keyframe, nanoemMotionModelKeyframeGetEdgeScaleFactor(keyframe));
    constraint_states = nanoemMotionModelKeyframeGetAllConstraintStateObjects(keyframe, &num_constraint_states);
    nanoem_status_ptr_assign_succeeded(status);
    for (i = 0; i < num_constraint_states; i++) {
        constraint_state = constraint_states[i];
        mutable_constraint_state = nanoemMutableMotionModelKeyframeConstraintStateCreateMutable(new_keyframe, status);
        nanoemMutableMotionModelKeyframeConstraintStateSetBoneName(mutable_constraint_state, nanoemMotionModelKeyframeConstraintStateGetBoneName(constraint_state), status);
        nanoemMutableMotionModelKeyframeConstraintStateSetEnabled(mutable_constraint_state, nanoemMotionModelKeyframeConstraintStateIsEnabled(constraint_state));
        nanoemMutableMotionModelKeyframeAddConstraintState(new_keyframe, mutable_constraint_state, status);
        nanoemMutableMotionModelKeyframeConstraintStateDestroy(mutable_constraint_state);
        if (nanoem_status_ptr_has_error(status)) {
            break;
        }
    }
    if (!nanoem_status_ptr_has_error(status)) {
        outside_parents = nanoemMotionModelKeyframeGetAllOutsideParentObjects(keyframe, &num_outside_parents);
        for (i = 0; i < num_outside_parents; i++) {
            outside_parent = outside_parents[i];
            mutable_outside_parent = nanoemMutableMotionOutsideParentCreateFromModelKeyframe(origin_keyframe, status);
            nanoemMutableMotionOutsideParentCopy(mutable_outside_parent, outside_parent, status);
            nanoemMutableMotionModelKeyframeAddOutsideParent(new_keyframe, mutable_outside_parent, status);
            nanoemMutableMotionOutsideParentDestroy(mutable_outside_parent);
            if (nanoem_status_ptr_has_error(status)) {
                break;
            }
        }
    }
    if (!nanoem_status_ptr_has_error(status)) {
        effect_parameters = nanoemMotionModelKeyframeGetAllEffectParameterObjects(keyframe, &num_effect_parameters);
        for (i = 0; i < num_effect_parameters; i++) {
            effect_parameter = effect_parameters[i];
            mutable_effect_parameter = nanoemMutableMotionEffectParameterCreateFromModelKeyframe(origin_keyframe, status);
            nanoemMutableMotionEffectParameterCopy(mutable_effect_parameter, effect_parameter, status);
            nanoemMutableMotionModelKeyframeAddEffectParameter(new_keyframe, mutable_effect_parameter, status);
            nanoemMutableMotionEffectParameterDestroy(mutable_effect_parameter);
            if (nanoem_status_ptr_has_error(status)) {
                break;
            }
        }
    }
}

nanoem_motion_model_keyframe_t *APIENTRY
nanoemMutableMotionModelKeyframeGetOriginObject(nanoem_mutable_motion_model_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->origin : NULL;
}

void APIENTRY
nanoemMutableMotionModelKeyframeAddConstraintState(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_motion_model_keyframe_constraint_state_t *state, nanoem_status_t *status)
{
    nanoem_motion_model_keyframe_constraint_state_t *origin_state, **ptr;
    nanoem_motion_model_keyframe_t *origin_keyframe;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(state)) {
        origin_keyframe = keyframe->origin;
        origin_state = state->origin;
        if (!state->base.is_in_motion) {
            ptr = (nanoem_motion_model_keyframe_constraint_state_t **) nanoemMutableObjectArrayResize(origin_keyframe->constraint_states,
                &keyframe->num_allocated_constraint_states,
                &origin_keyframe->num_constraint_states, status);
            if (nanoem_is_not_null(ptr)) {
                ptr[origin_keyframe->num_constraint_states - 1] = origin_state;
                origin_keyframe->constraint_states = ptr;
                state->base.is_in_motion = nanoem_true;
                nanoem_status_ptr_assign_succeeded(status);
            }
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_STATE_ALREADY_EXISTS);
        }
    }
}

void APIENTRY
nanoemMutableMotionModelKeyframeRemoveConstraintState(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_motion_model_keyframe_constraint_state_t *state, nanoem_status_t *status)
{
    nanoem_motion_model_keyframe_constraint_state_t *origin_state;
    nanoem_motion_model_keyframe_t *origin_keyframe;
    nanoem_rsize_t num_constraint_states, i;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(state)) {
        origin_keyframe = keyframe->origin;
        origin_state = state->origin;
        num_constraint_states = origin_keyframe->num_constraint_states;
        for (i = 0; i < num_constraint_states; i++) {
            if (origin_keyframe->constraint_states[i] == origin_state) {
                nanoem_crt_memmove(&origin_keyframe->constraint_states[i], &origin_keyframe->constraint_states[i + 1], (num_constraint_states - i) * sizeof(origin_keyframe));
                origin_keyframe->num_constraint_states = num_constraint_states - 1;
                state->base.is_in_motion = nanoem_false;
                nanoem_status_ptr_assign_succeeded(status);
                return;
            }
        }
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_STATE_NOT_FOUND);
    }
}

void APIENTRY
nanoemMutableMotionModelKeyframeAddOutsideParent(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_motion_outside_parent_t *op, nanoem_status_t *status)
{
    nanoem_motion_outside_parent_t *origin_op, **ptr;
    nanoem_motion_model_keyframe_t *origin_keyframe;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(op)) {
        origin_keyframe = keyframe->origin;
        origin_op = op->origin;
        if (!op->base.is_in_motion) {
            ptr = (nanoem_motion_outside_parent_t **) nanoemMutableObjectArrayResize(origin_keyframe->outside_parents,
                &keyframe->num_allocated_outside_parents,
                &origin_keyframe->num_outside_parents, status);
            if (nanoem_is_not_null(ptr)) {
                ptr[origin_keyframe->num_outside_parents - 1] = origin_op;
                origin_keyframe->outside_parents = ptr;
                op->base.is_in_motion = nanoem_true;
                nanoem_status_ptr_assign_succeeded(status);
            }
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_BINDING_ALREADY_EXISTS);
        }
    }
}

void APIENTRY
nanoemMutableMotionModelKeyframeRemoveOutsideParent(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_motion_outside_parent_t *op, nanoem_status_t *status)
{
    nanoem_motion_outside_parent_t *origin_op;
    nanoem_motion_model_keyframe_t *origin_keyframe;
    nanoem_rsize_t num_outside_parents, i;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(op)) {
        origin_keyframe = keyframe->origin;
        origin_op = op->origin;
        num_outside_parents = origin_keyframe->num_outside_parents;
        for (i = 0; i < num_outside_parents; i++) {
            if (origin_keyframe->outside_parents[i] == origin_op) {
                nanoem_crt_memmove(&origin_keyframe->outside_parents[i], &origin_keyframe->outside_parents[i + 1], (num_outside_parents - i) * sizeof(origin_keyframe));
                origin_keyframe->num_outside_parents = num_outside_parents - 1;
                op->base.is_in_motion = nanoem_false;
                nanoem_status_ptr_assign_succeeded(status);
                return;
            }
        }
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_BINDING_NOT_FOUND);
    }
}

void APIENTRY
nanoemMutableMotionModelKeyframeAddEffectParameter(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_motion_effect_parameter_t *parameter, nanoem_status_t *status)
{
    nanoem_motion_effect_parameter_t *origin_parameter, **ptr;
    nanoem_motion_model_keyframe_t *origin_keyframe;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(parameter)) {
        origin_keyframe = keyframe->origin;
        origin_parameter = parameter->origin;
        if (!parameter->base.is_in_motion) {
            if (origin_parameter->keyframe_type == NANOEM_PARENT_KEYFRAME_TYPE_MODEL) {
                ptr = (nanoem_motion_effect_parameter_t **) nanoemMutableObjectArrayResize(origin_keyframe->effect_parameters,
                    &keyframe->num_allocated_effect_parameters,
                    &origin_keyframe->num_effect_parameters, status);
                if (nanoem_is_not_null(ptr)) {
                    ptr[origin_keyframe->num_effect_parameters - 1] = origin_parameter;
                    origin_keyframe->effect_parameters = ptr;
                    parameter->base.is_in_motion = nanoem_true;
                    nanoem_status_ptr_assign_succeeded(status);
                }
            }
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_EFFECT_PARAMETER_ALREADY_EXISTS);
        }
    }
}

void APIENTRY
nanoemMutableMotionModelKeyframeRemoveEffectParameter(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_motion_effect_parameter_t *parameter, nanoem_status_t *status)
{
    nanoem_motion_effect_parameter_t *origin_parameter;
    nanoem_motion_model_keyframe_t *origin_keyframe;
    nanoem_rsize_t num_effect_parameters, i;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(parameter)) {
        origin_keyframe = keyframe->origin;
        origin_parameter = parameter->origin;
        num_effect_parameters = origin_keyframe->num_effect_parameters;
        for (i = 0; i < num_effect_parameters; i++) {
            if (origin_keyframe->effect_parameters[i] == origin_parameter) {
                nanoem_crt_memmove(&origin_keyframe->effect_parameters[i], &origin_keyframe->effect_parameters[i + 1], (num_effect_parameters - i) * sizeof(origin_keyframe));
                origin_keyframe->num_effect_parameters = num_effect_parameters - 1;
                parameter->base.is_in_motion = nanoem_false;
                nanoem_status_ptr_assign_succeeded(status);
                return;
            }
        }
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_EFFECT_PARAMETER_NOT_FOUND);
    }
}

void APIENTRY
nanoemMutableMotionModelKeyframeSaveToBuffer(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemMotionModelKeyframeSaveToBuffer(nanoemMutableMotionModelKeyframeGetOriginObject(keyframe), buffer, status);
}

void APIENTRY
nanoemMutableMotionModelKeyframeDestroy(nanoem_mutable_motion_model_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        if (nanoemMutableBaseKeyframeCanDelete(&keyframe->base)) {
            nanoemMotionModelKeyframeDestroy(keyframe->origin);
        }
        nanoem_free(keyframe);
    }
}

nanoem_mutable_motion_model_keyframe_constraint_state_t *APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateCreate(nanoem_motion_model_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_motion_model_keyframe_constraint_state_t *origin;
    nanoem_mutable_motion_model_keyframe_constraint_state_t *state = NULL;
    nanoem_bool_t is_in_motion;
    if (nanoem_is_not_null(keyframe)) {
        origin = (nanoem_motion_model_keyframe_constraint_state_t *) nanoem_calloc(1, sizeof(*origin), status);
        if (origin) {
            origin->parent_keyframe = keyframe;
            is_in_motion = nanoemObjectArrayContainsObject((const void *const *) keyframe->constraint_states, origin, keyframe->num_constraint_states);
            state = nanoemMutableMotionModelKeyframeConstraintStateCreateInternal(origin, nanoem_false, is_in_motion, status);
            if (nanoem_is_null(state)) {
                nanoem_free(origin);
            }
        }
    }
    return state;
}

nanoem_mutable_motion_model_keyframe_constraint_state_t *APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateCreateMutable(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_status_t *status)
{
    return nanoem_is_not_null(keyframe) ? nanoemMutableMotionModelKeyframeConstraintStateCreate(keyframe->origin, status) : NULL;
}

nanoem_mutable_motion_model_keyframe_constraint_state_t *APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateCreateAsReference(nanoem_motion_model_keyframe_constraint_state_t *origin, nanoem_status_t *status)
{
    const nanoem_motion_model_keyframe_t *parent_keyframe;
    nanoem_mutable_motion_model_keyframe_constraint_state_t *state = NULL;
    nanoem_bool_t is_in_motion;
    if (nanoem_is_not_null(state)) {
        parent_keyframe = origin->parent_keyframe;
        is_in_motion = nanoemObjectArrayContainsObject((const void *const *) parent_keyframe->constraint_states, origin, parent_keyframe->num_constraint_states);
        state = nanoemMutableMotionModelKeyframeConstraintStateCreateInternal(origin, nanoem_true, is_in_motion, status);
    }
    return state;
}

void APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateSetBoneName(nanoem_mutable_motion_model_keyframe_constraint_state_t *state, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    nanoem_unicode_string_t *new_name, *found_name;
    nanoem_motion_t *motion;
    int ret;
    if (nanoem_is_not_null(state) && nanoem_is_not_null(value)) {
        motion = state->origin->parent_keyframe->base.parent_motion;
        factory = motion->factory;
        new_name = nanoemUnicodeStringFactoryCloneString(factory, value, status);
        state->origin->bone_id = nanoemMotionResolveLocalBoneTrackId(motion, new_name, &found_name, &ret);
        if (found_name) {
            nanoemUtilDestroyString(new_name, factory);
        }
    }
}

void APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateSetEnabled(nanoem_mutable_motion_model_keyframe_constraint_state_t *state, nanoem_bool_t value)
{
    if (nanoem_is_not_null(state)) {
        state->origin->enabled = value ? 1 : 0;
    }
}

nanoem_motion_model_keyframe_constraint_state_t *APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateGetOriginObject(nanoem_mutable_motion_model_keyframe_constraint_state_t *state)
{
    return nanoem_is_not_null(state) ? state->origin : NULL;
}

void APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateSaveToBuffer(nanoem_mutable_motion_model_keyframe_constraint_state_t *state, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_unicode_string_t *name;
    nanoem_motion_model_keyframe_constraint_state_t *origin;
    nanoem_motion_t *motion;
    nanoem_unicode_string_factory_t *factory;
    nanoem_rsize_t length;
    nanoem_u8_t name_buffer[PMD_BONE_NAME_LENGTH], *name_cp932;
    if (nanoem_is_not_null(status) && nanoem_is_not_null(buffer)) {
        origin = state->origin;
        motion = origin->parent_keyframe->base.parent_motion;
        factory = motion->factory;
        name = nanoemMotionTrackBundleResolveName(motion->local_bone_motion_track_bundle, origin->bone_id);
        name_cp932 = nanoemUnicodeStringFactoryGetByteArrayEncoding(factory, name, &length, NANOEM_CODEC_TYPE_SJIS, status);
        if (!nanoem_status_ptr_has_error(status) && nanoem_is_not_null(name_cp932)) {
            nanoem_crt_memset(name_buffer, 0, sizeof(name_buffer));
            nanoemUtilCopyString((char *) name_buffer, sizeof(name_buffer), (const char *) name_cp932, length);
        }
        nanoemUnicodeStringFactoryDestroyByteArray(factory, name_cp932);
        nanoemMutableBufferWriteByteArray(buffer, name_buffer, sizeof(name_buffer), status);
        nanoemMutableBufferWriteByte(buffer, origin->enabled, status);
    }
}

void APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateDestroy(nanoem_mutable_motion_model_keyframe_constraint_state_t *state)
{
    if (nanoem_is_not_null(state)) {
        if (nanoemMutableBaseKeyframeCanDelete(&state->base)) {
            nanoem_free(state->origin);
        }
        nanoem_free(state);
    }
}

nanoem_mutable_motion_morph_keyframe_t *APIENTRY
nanoemMutableMotionMorphKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status)
{
    nanoem_motion_morph_keyframe_t *origin = nanoemMotionMorphKeyframeCreate(motion, status);
    return nanoemMutableMotionMorphKeyframeCreateInternal(origin, nanoem_false, nanoem_false, status);
}

nanoem_mutable_motion_morph_keyframe_t *APIENTRY
nanoemMutableMotionMorphKeyframeCreateAsReference(nanoem_motion_morph_keyframe_t *origin, nanoem_status_t *status)
{
    const nanoem_unicode_string_t *name;
    const nanoem_motion_t *motion;
    nanoem_bool_t is_in_motion = nanoem_false;
    if (nanoem_is_not_null(origin)) {
        motion = origin->base.parent_motion;
        name = nanoemMotionTrackBundleResolveName(motion->local_morph_motion_track_bundle, origin->morph_id);
        is_in_motion = nanoemMotionFindMorphKeyframeObject(motion, name, origin->base.frame_index) != NULL;
    }
    return nanoemMutableMotionMorphKeyframeCreateInternal(origin, nanoem_true, is_in_motion, status);
}

nanoem_mutable_motion_morph_keyframe_t *APIENTRY
nanoemMutableMotionMorphKeyframeCreateByFound(nanoem_motion_t *motion, const nanoem_unicode_string_t *name, nanoem_frame_index_t index, nanoem_status_t *status)
{
    nanoem_mutable_motion_morph_keyframe_t *keyframe = NULL;
    union nanoem_immutable_morph_keyframe_to_mutable_morph_keyframe_cast_t {
        const nanoem_motion_morph_keyframe_t *p;
        nanoem_motion_morph_keyframe_t *m;
    } u;
    if (nanoem_is_not_null(motion)) {
        u.p = nanoemMotionFindMorphKeyframeObject(motion, name, index);
        keyframe = nanoemMutableMotionMorphKeyframeCreateInternal(u.m, nanoem_true, nanoem_true, status);
    }
    return keyframe;
}

void APIENTRY
nanoemMutableMotionMorphKeyframeSetWeight(nanoem_mutable_motion_morph_keyframe_t *keyframe, nanoem_f32_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->weight = value;
    }
}

void APIENTRY
nanoemMutableMotionMorphKeyframeCopy(nanoem_mutable_motion_morph_keyframe_t *new_keyframe, const nanoem_motion_morph_keyframe_t *keyframe)
{
    nanoemMutableMotionMorphKeyframeSetWeight(new_keyframe, nanoemMotionMorphKeyframeGetWeight(keyframe));
}

void
nanoemMutableMotionMorphKeyframeSetFrameIndex(nanoem_mutable_motion_morph_keyframe_t *keyframe, nanoem_frame_index_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->base.frame_index = value;
    }
}

nanoem_motion_morph_keyframe_t *APIENTRY
nanoemMutableMotionMorphKeyframeGetOriginObject(nanoem_mutable_motion_morph_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->origin : NULL;
}

void APIENTRY
nanoemMutableMotionMorphKeyframeSaveToBuffer(nanoem_mutable_motion_morph_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemMotionMorphKeyframeSaveToBuffer(nanoemMutableMotionMorphKeyframeGetOriginObject(keyframe), buffer, status);
}

void APIENTRY
nanoemMutableMotionMorphKeyframeDestroy(nanoem_mutable_motion_morph_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        if (nanoemMutableBaseKeyframeCanDelete(&keyframe->base)) {
            nanoemMotionMorphKeyframeDestroy(keyframe->origin);
        }
        nanoem_free(keyframe);
    }
}

nanoem_mutable_motion_self_shadow_keyframe_t *APIENTRY
nanoemMutableMotionSelfShadowKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status)
{
    nanoem_motion_self_shadow_keyframe_t *origin = nanoemMotionSelfShadowKeyframeCreate(motion, status);
    return nanoemMutableMotionSelfShadowKeyframeCreateInternal(origin, nanoem_false, nanoem_false, status);
}

nanoem_mutable_motion_self_shadow_keyframe_t *APIENTRY
nanoemMutableMotionSelfShadowKeyframeCreateAsReference(nanoem_motion_self_shadow_keyframe_t *origin, nanoem_status_t *status)
{
    const nanoem_motion_t *motion;
    nanoem_bool_t is_in_motion = nanoem_false;
    if (nanoem_is_not_null(origin)) {
        motion = origin->base.parent_motion;
        is_in_motion = nanoemMotionFindSelfShadowKeyframeObject(motion, origin->base.frame_index) != NULL;
    }
    return nanoemMutableMotionSelfShadowKeyframeCreateInternal(origin, nanoem_true, is_in_motion, status);
}

nanoem_mutable_motion_self_shadow_keyframe_t *APIENTRY
nanoemMutableMotionSelfShadowKeyframeCreateByFound(nanoem_motion_t *motion, nanoem_frame_index_t index, nanoem_status_t *status)
{
    nanoem_mutable_motion_self_shadow_keyframe_t *keyframe = NULL;
    union nanoem_immutable_self_shadow_keyframe_to_mutable_self_shadow_keyframe_cast_t {
        const nanoem_motion_self_shadow_keyframe_t *p;
        nanoem_motion_self_shadow_keyframe_t *m;
    } u;
    if (nanoem_is_not_null(motion)) {
        u.p = nanoemMotionFindSelfShadowKeyframeObject(motion, index);
        keyframe = nanoemMutableMotionSelfShadowKeyframeCreateInternal(u.m, nanoem_true, nanoem_true, status);
    }
    return keyframe;
}

void APIENTRY
nanoemMutableMotionSelfShadowKeyframeSetDistance(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, nanoem_f32_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->distance = value;
    }
}

void APIENTRY
nanoemMutableMotionSelfShadowKeyframeSetMode(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, int value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->mode = value;
    }
}

void
nanoemMutableMotionSelfShadowKeyframeSetFrameIndex(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, nanoem_frame_index_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->base.frame_index = value;
    }
}

void APIENTRY
nanoemMutableMotionSelfShadowKeyframeCopy(nanoem_mutable_motion_self_shadow_keyframe_t *new_keyframe, const nanoem_motion_self_shadow_keyframe_t *keyframe)
{
    nanoemMutableMotionSelfShadowKeyframeSetDistance(new_keyframe, nanoemMotionSelfShadowKeyframeGetDistance(keyframe));
    nanoemMutableMotionSelfShadowKeyframeSetMode(new_keyframe, nanoemMotionSelfShadowKeyframeGetMode(keyframe));
}

nanoem_motion_self_shadow_keyframe_t *APIENTRY
nanoemMutableMotionSelfShadowKeyframeGetOriginObject(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->origin : NULL;
}

void APIENTRY
nanoemMutableMotionSelfShadowKeyframeSaveToBuffer(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemMotionSelfShadowKeyframeSaveToBuffer(nanoemMutableMotionSelfShadowKeyframeGetOriginObject(keyframe), buffer, status);
}

void APIENTRY
nanoemMutableMotionSelfShadowKeyframeDestroy(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        if (nanoemMutableBaseKeyframeCanDelete(&keyframe->base)) {
            nanoemMotionSelfShadowKeyframeDestroy(keyframe->origin);
        }
        nanoem_free(keyframe);
    }
}

nanoem_mutable_motion_t *APIENTRY
nanoemMutableMotionCreate(nanoem_unicode_string_factory_t *factory, nanoem_status_t *status)
{
    nanoem_mutable_motion_t *new_motion;
    nanoem_motion_t *motion;
    new_motion = (nanoem_mutable_motion_t *) nanoem_calloc(1, sizeof(*new_motion), status);
    if (nanoem_is_not_null(new_motion)) {
        motion = new_motion->origin = nanoemMotionCreate(factory, status);
        motion->accessory_keyframes = (nanoem_motion_accessory_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*motion->accessory_keyframes), status);
        motion->bone_keyframes = (nanoem_motion_bone_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*motion->bone_keyframes), status);
        motion->camera_keyframes = (nanoem_motion_camera_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*motion->camera_keyframes), status);
        motion->light_keyframes = (nanoem_motion_light_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*motion->light_keyframes), status);
        motion->model_keyframes = (nanoem_motion_model_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*motion->model_keyframes), status);
        motion->morph_keyframes = (nanoem_motion_morph_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*motion->morph_keyframes), status);
        motion->self_shadow_keyframes = (nanoem_motion_self_shadow_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*motion->self_shadow_keyframes), status);
        new_motion->is_reference = nanoem_false;
        new_motion->num_allocated_accessory_keyframes = __nanoem_default_allocation_size;
        new_motion->num_allocated_bone_keyframes = __nanoem_default_allocation_size;
        new_motion->num_allocated_camera_keyframes = __nanoem_default_allocation_size;
        new_motion->num_allocated_light_keyframes = __nanoem_default_allocation_size;
        new_motion->num_allocated_model_keyframes = __nanoem_default_allocation_size;
        new_motion->num_allocated_morph_keyframes = __nanoem_default_allocation_size;
        new_motion->num_allocated_self_shadow_keyframes = __nanoem_default_allocation_size;
    }
    return new_motion;
}

nanoem_mutable_motion_t *APIENTRY
nanoemMutableMotionCreateAsReference(nanoem_motion_t *motion, nanoem_status_t *status)
{
    nanoem_mutable_motion_t *new_motion = NULL;
    if (nanoem_is_not_null(motion)) {
        new_motion = (nanoem_mutable_motion_t *) nanoem_calloc(1, sizeof(*new_motion), status);
        if (nanoem_is_not_null(new_motion)) {
            new_motion->origin = motion;
            new_motion->is_reference = nanoem_true;
            new_motion->num_allocated_accessory_keyframes = motion->num_accessory_keyframes;
            new_motion->num_allocated_bone_keyframes = motion->num_bone_keyframes;
            new_motion->num_allocated_camera_keyframes = motion->num_camera_keyframes;
            new_motion->num_allocated_light_keyframes = motion->num_light_keyframes;
            new_motion->num_allocated_model_keyframes = motion->num_model_keyframes;
            new_motion->num_allocated_morph_keyframes = motion->num_morph_keyframes;
            new_motion->num_allocated_self_shadow_keyframes = motion->num_self_shadow_keyframes;
        }
    }
    return new_motion;
}

void APIENTRY
nanoemMutableMotionAddAccessoryKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status)
{
    nanoem_motion_t *origin_motion;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(keyframe)) {
        origin_motion = motion->origin;
        if (nanoemMotionFindAccessoryKeyframeObject(origin_motion, frame_index) == NULL) {
            nanoemMotionKeyframeObjectArrayAddObject((nanoem_motion_keyframe_object_t ***) &origin_motion->accessory_keyframes, (nanoem_motion_keyframe_object_t *) keyframe->origin, frame_index, &origin_motion->num_accessory_keyframes, &motion->num_allocated_accessory_keyframes, status);
            if (!nanoem_status_ptr_has_error(status)) {
                keyframe->base.is_in_motion = nanoem_true;
            }
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MOTION_ACCESSORY_KEYFRAME_ALREADY_EXISTS);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableMotionAddBoneKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_bone_keyframe_t *keyframe, const nanoem_unicode_string_t *name, nanoem_frame_index_t frame_index, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    nanoem_motion_t *origin_motion;
    kh_motion_track_bundle_t *track;
    int ret;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(keyframe) && nanoem_is_not_null(name)) {
        origin_motion = motion->origin;
        track = origin_motion->local_bone_motion_track_bundle;
        factory = origin_motion->factory;
        if (nanoemMotionFindBoneKeyframeObject(origin_motion, name, frame_index) == NULL) {
            nanoemMotionKeyframeObjectArrayAddObject((nanoem_motion_keyframe_object_t ***) &origin_motion->bone_keyframes, (nanoem_motion_keyframe_object_t *) keyframe->origin, frame_index, &origin_motion->num_bone_keyframes, &motion->num_allocated_bone_keyframes, status);
            if (!nanoem_status_ptr_has_error(status)) {
                keyframe->base.is_in_motion = nanoem_true;
                nanoemMutableMotionBoneKeyframeSetName(keyframe, name, status);
                if (!nanoem_status_ptr_has_error(status)) {
                    nanoemMotionTrackBundleAddKeyframe(track, (nanoem_motion_keyframe_object_t *) keyframe->origin, frame_index, name, factory, &ret);
                    nanoem_status_ptr_assign(status, ret >= 0 ? NANOEM_STATUS_SUCCESS : NANOEM_STATUS_ERROR_MALLOC_FAILED);
                }
            }
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_ALREADY_EXISTS);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableMotionAddCameraKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status)
{
    nanoem_motion_t *origin_motion;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(keyframe)) {
        origin_motion = motion->origin;
        if (nanoemMotionFindCameraKeyframeObject(origin_motion, frame_index) == NULL) {
            nanoemMotionKeyframeObjectArrayAddObject((nanoem_motion_keyframe_object_t ***) &origin_motion->camera_keyframes, (nanoem_motion_keyframe_object_t *) keyframe->origin, frame_index, &origin_motion->num_camera_keyframes, &motion->num_allocated_camera_keyframes, status);
            if (!nanoem_status_ptr_has_error(status)) {
                keyframe->base.is_in_motion = nanoem_true;
            }
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_ALREADY_EXISTS);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableMotionAddLightKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_light_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status)
{
    nanoem_motion_t *origin_motion;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(keyframe)) {
        origin_motion = motion->origin;
        if (nanoemMotionFindLightKeyframeObject(origin_motion, frame_index) == NULL) {
            nanoemMotionKeyframeObjectArrayAddObject((nanoem_motion_keyframe_object_t ***) &origin_motion->light_keyframes, (nanoem_motion_keyframe_object_t *) keyframe->origin, frame_index, &origin_motion->num_light_keyframes, &motion->num_allocated_light_keyframes, status);
            if (!nanoem_status_ptr_has_error(status)) {
                keyframe->base.is_in_motion = nanoem_true;
            }
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_ALREADY_EXISTS);
        }
    }
}

void APIENTRY
nanoemMutableMotionAddModelKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status)
{
    nanoem_motion_t *origin_motion;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(keyframe)) {
        origin_motion = motion->origin;
        if (nanoemMotionFindModelKeyframeObject(origin_motion, frame_index) == NULL) {
            nanoemMotionKeyframeObjectArrayAddObject((nanoem_motion_keyframe_object_t ***) &origin_motion->model_keyframes, (nanoem_motion_keyframe_object_t *) keyframe->origin, frame_index, &origin_motion->num_model_keyframes, &motion->num_allocated_model_keyframes, status);
            if (!nanoem_status_ptr_has_error(status)) {
                keyframe->base.is_in_motion = nanoem_true;
            }
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_ALREADY_EXISTS);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableMotionAddMorphKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_morph_keyframe_t *keyframe, const nanoem_unicode_string_t *name, nanoem_frame_index_t frame_index, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    nanoem_motion_t *origin_motion;
    kh_motion_track_bundle_t *track;
    int ret;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(keyframe) && nanoem_is_not_null(name)) {
        origin_motion = motion->origin;
        track = origin_motion->local_morph_motion_track_bundle;
        factory = origin_motion->factory;
        if (nanoemMotionFindMorphKeyframeObject(origin_motion, name, frame_index) == NULL) {
            nanoemMotionKeyframeObjectArrayAddObject((nanoem_motion_keyframe_object_t ***) &origin_motion->morph_keyframes, (nanoem_motion_keyframe_object_t *) keyframe->origin, frame_index, &origin_motion->num_morph_keyframes, &motion->num_allocated_morph_keyframes, status);
            if (!nanoem_status_ptr_has_error(status)) {
                nanoemMutableMotionMorphKeyframeSetName(keyframe, name, status);
                if (!nanoem_status_ptr_has_error(status)) {
                    nanoemMotionTrackBundleAddKeyframe(track, (nanoem_motion_keyframe_object_t *) keyframe->origin, frame_index, name, factory, &ret);
                    keyframe->base.is_in_motion = nanoem_true;
                    nanoem_status_ptr_assign(status, ret >= 0 ? NANOEM_STATUS_SUCCESS : NANOEM_STATUS_ERROR_MALLOC_FAILED);
                }
            }
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_ALREADY_EXISTS);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableMotionAddSelfShadowKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status)
{
    nanoem_motion_t *origin_motion;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(keyframe)) {
        origin_motion = motion->origin;
        if (nanoemMotionFindSelfShadowKeyframeObject(origin_motion, frame_index) == NULL) {
            nanoemMotionKeyframeObjectArrayAddObject((nanoem_motion_keyframe_object_t ***) &origin_motion->self_shadow_keyframes, (nanoem_motion_keyframe_object_t *) keyframe->origin, frame_index, &origin_motion->num_self_shadow_keyframes, &motion->num_allocated_self_shadow_keyframes, status);
            if (!nanoem_status_ptr_has_error(status)) {
                keyframe->base.is_in_motion = nanoem_true;
            }
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_ALREADY_EXISTS);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableMotionRemoveAccessoryKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_motion_t *origin_motion;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(keyframe)) {
        origin_motion = motion->origin;
        nanoemMotionKeyframeObjectArrayRemoveObject((nanoem_motion_keyframe_object_t **) origin_motion->accessory_keyframes, (nanoem_motion_keyframe_object_t *) keyframe->origin, &origin_motion->num_accessory_keyframes, NANOEM_STATUS_ERROR_MOTION_ACCESSORY_KEYFRAME_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            keyframe->base.is_in_motion = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableMotionRemoveBoneKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_bone_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_unicode_string_t *name;
    nanoem_motion_bone_keyframe_t *origin_keyframe;
    nanoem_motion_t *origin_motion;
    kh_motion_track_bundle_t *track;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(keyframe)) {
        origin_motion = motion->origin;
        origin_keyframe = keyframe->origin;
        nanoemMotionKeyframeObjectArrayRemoveObject((nanoem_motion_keyframe_object_t **) origin_motion->bone_keyframes, (nanoem_motion_keyframe_object_t *) origin_keyframe, &origin_motion->num_bone_keyframes, NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            track = origin_motion->local_bone_motion_track_bundle;
            name = nanoemMotionTrackBundleResolveName(track, origin_keyframe->bone_id);
            nanoemMotionTrackBundleRemoveKeyframe(track, origin_keyframe->base.frame_index, name, origin_motion->factory);
            keyframe->base.is_in_motion = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableMotionRemoveCameraKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_motion_t *origin_motion;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(keyframe)) {
        origin_motion = motion->origin;
        nanoemMotionKeyframeObjectArrayRemoveObject((nanoem_motion_keyframe_object_t **) origin_motion->camera_keyframes, (nanoem_motion_keyframe_object_t *) keyframe->origin, &origin_motion->num_camera_keyframes, NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            keyframe->base.is_in_motion = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableMotionRemoveLightKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_light_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_motion_t *origin_motion;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(keyframe)) {
        origin_motion = motion->origin;
        nanoemMotionKeyframeObjectArrayRemoveObject((nanoem_motion_keyframe_object_t **) origin_motion->light_keyframes, (nanoem_motion_keyframe_object_t *) keyframe->origin, &origin_motion->num_light_keyframes, NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            keyframe->base.is_in_motion = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableMotionRemoveModelKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_motion_t *origin_motion;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(keyframe)) {
        origin_motion = motion->origin;
        nanoemMotionKeyframeObjectArrayRemoveObject((nanoem_motion_keyframe_object_t **) origin_motion->model_keyframes, (nanoem_motion_keyframe_object_t *) keyframe->origin, &origin_motion->num_model_keyframes, NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            keyframe->base.is_in_motion = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableMotionRemoveMorphKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_morph_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_unicode_string_t *name;
    nanoem_motion_morph_keyframe_t *origin_keyframe;
    nanoem_motion_t *origin_motion;
    kh_motion_track_bundle_t *track;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(keyframe)) {
        origin_motion = motion->origin;
        origin_keyframe = keyframe->origin;
        nanoemMotionKeyframeObjectArrayRemoveObject((nanoem_motion_keyframe_object_t **) origin_motion->morph_keyframes, (nanoem_motion_keyframe_object_t *) origin_keyframe, &origin_motion->num_morph_keyframes, NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            track = origin_motion->local_morph_motion_track_bundle;
            name = nanoemMotionTrackBundleResolveName(track, origin_keyframe->morph_id);
            nanoemMotionTrackBundleRemoveKeyframe(track, origin_keyframe->base.frame_index, name, origin_motion->factory);
            keyframe->base.is_in_motion = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableMotionRemoveSelfShadowKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_motion_t *origin_motion;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(keyframe)) {
        origin_motion = motion->origin;
        nanoemMotionKeyframeObjectArrayRemoveObject((nanoem_motion_keyframe_object_t **) origin_motion->self_shadow_keyframes, (nanoem_motion_keyframe_object_t *) keyframe->origin, &origin_motion->num_self_shadow_keyframes, NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            keyframe->base.is_in_motion = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

NANOEM_DECL_INLINE static void
nanoemMutableMotionSetMaxFrameIndex(nanoem_motion_t *origin, nanoem_frame_index_t value)
{
    if (value > origin->max_frame_index) {
        origin->max_frame_index = value;
    }
}

void APIENTRY
nanoemMutableMotionSortAllKeyframes(nanoem_mutable_motion_t *motion)
{
    nanoem_motion_t *origin;
    if (nanoem_is_not_null(motion)) {
        origin = motion->origin;
        origin->max_frame_index = 0;
        if (origin->num_accessory_keyframes > 0) {
            nanoem_crt_qsort(origin->accessory_keyframes, origin->num_accessory_keyframes, sizeof(*origin->accessory_keyframes), nanoemMotionCompareKeyframe);
            nanoemMutableMotionSetMaxFrameIndex(origin, nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionAccessoryKeyframeGetKeyframeObject(origin->accessory_keyframes[origin->num_accessory_keyframes - 1])));
        }
        if (origin->num_bone_keyframes > 0) {
            nanoem_crt_qsort(origin->bone_keyframes, origin->num_bone_keyframes, sizeof(*origin->bone_keyframes), nanoemMotionCompareKeyframe);
            nanoemMutableMotionSetMaxFrameIndex(origin, nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(origin->bone_keyframes[origin->num_bone_keyframes - 1])));
        }
        if (origin->num_camera_keyframes > 0) {
            nanoem_crt_qsort(origin->camera_keyframes, origin->num_camera_keyframes, sizeof(*origin->camera_keyframes), nanoemMotionCompareKeyframe);
            nanoemMutableMotionSetMaxFrameIndex(origin, nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(origin->camera_keyframes[origin->num_camera_keyframes - 1])));
        }
        if (origin->num_light_keyframes > 0) {
            nanoem_crt_qsort(origin->light_keyframes, origin->num_light_keyframes, sizeof(*origin->light_keyframes), nanoemMotionCompareKeyframe);
            nanoemMutableMotionSetMaxFrameIndex(origin, nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionLightKeyframeGetKeyframeObject(origin->light_keyframes[origin->num_light_keyframes - 1])));
        }
        if (origin->num_model_keyframes > 0) {
            nanoem_crt_qsort(origin->model_keyframes, origin->num_model_keyframes, sizeof(*origin->model_keyframes), nanoemMotionCompareKeyframe);
            nanoemMutableMotionSetMaxFrameIndex(origin, nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionModelKeyframeGetKeyframeObject(origin->model_keyframes[origin->num_model_keyframes - 1])));
        }
        if (origin->num_morph_keyframes > 0) {
            nanoem_crt_qsort(origin->morph_keyframes, origin->num_morph_keyframes, sizeof(*origin->morph_keyframes), nanoemMotionCompareKeyframe);
            nanoemMutableMotionSetMaxFrameIndex(origin, nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(origin->morph_keyframes[origin->num_morph_keyframes - 1])));
        }
        if (origin->num_self_shadow_keyframes > 0) {
            nanoem_crt_qsort(origin->self_shadow_keyframes, origin->num_self_shadow_keyframes, sizeof(*origin->self_shadow_keyframes), nanoemMotionCompareKeyframe);
            nanoemMutableMotionSetMaxFrameIndex(origin, nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionSelfShadowKeyframeGetKeyframeObject(origin->self_shadow_keyframes[origin->num_self_shadow_keyframes - 1])));
        }
    }
}

void APIENTRY
nanoemMutableMotionSetAnnotation(nanoem_mutable_motion_t *motion, const char *key, const char *value, nanoem_status_t *status)
{
    int ret;
    if (nanoem_is_not_null(motion)) {
        nanoemAnnotationSet(&motion->origin->annotations, key, value, &ret, status);
    }
}

void APIENTRY
nanoemMutableMotionSetTargetModelName(nanoem_mutable_motion_t *motion, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    nanoem_unicode_string_t *old_name;
    nanoem_motion_t *origin;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(value)) {
        origin = motion->origin;
        factory = origin->factory;
        old_name = origin->target_model_name;
        origin->target_model_name = nanoemUnicodeStringFactoryCloneString(factory, value, status);
        if (old_name) {
            nanoemUtilDestroyString(old_name, factory);
        }
    }
}

nanoem_bool_t APIENTRY
nanoemMutableMotionSaveToBuffer(nanoem_mutable_motion_t *motion, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    nanoem_u8_t name_buffer[VMD_TARGET_MODEL_NAME_LENGTH_V2], *name_cp932;
    nanoem_rsize_t length;
    nanoem_status_ptr_assign(status , NANOEM_STATUS_ERROR_NULL_OBJECT);
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(buffer)) {
        nanoem_status_ptr_assign_succeeded(status);
        nanoemMutableBufferWriteByteArray(buffer, (const nanoem_u8_t *) __nanoem_vmd_signature_type2, sizeof(__nanoem_vmd_signature_type2), status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, 0, status);
        if (nanoem_status_ptr_has_error(status)) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_INVALID_SIGNATURE);
        }
        factory = motion->origin->factory;
        name_cp932 = nanoemUnicodeStringFactoryGetByteArrayEncoding(factory, motion->origin->target_model_name, &length, NANOEM_CODEC_TYPE_SJIS, status);
        nanoem_crt_memset(name_buffer, 0, sizeof(name_buffer));
        if (!nanoem_status_ptr_has_error(status) && nanoem_is_not_null(name_cp932)) {
            nanoemUtilCopyString((char *) name_buffer, sizeof(name_buffer), (const char *) name_cp932, length);
        }
        nanoemUnicodeStringFactoryDestroyByteArray(factory, name_cp932);
        nanoemMutableBufferWriteByteArray(buffer, name_buffer, sizeof(name_buffer), status);
        if (nanoem_status_ptr_has_error(status)) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MOTION_TARGET_NAME_CORRUPTED);
        }
        nanoemMutableMotionSaveToBufferBoneKeyframeBlockVMD(motion, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            return nanoem_false;
        }
        nanoemMutableMotionSaveToBufferMorphKeyframeBlockVMD(motion, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            return nanoem_false;
        }
        nanoemMutableMotionSaveToBufferCameraKeyframeBlockVMD(motion, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            return nanoem_false;
        }
        nanoemMutableMotionSaveToBufferLightKeyframeBlockVMD(motion, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            return nanoem_false;
        }
        nanoemMutableMotionSaveToBufferSelfShadowKeyframeBlockVMD(motion, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            return nanoem_false;
        }
        nanoemMutableMotionSaveToBufferModelKeyframeBlockVMD(motion, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            return nanoem_false;
        }
        nanoem_status_ptr_assign_succeeded(status);
    }
    return !nanoem_status_ptr_has_error(status);
}

void APIENTRY
nanoemMutableMotionResetAllocationSize(nanoem_mutable_motion_t *motion)
{
    const nanoem_motion_t *origin;
    if (nanoem_is_not_null(motion)) {
        origin = motion->origin;
        motion->num_allocated_accessory_keyframes = origin->num_accessory_keyframes;
        motion->num_allocated_bone_keyframes = origin->num_bone_keyframes;
        motion->num_allocated_camera_keyframes = origin->num_camera_keyframes;
        motion->num_allocated_light_keyframes = origin->num_light_keyframes;
        motion->num_allocated_model_keyframes = origin->num_model_keyframes;
        motion->num_allocated_morph_keyframes = origin->num_morph_keyframes;
        motion->num_allocated_self_shadow_keyframes = origin->num_self_shadow_keyframes;
    }
}

nanoem_motion_t *APIENTRY
nanoemMutableMotionGetOriginObject(nanoem_mutable_motion_t *motion)
{
    return nanoem_is_not_null(motion) ? motion->origin : NULL;
}

nanoem_motion_t *APIENTRY
nanoemMutableMotionGetOriginObjectReference(nanoem_mutable_motion_t *motion)
{
    nanoem_motion_t *origin = NULL;
    if (nanoem_is_not_null(motion)) {
        origin = motion->origin;
        motion->is_reference = nanoem_true;
    }
    return origin;
}

void APIENTRY
nanoemMutableMotionDestroy(nanoem_mutable_motion_t *motion)
{
    if (nanoem_is_not_null(motion)) {
        if (!motion->is_reference) {
            nanoemMotionDestroy(motion->origin);
        }
        nanoem_free(motion);
    }
}

static void
nanoemMutableModelVertexInitialize(nanoem_mutable_model_vertex_t *vertex, nanoem_model_vertex_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model)
{
    vertex->origin = origin;
    vertex->base.is_reference = is_reference;
    vertex->base.is_in_model = is_in_model;
    if (vertex->origin->type == NANOEM_MODEL_VERTEX_TYPE_UNKNOWN) {
        vertex->origin->type = NANOEM_MODEL_VERTEX_TYPE_BDEF2;
    }
}

static nanoem_mutable_model_vertex_t *
nanoemMutableModelVertexCreateInternal(nanoem_model_vertex_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model, nanoem_status_t *status)
{
    nanoem_mutable_model_vertex_t *vertex = NULL;
    if (nanoem_is_not_null(origin)) {
        vertex = (nanoem_mutable_model_vertex_t *) nanoem_calloc(1, sizeof(*vertex), status);
        if (nanoem_is_not_null(vertex)) {
            nanoemMutableModelVertexInitialize(vertex, origin, is_reference, is_in_model);
        }
    }
    return vertex;
}

static void
nanoemMutableModelMaterialInitialize(nanoem_mutable_model_material_t *material, nanoem_model_material_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model)
{
    material->origin = origin;
    material->base.is_reference = is_reference;
    material->base.is_in_model = is_in_model;
    if (material->origin->sphere_map_texture_type == NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_UNKNOWN) {
        material->origin->sphere_map_texture_type = NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE;
    }
}

static nanoem_mutable_model_material_t *
nanoemMutableModelMaterialCreateInternal(nanoem_model_material_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model, nanoem_status_t *status)
{
    nanoem_mutable_model_material_t *material = NULL;
    if (nanoem_is_not_null(origin)) {
        material = (nanoem_mutable_model_material_t *) nanoem_calloc(1, sizeof(*material), status);
        if (nanoem_is_not_null(material)) {
            nanoemMutableModelMaterialInitialize(material, origin, is_reference, is_in_model);
        }
    }
    return material;
}

static void
nanoemMutableModelBoneInitialize(nanoem_mutable_model_bone_t *bone, nanoem_model_bone_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model)
{
    bone->origin = origin;
    bone->base.is_reference = is_reference;
    bone->base.is_in_model = is_in_model;
}

static nanoem_mutable_model_bone_t *
nanoemMutableModelBoneCreateInternal(nanoem_model_bone_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model, nanoem_status_t *status)
{
    nanoem_mutable_model_bone_t *bone = NULL;
    if (nanoem_is_not_null(origin)) {
        bone = (nanoem_mutable_model_bone_t *) nanoem_calloc(1, sizeof(*bone), status);
        if (nanoem_is_not_null(bone)) {
            nanoemMutableModelBoneInitialize(bone, origin, is_reference, is_in_model);
        }
    }
    return bone;
}

static void
nanoemMutableModelConstraintInitialize(nanoem_mutable_model_constraint_t *constraint, nanoem_model_constraint_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model)
{
    constraint->origin = origin;
    constraint->base.is_reference = is_reference;
    constraint->base.is_in_model = is_in_model;
}

static nanoem_mutable_model_constraint_t *
nanoemMutableModelConstraintCreateInternal(nanoem_model_constraint_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model, nanoem_status_t *status)
{
    nanoem_mutable_model_constraint_t *constraint = NULL;
    if (nanoem_is_not_null(origin)) {
        constraint = (nanoem_mutable_model_constraint_t *) nanoem_calloc(1, sizeof(*constraint), status);
        if (nanoem_is_not_null(constraint)) {
            nanoemMutableModelConstraintInitialize(constraint, origin, is_reference, is_in_model);
        }
    }
    return constraint;
}

static void
nanoemMutableModelConstraintJointInitialize(nanoem_mutable_model_constraint_joint_t *joint, nanoem_model_constraint_joint_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model)
{
    joint->origin = origin;
    joint->base.is_reference = is_reference;
    joint->base.is_in_model = is_in_model;
}

static nanoem_mutable_model_constraint_joint_t *
nanoemMutableModelConstraintJointCreateInternal(nanoem_model_constraint_joint_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model, nanoem_status_t *status)
{
    nanoem_mutable_model_constraint_joint_t *joint = NULL;
    if (nanoem_is_not_null(origin)) {
        joint = (nanoem_mutable_model_constraint_joint_t *) nanoem_calloc(1, sizeof(*joint), status);
        if (nanoem_is_not_null(joint)) {
            nanoemMutableModelConstraintJointInitialize(joint, origin, is_reference, is_in_model);
        }
    }
    return joint;
}

static void
nanoemMutableModelTextureInitialize(nanoem_mutable_model_texture_t *texture, nanoem_model_texture_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model)
{
    texture->origin = origin;
    texture->base.is_reference = is_reference;
    texture->base.is_in_model = is_in_model;
}

static nanoem_mutable_model_texture_t *
nanoemMutableModelTextureCreateInternal(nanoem_model_texture_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model, nanoem_status_t *status)
{
    nanoem_mutable_model_texture_t *texture = NULL;
    if (nanoem_is_not_null(origin)) {
        texture = (nanoem_mutable_model_texture_t *) nanoem_calloc(1, sizeof(*texture), status);
        if (nanoem_is_not_null(texture)) {
            nanoemMutableModelTextureInitialize(texture, origin, is_reference, is_in_model);
        }
    }
    return texture;
}

static void
nanoemMutableModelMorphInitialize(nanoem_mutable_model_morph_t *morph, nanoem_model_morph_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model)
{
    morph->origin = origin;
    morph->base.is_reference = is_reference;
    morph->base.is_in_model = is_in_model;
    if (morph->origin->category == NANOEM_MODEL_MORPH_CATEGORY_UNKNOWN) {
        morph->origin->category = NANOEM_MODEL_MORPH_CATEGORY_OTHER;
    }
    if (morph->origin->type == NANOEM_MODEL_MORPH_TYPE_UNKNOWN) {
        morph->origin->type = NANOEM_MODEL_MORPH_TYPE_VERTEX;
    }
}

static nanoem_mutable_model_morph_t *
nanoemMutableModelMorphCreateInternal(nanoem_model_morph_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model, nanoem_status_t *status)
{
    nanoem_mutable_model_morph_t *morph = NULL;
    if (nanoem_is_not_null(origin)) {
        morph = (nanoem_mutable_model_morph_t *) nanoem_calloc(1, sizeof(*morph), status);
        if (nanoem_is_not_null(morph)) {
            nanoemMutableModelMorphInitialize(morph, origin, is_reference, is_in_model);
        }
    }
    return morph;
}

static void
nanoemMutableModelLabelInitialize(nanoem_mutable_model_label_t *label, nanoem_model_label_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model)
{
    label->origin = origin;
    label->base.is_reference = is_reference;
    label->base.is_in_model = is_in_model;
    label->num_allocated_items = origin->num_items;
}

static nanoem_mutable_model_label_t *
nanoemMutableModelLabelCreateInternal(nanoem_model_label_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model, nanoem_status_t *status)
{
    nanoem_mutable_model_label_t *label = NULL;
    if (nanoem_is_not_null(origin)) {
        label = (nanoem_mutable_model_label_t *) nanoem_calloc(1, sizeof(*label), status);
        if (nanoem_is_not_null(label)) {
            nanoemMutableModelLabelInitialize(label, origin, is_reference, is_in_model);
        }
    }
    return label;
}

static void
nanoemMutableModelRigidBodyInitialize(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_model_rigid_body_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model)
{
    rigid_body->origin = origin;
    rigid_body->base.is_reference = is_reference;
    rigid_body->base.is_in_model = is_in_model;
    if (rigid_body->origin->shape_type == NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_UNKNOWN) {
        rigid_body->origin->shape_type = NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_BOX;
    }
    if (rigid_body->origin->transform_type == NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_UNKNOWN) {
        rigid_body->origin->transform_type = NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_TO_SIMULATION;
    }
}

static nanoem_mutable_model_rigid_body_t *
nanoemMutableModelRigidBodyCreateInternal(nanoem_model_rigid_body_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model, nanoem_status_t *status)
{
    nanoem_mutable_model_rigid_body_t *rigid_body = NULL;
    if (nanoem_is_not_null(origin)) {
        rigid_body = (nanoem_mutable_model_rigid_body_t *) nanoem_calloc(1, sizeof(*rigid_body), status);
        if (nanoem_is_not_null(rigid_body)) {
            nanoemMutableModelRigidBodyInitialize(rigid_body, origin, is_reference, is_in_model);
        }
    }
    return rigid_body;
}

static void
nanoemMutableModelJointInitialize(nanoem_mutable_model_joint_t *joint, nanoem_model_joint_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model)
{
    joint->origin = origin;
    joint->base.is_reference = is_reference;
    joint->base.is_in_model = is_in_model;
    if (joint->origin->type == NANOEM_MODEL_JOINT_TYPE_UNKNOWN) {
        joint->origin->type = NANOEM_MODEL_JOINT_TYPE_GENERIC_6DOF_SPRING_CONSTRAINT;
    }
}

static nanoem_mutable_model_joint_t *
nanoemMutableModelJointCreateInternal(nanoem_model_joint_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model, nanoem_status_t *status)
{
    nanoem_mutable_model_joint_t *joint = NULL;
    if (nanoem_is_not_null(origin)) {
        joint = (nanoem_mutable_model_joint_t *) nanoem_calloc(1, sizeof(*joint), status);
        if (nanoem_is_not_null(joint)) {
            nanoemMutableModelJointInitialize(joint, origin, is_reference, is_in_model);
        }
    }
    return joint;
}

static void
nanoemMutableModelSoftBodyInitialize(nanoem_mutable_model_soft_body_t *soft_body, nanoem_model_soft_body_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model)
{
    soft_body->origin = origin;
    soft_body->base.is_reference = is_reference;
    soft_body->base.is_in_model = is_in_model;
}

static nanoem_mutable_model_soft_body_t *
nanoemMutableModelSoftBodyCreateInternal(nanoem_model_soft_body_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model, nanoem_status_t *status)
{
    nanoem_mutable_model_soft_body_t *soft_body = NULL;
    if (nanoem_is_not_null(origin)) {
        soft_body = (nanoem_mutable_model_soft_body_t *) nanoem_calloc(1, sizeof(*soft_body), status);
        if (nanoem_is_not_null(soft_body)) {
            nanoemMutableModelSoftBodyInitialize(soft_body, origin, is_reference, is_in_model);
        }
    }
    return soft_body;
}

static void
nanoemMutableModelSoftBodyAnchorInitialize(nanoem_mutable_model_soft_body_anchor_t *anchor, nanoem_model_soft_body_anchor_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_soft_body)
{
    anchor->origin = origin;
    anchor->base.is_reference = is_reference;
    anchor->base.is_in_soft_body = is_in_soft_body;
    anchor->origin->rigid_body_index = -1;
    anchor->origin->vertex_index = -1;
}

static nanoem_mutable_model_soft_body_anchor_t *
nanoemMutableModelSoftBodyAnchorCreateInternal(nanoem_model_soft_body_anchor_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_soft_body, nanoem_status_t *status)
{
    nanoem_mutable_model_soft_body_anchor_t *anchor = NULL;
    if (nanoem_is_not_null(origin)) {
        anchor = (nanoem_mutable_model_soft_body_anchor_t *) nanoem_calloc(1, sizeof(*anchor), status);
        if (nanoem_is_not_null(anchor)) {
            nanoemMutableModelSoftBodyAnchorInitialize(anchor, origin, is_reference, is_in_soft_body);
        }
    }
    return anchor;
}

NANOEM_DECL_INLINE static void
nanoemUnicodeStringFactoryAssignString(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_t **left, const nanoem_unicode_string_t *right, nanoem_codec_type_t codec, nanoem_status_t *status)
{
    nanoem_mark_unused(codec);
    nanoem_unicode_string_t *s = *left;
    *left = nanoemUnicodeStringFactoryCloneString(factory, right, status);
    nanoemUnicodeStringFactoryDestroyString(factory, s);
}

NANOEM_DECL_INLINE static void
nanoemMutableModelTextureAssign(nanoem_model_texture_t **left, const nanoem_model_texture_t *right, nanoem_status_t *status)
{
    const nanoem_model_t *model = nanoemModelTextureGetParentModel(right);
    nanoem_model_texture_t *assign = *left;
    nanoem_unicode_string_factory_t *factory = nanoem_is_not_null(model) ? model->factory : NULL;
    if (nanoem_is_not_null(factory)) {
        if (assign == NULL) {
            *left = assign = nanoemModelTextureCreate(model, status);
        }
        nanoemUnicodeStringFactoryAssignString(factory, &assign->path, right->path, nanoemModelGetCodecType(model), status);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

static int
nanoemMutableModelMaterialSetTexture(const nanoem_model_t *model, nanoem_mutable_model_texture_t *value, nanoem_status_t *status)
{
    union nanoem_const_to_model_t {
        const nanoem_model_t *c;
        nanoem_model_t *v;
    } um;
    const nanoem_codec_type_t codec = nanoemModelGetCodecType(model);
    nanoem_unicode_string_factory_t *factory = model->factory;
    nanoem_unicode_string_t *path = value->origin->path;
    nanoem_model_texture_t **textures = model->textures, *texture;
    nanoem_mutable_model_t *mutable_model;
    nanoem_rsize_t num_textures = model->num_textures, i;
    nanoem_status_ptr_assign_succeeded(status);
    for (i = 0; i < num_textures; i++) {
        texture = textures[i];
        if (texture && nanoemUnicodeStringFactoryCompareString(factory, texture->path, path) == 0) {
            nanoemUnicodeStringFactoryAssignString(factory, &texture->path, path, codec, status);
            return (int) i;
        }
    }
    um.c = model;
    mutable_model = nanoemMutableModelCreateAsReference(um.v, status);
    nanoemMutableModelInsertTextureObject(mutable_model, value, -1, status);
    nanoemMutableModelDestroy(mutable_model);
    return (int) model->num_textures - 1;
}

nanoem_mutable_model_vertex_t *APIENTRY
nanoemMutableModelVertexCreate(nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_vertex_t *vertex = nanoemModelVertexCreate(model, status);
    return nanoemMutableModelVertexCreateInternal(vertex, nanoem_false, nanoem_false, status);
}

nanoem_mutable_model_vertex_t *APIENTRY
nanoemMutableModelVertexCreateAsReference(nanoem_model_vertex_t *vertex, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    nanoem_mutable_model_vertex_t *mutable_vertex = NULL;
    nanoem_bool_t is_in_model;
    if (nanoem_is_not_null(vertex)) {
        origin_model = nanoemModelVertexGetParentModel(vertex);
        if (nanoem_is_not_null(origin_model)) {
            is_in_model = nanoemObjectArrayContainsIndexedObject((const void *const *) origin_model->vertices, vertex, origin_model->num_vertices, vertex->base.index);
            mutable_vertex = nanoemMutableModelVertexCreateInternal(vertex, nanoem_true, is_in_model, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return mutable_vertex;
}

void APIENTRY
nanoemMutableModelVertexSetOrigin(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(vertex) && nanoem_is_not_null(value) && vertex->origin->origin.values != value) {
        nanoem_crt_memcpy(vertex->origin->origin.values, value, sizeof(vertex->origin->origin));
    }
}

void APIENTRY
nanoemMutableModelVertexSetNormal(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(vertex) && nanoem_is_not_null(value) && vertex->origin->normal.values != value) {
        nanoem_crt_memcpy(vertex->origin->normal.values, value, sizeof(vertex->origin->normal));
    }
}

void APIENTRY
nanoemMutableModelVertexSetTexCoord(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(vertex) && nanoem_is_not_null(value) && vertex->origin->uv.values != value) {
        nanoem_crt_memcpy(vertex->origin->uv.values, value, sizeof(vertex->origin->uv));
    }
}

void APIENTRY
nanoemMutableModelVertexSetAdditionalUV(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value, nanoem_rsize_t index)
{
    if (nanoem_is_not_null(vertex) && nanoem_is_not_null(value) && index < 4 && vertex->origin->additional_uv[index].values != value) {
        nanoem_crt_memcpy(vertex->origin->additional_uv[index].values, value, sizeof(vertex->origin->additional_uv[0]));
    }
}

void APIENTRY
nanoemMutableModelVertexSetSdefC(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(vertex) && nanoem_is_not_null(value) && vertex->origin->sdef_c.values != value) {
        nanoem_crt_memcpy(vertex->origin->sdef_c.values, value, sizeof(vertex->origin->sdef_c));
    }
}

void APIENTRY
nanoemMutableModelVertexSetSdefR0(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(vertex) && nanoem_is_not_null(value) && vertex->origin->sdef_r0.values != value) {
        nanoem_crt_memcpy(vertex->origin->sdef_r0.values, value, sizeof(vertex->origin->sdef_r0));
    }
}

void APIENTRY
nanoemMutableModelVertexSetSdefR1(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(vertex) && nanoem_is_not_null(value) && vertex->origin->sdef_r1.values != value) {
        nanoem_crt_memcpy(vertex->origin->sdef_r1.values, value, sizeof(vertex->origin->sdef_r1));
    }
}

void APIENTRY
nanoemMutableModelVertexSetBoneObject(nanoem_mutable_model_vertex_t *vertex, const nanoem_model_bone_t *value, nanoem_rsize_t index)
{
    const nanoem_model_t *parent_model;
    if (nanoem_is_not_null(vertex) && index < 4) {
        parent_model = nanoemModelVertexGetParentModel(vertex->origin);
        vertex->origin->bone_indices[index] = nanoemModelResolveBoneObject(parent_model, value);
    }
}

void APIENTRY
nanoemMutableModelVertexSetBoneWeight(nanoem_mutable_model_vertex_t *vertex, nanoem_f32_t value, nanoem_rsize_t index)
{
    if (nanoem_is_not_null(vertex) && index < 4) {
        vertex->origin->bone_weights.values[index] = value;
    }
}

void APIENTRY
nanoemMutableModelVertexSetEdgeSize(nanoem_mutable_model_vertex_t *vertex, nanoem_f32_t value)
{
    if (nanoem_is_not_null(vertex)) {
        vertex->origin->edge_size = value;
    }
}

void APIENTRY
nanoemMutableModelVertexSetType(nanoem_mutable_model_vertex_t *vertex, nanoem_model_vertex_type_t value)
{
    if (nanoem_is_not_null(vertex)) {
        vertex->origin->type = value;
        switch (value) {
        case NANOEM_MODEL_VERTEX_TYPE_BDEF1:
            vertex->origin->num_bone_indices = vertex->origin->num_bone_weights = 1;
            break;
        case NANOEM_MODEL_VERTEX_TYPE_BDEF2:
        case NANOEM_MODEL_VERTEX_TYPE_SDEF:
            vertex->origin->num_bone_indices = 2;
            vertex->origin->num_bone_weights = 1;
            break;
        case NANOEM_MODEL_VERTEX_TYPE_BDEF4:
        case NANOEM_MODEL_VERTEX_TYPE_QDEF:
            vertex->origin->num_bone_indices = vertex->origin->num_bone_weights = 4;
            break;
        case NANOEM_MODEL_VERTEX_TYPE_MAX_ENUM:
        case NANOEM_MODEL_VERTEX_TYPE_UNKNOWN:
            vertex->origin->num_bone_indices = vertex->origin->num_bone_weights = 0;
            break;
        }
    }
}

void APIENTRY
nanoemMutableModelVertexCopy(nanoem_mutable_model_vertex_t *vertex, const nanoem_model_vertex_t *value)
{
    nanoem_rsize_t i = 0;
    nanoemMutableModelVertexSetOrigin(vertex, nanoemModelVertexGetOrigin(value));
    nanoemMutableModelVertexSetNormal(vertex, nanoemModelVertexGetNormal(value));
    nanoemMutableModelVertexSetTexCoord(vertex, nanoemModelVertexGetTexCoord(value));
    nanoemMutableModelVertexSetSdefC(vertex, nanoemModelVertexGetSdefC(value));
    nanoemMutableModelVertexSetSdefR0(vertex, nanoemModelVertexGetSdefR0(value));
    nanoemMutableModelVertexSetSdefR1(vertex, nanoemModelVertexGetSdefR1(value));
    nanoemMutableModelVertexSetEdgeSize(vertex, nanoemModelVertexGetEdgeSize(value));
    nanoemMutableModelVertexSetType(vertex, nanoemModelVertexGetType(value));
    for (i = 0; i < 4; i++) {
        nanoemMutableModelVertexSetAdditionalUV(vertex, nanoemModelVertexGetAdditionalUV(value, i), i);
        nanoemMutableModelVertexSetBoneObject(vertex, nanoemModelVertexGetBoneObject(value, i), i);
        nanoemMutableModelVertexSetBoneWeight(vertex, nanoemModelVertexGetBoneWeight(value, i), i);
    }
}

nanoem_model_vertex_t *APIENTRY
nanoemMutableModelVertexGetOriginObject(nanoem_mutable_model_vertex_t *vertex)
{
    return nanoem_is_not_null(vertex) ? vertex->origin : NULL;
}

void APIENTRY
nanoemMutableModelVertexSaveToBuffer(nanoem_mutable_model_vertex_t *vertex, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_vertex_t *origin;
    nanoem_model_vertex_type_t type;
    nanoem_u8_t weight;
    nanoem_rsize_t i, size;
    if (nanoem_is_not_null(vertex) && nanoem_is_not_null(buffer)) {
        origin = vertex->origin;
        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->origin, status);
        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->normal, status);
        nanoemMutableBufferWriteFloat32x2LittleEndian(buffer, &origin->uv, status);
        parent_model = nanoemModelVertexGetParentModel(origin);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
        }
        else if (nanoemModelIsPMX(parent_model)) {
            size = parent_model->info.additional_uv_size;
            for (i = 0; i < size; i++) {
                nanoemMutableBufferWriteFloat32x4LittleEndian(buffer, &origin->additional_uv[i], status);
            }
            type = origin->type;
            size = parent_model->info.bone_index_size;
            switch (type) {
            case NANOEM_MODEL_VERTEX_TYPE_BDEF1: {
                nanoemMutableBufferWriteByte(buffer, type, status);
                nanoemMutableBufferWriteInteger(buffer, origin->bone_indices[0], size, status);
                break;
            }
            case NANOEM_MODEL_VERTEX_TYPE_BDEF2: {
                if (origin->bone_weights.values[0] > 0.9995f) {
                    nanoemMutableBufferWriteByte(buffer, NANOEM_MODEL_VERTEX_TYPE_BDEF1, status);
                    nanoemMutableBufferWriteInteger(buffer, origin->bone_indices[0], size, status);
                }
                else if (origin->bone_weights.values[0] < 0.0005f) {
                    nanoemMutableBufferWriteByte(buffer, NANOEM_MODEL_VERTEX_TYPE_BDEF1, status);
                    nanoemMutableBufferWriteInteger(buffer, origin->bone_indices[1], size, status);
                }
                else if (origin->bone_weights.values[1] > origin->bone_weights.values[0]) {
                    nanoemMutableBufferWriteByte(buffer, type, status);
                    nanoemMutableBufferWriteInteger(buffer, origin->bone_indices[1], size, status);
                    nanoemMutableBufferWriteInteger(buffer, origin->bone_indices[0], size, status);
                    nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->bone_weights.values[1], status);
                }
                else {
                    nanoemMutableBufferWriteByte(buffer, type, status);
                    nanoemMutableBufferWriteInteger(buffer, origin->bone_indices[0], size, status);
                    nanoemMutableBufferWriteInteger(buffer, origin->bone_indices[1], size, status);
                    nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->bone_weights.values[0], status);
                }
                break;
            }
            case NANOEM_MODEL_VERTEX_TYPE_BDEF4:
            case NANOEM_MODEL_VERTEX_TYPE_QDEF: {
                nanoemMutableBufferWriteByte(buffer, type, status);
                nanoemMutableBufferWriteInteger(buffer, origin->bone_indices[0], size, status);
                nanoemMutableBufferWriteInteger(buffer, origin->bone_indices[1], size, status);
                nanoemMutableBufferWriteInteger(buffer, origin->bone_indices[2], size, status);
                nanoemMutableBufferWriteInteger(buffer, origin->bone_indices[3], size, status);
                nanoemMutableBufferWriteFloat32x4LittleEndian(buffer, &origin->bone_weights, status);
                break;
            }
            case NANOEM_MODEL_VERTEX_TYPE_SDEF: {
                nanoemMutableBufferWriteByte(buffer, type, status);
                nanoemMutableBufferWriteInteger(buffer, origin->bone_indices[0], size, status);
                nanoemMutableBufferWriteInteger(buffer, origin->bone_indices[1], size, status);
                nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->bone_weights.values[0], status);
                nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->sdef_c, status);
                nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->sdef_r0, status);
                nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->sdef_r1, status);
                break;
            }
            case NANOEM_MODEL_VERTEX_TYPE_MAX_ENUM:
            case NANOEM_MODEL_VERTEX_TYPE_UNKNOWN:
            default:
                nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_VERTEX_CORRUPTED);
            }
            nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->edge_size, status);
        }
        else {
            weight = origin->bone_weight_origin != 0 ? origin->bone_weight_origin : (nanoem_u8_t)(origin->bone_weights.values[0] * 100.0f);
            nanoemMutableBufferWriteInt16LittleEndian(buffer, origin->bone_indices[0], status);
            nanoemMutableBufferWriteInt16LittleEndian(buffer, origin->bone_indices[1], status);
            nanoemMutableBufferWriteByte(buffer, weight, status);
            nanoemMutableBufferWriteByte(buffer, origin->edge_size != 0.0f ? 0 : 1, status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelVertexDestroy(nanoem_mutable_model_vertex_t *vertex)
{
    if (nanoem_is_not_null(vertex)) {
        if (nanoemMutableBaseModelObjectCanDelete(&vertex->base)) {
            nanoemModelVertexDestroy(vertex->origin);
        }
        nanoem_free(vertex);
    }
}

nanoem_mutable_model_material_t *APIENTRY
nanoemMutableModelMaterialCreate(nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_material_t *material = nanoemModelMaterialCreate(model, status);
    return nanoemMutableModelMaterialCreateInternal(material, nanoem_false, nanoem_false, status);
}

nanoem_mutable_model_material_t *APIENTRY
nanoemMutableModelMaterialCreateAsReference(nanoem_model_material_t *material, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    nanoem_mutable_model_material_t *mutable_material = NULL;
    nanoem_bool_t is_in_model;
    if (nanoem_is_not_null(material)) {
        origin_model = nanoemModelMaterialGetParentModel(material);
        if (nanoem_is_not_null(origin_model)) {
            is_in_model = nanoemObjectArrayContainsIndexedObject((const void *const *) origin_model->materials, material, origin_model->num_materials, material->base.index);
            mutable_material = nanoemMutableModelMaterialCreateInternal(material, nanoem_true, is_in_model, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return mutable_material;
}

void APIENTRY
nanoemMutableModelMaterialSetName(nanoem_mutable_model_material_t *material, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(material)) {
        parent_model = nanoemModelMaterialGetParentModel(material->origin);
        if (nanoem_is_not_null(parent_model)) {
            factory = parent_model->factory;
            switch (language) {
            case NANOEM_LANGUAGE_TYPE_ENGLISH:
                nanoemUnicodeStringFactoryAssignString(factory, &material->origin->name_en, value, nanoemModelGetCodecType(parent_model), status);
                break;
            case NANOEM_LANGUAGE_TYPE_JAPANESE:
                nanoemUnicodeStringFactoryAssignString(factory, &material->origin->name_ja, value, nanoemModelGetCodecType(parent_model), status);
                break;
            case NANOEM_LANGUAGE_TYPE_MAX_ENUM:
            case NANOEM_LANGUAGE_TYPE_UNKNOWN:
            default:
                break;
            }
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMaterialSetDiffuseTextureObject(nanoem_mutable_model_material_t *material, nanoem_mutable_model_texture_t *value, nanoem_status_t *status)
{
    int index;
    if (nanoem_is_not_null(material)) {
        if (nanoem_is_not_null(value)) {
            index = nanoemMutableModelMaterialSetTexture(nanoemModelMaterialGetParentModel(material->origin), value, status);
            nanoemMutableModelTextureAssign(&material->origin->diffuse_texture, value->origin, status);
        }
        else {
            index = NANOEM_MODEL_OBJECT_NOT_FOUND;
        }
        material->origin->diffuse_texture_index = index;
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMaterialSetSphereMapTextureObject(nanoem_mutable_model_material_t *material, nanoem_mutable_model_texture_t *value, nanoem_status_t *status)
{
    int index;
    if (nanoem_is_not_null(material)) {
        if (nanoem_is_not_null(value)) {
            switch (material->origin->sphere_map_texture_type) {
            case NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_ADD:
                index = nanoemMutableModelMaterialSetTexture(nanoemModelMaterialGetParentModel(material->origin), value, status);
                nanoemMutableModelTextureAssign(&material->origin->sphere_map_texture_spa, value->origin, status);
                break;
            case NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_MULTIPLY:
                index = nanoemMutableModelMaterialSetTexture(nanoemModelMaterialGetParentModel(material->origin), value, status);
                nanoemMutableModelTextureAssign(&material->origin->sphere_map_texture_sph, value->origin, status);
                break;
            case NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_SUB_TEXTURE:
                index = nanoemMutableModelMaterialSetTexture(nanoemModelMaterialGetParentModel(material->origin), value, status);
                break;
            case NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_MAX_ENUM:
            case NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE:
            case NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_UNKNOWN:
            default:
                index = NANOEM_MODEL_OBJECT_NOT_FOUND;
                break;
            }
        }
        else {
            index = NANOEM_MODEL_OBJECT_NOT_FOUND;
        }
        material->origin->sphere_map_texture_index = index;
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMaterialSetToonTextureObject(nanoem_mutable_model_material_t *material, nanoem_mutable_model_texture_t *value, nanoem_status_t *status)
{
    int index;
    if (nanoem_is_not_null(material)) {
        index = nanoem_is_not_null(value) ? nanoemMutableModelMaterialSetTexture(nanoemModelMaterialGetParentModel(material->origin), value, status) : NANOEM_MODEL_OBJECT_NOT_FOUND;
        material->origin->toon_texture_index = index;
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMaterialSetClob(nanoem_mutable_model_material_t *material, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(material)) {
        parent_model = nanoemModelMaterialGetParentModel(material->origin);
        if (nanoem_is_not_null(parent_model)) {
            factory = parent_model->factory;
            nanoemUnicodeStringFactoryAssignString(factory, &material->origin->clob, value, nanoemModelGetCodecType(parent_model), status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMaterialSetAmbientColor(nanoem_mutable_model_material_t *material, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(material) && nanoem_is_not_null(value) && material->origin->ambient_color.values != value) {
        nanoem_crt_memcpy(material->origin->ambient_color.values, value, sizeof(material->origin->ambient_color));
    }
}

void APIENTRY
nanoemMutableModelMaterialSetDiffuseColor(nanoem_mutable_model_material_t *material, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(material) && nanoem_is_not_null(value) && material->origin->diffuse_color.values != value) {
        nanoem_crt_memcpy(material->origin->diffuse_color.values, value, sizeof(material->origin->diffuse_color));
    }
}

void APIENTRY
nanoemMutableModelMaterialSetSpecularColor(nanoem_mutable_model_material_t *material, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(material) && nanoem_is_not_null(value) && material->origin->specular_color.values != value) {
        nanoem_crt_memcpy(material->origin->specular_color.values, value, sizeof(material->origin->specular_color));
    }
}

void APIENTRY
nanoemMutableModelMaterialSetEdgeColor(nanoem_mutable_model_material_t *material, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(material) && nanoem_is_not_null(value) && material->origin->edge_color.values != value) {
        nanoem_crt_memcpy(material->origin->edge_color.values, value, sizeof(material->origin->edge_color));
    }
}

void APIENTRY
nanoemMutableModelMaterialSetDiffuseOpacity(nanoem_mutable_model_material_t *material, nanoem_f32_t value)
{
    if (nanoem_is_not_null(material)) {
        material->origin->diffuse_opacity = value;
    }
}

void APIENTRY
nanoemMutableModelMaterialSetEdgeOpacity(nanoem_mutable_model_material_t *material, nanoem_f32_t value)
{
    if (nanoem_is_not_null(material)) {
        material->origin->edge_opacity = value;
    }
}

void APIENTRY
nanoemMutableModelMaterialSetEdgeSize(nanoem_mutable_model_material_t *material, nanoem_f32_t value)
{
    if (nanoem_is_not_null(material)) {
        material->origin->edge_size = value;
    }
}

void APIENTRY
nanoemMutableModelMaterialSetSpecularPower(nanoem_mutable_model_material_t *material, nanoem_f32_t value)
{
    if (nanoem_is_not_null(material)) {
        material->origin->specular_power = value;
    }
}

void APIENTRY
nanoemMutableModelMaterialSetSphereMapTextureType(nanoem_mutable_model_material_t *material, nanoem_model_material_sphere_map_texture_type_t value)
{
    if (nanoem_is_not_null(material)) {
        material->origin->sphere_map_texture_type = value;
    }
}

void APIENTRY
nanoemMutableModelMaterialSetNumVertexIndices(nanoem_mutable_model_material_t *material, nanoem_rsize_t value)
{
    if (nanoem_is_not_null(material)) {
        material->origin->num_vertex_indices = value;
    }
}

void APIENTRY
nanoemMutableModelMaterialSetToonTextureIndex(nanoem_mutable_model_material_t *material, int value)
{
    if (nanoem_is_not_null(material) && value >= 0 && value < 10) {
        material->origin->toon_texture_index = value;
    }
}

void APIENTRY
nanoemMutableModelMaterialSetToonShared(nanoem_mutable_model_material_t *material, nanoem_bool_t value)
{
    if (nanoem_is_not_null(material)) {
        material->origin->is_toon_shared = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelMaterialSetCullingDisabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value)
{
    if (nanoem_is_not_null(material)) {
        material->origin->u.flags.is_culling_disabled = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelMaterialSetCastingShadowEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value)
{
    if (nanoem_is_not_null(material)) {
        material->origin->u.flags.is_casting_shadow_enabled = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelMaterialSetCastingShadowMapEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value)
{
    if (nanoem_is_not_null(material)) {
        material->origin->u.flags.is_casting_shadow_map_enabled = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelMaterialSetShadowMapEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value)
{
    if (nanoem_is_not_null(material)) {
        material->origin->u.flags.is_shadow_map_enabled = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelMaterialSetEdgeEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value)
{
    if (nanoem_is_not_null(material)) {
        material->origin->u.flags.is_edge_enabled = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelMaterialSetVertexColorEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value)
{
    if (nanoem_is_not_null(material)) {
        material->origin->u.flags.is_vertex_color_enabled = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelMaterialSetPointDrawEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value)
{
    if (nanoem_is_not_null(material)) {
        material->origin->u.flags.is_point_draw_enabled = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelMaterialSetLineDrawEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value)
{
    if (nanoem_is_not_null(material)) {
        material->origin->u.flags.is_line_draw_enabled = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelMaterialCopy(nanoem_mutable_model_material_t *material, const nanoem_model_material_t *value, nanoem_status_t *status)
{
    union nanoem_immutable_model_to_mutable_model_cast_t {
        const nanoem_model_t *p;
        nanoem_model_t *m;
    } u;
    nanoem_mutable_model_texture_t *texture;
    u.p = nanoemModelObjectGetParentModel(nanoemModelMaterialGetModelObject(nanoemMutableModelMaterialGetOriginObject(material)));
    nanoemMutableModelMaterialSetAmbientColor(material, nanoemModelMaterialGetAmbientColor(value));
    nanoemMutableModelMaterialSetCastingShadowEnabled(material, nanoemModelMaterialIsCastingShadowEnabled(value));
    nanoemMutableModelMaterialSetCastingShadowMapEnabled(material, nanoemModelMaterialIsCastingShadowMapEnabled(value));
    nanoemMutableModelMaterialSetClob(material, nanoemModelMaterialGetClob(value), status);
    nanoemMutableModelMaterialSetCullingDisabled(material, nanoemModelMaterialIsCullingDisabled(value));
    nanoemMutableModelMaterialSetDiffuseColor(material, nanoemModelMaterialGetDiffuseColor(value));
    nanoemMutableModelMaterialSetDiffuseOpacity(material, nanoemModelMaterialGetDiffuseOpacity(value));
    texture = nanoemMutableModelTextureCreate(u.m, status);
    nanoemMutableModelTextureCopy(texture, nanoemModelMaterialGetDiffuseTextureObject(value), status);
    nanoemMutableModelMaterialSetDiffuseTextureObject(material, texture, status);
    nanoemMutableModelTextureDestroy(texture);
    nanoemMutableModelMaterialSetEdgeColor(material, nanoemModelMaterialGetEdgeColor(value));
    nanoemMutableModelMaterialSetEdgeEnabled(material, nanoemModelMaterialIsEdgeEnabled(value));
    nanoemMutableModelMaterialSetEdgeOpacity(material, nanoemModelMaterialGetEdgeOpacity(value));
    nanoemMutableModelMaterialSetEdgeSize(material, nanoemModelMaterialGetEdgeSize(value));
    nanoemMutableModelMaterialSetLineDrawEnabled(material, nanoemModelMaterialIsLineDrawEnabled(value));
    nanoemMutableModelMaterialSetName(material, nanoemModelMaterialGetName(value, NANOEM_LANGUAGE_TYPE_JAPANESE), NANOEM_LANGUAGE_TYPE_JAPANESE, status);
    nanoemMutableModelMaterialSetName(material, nanoemModelMaterialGetName(value, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH, status);
    nanoemMutableModelMaterialSetNumVertexIndices(material, nanoemModelMaterialGetNumVertexIndices(value));
    nanoemMutableModelMaterialSetPointDrawEnabled(material, nanoemModelMaterialIsPointDrawEnabled(value));
    nanoemMutableModelMaterialSetShadowMapEnabled(material, nanoemModelMaterialIsShadowMapEnabled(value));
    nanoemMutableModelMaterialSetSpecularColor(material, nanoemModelMaterialGetSpecularColor(value));
    nanoemMutableModelMaterialSetSpecularPower(material, nanoemModelMaterialGetSpecularPower(value));
    texture = nanoemMutableModelTextureCreate(u.m, status);
    nanoemMutableModelTextureCopy(texture, nanoemModelMaterialGetSphereMapTextureObject(value), status);
    nanoemMutableModelMaterialSetSphereMapTextureObject(material, texture, status);
    nanoemMutableModelTextureDestroy(texture);
    nanoemMutableModelMaterialSetSphereMapTextureType(material, nanoemModelMaterialGetSphereMapTextureType(value));
    nanoemMutableModelMaterialSetToonShared(material, nanoemModelMaterialIsToonShared(value));
    nanoemMutableModelMaterialSetToonTextureIndex(material, nanoemModelMaterialGetToonTextureIndex(value));
    texture = nanoemMutableModelTextureCreate(u.m, status);
    nanoemMutableModelTextureCopy(texture, nanoemModelMaterialGetToonTextureObject(value), status);
    nanoemMutableModelMaterialSetToonTextureObject(material, texture, status);
    nanoemMutableModelTextureDestroy(texture);
    nanoemMutableModelMaterialSetVertexColorEnabled(material, nanoemModelMaterialIsVertexColorEnabled(value));
}

nanoem_model_material_t *APIENTRY
nanoemMutableModelMaterialGetOriginObject(nanoem_mutable_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->origin : NULL;
}

void APIENTRY
nanoemMutableModelMaterialSaveToBuffer(nanoem_mutable_model_material_t *material, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    const nanoem_model_material_t *origin;
    const nanoem_model_texture_t *diffuse_texture;
    nanoem_u8_t name_buffer[PMD_MATERIAL_DIFFUSE_TEXTURE_NAME_LENGTH + 1];
    nanoem_unicode_string_factory_t *factory;
    nanoem_codec_type_t codec;
    nanoem_rsize_t size;
    if (nanoem_is_not_null(material) && nanoem_is_not_null(buffer)) {
        origin = material->origin;
        parent_model = nanoemModelMaterialGetParentModel(origin);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
        }
        else if (nanoemModelIsPMX(parent_model)) {
            factory = parent_model->factory;
            codec = nanoemModelGetCodecType(parent_model);
            nanoemMutableBufferWriteUnicodeString(buffer, origin->name_ja, factory, codec, status);
            nanoemMutableBufferWriteUnicodeString(buffer, origin->name_en, factory, codec, status);
            nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->diffuse_color, status);
            nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->diffuse_opacity, status);
            nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->specular_color, status);
            nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->specular_power, status);
            nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->ambient_color, status);
            nanoemMutableBufferWriteByte(buffer, origin->u.value, status);
            nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->edge_color, status);
            nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->edge_opacity, status);
            nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->edge_size, status);
            size = parent_model->info.texture_index_size;
            nanoemMutableBufferWriteInteger(buffer, origin->diffuse_texture_index, size, status);
            nanoemMutableBufferWriteInteger(buffer, origin->sphere_map_texture_index, size, status);
            nanoemMutableBufferWriteByte(buffer, origin->sphere_map_texture_type, status);
            nanoemMutableBufferWriteByte(buffer, origin->is_toon_shared, status);
            nanoemMutableBufferWriteInteger(buffer, origin->toon_texture_index, origin->is_toon_shared ? 1 : size, status);
            nanoemMutableBufferWriteUnicodeString(buffer, origin->clob, factory, codec, status);
            nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) origin->num_vertex_indices, status);
        }
        else {
            factory = parent_model->factory;
            nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->diffuse_color, status);
            nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->diffuse_opacity, status);
            nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->specular_power, status);
            nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->specular_color, status);
            nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->ambient_color, status);
            nanoemMutableBufferWriteByte(buffer, origin->toon_texture_index, status);
            nanoemMutableBufferWriteByte(buffer, origin->u.flags.is_edge_enabled, status);
            nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) origin->num_vertex_indices, status);
            diffuse_texture = origin->diffuse_texture;
            nanoemMutableBufferWriteCp932String(buffer, diffuse_texture ? diffuse_texture->path : NULL, factory, name_buffer, sizeof(name_buffer), status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMaterialDestroy(nanoem_mutable_model_material_t *material)
{
    if (nanoem_is_not_null(material)) {
        if (nanoemMutableBaseModelObjectCanDelete(&material->base)) {
            nanoemModelMaterialDestroy(material->origin);
        }
        nanoem_free(material);
    }
}

nanoem_mutable_model_bone_t *APIENTRY
nanoemMutableModelBoneCreate(nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_bone_t *bone = nanoemModelBoneCreate(model, status);
    return nanoemMutableModelBoneCreateInternal(bone, nanoem_false, nanoem_false, status);
}

nanoem_mutable_model_bone_t *APIENTRY
nanoemMutableModelBoneCreateAsReference(nanoem_model_bone_t *bone, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    nanoem_mutable_model_bone_t *mutable_bone = NULL;
    nanoem_bool_t is_in_model;
    if (nanoem_is_not_null(bone)) {
        origin_model = nanoemModelBoneGetParentModel(bone);
        if (nanoem_is_not_null(origin_model)) {
            is_in_model = nanoemObjectArrayContainsIndexedObject((const void *const *) origin_model->bones, bone, origin_model->num_bones, bone->base.index);
            mutable_bone = nanoemMutableModelBoneCreateInternal(bone, nanoem_true, is_in_model, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return mutable_bone;
}

void APIENTRY
nanoemMutableModelBoneSetName(nanoem_mutable_model_bone_t *bone, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(bone)) {
        parent_model = nanoemModelBoneGetParentModel(bone->origin);
        if (nanoem_is_not_null(parent_model)) {
            factory = parent_model->factory;
            switch (language) {
            case NANOEM_LANGUAGE_TYPE_ENGLISH:
                nanoemUnicodeStringFactoryAssignString(factory, &bone->origin->name_en, value, nanoemModelGetCodecType(parent_model), status);
                break;
            case NANOEM_LANGUAGE_TYPE_JAPANESE:
                nanoemUnicodeStringFactoryAssignString(factory, &bone->origin->name_ja, value, nanoemModelGetCodecType(parent_model), status);
                break;
            case NANOEM_LANGUAGE_TYPE_MAX_ENUM:
            case NANOEM_LANGUAGE_TYPE_UNKNOWN:
            default:
                break;
            }
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelBoneSetParentBoneObject(nanoem_mutable_model_bone_t *bone, const nanoem_model_bone_t *value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->parent_bone_index = nanoemMutableModelBoneResolveBoneObject(bone, value);
    }
}

void APIENTRY
nanoemMutableModelBoneSetInherentParentBoneObject(nanoem_mutable_model_bone_t *bone, const nanoem_model_bone_t *value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->parent_inherent_bone_index = nanoemMutableModelBoneResolveBoneObject(bone, value);
    }
}

void APIENTRY
nanoemMutableModelBoneSetEffectorBoneObject(nanoem_mutable_model_bone_t *bone, const nanoem_model_bone_t *value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->effector_bone_index = nanoemMutableModelBoneResolveBoneObject(bone, value);
    }
}

void APIENTRY
nanoemMutableModelBoneSetTargetBoneObject(nanoem_mutable_model_bone_t *bone, const nanoem_model_bone_t *value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->target_bone_index = nanoemMutableModelBoneResolveBoneObject(bone, value);
        bone->origin->u.flags.has_destination_bone_index = 1;
    }
}

void APIENTRY
nanoemMutableModelBoneSetConstraintObject(nanoem_mutable_model_bone_t *bone, nanoem_mutable_model_constraint_t *value)
{
    if (nanoem_is_not_null(bone) && nanoem_is_not_null(value)) {
        bone->origin->constraint = value->origin;
        bone->origin->u.flags.has_constraint = 1;
        value->base.is_in_model = nanoem_true;
    }
}

void APIENTRY
nanoemMutableModelBoneRemoveConstraintObject(nanoem_mutable_model_bone_t *bone, nanoem_mutable_model_constraint_t *value)
{
    if (nanoem_is_not_null(bone) && nanoem_is_not_null(value) && bone->origin->constraint == value->origin) {
        bone->origin->constraint = NULL;
        bone->origin->u.flags.has_constraint = 0;
        value->base.is_in_model = nanoem_false;
    }
}

void APIENTRY
nanoemMutableModelBoneSetOrigin(nanoem_mutable_model_bone_t *bone, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(bone) && nanoem_is_not_null(value) && bone->origin->origin.values != value) {
        nanoem_crt_memcpy(bone->origin->origin.values, value, sizeof(bone->origin->origin));
    }
}

void APIENTRY
nanoemMutableModelBoneSetDestinationOrigin(nanoem_mutable_model_bone_t *bone, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(bone) && nanoem_is_not_null(value) && bone->origin->destination_origin.values != value) {
        nanoem_crt_memcpy(bone->origin->destination_origin.values, value, sizeof(bone->origin->destination_origin));
        bone->origin->u.flags.has_destination_bone_index = 0;
    }
}

void APIENTRY
nanoemMutableModelBoneSetFixedAxis(nanoem_mutable_model_bone_t *bone, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(bone) && nanoem_is_not_null(value) && bone->origin->fixed_axis.values != value) {
        nanoem_crt_memcpy(bone->origin->fixed_axis.values, value, sizeof(bone->origin->fixed_axis));
    }
}

void APIENTRY
nanoemMutableModelBoneSetLocalXAxis(nanoem_mutable_model_bone_t *bone, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(bone) && nanoem_is_not_null(value) && bone->origin->local_x_axis.values != value) {
        nanoem_crt_memcpy(bone->origin->local_x_axis.values, value, sizeof(bone->origin->local_x_axis));
    }
}

void APIENTRY
nanoemMutableModelBoneSetLocalZAxis(nanoem_mutable_model_bone_t *bone, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(bone) && nanoem_is_not_null(value) && bone->origin->local_z_axis.values != value) {
        nanoem_crt_memcpy(bone->origin->local_z_axis.values, value, sizeof(bone->origin->local_z_axis));
    }
}

void APIENTRY
nanoemMutableModelBoneSetInherentCoefficient(nanoem_mutable_model_bone_t *bone, nanoem_f32_t value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->inherent_coefficient = value;
    }
}

void APIENTRY
nanoemMutableModelBoneSetStageIndex(nanoem_mutable_model_bone_t *bone, int value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->stage_index = value;
    }
}

void APIENTRY
nanoemMutableModelBoneSetOffsetRelative(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->u.flags.has_destination_bone_index = value ? 0 : 1;
    }
}

void APIENTRY
nanoemMutableModelBoneSetRotateable(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->u.flags.is_rotateable = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelBoneSetMovable(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->u.flags.is_movable = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelBoneSetVisible(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->u.flags.is_visible = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelBoneSetUserHandleable(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->u.flags.is_user_handleable = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelBoneSetConstraintEnabled(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->u.flags.has_constraint = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelBoneSetLocalInherentEnabled(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->u.flags.has_local_inherent = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelBoneSetInherentTranslationEnabled(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->u.flags.has_inherent_translation = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelBoneSetInherentOrientationEnabled(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->u.flags.has_inherent_orientation = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelBoneSetFixedAxisEnabled(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->u.flags.has_fixed_axis = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelBoneSetLocalAxesEnabled(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->u.flags.has_local_axes = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelBoneSetAffectedByPhysicsSimulation(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->u.flags.is_affected_by_physics_simulation = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelBoneEnableExternalParentBone(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value)
{
    if (nanoem_is_not_null(bone)) {
        bone->origin->u.flags.has_external_parent_bone = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelBoneCopy(nanoem_mutable_model_bone_t *bone, const nanoem_model_bone_t *value, nanoem_status_t *status)
{
    union nanoem_immutable_model_to_mutable_model_cast_t {
        const nanoem_model_t *p;
        nanoem_model_t *m;
    } u;
    nanoem_mutable_model_constraint_t *constraint;
    if (nanoemModelBoneHasInherentTranslation(value)) {
        nanoemMutableModelBoneSetInherentTranslationEnabled(bone, nanoem_true);
        nanoemMutableModelBoneSetInherentCoefficient(bone, nanoemModelBoneGetInherentCoefficient(value));
        nanoemMutableModelBoneSetInherentParentBoneObject(bone, nanoemModelBoneGetInherentParentBoneObject(value));
    }
    else if (nanoemModelBoneHasInherentOrientation(value)) {
        nanoemMutableModelBoneSetInherentOrientationEnabled(bone, nanoem_true);
        nanoemMutableModelBoneSetInherentCoefficient(bone, nanoemModelBoneGetInherentCoefficient(value));
        nanoemMutableModelBoneSetInherentParentBoneObject(bone, nanoemModelBoneGetInherentParentBoneObject(value));
    }
    nanoemMutableModelBoneSetAffectedByPhysicsSimulation(bone, nanoemModelBoneIsAffectedByPhysicsSimulation(value));
    if (value->constraint) {
        u.p = nanoemModelObjectGetParentModel(nanoemModelBoneGetModelObject(nanoemMutableModelBoneGetOriginObject(bone)));
        constraint = nanoemMutableModelConstraintCreate(u.m, status);
        nanoemMutableModelConstraintCopy(constraint, value->constraint, status);
        nanoemMutableModelBoneSetConstraintObject(bone, constraint);
        nanoemMutableModelConstraintDestroy(constraint);
    }
    nanoemMutableModelBoneSetDestinationOrigin(bone, nanoemModelBoneGetDestinationOrigin(value));
    nanoemMutableModelBoneSetEffectorBoneObject(bone, nanoemModelBoneGetEffectorBoneObject(value));
    nanoemMutableModelBoneSetFixedAxis(bone, nanoemModelBoneGetFixedAxis(value));
    nanoemMutableModelBoneSetLocalXAxis(bone, nanoemModelBoneGetLocalXAxis(value));
    nanoemMutableModelBoneSetLocalZAxis(bone, nanoemModelBoneGetLocalZAxis(value));
    nanoemMutableModelBoneSetMovable(bone, nanoemModelBoneIsMovable(value));
    nanoemMutableModelBoneSetName(bone, nanoemModelBoneGetName(value, NANOEM_LANGUAGE_TYPE_JAPANESE), NANOEM_LANGUAGE_TYPE_JAPANESE, status);
    nanoemMutableModelBoneSetName(bone, nanoemModelBoneGetName(value, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH, status);
    nanoemMutableModelBoneSetOrigin(bone, nanoemModelBoneGetOrigin(value));
    nanoemMutableModelBoneSetParentBoneObject(bone, nanoemModelBoneGetParentBoneObject(value));
    nanoemMutableModelBoneSetRotateable(bone, nanoemModelBoneIsRotateable(value));
    nanoemMutableModelBoneSetStageIndex(bone, nanoemModelBoneGetStageIndex(value));
    nanoemMutableModelBoneSetTargetBoneObject(bone, nanoemModelBoneGetTargetBoneObject(value));
    nanoemMutableModelBoneSetUserHandleable(bone, nanoemModelBoneIsUserHandleable(value));
    nanoemMutableModelBoneSetVisible(bone, nanoemModelBoneIsVisible(value));
}

nanoem_model_bone_t *APIENTRY
nanoemMutableModelBoneGetOriginObject(nanoem_mutable_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->origin : NULL;
}

void APIENTRY
nanoemMutableModelBoneSaveToBuffer(nanoem_mutable_model_bone_t *bone, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    const nanoem_model_bone_t *origin;
    nanoem_u8_t name_buffer[PMD_BONE_NAME_LENGTH + 1];
    nanoem_mutable_model_constraint_t mutable_constraint;
    nanoem_unicode_string_factory_t *factory;
    nanoem_codec_type_t codec;
    nanoem_rsize_t size;
    if (nanoem_is_not_null(bone) && nanoem_is_not_null(buffer)) {
        origin = bone->origin;
        parent_model = nanoemModelBoneGetParentModel(origin);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
        }
        else if (nanoemModelIsPMX(parent_model)) {
            factory = parent_model->factory;
            codec = nanoemModelGetCodecType(parent_model);
            nanoemMutableBufferWriteUnicodeString(buffer, origin->name_ja, factory, codec, status);
            nanoemMutableBufferWriteUnicodeString(buffer, origin->name_en, factory, codec, status);
            nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->origin, status);
            size = parent_model->info.bone_index_size;
            nanoemMutableBufferWriteInteger(buffer, origin->parent_bone_index, size, status);
            nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->stage_index, status);
            nanoemMutableBufferWriteInt16LittleEndianUnsigned(buffer, origin->u.value, status);
            if (origin->u.flags.has_destination_bone_index) {
                nanoemMutableBufferWriteInteger(buffer, origin->target_bone_index, size, status);
            }
            else {
                nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->destination_origin, status);
            }
            if (origin->u.flags.has_inherent_translation || origin->u.flags.has_inherent_orientation) {
                nanoemMutableBufferWriteInteger(buffer, origin->parent_inherent_bone_index, size, status);
                nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->inherent_coefficient, status);
            }
            if (origin->u.flags.has_fixed_axis) {
                nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->fixed_axis, status);
            }
            if (origin->u.flags.has_local_axes) {
                nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->local_x_axis, status);
                nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->local_z_axis, status);
            }
            if (origin->u.flags.has_external_parent_bone) {
                nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->global_bone_index, status);
            }
            if (origin->u.flags.has_constraint) {
                nanoemMutableModelConstraintInitialize(&mutable_constraint, origin->constraint, nanoem_true, nanoem_true);
                nanoemMutableModelConstraintSaveToBuffer(&mutable_constraint, buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    return;
                }
            }
        }
        else {
            factory = parent_model->factory;
            nanoemMutableBufferWriteCp932String(buffer, origin->name_ja, factory, name_buffer, sizeof(name_buffer), status);
            nanoemMutableBufferWriteInt16LittleEndian(buffer, origin->parent_bone_index, status);
            nanoemMutableBufferWriteInt16LittleEndian(buffer, origin->target_bone_index, status);
            nanoemMutableBufferWriteByte(buffer, origin->type, status);
            nanoemMutableBufferWriteInt16LittleEndian(buffer, origin->effector_bone_index, status);
            nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->origin, status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelBoneDestroy(nanoem_mutable_model_bone_t *bone)
{
    if (nanoem_is_not_null(bone)) {
        if (nanoemMutableBaseModelObjectCanDelete(&bone->base)) {
            nanoemModelBoneDestroy(bone->origin);
        }
        nanoem_free(bone);
    }
}

nanoem_mutable_model_constraint_t *APIENTRY
nanoemMutableModelConstraintCreate(nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_constraint_t *constraint = nanoemModelConstraintCreate(model, status);
    return nanoemMutableModelConstraintCreateInternal(constraint, nanoem_false, nanoem_false, status);
}

nanoem_mutable_model_constraint_t *APIENTRY
nanoemMutableModelConstraintCreateAsReference(nanoem_model_constraint_t *constraint, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    const nanoem_model_bone_t *bone;
    nanoem_model_bone_t *const *bones;
    nanoem_mutable_model_constraint_t *mutable_constraint = NULL;
    nanoem_rsize_t num_bones, i;
    nanoem_bool_t is_in_model;
    if (nanoem_is_not_null(constraint)) {
        origin_model = nanoemModelConstraintGetParentModel(constraint);
        if (nanoem_is_null(origin_model)) {
            nanoem_status_ptr_assign_null_object(status);
            return mutable_constraint;
        }
        is_in_model = nanoemObjectArrayContainsIndexedObject((const void *const *) origin_model->constraints, constraint, origin_model->num_constraints, constraint->base.index);
        if (!is_in_model) {
            bones = origin_model->bones;
            num_bones = origin_model->num_bones;
            for (i = 0; i < num_bones; i++) {
                bone = bones[i];
                if (bone->constraint == constraint) {
                    is_in_model = nanoem_true;
                    break;
                }
            }
        }
        mutable_constraint = nanoemMutableModelConstraintCreateInternal(constraint, nanoem_true, is_in_model, status);
    }
    return mutable_constraint;
}

void APIENTRY
nanoemMutableModelConstraintSetEffectorBoneObject(nanoem_mutable_model_constraint_t *constraint, const nanoem_model_bone_t *value)
{
    if (nanoem_is_not_null(constraint)) {
        constraint->origin->effector_bone_index = nanoemMutableModelConstraintResolveBoneObject(constraint, value);
    }
}

void APIENTRY
nanoemMutableModelConstraintSetTargetBoneObject(nanoem_mutable_model_constraint_t *constraint, const nanoem_model_bone_t *value)
{
    if (nanoem_is_not_null(constraint)) {
        constraint->origin->target_bone_index = nanoemMutableModelConstraintResolveBoneObject(constraint, value);
    }
}

void APIENTRY
nanoemMutableModelConstraintSetAngleLimit(nanoem_mutable_model_constraint_t *constraint, nanoem_f32_t value)
{
    if (nanoem_is_not_null(constraint)) {
        constraint->origin->angle_limit = value;
    }
}

void APIENTRY
nanoemMutableModelConstraintSetNumIterations(nanoem_mutable_model_constraint_t *constraint, int value)
{
    if (nanoem_is_not_null(constraint)) {
        constraint->origin->num_iterations = value;
    }
}

void APIENTRY
nanoemMutableModelConstraintCopy(nanoem_mutable_model_constraint_t *constraint, const nanoem_model_constraint_t *value, nanoem_status_t *status)
{
    const nanoem_model_constraint_t *origin = nanoemMutableModelConstraintGetOriginObject(constraint);
    nanoem_model_constraint_joint_t *const *joints;
    nanoem_mutable_model_constraint_joint_t *joint;
    nanoem_rsize_t i, num_joints;
    nanoemMutableModelConstraintSetAngleLimit(constraint, nanoemModelConstraintGetAngleLimit(value));
    nanoemMutableModelConstraintSetEffectorBoneObject(constraint, nanoemModelConstraintGetEffectorBoneObject(value));
    nanoemMutableModelConstraintSetNumIterations(constraint, nanoemModelConstraintGetNumIterations(value));
    nanoemMutableModelConstraintSetTargetBoneObject(constraint, nanoemModelConstraintGetTargetBoneObject(value));
    joints = nanoemModelConstraintGetAllJointObjects(origin, &num_joints);
    for (i = 0; i < num_joints; i++) {
        joint = nanoemMutableModelConstraintJointCreateAsReference(joints[i], status);
        nanoemMutableModelConstraintRemoveJointObject(constraint, joint, status);
        nanoemMutableModelConstraintJointDestroy(joint);
    }
    joints = nanoemModelConstraintGetAllJointObjects(value, &num_joints);
    for (i = 0; i < num_joints; i++) {
        joint = nanoemMutableModelConstraintJointCreate(constraint, status);
        nanoemMutableModelConstraintJointCopy(joint, joints[i]);
        nanoemMutableModelConstraintInsertJointObject(constraint, joint, -1, status);
        nanoemMutableModelConstraintJointDestroy(joint);
    }
}

void APIENTRY
nanoemMutableModelConstraintInsertJointObject(nanoem_mutable_model_constraint_t *constraint, nanoem_mutable_model_constraint_joint_t *value, int index, nanoem_status_t *status)
{
    nanoem_model_constraint_t *origin_constraint;
    if (nanoem_is_not_null(constraint) && nanoem_is_not_null(value)) {
        origin_constraint = constraint->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_constraint->joints, (nanoem_model_object_t *) value->origin, index, NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_ALREADY_EXISTS, &origin_constraint->num_joints, &constraint->num_allocated_joints, status);
        if (!nanoem_status_ptr_has_error(status)) {
            value->origin->base.parent.constraint = origin_constraint;
            value->base.is_in_model = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelConstraintRemoveJointObject(nanoem_mutable_model_constraint_t *constraint, nanoem_mutable_model_constraint_joint_t *value, nanoem_status_t *status)
{
    nanoem_model_constraint_t *origin_constraint;
    if (nanoem_is_not_null(constraint) && nanoem_is_not_null(value)) {
        origin_constraint = constraint->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_constraint->joints, (nanoem_model_object_t *) value->origin, &origin_constraint->num_joints, NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_JOINT_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            value->origin->base.parent.constraint = NULL;
            value->base.is_in_model = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

nanoem_model_constraint_t *APIENTRY
nanoemMutableModelConstraintGetOriginObject(nanoem_mutable_model_constraint_t *constraint)
{
    return nanoem_is_not_null(constraint) ? constraint->origin : NULL;
}

void APIENTRY
nanoemMutableModelConstraintSaveToBuffer(nanoem_mutable_model_constraint_t *constraint, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_mutable_model_constraint_joint_t mutable_joint;
    nanoem_model_constraint_joint_t *const *joints;
    nanoem_model_constraint_t *origin;
    nanoem_rsize_t num_joints, i;
    if (nanoem_is_not_null(constraint) && nanoem_is_not_null(buffer)) {
        origin = constraint->origin;
        parent_model = nanoemModelConstraintGetParentModel(origin);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
            return;
        }
        joints = nanoemModelConstraintGetAllJointObjects(origin, &num_joints);
        if (nanoemModelIsPMX(parent_model)) {
            nanoemMutableBufferWriteInteger(buffer, origin->effector_bone_index, parent_model->info.bone_index_size, status);
            nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->num_iterations, status);
            nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->angle_limit, status);
            nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_joints, status);
        }
        else {
            nanoemMutableBufferWriteInt16LittleEndian(buffer, origin->target_bone_index, status);
            nanoemMutableBufferWriteInt16LittleEndian(buffer, origin->effector_bone_index, status);
            nanoemMutableBufferWriteByte(buffer, (nanoem_u8_t) num_joints, status);
            nanoemMutableBufferWriteInt16LittleEndianUnsigned(buffer, origin->num_iterations, status);
            nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->angle_limit, status);
        }
        for (i = 0; i < num_joints; i++) {
            nanoemMutableModelConstraintJointInitialize(&mutable_joint, joints[i], nanoem_true, nanoem_true);
            nanoemMutableModelConstraintJointSaveToBuffer(&mutable_joint, buffer, status);
            if (nanoem_status_ptr_has_error(status)) {
                return;
            }
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

nanoem_mutable_model_constraint_joint_t *APIENTRY
nanoemMutableModelConstraintJointCreate(nanoem_mutable_model_constraint_t *constraint, nanoem_status_t *status)
{
    nanoem_model_constraint_joint_t *joint = nanoemModelConstraintJointCreate(nanoemMutableModelConstraintGetOriginObject(constraint), status);
    return nanoemMutableModelConstraintJointCreateInternal(joint, nanoem_false, nanoem_false, status);
}

nanoem_mutable_model_constraint_joint_t *APIENTRY
nanoemMutableModelConstraintJointCreateAsReference(nanoem_model_constraint_joint_t *joint, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    const nanoem_model_constraint_t *parent_constraint;
    nanoem_mutable_model_constraint_joint_t *mutable_joint = NULL;
    nanoem_bool_t is_in_model;
    if (nanoem_is_not_null(joint)) {
        parent_constraint = nanoemModelConstraintJointGetParentConstraintObject(joint);
        origin_model = nanoemModelConstraintGetParentModel(parent_constraint);
        if (nanoem_is_not_null(origin_model)) {
            is_in_model = nanoemObjectArrayContainsIndexedObject((const void *const *) parent_constraint->joints, joint, parent_constraint->num_joints, joint->base.index);
            mutable_joint = nanoemMutableModelConstraintJointCreateInternal(joint, nanoem_true, is_in_model, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return mutable_joint;
}

void APIENTRY
nanoemMutableModelConstraintJointSetBoneObject(nanoem_mutable_model_constraint_joint_t *joint, const nanoem_model_bone_t *value)
{
    const nanoem_model_constraint_t *parent_constraint;
    const nanoem_model_t *parent_model;
    if (nanoem_is_not_null(joint) && nanoem_is_not_null(value)) {
        parent_constraint = nanoemModelConstraintJointGetParentConstraintObject(joint->origin);
        parent_model = nanoemModelConstraintGetParentModel(parent_constraint);
        joint->origin->bone_index = nanoemModelResolveBoneObject(parent_model, value);
    }
}

void APIENTRY
nanoemMutableModelConstraintJointSetUpperLimit(nanoem_mutable_model_constraint_joint_t *joint, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(joint) && nanoem_is_not_null(value) && joint->origin->upper_limit.values != value) {
        nanoem_crt_memcpy(joint->origin->upper_limit.values, value, sizeof(joint->origin->upper_limit));
    }
}

void APIENTRY
nanoemMutableModelConstraintJointSetLowerLimit(nanoem_mutable_model_constraint_joint_t *joint, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(joint) && nanoem_is_not_null(value) && joint->origin->lower_limit.values != value) {
        nanoem_crt_memcpy(joint->origin->lower_limit.values, value, sizeof(joint->origin->lower_limit));
    }
}

void APIENTRY
nanoemMutableModelConstraintJointSetAngleLimitEnabled(nanoem_mutable_model_constraint_joint_t *joint, nanoem_bool_t value)
{
    if (nanoem_is_not_null(joint)) {
        joint->origin->has_angle_limit = value;
    }
}

void APIENTRY
nanoemMutableModelConstraintJointCopy(nanoem_mutable_model_constraint_joint_t *joint, const nanoem_model_constraint_joint_t *value)
{
    nanoemMutableModelConstraintJointSetBoneObject(joint, nanoemModelConstraintJointGetBoneObject(value));
    nanoemMutableModelConstraintJointSetLowerLimit(joint, nanoemModelConstraintJointGetLowerLimit(value));
    nanoemMutableModelConstraintJointSetUpperLimit(joint, nanoemModelConstraintJointGetUpperLimit(value));
    nanoemMutableModelConstraintJointSetAngleLimitEnabled(joint, nanoemModelConstraintJointHasAngleLimit(value));
}

nanoem_model_constraint_joint_t *APIENTRY
nanoemMutableModelConstraintJointGetOriginObject(nanoem_mutable_model_constraint_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? joint->origin : NULL;
}

void APIENTRY
nanoemMutableModelConstraintJointSaveToBuffer(nanoem_mutable_model_constraint_joint_t *joint, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    const nanoem_model_constraint_joint_t *origin;
    nanoem_bool_t has_angle_limit;
    int bone_index;
    if (nanoem_is_not_null(joint) && nanoem_is_not_null(buffer)) {
        origin = joint->origin;
        parent_model = nanoemModelConstraintGetParentModel(nanoemModelConstraintJointGetParentConstraintObject(origin));
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
        }
        else if (nanoemModelIsPMX(parent_model)) {
            bone_index = nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(nanoemModelConstraintJointGetBoneObject(origin)));
            has_angle_limit = nanoemModelConstraintJointHasAngleLimit(origin);
            nanoemMutableBufferWriteInteger(buffer, bone_index, parent_model->info.bone_index_size, status);
            nanoemMutableBufferWriteByte(buffer, has_angle_limit, status);
            if (has_angle_limit) {
                nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->lower_limit, status);
                nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->upper_limit, status);
            }
        }
        else {
            nanoemMutableBufferWriteInt16LittleEndian(buffer, origin->bone_index, status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelConstraintJointDestroy(nanoem_mutable_model_constraint_joint_t *joint)
{
    if (nanoem_is_not_null(joint)) {
        if (nanoemMutableBaseModelObjectCanDelete(&joint->base)) {
            nanoemModelConstraintJointDestroy(joint->origin);
        }
        nanoem_free(joint);
    }
}

void APIENTRY
nanoemMutableModelConstraintDestroy(nanoem_mutable_model_constraint_t *constraint)
{
    if (nanoem_is_not_null(constraint)) {
        if (nanoemMutableBaseModelObjectCanDelete(&constraint->base)) {
            nanoemModelConstraintDestroy(constraint->origin);
        }
        nanoem_free(constraint);
    }
}

nanoem_mutable_model_texture_t *APIENTRY
nanoemMutableModelTextureCreate(nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_texture_t *texture = nanoemModelTextureCreate(model, status);
    return nanoemMutableModelTextureCreateInternal(texture, nanoem_false, nanoem_false, status);
}

nanoem_mutable_model_texture_t *APIENTRY
nanoemMutableModelTextureCreateAsReference(nanoem_model_texture_t *texture, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    nanoem_mutable_model_texture_t *mutable_texture = NULL;
    nanoem_bool_t is_in_model;
    if (nanoem_is_not_null(texture)) {
        origin_model = nanoemModelTextureGetParentModel(texture);
        if (nanoem_is_not_null(origin_model)) {
            is_in_model = nanoemObjectArrayContainsIndexedObject((const void *const *) origin_model->textures, texture, origin_model->num_textures, texture->base.index);
            mutable_texture = nanoemMutableModelTextureCreateInternal(texture, nanoem_true, is_in_model, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return mutable_texture;
}

void APIENTRY
nanoemMutableModelTextureSetPath(nanoem_mutable_model_texture_t *texture, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(texture)) {
        parent_model = nanoemModelTextureGetParentModel(texture->origin);
        if (nanoem_is_not_null(parent_model)) {
            factory = parent_model->factory;
            nanoemUnicodeStringFactoryAssignString(factory, &texture->origin->path, value, nanoemModelGetCodecType(parent_model), status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelTextureCopy(nanoem_mutable_model_texture_t *texture, const nanoem_model_texture_t *value, nanoem_status_t *status)
{
    nanoemMutableModelTextureSetPath(texture, nanoemModelTextureGetPath(value), status);
}

nanoem_model_texture_t *APIENTRY
nanoemMutableModelTextureGetOriginObject(nanoem_mutable_model_texture_t *texture)
{
    return nanoem_is_not_null(texture) ? texture->origin : NULL;
}

void APIENTRY
nanoemMutableModelTextureSaveToBuffer(nanoem_mutable_model_texture_t *texture, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    const nanoem_model_texture_t *origin;
    nanoem_codec_type_t codec;
    if (nanoem_is_not_null(texture) && nanoem_is_not_null(buffer)) {
        origin = texture->origin;
        parent_model = nanoemModelTextureGetParentModel(origin);
        if (nanoem_is_not_null(parent_model)) {
            codec = nanoemModelGetCodecType(parent_model);
            nanoemMutableBufferWriteUnicodeString(buffer, origin->path, parent_model->factory, codec, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelTextureDestroy(nanoem_mutable_model_texture_t *texture)
{
    if (nanoem_is_not_null(texture)) {
        if (nanoemMutableBaseModelObjectCanDelete(&texture->base)) {
            nanoemModelTextureDestroy(texture->origin);
        }
        nanoem_free(texture);
    }
}

static void
nanoemMutableModelMorphBoneInitialize(nanoem_mutable_model_morph_bone_t *bone, nanoem_model_morph_bone_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model)
{
    bone->origin = origin;
    bone->base.is_reference = is_reference;
    bone->base.is_in_morph = is_in_model;
}

static nanoem_mutable_model_morph_bone_t *
nanoemMutableModelMorphBoneCreateInternal(nanoem_model_morph_bone_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model, nanoem_status_t *status)
{
    nanoem_mutable_model_morph_bone_t *bone = NULL;
    if (nanoem_is_not_null(origin)) {
        bone = (nanoem_mutable_model_morph_bone_t *) nanoem_calloc(1, sizeof(*bone), status);
        if (nanoem_is_not_null(bone)) {
            nanoemMutableModelMorphBoneInitialize(bone, origin, is_reference, is_in_model);
        }
    }
    return bone;
}

static void
nanoemMutableModelMorphFlipInitialize(nanoem_mutable_model_morph_flip_t *flip, nanoem_model_morph_flip_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model)
{
    flip->origin = origin;
    flip->base.is_reference = is_reference;
    flip->base.is_in_morph = is_in_model;
}

static nanoem_mutable_model_morph_flip_t *
nanoemMutableModelMorphFlipCreateInternal(nanoem_model_morph_flip_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model, nanoem_status_t *status)
{
    nanoem_mutable_model_morph_flip_t *flip = NULL;
    if (nanoem_is_not_null(origin)) {
        flip = (nanoem_mutable_model_morph_flip_t *) nanoem_calloc(1, sizeof(*flip), status);
        if (nanoem_is_not_null(flip)) {
            nanoemMutableModelMorphFlipInitialize(flip, origin, is_reference, is_in_model);
        }
    }
    return flip;
}

static void
nanoemMutableModelMorphGroupInitialize(nanoem_mutable_model_morph_group_t *group, nanoem_model_morph_group_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model)
{
    group->origin = origin;
    group->base.is_reference = is_reference;
    group->base.is_in_morph = is_in_model;
}

static nanoem_mutable_model_morph_group_t *
nanoemMutableModelMorphGroupCreateInternal(nanoem_model_morph_group_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model, nanoem_status_t *status)
{
    nanoem_mutable_model_morph_group_t *group = NULL;
    if (nanoem_is_not_null(origin)) {
        group = (nanoem_mutable_model_morph_group_t *) nanoem_calloc(1, sizeof(*group), status);
        if (nanoem_is_not_null(group)) {
            nanoemMutableModelMorphGroupInitialize(group, origin, is_reference, is_in_model);
        }
    }
    return group;
}

static void
nanoemMutableModelMorphImpulseInitialize(nanoem_mutable_model_morph_impulse_t *impulse, nanoem_model_morph_impulse_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model)
{
    impulse->origin = origin;
    impulse->base.is_reference = is_reference;
    impulse->base.is_in_morph = is_in_model;
}

static nanoem_mutable_model_morph_impulse_t *
nanoemMutableModelMorphImpulseCreateInternal(nanoem_model_morph_impulse_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model, nanoem_status_t *status)
{
    nanoem_mutable_model_morph_impulse_t *impulse = NULL;
    if (nanoem_is_not_null(origin)) {
        impulse = (nanoem_mutable_model_morph_impulse_t *) nanoem_calloc(1, sizeof(*impulse), status);
        if (nanoem_is_not_null(impulse)) {
            nanoemMutableModelMorphImpulseInitialize(impulse, origin, is_reference, is_in_model);
        }
    }
    return impulse;
}

static void
nanoemMutableModelMorphMaterialInitialize(nanoem_mutable_model_morph_material_t *material, nanoem_model_morph_material_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model)
{
    material->origin = origin;
    material->base.is_reference = is_reference;
    material->base.is_in_morph = is_in_model;
}

static nanoem_mutable_model_morph_material_t *
nanoemMutableModelMorphMaterialCreateInternal(nanoem_model_morph_material_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model, nanoem_status_t *status)
{
    nanoem_mutable_model_morph_material_t *material = NULL;
    if (nanoem_is_not_null(origin)) {
        material = (nanoem_mutable_model_morph_material_t *) nanoem_calloc(1, sizeof(*material), status);
        if (nanoem_is_not_null(material)) {
            nanoemMutableModelMorphMaterialInitialize(material, origin, is_reference, is_in_model);
        }
    }
    return material;
}

static void
nanoemMutableModelMorphUVInitialize(nanoem_mutable_model_morph_uv_t *uv, nanoem_model_morph_uv_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model)
{
    uv->origin = origin;
    uv->base.is_reference = is_reference;
    uv->base.is_in_morph = is_in_model;
}

static nanoem_mutable_model_morph_uv_t *
nanoemMutableModelMorphUVCreateInternal(nanoem_model_morph_uv_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model, nanoem_status_t *status)
{
    nanoem_mutable_model_morph_uv_t *uv = NULL;
    if (nanoem_is_not_null(origin)) {
        uv = (nanoem_mutable_model_morph_uv_t *) nanoem_calloc(1, sizeof(*uv), status);
        if (nanoem_is_not_null(uv)) {
            nanoemMutableModelMorphUVInitialize(uv, origin, is_reference, is_in_model);
        }
    }
    return uv;
}

static void
nanoemMutableModelMorphVertexInitialize(nanoem_mutable_model_morph_vertex_t *vertex, nanoem_model_morph_vertex_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model)
{
    vertex->origin = origin;
    vertex->base.is_reference = is_reference;
    vertex->base.is_in_morph = is_in_model;
}

static nanoem_mutable_model_morph_vertex_t *
nanoemMutableModelMorphVertexCreateInternal(nanoem_model_morph_vertex_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_model, nanoem_status_t *status)
{
    nanoem_mutable_model_morph_vertex_t *vertex = NULL;
    if (nanoem_is_not_null(origin)) {
        vertex = (nanoem_mutable_model_morph_vertex_t *) nanoem_calloc(1, sizeof(*vertex), status);
        if (nanoem_is_not_null(vertex)) {
            nanoemMutableModelMorphVertexInitialize(vertex, origin, is_reference, is_in_model);
        }
    }
    return vertex;
}

nanoem_mutable_model_morph_bone_t *APIENTRY
nanoemMutableModelMorphBoneCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status)
{
    nanoem_model_morph_bone_t *bone = nanoemModelMorphBoneCreate(nanoemMutableModelMorphGetOriginObject(morph), status);
    return nanoemMutableModelMorphBoneCreateInternal(bone, nanoem_false, nanoem_false, status);
}

nanoem_mutable_model_morph_bone_t *APIENTRY
nanoemMutableModelMorphBoneCreateAsReference(nanoem_model_morph_bone_t *morph, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    const nanoem_model_morph_t *parent_morph;
    nanoem_mutable_model_morph_bone_t *mutable_morph = NULL;
    nanoem_bool_t is_in_morph;
    if (nanoem_is_not_null(morph)) {
        parent_morph = nanoemModelMorphBoneGetParentMorph(morph);
        origin_model = nanoemModelMorphGetParentModel(parent_morph);
        if (nanoem_is_not_null(origin_model)) {
            is_in_morph = nanoemObjectArrayContainsObject((const void *const *) parent_morph->u.bones, morph, parent_morph->num_objects);
            mutable_morph = nanoemMutableModelMorphBoneCreateInternal(morph, nanoem_true, is_in_morph, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return mutable_morph;
}

void APIENTRY
nanoemMutableModelMorphBoneSetBoneObject(nanoem_mutable_model_morph_bone_t *morph, const nanoem_model_bone_t *value)
{
    const nanoem_model_t *origin_model;
    const nanoem_model_morph_t *parent_morph;
    if (nanoem_is_not_null(morph)) {
        parent_morph = nanoemModelMorphBoneGetParentMorph(morph->origin);
        origin_model = nanoemModelMorphGetParentModel(parent_morph);
        morph->origin->bone_index = nanoemModelResolveBoneObject(origin_model, value);
    }
}

void APIENTRY
nanoemMutableModelMorphBoneSetTranslation(nanoem_mutable_model_morph_bone_t *morph, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value) && morph->origin->translation.values != value) {
        nanoem_crt_memcpy(morph->origin->translation.values, value, sizeof(morph->origin->translation));
    }
}

void APIENTRY
nanoemMutableModelMorphBoneSetOrientation(nanoem_mutable_model_morph_bone_t *morph, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value) && morph->origin->orientation.values != value) {
        nanoem_crt_memcpy(morph->origin->orientation.values, value, sizeof(morph->origin->orientation));
    }
}

void APIENTRY
nanoemMutableModelMorphBoneCopy(nanoem_mutable_model_morph_bone_t *morph, const nanoem_model_morph_bone_t *value)
{
    nanoemMutableModelMorphBoneSetBoneObject(morph, nanoemModelMorphBoneGetBoneObject(value));
    nanoemMutableModelMorphBoneSetOrientation(morph, nanoemModelMorphBoneGetOrientation(value));
    nanoemMutableModelMorphBoneSetTranslation(morph, nanoemModelMorphBoneGetTranslation(value));
}

void APIENTRY
nanoemMutableModelMorphBoneSaveToBuffer(nanoem_mutable_model_morph_bone_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    const nanoem_model_morph_bone_t *origin;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(buffer)) {
        origin = morph->origin;
        parent_model = nanoemModelMorphGetParentModel(origin->base.parent.morph);
        if (nanoem_is_not_null(origin) && nanoem_is_not_null(parent_model)) {
            nanoemMutableBufferWriteInteger(buffer, origin->bone_index, parent_model->info.bone_index_size, status);
            nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->translation, status);
            nanoemMutableBufferWriteFloat32x4LittleEndian(buffer, &origin->orientation, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

nanoem_model_morph_bone_t *APIENTRY
nanoemMutableModelMorphBoneGetOriginObject(nanoem_mutable_model_morph_bone_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->origin : NULL;
}

void APIENTRY
nanoemMutableModelMorphBoneDestroy(nanoem_mutable_model_morph_bone_t *morph)
{
    if (nanoem_is_not_null(morph)) {
        if (nanoemMutableBaseMorphObjectCanDelete(&morph->base)) {
            nanoemModelMorphBoneDestroy(morph->origin);
        }
        nanoem_free(morph);
    }
}

nanoem_mutable_model_morph_flip_t *APIENTRY
nanoemMutableModelMorphFlipCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status)
{
    nanoem_model_morph_flip_t *flip = nanoemModelMorphFlipCreate(nanoemMutableModelMorphGetOriginObject(morph), status);
    return nanoemMutableModelMorphFlipCreateInternal(flip, nanoem_false, nanoem_false, status);
}

nanoem_mutable_model_morph_flip_t *APIENTRY
nanoemMutableModelMorphFlipCreateAsReference(nanoem_model_morph_flip_t *morph, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    const nanoem_model_morph_t *parent_morph;
    nanoem_mutable_model_morph_flip_t *mutable_morph = NULL;
    nanoem_bool_t is_in_morph;
    if (nanoem_is_not_null(morph)) {
        parent_morph = nanoemModelMorphFlipGetParentMorph(morph);
        origin_model = nanoemModelMorphGetParentModel(parent_morph);
        if (nanoem_is_not_null(origin_model)) {
            is_in_morph = nanoemObjectArrayContainsObject((const void *const *) parent_morph->u.flips, morph, parent_morph->num_objects);
            mutable_morph = nanoemMutableModelMorphFlipCreateInternal(morph, nanoem_true, is_in_morph, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return mutable_morph;
}

void APIENTRY
nanoemMutableModelMorphFlipSetMorphObject(nanoem_mutable_model_morph_flip_t *morph, const nanoem_model_morph_t *value)
{
    const nanoem_model_morph_t *parent_morph;
    const nanoem_model_t *parent_model;
    if (nanoem_is_not_null(morph)) {
        parent_morph = nanoemModelMorphFlipGetParentMorph(morph->origin);
        parent_model = nanoemModelMorphGetParentModel(parent_morph);
        morph->origin->morph_index = nanoemModelResolveMorphObject(parent_model, value);
    }
}

void APIENTRY
nanoemMutableModelMorphFlipSetWeight(nanoem_mutable_model_morph_flip_t *morph, nanoem_f32_t value)
{
    if (nanoem_is_not_null(morph)) {
        morph->origin->weight = value;
    }
}

void APIENTRY
nanoemMutableModelMorphFlipCopy(nanoem_mutable_model_morph_flip_t *morph, const nanoem_model_morph_flip_t *value)
{
    nanoemMutableModelMorphFlipSetMorphObject(morph, nanoemModelMorphFlipGetMorphObject(value));
    nanoemMutableModelMorphFlipSetWeight(morph, nanoemModelMorphFlipGetWeight(value));
}

void APIENTRY
nanoemMutableModelMorphFlipSaveToBuffer(nanoem_mutable_model_morph_flip_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_flip_t *origin;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(buffer)) {
        origin = morph->origin;
        parent_model = nanoemModelMorphGetParentModel(origin->base.parent.morph);
        if (nanoem_is_not_null(origin) && nanoem_is_not_null(parent_model)) {
            nanoemMutableBufferWriteInteger(buffer, origin->morph_index, parent_model->info.morph_index_size, status);
            nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->weight, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

nanoem_model_morph_flip_t *APIENTRY
nanoemMutableModelMorphFlipGetOriginObject(nanoem_mutable_model_morph_flip_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->origin : NULL;
}

void APIENTRY
nanoemMutableModelMorphFlipDestroy(nanoem_mutable_model_morph_flip_t *morph)
{
    if (nanoem_is_not_null(morph)) {
        if (nanoemMutableBaseMorphObjectCanDelete(&morph->base)) {
            nanoemModelMorphFlipDestroy(morph->origin);
        }
        nanoem_free(morph);
    }
}

nanoem_mutable_model_morph_group_t *APIENTRY
nanoemMutableModelMorphGroupCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status)
{
    nanoem_model_morph_group_t *group = nanoemModelMorphGroupCreate(nanoemMutableModelMorphGetOriginObject(morph), status);
    return nanoemMutableModelMorphGroupCreateInternal(group, nanoem_false, nanoem_false, status);
}

nanoem_mutable_model_morph_group_t *APIENTRY
nanoemMutableModelMorphGroupCreateAsReference(nanoem_model_morph_group_t *morph, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    const nanoem_model_morph_t *parent_morph;
    nanoem_mutable_model_morph_group_t *mutable_morph = NULL;
    nanoem_bool_t is_in_morph;
    if (nanoem_is_not_null(morph)) {
        parent_morph = nanoemModelMorphGroupGetParentMorph(morph);
        origin_model = nanoemModelMorphGetParentModel(parent_morph);
        if (nanoem_is_not_null(origin_model)) {
            is_in_morph = nanoemObjectArrayContainsObject((const void *const *) parent_morph->u.groups, morph, parent_morph->num_objects);
            mutable_morph = nanoemMutableModelMorphGroupCreateInternal(morph, nanoem_true, is_in_morph, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return mutable_morph;
}

void APIENTRY
nanoemMutableModelMorphGroupSetMorphObject(nanoem_mutable_model_morph_group_t *morph, const nanoem_model_morph_t *value)
{
    const nanoem_model_morph_t *parent_morph;
    const nanoem_model_t *parent_model;
    if (nanoem_is_not_null(morph)) {
        parent_morph = nanoemModelMorphGroupGetParentMorph(morph->origin);
        parent_model = nanoemModelMorphGetParentModel(parent_morph);
        morph->origin->morph_index = nanoemModelResolveMorphObject(parent_model, value);
    }
}

void APIENTRY
nanoemMutableModelMorphGroupSetWeight(nanoem_mutable_model_morph_group_t *morph, nanoem_f32_t value)
{
    if (nanoem_is_not_null(morph)) {
        morph->origin->weight = value;
    }
}

void APIENTRY
nanoemMutableModelMorphGroupCopy(nanoem_mutable_model_morph_group_t *morph, const nanoem_model_morph_group_t *value)
{
    nanoemMutableModelMorphGroupSetMorphObject(morph, nanoemModelMorphGroupGetMorphObject(value));
    nanoemMutableModelMorphGroupSetWeight(morph, nanoemModelMorphGroupGetWeight(value));
}

void APIENTRY
nanoemMutableModelMorphGroupSaveToBuffer(nanoem_mutable_model_morph_group_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_group_t *origin;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(buffer)) {
        origin = morph->origin;
        parent_model = nanoemModelMorphGetParentModel(origin->base.parent.morph);
        if (nanoem_is_not_null(parent_model)) {
            nanoemMutableBufferWriteInteger(buffer, origin->morph_index, parent_model->info.morph_index_size, status);
            nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->weight, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

nanoem_model_morph_group_t *APIENTRY
nanoemMutableModelMorphGroupGetOriginObject(nanoem_mutable_model_morph_group_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->origin : NULL;
}

void APIENTRY
nanoemMutableModelMorphGroupDestroy(nanoem_mutable_model_morph_group_t *morph)
{
    if (nanoem_is_not_null(morph)) {
        if (nanoemMutableBaseMorphObjectCanDelete(&morph->base)) {
            nanoemModelMorphGroupDestroy(morph->origin);
        }
        nanoem_free(morph);
    }
}

nanoem_mutable_model_morph_impulse_t *APIENTRY
nanoemMutableModelMorphImpulseCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status)
{
    nanoem_model_morph_impulse_t *impulse = nanoemModelMorphImpulseCreate(nanoemMutableModelMorphGetOriginObject(morph), status);
    return nanoemMutableModelMorphImpulseCreateInternal(impulse, nanoem_false, nanoem_false, status);
}

nanoem_mutable_model_morph_impulse_t *APIENTRY
nanoemMutableModelMorphImpulseCreateAsReference(nanoem_model_morph_impulse_t *morph, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    const nanoem_model_morph_t *parent_morph;
    nanoem_mutable_model_morph_impulse_t *mutable_morph = NULL;
    nanoem_bool_t is_in_morph;
    if (nanoem_is_not_null(morph)) {
        parent_morph = nanoemModelMorphImpulseGetParentMorph(morph);
        origin_model = nanoemModelMorphGetParentModel(parent_morph);
        if (nanoem_is_not_null(origin_model)) {
            is_in_morph = nanoemObjectArrayContainsObject((const void *const *) parent_morph->u.impulses, morph, parent_morph->num_objects);
            mutable_morph = nanoemMutableModelMorphImpulseCreateInternal(morph, nanoem_true, is_in_morph, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return mutable_morph;
}

void APIENTRY
nanoemMutableModelMorphImpulseSetRigidBodyObject(nanoem_mutable_model_morph_impulse_t *morph, const nanoem_model_rigid_body_t *value)
{
    const nanoem_model_morph_t *parent_morph;
    const nanoem_model_t *parent_model;
    if (nanoem_is_not_null(morph)) {
        parent_morph = nanoemModelMorphImpulseGetParentMorph(morph->origin);
        parent_model = nanoemModelMorphGetParentModel(parent_morph);
        morph->origin->rigid_body_index = nanoemModelResolveRigidBodyObject(parent_model, value);
    }
}

void APIENTRY
nanoemMutableModelMorphImpulseSetTorque(nanoem_mutable_model_morph_impulse_t *morph, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value) && morph->origin->torque.values != value) {
        nanoem_crt_memcpy(morph->origin->torque.values, value, sizeof(morph->origin->torque));
    }
}

void APIENTRY
nanoemMutableModelMorphImpulseSetVelocity(nanoem_mutable_model_morph_impulse_t *morph, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value) && morph->origin->velocity.values != value) {
        nanoem_crt_memcpy(morph->origin->velocity.values, value, sizeof(morph->origin->velocity));
    }
}

void APIENTRY
nanoemMutableModelMorphImpulseSetLocal(nanoem_mutable_model_morph_impulse_t *morph, nanoem_bool_t value)
{
    if (nanoem_is_not_null(morph)) {
        morph->origin->is_local = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelMorphImpulseCopy(nanoem_mutable_model_morph_impulse_t *morph, const nanoem_model_morph_impulse_t *value)
{
    nanoemMutableModelMorphImpulseSetLocal(morph, nanoemModelMorphImpulseIsLocal(value));
    nanoemMutableModelMorphImpulseSetRigidBodyObject(morph, nanoemModelMorphImpulseGetRigidBodyObject(value));
    nanoemMutableModelMorphImpulseSetTorque(morph, nanoemModelMorphImpulseGetTorque(value));
    nanoemMutableModelMorphImpulseSetVelocity(morph, nanoemModelMorphImpulseGetVelocity(value));
}

void APIENTRY
nanoemMutableModelMorphImpulseSaveToBuffer(nanoem_mutable_model_morph_impulse_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_impulse_t *origin;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(buffer)) {
        origin = morph->origin;
        parent_model = nanoemModelMorphGetParentModel(origin->base.parent.morph);;
        if (nanoem_is_not_null(parent_model)) {
            nanoemMutableBufferWriteInteger(buffer, origin->rigid_body_index, parent_model->info.rigid_body_index_size, status);
            nanoemMutableBufferWriteByte(buffer, origin->is_local, status);
            nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->velocity, status);
            nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->torque, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

nanoem_model_morph_impulse_t *APIENTRY
nanoemMutableModelMorphImpulseGetOriginObject(nanoem_mutable_model_morph_impulse_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->origin : NULL;
}

void APIENTRY
nanoemMutableModelMorphImpulseDestroy(nanoem_mutable_model_morph_impulse_t *morph)
{
    if (nanoem_is_not_null(morph)) {
        if (nanoemMutableBaseMorphObjectCanDelete(&morph->base)) {
            nanoemModelMorphImpulseDestroy(morph->origin);
        }
        nanoem_free(morph);
    }
}

nanoem_mutable_model_morph_material_t *APIENTRY
nanoemMutableModelMorphMaterialCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status)
{
    nanoem_model_morph_material_t *material = nanoemModelMorphMaterialCreate(nanoemMutableModelMorphGetOriginObject(morph), status);
    return nanoemMutableModelMorphMaterialCreateInternal(material, nanoem_false, nanoem_false, status);
}

nanoem_mutable_model_morph_material_t *APIENTRY
nanoemMutableModelMorphMaterialCreateAsReference(nanoem_model_morph_material_t *morph, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    const nanoem_model_morph_t *parent_morph;
    nanoem_mutable_model_morph_material_t *mutable_morph = NULL;
    nanoem_bool_t is_in_morph;
    if (nanoem_is_not_null(morph)) {
        parent_morph = nanoemModelMorphMaterialGetParentMorph(morph);
        origin_model = nanoemModelMorphGetParentModel(parent_morph);
        if (nanoem_is_not_null(origin_model)) {
            is_in_morph = nanoemObjectArrayContainsObject((const void *const *) parent_morph->u.materials, morph, parent_morph->num_objects);
            mutable_morph = nanoemMutableModelMorphMaterialCreateInternal(morph, nanoem_true, is_in_morph, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return mutable_morph;
}

void APIENTRY
nanoemMutableModelMorphMaterialSetMaterialObject(nanoem_mutable_model_morph_material_t *morph, const nanoem_model_material_t *value)
{
    const nanoem_model_morph_t *parent_morph;
    const nanoem_model_t *parent_model;
    if (nanoem_is_not_null(morph)) {
        parent_morph = nanoemModelMorphMaterialGetParentMorph(morph->origin);
        parent_model = nanoemModelMorphGetParentModel(parent_morph);
        morph->origin->material_index = nanoemModelResolveMaterialObject(parent_model, value);
    }
}

void APIENTRY
nanoemMutableModelMorphMaterialSetAmbientColor(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value) && morph->origin->ambient_color.values != value) {
        nanoem_crt_memcpy(morph->origin->ambient_color.values, value, sizeof(morph->origin->ambient_color));
    }
}

void APIENTRY
nanoemMutableModelMorphMaterialSetDiffuseColor(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value) && morph->origin->diffuse_color.values != value) {
        nanoem_crt_memcpy(morph->origin->diffuse_color.values, value, sizeof(morph->origin->diffuse_color));
    }
}

void APIENTRY
nanoemMutableModelMorphMaterialSetSpecularColor(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value) && morph->origin->specular_color.values != value) {
        nanoem_crt_memcpy(morph->origin->specular_color.values, value, sizeof(morph->origin->specular_color));
    }
}

void APIENTRY
nanoemMutableModelMorphMaterialSetEdgeColor(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value) && morph->origin->edge_color.values != value) {
        nanoem_crt_memcpy(morph->origin->edge_color.values, value, sizeof(morph->origin->edge_color));
    }
}

void APIENTRY
nanoemMutableModelMorphMaterialSetDiffuseTextureBlend(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value) && morph->origin->diffuse_texture_blend.values != value) {
        nanoem_crt_memcpy(morph->origin->diffuse_texture_blend.values, value, sizeof(morph->origin->diffuse_texture_blend));
    }
}

void APIENTRY
nanoemMutableModelMorphMaterialSetSphereMapTextureBlend(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value) && morph->origin->sphere_map_texture_blend.values != value) {
        nanoem_crt_memcpy(morph->origin->sphere_map_texture_blend.values, value, sizeof(morph->origin->sphere_map_texture_blend));
    }
}

void APIENTRY
nanoemMutableModelMorphMaterialSetToonTextureBlend(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value) && morph->origin->toon_texture_blend.values != value) {
        nanoem_crt_memcpy(morph->origin->toon_texture_blend.values, value, sizeof(morph->origin->toon_texture_blend));
    }
}

void APIENTRY
nanoemMutableModelMorphMaterialSetDiffuseOpacity(nanoem_mutable_model_morph_material_t *morph, nanoem_f32_t value)
{
    if (nanoem_is_not_null(morph)) {
        morph->origin->diffuse_opacity = value;
    }
}

void APIENTRY
nanoemMutableModelMorphMaterialSetEdgeOpacity(nanoem_mutable_model_morph_material_t *morph, nanoem_f32_t value)
{
    if (nanoem_is_not_null(morph)) {
        morph->origin->edge_opacity = value;
    }
}

void APIENTRY
nanoemMutableModelMorphMaterialSetSpecularPower(nanoem_mutable_model_morph_material_t *morph, nanoem_f32_t value)
{
    if (nanoem_is_not_null(morph)) {
        morph->origin->specular_power = value;
    }
}

void APIENTRY
nanoemMutableModelMorphMaterialSetEdgeSize(nanoem_mutable_model_morph_material_t *morph, nanoem_f32_t value)
{
    if (nanoem_is_not_null(morph)) {
        morph->origin->edge_size = value;
    }
}

void APIENTRY
nanoemMutableModelMorphMaterialSetOperationType(nanoem_mutable_model_morph_material_t *morph, nanoem_model_morph_material_operation_type_t value)
{
    if (nanoem_is_not_null(morph)) {
        morph->origin->operation = value;
    }
}

void APIENTRY
nanoemMutableModelMorphMaterialCopy(nanoem_mutable_model_morph_material_t *morph, const nanoem_model_morph_material_t *value)
{
    nanoemMutableModelMorphMaterialSetAmbientColor(morph, nanoemModelMorphMaterialGetAmbientColor(value));
    nanoemMutableModelMorphMaterialSetDiffuseColor(morph, nanoemModelMorphMaterialGetDiffuseColor(value));
    nanoemMutableModelMorphMaterialSetDiffuseOpacity(morph, nanoemModelMorphMaterialGetDiffuseOpacity(value));
    nanoemMutableModelMorphMaterialSetDiffuseTextureBlend(morph, nanoemModelMorphMaterialGetDiffuseTextureBlend(value));
    nanoemMutableModelMorphMaterialSetEdgeColor(morph, nanoemModelMorphMaterialGetEdgeColor(value));
    nanoemMutableModelMorphMaterialSetEdgeOpacity(morph, nanoemModelMorphMaterialGetEdgeOpacity(value));
    nanoemMutableModelMorphMaterialSetEdgeSize(morph, nanoemModelMorphMaterialGetEdgeSize(value));
    nanoemMutableModelMorphMaterialSetMaterialObject(morph, nanoemModelMorphMaterialGetMaterialObject(value));
    nanoemMutableModelMorphMaterialSetOperationType(morph, nanoemModelMorphMaterialGetOperationType(value));
    nanoemMutableModelMorphMaterialSetSpecularColor(morph, nanoemModelMorphMaterialGetSpecularColor(value));
    nanoemMutableModelMorphMaterialSetSpecularPower(morph, nanoemModelMorphMaterialGetSpecularPower(value));
    nanoemMutableModelMorphMaterialSetSphereMapTextureBlend(morph, nanoemModelMorphMaterialGetSphereMapTextureBlend(value));
    nanoemMutableModelMorphMaterialSetToonTextureBlend(morph, nanoemModelMorphMaterialGetToonTextureBlend(value));
}

void APIENTRY
nanoemMutableModelMorphMaterialSaveToBuffer(nanoem_mutable_model_morph_material_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_material_t *origin;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(buffer)) {
        origin = morph->origin;
        parent_model = nanoemModelMorphGetParentModel(origin->base.parent.morph);
        if (nanoem_is_not_null(parent_model)) {
            nanoemMutableBufferWriteInteger(buffer, origin->material_index, parent_model->info.material_index_size, status);
            nanoemMutableBufferWriteByte(buffer, origin->operation, status);
            nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->diffuse_color, status);
            nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->diffuse_opacity, status);
            nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->specular_color, status);
            nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->specular_power, status);
            nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->ambient_color, status);
            nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->edge_color, status);
            nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->edge_opacity, status);
            nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->edge_size, status);
            nanoemMutableBufferWriteFloat32x4LittleEndian(buffer, &origin->diffuse_texture_blend, status);
            nanoemMutableBufferWriteFloat32x4LittleEndian(buffer, &origin->sphere_map_texture_blend, status);
            nanoemMutableBufferWriteFloat32x4LittleEndian(buffer, &origin->toon_texture_blend, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

nanoem_model_morph_material_t *APIENTRY
nanoemMutableModelMorphMaterialGetOriginObject(nanoem_mutable_model_morph_material_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->origin : NULL;
}

void APIENTRY
nanoemMutableModelMorphMaterialDestroy(nanoem_mutable_model_morph_material_t *morph)
{
    if (nanoem_is_not_null(morph)) {
        if (nanoemMutableBaseMorphObjectCanDelete(&morph->base)) {
            nanoemModelMorphMaterialDestroy(morph->origin);
        }
        nanoem_free(morph);
    }
}

nanoem_mutable_model_morph_uv_t *APIENTRY
nanoemMutableModelMorphUVCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status)
{
    nanoem_model_morph_uv_t *uv = nanoemModelMorphUVCreate(nanoemMutableModelMorphGetOriginObject(morph), status);
    return nanoemMutableModelMorphUVCreateInternal(uv, nanoem_false, nanoem_false, status);
}

nanoem_mutable_model_morph_uv_t *APIENTRY
nanoemMutableModelMorphUVCreateAsReference(nanoem_model_morph_uv_t *morph, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    const nanoem_model_morph_t *parent_morph;
    nanoem_mutable_model_morph_uv_t *mutable_morph = NULL;
    nanoem_bool_t is_in_morph;
    if (nanoem_is_not_null(morph)) {
        parent_morph = nanoemModelMorphUVGetParentMorph(morph);
        origin_model = nanoemModelMorphGetParentModel(parent_morph);
        if (nanoem_is_not_null(origin_model)) {
            is_in_morph = nanoemObjectArrayContainsObject((const void *const *) parent_morph->u.uvs, morph, parent_morph->num_objects);
            mutable_morph = nanoemMutableModelMorphUVCreateInternal(morph, nanoem_true, is_in_morph, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return mutable_morph;
}

void APIENTRY
nanoemMutableModelMorphUVSetVertexObject(nanoem_mutable_model_morph_uv_t *morph, const nanoem_model_vertex_t *value)
{
    if (nanoem_is_not_null(morph)) {
        morph->origin->vertex_index = nanoemModelObjectGetIndex(nanoemModelVertexGetModelObject(value));
    }
}

void APIENTRY
nanoemMutableModelMorphUVSetPosition(nanoem_mutable_model_morph_uv_t *morph, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value) && morph->origin->position.values != value) {
        nanoem_crt_memcpy(morph->origin->position.values, value, sizeof(morph->origin->position));
    }
}

void APIENTRY
nanoemMutableModelMorphUVCopy(nanoem_mutable_model_morph_uv_t *morph, const nanoem_model_morph_uv_t *value)
{
    nanoemMutableModelMorphUVSetPosition(morph, nanoemModelMorphUVGetPosition(value));
    nanoemMutableModelMorphUVSetVertexObject(morph, nanoemModelMorphUVGetVertexObject(value));
}

void APIENTRY
nanoemMutableModelMorphUVSaveToBuffer(nanoem_mutable_model_morph_uv_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_uv_t *origin;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(buffer)) {
        origin = morph->origin;
        parent_model = nanoemModelMorphGetParentModel(origin->base.parent.morph);
        if (nanoem_is_not_null(parent_model)) {
            nanoemMutableBufferWriteInteger(buffer, origin->vertex_index, parent_model->info.vertex_index_size, status);
            nanoemMutableBufferWriteFloat32x4LittleEndian(buffer, &origin->position, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

nanoem_model_morph_uv_t *APIENTRY
nanoemMutableModelMorphUVGetOriginObject(nanoem_mutable_model_morph_uv_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->origin : NULL;
}

void APIENTRY
nanoemMutableModelMorphUVDestroy(nanoem_mutable_model_morph_uv_t *morph)
{
    if (nanoem_is_not_null(morph)) {
        if (nanoemMutableBaseMorphObjectCanDelete(&morph->base)) {
            nanoemModelMorphUVDestroy(morph->origin);
        }
        nanoem_free(morph);
    }
}

nanoem_mutable_model_morph_vertex_t *APIENTRY
nanoemMutableModelMorphVertexCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status)
{
    nanoem_model_morph_vertex_t *vertex = nanoemModelMorphVertexCreate(nanoemMutableModelMorphGetOriginObject(morph), status);
    return nanoemMutableModelMorphVertexCreateInternal(vertex, nanoem_false, nanoem_false, status);
}

nanoem_mutable_model_morph_vertex_t *APIENTRY
nanoemMutableModelMorphVertexCreateAsReference(nanoem_model_morph_vertex_t *morph, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    const nanoem_model_morph_t *parent_morph;
    nanoem_mutable_model_morph_vertex_t *mutable_morph = NULL;
    nanoem_bool_t is_in_morph;
    if (nanoem_is_not_null(morph)) {
        parent_morph = nanoemModelMorphVertexGetParentMorph(morph);
        origin_model = nanoemModelMorphGetParentModel(parent_morph);
        if (nanoem_is_not_null(origin_model)) {
            is_in_morph = nanoemObjectArrayContainsObject((const void *const *) parent_morph->u.vertices, morph, parent_morph->num_objects);
            mutable_morph = nanoemMutableModelMorphVertexCreateInternal(morph, nanoem_true, is_in_morph, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return mutable_morph;
}

void APIENTRY
nanoemMutableModelMorphVertexSetVertexObject(nanoem_mutable_model_morph_vertex_t *morph, const nanoem_model_vertex_t *value)
{
    if (nanoem_is_not_null(morph)) {
        morph->origin->vertex_index = nanoemModelObjectGetIndex(nanoemModelVertexGetModelObject(value));
    }
}

void APIENTRY
nanoemMutableModelMorphVertexSetPosition(nanoem_mutable_model_morph_vertex_t *morph, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value) && morph->origin->position.values != value) {
        nanoem_crt_memcpy(morph->origin->position.values, value, sizeof(morph->origin->position));
    }
}

void APIENTRY
nanoemMutableModelMorphVertexCopy(nanoem_mutable_model_morph_vertex_t *morph, const nanoem_model_morph_vertex_t *value)
{
    nanoemMutableModelMorphVertexSetPosition(morph, nanoemModelMorphVertexGetPosition(value));
    nanoemMutableModelMorphVertexSetVertexObject(morph, nanoemModelMorphVertexGetVertexObject(value));
}

void APIENTRY
nanoemMutableModelMorphVertexSaveToBuffer(nanoem_mutable_model_morph_vertex_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_vertex_t *origin;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(buffer)) {
        origin = morph->origin;
        parent_model = nanoemModelMorphGetParentModel(origin->base.parent.morph);
        if (nanoem_is_not_null(parent_model)) {
            nanoemMutableBufferWriteInteger(buffer, origin->vertex_index, parent_model->info.vertex_index_size, status);
            nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->position, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

nanoem_model_morph_vertex_t *APIENTRY
nanoemMutableModelMorphVertexGetOriginObject(nanoem_mutable_model_morph_vertex_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->origin : NULL;
}

void APIENTRY
nanoemMutableModelMorphVertexDestroy(nanoem_mutable_model_morph_vertex_t *morph)
{
    if (nanoem_is_not_null(morph)) {
        if (nanoemMutableBaseMorphObjectCanDelete(&morph->base)) {
            nanoemModelMorphVertexDestroy(morph->origin);
        }
        nanoem_free(morph);
    }
}

nanoem_mutable_model_morph_t *APIENTRY
nanoemMutableModelMorphCreate(nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_morph_t *morph = nanoemModelMorphCreate(model, status);
    return nanoemMutableModelMorphCreateInternal(morph, nanoem_false, nanoem_false, status);
}

nanoem_mutable_model_morph_t *APIENTRY
nanoemMutableModelMorphCreateAsReference(nanoem_model_morph_t *morph, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    nanoem_mutable_model_morph_t *mutable_morph = NULL;
    nanoem_bool_t is_in_model;
    if (nanoem_is_not_null(morph)) {
        origin_model = nanoemModelMorphGetParentModel(morph);
        if (nanoem_is_not_null(origin_model)) {
            is_in_model = nanoemObjectArrayContainsIndexedObject((const void *const *) origin_model->morphs, morph, origin_model->num_morphs, morph->base.index);
            mutable_morph = nanoemMutableModelMorphCreateInternal(morph, nanoem_true, is_in_model, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return mutable_morph;
}

void APIENTRY
nanoemMutableModelMorphSetName(nanoem_mutable_model_morph_t *morph, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(morph)) {
        parent_model = nanoemModelMorphGetParentModel(morph->origin);
        if (nanoem_is_not_null(parent_model)) {
            factory = parent_model->factory;
            switch (language) {
            case NANOEM_LANGUAGE_TYPE_ENGLISH:
                nanoemUnicodeStringFactoryAssignString(factory, &morph->origin->name_en, value, nanoemModelGetCodecType(parent_model), status);
                break;
            case NANOEM_LANGUAGE_TYPE_JAPANESE:
                nanoemUnicodeStringFactoryAssignString(factory, &morph->origin->name_ja, value, nanoemModelGetCodecType(parent_model), status);
                break;
            case NANOEM_LANGUAGE_TYPE_MAX_ENUM:
            case NANOEM_LANGUAGE_TYPE_UNKNOWN:
            default:
                break;
            }
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMorphSetCategory(nanoem_mutable_model_morph_t *morph, nanoem_model_morph_category_t value)
{
    if (nanoem_is_not_null(morph)) {
        morph->origin->category = value;
    }
}

void APIENTRY
nanoemMutableModelMorphSetType(nanoem_mutable_model_morph_t *morph, nanoem_model_morph_type_t value)
{
    if (nanoem_is_not_null(morph)) {
        morph->origin->type = value;
    }
}

void APIENTRY
nanoemMutableModelMorphInsertBoneMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_bone_t *value, int index, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_t *origin_morph;
    nanoem_model_morph_bone_t *origin_bone_morph;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value)) {
        if (value->origin->base.parent.morph->type != NANOEM_MODEL_MORPH_TYPE_BONE) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_MORPH_TYPE_MISMATCH);
            return;
        }
        parent_model = nanoemModelMorphGetParentModel(value->origin->base.parent.morph);;
        if (!nanoemModelIsPMX(parent_model)) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_VERSION_INCOMPATIBLE);
            return;
        }
        origin_morph = morph->origin;
        origin_bone_morph = value->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_morph->u.bones, (nanoem_model_object_t *) origin_bone_morph, index, NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS, &origin_morph->num_objects, &morph->num_allocated_objects, status);
        if (!nanoem_status_ptr_has_error(status)) {
            value->origin->base.parent.morph = origin_morph;
            value->base.is_in_morph = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMorphInsertFlipMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_flip_t *value, int index, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_t *origin_morph;
    nanoem_model_morph_flip_t *origin_flip_morph;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value)) {
        if (value->origin->base.parent.morph->type != NANOEM_MODEL_MORPH_TYPE_FLIP) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_MORPH_TYPE_MISMATCH);
            return;
        }
        parent_model = nanoemModelMorphGetParentModel(value->origin->base.parent.morph);;
        if (!nanoemModelIsPMX21(parent_model)) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_VERSION_INCOMPATIBLE);
            return;
        }
        origin_morph = morph->origin;
        origin_flip_morph = value->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_morph->u.flips, (nanoem_model_object_t *) origin_flip_morph, index, NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS, &origin_morph->num_objects, &morph->num_allocated_objects, status);
        if (!nanoem_status_ptr_has_error(status)) {
            value->origin->base.parent.morph = origin_morph;
            value->base.is_in_morph = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMorphInsertGroupMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_group_t *value, int index, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_t *origin_morph;
    nanoem_model_morph_group_t *origin_group_morph;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value)) {
        if (value->origin->base.parent.morph->type != NANOEM_MODEL_MORPH_TYPE_GROUP) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_MORPH_TYPE_MISMATCH);
            return;
        }
        parent_model = nanoemModelMorphGetParentModel(value->origin->base.parent.morph);;
        if (!nanoemModelIsPMX(parent_model)) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_VERSION_INCOMPATIBLE);
            return;
        }
        origin_morph = morph->origin;
        origin_group_morph = value->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_morph->u.groups, (nanoem_model_object_t *) origin_group_morph, index, NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS, &origin_morph->num_objects, &morph->num_allocated_objects, status);
        if (!nanoem_status_ptr_has_error(status)) {
            value->origin->base.parent.morph = origin_morph;
            value->base.is_in_morph = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMorphInsertImpulseMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_impulse_t *value, int index, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_t *origin_morph;
    nanoem_model_morph_impulse_t *origin_impulse_morph;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value)) {
        if (value->origin->base.parent.morph->type != NANOEM_MODEL_MORPH_TYPE_IMPULUSE) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_MORPH_TYPE_MISMATCH);
            return;
        }
        parent_model = nanoemModelMorphGetParentModel(value->origin->base.parent.morph);
        if (!nanoemModelIsPMX21(parent_model)) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_VERSION_INCOMPATIBLE);
            return;
        }
        origin_morph = morph->origin;
        origin_impulse_morph = value->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_morph->u.impulses, (nanoem_model_object_t *) origin_impulse_morph, index, NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS, &origin_morph->num_objects, &morph->num_allocated_objects, status);
        if (!nanoem_status_ptr_has_error(status)) {
            value->origin->base.parent.morph = origin_morph;
            value->base.is_in_morph = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMorphInsertMaterialMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_material_t *value, int index, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_t *origin_morph;
    nanoem_model_morph_material_t *origin_material_morph;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value)) {
        if (value->origin->base.parent.morph->type != NANOEM_MODEL_MORPH_TYPE_MATERIAL) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_MORPH_TYPE_MISMATCH);
            return;
        }
        parent_model = nanoemModelMorphGetParentModel(value->origin->base.parent.morph);
        if (!nanoemModelIsPMX(parent_model)) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_VERSION_INCOMPATIBLE);
            return;
        }
        origin_morph = morph->origin;
        origin_material_morph = value->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_morph->u.materials, (nanoem_model_object_t *) origin_material_morph, index, NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS, &origin_morph->num_objects, &morph->num_allocated_objects, status);
        if (!nanoem_status_ptr_has_error(status)) {
            value->origin->base.parent.morph = origin_morph;
            value->base.is_in_morph = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMorphInsertUVMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_uv_t *value, int index, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_model_morph_t *origin_morph;
    nanoem_model_morph_uv_t *origin_uv_morph;
    nanoem_model_morph_type_t type;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value)) {
        type = value->origin->base.parent.morph->type;
        if (type < NANOEM_MODEL_MORPH_TYPE_UVA1 || type > NANOEM_MODEL_MORPH_TYPE_UVA4) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_MORPH_TYPE_MISMATCH);
            return;
        }
        parent_model = nanoemModelMorphGetParentModel(value->origin->base.parent.morph);
        if (!nanoemModelIsPMX(parent_model)) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_VERSION_INCOMPATIBLE);
            return;
        }
        origin_morph = morph->origin;
        origin_uv_morph = value->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_morph->u.uvs, (nanoem_model_object_t *) origin_uv_morph, index, NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS, &origin_morph->num_objects, &morph->num_allocated_objects, status);
        if (!nanoem_status_ptr_has_error(status)) {
            value->origin->base.parent.morph = origin_morph;
            value->base.is_in_morph = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMorphInsertVertexMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_vertex_t *value, int index, nanoem_status_t *status)
{
    nanoem_model_morph_t *origin_morph;
    nanoem_model_morph_vertex_t *origin_vertex_morph;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value)) {
        if (value->origin->base.parent.morph->type != NANOEM_MODEL_MORPH_TYPE_VERTEX) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_MORPH_TYPE_MISMATCH);
            return;
        }
        origin_morph = morph->origin;
        origin_vertex_morph = value->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_morph->u.vertices, (nanoem_model_object_t *) origin_vertex_morph, index, NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS, &origin_morph->num_objects, &morph->num_allocated_objects, status);
        if (!nanoem_status_ptr_has_error(status)) {
            value->origin->base.parent.morph = origin_morph;
            value->base.is_in_morph = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMorphRemoveBoneMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_bone_t *value, nanoem_status_t *status)
{
    nanoem_model_morph_t *origin_morph;
    nanoem_model_morph_bone_t *origin_bone_morph;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value)) {
        origin_morph = morph->origin;
        origin_bone_morph = value->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_morph->u.bones, (nanoem_model_object_t *) origin_bone_morph, &origin_morph->num_objects, NANOEM_STATUS_ERROR_MODEL_MORPH_BONE_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            value->origin->base.parent.morph = NULL;
            value->base.is_in_morph = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMorphRemoveFlipMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_flip_t *value, nanoem_status_t *status)
{
    nanoem_model_morph_t *origin_morph;
    nanoem_model_morph_flip_t *origin_flip_morph;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value)) {
        origin_morph = morph->origin;
        origin_flip_morph = value->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_morph->u.flips, (nanoem_model_object_t *) origin_flip_morph, &origin_morph->num_objects, NANOEM_STATUS_ERROR_MODEL_MORPH_FLIP_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            value->origin->base.parent.morph = NULL;
            value->base.is_in_morph = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMorphRemoveGroupMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_group_t *value, nanoem_status_t *status)
{
    nanoem_model_morph_t *origin_morph;
    nanoem_model_morph_group_t *origin_group_morph;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value)) {
        origin_morph = morph->origin;
        origin_group_morph = value->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_morph->u.groups, (nanoem_model_object_t *) origin_group_morph, &origin_morph->num_objects, NANOEM_STATUS_ERROR_MODEL_MORPH_GROUP_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            value->origin->base.parent.morph = NULL;
            value->base.is_in_morph = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMorphRemoveImpulseMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_impulse_t *value, nanoem_status_t *status)
{
    nanoem_model_morph_t *origin_morph;
    nanoem_model_morph_impulse_t *origin_impulse_morph;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value)) {
        origin_morph = morph->origin;
        origin_impulse_morph = value->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_morph->u.impulses, (nanoem_model_object_t *) origin_impulse_morph, &origin_morph->num_objects, NANOEM_STATUS_ERROR_MODEL_MORPH_IMPULSE_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            value->origin->base.parent.morph = NULL;
            value->base.is_in_morph = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMorphRemoveMaterialMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_material_t *value, nanoem_status_t *status)
{
    nanoem_model_morph_t *origin_morph;
    nanoem_model_morph_material_t *origin_material_morph;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value)) {
        origin_morph = morph->origin;
        origin_material_morph = value->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_morph->u.materials, (nanoem_model_object_t *) origin_material_morph, &origin_morph->num_objects, NANOEM_STATUS_ERROR_MODEL_MORPH_MATERIAL_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            value->origin->base.parent.morph = NULL;
            value->base.is_in_morph = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMorphRemoveUVMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_uv_t *value, nanoem_status_t *status)
{
    nanoem_model_morph_t *origin_morph;
    nanoem_model_morph_uv_t *origin_uv_morph;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value)) {
        origin_morph = morph->origin;
        origin_uv_morph = value->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_morph->u.uvs, (nanoem_model_object_t *) origin_uv_morph, &origin_morph->num_objects, NANOEM_STATUS_ERROR_MODEL_MORPH_UV_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            value->origin->base.parent.morph = NULL;
            value->base.is_in_morph = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMorphRemoveVertexMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_vertex_t *value, nanoem_status_t *status)
{
    nanoem_model_morph_t *origin_morph;
    nanoem_model_morph_vertex_t *origin_vertex_morph;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(value)) {
        origin_morph = morph->origin;
        origin_vertex_morph = value->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_morph->u.vertices, (nanoem_model_object_t *) origin_vertex_morph, &origin_morph->num_objects, NANOEM_STATUS_ERROR_MODEL_MORPH_VERTEX_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            value->origin->base.parent.morph = NULL;
            value->base.is_in_morph = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMorphCopy(nanoem_mutable_model_morph_t *morph, const nanoem_model_morph_t *value, nanoem_status_t *status)
{
    const nanoem_model_morph_t *origin = nanoemMutableModelMorphGetOriginObject(morph);
    nanoem_model_morph_bone_t *const *bones;
    nanoem_mutable_model_morph_bone_t *bone;
    nanoem_model_morph_flip_t *const *flips;
    nanoem_mutable_model_morph_flip_t *flip;
    nanoem_model_morph_group_t *const *groups;
    nanoem_mutable_model_morph_group_t *group;
    nanoem_model_morph_impulse_t *const *impulses;
    nanoem_mutable_model_morph_impulse_t *impulse;
    nanoem_model_morph_material_t *const *materials;
    nanoem_mutable_model_morph_material_t *material;
    nanoem_model_morph_uv_t *const *uvs;
    nanoem_mutable_model_morph_uv_t *uv;
    nanoem_model_morph_vertex_t *const *vertices;
    nanoem_mutable_model_morph_vertex_t *vertex;
    nanoem_rsize_t i, num_objects;
    nanoemMutableModelMorphSetCategory(morph, nanoemModelMorphGetCategory(value));
    nanoemMutableModelMorphSetName(morph, nanoemModelMorphGetName(value, NANOEM_LANGUAGE_TYPE_JAPANESE), NANOEM_LANGUAGE_TYPE_JAPANESE, status);
    nanoemMutableModelMorphSetName(morph, nanoemModelMorphGetName(value, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH, status);
    nanoemMutableModelMorphSetType(morph, nanoemModelMorphGetType(value));
    bones = nanoemModelMorphGetAllBoneMorphObjects(origin, &num_objects);
    for (i = 0; i < num_objects; i++) {
        bone = nanoemMutableModelMorphBoneCreateAsReference(bones[i], status);
        nanoemMutableModelMorphRemoveBoneMorphObject(morph, bone, status);
        nanoemMutableModelMorphBoneDestroy(bone);
    }
    bones = nanoemModelMorphGetAllBoneMorphObjects(value, &num_objects);
    for (i = 0; i < num_objects; i++) {
        bone = nanoemMutableModelMorphBoneCreate(morph, status);
        nanoemMutableModelMorphBoneCopy(bone, bones[i]);
        nanoemMutableModelMorphInsertBoneMorphObject(morph, bone, -1, status);
        nanoemMutableModelMorphBoneDestroy(bone);
    }
    flips = nanoemModelMorphGetAllFlipMorphObjects(origin, &num_objects);
    for (i = 0; i < num_objects; i++) {
        flip = nanoemMutableModelMorphFlipCreateAsReference(flips[i], status);
        nanoemMutableModelMorphRemoveFlipMorphObject(morph, flip, status);
        nanoemMutableModelMorphFlipDestroy(flip);
    }
    flips = nanoemModelMorphGetAllFlipMorphObjects(value, &num_objects);
    for (i = 0; i < num_objects; i++) {
        flip = nanoemMutableModelMorphFlipCreate(morph, status);
        nanoemMutableModelMorphFlipCopy(flip, flips[i]);
        nanoemMutableModelMorphInsertFlipMorphObject(morph, flip, -1, status);
        nanoemMutableModelMorphFlipDestroy(flip);
    }
    groups = nanoemModelMorphGetAllGroupMorphObjects(origin, &num_objects);
    for (i = 0; i < num_objects; i++) {
        group = nanoemMutableModelMorphGroupCreateAsReference(groups[i], status);
        nanoemMutableModelMorphRemoveGroupMorphObject(morph, group, status);
        nanoemMutableModelMorphGroupDestroy(group);
    }
    groups = nanoemModelMorphGetAllGroupMorphObjects(value, &num_objects);
    for (i = 0; i < num_objects; i++) {
        group = nanoemMutableModelMorphGroupCreate(morph, status);
        nanoemMutableModelMorphGroupCopy(group, groups[i]);
        nanoemMutableModelMorphInsertGroupMorphObject(morph, group, -1, status);
        nanoemMutableModelMorphGroupDestroy(group);
    }
    impulses = nanoemModelMorphGetAllImpulseMorphObjects(origin, &num_objects);
    for (i = 0; i < num_objects; i++) {
        impulse = nanoemMutableModelMorphImpulseCreateAsReference(impulses[i], status);
        nanoemMutableModelMorphRemoveImpulseMorphObject(morph, impulse, status);
        nanoemMutableModelMorphImpulseDestroy(impulse);
    }
    impulses = nanoemModelMorphGetAllImpulseMorphObjects(value, &num_objects);
    for (i = 0; i < num_objects; i++) {
        impulse = nanoemMutableModelMorphImpulseCreate(morph, status);
        nanoemMutableModelMorphImpulseCopy(impulse, impulses[i]);
        nanoemMutableModelMorphInsertImpulseMorphObject(morph, impulse, -1, status);
        nanoemMutableModelMorphImpulseDestroy(impulse);
    }
    materials = nanoemModelMorphGetAllMaterialMorphObjects(origin, &num_objects);
    for (i = 0; i < num_objects; i++) {
        material = nanoemMutableModelMorphMaterialCreateAsReference(materials[i], status);
        nanoemMutableModelMorphRemoveMaterialMorphObject(morph, material, status);
        nanoemMutableModelMorphMaterialDestroy(material);
    }
    materials = nanoemModelMorphGetAllMaterialMorphObjects(value, &num_objects);
    for (i = 0; i < num_objects; i++) {
        material = nanoemMutableModelMorphMaterialCreate(morph, status);
        nanoemMutableModelMorphMaterialCopy(material, materials[i]);
        nanoemMutableModelMorphInsertMaterialMorphObject(morph, material, -1, status);
        nanoemMutableModelMorphMaterialDestroy(material);
    }
    uvs = nanoemModelMorphGetAllUVMorphObjects(origin, &num_objects);
    for (i = 0; i < num_objects; i++) {
        uv = nanoemMutableModelMorphUVCreateAsReference(uvs[i], status);
        nanoemMutableModelMorphRemoveUVMorphObject(morph, uv, status);
        nanoemMutableModelMorphUVDestroy(uv);
    }
    uvs = nanoemModelMorphGetAllUVMorphObjects(value, &num_objects);
    for (i = 0; i < num_objects; i++) {
        uv = nanoemMutableModelMorphUVCreate(morph, status);
        nanoemMutableModelMorphUVCopy(uv, uvs[i]);
        nanoemMutableModelMorphInsertUVMorphObject(morph, uv, -1, status);
        nanoemMutableModelMorphUVDestroy(uv);
    }
    vertices = nanoemModelMorphGetAllVertexMorphObjects(origin, &num_objects);
    for (i = 0; i < num_objects; i++) {
        vertex = nanoemMutableModelMorphVertexCreateAsReference(vertices[i], status);
        nanoemMutableModelMorphRemoveVertexMorphObject(morph, vertex, status);
        nanoemMutableModelMorphVertexDestroy(vertex);
    }
    vertices = nanoemModelMorphGetAllVertexMorphObjects(value, &num_objects);
    for (i = 0; i < num_objects; i++) {
        vertex = nanoemMutableModelMorphVertexCreate(morph, status);
        nanoemMutableModelMorphVertexCopy(vertex, vertices[i]);
        nanoemMutableModelMorphInsertVertexMorphObject(morph, vertex, -1, status);
        nanoemMutableModelMorphVertexDestroy(vertex);
    }
}

nanoem_model_morph_t *APIENTRY
nanoemMutableModelMorphGetOriginObject(nanoem_mutable_model_morph_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->origin : NULL;
}

void APIENTRY
nanoemMutableModelMorphSaveToBuffer(nanoem_mutable_model_morph_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_morph_t *origin;
    const nanoem_model_t *parent_model;
    nanoem_u8_t name_buffer[PMD_MORPH_NAME_LENGTH + 1];
    nanoem_unicode_string_factory_t *factory;
    nanoem_mutable_model_morph_bone_t mutable_bone;
    nanoem_mutable_model_morph_flip_t mutable_flip;
    nanoem_mutable_model_morph_group_t mutable_group;
    nanoem_mutable_model_morph_impulse_t mutable_impulse;
    nanoem_mutable_model_morph_material_t mutable_material;
    nanoem_mutable_model_morph_uv_t mutable_uv;
    nanoem_mutable_model_morph_vertex_t mutable_vertex;
    nanoem_model_morph_vertex_t *origin_vertex;
    nanoem_codec_type_t codec;
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(morph) && nanoem_is_not_null(buffer)) {
        origin = morph->origin;
        num_objects = origin->num_objects;
        parent_model = nanoemModelMorphGetParentModel(origin);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
        }
        else if (nanoemModelIsPMX(parent_model)) {
            factory = parent_model->factory;
            codec = nanoemModelGetCodecType(parent_model);
            nanoemMutableBufferWriteUnicodeString(buffer, origin->name_ja, factory, codec, status);
            nanoemMutableBufferWriteUnicodeString(buffer, origin->name_en, factory, codec, status);
            nanoemMutableBufferWriteByte(buffer, origin->category, status);
            nanoemMutableBufferWriteByte(buffer, origin->type, status);
            nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_objects, status);
            switch (origin->type) {
            case NANOEM_MODEL_MORPH_TYPE_BONE:
                for (i = 0; i < num_objects; i++) {
                    nanoemMutableModelMorphBoneInitialize(&mutable_bone, origin->u.bones[i], nanoem_true, nanoem_true);
                    nanoemMutableModelMorphBoneSaveToBuffer(&mutable_bone, buffer, status);
                    if (nanoem_status_ptr_has_error(status)) {
                        break;
                    }
                }
                break;
            case NANOEM_MODEL_MORPH_TYPE_FLIP:
                for (i = 0; i < num_objects; i++) {
                    nanoemMutableModelMorphFlipInitialize(&mutable_flip, origin->u.flips[i], nanoem_true, nanoem_true);
                    nanoemMutableModelMorphFlipSaveToBuffer(&mutable_flip, buffer, status);
                    if (nanoem_status_ptr_has_error(status)) {
                        break;
                    }
                }
                break;
            case NANOEM_MODEL_MORPH_TYPE_GROUP:
                for (i = 0; i < num_objects; i++) {
                    nanoemMutableModelMorphGroupInitialize(&mutable_group, origin->u.groups[i], nanoem_true, nanoem_true);
                    nanoemMutableModelMorphGroupSaveToBuffer(&mutable_group, buffer, status);
                    if (nanoem_status_ptr_has_error(status)) {
                        break;
                    }
                }
                break;
            case NANOEM_MODEL_MORPH_TYPE_IMPULUSE:
                for (i = 0; i < num_objects; i++) {
                    nanoemMutableModelMorphImpulseInitialize(&mutable_impulse, origin->u.impulses[i], nanoem_true, nanoem_true);
                    nanoemMutableModelMorphImpulseSaveToBuffer(&mutable_impulse, buffer, status);
                    if (nanoem_status_ptr_has_error(status)) {
                        break;
                    }
                }
                break;
            case NANOEM_MODEL_MORPH_TYPE_MATERIAL:
                for (i = 0; i < num_objects; i++) {
                    nanoemMutableModelMorphMaterialInitialize(&mutable_material, origin->u.materials[i], nanoem_true, nanoem_true);
                    nanoemMutableModelMorphMaterialSaveToBuffer(&mutable_material, buffer, status);
                    if (nanoem_status_ptr_has_error(status)) {
                        break;
                    }
                }
                break;
            case NANOEM_MODEL_MORPH_TYPE_TEXTURE:
            case NANOEM_MODEL_MORPH_TYPE_UVA1:
            case NANOEM_MODEL_MORPH_TYPE_UVA2:
            case NANOEM_MODEL_MORPH_TYPE_UVA3:
            case NANOEM_MODEL_MORPH_TYPE_UVA4:
                for (i = 0; i < num_objects; i++) {
                    nanoemMutableModelMorphUVInitialize(&mutable_uv, origin->u.uvs[i], nanoem_true, nanoem_true);
                    nanoemMutableModelMorphUVSaveToBuffer(&mutable_uv, buffer, status);
                    if (nanoem_status_ptr_has_error(status)) {
                        break;
                    }
                }
                break;
            case NANOEM_MODEL_MORPH_TYPE_VERTEX:
                for (i = 0; i < num_objects; i++) {
                    nanoemMutableModelMorphVertexInitialize(&mutable_vertex, origin->u.vertices[i], nanoem_true, nanoem_true);
                    nanoemMutableModelMorphVertexSaveToBuffer(&mutable_vertex, buffer, status);
                    if (nanoem_status_ptr_has_error(status)) {
                        break;
                    }
                }
                break;
            case NANOEM_MODEL_MORPH_TYPE_MAX_ENUM:
            case NANOEM_MODEL_MORPH_TYPE_UNKNOWN:
            default:
                nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_MORPH_CORRUPTED);
            }
        }
        else {
            factory = parent_model->factory;
            nanoemMutableBufferWriteCp932String(buffer, origin->name_ja, factory, name_buffer, sizeof(name_buffer), status);
            nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_objects, status);
            nanoemMutableBufferWriteByte(buffer, origin->category, status);
            if (origin->category == NANOEM_MODEL_MORPH_CATEGORY_BASE) {
                for (i = 0; i < num_objects; i++) {
                    origin_vertex = origin->u.vertices[i];
                    if (nanoem_is_not_null(origin_vertex)) {
                        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin_vertex->vertex_index, status);
                        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin_vertex->position, status);
                    }
                }
            }
            else {
                for (i = 0; i < num_objects; i++) {
                    origin_vertex = origin->u.vertices[i];
                    if (nanoem_is_not_null(origin_vertex)) {
                        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin_vertex->relative_index, status);
                        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin_vertex->position, status);
                    }
                }
            }
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelMorphDestroy(nanoem_mutable_model_morph_t *morph)
{
    if (nanoem_is_not_null(morph)) {
        if (nanoemMutableBaseModelObjectCanDelete(&morph->base)) {
            nanoemModelMorphDestroy(morph->origin);
        }
        nanoem_free(morph);
    }
}

nanoem_mutable_model_label_t *APIENTRY
nanoemMutableModelLabelCreate(nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_label_t *label = nanoemModelLabelCreate(model, status);
    return nanoemMutableModelLabelCreateInternal(label, nanoem_false, nanoem_false, status);
}

nanoem_mutable_model_label_t *APIENTRY
nanoemMutableModelLabelCreateAsReference(nanoem_model_label_t *label, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    nanoem_mutable_model_label_t *mutable_label = NULL;
    nanoem_bool_t is_in_model;
    if (nanoem_is_not_null(label)) {
        origin_model = nanoemModelLabelGetParentModel(label);
        if (nanoem_is_not_null(origin_model)) {
            is_in_model = nanoemObjectArrayContainsIndexedObject((const void *const *) origin_model->labels, label, origin_model->num_labels, label->base.index);
            mutable_label = nanoemMutableModelLabelCreateInternal(label, nanoem_true, is_in_model, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return mutable_label;
}

void APIENTRY
nanoemMutableModelLabelSetName(nanoem_mutable_model_label_t *label, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(label)) {
        parent_model = nanoemModelLabelGetParentModel(label->origin);
        if (nanoem_is_not_null(parent_model)) {
            factory = parent_model->factory;
            switch (language) {
            case NANOEM_LANGUAGE_TYPE_ENGLISH:
                nanoemUnicodeStringFactoryAssignString(factory, &label->origin->name_en, value, nanoemModelGetCodecType(parent_model), status);
                break;
            case NANOEM_LANGUAGE_TYPE_JAPANESE:
                nanoemUnicodeStringFactoryAssignString(factory, &label->origin->name_ja, value, nanoemModelGetCodecType(parent_model), status);
                break;
            case NANOEM_LANGUAGE_TYPE_MAX_ENUM:
            case NANOEM_LANGUAGE_TYPE_UNKNOWN:
            default:
                break;
            }
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelLabelSetSpecial(nanoem_mutable_model_label_t *label, nanoem_bool_t value)
{
    if (nanoem_is_not_null(label)) {
        label->origin->is_special = value ? 1 : 0;
    }
}

void APIENTRY
nanoemMutableModelLabelCopy(nanoem_mutable_model_label_t *label, const nanoem_model_label_t *value, nanoem_status_t *status)
{
    const nanoem_model_label_t *origin = nanoemMutableModelLabelGetOriginObject(label);
    nanoem_model_label_item_t *const *items, *item;
    nanoem_mutable_model_label_item_t *mutable_item;
    nanoem_rsize_t i, num_items;
    nanoemMutableModelLabelSetName(label, nanoemModelLabelGetName(value, NANOEM_LANGUAGE_TYPE_JAPANESE), NANOEM_LANGUAGE_TYPE_JAPANESE, status);
    nanoemMutableModelLabelSetName(label, nanoemModelLabelGetName(value, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH, status);
    nanoemMutableModelLabelSetSpecial(label, nanoemModelLabelIsSpecial(value));
    items = nanoemModelLabelGetAllItemObjects(origin, &num_items);
    for (i = 0; i < num_items; i++) {
        item = items[i];
        switch (item->type) {
        case NANOEM_MODEL_LABEL_ITEM_TYPE_BONE:
            mutable_item = nanoemMutableModelLabelItemCreateFromBoneObject(label, item->u.bone, status);
            break;
        case NANOEM_MODEL_LABEL_ITEM_TYPE_MORPH:
            mutable_item = nanoemMutableModelLabelItemCreateFromMorphObject(label, item->u.morph, status);
            break;
        default:
            mutable_item = 0;
            break;
        }
        if (mutable_item) {
            nanoemMutableModelLabelRemoveItemObject(label, mutable_item, status);
            nanoemMutableModelLabelItemDestroy(mutable_item);
        }
    }
    items = nanoemModelLabelGetAllItemObjects(value, &num_items);
    for (i = 0; i < num_items; i++) {
        item = items[i];
        switch (item->type) {
        case NANOEM_MODEL_LABEL_ITEM_TYPE_BONE:
            mutable_item = nanoemMutableModelLabelItemCreateFromBoneObject(label, item->u.bone, status);
            break;
        case NANOEM_MODEL_LABEL_ITEM_TYPE_MORPH:
            mutable_item = nanoemMutableModelLabelItemCreateFromMorphObject(label, item->u.morph, status);
            break;
        default:
            mutable_item = 0;
            break;
        }
        if (mutable_item) {
            nanoemMutableModelLabelInsertItemObject(label, mutable_item, -1, status);
            nanoemMutableModelLabelItemDestroy(mutable_item);
        }
    }
}

nanoem_model_label_t *APIENTRY
nanoemMutableModelLabelGetOriginObject(nanoem_mutable_model_label_t *label)
{
    return nanoem_is_not_null(label) ? label->origin : NULL;
}

void APIENTRY
nanoemMutableModelLabelSaveToBuffer(nanoem_mutable_model_label_t *label, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_label_t *origin;
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    nanoem_model_label_item_t *item;
    nanoem_codec_type_t codec;
    nanoem_rsize_t num_items, i;
    if (nanoem_is_not_null(label) && nanoem_is_not_null(buffer)) {
        origin = label->origin;
        parent_model = nanoemModelLabelGetParentModel(origin);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
        }
        else if (nanoemModelIsPMX(parent_model)) {
            factory = parent_model->factory;
            codec = nanoemModelGetCodecType(parent_model);
            nanoemMutableBufferWriteUnicodeString(buffer, origin->name_ja, factory, codec, status);
            nanoemMutableBufferWriteUnicodeString(buffer, origin->name_en, factory, codec, status);
            nanoemMutableBufferWriteByte(buffer, origin->is_special, status);
            num_items = origin->num_items;
            nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_items, status);
            for (i = 0; i < num_items; i++) {
                item = origin->items[i];
                switch (item->type) {
                case NANOEM_MODEL_LABEL_ITEM_TYPE_BONE:
                    nanoemMutableBufferWriteByte(buffer, NANOEM_MODEL_LABEL_ITEM_TYPE_BONE, status);
                    nanoemMutableBufferWriteInteger(buffer, nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(item->u.bone)), parent_model->info.bone_index_size, status);
                    break;
                case NANOEM_MODEL_LABEL_ITEM_TYPE_MORPH:
                    nanoemMutableBufferWriteByte(buffer, NANOEM_MODEL_LABEL_ITEM_TYPE_MORPH, status);
                    nanoemMutableBufferWriteInteger(buffer, nanoemModelObjectGetIndex(nanoemModelMorphGetModelObject(item->u.morph)), parent_model->info.morph_index_size, status);
                    break;
                case NANOEM_MODEL_LABEL_ITEM_TYPE_MAX_ENUM:
                case NANOEM_MODEL_LABEL_ITEM_TYPE_UNKNOWN:
                    nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_LABEL_CORRUPTED);
                }
            }
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelLabelDestroy(nanoem_mutable_model_label_t *label)
{
    if (nanoem_is_not_null(label)) {
        if (nanoemMutableBaseModelObjectCanDelete(&label->base)) {
            nanoemModelLabelDestroy(label->origin);
        }
        nanoem_free(label);
    }
}

static nanoem_mutable_model_label_item_t *
nanoemMutableModelLabelItemCreateInternal(nanoem_model_label_item_t *origin, nanoem_bool_t is_reference, nanoem_bool_t is_in_label, nanoem_status_t *status)
{
    nanoem_mutable_model_label_item_t *item = NULL;
    if (nanoem_is_not_null(origin)) {
        item = (nanoem_mutable_model_label_item_t *) nanoem_calloc(1, sizeof(*item), status);
        if (nanoem_is_not_null(item)) {
            item->origin = origin;
            item->base.is_reference = is_reference;
            item->base.is_in_label = is_in_label;
        }
    }
    return item;
}

nanoem_mutable_model_label_item_t *APIENTRY
nanoemMutableModelLabelItemCreateAsReference(nanoem_model_label_item_t *item, nanoem_status_t *status)
{
    const nanoem_model_label_t *parent_label;
    nanoem_mutable_model_label_item_t *mutable_item = NULL;
    nanoem_bool_t is_in_label;
    if (nanoem_is_not_null(item)) {
        parent_label = item->base.parent.label;
        is_in_label = nanoemObjectArrayContainsIndexedObject((const void *const *) parent_label->items, item, parent_label->num_items, item->base.index);
        mutable_item = nanoemMutableModelLabelItemCreateInternal(item, nanoem_true, is_in_label, status);
    }
    return mutable_item;
}

nanoem_mutable_model_label_item_t *APIENTRY
nanoemMutableModelLabelItemCreateFromBoneObject(nanoem_mutable_model_label_t *label, nanoem_model_bone_t *bone, nanoem_status_t *status)
{
    nanoem_model_label_item_t *item = nanoemModelLabelItemCreate(nanoemMutableModelLabelGetOriginObject(label), status);
    if (item) {
        item->type = NANOEM_MODEL_LABEL_ITEM_TYPE_BONE;
        item->u.bone = bone;
        return nanoemMutableModelLabelItemCreateInternal(item, nanoem_false, nanoem_false, status);
    }
    return NULL;
}

nanoem_mutable_model_label_item_t *APIENTRY
nanoemMutableModelLabelItemCreateFromMorphObject(nanoem_mutable_model_label_t *label, nanoem_model_morph_t *morph, nanoem_status_t *status)
{
    nanoem_model_label_item_t *item = nanoemModelLabelItemCreate(nanoemMutableModelLabelGetOriginObject(label), status);
    if (item) {
        item->type = NANOEM_MODEL_LABEL_ITEM_TYPE_MORPH;
        item->u.morph = morph;
        return nanoemMutableModelLabelItemCreateInternal(item, nanoem_false, nanoem_false, status);
    }
    return NULL;
}

void APIENTRY
nanoemMutableModelLabelInsertItemObject(nanoem_mutable_model_label_t *label, nanoem_mutable_model_label_item_t *item, int index, nanoem_status_t *status)
{
    nanoem_model_label_t *origin_label;
    nanoem_model_label_item_t *origin_label_item;
    if (nanoem_is_not_null(label) && nanoem_is_not_null(item)) {
        origin_label = label->origin;
        origin_label_item = item->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_label->items, (nanoem_model_object_t *) origin_label_item, index, NANOEM_STATUS_ERROR_MODEL_LABEL_ALREADY_EXISTS, &origin_label->num_items, &label->num_allocated_items, status);
        if (!nanoem_status_ptr_has_error(status)) {
            origin_label_item->base.parent.label = origin_label;
            item->base.is_in_label = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelLabelRemoveItemObject(nanoem_mutable_model_label_t *label, nanoem_mutable_model_label_item_t *item, nanoem_status_t *status)
{
    nanoem_model_label_t *origin_label;
    nanoem_model_label_item_t *origin_label_item;
    if (nanoem_is_not_null(label) && nanoem_is_not_null(item)) {
        origin_label = label->origin;
        origin_label_item = item->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_label->items, (nanoem_model_object_t *) origin_label_item, &origin_label->num_items, NANOEM_STATUS_ERROR_MODEL_LABEL_ITEM_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            origin_label_item->base.parent.label = NULL;
            item->base.is_in_label = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

nanoem_model_label_item_t *APIENTRY
nanoemMutableModelLabelItemGetOriginObject(nanoem_mutable_model_label_item_t *label)
{
    return nanoem_is_not_null(label) ? label->origin : NULL;
}

void APIENTRY
nanoemMutableModelLabelItemDestroy(nanoem_mutable_model_label_item_t *item)
{
    if (nanoem_is_not_null(item)) {
        if (nanoemMutableBaseLabelItemObjectCanDelete(&item->base)) {
            nanoemModelLabelItemDestroy(item->origin);
        }
        nanoem_free(item);
    }
}

nanoem_mutable_model_rigid_body_t *APIENTRY
nanoemMutableModelRigidBodyCreate(nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_rigid_body_t *rigid_body = nanoemModelRigidBodyCreate(model, status);
    return nanoemMutableModelRigidBodyCreateInternal(rigid_body, nanoem_false, nanoem_false, status);
}

nanoem_mutable_model_rigid_body_t *APIENTRY
nanoemMutableModelRigidBodyCreateAsReference(nanoem_model_rigid_body_t *rigid_body, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    nanoem_mutable_model_rigid_body_t *mutable_rigid_body = NULL;
    nanoem_bool_t is_in_model;
    if (nanoem_is_not_null(rigid_body)) {
        origin_model = nanoemModelRigidBodyGetParentModel(rigid_body);
        if (nanoem_is_not_null(origin_model)) {
            is_in_model = nanoemObjectArrayContainsIndexedObject((const void *const *) origin_model->rigid_bodies, rigid_body, origin_model->num_rigid_bodies, rigid_body->base.index);
            mutable_rigid_body = nanoemMutableModelRigidBodyCreateInternal(rigid_body, nanoem_true, is_in_model, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return mutable_rigid_body;
}

void APIENTRY
nanoemMutableModelRigidBodySetName(nanoem_mutable_model_rigid_body_t *rigid_body, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(rigid_body)) {
        parent_model = nanoemModelRigidBodyGetParentModel(rigid_body->origin);
        if (nanoem_is_not_null(parent_model)) {
            factory = parent_model->factory;
            switch (language) {
            case NANOEM_LANGUAGE_TYPE_ENGLISH:
                nanoemUnicodeStringFactoryAssignString(factory, &rigid_body->origin->name_en, value, nanoemModelGetCodecType(parent_model), status);
                break;
            case NANOEM_LANGUAGE_TYPE_JAPANESE:
                nanoemUnicodeStringFactoryAssignString(factory, &rigid_body->origin->name_ja, value, nanoemModelGetCodecType(parent_model), status);
                break;
            case NANOEM_LANGUAGE_TYPE_MAX_ENUM:
            case NANOEM_LANGUAGE_TYPE_UNKNOWN:
            default:
                break;
            }
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelRigidBodySetBoneObject(nanoem_mutable_model_rigid_body_t *rigid_body, const nanoem_model_bone_t *value)
{
    const nanoem_model_t *parent_model;
    if (nanoem_is_not_null(rigid_body)) {
        parent_model = nanoemModelRigidBodyGetParentModel(rigid_body->origin);
        rigid_body->origin->bone_index = nanoemModelResolveBoneObject(parent_model, value);
    }
}

void APIENTRY
nanoemMutableModelRigidBodySetOrigin(nanoem_mutable_model_rigid_body_t *rigid_body, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(rigid_body) && nanoem_is_not_null(value) && rigid_body->origin->origin.values != value) {
        nanoem_crt_memcpy(rigid_body->origin->origin.values, value, sizeof(rigid_body->origin->origin));
    }
}

void APIENTRY
nanoemMutableModelRigidBodySetOrientation(nanoem_mutable_model_rigid_body_t *rigid_body, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(rigid_body) && nanoem_is_not_null(value) && rigid_body->origin->orientation.values != value) {
        nanoem_crt_memcpy(rigid_body->origin->orientation.values, value, sizeof(rigid_body->origin->orientation));
    }
}

void APIENTRY
nanoemMutableModelRigidBodySetShapeSize(nanoem_mutable_model_rigid_body_t *rigid_body, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(rigid_body) && nanoem_is_not_null(value) && rigid_body->origin->size.values != value) {
        nanoem_crt_memcpy(rigid_body->origin->size.values, value, sizeof(rigid_body->origin->size));
    }
}

void APIENTRY
nanoemMutableModelRigidBodySetMass(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_f32_t value)
{
    if (nanoem_is_not_null(rigid_body)) {
        rigid_body->origin->mass = value;
    }
}

void APIENTRY
nanoemMutableModelRigidBodySetLinearDamping(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_f32_t value)
{
    if (nanoem_is_not_null(rigid_body)) {
        rigid_body->origin->linear_damping = value;
    }
}

void APIENTRY
nanoemMutableModelRigidBodySetAngularDamping(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_f32_t value)
{
    if (nanoem_is_not_null(rigid_body)) {
        rigid_body->origin->angular_damping = value;
    }
}

void APIENTRY
nanoemMutableModelRigidBodySetFriction(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_f32_t value)
{
    if (nanoem_is_not_null(rigid_body)) {
        rigid_body->origin->friction = value;
    }
}

void APIENTRY
nanoemMutableModelRigidBodySetRestitution(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_f32_t value)
{
    if (nanoem_is_not_null(rigid_body)) {
        rigid_body->origin->restitution = value;
    }
}

void APIENTRY
nanoemMutableModelRigidBodySetShapeType(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_model_rigid_body_shape_type_t value)
{
    if (nanoem_is_not_null(rigid_body)) {
        rigid_body->origin->shape_type = value;
    }
}

void APIENTRY
nanoemMutableModelRigidBodySetTransformType(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_model_rigid_body_transform_type_t value)
{
    if (nanoem_is_not_null(rigid_body)) {
        rigid_body->origin->transform_type = value;
    }
}

void APIENTRY
nanoemMutableModelRigidBodySetCollisionGroupId(nanoem_mutable_model_rigid_body_t *rigid_body, int value)
{
    if (nanoem_is_not_null(rigid_body)) {
        rigid_body->origin->collision_group_id = value;
    }
}

void APIENTRY
nanoemMutableModelRigidBodySetCollisionMask(nanoem_mutable_model_rigid_body_t *rigid_body, int value)
{
    if (nanoem_is_not_null(rigid_body)) {
        rigid_body->origin->collision_mask = value;
    }
}

void APIENTRY
nanoemMutableModelRigidBodyCopy(nanoem_mutable_model_rigid_body_t *rigid_body, const nanoem_model_rigid_body_t *value, nanoem_status_t *status)
{
    nanoemMutableModelRigidBodySetAngularDamping(rigid_body, nanoemModelRigidBodyGetAngularDamping(value));
    nanoemMutableModelRigidBodySetBoneObject(rigid_body, nanoemModelRigidBodyGetBoneObject(value));
    nanoemMutableModelRigidBodySetCollisionGroupId(rigid_body, nanoemModelRigidBodyGetCollisionGroupId(value));
    nanoemMutableModelRigidBodySetCollisionMask(rigid_body, nanoemModelRigidBodyGetCollisionMask(value));
    nanoemMutableModelRigidBodySetFriction(rigid_body, nanoemModelRigidBodyGetFriction(value));
    nanoemMutableModelRigidBodySetLinearDamping(rigid_body, nanoemModelRigidBodyGetLinearDamping(value));
    nanoemMutableModelRigidBodySetName(rigid_body, nanoemModelRigidBodyGetName(value, NANOEM_LANGUAGE_TYPE_JAPANESE), NANOEM_LANGUAGE_TYPE_JAPANESE, status);
    nanoemMutableModelRigidBodySetName(rigid_body, nanoemModelRigidBodyGetName(value, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH, status);
    nanoemMutableModelRigidBodySetMass(rigid_body, nanoemModelRigidBodyGetMass(value));
    nanoemMutableModelRigidBodySetTransformType(rigid_body, nanoemModelRigidBodyGetTransformType(value));
    nanoemMutableModelRigidBodySetOrientation(rigid_body, nanoemModelRigidBodyGetOrientation(value));
    nanoemMutableModelRigidBodySetOrigin(rigid_body, nanoemModelRigidBodyGetOrigin(value));
    nanoemMutableModelRigidBodySetRestitution(rigid_body, nanoemModelRigidBodyGetRestitution(value));
    nanoemMutableModelRigidBodySetShapeSize(rigid_body, nanoemModelRigidBodyGetShapeSize(value));
    nanoemMutableModelRigidBodySetShapeType(rigid_body, nanoemModelRigidBodyGetShapeType(value));
}

nanoem_model_rigid_body_t *APIENTRY
nanoemMutableModelRigidBodyGetOriginObject(nanoem_mutable_model_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? rigid_body->origin : NULL;
}

void APIENTRY
nanoemMutableModelRigidBodySaveToBuffer(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_rigid_body_t *origin;
    const nanoem_model_t *parent_model;
    nanoem_u8_t name_buffer[PMD_RIGID_BODY_NAME_LENGTH + 1];
    nanoem_unicode_string_factory_t *factory;
    nanoem_codec_type_t codec;
    if (nanoem_is_not_null(rigid_body) && nanoem_is_not_null(buffer)) {
        origin = rigid_body->origin;
        parent_model = nanoemModelRigidBodyGetParentModel(origin);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
            return;
        }
        factory = parent_model->factory;
        if (nanoemModelIsPMX(parent_model)) {
            codec = nanoemModelGetCodecType(parent_model);
            nanoemMutableBufferWriteUnicodeString(buffer, origin->name_ja, factory, codec, status);
            nanoemMutableBufferWriteUnicodeString(buffer, origin->name_en, factory, codec, status);
            nanoemMutableBufferWriteInteger(buffer, origin->bone_index, parent_model->info.bone_index_size, status);
        }
        else {
            nanoemMutableBufferWriteCp932String(buffer, origin->name_ja, factory, name_buffer, sizeof(name_buffer), status);
            nanoemMutableBufferWriteInt16LittleEndian(buffer, origin->bone_index, status);
        }
        nanoemMutableBufferWriteByte(buffer, origin->collision_group_id, status);
        nanoemMutableBufferWriteInt16LittleEndianUnsigned(buffer, origin->collision_mask, status);
        nanoemMutableBufferWriteByte(buffer, origin->shape_type, status);
        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->size, status);
        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->origin, status);
        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->orientation, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->mass, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->linear_damping, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->angular_damping, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->restitution, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->friction, status);
        nanoemMutableBufferWriteByte(buffer, origin->transform_type, status);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelRigidBodyDestroy(nanoem_mutable_model_rigid_body_t *rigid_body)
{
    if (nanoem_is_not_null(rigid_body)) {
        if (nanoemMutableBaseModelObjectCanDelete(&rigid_body->base)) {
            nanoemModelRigidBodyDestroy(rigid_body->origin);
        }
        nanoem_free(rigid_body);
    }
}

nanoem_mutable_model_joint_t *APIENTRY
nanoemMutableModelJointCreate(nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_joint_t *joint = nanoemModelJointCreate(model ,status);
    return nanoemMutableModelJointCreateInternal(joint, nanoem_false, nanoem_false, status);
}

nanoem_mutable_model_joint_t *APIENTRY
nanoemMutableModelJointCreateAsReference(nanoem_model_joint_t *joint, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    nanoem_mutable_model_joint_t *mutable_joint = NULL;
    nanoem_bool_t is_in_model;
    if (nanoem_is_not_null(joint)) {
        origin_model = nanoemModelJointGetParentModel(joint);
        if (nanoem_is_not_null(origin_model)) {
            is_in_model = nanoemObjectArrayContainsIndexedObject((const void *const *) origin_model->joints, joint, origin_model->num_joints, joint->base.index);
            mutable_joint = nanoemMutableModelJointCreateInternal(joint, nanoem_true, is_in_model, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return mutable_joint;
}

void APIENTRY
nanoemMutableModelJointSetName(nanoem_mutable_model_joint_t *joint, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(joint)) {
        parent_model = nanoemModelJointGetParentModel(joint->origin);
        if (nanoem_is_not_null(parent_model)) {
            factory = parent_model->factory;
            switch (language) {
            case NANOEM_LANGUAGE_TYPE_ENGLISH:
                nanoemUnicodeStringFactoryAssignString(factory, &joint->origin->name_en, value, nanoemModelGetCodecType(parent_model), status);
                break;
            case NANOEM_LANGUAGE_TYPE_JAPANESE:
                nanoemUnicodeStringFactoryAssignString(factory, &joint->origin->name_ja, value, nanoemModelGetCodecType(parent_model), status);
                break;
            case NANOEM_LANGUAGE_TYPE_MAX_ENUM:
            case NANOEM_LANGUAGE_TYPE_UNKNOWN:
            default:
                break;
            }
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelJointSetRigidBodyAObject(nanoem_mutable_model_joint_t *joint, const nanoem_model_rigid_body_t *value)
{
    const nanoem_model_t *parent_model;
    if (nanoem_is_not_null(joint)) {
        parent_model = nanoemModelJointGetParentModel(joint->origin);
        joint->origin->rigid_body_a_index = nanoemModelResolveRigidBodyObject(parent_model, value);
    }
}

void APIENTRY
nanoemMutableModelJointSetRigidBodyBObject(nanoem_mutable_model_joint_t *joint, const nanoem_model_rigid_body_t *value)
{
    const nanoem_model_t *parent_model;
    if (nanoem_is_not_null(joint)) {
        parent_model = nanoemModelJointGetParentModel(joint->origin);
        joint->origin->rigid_body_b_index = nanoemModelResolveRigidBodyObject(parent_model, value);
    }
}

void APIENTRY
nanoemMutableModelJointSetOrigin(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(joint) && nanoem_is_not_null(value) && joint->origin->origin.values != value) {
        nanoem_crt_memcpy(joint->origin->origin.values, value, sizeof(joint->origin->origin));
    }
}

void APIENTRY
nanoemMutableModelJointSetOrientation(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(joint) && nanoem_is_not_null(value) && joint->origin->orientation.values != value) {
        nanoem_crt_memcpy(joint->origin->orientation.values, value, sizeof(joint->origin->orientation));
    }
}

void APIENTRY
nanoemMutableModelJointSetType(nanoem_mutable_model_joint_t *joint, nanoem_model_joint_type_t value)
{
    if (nanoem_is_not_null(joint)) {
        joint->origin->type = value;
    }
}

void APIENTRY
nanoemMutableModelJointSetLinearUpperLimit(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(joint) && nanoem_is_not_null(value) && joint->origin->linear_upper_limit.values != value) {
        nanoem_crt_memcpy(joint->origin->linear_upper_limit.values, value, sizeof(joint->origin->linear_upper_limit));
    }
}

void APIENTRY
nanoemMutableModelJointSetLinearLowerLimit(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(joint) && nanoem_is_not_null(value) && joint->origin->linear_lower_limit.values != value) {
        nanoem_crt_memcpy(joint->origin->linear_lower_limit.values, value, sizeof(joint->origin->linear_lower_limit));
    }
}

void APIENTRY
nanoemMutableModelJointSetLinearStiffness(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(joint) && nanoem_is_not_null(value) && joint->origin->linear_stiffness.values != value) {
        nanoem_crt_memcpy(joint->origin->linear_stiffness.values, value, sizeof(joint->origin->linear_stiffness));
    }
}

void APIENTRY
nanoemMutableModelJointSetAngularUpperLimit(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(joint) && nanoem_is_not_null(value) && joint->origin->angular_upper_limit.values != value) {
        nanoem_crt_memcpy(joint->origin->angular_upper_limit.values, value, sizeof(joint->origin->angular_upper_limit));
    }
}

void APIENTRY
nanoemMutableModelJointSetAngularLowerLimit(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(joint) && nanoem_is_not_null(value) && joint->origin->angular_lower_limit.values != value) {
        nanoem_crt_memcpy(joint->origin->angular_lower_limit.values, value, sizeof(joint->origin->angular_lower_limit));
    }
}

void APIENTRY
nanoemMutableModelJointSetAngularStiffness(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(joint) && nanoem_is_not_null(value) && joint->origin->angular_stiffness.values != value) {
        nanoem_crt_memcpy(joint->origin->angular_stiffness.values, value, sizeof(joint->origin->angular_stiffness));
    }
}

void APIENTRY
nanoemMutableModelJointCopy(nanoem_mutable_model_joint_t *joint, const nanoem_model_joint_t *value, nanoem_status_t *status)
{
    nanoemMutableModelJointSetAngularLowerLimit(joint, nanoemModelJointGetAngularLowerLimit(value));
    nanoemMutableModelJointSetAngularStiffness(joint, nanoemModelJointGetAngularStiffness(value));
    nanoemMutableModelJointSetAngularUpperLimit(joint, nanoemModelJointGetAngularUpperLimit(value));
    nanoemMutableModelJointSetLinearLowerLimit(joint, nanoemModelJointGetLinearLowerLimit(value));
    nanoemMutableModelJointSetLinearStiffness(joint, nanoemModelJointGetLinearStiffness(value));
    nanoemMutableModelJointSetLinearUpperLimit(joint, nanoemModelJointGetLinearUpperLimit(value));
    nanoemMutableModelJointSetName(joint, nanoemModelJointGetName(value, NANOEM_LANGUAGE_TYPE_JAPANESE), NANOEM_LANGUAGE_TYPE_JAPANESE, status);
    nanoemMutableModelJointSetName(joint, nanoemModelJointGetName(value, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH, status);
    nanoemMutableModelJointSetOrientation(joint, nanoemModelJointGetOrientation(value));
    nanoemMutableModelJointSetOrigin(joint, nanoemModelJointGetOrigin(value));
    nanoemMutableModelJointSetRigidBodyAObject(joint, nanoemModelJointGetRigidBodyAObject(value));
    nanoemMutableModelJointSetRigidBodyBObject(joint, nanoemModelJointGetRigidBodyBObject(value));
    nanoemMutableModelJointSetType(joint, nanoemModelJointGetType(value));
}

nanoem_model_joint_t *APIENTRY
nanoemMutableModelJointGetOriginObject(nanoem_mutable_model_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? joint->origin : NULL;
}

void APIENTRY
nanoemMutableModelJointSaveToBuffer(nanoem_mutable_model_joint_t *joint, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_joint_t *origin;
    const nanoem_model_t *parent_model;
    nanoem_u8_t name_buffer[PMD_JOINT_NAME_LENGTH + 1];
    nanoem_unicode_string_factory_t *factory;
    nanoem_codec_type_t codec;
    nanoem_rsize_t size;
    if (nanoem_is_not_null(joint) && nanoem_is_not_null(buffer)) {
        origin = joint->origin;
        parent_model = nanoemModelJointGetParentModel(origin);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
            return;
        }
        factory = parent_model->factory;
        if (nanoemModelIsPMX(parent_model)) {
            codec = nanoemModelGetCodecType(parent_model);
            size = parent_model->info.rigid_body_index_size;
            nanoemMutableBufferWriteUnicodeString(buffer, origin->name_ja, factory, codec, status);
            nanoemMutableBufferWriteUnicodeString(buffer, origin->name_en, factory, codec, status);
            nanoemMutableBufferWriteByte(buffer, origin->type, status);
            nanoemMutableBufferWriteInteger(buffer, origin->rigid_body_a_index, size, status);
            nanoemMutableBufferWriteInteger(buffer, origin->rigid_body_b_index, size, status);
        }
        else {
            nanoemMutableBufferWriteCp932String(buffer, origin->name_ja, factory, name_buffer, sizeof(name_buffer), status);
            nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->rigid_body_a_index, status);
            nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->rigid_body_b_index, status);
        }
        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->origin, status);
        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->orientation, status);
        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->linear_lower_limit, status);
        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->linear_upper_limit, status);
        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->angular_lower_limit, status);
        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->angular_upper_limit, status);
        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->linear_stiffness, status);
        nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->angular_stiffness, status);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelJointDestroy(nanoem_mutable_model_joint_t *joint)
{
    if (nanoem_is_not_null(joint)) {
        if (nanoemMutableBaseModelObjectCanDelete(&joint->base)) {
            nanoemModelJointDestroy(joint->origin);
        }
        nanoem_free(joint);
    }
}

nanoem_mutable_model_soft_body_t *APIENTRY
nanoemMutableModelSoftBodyCreate(nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_model_soft_body_t *soft_body = nanoemModelSoftBodyCreate(model ,status);
    return nanoemMutableModelSoftBodyCreateInternal(soft_body, nanoem_false, nanoem_false, status);
}

nanoem_mutable_model_soft_body_t *APIENTRY
nanoemMutableModelSoftBodyCreateAsReference(nanoem_model_soft_body_t *soft_body, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    nanoem_mutable_model_soft_body_t *mutable_soft_body = NULL;
    nanoem_bool_t is_in_model;
    if (nanoem_is_not_null(soft_body)) {
        origin_model = nanoemModelSoftBodyGetParentModel(soft_body);
        if (nanoem_is_not_null(origin_model)) {
            is_in_model = nanoemObjectArrayContainsIndexedObject((const void *const *) origin_model->soft_bodies, soft_body, origin_model->num_soft_bodies, soft_body->base.index);
            mutable_soft_body = nanoemMutableModelSoftBodyCreateInternal(soft_body, nanoem_true, is_in_model, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return mutable_soft_body;
}

void APIENTRY
nanoemMutableModelSoftBodySetName(nanoem_mutable_model_soft_body_t *soft_body, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(soft_body)) {
        parent_model = nanoemModelSoftBodyGetParentModel(soft_body->origin);
        if (nanoem_is_not_null(parent_model)) {
            factory = parent_model->factory;
            switch (language) {
            case NANOEM_LANGUAGE_TYPE_ENGLISH:
                nanoemUnicodeStringFactoryAssignString(factory, &soft_body->origin->name_en, value, nanoemModelGetCodecType(parent_model), status);
                break;
            case NANOEM_LANGUAGE_TYPE_JAPANESE:
                nanoemUnicodeStringFactoryAssignString(factory, &soft_body->origin->name_ja, value, nanoemModelGetCodecType(parent_model), status);
                break;
            case NANOEM_LANGUAGE_TYPE_MAX_ENUM:
            case NANOEM_LANGUAGE_TYPE_UNKNOWN:
            default:
                break;
            }
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

 void APIENTRY
nanoemMutableModelSoftBodySetMaterialObject(nanoem_mutable_model_soft_body_t *soft_body, const nanoem_model_material_t *value)
{
     const nanoem_model_t *parent_model;
     if (nanoem_is_not_null(soft_body)) {
         parent_model = nanoemModelSoftBodyGetParentModel(soft_body->origin);
         soft_body->origin->material_index = nanoemModelResolveMaterialObject(parent_model, value);
     }
}

void APIENTRY
nanoemMutableModelSoftBodySetAllPinnedVertexIndices(nanoem_mutable_model_soft_body_t *soft_body, const nanoem_u32_t *value, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_model_soft_body_t *origin;
    nanoem_rsize_t allocate_size;
    if (nanoem_is_not_null(soft_body) && nanoem_is_not_null(value) && length > 0) {
        origin = soft_body->origin;
        allocate_size = length * sizeof(*origin->pinned_vertex_indices);
        if (length >= soft_body->num_allocated_pin_vertex_indices) {
            origin->pinned_vertex_indices = (nanoem_u32_t *) nanoem_realloc(origin->pinned_vertex_indices, allocate_size, status);
        }
        if (nanoem_is_not_null(origin->pinned_vertex_indices)) {
            nanoem_crt_memcpy(origin->pinned_vertex_indices, value, allocate_size);
            soft_body->num_allocated_pin_vertex_indices = origin->num_pinned_vertex_indices = length;
            nanoem_status_ptr_assign_succeeded(status);
        }
        else {
            soft_body->num_allocated_pin_vertex_indices = origin->num_pinned_vertex_indices = 0;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

 void APIENTRY
nanoemMutableModelSoftBodySetShapeType(nanoem_mutable_model_soft_body_t *soft_body, nanoem_model_soft_body_shape_type_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->shape_type = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetAeroModel(nanoem_mutable_model_soft_body_t *soft_body, nanoem_model_soft_body_aero_model_type_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->aero_model = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetTotalMass(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->total_mass = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetCollisionMargin(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->collision_margin = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetVelocityCorrectionFactor(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->velocity_correction_factor = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetDampingCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->damping_coefficient = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetDragCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->drag_coefficient = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetLiftCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->lift_coefficient = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetPressureCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->pressure_coefficient = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetVolumeConversationCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->volume_conversation_coefficient = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetDynamicFrictionCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->dynamic_friction_coefficient = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetPoseMatchingCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->pose_matching_coefficient = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetRigidContactHardness(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->rigid_contact_hardness = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetKineticContactHardness(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->kinetic_contact_hardness = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetSoftContactHardness(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->soft_contact_hardness = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetAnchorHardness(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->anchor_hardness = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetSoftVSRigidHardness(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->soft_vs_rigid_hardness = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetSoftVSKineticHardness(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->soft_vs_kinetic_hardness = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetSoftVSSoftHardness(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->soft_vs_soft_hardness = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetSoftVSRigidImpulseSplit(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->soft_vs_rigid_impulse_split = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetSoftVSKineticImpulseSplit(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->soft_vs_kinetic_impulse_split = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetSoftVSSoftImpulseSplit(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->soft_vs_soft_impulse_split = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetLinearStiffnessCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->linear_stiffness_coefficient = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetAngularStiffnessCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->angular_stiffness_coefficient = value;
     }
}

 void APIENTRY
nanoemMutableModelSoftBodySetVolumeStiffnessCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value)
{
     if (nanoem_is_not_null(soft_body)) {
         soft_body->origin->volume_stiffness_coefficient = value;
     }
}

void APIENTRY
nanoemMutableModelSoftBodySetCollisionGroupId(nanoem_mutable_model_soft_body_t *soft_body, int value)
{
    if (nanoem_is_not_null(soft_body)) {
        soft_body->origin->collision_group_id = value;
    }
}

void APIENTRY
nanoemMutableModelSoftBodySetCollisionMask(nanoem_mutable_model_soft_body_t *soft_body, int value)
{
    if (nanoem_is_not_null(soft_body)) {
        soft_body->origin->collision_mask = value;
    }
}

void APIENTRY
nanoemMutableModelSoftBodySetBendingConstraintsDistance(nanoem_mutable_model_soft_body_t *soft_body, int value)
{
    if (nanoem_is_not_null(soft_body)) {
        soft_body->origin->bending_constraints_distance = value;
    }
}

void APIENTRY
nanoemMutableModelSoftBodySetClusterCount(nanoem_mutable_model_soft_body_t *soft_body, int value)
{
    if (nanoem_is_not_null(soft_body)) {
        soft_body->origin->cluster_count = value;
    }
}

void APIENTRY
nanoemMutableModelSoftBodySetVelocitySolverIterations(nanoem_mutable_model_soft_body_t *soft_body, int value)
{
    if (nanoem_is_not_null(soft_body)) {
        soft_body->origin->velocity_solver_iterations = value;
    }
}

void APIENTRY
nanoemMutableModelSoftBodySetPositionsSolverIterations(nanoem_mutable_model_soft_body_t *soft_body, int value)
{
    if (nanoem_is_not_null(soft_body)) {
        soft_body->origin->positions_solver_iterations = value;
    }
}

void APIENTRY
nanoemMutableModelSoftBodySetDriftSolverIterations(nanoem_mutable_model_soft_body_t *soft_body, int value)
{
    if (nanoem_is_not_null(soft_body)) {
        soft_body->origin->drift_solver_iterations = value;
    }
}

void APIENTRY
nanoemMutableModelSoftBodySetClusterSolverIterations(nanoem_mutable_model_soft_body_t *soft_body, int value)
{
    if (nanoem_is_not_null(soft_body)) {
        soft_body->origin->cluster_solver_iterations = value;
    }
}

void APIENTRY
nanoemMutableModelSoftBodySetBendingConstraintsEnabled(nanoem_mutable_model_soft_body_t *soft_body, nanoem_bool_t value)
{
    if (nanoem_is_not_null(soft_body)) {
        soft_body->origin->flags = value ? (soft_body->origin->flags | 0x1) : (soft_body->origin->flags & ~0x1);
    }
}

void APIENTRY
nanoemMutableModelSoftBodySetClustersEnabled(nanoem_mutable_model_soft_body_t *soft_body, nanoem_bool_t value)
{
    if (nanoem_is_not_null(soft_body)) {
        soft_body->origin->flags = value ? (soft_body->origin->flags | 0x2) : (soft_body->origin->flags & ~0x2);
    }
}

void APIENTRY
nanoemMutableModelSoftBodySetRandomizeConstraintsNeeded(nanoem_mutable_model_soft_body_t *soft_body, nanoem_bool_t value)
{
    if (nanoem_is_not_null(soft_body)) {
        soft_body->origin->flags = value ? (soft_body->origin->flags | 0x4) : (soft_body->origin->flags & ~0x4);
    }
}

void APIENTRY
nanoemMutableModelSoftBodyInsertAnchorObject(nanoem_mutable_model_soft_body_t *soft_body, nanoem_mutable_model_soft_body_anchor_t *anchor, int index, nanoem_status_t *status)
{
    nanoem_model_soft_body_t *origin_soft_body;
    nanoem_model_soft_body_anchor_t *origin_soft_body_anchor;
    if (nanoem_is_not_null(soft_body) && nanoem_is_not_null(anchor)) {
        origin_soft_body = soft_body->origin;
        origin_soft_body_anchor = anchor->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_soft_body->anchors, (nanoem_model_object_t *) origin_soft_body_anchor, index, NANOEM_STATUS_ERROR_MODEL_SOFT_BODY_ANCHOR_ALREADY_EXISTS, &origin_soft_body->num_anchors, &soft_body->num_allocated_anchors, status);
        if (!nanoem_status_ptr_has_error(status)) {
            origin_soft_body_anchor->base.parent.soft_body = origin_soft_body;
            anchor->base.is_in_soft_body = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelSoftBodyRemoveAnchorObject(nanoem_mutable_model_soft_body_t *soft_body, nanoem_mutable_model_soft_body_anchor_t *anchor, nanoem_status_t *status)
{
    nanoem_model_soft_body_t *origin_soft_body;
    nanoem_model_soft_body_anchor_t *origin_soft_body_anchor;
    if (nanoem_is_not_null(soft_body) && nanoem_is_not_null(anchor)) {
        origin_soft_body = soft_body->origin;
        origin_soft_body_anchor = anchor->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_soft_body->anchors, (nanoem_model_object_t *) origin_soft_body_anchor, &origin_soft_body->num_anchors, NANOEM_STATUS_ERROR_MODEL_SOFT_BODY_ANCHOR_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            origin_soft_body_anchor->base.parent.soft_body = NULL;
            anchor->base.is_in_soft_body = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelSoftBodyCopy(nanoem_mutable_model_soft_body_t *soft_body, const nanoem_model_soft_body_t *value, nanoem_status_t *status)
{
    nanoem_model_soft_body_anchor_t *const *anchors, *anchor;
    const nanoem_u32_t *vertex_indices;
    nanoem_mutable_model_soft_body_anchor_t *new_anchor;
    nanoem_rsize_t num_objects, i;
    vertex_indices = nanoemModelSoftBodyGetAllPinnedVertexIndices(value, &num_objects);
    nanoemMutableModelSoftBodySetName(soft_body, nanoemModelSoftBodyGetName(value, NANOEM_LANGUAGE_TYPE_JAPANESE), NANOEM_LANGUAGE_TYPE_JAPANESE, status);
    nanoemMutableModelSoftBodySetName(soft_body, nanoemModelSoftBodyGetName(value, NANOEM_LANGUAGE_TYPE_ENGLISH), NANOEM_LANGUAGE_TYPE_ENGLISH, status);
    nanoemMutableModelSoftBodySetMaterialObject(soft_body, nanoemModelSoftBodyGetMaterialObject(value));
    nanoemMutableModelSoftBodySetAllPinnedVertexIndices(soft_body, vertex_indices, num_objects, status);
    nanoemMutableModelSoftBodySetShapeType(soft_body, nanoemModelSoftBodyGetShapeType(value));
    nanoemMutableModelSoftBodySetAeroModel(soft_body, nanoemModelSoftBodyGetAeroModel(value));
    nanoemMutableModelSoftBodySetTotalMass(soft_body, nanoemModelSoftBodyGetTotalMass(value));
    nanoemMutableModelSoftBodySetCollisionMargin(soft_body, nanoemModelSoftBodyGetCollisionMargin(value));
    nanoemMutableModelSoftBodySetVelocityCorrectionFactor(soft_body, nanoemModelSoftBodyGetVelocityCorrectionFactor(value));
    nanoemMutableModelSoftBodySetDampingCoefficient(soft_body, nanoemModelSoftBodyGetDampingCoefficient(value));
    nanoemMutableModelSoftBodySetDragCoefficient(soft_body, nanoemModelSoftBodyGetDragCoefficient(value));
    nanoemMutableModelSoftBodySetLiftCoefficient(soft_body, nanoemModelSoftBodyGetLiftCoefficient(value));
    nanoemMutableModelSoftBodySetPressureCoefficient(soft_body, nanoemModelSoftBodyGetPressureCoefficient(value));
    nanoemMutableModelSoftBodySetVolumeConversationCoefficient(soft_body, nanoemModelSoftBodyGetVolumeConversationCoefficient(value));
    nanoemMutableModelSoftBodySetDynamicFrictionCoefficient(soft_body, nanoemModelSoftBodyGetDynamicFrictionCoefficient(value));
    nanoemMutableModelSoftBodySetPoseMatchingCoefficient(soft_body, nanoemModelSoftBodyGetPoseMatchingCoefficient(value));
    nanoemMutableModelSoftBodySetRigidContactHardness(soft_body, nanoemModelSoftBodyGetRigidContactHardness(value));
    nanoemMutableModelSoftBodySetKineticContactHardness(soft_body, nanoemModelSoftBodyGetKineticContactHardness(value));
    nanoemMutableModelSoftBodySetSoftContactHardness(soft_body, nanoemModelSoftBodyGetSoftContactHardness(value));
    nanoemMutableModelSoftBodySetAnchorHardness(soft_body, nanoemModelSoftBodyGetAnchorHardness(value));
    nanoemMutableModelSoftBodySetSoftVSRigidHardness(soft_body, nanoemModelSoftBodyGetSoftVSRigidHardness(value));
    nanoemMutableModelSoftBodySetSoftVSKineticHardness(soft_body, nanoemModelSoftBodyGetSoftVSKineticHardness(value));
    nanoemMutableModelSoftBodySetSoftVSSoftHardness(soft_body, nanoemModelSoftBodyGetSoftVSSoftHardness(value));
    nanoemMutableModelSoftBodySetSoftVSRigidImpulseSplit(soft_body, nanoemModelSoftBodyGetSoftVSRigidImpulseSplit(value));
    nanoemMutableModelSoftBodySetSoftVSKineticImpulseSplit(soft_body, nanoemModelSoftBodyGetSoftVSKineticImpulseSplit(value));
    nanoemMutableModelSoftBodySetSoftVSSoftImpulseSplit(soft_body, nanoemModelSoftBodyGetSoftVSSoftImpulseSplit(value));
    nanoemMutableModelSoftBodySetLinearStiffnessCoefficient(soft_body, nanoemModelSoftBodyGetLinearStiffnessCoefficient(value));
    nanoemMutableModelSoftBodySetAngularStiffnessCoefficient(soft_body, nanoemModelSoftBodyGetAngularStiffnessCoefficient(value));
    nanoemMutableModelSoftBodySetVolumeStiffnessCoefficient(soft_body, nanoemModelSoftBodyGetVolumeStiffnessCoefficient(value));
    nanoemMutableModelSoftBodySetCollisionGroupId(soft_body, nanoemModelSoftBodyGetCollisionGroupId(value));
    nanoemMutableModelSoftBodySetCollisionMask(soft_body, nanoemModelSoftBodyGetCollisionMask(value));
    nanoemMutableModelSoftBodySetBendingConstraintsDistance(soft_body, nanoemModelSoftBodyGetBendingConstraintsDistance(value));
    nanoemMutableModelSoftBodySetClusterCount(soft_body, nanoemModelSoftBodyGetClusterCount(value));
    nanoemMutableModelSoftBodySetVelocitySolverIterations(soft_body, nanoemModelSoftBodyGetVelocitySolverIterations(value));
    nanoemMutableModelSoftBodySetPositionsSolverIterations(soft_body, nanoemModelSoftBodyGetPositionsSolverIterations(value));
    nanoemMutableModelSoftBodySetDriftSolverIterations(soft_body, nanoemModelSoftBodyGetDriftSolverIterations(value));
    nanoemMutableModelSoftBodySetClusterSolverIterations(soft_body, nanoemModelSoftBodyGetClusterSolverIterations(value));
    nanoemMutableModelSoftBodySetBendingConstraintsEnabled(soft_body, nanoemModelSoftBodyIsBendingConstraintsEnabled(value));
    nanoemMutableModelSoftBodySetClustersEnabled(soft_body, nanoemModelSoftBodyIsClustersEnabled(value));
    nanoemMutableModelSoftBodySetRandomizeConstraintsNeeded(soft_body, nanoemModelSoftBodyIsRandomizeConstraintsNeeded(value));
    anchors = nanoemModelSoftBodyGetAllAnchorObjects(value, &num_objects);
    for (i = 0; i < num_objects; i++) {
        anchor = anchors[i];
        new_anchor = nanoemMutableModelSoftBodyAnchorCreate(soft_body, status);
        nanoemMutableModelSoftBodyAnchorSetRigidBodyObject(new_anchor, nanoemModelSoftBodyAnchorGetRigidBodyObject(anchor));
        nanoemMutableModelSoftBodyAnchorSetVertexObject(new_anchor, nanoemModelSoftBodyAnchorGetVertexObject(anchor));
        nanoemMutableModelSoftBodyAnchorSetNearEnabled(new_anchor, nanoemModelSoftBodyAnchorIsNearEnabled(anchor));
        nanoemMutableModelSoftBodyInsertAnchorObject(soft_body, new_anchor, -1, status);
        nanoemMutableModelSoftBodyAnchorDestroy(new_anchor);
    }
}

nanoem_model_soft_body_t *APIENTRY
nanoemMutableModelSoftBodyGetOriginObject(nanoem_mutable_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->origin : NULL;
}

void APIENTRY
nanoemMutableModelSoftBodySaveToBuffer(nanoem_mutable_model_soft_body_t *soft_body, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *parent_model;
    nanoem_unicode_string_factory_t *factory;
    nanoem_model_soft_body_t *origin;
    nanoem_model_soft_body_anchor_t *anchor;
    nanoem_codec_type_t codec;
    nanoem_rsize_t material_index_size, rigid_body_index_size, vertex_index_size, num_anchors, num_pin_vertex_indices, i;
    if (nanoem_is_not_null(soft_body) && nanoem_is_not_null(buffer)) {
        origin = soft_body->origin;
        parent_model = nanoemModelSoftBodyGetParentModel(origin);
        if (nanoem_is_null(parent_model)) {
            nanoem_status_ptr_assign_null_object(status);
            return;
        }
        factory = parent_model->factory;
        material_index_size = parent_model->info.material_index_size;
        rigid_body_index_size = parent_model->info.rigid_body_index_size;
        vertex_index_size = parent_model->info.vertex_index_size;
        codec = nanoemModelGetCodecType(parent_model);
        nanoemMutableBufferWriteUnicodeString(buffer, origin->name_ja, factory, codec, status);
        nanoemMutableBufferWriteUnicodeString(buffer, origin->name_en, factory, codec, status);
        nanoemMutableBufferWriteByte(buffer, origin->shape_type, status);
        nanoemMutableBufferWriteInteger(buffer, origin->material_index, material_index_size, status);
        nanoemMutableBufferWriteByte(buffer, origin->collision_group_id, status);
        nanoemMutableBufferWriteInt16LittleEndianUnsigned(buffer, origin->collision_mask, status);
        nanoemMutableBufferWriteByte(buffer, origin->flags, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->bending_constraints_distance, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->cluster_count, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->total_mass, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->collision_margin, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->aero_model, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->velocity_correction_factor, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->damping_coefficient, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->drag_coefficient, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->lift_coefficient, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->pressure_coefficient, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->volume_conversation_coefficient, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->dynamic_friction_coefficient, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->pose_matching_coefficient, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->rigid_contact_hardness, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->kinetic_contact_hardness, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->soft_contact_hardness, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->anchor_hardness, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->soft_vs_rigid_hardness, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->soft_vs_kinetic_hardness, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->soft_vs_soft_hardness, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->soft_vs_rigid_impulse_split, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->soft_vs_kinetic_impulse_split, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->soft_vs_soft_impulse_split, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->velocity_solver_iterations, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->positions_solver_iterations, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->drift_solver_iterations, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->cluster_solver_iterations, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->linear_stiffness_coefficient, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->angular_stiffness_coefficient, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->volume_stiffness_coefficient, status);
        num_anchors = origin->num_anchors;
        nanoemMutableBufferWriteInt32LittleEndian(buffer, num_anchors, status);
        if (num_anchors > 0) {
            for (i = 0; i < num_anchors; i++) {
                anchor = origin->anchors[i];
                nanoemMutableBufferWriteInteger(buffer, anchor->rigid_body_index, rigid_body_index_size, status);
                nanoemMutableBufferWriteInteger(buffer, anchor->vertex_index, rigid_body_index_size, status);
                nanoemMutableBufferWriteByte(buffer, anchor->is_near_enabled, status);
            }
        }
        num_pin_vertex_indices = origin->num_pinned_vertex_indices;
        nanoemMutableBufferWriteInt32LittleEndian(buffer, num_pin_vertex_indices, status);
        if (num_pin_vertex_indices > 0) {
            for (i = 0; i < num_pin_vertex_indices; i++) {
                nanoemMutableBufferWriteInteger(buffer, origin->pinned_vertex_indices[i], vertex_index_size, status);
            }
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelSoftBodyDestroy(nanoem_mutable_model_soft_body_t *soft_body)
{
    if (nanoem_is_not_null(soft_body)) {
        if (nanoemMutableBaseModelObjectCanDelete(&soft_body->base)) {
            nanoemModelSoftBodyDestroy(soft_body->origin);
        }
        nanoem_free(soft_body);
    }
}

nanoem_mutable_model_soft_body_anchor_t *APIENTRY
nanoemMutableModelSoftBodyAnchorCreate(nanoem_mutable_model_soft_body_t *soft_body, nanoem_status_t *status)
{
    nanoem_model_soft_body_anchor_t *origin = NULL;
    nanoem_mutable_model_soft_body_anchor_t *anchor = NULL;
    if (nanoem_is_not_null(soft_body)) {
        origin = nanoemModelSoftBodyAnchorCreate(soft_body->origin ,status);
        anchor = nanoemMutableModelSoftBodyAnchorCreateInternal(origin, nanoem_false, nanoem_false, status);
    }
    return anchor;
}

nanoem_mutable_model_soft_body_anchor_t *APIENTRY
nanoemMutableModelSoftBodyAnchorCreateAsReference(nanoem_model_soft_body_anchor_t *anchor, nanoem_status_t *status)
{
    const nanoem_model_t *origin_model;
    const nanoem_model_soft_body_t *origin_soft_body;
    nanoem_mutable_model_soft_body_anchor_t *mutable_anchor = NULL;
    nanoem_bool_t is_in_soft_body;
    if (nanoem_is_not_null(anchor)) {
        origin_soft_body = nanoemModelSoftBodyAnchorGetParentSoftBody(anchor);
        origin_model = nanoemModelSoftBodyGetParentModel(origin_soft_body);
        if (nanoem_is_not_null(origin_model) && nanoem_is_not_null(origin_soft_body)) {
            is_in_soft_body = nanoemObjectArrayContainsIndexedObject((const void *const *) origin_soft_body->anchors, anchor, origin_soft_body->num_anchors, anchor->base.index);
            mutable_anchor = nanoemMutableModelSoftBodyAnchorCreateInternal(anchor, nanoem_true, is_in_soft_body, status);
        }
        else {
            nanoem_status_ptr_assign_null_object(status);
        }
    }
    return mutable_anchor;
}

void APIENTRY
nanoemMutableModelSoftBodyAnchorSetRigidBodyObject(nanoem_mutable_model_soft_body_anchor_t *anchor, const nanoem_model_rigid_body_t *value)
{
    const nanoem_model_t *parent_model;
    const nanoem_model_soft_body_t *parent_soft_body;
    if (nanoem_is_not_null(anchor)) {
        parent_soft_body = nanoemModelSoftBodyAnchorGetParentSoftBody(anchor->origin);
        parent_model = nanoemModelSoftBodyGetParentModel(parent_soft_body);
        anchor->origin->rigid_body_index = nanoemModelResolveRigidBodyObject(parent_model, value);
    }
}

void APIENTRY
nanoemMutableModelSoftBodyAnchorSetVertexObject(nanoem_mutable_model_soft_body_anchor_t *anchor, const nanoem_model_vertex_t *value)
{
    if (nanoem_is_not_null(anchor)) {
        anchor->origin->vertex_index = nanoemModelObjectGetIndex(nanoemModelVertexGetModelObject(value));
    }
}

void APIENTRY
nanoemMutableModelSoftBodyAnchorSetNearEnabled(nanoem_mutable_model_soft_body_anchor_t *anchor, nanoem_bool_t value)
{
    if (nanoem_is_not_null(anchor)) {
        anchor->origin->is_near_enabled = value ? 1 : 0;
    }
}

nanoem_model_soft_body_anchor_t *APIENTRY
nanoemMutableModelSoftBodyAnchorGetOriginObject(nanoem_mutable_model_soft_body_anchor_t *anchor)
{
    return nanoem_is_not_null(anchor) ? anchor->origin : NULL;
}

void APIENTRY
nanoemMutableModelSoftBodyAnchorDestroy(nanoem_mutable_model_soft_body_anchor_t *anchor)
{
    if (nanoem_is_not_null(anchor)) {
        if (nanoemMutableBaseSoftBodyObjectObjectCanDelete(&anchor->base)) {
            nanoemModelSoftBodyAnchorDestroy(anchor->origin);
        }
        nanoem_free(anchor);
    }
}

nanoem_mutable_model_t *APIENTRY
nanoemMutableModelCreate(nanoem_unicode_string_factory_t *factory, nanoem_status_t *status)
{
    nanoem_mutable_model_t *new_model;
    nanoem_model_t *model;
    new_model = (nanoem_mutable_model_t *) nanoem_calloc(1, sizeof(*new_model), status);
    if (nanoem_is_not_null(new_model)) {
        model = new_model->origin = nanoemModelCreate(factory, status);
        model->vertices = (nanoem_model_vertex_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*model->vertices), status);
        model->vertex_indices = (nanoem_u32_t *) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*model->vertex_indices), status);
        model->materials = (nanoem_model_material_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*model->materials), status);
        model->bones = (nanoem_model_bone_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*model->bones), status);
        model->constraints = (nanoem_model_constraint_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*model->constraints), status);
        model->textures = (nanoem_model_texture_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*model->textures), status);
        model->morphs = (nanoem_model_morph_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*model->morphs), status);
        model->labels = (nanoem_model_label_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*model->labels), status);
        model->rigid_bodies = (nanoem_model_rigid_body_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*model->rigid_bodies), status);
        model->joints = (nanoem_model_joint_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*model->joints), status);
        new_model->is_reference = nanoem_false;
        new_model->num_allocated_vertices = __nanoem_default_allocation_size;
        new_model->num_allocated_vertex_indices = __nanoem_default_allocation_size;
        new_model->num_allocated_materials = __nanoem_default_allocation_size;
        new_model->num_allocated_bones = __nanoem_default_allocation_size;
        new_model->num_allocated_constraints = __nanoem_default_allocation_size;
        new_model->num_allocated_textures = __nanoem_default_allocation_size;
        new_model->num_allocated_morphs = __nanoem_default_allocation_size;
        new_model->num_allocated_labels = __nanoem_default_allocation_size;
        new_model->num_allocated_rigid_bodies = __nanoem_default_allocation_size;
        new_model->num_allocated_joints = __nanoem_default_allocation_size;
    }
    return new_model;
}

nanoem_mutable_model_t *APIENTRY
nanoemMutableModelCreateAsReference(nanoem_model_t *model, nanoem_status_t *status)
{
    nanoem_mutable_model_t *new_model = NULL;
    if (nanoem_is_not_null(model)) {
        new_model = (nanoem_mutable_model_t *) nanoem_calloc(1, sizeof(*new_model), status);
        if (nanoem_is_not_null(new_model)) {
            new_model->origin = model;
            new_model->is_reference = nanoem_true;
            new_model->num_allocated_vertices = model->num_vertices;
            new_model->num_allocated_vertex_indices = model->num_vertex_indices;
            new_model->num_allocated_materials = model->num_materials;
            new_model->num_allocated_bones = model->num_bones;
            new_model->num_allocated_constraints = model->num_constraints;
            new_model->num_allocated_textures = model->num_textures;
            new_model->num_allocated_morphs = model->num_morphs;
            new_model->num_allocated_labels = model->num_labels;
            new_model->num_allocated_rigid_bodies = model->num_rigid_bodies;
            new_model->num_allocated_joints = model->num_joints;
        }
    }
    return new_model;
}

void APIENTRY
nanoemMutableModelSetFormatType(nanoem_mutable_model_t *model, nanoem_model_format_type_t value)
{
    if (nanoem_is_not_null(model)) {
        switch (value) {
        case NANOEM_MODEL_FORMAT_TYPE_PMX_2_1:
            model->origin->version = 2.1f;
            break;
        case NANOEM_MODEL_FORMAT_TYPE_PMX_2_0:
            model->origin->version = 2.0f;
            break;
        case NANOEM_MODEL_FORMAT_TYPE_PMD_1_0:
            model->origin->version = 1.0f;
            break;
        default:
            break;
        }
    }
}

void APIENTRY
nanoemMutableModelSetCodecType(nanoem_mutable_model_t *model, nanoem_codec_type_t value)
{
    if (nanoem_is_not_null(model)) {
        switch (value) {
        case NANOEM_CODEC_TYPE_UTF16:
        default:
            model->origin->info.codec_type = 0;
            break;
        case NANOEM_CODEC_TYPE_UTF8:
            model->origin->info.codec_type = 1;
            break;
        }
    }
}

void APIENTRY
nanoemMutableModelSetAdditionalUVSize(nanoem_mutable_model_t *model, nanoem_rsize_t value)
{
    if (nanoem_is_not_null(model) && value >= 0 && value <= 4) {
        model->origin->info.additional_uv_size = value;
    }
}

void APIENTRY
nanoemMutableModelSetName(nanoem_mutable_model_t *model, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status)
{
    nanoem_model_t *origin;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(model)) {
        origin = model->origin;
        factory = origin->factory;
        switch (language) {
        case NANOEM_LANGUAGE_TYPE_ENGLISH:
            nanoemUnicodeStringFactoryAssignString(factory, &origin->name_en, value, nanoemModelGetCodecType(origin), status);
            break;
        case NANOEM_LANGUAGE_TYPE_JAPANESE:
            nanoemUnicodeStringFactoryAssignString(factory, &origin->name_ja, value, nanoemModelGetCodecType(origin), status);
            break;
        case NANOEM_LANGUAGE_TYPE_MAX_ENUM:
        case NANOEM_LANGUAGE_TYPE_UNKNOWN:
        default:
            break;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelSetComment(nanoem_mutable_model_t *model, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status)
{
    nanoem_model_t *origin;
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(model)) {
        origin = model->origin;
        factory = origin->factory;
        switch (language) {
        case NANOEM_LANGUAGE_TYPE_ENGLISH:
            nanoemUnicodeStringFactoryAssignString(factory, &origin->comment_en, value, nanoemModelGetCodecType(origin), status);
            break;
        case NANOEM_LANGUAGE_TYPE_JAPANESE:
            nanoemUnicodeStringFactoryAssignString(factory, &origin->comment_ja, value, nanoemModelGetCodecType(origin), status);
            break;
        case NANOEM_LANGUAGE_TYPE_MAX_ENUM:
        case NANOEM_LANGUAGE_TYPE_UNKNOWN:
        default:
            break;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelSetVertexIndices(nanoem_mutable_model_t *model, const nanoem_u32_t *value, nanoem_rsize_t length, nanoem_status_t *status)
{
    nanoem_model_t *origin;
    nanoem_rsize_t allocate_size;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(value) && length > 0) {
        origin = model->origin;
        allocate_size = length * sizeof(*origin->vertex_indices);
        if (length >= model->num_allocated_vertex_indices) {
            origin->vertex_indices = (nanoem_u32_t *) nanoem_realloc(origin->vertex_indices, allocate_size, status);
        }
        if (nanoem_is_not_null(origin->vertex_indices)) {
            nanoem_crt_memcpy(origin->vertex_indices, value, allocate_size);
            model->num_allocated_vertex_indices = origin->num_vertex_indices = length;
            nanoem_status_ptr_assign_succeeded(status);
        }
        else {
            model->num_allocated_vertex_indices = origin->num_vertex_indices = 0;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelInsertVertexObject(nanoem_mutable_model_t *model, nanoem_mutable_model_vertex_t *vertex, int index, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    nanoem_model_vertex_t *origin_vertex;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(vertex)) {
        origin_model = model->origin;
        origin_vertex = vertex->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_model->vertices, (nanoem_model_object_t *) origin_vertex, index, NANOEM_STATUS_ERROR_MODEL_VERTEX_ALREADY_EXISTS, &origin_model->num_vertices, &model->num_allocated_vertices, status);
        if (!nanoem_status_ptr_has_error(status)) {
            vertex->origin->base.parent.model = origin_model;
            vertex->base.is_in_model = nanoem_true;
            nanoemMutableModeVertexApplyChangeAllObjectIndices(origin_model, origin_vertex->base.index, 1);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelInsertMaterialObject(nanoem_mutable_model_t *model, nanoem_mutable_model_material_t *material, int index, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    nanoem_model_material_t *origin_material;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(material)) {
        origin_model = model->origin;
        origin_material = material->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_model->materials, (nanoem_model_object_t *) origin_material, index, NANOEM_STATUS_ERROR_MODEL_MATERIAL_ALREADY_EXISTS, &origin_model->num_materials, &model->num_allocated_materials, status);
        if (!nanoem_status_ptr_has_error(status)) {
            material->origin->base.parent.model = origin_model;
            material->base.is_in_model = nanoem_true;
            nanoemMutableModelMaterialApplyChangeAllObjectIndices(origin_model, origin_material->base.index, 1);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelInsertBoneObject(nanoem_mutable_model_t *model, nanoem_mutable_model_bone_t *bone, int index, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    nanoem_model_bone_t **ordered_bones, *origin_bone;
    nanoem_rsize_t num_bones;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(bone)) {
        origin_model = model->origin;
        origin_bone = bone->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_model->bones, (nanoem_model_object_t *) origin_bone, index, NANOEM_STATUS_ERROR_MODEL_BONE_ALREADY_EXISTS, &origin_model->num_bones, &model->num_allocated_bones, status);
        if (!nanoem_status_ptr_has_error(status)) {
            num_bones = origin_model->num_bones;
            ordered_bones = (nanoem_model_bone_t **) nanoem_realloc(origin_model->ordered_bones, sizeof(*ordered_bones) * num_bones, status);
            if (nanoem_is_not_null(ordered_bones)) {
                ordered_bones[num_bones - 1] = origin_bone;
                nanoem_crt_qsort(ordered_bones, num_bones, sizeof(*ordered_bones), origin_model->version > 1.0f ? nanoemModelCompareBonePMX : nanoemModelCompareBonePMD);
                origin_model->ordered_bones = ordered_bones;
                origin_bone->base.parent.model = origin_model;
                bone->base.is_in_model = nanoem_true;
                nanoemMutableModelBoneApplyChangeAllObjectIndices(origin_model, origin_bone->base.index, 1);
            }
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelInsertConstraintObject(nanoem_mutable_model_t *model, nanoem_mutable_model_constraint_t *constraint, int index, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(constraint)) {
        origin_model = model->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_model->constraints, (nanoem_model_object_t *) constraint->origin, index, NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_ALREADY_EXISTS, &origin_model->num_constraints, &model->num_allocated_constraints, status);
        if (!nanoem_status_ptr_has_error(status)) {
            constraint->origin->base.parent.model = origin_model;
            constraint->base.is_in_model = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelInsertTextureObject(nanoem_mutable_model_t *model, nanoem_mutable_model_texture_t *texture, int index, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    nanoem_model_texture_t *origin_texture;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(texture)) {
        origin_model = model->origin;
        origin_texture = texture->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_model->textures, (nanoem_model_object_t *) origin_texture, index, NANOEM_STATUS_ERROR_MODEL_TEXTURE_ALREADY_EXISTS, &origin_model->num_textures, &model->num_allocated_textures, status);
        if (!nanoem_status_ptr_has_error(status)) {
            texture->origin->base.parent.model = origin_model;
            texture->base.is_in_model = nanoem_true;
            nanoemMutableModelTextureApplyChangeAllObjectIndices(origin_model, origin_texture->base.index, 1);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelInsertMorphObject(nanoem_mutable_model_t *model, nanoem_mutable_model_morph_t *morph, int index, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    nanoem_model_morph_t *origin_morph;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(morph)) {
        origin_model = model->origin;
        origin_morph = morph->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_model->morphs, (nanoem_model_object_t *) origin_morph, index, NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS, &origin_model->num_morphs, &model->num_allocated_morphs, status);
        if (!nanoem_status_ptr_has_error(status)) {
            morph->origin->base.parent.model = origin_model;
            morph->base.is_in_model = nanoem_true;
            nanoemMutableModelMorphApplyChangeAllObjectIndices(origin_model, origin_morph->base.index, 1);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelInsertLabelObject(nanoem_mutable_model_t *model, nanoem_mutable_model_label_t *label, int index, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(label)) {
        origin_model = model->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_model->labels, (nanoem_model_object_t *) label->origin, index, NANOEM_STATUS_ERROR_MODEL_LABEL_ALREADY_EXISTS, &origin_model->num_labels, &model->num_allocated_labels, status);
        if (!nanoem_status_ptr_has_error(status)) {
            label->origin->base.parent.model = origin_model;
            label->base.is_in_model = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelInsertRigidBodyObject(nanoem_mutable_model_t *model, nanoem_mutable_model_rigid_body_t *rigid_body, int index, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    nanoem_model_rigid_body_t *origin_rigid_body;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(rigid_body)) {
        origin_model = model->origin;
        origin_rigid_body = rigid_body->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_model->rigid_bodies, (nanoem_model_object_t *) origin_rigid_body, index, NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_ALREADY_EXISTS, &origin_model->num_rigid_bodies, &model->num_allocated_rigid_bodies, status);
        if (!nanoem_status_ptr_has_error(status)) {
            rigid_body->origin->base.parent.model = origin_model;
            rigid_body->base.is_in_model = nanoem_true;
            nanoemMutableModelRigidBodyApplyChangeAllObjectIndices(origin_model, origin_rigid_body->base.index, 1);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelInsertJointObject(nanoem_mutable_model_t *model, nanoem_mutable_model_joint_t *joint, int index, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(joint)) {
        origin_model = model->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_model->joints, (nanoem_model_object_t *) joint->origin, index, NANOEM_STATUS_ERROR_MODEL_JOINT_ALREADY_EXISTS, &origin_model->num_joints, &model->num_allocated_joints, status);
        if (!nanoem_status_ptr_has_error(status)) {
            joint->origin->base.parent.model = origin_model;
            joint->base.is_in_model = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelInsertSoftBodyObject(nanoem_mutable_model_t *model, nanoem_mutable_model_soft_body_t *soft_body, int index, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(soft_body)) {
        origin_model = model->origin;
        nanoemModelObjectArrayInsertObject((nanoem_model_object_t ***) &origin_model->soft_bodies, (nanoem_model_object_t *) soft_body->origin, index, NANOEM_STATUS_ERROR_MODEL_SOFT_BODY_ALREADY_EXISTS, &origin_model->num_soft_bodies, &model->num_allocated_soft_bodies, status);
        if (!nanoem_status_ptr_has_error(status)) {
            soft_body->origin->base.parent.model = origin_model;
            soft_body->base.is_in_model = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelRemoveVertexObject(nanoem_mutable_model_t *model, nanoem_mutable_model_vertex_t *vertex, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    nanoem_model_vertex_t *origin_vertex;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(vertex)) {
        origin_model = model->origin;
        origin_vertex = vertex->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_model->vertices, (nanoem_model_object_t *) origin_vertex, &origin_model->num_vertices, NANOEM_STATUS_ERROR_MODEL_VERTEX_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            nanoemMutableModeVertexApplyChangeAllObjectIndices(origin_model, origin_vertex->base.index, -1);
            vertex->origin->base.parent.model = NULL;
            vertex->base.is_in_model = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelRemoveMaterialObject(nanoem_mutable_model_t *model, nanoem_mutable_model_material_t *material, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    nanoem_model_material_t *origin_material;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(material)) {
        origin_model = model->origin;
        origin_material = material->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_model->materials, (nanoem_model_object_t *) origin_material, &origin_model->num_materials, NANOEM_STATUS_ERROR_MODEL_MATERIAL_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            nanoemMutableModelMaterialApplyChangeAllObjectIndices(origin_model, origin_material->base.index, -1);
            material->origin->base.parent.model = NULL;
            material->base.is_in_model = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelRemoveBoneObject(nanoem_mutable_model_t *model, nanoem_mutable_model_bone_t *bone, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    nanoem_model_bone_t *origin_bone;
    nanoem_model_label_t *label;
    nanoem_model_label_item_t *label_item;
    nanoem_rsize_t num_objects, num_items, i, j;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(bone)) {
        origin_model = model->origin;
        origin_bone = bone->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_model->bones, (nanoem_model_object_t *) origin_bone, &origin_model->num_bones, NANOEM_STATUS_ERROR_MODEL_BONE_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            nanoemMutableModelBoneApplyChangeAllObjectIndices(origin_model, origin_bone->base.index, -1);
            bone->origin->base.parent.model = NULL;
            bone->base.is_in_model = nanoem_false;
            num_objects = origin_model->num_labels;
            for (i = 0; i < num_objects; i++) {
                label = origin_model->labels[i];
                num_items = label->num_items;
                for (j = 0; j < num_items; j++) {
                    label_item = label->items[j];
                    if (label_item->type == NANOEM_MODEL_LABEL_ITEM_TYPE_BONE && label_item->u.bone == origin_bone) {
                        label_item->u.bone = NULL;
                    }
                }
            }
            nanoem_crt_memcpy(origin_model->ordered_bones, origin_model->bones, sizeof(*origin_model->ordered_bones) * origin_model->num_bones);
            nanoem_crt_qsort(origin_model->ordered_bones, origin_model->num_bones, sizeof(*origin_model->ordered_bones), nanoemModelIsPMX(origin_model) ? nanoemModelCompareBonePMX : nanoemModelCompareBonePMD);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelRemoveConstraintObject(nanoem_mutable_model_t *model, nanoem_mutable_model_constraint_t *constraint, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(constraint)) {
        origin_model = model->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_model->constraints, (nanoem_model_object_t *) constraint->origin, &origin_model->num_constraints, NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            constraint->origin->base.parent.model = NULL;
            constraint->base.is_in_model = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelRemoveTextureObject(nanoem_mutable_model_t *model, nanoem_mutable_model_texture_t *texture, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    nanoem_model_texture_t *origin_texture;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(texture)) {
        origin_model = model->origin;
        origin_texture = texture->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_model->textures, (nanoem_model_object_t *) origin_texture, &origin_model->num_textures, NANOEM_STATUS_ERROR_MODEL_TEXTURE_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            nanoemMutableModelTextureApplyChangeAllObjectIndices(origin_model, origin_texture->base.index, -1);
            texture->origin->base.parent.model = NULL;
            texture->base.is_in_model = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelRemoveMorphObject(nanoem_mutable_model_t *model, nanoem_mutable_model_morph_t *morph, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    nanoem_model_morph_t *origin_morph;
    nanoem_model_label_t *label;
    nanoem_model_label_item_t *label_item;
    nanoem_rsize_t num_objects, num_items, i, j;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(morph)) {
        origin_model = model->origin;
        origin_morph = morph->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_model->morphs, (nanoem_model_object_t *) origin_morph, &origin_model->num_morphs, NANOEM_STATUS_ERROR_MODEL_MORPH_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            nanoemMutableModelMorphApplyChangeAllObjectIndices(origin_model, origin_morph->base.index, -1);
            morph->origin->base.parent.model = NULL;
            morph->base.is_in_model = nanoem_false;
            num_objects = origin_model->num_labels;
            for (i = 0; i < num_objects; i++) {
                label = origin_model->labels[i];
                num_items = label->num_items;
                for (j = 0; j < num_items; j++) {
                    label_item = label->items[j];
                    if (label_item->type == NANOEM_MODEL_LABEL_ITEM_TYPE_MORPH && label_item->u.morph == origin_morph) {
                        label_item->u.morph = NULL;
                    }
                }
            }
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelRemoveLabelObject(nanoem_mutable_model_t *model, nanoem_mutable_model_label_t *label, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(label)) {
        origin_model = model->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_model->labels, (nanoem_model_object_t *) label->origin, &origin_model->num_labels, NANOEM_STATUS_ERROR_MODEL_LABEL_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            label->origin->base.parent.model = NULL;
            label->base.is_in_model = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelRemoveRigidBodyObject(nanoem_mutable_model_t *model, nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    nanoem_model_rigid_body_t *origin_rigid_body;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(rigid_body)) {
        origin_model = model->origin;
        origin_rigid_body = rigid_body->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_model->rigid_bodies, (nanoem_model_object_t *) origin_rigid_body, &origin_model->num_rigid_bodies, NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            nanoemMutableModelRigidBodyApplyChangeAllObjectIndices(origin_model, origin_rigid_body->base.index, -1);
            rigid_body->origin->base.parent.model = NULL;
            rigid_body->base.is_in_model = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelRemoveJointObject(nanoem_mutable_model_t *model, nanoem_mutable_model_joint_t *joint, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(joint)) {
        origin_model = model->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_model->joints, (nanoem_model_object_t *) joint->origin, &origin_model->num_joints, NANOEM_STATUS_ERROR_MODEL_JOINT_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            joint->origin->base.parent.model = NULL;
            joint->base.is_in_model = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableModelRemoveSoftBodyObject(nanoem_mutable_model_t *model, nanoem_mutable_model_soft_body_t *soft_body, nanoem_status_t *status)
{
    nanoem_model_t *origin_model;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(soft_body)) {
        origin_model = model->origin;
        nanoemModelObjectArrayRemoveObject((nanoem_model_object_t **) origin_model->soft_bodies, (nanoem_model_object_t *) soft_body->origin, &origin_model->num_soft_bodies, NANOEM_STATUS_ERROR_MODEL_SOFT_BODY_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            soft_body->origin->base.parent.model = NULL;
            soft_body->base.is_in_model = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

nanoem_model_t *APIENTRY
nanoemMutableModelGetOriginObject(nanoem_mutable_model_t *model)
{
    return nanoem_is_not_null(model) ? model->origin : NULL;
}

static void
nanoemModelVerticesSaveToBuffer(const nanoem_model_t *origin, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_vertex_t *vertex;
    nanoem_mutable_model_vertex_t mutable_vertex;
    nanoem_rsize_t length, i;
    length = origin->num_vertices;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) length, status);
    for (i = 0; i < length; i++) {
        vertex = origin->vertices[i];
        nanoemMutableModelVertexInitialize(&mutable_vertex, vertex, nanoem_true, nanoem_true);
        nanoemMutableModelVertexSaveToBuffer(&mutable_vertex, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            return;
        }
    }
    nanoem_status_ptr_assign_succeeded(status);
}

static void
nanoemModelTexturesSaveToBuffer(const nanoem_model_t *origin, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_texture_t *texture;
    nanoem_mutable_model_texture_t mutable_texture;
    nanoem_rsize_t length, i;
    length = origin->num_textures;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) length, status);
    for (i = 0; i < length; i++) {
        texture = origin->textures[i];
        nanoemMutableModelTextureInitialize(&mutable_texture, texture, nanoem_true, nanoem_true);
        nanoemMutableModelTextureSaveToBuffer(&mutable_texture, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            return;
        }
    }
    nanoem_status_ptr_assign_succeeded(status);
}

static void
nanoemModelMaterialsSaveToBuffer(const nanoem_model_t *origin, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_material_t *material;
    nanoem_mutable_model_material_t mutable_material;
    nanoem_rsize_t length, i;
    length = origin->num_materials;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) length, status);
    for (i = 0; i < length; i++) {
        material = origin->materials[i];
        nanoemMutableModelMaterialInitialize(&mutable_material, material, nanoem_true, nanoem_true);
        nanoemMutableModelMaterialSaveToBuffer(&mutable_material, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            return;
        }
    }
    nanoem_status_ptr_assign_succeeded(status);
}

static void
nanoemModelBonesSaveToBuffer(const nanoem_model_t *origin, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_bone_t *bone;
    nanoem_mutable_model_bone_t mutable_bone;
    nanoem_rsize_t length, i;
    length = origin->num_bones;
    nanoemMutableBufferWriteInteger(buffer, (int) length, nanoemModelIsPMX(origin) ? 4 : 2, status);
    for (i = 0; i < length; i++) {
        bone = origin->bones[i];
        nanoemMutableModelBoneInitialize(&mutable_bone, bone, nanoem_true, nanoem_true);
        nanoemMutableModelBoneSaveToBuffer(&mutable_bone, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            return;
        }
    }
    nanoem_status_ptr_assign_succeeded(status);
}

static void
nanoemModelMorphsSaveToBuffer(const nanoem_model_t *origin, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_morph_t *morph;
    nanoem_mutable_model_morph_t mutable_morph;
    nanoem_rsize_t length, i;
    length = origin->num_morphs;
    nanoemMutableBufferWriteInteger(buffer, (int) length, nanoemModelIsPMX(origin) ? 4 : 2, status);
    for (i = 0; i < length; i++) {
        morph = origin->morphs[i];
        nanoemMutableModelMorphInitialize(&mutable_morph, morph, nanoem_true, nanoem_true);
        nanoemMutableModelMorphSaveToBuffer(&mutable_morph, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            return;
        }
    }
    nanoem_status_ptr_assign_succeeded(status);
}

static void
nanoemModelLabelsSaveToBufferPMD(const nanoem_model_t *origin, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory = origin->factory;
    nanoem_u8_t name_buffer[PMD_BONE_CATEGORY_NAME_LENGTH + 1];
    nanoem_model_label_t *label;
    nanoem_model_label_item_t *item;
    const nanoem_rsize_t num_labels = origin->num_labels;
    nanoem_rsize_t num_bones = 0, num_morphs = 0, num_bone_labels = 0, num_items, i, j;
    for (i = 0; i < num_labels; i++) {
        label = origin->labels[i];
        num_items = label->num_items;
        for (j = 0; j < num_items; j++) {
            item = label->items[j];
            switch (item->type) {
            case NANOEM_MODEL_LABEL_ITEM_TYPE_BONE:
                num_bones++;
                break;
            case NANOEM_MODEL_LABEL_ITEM_TYPE_MORPH:
                num_morphs++;
                break;
            case NANOEM_MODEL_LABEL_ITEM_TYPE_MAX_ENUM:
            case NANOEM_MODEL_LABEL_ITEM_TYPE_UNKNOWN:
                break;
            }
        }
        if (!label->is_special) {
            num_bone_labels++;
        }
    }
    nanoemMutableBufferWriteByte(buffer, (nanoem_u8_t) num_morphs, status);
    for (i = 0; i < num_labels; i++) {
        label = origin->labels[i];
        if (label->is_special) {
            num_items = label->num_items;
            for (j = 0; j < num_items; j++) {
                item = label->items[j];
                if (item->type == NANOEM_MODEL_LABEL_ITEM_TYPE_MORPH) {
                    nanoemMutableBufferWriteInt16LittleEndian(buffer, nanoemModelObjectGetIndex(nanoemModelMorphGetModelObject(item->u.morph)), status);
                }
            }
        }
    }
    nanoemMutableBufferWriteByte(buffer, (nanoem_u8_t) num_bone_labels, status);
    for (i = 0; i < num_labels; i++) {
        label = origin->labels[i];
        if (!label->is_special) {
            nanoemMutableBufferWriteCp932String(buffer, label->name_ja, factory, name_buffer, sizeof(name_buffer), status);
        }
    }
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_bones, status);
    for (i = 0; i < num_labels; i++) {
        label = origin->labels[i];
        if (!label->is_special) {
            num_items = label->num_items;
            for (j = 0; j < num_items; j++) {
                item = label->items[j];
                if (item->type == NANOEM_MODEL_LABEL_ITEM_TYPE_BONE) {
                    nanoemMutableBufferWriteInt16LittleEndian(buffer, nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(item->u.bone)), status);
                    nanoemMutableBufferWriteByte(buffer, (nanoem_u8_t) i, status);
                }
            }
        }
    }
}

static void
nanoemModelLabelsSaveToBuffer(const nanoem_model_t *origin, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_label_t *label;
    nanoem_mutable_model_label_t mutable_label;
    nanoem_rsize_t num_labels, i;
    if (nanoemModelIsPMX(origin)) {
        num_labels = origin->num_labels;
        nanoemMutableBufferWriteInteger(buffer, (int) num_labels, 4, status);
        nanoem_crt_memset(&mutable_label, 0, sizeof(mutable_label));
        for (i = 0; i < num_labels; i++) {
            label = origin->labels[i];
            nanoemMutableModelLabelInitialize(&mutable_label, label, nanoem_true, nanoem_true);
            nanoemMutableModelLabelSaveToBuffer(&mutable_label, buffer, status);
            if (nanoem_status_ptr_has_error(status)) {
                return;
            }
        }
    }
    else {
        nanoemModelLabelsSaveToBufferPMD(origin, buffer, status);
    }
}

static void
nanoemModelRigidBodiesSaveToBuffer(const nanoem_model_t *origin, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_rigid_body_t *rigid_body;
    nanoem_mutable_model_rigid_body_t mutable_rigid_body;
    nanoem_rsize_t length, i;
    length = origin->num_rigid_bodies;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) length, status);
    for (i = 0; i < length; i++) {
        rigid_body = origin->rigid_bodies[i];
        nanoemMutableModelRigidBodyInitialize(&mutable_rigid_body, rigid_body, nanoem_true, nanoem_true);
        nanoemMutableModelRigidBodySaveToBuffer(&mutable_rigid_body, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            return;
        }
    }
    nanoem_status_ptr_assign_succeeded(status);
}

static void
nanoemModelJointsSaveToBuffer(const nanoem_model_t *origin, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_joint_t *joint;
    nanoem_mutable_model_joint_t mutable_joint;
    nanoem_rsize_t length, i;
    length = origin->num_joints;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) length, status);
    for (i = 0; i < length; i++) {
        joint = origin->joints[i];
        nanoemMutableModelJointInitialize(&mutable_joint, joint, nanoem_true, nanoem_true);
        nanoemMutableModelJointSaveToBuffer(&mutable_joint, buffer, status);
        if (nanoem_status_ptr_has_error(status)) {
            return;
        }
    }
    nanoem_status_ptr_assign_succeeded(status);
}

static void
nanoemModelSoftBodiesSaveToBuffer(const nanoem_model_t *origin, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_soft_body_t *soft_body;
    nanoem_mutable_model_soft_body_t mutable_soft_body;
    nanoem_rsize_t length, i;
    if (nanoemModelIsPMX21(origin)) {
        length = origin->num_soft_bodies;
        nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) length, status);
        for (i = 0; i < length; i++) {
            soft_body = origin->soft_bodies[i];
            nanoemMutableModelSoftBodyInitialize(&mutable_soft_body, soft_body, nanoem_true, nanoem_true);
            nanoemMutableModelSoftBodySaveToBuffer(&mutable_soft_body, buffer, status);
            if (nanoem_status_ptr_has_error(status)) {
                return;
            }
        }
    }
    nanoem_status_ptr_assign_succeeded(status);
}

static void
nanoemModelEnglishBlockSaveToBufferPMD(const nanoem_model_t *origin, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_bool_t enable_english = nanoem_true;
    nanoem_u8_t name_buffer[PMD_MODEL_NAME_LENGTH + 1], comment_buffer[PMD_MODEL_COMMENT_LENGTH + 1],
                   bone_name_buffer[PMD_BONE_NAME_LENGTH + 1], morph_name_buffer[PMD_MORPH_NAME_LENGTH + 1],
                   bone_category_name_buffer[PMD_BONE_CATEGORY_NAME_LENGTH + 1];
    nanoem_unicode_string_factory_t *factory = origin->factory;
    nanoem_rsize_t num_objects, i;
    if (enable_english) {
        nanoemMutableBufferWriteByte(buffer, 1, status);
        nanoemMutableBufferWriteCp932String(buffer, origin->name_en, factory, name_buffer, sizeof(name_buffer), status);
        nanoemMutableBufferWriteCp932String(buffer, origin->comment_en, factory, comment_buffer, sizeof(comment_buffer), status);
        num_objects = origin->num_bones;
        for (i = 0; i < num_objects; i++) {
            nanoemMutableBufferWriteCp932String(buffer, origin->bones[i]->name_en, factory, bone_name_buffer, sizeof(bone_name_buffer), status);
        }
        num_objects = origin->num_morphs;
        if (num_objects > 0) {
            num_objects -= 1;
            for (i = 0; i < num_objects; i++) {
                nanoemMutableBufferWriteCp932String(buffer, origin->morphs[i + 1]->name_en, factory, morph_name_buffer, sizeof(morph_name_buffer), status);
            }
        }
        num_objects = origin->num_labels;
        if (num_objects > 0) {
            num_objects -= 1;
            for (i = 0; i < num_objects; i++) {
                nanoemMutableBufferWriteCp932String(buffer, origin->labels[i + 1]->name_en, factory, bone_category_name_buffer, sizeof(bone_category_name_buffer), status);
            }
        }
    }
    else {
        nanoemMutableBufferWriteByte(buffer, 0, status);
    }
}

static void
nanoemModelCustomTextureBlockSaveToBufferPMD(const nanoem_model_t *origin, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory = origin->factory;
    nanoem_unicode_string_t *texture_path = NULL;
    nanoem_u8_t path_buffer[PMD_TOON_TEXTURE_PATH_LENGTH + 1];
    nanoem_rsize_t num_objects = origin->num_textures, i;
    for (i = 0; i < 10; i++) {
        if (i < num_objects && nanoem_is_not_null(origin->textures[i])) {
            texture_path = origin->textures[i]->path;
        }
        nanoemMutableBufferWriteCp932String(buffer, texture_path, factory, path_buffer, sizeof(path_buffer), status);
    }
}

#define nanoem_save_check(expr) \
    do { \
        (expr); \
        if (nanoem_status_ptr_has_error(status)) { \
            return nanoem_false; \
        } \
    } while (0)

static nanoem_bool_t
nanoemMutableModelSaveToBufferPMD(nanoem_mutable_model_t *model, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_model_t *origin = model->origin;
    nanoem_unicode_string_factory_t *factory = origin->factory;
    nanoem_mutable_model_constraint_t *constraint;
    nanoem_u8_t name_buffer[PMD_MODEL_NAME_LENGTH + 1], comment_buffer[PMD_MODEL_COMMENT_LENGTH + 1];
    nanoem_rsize_t num_objects, i;
    int vertex_index;
    nanoemMutableBufferWriteByteArray(buffer, (const nanoem_u8_t *) __nanoem_pmd_signature, sizeof(__nanoem_pmd_signature) - 1, status);
    nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->version, status);
    nanoemMutableBufferWriteCp932String(buffer, origin->name_ja, factory, name_buffer, sizeof(name_buffer), status);
    nanoemMutableBufferWriteCp932String(buffer, origin->comment_ja, factory, comment_buffer, sizeof(comment_buffer), status);
    nanoem_save_check(nanoemModelVerticesSaveToBuffer(origin, buffer, status));
    num_objects = origin->num_vertex_indices;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_objects, status);
    for (i = 0; i < num_objects; i++) {
        vertex_index = origin->vertex_indices[i];
        nanoemMutableBufferWriteInt16LittleEndianUnsigned(buffer, vertex_index, status);
    }
    nanoem_save_check(nanoemModelMaterialsSaveToBuffer(origin, buffer, status));
    nanoem_save_check(nanoemModelBonesSaveToBuffer(origin, buffer, status));
    num_objects = origin->num_constraints;
    nanoemMutableBufferWriteInt16LittleEndianUnsigned(buffer, (nanoem_u16_t) num_objects, status);
    for (i = 0; i < num_objects; i++) {
        constraint = nanoemMutableModelConstraintCreateAsReference(origin->constraints[i], status);
        nanoemMutableModelConstraintSaveToBuffer(constraint, buffer, status);
        nanoemMutableModelConstraintDestroy(constraint);
        if (nanoem_status_ptr_has_error(status)) {
            return nanoem_false;
        }
    }
    nanoem_save_check(nanoemModelMorphsSaveToBuffer(origin, buffer, status));
    nanoem_save_check(nanoemModelLabelsSaveToBuffer(origin, buffer, status));
    nanoemModelEnglishBlockSaveToBufferPMD(origin, buffer, status);
    nanoemModelCustomTextureBlockSaveToBufferPMD(origin, buffer, status);
    nanoem_save_check(nanoemModelRigidBodiesSaveToBuffer(origin, buffer, status));
    nanoem_save_check(nanoemModelJointsSaveToBuffer(origin, buffer, status));
    return !nanoem_status_ptr_has_error(status);
}

static nanoem_bool_t
nanoemMutableModelSaveToBufferPMX(nanoem_mutable_model_t *model, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_model_t *origin = model->origin;
    nanoem_unicode_string_factory_t *factory = origin->factory;
    nanoem_codec_type_t codec = nanoemModelGetCodecType(origin);
    nanoem_rsize_t vertex_index_size, length, i;
    int vertex_index;
    origin->info_length = 8;
    origin->info.vertex_index_size = (unsigned int) nanoemModelGetVertexIndexSize(origin->num_vertices);
    origin->info.texture_index_size = (unsigned int) nanoemModelGetObjectIndexSize(origin->num_textures);
    origin->info.material_index_size = (unsigned int) nanoemModelGetObjectIndexSize(origin->num_materials);
    origin->info.bone_index_size = (unsigned int) nanoemModelGetObjectIndexSize(origin->num_bones);
    origin->info.morph_index_size = (unsigned int) nanoemModelGetObjectIndexSize(origin->num_morphs);
    origin->info.rigid_body_index_size = (unsigned int) nanoemModelGetObjectIndexSize(origin->num_rigid_bodies);
    nanoemMutableBufferWriteByteArray(buffer, (const nanoem_u8_t *) __nanoem_pmx_signature, sizeof(__nanoem_pmx_signature) - 1, status);
    nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->version, status);
    nanoemMutableBufferWriteByte(buffer, origin->info_length, status);
    nanoemMutableBufferWriteByte(buffer, origin->info.codec_type, status);
    nanoemMutableBufferWriteByte(buffer, origin->info.additional_uv_size, status);
    nanoemMutableBufferWriteByte(buffer, origin->info.vertex_index_size, status);
    nanoemMutableBufferWriteByte(buffer, origin->info.texture_index_size, status);
    nanoemMutableBufferWriteByte(buffer, origin->info.material_index_size, status);
    nanoemMutableBufferWriteByte(buffer, origin->info.bone_index_size, status);
    nanoemMutableBufferWriteByte(buffer, origin->info.morph_index_size, status);
    nanoemMutableBufferWriteByte(buffer, origin->info.rigid_body_index_size, status);
    nanoemMutableBufferWriteUnicodeString(buffer, origin->name_ja, factory, codec, status);
    nanoemMutableBufferWriteUnicodeString(buffer, origin->name_en, factory, codec, status);
    nanoemMutableBufferWriteUnicodeString(buffer, origin->comment_ja, factory, codec, status);
    nanoemMutableBufferWriteUnicodeString(buffer, origin->comment_en, factory, codec, status);
    nanoem_save_check(nanoemModelVerticesSaveToBuffer(origin, buffer, status));
    vertex_index_size = origin->info.vertex_index_size;
    length = origin->num_vertex_indices;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) length, status);
    for (i = 0; i < length; i++) {
        vertex_index = origin->vertex_indices[i];
        nanoemMutableBufferWriteInteger(buffer, vertex_index, vertex_index_size, status);
    }
    nanoem_save_check(nanoemModelTexturesSaveToBuffer(origin, buffer, status));
    nanoem_save_check(nanoemModelMaterialsSaveToBuffer(origin, buffer, status));
    nanoem_save_check(nanoemModelBonesSaveToBuffer(origin, buffer, status));
    nanoem_save_check(nanoemModelMorphsSaveToBuffer(origin, buffer, status));
    nanoem_save_check(nanoemModelLabelsSaveToBuffer(origin, buffer, status));
    nanoem_save_check(nanoemModelRigidBodiesSaveToBuffer(origin, buffer, status));
    nanoem_save_check(nanoemModelJointsSaveToBuffer(origin, buffer, status));
    nanoem_save_check(nanoemModelSoftBodiesSaveToBuffer(origin, buffer, status));
    return !nanoem_status_ptr_has_error(status);
}

#undef nanoem_save_check

nanoem_model_t *APIENTRY
nanoemMutableModelGetOriginObjectReference(nanoem_mutable_model_t *model)
{
    nanoem_model_t *origin = NULL;
    if (nanoem_is_not_null(model)) {
        origin = model->origin;
        model->is_reference = nanoem_true;
    }
    return origin;
}

nanoem_bool_t APIENTRY
nanoemMutableModelSaveToBuffer(nanoem_mutable_model_t *model, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_status_ptr_assign_null_object(status);
    if (nanoem_is_not_null(model)) {
        nanoem_status_ptr_assign_succeeded(status);
        if (nanoemModelIsPMX(nanoemMutableModelGetOriginObject(model))) {
            nanoemMutableModelSaveToBufferPMX(model, buffer, status);
        }
        else {
            nanoemMutableModelSaveToBufferPMD(model, buffer, status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
    return !nanoem_status_ptr_has_error(status);
}

void APIENTRY
nanoemMutableModelResetAllocationSize(nanoem_mutable_model_t *model)
{
    const nanoem_model_t *origin;
    if (nanoem_is_not_null(model)) {
        origin = model->origin;
        model->num_allocated_bones = origin->num_bones;
        model->num_allocated_constraints = origin->num_constraints;
        model->num_allocated_joints = origin->num_joints;
        model->num_allocated_labels = origin->num_labels;
        model->num_allocated_materials = origin->num_materials;
        model->num_allocated_morphs = origin->num_morphs;
        model->num_allocated_rigid_bodies = origin->num_rigid_bodies;
        model->num_allocated_soft_bodies = origin->num_soft_bodies;
        model->num_allocated_textures = origin->num_textures;
        model->num_allocated_vertex_indices = origin->num_vertex_indices;
        model->num_allocated_vertices = origin->num_vertices;
    }
}

void APIENTRY
nanoemMutableModelDestroy(nanoem_mutable_model_t *model)
{
    if (nanoem_is_not_null(model)) {
        if (!model->is_reference) {
            nanoemModelDestroy(model->origin);
        }
        nanoem_free(model);
    }
}
