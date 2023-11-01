/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "nanoem/io.hlsl"
#include "nanoem/accessory.hlsl"
#include "nanoem/shadow.hlsl"

vs_output_t
nanoemVSMain(vs_input_t input)
{
    float4 position = float4(input.position, 1),
           normal = normalize(float4(input.normal, 0));
    float3 ambient = vs.u_materialDiffuse.rgb * (vs.u_lightColor.rgb - 0.3) + vs.u_materialAmbient.rgb,
           diffuse = vs.u_materialDiffuse.rgb * saturate(dot(normal.xyz, -vs.u_lightDirection.xyz)),
           color = ambient + input.color0.rgb * diffuse;
    float2 sphere = mul(vs.u_modelViewMatrix, normal).xy * 0.5 + 0.5;
    vs_output_t output;
    output.position = mul(vs.u_modelViewProjectionMatrix, position);
    output.normal = normal.xyz;
    output.eye = vs.u_cameraPosition.xyz - (mul(vs.u_modelMatrix, position)).xyz;
    output.texcoord0 = input.texcoord0;
    output.texcoord1 = sphere;
    output.color0 = saturate(float4(color, vs.u_materialAmbient.a * vs.u_materialDiffuse.a));
    output.shadow0 = mul(vs.u_lightViewProjectionMatrix, position);
    return output;
}

float4
nanoemPSMain(ps_input_t input) : SV_TARGET0
{
    float4 baseColor = 1;
    if (hasDiffuseTexture()) {
        float4 tc = u_diffuseTexture.Sample(u_diffuseTextureSampler, input.texcoord0);
        baseColor *= tc;
    }
    if (hasSphereTexture()) {
        float4 tc = u_spheremapTexture.Sample(u_spheremapTextureSampler, input.texcoord1);
        if (isSphereTextureMultiply()) {
            baseColor *= tc;
        }
        else if (isSphereTextureAdditive()) {
            baseColor.rgb += tc.rgb;
        }
    }
    float3 lightPosition = -fs.u_lightDirection.xyz,
           normal = normalize(input.normal),
           ambientColor = fs.u_materialDiffuse.rgb * (fs.u_lightColor.rgb - 0.3) + fs.u_materialAmbient.rgb;
    float4 materialColor = baseColor * input.color0,
           shadowColor = baseColor * float4(ambientColor, materialColor.a);
    if (hasShadowMapTexture()) {
        float4 texcoord0 = input.shadow0 / input.shadow0.w;
#if !defined(WGSL)
        if (all(saturate(texcoord0.xy) == texcoord0.xy)) {
#else
        {
#endif
            float coverage = shadowCoverage(texcoord0, fs.u_shadowMapSize);
            coverage = min(saturate(dot(normal, lightPosition)), coverage);
            materialColor = lerp(shadowColor, materialColor, coverage);
        }
    }
    clip(materialColor.a - kAlphaTestThreshold);
    float specularPower = fs.u_materialSpecular.a;
    if (specularPower > 0) {
        float3 halfVector = normalize(lightPosition + normalize(input.eye));
        float specularAngle = max(dot(normal, halfVector), 0.0),
            spec = pow(specularAngle, specularPower);
        materialColor.rgb += fs.u_materialSpecular.rgb * fs.u_lightColor.rgb * spec;
    }
    return saturate(materialColor);
}
