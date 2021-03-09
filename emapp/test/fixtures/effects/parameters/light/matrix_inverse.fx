/* inverse */
float4x4 light_inverse_world : WORLDINVERSE < string Object = "Light"; >;
float4x4 light_inverse_view : VIEWINVERSE < string Object = "Light"; >;
float4x4 light_inverse_projection : PROJECTIONINVERSE < string Object = "Light"; >;
float4x4 light_inverse_world_view : WORLDVIEWINVERSE < string Object = "Light"; >;
float4x4 light_inverse_view_projection : VIEWPROJECTIONINVERSE < string Object = "Light"; >;
float4x4 light_inverse_world_view_projection : WORLDVIEWPROJECTIONINVERSE < string Object = "Light"; >;

#include "../shaders.fx"