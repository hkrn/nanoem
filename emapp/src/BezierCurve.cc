/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/BezierCurve.h"

#include "emapp/private/CommonInclude.h"

namespace nanoem {

const Vector2 BezierCurve::kP0 = Vector2(0);
const Vector2 BezierCurve::kP1 = Vector2(127);

BezierCurve::BezierCurve(const glm::u8vec2 &c0, const glm::u8vec2 &c1, nanoem_frame_index_t interval)
    : m_c0(c0)
    , m_c1(c1)
    , m_interval(interval)
{
    interval = glm::max(interval, nanoem_frame_index_t(16));
    m_parameters.resize(interval + 1);
    const Vector2 c0f(c0), c1f(c1);
    const nanoem_f32_t intervalFloat = nanoem_f32_t(interval);
    for (nanoem_frame_index_t i = 0; i <= interval; i++) {
        const nanoem_f32_t t = i / intervalFloat;
        const nanoem_f32_t it = 1.0f - t;
        const Vector2 &v = ((kP0 * glm::pow(it, 3.0f)) + (c0f * t * glm::pow(it, 2.0f) * 3.0f) +
                               (c1f * it * glm::pow(t, 2.0f) * 3.0f) + (kP1 * glm::pow(t, 3.0f))) /
            kP1;
        m_parameters[i] = v;
    }
}

BezierCurve::~BezierCurve() NANOEM_DECL_NOEXCEPT
{
}

nanoem_f32_t
BezierCurve::value(nanoem_f32_t value) const NANOEM_DECL_NOEXCEPT
{
    const nanoem_frame_index_t interval(length());
    Vector2 nearest(kP1);
    for (nanoem_frame_index_t i = 0; i < interval; i++) {
        const Vector2 v(m_parameters[i]);
        if (glm::abs(nearest.x - value) > glm::abs(v.x - value)) {
            nearest = v;
        }
    }
    return nearest.y;
}

nanoem_frame_index_t
BezierCurve::length() const NANOEM_DECL_NOEXCEPT
{
    return nanoem_frame_index_t(m_parameters.size());
}

BezierCurve::Pair
BezierCurve::split(const nanoem_f32_t t) const
{
    const nanoem_f32_t tv = glm::clamp(t, 0.0f, 1.0f);
    PointList points(4), left, right;
    points[0] = kP0;
    points[1] = m_c0;
    points[2] = m_c1;
    points[3] = kP1;
    splitBezierCurve(points, tv, left, right);
    BezierCurve *lvalue = nanoem_new(BezierCurve(left[1], left[2], nanoem_frame_index_t(m_interval * tv)));
    BezierCurve *rvalue = nanoem_new(BezierCurve(right[1], right[2], nanoem_frame_index_t(m_interval * (1.0f - tv))));
    return Pair(lvalue, rvalue);
}

glm::u8vec4
BezierCurve::toParameters() const NANOEM_DECL_NOEXCEPT
{
    return glm::u8vec4(m_c0, m_c1);
}

glm::u8vec2
BezierCurve::c0() const NANOEM_DECL_NOEXCEPT
{
    return m_c0;
}

glm::u8vec2
BezierCurve::c1() const NANOEM_DECL_NOEXCEPT
{
    return m_c1;
}

nanoem_u64_t
BezierCurve::toHash(const nanoem_u8_t *parameters, nanoem_frame_index_t interval) NANOEM_DECL_NOEXCEPT
{
    Hash hash;
    hash.value = 0;
    hash.c0x = parameters[0];
    hash.c0y = parameters[1];
    hash.c1x = parameters[2];
    hash.c1y = parameters[3];
    hash.interval = interval;
    return hash.value;
}

void
BezierCurve::splitBezierCurve(const PointList &points, nanoem_f32_t t, PointList &left, PointList &right)
{
    if (points.size() == 1) {
        const Vector2 point(points[0]);
        left.push_back(point);
        right.push_back(point);
    }
    else {
        const nanoem_rsize_t length = points.size() - 1;
        PointList newPoints(length);
        left.push_back(points[0]);
        right.push_back(points[length]);
        for (nanoem_rsize_t i = 0; i < length; i++) {
            newPoints[i] = (1.0f - t) * points[i] + t * points[i + 1];
        }
        splitBezierCurve(newPoints, t, left, right);
    }
}

} /* namespace nanoem */
