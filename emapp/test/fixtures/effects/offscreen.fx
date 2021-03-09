texture2D s_sceneGraphRenderTarget : RENDERCOLORTARGET <
  float2 ViewPortRatio = { 1.0, 1.0 };
  int MipLevels = 1;
  string Format = "X8R8G8B8" ;
>;
sampler2D s_sceneGraphSampler = sampler_state {
  texture = <s_sceneGraphRenderTarget>;
  MinFilter = LINEAR;
  MagFilter = LINEAR;
  MipFilter = NONE;
  AddressU  = CLAMP;
  AddressV  = CLAMP;
};

texture2D s_normalMapRenderTarget: OFFSCREENRENDERTARGET <
  string  Description = "NormalMap Render Target";
  float4 ClearColor = { 0, 0, 0, 1 };
  float  ClearDepth = 1.0;
  string Format = "X8R8G8B8" ;
  bool   AntiAlias = true;
  string DefaultEffect = 
    "self = hide;"
    "* = main.fx";
>;
sampler s_normalMapSampler = sampler_state {
  texture = <s_normalMapRenderTarget>;
  AddressU  = BORDER;
  AddressV = BORDER;
  Filter = NONE;
};

struct VS_OUTPUT
{
  float4 position : POSITION;
  float2 texcoord : TEXCOORD0;
};

VS_OUTPUT
vs_mix(float4 position: POSITION, float4 texcoord: TEXCOORD0)
{
  VS_OUTPUT output = (VS_OUTPUT) 0;
  output.position = position;
  output.texcoord = texcoord;
  return output;
}

float4
ps_mix(VS_OUTPUT output) : COLOR
{
  return tex2D(s_normalMapSampler, output.texcoord);
}

technique MainTechnique <
  string Script = 
    "RenderColorTarget0=s_sceneGraphMap;"
    "RenderDepthStencilTarget=DepthBuffer;"
    "ClearSetColor=ClearColor;"
    "ClearSetDepth=ClearDepth;"
    "Clear=Color;"
    "Clear=Depth;"
    "ScriptExternal=Color;"
    "RenderColorTarget0=;"
    "RenderDepthStencilTarget=;"
    "Pass=MixPass;"
    ;
>
{
  pass MixPass < string Script= "Draw=Buffer;"; > {
    AlphaBlendEnable = FALSE;
    VertexShader = compile vs_3_0 vs_mix();
    PixelShader  = compile ps_3_0 ps_mix();
  }
}
