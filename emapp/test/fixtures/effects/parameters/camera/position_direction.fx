/* position / direction */
float4 camera_position_4 : POSITION < string Object = "Camera"; >;
float4 camera_direction_4 : DIRECTION < string Object = "Camera"; >;
float3 camera_position_3 : POSITION < string Object = "Camera"; >;
float3 camera_direction_3 : DIRECTION < string Object = "Camera"; >;

#include "../shaders.fx"
