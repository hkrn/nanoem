/*
   Copyright (c) 2015-2020 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "nanoem/skinning.hlsl"

Texture2D u_matricesTexture : register(t0);
Texture2D u_morphWeightTexture : register(t1);
Texture2D u_sdefTexture : register(t2);
Texture2D u_verticesTexture : register(t3);
uniform float4 u_args;

static void
getBufferTextureCoord(uint width, uint height, uint index, out int3 coord)
{
    uint x = index % width;
    uint y = index / height;
    coord = int3(x, y, 0);
}

static float4x4
makeMatrix(float index)
{
    uint offset = uint(index) * 4;
	uint width, height;
	u_matricesTexture.GetDimensions(width, height);
    int3 coord;
    getBufferTextureCoord(width, height, offset + 0, coord);
	float4 x = u_matricesTexture.Load(coord);
    getBufferTextureCoord(width, height, offset + 1, coord);
    float4 y = u_matricesTexture.Load(coord);
    getBufferTextureCoord(width, height, offset + 2, coord);
    float4 z = u_matricesTexture.Load(coord);
    getBufferTextureCoord(width, height, offset + 3, coord);
    float4 w = u_matricesTexture.Load(coord);
    return transpose(float4x4(x, y, z, w));
}

static float3
makeVertexPositionDelta(uint index)
{
    uint numMorphDepths = uint(u_args.y);
    uint vertexTextureWidth, vertexTextureHeight, morphWeightTextureWidth, morphWeightTextureHeight;
	u_verticesTexture.GetDimensions(vertexTextureWidth, vertexTextureHeight);
	u_morphWeightTexture.GetDimensions(morphWeightTextureWidth, morphWeightTextureHeight);
    float3 vertexPositionDelta = 0;
    [loop]
    for (uint i = 0; i < numMorphDepths; i++) {
        uint offset = index * numMorphDepths + i;
    	int3 coord;
        getBufferTextureCoord(vertexTextureWidth, vertexTextureHeight, offset, coord);
        float4 value = u_verticesTexture.Load(coord);
        uint morphIndex = uint(value.w), morphOffset1 = morphIndex / 4, morphOffset2 = morphIndex % 4;
        getBufferTextureCoord(morphWeightTextureWidth, morphWeightTextureHeight, morphOffset1, coord);
        float weight = u_morphWeightTexture.Load(coord)[morphOffset2];
        if (weight != 0) {
            vertexPositionDelta += value.xyz * weight;
        }
    }
    return vertexPositionDelta;
}

struct vs_input_t {
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float4 texcoord : TEXCOORD0;
    float4 edge : TEXCOORD1;
    float4 uva1 : TEXCOORD2;
    float4 uva2 : TEXCOORD3;
    float4 uva3 : TEXCOORD4;
    float4 uva4 : TEXCOORD5;
    float4 weights : TEXCOORD6;
    float4 indices : TEXCOORD7;
    float4 info : TEXCOORD8;
};

struct vs_output_t {
    float4 sv_position : SV_POSITION;
	float4 position : TEXCOORD0;
    float4 normal : TEXCOORD1;
    float4 texcoord : TEXCOORD2;
	float4 edge : TEXCOORD3;
    float4 uva1 : TEXCOORD4;
    float4 uva2 : TEXCOORD5;
    float4 uva3 : TEXCOORD6;
    float4 uva4 : TEXCOORD7;
    float4 weights : TEXCOORD8;
    float4 indices : TEXCOORD9;
    float4 info : TEXCOORD10;
};

vs_output_t
nanoemVSMain(vs_input_t input)
{
    VertexUnit unit;
    unit.m_position = input.position;
    unit.m_normal = input.normal;
    unit.m_texcoord = input.texcoord;
    unit.m_edge = input.edge;
    unit.m_uva[0] = input.uva1;
    unit.m_uva[1] = input.uva2;
    unit.m_uva[2] = input.uva3;
    unit.m_uva[3] = input.uva4;
    unit.m_weights = input.weights;
    unit.m_indices = input.indices;
	unit.m_info = input.info;
    uint vertexIndex = uint(unit.m_info.z), sdefIndex = vertexIndex * 3, width, height;
	int3 coord;
	u_sdefTexture.GetDimensions(width, height);
    getBufferTextureCoord(width, height, sdefIndex + 0, coord);
    float4 sdefC = u_sdefTexture.Load(coord);
    getBufferTextureCoord(width, height, sdefIndex + 1, coord);
    float4 sdefR0 = u_sdefTexture.Load(coord);
    getBufferTextureCoord(width, height, sdefIndex + 2, coord);
    float4 sdefR1 = u_sdefTexture.Load(coord);
    SdefUnit sdef = { sdefC, sdefR0, sdefR1 };
    float3 vertexPositionDelta = makeVertexPositionDelta(vertexIndex);
    performSkinning(sdef, vertexPositionDelta, unit);
    unit.m_edge.xyz = unit.m_position.xyz + (unit.m_normal.xyz * unit.m_info.xxx) * u_args.xxx;
    vs_output_t output;
	output.sv_position = output.position = unit.m_position;
    output.normal = unit.m_normal;
    output.texcoord = unit.m_texcoord;
    output.edge = unit.m_edge;
    output.uva1 = unit.m_uva[0];
    output.uva2 = unit.m_uva[1];
    output.uva3 = unit.m_uva[2];
    output.uva4 = unit.m_uva[3];
    output.weights = unit.m_weights;
    output.indices = unit.m_indices;
	output.info = unit.m_info;
    return output;
}
