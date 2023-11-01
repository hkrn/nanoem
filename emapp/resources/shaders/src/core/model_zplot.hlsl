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
    output.position = output.shadow0 = position;
    output.normal = 1;
    output.texcoord0 = input.texcoord0.xy;
    output.texcoord1 = 0.5;
    return output;
}

float4
nanoemPSMain(ps_input_t input) : SV_TARGET0
{
    float3 rgb = (input.shadow0.z / input.shadow0.w) * 0.5 + 0.5;
    return coverageAlpha(input, float4(rgb, 1));
}
