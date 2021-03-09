struct vs_input_t {
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 texcoord0 : TEXCOORD0;
	float4 color0 : COLOR0;
};
struct vs_output_t {
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 texcoord0 : TEXCOORD0;
	float4 color0 : COLOR0;
};
struct ps_output_t {
	float4 color0 : COLOR0;
	float4 color1 : COLOR1;
	float4 color2 : COLOR2;
	float4 color3 : COLOR3;
};

struct struct_members {
	bool a;
	int b;
	float c;
	bool a_array[2];
	int b_array[3];
	float c_array[4];
	bool a_semantic : SEMANTIC0;
	int b_semantic : SEMANTIC1;
	float c_semantic : SEMANTIC2;
	bool a_annotation < bool key = false; >;
	int b_annotation < int key = 42; >;
	float c_annotation < float key = 42.0; >;
	bool a_semantic_annotation : SEMANTIC0 < bool key = false; >;
	int b_semantic_annotation : SEMANTIC1 < int key = 42; >;
	float c_semantic_annotation : SEMANTIC2 < float key = 42.0; >;
};

vs_output_t vs_syntax_struct(vs_input_t input)
{
	struct_members members = (struct_members) 0;
	vs_output_t output = (vs_output_t) 0;
	output.position = input.position;
	output.normal = input.normal;
	output.color0 = input.color0;
	output.texcoord0 = input.texcoord0;
	return output;
}

ps_output_t vs_syntax_struct(vs_output_t input)
{
	ps_output_t output = (ps_output_t) 0;
	output.color0 = input.color0;
	output.color1 = input.color0;
	output.color2 = input.color0;
	output.color3 = input.color0;
	return output;
}

technique syntax_test_technique {
  pass syntax_test_struct_pass {
  	VertexShader = compile vs_3_0 vs_syntax_global_struct();
  	PixelShader = compile vs_3_0 ps_syntax_global_struct();
  }
}
