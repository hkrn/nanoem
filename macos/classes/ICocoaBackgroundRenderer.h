/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_EMAPP_MACOS_ICOCOABACKGROUNDDRAWER_H_
#define NANOEM_EMAPP_MACOS_ICOCOABACKGROUNDDRAWER_H_

#include "emapp/Forward.h"

#import <CoreVideo/CVPixelBuffer.h>

namespace nanoem {

class Project;

namespace macos {

class ICocoaBackgroundRenderer {
public:
    virtual ~ICocoaBackgroundRenderer() noexcept
    {
    }

    virtual void draw(sg_pass pass, const Vector4 &rect, Project *project, CVPixelBufferRef pixelBuffer) = 0;
};

} /* namespace macos */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MACOS_ICOCOABACKGROUNDDRAWER_H_ */
