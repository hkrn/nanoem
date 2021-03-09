float a;
float b;
bool c;

float4 vs_syntax_if(float4 position : POSITION) : POSITION
{
	if (a) {
		return 1.0;
	}
	else {
		return 0.0;
	}
}

float4 ps_syntax_if() : COLOR0
{
	if (c)
		return 1.0;
	else
		return 0.0;
}

float4 vs_syntax_ternary() : COLOR0
{
	return c ? 1.0 : 0.0;
}

float4 ps_syntax_ternary() : COLOR0
{
	return c ? 0.0 : 1.0;
}

technique syntax_test_technique {
  pass syntax_test_if_pass {
  	VertexShader = compile vs_3_0 vs_syntax_if();
  	PixelShader = compile vs_3_0 ps_syntax_if();
  }
  pass syntax_test_ternary_pass {
  	VertexShader = compile vs_3_0 vs_syntax_ternary();
  	PixelShader = compile vs_3_0 ps_syntax_ternary();
  }
}
