/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODEL_IVERTEXWEIGHTBRUSH_H_
#define NANOEM_EMAPP_MODEL_IVERTEXWEIGHTBRUSH_H_

#include "emapp/Forward.h"

namespace nanoem {
namespace model {

class IVertexWeightBrush : private NonCopyable {
public:
    virtual ~IVertexWeightBrush() NANOEM_DECL_NOEXCEPT
    {
    }

    virtual void begin() = 0;
    virtual void end() = 0;

    virtual const nanoem_model_bone_t *activeBone() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setActiveBone(const nanoem_model_bone_t *value) = 0;
    virtual nanoem_f32_t radius() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setRadius(nanoem_f32_t value) = 0;
};

} /* namespace model */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODEL_IVERTEXWEIGHTBRUSH_H_ */
