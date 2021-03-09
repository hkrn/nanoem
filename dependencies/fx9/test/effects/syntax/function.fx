float vs_syntax_prototype(void);
float ps_syntax_prototype(void);

float vs_syntax_override(bool a);
float vs_syntax_override(bool a, int b);
float vs_syntax_override(bool a, int b, float c);
float ps_syntax_default(bool a = false, int b = 4, float c = 16.0);

float4 vs_syntax_function(float4 position : POSITION) : POSITION;
float4 ps_syntax_function() : COLOR0;
float4 vs_syntax_function_inout(float4 position : POSITION) : POSITION;
float4 ps_syntax_function_inout() : COLOR0;

float vs_syntax_prototype(void) {
	return 42.0;
}

float ps_syntax_prototype(void) {
	return 42.0;
}

float vs_syntax_override(bool a)
{
	return a ? 1.0 : 0.0;
}

float vs_syntax_override(bool a, int b)
{
	return vs_syntax_override(a) * b;
}

float vs_syntax_override(bool a, int b, float c)
{
	return vs_syntax_override(a, b) * c;
}

float ps_syntax_default(bool a = false, int b = 4, float c = 16.0)
{
	return a * b * c;
}

inline float vs_syntax_inline(void) {
	return 42.0;
}

inline float ps_syntax_inline(void) {
	return 42.0;
}

void function_inout(in float a, out float b, inout float c) {
	float d = c;
	c = a;
	b = d;
}

float4 vs_syntax_function(float4 position : POSITION) : POSITION
{
	return vs_syntax_override(vs_syntax_prototype(), 4, 16.0);
}

float4 ps_syntax_function() : COLOR0
{
	return ps_syntax_prototype() * ps_syntax_default();
}

float4 vs_syntax_function_inout(float4 position : POSITION) : POSITION
{
	float a = vs_syntax_inline(), b, c = 0.0;
	function_inout(a, b, c);
	return c;
}

float4 ps_syntax_function_inout() : COLOR0
{
	float a = ps_syntax_inline(), b, c = 0.0;
	function_inout(a, b, c);
	return b;
}

technique syntax_test_technique {
  pass syntax_test_function {
  	VertexShader = compile vs_3_0 vs_syntax_function();
  	PixelShader = compile vs_3_0 ps_syntax_function();
  }
  pass syntax_test_function_inout {
  	VertexShader = compile vs_3_0 vs_syntax_function_inout();
  	PixelShader = compile vs_3_0 ps_syntax_function_inout();
  }
}
