/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_SHADER_ACCESSORY_HLSL_
#define NANOEM_SHADER_ACCESSORY_HLSL_

#include "macros.hlsl"

struct accessory_parameters_t {
    float4x4 u_modelMatrix;
    float4x4 u_modelViewMatrix;
    float4x4 u_modelViewProjectionMatrix;
    float4x4 u_lightViewProjectionMatrix;
    float4 u_lightColor;
    float4 u_lightDirection;
    float4 u_cameraPosition;
    float4 u_materialAmbient;
    float4 u_materialDiffuse;
    float4 u_materialSpecular;
    float4 u_useTextureSampler;
    float4 u_sphereTextureType;
    float4 u_shadowMapSize;
};

GLSLANG_ANNOTATION([[vk::binding(1 + WGSL_TEXTURE_OFFSET, VK_DESCRIPTOR_SET_TEXTURE)]])
Texture2D u_diffuseTexture : register(t1);

GLSLANG_ANNOTATION([[vk::binding(1 + WGSL_SAMPLER_OFFSET, VK_DESCRIPTOR_SET_SAMPLER)]])
SamplerState u_diffuseTextureSampler : register(s1);

GLSLANG_ANNOTATION([[vk::binding(2 + WGSL_TEXTURE_OFFSET, VK_DESCRIPTOR_SET_TEXTURE)]])
Texture2D u_spheremapTexture : register(t2);

GLSLANG_ANNOTATION([[vk::binding(2 + WGSL_SAMPLER_OFFSET, VK_DESCRIPTOR_SET_SAMPLER)]])
SamplerState u_spheremapTextureSampler : register(s2);

#if defined(GLSLANG)
[[vk::binding(0 + WGSL_VS_UNIFORM_OFFSET, VK_DESCRIPTOR_SET_UNIFORM)]]
ConstantBuffer<accessory_parameters_t> vs : register(b0);
[[vk::binding(1 + WGSL_FS_UNIFORM_OFFSET, VK_DESCRIPTOR_SET_UNIFORM)]]
ConstantBuffer<accessory_parameters_t> fs : register(b1);
#else
#if defined(NANOEM_IS_VERTEX_SHADER)
cbuffer accessory_parameters_buffer_t : register(b0) {
    accessory_parameters_t vs;
};
accessory_parameters_t fs; /* stub */
#elif defined(NANOEM_IS_PIXEL_SHADER)
cbuffer accessory_parameters_buffer_t : register(b0) {
    accessory_parameters_t fs;
};
accessory_parameters_t vs; /* stub */
#else
#error NANOEM_IS_VERTEX_SHADER or NANOEM_IS_PIXEL_SHADER must be defined
#endif
#endif /* GLSLANG */

bool
hasDiffuseTexture()
{
  return fs.u_useTextureSampler.x != 0.0;
}

bool
hasSphereTexture()
{
  return fs.u_useTextureSampler.y != 0.0;
}

bool
hasShadowMapTexture()
{
  return fs.u_useTextureSampler.w != 0.0;
}

bool
isSphereTextureMultiply()
{
  return fs.u_sphereTextureType.x != 0.0;
}

bool
isSphereTextureAdditive()
{
  return fs.u_sphereTextureType.z != 0.0;
}

float4
coverageAlpha(ps_input_t input, float4 rgba)
{
    if (hasDiffuseTexture()) {
        float2 texcoord0 = input.texcoord0;
        float4 texel =  u_diffuseTexture.Sample(u_diffuseTextureSampler, texcoord0);
        rgba.a *= texel.a;
    }
    if (hasSphereTexture()) {
        float2 texcoord1 = input.texcoord1;
        if (isSphereTextureMultiply()) {
            rgba.a *= u_spheremapTexture.Sample(u_spheremapTextureSampler, texcoord1).a;
        }
        else if (isSphereTextureAdditive()) {
            rgba.a *= u_spheremapTexture.Sample(u_spheremapTextureSampler, texcoord1).a;
        }
    }
    clip(rgba.a - kAlphaTestThreshold);
    return rgba;
}

#endif /* NANOEM_SHADER_ACCESSORY_HLSL_ */
