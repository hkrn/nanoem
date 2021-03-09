/*
   Copyright (c) 2015-2020 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_SHADER_IO_HLSL_
#define NANOEM_SHADER_IO_HLSL_

#include "macros.hlsl"

struct vs_input_t {
    float3 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texcoord0 : TEXCOORD0;
    float4 uva1 : TEXCOORD1;
    float4 uva2 : TEXCOORD2;
    float4 uva3 : TEXCOORD3;
    float4 uva4 : TEXCOORD4;
    float4 color0 : COLOR0;
};

struct vs_output_t {
    float4 position : SV_POSITION;
    float4 color0 : COLOR0;
    float3 normal : TEXCOORD0;
    float2 texcoord0 : TEXCOORD1;
    float2 texcoord1 : TEXCOORD2;
    float3 eye : TEXCOORD3;
    float4 shadow0 : TEXCOORD4;
#ifdef NANOEM_IO_HAS_POINT
#ifdef GLSLANG
    [[vk::builtin("PointSize")]]
#endif /* GLSLANG */
    float psize : PSIZE;
#endif /* NANOEM_IO_HAS_POINT */
};
#if !defined(GLSLANG)
typedef vs_output_t ps_input_t;
#else
struct ps_input_t {
    float4 color0 : COLOR0;
    float3 normal : TEXCOORD0;
    float2 texcoord0 : TEXCOORD1;
    float2 texcoord1 : TEXCOORD2;
    float3 eye : TEXCOORD3;
    float4 shadow0 : TEXCOORD4;
};
#endif

static const float kAlphaTestThreshold = 0.005;

#endif /* NANOEM_SHADER_IO_HLSL_ */
