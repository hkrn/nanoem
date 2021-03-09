texture2D material_diffuse_texture : MATERIALTEXTURE;
sampler material_diffuse_texture_sampler = sampler_state {
    texture = <material_diffuse_texture>;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MIPFILTER = LINEAR;
    ADDRESSU  = WRAP;
    ADDRESSV  = WRAP;
};

texture2D material_spheremap_texture : MATERIALSPHEREMAP;
sampler material_spheremap_texture_sampler = sampler_state {
    texture = <material_spheremap_texture>;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MIPFILTER = LINEAR;
    ADDRESSU  = WRAP;
    ADDRESSV  = WRAP;
};

texture2D material_toon_texture : MATERIALTOONTEXTURE;
sampler material_toon_texture_sampler = sampler_state {
    texture = <material_toon_texture>;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MIPFILTER = LINEAR;
    ADDRESSU  = WRAP;
    ADDRESSV  = WRAP;
};

float4 material_diffuse_adding_texture : ADDINGTEXTURE;
float4 material_diffuse_multiplying_texture : MULTIPLYINGTEXTURE;
float4 material_spheremap_adding_texture : ADDINGSPHERETEXTURE;
float4 material_spheremap_multiplying_texture : MULTIPLYINGSPHERETEXTURE;

#include "../shaders.fx"