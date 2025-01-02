/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "nanoem/io.hlsl"

GLSLANG_ANNOTATION([[vk::binding(0 + WGSL_VS_UNIFORM_OFFSET, VK_DESCRIPTOR_SET_UNIFORM)]])
cbuffer grid_parameters_t : register(b0) {
    float4x4 u_modelViewProjectionMatrix;
};

GLSLANG_ANNOTATION([[vk::binding(1 + WGSL_TEXTURE_OFFSET, VK_DESCRIPTOR_SET_TEXTURE)]])
Texture2D u_texture : register(t0);

GLSLANG_ANNOTATION([[vk::binding(1 + WGSL_SAMPLER_OFFSET, VK_DESCRIPTOR_SET_SAMPLER)]])
SamplerState u_textureSampler : register(s0);

vs_output_t
nanoemVSMain(vs_input_t input)
{
    vs_output_t output;
    output.position = mul(u_modelViewProjectionMatrix, float4(input.position, 1));
    output.texcoord0 = input.texcoord0;
    output.color0 = input.color0;
#ifdef NANOEM_IO_HAS_POINT
    output.psize = 1.0;
#endif
    return output;
}

float4
nanoemPSMain(ps_input_t input) : SV_TARGET0
{
    return input.color0 * u_texture.Sample(u_textureSampler, input.texcoord0);
}
