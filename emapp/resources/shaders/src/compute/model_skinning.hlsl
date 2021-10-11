/*
   Copyright (c) 2015-2020 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "nanoem/skinning.hlsl"

typedef StructuredBuffer<SdefUnit> SdefUnitBuffer;
typedef StructuredBuffer<float> MorphWeightBuffer;
typedef StructuredBuffer<float4> VertexPositionDeltaBuffer;
typedef StructuredBuffer<float4x4> MatricesBuffer;
typedef StructuredBuffer<VertexUnit> VertexUnitBuffer;

#if defined(GLSLANG)
GLSLANG_ANNOTATION([[vk::binding(0, VK_DESCRIPTOR_SET_UNIFORM)]])
uint4 c_arg : register(b0);
GLSLANG_ANNOTATION([[vk::binding(1, VK_DESCRIPTOR_SET_BUFFER)]])
MatricesBuffer u_matricesBuffer : register(t1);
GLSLANG_ANNOTATION([[vk::binding(2, VK_DESCRIPTOR_SET_BUFFER)]])
MorphWeightBuffer u_morphWeightBuffer : register(t2);
GLSLANG_ANNOTATION([[vk::binding(3, VK_DESCRIPTOR_SET_BUFFER)]])
VertexUnitBuffer u_vertexBuffer : register(t3);
GLSLANG_ANNOTATION([[vk::binding(4, VK_DESCRIPTOR_SET_BUFFER)]])
SdefUnitBuffer u_sdefBuffer : register(t4);
GLSLANG_ANNOTATION([[vk::binding(5, VK_DESCRIPTOR_SET_BUFFER)]])
VertexPositionDeltaBuffer u_vertexPositionDeltasBuffer : register(t5);
GLSLANG_ANNOTATION([[vk::binding(6, VK_DESCRIPTOR_SET_BUFFER)]])
RWStructuredBuffer<VertexUnit> o_vertexBuffer : register(u6);
#else
RWByteAddressBuffer o_vertexBuffer : register(u0);
uint4 c_arg : register(b0);
VertexUnitBuffer u_vertexBuffer : register(t0);
MatricesBuffer u_matricesBuffer : register(t1);
VertexPositionDeltaBuffer u_vertexPositionDeltasBuffer : register(t2);
SdefUnitBuffer u_sdefBuffer : register(t3);
MorphWeightBuffer u_morphWeightBuffer : register(t4);
#endif

float4x4
makeMatrix(float index)
{
    return u_matricesBuffer[uint(index)];
}

[numthreads(256, 1, 1)]
void
nanoemCSMain(const uint3 gid : SV_DispatchThreadID)
{
    uint index = gid.x;
    if (index >= c_arg.x) {
        return;
    }
    VertexUnit unit = u_vertexBuffer[index];
    SdefUnit sdef = u_sdefBuffer[index];
    float3 vertexPositionDelta = 0;
    uint numMorphDepths = c_arg.y;
    [loop][allow_uav_condition]
    for (uint i = 0; i < numMorphDepths; i++) {
        uint offset = index * numMorphDepths + i;
        float4 value = u_vertexPositionDeltasBuffer[offset];
        float morphIndex = value.w;
        float weight = u_morphWeightBuffer[uint(morphIndex)];
        if (weight != 0) {
            vertexPositionDelta += value.xyz * weight;
        }
    }
    performSkinning(sdef, vertexPositionDelta, unit);
    unit.m_edge.xyz = unit.m_position.xyz + (unit.m_normal.xyz * unit.m_info.xxx) * asfloat(c_arg.z);
#if defined(GLSLANG)
    o_vertexBuffer[index] = unit;
#else
    uint offset = index * 176;
    o_vertexBuffer.Store4(offset +   0, asuint(unit.m_position));
    o_vertexBuffer.Store4(offset +  16, asuint(unit.m_normal));
    o_vertexBuffer.Store4(offset +  32, asuint(unit.m_texcoord));
    o_vertexBuffer.Store4(offset +  48, asuint(unit.m_edge));
    o_vertexBuffer.Store4(offset +  64, asuint(unit.m_uva[0]));
    o_vertexBuffer.Store4(offset +  80, asuint(unit.m_uva[1]));
    o_vertexBuffer.Store4(offset +  96, asuint(unit.m_uva[2]));
    o_vertexBuffer.Store4(offset + 112, asuint(unit.m_uva[3]));
    o_vertexBuffer.Store4(offset + 128, asuint(unit.m_weights));
    o_vertexBuffer.Store4(offset + 144, asuint(unit.m_indices));
    o_vertexBuffer.Store4(offset + 160, asuint(unit.m_info));
#endif
}
