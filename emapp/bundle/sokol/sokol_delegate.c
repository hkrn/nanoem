#include <stdio.h> /* puts */
#include <stdlib.h>

#if defined(SOKOL_DEBUG) && SOKOL_DEBUG
#define SOKOL_MALLOC(s) g_sgx_malloc(g_sgx_opaque, (s), __FILE__, __LINE__)
#define SOKOL_FREE(p) g_sgx_free(g_sgx_opaque, (p), __FILE__, __LINE__)
#define SOKOL_LOG(l) g_sgx_logger(g_sgx_opaque, (l), __FILE__, __LINE__)
#else
#define SOKOL_MALLOC(s) g_sgx_malloc(g_sgx_opaque, (s), NULL, 0)
#define SOKOL_FREE(p) g_sgx_free(g_sgx_opaque, (p), NULL, 0)
#define SOKOL_LOG(l) g_sgx_logger(g_sgx_opaque, (l), NULL, 0)
#define SOKOL_VALIDATE_NON_FATAL
#endif

/* API export */
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#if defined(SOKOL_STATIC) && !SOKOL_STATIC
#define SGX_API_DECL extern __declspec(dllexport)
#else
#define SGX_API_DECL extern
#endif /* SOKOL_STATIC */
#else
#define SGX_API_DECL extern
#ifndef APIENTRY
#define APIENTRY
#endif /* APIENTRY */
#endif /* _WIN32 */

typedef void *(*sgx_malloc_t)(void *, size_t, const char *, int);
typedef void (*sgx_free_t)(void *, void *, const char *, int);
typedef void (*sgx_logger_t)(void *, const char *, const char *, int);

static void *sgx_malloc_default(void *opaque, size_t size, const char *file, int line);
static void sgx_free_default(void *opaque, void *ptr, const char *file, int line);
static void sgx_logger_default(void *opaque, const char *message, const char *file, int line);
static void *g_sgx_opaque = NULL;

static sgx_malloc_t g_sgx_malloc = sgx_malloc_default;
static sgx_free_t g_sgx_free = sgx_free_default;
static sgx_logger_t g_sgx_logger = sgx_logger_default;

#define SOKOL_NO_DEPRECATED
#define SOKOL_TRACE_HOOKS
#include "sokol/sokol_gfx.h"

static void *
sgx_malloc_default(void *opaque, size_t size, const char *file, int line)
{
    _SOKOL_UNUSED(opaque);
    _SOKOL_UNUSED(file);
    _SOKOL_UNUSED(line);
    return malloc(size);
}

static void
sgx_free_default(void *opaque, void *ptr, const char *file, int line)
{
    _SOKOL_UNUSED(opaque);
    _SOKOL_UNUSED(file);
    _SOKOL_UNUSED(line);
    free(ptr);
}

static void
sgx_logger_default(void *opaque, const char *message, const char *file, int line)
{
    _SOKOL_UNUSED(opaque);
    _SOKOL_UNUSED(file);
    _SOKOL_UNUSED(line);
    puts(message);
}

SGX_API_DECL void APIENTRY
sgx_install_allocator_hooks(sgx_malloc_t m, sgx_free_t f, sgx_logger_t l, void *o)
{
    if (m && f) {
        g_sgx_malloc = m;
        g_sgx_free = f;
        g_sgx_logger = l;
    }
    else {
        g_sgx_malloc = sgx_malloc_default;
        g_sgx_free = sgx_free_default;
        g_sgx_logger = sgx_logger_default;
    }
    g_sgx_opaque = o;
}

/* setup and misc functions */
SGX_API_DECL bool APIENTRY
sgx_isvalid(void)
{
    return sg_isvalid();
}

SGX_API_DECL void APIENTRY
sgx_reset_state_cache(void)
{
    sg_reset_state_cache();
}

SGX_API_DECL sg_trace_hooks APIENTRY
sgx_install_trace_hooks(const sg_trace_hooks *trace_hooks)
{
    return sg_install_trace_hooks(trace_hooks);
}
SGX_API_DECL void APIENTRY
sgx_push_debug_group(const char *name)
{
    sg_push_debug_group(name);
}

SGX_API_DECL void APIENTRY
sgx_pop_debug_group(void)
{
    sg_pop_debug_group();
}

SGX_API_DECL sg_buffer APIENTRY
sgx_make_buffer(const sg_buffer_desc *desc)
{
    return sg_make_buffer(desc);
}

SGX_API_DECL sg_image APIENTRY
sgx_make_image(const sg_image_desc *desc)
{
    return sg_make_image(desc);
}

SGX_API_DECL sg_shader APIENTRY
sgx_make_shader(const sg_shader_desc *desc)
{
    return sg_make_shader(desc);
}

SGX_API_DECL sg_pipeline APIENTRY
sgx_make_pipeline(const sg_pipeline_desc *desc)
{
    return sg_make_pipeline(desc);
}

SGX_API_DECL sg_pass APIENTRY
sgx_make_pass(const sg_pass_desc *desc)
{
    return sg_make_pass(desc);
}

SGX_API_DECL void APIENTRY
sgx_destroy_buffer(sg_buffer buf)
{
    sg_destroy_buffer(buf);
}

SGX_API_DECL void APIENTRY
sgx_destroy_image(sg_image img)
{
    sg_destroy_image(img);
}

SGX_API_DECL void APIENTRY
sgx_destroy_shader(sg_shader shd)
{
    sg_destroy_shader(shd);
}

SGX_API_DECL void APIENTRY
sgx_destroy_pipeline(sg_pipeline pip)
{
    sg_destroy_pipeline(pip);
}

SGX_API_DECL void APIENTRY
sgx_destroy_pass(sg_pass pass)
{
    sg_destroy_pass(pass);
}

SGX_API_DECL void APIENTRY
sgx_update_buffer(sg_buffer buf, const void *data_ptr, int data_size)
{
    const sg_range range = { data_ptr, data_size };
    sg_update_buffer(buf, &range);
}

SGX_API_DECL void APIENTRY
sgx_update_image(sg_image img, const sg_image_data *data)
{
    sg_update_image(img, data);
}

SGX_API_DECL int APIENTRY
sgx_append_buffer(sg_buffer buf, const void *data_ptr, int data_size)
{
    const sg_range range = { data_ptr, data_size };
    return sg_append_buffer(buf, &range);
}

SGX_API_DECL bool APIENTRY
sgx_query_buffer_overflow(sg_buffer buf)
{
    return sg_query_buffer_overflow(buf);
}

SGX_API_DECL void APIENTRY
sgx_begin_default_pass(const sg_pass_action *pass_action, int width, int height)
{
    sg_begin_default_pass(pass_action, width, height);
}

SGX_API_DECL void APIENTRY
sgx_begin_pass(sg_pass pass, const sg_pass_action *pass_action)
{
    sg_begin_pass(pass, pass_action);
}

SGX_API_DECL void APIENTRY
sgx_apply_viewport(int x, int y, int width, int height, bool origin_top_left)
{
    sg_apply_viewport(x, y, width, height, origin_top_left);
}

SGX_API_DECL void APIENTRY
sgx_apply_scissor_rect(int x, int y, int width, int height, bool origin_top_left)
{
    sg_apply_scissor_rect(x, y, width, height, origin_top_left);
}

SGX_API_DECL void APIENTRY
sgx_apply_pipeline(sg_pipeline pip)
{
    sg_apply_pipeline(pip);
}

SGX_API_DECL void APIENTRY
sgx_apply_bindings(const sg_bindings *bindings)
{
    sg_apply_bindings(bindings);
}

SGX_API_DECL void APIENTRY
sgx_apply_uniforms(sg_shader_stage stage, int ub_index, const void *data, int num_bytes)
{
    const sg_range range = { data, num_bytes };
    sg_apply_uniforms(stage, ub_index, &range);
}

SGX_API_DECL void APIENTRY
sgx_draw(int base_element, int num_elements, int num_instances)
{
    sg_draw(base_element, num_elements, num_instances);
}

SGX_API_DECL void APIENTRY
sgx_end_pass(void)
{
    sg_end_pass();
}

SGX_API_DECL void APIENTRY
sgx_commit(void)
{
    sg_commit();
}

SGX_API_DECL sg_desc APIENTRY
sgx_query_desc(void)
{
    return sg_query_desc();
}
SGX_API_DECL sg_backend APIENTRY
sgx_query_backend(void)
{
    return sg_query_backend();
}

SGX_API_DECL sg_features APIENTRY
sgx_query_features(void)
{
    return sg_query_features();
}

SGX_API_DECL sg_limits APIENTRY
sgx_query_limits(void)
{
    return sg_query_limits();
}

SGX_API_DECL sg_pixelformat_info APIENTRY
sgx_query_pixelformat(sg_pixel_format fmt)
{
    return sg_query_pixelformat(fmt);
}

SGX_API_DECL sg_resource_state APIENTRY
sgx_query_buffer_state(sg_buffer buf)
{
    return sg_query_buffer_state(buf);
}

SGX_API_DECL sg_resource_state APIENTRY
sgx_query_image_state(sg_image img)
{
    return sg_query_image_state(img);
}

SGX_API_DECL sg_resource_state APIENTRY
sgx_query_shader_state(sg_shader shd)
{
    return sg_query_shader_state(shd);
}

SGX_API_DECL sg_resource_state APIENTRY
sgx_query_pipeline_state(sg_pipeline pip)
{
    return sg_query_pipeline_state(pip);
}

SGX_API_DECL sg_resource_state APIENTRY
sgx_query_pass_state(sg_pass pass)
{
    return sg_query_pass_state(pass);
}

SGX_API_DECL sg_buffer_info APIENTRY
sgx_query_buffer_info(sg_buffer buf)
{
    return sg_query_buffer_info(buf);
}

SGX_API_DECL sg_image_info APIENTRY
sgx_query_image_info(sg_image img)
{
    return sg_query_image_info(img);
}

SGX_API_DECL sg_shader_info APIENTRY
sgx_query_shader_info(sg_shader shd)
{
    return sg_query_shader_info(shd);
}

SGX_API_DECL sg_pipeline_info APIENTRY
sgx_query_pipeline_info(sg_pipeline pip)
{
    return sg_query_pipeline_info(pip);
}

SGX_API_DECL sg_pass_info APIENTRY
sgx_query_pass_info(sg_pass pass)
{
    return sg_query_pass_info(pass);
}

SGX_API_DECL sg_buffer_desc APIENTRY
sgx_query_buffer_defaults(const sg_buffer_desc *desc)
{
    return sg_query_buffer_defaults(desc);
}

SGX_API_DECL sg_image_desc APIENTRY
sgx_query_image_defaults(const sg_image_desc *desc)
{
    return sg_query_image_defaults(desc);
}

SGX_API_DECL sg_shader_desc APIENTRY
sgx_query_shader_defaults(const sg_shader_desc *desc)
{
    return sg_query_shader_defaults(desc);
}

SGX_API_DECL sg_pipeline_desc APIENTRY
sgx_query_pipeline_defaults(const sg_pipeline_desc *desc)
{
    return sg_query_pipeline_defaults(desc);
}

SGX_API_DECL sg_pass_desc APIENTRY
sgx_query_pass_defaults(const sg_pass_desc *desc)
{
    return sg_query_pass_defaults(desc);
}

SGX_API_DECL sg_buffer APIENTRY
sgx_alloc_buffer(void)
{
    return sg_alloc_buffer();
}

SGX_API_DECL sg_image APIENTRY
sgx_alloc_image(void)
{
    return sg_alloc_image();
}

SGX_API_DECL sg_shader APIENTRY
sgx_alloc_shader(void)
{
    return sg_alloc_shader();
}

SGX_API_DECL sg_pipeline APIENTRY
sgx_alloc_pipeline(void)
{
    return sg_alloc_pipeline();
}

SGX_API_DECL sg_pass APIENTRY
sgx_alloc_pass(void)
{
    return sg_alloc_pass();
}

SGX_API_DECL void APIENTRY
sgx_init_buffer(sg_buffer buf_id, const sg_buffer_desc *desc)
{
    sg_init_buffer(buf_id, desc);
}

SGX_API_DECL void APIENTRY
sgx_init_image(sg_image img_id, const sg_image_desc *desc)
{
    sg_init_image(img_id, desc);
}

SGX_API_DECL void APIENTRY
sgx_init_shader(sg_shader shd_id, const sg_shader_desc *desc)
{
    sg_init_shader(shd_id, desc);
}

SGX_API_DECL void APIENTRY
sgx_init_pipeline(sg_pipeline pip_id, const sg_pipeline_desc *desc)
{
    sg_init_pipeline(pip_id, desc);
}

SGX_API_DECL void APIENTRY
sgx_init_pass(sg_pass pass_id, const sg_pass_desc *desc)
{
    sg_init_pass(pass_id, desc);
}

SGX_API_DECL void APIENTRY
sgx_fail_buffer(sg_buffer buf_id)
{
    sg_fail_buffer(buf_id);
}

SGX_API_DECL void APIENTRY
sgx_fail_image(sg_image img_id)
{
    sg_fail_image(img_id);
}

SGX_API_DECL void APIENTRY
sgx_fail_shader(sg_shader shd_id)
{
    sg_fail_shader(shd_id);
}

SGX_API_DECL void APIENTRY
sgx_fail_pipeline(sg_pipeline pip_id)
{
    sg_fail_pipeline(pip_id);
}

SGX_API_DECL void APIENTRY
sgx_fail_pass(sg_pass pass_id)
{
    sg_fail_pass(pass_id);
}

SGX_API_DECL sg_context APIENTRY
sgx_setup_context(void)
{
    return sg_setup_context();
}

SGX_API_DECL void APIENTRY
sgx_activate_context(sg_context ctx_id)
{
    sg_activate_context(ctx_id);
}

SGX_API_DECL void APIENTRY
sgx_discard_context(sg_context ctx_id)
{
    sg_discard_context(ctx_id);
}

SGX_API_DECL void APIENTRY
sgx_dealloc_buffer(sg_buffer buf_id)
{
    sg_dealloc_buffer(buf_id);
}

SGX_API_DECL void APIENTRY
sgx_dealloc_image(sg_image img_id)
{
    sg_dealloc_image(img_id);
}

SGX_API_DECL void APIENTRY
sgx_dealloc_shader(sg_shader shd_id)
{
    sg_dealloc_shader(shd_id);
}

SGX_API_DECL void APIENTRY
sgx_dealloc_pipeline(sg_pipeline pip_id)
{
    sg_dealloc_pipeline(pip_id);
}

SGX_API_DECL void APIENTRY
sgx_dealloc_pass(sg_pass pass_id)
{
    sg_dealloc_pass(pass_id);
}

SGX_API_DECL bool APIENTRY
sgx_uninit_buffer(sg_buffer buf_id)
{
    return sg_uninit_buffer(buf_id);
}

SGX_API_DECL bool APIENTRY
sgx_uninit_image(sg_image img_id)
{
    return sg_uninit_image(img_id);
}

SGX_API_DECL bool APIENTRY
sgx_uninit_shader(sg_shader shd_id)
{
    return sg_uninit_shader(shd_id);
}

SGX_API_DECL bool APIENTRY
sgx_uninit_pipeline(sg_pipeline pip_id)
{
    return sg_uninit_pipeline(pip_id);
}

SGX_API_DECL bool APIENTRY
sgx_uninit_pass(sg_pass pass_id)
{
    return sg_uninit_pass(pass_id);
}
