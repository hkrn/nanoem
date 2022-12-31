/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/effect/GlobalUniform.h"

#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace effect {

GlobalUniform::Buffer::Buffer(sg_shader_stage stage)
    : m_stage(stage)
{
    m_float4.resize(128);
}

GlobalUniform::Buffer::~Buffer() NANOEM_DECL_NOEXCEPT
{
}

void
GlobalUniform::Buffer::apply(sg::PassBlock &pb) const
{
    pb.applyUniformBlock(m_stage, data(), size());
}

void
GlobalUniform::Buffer::reset()
{
    Inline::clearZeroMemory(m_float4.data(), size());
}

const void *
GlobalUniform::Buffer::data() const NANOEM_DECL_NOEXCEPT
{
    return m_float4.data();
}

size_t
GlobalUniform::Buffer::size() const NANOEM_DECL_NOEXCEPT
{
    return elementCount() * sizeof(m_float4[0]);
}

size_t
GlobalUniform::Buffer::elementCount() const NANOEM_DECL_NOEXCEPT
{
    return m_float4.size();
}

GlobalUniform::GlobalUniform()
    : m_vertexShaderBuffer(SG_SHADERSTAGE_VS)
    , m_pixelShaderBuffer(SG_SHADERSTAGE_FS)
    , m_preshaderVertexShaderBuffer(SG_SHADERSTAGE_VS)
    , m_preshaderPixelShaderBuffer(SG_SHADERSTAGE_FS)
{
}

GlobalUniform::~GlobalUniform() NANOEM_DECL_NOEXCEPT
{
}

} /* namespace effect */
} /* namespace nanoem */
