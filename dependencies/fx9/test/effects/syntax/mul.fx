float a;
float b;

float4 vs_syntax_mul(float4 position : POSITION) : POSITION
{
	return a * b;
}

float4 ps_syntax_mul() : COLOR0
{
	return b * a;
}

float4 vs_syntax_mul_assign(float4 position : POSITION) : POSITION
{
	float c = 0;
	c *= a;
	return c;
}

float4 ps_syntax_mul_assign() : COLOR0
{
	float c = 0;
	c *= b;
	return c;
}

technique syntax_test_technique {
  pass syntax_test_mul_pass {
  	VertexShader = compile vs_3_0 vs_syntax_mul();
  	PixelShader = compile vs_3_0 ps_syntax_mul();
  }
  pass syntax_test_mul_assign_pass {
  	VertexShader = compile vs_3_0 vs_syntax_mul_assign();
  	PixelShader = compile vs_3_0 ps_syntax_mul_assign();
  }
}
