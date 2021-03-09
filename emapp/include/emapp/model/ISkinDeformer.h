/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODEL_ISKINDEFORMER_H_
#define NANOEM_EMAPP_MODEL_ISKINDEFORMER_H_

#include "emapp/Forward.h"

namespace nanoem {
namespace model {

class ISkinDeformer {
public:
    virtual ~ISkinDeformer()
    {
    }

    virtual sg_buffer create(const sg_buffer_desc &desc, int bufferIndex) = 0;
    virtual void rebuildAllBones() = 0;
    virtual void destroy(sg_buffer value, int bufferIndex) NANOEM_DECL_NOEXCEPT = 0;
    virtual void execute(int bufferIndex) = 0;
};

} /* namespace model */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODEL_ISKINDEFORMER_H_ */
