/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

// based on
// https://drive.google.com/open?id=0B6jwWdrYAgJTdXZSd1Noa2hKbmM

#include "./document.h"

#include "./document_p.h"

KHASH_SET_INIT_INT64(frame_index_set)

nanoem_document_outside_parent_t *
nanoemDocumentOutsideParentCreate(const nanoem_document_t *document, nanoem_status_t *status)
{
    nanoem_document_outside_parent_t *parent;
    parent = (nanoem_document_outside_parent_t *) nanoem_calloc(1, sizeof(*parent), status);
    if (nanoem_is_not_null(parent)) {
        parent->base.parent.document = document;
    }
    return parent;
}

void
nanoemDocumentOutsideParentParse(nanoem_document_outside_parent_t *parent, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    parent->model_index = nanoemBufferReadInt32LittleEndian(buffer, status);
    parent->bone_index = nanoemBufferReadInt32LittleEndian(buffer, status);
}

const nanoem_document_model_t * APIENTRY
nanoemDocumentOutsideParentGetModelObject(const nanoem_document_outside_parent_t *parent)
{
    const nanoem_document_model_t *model = NULL;
    if (nanoem_is_not_null(parent)) {
        model = nanoemDocumentResolveModelObject(parent->base.parent.document, parent->model_index);
    }
    return model;
}

const nanoem_unicode_string_t * APIENTRY
nanoemDocumentOutsideParentGetBoneName(const nanoem_document_outside_parent_t *parent)
{
    const nanoem_document_model_t *model;
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(parent)) {
        model = nanoemDocumentResolveModelObject(parent->base.parent.document, parent->model_index);
        name = nanoemDocumentModelResolveBoneName(model, parent->bone_index);
    }
    return name;
}

void
nanoemDocumentOutsideParentDestroy(nanoem_document_outside_parent_t *parent)
{
    if (nanoem_is_not_null(parent)) {
        nanoem_free(parent);
    }
}

nanoem_frame_index_t APIENTRY
nanoemDocumentBaseKeyframeGetFrameIndex(const nanoem_document_base_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->frame_index : 0;
}

int APIENTRY
nanoemDocumentBaseKeyframeGetPreviousKeyframeIndex(const nanoem_document_base_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->previous_keyframe_index : -1;
}

int APIENTRY
nanoemDocumentBaseKeyframeGetNextKeyframeIndex(const nanoem_document_base_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->next_keyframe_index : -1;
}

int APIENTRY
nanoemDocumentBaseKeyframeGetIndex(const nanoem_document_base_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->object_index : 0;
}

nanoem_bool_t APIENTRY
nanoemDocumentBaseKeyframeIsSelected(const nanoem_document_base_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->is_selected : nanoem_false;
}

void APIENTRY
nanoemDocumentBaseKeyframeSetSelected(nanoem_document_base_keyframe_t *keyframe, nanoem_bool_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->is_selected = value ? nanoem_true : nanoem_false;
    }
}

nanoem_document_accessory_keyframe_t *
nanoemDocumentAccessoryKeyframeCreate(nanoem_document_accessory_t *accessory, nanoem_status_t *status)
{
    nanoem_document_accessory_keyframe_t *keyframe;
    keyframe = (nanoem_document_accessory_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
    if (nanoem_is_not_null(keyframe)) {
        keyframe->parent_accessory = accessory;
    }
    return keyframe;
}

void
nanoemDocumentAccessoryKeyframeInitialize(nanoem_document_accessory_keyframe_t *keyframe)
{
    keyframe->is_shadow_enabled = nanoem_true;
    keyframe->opacity = 1.0f;
    keyframe->parent_model_index = -1;
    keyframe->scale_factor = 10.0f;
    keyframe->visible = nanoem_true;
}

void
nanoemDocumentAccessoryKeyframeParse(nanoem_document_accessory_keyframe_t *keyframe, nanoem_buffer_t *buffer, nanoem_bool_t include_index, nanoem_status_t *status)
{
    nanoemDocumentBaseKeyframeParse(&keyframe->base, buffer, include_index, status);
    nanoemDocumentAccessoryUnpackOpacityAndVisible(nanoemBufferReadByte(buffer, status), &keyframe->opacity, &keyframe->visible);
    keyframe->parent_model_index = nanoemBufferReadInt32LittleEndian(buffer, status);
    keyframe->parent_model_bone_index = nanoemBufferReadInt32LittleEndian(buffer, status);
    nanoemBufferReadFloat32x3LittleEndian(buffer, &keyframe->translation, status);
    nanoemBufferReadFloat32x3LittleEndian(buffer, &keyframe->orientation, status);
    keyframe->scale_factor = nanoemBufferReadFloat32LittleEndian(buffer, status);
    keyframe->is_shadow_enabled = nanoemBufferReadByte(buffer, status) != 0;
    keyframe->base.is_selected = nanoemBufferReadByte(buffer, status) != 0;
    if (nanoem_status_ptr_has_error(status)) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_KEYFRAME_CORRUPTED);
    }
}

const nanoem_document_base_keyframe_t * APIENTRY
nanoemDocumentAccessoryKeyframeGetBaseKeyframeObject(const nanoem_document_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_document_base_keyframe_t *APIENTRY
nanoemDocumentAccessoryKeyframeGetBaseKeyframeObjectMutable(nanoem_document_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

const nanoem_document_model_t * APIENTRY
nanoemDocumentAccessoryKeyframeGetParentModelObject(const nanoem_document_accessory_keyframe_t *keyframe)
{
    const nanoem_document_model_t *model = NULL;
    if (nanoem_is_not_null(keyframe)) {
        model = nanoemDocumentResolveModelObject(keyframe->parent_accessory->base.parent.document, keyframe->parent_model_index);
    }
    return model;
}

const nanoem_unicode_string_t * APIENTRY
nanoemDocumentAccessoryKeyframeGetParentModelBoneName(const nanoem_document_accessory_keyframe_t *keyframe)
{
    const nanoem_document_model_t *model;
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(keyframe)) {
        model = nanoemDocumentAccessoryKeyframeGetParentModelObject(keyframe);
        name = nanoemDocumentModelResolveBoneName(model, keyframe->parent_model_bone_index);
    }
    return name;
}

const nanoem_f32_t * APIENTRY
nanoemDocumentAccessoryKeyframeGetTranslation(const nanoem_document_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->translation.values : __nanoem_null_vector3;
}

const nanoem_f32_t * APIENTRY
nanoemDocumentAccessoryKeyframeGetOrientation(const nanoem_document_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->orientation.values : __nanoem_null_vector3;
}

nanoem_f32_t APIENTRY
nanoemDocumentAccessoryKeyframeGetScaleFactor(const nanoem_document_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->scale_factor : 0.0f;
}

nanoem_f32_t APIENTRY
nanoemDocumentAccessoryKeyframeGetOpacity(const nanoem_document_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->opacity : 0.0f;
}

nanoem_bool_t APIENTRY
nanoemDocumentAccessoryKeyframeIsVisible(const nanoem_document_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->visible : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemDocumentAccessoryKeyframeIsShadowEnabled(const nanoem_document_accessory_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->is_shadow_enabled : nanoem_false;
}

void
nanoemDocumentAccessoryKeyframeDestroy(nanoem_document_accessory_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        nanoem_free(keyframe);
    }
}

nanoem_document_accessory_t *
nanoemDocumentAccessoryCreate(nanoem_document_t *document, nanoem_status_t *status)
{
    nanoem_document_accessory_t *accessory;
    accessory = (nanoem_document_accessory_t *) nanoem_calloc(1, sizeof(*accessory) ,status);
    if (nanoem_is_not_null(accessory)) {
        accessory->base.parent.document = document;
        accessory->factory = document->factory;
    }
    return accessory;
}

void
nanoemDocumentAccessoryParse(nanoem_document_accessory_t *accessory, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_document_t *parent_document;
    nanoem_document_accessory_keyframe_t *keyframe;
    nanoem_rsize_t i, num_objects;
    char name[NANOEM_PMM_ACCESSORY_NAME_MAX], path[NANOEM_PMM_PATH_MAX];
    parent_document = nanoemDocumentAccessoryGetParentDocument(accessory);
    accessory->base.index = nanoemBufferReadByte(buffer, status);
    accessory->name = nanoemDocumentReadFixedStringPMM(parent_document, buffer, name, NANOEM_PMM_ACCESSORY_NAME_MAX, status);
    accessory->path = nanoemDocumentReadFixedStringPMM(parent_document, buffer, path, NANOEM_PMM_PATH_MAX, status);
    accessory->draw_order_index = nanoemBufferReadByte(buffer, status);
    accessory->initial_accessory_keyframe = nanoemDocumentAccessoryKeyframeCreate(accessory, status);
    nanoemDocumentAccessoryKeyframeParse(accessory->initial_accessory_keyframe, buffer, nanoem_false, status);
    num_objects = nanoemBufferReadInt32LittleEndian(buffer, status);
    if (nanoemBufferCanReadLength(buffer, num_objects)) {
        accessory->accessory_keyframes = (nanoem_document_accessory_keyframe_t **) nanoem_calloc(num_objects, sizeof(*accessory->accessory_keyframes), status);
        if (nanoem_is_not_null(accessory->accessory_keyframes)) {
            accessory->num_accessory_keyframes = num_objects;
            for (i = 0; i < num_objects; i++) {
                keyframe = nanoemDocumentAccessoryKeyframeCreate(accessory, status);
                nanoemDocumentAccessoryKeyframeParse(keyframe, buffer, nanoem_true, status);
                if (!nanoem_status_ptr_has_error(status)) {
                    accessory->accessory_keyframes[i] = keyframe;
                }
                else {
                    nanoemDocumentAccessoryKeyframeDestroy(keyframe);
                    accessory->num_accessory_keyframes = i;
                    break;
                }
            }
        }
    }
    else if (num_objects > 0) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_CORRUPTED);
    }
    if (!nanoem_status_ptr_has_error(status)) {
        nanoemDocumentAccessoryUnpackOpacityAndVisible(nanoemBufferReadByte(buffer, status), &accessory->opacity, &accessory->visible);
        accessory->parent_model_index = nanoemBufferReadInt32LittleEndian(buffer, status);
        accessory->parent_model_bone_index = nanoemBufferReadInt32LittleEndian(buffer, status);
        nanoemBufferReadFloat32x3LittleEndian(buffer, &accessory->translation, status);
        accessory->scale_factor = nanoemBufferReadFloat32LittleEndian(buffer, status);
        nanoemBufferReadFloat32x3LittleEndian(buffer, &accessory->orientation, status);
        accessory->is_shadow_enabled = nanoemBufferReadByte(buffer, status) != 0;
        if (parent_document->version > 1) {
            accessory->is_add_blend_enabled = nanoemBufferReadByte(buffer, status) != 0;
        }
        accessory->all_accessory_keyframes_ptr = (nanoem_document_accessory_keyframe_t **) nanoem_calloc(1 + accessory->num_accessory_keyframes, sizeof(*accessory->all_accessory_keyframes_ptr), status);
        accessory->all_accessory_keyframes_ptr[0] = accessory->initial_accessory_keyframe;
        nanoem_crt_memcpy(accessory->all_accessory_keyframes_ptr + 1, accessory->accessory_keyframes, sizeof(*accessory->all_accessory_keyframes_ptr) * accessory->num_accessory_keyframes);
    }
}

nanoem_document_accessory_keyframe_t *const *APIENTRY
nanoemDocumentAccessoryGetAllAccessoryKeyframeObjects(const nanoem_document_accessory_t *accessory, nanoem_rsize_t *num_objects)
{
    if (nanoem_is_not_null(accessory)) {
        *num_objects = accessory->num_accessory_keyframes + 1;
        return accessory->all_accessory_keyframes_ptr;
    }
    else {
        *num_objects = 0;
        return NULL;
    }
}

const nanoem_unicode_string_t *APIENTRY
nanoemDocumentAccessoryGetName(const nanoem_document_accessory_t *accessory)
{
    return nanoem_is_not_null(accessory) ? accessory->name : NULL;
}

const nanoem_unicode_string_t *APIENTRY
nanoemDocumentAccessoryGetPath(const nanoem_document_accessory_t *accessory)
{
    return nanoem_is_not_null(accessory) ? accessory->path : NULL;
}

const nanoem_document_model_t *APIENTRY
nanoemDocumentAccessoryGetParentModelObject(const nanoem_document_accessory_t *accessory)
{
    const nanoem_document_model_t *model = NULL;
    if (nanoem_is_not_null(accessory)) {
        model = nanoemDocumentResolveModelObject(accessory->base.parent.document, accessory->parent_model_index);
    }
    return model;
}

const nanoem_unicode_string_t *APIENTRY
nanoemDocumentAccessoryGetParentModelBoneName(const nanoem_document_accessory_t *accessory)
{
    const nanoem_document_model_t *model;
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(accessory)) {
        model = nanoemDocumentAccessoryGetParentModelObject(accessory);
        name = nanoemDocumentModelResolveBoneName(model, accessory->parent_model_bone_index);
    }
    return name;
}

const nanoem_f32_t *APIENTRY
nanoemDocumentAccessoryGetTranslation(const nanoem_document_accessory_t *accessory)
{
    return nanoem_is_not_null(accessory) ? accessory->translation.values : __nanoem_null_vector3;
}

const nanoem_f32_t *APIENTRY
nanoemDocumentAccessoryGetOrientation(const nanoem_document_accessory_t *accessory)
{
    return nanoem_is_not_null(accessory) ? accessory->orientation.values : __nanoem_null_vector3;
}

nanoem_f32_t APIENTRY
nanoemDocumentAccessoryGetScaleFactor(const nanoem_document_accessory_t *accessory)
{
    return nanoem_is_not_null(accessory) ? accessory->scale_factor : 0.0f;
}

nanoem_f32_t APIENTRY
nanoemDocumentAccessoryGetOpacity(const nanoem_document_accessory_t *accessory)
{
    return nanoem_is_not_null(accessory) ? accessory->opacity : 0.0f;
}

int APIENTRY
nanoemDocumentAccessoryGetDrawOrderIndex(const nanoem_document_accessory_t *accessory)
{
    return nanoem_is_not_null(accessory) ? accessory->draw_order_index : -1;
}

int APIENTRY
nanoemDocumentAccessoryGetIndex(const nanoem_document_accessory_t *accessory)
{
    return nanoem_is_not_null(accessory) ? accessory->base.index : -1;
}

nanoem_bool_t APIENTRY
nanoemDocumentAccessoryIsVisible(const nanoem_document_accessory_t *accessory)
{
    return nanoem_is_not_null(accessory) ? accessory->visible : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemDocumentAccessoryIsShadowEnabled(const nanoem_document_accessory_t *accessory)
{
    return nanoem_is_not_null(accessory) ? accessory->is_shadow_enabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemDocumentAccessoryIsAddBlendEnabled(const nanoem_document_accessory_t *accessory)
{
    return nanoem_is_not_null(accessory) ? accessory->is_add_blend_enabled : nanoem_false;
}

void
nanoemDocumentAccessoryDestroy(nanoem_document_accessory_t *accessory)
{
    nanoem_unicode_string_factory_t *factory;
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(accessory)) {
        factory = accessory->factory;
        nanoemUnicodeStringFactoryDestroyString(factory, accessory->name);
        nanoemUnicodeStringFactoryDestroyString(factory, accessory->path);
        nanoemDocumentAccessoryKeyframeDestroy(accessory->initial_accessory_keyframe);
        num_objects = accessory->num_accessory_keyframes;
        for (i = 0; i < num_objects; i++) {
            nanoemDocumentAccessoryKeyframeDestroy(accessory->accessory_keyframes[i]);
        }
        nanoem_free(accessory->accessory_keyframes);
        nanoem_free(accessory->all_accessory_keyframes_ptr);
        nanoem_free(accessory);
    }
}

nanoem_document_model_bone_keyframe_t *
nanoemDocumentModelBoneKeyframeCreate(nanoem_document_model_t *model, nanoem_status_t *status)
{
    nanoem_document_model_bone_keyframe_t *keyframe;
    keyframe = (nanoem_document_model_bone_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
    if (nanoem_is_not_null(keyframe)) {
        keyframe->parent_model = model;
    }
    return keyframe;
}

static void
nanoemDocumentModelBoneKeyframeInitialize(nanoem_document_model_bone_keyframe_t *keyframe)
{
    nanoem_interpolation_t *interpolation;
    nanoem_rsize_t i, j;
    keyframe->orientation.values[3] = 1.0f;
    for (i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM; i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
         interpolation = &keyframe->interpolation[i];
         for (j = 0; j < sizeof(__nanoem_default_interpolation) / sizeof(__nanoem_default_interpolation[0]); j++) {
             interpolation->u.values[j] = __nanoem_default_interpolation[j];
         }
    }
}

void
nanoemDocumentModelBoneKeyframeParse(nanoem_document_model_bone_keyframe_t *keyframe, nanoem_buffer_t *buffer, nanoem_bool_t include_index, nanoem_status_t *status)
{
    nanoemDocumentBaseKeyframeParse(&keyframe->base, buffer, include_index, status);
    nanoemDocumentParseInterpolation(buffer, &keyframe->interpolation[0], status);
    nanoemDocumentParseInterpolation(buffer, &keyframe->interpolation[1], status);
    nanoemDocumentParseInterpolation(buffer, &keyframe->interpolation[2], status);
    nanoemDocumentParseInterpolation(buffer, &keyframe->interpolation[3], status);
    nanoemBufferReadFloat32x3LittleEndian(buffer, &keyframe->translation, status);
    nanoemBufferReadFloat32x4LittleEndian(buffer, &keyframe->orientation, status);
    keyframe->base.is_selected = nanoemBufferReadByte(buffer, status) != 0;
    if (keyframe->parent_model->base.parent.document->version > 1) {
        keyframe->is_physics_simulation_disabled = nanoemBufferReadByte(buffer, status) != 0;
    }
    if (nanoem_status_ptr_has_error(status)) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_KEYFRAME_CORRUPTED);
    }
}

const nanoem_document_base_keyframe_t * APIENTRY
nanoemDocumentModelBoneKeyframeGetBaseKeyframeObject(const nanoem_document_model_bone_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_document_base_keyframe_t *APIENTRY
nanoemDocumentModelBoneKeyframeGetBaseKeyframeObjectMutable(nanoem_document_model_bone_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

const nanoem_unicode_string_t * APIENTRY
nanoemDocumentModelBoneKeyframeGetName(const nanoem_document_model_bone_keyframe_t *keyframe)
{
    const nanoem_unicode_string_t *name = NULL;
    const nanoem_document_model_bone_keyframe_t *ko;
    nanoem_document_model_t *model;
    nanoem_rsize_t num_keyframes, num_initial_keyframes;
    nanoem_frame_index_t last_keyframe_index = 0, keyframe_index;
    int object_index;
    if (nanoem_is_not_null(keyframe)) {
        model = keyframe->parent_model;
        num_initial_keyframes = model->num_initial_bone_keyframes;
        keyframe_index = keyframe->base.previous_keyframe_index;
        if (keyframe_index > 0) {
            num_keyframes = model->num_bone_keyframes + model->num_initial_bone_keyframes;
            while (keyframe_index > 0 && keyframe_index < num_keyframes && keyframe_index != last_keyframe_index) {
                ko = model->all_bone_keyframes_ptr[keyframe_index];
                last_keyframe_index = keyframe_index;
                keyframe_index = ko->base.previous_keyframe_index;
            }
            if (last_keyframe_index < num_initial_keyframes) {
                object_index = model->all_bone_keyframes_ptr[last_keyframe_index]->base.object_index;
            }
            else {
                object_index = 0;
            }
        }
        else {
            object_index = keyframe->base.object_index;
            if ((nanoem_rsize_t) object_index >= num_initial_keyframes) {
                object_index = 0;
            }
        }
        name = nanoemDocumentModelResolveBoneName(model, object_index);
    }
    return name;
}

const nanoem_f32_t * APIENTRY
nanoemDocumentModelBoneKeyframeGetTranslation(const nanoem_document_model_bone_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->translation.values : __nanoem_null_vector3;
}

const nanoem_f32_t * APIENTRY
nanoemDocumentModelBoneKeyframeGetOrientation(const nanoem_document_model_bone_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->orientation.values : __nanoem_null_vector4;
}

const nanoem_u8_t * APIENTRY
nanoemDocumentModelBoneKeyframeGetInterpolation(const nanoem_document_model_bone_keyframe_t *keyframe, nanoem_motion_bone_keyframe_interpolation_type_t type)
{
    const nanoem_u8_t *parameters = __nanoem_default_interpolation;
    if (nanoem_is_not_null(keyframe) && type >= NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM && type < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM) {
        parameters = keyframe->interpolation[type].u.values;
    }
    return parameters;
}

nanoem_bool_t APIENTRY
nanoemDocumentModelBoneKeyframeIsPhysicsSimulationDisabled(const nanoem_document_model_bone_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->is_physics_simulation_disabled : nanoem_false;
}

void
nanoemDocumentModelBoneKeyframeDestroy(nanoem_document_model_bone_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        nanoem_free(keyframe);
    }
}

nanoem_document_camera_keyframe_t *
nanoemDocumentCameraKeyframeCreate(nanoem_document_camera_t *camera, nanoem_status_t *status)
{
    nanoem_document_camera_keyframe_t *keyframe;
    keyframe = (nanoem_document_camera_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
    if (nanoem_is_not_null(keyframe)) {
        keyframe->parent_camera = camera;
    }
    return keyframe;
}

static void
nanoemDocumentCameraKeyframeInitialize(nanoem_document_camera_keyframe_t *keyframe)
{
    nanoem_interpolation_t *interpolation;
    nanoem_rsize_t i, j;
    keyframe->fov = 30;
    keyframe->distance = -45;
    keyframe->is_perspective_view = nanoem_true;
    keyframe->look_at.values[1] = 10;
    keyframe->parent_model_index = -1;
    for (i = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM; i < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
         interpolation = &keyframe->interpolation[i];
         for (j = 0; j < sizeof(__nanoem_default_interpolation) / sizeof(__nanoem_default_interpolation[0]); j++) {
             interpolation->u.values[j] = __nanoem_default_interpolation[j];
         }
    }
}

void
nanoemDocumentCameraKeyframeParse(nanoem_document_camera_keyframe_t *keyframe, nanoem_buffer_t *buffer, nanoem_bool_t include_index, nanoem_status_t *status)
{
    nanoemDocumentBaseKeyframeParse(&keyframe->base, buffer, include_index, status);
    keyframe->distance = nanoemBufferReadFloat32LittleEndian(buffer, status);
    nanoemBufferReadFloat32x3LittleEndian(buffer, &keyframe->look_at, status);
    nanoemBufferReadFloat32x3LittleEndian(buffer, &keyframe->angle, status);
    if (keyframe->parent_camera->base.parent.document->version > 1) {
        keyframe->parent_model_index = nanoemBufferReadInt32LittleEndian(buffer, status);
        keyframe->parent_model_bone_index = nanoemBufferReadInt32LittleEndian(buffer, status);
    }
    nanoemDocumentParseInterpolation(buffer, &keyframe->interpolation[0], status);
    nanoemDocumentParseInterpolation(buffer, &keyframe->interpolation[1], status);
    nanoemDocumentParseInterpolation(buffer, &keyframe->interpolation[2], status);
    nanoemDocumentParseInterpolation(buffer, &keyframe->interpolation[3], status);
    nanoemDocumentParseInterpolation(buffer, &keyframe->interpolation[4], status);
    nanoemDocumentParseInterpolation(buffer, &keyframe->interpolation[5], status);
    keyframe->is_perspective_view = nanoemBufferReadByte(buffer, status) == 0;
    keyframe->fov = nanoemBufferReadInt32LittleEndian(buffer, status);
    keyframe->base.is_selected = nanoemBufferReadByte(buffer, status) != 0;
    if (nanoem_status_ptr_has_error(status)) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_CAMERA_KEYFRAME_CORRUPTED);
    }
}

const nanoem_document_base_keyframe_t * APIENTRY
nanoemDocumentCameraKeyframeGetBaseKeyframeObject(const nanoem_document_camera_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_document_base_keyframe_t *APIENTRY
nanoemDocumentCameraKeyframeGetBaseKeyframeObjectMutable(nanoem_document_camera_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

const nanoem_document_model_t * APIENTRY
nanoemDocumentCameraKeyframeGetParentModelObject(const nanoem_document_camera_keyframe_t *keyframe)
{
    const nanoem_document_model_t *model = NULL;
    if (nanoem_is_not_null(keyframe)) {
        model = nanoemDocumentResolveModelObject(keyframe->parent_camera->base.parent.document, keyframe->parent_model_index);
    }
    return model;
}

const nanoem_unicode_string_t * APIENTRY
nanoemDocumentCameraKeyframeGetParentModelBoneName(const nanoem_document_camera_keyframe_t *keyframe)
{
    const nanoem_document_model_t *model;
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(keyframe)) {
        model = nanoemDocumentCameraKeyframeGetParentModelObject(keyframe);
        name = nanoemDocumentModelResolveBoneName(model, keyframe->parent_model_bone_index);
    }
    return name;
}

const nanoem_f32_t * APIENTRY
nanoemDocumentCameraKeyframeGetLookAt(const nanoem_document_camera_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->look_at.values : __nanoem_null_vector3;
}

const nanoem_f32_t * APIENTRY
nanoemDocumentCameraKeyframeGetAngle(const nanoem_document_camera_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->angle.values : __nanoem_null_vector3;
}

const nanoem_u8_t * APIENTRY
nanoemDocumentCameraKeyframeGetInterpolation(const nanoem_document_camera_keyframe_t *keyframe, nanoem_motion_camera_keyframe_interpolation_type_t type)
{
    const nanoem_u8_t *parameters = __nanoem_default_interpolation;
    if (nanoem_is_not_null(keyframe) && type >= NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM && type < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM) {
        parameters = keyframe->interpolation[type].u.values;
    }
    return parameters;
}

nanoem_f32_t APIENTRY
nanoemDocumentCameraKeyframeGetDistance(const nanoem_document_camera_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->distance : 0.0f;
}

int APIENTRY
nanoemDocumentCameraKeyframeGetFov(const nanoem_document_camera_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->fov : 0;
}

nanoem_bool_t APIENTRY
nanoemDocumentCameraKeyframeIsPerspectiveView(const nanoem_document_camera_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->is_perspective_view : nanoem_false;
}

void
nanoemDocumentCameraKeyframeDestroy(nanoem_document_camera_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        nanoem_free(keyframe);
    }
}

nanoem_document_camera_t *
nanoemDocumentCameraCreate(nanoem_document_t *document, nanoem_status_t *status)
{
    nanoem_document_camera_t *camera;
    camera = (nanoem_document_camera_t *) nanoem_calloc(1, sizeof(*camera), status);
    if (nanoem_is_not_null(camera)) {
        camera->base.parent.document = document;
    }
    return camera;
}

static void
nanoemDocumentCameraInitialize(nanoem_document_camera_t *camera)
{
    camera->is_perspective = nanoem_true;
    camera->look_at.values[1] = 10.0f;
    camera->is_perspective = nanoem_true;
    camera->position.values[1] = 10.0f;
    camera->position.values[2] = 45.0f;
}

void
nanoemDocumentCameraParse(nanoem_document_camera_t *camera, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_document_camera_keyframe_t *keyframe;
    nanoem_rsize_t i, num_objects;
    camera->initial_camera_keyframe = nanoemDocumentCameraKeyframeCreate(camera, status);
    nanoemDocumentCameraKeyframeParse(camera->initial_camera_keyframe, buffer, nanoem_false, status);
    num_objects = nanoemBufferReadInt32LittleEndian(buffer, status);
    if (nanoemBufferCanReadLength(buffer, num_objects)) {
        camera->camera_keyframes = (nanoem_document_camera_keyframe_t **) nanoem_calloc(num_objects, sizeof(*camera->camera_keyframes), status);
        if (nanoem_is_not_null(camera->camera_keyframes)) {
            camera->num_camera_keyframes = num_objects;
            for (i = 0; i < num_objects; i++) {
                keyframe = nanoemDocumentCameraKeyframeCreate(camera, status);
                nanoemDocumentCameraKeyframeParse(keyframe, buffer, nanoem_true, status);
                if (!nanoem_status_ptr_has_error(status)) {
                    camera->camera_keyframes[i] = keyframe;
                }
                else {
                    nanoemDocumentCameraKeyframeDestroy(keyframe);
                    camera->num_camera_keyframes = i;
                    break;
                }
            }
        }
    }
    else if (num_objects > 0) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_CAMERA_CORRUPTED);
    }
    if (!nanoem_status_ptr_has_error(status)) {
        nanoemBufferReadFloat32x3LittleEndian(buffer, &camera->look_at, status);
        nanoemBufferReadFloat32x3LittleEndian(buffer, &camera->position, status);
        nanoemBufferReadFloat32x3LittleEndian(buffer, &camera->angle, status);
        camera->is_perspective = nanoemBufferReadByte(buffer, status) == 0;
        camera->all_camera_keyframes_ptr = (nanoem_document_camera_keyframe_t **) nanoem_calloc(1 + camera->num_camera_keyframes, sizeof(*camera->all_camera_keyframes_ptr), status);
        camera->all_camera_keyframes_ptr[0] = camera->initial_camera_keyframe;
        nanoem_crt_memcpy(camera->all_camera_keyframes_ptr + 1, camera->camera_keyframes, sizeof(*camera->all_camera_keyframes_ptr) * camera->num_camera_keyframes);
    }
}

nanoem_document_camera_keyframe_t *const *APIENTRY
nanoemDocumentCameraGetAllCameraKeyframeObjects(const nanoem_document_camera_t *camera, nanoem_rsize_t *num_objects)
{
    if (nanoem_is_not_null(camera) && camera->camera_keyframes) {
        *num_objects = camera->num_camera_keyframes + 1;
        return camera->all_camera_keyframes_ptr;
    }
    else {
        *num_objects = 0;
        return NULL;
    }
}

const nanoem_f32_t *APIENTRY
nanoemDocumentCameraGetLookAt(const nanoem_document_camera_t *camera)
{
    return nanoem_is_not_null(camera) ? camera->look_at.values : __nanoem_null_vector3;
}

const nanoem_f32_t *APIENTRY
nanoemDocumentCameraGetAngle(const nanoem_document_camera_t *camera)
{
    return nanoem_is_not_null(camera) ? camera->angle.values : __nanoem_null_vector3;
}

const nanoem_f32_t *APIENTRY
nanoemDocumentCameraGetPosition(const nanoem_document_camera_t *camera)
{
    return nanoem_is_not_null(camera) ? camera->position.values : __nanoem_null_vector3;
}

nanoem_f32_t APIENTRY
nanoemDocumentCameraGetDistance(const nanoem_document_camera_t *camera)
{
    return nanoem_is_not_null(camera) ? camera->position.values[2] - camera->look_at.values[2] : 0;
}

nanoem_f32_t APIENTRY
nanoemDocumentCameraGetFov(const nanoem_document_camera_t *camera)
{
    return nanoem_is_not_null(camera) ? camera->base.parent.document->camera_fov : 0;
}

nanoem_bool_t APIENTRY
nanoemDocumentCameraIsPerspectiveEnabled(const nanoem_document_camera_t *camera)
{
    return nanoem_is_not_null(camera) ? camera->is_perspective : nanoem_false;
}

void
nanoemDocumentCameraDestroy(nanoem_document_camera_t *camera)
{
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(camera)) {
        nanoemDocumentCameraKeyframeDestroy(camera->initial_camera_keyframe);
        num_objects = camera->num_camera_keyframes;
        for (i = 0; i < num_objects; i++) {
            nanoemDocumentCameraKeyframeDestroy(camera->camera_keyframes[i]);
        }
        nanoem_free(camera->camera_keyframes);
        nanoem_free(camera->all_camera_keyframes_ptr);
        nanoem_free(camera);
    }
}

nanoem_document_model_constraint_state_t *
nanoemDocumentModelConstraintStateCreate(nanoem_document_model_t *model, nanoem_status_t *status)
{
    nanoem_document_model_constraint_state_t *state;
    state = (nanoem_document_model_constraint_state_t *) nanoem_calloc(1, sizeof(*state), status);
    if (nanoem_is_not_null(state)) {
        state->enabled = nanoem_true;
        state->base.parent.model = model;
    }
    return state;
}

void
nanoemDocumentModelConstraintStateParse(nanoem_document_model_constraint_state_t *state, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    state->enabled = nanoemBufferReadByte(buffer, status) != 0;
}

const nanoem_unicode_string_t * APIENTRY
nanoemDocumentModelConstraintStateGetName(const nanoem_document_model_constraint_state_t *state)
{
    const nanoem_document_model_t *parent_model;
    const nanoem_unicode_string_t *name = NULL;
    int index, bone_index;
    if (nanoem_is_not_null(state)) {
        index = state->base.index;
        parent_model = state->base.parent.model;
        if ((nanoem_rsize_t) index < parent_model->num_constraint_bones) {
            bone_index = parent_model->constraint_bone_indices[index];
            name = nanoemDocumentModelResolveBoneName(parent_model, bone_index);
        }
    }
    return name;
}

nanoem_bool_t APIENTRY
nanoemDocumentModelConstraintStateIsEnabled(const nanoem_document_model_constraint_state_t *state)
{
    return nanoem_is_not_null(state) ? state->enabled : nanoem_false;
}

void
nanoemDocumentModelConstraintStateDestroy(nanoem_document_model_constraint_state_t *state)
{
    if (nanoem_is_not_null(state)) {
        nanoem_free(state);
    }
}

nanoem_document_gravity_keyframe_t *
nanoemDocumentGravityKeyframeCreate(nanoem_document_gravity_t *gravity, nanoem_status_t *status)
{
    nanoem_document_gravity_keyframe_t *keyframe;
    keyframe = (nanoem_document_gravity_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
    if (nanoem_is_not_null(keyframe)) {
        keyframe->parent_gravity = gravity;
    }
    return keyframe;
}

static void
nanoemDocumentGravityKeyframeInitialize(nanoem_document_gravity_keyframe_t *keyframe)
{
    keyframe->acceleration = 9.8f;
    keyframe->direction.values[1] = -1;
    keyframe->noise = 10;
}

void
nanoemDocumentGravityKeyframeParse(nanoem_document_gravity_keyframe_t *keyframe, nanoem_buffer_t *buffer, nanoem_bool_t include_index, nanoem_status_t *status)
{
    nanoemDocumentBaseKeyframeParse(&keyframe->base, buffer, include_index, status);
    keyframe->is_noise_enabled = nanoemBufferReadByte(buffer, status) != 0;
    keyframe->noise = nanoemBufferReadInt32LittleEndian(buffer, status);
    keyframe->acceleration = nanoemBufferReadFloat32LittleEndian(buffer, status);
    nanoemBufferReadFloat32x3LittleEndian(buffer, &keyframe->direction, status);
    keyframe->base.is_selected = nanoemBufferReadByte(buffer, status) != 0;
    if (nanoem_status_ptr_has_error(status)) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_GRAVITY_KEYFRAME_CORRUPTED);
    }
}

const nanoem_document_base_keyframe_t * APIENTRY
nanoemDocumentGravityKeyframeGetBaseKeyframeObject(const nanoem_document_gravity_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_document_base_keyframe_t *APIENTRY
nanoemDocumentGravityKeyframeGetBaseKeyframeObjectMutable(nanoem_document_gravity_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

const nanoem_f32_t *APIENTRY
nanoemDocumentGravityKeyframeGetDirection(const nanoem_document_gravity_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->direction.values : __nanoem_null_vector3;
}

nanoem_f32_t APIENTRY
nanoemDocumentGravityKeyframeGetAcceleration(const nanoem_document_gravity_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->acceleration : 0.0f;
}

int APIENTRY
nanoemDocumentGravityKeyframeGetNoise(const nanoem_document_gravity_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->noise : 0;
}

nanoem_bool_t APIENTRY
nanoemDocumentGravityKeyframeIsNoiseEnabled(const nanoem_document_gravity_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->is_noise_enabled : nanoem_false;
}

void
nanoemDocumentGravityKeyframeDestroy(nanoem_document_gravity_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        nanoem_free(keyframe);
    }
}

nanoem_document_gravity_t *
nanoemDocumentGravityCreate(nanoem_document_t *document, nanoem_status_t *status)
{
    nanoem_document_gravity_t *gravity;
    gravity = (nanoem_document_gravity_t *) nanoem_calloc(1, sizeof(*gravity), status);
    if (nanoem_is_not_null(gravity)) {
        gravity->base.parent.document = document;
    }
    return gravity;
}

static void
nanoemDocumentGravityInitialize(nanoem_document_gravity_t *gravity)
{
    gravity->acceleration = 9.8f;
    gravity->direction.values[1] = -1.0f;
    gravity->noise = 10;
}

void
nanoemDocumentGravityParse(nanoem_document_gravity_t *gravity, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_document_gravity_keyframe_t *keyframe;
    nanoem_rsize_t i, num_objects;
    gravity->acceleration = nanoemBufferReadFloat32LittleEndian(buffer, status);
    gravity->noise = nanoemBufferReadInt32LittleEndian(buffer, status);
    nanoemBufferReadFloat32x3LittleEndian(buffer, &gravity->direction, status);
    gravity->is_noise_enabled = nanoemBufferReadByte(buffer, status) != 0;
    gravity->initial_gravity_keyframe = nanoemDocumentGravityKeyframeCreate(gravity, status);
    nanoemDocumentGravityKeyframeParse(gravity->initial_gravity_keyframe, buffer, nanoem_false, status);
    num_objects = nanoemBufferReadInt32LittleEndian(buffer, status);
    if (nanoemBufferCanReadLength(buffer, num_objects)) {
        gravity->gravity_keyframes = (nanoem_document_gravity_keyframe_t **) nanoem_calloc(num_objects, sizeof(*gravity->gravity_keyframes), status);
        if (nanoem_is_not_null(gravity->gravity_keyframes)) {
            gravity->num_gravity_keyframes = num_objects;
            for (i = 0; i < num_objects; i++) {
                keyframe = nanoemDocumentGravityKeyframeCreate(gravity, status);
                nanoemDocumentGravityKeyframeParse(keyframe, buffer, nanoem_true, status);
                if (!nanoem_status_ptr_has_error(status)) {
                    gravity->gravity_keyframes[i] = keyframe;
                }
                else {
                    nanoemDocumentGravityKeyframeDestroy(keyframe);
                    gravity->num_gravity_keyframes = i;
                    break;
                }
            }
            if (!nanoem_status_ptr_has_error(status)) {
                gravity->all_gravity_keyframes_ptr = (nanoem_document_gravity_keyframe_t **) nanoem_calloc(1 + gravity->num_gravity_keyframes, sizeof(*gravity->all_gravity_keyframes_ptr), status);
                gravity->all_gravity_keyframes_ptr[0] = gravity->initial_gravity_keyframe;
                nanoem_crt_memcpy(gravity->all_gravity_keyframes_ptr + 1, gravity->gravity_keyframes, sizeof(*gravity->all_gravity_keyframes_ptr) * gravity->num_gravity_keyframes);
            }
        }
    }
    else if (num_objects > 0) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_GRAVITY_CORRUPTED);
    }
}

nanoem_document_gravity_keyframe_t *const *APIENTRY
nanoemDocumentGravityGetAllGravityKeyframeObjects(const nanoem_document_gravity_t *gravity, nanoem_rsize_t *num_objects)
{
    if (nanoem_is_not_null(gravity) && gravity->gravity_keyframes) {
        *num_objects = gravity->num_gravity_keyframes + 1;
        return gravity->all_gravity_keyframes_ptr;
    }
    else {
        *num_objects = 0;
        return NULL;
    }
}

const nanoem_f32_t *APIENTRY
nanoemDocumentGravityGetDirection(const nanoem_document_gravity_t *gravity)
{
    return nanoem_is_not_null(gravity) ? gravity->direction.values : __nanoem_null_vector3;
}

nanoem_f32_t APIENTRY
nanoemDocumentGravityGetAcceleration(const nanoem_document_gravity_t *gravity)
{
    return nanoem_is_not_null(gravity) ? gravity->acceleration : 0;
}

int APIENTRY
nanoemDocumentGravityGetNoise(const nanoem_document_gravity_t *gravity)
{
    return nanoem_is_not_null(gravity) ? gravity->noise : 0;
}

nanoem_bool_t APIENTRY
nanoemDocumentGravityIsNoiseEnabled(const nanoem_document_gravity_t *gravity)
{
    return nanoem_is_not_null(gravity) ? gravity->is_noise_enabled : nanoem_false;
}

void
nanoemDocumentGravityDestroy(nanoem_document_gravity_t *gravity)
{
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(gravity)) {
        nanoemDocumentGravityKeyframeDestroy(gravity->initial_gravity_keyframe);
        num_objects = gravity->num_gravity_keyframes;
        for (i = 0; i < num_objects; i++) {
            nanoemDocumentGravityKeyframeDestroy(gravity->gravity_keyframes[i]);
        }
        nanoem_free(gravity->all_gravity_keyframes_ptr);
        nanoem_free(gravity->gravity_keyframes);
        nanoem_free(gravity);
    }
}

nanoem_document_light_keyframe_t *
nanoemDocumentLightKeyframeCreate(nanoem_document_light_t *light, nanoem_status_t *status)
{
    nanoem_document_light_keyframe_t *keyframe;
    keyframe = (nanoem_document_light_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
    if (nanoem_is_not_null(keyframe)) {
        keyframe->parent_light = light;
    }
    return keyframe;
}

static void
nanoemDocumentLightKeyframeInitialize(nanoem_document_light_keyframe_t *keyframe)
{
    static const nanoem_f32_t color[] = { 0.6f, 0.6f, 0.6f, 0 },
            direction[] = { -0.5f, -1.0f, 0.5f, 0 };
    nanoem_crt_memcpy(keyframe->color.values, color, sizeof(color));
    nanoem_crt_memcpy(keyframe->direction.values, direction, sizeof(direction));
}

void
nanoemDocumentLightKeyframeParse(nanoem_document_light_keyframe_t *keyframe, nanoem_buffer_t *buffer, nanoem_bool_t include_index, nanoem_status_t *status)
{
    nanoemDocumentBaseKeyframeParse(&keyframe->base, buffer, include_index, status);
    nanoemBufferReadFloat32x3LittleEndian(buffer, &keyframe->color, status);
    nanoemBufferReadFloat32x3LittleEndian(buffer, &keyframe->direction, status);
    keyframe->base.is_selected = nanoemBufferReadByte(buffer, status) != 0;
    if (nanoem_status_ptr_has_error(status)) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_LIGHT_KEYFRAME_CORRUPTED);
    }
}

const nanoem_document_base_keyframe_t * APIENTRY
nanoemDocumentLightKeyframeGetBaseKeyframeObject(const nanoem_document_light_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_document_base_keyframe_t *APIENTRY
nanoemDocumentLightKeyframeGetBaseKeyframeObjectMutable(nanoem_document_light_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

const nanoem_f32_t * APIENTRY
nanoemDocumentLightKeyframeGetColor(const nanoem_document_light_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->color.values : __nanoem_null_vector3;
}

const nanoem_f32_t * APIENTRY
nanoemDocumentLightKeyframeGetDirection(const nanoem_document_light_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->direction.values : __nanoem_null_vector3;
}

void
nanoemDocumentLightKeyframeDestroy(nanoem_document_light_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        nanoem_free(keyframe);
    }
}

nanoem_document_light_t *
nanoemDocumentLightCreate(nanoem_document_t *document, nanoem_status_t *status)
{
    nanoem_document_light_t *light;
    light = (nanoem_document_light_t *) nanoem_calloc(1, sizeof(*light), status);
    if (nanoem_is_not_null(light)) {
        light->base.parent.document = document;
    }
    return light;
}

static void
nanoemDocumentLightInitialize(nanoem_document_light_t *light)
{
    static const nanoem_f32_t color[] = { 0.6f, 0.6f, 0.6f, 0 },
            direction[] = { -0.5f, -1.0f, 0.5f, 0 };
    nanoem_crt_memcpy(light->color.values, color, sizeof(color));
    nanoem_crt_memcpy(light->direction.values, direction, sizeof(direction));
}

void
nanoemDocumentLightParse(nanoem_document_light_t *light, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_document_light_keyframe_t *keyframe;
    nanoem_rsize_t i, num_objects;
    light->initial_light_keyframe = nanoemDocumentLightKeyframeCreate(light, status);
    nanoemDocumentLightKeyframeParse(light->initial_light_keyframe, buffer, nanoem_false, status);
    num_objects = nanoemBufferReadInt32LittleEndian(buffer, status);
    if (nanoemBufferCanReadLength(buffer, num_objects)) {
        light->light_keyframes = (nanoem_document_light_keyframe_t **) nanoem_calloc(num_objects, sizeof(*light->light_keyframes), status);
        if (nanoem_is_not_null(light->light_keyframes)) {
            light->num_light_keyframes = num_objects;
            for (i = 0; i < num_objects; i++) {
                keyframe = nanoemDocumentLightKeyframeCreate(light, status);
                nanoemDocumentLightKeyframeParse(keyframe, buffer, nanoem_true, status);
                if (!nanoem_status_ptr_has_error(status)) {
                    light->light_keyframes[i] = keyframe;
                }
                else {
                    nanoemDocumentLightKeyframeDestroy(keyframe);
                    light->num_light_keyframes = i;
                    break;
                }
            }
        }
    }
    else if (num_objects > 0) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_LIGHT_CORRUPTED);
    }
    if (!nanoem_status_ptr_has_error(status)) {
        nanoemBufferReadFloat32x3LittleEndian(buffer, &light->color, status);
        nanoemBufferReadFloat32x3LittleEndian(buffer, &light->direction, status);
        light->all_light_keyframes_ptr = (nanoem_document_light_keyframe_t **) nanoem_calloc(1 + light->num_light_keyframes, sizeof(*light->all_light_keyframes_ptr), status);
        light->all_light_keyframes_ptr[0] = light->initial_light_keyframe;
        nanoem_crt_memcpy(light->all_light_keyframes_ptr + 1, light->light_keyframes, sizeof(*light->all_light_keyframes_ptr) * light->num_light_keyframes);
    }
}

nanoem_document_light_keyframe_t *const *APIENTRY
nanoemDocumentLightGetAllLightKeyframeObjects(const nanoem_document_light_t *light, nanoem_rsize_t *num_objects)
{
    if (nanoem_is_not_null(light) && light->light_keyframes) {
        *num_objects = light->num_light_keyframes + 1;
        return light->all_light_keyframes_ptr;
    }
    else {
        *num_objects = 0;
        return NULL;
    }
}

const nanoem_f32_t *APIENTRY
nanoemDocumentLightGetColor(const nanoem_document_light_t *light)
{
    return nanoem_is_not_null(light) ? light->color.values : __nanoem_null_vector3;
}

const nanoem_f32_t *APIENTRY
nanoemDocumentLightGetDirection(const nanoem_document_light_t *light)
{
    return nanoem_is_not_null(light) ? light->direction.values : __nanoem_null_vector3;
}

void
nanoemDocumentLightDestroy(nanoem_document_light_t *light)
{
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(light)) {
        nanoemDocumentLightKeyframeDestroy(light->initial_light_keyframe);
        num_objects = light->num_light_keyframes;
        for (i = 0; i < num_objects; i++) {
            nanoemDocumentLightKeyframeDestroy(light->light_keyframes[i]);
        }
        nanoem_free(light->light_keyframes);
        nanoem_free(light->all_light_keyframes_ptr);
        nanoem_free(light);
    }
}

nanoem_document_model_keyframe_t *
nanoemDocumentModelKeyframeCreate(nanoem_document_model_t *model, nanoem_status_t *status)
{
    nanoem_document_model_keyframe_t *keyframe;
    keyframe = (nanoem_document_model_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
    if (nanoem_is_not_null(keyframe)) {
        keyframe->parent_model = model;
    }
    return keyframe;
}

void
nanoemDocumentModelKeyframeInitialize(nanoem_document_model_keyframe_t *keyframe)
{
    keyframe->visible = nanoem_true;
}

void
nanoemDocumentModelKeyframeParse(nanoem_document_model_keyframe_t *keyframe, nanoem_buffer_t *buffer, nanoem_bool_t include_index, nanoem_status_t *status)
{
    const nanoem_document_model_t *parent_model = keyframe->parent_model;
    const nanoem_document_t *document = parent_model->base.parent.document;
    nanoem_document_model_constraint_state_t *state;
    nanoem_document_outside_parent_t *outside_parent;
    nanoem_rsize_t i, num_objects;
    nanoemDocumentBaseKeyframeParse(&keyframe->base, buffer, include_index, status);
    keyframe->visible = nanoemBufferReadByte(buffer, status) != 0;
    num_objects = parent_model->num_constraint_bones;
    if (nanoemBufferCanReadLength(buffer, num_objects)) {
        keyframe->constraint_states = (nanoem_document_model_constraint_state_t **) nanoem_calloc(num_objects, sizeof(*keyframe->constraint_states), status);
        if (nanoem_is_not_null(keyframe->constraint_states)) {
            keyframe->num_constraint_states = num_objects;
            for (i = 0; i < num_objects; i++) {
                state = keyframe->constraint_states[i] = nanoemDocumentModelConstraintStateCreate(keyframe->parent_model, status);
                state->base.index = i;
                state->enabled = nanoemBufferReadByte(buffer, status) != 0;
            }
        }
    }
    else if (num_objects > 0) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_KEYFRAME_CORRUPTED);
    }
    if (document->version > 1) {
        num_objects = parent_model->num_outside_parent_subject_bones;
        if (nanoemBufferCanReadLength(buffer, num_objects)) {
            keyframe->outside_parents = (nanoem_document_outside_parent_t **) nanoem_calloc(num_objects, sizeof(*keyframe->outside_parents), status);
            if (nanoem_is_not_null(keyframe->outside_parents)) {
                keyframe->num_outside_parents = num_objects;
                for (i = 0; i < num_objects; i++) {
                    outside_parent = nanoemDocumentOutsideParentCreate(document, status);
                    nanoemDocumentOutsideParentParse(outside_parent, buffer, status);
                    if (!nanoem_status_ptr_has_error(status)) {
                        keyframe->outside_parents[i] = outside_parent;
                        outside_parent->base.index = (int) i;
                    }
                    else {
                        nanoemDocumentOutsideParentDestroy(outside_parent);
                        keyframe->num_outside_parents = i;
                        break;
                    }
                }
            }
        }
        else if (num_objects > 0) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_KEYFRAME_CORRUPTED);
        }
    }
    keyframe->base.is_selected = nanoemBufferReadByte(buffer, status) != 0;
}

nanoem_document_model_constraint_state_t *const * APIENTRY
nanoemDocumentModelKeyframeGetAllModelConstraintStateObjects(const nanoem_document_model_keyframe_t *keyframe, nanoem_rsize_t *num_objects)
{
    if (nanoem_is_not_null(keyframe)) {
        *num_objects = keyframe->num_constraint_states;
        return keyframe->constraint_states;
    }
    else {
        *num_objects = 0;
        return NULL;
    }
}

nanoem_document_outside_parent_t *const * APIENTRY
nanoemDocumentModelKeyframeGetAllOutsideParentObjects(const nanoem_document_model_keyframe_t *keyframe, nanoem_rsize_t *num_objects)
{
    if (nanoem_is_not_null(keyframe)) {
        *num_objects = keyframe->num_outside_parents;
        return keyframe->outside_parents;
    }
    else {
        *num_objects = 0;
        return NULL;
    }
}

const nanoem_document_base_keyframe_t * APIENTRY
nanoemDocumentModelKeyframeGetBaseKeyframeObject(const nanoem_document_model_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_document_base_keyframe_t *APIENTRY
nanoemDocumentModelKeyframeGetBaseKeyframeObjectMutable(nanoem_document_model_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_bool_t APIENTRY
nanoemDocumentModelKeyframeIsVisible(const nanoem_document_model_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->visible : nanoem_false;
}

void
nanoemDocumentModelKeyframeDestroy(nanoem_document_model_keyframe_t *keyframe)
{
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(keyframe)) {
        num_objects = keyframe->num_outside_parents;
        for (i = 0; i < num_objects; i++) {
            nanoemDocumentOutsideParentDestroy(keyframe->outside_parents[i]);
        }
        nanoem_free(keyframe->outside_parents);
        num_objects = keyframe->num_constraint_states;
        for (i = 0; i < num_objects; i++) {
            nanoemDocumentModelConstraintStateDestroy(keyframe->constraint_states[i]);
        }
        nanoem_free(keyframe->constraint_states);
        nanoem_free(keyframe);
    }
}

nanoem_document_model_morph_keyframe_t *
nanoemDocumentModelMorphKeyframeCreate(nanoem_document_model_t *model, nanoem_status_t *status)
{
    nanoem_document_model_morph_keyframe_t *keyframe;
    keyframe = (nanoem_document_model_morph_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
    if (nanoem_is_not_null(keyframe)) {
        keyframe->parent_model = model;
    }
    return keyframe;
}

void
nanoemDocumentModelMorphKeyframeParse(nanoem_document_model_morph_keyframe_t *keyframe, nanoem_buffer_t *buffer, nanoem_bool_t include_index, nanoem_status_t *status)
{
    nanoemDocumentBaseKeyframeParse(&keyframe->base, buffer, include_index, status);
    keyframe->weight = nanoemBufferReadFloat32LittleEndian(buffer, status);
    keyframe->base.is_selected = nanoemBufferReadByte(buffer, status) != 0;
    if (nanoem_status_ptr_has_error(status)) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_KEYFRAME_CORRUPTED);
    }
}

const nanoem_document_base_keyframe_t * APIENTRY
nanoemDocumentModelMorphKeyframeGetBaseKeyframeObject(const nanoem_document_model_morph_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_document_base_keyframe_t *APIENTRY
nanoemDocumentModelMorphKeyframeGetBaseKeyframeObjectMutable(nanoem_document_model_morph_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

const nanoem_unicode_string_t * APIENTRY
nanoemDocumentModelMorphKeyframeGetName(const nanoem_document_model_morph_keyframe_t *keyframe)
{
    const nanoem_unicode_string_t *name = NULL;
    const nanoem_document_model_morph_keyframe_t *ko;
    nanoem_document_model_t *model;
    nanoem_rsize_t num_keyframes, num_initial_keyframes;
    nanoem_frame_index_t last_keyframe_index = 0, keyframe_index;
    int object_index;
    if (nanoem_is_not_null(keyframe)) {
        model = keyframe->parent_model;
        num_initial_keyframes = model->num_initial_morph_keyframes;
        keyframe_index = keyframe->base.previous_keyframe_index;
        if (keyframe_index > 0) {
            num_keyframes = model->num_morph_keyframes + num_initial_keyframes;
            while (keyframe_index > 0 && keyframe_index < num_keyframes && keyframe_index != last_keyframe_index) {
                ko = model->all_morph_keyframes_ptr[keyframe_index];
                last_keyframe_index = keyframe_index;
                keyframe_index = ko->base.previous_keyframe_index;
            }
            if (last_keyframe_index < num_initial_keyframes) {
                object_index = model->all_morph_keyframes_ptr[last_keyframe_index]->base.object_index;
            }
            else {
                object_index = 0;
            }
        }
        else {
            object_index = keyframe->base.object_index;
            if ((nanoem_rsize_t) object_index >= num_initial_keyframes) {
                object_index = 0;
            }
        }
        name = nanoemDocumentModelResolveMorphName(model, object_index);
    }
    return name;
}

nanoem_f32_t APIENTRY
nanoemDocumentModelMorphKeyframeGetWeight(const nanoem_document_model_morph_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->weight : 0.0f;
}

void
nanoemDocumentModelMorphKeyframeDestroy(nanoem_document_model_morph_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        nanoem_free(keyframe);
    }
}

nanoem_document_model_bone_state_t *
nanoemDocumentModelBoneStateCreate(nanoem_document_model_t *model, nanoem_status_t *status)
{
    nanoem_document_model_bone_state_t *state;
    state = (nanoem_document_model_bone_state_t *) nanoem_calloc(1, sizeof(*state), status);
    if (nanoem_is_not_null(state)) {
        state->base.parent.model = model;
    }
    return state;
}

void
nanoemDocumentModelBoneStateParse(nanoem_document_model_bone_state_t *state, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_document_t *parent_document = state->base.parent.model->base.parent.document;
    nanoemBufferReadFloat32x3LittleEndian(buffer, &state->translation, status);
    nanoemBufferReadFloat32x4LittleEndian(buffer, &state->orientation, status);
    if (parent_document->version > 1) {
        state->dirty = nanoemBufferReadByte(buffer, status) != 0;
        state->disable_physics_simulation = nanoemBufferReadByte(buffer, status) != 0;
        state->num_tracks_selected = nanoemBufferReadByte(buffer, status);
    }
    else if (parent_document->version == 1) {
        nanoemBufferReadInt32LittleEndian(buffer, status);
        state->dirty = nanoemBufferReadByte(buffer, status);
        state->num_tracks_selected = nanoemBufferReadByte(buffer, status);
    }
    if (nanoem_status_ptr_has_error(status)) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_STATE_CORRUPTED);
    }
}

const nanoem_unicode_string_t * APIENTRY
nanoemDocumentModelBoneStateGetName(const nanoem_document_model_bone_state_t *state)
{
    return nanoem_is_not_null(state) ? nanoemDocumentModelResolveBoneName(state->base.parent.model, state->base.index) : NULL;
}

const nanoem_f32_t *APIENTRY
nanoemDocumentModelBoneStateGetTranslation(const nanoem_document_model_bone_state_t *state)
{
    return nanoem_is_not_null(state) ? state->translation.values : __nanoem_null_vector3;
}

const nanoem_f32_t *APIENTRY
nanoemDocumentModelBoneStateGetOrientation(const nanoem_document_model_bone_state_t *state)
{
    return nanoem_is_not_null(state) ? state->orientation.values : __nanoem_null_vector4;
}

int APIENTRY
nanoemDocumentModelBoneStateCountSelectedTracks(const nanoem_document_model_bone_state_t *state)
{
    return nanoem_is_not_null(state) ? state->num_tracks_selected : 0;
}

nanoem_bool_t APIENTRY
nanoemDocumentModelBoneStateIsDirty(const nanoem_document_model_bone_state_t *state)
{
    return nanoem_is_not_null(state) ? state->dirty : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemDocumentModelBoneStateIsPhysicsSimulationDisabled(const nanoem_document_model_bone_state_t *state)
{
    return nanoem_is_not_null(state) ? state->disable_physics_simulation : nanoem_false;
}

void
nanoemDocumentModelBoneStateDestroy(nanoem_document_model_bone_state_t *state)
{
    if (nanoem_is_not_null(state)) {
        nanoem_free(state);
    }
}

nanoem_document_model_morph_state_t *
nanoemDocumentModelMorphStateCreate(nanoem_document_model_t *model, nanoem_status_t *status)
{
    nanoem_document_model_morph_state_t *state;
    state = (nanoem_document_model_morph_state_t *) nanoem_calloc(1, sizeof(*state), status);
    if (nanoem_is_not_null(state)) {
        state->base.parent.model = model;
    }
    return state;
}

void
nanoemDocumentModelMorphStateParse(nanoem_document_model_morph_state_t *state, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    state->weight = nanoemBufferReadFloat32LittleEndian(buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_STATE_CORRUPTED);
    }
}

const nanoem_unicode_string_t * APIENTRY
nanoemDocumentModelMorphStateGetName(const nanoem_document_model_morph_state_t *state)
{
    return nanoem_is_not_null(state) ? nanoemDocumentModelResolveMorphName(state->base.parent.model, state->base.index) : NULL;
}

nanoem_f32_t APIENTRY
nanoemDocumentModelMorphStateGetWeight(const nanoem_document_model_morph_state_t *state)
{
    return nanoem_is_not_null(state) ? state->weight : 0.0f;
}

void
nanoemDocumentModelMorphStateDestroy(nanoem_document_model_morph_state_t *state)
{
    if (nanoem_is_not_null(state)) {
        nanoem_free(state);
    }
}

nanoem_document_model_outside_parent_state_t *
nanoemDocumentModelOutsideParentStateCreate(nanoem_document_model_t *model, nanoem_status_t *status)
{
    nanoem_document_model_outside_parent_state_t *state;
    state = (nanoem_document_model_outside_parent_state_t *) nanoem_calloc(1, sizeof(*state), status);
    if (nanoem_is_not_null(state)) {
        state->base.parent.model = model;
        state->outside_parent = nanoemDocumentOutsideParentCreate(model->base.parent.document, status);
    }
    return state;
}

void
nanoemDocumentModelOutsideParentStateParse(nanoem_document_model_outside_parent_state_t *state, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    state->begin = nanoemBufferReadInt32LittleEndian(buffer, status);
    state->end = nanoemBufferReadInt32LittleEndian(buffer, status);
    nanoemDocumentOutsideParentParse(state->outside_parent, buffer, status);
    if (nanoem_status_ptr_has_error(status)) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_CORRUPTED);
    }
}

nanoem_frame_index_t APIENTRY
nanoemDocumentModelOutsideParentStateGetBeginFrameIndex(const nanoem_document_model_outside_parent_state_t *state)
{
    return nanoem_is_not_null(state) ? state->begin : 0;
}

nanoem_frame_index_t APIENTRY
nanoemDocumentModelOutsideParentStateGetEndFrameIndex(const nanoem_document_model_outside_parent_state_t *state)
{
    return nanoem_is_not_null(state) ? state->end : 0;
}

const nanoem_document_model_t *APIENTRY
nanoemDocumentModelOutsideParentStateGetTargetModelObject(const nanoem_document_model_outside_parent_state_t *state)
{
    const nanoem_document_model_t *model = NULL;
    if (nanoem_is_not_null(state)) {
        model = nanoemDocumentResolveModelObject(state->base.parent.model->base.parent.document, state->outside_parent->model_index);
    }
    return model;
}

const nanoem_unicode_string_t *APIENTRY
nanoemDocumentModelOutsideParentStateGetTargetBoneName(const nanoem_document_model_outside_parent_state_t *state)
{
    const nanoem_document_model_t *model;
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(state)) {
        model = nanoemDocumentModelOutsideParentStateGetTargetModelObject(state);
        name = nanoemDocumentModelResolveBoneName(model, state->outside_parent->bone_index);
    }
    return name;
}

void
nanoemDocumentModelOutsideParentStateDestroy(nanoem_document_model_outside_parent_state_t *state)
{
    if (nanoem_is_not_null(state)) {
        nanoemDocumentOutsideParentDestroy(state->outside_parent);
        nanoem_free(state);
    }
}

nanoem_document_model_t *
nanoemDocumentModelCreate(nanoem_document_t *document, nanoem_status_t *status)
{
    nanoem_document_model_t *model;
    model = (nanoem_document_model_t *) nanoem_calloc(1, sizeof(*model), status);
    if (nanoem_is_not_null(model)) {
        model->base.parent.document = document;
        model->factory = document->factory;
    }
    return model;
}

void
nanoemDocumentModelParseInitialBoneKeyframes(nanoem_document_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_document_model_bone_keyframe_t *keyframe;
    nanoem_rsize_t i, num_objects = model->num_bones;
    if (nanoemBufferCanReadLength(buffer, num_objects)) {
        model->initial_bone_keyframes = (nanoem_document_model_bone_keyframe_t **) nanoem_calloc(num_objects, sizeof(*model->initial_bone_keyframes), status);
        if (nanoem_is_not_null(model->initial_bone_keyframes)) {
            model->num_initial_bone_keyframes = num_objects;
            for (i = 0; i < num_objects; i++) {
                keyframe = nanoemDocumentModelBoneKeyframeCreate(model, status);
                nanoemDocumentModelBoneKeyframeParse(keyframe, buffer, nanoem_false, status);
                if (!nanoem_status_ptr_has_error(status)) {
                    keyframe->base.object_index = (int) i;
                    model->initial_bone_keyframes[i] = keyframe;
                }
                else {
                    nanoemDocumentModelBoneKeyframeDestroy(keyframe);
                    model->num_initial_bone_keyframes = i;
                    break;
                }
            }
        }
    }
    else if (num_objects > 0) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_KEYFRAME_CORRUPTED);
    }
}

void
nanoemDocumentModelParseAllBoneKeyframes(nanoem_document_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_document_model_bone_keyframe_t *keyframe;
    nanoem_rsize_t i, num_objects;
    num_objects = nanoemBufferReadInt32LittleEndian(buffer, status);
    if (nanoemBufferCanReadLength(buffer, num_objects)) {
        model->bone_keyframes = (nanoem_document_model_bone_keyframe_t **) nanoem_calloc(num_objects, sizeof(*model->bone_keyframes), status);
        if (nanoem_is_not_null(model->bone_keyframes)) {
            model->num_bone_keyframes = num_objects;
            for (i = 0; i < num_objects; i++) {
                keyframe = nanoemDocumentModelBoneKeyframeCreate(model, status);
                nanoemDocumentModelBoneKeyframeParse(keyframe, buffer, nanoem_true, status);
                if (!nanoem_status_ptr_has_error(status)) {
                    model->bone_keyframes[i] = keyframe;
                }
                else {
                    nanoemDocumentModelBoneKeyframeDestroy(keyframe);
                    model->num_bone_keyframes = i;
                    break;
                }
            }
        }
    }
    else if (num_objects > 0) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_KEYFRAME_CORRUPTED);
    }
}

void
nanoemDocumentModelParseInitialModelKeyframes(nanoem_document_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    model->initial_model_keyframe = nanoemDocumentModelKeyframeCreate(model, status);
    nanoemDocumentModelKeyframeParse(model->initial_model_keyframe, buffer, nanoem_false, status);
}

void
nanoemDocumentModelParseAllModelKeyframes(nanoem_document_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_document_model_keyframe_t *keyframe;
    nanoem_rsize_t i, num_objects;
    num_objects = nanoemBufferReadInt32LittleEndian(buffer, status);
    if (nanoemBufferCanReadLength(buffer, num_objects)) {
        model->model_keyframes = (nanoem_document_model_keyframe_t **) nanoem_calloc(num_objects, sizeof(*model->model_keyframes), status);
        if (nanoem_is_not_null(model->model_keyframes)) {
            model->num_model_keyframes = num_objects;
            for (i = 0; i < num_objects; i++) {
                keyframe = nanoemDocumentModelKeyframeCreate(model, status);
                nanoemDocumentModelKeyframeParse(keyframe, buffer, nanoem_true, status);
                if (!nanoem_status_ptr_has_error(status)) {
                    model->model_keyframes[i] = keyframe;
                }
                else {
                    nanoemDocumentModelKeyframeDestroy(keyframe);
                    model->num_model_keyframes = i;
                    break;
                }
            }
        }
    }
    else if (num_objects > 0) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_KEYFRAME_CORRUPTED);
    }
}

void
nanoemDocumentModelParseInitialMorphKeyframes(nanoem_document_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_document_model_morph_keyframe_t *keyframe;
    nanoem_rsize_t i, num_objects = model->num_morphs;
    if (nanoemBufferCanReadLength(buffer, num_objects)) {
        model->initial_morph_keyframes = (nanoem_document_model_morph_keyframe_t **) nanoem_calloc(num_objects, sizeof(*model->initial_morph_keyframes), status);
        if (nanoem_is_not_null(model->initial_morph_keyframes)) {
            model->num_initial_morph_keyframes = num_objects;
            for (i = 0; i < num_objects; i++) {
                keyframe = nanoemDocumentModelMorphKeyframeCreate(model, status);
                nanoemDocumentModelMorphKeyframeParse(keyframe, buffer, nanoem_false, status);
                if (!nanoem_status_ptr_has_error(status)) {
                    keyframe->base.object_index = (int) i;
                    model->initial_morph_keyframes[i] = keyframe;
                }
                else {
                    nanoemDocumentModelMorphKeyframeDestroy(keyframe);
                    model->num_initial_morph_keyframes = i;
                    break;
                }
            }
        }
    }
    else if (num_objects > 0) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_KEYFRAME_CORRUPTED);
    }
}

void
nanoemDocumentModelParseAllMorphKeyframes(nanoem_document_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_document_model_morph_keyframe_t *keyframe;
    nanoem_rsize_t i, num_objects;
    num_objects = nanoemBufferReadInt32LittleEndian(buffer, status);
    if (nanoemBufferCanReadLength(buffer, num_objects)) {
        model->morph_keyframes = (nanoem_document_model_morph_keyframe_t **) nanoem_calloc(num_objects, sizeof(*model->morph_keyframes), status);
        if (nanoem_is_not_null(model->morph_keyframes)) {
            model->num_morph_keyframes = num_objects;
            for (i = 0; i < num_objects; i++) {
                keyframe = nanoemDocumentModelMorphKeyframeCreate(model, status);
                nanoemDocumentModelMorphKeyframeParse(keyframe, buffer, nanoem_true, status);
                if (!nanoem_status_ptr_has_error(status)) {
                    model->morph_keyframes[i] = keyframe;
                }
                else {
                    nanoemDocumentModelMorphKeyframeDestroy(keyframe);
                    model->num_morph_keyframes = i;
                    break;
                }
            }
        }
    }
    else if (num_objects > 0) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_KEYFRAME_CORRUPTED);
    }
}

void
nanoemDocumentModelParseAllBoneStates(nanoem_document_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_document_model_bone_state_t *bone_state;
    nanoem_rsize_t i, num_objects = model->num_bones;
    if (nanoemBufferCanReadLength(buffer, num_objects)) {
        model->bone_states = (nanoem_document_model_bone_state_t **) nanoem_calloc(num_objects, sizeof(*model->bone_states), status);
        if (nanoem_is_not_null(model->bone_states)) {
            model->num_bone_states = num_objects;
            for (i = 0; i < num_objects; i++) {
                bone_state = nanoemDocumentModelBoneStateCreate(model, status);
                nanoemDocumentModelBoneStateParse(bone_state, buffer, status);
                if (!nanoem_status_ptr_has_error(status)) {
                    bone_state->base.index = i;
                    model->bone_states[i] = bone_state;
                }
                else {
                    nanoemDocumentModelBoneStateDestroy(bone_state);
                    model->num_bone_states = i;
                    break;
                }
            }
        }
    }
    else if (num_objects > 0) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_STATE_CORRUPTED);
    }
}

void
nanoemDocumentModelParseAllConstraintStates(nanoem_document_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_document_model_constraint_state_t *constraint_state;
    nanoem_rsize_t i, num_objects = model->num_constraint_bones;
    if (nanoemBufferCanReadLength(buffer, num_objects)) {
        model->constraint_states = (nanoem_document_model_constraint_state_t **) nanoem_calloc(num_objects, sizeof(*model->constraint_states), status);
        if (nanoem_is_not_null(model->constraint_states)) {
            model->num_constraint_states = num_objects;
            for (i = 0; i < num_objects; i++) {
                constraint_state = nanoemDocumentModelConstraintStateCreate(model, status);
                nanoemDocumentModelConstraintStateParse(constraint_state, buffer, status);
                if (!nanoem_status_ptr_has_error(status)) {
                    constraint_state->base.index = i;
                    model->constraint_states[i] = constraint_state;
                }
                else {
                    nanoemDocumentModelConstraintStateDestroy(constraint_state);
                    model->num_constraint_states = i;
                    break;
                }
            }
        }
    }
    else if (num_objects > 0) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_CONSTRAINT_STATE_CORRUPTED);
    }
}

void
nanoemDocumentModelParseAllMorphStates(nanoem_document_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_document_model_morph_state_t *morph_state;
    nanoem_rsize_t i, num_objects = model->num_morphs;
    if (nanoemBufferCanReadLength(buffer, num_objects)) {
        model->morph_states = (nanoem_document_model_morph_state_t **) nanoem_calloc(num_objects, sizeof(*model->morph_states), status);
        if (nanoem_is_not_null(model->morph_states)) {
            model->num_morph_states = num_objects;
            for (i = 0; i < num_objects; i++) {
                morph_state = nanoemDocumentModelMorphStateCreate(model, status);
                nanoemDocumentModelMorphStateParse(morph_state, buffer, status);
                if (!nanoem_status_ptr_has_error(status)) {
                    morph_state->base.index = i;
                    model->morph_states[i] = morph_state;
                }
                else {
                    nanoemDocumentModelMorphStateDestroy(morph_state);
                    model->num_morph_states = i;
                    break;
                }
            }
        }
    }
    else if (num_objects > 0) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_STATE_CORRUPTED);
    }
}

void
nanoemDocumentModelParseAllOutsideParentStates(nanoem_document_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_document_model_outside_parent_state_t *outside_parent_state;
    nanoem_rsize_t i, num_objects = model->num_outside_parent_subject_bones;
    if (nanoemBufferCanReadLength(buffer, num_objects)) {
        model->outside_parent_states = (nanoem_document_model_outside_parent_state_t **) nanoem_calloc(num_objects, sizeof(*model->outside_parent_states), status);
        if (nanoem_is_not_null(model->outside_parent_states)) {
            model->num_outside_parent_states = num_objects;
            for (i = 0; i < num_objects; i++) {
                outside_parent_state = nanoemDocumentModelOutsideParentStateCreate(model, status);
                nanoemDocumentModelOutsideParentStateParse(outside_parent_state, buffer, status);
                if (!nanoem_status_ptr_has_error(status)) {
                    model->outside_parent_states[i] = outside_parent_state;
                }
                else {
                    nanoemDocumentModelOutsideParentStateDestroy(outside_parent_state);
                    model->num_outside_parent_states = i;
                    break;
                }
            }
        }
    }
    else if (num_objects > 0) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_CORRUPTED);
    }
}

#define nanoem_document_parse_check(expr) \
    do { \
        (expr); \
        if (nanoem_status_ptr_has_error(status)) { \
            return; \
        } \
    } while (0)

void
nanoemDocumentModelParsePMMv1(nanoem_document_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_document_t *document = nanoemDocumentModelGetParentDocument(model);
    const nanoem_model_constraint_t *constraint;
    const nanoem_unicode_string_t *bone_name;
    nanoem_unicode_string_factory_t *factory;
    nanoem_model_t *model_ptr;
    nanoem_rsize_t num_objects, num_bones, i, j;
    nanoem_model_bone_t *const *bones, *bone;
    nanoem_model_morph_t *const *morphs, *morph;
    nanoem_model_constraint_t *const *constraints;
    char name[NANOEM_PMMV1_MODEL_NAME_MAX], path[NANOEM_PMM_PATH_MAX];
    model->name_ja = nanoemDocumentReadFixedStringPMM(document, buffer, name, NANOEM_PMMV1_MODEL_NAME_MAX, status);
    model->path = nanoemDocumentReadFixedStringPMM(document, buffer, path, NANOEM_PMM_PATH_MAX, status);
    model->draw_order_index = nanoemBufferReadByte(buffer, status);
    model->visible = nanoemBufferReadByte(buffer, status) != 0;
    model->selected_bone_index = nanoemBufferReadInt32LittleEndian(buffer, status);
    model->selected_morph_indices[0] = nanoemBufferReadInt32LittleEndian(buffer, status);
    model->selected_morph_indices[1] = nanoemBufferReadInt32LittleEndian(buffer, status);
    model->selected_morph_indices[2] = nanoemBufferReadInt32LittleEndian(buffer, status);
    model->selected_morph_indices[3] = nanoemBufferReadInt32LittleEndian(buffer, status);
    model->num_expansion_states = nanoemBufferReadByte(buffer, status);
    nanoemDocumentGetAllExpansionStates(buffer, &model->expansion_state, &model->num_expansion_states, status);
    model->vertical_scroll = nanoemBufferReadInt32LittleEndian(buffer, status);
    model->last_frame_index = nanoemBufferReadInt32LittleEndian(buffer, status);
    if (document->parse_callback) {
        model_ptr = document->parse_callback(document->parse_callback_user_data, model->path, document->factory, status);
        if (!nanoem_status_ptr_has_error(status)) {
            if (model_ptr) {
                factory = document->factory;
                bones = nanoemModelGetAllBoneObjects(model_ptr, &num_objects);
                num_bones = num_objects;
                if (num_objects > 0) {
                    model->bone_names = (nanoem_unicode_string_t **) nanoem_calloc(num_objects, sizeof(*model->bone_names), status);
                    if (nanoem_is_not_null(model->bone_names)) {
                        for (i = 0; i < num_objects; i++) {
                            bone = bones[i];
                            model->bone_names[i] = nanoemUnicodeStringFactoryCloneString(factory, nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), status);
                            constraint = nanoemModelBoneGetConstraintObject(bone);
                            if (constraint) {
                                model->num_constraint_bones++;
                            }
                        }
                        model->num_bones = num_objects;
                    }
                }
                constraints = nanoemModelGetAllConstraintObjects(model_ptr, &num_objects);
                if (num_objects > 0) {
                    model->constraint_bone_indices = (int *) nanoem_calloc(num_objects, sizeof(*model->constraint_bone_indices), status);
                    if (nanoem_is_not_null(model->constraint_bone_indices)) {
                        for (i = 0; i < num_objects; i++) {
                            constraint = constraints[i];
                            bone_name = nanoemModelBoneGetName(nanoemModelConstraintGetTargetBoneObject(constraint), NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                            model->constraint_bone_indices[i] = -1;
                            for (j = 0; j < num_bones; j++) {
                                if (nanoemUnicodeStringFactoryCompareString(factory, model->bone_names[j], bone_name) == 0) {
                                    model->constraint_bone_indices[i] = (int) j;
                                }
                            }
                        }
                        model->num_constraint_bones = num_objects;
                    }
                }
                morphs = nanoemModelGetAllMorphObjects(model_ptr, &num_objects);
                if (num_objects > 0) {
                    model->morph_names = (nanoem_unicode_string_t **) nanoem_calloc(num_objects, sizeof(*model->morph_names), status);
                    if (nanoem_is_not_null(model->morph_names)) {
                        for (i = 0; i < num_objects; i++) {
                            morph = morphs[i];
                            model->morph_names[i] = nanoemUnicodeStringFactoryCloneString(factory, nanoemModelMorphGetName(morph, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), status);
                        }
                        model->num_morphs = num_objects;
                    }
                }
                nanoemModelDestroy(model_ptr);
                nanoem_document_parse_check(nanoemDocumentModelParseInitialBoneKeyframes(model, buffer, status));
                nanoem_document_parse_check(nanoemDocumentModelParseAllBoneKeyframes(model, buffer, status));
                nanoem_document_parse_check(nanoemDocumentModelParseInitialMorphKeyframes(model, buffer, status));
                nanoem_document_parse_check(nanoemDocumentModelParseAllMorphKeyframes(model, buffer, status));
                nanoem_document_parse_check(nanoemDocumentModelParseInitialModelKeyframes(model, buffer, status));
                nanoem_document_parse_check(nanoemDocumentModelParseAllModelKeyframes(model, buffer, status));
                nanoem_document_parse_check(nanoemDocumentModelParseAllBoneStates(model, buffer, status));
                nanoem_document_parse_check(nanoemDocumentModelParseAllMorphStates(model, buffer, status));
                nanoem_document_parse_check(nanoemDocumentModelParseAllConstraintStates(model, buffer, status));
            }
            else {
                nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_BUFFER_END);
            }
        }
        else {
            nanoemModelDestroy(model_ptr);
        }
    }
    else {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_BUFFER_END);
    }
}

void
nanoemDocumentModelParsePMMv2(nanoem_document_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_document_t *document = nanoemDocumentModelGetParentDocument(model);
    char path[NANOEM_PMM_PATH_MAX];
    model->name_ja = nanoemDocumentReadVariableStringPMM(document, buffer, status);
    model->name_en = nanoemDocumentReadVariableStringPMM(document, buffer, status);
    model->path = nanoemDocumentReadFixedStringPMM(document, buffer, path, NANOEM_PMM_PATH_MAX, status);
    model->num_fixed_tracks = nanoemBufferReadByte(buffer, status);
    model->num_bones = nanoemBufferReadInt32LittleEndian(buffer, status);
    nanoemDocumentReadAllStringsPMM(document, buffer, &model->bone_names, &model->num_bones, status);
    model->num_morphs = nanoemBufferReadInt32LittleEndian(buffer, status);
    nanoemDocumentReadAllStringsPMM(document, buffer, &model->morph_names, &model->num_morphs, status);
    model->num_constraint_bones = nanoemBufferReadInt32LittleEndian(buffer, status);
    nanoemDocumentGetAllBoneIndices(buffer, &model->constraint_bone_indices, &model->num_constraint_bones, status);
    model->num_outside_parent_subject_bones = nanoemBufferReadInt32LittleEndian(buffer, status);
    nanoemDocumentGetAllBoneIndices(buffer, &model->outside_parent_subject_bone_indices, &model->num_outside_parent_subject_bones, status);
    model->draw_order_index = nanoemBufferReadByte(buffer, status);
    model->visible = nanoemBufferReadByte(buffer, status) != 0;
    model->selected_bone_index = nanoemBufferReadInt32LittleEndian(buffer, status);
    model->selected_morph_indices[0] = nanoemBufferReadInt32LittleEndian(buffer, status);
    model->selected_morph_indices[1] = nanoemBufferReadInt32LittleEndian(buffer, status);
    model->selected_morph_indices[2] = nanoemBufferReadInt32LittleEndian(buffer, status);
    model->selected_morph_indices[3] = nanoemBufferReadInt32LittleEndian(buffer, status);
    model->num_expansion_states = nanoemBufferReadByte(buffer, status);
    nanoemDocumentGetAllExpansionStates(buffer, &model->expansion_state, &model->num_expansion_states, status);
    model->vertical_scroll = nanoemBufferReadInt32LittleEndian(buffer, status);
    model->last_frame_index = nanoemBufferReadInt32LittleEndian(buffer, status);
    nanoem_document_parse_check(nanoemDocumentModelParseInitialBoneKeyframes(model, buffer, status));
    nanoem_document_parse_check(nanoemDocumentModelParseAllBoneKeyframes(model, buffer, status));
    nanoem_document_parse_check(nanoemDocumentModelParseInitialMorphKeyframes(model, buffer, status));
    nanoem_document_parse_check(nanoemDocumentModelParseAllMorphKeyframes(model, buffer, status));
    nanoem_document_parse_check(nanoemDocumentModelParseInitialModelKeyframes(model, buffer, status));
    nanoem_document_parse_check(nanoemDocumentModelParseAllModelKeyframes(model, buffer, status));
    nanoem_document_parse_check(nanoemDocumentModelParseAllBoneStates(model, buffer, status));
    nanoem_document_parse_check(nanoemDocumentModelParseAllMorphStates(model, buffer, status));
    nanoem_document_parse_check(nanoemDocumentModelParseAllConstraintStates(model, buffer, status));
    nanoem_document_parse_check(nanoemDocumentModelParseAllOutsideParentStates(model, buffer, status));
    model->is_blend_enabled = nanoemBufferReadByte(buffer, status) != 0;
    model->edge_width = nanoemBufferReadFloat32LittleEndian(buffer, status);
    model->is_self_shadow_enabled = nanoemBufferReadByte(buffer, status) != 0;
    model->transform_order_index = nanoemBufferReadByte(buffer, status);
}

#undef nanoem_document_parse_check

void
nanoemDocumentModelParse(nanoem_document_model_t *model, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_document_t *document;
    nanoem_rsize_t num_all_keyframes;
    if (nanoem_is_not_null(model)) {
        document = nanoemDocumentModelGetParentDocument(model);
        model->base.index = nanoemBufferReadByte(buffer, status);
        if (document->version > 1) {
            nanoemDocumentModelParsePMMv2(model, buffer, status);
        }
        else if (document->version == 1) {
            nanoemDocumentModelParsePMMv1(model, buffer, status);
        }
        num_all_keyframes = 1 + model->num_model_keyframes;
        model->all_model_keyframes_ptr = (nanoem_document_model_keyframe_t **) nanoem_calloc(num_all_keyframes, sizeof(*model->all_model_keyframes_ptr), status);
        if (nanoem_is_not_null(model->all_model_keyframes_ptr)) {
            model->all_model_keyframes_ptr[0] = model->initial_model_keyframe;
            nanoem_crt_memcpy(model->all_model_keyframes_ptr + 1, model->model_keyframes, sizeof(*model->all_model_keyframes_ptr) * model->num_model_keyframes);
            model->num_all_model_keyframes = num_all_keyframes;
        }
        num_all_keyframes = model->num_initial_bone_keyframes + model->num_bone_keyframes;
        if (num_all_keyframes > 0) {
            model->all_bone_keyframes_ptr = (nanoem_document_model_bone_keyframe_t **) nanoem_calloc(num_all_keyframes, sizeof(*model->all_bone_keyframes_ptr), status);
            if (nanoem_is_not_null(model->all_bone_keyframes_ptr)) {
                nanoem_crt_memcpy(model->all_bone_keyframes_ptr, model->initial_bone_keyframes, sizeof(*model->all_bone_keyframes_ptr) * model->num_initial_bone_keyframes);
                nanoem_crt_memcpy(model->all_bone_keyframes_ptr + model->num_initial_bone_keyframes, model->bone_keyframes, sizeof(*model->all_bone_keyframes_ptr) * model->num_bone_keyframes);
                model->num_all_bone_keyframes = num_all_keyframes;
            }
        }
        num_all_keyframes = model->num_initial_morph_keyframes + model->num_morph_keyframes;
        if (num_all_keyframes > 0) {
            model->all_morph_keyframes_ptr = (nanoem_document_model_morph_keyframe_t **) nanoem_calloc(num_all_keyframes, sizeof(*model->all_morph_keyframes_ptr), status);
            if (nanoem_is_not_null(model->all_morph_keyframes_ptr)) {
                nanoem_crt_memcpy(model->all_morph_keyframes_ptr, model->initial_morph_keyframes, sizeof(*model->all_morph_keyframes_ptr) * model->num_initial_morph_keyframes);
                nanoem_crt_memcpy(model->all_morph_keyframes_ptr + model->num_initial_morph_keyframes, model->morph_keyframes, sizeof(*model->all_morph_keyframes_ptr) * model->num_morph_keyframes);
                model->num_all_morph_keyframes = num_all_keyframes;
            }
        }
    }
}

nanoem_document_model_bone_keyframe_t *const *APIENTRY
nanoemDocumentModelGetAllBoneKeyframeObjects(const nanoem_document_model_t *model, nanoem_rsize_t *num_objects)
{
    if (nanoem_is_not_null(model)) {
        *num_objects = model->num_all_bone_keyframes;
        return model->all_bone_keyframes_ptr;
    }
    else {
        *num_objects = 0;
        return NULL;
    }
}

nanoem_document_model_keyframe_t *const *APIENTRY
nanoemDocumentModelGetAllModelKeyframeObjects(const nanoem_document_model_t *model, nanoem_rsize_t *num_objects)
{
    if (nanoem_is_not_null(model)) {
        *num_objects = model->num_all_model_keyframes;
        return model->all_model_keyframes_ptr;
    }
    else {
        *num_objects = 0;
        return NULL;
    }
}

nanoem_document_model_morph_keyframe_t *const *APIENTRY
nanoemDocumentModelGetAllMorphKeyframeObjects(const nanoem_document_model_t *model, nanoem_rsize_t *num_objects)
{
    if (nanoem_is_not_null(model)) {
        *num_objects = model->num_all_morph_keyframes;
        return model->all_morph_keyframes_ptr;
    }
    else {
        *num_objects = 0;
        return NULL;
    }
}

nanoem_unicode_string_t *const *APIENTRY
nanoemDocumentModelGetAllBoneNameObjects(const nanoem_document_model_t *model, nanoem_rsize_t *num_objects)
{
    if (nanoem_is_not_null(model)) {
        *num_objects = model->num_bones;
        return model->bone_names;
    }
    else {
        *num_objects = 0;
        return NULL;
    }
}

nanoem_unicode_string_t *const *APIENTRY
nanoemDocumentModelGetAllMorphNameObjects(const nanoem_document_model_t *model, nanoem_rsize_t *num_objects)
{
    if (nanoem_is_not_null(model)) {
        *num_objects = model->num_morphs;
        return model->morph_names;
    }
    else {
        *num_objects = 0;
        return NULL;
    }
}

nanoem_document_model_bone_state_t *const *APIENTRY
nanoemDocumentModelGetAllBoneStateObjects(const nanoem_document_model_t *model, nanoem_rsize_t *num_objects)
{
    if (nanoem_is_not_null(model)) {
        *num_objects = model->num_bone_states;
        return model->bone_states;
    }
    else {
        *num_objects = 0;
        return NULL;
    }
}

nanoem_document_model_constraint_state_t *const *APIENTRY
nanoemDocumentModelGetAllConstraintStateObjects(const nanoem_document_model_t *model, nanoem_rsize_t *num_objects)
{
    if (nanoem_is_not_null(model)) {
        *num_objects = model->num_constraint_states;
        return model->constraint_states;
    }
    else {
        *num_objects = 0;
        return NULL;
    }
}

nanoem_document_model_morph_state_t *const *APIENTRY
nanoemDocumentModelGetAllMorphStateObjects(const nanoem_document_model_t *model, nanoem_rsize_t *num_objects)
{
    if (nanoem_is_not_null(model)) {
        *num_objects = model->num_morph_states;
        return model->morph_states;
    }
    else {
        *num_objects = 0;
        return NULL;
    }
}

nanoem_document_model_outside_parent_state_t *const *APIENTRY
nanoemDocumentModelGetAllOutsideParentStateObjects(const nanoem_document_model_t *model, nanoem_rsize_t *num_objects)
{
    if (nanoem_is_not_null(model)) {
        *num_objects = model->num_outside_parent_states;
        return model->outside_parent_states;
    }
    else {
        *num_objects = 0;
        return NULL;
    }
}

const nanoem_unicode_string_t *APIENTRY
nanoemDocumentModelGetName(const nanoem_document_model_t *model, nanoem_language_type_t language)
{
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(model)) {
        switch (language) {
        case NANOEM_LANGUAGE_TYPE_JAPANESE:
            name = model->name_ja;
            break;
        case NANOEM_LANGUAGE_TYPE_ENGLISH:
            name = model->name_en;
            break;
        default:
            break;
        }
    }
    return name;
}

const nanoem_unicode_string_t *APIENTRY
nanoemDocumentModelGetPath(const nanoem_document_model_t *model)
{
    return nanoem_is_not_null(model) ? model->path : NULL;
}

const nanoem_unicode_string_t *APIENTRY
nanoemDocumentModelGetSelectedBoneName(const nanoem_document_model_t *model)
{
    return nanoem_is_not_null(model) ? nanoemDocumentModelResolveBoneName(model, model->selected_bone_index) : NULL;
}

const nanoem_unicode_string_t *APIENTRY
nanoemDocumentModelGetSelectedMorphName(const nanoem_document_model_t *model, nanoem_model_morph_category_t category)
{
    const nanoem_unicode_string_t *name = NULL;
    int index;
    if (nanoem_is_not_null(model) && category > NANOEM_MODEL_MORPH_CATEGORY_BASE && category < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM) {
        index = model->selected_morph_indices[category - (NANOEM_MODEL_MORPH_CATEGORY_BASE + 1)];
        if ((nanoem_rsize_t) index < model->num_morphs) {
            name = model->morph_names[index];
        }
    }
    return name;
}

nanoem_f32_t APIENTRY
nanoemDocumentModelGetEdgeWidth(const nanoem_document_model_t *model)
{
    return nanoem_is_not_null(model) ? model->edge_width : 0;
}

nanoem_frame_index_t APIENTRY
nanoemDocumentModelGetLastFrameIndex(const nanoem_document_model_t *model)
{
    return nanoem_is_not_null(model) ? model->last_frame_index : 0;
}

int APIENTRY
nanoemDocumentModelGetDrawOrderIndex(const nanoem_document_model_t *model)
{
    return nanoem_is_not_null(model) ? model->draw_order_index : -1;
}

int APIENTRY
nanoemDocumentModelGetTransformOrderIndex(const nanoem_document_model_t *model)
{
    return nanoem_is_not_null(model) ? model->transform_order_index : -1;
}

int APIENTRY
nanoemDocumentModelGetIndex(const nanoem_document_model_t *model)
{
    return nanoem_is_not_null(model) ? model->base.index : -1;
}

nanoem_bool_t APIENTRY
nanoemDocumentModelIsBlendEnabled(const nanoem_document_model_t *model)
{
    return nanoem_is_not_null(model) ? model->is_blend_enabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemDocumentModelIsSelfShadowEnabled(const nanoem_document_model_t *model)
{
    return nanoem_is_not_null(model) ? model->is_self_shadow_enabled : nanoem_false;
}

nanoem_rsize_t APIENTRY
nanoemDocumentModelCountAllConstraintBones(const nanoem_document_model_t *model)
{
    return nanoem_is_not_null(model) ? model->num_constraint_bones : 0;
}

nanoem_rsize_t APIENTRY
nanoemDocumentModelCountAllOutsideParentSubjectBones(const nanoem_document_model_t *model)
{
    return nanoem_is_not_null(model) ? model->num_outside_parent_subject_bones : 0;
}

const nanoem_unicode_string_t * APIENTRY
nanoemDocumentModelGetConstraintBoneName(const nanoem_document_model_t *model, nanoem_rsize_t index)
{
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(model) && index < model->num_constraint_bones) {
        name = nanoemDocumentModelResolveBoneName(model, model->constraint_bone_indices[index]);
    }
    return name;
}

const nanoem_unicode_string_t * APIENTRY
nanoemDocumentModelGetOutsideParentSubjectBoneName(const nanoem_document_model_t *model, nanoem_rsize_t index)
{
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(model) && index < model->num_outside_parent_subject_bones) {
        name = nanoemDocumentModelResolveBoneName(model, model->outside_parent_subject_bone_indices[index]);
    }
    return name;
}

void
nanoemDocumentModelDestroy(nanoem_document_model_t *model)
{
    nanoem_unicode_string_factory_t *factory;
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(model)) {
        factory = model->factory;
        nanoemUnicodeStringFactoryDestroyString(factory, model->name_ja);
        nanoemUnicodeStringFactoryDestroyString(factory, model->name_en);
        nanoemUnicodeStringFactoryDestroyString(factory, model->path);
        num_objects = model->num_bones;
        for (i = 0; i < num_objects; i++) {
            nanoemUnicodeStringFactoryDestroyString(factory, model->bone_names[i]);
        }
        nanoem_free(model->bone_names);
        num_objects = model->num_morphs;
        for (i = 0; i < num_objects; i++) {
            nanoemUnicodeStringFactoryDestroyString(factory, model->morph_names[i]);
        }
        nanoem_free(model->morph_names);
        nanoem_free(model->constraint_bone_indices);
        nanoem_free(model->outside_parent_subject_bone_indices);
        nanoem_free(model->expansion_state);
        num_objects = model->num_initial_bone_keyframes;
        for (i = 0; i < num_objects; i++) {
            nanoemDocumentModelBoneKeyframeDestroy(model->initial_bone_keyframes[i]);
        }
        nanoem_free(model->initial_bone_keyframes);
        num_objects = model->num_bone_keyframes;
        for (i = 0; i < num_objects; i++) {
            nanoemDocumentModelBoneKeyframeDestroy(model->bone_keyframes[i]);
        }
        nanoem_free(model->bone_keyframes);
        num_objects = model->num_initial_morph_keyframes;
        for (i = 0; i < num_objects; i++) {
            nanoemDocumentModelMorphKeyframeDestroy(model->initial_morph_keyframes[i]);
        }
        nanoem_free(model->initial_morph_keyframes);
        num_objects = model->num_morph_keyframes;
        for (i = 0; i < num_objects; i++) {
            nanoemDocumentModelMorphKeyframeDestroy(model->morph_keyframes[i]);
        }
        nanoem_free(model->morph_keyframes);
        num_objects = model->num_bone_states;
        for (i = 0; i < num_objects; i++) {
            nanoemDocumentModelBoneStateDestroy(model->bone_states[i]);
        }
        nanoem_free(model->bone_states);
        num_objects = model->num_morph_states;
        for (i = 0; i < num_objects; i++) {
            nanoemDocumentModelMorphStateDestroy(model->morph_states[i]);
        }
        nanoem_free(model->morph_states);
        nanoemDocumentModelKeyframeDestroy(model->initial_model_keyframe);
        num_objects = model->num_model_keyframes;
        for (i = 0; i < num_objects; i++) {
            nanoemDocumentModelKeyframeDestroy(model->model_keyframes[i]);
        }
        nanoem_free(model->model_keyframes);
        num_objects = model->num_constraint_states;
        for (i = 0; i < num_objects; i++) {
            nanoemDocumentModelConstraintStateDestroy(model->constraint_states[i]);
        }
        nanoem_free(model->constraint_states);
        num_objects = model->num_outside_parent_states;
        for (i = 0; i < num_objects; i++) {
            nanoemDocumentModelOutsideParentStateDestroy(model->outside_parent_states[i]);
        }
        nanoem_free(model->outside_parent_states);
        nanoem_free(model->all_bone_keyframes_ptr);
        nanoem_free(model->all_model_keyframes_ptr);
        nanoem_free(model->all_morph_keyframes_ptr);
        nanoem_free(model);
    }
}

void
nanoemDocumentParseAllAccessories(nanoem_document_t *document, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_document_accessory_t *accessory;
    nanoem_rsize_t i, num_accessories;
    char name[NANOEM_PMM_ACCESSORY_NAME_MAX];
    num_accessories = nanoemBufferReadByte(buffer, status);
    if (nanoemBufferCanReadLength(buffer, num_accessories)) {
        document->accessory_names = (nanoem_unicode_string_t **) nanoem_calloc(num_accessories, sizeof(*document->accessory_names), status);
        if (nanoem_is_not_null(document->accessory_names)) {
            document->num_accessory_names = num_accessories;
            for (i = 0; i < num_accessories; i++) {
                document->accessory_names[i] = nanoemDocumentReadFixedStringPMM(document, buffer, name, NANOEM_PMM_ACCESSORY_NAME_MAX, status);
            }
        }
        document->accessories = (nanoem_document_accessory_t **) nanoem_calloc(num_accessories, sizeof(*document->accessories), status);
        if (nanoem_is_not_null(document->accessories)) {
            document->num_accessories = num_accessories;
            for (i = 0; i < num_accessories; i++) {
                accessory = nanoemDocumentAccessoryCreate(document, status);
                nanoemDocumentAccessoryParse(accessory, buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemDocumentAccessoryDestroy(accessory);
                    document->num_accessories = i;
                    break;
                }
                else {
                    document->accessories[i] = accessory;
                }
            }
        }
    }
    else if (num_accessories > 0) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_CORRUPTED);
    }
}

void
nanoemDocumentParseAllModels(nanoem_document_t *document, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_document_model_t *model;
    nanoem_rsize_t i, num_models;
    document->select_model_index = nanoemBufferReadByte(buffer, status);
    num_models = document->num_models = nanoemBufferReadByte(buffer, status);
    if (num_models > 0) {
        document->models = (nanoem_document_model_t **) nanoem_calloc(num_models, sizeof(*document->models), status);
        if (nanoem_is_not_null(document->models)) {
            if (document->version == 1) {
                for (i = 0; i < num_models; i++) {
                    nanoemBufferSkip(buffer, NANOEM_PMMV1_MODEL_NAME_MAX, status);
                }
            }
            for (i = 0; i < num_models; i++) {
                model = nanoemDocumentModelCreate(document, status);
                nanoemDocumentModelParse(model, buffer, status);
                if (nanoem_status_ptr_has_error(status)) {
                    nanoemDocumentModelDestroy(model);
                    document->num_models = i;
                    break;
                }
                else {
                    document->models[i] = model;
                }
            }
        }
    }
}

nanoem_document_self_shadow_keyframe_t *
nanoemDocumentSelfShadowKeyframeCreate(nanoem_document_self_shadow_t *self_shadow, nanoem_status_t *status)
{
    nanoem_document_self_shadow_keyframe_t *keyframe;
    keyframe = (nanoem_document_self_shadow_keyframe_t *) nanoem_calloc(1, sizeof(*keyframe), status);
    if (nanoem_is_not_null(keyframe)) {
        keyframe->parent_self_shadow = self_shadow;
    }
    return keyframe;
}

static void
nanoemDocumentSelfShadowKeyframeInitialize(nanoem_document_self_shadow_keyframe_t *keyframe)
{
    keyframe->distance = 0.01125f;
    keyframe->mode = 1;
}

void
nanoemDocumentSelfShadowKeyframeParse(nanoem_document_self_shadow_keyframe_t *keyframe, nanoem_buffer_t *buffer, nanoem_bool_t include_index, nanoem_status_t *status)
{
    nanoemDocumentBaseKeyframeParse(&keyframe->base, buffer, include_index, status);
    keyframe->mode = nanoemBufferReadByte(buffer, status);
    keyframe->distance = nanoemBufferReadFloat32LittleEndian(buffer, status);
    keyframe->base.is_selected = nanoemBufferReadByte(buffer, status) != 0;
    if (nanoem_status_ptr_has_error(status)) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_SELF_SHADOW_KEYFRAME_CORRUPTED);
    }
}

const nanoem_document_base_keyframe_t * APIENTRY
nanoemDocumentSelfShadowKeyframeGetBaseKeyframeObject(const nanoem_document_self_shadow_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_document_base_keyframe_t *APIENTRY
nanoemDocumentSelfShadowKeyframeGetBaseKeyframeObjectMutable(nanoem_document_self_shadow_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? &keyframe->base : NULL;
}

nanoem_f32_t APIENTRY
nanoemDocumentSelfShadowKeyframeGetDistance(const nanoem_document_self_shadow_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->distance : 0;
}

int APIENTRY
nanoemDocumentSelfShadowKeyframeGetMode(const nanoem_document_self_shadow_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->mode : 0;
}

void
nanoemDocumentSelfShadowKeyframeDestroy(nanoem_document_self_shadow_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        nanoem_free(keyframe);
    }
}

nanoem_document_self_shadow_t *
nanoemDocumentSelfShadowCreate(nanoem_document_t *document, nanoem_status_t *status)
{
    nanoem_document_self_shadow_t *self_shadow;
    self_shadow = (nanoem_document_self_shadow_t *) nanoem_calloc(1, sizeof(*self_shadow), status);
    if (nanoem_is_not_null(self_shadow)) {
        self_shadow->base.parent.document = document;
    }
    return self_shadow;
}

static void
nanoemDocumentSelfShadowInitialize(nanoem_document_self_shadow_t *self_shadow)
{
    self_shadow->distance = 0.01125f;
    self_shadow->is_self_shadow_enabled = nanoem_true;
}

void
nanoemDocumentSelfShadowParse(nanoem_document_self_shadow_t *self_shadow, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_document_self_shadow_keyframe_t *keyframe;
    nanoem_rsize_t i, num_objects;
    self_shadow->is_self_shadow_enabled = nanoemBufferReadByte(buffer, status);
    self_shadow->distance = nanoemBufferReadFloat32LittleEndian(buffer, status);
    self_shadow->initial_self_shadow_keyframe = nanoemDocumentSelfShadowKeyframeCreate(self_shadow, status);
    nanoemDocumentSelfShadowKeyframeParse(self_shadow->initial_self_shadow_keyframe, buffer, nanoem_false, status);
    num_objects = nanoemBufferReadInt32LittleEndian(buffer, status);
    if (nanoemBufferCanReadLength(buffer, num_objects)) {
        self_shadow->self_shadow_keyframes = (nanoem_document_self_shadow_keyframe_t **) nanoem_calloc(num_objects, sizeof(*self_shadow->self_shadow_keyframes), status);
        if (nanoem_is_not_null(self_shadow->self_shadow_keyframes)) {
            self_shadow->num_self_shadow_keyframes = num_objects;
            for (i = 0; i < num_objects; i++) {
                keyframe = nanoemDocumentSelfShadowKeyframeCreate(self_shadow, status);
                nanoemDocumentSelfShadowKeyframeParse(keyframe, buffer, nanoem_true, status);
                if (!nanoem_status_ptr_has_error(status)) {
                    self_shadow->self_shadow_keyframes[i] = keyframe;
                }
                else {
                    nanoemDocumentSelfShadowKeyframeDestroy(keyframe);
                    self_shadow->num_self_shadow_keyframes = i;
                    break;
                }
            }
            if (!nanoem_status_ptr_has_error(status)) {
                self_shadow->all_self_shadow_keyframes_ptr = (nanoem_document_self_shadow_keyframe_t **) nanoem_calloc(1 + self_shadow->num_self_shadow_keyframes, sizeof(*self_shadow->all_self_shadow_keyframes_ptr), status);
                self_shadow->all_self_shadow_keyframes_ptr[0] = self_shadow->initial_self_shadow_keyframe;
                nanoem_crt_memcpy(self_shadow->all_self_shadow_keyframes_ptr + 1, self_shadow->self_shadow_keyframes, sizeof(*self_shadow->all_self_shadow_keyframes_ptr) * self_shadow->num_self_shadow_keyframes);
            }
        }
    }
    else if (num_objects > 0) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_SELF_SHADOW_CORRUPTED);
    }
}

nanoem_document_self_shadow_keyframe_t *const *APIENTRY
nanoemDocumentSelfShadowGetAllSelfShadowKeyframeObjects(const nanoem_document_self_shadow_t *self_shadow, nanoem_rsize_t *num_objects)
{
    if (nanoem_is_not_null(self_shadow) && self_shadow->self_shadow_keyframes) {
        *num_objects = self_shadow->num_self_shadow_keyframes + 1;
        return self_shadow->all_self_shadow_keyframes_ptr;
    }
    else {
        *num_objects = 0;
        return NULL;
    }
}

nanoem_f32_t APIENTRY
nanoemDocumentSelfShadowGetDistance(const nanoem_document_self_shadow_t *self_shadow)
{
    return nanoem_is_not_null(self_shadow) ? self_shadow->distance : 0.0f;
}

nanoem_bool_t APIENTRY
nanoemDocumentSelfShadowIsEnabled(const nanoem_document_self_shadow_t *self_shadow)
{
    return nanoem_is_not_null(self_shadow) ? self_shadow->is_self_shadow_enabled : nanoem_false;
}

void
nanoemDocumentSelfShadowDestroy(nanoem_document_self_shadow_t *self_shadow)
{
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(self_shadow)) {
        nanoemDocumentSelfShadowKeyframeDestroy(self_shadow->initial_self_shadow_keyframe);
        num_objects = self_shadow->num_self_shadow_keyframes;
        for (i = 0; i < num_objects; i++) {
            nanoemDocumentSelfShadowKeyframeDestroy(self_shadow->self_shadow_keyframes[i]);
        }
        nanoem_free(self_shadow->all_self_shadow_keyframes_ptr);
        nanoem_free(self_shadow->self_shadow_keyframes);
        nanoem_free(self_shadow);
    }
}

nanoem_document_t *APIENTRY
nanoemDocumentCreate(nanoem_unicode_string_factory_t *factory, nanoem_status_t *status)
{
    nanoem_document_t *document = NULL;
    if (nanoem_is_not_null(factory)) {
        document = (nanoem_document_t *) nanoem_calloc(1, sizeof(*document), status);
        if (nanoem_is_not_null(document)) {
            document->factory = factory;
            document->camera = nanoemDocumentCameraCreate(document, status);
            document->gravity = nanoemDocumentGravityCreate(document, status);
            document->light = nanoemDocumentLightCreate(document, status);
            document->self_shadow = nanoemDocumentSelfShadowCreate(document, status);
            document->version = 2;
            document->output_width = 1024;
            document->output_height = 768;
            document->accessory_index_after_models = -1;
            document->camera_look_at_model_index = -1;
            document->expand_accessory_panel = nanoem_true;
            document->expand_bone_panel = nanoem_true;
            document->expand_camera_panel = nanoem_true;
            document->expand_light_panel = nanoem_true;
            document->expand_morph_panel = nanoem_true;
            document->expand_self_shadow_panel = nanoem_true;
            document->editing_cla = nanoem_true;
            document->editing_mode = NANOEM_DOCUMENT_EDITING_MODE_NONE;
            document->camera_fov = 30;
            document->timeline_width = 250;
            document->horizontal_scroll_thumb = 735;
            document->preferred_fps = 60.0f;
            document->is_grid_and_axis_shown = nanoem_true;
            document->is_information_shown = nanoem_true;
            document->is_physics_ground_enabled = nanoem_true;
            document->ground_shadow_brightness = 1.0f;
            document->physics_simulation_mode = NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_ENABLE_TRACING;
            document->screen_capture_mode = 2;
        }
    }
    return document;
}

#define nanoem_document_parse_check(expr) \
    do { \
        (expr); \
        if (nanoem_status_ptr_has_error(status)) { \
            return nanoem_false; \
        } \
    } while (0)

nanoem_bool_t
nanoemDocumentParse(nanoem_document_t *document, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_rsize_t num_objects, i;
    nanoem_u8_t model_index;
    char path[NANOEM_PMM_PATH_MAX];
    int selection_index, editing_mode, physics_simulation_mode;
    nanoemBufferSkip(buffer, 30, status);
    document->output_width = nanoemBufferReadInt32LittleEndian(buffer, status);
    document->output_height = nanoemBufferReadInt32LittleEndian(buffer, status);
    document->timeline_width = nanoemBufferReadInt32LittleEndian(buffer, status);
    document->camera_fov = nanoemBufferReadFloat32LittleEndian(buffer, status);
    document->editing_cla = nanoemBufferReadByte(buffer, status) != 0;
    document->expand_camera_panel = nanoemBufferReadByte(buffer, status) != 0;
    document->expand_light_panel = nanoemBufferReadByte(buffer, status) != 0;
    document->expand_accessory_panel = nanoemBufferReadByte(buffer, status) != 0;
    document->expand_bone_panel = nanoemBufferReadByte(buffer, status) != 0;
    document->expand_morph_panel = nanoemBufferReadByte(buffer, status) != 0;
    if (document->version > 1) {
        document->expand_self_shadow_panel = nanoemBufferReadByte(buffer, status) != 0;
    }
    nanoem_document_parse_check(nanoemDocumentParseAllModels(document, buffer, status));
    nanoem_document_parse_check(nanoemDocumentCameraParse(document->camera, buffer, status));
    nanoem_document_parse_check(nanoemDocumentLightParse(document->light, buffer, status));
    document->select_accessory_index = nanoemBufferReadByte(buffer, status);
    document->horizontal_scroll_for_accessory = nanoemBufferReadInt32LittleEndian(buffer, status);
    nanoem_document_parse_check(nanoemDocumentParseAllAccessories(document, buffer, status));
    document->current_frame_index = nanoemBufferReadInt32LittleEndian(buffer, status);
    document->horizontal_scroll = nanoemBufferReadInt32LittleEndian(buffer, status);
    document->horizontal_scroll_thumb = nanoemBufferReadInt32LittleEndian(buffer, status);
    editing_mode = nanoemBufferReadInt32LittleEndian(buffer, status);
    switch (editing_mode) {
    case NANOEM_DOCUMENT_EDITING_MODE_MOVE:
    case NANOEM_DOCUMENT_EDITING_MODE_NONE:
    case NANOEM_DOCUMENT_EDITING_MODE_ROTATE:
    case NANOEM_DOCUMENT_EDITING_MODE_SELECT:
    case NANOEM_DOCUMENT_EDITING_MODE_SELECT_BOX:
        document->editing_mode = editing_mode;
        break;
    default:
        document->editing_mode = NANOEM_DOCUMENT_EDITING_MODE_NONE;
        break;
    };
    document->camera_look_mode = nanoemBufferReadByte(buffer, status);
    document->is_loop_enabled = nanoemBufferReadByte(buffer, status) != 0;
    document->is_begin_frame_index_enabled = nanoemBufferReadByte(buffer, status) != 0;
    document->is_end_frame_index_enabled = nanoemBufferReadByte(buffer, status) != 0;
    document->begin_frame_index = nanoemBufferReadInt32LittleEndian(buffer, status);
    document->end_frame_index = nanoemBufferReadInt32LittleEndian(buffer, status);
    document->is_audio_enabled = nanoemBufferReadByte(buffer, status) != 0;
    document->audio_path = nanoemDocumentReadFixedStringPMM(document, buffer, path, NANOEM_PMM_PATH_MAX, status);
    document->background_video_offset_x = nanoemBufferReadInt32LittleEndian(buffer, status);
    document->background_video_offset_y = nanoemBufferReadInt32LittleEndian(buffer, status);
    document->background_video_scale_factor = nanoemBufferReadFloat32LittleEndian(buffer, status);
    document->background_video_path = nanoemDocumentReadFixedStringPMM(document, buffer, path, NANOEM_PMM_PATH_MAX, status);
    document->is_background_video_enabled = nanoemBufferReadInt32LittleEndian(buffer, status) != 0;
    document->background_image_offset_x = nanoemBufferReadInt32LittleEndian(buffer, status);
    document->background_image_offset_y = nanoemBufferReadInt32LittleEndian(buffer, status);
    document->background_image_scale_factor = nanoemBufferReadFloat32LittleEndian(buffer, status);
    document->background_image_path = nanoemDocumentReadFixedStringPMM(document, buffer, path, NANOEM_PMM_PATH_MAX, status);
    document->is_background_image_enabled = nanoemBufferReadByte(buffer, status) != 0;
    document->is_information_shown = nanoemBufferReadByte(buffer, status) != 0;
    document->is_grid_and_axis_shown = nanoemBufferReadByte(buffer, status) != 0;
    document->is_ground_shadow_shown = nanoemBufferReadByte(buffer, status) != 0;
    document->preferred_fps = nanoemBufferReadFloat32LittleEndian(buffer, status);
    document->screen_capture_mode = nanoemBufferReadInt32LittleEndian(buffer, status);
    document->accessory_index_after_models = nanoemBufferReadInt32LittleEndian(buffer, status);
    document->ground_shadow_brightness = nanoemBufferReadFloat32LittleEndian(buffer, status);
    if (document->version > 1) {
        document->is_translucent_ground_shadow_enabled = nanoemBufferReadByte(buffer, status) != 0;
        physics_simulation_mode = nanoemBufferReadByte(buffer, status);
        switch (physics_simulation_mode) {
        case NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_DISABLE:
        case NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_ENABLE_ANYTIME:
        case NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_ENABLE_PLAYING:
        case NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_ENABLE_TRACING:
            document->physics_simulation_mode = physics_simulation_mode;
            break;
        default:
            document->physics_simulation_mode = NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_DISABLE;
            break;
        }
        nanoem_document_parse_check(nanoemDocumentGravityParse(document->gravity, buffer, status));
        nanoem_document_parse_check(nanoemDocumentSelfShadowParse(document->self_shadow, buffer, status));
        document->edge_color.values[0] = nanoemBufferReadInt32LittleEndian(buffer, status) / 255.0f;
        document->edge_color.values[1] = nanoemBufferReadInt32LittleEndian(buffer, status) / 255.0f;
        document->edge_color.values[2] = nanoemBufferReadInt32LittleEndian(buffer, status) / 255.0f;
        document->is_black_background_enabled = nanoemBufferReadByte(buffer, status) != 0;
        document->camera_look_at_model_index = nanoemBufferReadInt32LittleEndian(buffer, status);
        document->camera_look_at_model_bone_index = nanoemBufferReadInt32LittleEndian(buffer, status);
        if (nanoemBufferCanReadLength(buffer, sizeof(document->unknown_matrix))) {
            nanoem_crt_memcpy(document->unknown_matrix, nanoemBufferGetDataPtr(buffer), sizeof(document->unknown_matrix));
        }
        nanoemBufferSkip(buffer, sizeof(document->unknown_matrix), status);
        document->is_following_look_at_enabled = nanoemBufferReadByte(buffer, status);
        document->unknown_boolean = nanoemBufferReadByte(buffer, status) != 0;
        document->is_physics_ground_enabled = nanoemBufferReadByte(buffer, status) != 0;
        document->current_frame_index_in_text_field = nanoemBufferReadInt32LittleEndian(buffer, status);
        if (!nanoemBufferIsEnd(buffer) && nanoemBufferReadByte(buffer, status) != 0) {
            num_objects = document->num_models;
            for (i = 0; i < num_objects; i++) {
                model_index = nanoemBufferReadByte(buffer, status);
                selection_index = nanoemBufferReadInt32LittleEndian(buffer, status);
                if (model_index < num_objects) {
                    document->models[model_index]->selection_index = selection_index;
                }
            }
        }
    }
    else if (document->version == 1) {
        /* unknown bytes */
        /* nanoemDocumentSelfShadowParse(document->self_shadow, buffer, status); */
    }
    return !nanoem_status_ptr_has_error(status);
}

#undef nanoem_document_parse_check

nanoem_bool_t APIENTRY
nanoemDocumentLoadFromBuffer(nanoem_document_t *document, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_u8_t *ptr;
    nanoem_bool_t result = nanoem_false;
    if (nanoem_is_not_null(document) && nanoem_is_not_null(buffer)) {
        nanoem_status_ptr_assign_succeeded(status);
        ptr = nanoemBufferGetDataPtr(buffer);
        if (nanoemBufferCanReadLength(buffer, sizeof(__nanoem_pmmv2_signature) - 1) &&
            nanoem_crt_memcmp(ptr, __nanoem_pmmv2_signature, sizeof(__nanoem_pmmv2_signature) - 1) == 0) {
            document->version = 2;
            result = nanoemDocumentParse(document, buffer, status);
        }
        else if (nanoemBufferCanReadLength(buffer, sizeof(__nanoem_pmmv1_signature) - 1) &&
            nanoem_crt_memcmp(ptr, __nanoem_pmmv1_signature, sizeof(__nanoem_pmmv1_signature) - 1) == 0) {
            document->version = 1;
            result = nanoemDocumentParse(document, buffer, status);
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_INVALID_SIGNATURE);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
    return result;
}

void APIENTRY
nanoemDocumentSetParseModelCallback(nanoem_document_t *document, nanoem_document_parse_model_callback_t callback)
{
    if (nanoem_is_not_null(document)) {
        document->parse_callback = callback;
    }
}

void APIENTRY
nanoemDocumentSetParseModelCallbackUserData(nanoem_document_t *document, void *user_data)
{
    if (nanoem_is_not_null(document)) {
        document->parse_callback_user_data = user_data;
    }
}

nanoem_document_model_t *const *APIENTRY
nanoemDocumentGetAllModelObjects(const nanoem_document_t *document, nanoem_rsize_t *num_objects)
{
    if (nanoem_is_not_null(document)) {
        *num_objects = document->num_models;
        return document->models;
    }
    else {
        *num_objects = 0;
        return NULL;
    }
}

nanoem_document_accessory_t *const *APIENTRY
nanoemDocumentGetAllAccessoryObjects(const nanoem_document_t *document, nanoem_rsize_t *num_objects)
{
    if (nanoem_is_not_null(document)) {
        *num_objects = document->num_accessories;
        return document->accessories;
    }
    else {
        *num_objects = 0;
        return NULL;
    }
}

const nanoem_document_camera_t *APIENTRY
nanoemDocumentGetCameraObject(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->camera : NULL;
}

const nanoem_document_light_t *APIENTRY
nanoemDocumentGetLightObject(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->light : NULL;
}

const nanoem_document_gravity_t *APIENTRY
nanoemDocumentGetGravityObject(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->gravity : NULL;
}

const nanoem_document_self_shadow_t *APIENTRY
nanoemDocumentGetSelfShadowObject(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->self_shadow : NULL;
}

const nanoem_unicode_string_t * APIENTRY
nanoemDocumentGetAudioPath(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->audio_path : NULL;
}

const nanoem_unicode_string_t * APIENTRY
nanoemDocumentGetBackgroundVideoPath(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->background_video_path : NULL;
}

const nanoem_unicode_string_t * APIENTRY
nanoemDocumentGetBackgroundImagePath(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->background_image_path : NULL;
}

const nanoem_f32_t * APIENTRY
nanoemDocumentGetEdgeColor(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->edge_color.values : __nanoem_null_vector3;
}

nanoem_f32_t APIENTRY
nanoemDocumentGetPreferredFPS(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->preferred_fps : 0.0f;
}

nanoem_f32_t APIENTRY
nanoemDocumentGetBackgroundVideoScaleFactor(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->background_video_scale_factor : 0.0f;
}

nanoem_f32_t APIENTRY
nanoemDocumentGetBackgroundImageScaleFactor(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->background_image_scale_factor : 0.0f;
}

nanoem_f32_t APIENTRY
nanoemDocumentGetGroundShadowBrightness(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->ground_shadow_brightness : 0.0f;
}

nanoem_document_physics_simulation_mode_t APIENTRY
nanoemDocumentGetPhysicsSimulationMode(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->physics_simulation_mode : NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_DISABLE;
}

nanoem_frame_index_t APIENTRY
nanoemDocumentGetCurrentFrameIndex(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->current_frame_index : 0;
}

nanoem_frame_index_t APIENTRY
nanoemDocumentGetCurrentFrameIndexInTextField(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->current_frame_index_in_text_field : 0;
}

nanoem_frame_index_t APIENTRY
nanoemDocumentGetBeginFrameIndex(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->begin_frame_index : 0;
}

nanoem_frame_index_t APIENTRY
nanoemDocumentGetEndFrameIndex(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->end_frame_index : 0;
}

int APIENTRY
nanoemDocumentGetEditingMode(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->editing_mode : 0;
}

int APIENTRY
nanoemDocumentGetViewportWidth(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->output_width : 0;
}

int APIENTRY
nanoemDocumentGetViewportHeight(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->output_height : 0;
}

int APIENTRY
nanoemDocumentGetTimelineWidth(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->timeline_width : 0;
}

int APIENTRY
nanoemDocumentGetSelectedModelIndex(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->select_model_index : -1;
}

int APIENTRY
nanoemDocumentGetSelectedAccessoryIndex(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->select_accessory_index : -1;
}

int APIENTRY
nanoemDocumentGetAccessoryIndexAfterModel(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->accessory_index_after_models : -1;
}

int APIENTRY
nanoemDocumentGetBackgroundVideoOffsetX(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->background_video_offset_x : 0;
}

int APIENTRY
nanoemDocumentGetBackgroundVideoOffsetY(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->background_video_offset_y : 0;
}

int APIENTRY
nanoemDocumentGetBackgroundImageOffsetX(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->background_image_offset_x : 0;
}

int APIENTRY
nanoemDocumentGetBackgroundImageOffsetY(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->background_image_offset_y : 0;
}

nanoem_bool_t APIENTRY
nanoemDocumentIsAudioEnabled(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->is_audio_enabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemDocumentIsBackgroundVideoEnabled(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->is_background_video_enabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemDocumentIsBackgroundImageEnabled(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->is_background_image_enabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemDocumentIsEditingCLAEnabled(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->editing_cla : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemDocumentIsGridAndAxisShown(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->is_grid_and_axis_shown : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemDocumentIsInformationShown(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->is_information_shown : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemDocumentIsGroundShadowShown(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->is_ground_shadow_shown : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemDocumentIsLoopEnabled(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->is_loop_enabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemDocumentIsBeginFrameIndexEnabled(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->is_begin_frame_index_enabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemDocumentIsEndFrameIndexEnabled(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->is_end_frame_index_enabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemDocumentIsPhysicsGroundEnabled(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->is_physics_ground_enabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemDocumentIsTranslucentGroundShadowEnabled(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->is_translucent_ground_shadow_enabled : nanoem_false;
}

nanoem_bool_t APIENTRY
nanoemDocumentIsBlackBackgroundEnabled(const nanoem_document_t *document)
{
    return nanoem_is_not_null(document) ? document->is_black_background_enabled : nanoem_false;
}

void APIENTRY
nanoemDocumentDestroy(nanoem_document_t *document)
{
    struct nanoem_unicode_string_factory_t *factory;
    nanoem_rsize_t num_objects, i;
    if (nanoem_is_not_null(document)) {
        factory = document->factory;
        num_objects = document->num_models;
        for (i = 0; i < num_objects; i++) {
            nanoemDocumentModelDestroy(document->models[i]);
        }
        nanoem_free(document->models);
        num_objects = document->num_accessories;
        for (i = 0; i < num_objects; i++) {
            nanoemDocumentAccessoryDestroy(document->accessories[i]);
        }
        num_objects = document->num_accessory_names;
        for (i = 0; i < num_objects; i++) {
            nanoemUnicodeStringFactoryDestroyString(factory, document->accessory_names[i]);
        }
        nanoem_free(document->accessories);
        nanoem_free(document->accessory_names);
        nanoemUnicodeStringFactoryDestroyString(factory, document->audio_path);
        nanoemUnicodeStringFactoryDestroyString(factory, document->background_video_path);
        nanoemUnicodeStringFactoryDestroyString(factory, document->background_image_path);
        nanoemDocumentCameraDestroy(document->camera);
        nanoemDocumentGravityDestroy(document->gravity);
        nanoemDocumentLightDestroy(document->light);
        nanoemDocumentSelfShadowDestroy(document->self_shadow);
        nanoem_free(document);
    }
}

NANOEM_DECL_INLINE static nanoem_bool_t
nanoemMutableDocumentObjectCanDelete(nanoem_mutable_document_base_object_t *base)
{
    return !base->is_in_object && !base->is_reference;
}

NANOEM_DECL_INLINE static void
nanoemUnicodeStringFactoryAssignString(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_t **left, const nanoem_unicode_string_t *right, nanoem_status_t *status)
{
    nanoem_unicode_string_t *s = *left;
    *left = nanoemUnicodeStringFactoryCloneString(factory, right, status);
    nanoemUnicodeStringFactoryDestroyString(factory, s);
}

static void
nanoemDocumentWriteVariableStringPMM(const nanoem_document_t *document, const nanoem_unicode_string_t *value, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_rsize_t length;
    nanoem_u8_t *bytes = nanoemUnicodeStringFactoryGetByteArrayEncoding(document->factory, value, &length, NANOEM_CODEC_TYPE_SJIS, status);
    if (nanoem_is_not_null(bytes)) {
        if (length >= 0xff) {
            length = 0xff;
        }
        nanoemMutableBufferWriteByte(buffer, (nanoem_u8_t) length, status);
        nanoemMutableBufferWriteByteArray(buffer, bytes, length, status);
        nanoemUnicodeStringFactoryDestroyByteArray(document->factory, bytes);
    }
    else {
        nanoemMutableBufferWriteByte(buffer, 0, status);
    }
}

static void
nanoemDocumentWriteFixedStringPMM(const nanoem_document_t *document, const nanoem_unicode_string_t *value, nanoem_u8_t *dest, nanoem_rsize_t dest_size, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_rsize_t length, i;
    nanoem_u8_t *bytes = nanoemUnicodeStringFactoryGetByteArrayEncoding(document->factory, value, &length, NANOEM_CODEC_TYPE_SJIS, status);
    if (nanoem_is_not_null(bytes)) {
        nanoem_crt_memset(dest, 0, dest_size);
        nanoemUtilCopyString((char *) dest, dest_size, (const char *) bytes, length);
        nanoemMutableBufferWriteByteArray(buffer, dest, dest_size, status);
        nanoemUnicodeStringFactoryDestroyByteArray(document->factory, bytes);
    }
    else {
        for (i = 0; i < dest_size; i++) {
            nanoemMutableBufferWriteByte(buffer, 0, status);
        }
    }
}

static int
nanoemMutableDocumentFindParentModelBoneIndex(nanoem_mutable_document_t *document, int parent_model_index, const nanoem_unicode_string_t *value)
{
    const nanoem_document_t *origin_document = document->origin;
    nanoem_unicode_string_factory_t *factory = origin_document->factory;
    nanoem_document_model_t *origin_model;
    nanoem_rsize_t i, num_bones;
    int found_bone_index = -1;
    if ((nanoem_rsize_t) parent_model_index < origin_document->num_models) {
        origin_model = origin_document->models[parent_model_index];
        if (nanoem_is_not_null(origin_model)) {
            num_bones = origin_model->num_bones;
            for (i = 0; i < num_bones; i++) {
                if (nanoemUnicodeStringFactoryCompareString(factory, origin_model->bone_names[i], value) == 0) {
                    found_bone_index = (int) i;
                    break;
                }
            }
        }
    }
    return found_bone_index;
}

static int
nanoemMutableDocumentFindParentModelMorphIndex(nanoem_mutable_document_t *document, int parent_model_index, const nanoem_unicode_string_t *value)
{
    nanoem_document_t *origin_document = document->origin;
    nanoem_unicode_string_factory_t *factory = origin_document->factory;
    nanoem_document_model_t *origin_model;
    nanoem_rsize_t i, num_morphs;
    int found_morph_index = -1;
    if ((nanoem_rsize_t) parent_model_index < origin_document->num_models) {
        origin_model = origin_document->models[parent_model_index];
        if (nanoem_is_not_null(origin_model)) {
            num_morphs = origin_model->num_morphs;
            for (i = 0; i < num_morphs; i++) {
                if (nanoemUnicodeStringFactoryCompareString(factory, origin_model->morph_names[i], value) == 0) {
                    found_morph_index = (int) i;
                    break;
                }
            }
        }
    }
    return found_morph_index;
}

static int
nanoemMutableDocumentModelResolveBoneId(nanoem_mutable_document_model_t *model, const nanoem_unicode_string_t *name)
{
    const nanoem_unicode_string_t *bone_name;
    nanoem_unicode_string_factory_t *factory = nanoemDocumentModelGetParentDocument(model->origin)->factory;
    nanoem_document_model_t *origin_model = model->origin;
    nanoem_unicode_string_t **bone_names = origin_model->bone_names;
    nanoem_rsize_t num_bone_names = origin_model->num_bones, i;
    int object_index = -1;
    for (i = 0; i < num_bone_names; i++) {
        bone_name = bone_names[i];
        if (nanoemUnicodeStringFactoryCompareString(factory, bone_name, name) == 0) {
            object_index = i;
            break;
        }
    }
    return object_index;
}

static int
nanoemMutableDocumentModelResolveConstraintBoneId(nanoem_mutable_document_model_t *model, const nanoem_unicode_string_t *name, nanoem_status_t *status)
{
    nanoem_document_model_t *origin_model;
    nanoem_rsize_t num_bone_indices, i;
    int *bone_indices, index = -1, bone_index = nanoemMutableDocumentModelResolveBoneId(model, name);
    if (bone_index != -1) {
        origin_model = model->origin;
        bone_indices = origin_model->constraint_bone_indices;
        num_bone_indices = origin_model->num_constraint_bones;
        for (i = 0; i < num_bone_indices; i++) {
            if (bone_indices[i] == bone_index) {
                nanoem_status_ptr_assign_succeeded(status);
                return i;
            }
        }
        bone_indices = (int *) nanoemMutableObjectArrayResize(origin_model->constraint_bone_indices,
            &model->num_allocated_constraint_bone_indices,
            &origin_model->num_constraint_bones, status);
        if (nanoem_is_not_null(bone_indices)) {
            index = (int) origin_model->num_constraint_bones - 1;
            bone_indices[index] = bone_index;
            origin_model->constraint_bone_indices = bone_indices;
            nanoem_status_ptr_assign_succeeded(status);
        }
    }
    return index;
}

static nanoem_bool_t
nanoemDocumentFrameIndexSetInsert(kh_frame_index_set_t *set, nanoem_frame_index_t frame_index)
{
    nanoem_bool_t res = nanoem_false;
    khint_t it = kh_get_frame_index_set(set, (khint64_t) frame_index);
    int ret = 0;
    if (it == kh_end(set)) {
        kh_put_frame_index_set(set, (khint64_t) frame_index, &ret);
        res = nanoem_true;
    }
    return res;
}

static nanoem_bool_t
nanoemDocumentModelFindBoneKeyframeIndex(nanoem_document_model_t *model, int bone_index, nanoem_frame_index_t frame_index, nanoem_document_keyframe_compare_result_t *previous, nanoem_document_keyframe_compare_result_t *next)
{
    nanoem_document_model_bone_keyframe_t *keyframe = model->initial_bone_keyframes[bone_index];
    nanoem_bool_t found = nanoem_false;
    kh_frame_index_set_t *set = kh_init_frame_index_set();
    while (keyframe->base.next_keyframe_index > 0) {
        found |= nanoemDocumentBaseKeyframeCompare(&keyframe->base, frame_index, previous, next);
        keyframe = model->all_bone_keyframes_ptr[keyframe->base.next_keyframe_index];
        if (!nanoemDocumentFrameIndexSetInsert(set, frame_index)) {
            break;
        }
    }
    kh_destroy_frame_index_set(set);
    found |= nanoemDocumentBaseKeyframeCompare(&keyframe->base, frame_index, previous, next);
    return found;
}

static int
nanoemMutableDocumentModelResolveMorphId(nanoem_mutable_document_model_t *model, const nanoem_unicode_string_t *name)
{
    const nanoem_unicode_string_t *morph_name;
    nanoem_unicode_string_factory_t *factory = nanoemDocumentModelGetParentDocument(model->origin)->factory;
    nanoem_document_model_t *origin_model = model->origin;
    nanoem_unicode_string_t **morph_names = origin_model->morph_names;
    nanoem_rsize_t num_morph_names = origin_model->num_morphs, i;
    int object_index = -1;
    for (i = 0; i < num_morph_names; i++) {
        morph_name = morph_names[i];
        if (nanoemUnicodeStringFactoryCompareString(factory, morph_name, name) == 0) {
            object_index = i;
            break;
        }
    }
    return object_index;
}

static nanoem_bool_t
nanoemDocumentModelFindMorphKeyframeIndex(nanoem_document_model_t *model, int morph_index, nanoem_frame_index_t frame_index, nanoem_document_keyframe_compare_result_t *previous, nanoem_document_keyframe_compare_result_t *next)
{
    nanoem_document_model_morph_keyframe_t *keyframe = model->initial_morph_keyframes[morph_index];
    nanoem_bool_t found = nanoem_false;
    kh_frame_index_set_t *set = kh_init_frame_index_set();
    while (keyframe->base.next_keyframe_index > 0) {
        found |= nanoemDocumentBaseKeyframeCompare(&keyframe->base, frame_index, previous, next);
        keyframe = model->all_morph_keyframes_ptr[keyframe->base.next_keyframe_index];
        if (!nanoemDocumentFrameIndexSetInsert(set, frame_index)) {
            break;
        }
    }
    kh_destroy_frame_index_set(set);
    found |= nanoemDocumentBaseKeyframeCompare(&keyframe->base, frame_index, previous, next);
    return found;
}

static nanoem_bool_t
nanoemDocumentModelFindModelKeyframeIndex(nanoem_document_model_t *model, nanoem_frame_index_t frame_index, nanoem_document_keyframe_compare_result_t *previous, nanoem_document_keyframe_compare_result_t *next)
{
    nanoem_document_model_keyframe_t *keyframe;
    nanoem_document_model_keyframe_t *const *keyframes = model->all_model_keyframes_ptr;
    nanoem_rsize_t num_keyframes = model->num_all_model_keyframes, i;
    nanoem_bool_t found = nanoem_false;
    for (i = 0; i < num_keyframes; i++) {
        keyframe = keyframes[i];
        found |= nanoemDocumentBaseKeyframeCompare(&keyframe->base, frame_index, previous, next);
    }
    return found;
}

nanoem_mutable_document_outside_parent_t * APIENTRY
nanoemMutableDocumentOutsideParentCreate(nanoem_mutable_document_t *document, nanoem_status_t *status)
{
    nanoem_mutable_document_outside_parent_t *op;
    op = (nanoem_mutable_document_outside_parent_t *) nanoem_calloc(sizeof(*op), 1, status);
    if (nanoem_is_not_null(op)) {
        op->base.parent.document = document;
        op->origin = nanoemDocumentOutsideParentCreate(nanoemMutableDocumentGetOrigin(document), status);
    }
    return op;
}

void APIENTRY
nanoemMutableDocumentOutsideParentSetModelObject(nanoem_mutable_document_outside_parent_t *op, const nanoem_document_model_t *value)
{
    if (nanoem_is_not_null(op) && nanoem_is_not_null(value)) {
        op->origin->model_index = nanoemDocumentModelGetIndex(value);
    }
}

void APIENTRY
nanoemMutableDocumentOutsideParentSetBoneName(nanoem_mutable_document_outside_parent_t *op, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_mutable_document_t *document;
    nanoem_document_outside_parent_t *origin;
    if (nanoem_is_not_null(op) && nanoem_is_not_null(value)) {
        document = op->base.parent.document;
        origin = op->origin;
        origin->bone_index = nanoemMutableDocumentFindParentModelBoneIndex(document, origin->model_index, value);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentOutsideParentDestroy(nanoem_mutable_document_outside_parent_t *op)
{
    if (nanoem_is_not_null(op)) {
        if (nanoemMutableDocumentObjectCanDelete(&op->base)) {
            nanoemDocumentOutsideParentDestroy(op->origin);
        }
        nanoem_free(op);
    }
}

nanoem_mutable_document_accessory_keyframe_t * APIENTRY
nanoemMutableDocumentAccessoryKeyframeCreate(nanoem_mutable_document_accessory_t *accessory, nanoem_status_t *status)
{
    nanoem_mutable_document_accessory_keyframe_t *keyframe;
    keyframe = (nanoem_mutable_document_accessory_keyframe_t *) nanoem_calloc(sizeof(*keyframe), 1, status);
    if (nanoem_is_not_null(keyframe)) {
        keyframe->base.parent.accessory = accessory;
        keyframe->origin = nanoemDocumentAccessoryKeyframeCreate(nanoemMutableDocumentAccessoryGetOrigin(accessory), status);
        if (nanoem_is_not_null(keyframe->origin)) {
            nanoemDocumentAccessoryKeyframeInitialize(keyframe->origin);
        }
    }
    return keyframe;
}

void APIENTRY
nanoemMutableDocumentAccessoryKeyframeSetParentModelObject(nanoem_mutable_document_accessory_keyframe_t *keyframe, const nanoem_document_model_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        keyframe->origin->parent_model_index = nanoemDocumentModelGetIndex(value);
    }
}

void APIENTRY
nanoemMutableDocumentAccessoryKeyframeSetParentModelBoneName(nanoem_mutable_document_accessory_keyframe_t *keyframe, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_mutable_document_t *document;
    int parent_model_index, parent_model_bone_index = 0;
    if (nanoem_is_not_null(keyframe)) {
        if (nanoem_is_not_null(value)) {
            document = keyframe->base.parent.accessory->base.parent.document;
            parent_model_index = keyframe->origin->parent_model_index;
            parent_model_bone_index = nanoemMutableDocumentFindParentModelBoneIndex(document, parent_model_index, value);
        }
        keyframe->origin->parent_model_bone_index = parent_model_bone_index;
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentAccessoryKeyframeSetTranslation(nanoem_mutable_document_accessory_keyframe_t *keyframe, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(keyframe->origin->translation.values, value, sizeof(keyframe->origin->translation.values));
    }
}

void APIENTRY
nanoemMutableDocumentAccessoryKeyframeSetOrientation(nanoem_mutable_document_accessory_keyframe_t *keyframe, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(keyframe->origin->orientation.values, value, sizeof(keyframe->origin->orientation.values));
    }
}

void APIENTRY
nanoemMutableDocumentAccessoryKeyframeSetScaleFactor(nanoem_mutable_document_accessory_keyframe_t *keyframe, nanoem_f32_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->scale_factor = value;
    }
}

void APIENTRY
nanoemMutableDocumentAccessoryKeyframeSetOpacity(nanoem_mutable_document_accessory_keyframe_t *keyframe, nanoem_f32_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->opacity = value;
    }
}

void APIENTRY
nanoemMutableDocumentAccessoryKeyframeSetVisible(nanoem_mutable_document_accessory_keyframe_t *keyframe, nanoem_bool_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->visible = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentAccessoryKeyframeSetShadowEnabled(nanoem_mutable_document_accessory_keyframe_t *keyframe, nanoem_bool_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->is_shadow_enabled = value ? nanoem_true : nanoem_false;
    }
}

nanoem_document_accessory_keyframe_t* APIENTRY
nanoemMutableDocumentAccessoryKeyframeGetOrigin(nanoem_mutable_document_accessory_keyframe_t* keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->origin : NULL;
}

void APIENTRY
nanoemMutableDocumentAccessoryKeyframeDestroy(nanoem_mutable_document_accessory_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        if (nanoemMutableDocumentObjectCanDelete(&keyframe->base)) {
            nanoemDocumentAccessoryKeyframeDestroy(keyframe->origin);
        }
        nanoem_free(keyframe);
    }
}

nanoem_mutable_document_model_bone_keyframe_t * APIENTRY
nanoemMutableDocumentModelBoneKeyframeCreate(nanoem_mutable_document_model_t *model, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_document_model_bone_keyframe_t *origin;
    nanoem_mutable_document_model_bone_keyframe_t *keyframe;
    keyframe = (nanoem_mutable_document_model_bone_keyframe_t *) nanoem_calloc(sizeof(*keyframe), 1, status);
    if (nanoem_is_not_null(keyframe)) {
        origin = keyframe->origin = nanoemDocumentModelBoneKeyframeCreate(nanoemMutableDocumentModelGetOrigin(model), status);
        if (nanoem_is_not_null(origin)) {
            keyframe->base.parent.model = model;
            keyframe->name = nanoemUnicodeStringFactoryCloneString(model->origin->base.parent.document->factory, value, status);
            nanoemDocumentModelBoneKeyframeInitialize(origin);
        }
    }
    return keyframe;
}

void APIENTRY
nanoemMutableDocumentModelBoneKeyframeSetTranslation(nanoem_mutable_document_model_bone_keyframe_t *keyframe, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(keyframe->origin->translation.values, value, sizeof(keyframe->origin->translation.values));
    }
}

void APIENTRY
nanoemMutableDocumentModelBoneKeyframeSetOrientation(nanoem_mutable_document_model_bone_keyframe_t *keyframe, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(keyframe->origin->orientation.values, value, sizeof(keyframe->origin->orientation.values));
    }
}

void APIENTRY
nanoemMutableDocumentModelBoneKeyframeSetInterpolation(nanoem_mutable_document_model_bone_keyframe_t *keyframe, nanoem_motion_bone_keyframe_interpolation_type_t type, const nanoem_u8_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)
            && type >= NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM
            && type < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM
            && keyframe->origin->interpolation[type].u.values != value) {
        nanoem_crt_memcpy(keyframe->origin->interpolation[type].u.values, value, sizeof(keyframe->origin->interpolation[type].u.values));
    }
}

void APIENTRY
nanoemMutableDocumentModelBoneKeyframeSetPhysicsSimulationDisabled(nanoem_mutable_document_model_bone_keyframe_t *keyframe, nanoem_bool_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->is_physics_simulation_disabled = value ? nanoem_true : nanoem_false;
    }
}

nanoem_document_model_bone_keyframe_t *APIENTRY
nanoemMutableDocumentModelBoneKeyframeGetOrigin(nanoem_mutable_document_model_bone_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->origin : NULL;
}

void APIENTRY
nanoemMutableDocumentModelBoneKeyframeDestroy(nanoem_mutable_document_model_bone_keyframe_t *keyframe)
{
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(keyframe)) {
        factory = keyframe->origin->parent_model->base.parent.document->factory;
        if (nanoemMutableDocumentObjectCanDelete(&keyframe->base)) {
            nanoemDocumentModelBoneKeyframeDestroy(keyframe->origin);
        }
        nanoemUnicodeStringFactoryDestroyString(factory, keyframe->name);
        nanoem_free(keyframe);
    }
}

nanoem_mutable_document_camera_keyframe_t * APIENTRY
nanoemMutableDocumentCameraKeyframeCreate(nanoem_mutable_document_camera_t *camera, nanoem_status_t *status)
{
    nanoem_mutable_document_camera_keyframe_t *keyframe;
    keyframe = (nanoem_mutable_document_camera_keyframe_t *) nanoem_calloc(sizeof(*keyframe), 1, status);
    if (nanoem_is_not_null(keyframe)) {
        keyframe->base.parent.camera = camera;
        keyframe->origin = nanoemDocumentCameraKeyframeCreate(nanoemMutableDocumentCameraGetOrigin(camera), status);
        if (nanoem_is_not_null(keyframe->origin)) {
            nanoemDocumentCameraKeyframeInitialize(keyframe->origin);
        }
    }
    return keyframe;
}

void APIENTRY
nanoemMutableDocumentCameraKeyframeSetParentModelObject(nanoem_mutable_document_camera_keyframe_t *keyframe, const nanoem_document_model_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        keyframe->origin->parent_model_index = nanoemDocumentModelGetIndex(value);
    }
}

void APIENTRY
nanoemMutableDocumentCameraKeyframeSetParentModelBoneName(nanoem_mutable_document_camera_keyframe_t *keyframe, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_mutable_document_t *document;
    int parent_model_index, parent_model_bone_index = 0;
    if (nanoem_is_not_null(keyframe)) {
        if (nanoem_is_not_null(value)) {
            document = keyframe->base.parent.camera->base.parent.document;
            parent_model_index = keyframe->origin->parent_model_index;
            parent_model_bone_index = nanoemMutableDocumentFindParentModelBoneIndex(document, parent_model_index, value);
        }
        keyframe->origin->parent_model_bone_index = parent_model_bone_index;
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentCameraKeyframeSetLookAt(nanoem_mutable_document_camera_keyframe_t *keyframe, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(keyframe->origin->look_at.values, value, sizeof(keyframe->origin->look_at.values));
    }
}

void APIENTRY
nanoemMutableDocumentCameraKeyframeSetAngle(nanoem_mutable_document_camera_keyframe_t *keyframe, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(keyframe->origin->angle.values, value, sizeof(keyframe->origin->angle.values));
    }
}

void APIENTRY
nanoemMutableDocumentCameraKeyframeSetInterpolation(nanoem_mutable_document_camera_keyframe_t *keyframe, nanoem_motion_camera_keyframe_interpolation_type_t type, const nanoem_u8_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)
            && type >= NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM
            && type < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM
            && keyframe->origin->interpolation[type].u.values != value) {
        nanoem_crt_memcpy(keyframe->origin->interpolation[type].u.values, value, sizeof(keyframe->origin->interpolation[type].u.values));
    }
}

void APIENTRY
nanoemMutableDocumentCameraKeyframeSetDistance(nanoem_mutable_document_camera_keyframe_t *keyframe, nanoem_f32_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->distance = value;
    }
}

void APIENTRY
nanoemMutableDocumentCameraKeyframeSetFov(nanoem_mutable_document_camera_keyframe_t *keyframe, int value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->fov = value;
    }
}

void APIENTRY
nanoemMutableDocumentCameraKeyframeSetPerspectiveView(nanoem_mutable_document_camera_keyframe_t *keyframe, nanoem_bool_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->is_perspective_view = value ? nanoem_true : nanoem_false;
    }
}

nanoem_document_camera_keyframe_t* APIENTRY
nanoemMutableDocumentCameraKeyframeGetOrigin(nanoem_mutable_document_camera_keyframe_t* keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->origin : NULL;
}

void APIENTRY
nanoemMutableDocumentCameraKeyframeDestroy(nanoem_mutable_document_camera_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        if (nanoemMutableDocumentObjectCanDelete(&keyframe->base)) {
            nanoemDocumentCameraKeyframeDestroy(keyframe->origin);
        }
        nanoem_free(keyframe);
    }
}

nanoem_mutable_document_gravity_keyframe_t * APIENTRY
nanoemMutableDocumentGravityKeyframeCreate(nanoem_mutable_document_gravity_t *gravity, nanoem_status_t *status)
{
    nanoem_mutable_document_gravity_keyframe_t *keyframe;
    keyframe = (nanoem_mutable_document_gravity_keyframe_t *) nanoem_calloc(sizeof(*keyframe), 1, status);
    if (nanoem_is_not_null(keyframe)) {
        keyframe->base.parent.gravity = gravity;
        keyframe->origin = nanoemDocumentGravityKeyframeCreate(nanoemMutableDocumentGravityGetOrigin(gravity), status);
        if (nanoem_is_not_null(keyframe->origin)) {
            nanoemDocumentGravityKeyframeInitialize(keyframe->origin);
        }
    }
    return keyframe;
}

void APIENTRY
nanoemMutableDocumentGravityKeyframeSetDirection(nanoem_mutable_document_gravity_keyframe_t *keyframe, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(keyframe->origin->direction.values, value, sizeof(keyframe->origin->direction.values));
    }
}

void APIENTRY
nanoemMutableDocumentGravityKeyframeSetAcceleration(nanoem_mutable_document_gravity_keyframe_t *keyframe, nanoem_f32_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->acceleration = value;
    }
}

void APIENTRY
nanoemMutableDocumentGravityKeyframeSetNoise(nanoem_mutable_document_gravity_keyframe_t *keyframe, int value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->noise = value;
    }
}

void APIENTRY
nanoemMutableDocumentGravityKeyframeSetNoiseEnabled(nanoem_mutable_document_gravity_keyframe_t *keyframe, nanoem_bool_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->is_noise_enabled = value ? nanoem_true : nanoem_false;
    }
}

nanoem_document_gravity_keyframe_t *APIENTRY
nanoemMutableDocumentGravityKeyframeGetOrigin(nanoem_mutable_document_gravity_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->origin : NULL;
}

void APIENTRY
nanoemMutableDocumentGravityKeyframeDestroy(nanoem_mutable_document_gravity_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        if (nanoemMutableDocumentObjectCanDelete(&keyframe->base)) {
            nanoemDocumentGravityKeyframeDestroy(keyframe->origin);
        }
        nanoem_free(keyframe);
    }
}

nanoem_mutable_document_light_keyframe_t * APIENTRY
nanoemMutableDocumentLightKeyframeCreate(nanoem_mutable_document_light_t *light, nanoem_status_t *status)
{
    nanoem_mutable_document_light_keyframe_t *keyframe;
    keyframe = (nanoem_mutable_document_light_keyframe_t *) nanoem_calloc(sizeof(*keyframe), 1, status);
    if (nanoem_is_not_null(keyframe)) {
        keyframe->base.parent.light = light;
        keyframe->origin = nanoemDocumentLightKeyframeCreate(nanoemMutableDocumentLightGetOrigin(light), status);
        if (nanoem_is_not_null(keyframe->origin)) {
            nanoemDocumentLightKeyframeInitialize(keyframe->origin);
        }
    }
    return keyframe;
}

void APIENTRY
nanoemMutableDocumentLightKeyframeSetColor(nanoem_mutable_document_light_keyframe_t *keyframe, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(keyframe->origin->color.values, value, sizeof(keyframe->origin->color.values));
    }
}

void APIENTRY
nanoemMutableDocumentLightKeyframeSetDirection(nanoem_mutable_document_light_keyframe_t *keyframe, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(keyframe->origin->direction.values, value, sizeof(keyframe->origin->direction.values));
    }
}

nanoem_document_light_keyframe_t *APIENTRY
nanoemMutableDocumentLightKeyframeGetOrigin(nanoem_mutable_document_light_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->origin : NULL;
}

void APIENTRY
nanoemMutableDocumentLightKeyframeDestroy(nanoem_mutable_document_light_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        if (nanoemMutableDocumentObjectCanDelete(&keyframe->base)) {
            nanoemDocumentLightKeyframeDestroy(keyframe->origin);
        }
        nanoem_free(keyframe);
    }
}

nanoem_mutable_document_model_keyframe_t * APIENTRY
nanoemMutableDocumentModelKeyframeCreate(nanoem_mutable_document_model_t *model, nanoem_status_t *status)
{
    nanoem_mutable_document_model_keyframe_t *keyframe;
    nanoem_document_model_keyframe_t *origin;
    keyframe = (nanoem_mutable_document_model_keyframe_t *) nanoem_calloc(sizeof(*keyframe), 1, status);
    if (nanoem_is_not_null(keyframe)) {
        keyframe->base.parent.model = model;
        origin = keyframe->origin = nanoemDocumentModelKeyframeCreate(nanoemMutableDocumentModelGetOrigin(model), status);
        if (nanoem_is_not_null(origin)) {
            nanoemDocumentModelKeyframeInitialize(origin);
            origin->constraint_states = (nanoem_document_model_constraint_state_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->constraint_states), status);
            origin->outside_parents = (nanoem_document_outside_parent_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->outside_parents), status);
            keyframe->num_allocated_constraint_states = __nanoem_default_allocation_size;
            keyframe->num_allocated_outside_parents = __nanoem_default_allocation_size;
        }
    }
    return keyframe;
}

nanoem_mutable_document_model_keyframe_t * APIENTRY
nanoemMutableDocumentModelKeyframeCreateAsReference(nanoem_mutable_document_model_t *model, nanoem_document_model_keyframe_t *reference, nanoem_status_t *status)
{
    nanoem_mutable_document_model_keyframe_t *keyframe = NULL;
    nanoem_document_model_t *origin_model;
    nanoem_document_keyframe_compare_result_t previous, next;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(reference)) {
        origin_model = model->origin;
        if (nanoemDocumentModelFindModelKeyframeIndex(origin_model, reference->base.frame_index, &previous, &next)) {
            keyframe = (nanoem_mutable_document_model_keyframe_t *) nanoem_calloc(sizeof(*keyframe), 1, status);
            if (nanoem_is_not_null(model)) {
                keyframe->base.parent.model = model;
                keyframe->origin = reference;
                keyframe->base.is_reference = nanoem_true;
            }
        }
    }
    return keyframe;
}

void APIENTRY
nanoemMutableDocumentModelKeyframeSetConstraintEnabled(nanoem_mutable_document_model_keyframe_t *keyframe, const nanoem_unicode_string_t *name, nanoem_bool_t value, nanoem_status_t *status)
{
    nanoem_mutable_document_model_t *parent_model;
    nanoem_document_model_t *origin_model;
    nanoem_document_model_constraint_state_t *constraint_state, **constraint_states;
    nanoem_document_model_keyframe_t *origin_keyframe;
    nanoem_rsize_t index;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(name)) {
        origin_keyframe = keyframe->origin;
        parent_model = keyframe->base.parent.model;
        index = nanoemMutableDocumentModelResolveConstraintBoneId(parent_model, name, status);
        if (index != NANOEM_RSIZE_MAX) {
            if (index >= origin_keyframe->num_constraint_states) {
                constraint_states = (nanoem_document_model_constraint_state_t **) nanoemMutableObjectArrayResize(origin_keyframe->constraint_states, &keyframe->num_allocated_constraint_states, &origin_keyframe->num_constraint_states, status);
                if (nanoem_is_not_null(constraint_states)) {
                    origin_model = nanoemMutableDocumentModelGetOrigin(parent_model);
                    constraint_state = nanoemDocumentModelConstraintStateCreate(origin_model, status);
                    if (nanoem_is_not_null(constraint_state)) {
                        constraint_states[index] = constraint_state;
                    }
                }
            }
            else {
                origin_keyframe->constraint_states[index]->enabled = value;
            }
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelKeyframeInsertOutsideParentObject(nanoem_mutable_document_model_keyframe_t *keyframe, nanoem_mutable_document_outside_parent_t *parent, int index, nanoem_status_t *status)
{
    nanoem_document_model_keyframe_t *origin_keyframe;
    nanoem_document_outside_parent_t *origin_outside_parent;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(parent)) {
        origin_keyframe = keyframe->origin;
        origin_outside_parent = parent->origin;
        nanoemDocumentObjectArrayInsertObject((nanoem_document_object_t ***) &origin_keyframe->outside_parents, (nanoem_document_object_t *) origin_outside_parent, index, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_ALREADY_EXISTS, &origin_keyframe->num_outside_parents, &keyframe->num_allocated_outside_parents, status);
        if (!nanoem_status_ptr_has_error(status)) {
            origin_outside_parent->base.parent.model = origin_keyframe->parent_model;
            parent->base.is_in_object = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelKeyframeRemoveOutsideParentObject(nanoem_mutable_document_model_keyframe_t *keyframe, nanoem_mutable_document_outside_parent_t *parent, nanoem_status_t *status)
{
    nanoem_document_model_keyframe_t *origin_keyframe;
    nanoem_document_outside_parent_t *origin_outside_parent;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(parent)) {
        origin_keyframe = keyframe->origin;
        origin_outside_parent = parent->origin;
        nanoemDocumentObjectArrayRemoveObject((nanoem_document_object_t **) origin_keyframe->outside_parents, (nanoem_document_object_t *) origin_outside_parent, &origin_keyframe->num_outside_parents, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            origin_outside_parent->base.parent.model = NULL;
            parent->base.is_in_object = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelKeyframeSetVisible(nanoem_mutable_document_model_keyframe_t *keyframe, nanoem_bool_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->visible = value ? nanoem_true : nanoem_false;
    }
}

nanoem_document_model_keyframe_t *APIENTRY
nanoemMutableDocumentModelKeyframeGetOrigin(nanoem_mutable_document_model_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->origin : NULL;
}

void APIENTRY
nanoemMutableDocumentModelKeyframeDestroy(nanoem_mutable_document_model_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        if (nanoemMutableDocumentObjectCanDelete(&keyframe->base)) {
            nanoemDocumentModelKeyframeDestroy(keyframe->origin);
        }
        nanoem_free(keyframe);
    }
}

nanoem_mutable_document_model_morph_keyframe_t * APIENTRY
nanoemMutableDocumentModelMorphKeyframeCreate(nanoem_mutable_document_model_t *model, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_mutable_document_model_morph_keyframe_t *keyframe;
    nanoem_document_model_morph_keyframe_t *origin;
    keyframe = (nanoem_mutable_document_model_morph_keyframe_t *) nanoem_calloc(sizeof(*keyframe), 1, status);
    if (nanoem_is_not_null(keyframe)) {
        origin = keyframe->origin = nanoemDocumentModelMorphKeyframeCreate(nanoemMutableDocumentModelGetOrigin(model), status);
        if (nanoem_is_not_null(origin)) {
            keyframe->base.parent.model = model;
            keyframe->name = nanoemUnicodeStringFactoryCloneString(model->origin->base.parent.document->factory, value, status);
        }
    }
    return keyframe;
}

void APIENTRY
nanoemMutableDocumentModelMorphKeyframeSetWeight(nanoem_mutable_document_model_morph_keyframe_t *keyframe, nanoem_f32_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->weight = value;
    }
}

nanoem_document_model_morph_keyframe_t *APIENTRY
nanoemMutableDocumentModelMorphKeyframeGetOrigin(nanoem_mutable_document_model_morph_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->origin : NULL;
}

void APIENTRY
nanoemMutableDocumentModelMorphKeyframeDestroy(nanoem_mutable_document_model_morph_keyframe_t *keyframe)
{
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(keyframe)) {
        factory = keyframe->origin->parent_model->base.parent.document->factory;
        if (nanoemMutableDocumentObjectCanDelete(&keyframe->base)) {
            nanoemDocumentModelMorphKeyframeDestroy(keyframe->origin);
        }
        nanoemUnicodeStringFactoryDestroyString(factory, keyframe->name);
        nanoem_free(keyframe);
    }
}

nanoem_mutable_document_self_shadow_keyframe_t * APIENTRY
nanoemMutableDocumentSelfShadowKeyframeCreate(nanoem_mutable_document_self_shadow_t *self_shadow, nanoem_status_t *status)
{
    nanoem_mutable_document_self_shadow_keyframe_t *keyframe;
    keyframe = (nanoem_mutable_document_self_shadow_keyframe_t *) nanoem_calloc(sizeof(*keyframe), 1, status);
    if (nanoem_is_not_null(keyframe)) {
        keyframe->base.parent.self_shadow = self_shadow;
        keyframe->origin = nanoemDocumentSelfShadowKeyframeCreate(nanoemMutableDocumentSelfShadowGetOrigin(self_shadow), status);
        if (nanoem_is_not_null(keyframe->origin)) {
            nanoemDocumentSelfShadowKeyframeInitialize(keyframe->origin);
        }
    }
    return keyframe;
}

void APIENTRY
nanoemMutableDocumentSelfShadowKeyframeSetDistance(nanoem_mutable_document_self_shadow_keyframe_t *keyframe, nanoem_f32_t value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->distance = value;
    }
}

void APIENTRY
nanoemMutableDocumentSelfShadowKeyframeSetMode(nanoem_mutable_document_self_shadow_keyframe_t *keyframe, int value)
{
    if (nanoem_is_not_null(keyframe)) {
        keyframe->origin->mode = value;
    }
}

nanoem_document_self_shadow_keyframe_t *APIENTRY
nanoemMutableDocumentSelfShadowKeyframeGetOrigin(nanoem_mutable_document_self_shadow_keyframe_t *keyframe)
{
    return nanoem_is_not_null(keyframe) ? keyframe->origin : NULL;
}

void APIENTRY
nanoemMutableDocumentSelfShadowKeyframeDestroy(nanoem_mutable_document_self_shadow_keyframe_t *keyframe)
{
    if (nanoem_is_not_null(keyframe)) {
        if (nanoemMutableDocumentObjectCanDelete(&keyframe->base)) {
            nanoemDocumentSelfShadowKeyframeDestroy(keyframe->origin);
        }
        nanoem_free(keyframe);
    }
}

nanoem_mutable_document_model_bone_state_t * APIENTRY
nanoemMutableDocumentModelBoneStateCreate(nanoem_mutable_document_model_t *model, nanoem_status_t *status)
{
    nanoem_mutable_document_model_bone_state_t *state;
    state = (nanoem_mutable_document_model_bone_state_t *) nanoem_calloc(sizeof(*state), 1, status);
    if (nanoem_is_not_null(state)) {
        state->base.parent.model = model;
        state->origin = nanoemDocumentModelBoneStateCreate(nanoemMutableDocumentModelGetOrigin(model), status);
    }
    return state;
}

void APIENTRY
nanoemMutableDocumentModelBoneStateSetTranslation(nanoem_mutable_document_model_bone_state_t *state, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(state) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(state->origin->translation.values, value, sizeof(state->origin->translation.values));
    }
}

void APIENTRY
nanoemMutableDocumentModelBoneStateSetOrientation(nanoem_mutable_document_model_bone_state_t *state, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(state) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(state->origin->orientation.values, value, sizeof(state->origin->orientation.values));
    }
}

void APIENTRY
nanoemMutableDocumentModelBoneStateSetPhysicsSimulationDisabled(nanoem_mutable_document_model_bone_state_t *state, nanoem_bool_t value)
{
    if (nanoem_is_not_null(state)) {
        state->origin->disable_physics_simulation = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentModelBoneStateDestroy(nanoem_mutable_document_model_bone_state_t *state)
{
    if (nanoem_is_not_null(state)) {
        if (nanoemMutableDocumentObjectCanDelete(&state->base)) {
            nanoemDocumentModelBoneStateDestroy(state->origin);
        }
        nanoem_free(state);
    }
}

nanoem_mutable_document_model_constraint_state_t * APIENTRY
nanoemMutableDocumentModelConstraintStateCreate(nanoem_mutable_document_model_t *model, nanoem_status_t *status)
{
    nanoem_mutable_document_model_constraint_state_t *state;
    state = (nanoem_mutable_document_model_constraint_state_t *) nanoem_calloc(sizeof(*state), 1, status);
    if (nanoem_is_not_null(state)) {
        state->base.parent.model = model;
        state->origin = nanoemDocumentModelConstraintStateCreate(nanoemMutableDocumentModelGetOrigin(model), status);
    }
    return state;
}

void APIENTRY
nanoemMutableDocumentModelConstraintStateSetEnabled(nanoem_mutable_document_model_constraint_state_t *state, nanoem_bool_t value)
{
    if (nanoem_is_not_null(state)) {
        state->origin->enabled = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentModelConstraintStateDestroy(nanoem_mutable_document_model_constraint_state_t *state)
{
    if (nanoem_is_not_null(state)) {
        if (nanoemMutableDocumentObjectCanDelete(&state->base)) {
            nanoemDocumentModelConstraintStateDestroy(state->origin);
        }
        nanoem_free(state);
    }
}

nanoem_mutable_document_model_morph_state_t * APIENTRY
nanoemMutableDocumentModelMorphStateCreate(nanoem_mutable_document_model_t *model, nanoem_status_t *status)
{
    nanoem_mutable_document_model_morph_state_t *state;
    state = (nanoem_mutable_document_model_morph_state_t *) nanoem_calloc(sizeof(*state), 1, status);
    if (nanoem_is_not_null(state)) {
        state->base.parent.model = model;
        state->origin = nanoemDocumentModelMorphStateCreate(nanoemMutableDocumentModelGetOrigin(model), status);
    }
    return state;
}

void APIENTRY
nanoemMutableDocumentModelMorphStateSetWeight(nanoem_mutable_document_model_morph_state_t *state, nanoem_f32_t value)
{
    if (nanoem_is_not_null(state)) {
        state->origin->weight = value;
    }
}

void APIENTRY
nanoemMutableDocumentModelMorphStateDestroy(nanoem_mutable_document_model_morph_state_t *state)
{
    if (nanoem_is_not_null(state)) {
        if (nanoemMutableDocumentObjectCanDelete(&state->base)) {
            nanoemDocumentModelMorphStateDestroy(state->origin);
        }
        nanoem_free(state);
    }
}

nanoem_mutable_document_model_outside_parent_state_t * APIENTRY
nanoemMutableDocumentModelOutsideParentStateCreate(nanoem_mutable_document_model_t *model, nanoem_status_t *status)
{
    nanoem_mutable_document_model_outside_parent_state_t *state;
    state = (nanoem_mutable_document_model_outside_parent_state_t *) nanoem_calloc(sizeof(*state), 1, status);
    if (nanoem_is_not_null(state)) {
        state->base.parent.model = model;
        state->origin = nanoemDocumentModelOutsideParentStateCreate(nanoemMutableDocumentModelGetOrigin(model), status);
    }
    return state;
}

void APIENTRY
nanoemMutableDocumentModelOutsideParentStateSetBeginFrameIndex(nanoem_mutable_document_model_outside_parent_state_t *state, nanoem_frame_index_t value)
{
    if (nanoem_is_not_null(state)) {
        state->origin->begin = value;
    }
}

void APIENTRY
nanoemMutableDocumentModelOutsideParentStateSetEndFrameIndex(nanoem_mutable_document_model_outside_parent_state_t *state, nanoem_frame_index_t value)
{
    if (nanoem_is_not_null(state)) {
        state->origin->end = value;
    }
}

void APIENTRY
nanoemMutableDocumentModelOutsideParentStateSetTargetModelObject(nanoem_mutable_document_model_outside_parent_state_t *state, const nanoem_document_model_t *value)
{
    nanoem_mutable_document_t *parent_document;
    nanoem_mutable_document_model_t *parent_model;
    nanoem_rsize_t index;
    int model_index;
    if (nanoem_is_not_null(state) && nanoem_is_not_null(value)) {
        parent_model = state->base.parent.model;
        parent_document = parent_model->base.parent.document;
        index = (nanoem_rsize_t) value->base.index;
        if (index < parent_document->origin->num_models) {
            model_index = parent_document->origin->models[index]->base.index;
            state->origin->outside_parent->model_index = model_index;
        }
    }
}

void APIENTRY
nanoemMutableDocumentModelOutsideParentStateSetTargetBoneName(nanoem_mutable_document_model_outside_parent_state_t *state, const nanoem_unicode_string_t *value)
{
    nanoem_unicode_string_factory_t *factory;
    nanoem_mutable_document_t *parent_document;
    nanoem_mutable_document_model_t *parent_model;
    nanoem_document_model_t *origin_model;
    nanoem_unicode_string_t **bone_names, *bone_name;
    nanoem_rsize_t num_bone_names, index, i;
    int bone_index = -1;
    if (nanoem_is_not_null(state) && nanoem_is_not_null(value)) {
        parent_model = state->base.parent.model;
        parent_document = parent_model->base.parent.document;
        factory = parent_document->origin->factory;
        index = (nanoem_rsize_t) state->origin->outside_parent->model_index;
        if (index < parent_document->origin->num_models) {
            origin_model = parent_document->origin->models[index];
            bone_names = origin_model->bone_names;
            num_bone_names = origin_model->num_bones;
            for (i = 0; i < num_bone_names; i++) {
                bone_name = bone_names[i];
                if (nanoemUnicodeStringFactoryCompareString(factory, bone_name, value) == 0) {
                    bone_index = i;
                    break;
                }
            }
            if (bone_index != -1) {
                state->origin->outside_parent->bone_index = bone_index;
            }
        }
    }
}

void APIENTRY
nanoemMutableDocumentModelOutsideParentStateDestroy(nanoem_mutable_document_model_outside_parent_state_t *state)
{
    if (nanoem_is_not_null(state)) {
        if (nanoemMutableDocumentObjectCanDelete(&state->base)) {
            nanoemDocumentModelOutsideParentStateDestroy(state->origin);
        }
        nanoem_free(state);
    }
}

nanoem_mutable_document_model_t * APIENTRY
nanoemMutableDocumentModelCreate(nanoem_mutable_document_t *document, nanoem_status_t *status)
{
    nanoem_mutable_document_model_t *model;
    nanoem_document_model_t *origin;
    model = (nanoem_mutable_document_model_t *) nanoem_calloc(sizeof(*model), 1, status);
    if (nanoem_is_not_null(model)) {
        model->base.parent.document = document;
        origin = model->origin = nanoemDocumentModelCreate(nanoemMutableDocumentGetOrigin(document), status);
        if (nanoem_is_not_null(origin)) {
            origin->all_bone_keyframes_ptr = (nanoem_document_model_bone_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->all_bone_keyframes_ptr), status);
            origin->all_model_keyframes_ptr = (nanoem_document_model_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->all_model_keyframes_ptr), status);
            origin->all_morph_keyframes_ptr = (nanoem_document_model_morph_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->all_morph_keyframes_ptr), status);
            origin->bone_keyframes = (nanoem_document_model_bone_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->bone_keyframes), status);
            origin->bone_states = (nanoem_document_model_bone_state_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->bone_states), status);
            origin->bone_names = (nanoem_unicode_string_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->bone_names), status);
            origin->constraint_bone_indices = (int *) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->constraint_bone_indices), status);
            origin->constraint_states = (nanoem_document_model_constraint_state_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->constraint_states), status);
            origin->expansion_state = (nanoem_u8_t *) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->expansion_state), status);
            origin->initial_bone_keyframes = (nanoem_document_model_bone_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->initial_bone_keyframes), status);
            origin->initial_model_keyframe = nanoemDocumentModelKeyframeCreate(origin, status);
            origin->initial_morph_keyframes = (nanoem_document_model_morph_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->initial_morph_keyframes), status);
            origin->morph_keyframes = (nanoem_document_model_morph_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->morph_keyframes), status);
            origin->morph_states = (nanoem_document_model_morph_state_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->morph_states), status);
            origin->morph_names = (nanoem_unicode_string_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->morph_names), status);
            origin->outside_parent_subject_bone_indices = (int *) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->outside_parent_subject_bone_indices), status);
            origin->outside_parent_states = (nanoem_document_model_outside_parent_state_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->outside_parent_states), status);
            origin->all_model_keyframes_ptr[0] = origin->initial_model_keyframe;
            origin->num_all_model_keyframes = 1;
            model->num_allocated_all_bone_keyframes = __nanoem_default_allocation_size;
            model->num_allocated_all_model_keyframes = __nanoem_default_allocation_size;
            model->num_allocated_all_morph_keyframes = __nanoem_default_allocation_size;
            model->num_allocated_bone_keyframes = __nanoem_default_allocation_size;
            model->num_allocated_bone_states = __nanoem_default_allocation_size;
            model->num_allocated_bones = __nanoem_default_allocation_size;
            model->num_allocated_constraint_states = __nanoem_default_allocation_size;
            model->num_allocated_expansion_states = __nanoem_default_allocation_size;
            model->num_allocated_inititial_bone_keyframes = __nanoem_default_allocation_size;
            model->num_allocated_inititial_morph_keyframes = __nanoem_default_allocation_size;
            model->num_allocated_morph_keyframes = __nanoem_default_allocation_size;
            model->num_allocated_morph_states = __nanoem_default_allocation_size;
            model->num_allocated_morphs = __nanoem_default_allocation_size;
            model->num_allocated_outside_parent_states = __nanoem_default_allocation_size;
            model->num_allocated_outside_parent_subject_bones = __nanoem_default_allocation_size;
            if (nanoem_is_not_null(origin->initial_model_keyframe)) {
                nanoemDocumentModelKeyframeInitialize(origin->initial_model_keyframe);
            }
            origin->visible = origin->is_self_shadow_enabled = nanoem_true;
        }
    }
    return model;
}

nanoem_mutable_document_model_t * APIENTRY
nanoemMutableDocumentModelCreateAsReference(nanoem_mutable_document_t *document, nanoem_document_model_t *reference, nanoem_status_t *status)
{
    nanoem_mutable_document_model_t *model = NULL;
    nanoem_document_t *origin_document;
    if (nanoem_is_not_null(document) && nanoem_is_not_null(reference)) {
        origin_document = document->origin;
        if (nanoemObjectArrayContainsIndexedObject((const void* const *) origin_document->models, reference, origin_document->num_models, reference->base.index)) {
            model = (nanoem_mutable_document_model_t *) nanoem_calloc(sizeof(*model), 1, status);
            if (nanoem_is_not_null(model)) {
                model->base.parent.document = document;
                model->origin = reference;
                model->base.is_reference = nanoem_true;
            }
        }
    }
    return model;
}

int APIENTRY
nanoemMutableDocumentModelRegisterBone(nanoem_mutable_document_model_t *model, const nanoem_unicode_string_t *name, nanoem_status_t *status)
{
    nanoem_document_model_bone_state_t **bone_states;
    nanoem_document_model_bone_keyframe_t **bone_keyframes, **all_bone_keyframes, *initial_bone_keyframe;
    nanoem_unicode_string_factory_t *factory;
    nanoem_document_model_t *origin_model;
    nanoem_unicode_string_t **bone_names;
    nanoem_rsize_t num_allocated_bones, num_bones;
    int bone_index = -1;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(name)) {
        origin_model = model->origin;
        factory = nanoemDocumentModelGetParentDocument(model->origin)->factory;
        num_allocated_bones = model->num_allocated_bones;
        num_bones = origin_model->num_bones;
        bone_names = (nanoem_unicode_string_t **) nanoemMutableObjectArrayResize(origin_model->bone_names,
            &model->num_allocated_bones,
            &origin_model->num_bones, status);
        if (nanoem_is_not_null(bone_names)) {
            bone_index = origin_model->num_bones - 1;
            bone_names[bone_index] = nanoemUnicodeStringFactoryCloneString(factory, name, status);
            origin_model->bone_names = bone_names;
        }
        else {
            return -1;
        }
        bone_keyframes = (nanoem_document_model_bone_keyframe_t **) nanoemMutableObjectArrayResize(origin_model->initial_bone_keyframes,
            &model->num_allocated_inititial_bone_keyframes,
            &origin_model->num_initial_bone_keyframes, status);
        if (nanoem_is_not_null(bone_keyframes)) {
            bone_index = origin_model->num_initial_bone_keyframes - 1;
            initial_bone_keyframe = bone_keyframes[bone_index] = nanoemDocumentModelBoneKeyframeCreate(origin_model, status);
            origin_model->initial_bone_keyframes = bone_keyframes;
            nanoem_status_ptr_assign_succeeded(status);
        }
        else {
            return -1;
        }
        all_bone_keyframes = (nanoem_document_model_bone_keyframe_t **) nanoemMutableObjectArrayResize(origin_model->all_bone_keyframes_ptr,
            &model->num_allocated_all_bone_keyframes,
            &origin_model->num_all_bone_keyframes, status);
        if (nanoem_is_not_null(all_bone_keyframes)) {
            bone_index = origin_model->num_all_bone_keyframes - 1;
            all_bone_keyframes[bone_index] = initial_bone_keyframe;
            origin_model->all_bone_keyframes_ptr = all_bone_keyframes;
            nanoem_status_ptr_assign_succeeded(status);
        }
        else {
            return -1;
        }
        bone_states = (nanoem_document_model_bone_state_t **) nanoemMutableObjectArrayResize(origin_model->bone_states,
            &num_allocated_bones,
            &num_bones, status);
        if (nanoem_is_not_null(bone_states)) {
            bone_index = num_bones - 1;
            bone_states[bone_index] = nanoemDocumentModelBoneStateCreate(origin_model, status);
            origin_model->bone_states = bone_states;
            nanoem_status_ptr_assign_succeeded(status);
        }
        else {
            return -1;
        }
    }
    return bone_index;
}

int APIENTRY
nanoemMutableDocumentModelRegisterMorph(nanoem_mutable_document_model_t *model, const nanoem_unicode_string_t *name, nanoem_status_t *status)
{
    nanoem_document_model_morph_state_t **morph_states;
    nanoem_document_model_morph_keyframe_t **morph_keyframes, **all_morph_keyframes, *initial_morph_keyframe;
    nanoem_unicode_string_factory_t *factory;
    nanoem_document_model_t *origin_model;
    nanoem_unicode_string_t **morph_names;
    nanoem_rsize_t num_allocated_morphs, num_morphs;
    int morph_index = -1;
    if (morph_index == -1) {
        factory = nanoemDocumentModelGetParentDocument(model->origin)->factory;
        origin_model = model->origin;
        morph_names = origin_model->morph_names;
        num_allocated_morphs = model->num_allocated_morphs;
        num_morphs = origin_model->num_morphs;
        morph_names = (nanoem_unicode_string_t **) nanoemMutableObjectArrayResize(origin_model->morph_names,
            &model->num_allocated_morphs,
            &origin_model->num_morphs, status);
        if (nanoem_is_not_null(morph_names)) {
            morph_index = origin_model->num_morphs - 1;
            morph_names[morph_index] = nanoemUnicodeStringFactoryCloneString(factory, name, status);
            origin_model->morph_names = morph_names;
        }
        else {
            return -1;
        }
        morph_keyframes = (nanoem_document_model_morph_keyframe_t **) nanoemMutableObjectArrayResize(origin_model->initial_morph_keyframes,
            &model->num_allocated_inititial_morph_keyframes,
            &origin_model->num_initial_morph_keyframes, status);
        if (nanoem_is_not_null(morph_keyframes)) {
            morph_index = origin_model->num_initial_morph_keyframes - 1;
            initial_morph_keyframe = morph_keyframes[morph_index] = nanoemDocumentModelMorphKeyframeCreate(origin_model, status);
            origin_model->initial_morph_keyframes = morph_keyframes;
            nanoem_status_ptr_assign_succeeded(status);
        }
        else {
            return -1;
        }
        all_morph_keyframes = (nanoem_document_model_morph_keyframe_t **) nanoemMutableObjectArrayResize(origin_model->all_morph_keyframes_ptr,
            &model->num_allocated_all_morph_keyframes,
            &origin_model->num_all_morph_keyframes, status);
        if (nanoem_is_not_null(all_morph_keyframes)) {
            morph_index = origin_model->num_all_morph_keyframes - 1;
            all_morph_keyframes[morph_index] = initial_morph_keyframe;
            origin_model->all_morph_keyframes_ptr = all_morph_keyframes;
            nanoem_status_ptr_assign_succeeded(status);
        }
        else {
            return -1;
        }
        morph_states = (nanoem_document_model_morph_state_t **) nanoemMutableObjectArrayResize(origin_model->morph_states,
            &num_allocated_morphs,
            &num_morphs, status);
        if (nanoem_is_not_null(morph_states)) {
            morph_index = num_morphs - 1;
            morph_states[morph_index] = nanoemDocumentModelMorphStateCreate(origin_model, status);
            origin_model->morph_states = morph_states;
            nanoem_status_ptr_assign_succeeded(status);
        }
        else {
            return -1;
        }
    }
    return morph_index;
}

static void
nanoemMutableDocumentModelUpdateAllBoneKeyframeObjects(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_bone_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_document_keyframe_compare_result_t previous, nanoem_document_keyframe_compare_result_t next, nanoem_status_t *status)
{
    nanoem_document_model_bone_keyframe_t **all_keyframes;
    nanoem_document_model_t *origin_model = model->origin;
    nanoem_document_model_bone_keyframe_t *origin_keyframe = keyframe->origin;
    nanoem_rsize_t index;
    all_keyframes = (nanoem_document_model_bone_keyframe_t **) nanoemMutableObjectArrayResize(origin_model->all_bone_keyframes_ptr,
        &model->num_allocated_all_bone_keyframes,
        &origin_model->num_all_bone_keyframes, status);
    if (nanoem_is_not_null(all_keyframes)) {
        index = origin_model->num_all_bone_keyframes - 1;
        all_keyframes[index] = origin_keyframe;
        origin_model->all_bone_keyframes_ptr = all_keyframes;
        keyframe->base.is_in_object = nanoem_true;
        nanoemDocumentBaseKeyframeSet(&origin_keyframe->base, frame_index, index, &previous, &next);
        nanoemDocumentKeyframeCompareResultSetKeyframeIndex(&previous, &next, index);
        nanoem_status_ptr_assign_succeeded(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelAddBoneKeyframeObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_bone_keyframe_t *keyframe, const nanoem_unicode_string_t *name,  nanoem_frame_index_t frame_index, nanoem_status_t *status)
{
    nanoem_document_model_bone_keyframe_t *origin_keyframe, **keyframes, *initial_keyframe;
    nanoem_document_model_t *origin_model;
    nanoem_document_keyframe_compare_result_t previous, next;
    nanoem_rsize_t index;
    int bone_index;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(keyframe) && nanoem_is_not_null(name)) {
        origin_model = model->origin;
        origin_keyframe = keyframe->origin;
        bone_index = nanoemMutableDocumentModelResolveBoneId(model, name);
        nanoemDocumentKeyframeCompareResultReset(&previous);
        nanoemDocumentKeyframeCompareResultReset(&next);
        if (bone_index == -1) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_BONE_NOT_FOUND);
            return;
        }
        else if (frame_index == 0) {
            initial_keyframe = origin_model->initial_bone_keyframes[bone_index];
            nanoem_crt_memcpy(initial_keyframe->translation.values, origin_keyframe->translation.values, sizeof(initial_keyframe->translation.values));
            nanoem_crt_memcpy(initial_keyframe->orientation.values, origin_keyframe->orientation.values, sizeof(initial_keyframe->orientation.values));
            nanoem_crt_memcpy(initial_keyframe->interpolation, origin_keyframe->interpolation, sizeof(initial_keyframe->interpolation));
            initial_keyframe->is_physics_simulation_disabled = origin_keyframe->is_physics_simulation_disabled;
            initial_keyframe->base.is_selected = origin_keyframe->base.is_selected;
            initial_keyframe->base.object_index = bone_index;
            return;
        }
        else if (nanoemDocumentModelFindBoneKeyframeIndex(origin_model, bone_index, frame_index, &previous, &next)) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_KEYFRAME_ALREADY_EXISTS);
            return;
        }
        keyframes = (nanoem_document_model_bone_keyframe_t **) nanoemMutableObjectArrayResize(origin_model->bone_keyframes,
            &model->num_allocated_bone_keyframes,
            &origin_model->num_bone_keyframes, status);
        if (nanoem_is_not_null(keyframes)) {
            index = origin_model->num_bone_keyframes - 1;
            keyframes[index] = origin_keyframe;
            origin_model->bone_keyframes = keyframes;
            nanoemMutableDocumentModelUpdateAllBoneKeyframeObjects(model, keyframe, frame_index, previous, next, status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelAddModelBoneStateObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_bone_state_t *state, const nanoem_unicode_string_t *name, nanoem_status_t *status)
{
    nanoem_document_model_t *origin_model;
    nanoem_document_model_bone_state_t *origin_state;
    int index;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(state)) {
        origin_model = model->origin;
        origin_state = state->origin;
        index = nanoemMutableDocumentModelResolveBoneId(model, name);
        if (index == -1) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_BONE_NOT_FOUND);
            return;
        }
        nanoemDocumentObjectArrayInsertObject((nanoem_document_object_t ***) &origin_model->bone_states, (nanoem_document_object_t *) origin_state, index, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_STATE_ALREADY_EXISTS, &origin_model->num_bone_states, &model->num_allocated_bone_states, status);
        if (!nanoem_status_ptr_has_error(status)) {
            origin_state->base.parent.model = origin_model;
            state->base.is_in_object = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelAddModelConstraintStateObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_constraint_state_t *state, const nanoem_unicode_string_t *name, nanoem_status_t *status)
{
    nanoem_document_model_t *origin_model;
    nanoem_document_model_constraint_state_t *origin_state;
    int index;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(state)) {
        origin_model = model->origin;
        origin_state = state->origin;
        index = nanoemMutableDocumentModelResolveConstraintBoneId(model, name, status);
        if (index == -1) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_BONE_NOT_FOUND);
            return;
        }
        nanoemDocumentObjectArrayInsertObject((nanoem_document_object_t ***) &origin_model->constraint_states, (nanoem_document_object_t *) origin_state, index, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_CONSTRAINT_STATE_ALREADY_EXISTS, &origin_model->num_constraint_states, &model->num_allocated_constraint_states, status);
        if (!nanoem_status_ptr_has_error(status)) {
            origin_state->base.parent.model = origin_model;
            state->base.is_in_object = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelAddModelMorphStateObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_morph_state_t *state, const nanoem_unicode_string_t *name, nanoem_status_t *status)
{
    nanoem_document_model_t *origin_model;
    nanoem_document_model_morph_state_t *origin_state;
    int index = -1;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(state)) {
        origin_model = model->origin;
        origin_state = state->origin;
        index = nanoemMutableDocumentModelResolveMorphId(model, name);
        if (index == -1) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_MORPH_NOT_FOUND);
            return;
        }
        nanoemDocumentObjectArrayInsertObject((nanoem_document_object_t ***) &origin_model->morph_states, (nanoem_document_object_t *) origin_state, index, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_STATE_ALREADY_EXISTS, &origin_model->num_morph_states, &model->num_allocated_morph_states, status);
        if (!nanoem_status_ptr_has_error(status)) {
            origin_state->base.parent.model = origin_model;
            state->base.is_in_object = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

static void
nanoemMutableDocumentModelUpdateAllModelKeyframeObjects(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_document_keyframe_compare_result_t previous, nanoem_document_keyframe_compare_result_t next, nanoem_status_t *status)
{
    nanoem_document_model_keyframe_t **all_keyframes;
    nanoem_document_model_t *origin_model = model->origin;
    nanoem_document_model_keyframe_t *origin_keyframe = keyframe->origin;
    nanoem_rsize_t index;
    all_keyframes = (nanoem_document_model_keyframe_t **) nanoemMutableObjectArrayResize(origin_model->all_model_keyframes_ptr,
        &model->num_allocated_all_model_keyframes,
        &origin_model->num_all_model_keyframes, status);
    if (nanoem_is_not_null(all_keyframes)) {
        index = origin_model->num_all_model_keyframes - 1;
        all_keyframes[index] = origin_keyframe;
        origin_model->all_model_keyframes_ptr = all_keyframes;
        keyframe->base.is_in_object = nanoem_true;
        nanoemDocumentBaseKeyframeSet(&origin_keyframe->base, frame_index, index, &previous, &next);
        nanoemDocumentKeyframeCompareResultSetKeyframeIndex(&previous, &next, index);
        nanoem_status_ptr_assign_succeeded(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelAddModelKeyframeObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status)
{
    nanoem_document_model_t *origin_model;
    nanoem_document_model_keyframe_t **keyframes, *origin_keyframe, *initial_keyframe;
    nanoem_document_model_constraint_state_t *src_state, *dst_state;
    nanoem_document_outside_parent_t *src_op, *dst_op;
    nanoem_document_keyframe_compare_result_t previous, next;
    nanoem_rsize_t index, num_constraint_states, num_outside_parents, i;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(keyframe)) {
        origin_model = model->origin;
        origin_keyframe = keyframe->origin;
        nanoemDocumentKeyframeCompareResultReset(&previous);
        nanoemDocumentKeyframeCompareResultReset(&next);
        if (frame_index == 0) {
            initial_keyframe = origin_model->initial_model_keyframe;
            initial_keyframe->base.is_selected = origin_keyframe->base.is_selected;
            initial_keyframe->visible = origin_keyframe->visible;
            num_constraint_states = origin_keyframe->num_constraint_states;
            if (num_constraint_states > 0) {
                initial_keyframe->constraint_states = (nanoem_document_model_constraint_state_t **) nanoem_calloc(num_constraint_states, sizeof(*initial_keyframe->constraint_states), status);
                if (nanoem_is_not_null(initial_keyframe->constraint_states)) {
                    initial_keyframe->num_constraint_states = num_constraint_states;
                    for (i = 0; i < num_constraint_states; i++) {
                        src_state = origin_keyframe->constraint_states[i];
                        dst_state = nanoemDocumentModelConstraintStateCreate(origin_model, status);
                        if (nanoem_is_not_null(dst_state)) {
                            dst_state->enabled = src_state->enabled;
                            initial_keyframe->constraint_states[i] = dst_state;
                        }
                        else {
                            initial_keyframe->num_constraint_states = i;
                            break;
                        }
                    }
                }
            }
            num_outside_parents = origin_keyframe->num_outside_parents;
            if (num_outside_parents > 0) {
                initial_keyframe->outside_parents = (nanoem_document_outside_parent_t **) nanoem_calloc(num_outside_parents, sizeof(*initial_keyframe->outside_parents), status);
                if (nanoem_is_not_null(initial_keyframe->outside_parents)) {
                    initial_keyframe->num_outside_parents = num_outside_parents;
                    for (i = 0; i < num_outside_parents; i++) {
                        src_op = origin_keyframe->outside_parents[i];
                        dst_op = nanoemDocumentOutsideParentCreate(origin_model->base.parent.document, status);
                        if (nanoem_is_not_null(dst_state)) {
                            dst_op->bone_index = src_op->bone_index;
                            dst_op->model_index = src_op->model_index;
                            initial_keyframe->outside_parents[i] = dst_op;
                        }
                        else {
                            initial_keyframe->num_outside_parents = i;
                            break;
                        }
                    }
                }
            }
            return;
        }
        else if (nanoemDocumentModelFindModelKeyframeIndex(origin_model, frame_index, &previous, &next)) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MODEL_KEYFRAME_ALREADY_EXISTS);
            return;
        }
        keyframes = (nanoem_document_model_keyframe_t **) nanoemMutableObjectArrayResize(origin_model->model_keyframes,
            &model->num_allocated_model_keyframes,
            &origin_model->num_model_keyframes, status);
        if (nanoem_is_not_null(keyframes)) {
            index = origin_model->num_model_keyframes - 1;
            keyframes[index] = origin_keyframe;
            origin_model->model_keyframes = keyframes;
            nanoemMutableDocumentModelUpdateAllModelKeyframeObjects(model, keyframe, frame_index, previous, next, status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

static void
nanoemMutableDocumentModelUpdateAllMorphKeyframeObjects(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_morph_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_document_keyframe_compare_result_t previous, nanoem_document_keyframe_compare_result_t next, nanoem_status_t *status)
{
    nanoem_document_model_morph_keyframe_t **all_keyframes;
    nanoem_document_model_t *origin_model = model->origin;
    nanoem_document_model_morph_keyframe_t *origin_keyframe = keyframe->origin;
    nanoem_rsize_t index;
    all_keyframes = (nanoem_document_model_morph_keyframe_t **) nanoemMutableObjectArrayResize(origin_model->all_morph_keyframes_ptr,
        &model->num_allocated_all_morph_keyframes,
        &origin_model->num_all_morph_keyframes, status);
    if (nanoem_is_not_null(all_keyframes)) {
        index = origin_model->num_all_morph_keyframes - 1;
        all_keyframes[index] = origin_keyframe;
        origin_model->all_morph_keyframes_ptr = all_keyframes;
        keyframe->base.is_in_object = nanoem_true;
        nanoemDocumentBaseKeyframeSet(&origin_keyframe->base, frame_index, index, &previous, &next);
        nanoemDocumentKeyframeCompareResultSetKeyframeIndex(&previous, &next, index);
        nanoem_status_ptr_assign_succeeded(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelAddMorphKeyframeObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_morph_keyframe_t *keyframe, const nanoem_unicode_string_t *name, nanoem_frame_index_t frame_index, nanoem_status_t *status)
{
    nanoem_document_model_morph_keyframe_t *origin_keyframe, **keyframes, *initial_keyframe;
    nanoem_document_model_t *origin_model;
    nanoem_document_keyframe_compare_result_t previous, next;
    nanoem_rsize_t index;
    int morph_index;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(keyframe)) {
        origin_model = model->origin;
        origin_keyframe = keyframe->origin;
        morph_index = nanoemMutableDocumentModelResolveMorphId(model, name);
        nanoemDocumentKeyframeCompareResultReset(&previous);
        nanoemDocumentKeyframeCompareResultReset(&next);
        if (morph_index == -1) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_MORPH_NOT_FOUND);
            return;
        }
        else if (frame_index == 0) {
            initial_keyframe = origin_model->initial_morph_keyframes[morph_index];
            initial_keyframe->weight = origin_keyframe->weight;
            initial_keyframe->base.is_selected = origin_keyframe->base.is_selected;
            initial_keyframe->base.object_index = morph_index;
            return;
        }
        else if (nanoemDocumentModelFindMorphKeyframeIndex(origin_model, morph_index, frame_index, &previous, &next)) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_KEYFRAME_ALREADY_EXISTS);
            return;
        }
        keyframes = (nanoem_document_model_morph_keyframe_t **) nanoemMutableObjectArrayResize(origin_model->morph_keyframes,
            &model->num_allocated_morph_keyframes,
            &origin_model->num_morph_keyframes, status);
        if (nanoem_is_not_null(keyframes)) {
            index = origin_model->num_morph_keyframes - 1;
            keyframes[index] = origin_keyframe;
            origin_model->morph_keyframes = keyframes;
            nanoemMutableDocumentModelUpdateAllMorphKeyframeObjects(model, keyframe, frame_index, previous, next, status);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelAddOutsideParentSubjectBone(nanoem_mutable_document_model_t *model, const nanoem_unicode_string_t *name, nanoem_status_t *status)
{
    nanoem_document_model_t *origin_model;
    int *subject_bone_indices, bone_index, index;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(name)) {
        origin_model = model->origin;
        bone_index = nanoemMutableDocumentModelResolveBoneId(model, name);
        if (bone_index != -1) {
            subject_bone_indices = (int *) nanoemMutableObjectArrayResize(origin_model->outside_parent_subject_bone_indices,
                &model->num_allocated_outside_parent_subject_bones,
                &origin_model->num_outside_parent_subject_bones, status);
            if (nanoem_is_not_null(subject_bone_indices)) {
                index = (int) origin_model->num_outside_parent_subject_bones - 1;
                subject_bone_indices[index] = bone_index;
                origin_model->outside_parent_subject_bone_indices = subject_bone_indices;
                nanoem_status_ptr_assign_succeeded(status);
            }
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MODEL_BONE_NOT_FOUND);
        }
    }
}

void APIENTRY
nanoemMutableDocumentModelInsertModelOutsideParentStateObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_outside_parent_state_t *state, int index, nanoem_status_t *status)
{
    nanoem_document_model_t *origin_model;
    nanoem_document_model_outside_parent_state_t *origin_state;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(state)) {
        origin_model = model->origin;
        origin_state = state->origin;
        nanoemDocumentObjectArrayInsertObject((nanoem_document_object_t ***) &origin_model->outside_parent_states, (nanoem_document_object_t *) origin_state, index, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_STATE_ALREADY_EXISTS, &origin_model->num_outside_parent_states, &model->num_allocated_outside_parent_states, status);
        if (!nanoem_status_ptr_has_error(status)) {
            origin_state->base.parent.model = origin_model;
            state->base.is_in_object = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelRemoveBoneKeyframeObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_bone_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_document_model_t *origin_model;
    nanoem_document_model_bone_keyframe_t **keyframes, *origin_keyframe;
    nanoem_rsize_t num_keyframes, i;
    int index, offset;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(keyframe)) {
        origin_model = model->origin;
        origin_keyframe = keyframe->origin;
        keyframes = origin_model->bone_keyframes;
        index = origin_keyframe->base.object_index;
        num_keyframes = origin_model->num_bone_keyframes;
        offset = nanoemObjectArrayFindIndexedObjectOffset((const void *const *) keyframes, origin_keyframe, num_keyframes, index);
        if (offset >= 0) {
            num_keyframes--;
            nanoem_crt_memmove(&keyframes[offset], &keyframes[offset + 1], (num_keyframes - offset) * sizeof(origin_keyframe));
            for (i = offset; i < num_keyframes; i++) {
                keyframes[i]->base.object_index--;
            }
            origin_model->num_bone_keyframes = num_keyframes;
            keyframe->base.is_in_object = nanoem_false;
            nanoemDocumentBaseKeyframeReset(&origin_keyframe->base);
            nanoem_status_ptr_assign_succeeded(status);
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_KEYFRAME_NOT_FOUND);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelRemoveModelBoneStateObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_bone_state_t *state, nanoem_status_t *status)
{
    nanoem_document_model_t *origin_model;
    nanoem_document_model_bone_state_t *origin_state;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(state)) {
        origin_model = model->origin;
        origin_state = state->origin;
        nanoemDocumentObjectArrayRemoveObject((nanoem_document_object_t **) origin_model->bone_states, (nanoem_document_object_t *) origin_state, &origin_model->num_bone_states, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_STATE_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            origin_state->base.parent.model = NULL;
            state->base.is_in_object = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelRemoveModelConstraintStateObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_constraint_state_t *state, nanoem_status_t *status)
{
    nanoem_document_model_t *origin_model;
    nanoem_document_model_constraint_state_t *origin_state;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(state)) {
        origin_model = model->origin;
        origin_state = state->origin;
        nanoemDocumentObjectArrayRemoveObject((nanoem_document_object_t **) origin_model->constraint_states, (nanoem_document_object_t *) origin_state, &origin_model->num_constraint_states, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_CONSTRAINT_STATE_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            origin_state->base.parent.model = NULL;
            state->base.is_in_object = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelRemoveModelMorphStateObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_morph_state_t *state, nanoem_status_t *status)
{
    nanoem_document_model_t *origin_model;
    nanoem_document_model_morph_state_t *origin_state;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(state)) {
        origin_model = model->origin;
        origin_state = state->origin;
        nanoemDocumentObjectArrayRemoveObject((nanoem_document_object_t **) origin_model->morph_states, (nanoem_document_object_t *) origin_state, &origin_model->num_morph_states, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_STATE_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            origin_state->base.parent.model = NULL;
            state->base.is_in_object = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelRemoveModelKeyframeObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_document_model_t *origin_model;
    nanoem_document_model_keyframe_t **keyframes, *origin_keyframe;
    nanoem_rsize_t num_keyframes, i;
    int index, offset;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(keyframe)) {
        origin_model = model->origin;
        origin_keyframe = keyframe->origin;
        keyframes = origin_model->model_keyframes;
        index = origin_keyframe->base.object_index;
        num_keyframes = origin_model->num_model_keyframes;
        offset = nanoemObjectArrayFindIndexedObjectOffset((const void *const *) keyframes, origin_keyframe, num_keyframes, index);
        if (offset >= 0) {
            num_keyframes--;
            nanoem_crt_memmove(&keyframes[offset], &keyframes[offset + 1], (num_keyframes - offset) * sizeof(origin_keyframe));
            for (i = offset; i < num_keyframes; i++) {
                keyframes[i]->base.object_index--;
            }
            origin_model->num_model_keyframes = num_keyframes;
            keyframe->base.is_in_object = nanoem_false;
            nanoemDocumentBaseKeyframeReset(&origin_keyframe->base);
            nanoem_status_ptr_assign_succeeded(status);
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MODEL_KEYFRAME_NOT_FOUND);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelRemoveMorphKeyframeObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_morph_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_document_model_t *origin_model;
    nanoem_document_model_morph_keyframe_t **keyframes, *origin_keyframe;
    nanoem_rsize_t num_keyframes, i;
    int index, offset;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(keyframe)) {
        origin_model = model->origin;
        origin_keyframe = keyframe->origin;
        keyframes = origin_model->morph_keyframes;
        index = origin_keyframe->base.object_index;
        num_keyframes = origin_model->num_morph_keyframes;
        offset = nanoemObjectArrayFindIndexedObjectOffset((const void *const *) keyframes, origin_keyframe, num_keyframes, index);
        if (offset >= 0) {
            num_keyframes--;
            nanoem_crt_memmove(&keyframes[offset], &keyframes[offset + 1], (num_keyframes - offset) * sizeof(origin_keyframe));
            for (i = offset; i < num_keyframes; i++) {
                keyframes[i]->base.object_index--;
            }
            origin_model->num_morph_keyframes = num_keyframes;
            keyframe->base.is_in_object = nanoem_false;
            nanoemDocumentBaseKeyframeReset(&origin_keyframe->base);
            nanoem_status_ptr_assign_succeeded(status);
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_KEYFRAME_NOT_FOUND);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelRemoveModelOutsideParentStateObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_outside_parent_state_t *state, nanoem_status_t *status)
{
    nanoem_document_model_t *origin_model;
    nanoem_document_model_outside_parent_state_t *origin_state;
    if (nanoem_is_not_null(model) && nanoem_is_not_null(state)) {
        origin_model = model->origin;
        origin_state = state->origin;
        nanoemDocumentObjectArrayRemoveObject((nanoem_document_object_t **) origin_model->outside_parent_states, (nanoem_document_object_t *) origin_state, &origin_model->num_outside_parent_states, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_STATE_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            origin_state->base.parent.model = NULL;
            state->base.is_in_object = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelSetName(nanoem_mutable_document_model_t *model, nanoem_language_type_t language, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(model)) {
        factory = model->origin->base.parent.document->factory;
        switch (language) {
        case NANOEM_LANGUAGE_TYPE_ENGLISH:
            nanoemUnicodeStringFactoryAssignString(factory, &model->origin->name_en, value, status);
            break;
        case NANOEM_LANGUAGE_TYPE_JAPANESE:
            nanoemUnicodeStringFactoryAssignString(factory, &model->origin->name_ja, value, status);
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
nanoemMutableDocumentModelSetPath(nanoem_mutable_document_model_t *model, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(model)) {
        factory = model->origin->base.parent.document->factory;
        nanoemUnicodeStringFactoryAssignString(factory, &model->origin->path, value, status);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelSetSelectedBoneName(nanoem_mutable_document_model_t *model, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_mutable_document_t *document;
    int selected_bone_index = -1;
    if (nanoem_is_not_null(model)) {
        if (nanoem_is_not_null(value)) {
            document = model->base.parent.document;
            selected_bone_index = nanoemMutableDocumentFindParentModelBoneIndex(document, model->origin->base.index, value);
        }
        model->origin->selected_bone_index = selected_bone_index;
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelSetSelectedMorphName(nanoem_mutable_document_model_t *model, nanoem_model_morph_category_t category, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_mutable_document_t *document;
    int selected_morph_index = -1;
    if (nanoem_is_not_null(model) && category > NANOEM_MODEL_MORPH_CATEGORY_BASE && category < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM) {
        if (nanoem_is_not_null(value)) {
            document = model->base.parent.document;
            selected_morph_index = nanoemMutableDocumentFindParentModelMorphIndex(document, model->origin->base.index, value);
        }
        model->origin->selected_morph_indices[category - 1] = selected_morph_index;
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentModelSetEdgeWidth(nanoem_mutable_document_model_t *model, nanoem_f32_t value)
{
    if (nanoem_is_not_null(model)) {
        model->origin->edge_width = value;
    }
}

void APIENTRY
nanoemMutableDocumentModelSetLastFrameIndex(nanoem_mutable_document_model_t *model, nanoem_frame_index_t value)
{
    if (nanoem_is_not_null(model)) {
        model->origin->last_frame_index = value;
    }
}

void APIENTRY
nanoemMutableDocumentModelSetDrawOrderIndex(nanoem_mutable_document_model_t *model, int value)
{
    if (nanoem_is_not_null(model)) {
        model->origin->draw_order_index = value;
    }
}

void APIENTRY
nanoemMutableDocumentModelSetTransformOrderIndex(nanoem_mutable_document_model_t *model, int value)
{
    if (nanoem_is_not_null(model)) {
        model->origin->transform_order_index = value;
    }
}

void APIENTRY
nanoemMutableDocumentModelSetBlendEnabled(nanoem_mutable_document_model_t *model, nanoem_bool_t value)
{
    if (nanoem_is_not_null(model)) {
        model->origin->is_blend_enabled = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentModelSetSelfShadowEnabled(nanoem_mutable_document_model_t *model, nanoem_bool_t value)
{
    if (nanoem_is_not_null(model)) {
        model->origin->is_self_shadow_enabled = value ? nanoem_true : nanoem_false;
    }
}

nanoem_document_model_t * APIENTRY
nanoemMutableDocumentModelGetOrigin(nanoem_mutable_document_model_t *model)
{
    return nanoem_is_not_null(model) ? model->origin : NULL;
}

void APIENTRY
nanoemMutableDocumentModelDestroy(nanoem_mutable_document_model_t *model)
{
    if (nanoem_is_not_null(model)) {
        if (nanoemMutableDocumentObjectCanDelete(&model->base)) {
            nanoemDocumentModelDestroy(model->origin);
        }
        nanoem_free(model);
    }
}

nanoem_mutable_document_accessory_t * APIENTRY
nanoemMutableDocumentAccessoryCreate(nanoem_mutable_document_t *document, nanoem_status_t *status)
{
    nanoem_mutable_document_accessory_t *accessory;
    nanoem_document_accessory_keyframe_t *initial_keyframe;
    nanoem_document_accessory_t *origin;
    accessory = (nanoem_mutable_document_accessory_t *) nanoem_calloc(sizeof(*accessory), 1, status);
    if (nanoem_is_not_null(accessory)) {
        accessory->base.parent.document = document;
        origin = accessory->origin = nanoemDocumentAccessoryCreate(nanoemMutableDocumentGetOrigin(document), status);
        if (nanoem_is_not_null(origin)) {
            origin->all_accessory_keyframes_ptr = (nanoem_document_accessory_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->all_accessory_keyframes_ptr), status);
            origin->accessory_keyframes = (nanoem_document_accessory_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->accessory_keyframes), status);
            origin->parent_model_index = -1;
            initial_keyframe = nanoemDocumentAccessoryKeyframeCreate(origin, status);
            if (nanoem_is_not_null(initial_keyframe)) {
                origin->all_accessory_keyframes_ptr[0] = origin->initial_accessory_keyframe = initial_keyframe;
                accessory->num_allocated_accessory_keyframes = __nanoem_default_allocation_size;
                nanoemDocumentAccessoryKeyframeInitialize(initial_keyframe);
            }
        }
    }
    return accessory;
}

nanoem_mutable_document_accessory_t * APIENTRY
nanoemMutableDocumentAccessoryCreateAsReference(nanoem_mutable_document_t *document, nanoem_document_accessory_t *reference, nanoem_status_t *status)
{
    nanoem_mutable_document_accessory_t *accessory = NULL;
    nanoem_document_t *origin_document;
    if (nanoem_is_not_null(document) && nanoem_is_not_null(reference)) {
        origin_document = document->origin;
        if (nanoemObjectArrayContainsIndexedObject((const void* const *) origin_document->accessories, reference, origin_document->num_accessories, reference->base.index)) {
            accessory = (nanoem_mutable_document_accessory_t *) nanoem_calloc(sizeof(*accessory), 1, status);
            if (nanoem_is_not_null(accessory)) {
                accessory->base.parent.document = document;
                accessory->origin = reference;
                accessory->base.is_reference = nanoem_true;
            }
        }
    }
    return accessory;
}

static nanoem_bool_t
nanoemDocumentAccessoryFindKeyframeIndex(nanoem_document_accessory_t *accessory, nanoem_frame_index_t frame_index, nanoem_document_keyframe_compare_result_t *previous, nanoem_document_keyframe_compare_result_t *next)
{
    nanoem_document_accessory_keyframe_t *keyframe;
    nanoem_document_accessory_keyframe_t *const *keyframes = accessory->all_accessory_keyframes_ptr;
    nanoem_rsize_t num_keyframes = accessory->num_accessory_keyframes + 1, i;
    nanoem_bool_t found = nanoem_false;
    for (i = 0; i < num_keyframes; i++) {
        keyframe = keyframes[i];
        found |= nanoemDocumentBaseKeyframeCompare(&keyframe->base, frame_index, previous, next);
    }
    return found;
}

void APIENTRY
nanoemMutableDocumentAccessoryAddAccessoryKeyframeObject(nanoem_mutable_document_accessory_t *accessory, nanoem_mutable_document_accessory_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status)
{
    nanoem_document_accessory_t *origin_accessory;
    nanoem_document_accessory_keyframe_t **keyframes, *origin_keyframe, *initial_keyframe;
    nanoem_document_keyframe_compare_result_t previous, next;
    nanoem_rsize_t index, num_allocated_all_accessory_keyframes, num_all_accessory_keyframes;
    if (nanoem_is_not_null(accessory) && nanoem_is_not_null(keyframe)) {
        origin_accessory = accessory->origin;
        origin_keyframe = keyframe->origin;
        nanoemDocumentKeyframeCompareResultReset(&previous);
        nanoemDocumentKeyframeCompareResultReset(&next);
        if (frame_index == 0) {
            initial_keyframe = origin_accessory->initial_accessory_keyframe;
            initial_keyframe->base.is_selected = origin_keyframe->base.is_selected;
            nanoem_crt_memcpy(initial_keyframe->orientation.values, origin_keyframe->orientation.values, sizeof(initial_keyframe->orientation.values));
            nanoem_crt_memcpy(initial_keyframe->translation.values, origin_keyframe->translation.values, sizeof(initial_keyframe->translation.values));
            initial_keyframe->is_shadow_enabled = origin_keyframe->is_shadow_enabled;
            initial_keyframe->opacity = origin_keyframe->opacity;
            initial_keyframe->parent_model_bone_index = origin_keyframe->parent_model_bone_index;
            initial_keyframe->parent_model_index = origin_keyframe->parent_model_index;
            initial_keyframe->scale_factor = origin_keyframe->scale_factor;
            initial_keyframe->visible = origin_keyframe->visible;
            nanoem_status_ptr_assign_succeeded(status);
            return;
        }
        else if (nanoemDocumentAccessoryFindKeyframeIndex(origin_accessory, frame_index, &previous, &next)) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_KEYFRAME_ALREADY_EXISTS);
            return;
        }
        num_all_accessory_keyframes = origin_accessory->num_accessory_keyframes + 1;
        num_allocated_all_accessory_keyframes = accessory->num_allocated_accessory_keyframes + 1;
        keyframes = (nanoem_document_accessory_keyframe_t **) nanoemMutableObjectArrayResize(origin_accessory->accessory_keyframes,
            &accessory->num_allocated_accessory_keyframes,
            &origin_accessory->num_accessory_keyframes, status);
        if (nanoem_is_not_null(keyframes)) {
            index = origin_accessory->num_accessory_keyframes - 1;
            keyframes[index] = origin_keyframe;
            origin_accessory->accessory_keyframes = keyframes;
            keyframes = (nanoem_document_accessory_keyframe_t **) nanoemMutableObjectArrayResize(origin_accessory->all_accessory_keyframes_ptr,
                &num_allocated_all_accessory_keyframes,
                &num_all_accessory_keyframes, status);
            if (nanoem_is_not_null(keyframes)) {
                index = num_all_accessory_keyframes - 1;
                keyframes[index] = origin_keyframe;
                origin_accessory->all_accessory_keyframes_ptr = keyframes;
                keyframe->base.is_in_object = nanoem_true;
                nanoemDocumentBaseKeyframeSet(&origin_keyframe->base, frame_index, index, &previous, &next);
                nanoemDocumentKeyframeCompareResultSetKeyframeIndex(&previous, &next, index);
                nanoem_status_ptr_assign_succeeded(status);
            }
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentAccessoryRemoveAccessoryKeyframeObject(nanoem_mutable_document_accessory_t *accessory, nanoem_mutable_document_accessory_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_document_accessory_t *origin_accessory;
    nanoem_document_accessory_keyframe_t **keyframes, *origin_keyframe;
    nanoem_rsize_t num_keyframes, i;
    int index, offset;
    if (nanoem_is_not_null(accessory) && nanoem_is_not_null(keyframe)) {
        origin_accessory = accessory->origin;
        origin_keyframe = keyframe->origin;
        keyframes = origin_accessory->accessory_keyframes;
        index = origin_keyframe->base.object_index;
        num_keyframes = origin_accessory->num_accessory_keyframes;
        offset = nanoemObjectArrayFindIndexedObjectOffset((const void *const *) keyframes, origin_keyframe, num_keyframes, index);
        if (offset >= 0) {
            num_keyframes--;
            nanoem_crt_memmove(&keyframes[offset], &keyframes[offset + 1], (num_keyframes - offset) * sizeof(origin_keyframe));
            for (i = offset; i < num_keyframes; i++) {
                keyframes[i]->base.object_index--;
            }
            origin_accessory->num_accessory_keyframes = num_keyframes;
            keyframe->base.is_in_object = nanoem_false;
            nanoemDocumentBaseKeyframeReset(&origin_keyframe->base);
            nanoem_status_ptr_assign_succeeded(status);
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_KEYFRAME_NOT_FOUND);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentAccessorySetName(nanoem_mutable_document_accessory_t *accessory, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(accessory)) {
        factory = accessory->origin->base.parent.document->factory;
        nanoemUnicodeStringFactoryAssignString(factory, &accessory->origin->name, value, status);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentAccessorySetPath(nanoem_mutable_document_accessory_t *accessory, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(accessory)) {
        factory = accessory->origin->base.parent.document->factory;
        nanoemUnicodeStringFactoryAssignString(factory, &accessory->origin->path, value, status);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentAccessorySetParentModelObject(nanoem_mutable_document_accessory_t *accessory, const nanoem_document_model_t *value)
{
    if (nanoem_is_not_null(accessory) && nanoem_is_not_null(value)) {
        accessory->origin->parent_model_index = nanoemDocumentModelGetIndex(value);
    }
}

void APIENTRY
nanoemMutableDocumentAccessorySetParentModelBoneName(nanoem_mutable_document_accessory_t *accessory, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_mutable_document_t *document;
    int parent_model_index, parent_model_bone_index = 0;
    if (nanoem_is_not_null(accessory)) {
        if (nanoem_is_not_null(value)) {
            document = accessory->base.parent.document;
            parent_model_index = accessory->origin->parent_model_index;
            parent_model_bone_index = nanoemMutableDocumentFindParentModelBoneIndex(document, parent_model_index, value);
        }
        accessory->origin->parent_model_bone_index = parent_model_bone_index;
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentAccessorySetTranslation(nanoem_mutable_document_accessory_t *accessory, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(accessory) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(accessory->origin->translation.values, value, sizeof(accessory->origin->translation.values));
    }
}

void APIENTRY
nanoemMutableDocumentAccessorySetOrientation(nanoem_mutable_document_accessory_t *accessory, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(accessory) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(accessory->origin->orientation.values, value, sizeof(accessory->origin->orientation.values));
    }
}

void APIENTRY
nanoemMutableDocumentAccessorySetScaleFactor(nanoem_mutable_document_accessory_t *accessory, nanoem_f32_t value)
{
    if (nanoem_is_not_null(accessory)) {
        accessory->origin->scale_factor = value;
    }
}

void APIENTRY
nanoemMutableDocumentAccessorySetOpacity(nanoem_mutable_document_accessory_t *accessory, nanoem_f32_t value)
{
    if (nanoem_is_not_null(accessory)) {
        accessory->origin->opacity = value;
    }
}

void APIENTRY
nanoemMutableDocumentAccessorySetDrawOrderIndex(nanoem_mutable_document_accessory_t *accessory, int value)
{
    if (nanoem_is_not_null(accessory)) {
        accessory->origin->draw_order_index = value;
    }
}

void APIENTRY
nanoemMutableDocumentAccessorySetVisible(nanoem_mutable_document_accessory_t *accessory, nanoem_bool_t value)
{
    if (nanoem_is_not_null(accessory)) {
        accessory->origin->visible = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentAccessorySetShadowEnabled(nanoem_mutable_document_accessory_t *accessory, nanoem_bool_t value)
{
    if (nanoem_is_not_null(accessory)) {
        accessory->origin->is_shadow_enabled = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentAccessorySetAddBlendEnabled(nanoem_mutable_document_accessory_t *accessory, nanoem_bool_t value)
{
    if (nanoem_is_not_null(accessory)) {
        accessory->origin->is_add_blend_enabled = value ? nanoem_true : nanoem_false;
    }
}

nanoem_document_accessory_t * APIENTRY
nanoemMutableDocumentAccessoryGetOrigin(nanoem_mutable_document_accessory_t *accessory)
{
    return nanoem_is_not_null(accessory) ? accessory->origin : NULL;
}

void APIENTRY
nanoemMutableDocumentAccessoryDestroy(nanoem_mutable_document_accessory_t *accessory)
{
    if (nanoem_is_not_null(accessory)) {
        if (nanoemMutableDocumentObjectCanDelete(&accessory->base)) {
            nanoemDocumentAccessoryDestroy(accessory->origin);
        }
        nanoem_free(accessory);
    }
}

nanoem_mutable_document_camera_t * APIENTRY
nanoemMutableDocumentCameraCreate(nanoem_mutable_document_t *document, nanoem_status_t *status)
{
    nanoem_mutable_document_camera_t *camera;
    nanoem_document_camera_keyframe_t *initial_keyframe;
    nanoem_document_camera_t *origin;
    camera = (nanoem_mutable_document_camera_t *) nanoem_calloc(sizeof(*camera), 1, status);
    if (nanoem_is_not_null(camera)) {
        camera->base.parent.document = document;
        origin = camera->origin = nanoemDocumentCameraCreate(nanoemMutableDocumentGetOrigin(document), status);
        if (nanoem_is_not_null(origin)) {
            nanoemDocumentCameraInitialize(origin);
            origin->all_camera_keyframes_ptr = (nanoem_document_camera_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->all_camera_keyframes_ptr), status);
            origin->camera_keyframes = (nanoem_document_camera_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->camera_keyframes), status);
            initial_keyframe = nanoemDocumentCameraKeyframeCreate(camera->origin, status);
            if (nanoem_is_not_null(initial_keyframe)) {
                origin->all_camera_keyframes_ptr[0] = origin->initial_camera_keyframe = initial_keyframe;
                camera->num_allocated_camera_keyframes = __nanoem_default_allocation_size;
                nanoemDocumentCameraKeyframeInitialize(initial_keyframe);
            }
        }
    }
    return camera;
}

static nanoem_bool_t
nanoemDocumentCameraFindKeyframeIndex(nanoem_document_camera_t *camera, nanoem_frame_index_t frame_index, nanoem_document_keyframe_compare_result_t *previous, nanoem_document_keyframe_compare_result_t *next)
{
    nanoem_document_camera_keyframe_t *keyframe;
    nanoem_document_camera_keyframe_t *const *keyframes = camera->all_camera_keyframes_ptr;
    nanoem_rsize_t num_keyframes = camera->num_camera_keyframes + 1, i;
    nanoem_bool_t found = nanoem_false;
    for (i = 0; i < num_keyframes; i++) {
        keyframe = keyframes[i];
        found |= nanoemDocumentBaseKeyframeCompare(&keyframe->base, frame_index, previous, next);
    }
    return found;
}

void APIENTRY
nanoemMutableDocumentCameraAddCameraKeyframeObject(nanoem_mutable_document_camera_t *camera, nanoem_mutable_document_camera_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status)
{
    nanoem_document_camera_t *origin_camera;
    nanoem_document_camera_keyframe_t **keyframes, *origin_keyframe, *initial_keyframe;
    nanoem_document_keyframe_compare_result_t previous, next;
    nanoem_rsize_t index, num_all_camera_keyframes, num_allocated_all_camera_keyframes;
    if (nanoem_is_not_null(camera) && nanoem_is_not_null(keyframe)) {
        origin_camera = camera->origin;
        origin_keyframe = keyframe->origin;
        nanoemDocumentKeyframeCompareResultReset(&previous);
        nanoemDocumentKeyframeCompareResultReset(&next);
        if (frame_index == 0) {
            initial_keyframe = origin_camera->initial_camera_keyframe;
            initial_keyframe->base.is_selected = origin_keyframe->base.is_selected;
            nanoem_crt_memcpy(initial_keyframe->angle.values, origin_keyframe->angle.values, sizeof(initial_keyframe->angle.values));
            nanoem_crt_memcpy(initial_keyframe->look_at.values, origin_keyframe->look_at.values, sizeof(initial_keyframe->look_at.values));
            nanoem_crt_memcpy(initial_keyframe->interpolation, origin_keyframe->interpolation, sizeof(initial_keyframe->interpolation));
            initial_keyframe->distance = origin_keyframe->distance;
            initial_keyframe->fov = origin_keyframe->fov;
            initial_keyframe->is_perspective_view = origin_keyframe->is_perspective_view;
            initial_keyframe->parent_model_bone_index = origin_keyframe->parent_model_bone_index;
            initial_keyframe->parent_model_index = origin_keyframe->parent_model_index;
            nanoem_status_ptr_assign_succeeded(status);
            return;
        }
        else if (nanoemDocumentCameraFindKeyframeIndex(origin_camera, frame_index, &previous, &next)) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_CAMERA_KEYFRAME_ALREADY_EXISTS);
            return;
        }
        num_all_camera_keyframes = origin_camera->num_camera_keyframes + 1;
        num_allocated_all_camera_keyframes = camera->num_allocated_camera_keyframes + 1;
        keyframes = (nanoem_document_camera_keyframe_t **) nanoemMutableObjectArrayResize(origin_camera->camera_keyframes,
            &camera->num_allocated_camera_keyframes,
            &origin_camera->num_camera_keyframes, status);
        if (nanoem_is_not_null(keyframes)) {
            index = origin_camera->num_camera_keyframes - 1;
            keyframes[index] = origin_keyframe;
            origin_camera->camera_keyframes = keyframes;
            keyframes = (nanoem_document_camera_keyframe_t **) nanoemMutableObjectArrayResize(origin_camera->all_camera_keyframes_ptr,
                &num_allocated_all_camera_keyframes,
                &num_all_camera_keyframes, status);
            if (nanoem_is_not_null(keyframes)) {
                index = num_all_camera_keyframes - 1;
                keyframes[index] = origin_keyframe;
                origin_camera->all_camera_keyframes_ptr = keyframes;
                keyframe->base.is_in_object = nanoem_true;
                nanoemDocumentBaseKeyframeSet(&origin_keyframe->base, frame_index, index, &previous, &next);
                nanoemDocumentKeyframeCompareResultSetKeyframeIndex(&previous, &next, index);
                nanoem_status_ptr_assign_succeeded(status);
            }
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentCameraRemoveCameraKeyframeObject(nanoem_mutable_document_camera_t *camera, nanoem_mutable_document_camera_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_document_camera_t *origin_camera;
    nanoem_document_camera_keyframe_t **keyframes, *origin_keyframe;
    nanoem_rsize_t num_keyframes, i;
    int index, offset;
    if (nanoem_is_not_null(camera) && nanoem_is_not_null(keyframe)) {
        origin_camera = camera->origin;
        origin_keyframe = keyframe->origin;
        keyframes = origin_camera->camera_keyframes;
        index = origin_keyframe->base.object_index;
        num_keyframes = origin_camera->num_camera_keyframes;
        offset = nanoemObjectArrayFindIndexedObjectOffset((const void *const *) keyframes, origin_keyframe, num_keyframes, index);
        if (offset >= 0) {
            num_keyframes--;
            nanoem_crt_memmove(&keyframes[offset], &keyframes[offset + 1], (num_keyframes - offset) * sizeof(origin_keyframe));
            for (i = offset; i < num_keyframes; i++) {
                keyframes[i]->base.object_index--;
            }
            origin_camera->num_camera_keyframes = num_keyframes;
            keyframe->base.is_in_object = nanoem_false;
            nanoemDocumentBaseKeyframeReset(&origin_keyframe->base);
            nanoem_status_ptr_assign_succeeded(status);
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_CAMERA_KEYFRAME_NOT_FOUND);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentCameraSetLookAt(nanoem_mutable_document_camera_t *camera, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(camera) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(camera->origin->look_at.values, value, sizeof(camera->origin->look_at.values));
    }
}

void APIENTRY
nanoemMutableDocumentCameraSetAngle(nanoem_mutable_document_camera_t *camera, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(camera) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(camera->origin->angle.values, value, sizeof(camera->origin->angle.values));
    }
}

void APIENTRY
nanoemMutableDocumentCameraSetPosition(nanoem_mutable_document_camera_t *camera, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(camera) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(camera->origin->position.values, value, sizeof(camera->origin->position.values));
    }
}

void APIENTRY
nanoemMutableDocumentCameraSetDistance(nanoem_mutable_document_camera_t *camera, nanoem_f32_t value)
{
    if (nanoem_is_not_null(camera)) {
        camera->origin->position.values[2] = camera->origin->look_at.values[2] + value;
    }
}

void APIENTRY
nanoemMutableDocumentCameraSetFov(nanoem_mutable_document_camera_t *camera, nanoem_f32_t value)
{
    if (nanoem_is_not_null(camera)) {
        camera->base.parent.document->origin->camera_fov = value;
    }
}

void APIENTRY
nanoemMutableDocumentCameraSetPerspectiveEnabled(nanoem_mutable_document_camera_t *camera, nanoem_bool_t value)
{
    if (nanoem_is_not_null(camera)) {
        camera->origin->is_perspective = value ? nanoem_true : nanoem_false;
    }
}

nanoem_document_camera_t * APIENTRY
nanoemMutableDocumentCameraGetOrigin(nanoem_mutable_document_camera_t *camera)
{
    return nanoem_is_not_null(camera) ? camera->origin : NULL;
}

void APIENTRY
nanoemMutableDocumentCameraDestroy(nanoem_mutable_document_camera_t *camera)
{
    if (nanoem_is_not_null(camera)) {
        if (nanoemMutableDocumentObjectCanDelete(&camera->base)) {
            nanoemDocumentCameraDestroy(camera->origin);
        }
        nanoem_free(camera);
    }
}

nanoem_mutable_document_gravity_t * APIENTRY
nanoemMutableDocumentGravityCreate(nanoem_mutable_document_t *document, nanoem_status_t *status)
{
    nanoem_mutable_document_gravity_t *gravity;
    nanoem_document_gravity_keyframe_t *initial_keyframe;
    nanoem_document_gravity_t *origin;
    gravity = (nanoem_mutable_document_gravity_t *) nanoem_calloc(sizeof(*gravity), 1, status);
    if (nanoem_is_not_null(gravity)) {
        gravity->base.parent.document = document;
        origin = gravity->origin = nanoemDocumentGravityCreate(nanoemMutableDocumentGetOrigin(document), status);
        if (nanoem_is_not_null(origin)) {
            nanoemDocumentGravityInitialize(origin);
            origin->all_gravity_keyframes_ptr = (nanoem_document_gravity_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->all_gravity_keyframes_ptr), status);
            origin->gravity_keyframes = (nanoem_document_gravity_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->gravity_keyframes), status);
            initial_keyframe = nanoemDocumentGravityKeyframeCreate(gravity->origin, status);
            if (nanoem_is_not_null(initial_keyframe)) {
                origin->all_gravity_keyframes_ptr[0] = origin->initial_gravity_keyframe = initial_keyframe;
                gravity->num_allocated_gravity_keyframes = __nanoem_default_allocation_size;
                nanoemDocumentGravityKeyframeInitialize(initial_keyframe);
            }
        }
    }
    return gravity;
}

static nanoem_bool_t
nanoemDocumentGravityFindKeyframeIndex(nanoem_document_gravity_t *gravity, nanoem_frame_index_t frame_index, nanoem_document_keyframe_compare_result_t *previous, nanoem_document_keyframe_compare_result_t *next)
{
    nanoem_document_gravity_keyframe_t *keyframe;
    nanoem_document_gravity_keyframe_t *const *keyframes = gravity->all_gravity_keyframes_ptr;
    nanoem_rsize_t num_keyframes = gravity->num_gravity_keyframes + 1, i;
    nanoem_bool_t found = nanoem_false;
    for (i = 0; i < num_keyframes; i++) {
        keyframe = keyframes[i];
        found |= nanoemDocumentBaseKeyframeCompare(&keyframe->base, frame_index, previous, next);
    }
    return found;
}

void APIENTRY
nanoemMutableDocumentGravityAddGravityKeyframeObject(nanoem_mutable_document_gravity_t *gravity, nanoem_mutable_document_gravity_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status)
{
    nanoem_document_gravity_t *origin_gravity;
    nanoem_document_gravity_keyframe_t **keyframes, *origin_keyframe, *initial_keyframe;
    nanoem_document_keyframe_compare_result_t previous, next;
    nanoem_rsize_t index, num_allocated_all_gravity_keyframes, num_all_gravity_keyframes;
    if (nanoem_is_not_null(gravity) && nanoem_is_not_null(keyframe)) {
        origin_gravity = gravity->origin;
        origin_keyframe = keyframe->origin;
        nanoemDocumentKeyframeCompareResultReset(&previous);
        nanoemDocumentKeyframeCompareResultReset(&next);
        if (frame_index == 0) {
            initial_keyframe = origin_gravity->initial_gravity_keyframe;
            initial_keyframe->base.is_selected = origin_keyframe->base.is_selected;
            nanoem_crt_memcpy(initial_keyframe->direction.values, origin_keyframe->direction.values, sizeof(initial_keyframe->direction.values));
            initial_keyframe->acceleration = origin_keyframe->acceleration;
            initial_keyframe->is_noise_enabled = origin_keyframe->is_noise_enabled;
            initial_keyframe->noise = origin_keyframe->noise;
            nanoem_status_ptr_assign_succeeded(status);
            return;
        }
        else if (nanoemDocumentGravityFindKeyframeIndex(origin_gravity, frame_index, &previous, &next)) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_GRAVITY_KEYFRAME_ALREADY_EXISTS);
            return;
        }
        num_all_gravity_keyframes = origin_gravity->num_gravity_keyframes + 1;
        num_allocated_all_gravity_keyframes = gravity->num_allocated_gravity_keyframes + 1;
        keyframes = (nanoem_document_gravity_keyframe_t **) nanoemMutableObjectArrayResize(origin_gravity->gravity_keyframes,
            &gravity->num_allocated_gravity_keyframes,
            &origin_gravity->num_gravity_keyframes, status);
        if (nanoem_is_not_null(keyframes)) {
            index = origin_gravity->num_gravity_keyframes - 1;
            keyframes[index] = origin_keyframe;
            origin_gravity->gravity_keyframes = keyframes;
            keyframes = (nanoem_document_gravity_keyframe_t **) nanoemMutableObjectArrayResize(origin_gravity->all_gravity_keyframes_ptr,
                &num_allocated_all_gravity_keyframes,
                &num_all_gravity_keyframes, status);
            if (nanoem_is_not_null(keyframes)) {
                index = num_all_gravity_keyframes - 1;
                keyframes[index] = origin_keyframe;
                origin_gravity->all_gravity_keyframes_ptr = keyframes;
                keyframe->base.is_in_object = nanoem_true;
                nanoemDocumentBaseKeyframeSet(&origin_keyframe->base, frame_index, index, &previous, &next);
                nanoemDocumentKeyframeCompareResultSetKeyframeIndex(&previous, &next, index);
                nanoem_status_ptr_assign_succeeded(status);
            }
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentGravityRemoveGravityKeyframeObject(nanoem_mutable_document_gravity_t *gravity, nanoem_mutable_document_gravity_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_document_gravity_t *origin_gravity;
    nanoem_document_gravity_keyframe_t **keyframes, *origin_keyframe;
    nanoem_rsize_t num_keyframes, i;
    int index, offset;
    if (nanoem_is_not_null(gravity) && nanoem_is_not_null(keyframe)) {
        origin_gravity = gravity->origin;
        origin_keyframe = keyframe->origin;
        keyframes = origin_gravity->gravity_keyframes;
        index = origin_keyframe->base.object_index;
        num_keyframes = origin_gravity->num_gravity_keyframes;
        offset = nanoemObjectArrayFindIndexedObjectOffset((const void *const *) keyframes, origin_keyframe, num_keyframes, index);
        if (offset >= 0) {
            num_keyframes--;
            nanoem_crt_memmove(&keyframes[offset], &keyframes[offset + 1], (num_keyframes - offset) * sizeof(origin_keyframe));
            for (i = offset; i < num_keyframes; i++) {
                keyframes[i]->base.object_index--;
            }
            origin_gravity->num_gravity_keyframes = num_keyframes;
            keyframe->base.is_in_object = nanoem_false;
            nanoemDocumentBaseKeyframeReset(&origin_keyframe->base);
            nanoem_status_ptr_assign_succeeded(status);
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_GRAVITY_KEYFRAME_NOT_FOUND);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentGravitySetDirection(nanoem_mutable_document_gravity_t *gravity, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(gravity) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(gravity->origin->direction.values, value, sizeof(gravity->origin->direction.values));
    }
}

void APIENTRY
nanoemMutableDocumentGravitySetAcceleration(nanoem_mutable_document_gravity_t *gravity, nanoem_f32_t value)
{
    if (nanoem_is_not_null(gravity)) {
        gravity->origin->acceleration = value;
    }
}

void APIENTRY
nanoemMutableDocumentGravitySetNoise(nanoem_mutable_document_gravity_t *gravity, int value)
{
    if (nanoem_is_not_null(gravity)) {
        gravity->origin->noise = value;
    }
}

void APIENTRY
nanoemMutableDocumentGravitySetNoiseEnabled(nanoem_mutable_document_gravity_t *gravity, nanoem_bool_t value)
{
    if (nanoem_is_not_null(gravity)) {
        gravity->origin->is_noise_enabled = value ? nanoem_true : nanoem_false;
    }
}

nanoem_document_gravity_t * APIENTRY
nanoemMutableDocumentGravityGetOrigin(nanoem_mutable_document_gravity_t *gravity)
{
    return nanoem_is_not_null(gravity) ? gravity->origin : NULL;
}

void APIENTRY
nanoemMutableDocumentGravityDestroy(nanoem_mutable_document_gravity_t *gravity)
{
    if (nanoem_is_not_null(gravity)) {
        if (nanoemMutableDocumentObjectCanDelete(&gravity->base)) {
            nanoemDocumentGravityDestroy(gravity->origin);
        }
        nanoem_free(gravity);
    }
}

nanoem_mutable_document_light_t * APIENTRY
nanoemMutableDocumentLightCreate(nanoem_mutable_document_t *document, nanoem_status_t *status)
{
    nanoem_mutable_document_light_t *light;
    nanoem_document_light_keyframe_t *initial_keyframe;
    nanoem_document_light_t *origin;
    light = (nanoem_mutable_document_light_t *) nanoem_calloc(sizeof(*light), 1, status);
    if (nanoem_is_not_null(light)) {
        light->base.parent.document = document;
        origin = light->origin = nanoemDocumentLightCreate(nanoemMutableDocumentGetOrigin(document), status);
        if (nanoem_is_not_null(origin)) {
            nanoemDocumentLightInitialize(origin);
            origin->all_light_keyframes_ptr = (nanoem_document_light_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->all_light_keyframes_ptr), status);
            origin->light_keyframes = (nanoem_document_light_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->light_keyframes), status);
            initial_keyframe = nanoemDocumentLightKeyframeCreate(light->origin, status);
            if (nanoem_is_not_null(initial_keyframe)) {
                origin->all_light_keyframes_ptr[0] = origin->initial_light_keyframe = initial_keyframe;
                light->num_allocated_light_keyframes = __nanoem_default_allocation_size;
                nanoemDocumentLightKeyframeInitialize(initial_keyframe);
            }
        }
    }
    return light;
}

static nanoem_bool_t
nanoemDocumentLightFindKeyframeIndex(nanoem_document_light_t *light, nanoem_frame_index_t frame_index, nanoem_document_keyframe_compare_result_t *previous, nanoem_document_keyframe_compare_result_t *next)
{
    nanoem_document_light_keyframe_t *keyframe;
    nanoem_document_light_keyframe_t *const *keyframes = light->all_light_keyframes_ptr;
    nanoem_rsize_t num_keyframes = light->num_light_keyframes + 1, i;
    nanoem_bool_t found = nanoem_false;
    for (i = 0; i < num_keyframes; i++) {
        keyframe = keyframes[i];
        found |= nanoemDocumentBaseKeyframeCompare(&keyframe->base, frame_index, previous, next);
    }
    return found;
}

void APIENTRY
nanoemMutableDocumentLightAddLightKeyframeObject(nanoem_mutable_document_light_t *light, nanoem_mutable_document_light_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status)
{
    nanoem_document_light_t *origin_light;
    nanoem_document_light_keyframe_t **keyframes, *origin_keyframe, *initial_keyframe;
    nanoem_document_keyframe_compare_result_t previous, next;
    nanoem_rsize_t index, num_allocated_all_light_keyframes, num_all_light_keyframes;
    if (nanoem_is_not_null(light) && nanoem_is_not_null(keyframe)) {
        origin_light = light->origin;
        origin_keyframe = keyframe->origin;
        nanoemDocumentKeyframeCompareResultReset(&previous);
        nanoemDocumentKeyframeCompareResultReset(&next);
        if (frame_index == 0) {
            initial_keyframe = origin_light->initial_light_keyframe;
            initial_keyframe->base.is_selected = origin_keyframe->base.is_selected;
            nanoem_crt_memcpy(initial_keyframe->color.values, origin_keyframe->color.values, sizeof(initial_keyframe->color.values));
            nanoem_crt_memcpy(initial_keyframe->direction.values, origin_keyframe->direction.values, sizeof(initial_keyframe->direction.values));
            nanoem_status_ptr_assign_succeeded(status);
            return;
        }
        else if (nanoemDocumentLightFindKeyframeIndex(origin_light, frame_index, &previous, &next)) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_LIGHT_KEYFRAME_ALREADY_EXISTS);
            return;
        }
        num_all_light_keyframes = origin_light->num_light_keyframes + 1;
        num_allocated_all_light_keyframes = light->num_allocated_light_keyframes + 1;
        keyframes = (nanoem_document_light_keyframe_t **) nanoemMutableObjectArrayResize(origin_light->light_keyframes,
            &light->num_allocated_light_keyframes,
            &origin_light->num_light_keyframes, status);
        if (nanoem_is_not_null(keyframes)) {
            index = origin_light->num_light_keyframes - 1;
            keyframes[index] = origin_keyframe;
            origin_light->light_keyframes = keyframes;
            keyframes = (nanoem_document_light_keyframe_t **) nanoemMutableObjectArrayResize(origin_light->all_light_keyframes_ptr,
                &num_allocated_all_light_keyframes,
                &num_all_light_keyframes, status);
            if (nanoem_is_not_null(keyframes)) {
                index = num_all_light_keyframes - 1;
                keyframes[index] = origin_keyframe;
                origin_light->all_light_keyframes_ptr = keyframes;
                keyframe->base.is_in_object = nanoem_true;
                nanoemDocumentBaseKeyframeSet(&origin_keyframe->base, frame_index, index, &previous, &next);
                nanoemDocumentKeyframeCompareResultSetKeyframeIndex(&previous, &next, index);
                nanoem_status_ptr_assign_succeeded(status);
            }
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentLightRemoveLightKeyframeObject(nanoem_mutable_document_light_t *light, nanoem_mutable_document_light_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_document_light_t *origin_light;
    nanoem_document_light_keyframe_t **keyframes, *origin_keyframe;
    nanoem_rsize_t num_keyframes, i;
    int index, offset;
    if (nanoem_is_not_null(light) && nanoem_is_not_null(keyframe)) {
        origin_light = light->origin;
        origin_keyframe = keyframe->origin;
        keyframes = origin_light->light_keyframes;
        index = origin_keyframe->base.object_index;
        num_keyframes = origin_light->num_light_keyframes;
        offset = nanoemObjectArrayFindIndexedObjectOffset((const void *const *) keyframes, origin_keyframe, num_keyframes, index);
        if (offset >= 0) {
            num_keyframes--;
            nanoem_crt_memmove(&keyframes[offset], &keyframes[offset + 1], (num_keyframes - offset) * sizeof(origin_keyframe));
            for (i = offset; i < num_keyframes; i++) {
                keyframes[i]->base.object_index--;
            }
            origin_light->num_light_keyframes = num_keyframes;
            keyframe->base.is_in_object = nanoem_false;
            nanoemDocumentBaseKeyframeReset(&origin_keyframe->base);
            nanoem_status_ptr_assign_succeeded(status);
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_LIGHT_KEYFRAME_NOT_FOUND);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentLightSetColor(nanoem_mutable_document_light_t *light, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(light) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(light->origin->color.values, value, sizeof(light->origin->color.values));
    }
}

void APIENTRY
nanoemMutableDocumentLightSetDirection(nanoem_mutable_document_light_t *light, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(light) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(light->origin->direction.values, value, sizeof(light->origin->direction.values));
    }
}

nanoem_document_light_t * APIENTRY
nanoemMutableDocumentLightGetOrigin(nanoem_mutable_document_light_t *light)
{
    return nanoem_is_not_null(light) ? light->origin : NULL;
}

void APIENTRY
nanoemMutableDocumentLightDestroy(nanoem_mutable_document_light_t *light)
{
    if (nanoem_is_not_null(light)) {
        if (nanoemMutableDocumentObjectCanDelete(&light->base)) {
            nanoemDocumentLightDestroy(light->origin);
        }
        nanoem_free(light);
    }
}

nanoem_mutable_document_self_shadow_t * APIENTRY
nanoemMutableDocumentSelfShadowCreate(nanoem_mutable_document_t *document, nanoem_status_t *status)
{
    nanoem_mutable_document_self_shadow_t *self_shadow;
    nanoem_document_self_shadow_keyframe_t *initial_keyframe;
    nanoem_document_self_shadow_t *origin;
    self_shadow = (nanoem_mutable_document_self_shadow_t *) nanoem_calloc(sizeof(*self_shadow), 1, status);
    if (nanoem_is_not_null(self_shadow)) {
        self_shadow->base.parent.document = document;
        origin = self_shadow->origin = nanoemDocumentSelfShadowCreate(nanoemMutableDocumentGetOrigin(document), status);
        if (nanoem_is_not_null(origin)) {
            nanoemDocumentSelfShadowInitialize(origin);
            origin->all_self_shadow_keyframes_ptr = (nanoem_document_self_shadow_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->all_self_shadow_keyframes_ptr), status);
            origin->self_shadow_keyframes = (nanoem_document_self_shadow_keyframe_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->self_shadow_keyframes), status);
            initial_keyframe = nanoemDocumentSelfShadowKeyframeCreate(self_shadow->origin, status);
            if (nanoem_is_not_null(initial_keyframe)) {
                origin->all_self_shadow_keyframes_ptr[0] = origin->initial_self_shadow_keyframe = initial_keyframe;
                self_shadow->num_allocated_self_shadow_keyframes = __nanoem_default_allocation_size;
                nanoemDocumentSelfShadowKeyframeInitialize(initial_keyframe);
            }
        }
    }
    return self_shadow;
}

static nanoem_bool_t
nanoemDocumentSelfShadowFindKeyframeIndex(nanoem_document_self_shadow_t *self_shadow, nanoem_frame_index_t frame_index, nanoem_document_keyframe_compare_result_t *previous, nanoem_document_keyframe_compare_result_t *next)
{
    nanoem_document_self_shadow_keyframe_t *keyframe;
    nanoem_document_self_shadow_keyframe_t *const *keyframes = self_shadow->all_self_shadow_keyframes_ptr;
    nanoem_rsize_t num_keyframes = self_shadow->num_self_shadow_keyframes + 1, i;
    nanoem_bool_t found = nanoem_false;
    for (i = 0; i < num_keyframes; i++) {
        keyframe = keyframes[i];
        found |= nanoemDocumentBaseKeyframeCompare(&keyframe->base, frame_index, previous, next);
    }
    return found;
}

void APIENTRY
nanoemMutableDocumentSelfShadowAddSelfShadowKeyframeObject(nanoem_mutable_document_self_shadow_t *self_shadow, nanoem_mutable_document_self_shadow_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status)
{
    nanoem_document_self_shadow_t *origin_self_shadow;
    nanoem_document_self_shadow_keyframe_t **keyframes, *origin_keyframe, *initial_keyframe;
    nanoem_document_keyframe_compare_result_t previous, next;
    nanoem_rsize_t index, num_allocated_all_self_shadow_keyframes, num_all_self_shadow_keyframes;
    if (nanoem_is_not_null(self_shadow) && nanoem_is_not_null(keyframe)) {
        origin_self_shadow = self_shadow->origin;
        origin_keyframe = keyframe->origin;
        nanoemDocumentKeyframeCompareResultReset(&previous);
        nanoemDocumentKeyframeCompareResultReset(&next);
        if (frame_index == 0) {
            initial_keyframe = origin_self_shadow->initial_self_shadow_keyframe;
            initial_keyframe->base.is_selected = origin_keyframe->base.is_selected;
            initial_keyframe->distance = origin_keyframe->distance;
            initial_keyframe->mode = origin_keyframe->mode;
            nanoem_status_ptr_assign_succeeded(status);
            return;
        }
        else if (nanoemDocumentSelfShadowFindKeyframeIndex(origin_self_shadow, frame_index, &previous, &next)) {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_SELF_SHADOW_KEYFRAME_ALREADY_EXISTS);
            return;
        }
        num_all_self_shadow_keyframes = origin_self_shadow->num_self_shadow_keyframes + 1;
        num_allocated_all_self_shadow_keyframes = self_shadow->num_allocated_self_shadow_keyframes + 1;
        keyframes = (nanoem_document_self_shadow_keyframe_t **) nanoemMutableObjectArrayResize(origin_self_shadow->self_shadow_keyframes,
            &self_shadow->num_allocated_self_shadow_keyframes,
            &origin_self_shadow->num_self_shadow_keyframes, status);
        if (nanoem_is_not_null(keyframes)) {
            index = origin_self_shadow->num_self_shadow_keyframes - 1;
            keyframes[index] = origin_keyframe;
            origin_self_shadow->self_shadow_keyframes = keyframes;
            keyframes = (nanoem_document_self_shadow_keyframe_t **) nanoemMutableObjectArrayResize(origin_self_shadow->all_self_shadow_keyframes_ptr,
                &num_allocated_all_self_shadow_keyframes,
                &num_all_self_shadow_keyframes, status);
            if (nanoem_is_not_null(keyframes)) {
                index = num_all_self_shadow_keyframes - 1;
                keyframes[index] = origin_keyframe;
                origin_self_shadow->all_self_shadow_keyframes_ptr = keyframes;
                keyframe->base.is_in_object = nanoem_true;
                nanoemDocumentBaseKeyframeSet(&origin_keyframe->base, frame_index, index, &previous, &next);
                nanoemDocumentKeyframeCompareResultSetKeyframeIndex(&previous, &next, index);
                nanoem_status_ptr_assign_succeeded(status);
            }
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentSelfShadowRemoveSelfShadowKeyframeObject(nanoem_mutable_document_self_shadow_t *self_shadow, nanoem_mutable_document_self_shadow_keyframe_t *keyframe, nanoem_status_t *status)
{
    nanoem_document_self_shadow_t *origin_self_shadow;
    nanoem_document_self_shadow_keyframe_t **keyframes, *origin_keyframe;
    nanoem_rsize_t num_keyframes, i;
    int index, offset;
    if (nanoem_is_not_null(self_shadow) && nanoem_is_not_null(keyframe)) {
        origin_self_shadow = self_shadow->origin;
        origin_keyframe = keyframe->origin;
        keyframes = origin_self_shadow->self_shadow_keyframes;
        index = origin_keyframe->base.object_index;
        num_keyframes = origin_self_shadow->num_self_shadow_keyframes;
        offset = nanoemObjectArrayFindIndexedObjectOffset((const void *const *) keyframes, origin_keyframe, num_keyframes, index);
        if (offset >= 0) {
            num_keyframes--;
            nanoem_crt_memmove(&keyframes[offset], &keyframes[offset + 1], (num_keyframes - offset) * sizeof(origin_keyframe));
            for (i = offset; i < num_keyframes; i++) {
                keyframes[i]->base.object_index--;
            }
            origin_self_shadow->num_self_shadow_keyframes = num_keyframes;
            keyframe->base.is_in_object = nanoem_false;
            nanoemDocumentBaseKeyframeReset(&origin_keyframe->base);
            nanoem_status_ptr_assign_succeeded(status);
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_DOCUMENT_SELF_SHADOW_KEYFRAME_NOT_FOUND);
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentSelfShadowSetDistance(nanoem_mutable_document_self_shadow_t *self_shadow, nanoem_f32_t value)
{
    if (nanoem_is_not_null(self_shadow)) {
        self_shadow->origin->distance = value;
    }
}

void APIENTRY
nanoemMutableDocumentSelfShadowSetEnabled(nanoem_mutable_document_self_shadow_t *self_shadow, nanoem_bool_t value)
{
    if (nanoem_is_not_null(self_shadow)) {
        self_shadow->origin->is_self_shadow_enabled = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentSelfShadowDestroy(nanoem_mutable_document_self_shadow_t *self_shadow)
{
    if (nanoem_is_not_null(self_shadow)) {
        if (nanoemMutableDocumentObjectCanDelete(&self_shadow->base)) {
            nanoemDocumentSelfShadowDestroy(self_shadow->origin);
        }
        nanoem_free(self_shadow);
    }
}

nanoem_document_self_shadow_t * APIENTRY
nanoemMutableDocumentSelfShadowGetOrigin(nanoem_mutable_document_self_shadow_t *self_shadow)
{
    return nanoem_is_not_null(self_shadow) ? self_shadow->origin : NULL;
}

nanoem_mutable_document_t *APIENTRY
nanoemMutableDocumentCreate(nanoem_unicode_string_factory_t *factory, nanoem_status_t *status)
{
    nanoem_mutable_document_t *document;
    nanoem_document_t *origin;
    document = (nanoem_mutable_document_t *) nanoem_calloc(1, sizeof(*document), status);
    if (nanoem_is_not_null(document)) {
        origin = document->origin = nanoemDocumentCreate(factory, status);
        if (nanoem_is_not_null(origin)) {
            origin->accessories = (nanoem_document_accessory_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->accessories), status);
            origin->models = (nanoem_document_model_t **) nanoem_calloc(__nanoem_default_allocation_size, sizeof(*origin->models), status);
            document->num_allocated_accessories = __nanoem_default_allocation_size;
            document->num_allocated_models = __nanoem_default_allocation_size;
        }
    }
    return document;
}

nanoem_mutable_document_t *APIENTRY
nanoemMutableDocumentCreateAsReference(nanoem_document_t *document, nanoem_status_t *status)
{
    nanoem_mutable_document_t *new_document = NULL;
    if (nanoem_is_not_null(document)) {
        new_document = (nanoem_mutable_document_t *) nanoem_calloc(1, sizeof(*new_document), status);
        if (nanoem_is_not_null(new_document)) {
            new_document->origin = document;
            new_document->num_allocated_accessories = document->num_accessories;
            new_document->num_allocated_models = document->num_models;
            new_document->base.is_reference = nanoem_true;
        }
    }
    return new_document;
}

void APIENTRY
nanoemMutableDocumentInsertAccessoryObject(nanoem_mutable_document_t *document, nanoem_mutable_document_accessory_t *accessory, int index, nanoem_status_t *status)
{
    nanoem_document_t *origin_document;
    nanoem_document_accessory_t *origin_accessory;
    if (nanoem_is_not_null(document) && nanoem_is_not_null(accessory)) {
        origin_document = document->origin;
        origin_accessory = accessory->origin;
        nanoemDocumentObjectArrayInsertObject((nanoem_document_object_t ***) &origin_document->accessories, (nanoem_document_object_t *) origin_accessory, index, NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_ALREADY_EXISTS, &origin_document->num_accessories, &document->num_allocated_accessories, status);
        if (!nanoem_status_ptr_has_error(status)) {
            origin_accessory->base.parent.document = origin_document;
            accessory->base.is_in_object = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentInsertModelObject(nanoem_mutable_document_t *document, nanoem_mutable_document_model_t *model, int index, nanoem_status_t *status)
{
    nanoem_document_t *origin_document;
    nanoem_document_model_t *origin_model;
    if (nanoem_is_not_null(document) && nanoem_is_not_null(model)) {
        origin_document = document->origin;
        origin_model = model->origin;
        nanoemDocumentObjectArrayInsertObject((nanoem_document_object_t ***) &origin_document->models, (nanoem_document_object_t *) origin_model, index, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_ALREADY_EXISTS, &origin_document->num_models, &document->num_allocated_models, status);
        if (!nanoem_status_ptr_has_error(status)) {
            origin_model->base.parent.document = origin_document;
            model->base.is_in_object = nanoem_true;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentRemoveAccessoryObject(nanoem_mutable_document_t *document, nanoem_mutable_document_accessory_t *accessory, nanoem_status_t *status)
{
    nanoem_document_t *origin_document;
    nanoem_document_accessory_t *origin_accessory;
    if (nanoem_is_not_null(document) && nanoem_is_not_null(accessory)) {
        origin_document = document->origin;
        origin_accessory = accessory->origin;
        nanoemDocumentObjectArrayRemoveObject((nanoem_document_object_t **) origin_document->accessories, (nanoem_document_object_t *) origin_accessory, &origin_document->num_accessories, NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            origin_accessory->base.parent.document = NULL;
            accessory->base.is_in_object = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentRemoveModelObject(nanoem_mutable_document_t *document, nanoem_mutable_document_model_t *model, nanoem_status_t *status)
{
    nanoem_document_t *origin_document;
    nanoem_document_model_t *origin_model;
    if (nanoem_is_not_null(document) && nanoem_is_not_null(model)) {
        origin_document = document->origin;
        origin_model = model->origin;
        nanoemDocumentObjectArrayRemoveObject((nanoem_document_object_t **) origin_document->models, (nanoem_document_object_t *) origin_model, &origin_document->num_models, NANOEM_STATUS_ERROR_DOCUMENT_MODEL_NOT_FOUND, status);
        if (!nanoem_status_ptr_has_error(status)) {
            origin_model->base.parent.document = NULL;
            model->base.is_in_object = nanoem_false;
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentSetCameraObject(nanoem_mutable_document_t *document, nanoem_mutable_document_camera_t *value)
{
    if (nanoem_is_not_null(document) && nanoem_is_not_null(value) && document->origin->camera != value->origin) {
        nanoemDocumentCameraDestroy(document->origin->camera);
        document->origin->camera = value->origin;
        value->base.is_in_object = nanoem_true;
    }
}

void APIENTRY
nanoemMutableDocumentSetGravityObject(nanoem_mutable_document_t *document, nanoem_mutable_document_gravity_t *value)
{
    if (nanoem_is_not_null(document) && nanoem_is_not_null(value) && document->origin->gravity != value->origin) {
        nanoemDocumentGravityDestroy(document->origin->gravity);
        document->origin->gravity = value->origin;
        value->base.is_in_object = nanoem_true;
    }
}

void APIENTRY
nanoemMutableDocumentSetLightObject(nanoem_mutable_document_t *document, nanoem_mutable_document_light_t *value)
{
    if (nanoem_is_not_null(document) && nanoem_is_not_null(value) && document->origin->light != value->origin) {
        nanoemDocumentLightDestroy(document->origin->light);
        document->origin->light = value->origin;
        value->base.is_in_object = nanoem_true;
    }
}

void APIENTRY
nanoemMutableDocumentSetSelfShadowObject(nanoem_mutable_document_t *document, nanoem_mutable_document_self_shadow_t *value)
{
    if (nanoem_is_not_null(document) && nanoem_is_not_null(value) && document->origin->self_shadow != value->origin) {
        nanoemDocumentSelfShadowDestroy(document->origin->self_shadow);
        document->origin->self_shadow = value->origin;
        value->base.is_in_object = nanoem_true;
    }
}

void APIENTRY
nanoemMutableDocumentSetAudioPath(nanoem_mutable_document_t *document, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(document)) {
        factory = document->origin->factory;
        nanoemUnicodeStringFactoryAssignString(factory, &document->origin->audio_path, value, status);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentSetBackgroundVideoPath(nanoem_mutable_document_t *document, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(document)) {
        factory = document->origin->factory;
        nanoemUnicodeStringFactoryAssignString(factory, &document->origin->background_video_path, value, status);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentSetBackgroundImagePath(nanoem_mutable_document_t *document, const nanoem_unicode_string_t *value, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory;
    if (nanoem_is_not_null(document)) {
        factory = document->origin->factory;
        nanoemUnicodeStringFactoryAssignString(factory, &document->origin->background_image_path, value, status);
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
}

void APIENTRY
nanoemMutableDocumentSetEdgeColor(nanoem_mutable_document_t *document, const nanoem_f32_t *value)
{
    if (nanoem_is_not_null(document) && nanoem_is_not_null(value)) {
        nanoem_crt_memcpy(document->origin->edge_color.values, value, sizeof(document->origin->edge_color.values));
    }
}

void APIENTRY
nanoemMutableDocumentSetPreferredFPS(nanoem_mutable_document_t *document, nanoem_f32_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->preferred_fps = value;
    }
}

void APIENTRY
nanoemMutableDocumentSetBackgroundVideoScaleFactor(nanoem_mutable_document_t *document, nanoem_f32_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->background_video_scale_factor = value;
    }
}

void APIENTRY
nanoemMutableDocumentSetBackgroundImageScaleFactor(nanoem_mutable_document_t *document, nanoem_f32_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->background_image_scale_factor = value;
    }
}

void APIENTRY
nanoemMutableDocumentSetPhysicsSimulationMode(nanoem_mutable_document_t *document, nanoem_document_physics_simulation_mode_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->physics_simulation_mode = value;
    }
}

void APIENTRY
nanoemMutableDocumentSetCurrentFrameIndex(nanoem_mutable_document_t *document, nanoem_frame_index_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->current_frame_index = value;
    }
}

void APIENTRY
nanoemMutableDocumentSetCurrentFrameIndexInTextField(nanoem_mutable_document_t *document, nanoem_frame_index_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->current_frame_index_in_text_field = value;
    }
}

void APIENTRY
nanoemMutableDocumentSetBeginFrameIndex(nanoem_mutable_document_t *document, nanoem_frame_index_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->begin_frame_index = value;
    }
}

void APIENTRY
nanoemMutableDocumentSetEndFrameIndex(nanoem_mutable_document_t *document, nanoem_frame_index_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->end_frame_index = value;
    }
}

void APIENTRY
nanoemMutableDocumentSetEditingMode(nanoem_mutable_document_t *document, nanoem_document_editing_mode_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->editing_mode = value;
    }
}

void APIENTRY
nanoemMutableDocumentSetViewportWidth(nanoem_mutable_document_t *document, int value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->output_width = value;
    }
}

void APIENTRY
nanoemMutableDocumentSetViewportHeight(nanoem_mutable_document_t *document, int value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->output_height = value;
    }
}

void APIENTRY
nanoemMutableDocumentSetTimelineWidth(nanoem_mutable_document_t *document, int value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->timeline_width = value;
    }
}

void APIENTRY
nanoemMutableDocumentSetSelectedModelIndex(nanoem_mutable_document_t *document, int value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->select_model_index = (nanoem_rsize_t) value < document->origin->num_models ? value : -1;
    }
}

void APIENTRY
nanoemMutableDocumentSetSelectedAccessoryIndex(nanoem_mutable_document_t *document, int value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->select_accessory_index = (nanoem_rsize_t) value < document->origin->num_accessories ? value : -1;
    }
}

void APIENTRY
nanoemMutableDocumentSetAccessoryIndexAfterModel(nanoem_mutable_document_t *document, int value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->accessory_index_after_models = (nanoem_rsize_t) value < document->origin->num_accessories ? value : -1;
    }
}

void APIENTRY
nanoemMutableDocumentSetBackgroundVideoOffsetX(nanoem_mutable_document_t *document, int value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->background_video_offset_x = value;
    }
}

void APIENTRY
nanoemMutableDocumentSetBackgroundVideoOffsetY(nanoem_mutable_document_t *document, int value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->background_video_offset_y = value;
    }
}

void APIENTRY
nanoemMutableDocumentSetBackgroundImageOffsetX(nanoem_mutable_document_t *document, int value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->background_image_offset_x = value;
    }
}

void APIENTRY
nanoemMutableDocumentSetBackgroundImageOffsetY(nanoem_mutable_document_t *document, int value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->background_image_offset_y = value;
    }
}

void APIENTRY
nanoemMutableDocumentSetAudioEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->is_audio_enabled = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentSetBackgroundVideoEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->is_background_video_enabled = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentSetBackgroundImageEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->is_background_image_enabled = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentSetEditingCLAEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->editing_cla = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentSetGridAndAxisShown(nanoem_mutable_document_t *document, nanoem_bool_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->is_grid_and_axis_shown = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentSetInformationShown(nanoem_mutable_document_t *document, nanoem_bool_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->is_information_shown = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentSetGroundShadowShown(nanoem_mutable_document_t *document, nanoem_bool_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->is_ground_shadow_shown = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentSetLoopEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->is_loop_enabled = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentSetBeginFrameIndexEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->is_begin_frame_index_enabled = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentSetEndFrameIndexEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->is_end_frame_index_enabled = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentSetPhysicsGroundEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->is_physics_ground_enabled = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentSetTranslucentGroundShadowEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->is_translucent_ground_shadow_enabled = value ? nanoem_true : nanoem_false;
    }
}

void APIENTRY
nanoemMutableDocumentSetBlackBackgroundEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value)
{
    if (nanoem_is_not_null(document)) {
        document->origin->is_black_background_enabled = value ? nanoem_true : nanoem_false;
    }
}

nanoem_document_t *APIENTRY
nanoemMutableDocumentGetOrigin(nanoem_mutable_document_t *document)
{
    return nanoem_is_not_null(document) ? document->origin : NULL;
}

void APIENTRY
nanoemMutableBufferWriteDocumentBaseKeyframe(nanoem_mutable_buffer_t *buffer, const nanoem_document_base_keyframe_t *base, nanoem_bool_t include_index, nanoem_status_t *status)
{
    if (include_index) {
        nanoemMutableBufferWriteInt32LittleEndian(buffer, base->object_index, status);
    }
    nanoemMutableBufferWriteInt32LittleEndian(buffer, base->frame_index, status);
    nanoemMutableBufferWriteInt32LittleEndian(buffer, base->previous_keyframe_index, status);
    nanoemMutableBufferWriteInt32LittleEndian(buffer, base->next_keyframe_index, status);
}

void APIENTRY
nanoemMutableBufferWriteKeyframeInterpolation(nanoem_mutable_buffer_t *buffer, const nanoem_interpolation_t *interpolation, nanoem_status_t *status)
{
    nanoemMutableBufferWriteByte(buffer, interpolation->u.alias.x0, status);
    nanoemMutableBufferWriteByte(buffer, interpolation->u.alias.y0, status);
    nanoemMutableBufferWriteByte(buffer, interpolation->u.alias.x1, status);
    nanoemMutableBufferWriteByte(buffer, interpolation->u.alias.y1, status);
}

static nanoem_u8_t
nanoemDocumentAccessoryPackOpacityAndVisible(nanoem_f32_t opacity, nanoem_bool_t visible)
{
    return visible | ((nanoem_u32_t) ((1.0f - opacity) * 100.0f) << 1);
}

void APIENTRY
nanoemDocumentAccessoryKeyframeSaveToBuffer(const nanoem_document_accessory_keyframe_t *keyframe, nanoem_bool_t include_index, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_u8_t opacity_and_visible = nanoemDocumentAccessoryPackOpacityAndVisible(keyframe->opacity, keyframe->visible);
    nanoemMutableBufferWriteDocumentBaseKeyframe(buffer, &keyframe->base, include_index, status);
    nanoemMutableBufferWriteByte(buffer, opacity_and_visible, status);
    nanoemMutableBufferWriteInt32LittleEndian(buffer, keyframe->parent_model_index, status);
    nanoemMutableBufferWriteInt32LittleEndian(buffer, keyframe->parent_model_bone_index, status);
    nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &keyframe->translation, status);
    nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &keyframe->orientation, status);
    nanoemMutableBufferWriteFloat32LittleEndian(buffer, keyframe->scale_factor, status);
    nanoemMutableBufferWriteByte(buffer, keyframe->is_shadow_enabled, status);
    nanoemMutableBufferWriteByte(buffer, keyframe->base.is_selected, status);
}

void APIENTRY
nanoemDocumentAccessorySaveToBuffer(const nanoem_document_accessory_t *accessory, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_document_t *parent_document;
    nanoem_u8_t name_buffer[NANOEM_PMM_ACCESSORY_NAME_MAX], path_buffer[NANOEM_PMM_PATH_MAX],
                   opacity_and_visible = nanoemDocumentAccessoryPackOpacityAndVisible(accessory->opacity, accessory->visible);;
    nanoem_rsize_t i, num_keyframes;
    parent_document = nanoemDocumentAccessoryGetParentDocument(accessory);
    nanoemMutableBufferWriteByte(buffer, accessory->base.index, status);
    nanoemDocumentWriteFixedStringPMM(parent_document, accessory->name, name_buffer, sizeof(name_buffer), buffer, status);
    nanoemDocumentWriteFixedStringPMM(parent_document, accessory->path, path_buffer, sizeof(path_buffer), buffer, status);
    nanoemMutableBufferWriteByte(buffer, accessory->draw_order_index, status);
    nanoemDocumentAccessoryKeyframeSaveToBuffer(accessory->initial_accessory_keyframe, nanoem_false, buffer, status);
    num_keyframes = accessory->num_accessory_keyframes;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_keyframes, status);
    for (i = 0; i < num_keyframes; i++) {
        nanoemDocumentAccessoryKeyframeSaveToBuffer(accessory->accessory_keyframes[i], nanoem_true, buffer, status);
    }
    opacity_and_visible = accessory->visible | ((nanoem_u32_t) ((1.0f - accessory->opacity) * 100.0f) << 1);
    nanoemMutableBufferWriteByte(buffer, opacity_and_visible, status);
    nanoemMutableBufferWriteInt32LittleEndian(buffer, accessory->parent_model_index, status);
    nanoemMutableBufferWriteInt32LittleEndian(buffer, accessory->parent_model_bone_index, status);
    nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &accessory->translation, status);
    nanoemMutableBufferWriteFloat32LittleEndian(buffer, accessory->scale_factor, status);
    nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &accessory->orientation, status);
    nanoemMutableBufferWriteByte(buffer, accessory->is_shadow_enabled, status);
    if (accessory->base.parent.document->version > 1) {
        nanoemMutableBufferWriteByte(buffer, accessory->is_add_blend_enabled, status);
    }
}

void APIENTRY
nanoemDocumentSaveAllAccessoriesToBuffer(const nanoem_document_t *document, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_u8_t name_buffer[NANOEM_PMM_ACCESSORY_NAME_MAX];
    nanoem_rsize_t i, num_accessories;
    num_accessories = document->num_accessories;
    nanoemMutableBufferWriteByte(buffer, (nanoem_u8_t) num_accessories, status);
    for (i = 0; i < num_accessories; i++) {
        nanoemDocumentWriteFixedStringPMM(document, document->accessories[i]->name, name_buffer, sizeof(name_buffer), buffer, status);
    }
    for (i = 0; i < num_accessories; i++) {
        nanoemDocumentAccessorySaveToBuffer(document->accessories[i], buffer, status);
    }
}

void APIENTRY
nanoemDocumentModelBoneKeyframeSaveToBuffer(const nanoem_document_model_bone_keyframe_t *keyframe, nanoem_bool_t include_index, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemMutableBufferWriteDocumentBaseKeyframe(buffer, &keyframe->base, include_index, status);
    nanoemMutableBufferWriteKeyframeInterpolation(buffer, &keyframe->interpolation[0], status);
    nanoemMutableBufferWriteKeyframeInterpolation(buffer, &keyframe->interpolation[1], status);
    nanoemMutableBufferWriteKeyframeInterpolation(buffer, &keyframe->interpolation[2], status);
    nanoemMutableBufferWriteKeyframeInterpolation(buffer, &keyframe->interpolation[3], status);
    nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &keyframe->translation, status);
    nanoemMutableBufferWriteFloat32x4LittleEndian(buffer, &keyframe->orientation, status);
    nanoemMutableBufferWriteByte(buffer, keyframe->base.is_selected, status);
    if (keyframe->parent_model->base.parent.document->version > 1) {
        nanoemMutableBufferWriteByte(buffer, keyframe->is_physics_simulation_disabled, status);
    }
}

void APIENTRY
nanoemDocumentOutsideParentSaveToBuffer(const nanoem_document_outside_parent_t *parent, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemMutableBufferWriteInt32LittleEndian(buffer, parent->model_index, status);
    nanoemMutableBufferWriteInt32LittleEndian(buffer, parent->bone_index, status);
}

void APIENTRY
nanoemDocumentModelKeyframeSaveToBuffer(const nanoem_document_model_keyframe_t *keyframe, nanoem_bool_t include_index, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_rsize_t i, num_enable_constraint_bones, num_outside_parent_bones, delta;
    nanoemMutableBufferWriteDocumentBaseKeyframe(buffer, &keyframe->base, include_index, status);
    nanoemMutableBufferWriteByte(buffer, keyframe->visible, status);
    num_enable_constraint_bones = keyframe->num_constraint_states;
    for (i = 0; i < num_enable_constraint_bones; i++) {
        nanoemMutableBufferWriteByte(buffer, keyframe->constraint_states[i]->enabled, status);
    }
    if (num_enable_constraint_bones < keyframe->parent_model->num_constraint_bones) {
        delta = keyframe->parent_model->num_constraint_bones - num_enable_constraint_bones;
        for (i = 0; i < delta; i++) {
            nanoemMutableBufferWriteByte(buffer, 1, status);
        }
    }
    if (keyframe->parent_model->base.parent.document->version > 1) {
        num_outside_parent_bones = keyframe->num_outside_parents;
        for (i = 0; i < num_outside_parent_bones; i++) {
            nanoemDocumentOutsideParentSaveToBuffer(keyframe->outside_parents[i], buffer, status);
        }
    }
    nanoemMutableBufferWriteByte(buffer, keyframe->base.is_selected, status);
}

void APIENTRY
nanoemDocumentModelBoneStateSaveToBuffer(const nanoem_document_model_bone_state_t *state, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &state->translation, status);
    nanoemMutableBufferWriteFloat32x4LittleEndian(buffer, &state->orientation, status);
    if (state->base.parent.model->base.parent.document->version > 1) {
        nanoemMutableBufferWriteByte(buffer, state->dirty, status);
        nanoemMutableBufferWriteByte(buffer, state->disable_physics_simulation, status);
        nanoemMutableBufferWriteByte(buffer, state->num_tracks_selected, status);
    }
    else {
        nanoemMutableBufferWriteInt32LittleEndian(buffer, 0, status); /* unknown */
        nanoemMutableBufferWriteByte(buffer, state->dirty, status);
        nanoemMutableBufferWriteByte(buffer, state->num_tracks_selected, status);
    }
}

void APIENTRY
nanoemDocumentModelConstraintStateSaveToBuffer(const nanoem_document_model_constraint_state_t *state, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemMutableBufferWriteByte(buffer, state->enabled, status);
}

void APIENTRY
nanoemDocumentModelMorphStateSaveToBuffer(const nanoem_document_model_morph_state_t *state, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemMutableBufferWriteFloat32LittleEndian(buffer, state->weight, status);
}

void APIENTRY
nanoemDocumentModelOutsideParentStateSaveToBuffer(const nanoem_document_model_outside_parent_state_t *state, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemMutableBufferWriteInt32LittleEndian(buffer, state->begin, status);
    nanoemMutableBufferWriteInt32LittleEndian(buffer, state->end, status);
    nanoemDocumentOutsideParentSaveToBuffer(state->outside_parent, buffer, status);
}

void APIENTRY
nanoemDocumentModelMorphKeyframeSaveToBuffer(const nanoem_document_model_morph_keyframe_t *keyframe, nanoem_bool_t include_index, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemMutableBufferWriteDocumentBaseKeyframe(buffer, &keyframe->base, include_index, status);
    nanoemMutableBufferWriteFloat32LittleEndian(buffer, keyframe->weight, status);
    nanoemMutableBufferWriteByte(buffer, keyframe->base.is_selected, status);
}

void APIENTRY
nanoemDocumentModelSaveToBufferPMMv1(const nanoem_document_model_t *model, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemMutableBufferWriteByte(buffer, model->base.index, status);
    /* FIXME: implement this */
}

void APIENTRY
nanoemDocumentModelSaveToBufferPMMv2(const nanoem_document_model_t *model, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_document_t *document = nanoemDocumentModelGetParentDocument(model);
    nanoem_u8_t path_buffer[NANOEM_PMM_PATH_MAX];
    nanoem_rsize_t i, num_bones, num_morphs, num_constraint_bones, num_outside_parent_bones, num_expansion_states, num_keyframes;
    nanoemMutableBufferWriteByte(buffer, model->base.index, status);
    nanoemDocumentWriteVariableStringPMM(document, model->name_ja, buffer, status);
    nanoemDocumentWriteVariableStringPMM(document, model->name_en, buffer, status);
    nanoemDocumentWriteFixedStringPMM(model->base.parent.document, model->path, path_buffer, sizeof(path_buffer), buffer, status);
    nanoemMutableBufferWriteByte(buffer, model->num_fixed_tracks, status);
    num_bones = model->num_bones;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_bones, status);
    for (i = 0; i < num_bones; i++) {
        nanoemDocumentWriteVariableStringPMM(document, model->bone_names[i], buffer, status);
    }
    num_morphs = model->num_morphs;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_morphs, status);
    for (i = 0; i < num_morphs; i++) {
        nanoemDocumentWriteVariableStringPMM(document, model->morph_names[i], buffer, status);
    }
    num_constraint_bones = model->num_constraint_bones;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_constraint_bones, status);
    for (i = 0; i < num_constraint_bones; i++) {
        nanoemMutableBufferWriteInt32LittleEndian(buffer, model->constraint_bone_indices[i], status);
    }
    num_outside_parent_bones = model->num_outside_parent_subject_bones;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_outside_parent_bones, status);
    for (i = 0; i < num_outside_parent_bones; i++) {
        nanoemMutableBufferWriteInt32LittleEndian(buffer, model->outside_parent_subject_bone_indices[i], status);
    }
    nanoemMutableBufferWriteByte(buffer, model->draw_order_index + 1, status);
    nanoemMutableBufferWriteByte(buffer, model->visible, status);
    nanoemMutableBufferWriteInt32LittleEndian(buffer, model->selected_bone_index, status);
    nanoemMutableBufferWriteInt32LittleEndian(buffer, model->selected_morph_indices[0], status);
    nanoemMutableBufferWriteInt32LittleEndian(buffer, model->selected_morph_indices[1], status);
    nanoemMutableBufferWriteInt32LittleEndian(buffer, model->selected_morph_indices[2], status);
    nanoemMutableBufferWriteInt32LittleEndian(buffer, model->selected_morph_indices[3], status);
    num_expansion_states = model->num_expansion_states;
    nanoemMutableBufferWriteByte(buffer, (nanoem_u8_t) num_expansion_states, status);
    for (i = 0; i < num_expansion_states; i++) {
        nanoemMutableBufferWriteByte(buffer, model->expansion_state[i], status);
    }
    nanoemMutableBufferWriteInt32LittleEndian(buffer, model->vertical_scroll, status);
    nanoemMutableBufferWriteInt32LittleEndian(buffer, model->last_frame_index, status);
    for (i = 0; i < num_bones; i++) {
        nanoemDocumentModelBoneKeyframeSaveToBuffer(model->initial_bone_keyframes[i], nanoem_false, buffer, status);
    }
    num_keyframes = model->num_bone_keyframes;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, num_keyframes, status);
    for (i = 0; i < num_keyframes; i++) {
        nanoemDocumentModelBoneKeyframeSaveToBuffer(model->bone_keyframes[i], nanoem_true, buffer, status);
    }
    for (i = 0; i < num_morphs; i++) {
        nanoemDocumentModelMorphKeyframeSaveToBuffer(model->initial_morph_keyframes[i], nanoem_false, buffer, status);
    }
    num_keyframes = model->num_morph_keyframes;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, num_keyframes, status);
    for (i = 0; i < num_keyframes; i++) {
        nanoemDocumentModelMorphKeyframeSaveToBuffer(model->morph_keyframes[i], nanoem_true, buffer, status);
    }
    nanoemDocumentModelKeyframeSaveToBuffer(model->initial_model_keyframe, nanoem_false, buffer, status);
    num_keyframes = model->num_model_keyframes;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, num_keyframes, status);
    for (i = 0; i < num_keyframes; i++) {
        nanoemDocumentModelKeyframeSaveToBuffer(model->model_keyframes[i], nanoem_true, buffer, status);
    }
    for (i = 0; i < num_bones; i++) {
        nanoemDocumentModelBoneStateSaveToBuffer(model->bone_states[i], buffer, status);
    }
    for (i = 0; i < num_morphs; i++) {
        nanoemDocumentModelMorphStateSaveToBuffer(model->morph_states[i], buffer, status);
    }
    for (i = 0; i < num_constraint_bones; i++) {
        nanoemDocumentModelConstraintStateSaveToBuffer(model->constraint_states[i], buffer, status);
    }
    for (i = 0; i < num_outside_parent_bones; i++) {
        nanoemDocumentModelOutsideParentStateSaveToBuffer(model->outside_parent_states[i], buffer, status);
    }
    nanoemMutableBufferWriteByte(buffer, model->is_blend_enabled, status);
    nanoemMutableBufferWriteFloat32LittleEndian(buffer, model->edge_width, status);
    nanoemMutableBufferWriteByte(buffer, model->is_self_shadow_enabled, status);
    nanoemMutableBufferWriteByte(buffer, model->transform_order_index, status);
}

void APIENTRY
nanoemDocumentSaveAllModelsToBuffer(const nanoem_document_t *document, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_u8_t name_buffer[NANOEM_PMMV1_MODEL_NAME_MAX];
    nanoem_rsize_t i, num_models;
    num_models = document->num_models;
    nanoemMutableBufferWriteByte(buffer, (nanoem_u8_t) document->select_model_index, status);
    nanoemMutableBufferWriteByte(buffer, (nanoem_u8_t) num_models, status);
    if (document->version == 1) {
        for (i = 0; i < num_models; i++) {
            nanoemDocumentWriteFixedStringPMM(document, document->models[i]->name_ja, name_buffer, sizeof(name_buffer), buffer, status);
        }
    }
    if (document->version > 1) {
        for (i = 0; i < num_models; i++) {
            nanoemDocumentModelSaveToBufferPMMv2(document->models[i], buffer, status);
        }
    }
    else if (document->version == 1) {
        for (i = 0; i < num_models; i++) {
            nanoemDocumentModelSaveToBufferPMMv1(document->models[i], buffer, status);
        }
    }
}

void APIENTRY
nanoemDocumentCameraKeyframeSaveToBuffer(const nanoem_document_camera_keyframe_t *keyframe, nanoem_bool_t include_index, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemMutableBufferWriteDocumentBaseKeyframe(buffer, &keyframe->base, include_index, status);
    nanoemMutableBufferWriteFloat32LittleEndian(buffer, keyframe->distance, status);
    nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &keyframe->look_at, status);
    nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &keyframe->angle, status);
    if (keyframe->parent_camera->base.parent.document->version > 1) {
        nanoemMutableBufferWriteInt32LittleEndian(buffer, keyframe->parent_model_index, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, keyframe->parent_model_bone_index, status);
    }
    nanoemMutableBufferWriteKeyframeInterpolation(buffer, &keyframe->interpolation[0], status);
    nanoemMutableBufferWriteKeyframeInterpolation(buffer, &keyframe->interpolation[1], status);
    nanoemMutableBufferWriteKeyframeInterpolation(buffer, &keyframe->interpolation[2], status);
    nanoemMutableBufferWriteKeyframeInterpolation(buffer, &keyframe->interpolation[3], status);
    nanoemMutableBufferWriteKeyframeInterpolation(buffer, &keyframe->interpolation[4], status);
    nanoemMutableBufferWriteKeyframeInterpolation(buffer, &keyframe->interpolation[5], status);
    nanoemMutableBufferWriteByte(buffer, keyframe->is_perspective_view ? 0 : 1, status);
    nanoemMutableBufferWriteInt32LittleEndian(buffer, keyframe->fov, status);
    nanoemMutableBufferWriteByte(buffer, keyframe->base.is_selected, status);
}

void APIENTRY
nanoemDocumentCameraSaveToBuffer(const nanoem_document_camera_t *camera, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_rsize_t i, num_keyframes;
    nanoemDocumentCameraKeyframeSaveToBuffer(camera->initial_camera_keyframe, nanoem_false, buffer, status);
    num_keyframes = camera->num_camera_keyframes;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_keyframes, status);
    for (i = 0; i < num_keyframes; i++) {
        nanoemDocumentCameraKeyframeSaveToBuffer(camera->camera_keyframes[i], nanoem_true, buffer, status);
    }
    nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &camera->look_at, status);
    nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &camera->position, status);
    nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &camera->angle, status);
    nanoemMutableBufferWriteByte(buffer, camera->is_perspective ? 0 : 1, status);
}

void APIENTRY
nanoemDocumentGravityKeyframeSaveToBuffer(const nanoem_document_gravity_keyframe_t *keyframe, nanoem_bool_t include_index, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemMutableBufferWriteDocumentBaseKeyframe(buffer, &keyframe->base, include_index, status);
    nanoemMutableBufferWriteByte(buffer, keyframe->is_noise_enabled, status);
    nanoemMutableBufferWriteInt32LittleEndian(buffer, keyframe->noise, status);
    nanoemMutableBufferWriteFloat32LittleEndian(buffer, keyframe->acceleration, status);
    nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &keyframe->direction, status);
    nanoemMutableBufferWriteByte(buffer, keyframe->base.is_selected, status);
}

void APIENTRY
nanoemDocumentGravitySaveToBuffer(const nanoem_document_gravity_t *gravity, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_rsize_t i, num_keyframes;
    nanoemMutableBufferWriteFloat32LittleEndian(buffer, gravity->acceleration, status);
    nanoemMutableBufferWriteInt32LittleEndian(buffer, gravity->noise, status);
    nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &gravity->direction, status);
    nanoemMutableBufferWriteByte(buffer, gravity->is_noise_enabled, status);
    nanoemDocumentGravityKeyframeSaveToBuffer(gravity->initial_gravity_keyframe, nanoem_false, buffer, status);
    num_keyframes = gravity->num_gravity_keyframes;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_keyframes, status);
    for (i = 0; i < num_keyframes; i++) {
        nanoemDocumentGravityKeyframeSaveToBuffer(gravity->gravity_keyframes[i], nanoem_true, buffer, status);
    }
}

void APIENTRY
nanoemDocumentLightKeyframeSaveToBuffer(const nanoem_document_light_keyframe_t *keyframe, nanoem_bool_t include_index, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemMutableBufferWriteDocumentBaseKeyframe(buffer, &keyframe->base, include_index, status);
    nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &keyframe->color, status);
    nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &keyframe->direction, status);
    nanoemMutableBufferWriteByte(buffer, keyframe->base.is_selected, status);
}

void APIENTRY
nanoemDocumentLightSaveToBuffer(const nanoem_document_light_t *light, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_rsize_t i, num_keyframes;
    nanoemDocumentLightKeyframeSaveToBuffer(light->initial_light_keyframe, nanoem_false, buffer, status);
    num_keyframes = light->num_light_keyframes;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_keyframes, status);
    for (i = 0; i < num_keyframes; i++) {
        nanoemDocumentLightKeyframeSaveToBuffer(light->light_keyframes[i], nanoem_true, buffer, status);
    }
    nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &light->color, status);
    nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &light->direction, status);
}

void APIENTRY
nanoemDocumentSelfShadowKeyframeSaveToBuffer(const nanoem_document_self_shadow_keyframe_t *keyframe, nanoem_bool_t include_index, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoemMutableBufferWriteDocumentBaseKeyframe(buffer, &keyframe->base, include_index, status);
    nanoemMutableBufferWriteByte(buffer, keyframe->mode, status);
    nanoemMutableBufferWriteFloat32LittleEndian(buffer, keyframe->distance, status);
    nanoemMutableBufferWriteByte(buffer, keyframe->base.is_selected, status);
}

void APIENTRY
nanoemDocumentSelfShadowSaveToBuffer(const nanoem_document_self_shadow_t *self_shadow, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_rsize_t i, num_keyframes;
    nanoemMutableBufferWriteByte(buffer, self_shadow->is_self_shadow_enabled, status);
    nanoemMutableBufferWriteFloat32LittleEndian(buffer, self_shadow->distance, status);
    nanoemDocumentSelfShadowKeyframeSaveToBuffer(self_shadow->initial_self_shadow_keyframe, nanoem_false, buffer, status);
    num_keyframes = self_shadow->num_self_shadow_keyframes;
    nanoemMutableBufferWriteInt32LittleEndian(buffer, (nanoem_i32_t) num_keyframes, status);
    for (i = 0; i < num_keyframes; i++) {
        nanoemDocumentSelfShadowKeyframeSaveToBuffer(self_shadow->self_shadow_keyframes[i], nanoem_true, buffer, status);
    }
}

nanoem_bool_t APIENTRY
nanoemMutableDocumentSaveToBuffer(nanoem_mutable_document_t *document, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    const nanoem_document_t *origin;
    nanoem_u8_t path_buffer[NANOEM_PMM_PATH_MAX];
    nanoem_rsize_t i, rest;
    if (nanoem_is_not_null(document) && nanoem_is_not_null(buffer)) {
        nanoem_status_ptr_assign_succeeded(status);
        origin = document->origin;
        switch (origin->version) {
        case 1:
            nanoemMutableBufferWriteByteArray(buffer, (const nanoem_u8_t *) __nanoem_pmmv1_signature, sizeof(__nanoem_pmmv1_signature) - 1, status);
            rest = 30 - (sizeof(__nanoem_pmmv1_signature) - 1);
            for (i = 0; i < rest; i++) {
                nanoemMutableBufferWriteByte(buffer, 0, status);
            }
            break;
        case 2:
        default:
            nanoemMutableBufferWriteByteArray(buffer, (const nanoem_u8_t *) __nanoem_pmmv2_signature, sizeof(__nanoem_pmmv2_signature) - 1, status);
            rest = 30 - (sizeof(__nanoem_pmmv2_signature) - 1);
            for (i = 0; i < rest; i++) {
                nanoemMutableBufferWriteByte(buffer, 0, status);
            }
            break;
        }
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->output_width, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->output_height, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->timeline_width, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->camera_fov, status);
        nanoemMutableBufferWriteByte(buffer, origin->editing_cla, status);
        nanoemMutableBufferWriteByte(buffer, origin->expand_camera_panel, status);
        nanoemMutableBufferWriteByte(buffer, origin->expand_light_panel, status);
        nanoemMutableBufferWriteByte(buffer, origin->expand_accessory_panel, status);
        nanoemMutableBufferWriteByte(buffer, origin->expand_bone_panel, status);
        nanoemMutableBufferWriteByte(buffer, origin->expand_morph_panel, status);
        if (origin->version > 1) {
            nanoemMutableBufferWriteByte(buffer, origin->expand_self_shadow_panel, status);
        }
        nanoemDocumentSaveAllModelsToBuffer(origin, buffer, status);
        nanoemDocumentCameraSaveToBuffer(origin->camera, buffer, status);
        nanoemDocumentLightSaveToBuffer(origin->light, buffer, status);
        nanoemMutableBufferWriteByte(buffer, origin->select_accessory_index, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->horizontal_scroll_for_accessory, status);
        nanoemDocumentSaveAllAccessoriesToBuffer(origin, buffer, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->current_frame_index, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->horizontal_scroll, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->horizontal_scroll_thumb, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->editing_mode, status);
        nanoemMutableBufferWriteByte(buffer, origin->camera_look_mode, status);
        nanoemMutableBufferWriteByte(buffer, origin->is_loop_enabled, status);
        nanoemMutableBufferWriteByte(buffer, origin->is_begin_frame_index_enabled, status);
        nanoemMutableBufferWriteByte(buffer, origin->is_end_frame_index_enabled, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->begin_frame_index, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->end_frame_index, status);
        nanoemMutableBufferWriteByte(buffer, origin->is_audio_enabled, status);
        nanoemDocumentWriteFixedStringPMM(origin, origin->audio_path, path_buffer, sizeof(path_buffer), buffer, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->background_video_offset_x, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->background_video_offset_y, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->background_video_scale_factor, status);
        nanoemDocumentWriteFixedStringPMM(origin, origin->background_video_path, path_buffer, sizeof(path_buffer), buffer, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->is_background_video_enabled, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->background_image_offset_x, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->background_image_offset_y, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->background_image_scale_factor, status);
        nanoemDocumentWriteFixedStringPMM(origin, origin->background_image_path, path_buffer, sizeof(path_buffer), buffer, status);
        nanoemMutableBufferWriteByte(buffer, origin->is_background_image_enabled, status);
        nanoemMutableBufferWriteByte(buffer, origin->is_information_shown, status);
        nanoemMutableBufferWriteByte(buffer, origin->is_grid_and_axis_shown, status);
        nanoemMutableBufferWriteByte(buffer, origin->is_ground_shadow_shown, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->preferred_fps, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->screen_capture_mode, status);
        nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->accessory_index_after_models, status);
        nanoemMutableBufferWriteFloat32LittleEndian(buffer, origin->ground_shadow_brightness, status);
        if (origin->version > 1) {
            nanoemMutableBufferWriteByte(buffer, origin->is_translucent_ground_shadow_enabled, status);
            nanoemMutableBufferWriteByte(buffer, origin->physics_simulation_mode, status);
            nanoemDocumentGravitySaveToBuffer(origin->gravity, buffer, status);
            nanoemDocumentSelfShadowSaveToBuffer(origin->self_shadow, buffer, status);
            nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, &origin->edge_color, status);
            nanoemMutableBufferWriteByte(buffer, origin->is_black_background_enabled, status);
            nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->camera_look_at_model_index, status);
            nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->camera_look_at_model_bone_index, status);
            for (i = 0; i < 16; i++) {
                nanoemMutableBufferWriteFloat32LittleEndian(buffer, i % 5 == 0 ? 1 : 0, status);
            }
            nanoemMutableBufferWriteByte(buffer, origin->is_following_look_at_enabled, status);
            nanoemMutableBufferWriteByte(buffer, 0, status);
            nanoemMutableBufferWriteByte(buffer, origin->is_physics_ground_enabled, status);
            nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->current_frame_index_in_text_field, status);
            if (origin->num_models > 0) {
                nanoemMutableBufferWriteByte(buffer, 1, status);
                for (i = 0; i < origin->num_models; i++) {
                    nanoemMutableBufferWriteByte(buffer, (nanoem_u8_t) i, status);
                    nanoemMutableBufferWriteInt32LittleEndian(buffer, origin->models[i]->selection_index, status);
                }
            }
            else {
                nanoemMutableBufferWriteByte(buffer, 0, status);
            }
        }
    }
    else {
        nanoem_status_ptr_assign_null_object(status);
    }
    return !nanoem_status_ptr_has_error(status);
}

void APIENTRY
nanoemMutableDocumentDestroy(nanoem_mutable_document_t *document)
{
    if (nanoem_is_not_null(document)) {
        if (nanoemMutableDocumentObjectCanDelete(&document->base)) {
            nanoemDocumentDestroy(document->origin);
        }
        nanoem_free(document);
    }
}
