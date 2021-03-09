float4 vs_syntax_for(float4 position : POSITION) : POSITION
{
	float total = 0.0;
	[unroll]
	for (float i = 0; i < 8; i++) {
		total += i;
	}
	return total;
}

float4 ps_syntax_for() : COLOR0
{
	float total = 0.0;
	[unroll]
	for (float i = 8; i < 0; i--) {
		total -= i;
	}
	return total;
}

technique syntax_test_technique {
  pass syntax_test_for_pass {
  	VertexShader = compile vs_3_0 vs_syntax_for();
  	PixelShader = compile vs_3_0 ps_syntax_for();
  }
}
