#include "foo\\bar\\baz\\config.fxsub"

float4x4 g_worldViewProjectMatrix : WORLDVIEWPROJECTION;

struct VS_OUTPUT
{
    float4 position : POSITION;
    float3 normal   : TEXCOORD1;
    float2 texcoord : TEXCOORD2;
};

texture2D s_texture <
	string ResourceName = "textures\\foo\\bar\\baz\\1px.png";
	int MipLevels = 1;
	string Format = "A8R8G8B8";
>;
sampler s_sampler = sampler_state
{
    texture = <s_texture>;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MIPFILTER = LINEAR;
    ADDRESSU  = WRAP;
    ADDRESSV  = WRAP;
};

VS_OUTPUT
vs_main(float4 position : POSITION, float3 normal : NORMAL, float2 texcoord : TEXCOORD0)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.position = mul(position, g_worldViewProjectMatrix);
    output.normal = normalize(normal);
    output.texcoord = texcoord;
    return output;
}

float4
ps_main(VS_OUTPUT input) : COLOR
{
    return float4(input.normal, 1) * tex2D(s_sampler, input.texcoord) * FX9_MACRO_VALUE_ONE;
}

technique MainTechnique < string MMDPass = "object"; >
{
    pass DrawObject
    {
        VertexShader = compile vs_2_0 vs_main();
        PixelShader  = compile ps_2_0 ps_main();
    }
}
