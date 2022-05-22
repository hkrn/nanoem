/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "motion.h"

#include "../nanoem_p.h"
#include "./mutable_p.h"

#include "./motion.pb-c.c"

#include <stdio.h>

typedef struct nanoem_protobufc_buffer_writer_t nanoem_protobufc_buffer_writer_t;

struct nanoem_protobufc_buffer_writer_t {
    ProtobufCBuffer base;
    nanoem_mutable_buffer_t *ptr;
    nanoem_status_t status;
};

static void
nanoemProtobufCBufferWriterAppend(ProtobufCBuffer *buffer, size_t len, const uint8_t *data)
{
    nanoem_protobufc_buffer_writer_t *writer = (nanoem_protobufc_buffer_writer_t *) buffer;
    nanoemMutableBufferWriteByteArray(writer->ptr, data, len, &writer->status);
}

static void *
nanoemProtobufCAllocatorAlloc(void *opaque, size_t size)
{
    nanoem_mark_unused(opaque);
    return nanoem_malloc(size, NULL);
}

static void
nanoemProtobufCAllocatorFree(void *opaque, void *ptr)
{
    nanoem_mark_unused(opaque);
    nanoem_free(ptr);
}

static ProtobufCAllocator __nanoem_protobuf_allocator = {
    nanoemProtobufCAllocatorAlloc,
    nanoemProtobufCAllocatorFree,
    NULL
};

NANOEM_DECL_INLINE static void
nanoemMotionReadVector3NMD(nanoem_f128_t *dst, const Nanoem__Motion__Vector3 *src)
{
    dst->values[0] = src->x;
    dst->values[1] = src->y;
    dst->values[2] = src->z;
}

NANOEM_DECL_INLINE static void
nanoemMotionReadVector4NMD(nanoem_f128_t *dst, const Nanoem__Motion__Vector4 *src)
{
    dst->values[0] = src->x;
    dst->values[1] = src->y;
    dst->values[2] = src->z;
    dst->values[3] = src->w;
}

static void
nanoemMotionReadInterpolation(nanoem_u8_t *value, const Nanoem__Motion__InterpolationUnit *interpolation)
{
    Nanoem__Motion__IntegerInterpolation *integer_type;
    Nanoem__Motion__FloatInterpolation *float_type;
    if (interpolation) {
        switch (interpolation->unit_case) {
        case NANOEM__MOTION__INTERPOLATION_UNIT__UNIT_INTEGER_TYPE:
            integer_type = interpolation->integer_type;
            *value++ = (uint8_t) integer_type->x0;
            *value++ = (uint8_t) integer_type->y0;
            *value++ = (uint8_t) integer_type->x1;
            *value++ = (uint8_t) integer_type->y1;
            break;
        case NANOEM__MOTION__INTERPOLATION_UNIT__UNIT_FLOAT_TYPE:
            float_type = interpolation->float_type;
            value[0] = (uint8_t) (float_type->x0 * 127.0f);
            value[1] = (uint8_t) (float_type->y0 * 127.0f);
            value[2] = (uint8_t) (float_type->x1 * 127.0f);
            value[3] = (uint8_t) (float_type->y1 * 127.0f);
            break;
        default:
            break;
        }
    }
}

static void
nanoemMotionReadAnnotationsNMD(kh_annotation_t **annotations_ptr, Nanoem__Motion__Annotation *const *annotation_messages, size_t num_annotation_messages, nanoem_status_t *status)
{
    const Nanoem__Motion__Annotation *annotation_message;
    kh_annotation_t *annotations;
    khiter_t it;
    size_t i;
    char *key_ptr, *value_ptr;
    int ret;
    if (num_annotation_messages > 0) {
        if (*annotations_ptr) {
            annotations = *annotations_ptr;
        }
        else {
            annotations = *annotations_ptr = kh_init_annotation();
        }
        if (nanoem_is_not_null(annotations)) {
            for (i = 0; i < num_annotation_messages; i++) {
                annotation_message = annotation_messages[i];
                key_ptr = nanoemUtilCloneString(annotation_message->name, status);
                it = kh_put_annotation(annotations, key_ptr, &ret);
                if (ret > 0) {
                    kh_value(annotations, it) = nanoemUtilCloneString(annotation_message->value, status);
                }
                else if (ret == 0) {
                    value_ptr = kh_value(annotations, it);
                    kh_value(annotations, it) = nanoemUtilCloneString(annotation_message->value, status);
                    nanoem_free(value_ptr);
                }
                else {
                    nanoem_free(key_ptr);
                    break;
                }
            }
        }
    }
}

NANOEM_DECL_INLINE static void
nanoemMotionReadKeyframeCommonNMD(nanoem_motion_keyframe_object_t *dst, const Nanoem__Motion__KeyframeCommon *src, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    dst->frame_index = ((nanoem_frame_index_t) src->frame_index) + offset;
    dst->is_selected = src->has_is_selected ? src->is_selected : nanoem_false;
    nanoemMotionSetMaxFrameIndex(dst->parent_motion, dst);
    nanoemMotionReadAnnotationsNMD(&dst->annotations, src->annotations, src->n_annotations, status);
}

NANOEM_DECL_INLINE static void
nanoemMutableMotionWriteVector3NMD(Nanoem__Motion__Vector3 *dst, const nanoem_f128_t *src)
{
    dst->x = src->values[0];
    dst->y = src->values[1];
    dst->z = src->values[2];
}

NANOEM_DECL_INLINE static void
nanoemMutableMotionWriteVector4NMD(Nanoem__Motion__Vector4 *dst, const nanoem_f128_t *src)
{
    dst->x = src->values[0];
    dst->y = src->values[1];
    dst->z = src->values[2];
    dst->w = src->values[3];
}

static void
nanoemMutableMotionWriteAnnotationsNMD(Nanoem__Motion__Annotation ***annotation_messages_ptr, size_t *num_annotation_messages, const kh_annotation_t *annotations, nanoem_status_t *status)
{
    Nanoem__Motion__Annotation **annotation_messages, *annotation_message;
    nanoem_rsize_t i = 0;
    khiter_t it, end;
    if (nanoem_is_not_null(annotations) && kh_size(annotations) > 0) {
        *num_annotation_messages = kh_size(annotations);
        annotation_messages = *annotation_messages_ptr = (Nanoem__Motion__Annotation **) nanoem_calloc(*num_annotation_messages, sizeof(annotation_messages), status);
        if (nanoem_is_not_null(annotation_messages)) {
            for (it = 0, end = kh_end(annotations); it != end; it++) {
                if (kh_exist(annotations, it)) {
                    annotation_message = annotation_messages[i++] = (Nanoem__Motion__Annotation *) nanoem_calloc(1, sizeof(*annotation_message), status);
                    if (nanoem_is_not_null(annotation_message)) {
                        nanoem__motion__annotation__init(annotation_message);
                        annotation_message->name = nanoemUtilCloneString(kh_key(annotations, it), status);
                        if (nanoem_is_null(annotation_message->name)) {
                            *num_annotation_messages = i;
                            return;
                        }
                        annotation_message->value = nanoemUtilCloneString(kh_value(annotations, it), status);
                        if (nanoem_is_null(annotation_message->value)) {
                            *num_annotation_messages = i;
                            return;
                        }
                    }
                    else {
                        *num_annotation_messages = i;
                        return;
                    }
                }
            }
            nanoem_status_ptr_assign_succeeded(status);
        }
    }
}

NANOEM_DECL_INLINE static void
nanoemMutableMotionWriteKeyframeCommonNMD(Nanoem__Motion__KeyframeCommon *dst, const nanoem_motion_keyframe_object_t *src, nanoem_status_t *status)
{
    dst->frame_index = (uint64_t) src->frame_index;
    dst->is_selected = src->is_selected;
    dst->has_is_selected = 1;
    nanoemMutableMotionWriteAnnotationsNMD(&dst->annotations, &dst->n_annotations, src->annotations, status);
}

NANOEM_DECL_INLINE static void
nanoemMutableMotionCreateVector3NMD(Nanoem__Motion__Vector3 **dst, const nanoem_f128_t *src, nanoem_status_t *status)
{
    Nanoem__Motion__Vector3 *ptr;
    ptr = *dst = (Nanoem__Motion__Vector3 *) nanoem_calloc(1, sizeof(*ptr), status);
    if (nanoem_is_not_null(ptr)) {
        nanoem__motion__vector3__init(ptr);
        nanoemMutableMotionWriteVector3NMD(ptr, src);
    }
}

NANOEM_DECL_INLINE static void
nanoemMutableMotionCreateVector4NMD(Nanoem__Motion__Vector4 **dst, const nanoem_f128_t *src, nanoem_status_t *status)
{
    Nanoem__Motion__Vector4 *ptr;
    ptr = *dst = (Nanoem__Motion__Vector4 *) nanoem_calloc(1, sizeof(*ptr), status);
    if (nanoem_is_not_null(ptr)) {
        nanoem__motion__vector4__init(ptr);
        nanoemMutableMotionWriteVector4NMD(ptr, src);
    }
}

NANOEM_DECL_INLINE static void
nanoemMutableMotionCreateKeyframeCommonNMD(Nanoem__Motion__KeyframeCommon **dst, const nanoem_motion_keyframe_object_t *src, nanoem_status_t *status)
{
    Nanoem__Motion__KeyframeCommon *ptr;
    ptr = *dst = (Nanoem__Motion__KeyframeCommon *) nanoem_calloc(1, sizeof(*ptr), status);
    if (nanoem_is_not_null(ptr)) {
        nanoem__motion__keyframe_common__init(ptr);
        nanoemMutableMotionWriteKeyframeCommonNMD(ptr, src, status);
    }
}

NANOEM_DECL_INLINE static void
nanoemMutableMotionCreateInterpolationNMD(Nanoem__Motion__InterpolationUnit **dst, const nanoem_u8_t *src, nanoem_status_t *status)
{
    Nanoem__Motion__InterpolationUnit *ptr;
    ptr = *dst = (Nanoem__Motion__InterpolationUnit *) nanoem_calloc(1, sizeof(*ptr), status);
    if (nanoem_is_not_null(ptr)) {
        nanoem__motion__interpolation_unit__init(ptr);
        ptr->integer_type = (Nanoem__Motion__IntegerInterpolation *) nanoem_calloc(1, sizeof(*ptr->integer_type), status);
        ptr->unit_case = NANOEM__MOTION__INTERPOLATION_UNIT__UNIT_INTEGER_TYPE;
        if (nanoem_is_not_null(ptr->float_type)) {
            nanoem__motion__integer_interpolation__init(ptr->integer_type);
            ptr->integer_type->x0 = src[0];
            ptr->integer_type->y0 = src[1];
            ptr->integer_type->x1 = src[2];
            ptr->integer_type->y1 = src[3];
        }
    }
}

static void
nanoemMotionReadEffectParametersNMD(nanoem_motion_effect_parameter_t *effect_parameter, const Nanoem__Motion__EffectParameter *effect_parameter_message)
{
    effect_parameter->parameter_id = (nanoem_motion_track_index_t) effect_parameter_message->track_index;
    switch (effect_parameter_message->value_case) {
    case NANOEM__MOTION__EFFECT_PARAMETER__VALUE_B:
        effect_parameter->value_type = NANOEM_MOTION_EFFECT_PARAMETER_TYPE_BOOL;
        effect_parameter->value.i = effect_parameter_message->b;
        break;
    case NANOEM__MOTION__EFFECT_PARAMETER__VALUE_I:
        effect_parameter->value_type = NANOEM_MOTION_EFFECT_PARAMETER_TYPE_INT;
        effect_parameter->value.i = effect_parameter_message->i;
        break;
    case NANOEM__MOTION__EFFECT_PARAMETER__VALUE_F:
        effect_parameter->value_type = NANOEM_MOTION_EFFECT_PARAMETER_TYPE_FLOAT;
        effect_parameter->value.f = effect_parameter_message->f;
        break;
    case NANOEM__MOTION__EFFECT_PARAMETER__VALUE_FV:
        effect_parameter->value_type = NANOEM_MOTION_EFFECT_PARAMETER_TYPE_VECTOR4;
        effect_parameter->value.v.values[0] = effect_parameter_message->fv->x;
        effect_parameter->value.v.values[1] = effect_parameter_message->fv->y;
        effect_parameter->value.v.values[2] = effect_parameter_message->fv->z;
        effect_parameter->value.v.values[3] = effect_parameter_message->fv->w;
        break;
    default:
        break;
    }
}

static void
nanoemMotionReadAccessoryKeyframeEffectParametersNMD(nanoem_motion_accessory_keyframe_t *accessory_keyframe, const Nanoem__Motion__AccessoryKeyframe *accessory_keyframe_message, nanoem_status_t *status)
{
    nanoem_motion_effect_parameter_t *effect_parameter = NULL;
    nanoem_rsize_t num_effect_parameters, i;
    num_effect_parameters = accessory_keyframe_message->n_effect_parameters;
    if (num_effect_parameters > 0) {
        accessory_keyframe->num_effect_parameters = num_effect_parameters;
        accessory_keyframe->effect_parameters = (nanoem_motion_effect_parameter_t **) nanoem_calloc(num_effect_parameters, sizeof(*accessory_keyframe->effect_parameters), status);
        if (nanoem_is_not_null(accessory_keyframe->effect_parameters)) {
            for (i = 0; i < num_effect_parameters; i++) {
                effect_parameter = accessory_keyframe->effect_parameters[i] = nanoemMotionEffectParameterCreateFromAccessoryKeyframe(accessory_keyframe, status);
                if (nanoem_is_not_null(effect_parameter)) {
                    nanoemMotionReadEffectParametersNMD(effect_parameter, accessory_keyframe_message->effect_parameters[i]);
                }
                else {
                    accessory_keyframe->num_effect_parameters = i;
                    break;
                }
            }
        }
    }
}

static void
nanoemMotionReadAccessoryKeyframeBundleUnitNMD(nanoem_motion_t *motion, const Nanoem__Motion__AccessoryKeyframeBundle *accessory_keyframe_bundle_message, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    const Nanoem__Motion__AccessoryKeyframe *accessory_keyframe_message;
    const Nanoem__Motion__ModelBinding *outside_parent_message;
    nanoem_motion_accessory_keyframe_t *accessory_keyframe;
    nanoem_motion_outside_parent_t *outside_parent;
    nanoem_rsize_t num_accessory_keyframe, i;
    num_accessory_keyframe = motion->num_accessory_keyframes = accessory_keyframe_bundle_message->n_keyframes;
    if (num_accessory_keyframe > 0) {
        if (motion->accessory_keyframes) {
            nanoem_free(motion->accessory_keyframes);
        }
        motion->accessory_keyframes = (nanoem_motion_accessory_keyframe_t **) nanoem_calloc(num_accessory_keyframe, sizeof(*motion->accessory_keyframes), status);
        if (nanoem_is_not_null(motion->accessory_keyframes)) {
            for (i = 0; i < num_accessory_keyframe; i++) {
                accessory_keyframe_message = accessory_keyframe_bundle_message->keyframes[i];
                accessory_keyframe = motion->accessory_keyframes[i] = nanoemMotionAccessoryKeyframeCreate(motion, status);
                if (nanoem_is_not_null(accessory_keyframe)) {
                    nanoemMotionReadKeyframeCommonNMD(&accessory_keyframe->base, accessory_keyframe_message->common, offset, status);
                    if (nanoem_status_ptr_has_error(status)) {
                        motion->num_accessory_keyframes = i;
                        break;
                    }
                    nanoemMotionReadAccessoryKeyframeEffectParametersNMD(accessory_keyframe, accessory_keyframe_message, status);
                    if (nanoem_status_ptr_has_error(status)) {
                        motion->num_accessory_keyframes = i;
                        break;
                    }
                    accessory_keyframe->scale_factor = accessory_keyframe_message->scale_factor;
                    accessory_keyframe->opacity = accessory_keyframe_message->opacity;
                    accessory_keyframe->accessory_id = (nanoem_motion_track_index_t) accessory_keyframe_message->track_index;
                    accessory_keyframe->is_shadow_enabled = accessory_keyframe_message->has_is_shadow_enabled ? accessory_keyframe_message->is_shadow_enabled : nanoem_true;
                    accessory_keyframe->is_add_blending_enabled = accessory_keyframe_message->has_is_add_blending_enabled ? accessory_keyframe_message->is_add_blending_enabled : nanoem_false;
                    accessory_keyframe->visible = accessory_keyframe_message->has_visible ? accessory_keyframe_message->visible : nanoem_true;
                    outside_parent_message = accessory_keyframe_message->binding;
                    if (outside_parent_message) {
                        outside_parent = accessory_keyframe->outside_parent = nanoemMotionOutsideParentCreateFromAccessoryKeyframe(accessory_keyframe, status);
                        if (nanoem_is_not_null(outside_parent)) {
                            outside_parent->global_model_track_index = (nanoem_motion_track_index_t) outside_parent_message->global_object_track_index;
                            outside_parent->global_bone_track_index = (nanoem_motion_track_index_t) outside_parent_message->global_bone_track_index;
                        }
                        else {
                            break;
                        }
                    }
                    nanoemMotionReadVector3NMD(&accessory_keyframe->translation, accessory_keyframe_message->translation);
                    nanoemMotionReadVector4NMD(&accessory_keyframe->orientation, accessory_keyframe_message->orientation);
                }
                else {
                    motion->num_accessory_keyframes = i;
                    break;
                }
            }
        }
    }
}

static void
nanoemMotionReadBoneKeyframeBundleUnitNMD(nanoem_motion_t *motion, const Nanoem__Motion__BoneKeyframeBundle *bone_keyframe_bundle_message, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    const Nanoem__Motion__BoneKeyframe *bone_keyframe_message;
    const Nanoem__Motion__Track *track_message;
    const Nanoem__Motion__BoneKeyframeInterpolation *interpolation_message;
    nanoem_unicode_string_factory_t *factory = motion->factory;
    nanoem_motion_bone_keyframe_t *bone_keyframe;
    nanoem_rsize_t num_bone_keyframes, num_local_tracks, i;
    nanoem_motion_track_t track;
    int ret;
    num_bone_keyframes = motion->num_bone_keyframes = bone_keyframe_bundle_message->n_keyframes;
    if (num_bone_keyframes > 0) {
        if (motion->bone_keyframes) {
            nanoem_free(motion->bone_keyframes);
        }
        motion->bone_keyframes = (nanoem_motion_bone_keyframe_t **) nanoem_calloc(num_bone_keyframes, sizeof(*motion->bone_keyframes), status);
        if (nanoem_is_not_null(motion->bone_keyframes)) {
            num_local_tracks = bone_keyframe_bundle_message->n_local_tracks;
            for (i = 0; i < num_local_tracks; i++) {
                track_message = bone_keyframe_bundle_message->local_tracks[i];
                track.name = nanoemUnicodeStringFactoryCreateStringWithEncoding(factory, (const nanoem_u8_t *) track_message->name, nanoem_crt_strlen(track_message->name), NANOEM_CODEC_TYPE_UTF8, status);
                if (!nanoem_status_ptr_has_error(status)) {
                    track.factory = factory;
                    track.id = (nanoem_motion_track_index_t) track_message->index;
                    track.keyframes = kh_init_keyframe_map();
                    kh_put_motion_track_bundle(motion->local_bone_motion_track_bundle, track, &ret);
                    if (ret >= 0 && motion->local_bone_motion_track_allocated_id < track.id) {
                        motion->local_bone_motion_track_allocated_id = track.id;
                    }
                    else if (ret < 0) {
                        motion->num_bone_keyframes = i;
                        break;
                    }
                }
            }
            for (i = 0; i < num_bone_keyframes; i++) {
                bone_keyframe_message = bone_keyframe_bundle_message->keyframes[i];
                bone_keyframe = motion->bone_keyframes[i] = nanoemMotionBoneKeyframeCreate(motion, status);
                if (nanoem_is_not_null(bone_keyframe)) {
                    nanoemMotionReadKeyframeCommonNMD(&bone_keyframe->base, bone_keyframe_message->common, offset, status);
                    if (nanoem_status_ptr_has_error(status)) {
                        motion->num_bone_keyframes = i;
                        break;
                    }
                    nanoemMotionReadVector3NMD(&bone_keyframe->translation, bone_keyframe_message->local_translation);
                    nanoemMotionReadVector4NMD(&bone_keyframe->orientation, bone_keyframe_message->local_orientation);
                    interpolation_message = bone_keyframe_message->interpolation;
                    if (interpolation_message) {
                        nanoemMotionReadInterpolation(bone_keyframe->interplation[0].u.values, interpolation_message->x);
                        nanoemMotionReadInterpolation(bone_keyframe->interplation[1].u.values, interpolation_message->y);
                        nanoemMotionReadInterpolation(bone_keyframe->interplation[2].u.values, interpolation_message->z);
                        nanoemMotionReadInterpolation(bone_keyframe->interplation[3].u.values, interpolation_message->orientation);
                    }
                    if (bone_keyframe_message->has_stage_index) {
                        bone_keyframe->stage_index = bone_keyframe_message->stage_index;
                    }
                    if (bone_keyframe_message->has_is_physics_simulation_enabled) {
                        bone_keyframe->is_physics_simulation_enabled = bone_keyframe_message->is_physics_simulation_enabled;
                    }
                    bone_keyframe->bone_id = (nanoem_motion_track_index_t) bone_keyframe_message->track_index;
                    nanoemMotionTrackBundleAddKeyframe(motion->local_bone_motion_track_bundle,
                                                       (nanoem_motion_keyframe_object_t *) bone_keyframe,
                                                       bone_keyframe->base.frame_index,
                                                       nanoemMotionTrackBundleResolveName(motion->local_bone_motion_track_bundle, bone_keyframe->bone_id),
                                                       factory,
                                                       &ret);
                    if (ret < 0) {
                        motion->num_bone_keyframes = i;
                        break;
                    }
                }
                else {
                    motion->num_bone_keyframes = i;
                    break;
                }
            }
        }
    }
}

static void
nanoemMotionReadCameraKeyframeBundleUnitNMD(nanoem_motion_t *motion, const Nanoem__Motion__CameraKeyframeBundle *camera_keyframe_bundle_message, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    const Nanoem__Motion__CameraKeyframe *camera_keyframe_message;
    const Nanoem__Motion__CameraKeyframeInterpolation *interpolation_message;
    nanoem_motion_camera_keyframe_t *camera_keyframe;
    nanoem_rsize_t num_camera_keyframes, i;
    num_camera_keyframes = motion->num_camera_keyframes = camera_keyframe_bundle_message->n_keyframes;
    if (num_camera_keyframes > 0) {
        if (motion->camera_keyframes) {
            nanoem_free(motion->camera_keyframes);
        }
        motion->camera_keyframes = (nanoem_motion_camera_keyframe_t **) nanoem_calloc(num_camera_keyframes, sizeof(*motion->camera_keyframes), status);
        if (nanoem_is_not_null(motion->camera_keyframes)) {
            for (i = 0; i < num_camera_keyframes; i++) {
                camera_keyframe_message = camera_keyframe_bundle_message->keyframes[i];
                camera_keyframe = motion->camera_keyframes[i] = nanoemMotionCameraKeyframeCreate(motion, status);
                if (nanoem_is_not_null(camera_keyframe)) {
                    nanoemMotionReadKeyframeCommonNMD(&camera_keyframe->base, camera_keyframe_message->common, offset, status);
                    if (nanoem_status_ptr_has_error(status)) {
                        motion->num_camera_keyframes = i;
                        break;
                    }
                    nanoemMotionReadVector3NMD(&camera_keyframe->angle, camera_keyframe_message->angle);
                    nanoemMotionReadVector3NMD(&camera_keyframe->look_at, camera_keyframe_message->look_at);
                    interpolation_message = camera_keyframe_message->interpolation;
                    if (interpolation_message) {
                        nanoemMotionReadInterpolation(camera_keyframe->interplation[0].u.values, interpolation_message->x);
                        nanoemMotionReadInterpolation(camera_keyframe->interplation[1].u.values, interpolation_message->y);
                        nanoemMotionReadInterpolation(camera_keyframe->interplation[2].u.values, interpolation_message->z);
                        nanoemMotionReadInterpolation(camera_keyframe->interplation[3].u.values, interpolation_message->angle);
                        nanoemMotionReadInterpolation(camera_keyframe->interplation[4].u.values, interpolation_message->fov);
                        nanoemMotionReadInterpolation(camera_keyframe->interplation[5].u.values, interpolation_message->distance);
                    }
                    camera_keyframe->distance = camera_keyframe_message->distance;
                    camera_keyframe->fov = (int) camera_keyframe_message->fov;
                    camera_keyframe->is_perspective_view = camera_keyframe_message->has_is_perspective_view_enabled ? camera_keyframe_message->is_perspective_view_enabled : 1;
                    if (camera_keyframe_message->has_stage_index) {
                        camera_keyframe->stage_index = camera_keyframe_message->stage_index;
                    }
                }
                else {
                    motion->num_camera_keyframes = i;
                    break;
                }
            }
        }
    }
}

static void
nanoemMotionReadLightKeyframeBundleUnitNMD(nanoem_motion_t *motion, const Nanoem__Motion__LightKeyframeBundle *light_keyframe_bundle_message, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    const Nanoem__Motion__LightKeyframe *light_keyframe_message;
    nanoem_motion_light_keyframe_t *light_keyframe;
    nanoem_rsize_t num_light_keyframes, i;
    num_light_keyframes = motion->num_light_keyframes = light_keyframe_bundle_message->n_keyframes;
    if (num_light_keyframes > 0) {
        if (motion->light_keyframes) {
            nanoem_free(motion->light_keyframes);
        }
        motion->light_keyframes = (nanoem_motion_light_keyframe_t **) nanoem_calloc(num_light_keyframes, sizeof(*motion->light_keyframes), status);
        if (nanoem_is_not_null(motion->light_keyframes)) {
            for (i = 0; i < num_light_keyframes; i++) {
                light_keyframe_message = light_keyframe_bundle_message->keyframes[i];
                light_keyframe = motion->light_keyframes[i] = nanoemMotionLightKeyframeCreate(motion, status);
                if (nanoem_is_not_null(light_keyframe)) {
                    nanoemMotionReadKeyframeCommonNMD(&light_keyframe->base, light_keyframe_message->common, offset, status);
                    if (nanoem_status_ptr_has_error(status)) {
                        motion->num_light_keyframes = i;
                        break;
                    }
                    nanoemMotionReadVector3NMD(&light_keyframe->color, light_keyframe_message->color);
                    nanoemMotionReadVector3NMD(&light_keyframe->direction, light_keyframe_message->direction);
                }
                else {
                    motion->num_light_keyframes = i;
                    break;
                }
            }
        }
    }
}

static void
nanoemMotionReadModelKeyframeConstraintStatesNMD(nanoem_motion_model_keyframe_t *model_keyframe, const Nanoem__Motion__ModelKeyframe *model_keyframe_message, nanoem_status_t *status)
{
    const Nanoem__Motion__ConstraintState *constraint_state_message;
    nanoem_motion_model_keyframe_constraint_state_t *constraint_state;
    nanoem_rsize_t num_model_constraint_states, i;
    num_model_constraint_states = model_keyframe_message->n_constraint_states;
    if (num_model_constraint_states > 0) {
        model_keyframe->num_constraint_states = num_model_constraint_states;
        model_keyframe->constraint_states = (nanoem_motion_model_keyframe_constraint_state_t **) nanoem_calloc(model_keyframe_message->n_constraint_states, sizeof(*model_keyframe->constraint_states), status);
        if (nanoem_is_not_null(model_keyframe->constraint_states)) {
            for (i = 0; i < num_model_constraint_states; i++) {
                constraint_state = model_keyframe->constraint_states[i] = (nanoem_motion_model_keyframe_constraint_state_t *) nanoem_calloc(1, sizeof(*constraint_state), status);
                if (nanoem_is_not_null(constraint_state)) {
                    constraint_state_message = model_keyframe_message->constraint_states[i];
                    constraint_state->bone_id = (nanoem_motion_track_index_t) constraint_state_message->track_index;
                    constraint_state->enabled = constraint_state_message->enabled;
                    constraint_state->parent_keyframe = model_keyframe;
                }
                else {
                    model_keyframe->num_constraint_states = i;
                    break;
                }
            }
        }
    }
}

static void
nanoemMotionReadModelKeyframeEffectParametersNMD(nanoem_motion_model_keyframe_t *model_keyframe, const Nanoem__Motion__ModelKeyframe *model_keyframe_message, nanoem_status_t *status)
{
    nanoem_motion_effect_parameter_t *effect_parameter = NULL;
    nanoem_rsize_t num_effect_parameters, i;
    num_effect_parameters = model_keyframe_message->n_effect_parameters;
    if (num_effect_parameters > 0) {
        model_keyframe->num_effect_parameters = num_effect_parameters;
        model_keyframe->effect_parameters = (nanoem_motion_effect_parameter_t **) nanoem_calloc(num_effect_parameters, sizeof(*model_keyframe->effect_parameters), status);
        if (nanoem_is_not_null(model_keyframe->effect_parameters)) {
            for (i = 0; i < num_effect_parameters; i++) {
                effect_parameter = model_keyframe->effect_parameters[i] = nanoemMotionEffectParameterCreateFromModelKeyframe(model_keyframe, status);
                if (nanoem_is_not_null(effect_parameter)) {
                    nanoemMotionReadEffectParametersNMD(effect_parameter, model_keyframe_message->effect_parameters[i]);
                }
                else {
                    model_keyframe->num_effect_parameters = i;
                    break;
                }
            }
        }
    }
}

static void
nanoemMotionReadModelKeyframeModelBindingsNMD(nanoem_motion_model_keyframe_t *model_keyframe, const Nanoem__Motion__ModelKeyframe *model_keyframe_message, nanoem_status_t *status)
{
    const Nanoem__Motion__ModelBinding *outside_parent_message;
    nanoem_motion_outside_parent_t *binding = NULL;
    nanoem_rsize_t num_bindings, i;
    num_bindings = model_keyframe_message->n_bindings;
    if (num_bindings > 0) {
        model_keyframe->num_outside_parents = num_bindings;
        model_keyframe->outside_parents = (nanoem_motion_outside_parent_t **) nanoem_calloc(num_bindings, sizeof(*model_keyframe->outside_parents), status);
        if (nanoem_is_not_null(model_keyframe->outside_parents)) {
            for (i = 0; i < num_bindings; i++) {
                binding = model_keyframe->outside_parents[i] = nanoemMotionOutsideParentCreateFromModelKeyframe(model_keyframe, status);
                if (nanoem_is_not_null(binding)) {
                    outside_parent_message = model_keyframe_message->bindings[i];
                    if (outside_parent_message->has_local_bone_track_index) {
                        binding->local_bone_track_index = (nanoem_motion_track_index_t) outside_parent_message->local_bone_track_index;
                    }
                    binding->global_bone_track_index = (nanoem_motion_track_index_t) outside_parent_message->global_bone_track_index;
                    binding->global_model_track_index = (nanoem_motion_track_index_t) outside_parent_message->global_object_track_index;
                }
                else {
                    model_keyframe->num_outside_parents = i;
                    break;
                }
            }
        }
    }
}

static void
nanoemMotionReadModelKeyframeEdgeNMD(nanoem_motion_model_keyframe_t *model_keyframe, const Nanoem__Motion__ModelKeyframe *model_keyframe_message)
{
    const Nanoem__Motion__Edge *edge_message = model_keyframe_message->edge;
    const Nanoem__Motion__Vector4 *edge_color_message;
    if (edge_message) {
        model_keyframe->edge_scale_factor = edge_message->scale_factor;
        edge_color_message = edge_message->color;
        if (edge_color_message) {
            nanoemMotionReadVector4NMD(&model_keyframe->edge_color, edge_color_message);
        }
        model_keyframe->has_edge_option = nanoem_true;
    }
}

static void
nanoemMotionReadModelKeyframeBundleUnitNMD(nanoem_motion_t *motion, const Nanoem__Motion__ModelKeyframeBundle *model_keyframe_bundle_message, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    const Nanoem__Motion__ModelKeyframe *model_keyframe_message;
    nanoem_motion_model_keyframe_t *model_keyframe;
    nanoem_rsize_t num_model_keyframes, i;
    num_model_keyframes = motion->num_model_keyframes = model_keyframe_bundle_message->n_keyframes;
    if (num_model_keyframes > 0) {
        if (motion->model_keyframes) {
            nanoem_free(motion->model_keyframes);
        }
        motion->model_keyframes = (nanoem_motion_model_keyframe_t **) nanoem_calloc(num_model_keyframes, sizeof(*motion->model_keyframes), status);
        if (nanoem_is_not_null(motion->model_keyframes)) {
            for (i = 0; i < num_model_keyframes; i++) {
                model_keyframe_message = model_keyframe_bundle_message->keyframes[i];
                model_keyframe = motion->model_keyframes[i] = nanoemMotionModelKeyframeCreate(motion, status);
                if (nanoem_is_not_null(model_keyframe)) {
                    nanoemMotionReadKeyframeCommonNMD(&model_keyframe->base, model_keyframe_message->common, offset, status);
                    if (nanoem_status_ptr_has_error(status)) {
                        motion->num_model_keyframes = i;
                        break;
                    }
                    nanoemMotionReadModelKeyframeConstraintStatesNMD(model_keyframe, model_keyframe_message, status);
                    if (nanoem_status_ptr_has_error(status)) {
                        motion->num_model_keyframes = i;
                        break;
                    }
                    nanoemMotionReadModelKeyframeEffectParametersNMD(model_keyframe, model_keyframe_message, status);
                    if (nanoem_status_ptr_has_error(status)) {
                        motion->num_model_keyframes = i;
                        break;
                    }
                    nanoemMotionReadModelKeyframeModelBindingsNMD(model_keyframe, model_keyframe_message, status);
                    if (nanoem_status_ptr_has_error(status)) {
                        motion->num_model_keyframes = i;
                        break;
                    }
                    model_keyframe->visible = model_keyframe_message->visible;
                    if (model_keyframe_message->has_is_add_blending_enabled) {
                        model_keyframe->is_add_blending_enabled = model_keyframe_message->is_add_blending_enabled;
                    }
                    if (model_keyframe_message->has_is_physics_simulation_enabled) {
                        model_keyframe->is_physics_simulation_enabled = model_keyframe_message->is_physics_simulation_enabled;
                    }
                    nanoemMotionReadModelKeyframeEdgeNMD(model_keyframe, model_keyframe_message);
                }
                else {
                    motion->num_model_keyframes = i;
                    break;
                }
            }
        }
    }
}

static void
nanoemMotionReadMorphKeyframeBundleUnitNMD(nanoem_motion_t *motion, const Nanoem__Motion__MorphKeyframeBundle *morph_keyframe_bundle_message, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    const Nanoem__Motion__MorphKeyframe *morph_keyframe_message;
    const Nanoem__Motion__Track *track_message;
    nanoem_unicode_string_factory_t *factory = motion->factory;
    nanoem_motion_morph_keyframe_t *morph_keyframe;
    nanoem_rsize_t num_morph_keyframes, num_local_tracks, i;
    nanoem_motion_track_t track;
    int ret;
    num_morph_keyframes = motion->num_morph_keyframes = morph_keyframe_bundle_message->n_keyframes;
    if (num_morph_keyframes > 0) {
        if (motion->morph_keyframes) {
            nanoem_free(motion->morph_keyframes);
        }
        motion->morph_keyframes = (nanoem_motion_morph_keyframe_t **) nanoem_calloc(num_morph_keyframes, sizeof(*motion->morph_keyframes), status);
        if (nanoem_is_not_null(motion->morph_keyframes)) {
            num_local_tracks = morph_keyframe_bundle_message->n_local_tracks;
            for (i = 0; i < num_local_tracks; i++) {
                track_message = morph_keyframe_bundle_message->local_tracks[i];
                track.name = nanoemUnicodeStringFactoryCreateStringWithEncoding(factory, (const nanoem_u8_t *) track_message->name, nanoem_crt_strlen(track_message->name), NANOEM_CODEC_TYPE_UTF8, status);
                if (!nanoem_status_ptr_has_error(status)) {
                    track.factory = factory;
                    track.id = (nanoem_motion_track_index_t) track_message->index;
                    track.keyframes = kh_init_keyframe_map();
                    kh_put_motion_track_bundle(motion->local_morph_motion_track_bundle, track, &ret);
                    if (ret >= 0 && motion->local_morph_motion_track_allocated_id < track.id) {
                        motion->local_morph_motion_track_allocated_id = track.id;
                    }
                    else if (ret < 0) {
                        motion->num_morph_keyframes = i;
                        break;
                    }
                }
            }
            for (i = 0; i < num_morph_keyframes; i++) {
                morph_keyframe_message = morph_keyframe_bundle_message->keyframes[i];
                morph_keyframe = motion->morph_keyframes[i] = nanoemMotionMorphKeyframeCreate(motion, status);
                if (nanoem_is_not_null(morph_keyframe)) {
                    nanoemMotionReadKeyframeCommonNMD(&morph_keyframe->base, morph_keyframe_message->common, offset, status);
                    if (nanoem_status_ptr_has_error(status)) {
                        motion->num_morph_keyframes = i;
                        break;
                    }
                    morph_keyframe->morph_id = (nanoem_motion_track_index_t) morph_keyframe_message->track_index;
                    morph_keyframe->weight = morph_keyframe_message->weight;
                    nanoemMotionTrackBundleAddKeyframe(motion->local_morph_motion_track_bundle,
                                                       (nanoem_motion_keyframe_object_t *) morph_keyframe,
                                                       morph_keyframe->base.frame_index,
                                                       nanoemMotionTrackBundleResolveName(motion->local_morph_motion_track_bundle, morph_keyframe->morph_id),
                                                       factory,
                                                       &ret);
                    if (ret < 0) {
                        motion->num_morph_keyframes = i;
                        break;
                    }
                }
                else {
                    motion->num_morph_keyframes = i;
                    break;
                }
            }
        }
    }
}

static void
nanoemMotionReadSelfShadowKeyframeBundleUnitNMD(nanoem_motion_t *motion, const Nanoem__Motion__SelfShadowKeyframeBundle *self_shadow_keyframe_bundle_message, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    const Nanoem__Motion__SelfShadowKeyframe *self_shadow_keyframe_message;
    nanoem_motion_self_shadow_keyframe_t *self_shadow_keyframe;
    nanoem_rsize_t num_self_shadow_keyframes, i;
    num_self_shadow_keyframes = motion->num_self_shadow_keyframes = self_shadow_keyframe_bundle_message->n_keyframes;
    if (num_self_shadow_keyframes > 0) {
        if (motion->self_shadow_keyframes) {
            nanoem_free(motion->self_shadow_keyframes);
        }
        motion->self_shadow_keyframes = (nanoem_motion_self_shadow_keyframe_t **) nanoem_calloc(num_self_shadow_keyframes, sizeof(*motion->self_shadow_keyframes), status);
        if (nanoem_is_not_null(motion->self_shadow_keyframes)) {
            for (i = 0; i < num_self_shadow_keyframes; i++) {
                self_shadow_keyframe_message = self_shadow_keyframe_bundle_message->keyframes[i];
                self_shadow_keyframe = motion->self_shadow_keyframes[i] = nanoemMotionSelfShadowKeyframeCreate(motion, status);
                nanoemMotionReadKeyframeCommonNMD(&self_shadow_keyframe->base, self_shadow_keyframe_message->common, offset, status);
                if (nanoem_status_ptr_has_error(status)) {
                    motion->num_self_shadow_keyframes = i;
                    break;
                }
                if (nanoem_is_not_null(self_shadow_keyframe)) {
                    self_shadow_keyframe->distance = self_shadow_keyframe_message->distance;
                    self_shadow_keyframe->mode = self_shadow_keyframe_message->mode;
                }
                else {
                    motion->num_self_shadow_keyframes = i;
                    break;
                }
            }
        }
    }
}

nanoem_bool_t APIENTRY
nanoemMotionLoadFromBufferNMD(nanoem_motion_t *motion, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status)
{
    Nanoem__Motion__Motion *motion_message;
    Nanoem__Motion__KeyframeBundleUnit *keyframe_bundle;
    Nanoem__Motion__Track *track_message;
    nanoem_unicode_string_factory_t *factory;
    nanoem_motion_track_t track;
    nanoem_rsize_t num_keyframe_bundles, num_global_tracks, i;
    const char *target_object_name;
    int ret;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(buffer)) {
        nanoem_status_ptr_assign_succeeded(status);
        factory = motion->factory;
        motion_message = nanoem__motion__motion__unpack(&__nanoem_protobuf_allocator, buffer->length, buffer->data);
        if (nanoem_is_not_null(motion_message)) {
            num_global_tracks = motion_message->n_global_tracks;
            if (!motion->global_motion_track_bundle) {
                motion->global_motion_track_bundle = kh_init_motion_track_bundle();
            }
            nanoemMotionReadAnnotationsNMD(&motion->annotations, motion_message->annotations, motion_message->n_annotations, status);
            if (!nanoem_status_ptr_has_error(status)) {
                target_object_name = motion_message->main_object_name;
                motion->preferred_fps = motion_message->preferred_fps;
                motion->target_model_name = nanoemUnicodeStringFactoryCreateStringWithEncoding(factory, (const nanoem_u8_t *) target_object_name, target_object_name ? nanoem_crt_strlen(target_object_name) : 0, NANOEM_CODEC_TYPE_UTF8, status);
                for (i = 0; i < num_global_tracks; i++) {
                    track_message = motion_message->global_tracks[i];
                    track.name = nanoemUnicodeStringFactoryCreateStringWithEncoding(factory, (const nanoem_u8_t *) track_message->name, nanoem_crt_strlen(track_message->name), NANOEM_CODEC_TYPE_UTF8, status);
                    if (!nanoem_status_ptr_has_error(status)) {
                        track.factory = factory;
                        track.id = (nanoem_motion_track_index_t) track_message->index;
                        track.keyframes = kh_init_keyframe_map();
                        kh_put_motion_track_bundle(motion->global_motion_track_bundle, track, &ret);
                        if (motion->global_motion_track_allocated_id < track.id) {
                            motion->global_motion_track_allocated_id = track.id;
                        }
                    }
                }
                num_keyframe_bundles = motion_message->n_keyframe_bundles;
                for (i = 0; i < num_keyframe_bundles; i++) {
                    keyframe_bundle = motion_message->keyframe_bundles[i];
                    switch (keyframe_bundle->unit_case) {
                    case NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_BONE:
                        if (motion->num_bone_keyframes == 0) {
                            nanoemMotionReadBoneKeyframeBundleUnitNMD(motion, keyframe_bundle->bone, offset, status);
                        }
                        break;
                    case NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_MODEL:
                        if (motion->num_model_keyframes == 0) {
                            nanoemMotionReadModelKeyframeBundleUnitNMD(motion, keyframe_bundle->model, offset, status);
                        }
                        break;
                    case NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_MORPH:
                        if (motion->num_morph_keyframes == 0) {
                            nanoemMotionReadMorphKeyframeBundleUnitNMD(motion, keyframe_bundle->morph, offset, status);
                        }
                        break;
                    case NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_ACCESSORY:
                        if (motion->num_accessory_keyframes == 0) {
                            nanoemMotionReadAccessoryKeyframeBundleUnitNMD(motion, keyframe_bundle->accessory, offset, status);
                        }
                        break;
                    case NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_CAMERA:
                        if (motion->num_camera_keyframes == 0) {
                            nanoemMotionReadCameraKeyframeBundleUnitNMD(motion, keyframe_bundle->camera, offset, status);
                        }
                        break;
                    case NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_LIGHT:
                        if (motion->num_light_keyframes == 0) {
                            nanoemMotionReadLightKeyframeBundleUnitNMD(motion, keyframe_bundle->light, offset, status);
                        }
                        break;
                    case NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_SELF_SHADOW:
                        if (motion->num_self_shadow_keyframes == 0) {
                            nanoemMotionReadSelfShadowKeyframeBundleUnitNMD(motion, keyframe_bundle->self_shadow, offset, status);
                        }
                        break;
                    case NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT__NOT_SET:
                    default:
                        break;
                    }
                    if (nanoem_status_ptr_has_error(status)) {
                        break;
                    }
                }
            }
            nanoem__motion__motion__free_unpacked(motion_message, &__nanoem_protobuf_allocator);
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_INVALID_SIGNATURE);
        }
    }
    return !nanoem_status_ptr_has_error(status);
}

static void
nanoemMutableMotionWriteEffectParameter(Nanoem__Motion__EffectParameter *effect_parameter_message, const nanoem_motion_effect_parameter_t *effect_parameter, nanoem_status_t *status)
{
    nanoem__motion__effect_parameter__init(effect_parameter_message);
    effect_parameter_message->track_index = (uint64_t) effect_parameter->parameter_id;
    switch (effect_parameter->value_type) {
    case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_BOOL:
        effect_parameter_message->value_case = NANOEM__MOTION__EFFECT_PARAMETER__VALUE_B;
        effect_parameter_message->b = effect_parameter->value.i;
        break;
    case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_INT:
        effect_parameter_message->value_case = NANOEM__MOTION__EFFECT_PARAMETER__VALUE_I;
        effect_parameter_message->i = effect_parameter->value.i;
        break;
    case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_FLOAT:
        effect_parameter_message->value_case = NANOEM__MOTION__EFFECT_PARAMETER__VALUE_F;
        effect_parameter_message->f = effect_parameter->value.f;
        break;
    case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_VECTOR4:
        effect_parameter_message->value_case = NANOEM__MOTION__EFFECT_PARAMETER__VALUE_FV;
        nanoemMutableMotionCreateVector4NMD(&effect_parameter_message->fv, &effect_parameter->value.v, status);
        break;
    default:
        effect_parameter_message->value_case = NANOEM__MOTION__EFFECT_PARAMETER__VALUE__NOT_SET;
        break;
    }
}

static void
nanoemMutableMotionWriteAccessoryKeyframeEffectParametersNMD(const nanoem_motion_accessory_keyframe_t *accessory_keyframe, Nanoem__Motion__AccessoryKeyframe *accessory_keyframe_message, nanoem_status_t *status)
{
    Nanoem__Motion__EffectParameter *effect_parameter_message;
    nanoem_rsize_t num_effect_parameters, i;
    num_effect_parameters = accessory_keyframe_message->n_effect_parameters = accessory_keyframe->num_effect_parameters;
    if (num_effect_parameters > 0) {
        accessory_keyframe_message->effect_parameters = (Nanoem__Motion__EffectParameter **) nanoem_calloc(num_effect_parameters, sizeof(*accessory_keyframe_message->effect_parameters), status);
        if (nanoem_is_not_null(accessory_keyframe_message->effect_parameters)) {
            for (i = 0; i < num_effect_parameters; i++) {
                effect_parameter_message = accessory_keyframe_message->effect_parameters[i] = (Nanoem__Motion__EffectParameter *) nanoem_calloc(1, sizeof(*effect_parameter_message), status);
                if (nanoem_is_not_null(effect_parameter_message)) {
                    nanoemMutableMotionWriteEffectParameter(effect_parameter_message, accessory_keyframe->effect_parameters[i], status);
                }
                else {
                    accessory_keyframe_message->n_effect_parameters = i;
                    return;
                }
            }
            nanoem_status_ptr_assign_succeeded(status);
        }
    }
}

static void
nanoemMutableMotionWriteAccessoryKeyframeBundleUnitNMD(const nanoem_motion_t *motion, Nanoem__Motion__Motion *motion_message, nanoem_rsize_t *keyframe_bundle_index, nanoem_status_t *status)
{
    Nanoem__Motion__KeyframeBundleUnit *keyframe_unit_message;
    Nanoem__Motion__AccessoryKeyframeBundle *accessory_keyframe_bundle_message;
    Nanoem__Motion__AccessoryKeyframe *accessory_keyframe_message;
    Nanoem__Motion__ModelBinding *outside_parent_message;
    const nanoem_motion_accessory_keyframe_t *accessory_keyframe;
    const nanoem_motion_outside_parent_t *outside_parent;
    nanoem_rsize_t num_accessory_keyframes, i;
    if (motion->num_accessory_keyframes > 0) {
        keyframe_unit_message = motion_message->keyframe_bundles[*keyframe_bundle_index] = (Nanoem__Motion__KeyframeBundleUnit *) nanoem_calloc(1, sizeof(*keyframe_unit_message), status);
        *keyframe_bundle_index += 1;
        if (nanoem_is_not_null(keyframe_unit_message)) {
            nanoem__motion__keyframe_bundle_unit__init(keyframe_unit_message);
            keyframe_unit_message->unit_case = NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_ACCESSORY;
            accessory_keyframe_bundle_message = keyframe_unit_message->accessory = (Nanoem__Motion__AccessoryKeyframeBundle *) nanoem_calloc(1, sizeof(*accessory_keyframe_bundle_message), status);
            if (nanoem_is_not_null(accessory_keyframe_bundle_message)) {
                nanoem__motion__accessory_keyframe_bundle__init(accessory_keyframe_bundle_message);
                num_accessory_keyframes = accessory_keyframe_bundle_message->n_keyframes = motion->num_accessory_keyframes;
                accessory_keyframe_bundle_message->keyframes = (Nanoem__Motion__AccessoryKeyframe **) nanoem_calloc(num_accessory_keyframes, sizeof(*accessory_keyframe_bundle_message->keyframes), status);
                for (i = 0; i < num_accessory_keyframes; i++) {
                    accessory_keyframe = motion->accessory_keyframes[i];
                    accessory_keyframe_message = accessory_keyframe_bundle_message->keyframes[i] = (Nanoem__Motion__AccessoryKeyframe *) nanoem_calloc(1, sizeof(*accessory_keyframe_message), status);
                    if (nanoem_is_not_null(accessory_keyframe_message)) {
                        nanoem__motion__accessory_keyframe__init(accessory_keyframe_message);
                        nanoemMutableMotionCreateKeyframeCommonNMD(&accessory_keyframe_message->common, &accessory_keyframe->base, status);
                        if (nanoem_status_ptr_has_error(status)) {
                            num_accessory_keyframes = accessory_keyframe_bundle_message->n_keyframes = i;
                            return;
                        }
                        nanoemMutableMotionCreateVector3NMD(&accessory_keyframe_message->translation, &accessory_keyframe->translation, status);
                        nanoemMutableMotionCreateVector4NMD(&accessory_keyframe_message->orientation, &accessory_keyframe->orientation, status);
                        accessory_keyframe_message->scale_factor = accessory_keyframe->scale_factor;
                        accessory_keyframe_message->opacity = accessory_keyframe->opacity;
                        accessory_keyframe_message->is_shadow_enabled = accessory_keyframe->is_shadow_enabled;
                        accessory_keyframe_message->has_is_shadow_enabled = 1;
                        accessory_keyframe_message->is_add_blending_enabled = accessory_keyframe->is_add_blending_enabled;
                        accessory_keyframe_message->has_is_add_blending_enabled = 1;
                        accessory_keyframe_message->visible = accessory_keyframe->visible;
                        accessory_keyframe_message->has_visible = 1;
                        accessory_keyframe_message->track_index = (uint64_t) accessory_keyframe->accessory_id;
                        outside_parent = accessory_keyframe->outside_parent;
                        if (outside_parent) {
                            outside_parent_message = accessory_keyframe_message->binding = (Nanoem__Motion__ModelBinding *) nanoem_calloc(1, sizeof(*outside_parent_message), status);
                            if (nanoem_is_not_null(outside_parent_message)) {
                                nanoem__motion__model_binding__init(outside_parent_message);
                                outside_parent_message->global_object_track_index = (uint64_t) outside_parent->global_model_track_index;
                                outside_parent_message->global_bone_track_index = (uint64_t) outside_parent->global_bone_track_index;
                            }
                            else {
                                accessory_keyframe_bundle_message->n_keyframes = i;
                                return;
                            }
                        }
                        nanoemMutableMotionWriteAccessoryKeyframeEffectParametersNMD(accessory_keyframe, accessory_keyframe_message, status);
                        if (nanoem_status_ptr_has_error(status)) {
                            accessory_keyframe_bundle_message->n_keyframes = i;
                            return;
                        }
                    }
                    else {
                        accessory_keyframe_bundle_message->n_keyframes = i;
                        return;
                    }
                }
                nanoem_status_ptr_assign_succeeded(status);
            }
        }
    }
}

static void
nanoemMutableMotionWriteBoneTrackBundleNMD(const nanoem_motion_t *motion, Nanoem__Motion__BoneKeyframeBundle *bone_keyframe_bundle_message, nanoem_status_t *status)
{
    Nanoem__Motion__Track **track_bundle_message, *track_message;
    const nanoem_motion_track_t *track;
    kh_motion_track_bundle_t *track_bundle = motion->local_bone_motion_track_bundle;
    nanoem_unicode_string_factory_t *factory = motion->factory;
    nanoem_rsize_t length, i;
    nanoem_status_t st = NANOEM_STATUS_SUCCESS;
    khiter_t it, end;
    char *bytes;
    bone_keyframe_bundle_message->n_local_tracks = kh_size(track_bundle);
    track_bundle_message = bone_keyframe_bundle_message->local_tracks = (Nanoem__Motion__Track **) nanoem_calloc(kh_size(track_bundle), sizeof(*track_bundle_message), status);
    if (nanoem_is_not_null(track_bundle_message)) {
        i = 0;
        for (it = 0, end = kh_end(track_bundle); it != end; it++) {
            if (kh_exist(track_bundle, it)) {
                track_message = track_bundle_message[i++] = (Nanoem__Motion__Track *) nanoem_calloc(1, sizeof(*track_message), status);
                if (nanoem_is_not_null(track_message)) {
                    nanoem__motion__track__init(track_message);
                    track = &kh_key(track_bundle, it);
                    bytes = (char *) nanoemUnicodeStringFactoryGetByteArrayEncoding(factory, track->name, &length, NANOEM_CODEC_TYPE_UTF8, &st);
                    if (st == NANOEM_STATUS_SUCCESS) {
                        track_message->index = (uint64_t) track->id;
                        track_message->name = nanoemUtilCloneString(bytes, status);
                        nanoemUnicodeStringFactoryDestroyByteArray(factory, (nanoem_u8_t *)  bytes);
                    }
                    else {
                        return;
                    }
                }
                else {
                    return;
                }
            }
        }
        nanoem_status_ptr_assign_succeeded(status);
    }
}

static void
nanoemMutableMotionWriteBoneKeyframeBundleUnitNMD(const nanoem_motion_t *motion, Nanoem__Motion__Motion *motion_message, nanoem_rsize_t *keyframe_bundle_index, nanoem_status_t *status)
{
    Nanoem__Motion__KeyframeBundleUnit *keyframe_unit_message;
    Nanoem__Motion__BoneKeyframeBundle *bone_keyframe_bundle_message;
    Nanoem__Motion__BoneKeyframe *bone_keyframe_message;
    Nanoem__Motion__BoneKeyframeInterpolation *interpolation_message;
    const nanoem_motion_bone_keyframe_t *bone_keyframe;
    nanoem_rsize_t num_bone_keyframes, i;
    if (motion->num_bone_keyframes > 0) {
        keyframe_unit_message = motion_message->keyframe_bundles[*keyframe_bundle_index] = (Nanoem__Motion__KeyframeBundleUnit *) nanoem_calloc(1, sizeof(*keyframe_unit_message), status);
        *keyframe_bundle_index += 1;
        if (nanoem_is_not_null(keyframe_unit_message)) {
            nanoem__motion__keyframe_bundle_unit__init(keyframe_unit_message);
            keyframe_unit_message->unit_case = NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_BONE;
            bone_keyframe_bundle_message = keyframe_unit_message->bone = (Nanoem__Motion__BoneKeyframeBundle *) nanoem_calloc(1, sizeof(*bone_keyframe_bundle_message), status);
            if (nanoem_is_not_null(bone_keyframe_bundle_message)) {
                nanoem__motion__bone_keyframe_bundle__init(bone_keyframe_bundle_message);
                nanoemMutableMotionWriteBoneTrackBundleNMD(motion, bone_keyframe_bundle_message, status);
                if (*status == NANOEM_STATUS_SUCCESS) {
                    num_bone_keyframes = bone_keyframe_bundle_message->n_keyframes = motion->num_bone_keyframes;
                    bone_keyframe_bundle_message->keyframes = (Nanoem__Motion__BoneKeyframe **) nanoem_calloc(num_bone_keyframes, sizeof(*bone_keyframe_bundle_message->keyframes), status);
                    for (i = 0; i < num_bone_keyframes; i++) {
                        bone_keyframe = motion->bone_keyframes[i];
                        bone_keyframe_message = bone_keyframe_bundle_message->keyframes[i] = (Nanoem__Motion__BoneKeyframe *) nanoem_calloc(1, sizeof(*bone_keyframe_message), status);
                        if (nanoem_is_not_null(bone_keyframe_message)) {
                            nanoem__motion__bone_keyframe__init(bone_keyframe_message);
                            nanoemMutableMotionCreateKeyframeCommonNMD(&bone_keyframe_message->common, &bone_keyframe->base, status);
                            if (nanoem_status_ptr_has_error(status)) {
                                num_bone_keyframes = bone_keyframe_bundle_message->n_keyframes = i;
                                return;
                            }
                            nanoemMutableMotionCreateVector3NMD(&bone_keyframe_message->local_translation, &bone_keyframe->translation, status);
                            nanoemMutableMotionCreateVector4NMD(&bone_keyframe_message->local_orientation, &bone_keyframe->orientation, status);
                            interpolation_message = bone_keyframe_message->interpolation = (Nanoem__Motion__BoneKeyframeInterpolation *) nanoem_calloc(1, sizeof(*interpolation_message), status);
                            nanoem__motion__bone_keyframe_interpolation__init(interpolation_message);
                            if (!nanoemMotionBoneKeyframeIsLinearInterpolation(bone_keyframe, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X)) {
                                nanoemMutableMotionCreateInterpolationNMD(&interpolation_message->x, bone_keyframe->interplation[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X].u.values, status);
                            }
                            if (!nanoemMotionBoneKeyframeIsLinearInterpolation(bone_keyframe, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y)) {
                                nanoemMutableMotionCreateInterpolationNMD(&interpolation_message->y, bone_keyframe->interplation[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y].u.values, status);
                            }
                            if (!nanoemMotionBoneKeyframeIsLinearInterpolation(bone_keyframe, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z)) {
                                nanoemMutableMotionCreateInterpolationNMD(&interpolation_message->z, bone_keyframe->interplation[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z].u.values, status);
                            }
                            if (!nanoemMotionBoneKeyframeIsLinearInterpolation(bone_keyframe, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION)) {
                                nanoemMutableMotionCreateInterpolationNMD(&interpolation_message->orientation, bone_keyframe->interplation[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION].u.values, status);
                            }
                            bone_keyframe_message->track_index = (uint64_t) bone_keyframe->bone_id;
                            bone_keyframe_message->has_is_physics_simulation_enabled = 1;
                            bone_keyframe_message->is_physics_simulation_enabled = bone_keyframe->is_physics_simulation_enabled;
                            if (bone_keyframe->stage_index > 0) {
                                bone_keyframe_message->has_stage_index = 1;
                                bone_keyframe_message->stage_index = bone_keyframe->stage_index;
                            }
                        }
                        else {
                            bone_keyframe_bundle_message->n_keyframes = i;
                            return;
                        }
                    }
                    nanoem_status_ptr_assign_succeeded(status);
                }
            }
        }
    }
}

static void
nanoemMutableMotionWriteCameraKeyframeBundleUnitNMD(const nanoem_motion_t *motion, Nanoem__Motion__Motion *motion_message, nanoem_rsize_t *keyframe_bundle_index, nanoem_status_t *status)
{
    Nanoem__Motion__KeyframeBundleUnit *keyframe_unit_message;
    Nanoem__Motion__CameraKeyframeBundle *camera_keyframe_bundle_message;
    Nanoem__Motion__CameraKeyframe *camera_keyframe_message;
    Nanoem__Motion__CameraKeyframeInterpolation *interpolation_message;
    const nanoem_motion_camera_keyframe_t *camera_keyframe;
    nanoem_rsize_t num_camera_keyframes, i;
    if (motion->num_camera_keyframes > 0) {
        keyframe_unit_message = motion_message->keyframe_bundles[*keyframe_bundle_index] = (Nanoem__Motion__KeyframeBundleUnit *) nanoem_calloc(1, sizeof(*keyframe_unit_message), status);
        *keyframe_bundle_index += 1;
        if (nanoem_is_not_null(keyframe_unit_message)) {
            nanoem__motion__keyframe_bundle_unit__init(keyframe_unit_message);
            keyframe_unit_message->unit_case = NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_CAMERA;
            camera_keyframe_bundle_message = keyframe_unit_message->camera = (Nanoem__Motion__CameraKeyframeBundle *) nanoem_calloc(1, sizeof(*camera_keyframe_bundle_message), status);
            if (nanoem_is_not_null(camera_keyframe_bundle_message)) {
                nanoem__motion__camera_keyframe_bundle__init(camera_keyframe_bundle_message);
                num_camera_keyframes = camera_keyframe_bundle_message->n_keyframes = motion->num_camera_keyframes;
                camera_keyframe_bundle_message->keyframes = (Nanoem__Motion__CameraKeyframe **) nanoem_calloc(num_camera_keyframes, sizeof(*camera_keyframe_bundle_message->keyframes), status);
                for (i = 0; i < num_camera_keyframes; i++) {
                    camera_keyframe = motion->camera_keyframes[i];
                    camera_keyframe_message = camera_keyframe_bundle_message->keyframes[i] = (Nanoem__Motion__CameraKeyframe *) nanoem_calloc(1, sizeof(*camera_keyframe_message), status);
                    if (nanoem_is_not_null(camera_keyframe_message)) {
                        nanoem__motion__camera_keyframe__init(camera_keyframe_message);
                        nanoemMutableMotionCreateKeyframeCommonNMD(&camera_keyframe_message->common, &camera_keyframe->base, status);
                        if (nanoem_status_ptr_has_error(status)) {
                            num_camera_keyframes = camera_keyframe_bundle_message->n_keyframes = i;
                            return;
                        }
                        nanoemMutableMotionCreateVector3NMD(&camera_keyframe_message->angle, &camera_keyframe->angle, status);
                        nanoemMutableMotionCreateVector3NMD(&camera_keyframe_message->look_at, &camera_keyframe->look_at, status);
                        camera_keyframe_message->distance = camera_keyframe->distance;
                        camera_keyframe_message->fov = camera_keyframe->fov;
                        camera_keyframe_message->is_perspective_view_enabled = camera_keyframe->is_perspective_view;
                        camera_keyframe_message->has_is_perspective_view_enabled = 1;
                        if (camera_keyframe->stage_index > 0) {
                            camera_keyframe_message->has_stage_index = 1;
                            camera_keyframe_message->stage_index = camera_keyframe->stage_index;
                        }
                        interpolation_message = camera_keyframe_message->interpolation = (Nanoem__Motion__CameraKeyframeInterpolation *) nanoem_calloc(1, sizeof(*interpolation_message), status);
                        nanoem__motion__camera_keyframe_interpolation__init(interpolation_message);
                        if (!nanoemMotionCameraKeyframeIsLinearInterpolation(camera_keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X)) {
                            nanoemMutableMotionCreateInterpolationNMD(&interpolation_message->x, camera_keyframe->interplation[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X].u.values, status);
                        }
                        if (!nanoemMotionCameraKeyframeIsLinearInterpolation(camera_keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y)) {
                            nanoemMutableMotionCreateInterpolationNMD(&interpolation_message->y, camera_keyframe->interplation[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y].u.values, status);
                        }
                        if (!nanoemMotionCameraKeyframeIsLinearInterpolation(camera_keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z)) {
                            nanoemMutableMotionCreateInterpolationNMD(&interpolation_message->z, camera_keyframe->interplation[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z].u.values, status);
                        }
                        if (!nanoemMotionCameraKeyframeIsLinearInterpolation(camera_keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE)) {
                            nanoemMutableMotionCreateInterpolationNMD(&interpolation_message->angle, camera_keyframe->interplation[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE].u.values, status);
                        }
                        if (!nanoemMotionCameraKeyframeIsLinearInterpolation(camera_keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV)) {
                            nanoemMutableMotionCreateInterpolationNMD(&interpolation_message->fov, camera_keyframe->interplation[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV].u.values, status);
                        }
                        if (!nanoemMotionCameraKeyframeIsLinearInterpolation(camera_keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE)) {
                            nanoemMutableMotionCreateInterpolationNMD(&interpolation_message->distance, camera_keyframe->interplation[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE].u.values, status);
                        }
                    }
                    else {
                        camera_keyframe_bundle_message->n_keyframes = i;
                        return;
                    }
                }
                nanoem_status_ptr_assign_succeeded(status);
            }
        }
    }
}

static void
nanoemMutableMotionWriteLightKeyframeBundleUnitNMD(const nanoem_motion_t *motion, Nanoem__Motion__Motion *motion_message, nanoem_rsize_t *keyframe_bundle_index, nanoem_status_t *status)
{
    Nanoem__Motion__KeyframeBundleUnit *keyframe_unit_message;
    Nanoem__Motion__LightKeyframeBundle *light_keyframe_bundle_message;
    Nanoem__Motion__LightKeyframe *light_keyframe_message;
    const nanoem_motion_light_keyframe_t *light_keyframe;
    nanoem_rsize_t num_light_keyframes, i;
    if (motion->num_light_keyframes > 0) {
        keyframe_unit_message = motion_message->keyframe_bundles[*keyframe_bundle_index] = (Nanoem__Motion__KeyframeBundleUnit *) nanoem_calloc(1, sizeof(*keyframe_unit_message), status);
        *keyframe_bundle_index += 1;
        if (nanoem_is_not_null(keyframe_unit_message)) {
            nanoem__motion__keyframe_bundle_unit__init(keyframe_unit_message);
            keyframe_unit_message->unit_case = NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_LIGHT;
            light_keyframe_bundle_message = keyframe_unit_message->light = (Nanoem__Motion__LightKeyframeBundle *) nanoem_calloc(1, sizeof(*light_keyframe_bundle_message), status);
            if (nanoem_is_not_null(light_keyframe_bundle_message)) {
                nanoem__motion__light_keyframe_bundle__init(light_keyframe_bundle_message);
                num_light_keyframes = light_keyframe_bundle_message->n_keyframes = motion->num_light_keyframes;
                light_keyframe_bundle_message->keyframes = (Nanoem__Motion__LightKeyframe **) nanoem_calloc(num_light_keyframes, sizeof(*light_keyframe_bundle_message->keyframes), status);
                for (i = 0; i < num_light_keyframes; i++) {
                    light_keyframe = motion->light_keyframes[i];
                    light_keyframe_message = light_keyframe_bundle_message->keyframes[i] = (Nanoem__Motion__LightKeyframe *) nanoem_calloc(1, sizeof(*light_keyframe_message), status);
                    if (nanoem_is_not_null(light_keyframe_message)) {
                        nanoem__motion__light_keyframe__init(light_keyframe_message);
                        nanoemMutableMotionCreateKeyframeCommonNMD(&light_keyframe_message->common, &light_keyframe->base, status);
                        if (nanoem_status_ptr_has_error(status)) {
                            num_light_keyframes = light_keyframe_bundle_message->n_keyframes = i;
                            return;
                        }
                        nanoemMutableMotionCreateVector3NMD(&light_keyframe_message->color, &light_keyframe->color, status);
                        nanoemMutableMotionCreateVector3NMD(&light_keyframe_message->direction, &light_keyframe->direction, status);
                    }
                    else {
                        light_keyframe_bundle_message->n_keyframes = i;
                        return;
                    }
                }
                nanoem_status_ptr_assign_succeeded(status);
            }
        }
    }
}

static void
nanoemMutableMotionWriteModelKeyframeConstraintStatesNMD(const nanoem_motion_model_keyframe_t *model_keyframe, Nanoem__Motion__ModelKeyframe *model_keyframe_message, nanoem_status_t *status)
{
    Nanoem__Motion__ConstraintState *constraint_state_message;
    const nanoem_motion_model_keyframe_constraint_state_t *constraint_state;
    nanoem_rsize_t num_constraint_states, i;
    num_constraint_states = model_keyframe_message->n_constraint_states = model_keyframe->num_constraint_states;
    if (num_constraint_states > 0) {
        model_keyframe_message->constraint_states = (Nanoem__Motion__ConstraintState **) nanoem_calloc(num_constraint_states, sizeof(*model_keyframe_message->constraint_states), status);
        if (nanoem_is_not_null(model_keyframe_message->constraint_states)) {
            for (i = 0; i < num_constraint_states; i++) {
                constraint_state_message = model_keyframe_message->constraint_states[i] = (Nanoem__Motion__ConstraintState *) nanoem_calloc(1, sizeof(*constraint_state_message), status);
                if (nanoem_is_not_null(constraint_state_message)) {
                    nanoem__motion__constraint_state__init(constraint_state_message);
                    constraint_state = model_keyframe->constraint_states[i];
                    constraint_state_message->track_index = (uint64_t) constraint_state->bone_id;
                    constraint_state_message->enabled = constraint_state->enabled;
                }
                else {
                    model_keyframe_message->n_constraint_states = i;
                    return;
                }
            }
            nanoem_status_ptr_assign_succeeded(status);
        }
    }
}

static void
nanoemMutableMotionWriteModelKeyframeEffectParametersNMD(const nanoem_motion_model_keyframe_t *model_keyframe, Nanoem__Motion__ModelKeyframe *model_keyframe_message, nanoem_status_t *status)
{
    Nanoem__Motion__EffectParameter *effect_parameter_message;
    nanoem_rsize_t num_effect_parameters, i;
    num_effect_parameters = model_keyframe_message->n_effect_parameters = model_keyframe->num_effect_parameters;
    if (num_effect_parameters > 0) {
        model_keyframe_message->effect_parameters = (Nanoem__Motion__EffectParameter **) nanoem_calloc(num_effect_parameters, sizeof(*model_keyframe_message->effect_parameters), status);
        if (nanoem_is_not_null(model_keyframe_message->effect_parameters)) {
            for (i = 0; i < num_effect_parameters; i++) {
                effect_parameter_message = model_keyframe_message->effect_parameters[i] = (Nanoem__Motion__EffectParameter *) nanoem_calloc(1, sizeof(*effect_parameter_message), status);
                if (nanoem_is_not_null(effect_parameter_message)) {
                    nanoemMutableMotionWriteEffectParameter(effect_parameter_message, model_keyframe->effect_parameters[i], status);
                }
                else {
                    model_keyframe_message->n_effect_parameters = i;
                    return;
                }
            }
            nanoem_status_ptr_assign_succeeded(status);
        }
    }
}

static void
nanoemMutableMotionWriteModelKeyframeModelBindingsNMD(const nanoem_motion_model_keyframe_t *model_keyframe, Nanoem__Motion__ModelKeyframe *model_keyframe_message, nanoem_status_t *status)
{
    Nanoem__Motion__ModelBinding *outside_parent_message;
    const nanoem_motion_outside_parent_t *ops;
    nanoem_rsize_t num_ops, i;
    num_ops = model_keyframe_message->n_bindings = model_keyframe->num_outside_parents;
    if (num_ops > 0) {
        model_keyframe_message->bindings = (Nanoem__Motion__ModelBinding **) nanoem_calloc(num_ops, sizeof(*model_keyframe_message->bindings), status);
        if (nanoem_is_not_null(model_keyframe_message->bindings)) {
            for (i = 0; i < num_ops; i++) {
                outside_parent_message = model_keyframe_message->bindings[i] = (Nanoem__Motion__ModelBinding *) nanoem_calloc(1, sizeof(*outside_parent_message), status);
                if (nanoem_is_not_null(outside_parent_message)) {
                    nanoem__motion__model_binding__init(outside_parent_message);
                    ops = model_keyframe->outside_parents[i];
                    outside_parent_message->has_local_bone_track_index = 1;
                    outside_parent_message->local_bone_track_index = ops->local_bone_track_index;
                    outside_parent_message->global_bone_track_index = ops->global_bone_track_index;
                    outside_parent_message->global_object_track_index = ops->global_model_track_index;
                }
                else {
                    model_keyframe_message->n_bindings = i;
                    return;
                }
            }
            nanoem_status_ptr_assign_succeeded(status);
        }
    }
}

static Nanoem__Motion__Edge *
nanoemMutableMotionWriteModelKeyframeEdgeNMD(const nanoem_motion_model_keyframe_t *model_keyframe, nanoem_status_t *status)
{
    Nanoem__Motion__Edge *edge_message;
    edge_message = (Nanoem__Motion__Edge *) nanoem_calloc(1, sizeof(*edge_message), status);
    if (nanoem_is_not_null(edge_message)) {
        nanoem__motion__edge__init(edge_message);
        nanoemMutableMotionCreateVector4NMD(&edge_message->color, &model_keyframe->edge_color, status);
        if (nanoem_is_not_null(edge_message->color)) {
            edge_message->scale_factor = model_keyframe->edge_scale_factor;
        }
    }
    return edge_message;
}

static void
nanoemMutableMotionWriteModelKeyframeBundleUnitNMD(const nanoem_motion_t *motion, Nanoem__Motion__Motion *motion_message, nanoem_rsize_t *keyframe_bundle_index, nanoem_status_t *status)
{
    Nanoem__Motion__KeyframeBundleUnit *keyframe_unit_message;
    Nanoem__Motion__ModelKeyframeBundle *model_keyframe_bundle_message;
    Nanoem__Motion__ModelKeyframe *model_keyframe_message;
    Nanoem__Motion__Edge *edge_message;
    const nanoem_motion_model_keyframe_t *model_keyframe;
    nanoem_rsize_t num_model_keyframes, i;
    if (motion->num_model_keyframes > 0) {
        keyframe_unit_message = motion_message->keyframe_bundles[*keyframe_bundle_index] = (Nanoem__Motion__KeyframeBundleUnit *) nanoem_calloc(1, sizeof(*keyframe_unit_message), status);
        *keyframe_bundle_index += 1;
        if (nanoem_is_not_null(keyframe_unit_message)) {
            nanoem__motion__keyframe_bundle_unit__init(keyframe_unit_message);
            keyframe_unit_message->unit_case = NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_MODEL;
            model_keyframe_bundle_message = keyframe_unit_message->model = (Nanoem__Motion__ModelKeyframeBundle *) nanoem_calloc(1, sizeof(*model_keyframe_bundle_message), status);
            if (nanoem_is_not_null(model_keyframe_bundle_message)) {
                nanoem__motion__model_keyframe_bundle__init(model_keyframe_bundle_message);
                num_model_keyframes = model_keyframe_bundle_message->n_keyframes = motion->num_model_keyframes;
                model_keyframe_bundle_message->keyframes = (Nanoem__Motion__ModelKeyframe **) nanoem_calloc(num_model_keyframes, sizeof(*model_keyframe_bundle_message->keyframes), status);
                for (i = 0; i < num_model_keyframes; i++) {
                    model_keyframe = motion->model_keyframes[i];
                    model_keyframe_message = model_keyframe_bundle_message->keyframes[i] = (Nanoem__Motion__ModelKeyframe *) nanoem_calloc(1, sizeof(*model_keyframe_message), status);
                    if (nanoem_is_not_null(model_keyframe_message)) {
                        nanoem__motion__model_keyframe__init(model_keyframe_message);
                        nanoemMutableMotionCreateKeyframeCommonNMD(&model_keyframe_message->common, &model_keyframe->base, status);
                        if (nanoem_status_ptr_has_error(status)) {
                            num_model_keyframes = model_keyframe_bundle_message->n_keyframes = i;
                            return;
                        }
                        nanoemMutableMotionWriteModelKeyframeConstraintStatesNMD(model_keyframe, model_keyframe_message, status);
                        if (nanoem_status_ptr_has_error(status)) {
                            model_keyframe_bundle_message->n_keyframes = i;
                            return;
                        }
                        nanoemMutableMotionWriteModelKeyframeEffectParametersNMD(model_keyframe, model_keyframe_message, status);
                        if (nanoem_status_ptr_has_error(status)) {
                            model_keyframe_bundle_message->n_keyframes = i;
                            return;
                        }
                        nanoemMutableMotionWriteModelKeyframeModelBindingsNMD(model_keyframe, model_keyframe_message, status);
                        if (nanoem_status_ptr_has_error(status)) {
                            model_keyframe_bundle_message->n_keyframes = i;
                            return;
                        }
                        edge_message = model_keyframe_message->edge = nanoemMutableMotionWriteModelKeyframeEdgeNMD(model_keyframe, status);
                        if (nanoem_is_null(edge_message)) {
                            model_keyframe_bundle_message->n_keyframes = i;
                            return;
                        }
                        model_keyframe_message->visible = model_keyframe->visible;
                        model_keyframe_message->has_is_physics_simulation_enabled = 1;
                        model_keyframe_message->is_physics_simulation_enabled = model_keyframe->is_physics_simulation_enabled;
                        if (model_keyframe->is_add_blending_enabled) {
                            model_keyframe_message->has_is_add_blending_enabled = 1;
                            model_keyframe_message->is_add_blending_enabled = model_keyframe->is_add_blending_enabled;
                        }
                    }
                    else {
                        model_keyframe_bundle_message->n_keyframes = i;
                        return;
                    }
                }
                nanoem_status_ptr_assign_succeeded(status);
            }
        }
    }
}

static void
nanoemMutableMotionWriteMorphTrackBundleNMD(const nanoem_motion_t *motion, Nanoem__Motion__MorphKeyframeBundle *morph_keyframe_bundle_message, nanoem_status_t *status)
{
    Nanoem__Motion__Track **track_bundle_message, *track_message;
    const nanoem_motion_track_t *track;
    kh_motion_track_bundle_t *track_bundle = motion->local_morph_motion_track_bundle;
    nanoem_unicode_string_factory_t *factory = motion->factory;
    nanoem_rsize_t length, i;
    khiter_t it, end;
    char *bytes;
    morph_keyframe_bundle_message->n_local_tracks = kh_size(track_bundle);
    track_bundle_message = morph_keyframe_bundle_message->local_tracks = (Nanoem__Motion__Track **) nanoem_calloc(kh_size(track_bundle), sizeof(*track_bundle_message), status);
    if (nanoem_is_not_null(track_bundle_message)) {
        i = 0;
        for (it = 0, end = kh_end(track_bundle); it != end; it++) {
            if (kh_exist(track_bundle, it)) {
                track_message = track_bundle_message[i++] = (Nanoem__Motion__Track *) nanoem_calloc(1, sizeof(*track_message), status);
                if (nanoem_is_not_null(track_message)) {
                    nanoem__motion__track__init(track_message);
                    track = &kh_key(track_bundle, it);
                    bytes = (char *) nanoemUnicodeStringFactoryGetByteArrayEncoding(factory, track->name, &length, NANOEM_CODEC_TYPE_UTF8, status);
                    if (bytes) {
                        track_message->index = (uint64_t) track->id;
                        track_message->name = nanoemUtilCloneString(bytes ,status);
                        nanoemUnicodeStringFactoryDestroyByteArray(factory, (nanoem_u8_t *) bytes);
                    }
                    else {
                        return;
                    }
                }
                else {
                    return;
                }
            }
        }
        nanoem_status_ptr_assign_succeeded(status);
    }
}

static void
nanoemMutableMotionWriteMorphKeyframeBundleUnitNMD(const nanoem_motion_t *motion, Nanoem__Motion__Motion *motion_message, nanoem_rsize_t *keyframe_bundle_index, nanoem_status_t *status)
{
    static const uint8_t fixed_interpolation_values[] = { 20, 20, 107, 107 };
    Nanoem__Motion__KeyframeBundleUnit *keyframe_unit_message;
    Nanoem__Motion__MorphKeyframeBundle *morph_keyframe_bundle_message;
    Nanoem__Motion__MorphKeyframe *morph_keyframe_message;
    Nanoem__Motion__MorphKeyframeInterpolation *interpolation_message;
    const nanoem_motion_morph_keyframe_t *morph_keyframe;
    nanoem_rsize_t num_morph_keyframes, i;
    if (motion->num_morph_keyframes > 0) {
        keyframe_unit_message = motion_message->keyframe_bundles[*keyframe_bundle_index] = (Nanoem__Motion__KeyframeBundleUnit *) nanoem_calloc(1, sizeof(*keyframe_unit_message), status);
        *keyframe_bundle_index += 1;
        if (nanoem_is_not_null(keyframe_unit_message)) {
            nanoem__motion__keyframe_bundle_unit__init(keyframe_unit_message);
            keyframe_unit_message->unit_case = NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_MORPH;
            morph_keyframe_bundle_message = keyframe_unit_message->morph = (Nanoem__Motion__MorphKeyframeBundle *) nanoem_calloc(1, sizeof(*morph_keyframe_bundle_message), status);
            if (nanoem_is_not_null(morph_keyframe_bundle_message)) {
                nanoem__motion__morph_keyframe_bundle__init(morph_keyframe_bundle_message);
                nanoemMutableMotionWriteMorphTrackBundleNMD(motion, morph_keyframe_bundle_message, status);
                if (*status == NANOEM_STATUS_SUCCESS) {
                    num_morph_keyframes = morph_keyframe_bundle_message->n_keyframes = motion->num_morph_keyframes;
                    morph_keyframe_bundle_message->keyframes = (Nanoem__Motion__MorphKeyframe **) nanoem_calloc(num_morph_keyframes, sizeof(*morph_keyframe_bundle_message->keyframes), status);
                    for (i = 0; i < num_morph_keyframes; i++) {
                        morph_keyframe = motion->morph_keyframes[i];
                        morph_keyframe_message = morph_keyframe_bundle_message->keyframes[i] = (Nanoem__Motion__MorphKeyframe *) nanoem_calloc(1, sizeof(*morph_keyframe_message), status);
                        if (nanoem_is_not_null(morph_keyframe_message)) {
                            nanoem__motion__morph_keyframe__init(morph_keyframe_message);
                            nanoemMutableMotionCreateKeyframeCommonNMD(&morph_keyframe_message->common, &morph_keyframe->base, status);
                            if (nanoem_status_ptr_has_error(status)) {
                                num_morph_keyframes = morph_keyframe_bundle_message->n_keyframes = i;
                                return;
                            }
                            morph_keyframe_message->weight = morph_keyframe->weight;
                            morph_keyframe_message->track_index = (uint64_t) morph_keyframe->morph_id;
                            interpolation_message = morph_keyframe_message->interpolation = (Nanoem__Motion__MorphKeyframeInterpolation *) nanoem_calloc(1, sizeof(*interpolation_message), status);
                            nanoem__motion__morph_keyframe_interpolation__init(interpolation_message);
                            nanoemMutableMotionCreateInterpolationNMD(&interpolation_message->weight, fixed_interpolation_values, status);
                        }
                        else {
                            morph_keyframe_bundle_message->n_keyframes = i;
                            return;
                        }
                    }
                    nanoem_status_ptr_assign_succeeded(status);
                }
            }
        }
    }
}

static void
nanoemMutableMotionWritePhysicsWorldKeyframeBundleUnitNMD(const nanoem_motion_t *motion, Nanoem__Motion__Motion *motion_message, nanoem_rsize_t *keyframe_bundle_index, nanoem_status_t *status)
{
    nanoem_mark_unused(motion);
    nanoem_mark_unused(motion_message);
    nanoem_mark_unused(keyframe_bundle_index);
    nanoem_mark_unused(status);
}

static void
nanoemMutableMotionWriteSelfShadowKeyframeBundleUnitNMD(const nanoem_motion_t *motion, Nanoem__Motion__Motion *motion_message, nanoem_rsize_t *keyframe_bundle_index, nanoem_status_t *status)
{
    Nanoem__Motion__KeyframeBundleUnit *keyframe_unit_message;
    Nanoem__Motion__SelfShadowKeyframeBundle *self_shadow_keyframe_bundle_message;
    Nanoem__Motion__SelfShadowKeyframe *self_shadow_keyframe_message;
    const nanoem_motion_self_shadow_keyframe_t *self_shadow_keyframe;
    nanoem_rsize_t num_self_shadow_keyframes, i;
    if (motion->num_self_shadow_keyframes > 0) {
        keyframe_unit_message = motion_message->keyframe_bundles[*keyframe_bundle_index] = (Nanoem__Motion__KeyframeBundleUnit *) nanoem_calloc(1, sizeof(*keyframe_unit_message), status);
        *keyframe_bundle_index += 1;
        if (nanoem_is_not_null(keyframe_unit_message)) {
            nanoem__motion__keyframe_bundle_unit__init(keyframe_unit_message);
            keyframe_unit_message->unit_case = NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_SELF_SHADOW;
            self_shadow_keyframe_bundle_message = keyframe_unit_message->self_shadow = (Nanoem__Motion__SelfShadowKeyframeBundle *) nanoem_calloc(1, sizeof(*self_shadow_keyframe_bundle_message), status);
            if (nanoem_is_not_null(self_shadow_keyframe_bundle_message)) {
                nanoem__motion__self_shadow_keyframe_bundle__init(self_shadow_keyframe_bundle_message);
                num_self_shadow_keyframes = self_shadow_keyframe_bundle_message->n_keyframes = motion->num_self_shadow_keyframes;
                self_shadow_keyframe_bundle_message->keyframes = (Nanoem__Motion__SelfShadowKeyframe **) nanoem_calloc(num_self_shadow_keyframes, sizeof(*self_shadow_keyframe_bundle_message->keyframes), status);
                for (i = 0; i < num_self_shadow_keyframes; i++) {
                    self_shadow_keyframe = motion->self_shadow_keyframes[i];
                    self_shadow_keyframe_message = self_shadow_keyframe_bundle_message->keyframes[i] = (Nanoem__Motion__SelfShadowKeyframe *) nanoem_calloc(1, sizeof(*self_shadow_keyframe_message), status);
                    if (nanoem_is_not_null(self_shadow_keyframe_message)) {
                        nanoem__motion__self_shadow_keyframe__init(self_shadow_keyframe_message);
                        nanoemMutableMotionCreateKeyframeCommonNMD(&self_shadow_keyframe_message->common, &self_shadow_keyframe->base, status);
                        if (nanoem_status_ptr_has_error(status)) {
                            num_self_shadow_keyframes = self_shadow_keyframe_bundle_message->n_keyframes = i;
                            return;
                        }
                        self_shadow_keyframe_message->distance = self_shadow_keyframe->distance;
                        self_shadow_keyframe_message->mode = self_shadow_keyframe->mode;
                    }
                    else {
                        self_shadow_keyframe_bundle_message->n_keyframes = i;
                        return;
                    }
                }
                nanoem_status_ptr_assign_succeeded(status);
            }
        }
    }
}

static void
nanoemMutableMotionWriteGlobalTrackBundleNMD(const nanoem_motion_t *motion, Nanoem__Motion__Motion *motion_message, nanoem_status_t *status)
{
    Nanoem__Motion__Track **track_bundle_message, *track_message;
    const nanoem_motion_track_t *track;
    kh_motion_track_bundle_t *track_bundle = motion->global_motion_track_bundle;
    nanoem_unicode_string_factory_t *factory = motion->factory;
    nanoem_rsize_t num_global_tracks, length, i;
    nanoem_status_t st = NANOEM_STATUS_SUCCESS;
    khiter_t it, end;
    char *bytes;
    if (track_bundle) {
        num_global_tracks = kh_size(track_bundle);
        if (num_global_tracks > 0) {
            motion_message->n_global_tracks = num_global_tracks;
            track_bundle_message = motion_message->global_tracks = (Nanoem__Motion__Track **) nanoem_calloc(num_global_tracks, sizeof(*track_bundle_message), status);
            if (nanoem_is_not_null(track_bundle_message)) {
                i = 0;
                for (it = 0, end = kh_end(track_bundle); it != end; it++) {
                    if (kh_exist(track_bundle, it)) {
                        track_message = track_bundle_message[i++] = (Nanoem__Motion__Track *) nanoem_calloc(1, sizeof(*track_message), status);
                        if (nanoem_is_not_null(track_message)) {
                            nanoem__motion__track__init(track_message);
                            track = &kh_key(track_bundle, it);
                            bytes = (char *) nanoemUnicodeStringFactoryGetByteArrayEncoding(factory, track->name, &length, NANOEM_CODEC_TYPE_UTF8, &st);
                            if (st == NANOEM_STATUS_SUCCESS) {
                                track_message->index = (uint64_t) track->id;
                                track_message->name = nanoemUtilCloneString(bytes, status);
                                nanoemUnicodeStringFactoryDestroyByteArray(factory, (nanoem_u8_t *) bytes);
                            }
                            else {
                                return;
                            }
                        }
                        else {
                            return;
                        }
                    }
                }
                nanoem_status_ptr_assign_succeeded(status);
            }
        }
    }
}

static void
nanoemMutableMotionDestroyAnnotationsNMD(Nanoem__Motion__Annotation **annotation_messages, size_t num_annotations)
{
    Nanoem__Motion__Annotation *annotation_message;
    size_t i;
    for (i = 0; i < num_annotations; i++) {
        annotation_message = annotation_messages[i];
        if (annotation_message) {
            nanoem_free(annotation_message->name);
            nanoem_free(annotation_message->value);
            nanoem_free(annotation_message);
        }
    }
    nanoem_free(annotation_messages);
}

static void
nanoemMutableMotionDestroyTrackNMD(Nanoem__Motion__Track *track_message)
{
    if (track_message) {
        nanoem_free(track_message->name);
        nanoem_free(track_message);
    }
}

static void
nanoemMutableMotionDestroyEffectParametersNMD(Nanoem__Motion__EffectParameter **effect_parameter_messages, size_t num_effect_parameters)
{
    Nanoem__Motion__EffectParameter *effect_parameter_message;
    size_t i;
    for (i = 0; i < num_effect_parameters; i++) {
        effect_parameter_message = effect_parameter_messages[i];
        if (effect_parameter_message) {
            if (effect_parameter_message->value_case == NANOEM__MOTION__EFFECT_PARAMETER__VALUE_FV) {
                nanoem_free(effect_parameter_message->fv);
            }
            nanoem_free(effect_parameter_message);
        }
    }
    nanoem_free(effect_parameter_messages);
}

static void
nanoemMutableMotionDestroyTrackBundleNMD(Nanoem__Motion__Track **track_messages, size_t num_local_tracks)
{
    Nanoem__Motion__Track *track_message;
    size_t i;
    for (i = 0; i < num_local_tracks; i++) {
        track_message = track_messages[i];
        nanoemMutableMotionDestroyTrackNMD(track_message);
    }
    nanoem_free(track_messages);
}

static void
nanoemMutableMotionDestroyKeyframeCommonNMD(Nanoem__Motion__KeyframeCommon *keyframe_common_message)
{
    nanoemMutableMotionDestroyAnnotationsNMD(keyframe_common_message->annotations, keyframe_common_message->n_annotations);
    nanoem_free(keyframe_common_message);
}

static void
nanoemMutableMotionDestroyKeyframeInterpolationNMD(Nanoem__Motion__InterpolationUnit *interpolation_message)
{
    if (interpolation_message) {
        switch (interpolation_message->unit_case) {
        case NANOEM__MOTION__INTERPOLATION_UNIT__UNIT_FLOAT_TYPE:
            nanoem_free(interpolation_message->float_type);
            break;
        case NANOEM__MOTION__INTERPOLATION_UNIT__UNIT_INTEGER_TYPE:
            nanoem_free(interpolation_message->integer_type);
            break;
        case NANOEM__MOTION__INTERPOLATION_UNIT__UNIT__NOT_SET:
        default:
            break;
        }
        nanoem_free(interpolation_message);
    }
}

static void
nanoemMutableMotionDestroyAccessoryKeyframeBundleNMD(Nanoem__Motion__AccessoryKeyframeBundle *accessory_keyframe_bundle_message)
{
    Nanoem__Motion__AccessoryKeyframe *keyframe;
    size_t num_keyframes = accessory_keyframe_bundle_message->n_keyframes, i;
    nanoemMutableMotionDestroyAnnotationsNMD(accessory_keyframe_bundle_message->annotations, accessory_keyframe_bundle_message->n_annotations);
    for (i = 0; i < num_keyframes; i++) {
        keyframe = accessory_keyframe_bundle_message->keyframes[i];
        if (keyframe) {
            nanoemMutableMotionDestroyKeyframeCommonNMD(keyframe->common);
            nanoemMutableMotionDestroyEffectParametersNMD(keyframe->effect_parameters, keyframe->n_effect_parameters);
            nanoem_free(keyframe->binding);
            nanoem_free(keyframe->translation);
            nanoem_free(keyframe->orientation);
            nanoem_free(keyframe);
        }
    }
    nanoem_free(accessory_keyframe_bundle_message->keyframes);
    nanoem_free(accessory_keyframe_bundle_message);
}

static void
nanoemMutableMotionDestroyBoneKeyframeBundleNMD(Nanoem__Motion__BoneKeyframeBundle *bone_keyframe_bundle_message)
{
    Nanoem__Motion__BoneKeyframe *keyframe;
    Nanoem__Motion__BoneKeyframeInterpolation *interpolation;
    size_t num_keyframes = bone_keyframe_bundle_message->n_keyframes, i;
    nanoemMutableMotionDestroyAnnotationsNMD(bone_keyframe_bundle_message->annotations, bone_keyframe_bundle_message->n_annotations);
    nanoemMutableMotionDestroyTrackBundleNMD(bone_keyframe_bundle_message->local_tracks, bone_keyframe_bundle_message->n_local_tracks);
    for (i = 0; i < num_keyframes; i++) {
        keyframe = bone_keyframe_bundle_message->keyframes[i];
        if (keyframe) {
            interpolation = keyframe->interpolation;
            nanoemMutableMotionDestroyKeyframeCommonNMD(keyframe->common);
            if (interpolation) {
                nanoemMutableMotionDestroyKeyframeInterpolationNMD(interpolation->x);
                nanoemMutableMotionDestroyKeyframeInterpolationNMD(interpolation->y);
                nanoemMutableMotionDestroyKeyframeInterpolationNMD(interpolation->z);
                nanoemMutableMotionDestroyKeyframeInterpolationNMD(interpolation->orientation);
            }
            nanoem_free(interpolation);
            nanoem_free(keyframe->local_translation);
            nanoem_free(keyframe->local_orientation);
            nanoem_free(keyframe);
        }
    }
    nanoem_free(bone_keyframe_bundle_message->keyframes);
    nanoem_free(bone_keyframe_bundle_message);
}

static void
nanoemMutableMotionDestroyCameraKeyframeBundleNMD(Nanoem__Motion__CameraKeyframeBundle *camera_keyframe_bundle_message)
{
    Nanoem__Motion__CameraKeyframe *keyframe;
    Nanoem__Motion__CameraKeyframeInterpolation *interpolation;
    size_t num_keyframes = camera_keyframe_bundle_message->n_keyframes, i;
    nanoemMutableMotionDestroyAnnotationsNMD(camera_keyframe_bundle_message->annotations, camera_keyframe_bundle_message->n_annotations);
    for (i = 0; i < num_keyframes; i++) {
        keyframe = camera_keyframe_bundle_message->keyframes[i];
        if (keyframe) {
            interpolation = keyframe->interpolation;
            nanoemMutableMotionDestroyKeyframeCommonNMD(keyframe->common);
            if (interpolation) {
                nanoemMutableMotionDestroyKeyframeInterpolationNMD(interpolation->x);
                nanoemMutableMotionDestroyKeyframeInterpolationNMD(interpolation->y);
                nanoemMutableMotionDestroyKeyframeInterpolationNMD(interpolation->z);
                nanoemMutableMotionDestroyKeyframeInterpolationNMD(interpolation->angle);
                nanoemMutableMotionDestroyKeyframeInterpolationNMD(interpolation->distance);
                nanoemMutableMotionDestroyKeyframeInterpolationNMD(interpolation->fov);
            }
            nanoem_free(interpolation);
            nanoem_free(keyframe->look_at);
            nanoem_free(keyframe->angle);
            nanoem_free(keyframe);
        }
    }
    nanoem_free(camera_keyframe_bundle_message->keyframes);
    nanoem_free(camera_keyframe_bundle_message);
}

static void
nanoemMutableMotionDestroyLightKeyframeBundleNMD(Nanoem__Motion__LightKeyframeBundle *light_keyframe_bundle_message)
{
    Nanoem__Motion__LightKeyframe *keyframe;
    size_t num_keyframes = light_keyframe_bundle_message->n_keyframes, i;
    nanoemMutableMotionDestroyAnnotationsNMD(light_keyframe_bundle_message->annotations, light_keyframe_bundle_message->n_annotations);
    for (i = 0; i < num_keyframes; i++) {
        keyframe = light_keyframe_bundle_message->keyframes[i];
        if (keyframe) {
            nanoemMutableMotionDestroyKeyframeCommonNMD(keyframe->common);
            nanoem_free(keyframe->color);
            nanoem_free(keyframe->direction);
            nanoem_free(keyframe);
        }
    }
    nanoem_free(light_keyframe_bundle_message->keyframes);
    nanoem_free(light_keyframe_bundle_message);
}

static void
nanoemMutableMotionDestroyConstraintStatesNMD(Nanoem__Motion__ConstraintState **constraint_state_messages, size_t num_constraint_states)
{
    size_t i;
    for (i = 0; i < num_constraint_states; i++) {
        nanoem_free(constraint_state_messages[i]);
    }
    nanoem_free(constraint_state_messages);
}

static void
nanoemMutableMotionDestroyModelBindingsNMD(Nanoem__Motion__ModelBinding **outside_parent_messages, size_t num_outside_parents)
{
    size_t i;
    for (i = 0; i < num_outside_parents; i++) {
        nanoem_free(outside_parent_messages[i]);
    }
    nanoem_free(outside_parent_messages);
}

static void
nanoemMutableMotionDestroyModelKeyframeBundleNMD(Nanoem__Motion__ModelKeyframeBundle *model_keyframe_bundle_message)
{
    Nanoem__Motion__ModelKeyframe *keyframe;
    size_t num_keyframes = model_keyframe_bundle_message->n_keyframes, i;
    nanoemMutableMotionDestroyAnnotationsNMD(model_keyframe_bundle_message->annotations, model_keyframe_bundle_message->n_annotations);
    for (i = 0; i < num_keyframes; i++) {
        keyframe = model_keyframe_bundle_message->keyframes[i];
        if (keyframe) {
            nanoemMutableMotionDestroyKeyframeCommonNMD(keyframe->common);
            nanoemMutableMotionDestroyConstraintStatesNMD(keyframe->constraint_states, keyframe->n_constraint_states);
            nanoemMutableMotionDestroyModelBindingsNMD(keyframe->bindings, keyframe->n_bindings);
            nanoemMutableMotionDestroyEffectParametersNMD(keyframe->effect_parameters, keyframe->n_effect_parameters);
            if (nanoem_is_not_null(keyframe->edge)) {
                nanoem_free(keyframe->edge->color);
                nanoem_free(keyframe->edge);
            }
            nanoem_free(keyframe);
        }
    }
    nanoem_free(model_keyframe_bundle_message->keyframes);
    nanoem_free(model_keyframe_bundle_message);
}

static void
nanoemMutableMotionDestroyMorphKeyframeBundleNMD(Nanoem__Motion__MorphKeyframeBundle *morph_keyframe_bundle_message)
{
    Nanoem__Motion__MorphKeyframe *keyframe;
    Nanoem__Motion__MorphKeyframeInterpolation *interpolation;
    size_t num_keyframes = morph_keyframe_bundle_message->n_keyframes, i;
    nanoemMutableMotionDestroyAnnotationsNMD(morph_keyframe_bundle_message->annotations, morph_keyframe_bundle_message->n_annotations);
    nanoemMutableMotionDestroyTrackBundleNMD(morph_keyframe_bundle_message->local_tracks, morph_keyframe_bundle_message->n_local_tracks);
    for (i = 0; i < num_keyframes; i++) {
        keyframe = morph_keyframe_bundle_message->keyframes[i];
        if (keyframe) {
            interpolation = keyframe->interpolation;
            nanoemMutableMotionDestroyKeyframeCommonNMD(keyframe->common);
            nanoemMutableMotionDestroyKeyframeInterpolationNMD(interpolation->weight);
            nanoem_free(interpolation);
            nanoem_free(keyframe);
        }
    }
    nanoem_free(morph_keyframe_bundle_message->keyframes);
    nanoem_free(morph_keyframe_bundle_message);
}

static void
nanoemMutableMotionDestroySelfShadowKeyframeBundleNMD(Nanoem__Motion__SelfShadowKeyframeBundle *self_shadow_keyframe_bundle_message)
{
    Nanoem__Motion__SelfShadowKeyframe *keyframe;
    size_t num_keyframes = self_shadow_keyframe_bundle_message->n_keyframes, i;
    nanoemMutableMotionDestroyAnnotationsNMD(self_shadow_keyframe_bundle_message->annotations, self_shadow_keyframe_bundle_message->n_annotations);
    for (i = 0; i < num_keyframes; i++) {
        keyframe = self_shadow_keyframe_bundle_message->keyframes[i];
        if (keyframe) {
            nanoemMutableMotionDestroyKeyframeCommonNMD(keyframe->common);
            nanoem_free(keyframe);
        }
    }
    nanoem_free(self_shadow_keyframe_bundle_message->keyframes);
    nanoem_free(self_shadow_keyframe_bundle_message);
}

static void
nanoemMutableMotionDestroyNMD(nanoem_motion_t *motion, Nanoem__Motion__Motion *motion_message)
{
    Nanoem__Motion__KeyframeBundleUnit *keyframe_bundle_message;
    size_t num_keyframe_bundles = motion_message->n_keyframe_bundles, i;
    nanoemMutableMotionDestroyAnnotationsNMD(motion_message->annotations, motion_message->n_annotations);
    nanoemMutableMotionDestroyTrackBundleNMD(motion_message->global_tracks, motion_message->n_global_tracks);
    nanoemUnicodeStringFactoryDestroyByteArray(motion->factory, (nanoem_u8_t *)  motion_message->main_object_name);
    for (i = 0; i < num_keyframe_bundles; i++) {
        keyframe_bundle_message = motion_message->keyframe_bundles[i];
        if (keyframe_bundle_message) {
            switch (keyframe_bundle_message->unit_case) {
            case NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_BONE:
                nanoemMutableMotionDestroyBoneKeyframeBundleNMD(keyframe_bundle_message->bone);
                break;
            case NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_MODEL:
                nanoemMutableMotionDestroyModelKeyframeBundleNMD(keyframe_bundle_message->model);
                break;
            case NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_MORPH:
                nanoemMutableMotionDestroyMorphKeyframeBundleNMD(keyframe_bundle_message->morph);
                break;
            case NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_ACCESSORY:
                nanoemMutableMotionDestroyAccessoryKeyframeBundleNMD(keyframe_bundle_message->accessory);
                break;
            case NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_CAMERA:
                nanoemMutableMotionDestroyCameraKeyframeBundleNMD(keyframe_bundle_message->camera);
                break;
            case NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_LIGHT:
                nanoemMutableMotionDestroyLightKeyframeBundleNMD(keyframe_bundle_message->light);
                break;
            case NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_SELF_SHADOW:
                nanoemMutableMotionDestroySelfShadowKeyframeBundleNMD(keyframe_bundle_message->self_shadow);
                break;
            case NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT__NOT_SET:
            default:
                break;
            }
            nanoem_free(keyframe_bundle_message);
        }
    }
    nanoem_free(motion_message->keyframe_bundles);
}

#define nanoem_write_check_nmd(expr) \
    do { \
        (expr); \
        if (nanoem_status_ptr_has_error(status)) { \
            nanoemMutableMotionDestroyNMD(origin, &motion_message); \
            return nanoem_false; \
        } \
    } while (0)

nanoem_bool_t APIENTRY
nanoemMutableMotionSaveToBufferNMD(nanoem_mutable_motion_t *motion, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_motion_t *origin;
    Nanoem__Motion__Motion motion_message = NANOEM__MOTION__MOTION__INIT;
    nanoem_protobufc_buffer_writer_t writer;
    nanoem_status_t st = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t length;
    writer.base.append = nanoemProtobufCBufferWriterAppend;
    writer.ptr = buffer;
    writer.status = NANOEM_STATUS_SUCCESS;
    if (nanoem_is_not_null(motion) && nanoem_is_not_null(buffer)) {
        nanoem_status_ptr_assign_succeeded(status);
        origin = motion->origin;
        motion_message.keyframe_bundles = (Nanoem__Motion__KeyframeBundleUnit **) nanoem_calloc(NANOEM__MOTION__KEYFRAME_BUNDLE_UNIT__UNIT_SELF_SHADOW, sizeof(*motion_message.keyframe_bundles), status);
        motion_message.main_object_name = (char *) nanoemUnicodeStringFactoryGetByteArray(origin->factory, origin->target_model_name, &length, &st);
        motion_message.preferred_fps = origin->preferred_fps;
        nanoem_write_check_nmd(nanoemMutableMotionWriteAnnotationsNMD(&motion_message.annotations, &motion_message.n_annotations, origin->annotations, status));
        nanoem_write_check_nmd(nanoemMutableMotionWriteGlobalTrackBundleNMD(origin, &motion_message, status));
        nanoem_write_check_nmd(nanoemMutableMotionWriteAccessoryKeyframeBundleUnitNMD(origin, &motion_message, &motion_message.n_keyframe_bundles, status));
        nanoem_write_check_nmd(nanoemMutableMotionWriteBoneKeyframeBundleUnitNMD(origin, &motion_message, &motion_message.n_keyframe_bundles, status));
        nanoem_write_check_nmd(nanoemMutableMotionWriteCameraKeyframeBundleUnitNMD(origin, &motion_message, &motion_message.n_keyframe_bundles, status));
        nanoem_write_check_nmd(nanoemMutableMotionWriteLightKeyframeBundleUnitNMD(origin, &motion_message, &motion_message.n_keyframe_bundles, status));
        nanoem_write_check_nmd(nanoemMutableMotionWriteModelKeyframeBundleUnitNMD(origin, &motion_message, &motion_message.n_keyframe_bundles, status));
        nanoem_write_check_nmd(nanoemMutableMotionWriteMorphKeyframeBundleUnitNMD(origin, &motion_message, &motion_message.n_keyframe_bundles, status));
        nanoem_write_check_nmd(nanoemMutableMotionWritePhysicsWorldKeyframeBundleUnitNMD(origin, &motion_message, &motion_message.n_keyframe_bundles, status));
        nanoem_write_check_nmd(nanoemMutableMotionWriteSelfShadowKeyframeBundleUnitNMD(origin, &motion_message, &motion_message.n_keyframe_bundles, status));
        if (protobuf_c_message_check(&motion_message.base) != 0) {
            nanoem__motion__motion__pack_to_buffer(&motion_message, (ProtobufCBuffer *) &writer);
        }
        else {
            nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_BUFFER_END);
        }
        if (writer.status != NANOEM_STATUS_SUCCESS) {
            nanoem_status_ptr_assign(status, writer.status);
        }
        nanoemMutableMotionDestroyNMD(origin, &motion_message);
    }
    return !nanoem_status_ptr_has_error(status);
}

#undef nanoem_write_check
