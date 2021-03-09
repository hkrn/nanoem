float a;
float b;

float4 vs_syntax_and(float4 position : POSITION) : POSITION
{
	return a && b;
}

float4 ps_syntax_and() : COLOR0
{
	return b && a;
}

technique syntax_test_technique {
  pass syntax_test_and_pass {
  	VertexShader = compile vs_3_0 vs_syntax_and();
  	PixelShader = compile vs_3_0 ps_syntax_and();
  }
}
