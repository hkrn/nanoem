/* inverse + transpose */
float4x4 camera_inverse_transpose_world : WORLDINVERSETRANSPOSE;
float4x4 camera_inverse_transpose_view : VIEWINVERSETRANSPOSE;
float4x4 camera_inverse_transpose_projection : PROJECTIONINVERSETRANSPOSE;
float4x4 camera_inverse_transpose_world_view : WORLDVIEWINVERSETRANSPOSE;
float4x4 camera_inverse_transpose_view_projection : VIEWPROJECTIONINVERSETRANSPOSE;
float4x4 camera_inverse_transpose_world_view_projection : WORLDVIEWPROJECTIONINVERSETRANSPOSE;

#include "../shaders.fx"
