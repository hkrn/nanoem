/* inverse */
float4x4 camera_inverse_world : WORLDINVERSE;
float4x4 camera_inverse_view : VIEWINVERSE;
float4x4 camera_inverse_projection : PROJECTIONINVERSE;
float4x4 camera_inverse_world_view : WORLDVIEWINVERSE;
float4x4 camera_inverse_view_projection : VIEWPROJECTIONINVERSE;
float4x4 camera_inverse_world_view_projection : WORLDVIEWPROJECTIONINVERSE;

#include "../shaders.fx"
