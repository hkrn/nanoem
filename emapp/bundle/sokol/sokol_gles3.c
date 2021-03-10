#if defined(NANOEM_ENABLE_SOKOL_GLES3_COMPAT)
#include "GL/gl3w.h"
#else
#include <GLES3/gl3.h>
#define gl3wInit()
#define glGetTexImage(a, b, c, d, e)
#endif

#if defined(SOKOL_DEBUG) && SOKOL_DEBUG
#include <stdio.h>
#if defined(_WIN32) && _WIN32
#define SOKOL_ASSERT(c) if (!(c)) { char _buffer[1024]; snprintf(_buffer, sizeof(_buffer), "[%s:%d] ASSERTION (%s != 0)\n", __FILE__, __LINE__, #c); OutputDebugStringA(_buffer); abort(); }
#define SOKOL_LOG(m) do { char _buffer[1024]; snprintf(_buffer, sizeof(_buffer), "[%s:%d] %s\n", __FILE__, __LINE__, m); OutputDebugStringA(_buffer); } while (0)
#else
#define SOKOL_ASSERT(c) if (!(c)) { fprintf(stderr, "[%s:%d] ASSERTION (%s != 0)\n", __FILE__, __LINE__, #c); abort(); }
#define SOKOL_LOG(m) do { fprintf(stdout, "[%s:%d] %s\n", __FILE__, __LINE__, m); } while (0)
#endif /* _WIN32 */
#else /* SOKOL_DEBUG */
#define SOKOL_ASSERT(c) (void) 0
#define SOKOL_LOG(m) (void) 0
#endif /* SOKOL_DEBUG */
#define SOKOL_UNUSED(a) (void)(a)

#define SOKOL_IMPL
#define SOKOL_GLES3
#include "./sokol_delegate.c"

/* GL3W */
#if defined(NANOEM_ENABLE_SOKOL_GLES3_COMPAT)
#include "gl3w.c"
#endif

SGX_API_DECL void APIENTRY
sgx_setup(const sg_desc * desc)
{
    gl3wInit();
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
    SOKOL_UNUSED(buffer);
    SOKOL_UNUSED(text);
}

SGX_API_DECL void APIENTRY
sgx_label_image(sg_image image, const char *text)
{
    SOKOL_UNUSED(image);
    SOKOL_UNUSED(text);
}

SGX_API_DECL void APIENTRY
sgx_label_shader(sg_shader shader, const char *text)
{
    SOKOL_UNUSED(shader);
    SOKOL_UNUSED(text);
}

SGX_API_DECL void APIENTRY
sgx_label_pass(sg_pass pass, const char *text)
{
    SOKOL_UNUSED(pass);
    SOKOL_UNUSED(text);
}

SGX_API_DECL void APIENTRY
sgx_label_pipeline(sg_pipeline pipeline, const char *text)
{
    SOKOL_UNUSED(pipeline);
    SOKOL_UNUSED(text);
}

SGX_API_DECL void APIENTRY
sgx_push_group(const char *text)
{
    SOKOL_UNUSED(text);
}

SGX_API_DECL void APIENTRY
sgx_pop_group(void)
{
}

SGX_API_DECL void APIENTRY
sgx_read_image(sg_image image, void *data, size_t size)
{
    const _sg_image_t *ptr = _sg_lookup_image(&_sg.pools, image.id);
    if (ptr) {
        const GLenum target = ptr->gl.target,
                format = _sg_gl_teximage_format(ptr->cmn.pixel_format),
                type = _sg_gl_teximage_type(ptr->cmn.pixel_format);
        glBindTexture(target, ptr->gl.tex[ptr->cmn.active_slot]);
        {
            SOKOL_UNUSED(size);
            glGetTexImage(target, 0, format, type, data);
        }
        glBindTexture(target, 0);
    }
}

SGX_API_DECL void APIENTRY
sgx_read_pass(sg_pass pass, void *data, size_t size)
{
    SOKOL_UNUSED(size);
    _sg_pass_t *ptr = _sg_lookup_pass(&_sg.pools, pass.id);
    if (ptr) {
        const _sg_pass_attachment_common_t *attachment = &ptr->cmn.color_atts[0];
        const _sg_image_t *image = _sg_lookup_image(&_sg.pools, attachment->image_id.id);
        const GLuint msaa = image->gl.msaa_render_buffer;
        const GLenum format = _sg_gl_teximage_format(image->cmn.pixel_format),
                type = _sg_gl_teximage_type(image->cmn.pixel_format);
        glBindFramebuffer(GL_FRAMEBUFFER, msaa != 0 ? msaa : ptr->gl.fb);
        glReadPixels(0, 0, image->cmn.width, image->cmn.height, format, type, data);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

SGX_API_DECL void * APIENTRY
sgx_map_buffer(sg_buffer buffer)
{
    SOKOL_UNUSED(buffer);
    return NULL;
}

SGX_API_DECL void APIENTRY
sgx_unmap_buffer(sg_buffer buffer, void *address)
{
    SOKOL_UNUSED(buffer);
    SOKOL_UNUSED(address);
}

SGX_API_DECL void APIENTRY
sgx_insert_marker(const char *text)
{
    SOKOL_UNUSED(text);
}
