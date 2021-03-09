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
#define SOKOL_GLES2
#include <GLES2/gl2.h>
#include "./sokol_delegate.c"

SOKOL_DECL_API void
sgx_label_buffer(sg_buffer buffer, const char *text)
{
    SOKOL_UNUSED(buffer);
    SOKOL_UNUSED(text);
}

SOKOL_DECL_API void
sgx_label_image(sg_image image, const char *text)
{
    SOKOL_UNUSED(image);
    SOKOL_UNUSED(text);
}

SOKOL_DECL_API void
sgx_label_shader(sg_shader shader, const char *text)
{
    SOKOL_UNUSED(shader);
    SOKOL_UNUSED(text);
}

SOKOL_DECL_API void
sgx_label_pass(sg_pass pass, const char *text)
{
    SOKOL_UNUSED(pass);
    SOKOL_UNUSED(text);
}

SOKOL_DECL_API void
sgx_label_pipeline(sg_pipeline pipeline, const char *text)
{
    SOKOL_UNUSED(pipeline);
    SOKOL_UNUSED(text);
}

SOKOL_DECL_API void
sgx_push_group(const char *text)
{
    SOKOL_UNUSED(text);
}

SOKOL_DECL_API void
sgx_pop_group(void)
{
}

SOKOL_DECL_API void
sgx_read_image(sg_image image, void *data, size_t size)
{
    const _sg_image_t *ptr = _sg_lookup_image(&_sg.pools, image.id);
    if (ptr) {
        const GLenum target = ptr->gl_target,
                format = _sg_gl_teximage_format(ptr->pixel_format),
                type = _sg_gl_teximage_type(ptr->pixel_format);
        glBindTexture(target, ptr->gl_tex[ptr->active_slot]);
        {
            SOKOL_UNUSED(size);
            glGetTexImage(target, 0, format, type, data);
        }
        glBindTexture(target, 0);
    }
}

SOKOL_DECL_API void
sgx_read_pass(sg_pass pass, void *data, size_t size)
{
    SOKOL_UNUSED(size);
    _sg_pass_t *ptr = _sg_lookup_pass(&_sg.pools, pass.id);
    if (ptr) {
        const _sg_attachment_t *attachment = &ptr->color_atts[0];
        const _sg_image_t *image = attachment->image;
        const GLuint msaa = attachment->gl_msaa_resolve_buffer;
        const GLenum format = _sg_gl_teximage_format(image->pixel_format),
                type = _sg_gl_teximage_type(image->pixel_format);
        glBindFramebuffer(GL_FRAMEBUFFER, msaa != 0 ? msaa : ptr->gl_fb);
        glReadPixels(0, 0, image->width, image->height, format, type, data);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

SOKOL_DECL_API intptr_t
sgx_get_native_pass_handle(sg_pass pass)
{
    intptr_t handle = 0;
    _sg_pass_t *ptr = _sg_lookup_pass(&_sg.pools, pass.id);
    if (ptr) {
        handle = ptr->gl_fb;
    }
    return handle;
}

SOKOL_DECL_API void *
sgx_map_buffer(sg_buffer buffer)
{
    SOKOL_UNUSED(buffer);
    return NULL;
}

SOKOL_DECL_API void
sgx_unmap_buffer(sg_buffer buffer, void *address)
{
    SOKOL_UNUSED(buffer);
    SOKOL_UNUSED(address);
}

SOKOL_DECL_API void
sgx_insert_marker(const char *text)
{
    SOKOL_UNUSED(text);
}

SOKOL_DECL_API uint32_t
sgx_get_renderer_type(void)
{
    return 'O' << 0 | 'E' << 8 | 'S' << 16 | '2' << 24; /* OES2 */
}
