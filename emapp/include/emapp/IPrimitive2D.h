/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_IPRIMITIVE2D_H_
#define NANOEM_EMAPP_IPRIMITIVE2D_H_

#include "emapp/Forward.h"

namespace nanoem {

class IPrimitive2D {
public:
    virtual ~IPrimitive2D() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual void strokeLine(const Vector2 &from, const Vector2 &to, const Vector4 &color, nanoem_f32_t thickness) = 0;
    virtual void strokeRect(
        const Vector4 &rect, const Vector4 &color, nanoem_f32_t roundness, nanoem_f32_t thickness) = 0;
    virtual void fillRect(const Vector4 &rect, const Vector4 &color, nanoem_f32_t roundness) = 0;
    virtual void strokeCircle(const Vector4 &rect, const Vector4 &color, nanoem_f32_t thickness) = 0;
    virtual void fillCircle(const Vector4 &rect, const Vector4 &color) = 0;
    virtual void strokeCurve(const Vector2 &a, const Vector2 &c0, const Vector2 &c1, const Vector2 &b,
        const Vector4 &color, nanoem_f32_t thickness) = 0;
    virtual void drawTooltip(const char *text, size_t length) = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_IPRIMITIVE2D_H_ */
