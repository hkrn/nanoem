/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_IIMAGEVIEW_H_
#define NANOEM_EMAPP_IIMAGEVIEW_H_

#include "emapp/Forward.h"

namespace nanoem {

class IImageView {
public:
    virtual ~IImageView() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual sg_image imageHandle() const NANOEM_DECL_NOEXCEPT = 0;
    virtual sg_sampler samplerHandle() const NANOEM_DECL_NOEXCEPT = 0;
    virtual sg_image_desc imageDescription() const NANOEM_DECL_NOEXCEPT = 0;
    virtual sg_sampler_desc samplerDescription() const NANOEM_DECL_NOEXCEPT = 0;
    virtual const ByteArray *originData() const NANOEM_DECL_NOEXCEPT = 0;
    virtual const ByteArray *mipmapData(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT = 0;
    virtual const char *filenameConstString() const NANOEM_DECL_NOEXCEPT = 0;
    virtual String filename() const = 0;
    virtual bool isFileExist() const NANOEM_DECL_NOEXCEPT = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_IIMAGEVIEW_H_ */
