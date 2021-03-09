float a;
float b;

float4 vs_syntax_ge(float4 position : POSITION) : POSITION
{
	return a >= b;
}

float4 ps_syntax_ge() : COLOR0
{
	return b >= a;
}

technique syntax_test_technique {
  pass syntax_test_ge_pass {
  	VertexShader = compile vs_3_0 vs_syntax_ge();
  	PixelShader = compile vs_3_0 ps_syntax_ge();
  }
}
