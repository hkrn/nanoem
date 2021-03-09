float a;
float b;

float4 vs_syntax_add(float4 position : POSITION) : POSITION
{
	return a + b;
}

float4 ps_syntax_add() : COLOR0
{
	return b + a;
}

float4 vs_syntax_add_assign(float4 position : POSITION) : POSITION
{
	float c = 0;
	c += a;
	return c;
}

float4 ps_syntax_add_assign() : COLOR0
{
	float c = 0;
	c += b;
	return c;
}

float4 vs_syntax_pre_incr(float4 position : POSITION) : POSITION
{
	float c = a;
	++c;
	return c;
}

float4 ps_syntax_post_incr() : COLOR0
{
	float c = b;
	c++;
	return c;
}

technique syntax_test_technique {
  pass syntax_test_add_pass {
  	VertexShader = compile vs_3_0 vs_syntax_add();
  	PixelShader = compile vs_3_0 ps_syntax_add();
  }
  pass syntax_test_add_assign_pass {
  	VertexShader = compile vs_3_0 vs_syntax_add_assign();
  	PixelShader = compile vs_3_0 ps_syntax_add_assign();
  }
  pass syntax_test_incr {
    VertexShader = compile vs_3_0 vs_syntax_pre_incr();
    PixelShader = compile vs_3_0 ps_syntax_post_incr();
  }
}
