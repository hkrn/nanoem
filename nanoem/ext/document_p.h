/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_DOCUMENT_PRIVATE_H_
#define NANOEM_DOCUMENT_PRIVATE_H_

#include "./mutable_p.h"
#include "../nanoem_p.h"

#define nanoem_crt_memchr(src, chr, size) memchr((src), (chr), (size))

#define NANOEM_PMM_PATH_MAX 256
#define NANOEM_PMM_ACCESSORY_NAME_MAX 100
#define NANOEM_PMMV1_MODEL_NAME_MAX 20

static const char __nanoem_pmmv1_signature[] = "Polygon Movie maker 0001";
static const char __nanoem_pmmv2_signature[] = "Polygon Movie maker 0002";

struct nanoem_document_base_keyframe_t {
    int object_index;
    nanoem_frame_index_t frame_index;
    nanoem_rsize_t previous_keyframe_index;
    nanoem_rsize_t next_keyframe_index;
    nanoem_bool_t is_selected;
};

typedef struct nanoem_document_object_t nanoem_document_object_t;
struct nanoem_document_object_t {
    int index;
    union {
        const nanoem_document_accessory_t *accessory;
        const nanoem_document_model_t *model;
        const nanoem_document_t *document;
    } parent;
};

struct nanoem_document_outside_parent_t {
    nanoem_document_object_t base;
    int model_index;
    int bone_index;
};

struct nanoem_document_accessory_keyframe_t {
    nanoem_document_base_keyframe_t base;
    nanoem_document_accessory_t *parent_accessory;
    nanoem_f32_t opacity;
    nanoem_bool_t visible;
    int parent_model_index;
    int parent_model_bone_index;
    nanoem_f128_t translation;
    nanoem_f128_t orientation;
    nanoem_f32_t scale_factor;
    nanoem_bool_t is_shadow_enabled;
};

struct nanoem_document_model_bone_keyframe_t {
    nanoem_document_base_keyframe_t base;
    nanoem_document_model_t *parent_model;
    nanoem_interpolation_t interpolation[4];
    nanoem_f128_t translation;
    nanoem_f128_t orientation;
    nanoem_bool_t is_physics_simulation_disabled;
};

struct nanoem_document_camera_keyframe_t {
    nanoem_document_base_keyframe_t base;
    nanoem_document_camera_t *parent_camera;
    nanoem_f32_t distance;
    nanoem_f128_t look_at;
    nanoem_f128_t angle;
    int parent_model_index;
    int parent_model_bone_index;
    nanoem_interpolation_t interpolation[6];
    nanoem_bool_t is_perspective_view;
    int fov;
};

struct nanoem_document_gravity_keyframe_t {
    nanoem_document_base_keyframe_t base;
    nanoem_document_gravity_t *parent_gravity;
    nanoem_bool_t is_noise_enabled;
    int noise;
    nanoem_f32_t acceleration;
    nanoem_f128_t direction;
};

struct nanoem_document_light_keyframe_t {
    nanoem_document_base_keyframe_t base;
    nanoem_document_light_t *parent_light;
    nanoem_f128_t color;
    nanoem_f128_t direction;
};

struct nanoem_document_model_keyframe_t {
    nanoem_document_base_keyframe_t base;
    nanoem_document_model_t *parent_model;
    nanoem_bool_t visible;
    nanoem_rsize_t num_constraint_states;
    nanoem_document_model_constraint_state_t **constraint_states;
    nanoem_rsize_t num_outside_parents;
    nanoem_document_outside_parent_t **outside_parents;
};

struct nanoem_document_model_morph_keyframe_t {
    nanoem_document_base_keyframe_t base;
    nanoem_document_model_t *parent_model;
    nanoem_f32_t weight;
};

struct nanoem_document_self_shadow_keyframe_t {
    nanoem_document_base_keyframe_t base;
    nanoem_document_self_shadow_t *parent_self_shadow;
    nanoem_u8_t mode;
    nanoem_f32_t distance;
};

struct nanoem_document_model_bone_state_t {
    nanoem_document_object_t base;
    nanoem_f128_t translation;
    nanoem_f128_t orientation;
    nanoem_bool_t dirty;
    nanoem_bool_t disable_physics_simulation;
    nanoem_u8_t num_tracks_selected;
};

struct nanoem_document_model_morph_state_t {
    nanoem_document_object_t base;
    nanoem_f32_t weight;
};

struct nanoem_document_model_constraint_state_t {
    nanoem_document_object_t base;
    nanoem_bool_t enabled;
};

struct nanoem_document_model_outside_parent_state_t {
    nanoem_document_object_t base;
    nanoem_frame_index_t begin;
    nanoem_frame_index_t end;
    nanoem_document_outside_parent_t *outside_parent;
};

struct nanoem_document_model_t {
    nanoem_document_object_t base;
    nanoem_unicode_string_factory_t *factory;
    nanoem_unicode_string_t *name_ja;
    nanoem_unicode_string_t *name_en;
    nanoem_unicode_string_t *path;
    nanoem_u8_t num_fixed_tracks;
    nanoem_rsize_t num_bones;
    nanoem_unicode_string_t **bone_names;
    nanoem_rsize_t num_morphs;
    nanoem_unicode_string_t **morph_names;
    nanoem_rsize_t num_constraint_bones;
    int *constraint_bone_indices;
    nanoem_rsize_t num_outside_parent_subject_bones;
    int *outside_parent_subject_bone_indices;
    nanoem_u8_t draw_order_index;
    nanoem_bool_t visible;
    int selected_bone_index;
    int selected_morph_indices[4];
    nanoem_rsize_t num_expansion_states;
    nanoem_u8_t *expansion_state;
    int vertical_scroll;
    nanoem_frame_index_t last_frame_index;
    nanoem_rsize_t num_initial_bone_keyframes;
    nanoem_document_model_bone_keyframe_t **initial_bone_keyframes;
    nanoem_rsize_t num_bone_keyframes;
    nanoem_document_model_bone_keyframe_t **bone_keyframes;
    nanoem_rsize_t num_initial_morph_keyframes;
    nanoem_document_model_morph_keyframe_t **initial_morph_keyframes;
    nanoem_rsize_t num_morph_keyframes;
    nanoem_document_model_morph_keyframe_t **morph_keyframes;
    nanoem_document_model_keyframe_t *initial_model_keyframe;
    nanoem_rsize_t num_model_keyframes;
    nanoem_document_model_keyframe_t **model_keyframes;
    nanoem_rsize_t num_bone_states;
    nanoem_document_model_bone_state_t **bone_states;
    nanoem_rsize_t num_morph_states;
    nanoem_document_model_morph_state_t **morph_states;
    nanoem_rsize_t num_constraint_states;
    nanoem_document_model_constraint_state_t **constraint_states;
    nanoem_rsize_t num_outside_parent_states;
    nanoem_document_model_outside_parent_state_t **outside_parent_states;
    nanoem_bool_t is_blend_enabled;
    nanoem_f32_t edge_width;
    nanoem_bool_t is_self_shadow_enabled;
    nanoem_u8_t transform_order_index;
    nanoem_document_model_bone_keyframe_t **all_bone_keyframes_ptr;
    nanoem_rsize_t num_all_bone_keyframes;
    nanoem_document_model_keyframe_t **all_model_keyframes_ptr;
    nanoem_rsize_t num_all_model_keyframes;
    nanoem_document_model_morph_keyframe_t **all_morph_keyframes_ptr;
    nanoem_rsize_t num_all_morph_keyframes;
    int selection_index;
};

struct nanoem_document_camera_t {
    nanoem_document_object_t base;
    nanoem_document_camera_keyframe_t *initial_camera_keyframe;
    nanoem_rsize_t num_camera_keyframes;
    nanoem_document_camera_keyframe_t **camera_keyframes;
    nanoem_f128_t look_at;
    nanoem_f128_t position;
    nanoem_f128_t angle;
    nanoem_bool_t is_perspective;
    nanoem_document_camera_keyframe_t **all_camera_keyframes_ptr;
};

struct nanoem_document_light_t {
    nanoem_document_object_t base;
    nanoem_document_light_keyframe_t *initial_light_keyframe;
    nanoem_rsize_t num_light_keyframes;
    nanoem_document_light_keyframe_t **light_keyframes;
    nanoem_f128_t color;
    nanoem_f128_t direction;
    nanoem_document_light_keyframe_t **all_light_keyframes_ptr;
};

struct nanoem_document_accessory_t {
    nanoem_document_object_t base;
    nanoem_unicode_string_factory_t *factory;
    nanoem_unicode_string_t *name;
    nanoem_unicode_string_t *path;
    int draw_order_index;
    nanoem_document_accessory_keyframe_t *initial_accessory_keyframe;
    nanoem_rsize_t num_accessory_keyframes;
    nanoem_document_accessory_keyframe_t **accessory_keyframes;
    nanoem_f32_t opacity;
    nanoem_bool_t visible;
    int parent_model_index;
    int parent_model_bone_index;
    nanoem_f128_t translation;
    nanoem_f128_t orientation;
    nanoem_f32_t scale_factor;
    nanoem_bool_t is_shadow_enabled;
    nanoem_bool_t is_add_blend_enabled;
    nanoem_document_accessory_keyframe_t **all_accessory_keyframes_ptr;
};

struct nanoem_document_gravity_t {
    nanoem_document_object_t base;
    nanoem_f32_t acceleration;
    int noise;
    nanoem_f128_t direction;
    nanoem_bool_t is_noise_enabled;
    nanoem_document_gravity_keyframe_t *initial_gravity_keyframe;
    nanoem_rsize_t num_gravity_keyframes;
    nanoem_document_gravity_keyframe_t **gravity_keyframes;
    nanoem_document_gravity_keyframe_t **all_gravity_keyframes_ptr;
};

struct nanoem_document_self_shadow_t {
    nanoem_document_object_t base;
    nanoem_document_self_shadow_keyframe_t *initial_self_shadow_keyframe;
    nanoem_rsize_t num_self_shadow_keyframes;
    nanoem_document_self_shadow_keyframe_t **self_shadow_keyframes;
    nanoem_bool_t is_self_shadow_enabled;
    nanoem_f32_t distance;
    nanoem_document_self_shadow_keyframe_t **all_self_shadow_keyframes_ptr;
};

struct nanoem_document_t {
    nanoem_unicode_string_factory_t *factory;
    int output_width;
    int output_height;
    int timeline_width;
    nanoem_f32_t camera_fov;
    nanoem_bool_t editing_cla;
    nanoem_bool_t expand_camera_panel;
    nanoem_bool_t expand_light_panel;
    nanoem_bool_t expand_accessory_panel;
    nanoem_bool_t expand_bone_panel;
    nanoem_bool_t expand_morph_panel;
    nanoem_bool_t expand_self_shadow_panel;
    nanoem_u8_t select_model_index;
    nanoem_rsize_t num_models;
    nanoem_document_model_t **models;
    nanoem_document_camera_t *camera;
    nanoem_document_light_t *light;
    nanoem_u8_t select_accessory_index;
    int horizontal_scroll_for_accessory;
    nanoem_rsize_t num_accessory_names;
    nanoem_unicode_string_t **accessory_names;
    nanoem_rsize_t num_accessories;
    nanoem_document_accessory_t **accessories;
    nanoem_frame_index_t current_frame_index;
    int horizontal_scroll;
    int horizontal_scroll_thumb;
    nanoem_document_editing_mode_t editing_mode;
    nanoem_u8_t camera_look_mode;
    nanoem_bool_t is_loop_enabled;
    nanoem_bool_t is_begin_frame_index_enabled;
    nanoem_bool_t is_end_frame_index_enabled;
    nanoem_frame_index_t begin_frame_index;
    nanoem_frame_index_t end_frame_index;
    nanoem_bool_t is_audio_enabled;
    nanoem_unicode_string_t *audio_path;
    int background_video_offset_x;
    int background_video_offset_y;
    nanoem_f32_t background_video_scale_factor;
    nanoem_unicode_string_t *background_video_path;
    nanoem_bool_t is_background_video_enabled;
    int background_image_offset_x;
    int background_image_offset_y;
    nanoem_f32_t background_image_scale_factor;
    nanoem_unicode_string_t *background_image_path;
    nanoem_bool_t is_background_image_enabled;
    nanoem_bool_t is_information_shown;
    nanoem_bool_t is_grid_and_axis_shown;
    nanoem_bool_t is_ground_shadow_shown;
    nanoem_f32_t preferred_fps;
    int screen_capture_mode;
    int accessory_index_after_models;
    nanoem_f32_t ground_shadow_brightness;
    nanoem_bool_t is_translucent_ground_shadow_enabled;
    nanoem_document_physics_simulation_mode_t physics_simulation_mode;
    nanoem_document_gravity_t *gravity;
    nanoem_document_self_shadow_t *self_shadow;
    nanoem_f128_t edge_color;
    nanoem_bool_t is_black_background_enabled;
    int camera_look_at_model_index;
    int camera_look_at_model_bone_index;
    nanoem_f32_t unknown_matrix[16];
    nanoem_bool_t is_following_look_at_enabled;
    nanoem_bool_t unknown_boolean;
    nanoem_bool_t is_physics_ground_enabled;
    nanoem_frame_index_t current_frame_index_in_text_field;
    nanoem_document_parse_model_callback_t parse_callback;
    void *parse_callback_user_data;
    int version;
};

typedef struct nanoem_mutable_document_base_object_t nanoem_mutable_document_base_object_t;
struct nanoem_mutable_document_base_object_t {
    union {
        nanoem_mutable_document_accessory_t *accessory;
        nanoem_mutable_document_camera_t *camera;
        nanoem_mutable_document_t *document;
        nanoem_mutable_document_gravity_t *gravity;
        nanoem_mutable_document_light_t *light;
        nanoem_mutable_document_model_t *model;
        nanoem_mutable_document_self_shadow_t *self_shadow;
    } parent;
    nanoem_bool_t is_reference;
    nanoem_bool_t is_in_object;
};

struct nanoem_mutable_document_outside_parent_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_outside_parent_t *origin;
};

struct nanoem_mutable_document_accessory_keyframe_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_accessory_keyframe_t *origin;
};

struct nanoem_mutable_document_model_bone_keyframe_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_model_bone_keyframe_t *origin;
    nanoem_unicode_string_t *name;
};

struct nanoem_mutable_document_camera_keyframe_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_camera_keyframe_t *origin;
};

struct nanoem_mutable_document_gravity_keyframe_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_gravity_keyframe_t *origin;
};

struct nanoem_mutable_document_light_keyframe_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_light_keyframe_t *origin;
};

struct nanoem_mutable_document_model_keyframe_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_model_keyframe_t *origin;
    nanoem_rsize_t num_allocated_constraint_states;
    nanoem_rsize_t num_allocated_outside_parents;
};

struct nanoem_mutable_document_model_morph_keyframe_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_model_morph_keyframe_t *origin;
    nanoem_unicode_string_t *name;
};

struct nanoem_mutable_document_self_shadow_keyframe_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_self_shadow_keyframe_t *origin;
};

struct nanoem_mutable_document_model_bone_state_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_model_bone_state_t *origin;
};

struct nanoem_mutable_document_model_morph_state_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_model_morph_state_t *origin;
};

struct nanoem_mutable_document_model_constraint_state_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_model_constraint_state_t *origin;
};

struct nanoem_mutable_document_model_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_model_t *origin;
    nanoem_rsize_t num_allocated_all_bone_keyframes;
    nanoem_rsize_t num_allocated_all_model_keyframes;
    nanoem_rsize_t num_allocated_all_morph_keyframes;
    nanoem_rsize_t num_allocated_bones;
    nanoem_rsize_t num_allocated_bone_keyframes;
    nanoem_rsize_t num_allocated_bone_states;
    nanoem_rsize_t num_allocated_constraint_bone_indices;
    nanoem_rsize_t num_allocated_constraint_states;
    nanoem_rsize_t num_allocated_expansion_states;
    nanoem_rsize_t num_allocated_inititial_bone_keyframes;
    nanoem_rsize_t num_allocated_inititial_morph_keyframes;
    nanoem_rsize_t num_allocated_model_keyframes;
    nanoem_rsize_t num_allocated_morphs;
    nanoem_rsize_t num_allocated_morph_keyframes;
    nanoem_rsize_t num_allocated_morph_states;
    nanoem_rsize_t num_allocated_outside_parent_states;
    nanoem_rsize_t num_allocated_outside_parent_subject_bones;
};

struct nanoem_mutable_document_accessory_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_accessory_t *origin;
    nanoem_rsize_t num_allocated_accessory_keyframes;
};

struct nanoem_mutable_document_camera_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_camera_t *origin;
    nanoem_rsize_t num_allocated_camera_keyframes;
};

struct nanoem_mutable_document_gravity_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_gravity_t *origin;
    nanoem_rsize_t num_allocated_gravity_keyframes;
};

struct nanoem_mutable_document_light_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_light_t *origin;
    nanoem_rsize_t num_allocated_light_keyframes;
};

struct nanoem_mutable_document_self_shadow_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_self_shadow_t *origin;
    nanoem_rsize_t num_allocated_self_shadow_keyframes;
};

struct nanoem_mutable_document_model_outside_parent_state_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_model_outside_parent_state_t *origin;
};

struct nanoem_mutable_document_t {
    nanoem_mutable_document_base_object_t base;
    nanoem_document_t *origin;
    nanoem_rsize_t num_allocated_accessories;
    nanoem_rsize_t num_allocated_models;
};

struct nanoem_document_keyframe_compare_result_t {
    nanoem_document_base_keyframe_t *keyframe;
    nanoem_rsize_t index;
    nanoem_rsize_t delta;
};
typedef struct nanoem_document_keyframe_compare_result_t nanoem_document_keyframe_compare_result_t;

static void
nanoemDocumentBaseKeyframeReset(nanoem_document_base_keyframe_t *keyframe)
{
    keyframe->object_index = 0;
    keyframe->frame_index = 0;
    keyframe->previous_keyframe_index = 0;
    keyframe->next_keyframe_index = 0;
    keyframe->is_selected = nanoem_false;
}

static void
nanoemDocumentBaseKeyframeSet(nanoem_document_base_keyframe_t *keyframe, nanoem_frame_index_t frame_index, int index, const nanoem_document_keyframe_compare_result_t *previous, const nanoem_document_keyframe_compare_result_t *next)
{
    keyframe->frame_index = frame_index;
    keyframe->object_index = index;
    if (previous->keyframe) {
        keyframe->previous_keyframe_index = previous->index;
    }
    if (next->keyframe) {
        keyframe->next_keyframe_index = next->index;
    }
}

static void
nanoemDocumentKeyframeCompareResultReset(nanoem_document_keyframe_compare_result_t *result)
{
    result->keyframe = NULL;
    result->index = 0;
    result->delta = NANOEM_RSIZE_MAX;
}

static void
nanoemDocumentKeyframeCompareResultSetKeyframeIndex(nanoem_document_keyframe_compare_result_t *previous, nanoem_document_keyframe_compare_result_t *next, int object_index)
{
    nanoem_document_base_keyframe_t *previous_keyframe = previous->keyframe, *next_keyframe = next->keyframe;
    if (previous_keyframe) {
        previous_keyframe->next_keyframe_index = object_index;
    }
    if (next_keyframe) {
        next_keyframe->previous_keyframe_index = object_index;
    }
}

static nanoem_bool_t
nanoemDocumentBaseKeyframeCompare(nanoem_document_base_keyframe_t *keyframe, nanoem_frame_index_t frame_index,
                                  nanoem_document_keyframe_compare_result_t *previous, nanoem_document_keyframe_compare_result_t *next)
{
    nanoem_frame_index_t current_frame_index = keyframe->frame_index, delta;
    nanoem_bool_t found = nanoem_false;
    if (current_frame_index > frame_index) {
        delta = current_frame_index - frame_index;
        if (delta < next->delta) {
            next->keyframe = keyframe;
            next->index = keyframe->object_index;
            next->delta = delta;
        }
    }
    else if (current_frame_index == frame_index) {
        found = nanoem_true;
    }
    else if (current_frame_index < frame_index) {
        delta = frame_index - current_frame_index;
        if (delta < previous->delta) {
            previous->keyframe = keyframe;
            previous->index = keyframe->object_index;
            previous->delta = delta;
        }
    }
    return found;
}

static nanoem_unicode_string_t *
nanoemDocumentReadFixedStringPMM(const nanoem_document_t *document, nanoem_buffer_t *buffer, char *source, nanoem_rsize_t source_size, nanoem_status_t *status)
{
    const char *data_ptr;
    char *ptr;
    nanoem_unicode_string_t *dst = NULL;
    nanoem_rsize_t length = source_size;
    if (nanoemBufferCanReadLength(buffer, source_size)) {
        nanoem_crt_memset(source, 0, source_size);
        data_ptr = (const char *) nanoemBufferGetDataPtr(buffer);
        ptr = (char *) nanoem_crt_memchr(data_ptr, 0, source_size);
        if (ptr != NULL) {
            *ptr = 0;
            length = ptr - data_ptr;
        }
        nanoemUtilCopyString(source, source_size, data_ptr, length);
        dst = nanoemUnicodeStringFactoryCreateStringWithEncoding(document->factory, (nanoem_u8_t *) source, length, NANOEM_CODEC_TYPE_SJIS, status);
        nanoemBufferSkip(buffer, source_size, status);
    }
    return dst;
}

static nanoem_unicode_string_t *
nanoemDocumentReadVariableStringPMM(const nanoem_document_t *document, nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory = document->factory;
    nanoem_unicode_string_t *dst = NULL;
    nanoem_rsize_t length = nanoemBufferReadByte(buffer, status);
    char *src = NULL;
    if (!nanoem_status_ptr_has_error(status)) {
        src = nanoemBufferReadBuffer(buffer, length, status);
    }
    if (nanoem_is_not_null(src)) {
        dst = nanoemUnicodeStringFactoryCreateStringWithEncoding(factory, (nanoem_u8_t *) src, length, NANOEM_CODEC_TYPE_SJIS, status);
        nanoem_free(src);
    }
    return dst;
}

static void
nanoemDocumentReadAllStringsPMM(const nanoem_document_t *document, nanoem_buffer_t *buffer, nanoem_unicode_string_t ***names_ptr, nanoem_rsize_t *num_objects_ptr, nanoem_status_t *status)
{
    nanoem_unicode_string_t **names;
    nanoem_rsize_t i, num_objects = *num_objects_ptr;
    if (nanoemBufferCanReadLength(buffer, num_objects)) {
        names = (nanoem_unicode_string_t **) nanoem_calloc(num_objects, sizeof(*names), status);
        if (nanoem_is_not_null(names)) {
            for (i = 0; i < num_objects; i++) {
                names[i] = nanoemDocumentReadVariableStringPMM(document, buffer, status);
            }
            *names_ptr = names;
        }
    }
    else if (num_objects > 0) {
        *num_objects_ptr = 0;
    }
}

static void
nanoemDocumentGetAllBoneIndices(nanoem_buffer_t *buffer, int **indices_ptr, nanoem_rsize_t *num_objects_ptr, nanoem_status_t *status)
{
    int *indices;
    nanoem_rsize_t i, num_objects = *num_objects_ptr;
    if (nanoemBufferCanReadLength(buffer, num_objects)) {
        indices = (int *) nanoem_calloc(num_objects, sizeof(*indices), status);
        if (nanoem_is_not_null(indices)) {
            for (i = 0; i < num_objects; i++) {
                indices[i] = nanoemBufferReadInt32LittleEndian(buffer, status);
            }
            *indices_ptr = indices;
        }
    }
    else if (num_objects > 0) {
        *num_objects_ptr = 0;
    }
}

static void
nanoemDocumentGetAllExpansionStates(nanoem_buffer_t *buffer, nanoem_u8_t **indices_ptr, nanoem_rsize_t *num_objects_ptr, nanoem_status_t *status)
{
    nanoem_u8_t *indices;
    nanoem_rsize_t i, num_objects = *num_objects_ptr;
    if (nanoemBufferCanReadLength(buffer, num_objects)) {
        indices = (nanoem_u8_t *) nanoem_calloc(num_objects, sizeof(*indices), status);
        if (nanoem_is_not_null(indices)) {
            for (i = 0; i < num_objects; i++) {
                indices[i] = nanoemBufferReadByte(buffer, status);
            }
            *indices_ptr = indices;
        }
    }
    else if (num_objects > 0) {
        *num_objects_ptr = 0;
    }
}

static void
nanoemDocumentBaseKeyframeParse(nanoem_document_base_keyframe_t *keyframe, nanoem_buffer_t *buffer, nanoem_bool_t include_index, nanoem_status_t *status)
{
    if (include_index) {
        keyframe->object_index = nanoemBufferReadInt32LittleEndian(buffer, status);
    }
    keyframe->frame_index = nanoemBufferReadInt32LittleEndian(buffer, status);
    keyframe->previous_keyframe_index = nanoemBufferReadInt32LittleEndian(buffer, status);
    keyframe->next_keyframe_index = nanoemBufferReadInt32LittleEndian(buffer, status);
}

static void
nanoemDocumentParseInterpolation(nanoem_buffer_t *buffer, nanoem_interpolation_t *parameters, nanoem_status_t *status)
{
    parameters->u.alias.x0 = nanoemBufferReadByte(buffer, status);
    parameters->u.alias.y0 = nanoemBufferReadByte(buffer, status);
    parameters->u.alias.x1 = nanoemBufferReadByte(buffer, status);
    parameters->u.alias.y1 = nanoemBufferReadByte(buffer, status);
}

static const nanoem_document_model_t *
nanoemDocumentResolveModelObject(const nanoem_document_t *document, int index)
{
    const nanoem_document_model_t *model = NULL;
    if (index >= 0 && (nanoem_rsize_t) index < document->num_models) {
        model = document->models[index];
    }
    return model;
}

static const nanoem_unicode_string_t *
nanoemDocumentModelResolveBoneName(const nanoem_document_model_t *model, int index)
{
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(model) && (nanoem_rsize_t) index < model->num_bones) {
        name = model->bone_names[index];
    }
    return name;
}

static const nanoem_unicode_string_t *
nanoemDocumentModelResolveMorphName(const nanoem_document_model_t *model, int index)
{
    const nanoem_unicode_string_t *name = NULL;
    if (nanoem_is_not_null(model) && (nanoem_rsize_t) index < model->num_morphs) {
        name = model->morph_names[index];
    }
    return name;
}

static void
nanoemDocumentAccessoryUnpackOpacityAndVisible(nanoem_u8_t opacity_and_visible, nanoem_f32_t *opacity, nanoem_bool_t *visible)
{
    *opacity = (100 - ((opacity_and_visible & 0xfe) >> 1)) * 0.01f;
    *visible = (opacity_and_visible & 0x1) != 0;
}

NANOEM_DECL_INLINE static const nanoem_document_t *
nanoemDocumentAccessoryGetParentDocument(const nanoem_document_accessory_t *accessory)
{
    return accessory->base.parent.document;
}

NANOEM_DECL_INLINE static const nanoem_document_t *
nanoemDocumentModelGetParentDocument(const nanoem_document_model_t *model)
{
    return model->base.parent.document;
}

static void
nanoemDocumentObjectArrayInsertObject(nanoem_document_object_t ***items, nanoem_document_object_t *item, int index, nanoem_status_t already_exists_error, nanoem_rsize_t *num_items, nanoem_rsize_t *num_allocated_items, nanoem_status_t *status)
{
    nanoem_document_object_t **new_items, **old_items = *items;
    nanoem_rsize_t nb = *num_items;
    int i;
    if (nanoemObjectArrayContainsIndexedObject((const void *const *) old_items, item, nb, item->index)) {
        nanoem_status_ptr_assign(status, already_exists_error);
        return;
    }
    new_items = (nanoem_document_object_t **) nanoemMutableObjectArrayResize(old_items, num_allocated_items, num_items, status);
    if (nanoem_is_not_null(new_items)) {
        nb = *num_items;
        if (index >= 0 && (nanoem_rsize_t) index < nb - 1) {
            nanoem_crt_memmove(&new_items[index + 1], &new_items[index], (nb - index) * sizeof(item));
            for (i = index; i < (int) nb; i++) {
                new_items[i]->index++;
            }
        }
        else {
            index = (int) nb - 1;
        }
        new_items[index] = item;
        *items = new_items;
        item->index = index;
        nanoem_status_ptr_assign_succeeded(status);
    }
}

static void
nanoemDocumentObjectArrayRemoveObject(nanoem_document_object_t **items, nanoem_document_object_t *item, nanoem_rsize_t *num_items, nanoem_status_t not_found_error, nanoem_status_t *status)
{
    nanoem_rsize_t nb = *num_items, i;
    int index = item->index, offset;
    offset = nanoemObjectArrayFindIndexedObjectOffset((const void *const *) items, item, nb, index);
    if (offset >= 0) {
        nb--;
        nanoem_crt_memmove(&items[offset], &items[offset + 1], (nb - offset) * sizeof(item));
        for (i = offset; i < nb; i++) {
            items[i]->index--;
        }
        *num_items = nb;
        nanoem_status_ptr_assign_succeeded(status);
    }
    else {
        nanoem_status_ptr_assign(status, not_found_error);
    }
}

#endif /* NANOEM_DOCUMENT_PRIVATE_H_ */
