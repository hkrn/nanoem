/*
   Copyright (c) 2015-2020 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_SHADER_SKINNING_HLSL_
#define NANOEM_SHADER_SKINNING_HLSL_

#include "macros.hlsl"

#define NANOEM_VERTEX_TYPE_BDEF1 0
#define NANOEM_VERTEX_TYPE_BDEF2 1
#define NANOEM_VERTEX_TYPE_BDEF4 2
#define NANOEM_VERTEX_TYPE_SDEF 3
#define NANOEM_VERTEX_TYPE_QDEF 4

struct VertexUnit {
    float4 m_position;
    float4 m_normal;
    float4 m_texcoord;
    float4 m_edge;
    float4 m_uva[4];
    float4 m_weights;
    float4 m_indices;
    float4 m_info; /* edgeSize,type,vertexIndex,padding */
};

struct SdefUnit {
    float4 m_c;
    float4 m_r0;
    float4 m_r1;
};

float4x4
makeMatrix(float index);

static float3x3
shrinkMatrix(const float4x4 m)
{
    return float3x3(m[0].xyz, m[1].xyz, m[2].xyz);
}

static float4
toQuaternion(const float4x4 m)
{
    float4 q;
    float x = m[0][0] - m[1][1] - m[2][2],
          y = m[1][1] - m[0][0] - m[2][2],
          z = m[2][2] - m[0][0] - m[1][1],
          w = m[0][0] + m[1][1] + m[2][2];
    float biggestValue = w;
    int biggestIndex = 0;
    if (x > biggestValue) {
    	biggestValue = x;
    	biggestIndex = 1;
    }
    if (y > biggestValue) {
    	biggestValue = y;
    	biggestIndex = 2;
    }
    if (z > biggestValue) {
    	biggestValue = z;
    	biggestIndex = 3;
    }
    float biggest = sqrt(biggestValue + 1.0) * 0.5, mult = 0.25 / biggest;
    switch(biggestIndex) {
    case 0: {
    	q.x = (m[1][2] - m[2][1]) * mult;
    	q.y = (m[2][0] - m[0][2]) * mult;
    	q.z = (m[0][1] - m[1][0]) * mult;
    	q.w = biggest;
    	break;
    }
    case 1: {
    	q.x = biggest;
    	q.y = (m[0][1] + m[1][0]) * mult;
    	q.z = (m[2][0] + m[0][2]) * mult;
    	q.w = (m[1][2] - m[2][1]) * mult;
    	break;
    }
    case 2: {
    	q.x = (m[0][1] + m[1][0]) * mult;
    	q.y = biggest;
    	q.z = (m[1][2] + m[2][1]) * mult;
    	q.w = (m[2][0] - m[0][2]) * mult;
    	break;
    }
    case 3: {
    	q.x = (m[2][0] + m[0][2]) * mult;
    	q.y = (m[1][2] + m[2][1]) * mult;
    	q.z = biggest;
    	q.w = (m[0][1] - m[1][0]) * mult;
    	break;
    }
    default:
        q = float4(0, 0, 0, 1);
    	break;
    }
    return q;
}

static float4
slerp(const float4 x, const float4 y, float a)
{
    float4 z = y;
    float theta = dot(x, y);
    if (theta < 0) {
        z = -y;
        theta = -theta;
    }
    float4 result;
    if (theta >= 1) {
        result = lerp(x, z, a);
    }
    else {
        float angle = acos(theta);
        result = (sin((1.0 - a) * angle) * x + sin(a * angle) * z) / sin(angle);
    }
    return result;
}

static float4x4
toMatrix(const float4 q)
{
    float4x4 m;
    m[0] = float4(
        1.0 - 2.0 * q.y * q.y - 2.0 * q.z * q.z,
        2.0 * q.x * q.y + 2.0 * q.w * q.z,
        2.0 * q.x * q.z - 2.0 * q.w * q.y,
        0.0
    );
    m[1] = float4(
        2.0 * q.x * q.y - 2.0 * q.w * q.z,
        1.0 - 2.0 * q.x * q.x - 2.0 * q.z * q.z,
        2.0 * q.y * q.z + 2.0 * q.w * q.x,
        0.0
    );
    m[2] = float4(
        2.0 * q.x * q.z + 2.0 * q.w * q.y,
        2.0 * q.y * q.z - 2.0 * q.w * q.x,
        1.0 - 2.0 * q.x * q.x - 2.0 * q.y * q.y,
        0.0
    );
    m[3] = float4(0, 0, 0, 1);
    return m;
}

static void
performSkinningBDEF1(const VertexUnit unit,
                     const float3 vertexPositionDelta,
                     out float4 position,
                     out float3 normal)
{
    float4x4 m0 = makeMatrix(unit.m_indices.x);
    position = mul(m0, float4(unit.m_position.xyz + vertexPositionDelta, 1));
    normal = mul(shrinkMatrix(m0), unit.m_normal.xyz);
}

static void
performSkinningBDEF2(const VertexUnit unit,
                     const float3 vertexPositionDelta,
                     out float4 position,
                     out float3 normal)
{
    float weight = unit.m_weights.x;
    position = float4(unit.m_position.xyz + vertexPositionDelta, 1);
    normal = unit.m_normal.xyz;
    if (weight == 0) {
        float4x4 m1 = makeMatrix(unit.m_indices.y);
        position = mul(m1, position);
        normal = mul(shrinkMatrix(m1), normal);
    }
    else if (weight == 1) {
        float4x4 m0 = makeMatrix(unit.m_indices.x);
        position = mul(m0, position);
        normal = mul(shrinkMatrix(m0), normal);
    }
    else {
        float4 indices = unit.m_indices;
        float4x4 m0 = makeMatrix(indices.x),
                 m1 = makeMatrix(indices.y);
        position = lerp(mul(m1, position), mul(m0, position), weight);
        normal = lerp(mul(shrinkMatrix(m1), normal), mul(shrinkMatrix(m0), normal), weight);
    }
}

static void
performSkinningBDEF4(const VertexUnit unit,
                     const float3 vertexPositionDelta,
                     out float4 position,
                     out float3 normal)
{
    float4 weights = unit.m_weights, indices = unit.m_indices;
    float4x4 m0 = makeMatrix(indices.x),
             m1 = makeMatrix(indices.y),
             m2 = makeMatrix(indices.z),
             m3 = makeMatrix(indices.w);
    position = float4(unit.m_position.xyz + vertexPositionDelta, 1);
    normal = unit.m_normal.xyz;
	position = mul(m0, position) * weights.xxxx
           	 + mul(m1, position) * weights.yyyy
             + mul(m2, position) * weights.zzzz
             + mul(m3, position) * weights.wwww;
    normal = mul(shrinkMatrix(m0), normal) * weights.xxx
           + mul(shrinkMatrix(m1), normal) * weights.yyy
           + mul(shrinkMatrix(m2), normal) * weights.zzz
           + mul(shrinkMatrix(m3), normal) * weights.www;
}

static void
performSkinningSDEF(const VertexUnit unit,
                    const SdefUnit sdef,
                    const float3 vertexPositionDelta,
                    out float4 position,
                    out float3 normal)
{
    float2 weights = unit.m_weights.xy, indices = unit.m_indices.xy;
    float4x4 m0 = makeMatrix(indices.x),
             m1 = makeMatrix(indices.y);
    float3 sdefC = sdef.m_c.xyz,
           sdefR0 = sdef.m_r0.xyz,
           sdefR1 = sdef.m_r1.xyz,
           sdefI = sdefR0 * weights.xxx + sdefR1 * weights.yyy,
           sdefR0N = sdefC + sdefR0 - sdefI,
           sdefR1N = sdefC + sdefR1 - sdefI,
           r0 = mul(m0, float4(sdefR0N, 1.0)).xyz,
           r1 = mul(m1, float4(sdefR1N, 1.0)).xyz,
           c0 = mul(m0, float4(sdefC, 1.0)).xyz,
           c1 = mul(m1, float4(sdefC, 1.0)).xyz,
           delta = (r0 + c0 - sdefC) * weights.xxx + (r1 + c1 - sdefC) * weights.yyy,
           t = (sdefC + delta) * 0.5;
    float4 q0 = toQuaternion(m0), q1 = toQuaternion(m1),
           p = float4(unit.m_position.xyz + vertexPositionDelta - sdefC, 1);
    float4x4 m = toMatrix(slerp(q1, q0, weights.x));
    position.xyz = mul(m, p).xyz + t;
    normal = mul(shrinkMatrix(m), unit.m_normal.xyz);
}

void
performSkinning(const SdefUnit sdef,
                const float3 vertexPositionDelta,
                inout VertexUnit unit)
{
    float4 position;
    float3 normal;
    uint type = uint(unit.m_info.y);
    if (type == NANOEM_VERTEX_TYPE_BDEF1) {
        performSkinningBDEF1(unit, vertexPositionDelta, position, normal);
    }
    else if (type == NANOEM_VERTEX_TYPE_BDEF2) {
        performSkinningBDEF2(unit, vertexPositionDelta, position, normal);
    }
    else if (type == NANOEM_VERTEX_TYPE_SDEF) {
#if defined(NANOEM_ENABLE_SDEF)
        performSkinningSDEF(unit, sdef, vertexPositionDelta, position, normal);
#else
        performSkinningBDEF2(unit, vertexPositionDelta, position, normal);
#endif
    }
    else if (type == NANOEM_VERTEX_TYPE_BDEF4 || type == NANOEM_VERTEX_TYPE_QDEF) {
        performSkinningBDEF4(unit, vertexPositionDelta, position, normal);
    }
    else {
        position = unit.m_position;
        normal = unit.m_normal.xyz;
    }
    unit.m_position = float4(position.xyz, unit.m_position.w);
    unit.m_normal = float4(normalize(normal), unit.m_normal.w);
}

#endif /* NANOEM_SHADER_SKINNING_HLSL_ */
