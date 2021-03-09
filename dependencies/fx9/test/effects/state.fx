float4 stub_vs(float4 position : POSITION) : POSITION
{
	return float4(0, 0, 0, 1);
}

float4 stub_ps() : COLOR0
{
	return float4(1, 1, 1, 1);
}

technique state_test_technique {
  pass all_states {
  	ZEnable = false;
  	FillMode = "";
  	ShadeMode = "";
  	ZWriteEnable = "";
  	AlphaTestEnable = "";
  	LastPixel = "";
  	SrcBlend = "";
  	DestBlend = "";
  	CullMode = "";
  	ZFunc = "";
  	AlphaRef = "";
  	AlphaFunc = "";
  	DitherEnable = "";
  	AlphaBlendEnable = false;
  	FogEnable = false;
  	SpecularEnable = false;
  	FogColor = "";
  	SpecularEnable = "";
  	FogColor = "";
  	FogTableMode = "";
  	FogStart = "";
  	FogEnd = "";
  	FogDensity = "";
  	RangeFogEnable = "";
  	StencilEnable = false;
  	StencilFail = "";
  	StencilZFail = "";
  	StencilPass = "";
  	StencilFunc = "";
  	StencilRef = "";
  	StencilMask = "";
  	StencilWriteMask = "";
  	TextureFactor = "";
  	Wrap0 = "";
  	Wrap1 = "";
  	Wrap2 = "";
  	Wrap3 = "";
  	Wrap4 = "";
  	Wrap5 = "";
  	Wrap6 = "";
  	Wrap7 = "";
  	Clipping = "";
  	Lighting = "";
  	Ambient = "";
  	FogVertexMode = "";
  	ColorVertex = "";
  	LocalViewer = "";
  	NormalizeNormals = "";
  	DiffuseMaterialSource = "";
  	SpecularMaterialSource = "";
  	AmbientMaterialSource = "";
  	EmissiveMaterialSource = "";
  	ClipPlaneEnable = "";
  	PointSize = "";
  	PointSize_Min = "";
  	PointSpriteEnable = "";
  	PointScaleEnable = "";
  	PointScale_A = "";
  	PointScale_B = "";
  	PointScale_C = "";
  	MultiSampleAliases = "";
  	MultiSampleMask = "";
  	PatchEdgeStyle = "";
  	DebugMonitorToken = "";
  	PointSize_Max = "";
  	IndexedVertexBlendEnable = "";
  	ColorWhiteEnable = "";
  	TweenFactor = "";
  	BlendOp = "";
  	PositionDegree = "";
  	NormalDegree = "";
  	ScissorTestEnable = "";
  	SlopeScaleDepthBias = "";
  	AntialiasedLineEnable = false;
  	MinTessellationLevel = 0;
  	MaxTessellationLevel = 0;
  	Adaptiveness_X = 0;
  	Adaptiveness_Y = 0;
  	Adaptiveness_Z = 0;
  	Adaptiveness_W = 0;
  	EnableAdaptiveTessellation = false;
  	TwoSidedStencilMode = "";
  	CCW_StencilFail = "";
  	CCW_StencilZFail = "";
  	CCW_StencilPass = "";
  	ColorWhiteEnable1 = "";
  	ColorWhiteEnable2 = "";
  	ColorWhiteEnable3 = "";
  	BlendFactor = "";
  	SRGBWriteEnable = "";
  	DepthBias = "";
  	Wrap8 = "";
  	Wrap9 = "";
  	Wrap10 = "";
  	Wrap11 = "";
  	Wrap12 = "";
  	Wrap13 = "";
  	Wrap14 = "";
  	Wrap15 = "";
  	SeparateAlphaBlendEnable = false;
  	SrcBlendAlpha = "";
  	DestBlendAlpha = "";
  	BlendOpAlpha = "";
  	VertexShader = compile vs_3_0 stub_vs();
  	PixelShader = compile vs_3_0 stub_ps();
  }
}
