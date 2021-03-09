/* transpose */
float4x4 light_transpose_world : WORLDTRANSPOSE < string Object = "Light"; >;
float4x4 light_transpose_view : VIEWTRANSPOSE < string Object = "Light"; >;
float4x4 light_transpose_projection : PROJECTIONTRANSPOSE < string Object = "Light"; >;
float4x4 light_transpose_world_view : WORLDVIEWTRANSPOSE < string Object = "Light"; >;
float4x4 light_transpose_view_projection : VIEWPROJECTIONTRANSPOSE < string Object = "Light"; >;
float4x4 light_transpose_world_view_projection : WORLDVIEWPROJECTIONTRANSPOSE < string Object = "Light"; >;

#include "../shaders.fx"