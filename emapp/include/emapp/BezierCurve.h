/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_BEZIERCURVE_H_
#define NANOEM_EMAPP_BEZIERCURVE_H_

#include "emapp/Forward.h"

namespace nanoem {

class BezierCurve NANOEM_DECL_SEALED : private NonCopyable {
public:
    typedef tinystl::unordered_map<nanoem_u64_t, BezierCurve *, TinySTLAllocator> Map;
    typedef tinystl::pair<BezierCurve *, BezierCurve *> Pair;

    BezierCurve(const Vector2U8 &c0, const Vector2U8 &c1, nanoem_frame_index_t interval);
    ~BezierCurve() NANOEM_DECL_NOEXCEPT;

    nanoem_f32_t value(nanoem_f32_t value) const NANOEM_DECL_NOEXCEPT;
    nanoem_frame_index_t length() const NANOEM_DECL_NOEXCEPT;
    Pair split(const nanoem_f32_t t) const;
    Vector4U8 toParameters() const NANOEM_DECL_NOEXCEPT;
    Vector2U8 c0() const NANOEM_DECL_NOEXCEPT;
    Vector2U8 c1() const NANOEM_DECL_NOEXCEPT;

    static nanoem_u64_t toHash(const nanoem_u8_t *parameters, nanoem_frame_index_t interval) NANOEM_DECL_NOEXCEPT;

private:
    typedef tinystl::vector<Vector2, nanoem::TinySTLAllocator> PointList;
    struct Hash {
        union {
            nanoem_u64_t c0x : 7;
            nanoem_u64_t c0y : 7;
            nanoem_u64_t c1x : 7;
            nanoem_u64_t c1y : 7;
            nanoem_u64_t interval : 36;
        };
        nanoem_u64_t value;
    };
    static void splitBezierCurve(const PointList &points, nanoem_f32_t t, PointList &left, PointList &right);
    static const Vector2 kP0;
    static const Vector2 kP1;
    PointList m_parameters;
    Vector2U8 m_c0;
    Vector2U8 m_c1;
    nanoem_frame_index_t m_interval;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_BEZIERCURVE_H_ */
