/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_MUTABLE_PRIVATE_H_
#define NANOEM_MUTABLE_PRIVATE_H_

/* THIS IS A PRIVATE DECLARATIONS OF NANOEM_C DO NOT INCLUDE DIRECTLY */

#include "../nanoem_p.h"
#include "./mutable.h"

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L /* C11 */
#ifndef NANOEM_MUTABLE_STATIC_ASSERT
#define NANOEM_MUTABLE_STATIC_ASSERT _Static_assert
#endif /* NANOEM_MUTABLE_STATIC_ASSERT */
#elif !defined(__cplusplus)
#ifndef NANOEM_MUTABLE_STATIC_ASSERT
#define __NANOEM_MUTABLE_SA_CONCAT(a, b) __NANOEM_MUTABLE_SA_CONCAT2(a, b)
#define __NANOEM_MUTABLE_SA_CONCAT2(a, b) a##b
#define NANOEM_MUTABLE_STATIC_ASSERT(cond, message)                              \
    enum { __NANOEM_MUTABLE_SA_CONCAT(NANOEM_MUTABLE_STATIC_ASSERT_LINE_AT_, __LINE__) = \
               sizeof(struct __NANOEM_MUTABLE_SA_CONCAT(__nanoem_mutable_static_assert_line_at_, __LINE__) { int static_assertion_failed[(cond) ? 1 : -1]; }) }
#endif /* NANOEM_MUTABLE_STATIC_ASSERT */
#else
#define NANOEM_MUTABLE_STATIC_ASSERT(cond, message)
#endif /* __STDC_VERSION__ >= 201112L */

#define nanoem_crt_memmove(dst, src, size) memmove((dst), (src), (size))
#define nanoem_crt_memset(dst, c, size) memset((dst), (c), (size))

static const char
    __nanoem_pmx_signature[] = "PMX ";
static const nanoem_rsize_t __nanoem_default_allocation_size = 8;

typedef void (*nanoem_mutable_buffer_write_byte_array_callback_t)(void *opaque, nanoem_mutable_buffer_t *, const nanoem_u8_t *, nanoem_rsize_t , nanoem_status_t *);
struct nanoem_mutable_buffer_t {
    nanoem_rsize_t allocated_length;
    nanoem_rsize_t actual_length;
    nanoem_rsize_t offset;
    nanoem_u8_t *data;
    nanoem_mutable_buffer_write_byte_array_callback_t write_byte_array;
    void *opaque;
};

typedef struct nanoem_mutable_base_keyframe_t nanoem_mutable_base_keyframe_t;
struct nanoem_mutable_base_keyframe_t {
    nanoem_bool_t is_reference;
    nanoem_bool_t is_in_motion;
};

struct nanoem_mutable_motion_effect_parameter_t {
    nanoem_mutable_base_keyframe_t base;
    nanoem_motion_effect_parameter_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_motion_effect_parameter_t, base) == 0, "mutable_effect_parameter_t inherits mutable_base_keyframe_t");

struct nanoem_mutable_motion_outside_parent_t {
    nanoem_mutable_base_keyframe_t base;
    nanoem_motion_outside_parent_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_motion_outside_parent_t, base) == 0, "mutable_outside_parent_t inherits mutable_base_keyframe_t");

struct nanoem_mutable_motion_accessory_keyframe_t {
    nanoem_mutable_base_keyframe_t base;
    nanoem_motion_accessory_keyframe_t *origin;
    nanoem_rsize_t num_allocated_effect_parameters;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_motion_accessory_keyframe_t, base) == 0, "mutable_accessory_keyframe_t inherits mutable_base_keyframe_t");

struct nanoem_mutable_motion_bone_keyframe_t {
    nanoem_mutable_base_keyframe_t base;
    nanoem_motion_bone_keyframe_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_motion_bone_keyframe_t, base) == 0, "mutable_bone_keyframe_t inherits mutable_base_keyframe_t");

struct nanoem_mutable_motion_camera_keyframe_t {
    nanoem_mutable_base_keyframe_t base;
    nanoem_motion_camera_keyframe_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_motion_camera_keyframe_t, base) == 0, "mutable_camera_keyframe_t inherits mutable_base_keyframe_t");

struct nanoem_mutable_motion_light_keyframe_t {
    nanoem_mutable_base_keyframe_t base;
    nanoem_motion_light_keyframe_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_motion_light_keyframe_t, base) == 0, "mutable_light_keyframe_t inherits mutable_base_keyframe_t");

struct nanoem_mutable_motion_model_keyframe_t {
    nanoem_mutable_base_keyframe_t base;
    nanoem_motion_model_keyframe_t *origin;
    nanoem_rsize_t num_allocated_constraint_states;
    nanoem_rsize_t num_allocated_effect_parameters;
    nanoem_rsize_t num_allocated_outside_parents;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_motion_model_keyframe_t, base) == 0, "mutable_model_keyframe_t inherits mutable_base_keyframe_t");

struct nanoem_mutable_motion_model_keyframe_constraint_state_t {
    nanoem_mutable_base_keyframe_t base;
    nanoem_motion_model_keyframe_constraint_state_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_motion_model_keyframe_constraint_state_t, base) == 0, "mutable_model_keyframe_constraint_state_t inherits mutable_base_keyframe_t");

struct nanoem_mutable_motion_morph_keyframe_t {
    nanoem_mutable_base_keyframe_t base;
    nanoem_motion_morph_keyframe_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_motion_morph_keyframe_t, base) == 0, "mutable_morph_keyframe_t inherits mutable_base_keyframe_t");

struct nanoem_mutable_motion_self_shadow_keyframe_t {
    nanoem_mutable_base_keyframe_t base;
    nanoem_motion_self_shadow_keyframe_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_motion_self_shadow_keyframe_t, base) == 0, "mutable_self_shadow_keyframe_t inherits mutable_base_keyframe_t");

struct nanoem_mutable_motion_t {
    nanoem_motion_t *origin;
    nanoem_bool_t is_reference;
    nanoem_rsize_t num_allocated_accessory_keyframes;
    nanoem_rsize_t num_allocated_bone_keyframes;
    nanoem_rsize_t num_allocated_camera_keyframes;
    nanoem_rsize_t num_allocated_light_keyframes;
    nanoem_rsize_t num_allocated_model_keyframes;
    nanoem_rsize_t num_allocated_morph_keyframes;
    nanoem_rsize_t num_allocated_self_shadow_keyframes;
};

typedef struct nanoem_mutable_base_model_object_t nanoem_mutable_base_model_object_t;
struct nanoem_mutable_base_model_object_t {
    nanoem_bool_t is_reference;
    nanoem_bool_t is_in_model;
};

typedef struct nanoem_mutable_base_label_object_t nanoem_mutable_base_label_object_t;
struct nanoem_mutable_base_label_object_t {
    nanoem_bool_t is_reference;
    nanoem_bool_t is_in_label;
};

typedef struct nanoem_mutable_base_morph_object_t nanoem_mutable_base_morph_object_t;
struct nanoem_mutable_base_morph_object_t {
    nanoem_bool_t is_reference;
    nanoem_bool_t is_in_morph;
};

typedef struct nanoem_mutable_base_soft_body_object_t nanoem_mutable_base_soft_body_object_t;
struct nanoem_mutable_base_soft_body_object_t {
    nanoem_bool_t is_reference;
    nanoem_bool_t is_in_soft_body;
};

struct nanoem_mutable_model_t {
    nanoem_model_t *origin;
    nanoem_bool_t is_reference;
    nanoem_rsize_t num_allocated_vertices;
    nanoem_rsize_t num_allocated_vertex_indices;
    nanoem_rsize_t num_allocated_materials;
    nanoem_rsize_t num_allocated_bones;
    nanoem_rsize_t num_allocated_constraints;
    nanoem_rsize_t num_allocated_textures;
    nanoem_rsize_t num_allocated_morphs;
    nanoem_rsize_t num_allocated_labels;
    nanoem_rsize_t num_allocated_rigid_bodies;
    nanoem_rsize_t num_allocated_joints;
    nanoem_rsize_t num_allocated_soft_bodies;
};

struct nanoem_mutable_model_vertex_t {
    nanoem_mutable_base_model_object_t base;
    nanoem_model_vertex_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_vertex_t, base) == 0, "mutable_vertex_t inherits mutable_base_model_object_t");

struct nanoem_mutable_model_material_t {
    nanoem_mutable_base_model_object_t base;
    nanoem_model_material_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_material_t, base) == 0, "mutable_material_t inherits mutable_base_model_object_t");

struct nanoem_mutable_model_bone_t {
    nanoem_mutable_base_model_object_t base;
    nanoem_model_bone_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_bone_t, base) == 0, "mutable_bone_t inherits mutable_base_model_object_t");

struct nanoem_mutable_model_constraint_joint_t {
    nanoem_mutable_base_model_object_t base;
    nanoem_model_constraint_joint_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_constraint_joint_t, base) == 0, "mutable_constraint_joint_t inherits mutable_base_model_object_t");

struct nanoem_mutable_model_constraint_t {
    nanoem_mutable_base_model_object_t base;
    nanoem_model_constraint_t *origin;
    nanoem_rsize_t num_allocated_joints;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_constraint_t, base) == 0, "nanoem_mutable_model_constraint_t inherits mutable_base_model_object_t");

struct nanoem_mutable_model_morph_group_t {
    nanoem_mutable_base_morph_object_t base;
    nanoem_model_morph_group_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_morph_group_t, base) == 0, "mutable_morph_group_t inherits mutable_base_morph_object_t");

struct nanoem_mutable_model_morph_vertex_t {
    nanoem_mutable_base_morph_object_t base;
    nanoem_model_morph_vertex_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_morph_vertex_t, base) == 0, "mutable_morph_vertex_t inherits mutable_base_morph_object_t");

struct nanoem_mutable_model_morph_bone_t {
    nanoem_mutable_base_morph_object_t base;
    nanoem_model_morph_bone_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_morph_bone_t, base) == 0, "mutable_morph_bone_t inherits mutable_base_morph_object_t");

struct nanoem_mutable_model_morph_uv_t {
    nanoem_mutable_base_morph_object_t base;
    nanoem_model_morph_uv_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_morph_uv_t, base) == 0, "mutable_morph_uv_t inherits mutable_base_morph_object_t");

struct nanoem_mutable_model_morph_material_t {
    nanoem_mutable_base_morph_object_t base;
    nanoem_model_morph_material_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_morph_material_t, base) == 0, "mutable_morph_material_t inherits mutable_base_morph_object_t");

struct nanoem_mutable_model_morph_flip_t {
    nanoem_mutable_base_morph_object_t base;
    nanoem_model_morph_flip_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_morph_flip_t, base) == 0, "mutable_morph_flip_t inherits mutable_base_morph_object_t");

struct nanoem_mutable_model_morph_impulse_t {
    nanoem_mutable_base_morph_object_t base;
    nanoem_model_morph_impulse_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_morph_flip_t, base) == 0, "mutable_morph_flip_t inherits mutable_base_morph_object_t");

struct nanoem_mutable_model_morph_t {
    nanoem_mutable_base_model_object_t base;
    nanoem_model_morph_t *origin;
    nanoem_rsize_t num_allocated_objects;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_morph_t, base) == 0, "mutable_morph_t inherits mutable_base_model_object_t");

struct nanoem_mutable_model_label_item_t {
    nanoem_mutable_base_label_object_t base;
    nanoem_model_label_item_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_label_item_t, base) == 0, "mutable_label_item_t inherits mutable_base_label_object_t");

struct nanoem_mutable_model_label_t {
    nanoem_mutable_base_model_object_t base;
    nanoem_model_label_t *origin;
    nanoem_rsize_t num_allocated_items;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_label_item_t, base) == 0, "mutable_label_t inherits mutable_base_model_object_t");

struct nanoem_mutable_model_rigid_body_t {
    nanoem_mutable_base_model_object_t base;
    nanoem_model_rigid_body_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_rigid_body_t, base) == 0, "mutable_rigid_body_t inherits mutable_base_model_object_t");

struct nanoem_mutable_model_joint_t {
    nanoem_mutable_base_model_object_t base;
    nanoem_model_joint_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_joint_t, base) == 0, "mutable_joint_t inherits mutable_base_model_object_t");

struct nanoem_mutable_model_soft_body_t {
    nanoem_mutable_base_model_object_t base;
    nanoem_model_soft_body_t *origin;
    nanoem_rsize_t num_allocated_anchors;
    nanoem_rsize_t num_allocated_pin_vertex_indices;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_soft_body_t, base) == 0, "mutable_soft_body_t inherits mutable_base_model_object_t");

struct nanoem_mutable_model_soft_body_anchor_t {
    nanoem_mutable_base_soft_body_object_t base;
    nanoem_model_soft_body_anchor_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_soft_body_anchor_t, base) == 0, "mutable_soft_body_anchor_t inherits mutable_base_soft_body_object_t");

struct nanoem_mutable_model_texture_t {
    nanoem_mutable_base_model_object_t base;
    nanoem_model_texture_t *origin;
};
NANOEM_MUTABLE_STATIC_ASSERT(offsetof(nanoem_mutable_model_texture_t, base) == 0, "mutable_texture_t inherits mutable_base_model_object_t");

NANOEM_DECL_INTERNAL void
nanoemMutableMotionAccessoryKeyframeSetFrameIndex(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_frame_index_t value);
NANOEM_DECL_INTERNAL void
nanoemMutableMotionBoneKeyframeSetFrameIndex(nanoem_mutable_motion_bone_keyframe_t *keyframe, nanoem_frame_index_t value);
NANOEM_DECL_INTERNAL void
nanoemMutableMotionBoneKeyframeSetName(nanoem_mutable_motion_bone_keyframe_t *keyframe, const nanoem_unicode_string_t *value, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemMutableMotionCameraKeyframeSetFrameIndex(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_frame_index_t value);
NANOEM_DECL_INTERNAL void
nanoemMutableMotionLightKeyframeSetFrameIndex(nanoem_mutable_motion_light_keyframe_t *keyframe, nanoem_frame_index_t value);
NANOEM_DECL_INTERNAL void
nanoemMutableMotionModelKeyframeSetFrameIndex(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_frame_index_t value);
NANOEM_DECL_INTERNAL void
nanoemMutableMotionMorphKeyframeSetFrameIndex(nanoem_mutable_motion_morph_keyframe_t *keyframe, nanoem_frame_index_t value);
NANOEM_DECL_INTERNAL void
nanoemMutableMotionMorphKeyframeSetName(nanoem_mutable_motion_morph_keyframe_t *keyframe, const nanoem_unicode_string_t *value, nanoem_status_t *status);
NANOEM_DECL_INTERNAL void
nanoemMutableMotionSelfShadowKeyframeSetFrameIndex(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, nanoem_frame_index_t value);

nanoem_pragma_diagnostics_push();
nanoem_pragma_diagnostics_ignore_clang_gcc("-Wunused-function");
nanoem_pragma_diagnostics_ignore_msvc(4930);

static void *
nanoemMutableObjectArrayResizeMemory(void *objects, nanoem_rsize_t *num_allocated_objects, nanoem_rsize_t *num_objects, nanoem_status_t *status, const char *filename, int line)
{
    nanoem_rsize_t na = *num_allocated_objects, nb = *num_objects + 1;
    if (na <= nb) {
        na = nb * 2;
        if (na > nb) {
            objects = nanoemMemoryResize(objects, sizeof(objects) * na, status, filename, line);
            *num_allocated_objects = na;
        }
        else {
            return NULL;
        }
    }
    *num_objects = nb;
    return objects;
}

#ifdef NANOEM_ENABLE_DEBUG_ALLOCATOR
#define nanoemMutableObjectArrayResize(objects, num_allocated_objects, num_objects, status) nanoemMutableObjectArrayResizeMemory((objects), (num_allocated_objects), (num_objects), (status), __FILE__, __LINE__)
#else
#define nanoemMutableObjectArrayResize(objects, num_allocated_objects, num_objects, status) nanoemMutableObjectArrayResizeMemory((objects), (num_allocated_objects), (num_objects), (status), NULL, 0)
#endif /* NANOEM_ENABLE_DEBUG_ALLOCATOR */

static int
nanoemObjectArrayFindObjectOffset(const void *const *objects, const void *ptr, nanoem_rsize_t num_objects)
{
    nanoem_rsize_t i;
    int offset = -1;
    if (num_objects > 0 && nanoem_is_not_null(objects)) {
        for (i = 0; i < num_objects; i++) {
            if (objects[i] == ptr) {
                offset = (int) i;
                break;
            }
        }
    }
    return offset;
}

NANOEM_DECL_INLINE static nanoem_bool_t
nanoemMutableBaseKeyframeCanDelete(nanoem_mutable_base_keyframe_t *base)
{
    return base->is_in_motion == nanoem_false;
}

NANOEM_DECL_INLINE static nanoem_bool_t
nanoemMutableBaseModelObjectCanDelete(nanoem_mutable_base_model_object_t *base)
{
    return base->is_in_model == nanoem_false;
}

NANOEM_DECL_INLINE static nanoem_bool_t
nanoemMutableBaseLabelItemObjectCanDelete(nanoem_mutable_base_label_object_t *base)
{
    return base->is_in_label == nanoem_false;
}

NANOEM_DECL_INLINE static nanoem_bool_t
nanoemMutableBaseMorphObjectCanDelete(nanoem_mutable_base_morph_object_t *base)
{
    return base->is_in_morph == nanoem_false;
}

NANOEM_DECL_INLINE static nanoem_bool_t
nanoemMutableBaseSoftBodyObjectObjectCanDelete(nanoem_mutable_base_soft_body_object_t *base)
{
    return base->is_in_soft_body == nanoem_false;
}

NANOEM_DECL_INLINE static nanoem_bool_t
nanoemModelIsPMX(const nanoem_model_t *model)
{
    return (int) (model->version * 10) >= 20;
}

NANOEM_DECL_INLINE static nanoem_bool_t
nanoemModelIsPMX21(const nanoem_model_t *model)
{
    return (int) (model->version * 10) >= 21;
}

NANOEM_DECL_INLINE static nanoem_rsize_t
nanoemModelGetVertexIndexSize(nanoem_rsize_t size)
{
    if (size <= 0xff) {
        return 1;
    }
    else if (size <= 0xffff) {
        return 2;
    }
    else {
        return 4;
    }
}

NANOEM_DECL_INLINE static nanoem_rsize_t
nanoemModelGetObjectIndexSize(nanoem_rsize_t size)
{
    if (size <= 0x7f) {
        return 1;
    }
    else if (size <= 0x7fff) {
        return 2;
    }
    else {
        return 4;
    }
}

NANOEM_DECL_INLINE static void
nanoemMutableBufferWriteFloat32x2LittleEndian(nanoem_mutable_buffer_t *buffer, const nanoem_f128_t *value, nanoem_status_t *status)
{
    nanoemMutableBufferWriteFloat32LittleEndian(buffer, value->values[0], status);
    nanoemMutableBufferWriteFloat32LittleEndian(buffer, value->values[1], status);
}

NANOEM_DECL_INLINE static void
nanoemMutableBufferWriteFloat32x3LittleEndian(nanoem_mutable_buffer_t *buffer, const nanoem_f128_t *value, nanoem_status_t *status)
{
    nanoemMutableBufferWriteFloat32x2LittleEndian(buffer, value, status);
    nanoemMutableBufferWriteFloat32LittleEndian(buffer, value->values[2], status);
}

NANOEM_DECL_INLINE static void
nanoemMutableBufferWriteFloat32x4LittleEndian(nanoem_mutable_buffer_t *buffer, const nanoem_f128_t *value, nanoem_status_t *status)
{
    nanoemMutableBufferWriteFloat32x3LittleEndian(buffer, value, status);
    nanoemMutableBufferWriteFloat32LittleEndian(buffer, value->values[3], status);
}

static nanoem_bool_t
nanoemObjectArrayContainsObject(const void *const *objects, const void *ptr, nanoem_rsize_t num_objects)
{
    return nanoemObjectArrayFindObjectOffset(objects, ptr, num_objects) != -1;
}

static int
nanoemObjectArrayFindIndexedObjectOffset(const void *const *objects, const void *ptr, nanoem_rsize_t num_objects, int index)
{
    nanoem_rsize_t i;
    int offset = -1;
    if (index >= 0 && num_objects > 0 && nanoem_is_not_null(objects)) {
        if ((nanoem_rsize_t) index < num_objects && objects[index] == ptr) {
            offset = index;
        }
        else {
            for (i = 0; i < num_objects; i++) {
                if (objects[i] == ptr) {
                    offset = (int) i;
                    break;
                }
            }
        }
    }
    return offset;
}

static nanoem_bool_t
nanoemObjectArrayContainsIndexedObject(const void *const *objects, const void *ptr, nanoem_rsize_t num_objects, int index)
{
    return nanoemObjectArrayFindIndexedObjectOffset(objects, ptr, num_objects, index) != -1;
}

static void
nanoemModelObjectArrayInsertObject(nanoem_model_object_t ***items, nanoem_model_object_t *item, int index, nanoem_status_t already_exists_error, nanoem_rsize_t *num_items, nanoem_rsize_t *num_allocated_items, nanoem_status_t *status)
{
    nanoem_model_object_t **new_items, **old_items = *items;
    nanoem_rsize_t nb = *num_items;
    int i;
    if (nanoemObjectArrayContainsIndexedObject((const void *const *) old_items, item, nb, item->index)) {
        nanoem_status_ptr_assign(status, already_exists_error);
        return;
    }
    new_items = (nanoem_model_object_t **) nanoemMutableObjectArrayResize(old_items, num_allocated_items, num_items, status);
    if (nanoem_is_not_null(new_items)) {
        nb = *num_items;
        if (index >= 0 && (nanoem_rsize_t) index < nb - 1) {
            nanoem_crt_memmove(&new_items[index + 1], &new_items[index], (nb - index) * sizeof(item));
            for (i = index + 1; i < (int) nb; i++) {
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
nanoemModelObjectArrayRemoveObject(nanoem_model_object_t **items, nanoem_model_object_t *item, nanoem_rsize_t *num_items, nanoem_status_t not_found_error, nanoem_status_t *status)
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

static void
nanoemMotionKeyframeObjectArrayAddObject(nanoem_motion_keyframe_object_t ***items, nanoem_motion_keyframe_object_t *item, nanoem_frame_index_t frame_index, nanoem_rsize_t *num_items, nanoem_rsize_t *num_allocated_items, nanoem_status_t *status)
{
    nanoem_motion_keyframe_object_t **new_items, **old_items = *items;
    new_items = (nanoem_motion_keyframe_object_t **) nanoemMutableObjectArrayResize(old_items, num_allocated_items, num_items, status);
    if (nanoem_is_not_null(new_items)) {
        new_items[*num_items - 1] = item;
        *items = new_items;
        item->frame_index = frame_index;
        nanoem_status_ptr_assign_succeeded(status);
    }
}

static void
nanoemMotionKeyframeObjectArrayRemoveObject(nanoem_motion_keyframe_object_t **items, nanoem_motion_keyframe_object_t *item, nanoem_rsize_t *num_items, nanoem_status_t not_found_error, nanoem_status_t *status)
{
    nanoem_rsize_t nb = *num_items;
    int index = item->index, i;
    if ((nanoem_rsize_t) index < nb && items[index] == item) {
        nanoem_crt_memmove(&items[index], &items[index + 1], (nb - index) * sizeof(item));
        *num_items = nb - 1;
        nanoem_status_ptr_assign_succeeded(status);
        return;
    }
    else {
        for (i = 0; i < (int) nb; i++) {
            if (items[i] == item) {
                nanoem_crt_memmove(&items[i], &items[i + 1], (nb - i) * sizeof(item));
                *num_items = nb - 1;
                nanoem_status_ptr_assign_succeeded(status);
                return;
            }
        }
    }
    nanoem_status_ptr_assign(status, not_found_error);
}

nanoem_pragma_diagnostics_pop();

#endif /* NANOEM_MUTABLE_PRIVATE_H_ */
