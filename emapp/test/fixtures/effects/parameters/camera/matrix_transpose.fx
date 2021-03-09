/* transpose */
float4x4 camera_transpose_world : WORLDTRANSPOSE;
float4x4 camera_transpose_view : VIEWTRANSPOSE;
float4x4 camera_transpose_projection : PROJECTIONTRANSPOSE;
float4x4 camera_transpose_world_view : WORLDVIEWTRANSPOSE;
float4x4 camera_transpose_view_projection : VIEWPROJECTIONTRANSPOSE;
float4x4 camera_transpose_world_view_projection : WORLDVIEWPROJECTIONTRANSPOSE;

#include "../shaders.fx"
