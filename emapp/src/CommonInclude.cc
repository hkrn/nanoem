/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/private/CommonInclude.h"

#include "emapp/Constants.h"
#include "emapp/StringUtils.h"

#include "sokol/sokol_time.h"

extern "C" {

extern void APIENTRY sgx_label_buffer(sg_buffer buffer, const char *text);
extern void APIENTRY sgx_label_image(sg_image image, const char *text);
extern void APIENTRY sgx_label_shader(sg_shader shader, const char *text);
extern void APIENTRY sgx_label_pass(sg_pass pass, const char *text);
extern void APIENTRY sgx_read_pass(sg_pass pass, sg_buffer buffer, void *data, size_t size);
extern void APIENTRY sgx_push_group(const char *text);
extern void APIENTRY sgx_pop_group();

#if defined(NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN)
extern bool APIENTRY sgx_isvalid(void);
extern bool APIENTRY sgx_query_buffer_overflow(sg_buffer buf);
extern int APIENTRY sgx_append_buffer(sg_buffer buf, const void *data_ptr, int data_size);
extern sg_backend APIENTRY sgx_query_backend(void);
extern sg_buffer APIENTRY sgx_alloc_buffer(void);
extern sg_buffer APIENTRY sgx_make_buffer(const sg_buffer_desc *desc);
extern sg_buffer_desc APIENTRY sgx_query_buffer_defaults(const sg_buffer_desc *desc);
extern sg_buffer_info APIENTRY sgx_query_buffer_info(sg_buffer buf);
extern sg_context APIENTRY sgx_setup_context(void);
extern sg_desc APIENTRY sgx_query_desc(void);
extern sg_features APIENTRY sgx_query_features(void);
extern sg_image APIENTRY sgx_alloc_image(void);
extern sg_image APIENTRY sgx_make_image(const sg_image_desc *desc);
extern sg_image_desc APIENTRY sgx_query_image_defaults(const sg_image_desc *desc);
extern sg_image_info APIENTRY sgx_query_image_info(sg_image img);
extern sg_limits APIENTRY sgx_query_limits(void);
extern sg_pass APIENTRY sgx_alloc_pass(void);
extern sg_pass APIENTRY sgx_make_pass(const sg_pass_desc *desc);
extern sg_pass_desc APIENTRY sgx_query_pass_defaults(const sg_pass_desc *desc);
extern sg_pass_info APIENTRY sgx_query_pass_info(sg_pass pass);
extern sg_pipeline APIENTRY sgx_alloc_pipeline(void);
extern sg_pipeline APIENTRY sgx_make_pipeline(const sg_pipeline_desc *desc);
extern sg_pipeline_desc APIENTRY sgx_query_pipeline_defaults(const sg_pipeline_desc *desc);
extern sg_pipeline_info APIENTRY sgx_query_pipeline_info(sg_pipeline pip);
extern sg_pixelformat_info APIENTRY sgx_query_pixelformat(sg_pixel_format fmt);
extern sg_resource_state APIENTRY sgx_query_buffer_state(sg_buffer buf);
extern sg_resource_state APIENTRY sgx_query_image_state(sg_image img);
extern sg_resource_state APIENTRY sgx_query_pass_state(sg_pass pass);
extern sg_resource_state APIENTRY sgx_query_pipeline_state(sg_pipeline pip);
extern sg_resource_state APIENTRY sgx_query_shader_state(sg_shader shd);
extern sg_shader APIENTRY sgx_alloc_shader(void);
extern sg_shader APIENTRY sgx_make_shader(const sg_shader_desc *desc);
extern sg_shader_desc APIENTRY sgx_query_shader_defaults(const sg_shader_desc *desc);
extern sg_shader_info APIENTRY sgx_query_shader_info(sg_shader shd);
extern sg_trace_hooks APIENTRY sgx_install_trace_hooks(const sg_trace_hooks *trace_hooks);
extern void APIENTRY sgx_activate_context(sg_context ctx_id);
extern void APIENTRY sgx_apply_bindings(const sg_bindings *bindings);
extern void APIENTRY sgx_apply_pipeline(sg_pipeline pip);
extern void APIENTRY sgx_apply_scissor_rect(int x, int y, int width, int height, bool origin_top_left);
extern void APIENTRY sgx_apply_uniforms(sg_shader_stage stage, int ub_index, const void *data, int num_bytes);
extern void APIENTRY sgx_apply_viewport(int x, int y, int width, int height, bool origin_top_left);
extern void APIENTRY sgx_begin_default_pass(const sg_pass_action *pass_action, int width, int height);
extern void APIENTRY sgx_begin_pass(sg_pass pass, const sg_pass_action *pass_action);
extern void APIENTRY sgx_commit(void);
extern void APIENTRY sgx_destroy_buffer(sg_buffer buf);
extern void APIENTRY sgx_destroy_image(sg_image img);
extern void APIENTRY sgx_destroy_pass(sg_pass pass);
extern void APIENTRY sgx_destroy_pipeline(sg_pipeline pip);
extern void APIENTRY sgx_destroy_shader(sg_shader shd);
extern void APIENTRY sgx_discard_context(sg_context ctx_id);
extern void APIENTRY sgx_draw(int base_element, int num_elements, int num_instances);
extern void APIENTRY sgx_end_pass(void);
extern void APIENTRY sgx_fail_buffer(sg_buffer buf_id);
extern void APIENTRY sgx_fail_image(sg_image img_id);
extern void APIENTRY sgx_fail_pass(sg_pass pass_id);
extern void APIENTRY sgx_fail_pipeline(sg_pipeline pip_id);
extern void APIENTRY sgx_fail_shader(sg_shader shd_id);
extern void APIENTRY sgx_init_buffer(sg_buffer buf_id, const sg_buffer_desc *desc);
extern void APIENTRY sgx_init_image(sg_image img_id, const sg_image_desc *desc);
extern void APIENTRY sgx_init_pass(sg_pass pass_id, const sg_pass_desc *desc);
extern void APIENTRY sgx_init_pipeline(sg_pipeline pip_id, const sg_pipeline_desc *desc);
extern void APIENTRY sgx_init_shader(sg_shader shd_id, const sg_shader_desc *desc);
extern void APIENTRY sgx_insert_marker(const char *text);
extern void APIENTRY sgx_label_buffer(sg_buffer buffer, const char *text);
extern void APIENTRY sgx_label_image(sg_image image, const char *text);
extern void APIENTRY sgx_label_pass(sg_pass pass, const char *text);
extern void APIENTRY sgx_label_pipeline(sg_pipeline pipeline, const char *text);
extern void APIENTRY sgx_label_shader(sg_shader shader, const char *text);
extern void APIENTRY sgx_pop_debug_group(void);
extern void APIENTRY sgx_pop_group();
extern void APIENTRY sgx_push_debug_group(const char *name);
extern void APIENTRY sgx_push_group(const char *text);
extern void APIENTRY sgx_read_image(sg_image image, void *data, size_t size);
extern void APIENTRY sgx_read_pass(sg_pass pass, void *data, size_t size);
extern void APIENTRY sgx_reset_state_cache(void);
extern void APIENTRY sgx_setup(const sg_desc *desc);
extern void APIENTRY sgx_shutdown(void);
extern void APIENTRY sgx_unmap_buffer(sg_buffer buffer, void *address);
extern void APIENTRY sgx_update_buffer(sg_buffer buf, const void *data_ptr, int data_size);
extern void APIENTRY sgx_update_image(sg_image img, const sg_image_content *data);
extern void *APIENTRY sgx_map_buffer(sg_buffer buffer);
#endif

#if !defined(NANOEM_ENABLE_ICU) && !defined(NANOEM_ENABLE_MBWC) && !defined(NANOEM_ENABLE_CFSTRING) &&                 \
    !defined(NANOEM_ENABLE_QT)
nanoem_unicode_string_factory_t *APIENTRY
nanoemUnicodeStringFactoryCreateEXT(nanoem_status_t *status)
{
    return nanoemUnicodeStringFactoryCreate(status);
}
void APIENTRY
nanoemUnicodeStringFactoryDestroyEXT(nanoem_unicode_string_factory_t *factory)
{
    nanoemUnicodeStringFactoryDestroy(factory);
}
void APIENTRY
nanoemUnicodeStringFactoryToUtf8OnStackEXT(nanoem_unicode_string_factory_t *factory,
    const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_u8_t *buffer, nanoem_rsize_t capacity,
    nanoem_status_t *status)
{
    char *s = reinterpret_cast<char *>(nanoemUnicodeStringFactoryGetByteArray(factory, string, length, status));
    strncpy(reinterpret_cast<char *>(buffer), s, capacity);
    nanoemUnicodeStringFactoryDestroyByteArray(factory, reinterpret_cast<nanoem_u8_t *>(s));
}
nanoem_rsize_t APIENTRY
nanoemUnicodeStringGetLength(const nanoem_unicode_string_t *string)
{
    return string ? strlen(reinterpret_cast<const char *>(string)) : 0;
}
#endif

} /* extern C APIENTRY */

namespace nanoem {

const char *const Constants::kGlobalSansFontFace = "sans";
const char *const Constants::kGlobalIconFontFace = "icon";
nanoem_f32_t Constants::kEpsilon = glm::epsilon<nanoem_f32_t>();
const Matrix4x4 Constants::kIdentity = Matrix4x4(1);
const Quaternion Constants::kZeroQ = Quaternion(1, 0, 0, 0);
const Vector4 Constants::kZeroV4 = Vector4(0);
const Vector3 Constants::kEpsilonVec3 = Vector3(glm::epsilon<nanoem_f32_t>());
const Vector3 Constants::kUnitX = Vector3(1, 0, 0);
const Vector3 Constants::kUnitY = Vector3(0, 1, 0);
const Vector3 Constants::kUnitZ = Vector3(0, 0, 1);
const Vector3 Constants::kZeroV3 = Vector3(0);
const nanoem_u32_t Constants::kHalfBaseFPS = 30;
const nanoem_f32_t Constants::kHalfBaseFPSFloat = nanoem_f32_t(Constants::kHalfBaseFPS);
const Vector4 Constants::kOrientateDirection = Vector4(1);
const Vector3 Constants::kTranslateDirection = Vector3(1);

namespace sg {

PFN_sgx_activate_context activate_context = nullptr;
PFN_sgx_add_commit_listener add_commit_listener = nullptr;
PFN_sgx_alloc_buffer alloc_buffer = nullptr;
PFN_sgx_alloc_image alloc_image = nullptr;
PFN_sgx_alloc_pass alloc_pass = nullptr;
PFN_sgx_alloc_pipeline alloc_pipeline = nullptr;
PFN_sgx_alloc_sampler alloc_sampler = nullptr;
PFN_sgx_alloc_shader alloc_shader = nullptr;
PFN_sgx_append_buffer append_buffer = nullptr;
PFN_sgx_apply_bindings apply_bindings = nullptr;
PFN_sgx_apply_pipeline apply_pipeline = nullptr;
PFN_sgx_apply_scissor_rect apply_scissor_rect = nullptr;
PFN_sgx_apply_uniforms apply_uniforms = nullptr;
PFN_sgx_apply_viewport apply_viewport = nullptr;
PFN_sgx_begin_default_pass begin_default_pass = nullptr;
PFN_sgx_begin_pass begin_pass = nullptr;
PFN_sgx_commit commit = nullptr;
PFN_sgx_dealloc_buffer dealloc_buffer = nullptr;
PFN_sgx_dealloc_image dealloc_image = nullptr;
PFN_sgx_dealloc_pass dealloc_pass = nullptr;
PFN_sgx_dealloc_pipeline dealloc_pipeline = nullptr;
PFN_sgx_dealloc_sampler dealloc_sampler = nullptr;
PFN_sgx_dealloc_shader dealloc_shader = nullptr;
PFN_sgx_destroy_buffer destroy_buffer = nullptr;
PFN_sgx_destroy_image destroy_image = nullptr;
PFN_sgx_destroy_pass destroy_pass = nullptr;
PFN_sgx_destroy_pipeline destroy_pipeline = nullptr;
PFN_sgx_destroy_sampler destroy_sampler = nullptr;
PFN_sgx_destroy_shader destroy_shader = nullptr;
PFN_sgx_discard_context discard_context = nullptr;
PFN_sgx_draw draw = nullptr;
PFN_sgx_end_pass end_pass = nullptr;
PFN_sgx_insert_marker insert_marker = nullptr;
PFN_sgx_label_buffer label_buffer = nullptr;
PFN_sgx_label_image label_image = nullptr;
PFN_sgx_label_pass label_pass = nullptr;
PFN_sgx_label_pipeline label_pipeline = nullptr;
PFN_sgx_label_sampler label_sampler = nullptr;
PFN_sgx_label_shader label_shader = nullptr;
PFN_sgx_fail_buffer fail_buffer = nullptr;
PFN_sgx_fail_image fail_image = nullptr;
PFN_sgx_fail_pass fail_pass = nullptr;
PFN_sgx_fail_pipeline fail_pipeline = nullptr;
PFN_sgx_fail_sampler fail_sampler = nullptr;
PFN_sgx_fail_shader fail_shader = nullptr;
PFN_sgx_init_buffer init_buffer = nullptr;
PFN_sgx_init_image init_image = nullptr;
PFN_sgx_init_pass init_pass = nullptr;
PFN_sgx_init_pipeline init_pipeline = nullptr;
PFN_sgx_init_sampler init_sampler = nullptr;
PFN_sgx_init_shader init_shader = nullptr;
PFN_sgx_install_trace_hooks install_trace_hooks = nullptr;
PFN_sgx_isvalid isvalid = nullptr;
PFN_sgx_make_buffer make_buffer = nullptr;
PFN_sgx_make_image make_image = nullptr;
PFN_sgx_make_pass make_pass = nullptr;
PFN_sgx_make_pipeline make_pipeline = nullptr;
PFN_sgx_make_sampler make_sampler = nullptr;
PFN_sgx_make_shader make_shader = nullptr;
PFN_sgx_map_buffer map_buffer = nullptr;
PFN_sgx_pop_group pop_group = nullptr;
PFN_sgx_push_group push_group = nullptr;
PFN_sgx_pop_debug_group pop_debug_group = nullptr;
PFN_sgx_push_debug_group push_debug_group = nullptr;
PFN_sgx_query_backend query_backend = nullptr;
PFN_sgx_query_buffer_defaults query_buffer_defaults = nullptr;
PFN_sgx_query_buffer_info query_buffer_info = nullptr;
PFN_sgx_query_buffer_overflow query_buffer_overflow = nullptr;
PFN_sgx_query_buffer_state query_buffer_state = nullptr;
PFN_sgx_query_buffer_will_overflow query_buffer_will_overflow = nullptr;
PFN_sgx_query_desc query_desc = nullptr;
PFN_sgx_query_features query_features = nullptr;
PFN_sgx_query_image_defaults query_image_defaults = nullptr;
PFN_sgx_query_image_info query_image_info = nullptr;
PFN_sgx_query_image_state query_image_state = nullptr;
PFN_sgx_query_limits query_limits = nullptr;
PFN_sgx_query_pass_defaults query_pass_defaults = nullptr;
PFN_sgx_query_pass_info query_pass_info = nullptr;
PFN_sgx_query_pass_state query_pass_state = nullptr;
PFN_sgx_query_pipeline_defaults query_pipeline_defaults = nullptr;
PFN_sgx_query_pipeline_info query_pipeline_info = nullptr;
PFN_sgx_query_pipeline_state query_pipeline_state = nullptr;
PFN_sgx_query_pixelformat query_pixelformat = nullptr;
PFN_sgx_query_sampler_defaults query_sampler_defaults = nullptr;
PFN_sgx_query_sampler_info query_sampler_info = nullptr;
PFN_sgx_query_sampler_state query_sampler_state = nullptr;
PFN_sgx_query_shader_defaults query_shader_defaults = nullptr;
PFN_sgx_query_shader_info query_shader_info = nullptr;
PFN_sgx_query_shader_state query_shader_state = nullptr;
PFN_sgx_read_image read_image = nullptr;
PFN_sgx_read_pass read_pass = nullptr;
PFN_sgx_read_pass_async read_pass_async = nullptr;
PFN_sgx_remove_commit_listener remove_commit_listener = nullptr;
PFN_sgx_reset_state_cache reset_state_cache = nullptr;
PFN_sgx_setup setup = nullptr;
PFN_sgx_setup_context setup_context = nullptr;
PFN_sgx_shutdown shutdown = nullptr;
PFN_sgx_uninit_buffer uninit_buffer = nullptr;
PFN_sgx_uninit_image uninit_image = nullptr;
PFN_sgx_uninit_pass uninit_pass = nullptr;
PFN_sgx_uninit_pipeline uninit_pipeline = nullptr;
PFN_sgx_uninit_sampler uninit_sampler = nullptr;
PFN_sgx_uninit_shader uninit_shader = nullptr;
PFN_sgx_unmap_buffer unmap_buffer = nullptr;
PFN_sgx_update_buffer update_buffer = nullptr;
PFN_sgx_update_image update_image = nullptr;

void *
openSharedLibrary(const char *dllPath)
{
#if defined(NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN)
    void *handle = nullptr;
    activate_context = sgx_activate_context;
    alloc_buffer = sgx_alloc_buffer;
    alloc_image = sgx_alloc_image;
    alloc_pass = sgx_alloc_pass;
    alloc_pipeline = sgx_alloc_pipeline;
    alloc_shader = sgx_alloc_shader;
    append_buffer = sgx_append_buffer;
    apply_bindings = sgx_apply_bindings;
    apply_pipeline = sgx_apply_pipeline;
    apply_scissor_rect = sgx_apply_scissor_rect;
    apply_uniforms = sgx_apply_uniforms;
    apply_viewport = sgx_apply_viewport;
    begin_default_pass = sgx_begin_default_pass;
    begin_pass = sgx_begin_pass;
    commit = sgx_commit;
    dealloc_buffer = sgx_dealloc_buffer;
    dealloc_image = sgx_dealloc_image;
    dealloc_shader = sgx_dealloc_shader;
    dealloc_pipeline = sgx_dealloc_pipeline;
    dealloc_pass = sgx_dealloc_pass;
    shutdown = sgx_shutdown;
    destroy_buffer = sgx_destroy_buffer;
    destroy_image = sgx_destroy_image;
    destroy_pass = sgx_destroy_pass;
    destroy_pipeline = sgx_destroy_pipeline;
    destroy_shader = sgx_destroy_shader;
    discard_context = sgx_discard_context;
    draw = sgx_draw;
    end_pass = sgx_end_pass;
    insert_marker = sgx_insert_marker;
    label_buffer = sgx_label_buffer;
    label_image = sgx_label_image;
    label_pass = sgx_label_pass;
    label_pipeline = sgx_label_pipeline;
    label_shader = sgx_label_shader;
    fail_buffer = sgx_fail_buffer;
    fail_image = sgx_fail_image;
    fail_pass = sgx_fail_pass;
    fail_pipeline = sgx_fail_pipeline;
    fail_shader = sgx_fail_shader;
    init_buffer = sgx_init_buffer;
    init_image = sgx_init_image;
    init_pass = sgx_init_pass;
    init_pipeline = sgx_init_pipeline;
    init_shader = sgx_init_shader;
    install_trace_hooks = sgx_install_trace_hooks;
    isvalid = sgx_isvalid;
    make_buffer = sgx_make_buffer;
    make_image = sgx_make_image;
    make_pass = sgx_make_pass;
    make_pipeline = sgx_make_pipeline;
    make_shader = sgx_make_shader;
    map_buffer = sgx_map_buffer;
    pop_group = sgx_pop_group;
    push_group = sgx_push_group;
    pop_debug_group = sgx_pop_debug_group;
    push_debug_group = sgx_push_debug_group;
    query_backend = sgx_query_backend;
    query_buffer_defaults = sgx_query_buffer_defaults;
    query_buffer_info = sgx_query_buffer_info;
    query_buffer_overflow = sgx_query_buffer_overflow;
    query_buffer_state = sgx_query_buffer_state;
    query_desc = sgx_query_desc;
    query_features = sgx_query_features;
    query_image_defaults = sgx_query_image_defaults;
    query_image_info = sgx_query_image_info;
    query_image_state = sgx_query_image_state;
    query_limits = sgx_query_limits;
    query_pass_defaults = sgx_query_pass_defaults;
    query_pass_info = sgx_query_pass_info;
    query_pass_state = sgx_query_pass_state;
    query_pipeline_defaults = sgx_query_pipeline_defaults;
    query_pipeline_info = sgx_query_pipeline_info;
    query_pipeline_state = sgx_query_pipeline_state;
    query_pixelformat = sgx_query_pixelformat;
    query_shader_defaults = sgx_query_shader_defaults;
    query_shader_info = sgx_query_shader_info;
    query_shader_state = sgx_query_shader_state;
    reset_state_cache = sgx_reset_state_cache;
    setup = sgx_setup;
    read_image = sgx_read_image;
    read_pass = sgx_read_pass;
    read_pass_async = sgx_read_pass_async;
    setup_context = sgx_setup_context;
    shutdown = sgx_shutdown;
    uninit_buffer = sgx_uninit_buffer;
    uninit_image = sgx_uninit_image;
    uninit_shader = sgx_uninit_shader;
    uninit_pipeline = sgx_uninit_pipeline;
    uninit_pass = sgx_uninit_pass;
    unmap_buffer = sgx_unmap_buffer;
    update_buffer = sgx_update_buffer;
    update_image = sgx_update_image;
#else
    void *handle = bx::dlopen(dllPath);
    if (handle) {
        activate_context = reinterpret_cast<PFN_sgx_activate_context>(bx::dlsym(handle, "sgx_activate_context"));
        add_commit_listener =
            reinterpret_cast<PFN_sgx_add_commit_listener>(bx::dlsym(handle, "sgx_add_commit_listener"));
        alloc_buffer = reinterpret_cast<PFN_sgx_alloc_buffer>(bx::dlsym(handle, "sgx_alloc_buffer"));
        alloc_image = reinterpret_cast<PFN_sgx_alloc_image>(bx::dlsym(handle, "sgx_alloc_image"));
        alloc_pass = reinterpret_cast<PFN_sgx_alloc_pass>(bx::dlsym(handle, "sgx_alloc_pass"));
        alloc_pipeline = reinterpret_cast<PFN_sgx_alloc_pipeline>(bx::dlsym(handle, "sgx_alloc_pipeline"));
        alloc_sampler = reinterpret_cast<PFN_sgx_alloc_sampler>(bx::dlsym(handle, "sgx_alloc_sampler"));
        alloc_shader = reinterpret_cast<PFN_sgx_alloc_shader>(bx::dlsym(handle, "sgx_alloc_shader"));
        append_buffer = reinterpret_cast<PFN_sgx_append_buffer>(bx::dlsym(handle, "sgx_append_buffer"));
        apply_bindings = reinterpret_cast<PFN_sgx_apply_bindings>(bx::dlsym(handle, "sgx_apply_bindings"));
        apply_pipeline = reinterpret_cast<PFN_sgx_apply_pipeline>(bx::dlsym(handle, "sgx_apply_pipeline"));
        apply_scissor_rect = reinterpret_cast<PFN_sgx_apply_scissor_rect>(bx::dlsym(handle, "sgx_apply_scissor_rect"));
        apply_uniforms = reinterpret_cast<PFN_sgx_apply_uniforms>(bx::dlsym(handle, "sgx_apply_uniforms"));
        apply_viewport = reinterpret_cast<PFN_sgx_apply_viewport>(bx::dlsym(handle, "sgx_apply_viewport"));
        begin_default_pass = reinterpret_cast<PFN_sgx_begin_default_pass>(bx::dlsym(handle, "sgx_begin_default_pass"));
        begin_pass = reinterpret_cast<PFN_sgx_begin_pass>(bx::dlsym(handle, "sgx_begin_pass"));
        commit = reinterpret_cast<PFN_sgx_commit>(bx::dlsym(handle, "sgx_commit"));
        dealloc_buffer = reinterpret_cast<PFN_sgx_dealloc_buffer>(bx::dlsym(handle, "sgx_dealloc_buffer"));
        dealloc_image = reinterpret_cast<PFN_sgx_dealloc_image>(bx::dlsym(handle, "sgx_dealloc_image"));
        dealloc_pipeline = reinterpret_cast<PFN_sgx_dealloc_pipeline>(bx::dlsym(handle, "sgx_dealloc_pipeline"));
        dealloc_pass = reinterpret_cast<PFN_sgx_dealloc_pass>(bx::dlsym(handle, "sgx_dealloc_pass"));
        dealloc_sampler = reinterpret_cast<PFN_sgx_dealloc_sampler>(bx::dlsym(handle, "sgx_dealloc_sampler"));
        dealloc_shader = reinterpret_cast<PFN_sgx_dealloc_shader>(bx::dlsym(handle, "sgx_dealloc_shader"));
        destroy_buffer = reinterpret_cast<PFN_sgx_destroy_buffer>(bx::dlsym(handle, "sgx_destroy_buffer"));
        destroy_image = reinterpret_cast<PFN_sgx_destroy_image>(bx::dlsym(handle, "sgx_destroy_image"));
        destroy_pass = reinterpret_cast<PFN_sgx_destroy_pass>(bx::dlsym(handle, "sgx_destroy_pass"));
        destroy_pipeline = reinterpret_cast<PFN_sgx_destroy_pipeline>(bx::dlsym(handle, "sgx_destroy_pipeline"));
        destroy_sampler = reinterpret_cast<PFN_sgx_destroy_sampler>(bx::dlsym(handle, "sgx_destroy_sampler"));
        destroy_shader = reinterpret_cast<PFN_sgx_destroy_shader>(bx::dlsym(handle, "sgx_destroy_shader"));
        discard_context = reinterpret_cast<PFN_sgx_discard_context>(bx::dlsym(handle, "sgx_discard_context"));
        draw = reinterpret_cast<PFN_sgx_draw>(bx::dlsym(handle, "sgx_draw"));
        end_pass = reinterpret_cast<PFN_sgx_end_pass>(bx::dlsym(handle, "sgx_end_pass"));
        insert_marker = reinterpret_cast<PFN_sgx_insert_marker>(bx::dlsym(handle, "sgx_insert_marker"));
        label_buffer = reinterpret_cast<PFN_sgx_label_buffer>(bx::dlsym(handle, "sgx_label_buffer"));
        label_image = reinterpret_cast<PFN_sgx_label_image>(bx::dlsym(handle, "sgx_label_image"));
        label_pass = reinterpret_cast<PFN_sgx_label_pass>(bx::dlsym(handle, "sgx_label_pass"));
        label_pipeline = reinterpret_cast<PFN_sgx_label_pipeline>(bx::dlsym(handle, "sgx_label_pipeline"));
        label_sampler = reinterpret_cast<PFN_sgx_label_sampler>(bx::dlsym(handle, "sgx_label_sampler"));
        label_shader = reinterpret_cast<PFN_sgx_label_shader>(bx::dlsym(handle, "sgx_label_shader"));
        fail_buffer = reinterpret_cast<PFN_sgx_fail_buffer>(bx::dlsym(handle, "sgx_fail_buffer"));
        fail_image = reinterpret_cast<PFN_sgx_fail_image>(bx::dlsym(handle, "sgx_fail_image"));
        fail_pass = reinterpret_cast<PFN_sgx_fail_pass>(bx::dlsym(handle, "sgx_fail_pass"));
        fail_pipeline = reinterpret_cast<PFN_sgx_fail_pipeline>(bx::dlsym(handle, "sgx_fail_pipeline"));
        fail_sampler = reinterpret_cast<PFN_sgx_fail_sampler>(bx::dlsym(handle, "sgx_fail_sampler"));
        fail_shader = reinterpret_cast<PFN_sgx_fail_shader>(bx::dlsym(handle, "sgx_fail_shader"));
        init_buffer = reinterpret_cast<PFN_sgx_init_buffer>(bx::dlsym(handle, "sgx_init_buffer"));
        init_image = reinterpret_cast<PFN_sgx_init_image>(bx::dlsym(handle, "sgx_init_image"));
        init_pass = reinterpret_cast<PFN_sgx_init_pass>(bx::dlsym(handle, "sgx_init_pass"));
        init_pipeline = reinterpret_cast<PFN_sgx_init_pipeline>(bx::dlsym(handle, "sgx_init_pipeline"));
        init_sampler = reinterpret_cast<PFN_sgx_init_sampler>(bx::dlsym(handle, "sgx_init_sampler"));
        init_shader = reinterpret_cast<PFN_sgx_init_shader>(bx::dlsym(handle, "sgx_init_shader"));
        install_trace_hooks =
            reinterpret_cast<PFN_sgx_install_trace_hooks>(bx::dlsym(handle, "sgx_install_trace_hooks"));
        isvalid = reinterpret_cast<PFN_sgx_isvalid>(bx::dlsym(handle, "sgx_isvalid"));
        make_buffer = reinterpret_cast<PFN_sgx_make_buffer>(bx::dlsym(handle, "sgx_make_buffer"));
        make_image = reinterpret_cast<PFN_sgx_make_image>(bx::dlsym(handle, "sgx_make_image"));
        make_pass = reinterpret_cast<PFN_sgx_make_pass>(bx::dlsym(handle, "sgx_make_pass"));
        make_pipeline = reinterpret_cast<PFN_sgx_make_pipeline>(bx::dlsym(handle, "sgx_make_pipeline"));
        make_sampler = reinterpret_cast<PFN_sgx_make_sampler>(bx::dlsym(handle, "sgx_make_sampler"));
        make_shader = reinterpret_cast<PFN_sgx_make_shader>(bx::dlsym(handle, "sgx_make_shader"));
        map_buffer = reinterpret_cast<PFN_sgx_map_buffer>(bx::dlsym(handle, "sgx_map_buffer"));
        pop_group = reinterpret_cast<PFN_sgx_pop_group>(bx::dlsym(handle, "sgx_pop_group"));
        push_group = reinterpret_cast<PFN_sgx_push_group>(bx::dlsym(handle, "sgx_push_group"));
        pop_debug_group = reinterpret_cast<PFN_sgx_pop_debug_group>(bx::dlsym(handle, "sgx_pop_debug_group"));
        push_debug_group = reinterpret_cast<PFN_sgx_push_debug_group>(bx::dlsym(handle, "sgx_push_debug_group"));
        query_backend = reinterpret_cast<PFN_sgx_query_backend>(bx::dlsym(handle, "sgx_query_backend"));
        query_buffer_defaults =
            reinterpret_cast<PFN_sgx_query_buffer_defaults>(bx::dlsym(handle, "sgx_query_buffer_defaults"));
        query_buffer_info = reinterpret_cast<PFN_sgx_query_buffer_info>(bx::dlsym(handle, "sgx_query_buffer_info"));
        query_buffer_overflow =
            reinterpret_cast<PFN_sgx_query_buffer_overflow>(bx::dlsym(handle, "sgx_query_buffer_overflow"));
        query_buffer_state = reinterpret_cast<PFN_sgx_query_buffer_state>(bx::dlsym(handle, "sgx_query_buffer_state"));
        query_buffer_will_overflow =
            reinterpret_cast<PFN_sgx_query_buffer_will_overflow>(bx::dlsym(handle, "sgx_query_buffer_will_overflow"));
        query_desc = reinterpret_cast<PFN_sgx_query_desc>(bx::dlsym(handle, "sgx_query_desc"));
        query_features = reinterpret_cast<PFN_sgx_query_features>(bx::dlsym(handle, "sgx_query_features"));
        query_image_defaults =
            reinterpret_cast<PFN_sgx_query_image_defaults>(bx::dlsym(handle, "sgx_query_image_defaults"));
        query_image_info = reinterpret_cast<PFN_sgx_query_image_info>(bx::dlsym(handle, "sgx_query_image_info"));
        query_image_state = reinterpret_cast<PFN_sgx_query_image_state>(bx::dlsym(handle, "sgx_query_image_state"));
        query_limits = reinterpret_cast<PFN_sgx_query_limits>(bx::dlsym(handle, "sgx_query_limits"));
        query_pass_defaults =
            reinterpret_cast<PFN_sgx_query_pass_defaults>(bx::dlsym(handle, "sgx_query_pass_defaults"));
        query_pass_info = reinterpret_cast<PFN_sgx_query_pass_info>(bx::dlsym(handle, "sgx_query_pass_info"));
        query_pass_state = reinterpret_cast<PFN_sgx_query_pass_state>(bx::dlsym(handle, "sgx_query_pass_state"));
        query_pipeline_defaults =
            reinterpret_cast<PFN_sgx_query_pipeline_defaults>(bx::dlsym(handle, "sgx_query_pipeline_defaults"));
        query_pipeline_info =
            reinterpret_cast<PFN_sgx_query_pipeline_info>(bx::dlsym(handle, "sgx_query_pipeline_info"));
        query_pipeline_state =
            reinterpret_cast<PFN_sgx_query_pipeline_state>(bx::dlsym(handle, "sgx_query_pipeline_state"));
        query_pixelformat = reinterpret_cast<PFN_sgx_query_pixelformat>(bx::dlsym(handle, "sgx_query_pixelformat"));
        query_shader_defaults =
            reinterpret_cast<PFN_sgx_query_shader_defaults>(bx::dlsym(handle, "sgx_query_shader_defaults"));
        query_sampler_defaults =
            reinterpret_cast<PFN_sgx_query_sampler_defaults>(bx::dlsym(handle, "sgx_query_sampler_defaults"));
        query_sampler_info = reinterpret_cast<PFN_sgx_query_sampler_info>(bx::dlsym(handle, "sgx_query_sampler_info"));
        query_sampler_state = reinterpret_cast<PFN_sgx_query_sampler_state>(bx::dlsym(handle, "sgx_query_sampler_state"));
        query_shader_info = reinterpret_cast<PFN_sgx_query_shader_info>(bx::dlsym(handle, "sgx_query_shader_info"));
        query_shader_state = reinterpret_cast<PFN_sgx_query_shader_state>(bx::dlsym(handle, "sgx_query_shader_state"));
        reset_state_cache = reinterpret_cast<PFN_sgx_reset_state_cache>(bx::dlsym(handle, "sgx_reset_state_cache"));
        setup = reinterpret_cast<PFN_sgx_setup>(bx::dlsym(handle, "sgx_setup"));
        read_image = reinterpret_cast<PFN_sgx_read_image>(bx::dlsym(handle, "sgx_read_image"));
        read_pass = reinterpret_cast<PFN_sgx_read_pass>(bx::dlsym(handle, "sgx_read_pass"));
        read_pass_async = reinterpret_cast<PFN_sgx_read_pass_async>(bx::dlsym(handle, "sgx_read_pass_async"));
        remove_commit_listener =
            reinterpret_cast<PFN_sgx_remove_commit_listener>(bx::dlsym(handle, "sgx_remove_commit_listener"));
        setup_context = reinterpret_cast<PFN_sgx_setup_context>(bx::dlsym(handle, "sgx_setup_context"));
        shutdown = reinterpret_cast<PFN_sgx_shutdown>(bx::dlsym(handle, "sgx_shutdown"));
        uninit_buffer = reinterpret_cast<PFN_sgx_uninit_buffer>(bx::dlsym(handle, "sgx_uninit_buffer"));
        uninit_image = reinterpret_cast<PFN_sgx_uninit_image>(bx::dlsym(handle, "sgx_uninit_image"));
        uninit_pipeline = reinterpret_cast<PFN_sgx_uninit_pipeline>(bx::dlsym(handle, "sgx_uninit_pipeline"));
        uninit_pass = reinterpret_cast<PFN_sgx_uninit_pass>(bx::dlsym(handle, "sgx_uninit_pass"));
        uninit_sampler = reinterpret_cast<PFN_sgx_uninit_sampler>(bx::dlsym(handle, "sgx_uninit_sampler"));
        uninit_shader = reinterpret_cast<PFN_sgx_uninit_shader>(bx::dlsym(handle, "sgx_uninit_shader"));
        unmap_buffer = reinterpret_cast<PFN_sgx_unmap_buffer>(bx::dlsym(handle, "sgx_unmap_buffer"));
        update_buffer = reinterpret_cast<PFN_sgx_update_buffer>(bx::dlsym(handle, "sgx_update_buffer"));
        update_image = reinterpret_cast<PFN_sgx_update_image>(bx::dlsym(handle, "sgx_update_image"));
    }
#endif
    return handle;
}

void
closeSharedLibrary(void *&handle)
{
    if (handle) {
        bx::dlclose(handle);
        handle = nullptr;
    }
}

bool
is_valid(sg_buffer value) NANOEM_DECL_NOEXCEPT
{
    return value.id != SG_INVALID_ID;
}

bool
is_valid(sg_image value) NANOEM_DECL_NOEXCEPT
{
    return value.id != SG_INVALID_ID;
}

bool
is_valid(sg_pass value) NANOEM_DECL_NOEXCEPT
{
    return value.id != SG_INVALID_ID;
}

bool
is_valid(sg_pipeline value) NANOEM_DECL_NOEXCEPT
{
    return value.id != SG_INVALID_ID;
}

bool
is_valid(sg_sampler value) NANOEM_DECL_NOEXCEPT
{
    return value.id != SG_INVALID_ID;
}

bool
is_valid(sg_shader value) NANOEM_DECL_NOEXCEPT
{
    return value.id != SG_INVALID_ID;
}

bool
is_backend_metal(sg_backend value) NANOEM_DECL_NOEXCEPT
{
    switch (value) {
    case SG_BACKEND_METAL_MACOS:
    case SG_BACKEND_METAL_IOS:
    case SG_BACKEND_METAL_SIMULATOR:
        return true;
    default:
        return false;
    }
}

void
insert_marker_format(const char *format, ...)
{
    char text[Inline::kMarkerStringLength];
    va_list ap;
    va_start(ap, format);
    StringUtils::formatVA(text, sizeof(text), format, ap);
    sg::insert_marker(text);
    va_end(ap);
}

void
push_group_format(const char *format, ...)
{
    char label[Inline::kMarkerStringLength];
    va_list ap;
    va_start(ap, format);
    StringUtils::formatVA(label, sizeof(label), format, ap);
    sg::push_group(label);
    va_end(ap);
}

void
PassBlock::initializeClearAction(sg_pass_action &action) NANOEM_DECL_NOEXCEPT
{
    Inline::clearZeroMemory(action);
    for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
        action.colors[i].load_action = SG_LOADACTION_CLEAR;
        action.colors[i].store_action = SG_STOREACTION_STORE;
    }
    action.depth.load_action = action.stencil.load_action = SG_LOADACTION_CLEAR;
    action.depth.store_action = action.stencil.store_action = SG_STOREACTION_STORE;
}

void
PassBlock::initializeLoadStoreAction(sg_pass_action &action) NANOEM_DECL_NOEXCEPT
{
    Inline::clearZeroMemory(action);
    for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
        action.colors[i].load_action = SG_LOADACTION_LOAD;
    }
    action.depth.load_action = action.stencil.load_action = SG_LOADACTION_LOAD;
}

PassBlock::PassBlock() NANOEM_DECL_NOEXCEPT : m_drawQueue(nullptr)
{
}

PassBlock::PassBlock(IDrawQueue *queue, sg_pass pass, const sg_pass_action &action) NANOEM_DECL_NOEXCEPT
    : m_drawQueue(queue)
{
    queue ? queue->beginPass(pass, action) : sg::begin_pass(pass, &action);
}

PassBlock::~PassBlock() NANOEM_DECL_NOEXCEPT
{
    if (m_drawQueue) {
        m_drawQueue->endPass();
    }
}

void
PassBlock::applyBindings(const sg_bindings &bindings)
{
    sg::apply_bindings(&bindings);
}

void
PassBlock::applyPipeline(sg_pipeline pipeline)
{
    nanoem_parameter_assert(sg::is_valid(pipeline), "pipeline must be present");
    sg::apply_pipeline(pipeline);
}

void
PassBlock::applyPipelineBindings(sg_pipeline pipeline, const sg_bindings &bindings)
{
    nanoem_parameter_assert(sg::is_valid(pipeline), "pipeline must be present");
    if (m_drawQueue) {
        m_drawQueue->applyPipelineBindings(pipeline, bindings);
    }
    else {
        sg::apply_pipeline(pipeline);
        sg::apply_bindings(&bindings);
    }
}

void
PassBlock::applyScissorRect(int x, int y, int width, int height)
{
    if (m_drawQueue) {
        m_drawQueue->applyScissorRect(x, y, width, height);
    }
    else {
        sg::apply_scissor_rect(x, y, width, height, true);
    }
}

void
PassBlock::applyViewport(int x, int y, int width, int height)
{
    if (m_drawQueue) {
        m_drawQueue->applyViewport(x, y, width, height);
    }
    else {
        sg::apply_viewport(x, y, width, height, true);
    }
}

void
PassBlock::applyUniformBlock(const void *data, nanoem_rsize_t size)
{
    if (m_drawQueue) {
        m_drawQueue->applyUniformBlock(data, size);
    }
    else {
        int sz = Inline::saturateInt32(size);
        sg::apply_uniforms(SG_SHADERSTAGE_VS, 0, data, sz);
        sg::apply_uniforms(SG_SHADERSTAGE_FS, 0, data, sz);
    }
}

void
PassBlock::applyUniformBlock(sg_shader_stage stage, const void *data, nanoem_rsize_t size)
{
    if (m_drawQueue) {
        m_drawQueue->applyUniformBlock(stage, data, size);
    }
    else {
        int sz = Inline::saturateInt32(size);
        sg::apply_uniforms(stage, 0, data, sz);
    }
}

void
PassBlock::draw(int offset, int count)
{
    if (m_drawQueue) {
        m_drawQueue->draw(offset, count);
    }
    else {
        sg::draw(offset, count, 1);
    }
}

void
PassBlock::registerCallback(Callback callback, void *userData)
{
    if (m_drawQueue) {
        m_drawQueue->registerCallback(callback, userData);
    }
}

void
QuadVertexUnit::generateQuadUV(
    nanoem_f32_t &minu, nanoem_f32_t &minv, nanoem_f32_t &maxu, nanoem_f32_t &maxv) NANOEM_DECL_NOEXCEPT
{
    if (sg::query_features().origin_top_left) {
        minu = 0.0f;
        minv = 0.0f;
        maxu = 1.0f;
        maxv = 1.0f;
    }
    else {
        minu = 0.0f;
        minv = 1.0f;
        maxu = 1.0f;
        maxv = 0.0f;
    }
}

void
QuadVertexUnit::generateQuadTriStrip(QuadVertexUnit *vertices) NANOEM_DECL_NOEXCEPT
{
    generateQuadTriStrip(vertices, -1, 1, 1, -1);
}

void
QuadVertexUnit::generateQuadTriStrip(QuadVertexUnit *vertices, nanoem_f32_t minx, nanoem_f32_t miny, nanoem_f32_t maxx,
    nanoem_f32_t maxy) NANOEM_DECL_NOEXCEPT
{
    nanoem_f32_t minu, minv, maxu, maxv;
    generateQuadUV(minu, minv, maxu, maxv);
    vertices[0].m_position = Vector4(minx, miny, 0, 0);
    vertices[1].m_position = Vector4(maxx, miny, 0, 0);
    vertices[2].m_position = Vector4(minx, maxy, 0, 0);
    vertices[3].m_position = Vector4(maxx, maxy, 0, 0);
    vertices[0].m_texcoord = Vector4(minu, minv, 0, 0);
    vertices[1].m_texcoord = Vector4(maxu, minv, 0, 0);
    vertices[2].m_texcoord = Vector4(minu, maxv, 0, 0);
    vertices[3].m_texcoord = Vector4(maxu, maxv, 0, 0);
}

} /* namespace sg */

void *
TinySTLAllocator::static_allocate(size_t bytes)
{
    return BX_ALLOC(g_tinystl_allocator, bytes);
}

void
TinySTLAllocator::static_deallocate(void *ptr, size_t bytes) NANOEM_DECL_NOEXCEPT
{
    BX_UNUSED_1(bytes);
    if (ptr) {
        BX_FREE(g_tinystl_allocator, ptr);
    }
}

} /* namespace nanoem */
