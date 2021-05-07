/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_EMAPP_MACOS_METALBACKGROUNDDRAWER_H_
#define NANOEM_EMAPP_MACOS_METALBACKGROUNDDRAWER_H_

#include "ICocoaBackgroundRenderer.h"

#import <IOSurface/IOSurface.h>

@protocol MTLTexture;

namespace nanoem {

class Project;

namespace internal {
class BlitPass;
}

namespace macos {

class MetalBackgroundRenderer final : public ICocoaBackgroundRenderer, private NonCopyable {
public:
    MetalBackgroundRenderer(id<MTLDevice> device);
    ~MetalBackgroundRenderer() noexcept;

    void draw(sg_pass pass, const Vector4 &rect, Project *project, CVPixelBufferRef pixelBuffer) override;

private:
    static const char *const kLabelPrefix;
    id<MTLTexture> createTexture(CVPixelBufferRef pixelBuffer, sg_image_desc &desc);

    id<MTLDevice> m_device = nil;
    IOSurfaceRef m_previousSurface = nullptr;
    sg_image m_imageHandle = { SG_INVALID_ID };
    Vector4 m_lastRect;
};

} /* namespace macos */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MACOS_METALBACKGROUNDDRAWER_H_ */
