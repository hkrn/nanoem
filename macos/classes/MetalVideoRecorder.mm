/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import "MetalVideoRecorder.h"
#import <AVFoundation/AVFoundation.h>
#import <Metal/Metal.h>

#include "emapp/Project.h"
#include "emapp/internal/BlitPass.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace macos {

MetalVideoRecorder::MetalVideoRecorder(Project *projectPtr, id<MTLDevice> device)
    : BaseVideoRecorder(projectPtr)
    , m_device(device)
{
    enableMetalCompatibility();
}

MetalVideoRecorder::~MetalVideoRecorder() noexcept
{
    m_texture = nil;
}

bool
MetalVideoRecorder::capture(nanoem_frame_index_t frameIndex)
{
    CVPixelBufferRef pixelBuffer = nullptr;
    IOSurfaceRef surface = nullptr;
    acquirePixelBuffer(&pixelBuffer, &surface);
    bool appended = false;
    if (pixelBuffer && surface) {
        SG_PUSH_GROUP("macos::MetalVideoRecorder::capture()");
        sg_image_desc ide;
        m_texture = createTexture(surface, ide);
        sg::destroy_image(m_colorImage);
        m_colorImage = sg::make_image(&ide);
        updateDepthImage(surface);
        sg_pass_desc pd;
        Inline::clearZeroMemory(pd);
        pd.color_attachments[0].image = m_colorImage;
        pd.depth_stencil_attachment.image = m_depthImage;
        sg::destroy_pass(m_videoFramePass);
        m_videoFramePass = sg::make_pass(&pd);
        project()->setRenderPassName(m_videoFramePass, "@nanoem/MetalVideoRecorder/CapturePass");
        sg::PassBlock::IDrawQueue *drawQueue = project()->sharedBatchDrawQueue();
        blitPass(drawQueue, m_videoFramePass);
        sg_pass_action pa = {};
        struct CallbackArgument {
            MetalVideoRecorder *m_recorder;
            nanoem_frame_index_t m_frameIndex;
            CVPixelBufferRef m_pixelBuffer;
        };
        CallbackArgument *argument = nanoem_new(CallbackArgument);
        argument->m_recorder = this;
        argument->m_frameIndex = frameIndex;
        argument->m_pixelBuffer = pixelBuffer;
        sg::PassBlock pb(drawQueue, m_videoFramePass, pa);
        pb.registerCallback(
            [](sg_pass, void *opaque) {
                auto args = static_cast<CallbackArgument *>(opaque);
                MetalVideoRecorder *self = args->m_recorder;
                self->appendAudioSampleBuffer(args->m_frameIndex);
                self->appendPixelBuffer(args->m_frameIndex, args->m_pixelBuffer);
                nanoem_delete(args);
            },
            argument);
        appended = true;
        SG_POP_GROUP();
    }
    return appended;
}

MTLPixelFormat
MetalVideoRecorder::metalPixelFormat(OSType format) noexcept
{
    switch (format) {
    case kCVPixelFormatType_32ARGB:
        return MTLPixelFormatBGRA8Unorm;
    case kCVPixelFormatType_64RGBAHalf:
        return MTLPixelFormatRGBA16Float;
    case kCVPixelFormatType_128RGBAFloat:
        return MTLPixelFormatRGBA32Float;
    default:
        return MTLPixelFormatBGRA8Unorm;
    }
}

id<MTLTexture>
MetalVideoRecorder::createTexture(IOSurfaceRef surface, sg_image_desc &ide)
{
    Inline::clearZeroMemory(ide);
    ide.pixel_format = pixelFormat();
    ide.render_target = true;
    MTLTextureDescriptor *td = [[MTLTextureDescriptor alloc] init];
    if (ide.pixel_format == _SG_PIXELFORMAT_DEFAULT) {
        ide.pixel_format = SG_PIXELFORMAT_RGBA8;
    }
    ide.usage = SG_USAGE_IMMUTABLE;
    ide.width = td.width = IOSurfaceGetWidth(surface);
    ide.height = td.height = IOSurfaceGetHeight(surface);
    ide.mag_filter = ide.min_filter = SG_FILTER_LINEAR;
    ide.wrap_u = ide.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    td.textureType = MTLTextureType2D;
    td.pixelFormat = metalPixelFormat(internalPixelFormat());
    /* IOSurface doesn't allow MSAA (sampleCount > 1) */
    td.sampleCount = td.mipmapLevelCount = td.depth = td.arrayLength = 1;
    if (ide.render_target) {
        td.usage = MTLTextureUsageRenderTarget;
    }
    id<MTLTexture> texture = [m_device newTextureWithDescriptor:td iosurface:surface plane:0];
    ide.mtl_textures[0] = (__bridge const void *) texture;
    return texture;
}

void
MetalVideoRecorder::updateDepthImage(IOSurfaceRef surface)
{
    int width = IOSurfaceGetWidth(surface), height = IOSurfaceGetHeight(surface);
    if (width > 0 && height > 0 && (m_description.width != width || m_description.height != height)) {
        SG_PUSH_GROUP("macos::MetalVideoRecorder::updateDepthImage()");
        sg::destroy_image(m_depthImage);
        m_description.width = width;
        m_description.height = height;
        sg_image_desc ide(m_description);
        ide.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
        m_depthImage = sg::make_image(&ide);
        updateAllMSAAImages(width, height);
        SG_POP_GROUP();
    }
}

} /* namespace macos */
} /* namespace nanoem */
