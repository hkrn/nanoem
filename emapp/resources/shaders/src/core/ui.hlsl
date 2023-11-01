/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "nanoem/io.hlsl"

struct ui_parameters_t {
    float4 u_viewportTexel;
    float4 u_scaleFactor;
    float4 u_mul;
    float4 u_add;
};

GLSLANG_ANNOTATION([[vk::binding(0 + WGSL_TEXTURE_OFFSET, VK_DESCRIPTOR_SET_TEXTURE)]])
Texture2D u_texture : register(t0);

GLSLANG_ANNOTATION([[vk::binding(0 + WGSL_SAMPLER_OFFSET, VK_DESCRIPTOR_SET_SAMPLER)]])
SamplerState u_textureSampler : register(s0);

GLSLANG_ANNOTATION([[vk::binding(0 +  WGSL_VS_UNIFORM_OFFSET, VK_DESCRIPTOR_SET_UNIFORM)]])
ConstantBuffer<ui_parameters_t> vs : register(b0);

GLSLANG_ANNOTATION([[vk::binding(1 + WGSL_FS_UNIFORM_OFFSET, VK_DESCRIPTOR_SET_UNIFORM)]])
ConstantBuffer<ui_parameters_t> fs : register(b1);

vs_output_t
nanoemVSMain(vs_input_t input)
{
    vs_output_t output;
    float2 position = ((vs.u_viewportTexel.xy + input.position.xy * vs.u_viewportTexel.zw) - 0.5) * vs.u_scaleFactor;
    output.position = float4(position, 0, 1);
    output.texcoord0 = input.texcoord0;
    output.color0 = input.color0;
    return output;
}

float4
nanoemPSMain(ps_input_t input) : SV_TARGET0
{
    return input.color0 * u_texture.Sample(u_textureSampler, input.texcoord0) * fs.u_mul + fs.u_add;
}
