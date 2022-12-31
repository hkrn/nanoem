/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_EMAPP_INTERNAL_IDRAGGINGSTATE
#define NANOEM_EMAPP_INTERNAL_IDRAGGINGSTATE

#include "emapp/Forward.h"

namespace nanoem {
namespace internal {

class IDraggingState {
public:
    virtual ~IDraggingState()
    {
    }

    virtual void transform(const Vector2SI32 &logicalCursorPosition) = 0;
    virtual void commit(const Vector2SI32 &logicalCursorPosition) = 0;

    virtual const char *name() const NANOEM_DECL_NOEXCEPT = 0;
    virtual nanoem_f32_t scaleFactor() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setScaleFactor(nanoem_f32_t value) = 0;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IDRAGGINGSTATE */
