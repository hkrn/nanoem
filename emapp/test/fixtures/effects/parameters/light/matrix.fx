/* standard */
float4x4 light_world : WORLD < string Object = "Light"; >;
float4x4 light_view : VIEW < string Object = "Light"; >;
float4x4 light_projection : PROJECTION < string Object = "Light"; >;
float4x4 light_world_view : WORLDVIEW < string Object = "Light"; >;
float4x4 light_view_projection : VIEWPROJECTION < string Object = "Light"; >;
float4x4 light_world_view_projection : WORLDVIEWPROJECTION < string Object = "Light"; >;

#include "../shaders.fx"