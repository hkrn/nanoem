/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_IBACKGROUNDVIDEORENDERER_H_
#define NANOEM_EMAPP_IBACKGROUNDVIDEORENDERER_H_

#include "emapp/Forward.h"

namespace nanoem {

class Error;
class Project;
class URI;

class IBackgroundVideoRenderer {
public:
    virtual ~IBackgroundVideoRenderer() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual bool load(const URI &fileURI, Error &error) = 0;
    virtual void draw(sg_pass pass, const Vector4 &rect, nanoem_f32_t scaleFactor, Project *project) = 0;
    virtual void seek(nanoem_f64_t seconds) = 0;
    virtual void flush() = 0;
    virtual void destroy() = 0;
    virtual URI fileURI() const NANOEM_DECL_NOEXCEPT = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_IBACKGROUNDVIDEORENDERER_H_ */
