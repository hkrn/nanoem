/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import "MetalBackgroundRenderer.h"
#import <Metal/Metal.h>

#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/internal/BlitPass.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace macos {

const char *const MetalBackgroundRenderer::kLabelPrefix = "@nanoem/MetalBackgroundVideoRenderer";

MetalBackgroundRenderer::MetalBackgroundRenderer(id<MTLDevice> device)
    : m_device(device)
    , m_lastRect(0)
{
}

MetalBackgroundRenderer::~MetalBackgroundRenderer() noexcept
{
    IOSurfaceDecrementUseCount(m_previousSurface);
    m_previousSurface = nullptr;
    sg::destroy_image(m_imageHandle);
    m_imageHandle = { SG_INVALID_ID };
    m_device = nil;
}

void
MetalBackgroundRenderer::draw(sg_pass pass, const Vector4 &rect, Project *project, CVPixelBufferRef pixelBuffer)
{
    SG_PUSH_GROUP("macos::MetalBackgroundRenderer::draw");
    sg_image_desc desc;
    Inline::clearZeroMemory(desc);
    if (id<MTLTexture> texture = createTexture(pixelBuffer, desc)) {
        desc.mtl_textures[0] = (__bridge const void *) texture;
        char label[Inline::kMarkerStringLength];
        StringUtils::format(label, sizeof(label), "%s/ColorImage", kLabelPrefix);
        desc.label = label;
        sg::destroy_image(m_imageHandle);
        m_imageHandle = sg::make_image(&desc);
        SG_LABEL_IMAGE(m_imageHandle, label);
    }
    internal::BlitPass *blitter = project->sharedImageBlitter();
    if (m_lastRect != rect) {
        blitter->markAsDirty();
        m_lastRect = rect;
    }
    const sg::NamedPass namedPass(tinystl::make_pair(pass, Project::kViewportPrimaryName));
    const sg::NamedImage namedImage(tinystl::make_pair(m_imageHandle, kLabelPrefix));
    const PixelFormat format(project->findRenderPassPixelFormat(pass, project->sampleCount()));
    blitter->blit(project->sharedBatchDrawQueue(), namedPass, namedImage, rect, format);
    SG_POP_GROUP();
}

id<MTLTexture>
MetalBackgroundRenderer::createTexture(CVPixelBufferRef pixelBuffer, sg_image_desc &desc)
{
    IOSurfaceRef currentSurface = CVPixelBufferGetIOSurface(pixelBuffer);
    id<MTLTexture> texture = nil;
    if (currentSurface && currentSurface != m_previousSurface) {
        IOSurfaceDecrementUseCount(m_previousSurface);
        m_previousSurface = currentSurface;
        IOSurfaceIncrementUseCount(currentSurface);
        MTLTextureDescriptor *td = [[MTLTextureDescriptor alloc] init];
        switch (CVPixelBufferGetPixelFormatType(pixelBuffer)) {
        case kCVPixelFormatType_128RGBAFloat:
            desc.pixel_format = SG_PIXELFORMAT_RGBA32F;
            td.pixelFormat = MTLPixelFormatRGBA32Float;
            break;
        case kCVPixelFormatType_64ARGB:
            desc.pixel_format = SG_PIXELFORMAT_RGBA16F;
            td.pixelFormat = MTLPixelFormatRGBA16Float;
            break;
        case kCVPixelFormatType_30RGB:
            desc.pixel_format = SG_PIXELFORMAT_RGB10A2;
            td.pixelFormat = MTLPixelFormatRGB10A2Unorm;
            break;
        case kCVPixelFormatType_32ARGB:
        case kCVPixelFormatType_32BGRA:
            desc.pixel_format = SG_PIXELFORMAT_RGBA8;
            td.pixelFormat = MTLPixelFormatBGRA8Unorm;
            break;
        default:
            /* unknown pixel format */
            return nil;
        }
        desc.usage = SG_USAGE_IMMUTABLE;
        desc.width = td.width = IOSurfaceGetWidth(currentSurface);
        desc.height = td.height = IOSurfaceGetHeight(currentSurface);
        desc.mag_filter = desc.min_filter = SG_FILTER_LINEAR;
        desc.wrap_u = desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
        td.textureType = MTLTextureType2D;
        /* IOSurface doesn't allow MSAA (sampleCount > 1) */
        td.sampleCount = td.mipmapLevelCount = td.depth = td.arrayLength = 1;
        if (desc.render_target) {
            td.usage = MTLTextureUsageRenderTarget;
        }
        texture = [m_device newTextureWithDescriptor:td iosurface:currentSurface plane:0];
    }
    return texture;
}

} /* namespace macos */
} /* namespace nanoem */
