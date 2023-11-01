/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_SHADER_SHADOW_HLSL_
#define NANOEM_SHADER_SHADOW_HLSL_

#include "macros.hlsl"

GLSLANG_ANNOTATION([[vk::binding(0 + WGSL_TEXTURE_OFFSET, VK_DESCRIPTOR_SET_TEXTURE)]])
Texture2D u_shadowTexture : register(t0);

GLSLANG_ANNOTATION([[vk::binding(0 + WGSL_SAMPLER_OFFSET, VK_DESCRIPTOR_SET_SAMPLER)]])
SamplerState u_shadowTextureSampler : register(s0);

float
shadowCoverage(float4 texcoord, float4 sms)
{
    const float kThresholdType1 = 1500.0;
    const float kThresholdType2 = 8000.0;
    float receiverDepth = texcoord.z,
        shadowMapDepth = u_shadowTexture.Sample(u_shadowTextureSampler, texcoord.xy).x,
        component = saturate(receiverDepth - shadowMapDepth);
    int converageTypeInt = int(sms.w);
    if (converageTypeInt == 2) {
        component = saturate(component * kThresholdType2 * texcoord.y - 0.3);
    }
    else if (converageTypeInt == 1) {
        component = saturate(component * kThresholdType1 - 0.3);
    }
    return 1.0 - component;
}

#endif /* NANOEM_SHADER_SHADOW_HLSL_ */
