/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_IDEBUGCAPTURE_H_
#define NANOEM_EMAPP_IDEBUGCAPTURE_H_

#include "emapp/Forward.h"

namespace nanoem {

class IDebugCapture {
public:
    virtual ~IDebugCapture() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual void start(const char *label) = 0;
    virtual void stop() = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_IDEBUGCAPTURE_H_ */
