/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_BOUNDINGBOX_H_
#define NANOEM_EMAPP_BOUNDINGBOX_H_

#include "emapp/Forward.h"

namespace nanoem {

struct BoundingBox {
    explicit BoundingBox()
        : m_min(FLT_MAX)
        , m_max(-FLT_MAX)
    {
    }
    ~BoundingBox()
    {
    }

    inline void
    reset()
    {
        m_min = Vector3(FLT_MAX);
        m_max = Vector3(-FLT_MAX);
    }
    inline void
    set(const Vector3 &value)
    {
        m_min = glm::min(m_min, value);
        m_max = glm::max(m_max, value);
    }
    inline void
    set(const Vector3 &minValue, const Vector3 &maxValue)
    {
        m_min = glm::min(m_min, minValue);
        m_max = glm::max(m_max, maxValue);
    }
    inline void
    set(const BoundingBox &value)
    {
        m_min = glm::min(m_min, value.m_min);
        m_max = glm::max(m_max, value.m_max);
    }

    Vector3 m_min;
    Vector3 m_max;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_BOUNDINGBOX_H_ */
