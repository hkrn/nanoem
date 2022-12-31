/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#pragma once
#ifndef NANOEM_EMAPP_EFFECT_GLOBALUNIFORM_H_
#define NANOEM_EMAPP_EFFECT_GLOBALUNIFORM_H_

#include "emapp/Forward.h"

namespace nanoem {
namespace effect {

struct GlobalUniform {
    typedef tinystl::vector<Vector4, TinySTLAllocator> Vector4List;
    static const int kMaxVertexShaderSamplerSize = 4;
    static const int kMaxVertexShaderUniformVectorsBool = 32;
    static const int kMaxVertexShaderUniformVectorsInt = 32;
    static const int kMaxVertexShaderUniformVectorsFloat = 256;
    static const int kMaxPixelShaderSamplerSize = 64;
    static const int kMaxPixelShaderUniformVectorsBool = 32;
    static const int kMaxPixelShaderUniformVectorsInt = 32;
    static const int kMaxPixelShaderUniformVectorsFloat = 192;

    struct Buffer {
        Buffer(sg_shader_stage stage);
        ~Buffer() NANOEM_DECL_NOEXCEPT;
        void apply(sg::PassBlock &pb) const;
        void reset();
        const void *data() const NANOEM_DECL_NOEXCEPT;
        size_t size() const NANOEM_DECL_NOEXCEPT;
        size_t elementCount() const NANOEM_DECL_NOEXCEPT;
        const sg_shader_stage m_stage;
        Vector4List m_float4;
    };

    GlobalUniform();
    ~GlobalUniform() NANOEM_DECL_NOEXCEPT;

    Buffer m_vertexShaderBuffer;
    Buffer m_pixelShaderBuffer;
    Buffer m_preshaderVertexShaderBuffer;
    Buffer m_preshaderPixelShaderBuffer;
};

} /* namespace effect */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_EFFECT_GLOBALUNIFORM_H_ */
