static bool var_bool_1x1 = false;
static bool2 var_bool_1x2 = bool2(false, true);
static bool3 var_bool_1x3 = bool3(false, true, false);
static bool4 var_bool_1x4 = bool4(false, true, false, true);
static bool2x2 var_bool_2x2 = bool2x2(bool2(false, true), bool2(false, true));
static bool2x3 var_bool_2x3 = bool2x3(bool2(false, true), bool2(false, true), bool2(false, true));
static bool2x4 var_bool_2x4 = bool2x4(bool2(false, true), bool2(false, true), bool2(false, true), bool2(false, true));
static bool3x2 var_bool_3x2 = bool3x2(bool3(false, true, false), bool2(true, false, true));
static bool3x3 var_bool_3x3 = bool3x3(bool3(false, true, false), bool2(true, false, true), bool3(false, true, false));
static bool3x4 var_bool_3x4 = bool3x4(bool3(false, true, false), bool2(true, false, true), bool3(false, true, false), bool2(true, false, true));
static bool4x2 var_bool_4x2 = bool4x2(bool4(false, true, false, true), bool4(false, true, false, true));
static bool4x3 var_bool_4x3 = bool4x3(bool4(false, true, false, true), bool4(false, true, false, true), bool4(false, true, false, true));
static bool4x4 var_bool_4x4 = bool4x4(bool4(false, true, false, true), bool4(false, true, false, true), bool4(false, true, false, true), bool4(false, true, false, true));

static half var_half_1x1 = 0;
static half2 var_half_1x2 = half2(0, 1);
static half3 var_half_1x3 = half3(0, 1, 2);
static half4 var_half_1x4 = half4(0, 1, 2, 3);
static half2x2 var_half_2x2 = half2x2(half2(0, 1), half2(2, 3));
static half2x3 var_half_2x3 = half2x3(half2(0, 1), half2(2, 3), half2(4, 5));
static half2x4 var_half_2x4 = half2x4(half2(0, 1), half2(2, 3), half2(4, 5), half2(6, 7));
static half3x2 var_half_3x2 = half3x2(half3(0, 1, 2), half3(3, 4, 5));
static half3x3 var_half_3x3 = half3x3(half3(0, 1, 2), half3(3, 4, 5), half3(6, 7, 8));
static half3x4 var_half_3x4 = half3x4(half3(0, 1, 2), half3(3, 4, 5), half3(6, 7, 8), half3(9, 10, 11));
static half4x2 var_half_4x2 = half4x2(half4(0, 1, 2, 3), half4(4, 5, 6, 7));
static half4x3 var_half_4x3 = half4x3(half4(0, 1, 2, 3), half4(4, 5, 6, 7), half4(8, 9, 10, 11));
static half4x4 var_half_4x4 = half4x4(half4(0, 1, 2, 3), half4(4, 5, 6, 7), half4(8, 9, 10, 11), half4(12, 13, 14, 15));

static int var_int_1x1 = 0;
static int2 var_int_1x2 = int2(0, 1);
static int3 var_int_1x3 = int3(0, 1, 2);
static int4 var_int_1x4 = int4(0, 1, 2, 3);
static int2x2 var_int_2x2 = int2x2(int2(0, 1), int2(2, 3));
static int2x3 var_int_2x3 = int2x3(int2(0, 1), int2(2, 3), int2(4, 5));
static int2x4 var_int_2x4 = int2x4(int2(0, 1), int2(2, 3), int2(4, 5), int2(6, 7));
static int3x2 var_int_3x2 = int3x2(int3(0, 1, 2), int3(3, 4, 5));
static int3x3 var_int_3x3 = int3x3(int3(0, 1, 2), int3(3, 4, 5), int3(6, 7, 8));
static int3x4 var_int_3x4 = int3x4(int3(0, 1, 2), int3(3, 4, 5), int3(6, 7, 8), int3(9, 10, 11));
static int4x2 var_int_4x2 = int4x2(int4(0, 1, 2, 3), int4(4, 5, 6, 7));
static int4x3 var_int_4x3 = int4x3(int4(0, 1, 2, 3), int4(4, 5, 6, 7), int4(8, 9, 10, 11));
static int4x4 var_int_4x4 = int4x4(int4(0, 1, 2, 3), int4(4, 5, 6, 7), int4(8, 9, 10, 11), int4(12, 13, 14, 15));

static float var_float_1x1 = 0;
static float2 var_float_1x2 = float2(0, 1);
static float3 var_float_1x3 = float3(0, 1, 2);
static float4 var_float_1x4 = float4(0, 1, 2, 3);
static float2x2 var_float_2x2 = float2x2(float2(0, 1), float2(2, 3));
static float2x3 var_float_2x3 = float2x3(float2(0, 1), float2(2, 3), float2(4, 5));
static float2x4 var_float_2x4 = float2x4(float2(0, 1), float2(2, 3), float2(4, 5), float2(6, 7));
static float3x2 var_float_3x2 = float3x2(float3(0, 1, 2), float3(3, 4, 5));
static float3x3 var_float_3x3 = float3x3(float3(0, 1, 2), float3(3, 4, 5), float3(6, 7, 8));
static float3x4 var_float_3x4 = float3x4(float3(0, 1, 2), float3(3, 4, 5), float3(6, 7, 8), float3(9, 10, 11));
static float4x2 var_float_4x2 = float4x2(float4(0, 1, 2, 3), float4(4, 5, 6, 7));
static float4x3 var_float_4x3 = float4x3(float4(0, 1, 2, 3), float4(4, 5, 6, 7), float4(8, 9, 10, 11));
static float4x4 var_float_4x4 = float4x4(float4(0, 1, 2, 3), float4(4, 5, 6, 7), float4(8, 9, 10, 11), float4(12, 13, 14, 15));

static shared bool shared_var_bool;
static shared half shared_var_half;
static shared int shared_var_int;
static shared float shared_var_float;

float4 vs_syntax_global_variables(float4 position : POSITION) : POSITION
{
	return 0.0;
}

float4 ps_syntax_global_variables() : COLOR0
{
	return 0.0;
}

technique syntax_test_technique {
  pass syntax_test_ge_pass {
  	VertexShader = compile vs_3_0 vs_syntax_global_variables();
  	PixelShader = compile vs_3_0 ps_syntax_global_variables();
  }
}
