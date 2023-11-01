/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "nanoem/io.hlsl"
#include "nanoem/model.hlsl"

vs_output_t
nanoemVSMain(vs_input_t input)
{
    float4 position = mul(vs.u_modelViewProjectionMatrix, float4(input.position, 1));
    vs_output_t output;
    output.position = position;
    output.texcoord0 = input.texcoord0.xy;
    output.color0 = vs.u_lightColor;
    return output;
}

float4
nanoemPSMain(ps_input_t input) : SV_TARGET0
{
    float4 color = input.color0;
    if (hasDiffuseTexture()) {
        float2 texcoord0 = input.texcoord0;
        float4 texel =  u_diffuseTexture.Sample(u_diffuseTextureSampler, texcoord0);
        color.a *= texel.a;
    }
    return color;
}
