/* inverse + transpose */
float4x4 light_inverse_transpose_world : WORLDINVERSETRANSPOSE < string Object = "Light"; >;
float4x4 light_inverse_transpose_view : VIEWINVERSETRANSPOSE < string Object = "Light"; >;
float4x4 light_inverse_transpose_projection : PROJECTIONINVERSETRANSPOSE < string Object = "Light"; >;
float4x4 light_inverse_transpose_world_view : WORLDVIEWINVERSETRANSPOSE < string Object = "Light"; >;
float4x4 light_inverse_transpose_view_projection : VIEWPROJECTIONINVERSETRANSPOSE < string Object = "Light"; >;
float4x4 light_inverse_transpose_world_view_projection : WORLDVIEWPROJECTIONINVERSETRANSPOSE < string Object = "Light"; >;

#include "../shaders.fx"