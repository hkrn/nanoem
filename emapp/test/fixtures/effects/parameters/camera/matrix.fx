/* standard */
float4x4 camera_world : WORLD;
float4x4 camera_view : VIEW;
float4x4 camera_projection : PROJECTION;
float4x4 camera_world_view : WORLDVIEW;
float4x4 camera_view_projection : VIEWPROJECTION;
float4x4 camera_world_view_projection : WORLDVIEWPROJECTION;

#include "../shaders.fx"
