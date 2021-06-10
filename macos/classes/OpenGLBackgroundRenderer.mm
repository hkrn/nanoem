/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import "OpenGLBackgroundRenderer.h"

#import <AppKit/AppKit.h>

#include "emapp/Project.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace macos {

OpenGLBackgroundRenderer::OpenGLBackgroundRenderer(NSOpenGLContext *context, void *func)
    : _sgx_get_native_pass_handle(reinterpret_cast<PFN_sgx_get_native_pass_handle>(func))
    , m_context(context)
{
    glGenBuffers(1, &m_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), 0, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenTextures(1, &m_texture);
    GLuint program = m_program = glCreateProgram();
    auto compileShader = [program](GLenum type, const char *source) -> GLuint {
        GLuint vs = glCreateShader(type);
        glShaderSource(vs, 1, &source, 0);
        glCompileShader(vs);
        GLint status;
        glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
        if (status == 0) {
            GLint length;
            glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &length);
            if (length > 0) {
                char buffer[1024];
                glGetShaderInfoLog(vs, sizeof(buffer), &length, buffer);
                NSLog(@"%s", buffer);
            }
        }
        else {
            glAttachShader(program, vs);
        }
        return vs;
    };
    GLuint vs = compileShader(GL_VERTEX_SHADER, R"(
#version 330
layout(location=0) in vec2 a_position;
layout(location=2) in vec2 a_texcoord;
out vec2 v_texcoord;
void main()
{
  gl_Position = vec4(a_position, 0, 1);
  v_texcoord = a_texcoord;
}
                                                )");
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, R"(
#version 330
in vec2 v_texcoord;
layout(location = 0) out vec4 o_color;
uniform sampler2DRect u_sampler;
void main()
{
  o_color = texture(u_sampler, v_texcoord * textureSize(u_sampler));
}
                                                  )");
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == 0) {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        if (length > 0) {
            char buffer[1024];
            glGetProgramInfoLog(program, sizeof(buffer), &length, buffer);
            NSLog(@"%s", buffer);
        }
    }
    else {
        m_sampler = glGetUniformLocation(program, "u_sampler");
    }
}

OpenGLBackgroundRenderer::~OpenGLBackgroundRenderer() noexcept
{
    IOSurfaceDecrementUseCount(m_previousSurface);
    m_previousSurface = nullptr;
    glDeleteBuffers(1, &m_buffer);
    m_buffer = 0;
    glDeleteTextures(1, &m_texture);
    m_texture = 0;
    glDeleteProgram(m_program);
    m_program = 0;
}

void
OpenGLBackgroundRenderer::draw(sg_pass pass, const Vector4 &rect, Project *project, CVPixelBufferRef pixelBuffer)
{
    IOSurfaceRef currentSurface = CVPixelBufferGetIOSurface(pixelBuffer);
    if (currentSurface) {
        if (currentSurface != m_previousSurface) {
            IOSurfaceDecrementUseCount(m_previousSurface);
            m_previousSurface = currentSurface;
            IOSurfaceIncrementUseCount(currentSurface);
        }
        m_rect = rect;
        IOSurfaceIncrementUseCount(currentSurface);
        sg_pass_action pa = {};
        sg::PassBlock block(project->sharedBatchDrawQueue(), pass, pa);
        block.registerCallback(
            [](sg_pass pass, void *opaque) {
                SG_PUSH_GROUP("macos::OpenGLBackgroundRenderer::draw");
                auto self = static_cast<OpenGLBackgroundRenderer *>(opaque);
                self->internalDraw(pass);
                SG_POP_GROUP();
            },
            this);
    }
}

void
OpenGLBackgroundRenderer::internalDraw(sg_pass pass)
{
    IOSurfaceRef currentSurface = m_previousSurface;
    size_t width = IOSurfaceGetWidth(currentSurface), height = IOSurfaceGetHeight(currentSurface);
    glBindFramebuffer(GL_FRAMEBUFFER, _sgx_get_native_pass_handle(pass));
    glUseProgram(m_program);
    glActiveTexture(GL_TEXTURE0);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_RECTANGLE, m_texture);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUniform1i(m_sampler, 0);
    CGLTexImageIOSurface2D(m_context.CGLContextObj, GL_TEXTURE_RECTANGLE, GL_RGBA8, width, height, GL_BGRA,
        GL_UNSIGNED_INT_8_8_8_8_REV, currentSurface, 0);
    glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
    sg::QuadVertexUnit *vertices = m_vertices;
    const Vector4 rect(m_rect);
    float px = rect.x - 1, py = -rect.y + 1, pw = rect.z * 2.0f, ph = rect.w * -2.0f;
    vertices[0].m_position = Vector4(px, py, 0, 1);
    vertices[1].m_position = Vector4(pw + px, py, 0, 1);
    vertices[2].m_position = Vector4(px, ph + py, 0, 1);
    vertices[3].m_position = Vector4(pw + px, ph + py, 0, 1);
    vertices[0].m_texcoord = Vector4(0, 0, 0, 0);
    vertices[1].m_texcoord = Vector4(1, 0, 0, 0);
    vertices[2].m_texcoord = Vector4(0, 1, 0, 0);
    vertices[3].m_texcoord = Vector4(1, 1, 0, 0);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_vertices), m_vertices);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(*vertices),
        reinterpret_cast<const void *>(offsetof(sg::QuadVertexUnit, m_position)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(*vertices),
        reinterpret_cast<const void *>(offsetof(sg::QuadVertexUnit, m_texcoord)));
    glEnableVertexAttribArray(2);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);
    glUseProgram(0);
    glDisableVertexAttribArray(2);
    IOSurfaceDecrementUseCount(currentSurface);
}

} /* namespace macos */
} /* namespace nanoem */
