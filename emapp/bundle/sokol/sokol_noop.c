#define SOKOL_DUMMY_BACKEND
#define SOKOL_IMPL
#include "./sokol_delegate.c"

#include <stdlib.h>

SGX_API_DECL void APIENTRY
sgx_setup(const sg_desc *desc)
{
    sg_setup(desc);
}

SGX_API_DECL void APIENTRY
sgx_shutdown(void)
{
    sg_shutdown();
}

SGX_API_DECL void APIENTRY
sgx_label_buffer(sg_buffer buffer, const char *text)
{
    _SOKOL_UNUSED(buffer);
    _SOKOL_UNUSED(text);
}

SGX_API_DECL void APIENTRY
sgx_label_image(sg_image image, const char *text)
{
    _SOKOL_UNUSED(image);
    _SOKOL_UNUSED(text);
}

SGX_API_DECL void APIENTRY
sgx_label_shader(sg_shader shader, const char *text)
{
    _SOKOL_UNUSED(shader);
    _SOKOL_UNUSED(text);
}

SGX_API_DECL void APIENTRY
sgx_label_pass(sg_pass pass, const char *text)
{
    _SOKOL_UNUSED(pass);
    _SOKOL_UNUSED(text);
}

SGX_API_DECL void APIENTRY
sgx_label_pipeline(sg_pipeline pipeline, const char *text)
{
    _SOKOL_UNUSED(pipeline);
    _SOKOL_UNUSED(text);
}

SGX_API_DECL void APIENTRY
sgx_push_group(const char *text)
{
    sg_push_debug_group(text);
}

SGX_API_DECL void APIENTRY
sgx_pop_group(void)
{
    sg_pop_debug_group();
}

SGX_API_DECL void APIENTRY
sgx_read_image(sg_image image, sg_buffer buffer, void *data, size_t size)
{
    _SOKOL_UNUSED(image);
    _SOKOL_UNUSED(buffer);
    _SOKOL_UNUSED(data);
    _SOKOL_UNUSED(size);
}

SGX_API_DECL void APIENTRY
sgx_read_pass(sg_pass pass, sg_buffer buffer, void *data, size_t size)
{
    _SOKOL_UNUSED(pass);
    _SOKOL_UNUSED(buffer);
    _SOKOL_UNUSED(data);
    _SOKOL_UNUSED(size);
}

SGX_API_DECL void * APIENTRY
sgx_map_buffer(sg_buffer buffer)
{
    _SOKOL_UNUSED(buffer);
    return 0;
}

SGX_API_DECL void APIENTRY
sgx_unmap_buffer(sg_buffer buffer, void *address)
{
    _SOKOL_UNUSED(buffer);
    _SOKOL_UNUSED(address);
}

SGX_API_DECL void APIENTRY
sgx_insert_marker(const char *text)
{
    _SOKOL_UNUSED(text);
}
