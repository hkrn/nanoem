float a;
float b;

float4 vs_syntax_minus(float4 position : POSITION) : POSITION
{
	return a - b;
}

float4 ps_syntax_minus() : COLOR0
{
	return b - a;
}

float4 vs_syntax_minus_assign(float4 position : POSITION) : POSITION
{
	float c = 0;
	c -= a;
	return c;
}

float4 ps_syntax_minus_assign() : COLOR0
{
	float c = 0;
	c -= b;
	return c;
}

float4 vs_syntax_pre_decr(float4 position : POSITION) : POSITION
{
	float c = a;
	--c;
	return c;
}

float4 ps_syntax_post_decr() : COLOR0
{
	float c = b;
	c--;
	return c;
}

technique syntax_test_technique {
  pass syntax_test_minus_pass {
  	VertexShader = compile vs_3_0 vs_syntax_minus();
  	PixelShader = compile vs_3_0 ps_syntax_minus();
  }
  pass syntax_test_minus_assign_pass {
  	VertexShader = compile vs_3_0 vs_syntax_minus_assign();
  	PixelShader = compile vs_3_0 ps_syntax_minus_assign();
  }
  pass syntax_test_decr {
    VertexShader = compile vs_3_0 vs_syntax_pre_decr();
    PixelShader = compile vs_3_0 ps_syntax_post_decr();
  }
}
