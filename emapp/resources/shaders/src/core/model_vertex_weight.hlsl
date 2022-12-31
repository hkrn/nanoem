/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "nanoem/io.hlsl"
#include "nanoem/model.hlsl"

vs_output_t
nanoemVSMain(vs_input_t input)
{
    float4 position = float4(input.position, 1);
    vs_output_t output;
    output.position = mul(u_modelViewProjectionMatrix, position);
    output.color0 = input.color0;
    return output;
}

float4
nanoemPSMain(ps_input_t input) : SV_TARGET0
{
    return saturate(input.color0);
}
