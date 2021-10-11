#include "GL/gl3w.h"

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "./sokol_delegate.c"

/* GL3W */
#include "gl3w.c"

#if defined(SOKOL_DEBUG) && SOKOL_DEBUG
static void APIENTRY
sgx_debug_callback(GLenum source, GLenum type, GLuint eid, GLenum severity, GLsizei length, const GLchar *message, const void *user_param)
{
    _SOKOL_UNUSED(source);
    _SOKOL_UNUSED(type);
    _SOKOL_UNUSED(eid);
    _SOKOL_UNUSED(severity);
    _SOKOL_UNUSED(length);
    _SOKOL_UNUSED(user_param);
    SOKOL_LOG(message);
}
#endif /* SOKOL_DEBUG */

SGX_API_DECL void APIENTRY
sgx_setup(const sg_desc *desc)
{
    gl3wInit();
#if defined(SOKOL_DEBUG) && SOKOL_DEBUG
    if (glDebugMessageCallback) {
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_TRUE);
        glDebugMessageCallback(sgx_debug_callback, NULL);
        _SG_GL_CHECK_ERROR();
    }
#endif /* SOKOL_DEBUG */
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
    _sg_buffer_t *ptr = _sg_lookup_buffer(&_sg.pools, buffer.id);
    if (glObjectLabel && ptr && text) {
        GLuint id;
        if ((id = ptr->gl.buf[0]) != 0) {
            glObjectLabel(GL_BUFFER, id, -1, text);
            _SG_GL_CHECK_ERROR();
        }
    }
}

SGX_API_DECL void APIENTRY
sgx_label_image(sg_image image, const char *text)
{
    _sg_image_t *ptr = _sg_lookup_image(&_sg.pools, image.id);
    if (glObjectLabel && ptr && text) {
        GLuint id;
        if ((id = ptr->gl.msaa_render_buffer) != 0) {
            glObjectLabel(GL_RENDERBUFFER, id, -1, text);
            _SG_GL_CHECK_ERROR();
        }
        else if ((id = ptr->gl.depth_render_buffer) != 0) {
            glObjectLabel(GL_RENDERBUFFER, id, -1, text);
            _SG_GL_CHECK_ERROR();
        }
        else if ((id = ptr->gl.tex[ptr->cmn.active_slot]) != 0) {
            glObjectLabel(GL_TEXTURE, id, -1, text);
        }
    }
}

SGX_API_DECL void APIENTRY
sgx_label_shader(sg_shader shader, const char *text)
{
    _sg_shader_t *ptr = _sg_lookup_shader(&_sg.pools, shader.id);
    if (glObjectLabel && ptr && text) {
        GLuint id;
        if ((id = ptr->gl.prog) != 0) {
            glObjectLabel(GL_PROGRAM, id, -1, text);
            _SG_GL_CHECK_ERROR();
        }
    }
}

SGX_API_DECL void APIENTRY
sgx_label_pass(sg_pass pass, const char *text)
{
    _sg_pass_t *ptr = _sg_lookup_pass(&_sg.pools, pass.id);
    if (glObjectLabel && ptr && text) {
        GLuint id;
        if ((id = ptr->gl.fb) != 0) {
            glObjectLabel(GL_FRAMEBUFFER, id, -1, text);
            _SG_GL_CHECK_ERROR();
        }
    }
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
    if (glPushDebugGroup && text) {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, text);
        _SG_GL_CHECK_ERROR();
    }
    if (text) {
        sg_push_debug_group(text);
    }
}

SGX_API_DECL void APIENTRY
sgx_pop_group(void)
{
    if (glPopDebugGroup) {
        glPopDebugGroup();
        _SG_GL_CHECK_ERROR();
    }
    sg_pop_debug_group();
}

SGX_API_DECL void APIENTRY
sgx_read_image(sg_image image,  sg_buffer buffer, void *data, size_t size)
{
    _SOKOL_UNUSED(buffer);
    const _sg_image_t *ptr = _sg_lookup_image(&_sg.pools, image.id);
    if (ptr) {
        const GLenum target = ptr->gl.target,
                format = _sg_gl_teximage_format(ptr->cmn.pixel_format),
                type = _sg_gl_teximage_type(ptr->cmn.pixel_format);
        glBindTexture(target, ptr->gl.tex[ptr->cmn.active_slot]);
#if defined(SOKOL_DEBUG) && SOKOL_DEBUG
        if (glGetnTexImage) {
            glGetnTexImage(target, 0, format, type, size, data);
        }
        else
#endif /* SOKOL_DEBUG */
        {
            _SOKOL_UNUSED(size);
            glGetTexImage(target, 0, format, type, data);
        }
        glBindTexture(target, 0);
        _SG_GL_CHECK_ERROR();
    }
}

SGX_API_DECL void APIENTRY
sgx_read_pass(sg_pass pass, sg_buffer buffer, void *data, size_t size)
{
    _SOKOL_UNUSED(buffer);
    _sg_pass_t *ptr = _sg_lookup_pass(&_sg.pools, pass.id);
    if (ptr) {
        const _sg_pass_attachment_common_t *attachment = &ptr->cmn.color_atts[0];
        const _sg_image_t *image = _sg_lookup_image(&_sg.pools, attachment->image_id.id);
        const GLuint msaa = image->gl.msaa_render_buffer;
        const GLenum format = _sg_gl_teximage_format(image->cmn.pixel_format),
                type = _sg_gl_teximage_type(image->cmn.pixel_format);
        glBindFramebuffer(GL_FRAMEBUFFER, msaa != 0 ? msaa : ptr->gl.fb);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
#if defined(SOKOL_DEBUG) && SOKOL_DEBUG
        if (glReadnPixels) {
            glReadnPixels(0, 0, image->cmn.width, image->cmn.height, format, type, size, data);
        }
        else
#endif /* SOKOL_DEBUG */
        {
            _SOKOL_UNUSED(size);
            glReadPixels(0, 0, image->cmn.width, image->cmn.height, format, type, data);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        _SG_GL_CHECK_ERROR();
    }
}

SGX_API_DECL intptr_t APIENTRY
sgx_get_native_pass_handle(sg_pass pass)
{
    intptr_t handle = 0;
    _sg_pass_t *ptr = _sg_lookup_pass(&_sg.pools, pass.id);
    if (ptr) {
        handle = ptr->gl.fb;
    }
    return handle;
}

SGX_API_DECL void * APIENTRY
sgx_map_buffer(sg_buffer buffer)
{
    void *address = 0;
#if defined(SOKOL_DEBUG) && SOKOL_DEBUG
    _sg_buffer_t *ptr = _sg_lookup_buffer(&_sg.pools, buffer.id);
    if ((glMapBuffer || glMapNamedBuffer) && ptr) {
        GLuint id;
        if ((id = ptr->gl.buf[0]) != 0) {
            if (glMapNamedBuffer) {
                address = glMapNamedBuffer(id, GL_WRITE_ONLY);
            }
            else {
                GLenum type = _sg_gl_buffer_target(ptr->cmn.type);
                glBindBuffer(type, id);
                address = glMapBuffer(type, GL_WRITE_ONLY);
                glBindBuffer(type, 0);
            }
        }
        _SG_GL_CHECK_ERROR();
    }
#else
    _SOKOL_UNUSED(buffer);
#endif /* SOKOL_DEBUG */
    return address;
}

SGX_API_DECL void APIENTRY
sgx_unmap_buffer(sg_buffer buffer, void *address)
{
    _sg_buffer_t *ptr = _sg_lookup_buffer(&_sg.pools, buffer.id);
    _SOKOL_UNUSED(address);
#if defined(SOKOL_DEBUG) && SOKOL_DEBUG
    if (glUnmapBuffer && address) {
        GLuint id;
        if ((id = ptr->gl.buf[0]) != 0) {
            if (glUnmapNamedBuffer) {
                glUnmapNamedBuffer(id);
            }
            else {
                GLenum type = _sg_gl_buffer_target(ptr->cmn.type);
                glBindBuffer(type, id);
                glUnmapBuffer(type);
                glBindBuffer(type, 0);
            }
        }
        _SG_GL_CHECK_ERROR();
    }
#else
    _SOKOL_UNUSED(ptr);
#endif /* SOKOL_DEBUG */
}

SGX_API_DECL void APIENTRY
sgx_insert_marker(const char *text)
{
    if (glDebugMessageInsert) {
        glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1, text);
    }
}
