/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "nanoem/io.hlsl"
#include "nanoem/model.hlsl"
#include "nanoem/shadow.hlsl"

vs_output_t
nanoemVSMain(vs_input_t input)
{
    float4 position = float4(input.position, 1),
           normal = float4(input.normal, 0);
    float3 color = (u_materialDiffuse.rgb * u_lightColor.rgb + u_materialAmbient.rgb)
                 * lerp(1.0, input.uva1.xyz, u_enableVertexColor.xyz);
    float2 sphere = normalize(mul(u_modelViewMatrix, normal)).xy * 0.5 + 0.5;
    vs_output_t output;
    output.position = mul(u_modelViewProjectionMatrix, position);
    output.normal = normal.xyz;
    output.eye = u_cameraPosition.xyz - (mul(u_modelMatrix, position)).xyz;
    output.texcoord0 = input.texcoord0;
    output.texcoord1 = sphere;
    output.color0 = saturate(float4(color, u_materialDiffuse.a));
    output.shadow0 = mul(u_lightViewProjectionMatrix, position);
#ifdef NANOEM_IO_HAS_POINT
    output.psize = 1.0;
#endif
    return output;
}

float4
nanoemPSMain(ps_input_t input) : SV_TARGET0
{
    const int kToonFactor = 3;
    float4 materialColor = input.color0;
    if (hasDiffuseTexture()) {
        float2 texcoord0 = input.texcoord0;
        float4 texel = u_diffuseTexture.Sample(u_diffuseTextureSampler, texcoord0);
        materialColor.rgb *= (texel.rgb * u_diffuseTextureBlendFactor.rgb) * u_diffuseTextureBlendFactor.a;
        materialColor.a *= texel.a;
    }
    if (hasSphereTexture()) {
        float2 texcoord1 = input.texcoord1;
        if (isSphereTextureMultiply()) {
            float4 texel = u_spheremapTexture.Sample(u_spheremapTextureSampler, texcoord1);
            materialColor.rgb *= (texel.rgb * u_sphereTextureBlendFactor.rgb) * u_sphereTextureBlendFactor.a;
            materialColor.a *= texel.a;
        }
        else if (isSphereTextureAsSubTexture()) {
            materialColor *= u_spheremapTexture.Sample(u_spheremapTextureSampler, texcoord1);
        }
        else if (isSphereTextureAdditive()) {
            float4 texel = u_spheremapTexture.Sample(u_spheremapTextureSampler, texcoord1);
            materialColor.rgb += (texel.rgb * u_sphereTextureBlendFactor.rgb) * u_sphereTextureBlendFactor.a;
            materialColor.a *= texel.a;
        }
    }
    float3 lightPosition = -u_lightDirection.xyz;
    if (hasShadowMapTexture()) {
        float4 texcoord0 = input.shadow0 / input.shadow0.w;
        if (all(saturate(texcoord0.xy) == texcoord0.xy)) {
            float4 toonColor = u_toonTexture.Sample(u_toonTextureSampler, float2(0, 1)),
                   shadowColor = materialColor;
            shadowColor.rgb *= (toonColor.rgb * u_toonTextureBlendFactor.rgb) * u_toonTextureBlendFactor.a;
            shadowColor.a *= toonColor.a;
            float coverage = shadowCoverage(texcoord0, u_shadowMapSize);
            coverage = min(saturate(dot(input.normal, lightPosition) * kToonFactor), coverage);
            materialColor = lerp(shadowColor, materialColor, coverage);
        }
    }
    else if (hasToonTexture()) {
        float y = 0.5 - dot(input.normal, lightPosition) * 0.5;
        float4 toonColor = u_toonTexture.Sample(u_toonTextureSampler, float2(0, y));
        toonColor.rgb *= (toonColor.rgb * u_toonTextureBlendFactor.rgb) * u_toonTextureBlendFactor.a;
        toonColor.a *= toonColor.a;
        materialColor *= toonColor;
    }
    clip(materialColor.a - kAlphaTestThreshold);
    float specularPower = u_materialSpecular.a;
    if (specularPower > 0) {
        float3 halfVector = normalize(lightPosition + normalize(input.eye));
        float specularAngle = max(dot(normalize(input.normal), halfVector), 0.0),
            spec = pow(specularAngle, specularPower);
        materialColor.rgb += u_materialSpecular.rgb * u_lightColor.rgb * spec;
    }
    return saturate(materialColor);
}
