/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_PRIVATE_H_
#define NANOEM_PRIVATE_H_

/* THIS IS A PRIVATE DECLARATIONS OF NANOEM_C DO NOT INCLUDE DIRECTLY */

#include "nanoem.h"

#include <stdlib.h>

#define nanoem_stringify(x) nanoem_stringify_(x)
#define nanoem_stringify_(x) #x

#define nanoem_status_ptr_assign(status_ptr, value) \
  do { \
    if (nanoem_likely(status_ptr != NULL)) { \
      *(status_ptr) = (value); \
    } \
  } while (0)

#define nanoem_status_ptr_has_error(status_ptr) \
    (nanoem_likely(status_ptr) ? (nanoem_unlikely(*status_ptr != NANOEM_STATUS_SUCCESS) ? nanoem_true : nanoem_false) : nanoem_false)
#define nanoem_status_ptr_assign_succeeded(status_ptr) \
    nanoem_status_ptr_assign(status_ptr, NANOEM_STATUS_SUCCESS)
#define nanoem_status_ptr_assign_null_object(status_ptr) \
    nanoem_status_ptr_assign(status_ptr, NANOEM_STATUS_ERROR_NULL_OBJECT)
#define nanoem_status_ptr_assign_select(status_ptr, error) \
    nanoem_status_ptr_assign(status_ptr, (nanoem_status_ptr_has_error(status_ptr) ? error : NANOEM_STATUS_SUCCESS))

#define nanoem_crt_memcmp(left, right, size) memcmp((left), (right), (size))
#define nanoem_crt_memcpy(dst, src, size) memcpy((dst), (src), (size))
#define nanoem_crt_strcmp(left, right) strcmp((left), (right))
#define nanoem_crt_strncpy(dst, src, size) strncpy((dst), (src), (size))
#define nanoem_crt_strchr(str, chr) strchr((str), (chr))
#define nanoem_crt_strlen(str) strlen((str))
#define nanoem_crt_qsort(base, count, size, predict) qsort((base), (count), (size), (predict))
#define nanoem_crt_bsearch(key, base, count, size, predict) bsearch((key), (base), (count), (size), (predict))

#if defined(__clang__)
#define nanoem_pragma_diagnostics_push() _Pragma("clang diagnostic push")
#define nanoem_pragma_diagnostics_ignore_clang(x) _Pragma(nanoem_stringify(clang diagnostic ignored x))
#define nanoem_pragma_diagnostics_ignore_clang_gcc(x) nanoem_pragma_diagnostics_ignore_clang(x)
#define nanoem_pragma_diagnostics_ignore_gcc(x)
#define nanoem_pragma_diagnostics_ignore_msvc(x)
#define nanoem_pragma_diagnostics_pop() _Pragma("clang diagnostic pop")
#define nanoem_decl_align16(x) x __attribute__((aligned(16)))
#elif defined(__GNUC__)
#define nanoem_pragma_diagnostics_push() _Pragma("GCC diagnostic push")
#define nanoem_pragma_diagnostics_ignore_clang(x)
#define nanoem_pragma_diagnostics_ignore_gcc(x) _Pragma(nanoem_stringify(GCC diagnostic ignored x))
#define nanoem_pragma_diagnostics_ignore_clang_gcc(x) nanoem_pragma_diagnostics_ignore_gcc(x)
#define nanoem_pragma_diagnostics_ignore_msvc(x)
#define nanoem_pragma_diagnostics_pop() _Pragma("GCC diagnostic pop")
#define nanoem_decl_align16(x) x __attribute__((aligned(16)))
#elif defined(_MSC_VER)
#define nanoem_pragma_diagnostics_push() __pragma(warning(push))
#define nanoem_pragma_diagnostics_ignore_clang(x)
#define nanoem_pragma_diagnostics_ignore_gcc(x)
#define nanoem_pragma_diagnostics_ignore_clang_gcc(x)
#define nanoem_pragma_diagnostics_ignore_msvc(x) __pragma(warning(disable:x))
#define nanoem_pragma_diagnostics_pop() __pragma(warning(pop))
#define nanoem_decl_align16(x) __declspec(align(16)) x
#else
#define nanoem_pragma_diagnostics_push()
#define nanoem_pragma_diagnostics_ignore_clang(x)
#define nanoem_pragma_diagnostics_ignore_gcc(x)
#define nanoem_pragma_diagnostics_ignore_clang_gcc(x)
#define nanoem_pragma_diagnostics_ignore_msvc(x)
#define nanoem_pragma_diagnostics_pop()
#define nanoem_decl_align16(x) x
#endif

#if defined(NANOEM_NO_TLS)
#define NANOEM_DECL_TLS
#elif !defined(NANOEM_DECL_TLS)
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define NANOEM_DECL_TLS _Thread_local
#else
#define NANOEM_DECL_TLS
#endif /* NANOEM_DECL_TLS */
#endif /* NANOEM_NO_TLS */

#ifndef NANOEM_DECL_INTERNAL
#define NANOEM_DECL_INTERNAL NANOEM_DECL_API
#endif

/* size limitation macros */
#define NANOEM_FRAME_INDEX_MAX_SIZE (~(nanoem_frame_index_t)(0))
#define PMD_BONE_NAME_LENGTH 20
#define PMD_BONE_CATEGORY_NAME_LENGTH 50
#define PMD_BONE_MAX_DEPTH 256
#define PMD_JOINT_NAME_LENGTH 20
#define PMD_MATERIAL_DIFFUSE_TEXTURE_NAME_LENGTH 20
#define PMD_MODEL_NAME_LENGTH 20
#define PMD_MODEL_COMMENT_LENGTH 256
#define PMD_TOON_TEXTURE_PATH_LENGTH 100
#define PMD_MORPH_NAME_LENGTH 20
#define PMD_RIGID_BODY_NAME_LENGTH 20
#define VMD_SIGNATURE_SIZE 30
#define VMD_TARGET_MODEL_NAME_LENGTH_V1 10
#define VMD_TARGET_MODEL_NAME_LENGTH_V2 20
#define VMD_BONE_KEYFRAME_NAME_LENGTH 15
#define VMD_MORPH_KEYFRAME_NAME_LENGTH 15
#define VMD_CAMERA_KEYFRAME_TWEAK_LENGTH 61
#define VMD_LIGHT_KEYFRAME_TWEAK_LENGTH 28
#define VMD_SELF_SHADOW_KEYFRAME_TWEAK_LENGTH 9

NANOEM_DECL_TLS NANOEM_DECL_API const nanoem_global_allocator_t *
    __nanoem_global_allocator;
static const int NANOEM_MODEL_OBJECT_NOT_FOUND = -1;
static const int NANOEM_MOTION_OBJECT_NOT_FOUND = -1;

static void *
nanoemMemoryAllocate(nanoem_rsize_t size, nanoem_status_t *status, const char *filename, int line)
{
    void *p = NULL;
    if (nanoem_likely(size <= NANOEM_RSIZE_MAX)) {
        if (__nanoem_global_allocator != NULL) {
            p = __nanoem_global_allocator->malloc(__nanoem_global_allocator->opaque, size, filename, line);
        }
        else {
            p = malloc(size);
        }
    }
    if (nanoem_is_null(p)) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MALLOC_FAILED);
    }
    return p;
}

static void *
nanoemMemoryResize(void *ptr, nanoem_rsize_t size, nanoem_status_t *status, const char *filename, int line)
{
    void *p = NULL;
    if (nanoem_likely(size > 0 && size <= NANOEM_RSIZE_MAX)) {
        if (__nanoem_global_allocator != NULL) {
            p = __nanoem_global_allocator->realloc(__nanoem_global_allocator->opaque, ptr, size, filename, line);
        }
        else {
            p = realloc(ptr, size);
        }
    }
    if (nanoem_is_null(p)) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_REALLOC_FAILED);
    }
    return p;
}

static void *
nanoemMemoryAllocateSafe(nanoem_rsize_t length, nanoem_rsize_t size, nanoem_status_t *status, const char *filename, int line)
{
    void *p = NULL;
    if (nanoem_likely(length > 0 && size > 0 && length <= (NANOEM_RSIZE_MAX / size))) {
        if (__nanoem_global_allocator != NULL) {
            p = __nanoem_global_allocator->calloc(__nanoem_global_allocator->opaque, length, size, filename, line);
        }
        else {
            p = calloc(length, size);
        }
    }
    if (nanoem_is_null(p)) {
        nanoem_status_ptr_assign(status, NANOEM_STATUS_ERROR_MALLOC_FAILED);
    }
    return p;
}

static void
nanoemMemoryRelease(void *ptr, const char *filename, int line)
{
    if (ptr) {
        if (__nanoem_global_allocator != NULL) {
            __nanoem_global_allocator->free(__nanoem_global_allocator->opaque, ptr, filename, line);
        }
        else {
            free(ptr);
        }
    }
}

/* allocation macros */
#ifdef NANOEM_ENABLE_DEBUG_ALLOCATOR
#define nanoem_malloc(size, status) nanoemMemoryAllocate((size), (status), __FILE__, __LINE__)
#define nanoem_realloc(ptr, size, status) nanoemMemoryResize((ptr), (size), (status), __FILE__, __LINE__)
#define nanoem_calloc(length, size, status) nanoemMemoryAllocateSafe((length), (size), (status), __FILE__, __LINE__)
#define nanoem_free(ptr) nanoemMemoryRelease((ptr), __FILE__, __LINE__)
#else
#define nanoem_malloc(size, status) nanoemMemoryAllocate((size), (status), NULL, 0)
#define nanoem_realloc(ptr, size, status) nanoemMemoryResize((ptr), (size), (status), NULL, 0)
#define nanoem_calloc(length, size, status) nanoemMemoryAllocateSafe((length), (size), (status), NULL, 0)
#define nanoem_free(ptr) nanoemMemoryRelease((ptr), NULL, 0)
#endif

/* khash */
#ifdef NANOEM_ENABLE_DEBUG_ALLOCATOR
#define kmalloc(Z) nanoemMemoryAllocate((Z), NULL, __FILE__, __LINE__)
#define krealloc(P, Z) nanoemMemoryResize((P), (Z), (NULL), __FILE__, __LINE__)
#define kcalloc(N, Z) nanoemMemoryAllocateSafe((N), (Z), NULL, __FILE__, __LINE__)
#define kfree(P) nanoemMemoryRelease((P), __FILE__, __LINE__)
#else
#define kmalloc(Z) nanoemMemoryAllocate((Z), NULL, NULL, 0)
#define krealloc(P, Z) nanoemMemoryResize((P), (Z), NULL, NULL, 0)
#define kcalloc(N, Z) nanoemMemoryAllocateSafe((N), (Z), NULL, NULL, 0)
#define kfree(P) nanoemMemoryRelease((P), NULL, 0)
#endif
#include "./khash.h"

struct nanoem_unicode_string_factory_t {
    nanoem_unicode_string_factory_from_codec_t from_cp932;
    nanoem_unicode_string_factory_from_codec_t from_utf8;
    nanoem_unicode_string_factory_from_codec_t from_utf16;
    nanoem_unicode_string_factory_to_codec_t to_cp932;
    nanoem_unicode_string_factory_to_codec_t to_utf8;
    nanoem_unicode_string_factory_to_codec_t to_utf16;
    nanoem_unicode_string_factory_hash_t hash;
    nanoem_unicode_string_factory_compare_t compare;
    nanoem_unicode_string_factory_get_cache_t get_cache;
    nanoem_unicode_string_factory_set_cache_t set_cache;
    nanoem_unicode_string_factory_destroy_string_t destroy_string;
    nanoem_unicode_string_factory_destroy_byte_array_t destroy_byte_array;
    void *opaque_data;
};

nanoem_decl_align16(static const) nanoem_f32_t
    __nanoem_null_vector3[] = { 0, 0, 0, 0 };
nanoem_decl_align16(static const) nanoem_f32_t
    __nanoem_null_vector4[] = { 0, 0, 0, 0 };

static const nanoem_u8_t
    __nanoem_default_interpolation[] = { 20, 20, 107, 107 };
static const char
    __nanoem_pmd_signature[] = "Pmd";
static const char
    __nanoem_vmd_signature_type1[] = "Vocaloid Motion Data file";
static const char
    __nanoem_vmd_signature_type2[] = "Vocaloid Motion Data 0002";

KHASH_INIT(annotation, char *, char *, 1, kh_str_hash_func, kh_str_hash_equal)
KHASH_INIT(string_cache, char *, nanoem_i32_t, 1, kh_str_hash_func, kh_str_hash_equal)
KHASH_MAP_INIT_INT(keyframe_map, nanoem_motion_keyframe_object_t *)

enum {
    nanoem_false,
    nanoem_true
};

typedef struct nanoem_motion_track_t nanoem_motion_track_t;
typedef int nanoem_motion_track_index_t;
struct nanoem_motion_track_t {
    nanoem_motion_track_index_t id;
    nanoem_unicode_string_factory_t *factory;
    nanoem_unicode_string_t *name;
    kh_keyframe_map_t *keyframes;
};
#define nanoem_motion_track_hash_equal(a, b) ((a).factory->compare((a).factory->opaque_data, (a).name, (b).name) == 0)
#define nanoem_motion_track_hash_func(a) ((a).factory->hash((a).factory->opaque_data, (a).name))
KHASH_INIT(motion_track_bundle, nanoem_motion_track_t, char, 0, nanoem_motion_track_hash_func, nanoem_motion_track_hash_equal)


struct nanoem_f128_components_t {
    float x;
    float y;
    float z;
    float w;
};

typedef struct nanoem_f128_t nanoem_f128_t;
nanoem_decl_align16(struct)
nanoem_f128_t {
    float values[4];
};

struct nanoem_buffer_t {
    const nanoem_u8_t *data;
    nanoem_rsize_t length;
    nanoem_rsize_t offset;
};

typedef struct nanoem_info_t nanoem_info_t;
struct nanoem_info_t {
    unsigned int codec_type : 2;
    unsigned int additional_uv_size : 3;
    unsigned int vertex_index_size : 3;
    unsigned int texture_index_size : 3;
    unsigned int material_index_size : 3;
    unsigned int bone_index_size : 3;
    unsigned int morph_index_size : 3;
    unsigned int rigid_body_index_size : 3;
    unsigned int padding : 1;
};

struct nanoem_model_t {
    nanoem_f32_t version;
    nanoem_u8_t info_length;
    nanoem_info_t info;
    nanoem_unicode_string_factory_t *factory;
    nanoem_unicode_string_t *name_ja;
    nanoem_unicode_string_t *name_en;
    nanoem_unicode_string_t *comment_ja;
    nanoem_unicode_string_t *comment_en;
    nanoem_rsize_t num_vertices;
    nanoem_model_vertex_t **vertices;
    nanoem_rsize_t num_vertex_indices;
    nanoem_u32_t *vertex_indices;
    nanoem_rsize_t num_materials;
    nanoem_model_material_t **materials;
    nanoem_rsize_t num_bones;
    nanoem_model_bone_t **bones;
    nanoem_model_bone_t **ordered_bones;
    nanoem_rsize_t num_constraints;
    nanoem_model_constraint_t **constraints;
    nanoem_rsize_t num_textures;
    nanoem_model_texture_t **textures;
    nanoem_rsize_t num_morphs;
    nanoem_model_morph_t **morphs;
    nanoem_rsize_t num_labels;
    nanoem_model_label_t **labels;
    nanoem_rsize_t num_rigid_bodies;
    nanoem_model_rigid_body_t **rigid_bodies;
    nanoem_rsize_t num_joints;
    nanoem_model_joint_t **joints;
    nanoem_rsize_t num_soft_bodies;
    nanoem_model_soft_body_t **soft_bodies;
    nanoem_user_data_t *user_data;
};

struct nanoem_model_object_t {
    int index;
    union {
        const nanoem_model_t *model;
        const nanoem_model_constraint_t *constraint;
        const nanoem_model_morph_t *morph;
        const nanoem_model_label_t *label;
        const nanoem_model_soft_body_t *soft_body;
    } parent;
    nanoem_user_data_t *user_data;
};

struct nanoem_model_vertex_t {
    nanoem_model_object_t base;
    nanoem_f128_t origin;
    nanoem_f128_t normal;
    nanoem_f128_t uv;
    nanoem_f128_t additional_uv[4];
    nanoem_model_vertex_type_t type;
    nanoem_rsize_t num_bone_indices;
    int bone_indices[4];
    nanoem_rsize_t num_bone_weights;
    nanoem_f128_t bone_weights;
    nanoem_f128_t sdef_c;
    nanoem_f128_t sdef_r0;
    nanoem_f128_t sdef_r1;
    nanoem_f32_t edge_size;
    nanoem_u8_t bone_weight_origin;
};
NANOEM_STATIC_ASSERT(offsetof(nanoem_model_vertex_t, base) == 0, "vertex_t inherits model_object_t");

struct nanoem_model_material_t {
    nanoem_model_object_t base;
    nanoem_unicode_string_t *name_ja;
    nanoem_unicode_string_t *name_en;
    nanoem_f128_t diffuse_color;
    nanoem_f32_t diffuse_opacity;
    nanoem_f32_t specular_power;
    nanoem_f128_t specular_color;
    nanoem_f128_t ambient_color;
    nanoem_f128_t edge_color;
    nanoem_f32_t edge_opacity;
    nanoem_f32_t edge_size;
    int diffuse_texture_index;
    int sphere_map_texture_index;
    int toon_texture_index;
    nanoem_model_material_sphere_map_texture_type_t sphere_map_texture_type;
    nanoem_bool_t is_toon_shared;
    nanoem_rsize_t num_vertex_indices;
    union nanoem_model_material_flags_union_t {
        struct nanoem_model_material_flags_t {
            unsigned int is_culling_disabled : 1;
            unsigned int is_casting_shadow_enabled : 1;
            unsigned int is_casting_shadow_map_enabled : 1;
            unsigned int is_shadow_map_enabled : 1;
            unsigned int is_edge_enabled : 1;
            unsigned int is_vertex_color_enabled : 1;
            unsigned int is_point_draw_enabled : 1;
            unsigned int is_line_draw_enabled : 1;
        } flags;
        nanoem_u8_t value;
    } u;
    nanoem_model_texture_t *sphere_map_texture_sph;
    nanoem_model_texture_t *sphere_map_texture_spa;
    nanoem_model_texture_t *diffuse_texture;
    nanoem_unicode_string_t *clob;
};
NANOEM_STATIC_ASSERT(offsetof(nanoem_model_material_t, base) == 0, "material_t inherits model_object_t");

enum nanoem_model_bone_type_t {
    NANOEM_MOTION_BONE_TYPE_ROTATABLE,
    NANOEM_MOTION_BONE_TYPE_ROTATABLE_AND_MOVABLE,
    NANOEM_MOTION_BONE_TYPE_CONSTRAINT_EFFECTOR,
    NANOEM_MOTION_BONE_TYPE_UNKNOWN,
    NANOEM_MOTION_BONE_TYPE_CONSTRAINT_JOINT,
    NANOEM_MOTION_BONE_TYPE_INHERENT_ORIENTATION_JOINT,
    NANOEM_MOTION_BONE_TYPE_CONSTRAINT_ROOT,
    NANOEM_MOTION_BONE_TYPE_INVISIBLE,
    NANOEM_MOTION_BONE_TYPE_FIXED_AXIS,
    NANOEM_MOTION_BONE_TYPE_INHERENT_ORIENTATION_EFFECTOR,
    NANOEM_MOTION_BONE_TYPE_MAX_ENUM
};
typedef enum nanoem_model_bone_type_t nanoem_model_bone_type_t;

struct nanoem_model_bone_t {
    nanoem_model_object_t base;
    nanoem_unicode_string_t *name_ja;
    nanoem_unicode_string_t *name_en;
    nanoem_model_constraint_t *constraint;
    nanoem_f128_t origin;
    nanoem_f128_t destination_origin;
    nanoem_f128_t fixed_axis;
    nanoem_f128_t local_x_axis;
    nanoem_f128_t local_z_axis;
    nanoem_f32_t inherent_coefficient;
    int parent_bone_index;
    int parent_inherent_bone_index;
    int effector_bone_index;
    int target_bone_index;
    int global_bone_index;
    int stage_index;
    nanoem_model_bone_type_t type;
    union nanoem_model_bone_flags_union_t {
        struct nanoem_model_bone_flags_t {
            unsigned int has_destination_bone_index : 1;
            unsigned int is_rotateable : 1;
            unsigned int is_movable : 1;
            unsigned int is_visible : 1;
            unsigned int is_user_handleable : 1;
            unsigned int has_constraint : 1;
            unsigned int padding_1 : 1;
            unsigned int has_local_inherent : 1;
            unsigned int has_inherent_orientation : 1;
            unsigned int has_inherent_translation : 1;
            unsigned int has_fixed_axis : 1;
            unsigned int has_local_axes : 1;
            unsigned int is_affected_by_physics_simulation : 1;
            unsigned int has_external_parent_bone : 1;
            unsigned int padding_2 : 2;
        } flags;
        nanoem_u16_t value;
    } u;
};
NANOEM_STATIC_ASSERT(offsetof(nanoem_model_bone_t, base) == 0, "bone_t inherits model_object_t");

struct nanoem_model_constraint_joint_t {
    nanoem_model_object_t base;
    int bone_index;
    nanoem_bool_t has_angle_limit;
    nanoem_f128_t lower_limit;
    nanoem_f128_t upper_limit;
};

struct nanoem_model_constraint_t {
    nanoem_model_object_t base;
    int effector_bone_index;
    int target_bone_index;
    int num_iterations;
    nanoem_rsize_t num_joints;
    nanoem_f32_t angle_limit;
    nanoem_model_constraint_joint_t **joints;
};
NANOEM_STATIC_ASSERT(offsetof(nanoem_model_constraint_t, base) == 0, "constraint_t inherits model_object_t");

struct nanoem_model_morph_group_t {
    nanoem_model_object_t base;
    int morph_index;
    nanoem_f32_t weight;
};

struct nanoem_model_morph_vertex_t {
    nanoem_model_object_t base;
    int vertex_index;
    int relative_index;
    nanoem_f128_t position;
};

struct nanoem_model_morph_bone_t {
    nanoem_model_object_t base;
    int bone_index;
    nanoem_f128_t translation;
    nanoem_f128_t orientation;
};

struct nanoem_model_morph_uv_t {
    nanoem_model_object_t base;
    const nanoem_model_morph_t *parent_morph;
    int vertex_index;
    nanoem_f128_t position;
};

struct nanoem_model_morph_material_t {
    nanoem_model_object_t base;
    int material_index;
    nanoem_model_morph_material_operation_type_t operation;
    nanoem_f128_t diffuse_color;
    nanoem_f32_t diffuse_opacity;
    nanoem_f128_t specular_color;
    nanoem_f32_t specular_power;
    nanoem_f128_t ambient_color;
    nanoem_f128_t edge_color;
    nanoem_f32_t edge_opacity;
    nanoem_f32_t edge_size;
    nanoem_f128_t diffuse_texture_blend;
    nanoem_f128_t sphere_map_texture_blend;
    nanoem_f128_t toon_texture_blend;
};

struct nanoem_model_morph_flip_t {
    nanoem_model_object_t base;
    int morph_index;
    nanoem_f32_t weight;
};

struct nanoem_model_morph_impulse_t {
    nanoem_model_object_t base;
    int rigid_body_index;
    nanoem_bool_t is_local;
    nanoem_f128_t velocity;
    nanoem_f128_t torque;
};

struct nanoem_model_morph_t {
    nanoem_model_object_t base;
    nanoem_unicode_string_t *name_ja;
    nanoem_unicode_string_t *name_en;
    nanoem_model_morph_type_t type;
    nanoem_model_morph_category_t category;
    nanoem_rsize_t num_objects;
    union {
        nanoem_model_morph_group_t **groups;
        nanoem_model_morph_vertex_t **vertices;
        nanoem_model_morph_bone_t **bones;
        nanoem_model_morph_uv_t **uvs;
        nanoem_model_morph_material_t **materials;
        nanoem_model_morph_flip_t **flips;
        nanoem_model_morph_impulse_t **impulses;
    } u;
};
NANOEM_STATIC_ASSERT(offsetof(nanoem_model_morph_t, base) == 0, "morph_t inherits model_object_t");

struct nanoem_model_label_item_t {
    nanoem_model_object_t base;
    nanoem_model_label_item_type_t type;
    union {
        nanoem_model_bone_t *bone;
        nanoem_model_morph_t *morph;
    } u;
};

struct nanoem_model_label_t {
    nanoem_model_object_t base;
    nanoem_unicode_string_t *name_ja;
    nanoem_unicode_string_t *name_en;
    nanoem_bool_t is_special;
    nanoem_rsize_t num_items;
    nanoem_model_label_item_t **items;
};
NANOEM_STATIC_ASSERT(offsetof(nanoem_model_label_t, base) == 0, "label_t inherits model_object_t");

struct nanoem_model_rigid_body_t {
    nanoem_model_object_t base;
    nanoem_unicode_string_t *name_ja;
    nanoem_unicode_string_t *name_en;
    int bone_index;
    int collision_group_id;
    int collision_mask;
    nanoem_model_rigid_body_shape_type_t shape_type;
    nanoem_f128_t size;
    nanoem_f128_t origin;
    nanoem_f128_t orientation;
    nanoem_f32_t mass;
    nanoem_f32_t linear_damping;
    nanoem_f32_t angular_damping;
    nanoem_f32_t restitution;
    nanoem_f32_t friction;
    nanoem_model_rigid_body_transform_type_t transform_type;
    nanoem_bool_t is_bone_relative;
};
NANOEM_STATIC_ASSERT(offsetof(nanoem_model_rigid_body_t, base) == 0, "rigid_body_t inherits model_object_t");

struct nanoem_model_joint_t {
    nanoem_model_object_t base;
    nanoem_unicode_string_t *name_ja;
    nanoem_unicode_string_t *name_en;
    int rigid_body_a_index;
    int rigid_body_b_index;
    nanoem_model_joint_type_t type;
    nanoem_f128_t origin;
    nanoem_f128_t orientation;
    nanoem_f128_t linear_lower_limit;
    nanoem_f128_t linear_upper_limit;
    nanoem_f128_t angular_lower_limit;
    nanoem_f128_t angular_upper_limit;
    nanoem_f128_t linear_stiffness;
    nanoem_f128_t angular_stiffness;
};
NANOEM_STATIC_ASSERT(offsetof(nanoem_model_joint_t, base) == 0, "joint_t inherits model_object_t");

struct nanoem_model_soft_body_t {
    nanoem_model_object_t base;
    nanoem_unicode_string_t *name_ja;
    nanoem_unicode_string_t *name_en;
    nanoem_model_soft_body_shape_type_t shape_type;
    int material_index;
    nanoem_u8_t collision_group_id;
    nanoem_u16_t collision_mask;
    nanoem_u8_t flags;
    int bending_constraints_distance;
    int cluster_count;
    nanoem_f32_t total_mass;
    nanoem_f32_t collision_margin;
    nanoem_model_soft_body_aero_model_type_t aero_model;
    nanoem_f32_t velocity_correction_factor;
    nanoem_f32_t damping_coefficient;
    nanoem_f32_t drag_coefficient;
    nanoem_f32_t lift_coefficient;
    nanoem_f32_t pressure_coefficient;
    nanoem_f32_t volume_conversation_coefficient;
    nanoem_f32_t dynamic_friction_coefficient;
    nanoem_f32_t pose_matching_coefficient;
    nanoem_f32_t rigid_contact_hardness;
    nanoem_f32_t kinetic_contact_hardness;
    nanoem_f32_t soft_contact_hardness;
    nanoem_f32_t anchor_hardness;
    nanoem_f32_t soft_vs_rigid_hardness;
    nanoem_f32_t soft_vs_kinetic_hardness;
    nanoem_f32_t soft_vs_soft_hardness;
    nanoem_f32_t soft_vs_rigid_impulse_split;
    nanoem_f32_t soft_vs_kinetic_impulse_split;
    nanoem_f32_t soft_vs_soft_impulse_split;
    int velocity_solver_iterations;
    int positions_solver_iterations;
    int drift_solver_iterations;
    int cluster_solver_iterations;
    nanoem_f32_t linear_stiffness_coefficient;
    nanoem_f32_t angular_stiffness_coefficient;
    nanoem_f32_t volume_stiffness_coefficient;
    nanoem_rsize_t num_anchors;
    nanoem_model_soft_body_anchor_t **anchors;
    nanoem_rsize_t num_pinned_vertex_indices;
    nanoem_u32_t *pinned_vertex_indices;
};
NANOEM_STATIC_ASSERT(offsetof(nanoem_model_soft_body_t, base) == 0, "soft_body_t inherits model_object_t");

struct nanoem_model_soft_body_anchor_t {
    nanoem_model_object_t base;
    int rigid_body_index;
    int vertex_index;
    nanoem_bool_t is_near_enabled;
};
NANOEM_STATIC_ASSERT(offsetof(nanoem_model_soft_body_anchor_t, base) == 0, "soft_body_anchor_t inherits model_object_t");

struct nanoem_model_texture_t {
    nanoem_model_object_t base;
    nanoem_unicode_string_t *path;
};
NANOEM_STATIC_ASSERT(offsetof(nanoem_model_texture_t, base) == 0, "texture_t inherits model_object_t");

struct nanoem_motion_t {
    nanoem_unicode_string_factory_t *factory;
    kh_annotation_t *annotations;
    nanoem_unicode_string_t *target_model_name;
    nanoem_rsize_t num_accessory_keyframes;
    nanoem_motion_accessory_keyframe_t **accessory_keyframes;
    nanoem_rsize_t num_bone_keyframes;
    nanoem_motion_bone_keyframe_t **bone_keyframes;
    nanoem_rsize_t num_morph_keyframes;
    nanoem_motion_morph_keyframe_t **morph_keyframes;
    nanoem_rsize_t num_camera_keyframes;
    nanoem_motion_camera_keyframe_t **camera_keyframes;
    nanoem_rsize_t num_light_keyframes;
    nanoem_motion_light_keyframe_t **light_keyframes;
    nanoem_rsize_t num_model_keyframes;
    nanoem_motion_self_shadow_keyframe_t **self_shadow_keyframes;
    nanoem_rsize_t num_self_shadow_keyframes;
    nanoem_motion_model_keyframe_t **model_keyframes;
    nanoem_motion_track_index_t local_bone_motion_track_allocated_id;
    kh_motion_track_bundle_t *local_bone_motion_track_bundle;
    nanoem_motion_track_index_t local_morph_motion_track_allocated_id;
    kh_motion_track_bundle_t *local_morph_motion_track_bundle;
    nanoem_motion_track_index_t global_motion_track_allocated_id;
    kh_motion_track_bundle_t *global_motion_track_bundle;
    nanoem_motion_format_type_t type;
    nanoem_frame_index_t max_frame_index;
    nanoem_f32_t preferred_fps;
    nanoem_user_data_t *user_data;
};

struct nanoem_motion_keyframe_object_t {
    int index;
    nanoem_motion_t *parent_motion;
    nanoem_frame_index_t frame_index;
    nanoem_bool_t is_selected;
    nanoem_user_data_t *user_data;
    kh_annotation_t *annotations;
};

enum nanoem_parent_keyframe_type_t {
    NANOEM_PARENT_KEYFRAME_TYPE_ACCESSORY,
    NANOEM_PARENT_KEYFRAME_TYPE_MODEL,
    NANOEM_PARENT_KEYFRAME_TYPE_CAMERA
};
typedef enum nanoem_parent_keyframe_type_t nanoem_parent_keyframe_type_t;

struct nanoem_motion_effect_parameter_t {
    nanoem_motion_track_index_t parameter_id;
    nanoem_parent_keyframe_type_t keyframe_type;
    union {
        nanoem_motion_accessory_keyframe_t *accessory;
        nanoem_motion_model_keyframe_t *model;
    } keyframe;
    nanoem_motion_effect_parameter_type_t value_type;
    union {
        int i;
        nanoem_f32_t f;
        nanoem_f128_t v;
    } value;
};

struct nanoem_motion_outside_parent_t {
    nanoem_parent_keyframe_type_t keyframe_type;
    union {
        nanoem_motion_accessory_keyframe_t *accessory;
        nanoem_motion_camera_keyframe_t *camera;
        nanoem_motion_model_keyframe_t *model;
    } keyframe;
    nanoem_motion_track_index_t global_model_track_index;
    nanoem_motion_track_index_t global_bone_track_index;
    nanoem_motion_track_index_t local_bone_track_index;
};

struct nanoem_motion_accessory_keyframe_t {
    nanoem_motion_keyframe_object_t base;
    nanoem_f128_t translation;
    nanoem_f128_t orientation;
    nanoem_f32_t scale_factor;
    nanoem_f32_t opacity;
    nanoem_bool_t is_add_blending_enabled;
    nanoem_bool_t is_shadow_enabled;
    nanoem_bool_t visible;
    nanoem_rsize_t num_effect_parameters;
    nanoem_motion_effect_parameter_t **effect_parameters;
    nanoem_motion_track_index_t accessory_id;
    nanoem_motion_outside_parent_t *outside_parent;
};
NANOEM_STATIC_ASSERT(offsetof(nanoem_motion_accessory_keyframe_t, base) == 0, "accessory_keyframe_t inherits keyframe_object_t");

typedef struct nanoem_keyframe_interpolation_t nanoem_interpolation_t;
struct nanoem_keyframe_interpolation_t {
    union {
        struct {
            nanoem_u8_t x0;
            nanoem_u8_t y0;
            nanoem_u8_t x1;
            nanoem_u8_t y1;
        } alias;
        nanoem_u8_t values[4];
    } u;
};

struct nanoem_motion_bone_keyframe_t {
    nanoem_motion_keyframe_object_t base;
    nanoem_f128_t translation;
    nanoem_f128_t orientation;
    nanoem_interpolation_t interplation[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM];
    nanoem_motion_track_index_t bone_id;
    nanoem_u32_t stage_index;
    nanoem_bool_t is_physics_simulation_enabled;
};
NANOEM_STATIC_ASSERT(offsetof(nanoem_motion_bone_keyframe_t, base) == 0, "bone_keyframe_t inherits keyframe_object_t");

struct nanoem_motion_camera_keyframe_t {
    nanoem_motion_keyframe_object_t base;
    nanoem_f128_t look_at;
    nanoem_f128_t angle;
    nanoem_f32_t distance;
    int fov;
    nanoem_interpolation_t interplation[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM];
    nanoem_bool_t is_perspective_view;
    nanoem_u32_t stage_index;
    nanoem_motion_outside_parent_t *outside_parent;
};
NANOEM_STATIC_ASSERT(offsetof(nanoem_motion_camera_keyframe_t, base) == 0, "camera_keyframe_t inherits keyframe_object_t");

struct nanoem_motion_light_keyframe_t {
    nanoem_motion_keyframe_object_t base;
    nanoem_f128_t color;
    nanoem_f128_t direction;
};
NANOEM_STATIC_ASSERT(offsetof(nanoem_motion_light_keyframe_t, base) == 0, "light_keyframe_t inherits keyframe_object_t");

struct nanoem_motion_model_keyframe_constraint_state_t {
    const nanoem_motion_model_keyframe_t *parent_keyframe;
    nanoem_motion_track_index_t bone_id;
    nanoem_bool_t enabled;
};

struct nanoem_motion_model_keyframe_t {
    nanoem_motion_keyframe_object_t base;
    nanoem_bool_t visible;
    nanoem_rsize_t num_constraint_states;
    nanoem_motion_model_keyframe_constraint_state_t **constraint_states;
    nanoem_rsize_t num_effect_parameters;
    nanoem_motion_effect_parameter_t **effect_parameters;
    nanoem_rsize_t num_outside_parents;
    nanoem_motion_outside_parent_t **outside_parents;
    nanoem_bool_t has_edge_option;
    nanoem_f32_t edge_scale_factor;
    nanoem_f128_t edge_color;
    nanoem_bool_t is_add_blending_enabled;
    nanoem_bool_t is_physics_simulation_enabled;
};
NANOEM_STATIC_ASSERT(offsetof(nanoem_motion_model_keyframe_t, base) == 0, "model_keyframe_t inherits keyframe_object_t");

struct nanoem_motion_morph_keyframe_t {
    nanoem_motion_keyframe_object_t base;
    nanoem_f32_t weight;
    nanoem_motion_track_index_t morph_id;
};
NANOEM_STATIC_ASSERT(offsetof(nanoem_motion_morph_keyframe_t, base) == 0, "morph_keyframe_t inherits keyframe_object_t");

struct nanoem_motion_self_shadow_keyframe_t {
    nanoem_motion_keyframe_object_t base;
    nanoem_f32_t distance;
    int mode;
};
NANOEM_STATIC_ASSERT(offsetof(nanoem_motion_self_shadow_keyframe_t, base) == 0, "self_shadow_keyframe_t inherits keyframe_object_t");

enum nanoem_user_data_destroy_callback_type_t {
    NANOEM_USER_DATA_DESTROY_CALLBACK_NONE,
    NANOEM_USER_DATA_DESTROY_CALLBACK_MODEL,
    NANOEM_USER_DATA_DESTROY_CALLBACK_MOTION,
    NANOEM_USER_DATA_DESTROY_CALLBACK_MODEL_OBJECT,
    NANOEM_USER_DATA_DESTROY_CALLBACK_KEYFRAME_OBJECT,
    NANOEM_USER_DATA_DESTROY_CALLBACK_MAX_ENUM
};

struct nanoem_user_data_t {
    union nanoem_user_data_destroy_callback_union_t {
        nanoem_user_data_on_destroy_model_t model;
        nanoem_user_data_on_destroy_motion_t motion;
        nanoem_user_data_on_destroy_model_object_t model_object;
        nanoem_user_data_on_destroy_keyframe_object_t keyframe_object;
    } destroy;
    enum nanoem_user_data_destroy_callback_type_t type;
    void *opaque;
};

NANOEM_DECL_INTERNAL int
nanoemModelCompareBonePMD(const void *a, const void *b);
NANOEM_DECL_INTERNAL int
nanoemModelCompareBonePMX(const void *a, const void *b);

NANOEM_DECL_INTERNAL nanoem_model_vertex_t *
nanoemModelVertexCreate(const nanoem_model_t *model, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelVertexParsePMD(nanoem_model_vertex_t *vertex, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelVertexParsePMX(nanoem_model_vertex_t *vertex, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelVertexDestroy(nanoem_model_vertex_t *vertex);

NANOEM_DECL_INTERNAL nanoem_model_material_t *
nanoemModelMaterialCreate(const nanoem_model_t *model, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelMaterialParsePMD(nanoem_model_material_t *material, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelMaterialParsePMX(nanoem_model_material_t *material, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelMaterialDestroy(nanoem_model_material_t *material);

NANOEM_DECL_INTERNAL nanoem_model_bone_t *
nanoemModelBoneCreate(const nanoem_model_t *model, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelBoneParsePMD(nanoem_model_bone_t *bone, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelBoneParsePMX(nanoem_model_bone_t *bone, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelBoneDestroy(nanoem_model_bone_t *bone);

NANOEM_DECL_INTERNAL nanoem_model_constraint_t *
nanoemModelConstraintCreate(const nanoem_model_t *model, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelConstraintParsePMD(nanoem_model_constraint_t *constraint, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelConstraintParsePMX(nanoem_model_constraint_t *constraint, nanoem_model_bone_t *bone, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelConstraintDestroy(nanoem_model_constraint_t *constraint);
NANOEM_DECL_INTERNAL nanoem_model_constraint_joint_t *
nanoemModelConstraintJointCreate(const nanoem_model_constraint_t *constraint, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelConstraintJointDestroy(nanoem_model_constraint_joint_t *joint);

NANOEM_DECL_INTERNAL nanoem_model_texture_t *
nanoemModelTextureCreate(const nanoem_model_t *model, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelTextureParsePMD(nanoem_model_texture_t *texture, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelTextureParsePMX(nanoem_model_texture_t *texture, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelTextureDestroy(nanoem_model_texture_t *texture);

NANOEM_DECL_INTERNAL nanoem_model_morph_bone_t *
nanoemModelMorphBoneCreate(const nanoem_model_morph_t *parent, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelMorphBoneDestroy(nanoem_model_morph_bone_t *morph);
NANOEM_DECL_INTERNAL nanoem_model_morph_flip_t *
nanoemModelMorphFlipCreate(const nanoem_model_morph_t *parent, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelMorphFlipDestroy(nanoem_model_morph_flip_t *morph);
NANOEM_DECL_INTERNAL nanoem_model_morph_group_t *
nanoemModelMorphGroupCreate(const nanoem_model_morph_t *parent, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelMorphGroupDestroy(nanoem_model_morph_group_t *morph);
NANOEM_DECL_INTERNAL nanoem_model_morph_impulse_t *
nanoemModelMorphImpulseCreate(const nanoem_model_morph_t *parent, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelMorphImpulseDestroy(nanoem_model_morph_impulse_t *morph);
NANOEM_DECL_INTERNAL nanoem_model_morph_material_t *
nanoemModelMorphMaterialCreate(const nanoem_model_morph_t *parent, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelMorphMaterialDestroy(nanoem_model_morph_material_t *morph);
NANOEM_DECL_INTERNAL nanoem_model_morph_uv_t *
nanoemModelMorphUVCreate(const nanoem_model_morph_t *parent, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelMorphUVDestroy(nanoem_model_morph_uv_t *morph);
NANOEM_DECL_INTERNAL nanoem_model_morph_vertex_t *
nanoemModelMorphVertexCreate(const nanoem_model_morph_t *parent, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelMorphVertexDestroy(nanoem_model_morph_vertex_t *morph);
NANOEM_DECL_INTERNAL nanoem_model_morph_t *
nanoemModelMorphCreate(const nanoem_model_t *model, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelMorphParsePMD(nanoem_model_morph_t *morph, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelMorphParsePMX(nanoem_model_morph_t *morph, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelMorphDestroy(nanoem_model_morph_t *morph);

NANOEM_DECL_INTERNAL nanoem_model_label_item_t *
nanoemModelLabelItemCreate(const nanoem_model_label_t *label, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelLabelItemDestroy(nanoem_model_label_item_t *item);
NANOEM_DECL_INTERNAL nanoem_model_label_t *
nanoemModelLabelCreate(const nanoem_model_t *model, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelLabelParsePMX(nanoem_model_label_t *label, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelLabelDestroy(nanoem_model_label_t *label);

NANOEM_DECL_INTERNAL nanoem_model_rigid_body_t *
nanoemModelRigidBodyCreate(const nanoem_model_t *model, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelRigidBodyParsePMD(nanoem_model_rigid_body_t *rigid_body, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelRigidBodyParsePMX(nanoem_model_rigid_body_t *rigid_body, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelRigidBodyDestroy(nanoem_model_rigid_body_t *rigid_body);

NANOEM_DECL_INTERNAL nanoem_model_joint_t *
nanoemModelJointCreate(const nanoem_model_t *model, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelJointParsePMD(nanoem_model_joint_t *joint, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelJointParsePMX(nanoem_model_joint_t *joint, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelJointDestroy(nanoem_model_joint_t *joint);

NANOEM_DECL_INTERNAL nanoem_model_soft_body_anchor_t *
nanoemModelSoftBodyAnchorCreate(const nanoem_model_soft_body_t *parent, nanoem_status_t *status);
NANOEM_DECL_INTERNAL nanoem_model_soft_body_t *
nanoemModelSoftBodyCreate(const nanoem_model_t *model, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelSoftBodyParsePMX(nanoem_model_soft_body_t *soft_body, nanoem_buffer_t *buffer, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemModelSoftBodyAnchorDestroy(nanoem_model_soft_body_anchor_t *anchor);
NANOEM_DECL_INTERNAL void
nanoemModelSoftBodyDestroy(nanoem_model_soft_body_t *soft_body);

NANOEM_DECL_INTERNAL nanoem_motion_effect_parameter_t *
nanoemMotionEffectParameterCreateFromAccessoryKeyframe(nanoem_motion_accessory_keyframe_t *keyframe, nanoem_status_t *status);
NANOEM_DECL_INTERNAL nanoem_motion_effect_parameter_t *
nanoemMotionEffectParameterCreateFromModelKeyframe(nanoem_motion_model_keyframe_t *keyframe, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemMotionEffectParameterDestroy(nanoem_motion_effect_parameter_t *parameter);

NANOEM_DECL_INTERNAL nanoem_motion_outside_parent_t *
nanoemMotionOutsideParentCreateFromAccessoryKeyframe(nanoem_motion_accessory_keyframe_t *keyframe, nanoem_status_t *status);
NANOEM_DECL_INTERNAL nanoem_motion_outside_parent_t *
nanoemMotionOutsideParentCreateFromCameraKeyframe(nanoem_motion_camera_keyframe_t *keyframe, nanoem_status_t *status);
NANOEM_DECL_INTERNAL nanoem_motion_outside_parent_t *
nanoemMotionOutsideParentCreateFromModelKeyframe(nanoem_motion_model_keyframe_t *keyframe, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemMotionOutsideParentDestroy(nanoem_motion_outside_parent_t *op);

NANOEM_DECL_INTERNAL nanoem_motion_accessory_keyframe_t *
nanoemMotionAccessoryKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemMotionAccessoryKeyframeDestroy(nanoem_motion_accessory_keyframe_t *keyframe);

NANOEM_DECL_INTERNAL nanoem_motion_bone_keyframe_t *
nanoemMotionBoneKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemMotionBoneKeyframeParseVMD(nanoem_motion_bone_keyframe_t *keyframe, nanoem_buffer_t *buffer, kh_string_cache_t *cache, nanoem_frame_index_t offset, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemMotionBoneKeyframeDestroy(nanoem_motion_bone_keyframe_t *keyframe);

NANOEM_DECL_INTERNAL nanoem_motion_camera_keyframe_t *
nanoemMotionCameraKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemMotionCameraKeyframeParseVMD(nanoem_motion_camera_keyframe_t *keyframe, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemMotionCameraKeyframeDestroy(nanoem_motion_camera_keyframe_t *keyframe);

NANOEM_DECL_INTERNAL nanoem_motion_light_keyframe_t *
nanoemMotionLightKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemMotionLightKeyframeParseVMD(nanoem_motion_light_keyframe_t *keyframe, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemMotionLightKeyframeDestroy(nanoem_motion_light_keyframe_t *keyframe);

NANOEM_DECL_INTERNAL nanoem_motion_model_keyframe_t *
nanoemMotionModelKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemMotionModelKeyframeParseVMD(nanoem_motion_model_keyframe_t *keyframe, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemMotionModelKeyframeDestroy(nanoem_motion_model_keyframe_t *keyframe);

NANOEM_DECL_INTERNAL nanoem_motion_morph_keyframe_t *
nanoemMotionMorphKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemMotionMorphKeyframeParseVMD(nanoem_motion_morph_keyframe_t *keyframe, nanoem_buffer_t *buffer, kh_string_cache_t *cache, nanoem_frame_index_t offset, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemMotionMorphKeyframeDestroy(nanoem_motion_morph_keyframe_t *keyframe);

NANOEM_DECL_INTERNAL nanoem_motion_self_shadow_keyframe_t *
nanoemMotionSelfShadowKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemMotionSelfShadowKeyframeParseVMD(nanoem_motion_self_shadow_keyframe_t *keyframe, nanoem_buffer_t *buffer, nanoem_frame_index_t offset, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemMotionSelfShadowKeyframeDestroy(nanoem_motion_self_shadow_keyframe_t *keyframe);

NANOEM_DECL_INTERNAL void
nanoemStringCacheDestroy(kh_string_cache_t *cache);
NANOEM_DECL_INTERNAL void
nanoemMotionTrackBundleAddKeyframe(kh_motion_track_bundle_t *bundle, nanoem_motion_keyframe_object_t *keyframe, nanoem_frame_index_t frame_index, const nanoem_unicode_string_t *name, nanoem_unicode_string_factory_t *factory, int *ret);
NANOEM_DECL_INTERNAL void
nanoemMotionTrackBundleDestroy(kh_motion_track_bundle_t *bundle, nanoem_unicode_string_factory_t *factory);

nanoem_pragma_diagnostics_push();
nanoem_pragma_diagnostics_ignore_clang_gcc("-Wunused-function");
nanoem_pragma_diagnostics_ignore_msvc(4930);

static char *
nanoemUtilCloneString(const char *source, nanoem_status_t *status)
{
    char *dest = NULL;
    nanoem_rsize_t len;
    if (nanoem_is_not_null(source)) {
        len = nanoem_crt_strlen(source);
        dest = (char *) nanoem_malloc(len + 1, status);
        if (nanoem_is_not_null(dest)) {
            nanoem_crt_memcpy(dest, source, len);
            dest[len] = '\0';
        }
    }
    return dest;
}

NANOEM_DECL_INLINE static void
nanoemUtilCopyString(char *dest, nanoem_rsize_t dest_size, const char *src, nanoem_rsize_t src_size)
{
#if defined(__STDC_LIB_EXT1__)
    strncpy_s(dest, dest_size, src, src_size);
#else
    nanoem_rsize_t capacity = src_size >= dest_size ? dest_size - 1 : src_size;
    nanoem_crt_strncpy(dest, src, capacity);
    dest[capacity] = '\0';
#endif
}

NANOEM_DECL_INLINE static void
nanoemUtilDestroyString(nanoem_unicode_string_t *s, nanoem_unicode_string_factory_t *factory)
{
    if (nanoem_is_not_null(factory) && nanoem_is_not_null(s)) {
        factory->destroy_string(factory->opaque_data, s);
    }
}

static const char *
nanoemAnnotationGet(const kh_annotation_t *annotations, const char *key)
{
    khiter_t it;
    const char *value = NULL;
    if (annotations && nanoem_is_not_null(key)) {
        it = kh_get_annotation(annotations, (char *) key);
        value = kh_exist(annotations, it) ? kh_value(annotations, it) : NULL;
    }
    return value;
}

static void
nanoemAnnotationSet(kh_annotation_t **annotations_ptr, const char *key, const char *value, int *ret, nanoem_status_t *status)
{
    kh_annotation_t *annotations;
    khiter_t it;
    char *key_ptr, *value_ptr;
    if (*annotations_ptr) {
        annotations = *annotations_ptr;
    }
    else {
        annotations = *annotations_ptr = kh_init_annotation();
    }
    key_ptr = nanoemUtilCloneString(key, status);
    it = kh_put_annotation(annotations, key_ptr, ret);
    if (*ret > 0) {
        kh_value(annotations, it) = nanoemUtilCloneString(value, status);
    }
    else if (*ret == 0) {
        value_ptr = kh_value(annotations, it);
        kh_value(annotations, it) = nanoemUtilCloneString(value, status);
        nanoem_free(value_ptr);
    }
    else {
        nanoem_free(key_ptr);
    }
}

static void
nanoemAnnotationDestroy(kh_annotation_t *annotations)
{
    khiter_t it, end;
    if (annotations) {
        for (it = 0, end = kh_end(annotations); it < end; it++) {
            if (kh_exist(annotations, it)) {
                nanoem_free(kh_key(annotations, it));
                nanoem_free(kh_value(annotations, it));
            }
        }
        kh_destroy_annotation(annotations);
    }
}

NANOEM_DECL_INLINE static nanoem_bool_t
nanoemBufferCanReadLengthInternal(const nanoem_buffer_t *buffer, nanoem_rsize_t length)
{
    return nanoem_is_not_null(buffer) && buffer->length >= buffer->offset && buffer->length - buffer->offset >= length;
}

NANOEM_DECL_INLINE static nanoem_bool_t
nanoemBufferIsEnd(nanoem_buffer_t *buffer)
{
    return buffer->length <= buffer->offset;
}

NANOEM_DECL_INLINE static nanoem_rsize_t
nanoemBufferReadLength(nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_bool_t ok;
    nanoem_rsize_t length = (nanoem_rsize_t) nanoemBufferReadInt32LittleEndian(buffer, status);
    if (!nanoem_status_ptr_has_error(status)) {
        ok = nanoemBufferCanReadLengthInternal(buffer, length);
        length = ok ? length : 0;
    }
    return length;
}

NANOEM_DECL_INLINE static nanoem_f32_t
nanoemBufferReadClampedLittleEndian(nanoem_buffer_t *buffer, nanoem_status_t *status)
{
    nanoem_f32_t v = nanoemBufferReadFloat32LittleEndian(buffer, status);
    return v > 1.0f ? 1.0f : (v < 0.0f ? 0.0f : v);
}

NANOEM_DECL_INLINE static void
nanoemBufferReadFloat32x3LittleEndian(nanoem_buffer_t *buffer, nanoem_f128_t *values, nanoem_status_t *status)
{
    values->values[0] = nanoemBufferReadFloat32LittleEndian(buffer, status);
    values->values[1] = nanoemBufferReadFloat32LittleEndian(buffer, status);
    values->values[2] = nanoemBufferReadFloat32LittleEndian(buffer, status);
}

NANOEM_DECL_INLINE static void
nanoemBufferReadFloat32x4LittleEndian(nanoem_buffer_t *buffer, nanoem_f128_t *values, nanoem_status_t *status)
{
    nanoemBufferReadFloat32x3LittleEndian(buffer, values, status);
    values->values[3] = nanoemBufferReadFloat32LittleEndian(buffer, status);
}

NANOEM_DECL_INLINE static void
nanoemModelObjectInitialize(nanoem_model_object_t *object, const nanoem_model_t *model)
{
    object->parent.model = model;
    object->index = NANOEM_MODEL_OBJECT_NOT_FOUND;
}

NANOEM_DECL_INLINE static nanoem_model_vertex_t *
nanoemModelGetOneVertexObject(const nanoem_model_t *model, int index)
{
    return nanoem_is_not_null(model) && index >= 0 && (nanoem_rsize_t) index < model->num_vertices ? model->vertices[index] : NULL;
}

NANOEM_DECL_INLINE static nanoem_model_material_t *
nanoemModelGetOneMaterialObject(const nanoem_model_t *model, int index)
{
    return nanoem_is_not_null(model) && index >= 0 && (nanoem_rsize_t) index < model->num_materials ? model->materials[index] : NULL;
}

NANOEM_DECL_INLINE static nanoem_model_bone_t *
nanoemModelGetOneBoneObject(const nanoem_model_t *model, int index)
{
    return nanoem_is_not_null(model) && index >= 0 && (nanoem_rsize_t) index < model->num_bones ? model->bones[index] : NULL;
}

NANOEM_DECL_INLINE static nanoem_model_morph_t *
nanoemModelGetOneMorphObject(const nanoem_model_t *model, int index)
{
    return nanoem_is_not_null(model) && index >= 0 && (nanoem_rsize_t) index < model->num_morphs ? model->morphs[index] : NULL;
}

NANOEM_DECL_INLINE static nanoem_model_rigid_body_t *
nanoemModelGetOneRigidBodyObject(const nanoem_model_t *model, int index)
{
    return nanoem_is_not_null(model) && index >= 0 && (nanoem_rsize_t) index < model->num_rigid_bodies ? model->rigid_bodies[index] : NULL;
}

NANOEM_DECL_INLINE static nanoem_model_texture_t *
nanoemModelGetOneTextureObject(const nanoem_model_t *model, int index)
{
    return nanoem_is_not_null(model) && index >= 0 && (nanoem_rsize_t) index < model->num_textures ? model->textures[index] : NULL;
}

NANOEM_DECL_INLINE static nanoem_unicode_string_t *
nanoemMotionTrackBundleResolveName(const kh_motion_track_bundle_t *bundle, int id)
{
    nanoem_motion_track_t *track;
    nanoem_unicode_string_t *s = NULL;
    khiter_t it, end;
    if (nanoem_is_not_null(bundle)) {
        end = kh_end(bundle);
        for (it = kh_begin(bundle); it != end; it++) {
            track = &kh_key(bundle, it);
            if (kh_exist(bundle, it) && track->id == id) {
                s = track->name;
                break;
            }
        }
    }
    return s;
}

static int
nanoemBufferReadInteger(nanoem_buffer_t *buffer, nanoem_rsize_t size, nanoem_status_t *status)
{
    switch (size) {
    case 4:
        return nanoemBufferReadInt32LittleEndian(buffer, status);
    case 2:
        return nanoemBufferReadInt16LittleEndianUnsigned(buffer, status);
    case 1:
        return nanoemBufferReadByte(buffer, status);
    default:
        *status = NANOEM_STATUS_ERROR_BUFFER_END;
        return 0;
    }
}

static int
nanoemBufferReadIntegerNullable(nanoem_buffer_t *buffer, nanoem_rsize_t size, nanoem_status_t *status)
{
    int value = nanoemBufferReadInteger(buffer, size, status);
    if ((size == 2 && value == 0xffff) || (size == 1 && value == 0xff)) {
        value = -1;
    }
    return value;
}

static nanoem_motion_track_index_t
nanoemMotionTrackBundleResolveId(kh_motion_track_bundle_t *bundle, nanoem_unicode_string_t *name, nanoem_unicode_string_factory_t *factory, nanoem_motion_track_index_t *allocated_id, nanoem_unicode_string_t **found_name, int *ret)
{
    nanoem_motion_track_t pair;
    khiter_t it;
    nanoem_motion_track_index_t id = 0;
    pair.factory = factory;
    pair.id = 0;
    pair.keyframes = NULL;
    pair.name = name;
    *found_name = NULL;
    if (nanoem_is_not_null(bundle) && nanoem_is_not_null(name) && nanoem_is_not_null(factory)) {
        it = kh_get_motion_track_bundle(bundle, pair);
        if (it != kh_end(bundle)) {
            id = kh_key(bundle, it).id;
            *found_name = kh_key(bundle, it).name;
        }
        else {
            id = pair.id = (++*allocated_id);
            pair.name = name;
            pair.keyframes = kh_init_keyframe_map();
            kh_put_motion_track_bundle(bundle, pair, ret);
        }
    }
    return id;
}

NANOEM_DECL_INLINE static nanoem_motion_track_index_t
nanoemMotionResolveLocalBoneTrackId(nanoem_motion_t *motion, nanoem_unicode_string_t *name, nanoem_unicode_string_t **found_name, int *ret)
{
    nanoem_unicode_string_factory_t *factory = motion->factory;
    return nanoem_is_not_null(motion) ? nanoemMotionTrackBundleResolveId(motion->local_bone_motion_track_bundle, name, factory, &motion->local_bone_motion_track_allocated_id, found_name, ret) : 0;
}

NANOEM_DECL_INLINE static nanoem_motion_track_index_t
nanoemMotionResolveLocalMorphTrackId(nanoem_motion_t *motion, nanoem_unicode_string_t *name, nanoem_unicode_string_t **found_name, int *ret)
{
    nanoem_unicode_string_factory_t *factory = motion->factory;
    return nanoem_is_not_null(motion) ? nanoemMotionTrackBundleResolveId(motion->local_morph_motion_track_bundle, name, factory, &motion->local_morph_motion_track_allocated_id, found_name, ret) : 0;
}

NANOEM_DECL_INLINE static nanoem_motion_track_index_t
nanoemMotionResolveGlobalTrackId(nanoem_motion_t *motion, nanoem_unicode_string_t *name, nanoem_unicode_string_t **found_name, int *ret)
{
    nanoem_unicode_string_factory_t *factory = motion->factory;
    return nanoem_is_not_null(motion) ? nanoemMotionTrackBundleResolveId(motion->global_motion_track_bundle, name, factory, &motion->global_motion_track_allocated_id, found_name, ret) : 0;
}

NANOEM_DECL_INLINE static void
nanoemMotionSetMaxFrameIndex(nanoem_motion_t *motion, const nanoem_motion_keyframe_object_t *base)
{
    if (base->frame_index > motion->max_frame_index) {
        motion->max_frame_index = base->frame_index;
    }
}

NANOEM_DECL_INLINE static kh_keyframe_map_t *
nanoemMotionFindKeyframesMap(kh_motion_track_bundle_t *track_bundle, const nanoem_unicode_string_t *name, nanoem_unicode_string_factory_t *factory)
{
    nanoem_motion_track_t track;
    kh_keyframe_map_t *keyframes_map = NULL;
    khiter_t it;
    union nanoem_const_to_mutable_unicode_string_cast_t {
        const nanoem_unicode_string_t *s;
        nanoem_unicode_string_t *m;
    } u;
    if (nanoem_is_not_null(name)) {
        u.s = name;
        track.factory = factory;
        track.name = u.m;
        track.id = 0;
        track.keyframes = NULL;
        it = kh_get_motion_track_bundle(track_bundle, track);
        if (it != kh_end(track_bundle)) {
            keyframes_map = kh_key(track_bundle, it).keyframes;
        }
    }
    return keyframes_map;
}

NANOEM_DECL_INLINE static int
nanoemMotionCompareKeyframe(const void *a, const void *b)
{
    union nanoem_void_to_base_keyframe_cast_t {
        const void *p;
        const nanoem_motion_keyframe_object_t **k;
    } u;
    const nanoem_motion_keyframe_object_t *left, *right;
    u.p = a;
    left = *u.k;
    u.p = b;
    right = *u.k;
    return left->frame_index - right->frame_index;
}

static void
nanoemMotionGetNearestKeyframes(nanoem_motion_keyframe_object_t *keyframe, nanoem_frame_index_t base_frame_index,
    nanoem_frame_index_t *prev_nearest, nanoem_frame_index_t *next_nearest,
    nanoem_motion_keyframe_object_t **prev_keyframe, nanoem_motion_keyframe_object_t **next_keyframe,
    nanoem_motion_keyframe_object_t **last_keyframe)
{
    const nanoem_frame_index_t frame_index = keyframe->frame_index;
    if (prev_keyframe && base_frame_index > frame_index && base_frame_index - *prev_nearest > base_frame_index - frame_index) {
        *prev_nearest = frame_index;
        *prev_keyframe = keyframe;
    }
    else if (next_keyframe && base_frame_index < frame_index && *next_nearest - base_frame_index> frame_index - base_frame_index) {
        *next_nearest = frame_index;
        *next_keyframe = keyframe;
    }
    if (!*last_keyframe || (*last_keyframe && frame_index > (*last_keyframe)->frame_index)) {
        *last_keyframe = keyframe;
    }
}

NANOEM_DECL_INLINE static const nanoem_model_t *
nanoemModelVertexGetParentModel(const nanoem_model_vertex_t *vertex)
{
    return nanoem_is_not_null(vertex) ? vertex->base.parent.model : NULL;
}

NANOEM_DECL_INLINE static const nanoem_model_t *
nanoemModelMaterialGetParentModel(const nanoem_model_material_t *material)
{
    return nanoem_is_not_null(material) ? material->base.parent.model : NULL;
}

NANOEM_DECL_INLINE static const nanoem_model_t *
nanoemModelTextureGetParentModel(const nanoem_model_texture_t *texture)
{
    return nanoem_is_not_null(texture) ? texture->base.parent.model : NULL;
}

NANOEM_DECL_INLINE static const nanoem_model_t *
nanoemModelBoneGetParentModel(const nanoem_model_bone_t *bone)
{
    return nanoem_is_not_null(bone) ? bone->base.parent.model : NULL;
}

NANOEM_DECL_INLINE static const nanoem_model_t *
nanoemModelConstraintGetParentModel(const nanoem_model_constraint_t *constraint)
{
    return nanoem_is_not_null(constraint) ? constraint->base.parent.model : NULL;
}

NANOEM_DECL_INLINE static const nanoem_model_constraint_t *
nanoemModelConstraintJointGetParentConstraintObject(const nanoem_model_constraint_joint_t *joint)
{
    return nanoem_is_not_null(joint) ?  joint->base.parent.constraint : NULL;
}

NANOEM_DECL_INLINE static const nanoem_model_morph_t *
nanoemModelMorphBoneGetParentMorph(const nanoem_model_morph_bone_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->base.parent.morph : NULL;
}

NANOEM_DECL_INLINE static const nanoem_model_morph_t *
nanoemModelMorphFlipGetParentMorph(const nanoem_model_morph_flip_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->base.parent.morph : NULL;
}

NANOEM_DECL_INLINE static const nanoem_model_morph_t *
nanoemModelMorphGroupGetParentMorph(const nanoem_model_morph_group_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->base.parent.morph : NULL;
}

NANOEM_DECL_INLINE static const nanoem_model_morph_t *
nanoemModelMorphImpulseGetParentMorph(const nanoem_model_morph_impulse_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->base.parent.morph : NULL;
}

NANOEM_DECL_INLINE static const nanoem_model_morph_t *
nanoemModelMorphMaterialGetParentMorph(const nanoem_model_morph_material_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->base.parent.morph : NULL;
}

NANOEM_DECL_INLINE static const nanoem_model_morph_t *
nanoemModelMorphUVGetParentMorph(const nanoem_model_morph_uv_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->base.parent.morph : NULL;
}

NANOEM_DECL_INLINE static const nanoem_model_morph_t *
nanoemModelMorphVertexGetParentMorph(const nanoem_model_morph_vertex_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->base.parent.morph : NULL;
}

NANOEM_DECL_INLINE static const nanoem_model_t *
nanoemModelMorphGetParentModel(const nanoem_model_morph_t *morph)
{
    return nanoem_is_not_null(morph) ? morph->base.parent.model : NULL;
}

NANOEM_DECL_INLINE static const nanoem_model_t *
nanoemModelLabelGetParentModel(const nanoem_model_label_t *label)
{
    return nanoem_is_not_null(label) ? label->base.parent.model : NULL;
}

NANOEM_DECL_INLINE static const nanoem_model_t *
nanoemModelRigidBodyGetParentModel(const nanoem_model_rigid_body_t *rigid_body)
{
    return nanoem_is_not_null(rigid_body) ? rigid_body->base.parent.model : NULL;
}

NANOEM_DECL_INLINE static const nanoem_model_t *
nanoemModelJointGetParentModel(const nanoem_model_joint_t *joint)
{
    return nanoem_is_not_null(joint) ? joint->base.parent.model : NULL;
}

NANOEM_DECL_INLINE static const nanoem_model_t *
nanoemModelSoftBodyGetParentModel(const nanoem_model_soft_body_t *soft_body)
{
    return nanoem_is_not_null(soft_body) ? soft_body->base.parent.model : NULL;
}

NANOEM_DECL_INLINE static const nanoem_model_soft_body_t *
nanoemModelSoftBodyAnchorGetParentSoftBody(const nanoem_model_soft_body_anchor_t *anchor)
{
    return nanoem_is_not_null(anchor) ? anchor->base.parent.soft_body : NULL;
}

nanoem_pragma_diagnostics_pop();

#endif /* NANOEM_PRIVATE_H_ */
