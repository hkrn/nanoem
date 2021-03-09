/* material */
float4 material_diffuse : DIFFUSE  < string Object = "Geometry"; >;
float3 material_ambient : AMBIENT  < string Object = "Geometry"; >;
float3 material_emissive : EMISSIVE < string Object = "Geometry"; >;
float3 material_specular : SPECULAR < string Object = "Geometry"; >;
float  material_specular_power : SPECULARPOWER < string Object = "Geometry"; >;
float3 material_toon : TOONCOLOR;
float3 material_edge_color : EDGECOLOR;
float4 material_ground_shadow_color : GROUNDSHADOWCOLOR;

#include "../shaders.fx"