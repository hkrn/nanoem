/* light */
float3 light_diffuse : DIFFUSE   < string Object = "Light"; >;
float3 light_ambient : AMBIENT   < string Object = "Light"; >;
float3 light_specular : SPECULAR  < string Object = "Light"; >;

#include "../shaders.fx"