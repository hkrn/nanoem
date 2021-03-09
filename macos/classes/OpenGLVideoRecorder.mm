/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import "OpenGLVideoRecorder.h"

#import <AppKit/AppKit.h>
#import <OpenGL/gl3.h>

#include "emapp/Project.h"
#include "emapp/internal/BlitPass.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace macos {

OpenGLVideoRecorder::OpenGLVideoRecorder(Project *projectPtr, NSOpenGLContext *context, void *func)
    : BaseVideoRecorder(projectPtr)
    , _sgx_get_native_pass_handle(reinterpret_cast<PFN_sgx_get_native_pass_handle>(func))
    , m_context(context)
{
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_RECTANGLE, m_texture);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);
    enableOpenGLCompatibility();
}

OpenGLVideoRecorder::~OpenGLVideoRecorder() noexcept
{
    if (m_texture) {
        glDeleteTextures(1, &m_texture);
        m_texture = 0;
    }
    m_context = nil;
}

bool
OpenGLVideoRecorder::capture(nanoem_frame_index_t frameIndex)
{
    CVPixelBufferRef pixelBuffer = nullptr;
    IOSurfaceRef surface = nullptr;
    acquirePixelBuffer(&pixelBuffer, &surface);
    bool appended = false;
    if (pixelBuffer && surface) {
        /* flush OpenGL errors to handle below process correctly  */
        while (glGetError()) {
        }
        SG_PUSH_GROUP("macos::OpenGLVideoRecorder::capture()");
        size_t width = CVPixelBufferGetWidth(pixelBuffer), height = CVPixelBufferGetHeight(pixelBuffer);
        glBindTexture(GL_TEXTURE_RECTANGLE, m_texture);
        CGLTexImageIOSurface2D(m_context.CGLContextObj, GL_TEXTURE_RECTANGLE, GL_RGBA8, width, height, GL_BGRA,
            GL_UNSIGNED_INT_8_8_8_8_REV, surface, 0);
        glBindTexture(GL_TEXTURE_RECTANGLE, 0);
        updateDepthImage(surface);
        sg_pass_desc pd;
        Inline::clearZeroMemory(pd);
        pd.color_attachments[0].image = m_colorImage;
        pd.depth_stencil_attachment.image = m_depthImage;
        sg::destroy_pass(m_videoFramePass);
        m_videoFramePass = sg::make_pass(&pd);
        project()->setRenderPassName(m_videoFramePass, "@nanoem/OpenGLVideoRecorder/CapturePass");
        GLuint fbo = static_cast<GLuint>(_sgx_get_native_pass_handle(m_videoFramePass));
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_texture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        sg::PassBlock::IDrawQueue *drawQueue = project()->sharedBatchDrawQueue();
        blitPass(drawQueue, m_videoFramePass);
        appended = appendAudioSampleBuffer(frameIndex) && appendPixelBuffer(frameIndex, pixelBuffer);
        SG_POP_GROUP();
    }
    return appended;
}

void
OpenGLVideoRecorder::updateDepthImage(IOSurfaceRef surface)
{
    int width = IOSurfaceGetWidth(surface), height = IOSurfaceGetHeight(surface),
        sampleCount = int(project()->sampleCount());
    if (m_description.width != width || m_description.height != height || m_description.sample_count != sampleCount) {
        SG_PUSH_GROUP("macos::OpenGLVideoRecorder::updateDepthImage()");
        sg::destroy_image(m_colorImage);
        sg::destroy_image(m_depthImage);
        m_description.width = width;
        m_description.height = height;
        m_colorImage = sg::make_image(&m_description);
        sg_image_desc ide(m_description);
        ide.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
        m_depthImage = sg::make_image(&ide);
        updateAllMSAAImages(width, height);
        SG_POP_GROUP();
    }
}

} /* namespace macos */
} /* namespace nanoem */
