/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_EMAPP_MACOS_OPENGLBACKGROUNDDRAWER_H_
#define NANOEM_EMAPP_MACOS_OPENGLBACKGROUNDDRAWER_H_

#include "ICocoaBackgroundRenderer.h"

#import <OpenGL/gl3.h>

@class NSOpenGLContext;

namespace nanoem {

class Project;

namespace macos {

class OpenGLBackgroundRenderer final : public ICocoaBackgroundRenderer, private NonCopyable {
public:
    OpenGLBackgroundRenderer(NSOpenGLContext *context, void *func);
    ~OpenGLBackgroundRenderer() noexcept override;

    void draw(sg_pass pass, const Vector4 &rect, Project *project, CVPixelBufferRef pixelBuffer) override;

private:
    typedef intptr_t (*PFN_sgx_get_native_pass_handle)(sg_pass);
    void internalDraw(sg_pass pass);

    PFN_sgx_get_native_pass_handle _sgx_get_native_pass_handle = nullptr;
    sg::QuadVertexUnit m_vertices[4];
    NSOpenGLContext *m_context = nil;
    IOSurfaceRef m_previousSurface = nullptr;
    Vector4 m_rect;
    GLuint m_buffer = 0;
    GLuint m_program = 0;
    GLuint m_sampler = 0;
    GLuint m_texture = 0;
};

} /* namespace macos */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MACOS_OPENGLBACKGROUNDDRAWER_H_ */
