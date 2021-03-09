/*
   Copyright (c) 2015-2020 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "nanoem/io.hlsl"

cbuffer grid_parameters_t : register(b0) {
    float4x4 u_modelViewProjectionMatrix;
};

vs_output_t
nanoemVSMain(vs_input_t input)
{
    vs_output_t output;
    output.position = mul(u_modelViewProjectionMatrix, float4(input.position, 1));
    output.color0 = input.color0;
#ifdef NANOEM_IO_HAS_POINT
    output.psize = 1.0;
#endif
    return output;
}

float4
nanoemPSMain(ps_input_t input) : SV_TARGET0
{
    return input.color0;
}
