/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_CONSTANTS_H_
#define NANOEM_EMAPP_CONSTANTS_H_

#include "emapp/Forward.h"

namespace nanoem {

class Constants NANOEM_DECL_SEALED : private NonCopyable {
public:
    static const char *const kGlobalSansFontFace;
    static const char *const kGlobalIconFontFace;
    static nanoem_f32_t kEpsilon;
    static const Quaternion kZeroQ;
    static const Matrix4x4 kIdentity;
    static const Vector4 kOrientateDirection;
    static const Vector4 kZeroV4;
    static const Vector3 kEpsilonVec3;
    static const Vector3 kTranslateDirection;
    static const Vector3 kUnitX;
    static const Vector3 kUnitY;
    static const Vector3 kUnitZ;
    static const Vector3 kZeroV3;
    static const nanoem_u64_t kStateCullCW;
    static const nanoem_u64_t kStateCullCCW;
    static const nanoem_u32_t kHalfBaseFPS;
    static const nanoem_f32_t kHalfBaseFPSFloat;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_CONSTANTS_H_ */
