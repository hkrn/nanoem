/*
   Copyright (c) 2015-2020 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "nanoem/io.hlsl"

GLSLANG_ANNOTATION([[vk::binding(0, VK_DESCRIPTOR_SET_TEXTURE)]])
Texture2D u_texture : register(t0);
GLSLANG_ANNOTATION([[vk::binding(1, VK_DESCRIPTOR_SET_SAMPLER)]])
SamplerState u_textureSampler : register(s0);

vs_output_t
nanoemVSMain(vs_input_t input)
{
    vs_output_t output;
    output.position = float4(input.position, 1);
    output.texcoord0 = input.texcoord0;
    return output;
}

float4
nanoemPSMain(ps_input_t input) : SV_TARGET0
{
    return u_texture.Sample(u_textureSampler, input.texcoord0);
}
