float4 stub_vs(float4 position : POSITION) : POSITION
{
	return float4(0, 0, 0, 1);
}

float4 stub_ps() : COLOR0
{
	return float4(1, 1, 1, 1);
}

technique technique_script_test_technque <
    string Script =
      "RenderTargetColor0="
      "ScriptExternal=color"
      "RenderTargetColor0="
      "ClearSetColor="
      "ClearSetDepth="
      "Clear=Color"
      "Clear=Depth"
      ;
  > {
  pass technique_script_test_pass {
  	VertexShader = compile vs_3_0 stub_vs();
  	PixelShader = compile vs_3_0 stub_ps();
  }
}
