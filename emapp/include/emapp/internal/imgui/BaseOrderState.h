/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_BASEORDERSTATE_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_BASEORDERSTATE_H_

#include "emapp/Forward.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct BaseOrderState {
    template <typename T>
    static void
    setDrawableOrderAt(tinystl::vector<T *, TinySTLAllocator> &drawables, T *drawable, int index)
    {
        for (typename tinystl::vector<T *, TinySTLAllocator>::iterator it = drawables.begin(), end = drawables.end();
             it != end; ++it) {
            if (*it == drawable && it + index >= drawables.begin() && it + index < drawables.end()) {
                drawables.erase(it);
                drawables.insert(it + index, drawable);
                break;
            }
        }
    }
    virtual void setOrderAt(int index) = 0;
    virtual bool isOrderBegin() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool isOrderEnd() const NANOEM_DECL_NOEXCEPT = 0;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_BASEORDERSTATE_H_ */
