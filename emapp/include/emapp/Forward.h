/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_FORWARD_H_
#define NANOEM_EMAPP_FORWARD_H_

#include <float.h>
#include <stdlib.h>

#if __cplusplus >= 201103L
#define NANOEM_DECL_SEALED final
#define NANOEM_DECL_NOEXCEPT_OVERRIDE noexcept override
#define NANOEM_DECL_OVERRIDE override
#define NANOEM_DECL_NOEXCEPT noexcept
#else
#define NANOEM_DECL_SEALED
#define NANOEM_DECL_NOEXCEPT_OVERRIDE
#define NANOEM_DECL_OVERRIDE
#define NANOEM_DECL_NOEXCEPT
#endif

namespace nanoem {

#if __cplusplus >= 201103L
class NonCopyable {
    NonCopyable(NonCopyable const &) = delete;
    NonCopyable(NonCopyable &&) = delete;
    NonCopyable &operator=(NonCopyable const &) = delete;
    NonCopyable &operator=(NonCopyable &&) = delete;

protected:
    NonCopyable() = default;
    virtual ~NonCopyable() NANOEM_DECL_NOEXCEPT = default;
};
#else
class NonCopyable {
    NonCopyable &operator=(const NonCopyable &right);
    NonCopyable(const NonCopyable &right);

protected:
    NonCopyable()
    {
    }
    virtual ~NonCopyable() NANOEM_DECL_NOEXCEPT
    {
    }
};
#endif

class TinySTLAllocator : private NonCopyable {
public:
    struct DoNothing : private NonCopyable { };
    static void *static_allocate(size_t bytes);
    static void static_deallocate(void *ptr, size_t bytes) NANOEM_DECL_NOEXCEPT;
};

} /* namespace nanoem */
#define TINYSTL_ALLOCATOR nanoem::TinySTLAllocator::DoNothing

#ifndef NDEBUG
#include "bx/debug.h"
#include <stdio.h>
#define _BX_TRACE(_format, ...)                                                                                        \
    BX_MACRO_BLOCK_BEGIN                                                                                               \
    char buffer[1024];                                                                                                 \
    snprintf(buffer, sizeof(buffer), BX_FILE_LINE_LITERAL _format "\n", ##__VA_ARGS__);                                \
    bx::debugOutput(buffer);                                                                                           \
    BX_MACRO_BLOCK_END
#define _BX_WARN(_condition, _format, ...)                                                                             \
    BX_MACRO_BLOCK_BEGIN                                                                                               \
    if (!BX_IGNORE_C4127(_condition)) {                                                                                \
        BX_TRACE(_format, ##__VA_ARGS__);                                                                              \
    }                                                                                                                  \
    BX_MACRO_BLOCK_END
#undef BX_TRACE
#undef BX_WARN
#undef BX_ASSERT
#define BX_TRACE _BX_TRACE
#define BX_WARN _BX_WARN
#define BX_ASSERT _BX_WARN
#elif BX_PLATFORM_OSX || BX_PLATFORM_IOS
#define NSLog(...)
#endif /* NDEBUG */

#ifndef nanoem_assert
#ifndef NDEBUG
#define nanoem_assert(cond, message)                                                                                   \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            fflush(stderr);                                                                                            \
            fprintf(stderr, "ASSERTION FAILURE: %s at %s:%d\n", (message), __FILE__, __LINE__);                        \
            abort();                                                                                                   \
        }                                                                                                              \
    } while (0);
#else
#define nanoem_assert(cond, message)
#endif
#endif /* nanoem_parameter_assert */

#ifndef nanoem_parameter_assert
#define nanoem_parameter_assert(cond, message) nanoem_assert(cond, message)
#endif /* nanoem_parameter_assert */

#include "bx/bx.h"

#include "tinystl/allocator.h"
#include "tinystl/string.h"
#include "tinystl/unordered_map.h"
#include "tinystl/unordered_set.h"
#include "tinystl/vector.h"

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_XYZW_ONLY
#if GLM_VERSION > 990
#define GLM_FORCE_SINGLE_ONLY
#endif
#include "glm/fwd.hpp"
#include "glm/gtc/quaternion.hpp"

#include "nanoem/nanoem.h"

namespace nanoem {

/* for compatibility */
#if GLM_VERSION > 990
const int GLM_LEFT_HANDED = 1;
const int GLM_RIGHT_HANDED = 2;
#endif

typedef struct sg_buffer {
    uint32_t id;
} sg_buffer;
typedef struct sg_image {
    uint32_t id;
} sg_image;
typedef struct sg_shader {
    uint32_t id;
} sg_shader;
typedef struct sg_pipeline {
    uint32_t id;
} sg_pipeline;
typedef struct sg_pass {
    uint32_t id;
} sg_pass;
typedef struct sg_context {
    uint32_t id;
} sg_context;

typedef struct sg_range {
    const void *ptr;
    size_t size;
} sg_range;

enum {
    SG_INVALID_ID = 0,
    SG_NUM_SHADER_STAGES = 2,
    SG_NUM_INFLIGHT_FRAMES = 2,
    SG_MAX_COLOR_ATTACHMENTS = 4,
    SG_MAX_SHADERSTAGE_BUFFERS = 8,
    SG_MAX_SHADERSTAGE_IMAGES = 12,
    SG_MAX_SHADERSTAGE_UBS = 4,
    SG_MAX_UB_MEMBERS = 16,
    SG_MAX_VERTEX_ATTRIBUTES = 16,
    SG_MAX_MIPMAPS = 16,
    SG_MAX_TEXTUREARRAY_LAYERS = 128
};

typedef struct sg_color {
    float r, g, b, a;
} sg_color;

typedef enum sg_backend {
    SG_BACKEND_GLCORE33,
    SG_BACKEND_GLES2,
    SG_BACKEND_GLES3,
    SG_BACKEND_D3D11,
    SG_BACKEND_METAL_IOS,
    SG_BACKEND_METAL_MACOS,
    SG_BACKEND_METAL_SIMULATOR,
    SG_BACKEND_WGPU,
    SG_BACKEND_DUMMY,
} sg_backend;

typedef enum sg_pixel_format {
    _SG_PIXELFORMAT_DEFAULT,
    SG_PIXELFORMAT_NONE,

    SG_PIXELFORMAT_R8,
    SG_PIXELFORMAT_R8SN,
    SG_PIXELFORMAT_R8UI,
    SG_PIXELFORMAT_R8SI,

    SG_PIXELFORMAT_R16,
    SG_PIXELFORMAT_R16SN,
    SG_PIXELFORMAT_R16UI,
    SG_PIXELFORMAT_R16SI,
    SG_PIXELFORMAT_R16F,
    SG_PIXELFORMAT_RG8,
    SG_PIXELFORMAT_RG8SN,
    SG_PIXELFORMAT_RG8UI,
    SG_PIXELFORMAT_RG8SI,

    SG_PIXELFORMAT_R32UI,
    SG_PIXELFORMAT_R32SI,
    SG_PIXELFORMAT_R32F,
    SG_PIXELFORMAT_RG16,
    SG_PIXELFORMAT_RG16SN,
    SG_PIXELFORMAT_RG16UI,
    SG_PIXELFORMAT_RG16SI,
    SG_PIXELFORMAT_RG16F,
    SG_PIXELFORMAT_RGBA8,
    SG_PIXELFORMAT_RGBA8SN,
    SG_PIXELFORMAT_RGBA8UI,
    SG_PIXELFORMAT_RGBA8SI,
    SG_PIXELFORMAT_BGRA8,
    SG_PIXELFORMAT_RGB10A2,
    SG_PIXELFORMAT_RG11B10F,

    SG_PIXELFORMAT_RG32UI,
    SG_PIXELFORMAT_RG32SI,
    SG_PIXELFORMAT_RG32F,
    SG_PIXELFORMAT_RGBA16,
    SG_PIXELFORMAT_RGBA16SN,
    SG_PIXELFORMAT_RGBA16UI,
    SG_PIXELFORMAT_RGBA16SI,
    SG_PIXELFORMAT_RGBA16F,

    SG_PIXELFORMAT_RGBA32UI,
    SG_PIXELFORMAT_RGBA32SI,
    SG_PIXELFORMAT_RGBA32F,

    SG_PIXELFORMAT_DEPTH,
    SG_PIXELFORMAT_DEPTH_STENCIL,

    SG_PIXELFORMAT_BC1_RGBA,
    SG_PIXELFORMAT_BC2_RGBA,
    SG_PIXELFORMAT_BC3_RGBA,
    SG_PIXELFORMAT_BC4_R,
    SG_PIXELFORMAT_BC4_RSN,
    SG_PIXELFORMAT_BC5_RG,
    SG_PIXELFORMAT_BC5_RGSN,
    SG_PIXELFORMAT_BC6H_RGBF,
    SG_PIXELFORMAT_BC6H_RGBUF,
    SG_PIXELFORMAT_BC7_RGBA,
    SG_PIXELFORMAT_PVRTC_RGB_2BPP,
    SG_PIXELFORMAT_PVRTC_RGB_4BPP,
    SG_PIXELFORMAT_PVRTC_RGBA_2BPP,
    SG_PIXELFORMAT_PVRTC_RGBA_4BPP,
    SG_PIXELFORMAT_ETC2_RGB8,
    SG_PIXELFORMAT_ETC2_RGB8A1,
    SG_PIXELFORMAT_ETC2_RGBA8,
    SG_PIXELFORMAT_ETC2_RG11,
    SG_PIXELFORMAT_ETC2_RG11SN,

    _SG_PIXELFORMAT_NUM,
    _SG_PIXELFORMAT_FORCE_U32 = 0x7FFFFFFF
} sg_pixel_format;

typedef struct sg_pixelformat_info {
    bool sample; // pixel format can be sampled in shaders
    bool filter; // pixel format can be sampled with filtering
    bool render; // pixel format can be used as render target
    bool blend; // alpha-blending is supported
    bool msaa; // pixel format can be used as MSAA render target
    bool depth; // pixel format is a depth format
#if defined(SOKOL_ZIG_BINDINGS)
    uint32_t __pad[3];
#endif
} sg_pixelformat_info;

typedef struct sg_features {
    bool instancing; // hardware instancing supported
    bool origin_top_left; // framebuffer and texture origin is in top left corner
    bool multiple_render_targets; // offscreen render passes can have multiple render targets attached
    bool msaa_render_targets; // offscreen render passes support MSAA antialiasing
    bool imagetype_3d; // creation of SG_IMAGETYPE_3D images is supported
    bool imagetype_array; // creation of SG_IMAGETYPE_ARRAY images is supported
    bool image_clamp_to_border; // border color and clamp-to-border UV-wrap mode is supported
    bool mrt_independent_blend_state; // multiple-render-target rendering can use per-render-target blend state
    bool mrt_independent_write_mask; // multiple-render-target rendering can use per-render-target color write masks
    bool blend_op_minmax; // min/max blend operation mode is supported
#if defined(SOKOL_ZIG_BINDINGS)
    uint32_t __pad[2];
#endif
} sg_features;

typedef struct sg_limits {
    int max_image_size_2d; // max width/height of SG_IMAGETYPE_2D images
    int max_image_size_cube; // max width/height of SG_IMAGETYPE_CUBE images
    int max_image_size_3d; // max width/height/depth of SG_IMAGETYPE_3D images
    int max_image_size_array; // max width/height of SG_IMAGETYPE_ARRAY images
    int max_image_array_layers; // max number of layers in SG_IMAGETYPE_ARRAY images
    int max_vertex_attrs; // <= SG_MAX_VERTEX_ATTRIBUTES or less (on some GLES2 impls)
    int gl_max_vertex_uniform_vectors; // <= GL_MAX_VERTEX_UNIFORM_VECTORS (only on GL backends)
} sg_limits;

typedef enum sg_resource_state {
    SG_RESOURCESTATE_INITIAL,
    SG_RESOURCESTATE_ALLOC,
    SG_RESOURCESTATE_VALID,
    SG_RESOURCESTATE_FAILED,
    SG_RESOURCESTATE_INVALID,
    _SG_RESOURCESTATE_FORCE_U32 = 0x7FFFFFFF
} sg_resource_state;

typedef enum sg_usage {
    _SG_USAGE_DEFAULT, /* value 0 reserved for default-init */
    SG_USAGE_IMMUTABLE,
    SG_USAGE_DYNAMIC,
    SG_USAGE_STREAM,
    _SG_USAGE_NUM,
    _SG_USAGE_FORCE_U32 = 0x7FFFFFFF
} sg_usage;

typedef enum sg_buffer_type {
    _SG_BUFFERTYPE_DEFAULT, /* value 0 reserved for default-init */
    SG_BUFFERTYPE_VERTEXBUFFER,
    SG_BUFFERTYPE_INDEXBUFFER,
    _SG_BUFFERTYPE_NUM,
    _SG_BUFFERTYPE_FORCE_U32 = 0x7FFFFFFF
} sg_buffer_type;

typedef enum sg_index_type {
    _SG_INDEXTYPE_DEFAULT, /* value 0 reserved for default-init */
    SG_INDEXTYPE_NONE,
    SG_INDEXTYPE_UINT16,
    SG_INDEXTYPE_UINT32,
    _SG_INDEXTYPE_NUM,
    _SG_INDEXTYPE_FORCE_U32 = 0x7FFFFFFF
} sg_index_type;

typedef enum sg_image_type {
    _SG_IMAGETYPE_DEFAULT, /* value 0 reserved for default-init */
    SG_IMAGETYPE_2D,
    SG_IMAGETYPE_CUBE,
    SG_IMAGETYPE_3D,
    SG_IMAGETYPE_ARRAY,
    _SG_IMAGETYPE_NUM,
    _SG_IMAGETYPE_FORCE_U32 = 0x7FFFFFFF
} sg_image_type;

typedef enum sg_sampler_type {
    _SG_SAMPLERTYPE_DEFAULT, /* value 0 reserved for default-init */
    SG_SAMPLERTYPE_FLOAT,
    SG_SAMPLERTYPE_SINT,
    SG_SAMPLERTYPE_UINT,
} sg_sampler_type;

typedef enum sg_cube_face {
    SG_CUBEFACE_POS_X,
    SG_CUBEFACE_NEG_X,
    SG_CUBEFACE_POS_Y,
    SG_CUBEFACE_NEG_Y,
    SG_CUBEFACE_POS_Z,
    SG_CUBEFACE_NEG_Z,
    SG_CUBEFACE_NUM,
    _SG_CUBEFACE_FORCE_U32 = 0x7FFFFFFF
} sg_cube_face;

typedef enum sg_shader_stage {
    SG_SHADERSTAGE_VS,
    SG_SHADERSTAGE_FS,
    _SG_SHADERSTAGE_FORCE_U32 = 0x7FFFFFFF
} sg_shader_stage;

typedef enum sg_primitive_type {
    _SG_PRIMITIVETYPE_DEFAULT, /* value 0 reserved for default-init */
    SG_PRIMITIVETYPE_POINTS,
    SG_PRIMITIVETYPE_LINES,
    SG_PRIMITIVETYPE_LINE_STRIP,
    SG_PRIMITIVETYPE_TRIANGLES,
    SG_PRIMITIVETYPE_TRIANGLE_STRIP,
    _SG_PRIMITIVETYPE_NUM,
    _SG_PRIMITIVETYPE_FORCE_U32 = 0x7FFFFFFF
} sg_primitive_type;

typedef enum sg_filter {
    _SG_FILTER_DEFAULT, /* value 0 reserved for default-init */
    SG_FILTER_NEAREST,
    SG_FILTER_LINEAR,
    SG_FILTER_NEAREST_MIPMAP_NEAREST,
    SG_FILTER_NEAREST_MIPMAP_LINEAR,
    SG_FILTER_LINEAR_MIPMAP_NEAREST,
    SG_FILTER_LINEAR_MIPMAP_LINEAR,
    _SG_FILTER_NUM,
    _SG_FILTER_FORCE_U32 = 0x7FFFFFFF
} sg_filter;

typedef enum sg_wrap {
    _SG_WRAP_DEFAULT, /* value 0 reserved for default-init */
    SG_WRAP_REPEAT,
    SG_WRAP_CLAMP_TO_EDGE,
    SG_WRAP_CLAMP_TO_BORDER,
    SG_WRAP_MIRRORED_REPEAT,
    _SG_WRAP_NUM,
    _SG_WRAP_FORCE_U32 = 0x7FFFFFFF
} sg_wrap;

typedef enum sg_border_color {
    _SG_BORDERCOLOR_DEFAULT, /* value 0 reserved for default-init */
    SG_BORDERCOLOR_TRANSPARENT_BLACK,
    SG_BORDERCOLOR_OPAQUE_BLACK,
    SG_BORDERCOLOR_OPAQUE_WHITE,
    _SG_BORDERCOLOR_NUM,
    _SG_BORDERCOLOR_FORCE_U32 = 0x7FFFFFFF
} sg_border_color;

typedef enum sg_vertex_format {
    SG_VERTEXFORMAT_INVALID,
    SG_VERTEXFORMAT_FLOAT,
    SG_VERTEXFORMAT_FLOAT2,
    SG_VERTEXFORMAT_FLOAT3,
    SG_VERTEXFORMAT_FLOAT4,
    SG_VERTEXFORMAT_BYTE4,
    SG_VERTEXFORMAT_BYTE4N,
    SG_VERTEXFORMAT_UBYTE4,
    SG_VERTEXFORMAT_UBYTE4N,
    SG_VERTEXFORMAT_SHORT2,
    SG_VERTEXFORMAT_SHORT2N,
    SG_VERTEXFORMAT_USHORT2N,
    SG_VERTEXFORMAT_SHORT4,
    SG_VERTEXFORMAT_SHORT4N,
    SG_VERTEXFORMAT_USHORT4N,
    SG_VERTEXFORMAT_UINT10_N2,
    _SG_VERTEXFORMAT_NUM,
    _SG_VERTEXFORMAT_FORCE_U32 = 0x7FFFFFFF
} sg_vertex_format;

typedef enum sg_vertex_step {
    _SG_VERTEXSTEP_DEFAULT, /* value 0 reserved for default-init */
    SG_VERTEXSTEP_PER_VERTEX,
    SG_VERTEXSTEP_PER_INSTANCE,
    _SG_VERTEXSTEP_NUM,
    _SG_VERTEXSTEP_FORCE_U32 = 0x7FFFFFFF
} sg_vertex_step;

typedef enum sg_uniform_type {
    SG_UNIFORMTYPE_INVALID,
    SG_UNIFORMTYPE_FLOAT,
    SG_UNIFORMTYPE_FLOAT2,
    SG_UNIFORMTYPE_FLOAT3,
    SG_UNIFORMTYPE_FLOAT4,
    SG_UNIFORMTYPE_INT,
    SG_UNIFORMTYPE_INT2,
    SG_UNIFORMTYPE_INT3,
    SG_UNIFORMTYPE_INT4,
    SG_UNIFORMTYPE_MAT4,
    _SG_UNIFORMTYPE_NUM,
    _SG_UNIFORMTYPE_FORCE_U32 = 0x7FFFFFFF
} sg_uniform_type;

typedef enum sg_uniform_layout {
    _SG_UNIFORMLAYOUT_DEFAULT,
    SG_UNIFORMLAYOUT_NATIVE,
    SG_UNIFORMLAYOUT_STD140,
    _SG_UNIFORMLAYOUT_NUM,
    _SG_UNIFORMLAYOUT_FORCE_U32 = 0x7FFFFFFF
} sg_uniform_layout;

typedef enum sg_cull_mode {
    _SG_CULLMODE_DEFAULT, /* value 0 reserved for default-init */
    SG_CULLMODE_NONE,
    SG_CULLMODE_FRONT,
    SG_CULLMODE_BACK,
    _SG_CULLMODE_NUM,
    _SG_CULLMODE_FORCE_U32 = 0x7FFFFFFF
} sg_cull_mode;

typedef enum sg_face_winding {
    _SG_FACEWINDING_DEFAULT, /* value 0 reserved for default-init */
    SG_FACEWINDING_CCW,
    SG_FACEWINDING_CW,
    _SG_FACEWINDING_NUM,
    _SG_FACEWINDING_FORCE_U32 = 0x7FFFFFFF
} sg_face_winding;

typedef enum sg_compare_func {
    _SG_COMPAREFUNC_DEFAULT, /* value 0 reserved for default-init */
    SG_COMPAREFUNC_NEVER,
    SG_COMPAREFUNC_LESS,
    SG_COMPAREFUNC_EQUAL,
    SG_COMPAREFUNC_LESS_EQUAL,
    SG_COMPAREFUNC_GREATER,
    SG_COMPAREFUNC_NOT_EQUAL,
    SG_COMPAREFUNC_GREATER_EQUAL,
    SG_COMPAREFUNC_ALWAYS,
    _SG_COMPAREFUNC_NUM,
    _SG_COMPAREFUNC_FORCE_U32 = 0x7FFFFFFF
} sg_compare_func;

typedef enum sg_stencil_op {
    _SG_STENCILOP_DEFAULT, /* value 0 reserved for default-init */
    SG_STENCILOP_KEEP,
    SG_STENCILOP_ZERO,
    SG_STENCILOP_REPLACE,
    SG_STENCILOP_INCR_CLAMP,
    SG_STENCILOP_DECR_CLAMP,
    SG_STENCILOP_INVERT,
    SG_STENCILOP_INCR_WRAP,
    SG_STENCILOP_DECR_WRAP,
    _SG_STENCILOP_NUM,
    _SG_STENCILOP_FORCE_U32 = 0x7FFFFFFF
} sg_stencil_op;

typedef enum sg_blend_factor {
    _SG_BLENDFACTOR_DEFAULT, /* value 0 reserved for default-init */
    SG_BLENDFACTOR_ZERO,
    SG_BLENDFACTOR_ONE,
    SG_BLENDFACTOR_SRC_COLOR,
    SG_BLENDFACTOR_ONE_MINUS_SRC_COLOR,
    SG_BLENDFACTOR_SRC_ALPHA,
    SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
    SG_BLENDFACTOR_DST_COLOR,
    SG_BLENDFACTOR_ONE_MINUS_DST_COLOR,
    SG_BLENDFACTOR_DST_ALPHA,
    SG_BLENDFACTOR_ONE_MINUS_DST_ALPHA,
    SG_BLENDFACTOR_SRC_ALPHA_SATURATED,
    SG_BLENDFACTOR_BLEND_COLOR,
    SG_BLENDFACTOR_ONE_MINUS_BLEND_COLOR,
    SG_BLENDFACTOR_BLEND_ALPHA,
    SG_BLENDFACTOR_ONE_MINUS_BLEND_ALPHA,
    _SG_BLENDFACTOR_NUM,
    _SG_BLENDFACTOR_FORCE_U32 = 0x7FFFFFFF
} sg_blend_factor;

typedef enum sg_blend_op {
    _SG_BLENDOP_DEFAULT, /* value 0 reserved for default-init */
    SG_BLENDOP_ADD,
    SG_BLENDOP_SUBTRACT,
    SG_BLENDOP_REVERSE_SUBTRACT,
    SG_BLENDOP_MIN,
    SG_BLENDOP_MAX,
    _SG_BLENDOP_NUM,
    _SG_BLENDOP_FORCE_U32 = 0x7FFFFFFF
} sg_blend_op;

typedef enum sg_color_mask {
    _SG_COLORMASK_DEFAULT = 0, /* value 0 reserved for default-init */
    SG_COLORMASK_NONE = 0x10, /* special value for 'all channels disabled */
    SG_COLORMASK_R = 0x1,
    SG_COLORMASK_G = 0x2,
    SG_COLORMASK_RG = 0x3,
    SG_COLORMASK_B = 0x4,
    SG_COLORMASK_RB = 0x5,
    SG_COLORMASK_GB = 0x6,
    SG_COLORMASK_RGB = 0x7,
    SG_COLORMASK_A = 0x8,
    SG_COLORMASK_RA = 0x9,
    SG_COLORMASK_GA = 0xA,
    SG_COLORMASK_RGA = 0xB,
    SG_COLORMASK_BA = 0xC,
    SG_COLORMASK_RBA = 0xD,
    SG_COLORMASK_GBA = 0xE,
    SG_COLORMASK_RGBA = 0xF,
    _SG_COLORMASK_FORCE_U32 = 0x7FFFFFFF
} sg_color_mask;

typedef enum sg_action {
    _SG_ACTION_DEFAULT,
    SG_ACTION_CLEAR,
    SG_ACTION_LOAD,
    SG_ACTION_DONTCARE,
    _SG_ACTION_NUM,
    _SG_ACTION_FORCE_U32 = 0x7FFFFFFF
} sg_action;

typedef struct sg_color_attachment_action {
    sg_action action;
    sg_color value;
} sg_color_attachment_action;

typedef struct sg_depth_attachment_action {
    sg_action action;
    float value;
} sg_depth_attachment_action;

typedef struct sg_stencil_attachment_action {
    sg_action action;
    uint8_t value;
} sg_stencil_attachment_action;

typedef struct sg_pass_action {
    uint32_t _start_canary;
    sg_color_attachment_action colors[SG_MAX_COLOR_ATTACHMENTS];
    sg_depth_attachment_action depth;
    sg_stencil_attachment_action stencil;
    uint32_t _end_canary;
} sg_pass_action;

typedef struct sg_bindings {
    uint32_t _start_canary;
    sg_buffer vertex_buffers[SG_MAX_SHADERSTAGE_BUFFERS];
    int vertex_buffer_offsets[SG_MAX_SHADERSTAGE_BUFFERS];
    sg_buffer index_buffer;
    int index_buffer_offset;
    sg_image vs_images[SG_MAX_SHADERSTAGE_IMAGES];
    sg_image fs_images[SG_MAX_SHADERSTAGE_IMAGES];
    uint32_t _end_canary;
} sg_bindings;

typedef struct sg_buffer_desc {
    uint32_t _start_canary;
    size_t size;
    sg_buffer_type type;
    sg_usage usage;
    sg_range data;
    const char *label;
    /* GL specific */
    uint32_t gl_buffers[SG_NUM_INFLIGHT_FRAMES];
    /* Metal specific */
    const void *mtl_buffers[SG_NUM_INFLIGHT_FRAMES];
    /* D3D11 specific */
    const void *d3d11_buffer;
    /* WebGPU specific */
    const void *wgpu_buffer;
    uint32_t _end_canary;
} sg_buffer_desc;

typedef struct sg_image_data {
    sg_range subimage[SG_CUBEFACE_NUM][SG_MAX_MIPMAPS];
} sg_image_data;

typedef struct sg_image_desc {
    uint32_t _start_canary;
    sg_image_type type;
    bool render_target;
    int width;
    int height;
    int num_slices;
    int num_mipmaps;
    sg_usage usage;
    sg_pixel_format pixel_format;
    int sample_count;
    sg_filter min_filter;
    sg_filter mag_filter;
    sg_wrap wrap_u;
    sg_wrap wrap_v;
    sg_wrap wrap_w;
    sg_border_color border_color;
    uint32_t max_anisotropy;
    float min_lod;
    float max_lod;
    sg_image_data data;
    const char *label;
    /* GL specific */
    uint32_t gl_textures[SG_NUM_INFLIGHT_FRAMES];
    uint32_t gl_texture_target;
    /* Metal specific */
    const void *mtl_textures[SG_NUM_INFLIGHT_FRAMES];
    /* D3D11 specific */
    const void *d3d11_texture;
    const void *d3d11_shader_resource_view;
    /* WebGPU specific */
    const void *wgpu_texture;
    uint32_t _end_canary;
} sg_image_desc;

typedef struct sg_shader_attr_desc {
    const char *name; // GLSL vertex attribute name (only strictly required for GLES2)
    const char *sem_name; // HLSL semantic name
    int sem_index; // HLSL semantic index
} sg_shader_attr_desc;

typedef struct sg_shader_uniform_desc {
    const char *name;
    sg_uniform_type type;
    int array_count;
} sg_shader_uniform_desc;

typedef struct sg_shader_uniform_block_desc {
    size_t size;
    sg_uniform_layout layout;
    sg_shader_uniform_desc uniforms[SG_MAX_UB_MEMBERS];
} sg_shader_uniform_block_desc;

typedef struct sg_shader_image_desc {
    const char *name;
    sg_image_type image_type;
    sg_sampler_type sampler_type;
} sg_shader_image_desc;

typedef struct sg_shader_stage_desc {
    const char *source;
    sg_range bytecode;
    const char *entry;
    const char *d3d11_target;
    sg_shader_uniform_block_desc uniform_blocks[SG_MAX_SHADERSTAGE_UBS];
    sg_shader_image_desc images[SG_MAX_SHADERSTAGE_IMAGES];
} sg_shader_stage_desc;

typedef struct sg_shader_desc {
    uint32_t _start_canary;
    sg_shader_attr_desc attrs[SG_MAX_VERTEX_ATTRIBUTES];
    sg_shader_stage_desc vs;
    sg_shader_stage_desc fs;
    const char *label;
    uint32_t _end_canary;
} sg_shader_desc;

typedef struct sg_buffer_layout_desc {
    int stride;
    sg_vertex_step step_func;
    int step_rate;
#if defined(SOKOL_ZIG_BINDINGS)
    uint32_t __pad[2];
#endif
} sg_buffer_layout_desc;

typedef struct sg_vertex_attr_desc {
    int buffer_index;
    int offset;
    sg_vertex_format format;
#if defined(SOKOL_ZIG_BINDINGS)
    uint32_t __pad[2];
#endif
} sg_vertex_attr_desc;

typedef struct sg_layout_desc {
    sg_buffer_layout_desc buffers[SG_MAX_SHADERSTAGE_BUFFERS];
    sg_vertex_attr_desc attrs[SG_MAX_VERTEX_ATTRIBUTES];
} sg_layout_desc;

typedef struct sg_stencil_face_state {
    sg_compare_func compare;
    sg_stencil_op fail_op;
    sg_stencil_op depth_fail_op;
    sg_stencil_op pass_op;
} sg_stencil_face_state;

typedef struct sg_stencil_state {
    bool enabled;
    sg_stencil_face_state front;
    sg_stencil_face_state back;
    uint8_t read_mask;
    uint8_t write_mask;
    uint8_t ref;
} sg_stencil_state;

typedef struct sg_depth_state {
    sg_pixel_format pixel_format;
    sg_compare_func compare;
    bool write_enabled;
    float bias;
    float bias_slope_scale;
    float bias_clamp;
} sg_depth_state;

typedef struct sg_blend_state {
    bool enabled;
    sg_blend_factor src_factor_rgb;
    sg_blend_factor dst_factor_rgb;
    sg_blend_op op_rgb;
    sg_blend_factor src_factor_alpha;
    sg_blend_factor dst_factor_alpha;
    sg_blend_op op_alpha;
} sg_blend_state;

typedef struct sg_color_state {
    sg_pixel_format pixel_format;
    sg_color_mask write_mask;
    sg_blend_state blend;
} sg_color_state;

typedef struct sg_pipeline_desc {
    uint32_t _start_canary;
    sg_shader shader;
    sg_layout_desc layout;
    sg_depth_state depth;
    sg_stencil_state stencil;
    int color_count;
    sg_color_state colors[SG_MAX_COLOR_ATTACHMENTS];
    sg_primitive_type primitive_type;
    sg_index_type index_type;
    sg_cull_mode cull_mode;
    sg_face_winding face_winding;
    int sample_count;
    sg_color blend_color;
    bool alpha_to_coverage_enabled;
    const char *label;
    uint32_t _end_canary;
} sg_pipeline_desc;

typedef struct sg_pass_attachment_desc {
    sg_image image;
    int mip_level;
    int slice; /* cube texture: face; array texture: layer; 3D texture: slice */
} sg_pass_attachment_desc;

typedef struct sg_pass_desc {
    uint32_t _start_canary;
    sg_pass_attachment_desc color_attachments[SG_MAX_COLOR_ATTACHMENTS];
    sg_pass_attachment_desc depth_stencil_attachment;
    const char *label;
    uint32_t _end_canary;
} sg_pass_desc;

typedef struct sg_trace_hooks {
    void *user_data;
    void (*reset_state_cache)(void *user_data);
    void (*make_buffer)(const sg_buffer_desc *desc, sg_buffer result, void *user_data);
    void (*make_image)(const sg_image_desc *desc, sg_image result, void *user_data);
    void (*make_shader)(const sg_shader_desc *desc, sg_shader result, void *user_data);
    void (*make_pipeline)(const sg_pipeline_desc *desc, sg_pipeline result, void *user_data);
    void (*make_pass)(const sg_pass_desc *desc, sg_pass result, void *user_data);
    void (*destroy_buffer)(sg_buffer buf, void *user_data);
    void (*destroy_image)(sg_image img, void *user_data);
    void (*destroy_shader)(sg_shader shd, void *user_data);
    void (*destroy_pipeline)(sg_pipeline pip, void *user_data);
    void (*destroy_pass)(sg_pass pass, void *user_data);
    void (*update_buffer)(sg_buffer buf, const sg_range *data, void *user_data);
    void (*update_image)(sg_image img, const sg_image_data *data, void *user_data);
    void (*append_buffer)(sg_buffer buf, const sg_range *data, int result, void *user_data);
    void (*begin_default_pass)(const sg_pass_action *pass_action, int width, int height, void *user_data);
    void (*begin_pass)(sg_pass pass, const sg_pass_action *pass_action, void *user_data);
    void (*apply_viewport)(int x, int y, int width, int height, bool origin_top_left, void *user_data);
    void (*apply_scissor_rect)(int x, int y, int width, int height, bool origin_top_left, void *user_data);
    void (*apply_pipeline)(sg_pipeline pip, void *user_data);
    void (*apply_bindings)(const sg_bindings *bindings, void *user_data);
    void (*apply_uniforms)(sg_shader_stage stage, int ub_index, const sg_range *data, void *user_data);
    void (*draw)(int base_element, int num_elements, int num_instances, void *user_data);
    void (*end_pass)(void *user_data);
    void (*commit)(void *user_data);
    void (*alloc_buffer)(sg_buffer result, void *user_data);
    void (*alloc_image)(sg_image result, void *user_data);
    void (*alloc_shader)(sg_shader result, void *user_data);
    void (*alloc_pipeline)(sg_pipeline result, void *user_data);
    void (*alloc_pass)(sg_pass result, void *user_data);
    void (*dealloc_buffer)(sg_buffer buf_id, void *user_data);
    void (*dealloc_image)(sg_image img_id, void *user_data);
    void (*dealloc_shader)(sg_shader shd_id, void *user_data);
    void (*dealloc_pipeline)(sg_pipeline pip_id, void *user_data);
    void (*dealloc_pass)(sg_pass pass_id, void *user_data);
    void (*init_buffer)(sg_buffer buf_id, const sg_buffer_desc *desc, void *user_data);
    void (*init_image)(sg_image img_id, const sg_image_desc *desc, void *user_data);
    void (*init_shader)(sg_shader shd_id, const sg_shader_desc *desc, void *user_data);
    void (*init_pipeline)(sg_pipeline pip_id, const sg_pipeline_desc *desc, void *user_data);
    void (*init_pass)(sg_pass pass_id, const sg_pass_desc *desc, void *user_data);
    void (*uninit_buffer)(sg_buffer buf_id, void *user_data);
    void (*uninit_image)(sg_image img_id, void *user_data);
    void (*uninit_shader)(sg_shader shd_id, void *user_data);
    void (*uninit_pipeline)(sg_pipeline pip_id, void *user_data);
    void (*uninit_pass)(sg_pass pass_id, void *user_data);
    void (*fail_buffer)(sg_buffer buf_id, void *user_data);
    void (*fail_image)(sg_image img_id, void *user_data);
    void (*fail_shader)(sg_shader shd_id, void *user_data);
    void (*fail_pipeline)(sg_pipeline pip_id, void *user_data);
    void (*fail_pass)(sg_pass pass_id, void *user_data);
    void (*push_debug_group)(const char *name, void *user_data);
    void (*pop_debug_group)(void *user_data);
    void (*err_buffer_pool_exhausted)(void *user_data);
    void (*err_image_pool_exhausted)(void *user_data);
    void (*err_shader_pool_exhausted)(void *user_data);
    void (*err_pipeline_pool_exhausted)(void *user_data);
    void (*err_pass_pool_exhausted)(void *user_data);
    void (*err_context_mismatch)(void *user_data);
    void (*err_pass_invalid)(void *user_data);
    void (*err_draw_invalid)(void *user_data);
    void (*err_bindings_invalid)(void *user_data);
} sg_trace_hooks;

typedef struct sg_slot_info {
    sg_resource_state state; /* the current state of this resource slot */
    uint32_t res_id; /* type-neutral resource if (e.g. sg_buffer.id) */
    uint32_t ctx_id; /* the context this resource belongs to */
} sg_slot_info;

typedef struct sg_buffer_info {
    sg_slot_info slot; /* resource pool slot info */
    uint32_t update_frame_index; /* frame index of last sg_update_buffer() */
    uint32_t append_frame_index; /* frame index of last sg_append_buffer() */
    int append_pos; /* current position in buffer for sg_append_buffer() */
    bool append_overflow; /* is buffer in overflow state (due to sg_append_buffer) */
    int num_slots; /* number of renaming-slots for dynamically updated buffers */
    int active_slot; /* currently active write-slot for dynamically updated buffers */
} sg_buffer_info;

typedef struct sg_image_info {
    sg_slot_info slot; /* resource pool slot info */
    uint32_t upd_frame_index; /* frame index of last sg_update_image() */
    int num_slots; /* number of renaming-slots for dynamically updated images */
    int active_slot; /* currently active write-slot for dynamically updated images */
    int width; /* image width */
    int height; /* image height */
} sg_image_info;

typedef struct sg_shader_info {
    sg_slot_info slot; /* resoure pool slot info */
} sg_shader_info;

typedef struct sg_pipeline_info {
    sg_slot_info slot; /* resource pool slot info */
} sg_pipeline_info;

typedef struct sg_pass_info {
    sg_slot_info slot; /* resource pool slot info */
} sg_pass_info;

typedef struct sg_gl_context_desc {
    bool force_gles2;
} sg_gl_context_desc;

typedef struct sg_metal_context_desc {
    const void *device;
    const void *(*renderpass_descriptor_cb)(void);
    const void *(*renderpass_descriptor_userdata_cb)(void *);
    const void *(*drawable_cb)(void);
    const void *(*drawable_userdata_cb)(void *);
    void *user_data;
} sg_metal_context_desc;

typedef struct sg_d3d11_context_desc {
    const void *device;
    const void *device_context;
    const void *(*render_target_view_cb)(void);
    const void *(*render_target_view_userdata_cb)(void *);
    const void *(*depth_stencil_view_cb)(void);
    const void *(*depth_stencil_view_userdata_cb)(void *);
    void *user_data;
} sg_d3d11_context_desc;

typedef struct sg_wgpu_context_desc {
    const void *device; /* WGPUDevice */
    const void *(*render_view_cb)(void); /* returns WGPUTextureView */
    const void *(*render_view_userdata_cb)(void *);
    const void *(*resolve_view_cb)(void); /* returns WGPUTextureView */
    const void *(*resolve_view_userdata_cb)(void *);
    const void *(*depth_stencil_view_cb)(void); /* returns WGPUTextureView, must be WGPUTextureFormat_Depth24Plus8 */
    const void *(*depth_stencil_view_userdata_cb)(void *);
    void *user_data;
} sg_wgpu_context_desc;

typedef struct sg_context_desc {
    sg_pixel_format color_format;
    sg_pixel_format depth_format;
    int sample_count;
    sg_gl_context_desc gl;
    sg_metal_context_desc metal;
    sg_d3d11_context_desc d3d11;
    sg_wgpu_context_desc wgpu;
} sg_context_desc;

typedef struct sg_desc {
    uint32_t _start_canary;
    int buffer_pool_size;
    int image_pool_size;
    int shader_pool_size;
    int pipeline_pool_size;
    int pass_pool_size;
    int context_pool_size;
    int uniform_buffer_size;
    int staging_buffer_size;
    int sampler_cache_size;
    sg_context_desc context;
    uint32_t _end_canary;
} sg_desc;

/* end of sokol_gfx.h */

typedef tinystl::stringT<TinySTLAllocator> String;
typedef tinystl::vector<char, TinySTLAllocator> MutableString;
typedef tinystl::vector<wchar_t, TinySTLAllocator> MutableWideString;
typedef tinystl::vector<nanoem_u8_t, TinySTLAllocator> ByteArray;
typedef tinystl::vector<ByteArray, TinySTLAllocator> ByteArrayList;
typedef tinystl::vector<String, TinySTLAllocator> StringList;
typedef tinystl::unordered_set<String, TinySTLAllocator> StringSet;
typedef tinystl::pair<String, String> StringPair;
typedef tinystl::vector<StringPair, TinySTLAllocator> StringPairList;
typedef tinystl::vector<MutableString, TinySTLAllocator> MutableStringList;
typedef tinystl::vector<nanoem_f32_t, TinySTLAllocator> FloatList;
typedef tinystl::vector<nanoem_u32_t, TinySTLAllocator> VertexIndexList;
typedef tinystl::unordered_map<nanoem_i32_t, nanoem_i32_t, TinySTLAllocator> Int32HashMap;
typedef tinystl::unordered_map<nanoem_u32_t, nanoem_u32_t, TinySTLAllocator> UInt32HashMap;
typedef tinystl::unordered_map<String, String, TinySTLAllocator> StringMap;
typedef tinystl::unordered_map<String, StringPair, TinySTLAllocator> StringPairMap;
typedef tinystl::unordered_map<nanoem_u32_t, sg_pipeline, TinySTLAllocator> PipelineMap;
typedef tinystl::vector<sg_image, TinySTLAllocator> TextureHandleList;
typedef glm::vec<2, nanoem_f32_t, glm::packed_highp> Vector2;
typedef glm::vec<3, nanoem_f32_t, glm::packed_highp> Vector3;
typedef glm::vec<4, nanoem_f32_t, glm::packed_highp> Vector4;
typedef glm::vec<2, nanoem_i32_t, glm::packed_highp> Vector2SI32;
typedef glm::vec<3, nanoem_i32_t, glm::packed_highp> Vector3SI32;
typedef glm::vec<4, nanoem_i32_t, glm::packed_highp> Vector4SI32;
typedef glm::vec<2, nanoem_u8_t, glm::packed_highp> Vector2U8;
typedef glm::vec<3, nanoem_u8_t, glm::packed_highp> Vector3U8;
typedef glm::vec<4, nanoem_u8_t, glm::packed_highp> Vector4U8;
typedef glm::vec<2, nanoem_u16_t, glm::packed_highp> Vector2UI16;
typedef glm::vec<3, nanoem_u16_t, glm::packed_highp> Vector3UI16;
typedef glm::vec<4, nanoem_u16_t, glm::packed_highp> Vector4UI16;
typedef glm::vec<2, nanoem_u32_t, glm::packed_highp> Vector2UI32;
typedef glm::vec<3, nanoem_u32_t, glm::packed_highp> Vector3UI32;
typedef glm::vec<4, nanoem_u32_t, glm::packed_highp> Vector4UI32;
typedef glm::qua<nanoem_f32_t, glm::packed_highp> Quaternion;
typedef glm::mat<3, 3, nanoem_f32_t, glm::packed_highp> Matrix3x3;
typedef glm::mat<4, 4, nanoem_f32_t, glm::packed_highp> Matrix4x4;

namespace sg {

typedef bool(APIENTRY *PFN_sgx_isvalid)(void);
extern PFN_sgx_isvalid isvalid;
typedef bool(APIENTRY *PFN_sgx_query_buffer_overflow)(sg_buffer buf);
extern PFN_sgx_query_buffer_overflow query_buffer_overflow;
typedef int(APIENTRY *PFN_sgx_append_buffer)(sg_buffer buf, const void *data_ptr, int data_size);
extern PFN_sgx_append_buffer append_buffer;
typedef sg_backend(APIENTRY *PFN_sgx_query_backend)(void);
extern PFN_sgx_query_backend query_backend;
typedef sg_buffer(APIENTRY *PFN_sgx_alloc_buffer)(void);
extern PFN_sgx_alloc_buffer alloc_buffer;
typedef sg_buffer(APIENTRY *PFN_sgx_make_buffer)(const sg_buffer_desc *desc);
extern PFN_sgx_make_buffer make_buffer;
typedef sg_buffer_desc(APIENTRY *PFN_sgx_query_buffer_defaults)(const sg_buffer_desc *desc);
extern PFN_sgx_query_buffer_defaults query_buffer_defaults;
typedef sg_buffer_info(APIENTRY *PFN_sgx_query_buffer_info)(sg_buffer buf);
extern PFN_sgx_query_buffer_info query_buffer_info;
typedef sg_context(APIENTRY *PFN_sgx_setup_context)(void);
extern PFN_sgx_setup_context setup_context;
typedef sg_desc(APIENTRY *PFN_sgx_query_desc)(void);
extern PFN_sgx_query_desc query_desc;
typedef sg_features(APIENTRY *PFN_sgx_query_features)(void);
extern PFN_sgx_query_features query_features;
typedef sg_image(APIENTRY *PFN_sgx_alloc_image)(void);
extern PFN_sgx_alloc_image alloc_image;
typedef sg_image(APIENTRY *PFN_sgx_make_image)(const sg_image_desc *desc);
extern PFN_sgx_make_image make_image;
typedef sg_image_desc(APIENTRY *PFN_sgx_query_image_defaults)(const sg_image_desc *desc);
extern PFN_sgx_query_image_defaults query_image_defaults;
typedef sg_image_info(APIENTRY *PFN_sgx_query_image_info)(sg_image img);
extern PFN_sgx_query_image_info query_image_info;
typedef sg_limits(APIENTRY *PFN_sgx_query_limits)(void);
extern PFN_sgx_query_limits query_limits;
typedef sg_pass(APIENTRY *PFN_sgx_alloc_pass)(void);
extern PFN_sgx_alloc_pass alloc_pass;
typedef sg_pass(APIENTRY *PFN_sgx_make_pass)(const sg_pass_desc *desc);
extern PFN_sgx_make_pass make_pass;
typedef sg_pass_desc(APIENTRY *PFN_sgx_query_pass_defaults)(const sg_pass_desc *desc);
extern PFN_sgx_query_pass_defaults query_pass_defaults;
typedef sg_pass_info(APIENTRY *PFN_sgx_query_pass_info)(sg_pass pass);
extern PFN_sgx_query_pass_info query_pass_info;
typedef sg_pipeline(APIENTRY *PFN_sgx_alloc_pipeline)(void);
extern PFN_sgx_alloc_pipeline alloc_pipeline;
typedef sg_pipeline(APIENTRY *PFN_sgx_make_pipeline)(const sg_pipeline_desc *desc);
extern PFN_sgx_make_pipeline make_pipeline;
typedef sg_pipeline_desc(APIENTRY *PFN_sgx_query_pipeline_defaults)(const sg_pipeline_desc *desc);
extern PFN_sgx_query_pipeline_defaults query_pipeline_defaults;
typedef sg_pipeline_info(APIENTRY *PFN_sgx_query_pipeline_info)(sg_pipeline pip);
extern PFN_sgx_query_pipeline_info query_pipeline_info;
typedef sg_pixelformat_info(APIENTRY *PFN_sgx_query_pixelformat)(sg_pixel_format fmt);
extern PFN_sgx_query_pixelformat query_pixelformat;
typedef sg_resource_state(APIENTRY *PFN_sgx_query_buffer_state)(sg_buffer buf);
extern PFN_sgx_query_buffer_state query_buffer_state;
typedef sg_resource_state(APIENTRY *PFN_sgx_query_image_state)(sg_image img);
extern PFN_sgx_query_image_state query_image_state;
typedef sg_resource_state(APIENTRY *PFN_sgx_query_pass_state)(sg_pass pass);
extern PFN_sgx_query_pass_state query_pass_state;
typedef sg_resource_state(APIENTRY *PFN_sgx_query_pipeline_state)(sg_pipeline pip);
extern PFN_sgx_query_pipeline_state query_pipeline_state;
typedef sg_resource_state(APIENTRY *PFN_sgx_query_shader_state)(sg_shader shd);
extern PFN_sgx_query_shader_state query_shader_state;
typedef sg_shader(APIENTRY *PFN_sgx_alloc_shader)(void);
extern PFN_sgx_alloc_shader alloc_shader;
typedef sg_shader(APIENTRY *PFN_sgx_make_shader)(const sg_shader_desc *desc);
extern PFN_sgx_make_shader make_shader;
typedef sg_shader_desc(APIENTRY *PFN_sgx_query_shader_defaults)(const sg_shader_desc *desc);
extern PFN_sgx_query_shader_defaults query_shader_defaults;
typedef sg_shader_info(APIENTRY *PFN_sgx_query_shader_info)(sg_shader shd);
extern PFN_sgx_query_shader_info query_shader_info;
typedef sg_trace_hooks(APIENTRY *PFN_sgx_install_trace_hooks)(const sg_trace_hooks *trace_hooks);
extern PFN_sgx_install_trace_hooks install_trace_hooks;
typedef void(APIENTRY *PFN_sgx_activate_context)(sg_context ctx_id);
extern PFN_sgx_activate_context activate_context;
typedef void(APIENTRY *PFN_sgx_apply_bindings)(const sg_bindings *bindings);
extern PFN_sgx_apply_bindings apply_bindings;
typedef void(APIENTRY *PFN_sgx_apply_pipeline)(sg_pipeline pip);
extern PFN_sgx_apply_pipeline apply_pipeline;
typedef void(APIENTRY *PFN_sgx_apply_scissor_rect)(int x, int y, int width, int height, bool origin_top_left);
extern PFN_sgx_apply_scissor_rect apply_scissor_rect;
typedef void(APIENTRY *PFN_sgx_apply_uniforms)(sg_shader_stage stage, int ub_index, const void *data, int num_bytes);
extern PFN_sgx_apply_uniforms apply_uniforms;
typedef void(APIENTRY *PFN_sgx_apply_viewport)(int x, int y, int width, int height, bool origin_top_left);
extern PFN_sgx_apply_viewport apply_viewport;
typedef void(APIENTRY *PFN_sgx_begin_default_pass)(const sg_pass_action *pass_action, int width, int height);
extern PFN_sgx_begin_default_pass begin_default_pass;
typedef void(APIENTRY *PFN_sgx_begin_pass)(sg_pass pass, const sg_pass_action *pass_action);
extern PFN_sgx_begin_pass begin_pass;
typedef void(APIENTRY *PFN_sgx_commit)(void);
extern PFN_sgx_commit commit;
typedef void(APIENTRY *PFN_sgx_destroy_buffer)(sg_buffer buf);
extern PFN_sgx_destroy_buffer destroy_buffer;
typedef void(APIENTRY *PFN_sgx_destroy_image)(sg_image img);
extern PFN_sgx_destroy_image destroy_image;
typedef void(APIENTRY *PFN_sgx_destroy_pass)(sg_pass pass);
extern PFN_sgx_destroy_pass destroy_pass;
typedef void(APIENTRY *PFN_sgx_destroy_pipeline)(sg_pipeline pip);
extern PFN_sgx_destroy_pipeline destroy_pipeline;
typedef void(APIENTRY *PFN_sgx_destroy_shader)(sg_shader shd);
extern PFN_sgx_destroy_shader destroy_shader;
typedef void(APIENTRY *PFN_sgx_discard_context)(sg_context ctx_id);
extern PFN_sgx_discard_context discard_context;
typedef void(APIENTRY *PFN_sgx_draw)(int base_element, int num_elements, int num_instances);
extern PFN_sgx_draw draw;
typedef void(APIENTRY *PFN_sgx_end_pass)(void);
extern PFN_sgx_end_pass end_pass;
typedef void(APIENTRY *PFN_sgx_label_buffer)(sg_buffer buffer, const char *text);
extern PFN_sgx_label_buffer label_buffer;
typedef void(APIENTRY *PFN_sgx_label_image)(sg_image image, const char *text);
extern PFN_sgx_label_image label_image;
typedef void(APIENTRY *PFN_sgx_label_shader)(sg_shader shader, const char *text);
extern PFN_sgx_label_shader label_shader;
typedef void(APIENTRY *PFN_sgx_label_pass)(sg_pass pass, const char *text);
extern PFN_sgx_label_pass label_pass;
typedef void(APIENTRY *PFN_sgx_label_pipeline)(sg_pipeline pipeline, const char *text);
extern PFN_sgx_label_pipeline label_pipeline;
typedef void(APIENTRY *PFN_sgx_push_group)(const char *text);
extern PFN_sgx_push_group push_group;
typedef void(APIENTRY *PFN_sgx_insert_marker)(const char *text);
extern PFN_sgx_insert_marker insert_marker;
typedef void(APIENTRY *PFN_sgx_pop_group)();
extern PFN_sgx_pop_group pop_group;
typedef void(APIENTRY *PFN_sgx_read_image)(sg_image image, sg_buffer buffer, void *data, size_t size);
extern PFN_sgx_read_image read_image;
typedef void(APIENTRY *PFN_sgx_read_pass)(sg_pass pass, sg_buffer buffer, void *data, size_t size);
extern PFN_sgx_read_pass read_pass;
typedef void (*sg_read_pass_async_callback)(const void *, size_t, void *);
typedef void(APIENTRY *PFN_sgx_read_pass_async)(
    sg_pass pass, sg_buffer buffer, sg_read_pass_async_callback callback, void *opaque);
extern PFN_sgx_read_pass_async read_pass_async;
typedef nanoem_u32_t(APIENTRY *PFN_sgx_get_renderer_type)();
extern PFN_sgx_get_renderer_type get_renderer_type;
typedef void *(APIENTRY *PFN_sgx_map_buffer)(sg_buffer buffer);
extern PFN_sgx_map_buffer map_buffer;
typedef void(APIENTRY *PFN_sgx_unmap_buffer)(sg_buffer buffer, void *address);
extern PFN_sgx_unmap_buffer unmap_buffer;
typedef void(APIENTRY *PFN_sgx_fail_buffer)(sg_buffer buf_id);
extern PFN_sgx_fail_buffer fail_buffer;
typedef void(APIENTRY *PFN_sgx_fail_image)(sg_image img_id);
extern PFN_sgx_fail_image fail_image;
typedef void(APIENTRY *PFN_sgx_fail_pass)(sg_pass pass_id);
extern PFN_sgx_fail_pass fail_pass;
typedef void(APIENTRY *PFN_sgx_fail_pipeline)(sg_pipeline pip_id);
extern PFN_sgx_fail_pipeline fail_pipeline;
typedef void(APIENTRY *PFN_sgx_fail_shader)(sg_shader shd_id);
extern PFN_sgx_fail_shader fail_shader;
typedef void(APIENTRY *PFN_sgx_init_buffer)(sg_buffer buf_id, const sg_buffer_desc *desc);
extern PFN_sgx_init_buffer init_buffer;
typedef void(APIENTRY *PFN_sgx_init_image)(sg_image img_id, const sg_image_desc *desc);
extern PFN_sgx_init_image init_image;
typedef void(APIENTRY *PFN_sgx_init_pass)(sg_pass pass_id, const sg_pass_desc *desc);
extern PFN_sgx_init_pass init_pass;
typedef void(APIENTRY *PFN_sgx_init_pipeline)(sg_pipeline pip_id, const sg_pipeline_desc *desc);
extern PFN_sgx_init_pipeline init_pipeline;
typedef void(APIENTRY *PFN_sgx_init_shader)(sg_shader shd_id, const sg_shader_desc *desc);
extern PFN_sgx_init_shader init_shader;
typedef void(APIENTRY *PFN_sgx_pop_debug_group)(void);
extern PFN_sgx_pop_debug_group pop_debug_group;
typedef void(APIENTRY *PFN_sgx_push_debug_group)(const char *name);
extern PFN_sgx_push_debug_group push_debug_group;
typedef void(APIENTRY *PFN_sgx_reset_state_cache)(void);
extern PFN_sgx_reset_state_cache reset_state_cache;
typedef void(APIENTRY *PFN_sgx_setup)(const sg_desc *desc);
extern PFN_sgx_setup setup;
typedef void(APIENTRY *PFN_sgx_shutdown)(void);
extern PFN_sgx_shutdown shutdown;
typedef void(APIENTRY *PFN_sgx_update_buffer)(sg_buffer buf, const void *data_ptr, int data_size);
extern PFN_sgx_update_buffer update_buffer;
typedef void(APIENTRY *PFN_sgx_update_image)(sg_image img, const sg_image_data *data);
extern PFN_sgx_update_image update_image;
typedef void(APIENTRY *PFN_sg_dealloc_buffer)(sg_buffer buf_id);
extern PFN_sg_dealloc_buffer dealloc_buffer;
typedef void(APIENTRY *PFN_sg_dealloc_image)(sg_image img_id);
extern PFN_sg_dealloc_image dealloc_image;
typedef void(APIENTRY *PFN_sg_dealloc_shader)(sg_shader shd_id);
extern PFN_sg_dealloc_shader dealloc_shader;
typedef void(APIENTRY *PFN_sg_dealloc_pipeline)(sg_pipeline pip_id);
extern PFN_sg_dealloc_pipeline dealloc_pipeline;
typedef void(APIENTRY *PFN_sg_dealloc_pass)(sg_pass pass_id);
extern PFN_sg_dealloc_pass dealloc_pass;
typedef bool(APIENTRY *PFN_sg_uninit_buffer)(sg_buffer buf_id);
extern PFN_sg_uninit_buffer uninit_buffer;
typedef bool(APIENTRY *PFN_sg_uninit_image)(sg_image img_id);
extern PFN_sg_uninit_image uninit_image;
typedef bool(APIENTRY *PFN_sg_uninit_shader)(sg_shader shd_id);
extern PFN_sg_uninit_shader uninit_shader;
typedef bool(APIENTRY *PFN_sg_uninit_pipeline)(sg_pipeline pip_id);
extern PFN_sg_uninit_pipeline uninit_pipeline;
typedef bool(APIENTRY *PFN_sg_uninit_pass)(sg_pass pass_id);
extern PFN_sg_uninit_pass uninit_pass;

void *openSharedLibrary(const char *dllPath);
void closeSharedLibrary(void *&handle);
bool is_valid(sg_buffer value) NANOEM_DECL_NOEXCEPT;
bool is_valid(sg_image value) NANOEM_DECL_NOEXCEPT;
bool is_valid(sg_pass value) NANOEM_DECL_NOEXCEPT;
bool is_valid(sg_pipeline value) NANOEM_DECL_NOEXCEPT;
bool is_valid(sg_shader value) NANOEM_DECL_NOEXCEPT;
bool is_backend_metal(sg_backend value) NANOEM_DECL_NOEXCEPT;
void insert_marker_format(const char *format, ...);
void push_group_format(const char *format, ...);

class PassBlock NANOEM_DECL_SEALED : private NonCopyable {
public:
    typedef void (*Callback)(sg_pass, void *);
    class IDrawQueue {
    public:
        virtual ~IDrawQueue() NANOEM_DECL_NOEXCEPT
        {
        }
        virtual void beginPass(sg_pass pass, const sg_pass_action &action) = 0;
        virtual void endPass() = 0;
        virtual void applyPipelineBindings(sg_pipeline pipeline, const sg_bindings &bindings) = 0;
        virtual void applyViewport(int x, int y, int width, int height) = 0;
        virtual void applyScissorRect(int x, int y, int width, int height) = 0;
        virtual void applyUniformBlock(const void *data, nanoem_rsize_t size) = 0;
        virtual void applyUniformBlock(sg_shader_stage stage, const void *data, nanoem_rsize_t size) = 0;
        virtual void draw(int offset, int count) = 0;
        virtual void registerCallback(Callback callback, void *userData) = 0;
    };
    PassBlock() NANOEM_DECL_NOEXCEPT;
    PassBlock(IDrawQueue *queue, sg_pass pass, const sg_pass_action &action) NANOEM_DECL_NOEXCEPT;
    ~PassBlock() NANOEM_DECL_NOEXCEPT;

    void applyBindings(const sg_bindings &bindings);
    void applyPipeline(sg_pipeline pipeline);
    void applyPipelineBindings(sg_pipeline pipeline, const sg_bindings &bindings);
    void applyViewport(int x, int y, int width, int height);
    void applyScissorRect(int x, int y, int width, int height);
    void applyUniformBlock(const void *data, nanoem_rsize_t size);
    void applyUniformBlock(sg_shader_stage stage, const void *data, nanoem_rsize_t size);
    void draw(int offset, int count);
    void registerCallback(Callback callback, void *userData);

private:
    IDrawQueue *m_drawQueue;
};
typedef tinystl::pair<sg_image, const char *> NamedImage;
typedef tinystl::pair<sg_pass, const char *> NamedPass;

BX_ALIGN_DECL_16(struct)
LineVertexUnit
{
    Vector3 m_position;
    Vector4U8 m_color;
};

BX_ALIGN_DECL_16(struct)
QuadVertexUnit
{
    Vector4 m_position;
    Vector4 m_texcoord;
    static void generateQuadUV(nanoem_f32_t & minu, nanoem_f32_t & minv, nanoem_f32_t & maxu, nanoem_f32_t & maxv)
        NANOEM_DECL_NOEXCEPT;
    static void generateQuadTriStrip(QuadVertexUnit * vertices) NANOEM_DECL_NOEXCEPT;
    static void generateQuadTriStrip(QuadVertexUnit * vertices, nanoem_f32_t minx, nanoem_f32_t miny, nanoem_f32_t maxx,
        nanoem_f32_t maxy) NANOEM_DECL_NOEXCEPT;
};

} /* namespace sg */
} /* namespace nanoem */

#endif /* NANOEM_CORE_FORWARD_H_ */
