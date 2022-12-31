/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COLOR_H_
#define NANOEM_EMAPP_COLOR_H_

#include "emapp/Forward.h"

namespace nanoem {

class Color : private NonCopyable {
public:
    static Vector3
    jet(nanoem_f32_t value)
    {
        nanoem_f32_t v = glm::clamp(value, 0.0f, 1.0f) * 2.0f - 1.0f;
        Vector3 color;
        color.x = jetColorComponent(v - 0.5f);
        color.y = jetColorComponent(v);
        color.z = jetColorComponent(v + 0.5f);
        return color;
    }
    static Vector3
    hotToCold(nanoem_f32_t value)
    {
        nanoem_f32_t v = glm::clamp(value, 0.0f, 1.0f);
        Vector3 color;
        if (v < 0.25f) {
            color.x = 0.0f;
            color.y = 4.0f * v;
            color.z = 1.0f;
        }
        else if (v < 0.5f) {
            color.x = 0.0f;
            color.y = 1.0f;
            color.z = 1.0f + 4.0f * (0.25f - v);
        }
        else if (v < 0.75f) {
            color.x = 4.0f * (v - 0.5f);
            color.y = 1.0f;
            color.z = 0.0f;
        }
        else {
            color.x = 1.0f;
            color.y = 1.0f + 4.0f * (0.75f - v);
            color.z = 0.0f;
        }
        return color;
    }

private:
    static nanoem_f32_t
    jetInterpolate(nanoem_f32_t value, nanoem_f32_t y0, nanoem_f32_t x0, nanoem_f32_t y1, nanoem_f32_t x1)
    {
        return (value - x0) * (y1 - y0) / (x1 - x0) + y0;
    }
    static nanoem_f32_t
    jetColorComponent(nanoem_f32_t value)
    {
        if (value <= -0.75f) {
            return 0.0f;
        }
        else if (value <= -0.25f) {
            return jetInterpolate(value, 0.0f, -0.75f, 1.0f, -0.25f);
        }
        else if (value <= 0.25f) {
            return 1.0f;
        }
        else if (value <= 0.75f) {
            return jetInterpolate(value, 1.0f, 0.25f, 0.0f, 0.75f);
        }
        else {
            return 0.0f;
        }
    }
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COLOR_H_ */
