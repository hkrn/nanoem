/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/effect/Common.h"

#include "../protoc/effect.pb-c.h"

#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace effect {

typedef tinystl::unordered_map<uint32_t, sg_image_desc, TinySTLAllocator> SamplerIndexDescriptionMap;
typedef tinystl::unordered_map<uint32_t, uint32_t, TinySTLAllocator> SamplerIndexTextureIndexMap;

template <typename TPass, typename TState>
static void
convertPipeline(const TPass *passPtr, PipelineDescriptor &desc)
{
    SG_PUSH_GROUPF("effect::convertPipeline(size=%d)", passPtr->n_render_states);
    const size_t numRenderStates = passPtr->n_render_states;
    for (size_t i = 0; i < numRenderStates; i++) {
        const TState *state = passPtr->render_states[i];
        effect::RenderState::convertPipeline(state->key, state->value, desc);
    }
    SG_POP_GROUP();
}

template <typename TTexture, typename TSamplerState>
static void
convertSamplerDescription(const TTexture *texturePtr, sg_sampler_desc &samplerDescription)
{
    const size_t numSamplerStates = texturePtr->n_sampler_states;
    samplerDescription.mag_filter = samplerDescription.min_filter = SG_FILTER_LINEAR;
    for (size_t i = 0; i < numSamplerStates; i++) {
        const TSamplerState *state = texturePtr->sampler_states[i];
        effect::RenderState::convertSamplerState(state->key, state->value, samplerDescription);
    }
}

template <typename TSymbol>
static void
countRegisterSet(TSymbol *const *symbolsPtr, size_t numSymbols, GlobalUniform::Buffer &buffer)
{
    uint32_t numBoolRegisterCount = 0, numFloat4RegisterCount = 0, numInt4RegisterCount = 0;
    for (size_t i = 0; i < numSymbols; i++) {
        const TSymbol *symbolPtr = symbolsPtr[i];
        const uint32_t size = symbolPtr->register_index + symbolPtr->register_count;
        switch (static_cast<Fx9__Effect__Dx9ms__RegisterSet>(symbolPtr->register_set)) {
        case FX9__EFFECT__DX9MS__REGISTER_SET__RS_BOOL: {
            numBoolRegisterCount = glm::max(size, numBoolRegisterCount);
            break;
        }
        case FX9__EFFECT__DX9MS__REGISTER_SET__RS_FLOAT4: {
            numFloat4RegisterCount = glm::max(size, numFloat4RegisterCount);
            break;
        }
        case FX9__EFFECT__DX9MS__REGISTER_SET__RS_INT4: {
            numInt4RegisterCount = glm::max(size, numInt4RegisterCount);
            break;
        }
        default:
            break;
        }
    }
    if (numFloat4RegisterCount > buffer.m_float4.size()) {
        buffer.m_float4.resize(numFloat4RegisterCount);
    }
}

template <typename TSymbol, typename TType>
static inline RegisterIndex
createSymbolRegisterIndex(const TSymbol *symbolPtr, TType min, TType max)
{
    RegisterIndex index;
    index.m_index = symbolPtr->register_index;
    index.m_count = symbolPtr->register_count;
    index.m_type = glm::clamp(uint32_t(symbolPtr->register_set), uint32_t(min), uint32_t(max));
    return index;
}

} /* namespace effect */
} /* namespace nanoem */
