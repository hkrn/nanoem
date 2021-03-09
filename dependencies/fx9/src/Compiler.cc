/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#include "fx9/Compiler.h"

/* win32 */
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <regex>

/* GLSLang */
#include "OGLCompilersDLL/InitializeDll.h"
#include "SPIRV/GlslangToSpv.h"
#include "SPIRV/Logger.h"
#include "SPIRV/disassemble.h"
#include "glslang/MachineIndependent/ScanContext.h"
#include "glslang/MachineIndependent/preprocessor/PpContext.h"

/* SPIRV-Cross */
#include "spirv_cross/spirv_cross.hpp"
#include "spirv_cross/spirv_glsl.hpp"
#include "spirv_cross/spirv_hlsl.hpp"
#include "spirv_cross/spirv_msl.hpp"

/* SPIRV-Tools */
#if defined(FX9_ENABLE_OPTIMIZER)
#include "spirv-tools/libspirv.hpp"
#include "spirv-tools/optimizer.hpp"
#endif

/* protobuf-c */
#include "effect.pb-c.h"
#include "effect_dx9ms.pb-c.h"
#include "effect_msl.pb-c.h"
#include "effect_spirv.pb-c.h"

using namespace glslang;

namespace {

static void
getDefaultResourceLimit(TBuiltInResource &resources)
{
    /* values from StandAlone/ResourceLimits.cpp */
    resources = { /* .MaxLights = */ 32,
        /* .MaxClipPlanes = */ 6,
        /* .MaxTextureUnits = */ 32,
        /* .MaxTextureCoords = */ 32,
        /* .MaxVertexAttribs = */ 64,
        /* .MaxVertexUniformComponents = */ 4096,
        /* .MaxVaryingFloats = */ 64,
        /* .MaxVertexTextureImageUnits = */ 32,
        /* .MaxCombinedTextureImageUnits = */ 80,
        /* .MaxTextureImageUnits = */ 32,
        /* .MaxFragmentUniformComponents = */ 4096,
        /* .MaxDrawBuffers = */ 32,
        /* .MaxVertexUniformVectors = */ 128,
        /* .MaxVaryingVectors = */ 8,
        /* .MaxFragmentUniformVectors = */ 16,
        /* .MaxVertexOutputVectors = */ 16,
        /* .MaxFragmentInputVectors = */ 15,
        /* .MinProgramTexelOffset = */ -8,
        /* .MaxProgramTexelOffset = */ 7,
        /* .MaxClipDistances = */ 8,
        /* .MaxComputeWorkGroupCountX = */ 65535,
        /* .MaxComputeWorkGroupCountY = */ 65535,
        /* .MaxComputeWorkGroupCountZ = */ 65535,
        /* .MaxComputeWorkGroupSizeX = */ 1024,
        /* .MaxComputeWorkGroupSizeY = */ 1024,
        /* .MaxComputeWorkGroupSizeZ = */ 64,
        /* .MaxComputeUniformComponents = */ 1024,
        /* .MaxComputeTextureImageUnits = */ 16,
        /* .MaxComputeImageUniforms = */ 8,
        /* .MaxComputeAtomicCounters = */ 8,
        /* .MaxComputeAtomicCounterBuffers = */ 1,
        /* .MaxVaryingComponents = */ 60,
        /* .MaxVertexOutputComponents = */ 64,
        /* .MaxGeometryInputComponents = */ 64,
        /* .MaxGeometryOutputComponents = */ 128,
        /* .MaxFragmentInputComponents = */ 128,
        /* .MaxImageUnits = */ 8,
        /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
        /* .MaxCombinedShaderOutputResources = */ 8,
        /* .MaxImageSamples = */ 0,
        /* .MaxVertexImageUniforms = */ 0,
        /* .MaxTessControlImageUniforms = */ 0,
        /* .MaxTessEvaluationImageUniforms = */ 0,
        /* .MaxGeometryImageUniforms = */ 0,
        /* .MaxFragmentImageUniforms = */ 8,
        /* .MaxCombinedImageUniforms = */ 8,
        /* .MaxGeometryTextureImageUnits = */ 16,
        /* .MaxGeometryOutputVertices = */ 256,
        /* .MaxGeometryTotalOutputComponents = */ 1024,
        /* .MaxGeometryUniformComponents = */ 1024,
        /* .MaxGeometryVaryingComponents = */ 64,
        /* .MaxTessControlInputComponents = */ 128,
        /* .MaxTessControlOutputComponents = */ 128,
        /* .MaxTessControlTextureImageUnits = */ 16,
        /* .MaxTessControlUniformComponents = */ 1024,
        /* .MaxTessControlTotalOutputComponents = */ 4096,
        /* .MaxTessEvaluationInputComponents = */ 128,
        /* .MaxTessEvaluationOutputComponents = */ 128,
        /* .MaxTessEvaluationTextureImageUnits = */ 16,
        /* .MaxTessEvaluationUniformComponents = */ 1024,
        /* .MaxTessPatchComponents = */ 120,
        /* .MaxPatchVertices = */ 32,
        /* .MaxTessGenLevel = */ 64,
        /* .MaxViewports = */ 16,
        /* .MaxVertexAtomicCounters = */ 0,
        /* .MaxTessControlAtomicCounters = */ 0,
        /* .MaxTessEvaluationAtomicCounters = */ 0,
        /* .MaxGeometryAtomicCounters = */ 0,
        /* .MaxFragmentAtomicCounters = */ 8,
        /* .MaxCombinedAtomicCounters = */ 8,
        /* .MaxAtomicCounterBindings = */ 1,
        /* .MaxVertexAtomicCounterBuffers = */ 0,
        /* .MaxTessControlAtomicCounterBuffers = */ 0,
        /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
        /* .MaxGeometryAtomicCounterBuffers = */ 0,
        /* .MaxFragmentAtomicCounterBuffers = */ 1,
        /* .MaxCombinedAtomicCounterBuffers = */ 1,
        /* .MaxAtomicCounterBufferSize = */ 16384,
        /* .MaxTransformFeedbackBuffers = */ 4,
        /* .MaxTransformFeedbackInterleavedComponents = */ 64,
        /* .MaxCullDistances = */ 8,
        /* .MaxCombinedClipAndCullDistances = */ 8,
        /* .MaxSamples = */ 4,
        /* .maxMeshOutputVerticesNV = */ 256,
        /* .maxMeshOutputPrimitivesNV = */ 512,
        /* .maxMeshWorkGroupSizeX_NV = */ 32,
        /* .maxMeshWorkGroupSizeY_NV = */ 1,
        /* .maxMeshWorkGroupSizeZ_NV = */ 1,
        /* .maxTaskWorkGroupSizeX_NV = */ 32,
        /* .maxTaskWorkGroupSizeY_NV = */ 1,
        /* .maxTaskWorkGroupSizeZ_NV = */ 1,
        /* .maxMeshViewCountNV = */ 4,
        /* .maxDualSourceDrawBuffersEXT = */ 1,

        /* .limits = */
        {
            /* .nonInductiveForLoops = */ 1,
            /* .whileLoops = */ 1,
            /* .doWhileLoops = */ 1,
            /* .generalUniformIndexing = */ 1,
            /* .generalAttributeMatrixVectorIndexing = */ 1,
            /* .generalVaryingIndexing = */ 1,
            /* .generalSamplerIndexing = */ 1,
            /* .generalVariableIndexing = */ 1,
            /* .generalConstantMatrixVectorIndexing = */ 1,
        } };
}

static std::string
trimFragColorVariable(const std::string &source)
{
    static const char *kRemovingCodes[] = { "out vec4 gl_FragColor;\n" };
    std::string trimmed(source);
    for (size_t i = 0; i < sizeof(kRemovingCodes) / sizeof(kRemovingCodes[0]); i++) {
        auto it = trimmed.find(kRemovingCodes[i]);
        if (it != std::string::npos) {
            trimmed = trimmed.erase(it, strlen(kRemovingCodes[i]));
        }
    }
    return trimmed;
}

static void
setVector4f(Fx9__Effect__Vector4f *vec, size_t index, float value)
{
    switch (index) {
    case 0:
        vec->x = value;
        break;
    case 1:
        vec->y = value;
        break;
    case 2:
        vec->z = value;
        break;
    case 3:
        vec->w = value;
        break;
    default:
        break;
    }
}

static void
setVector4i(Fx9__Effect__Vector4i *vec, size_t index, int value)
{
    switch (index) {
    case 0:
        vec->x = value;
        break;
    case 1:
        vec->y = value;
        break;
    case 2:
        vec->z = value;
        break;
    case 3:
        vec->w = value;
        break;
    default:
        break;
    }
}

static inline SpvVersion
createSpvVersion()
{
    SpvVersion spv;
    spv.spv = 0x10000;
    spv.vulkan = 100;
    return spv;
}

class BuiltInParsables : public TBuiltInParseables {
public:
    BuiltInParsables(fx9::Compiler::LanguageType language)
        : m_language(language)
    {
    }
    ~BuiltInParsables() override
    {
    }

    void
    initialize(int /* version */, EProfile /* profile */, const SpvVersion & /* spvVersion */) override
    {
        static const char *kGenTypeOneArgumentFunctions[] = { "abs", "acos", "asin", "atan", "ceil", "cos", "cosh",
            "ddx", "ddy", "degrees", "exp", "exp2", "floor", "frac", "log", "log10", "log2", "normalize", "radians",
            "round", "rsqrt", "saturate", "sign", "sin", "sinh", "sqrt", "tan", "tanh", "trunc" };
        static const char *kGenTypeTwoArgumentFunctions[] = { "atan2", "fmod", "max", "min", "mul", "pow", "reflect",
            "step" };
        static const char *kGenTypeThreeArgumentFunctions[] = { "faceforward", "fwidth", "lerp", "smoothstep" };
        static const char *kGenTypeTwoArgumentAndScalarFunctions[] = { "lerp", "refract" };
        static const char *kAggregatedGenTypeOneArgumentFunctions[] = { "length", "noise" };
        static const char *kAggregatedGenTypeTwoArgumentFunctions[] = { "distance", "dot" };
        static const char *kGenTypeTestFunctions[] = { "isfinite", "isinf", "isnan" };
        static const char *kOperandFunctions[] = { "all", "any" };
        m_commonString = "float clamp(float, float, float);\n"
                         "vec2 clamp(vec2, vec2, vec2);\n"
                         "vec3 clamp(vec3, vec3, vec3);\n"
                         "vec4 clamp(vec4, vec4, vec4);\n"
                         "vec2 clamp(vec2, vec2, float);\n"
                         "vec3 clamp(vec3, vec3, float);\n"
                         "vec4 clamp(vec4, vec4, float);\n"
                         "void clip(float);\n"
                         "void clip(vec2);\n"
                         "void clip(vec3);\n"
                         "void clip(vec4);\n"
                         "vec3 cross(vec3, vec3);\n"
                         "float determinant(mat2);\n"
                         "float determinant(mat3);\n"
                         "float determinant(mat4);\n"
                         "float frexp(float, out int);\n"
                         "vec2 frexp(vec2, out ivec2);\n"
                         "vec3 frexp(vec3, out ivec3);\n"
                         "vec4 frexp(vec4, out ivec4);\n"
                         "vec2 fmod(vec2, float);\n"
                         "vec3 fmod(vec3, float);\n"
                         "vec4 fmod(vec4, float);\n"
                         "float modf(float, out float);\n"
                         "vec2 modf(vec2, out vec2);\n"
                         "vec3 modf(vec3, out vec3);\n"
                         "vec4 modf(vec4, out vec4);\n"
                         "float mul(float, float);\n"
                         "vec2 mul(vec2, float);\n"
                         "vec3 mul(vec3, float);\n"
                         "vec4 mul(vec4, float);\n"
                         "vec2 mul(vec2, mat2);\n"
                         "vec2 mul(vec3, mat2x3);\n"
                         "vec2 mul(vec4, mat2x4);\n"
                         "vec3 mul(vec3, mat3);\n"
                         "vec3 mul(vec2, mat3x2);\n"
                         "vec3 mul(vec4, mat3x4);\n"
                         "vec4 mul(vec4, mat4);\n"
                         "vec4 mul(vec2, mat4x2);\n"
                         "vec4 mul(vec3, mat4x3);\n"
                         "vec2 mul(mat2, vec2);\n"
                         "vec3 mul(mat2x3, vec2);\n"
                         "vec4 mul(mat2x4, vec2);\n"
                         "vec3 mul(mat3, vec3);\n"
                         "vec2 mul(mat3x2, vec3);\n"
                         "vec4 mul(mat3x4, vec3);\n"
                         "vec4 mul(mat4, vec4);\n"
                         "vec2 mul(mat4x2, vec4);\n"
                         "vec3 mul(mat4x3, vec4);\n"
                         "mat2 mul(mat2, mat2);\n"
                         "mat3 mul(mat3, mat3);\n"
                         "mat4 mul(mat4, mat4);\n"
                         "float refract(float, float, float);\n"
                         "void sincos(float, out float, out float);\n"
                         "void sincos(vec2, out vec2, out vec2);\n"
                         "void sincos(vec3, out vec3, out vec3);\n"
                         "void sincos(vec4, out vec4, out vec4);\n"
                         "vec2 smoothstep(vec2, vec2, float);\n"
                         "vec3 smoothstep(vec3, vec3, float);\n"
                         "vec4 smoothstep(vec4, vec4, float);\n"
                         "vec2 step(float, vec2);\n"
                         "vec3 step(float, vec3);\n"
                         "vec4 step(float, vec4);\n"
                         "vec4 tex1D(sampler1D, float);\n"
                         "vec4 tex1Dbias(sampler1D, vec4);\n"
                         "vec4 tex1Dgrad(sampler1D, float, float, float);\n"
                         "vec4 tex1Dlod(sampler1D, vec4);\n"
                         "vec4 tex1Dproj(sampler1D, vec4);\n"
                         "vec4 tex2D(sampler2D, vec2);\n"
                         "vec4 tex2Dbias(sampler2D, vec4);\n"
                         "vec4 tex2Dgrad(sampler2D, vec2, float, float);\n"
                         "vec4 tex2Dlod(sampler2D, vec4);\n"
                         "vec4 tex2Dproj(sampler2D, vec4);\n"
                         "vec4 tex3D(sampler3D, vec3);\n"
                         "vec4 tex3Dbias(sampler3D, vec4);\n"
                         "vec4 tex3Dgrad(sampler3D, vec3, float, float);\n"
                         "vec4 tex3Dlod(sampler3D, vec4);\n"
                         "vec4 tex3Dproj(sampler3D, vec4);\n"
                         "vec4 texCUBE(samplerCube, vec3);\n"
                         "vec4 texCUBEbias(samplerCube, vec4);\n"
                         "vec4 texCUBEgrad(samplerCube, vec3, float, float);\n"
                         "vec4 texCUBElod(samplerCube, vec4);\n"
                         "vec4 texCUBEproj(samplerCube, vec4);\n"
                         "mat2 transpose(mat2);\n"
                         "mat3 transpose(mat3);\n"
                         "mat4 transpose(mat4);\n"
                         "mat2x3 transpose(mat3x2);\n"
                         "mat2x4 transpose(mat4x2);\n"
                         "mat3x2 transpose(mat2x3);\n"
                         "mat3x4 transpose(mat4x3);\n"
                         "mat4x2 transpose(mat2x4);\n"
                         "mat4x3 transpose(mat3x4);\n";
        char buffer[1024];
        for (size_t i = 0; i < sizeof(kGenTypeOneArgumentFunctions) / sizeof(kGenTypeOneArgumentFunctions[0]); i++) {
            const char *name = kGenTypeOneArgumentFunctions[i];
            snprintf(buffer, sizeof(buffer),
                "float %s(float);\n"
                "vec2 %s(vec2);\n"
                "vec3 %s(vec3);\n"
                "vec4 %s(vec4);\n",
                name, name, name, name);
            m_commonString.append(buffer);
        }
        for (size_t i = 0; i < sizeof(kGenTypeTwoArgumentFunctions) / sizeof(kGenTypeTwoArgumentFunctions[0]); i++) {
            const char *name = kGenTypeTwoArgumentFunctions[i];
            snprintf(buffer, sizeof(buffer),
                "float %s(float, float);\n"
                "vec2 %s(vec2, vec2);\n"
                "vec3 %s(vec3, vec3);\n"
                "vec4 %s(vec4, vec4);\n",
                name, name, name, name);
            m_commonString.append(buffer);
        }
        for (size_t i = 0; i < sizeof(kGenTypeThreeArgumentFunctions) / sizeof(kGenTypeThreeArgumentFunctions[0]);
             i++) {
            const char *name = kGenTypeThreeArgumentFunctions[i];
            snprintf(buffer, sizeof(buffer),
                "float %s(float, float, float);\n"
                "vec2 %s(vec2, vec2, vec2);\n"
                "vec3 %s(vec3, vec3, vec3);\n"
                "vec4 %s(vec4, vec4, vec4);\n",
                name, name, name, name);
            m_commonString.append(buffer);
        }
        for (size_t i = 0;
             i < sizeof(kGenTypeTwoArgumentAndScalarFunctions) / sizeof(kGenTypeTwoArgumentAndScalarFunctions[0]);
             i++) {
            const char *name = kGenTypeTwoArgumentAndScalarFunctions[i];
            snprintf(buffer, sizeof(buffer),
                "float %s(float, float, float);\n"
                "vec2 %s(vec2, vec2, float);\n"
                "vec3 %s(vec3, vec3, float);\n"
                "vec4 %s(vec4, vec4, float);\n",
                name, name, name, name);
            m_commonString.append(buffer);
        }
        for (size_t i = 0;
             i < sizeof(kAggregatedGenTypeOneArgumentFunctions) / sizeof(kAggregatedGenTypeOneArgumentFunctions[0]);
             i++) {
            const char *name = kAggregatedGenTypeOneArgumentFunctions[i];
            snprintf(buffer, sizeof(buffer),
                "float %s(float);\n"
                "float %s(vec2);\n"
                "float %s(vec3);\n"
                "float %s(vec4);\n",
                name, name, name, name);
            m_commonString.append(buffer);
        }
        for (size_t i = 0;
             i < sizeof(kAggregatedGenTypeTwoArgumentFunctions) / sizeof(kAggregatedGenTypeTwoArgumentFunctions[0]);
             i++) {
            const char *name = kAggregatedGenTypeTwoArgumentFunctions[i];
            snprintf(buffer, sizeof(buffer),
                "float %s(float, float);\n"
                "float %s(vec2, vec2);\n"
                "float %s(vec3, vec3);\n"
                "float %s(vec4, vec4);\n",
                name, name, name, name);
            m_commonString.append(buffer);
        }
        for (size_t i = 0; i < sizeof(kGenTypeTestFunctions) / sizeof(kGenTypeTestFunctions[0]); i++) {
            const char *name = kGenTypeTestFunctions[i];
            snprintf(buffer, sizeof(buffer),
                "bool %s(float);\n"
                "bvec2 %s(vec2);\n"
                "bvec3 %s(vec3);\n"
                "bvec4 %s(vec4);\n",
                name, name, name, name);
            m_commonString.append(buffer);
        }
        for (size_t i = 0; i < sizeof(kOperandFunctions) / sizeof(kOperandFunctions[0]); i++) {
            const char *name = kOperandFunctions[i];
            snprintf(buffer, sizeof(buffer),
                "bool %s(bool);\n"
                "bool %s(bvec2);\n"
                "bool %s(bvec3);\n"
                "bool %s(bvec4);\n",
                name, name, name, name);
            m_commonString.append(buffer);
        }
    }
    void
    initialize(const TBuiltInResource & /* resources */, int /* version */, EProfile /* profile */,
        const SpvVersion & /* spvVersion */, EShLanguage /* language */) override
    {
    }
    const TString &
    getCommonString() const override
    {
        return m_commonString;
    }
    const TString &getStageString(EShLanguage /* language */) const override
    {
        return m_stageString;
    }
    void
    identifyBuiltIns(int /* version */, EProfile /* profile */, const SpvVersion & /* spvVersion */,
        EShLanguage /* language */, TSymbolTable &symbolTable) override
    {
        symbolTable.relateToOperator("abs", EOpAbs);
        symbolTable.relateToOperator("acos", EOpAcos);
        symbolTable.relateToOperator("all", EOpAll);
        symbolTable.relateToOperator("any", EOpAny);
        symbolTable.relateToOperator("asin", EOpAsin);
        symbolTable.relateToOperator("atan", EOpAtan);
        symbolTable.relateToOperator("atan2", EOpAtan);
        symbolTable.relateToOperator("ceil", EOpCeil);
        symbolTable.relateToOperator("clamp", EOpClamp);
        symbolTable.relateToOperator("clip", EOpClip);
        symbolTable.relateToOperator("cos", EOpCos);
        symbolTable.relateToOperator("cosh", EOpCosh);
        symbolTable.relateToOperator("cross", EOpCross);
        symbolTable.relateToOperator("ddx", EOpDPdx);
        symbolTable.relateToOperator("ddy", EOpDPdy);
        symbolTable.relateToOperator("degrees", EOpDegrees);
        symbolTable.relateToOperator("determinant", EOpDeterminant);
        symbolTable.relateToOperator("distance", EOpDistance);
        symbolTable.relateToOperator("dot", EOpDot);
        symbolTable.relateToOperator("exp", EOpExp);
        symbolTable.relateToOperator("exp2", EOpExp2);
        symbolTable.relateToOperator("faceforward", EOpFaceForward);
        symbolTable.relateToOperator("floor", EOpFloor);
        symbolTable.relateToOperator("fmod", EOpMod);
        symbolTable.relateToOperator("frac", EOpFract);
        symbolTable.relateToOperator("frexp", EOpFrexp);
        symbolTable.relateToOperator("fwidth", EOpFwidth);
        symbolTable.relateToOperator("isfinite", EOpIsFinite);
        symbolTable.relateToOperator("isinf", EOpIsInf);
        symbolTable.relateToOperator("isnan", EOpIsNan);
        symbolTable.relateToOperator("ldexp", EOpLdexp);
        symbolTable.relateToOperator("length", EOpLength);
        symbolTable.relateToOperator("lerp", EOpMix);
        symbolTable.relateToOperator("lit", EOpLit);
        symbolTable.relateToOperator("log", EOpLog);
        symbolTable.relateToOperator("log10", EOpLog10);
        symbolTable.relateToOperator("log2", EOpLog2);
        symbolTable.relateToOperator("max", EOpMax);
        symbolTable.relateToOperator("min", EOpMin);
        symbolTable.relateToOperator("modf", EOpModf);
        symbolTable.relateToOperator("mul", EOpGenMul);
        symbolTable.relateToOperator("noise", EOpNoise);
        symbolTable.relateToOperator("normalize", EOpNormalize);
        symbolTable.relateToOperator("pow", EOpPow);
        symbolTable.relateToOperator("radians", EOpRadians);
        symbolTable.relateToOperator("rcp", EOpRcp);
        symbolTable.relateToOperator("reflect", EOpReflect);
        symbolTable.relateToOperator("refract", EOpRefract);
        symbolTable.relateToOperator("rsqrt", EOpInverseSqrt);
        symbolTable.relateToOperator("saturate", EOpSaturate);
        symbolTable.relateToOperator("sign", EOpSign);
        symbolTable.relateToOperator("sin", EOpSin);
        symbolTable.relateToOperator("sincos", EOpSinCos);
        symbolTable.relateToOperator("sinh", EOpSinh);
        symbolTable.relateToOperator("smoothstep", EOpSmoothStep);
        symbolTable.relateToOperator("sqrt", EOpSqrt);
        symbolTable.relateToOperator("step", EOpStep);
        symbolTable.relateToOperator("tan", EOpTan);
        symbolTable.relateToOperator("tanh", EOpTanh);
        symbolTable.relateToOperator("tex1D", EOpTexture);
        symbolTable.relateToOperator("tex1Dbias", EOpTextureBias);
        symbolTable.relateToOperator("tex1Dgrad", EOpTextureGrad);
        symbolTable.relateToOperator("tex1Dlod", EOpTextureLod);
        symbolTable.relateToOperator("tex1Dproj", EOpTextureProj);
        symbolTable.relateToOperator("tex2D", EOpTexture);
        symbolTable.relateToOperator("tex2Dbias", EOpTextureBias);
        symbolTable.relateToOperator("tex2Dgrad", EOpTextureGrad);
        symbolTable.relateToOperator("tex2Dlod", EOpTextureLod);
        symbolTable.relateToOperator("tex2Dproj", EOpTextureProj);
        symbolTable.relateToOperator("tex3D", EOpTexture);
        symbolTable.relateToOperator("tex3Dbias", EOpTextureBias);
        symbolTable.relateToOperator("tex3Dgrad", EOpTextureGrad);
        symbolTable.relateToOperator("tex3Dlod", EOpTextureLod);
        symbolTable.relateToOperator("tex3Dproj", EOpTextureProj);
        symbolTable.relateToOperator("texCUBE", EOpTexture);
        symbolTable.relateToOperator("texCUBEbias", EOpTextureBias);
        symbolTable.relateToOperator("texCUBEgrad", EOpTextureGrad);
        symbolTable.relateToOperator("texCUBElod", EOpTextureLod);
        symbolTable.relateToOperator("texCUBEproj", EOpTextureProj);
        symbolTable.relateToOperator("transpose", EOpTranspose);
        symbolTable.relateToOperator("trunc", EOpTrunc);
        if (m_language == fx9::Compiler::kLanguageTypeHLSL) {
            symbolTable.relateToOperator("round", EOpRound);
        }
        else {
            symbolTable.relateToOperator("round", EOpRoundEven);
        }
    }
    void
    identifyBuiltIns(int /* version */, EProfile /* profile */, const SpvVersion & /* spvVersion */,
        EShLanguage /* language */, TSymbolTable & /* symbolTable */, const TBuiltInResource & /* resources */) override
    {
    }

private:
    TString m_commonString;
    TString m_stageString;
    fx9::Compiler::LanguageType m_language;
};

} /* namespace anonymous */

namespace fx9 {

Compiler::BaseParameterConverter::BaseParameterConverter(
    const ParserContext &parser, Compiler *parent, EffectProduct &effectProduct, void *opaque)
    : m_parser(parser)
    , m_effectProduct(effectProduct)
    , m_parent(parent)
    , m_opaque(opaque)
{
}

Compiler::BaseParameterConverter::~BaseParameterConverter()
{
}

void
Compiler::BaseParameterConverter::convertAllConstantParameters(void *opaque, size_t &index)
{
    Fx9__Effect__Parameter **parameters = static_cast<Fx9__Effect__Parameter **>(opaque), *parameterPtr = nullptr;
    const auto &constantNodes = m_parser.constantNodes();
    for (auto it = constantNodes.begin(), end = constantNodes.end(); it != end; ++it) {
        const glslang::TString &name = it->first;
        const ParserContext::NodeItem &value = it->second;
        parameterPtr = parameters[index++] = m_parent->allocate<Fx9__Effect__Parameter>();
        convertParameter(*it->second.m_type, name, &value, nullptr, parameterPtr);
    }
}

void
Compiler::BaseParameterConverter::convertAllUniformParameters(void *opaque, size_t &index)
{
    Fx9__Effect__Parameter **parameters = static_cast<Fx9__Effect__Parameter **>(opaque), *parameterPtr = nullptr;
    const auto &uniformNodes = m_parser.uniformNodes();
    for (auto it = uniformNodes.begin(), end = uniformNodes.end(); it != end; ++it) {
        const glslang::TString &name = it->first;
        const ParserContext::NodeItem &value = it->second;
        parameterPtr = parameters[index++] = m_parent->allocate<Fx9__Effect__Parameter>();
        convertParameter(*value.m_type, name, &value, value.m_semanticAnnotationNode, parameterPtr);
    }
}

void
Compiler::BaseParameterConverter::convertAllSamplerParameters(void *opaque, size_t &index)
{
    Fx9__Effect__Parameter **parameters = static_cast<Fx9__Effect__Parameter **>(opaque), *parameterPtr = nullptr;
    const auto &samplerNodes = m_parser.samplerNodes();
    for (auto it = samplerNodes.begin(), end = samplerNodes.end(); it != end; ++it) {
        const glslang::TString &name = it->first;
        const ParserContext::SamplerNodeItem &value = it->second;
        parameterPtr = parameters[index++] = m_parent->allocate<Fx9__Effect__Parameter>();
        convertParameter(*value.m_type, name, &value, value.m_semanticAnnotationNode, parameterPtr);
    }
}

void
Compiler::BaseParameterConverter::convertAllTextureParameters(void *opaque, size_t &index)
{
    Fx9__Effect__Parameter **parameters = static_cast<Fx9__Effect__Parameter **>(opaque), *parameterPtr = nullptr;
    const auto &textureNodes = m_parser.textureNodes();
    for (auto it = textureNodes.begin(), end = textureNodes.end(); it != end; ++it) {
        const glslang::TString &name = it->first;
        const ParserContext::TextureNodeItem &value = it->second;
        parameterPtr = parameters[index++] = m_parent->allocate<Fx9__Effect__Parameter>();
        convertParameter(*value.m_type, name, nullptr, value.m_semanticAnnotationNode, parameterPtr);
    }
}

void
Compiler::BaseParameterConverter::convertAll()
{
    Fx9__Effect__Effect *message = static_cast<Fx9__Effect__Effect *>(m_opaque);
    if (m_parser.hasAnyParameterNodes()) {
        Fx9__Effect__Parameter **parameters = nullptr;
        message->n_parameters = m_parser.countAllParameterNodes();
        message->parameters = parameters = m_parent->allocateArray<Fx9__Effect__Parameter>(message->n_parameters);
        size_t index = 0;
        convertAllConstantParameters(parameters, index);
        convertAllUniformParameters(parameters, index);
        convertAllSamplerParameters(parameters, index);
        convertAllTextureParameters(parameters, index);
    }
}

void
Compiler::BaseParameterConverter::fillParameterValues(const ParserContext::NodeItem *value, void *opaque)
{
    if (value) {
        Fx9__Effect__Parameter *parameterPtr = static_cast<Fx9__Effect__Parameter *>(opaque);
        const TIntermTyped *initializerNode = value->m_initializerNode;
        if (initializerNode) {
            size_t offset = 0;
            if (const TIntermAggregate *aggregateNode = initializerNode->getAsAggregate()) {
                const TIntermSequence sequence = aggregateNode->getSequence();
                /* force recognize as vector even it was scalar */
                parameterPtr->class_common = FX9__EFFECT__PARAMETER__CLASS_COMMON__PC_VECTOR;
                if (value->m_type->isMatrix() &&
                    size_t(value->m_type->getMatrixCols() * value->m_type->getMatrixRows()) == sequence.size()) {
                    parameterPtr->value.len = value->m_type->getMatrixRows() * 16;
                    parameterPtr->value.data =
                        static_cast<uint8_t *>(m_parent->m_allocator->allocate(parameterPtr->value.len));
                }
                else if (value->m_type->isVector() && value->m_type->getVectorSize() == sequence.size()) {
                    size_t numComponents = sequence.size();
                    parameterPtr->value.len = 16;
                    parameterPtr->value.data =
                        static_cast<uint8_t *>(m_parent->m_allocator->allocate(parameterPtr->value.len));
                    for (auto it = sequence.begin(), end = sequence.end(); it != end; ++it) {
                        TIntermNode *node = *it;
                        if (const TIntermConstantUnion *constantUnionNode = node->getAsConstantUnion()) {
                            fillParameterFromConstantUnionNode(parameterPtr, constantUnionNode, 1, offset);
                        }
                        else {
                            reinterpret_cast<int *>(parameterPtr->value.data)[offset] = 0;
                        }
                    }
                    for (size_t i = numComponents; i < 4; i++) {
                        reinterpret_cast<int *>(parameterPtr->value.data)[offset + i - numComponents] = 0;
                    }
                }
                else {
                    parameterPtr->value.len = 16 * sequence.size();
                    parameterPtr->value.data =
                        static_cast<uint8_t *>(m_parent->m_allocator->allocate(parameterPtr->value.len));
                    for (auto it = sequence.begin(), end = sequence.end(); it != end; ++it) {
                        TIntermNode *node = *it;
                        if (const TIntermAggregate *aggregateNode = node->getAsAggregate()) {
                            const TIntermSequence &sequence2 = aggregateNode->getSequence();
                            size_t numComponents = sequence2.size();
                            for (auto it2 = sequence2.begin(), end2 = sequence2.end(); it2 != end2; ++it2) {
                                TIntermNode *node2 = *it2;
                                if (const TIntermConstantUnion *constantUnionNode = node2->getAsConstantUnion()) {
                                    fillParameterFromConstantUnionNode(parameterPtr, constantUnionNode, 1, offset);
                                }
                            }
                            if (numComponents < 4) {
                                for (size_t i = numComponents; i < 4; i++) {
                                    reinterpret_cast<int *>(parameterPtr->value.data)[offset + i - numComponents] = 0;
                                }
                                offset += (4 - numComponents);
                            }
                        }
                        else if (const TIntermConstantUnion *constantUnionNode = node->getAsConstantUnion()) {
                            fillParameterFromConstantUnionNode(parameterPtr, constantUnionNode, 4, offset);
                        }
                    }
                }
            }
            else if (const TIntermConstantUnion *constantUnionNode = initializerNode->getAsConstantUnion()) {
                const TConstUnionArray &values = constantUnionNode->getConstArray();
                parameterPtr->value.len = 16;
                parameterPtr->value.data =
                    static_cast<uint8_t *>(m_parent->m_allocator->allocate(parameterPtr->value.len));
                fillParameterFromConstantUnionNode(parameterPtr, constantUnionNode, 4, offset);
            }
        }
    }
}

void
Compiler::BaseParameterConverter::fillParameterFromConstantUnionNode(
    void *opaque, const TIntermConstantUnion *constantUnionNode, int maxComponents, size_t &offset)
{
    const TConstUnionArray &values = constantUnionNode->getConstArray();
    Fx9__Effect__Parameter *parameterPtr = static_cast<Fx9__Effect__Parameter *>(opaque);
    int numComponents = std::min(values.size(), maxComponents);
    switch (constantUnionNode->getBasicType()) {
    case EbtBool: {
        for (int i = 0; i < numComponents; i++) {
            const int v = values[i].getBConst() ? 1 : 0;
            assignIntegerParameter(parameterPtr, offset + i, v);
        }
        break;
    }
    case EbtFloat: {
        for (int i = 0; i < numComponents; i++) {
            const float v = float(values[i].getDConst());
            assignFloatParameter(parameterPtr, offset + i, v);
        }
        break;
    }
    case EbtInt: {
        for (int i = 0; i < numComponents; i++) {
            const int v = values[i].getIConst();
            assignIntegerParameter(parameterPtr, offset + i, v);
        }
        break;
    }
    default:
        break;
    }
    for (int i = numComponents; i < maxComponents; i++) {
        reinterpret_cast<int *>(parameterPtr->value.data)[offset + i] = 0;
    }
    offset += maxComponents;
}

void
Compiler::BaseParameterConverter::assignIntegerParameter(void *opaque, size_t offset, int value)
{
    Fx9__Effect__Parameter *parameterPtr = static_cast<Fx9__Effect__Parameter *>(opaque);
    auto typed = parameterPtr->type_common;
    switch (typed) {
    case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_BOOL:
    case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_INT:
        reinterpret_cast<int *>(parameterPtr->value.data)[offset] = value;
        break;
    case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_FLOAT:
        reinterpret_cast<float *>(parameterPtr->value.data)[offset] = float(value);
        break;
    default:
        break;
    }
}

void
Compiler::BaseParameterConverter::assignFloatParameter(void *opaque, size_t offset, float value)
{
    Fx9__Effect__Parameter *parameterPtr = static_cast<Fx9__Effect__Parameter *>(opaque);
    auto typed = parameterPtr->type_common;
    switch (typed) {
    case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_BOOL:
    case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_INT:
        reinterpret_cast<int *>(parameterPtr->value.data)[offset] = int(value);
        break;
    case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_FLOAT:
        reinterpret_cast<float *>(parameterPtr->value.data)[offset] = value;
        break;
    default:
        break;
    }
}

void
Compiler::BaseParameterConverter::convertParameterType(const glslang::TType &type, void *opaque)
{
    Fx9__Effect__Parameter *parameterPtr = static_cast<Fx9__Effect__Parameter *>(opaque);
    auto setTypeCommon = [parameterPtr](Fx9__Effect__Parameter__TypeCommon value) {
        parameterPtr->type_common = value;
        parameterPtr->has_type_common = 1;
    };
    switch (type.getBasicType()) {
    case EbtBool:
        setTypeCommon(FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_BOOL);
        break;
    case EbtFloat:
        setTypeCommon(FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_FLOAT);
        break;
    case EbtInt:
        setTypeCommon(FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_INT);
        break;
    default:
        setTypeCommon(FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_VOID);
        break;
    }
}

void
Compiler::BaseParameterConverter::convertParameter(const glslang::TType &type, const glslang::TString &name,
    const ParserContext::NodeItem *value, const glslang::TIntermBinary *semanticAnnotationNode, void *opaque)
{
    Fx9__Effect__Parameter *parameterPtr = static_cast<Fx9__Effect__Parameter *>(opaque);
    fx9__effect__parameter__init(parameterPtr);
    m_parent->copyString(name, &parameterPtr->name);
    parameterPtr->num_columns = std::max(type.getMatrixCols(), 1);
    parameterPtr->num_rows = std::max(type.getMatrixRows(), 1);
    auto setClassCommon = [parameterPtr](Fx9__Effect__Parameter__ClassCommon value) {
        parameterPtr->class_common = value;
        parameterPtr->has_class_common = 1;
    };
    if (const TArraySizes *sizes = type.getArraySizes()) {
        parameterPtr->num_elements = sizes->getOuterSize();
    }
    if (type.getQualifier().storage == EvqShared) {
        // D3DX_PARAMETER_SHARED
        parameterPtr->flags |= (1 << 0);
    }
    if (type.isOpaque()) {
        auto setTypeCommon = [parameterPtr](Fx9__Effect__Parameter__TypeCommon value) {
            parameterPtr->type_common = value;
            parameterPtr->has_type_common = 1;
        };
        setClassCommon(FX9__EFFECT__PARAMETER__CLASS_COMMON__PC_OBJECT);
        const TSampler &sampler = type.getSampler();
        if (sampler.isCombined()) {
            switch (sampler.dim) {
            case EsdNone:
                setTypeCommon(FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_SAMPLER);
                break;
            case Esd1D:
                setTypeCommon(FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_SAMPLER1D);
                break;
            case Esd2D:
                setTypeCommon(FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_SAMPLER2D);
                break;
            case Esd3D:
                setTypeCommon(FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_SAMPLER3D);
                break;
            case EsdCube:
                setTypeCommon(FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_SAMPLERCUBE);
                break;
            default:
                break;
            }
        }
        else {
            switch (sampler.dim) {
            case EsdNone:
                setTypeCommon(FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_TEXTURE);
                break;
            case Esd1D:
                setTypeCommon(FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_TEXTURE1D);
                break;
            case Esd2D:
                setTypeCommon(FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_TEXTURE2D);
                break;
            case Esd3D:
                setTypeCommon(FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_TEXTURE3D);
                break;
            case EsdCube:
                setTypeCommon(FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_TEXTURECUBE);
                break;
            default:
                break;
            }
        }
    }
    else if (type.isStruct()) {
        Fx9__Effect__Parameter **members = nullptr, *memberPtr;
        setClassCommon(FX9__EFFECT__PARAMETER__CLASS_COMMON__PC_STRUCT);
        const TTypeList *fields = type.getStruct();
        parameterPtr->n_struct_members = fields->size();
        parameterPtr->struct_members = members =
            m_parent->allocateArray<Fx9__Effect__Parameter>(parameterPtr->n_struct_members);
        size_t index = 0;
        for (auto it = fields->begin(), end = fields->end(); it != end; ++it) {
            memberPtr = members[index++] = m_parent->allocate<Fx9__Effect__Parameter>();
            convertParameter(*it->type, it->type->getFieldName(), nullptr, nullptr, memberPtr);
        }
    }
    else if (type.isMatrix()) {
        setClassCommon(FX9__EFFECT__PARAMETER__CLASS_COMMON__PC_MATRIX_ROWS);
        convertParameterType(type, parameterPtr);
        fillParameterValues(value, parameterPtr);
    }
    else if (type.isVector()) {
        setClassCommon(FX9__EFFECT__PARAMETER__CLASS_COMMON__PC_VECTOR);
        convertParameterType(type, parameterPtr);
        fillParameterValues(value, parameterPtr);
    }
    else if (type.isScalar() || type.isArray()) {
        setClassCommon(FX9__EFFECT__PARAMETER__CLASS_COMMON__PC_SCALAR);
        convertParameterType(type, parameterPtr);
        fillParameterValues(value, parameterPtr);
    }
    if (semanticAnnotationNode) {
        if (const TIntermTyped *semanticNode = semanticAnnotationNode->getLeft()) {
            m_parent->copyString(semanticNode->getAsSymbolNode()->getName(), &parameterPtr->semantic);
        }
        if (const TIntermTyped *annotationNode = semanticAnnotationNode->getRight()) {
            m_parent->convertAllParameterAnnotations(annotationNode, parameterPtr);
        }
    }
}

Compiler::BasePassShader::BasePassShader(
    Compiler *parent, const char *path, const glslang::TString &source, EffectProduct &effectProduct, void *opaque)
    : m_source(source)
    , m_path(path)
    , m_effectProduct(effectProduct)
    , m_parent(parent)
    , m_opaque(opaque)
{
}

Compiler::BasePassShader::~BasePassShader()
{
}

void
Compiler::BasePassShader::convertPassState(
    const TString &name, const ParserContext::PassState &state, uint32_t &key, uint32_t &value) const
{
    TString nameUpperCase(name);
    std::transform(name.begin(), name.end(), nameUpperCase.begin(), ::toupper);
    auto it = m_parent->m_renderStateEnumConversions.find(nameUpperCase.c_str());
    key = value = 0;
    if (it != m_parent->m_renderStateEnumConversions.end()) {
        switch (it->second) {
        case 7: {
            value = resolveRenderStateValue(state, m_parent->m_renderStateZBufferTypeValueConversions);
            break;
        }
        case 8: {
            value = resolveRenderStateValue(state, m_parent->m_renderStateFillModeValueConversions);
            break;
        }
        case 9: {
            value = resolveRenderStateValue(state, m_parent->m_renderStateShadeModeValueConversions);
            break;
        }
        case 19:
        case 20:
        case 207:
        case 208: {
            value = resolveRenderStateValue(state, m_parent->m_renderStateBlendModeValueConversions);
            break;
        }
        case 22: {
            value = resolveRenderStateValue(state, m_parent->m_renderStateCullValueConversions);
            break;
        }
        case 23:
        case 25:
        case 56:
        case 189: {
            value = resolveRenderStateValue(state, m_parent->m_renderStateCmpFuncValueConversions);
            break;
        }
        case 35: {
            value = resolveRenderStateValue(state, m_parent->m_renderStateFogModeValueConversions);
            break;
        }
        case 53:
        case 54:
        case 55:
        case 186:
        case 187:
        case 188: {
            value = resolveRenderStateValue(state, m_parent->m_renderStateStencilOpValueConversions);
            break;
        }
        case 171: {
            value = resolveRenderStateValue(state, m_parent->m_renderStateBlendOpValueConversions);
            break;
        }
        default:
            const TIntermNode *valueNode = state.m_value;
            if (const TIntermSymbol *symbolNode = valueNode->getAsSymbolNode()) {
                const TString &name = symbolNode->getName();
                value = name == "TRUE" ? 1 : 0;
            }
            else if (const TIntermConstantUnion *constantUnionNode = valueNode->getAsConstantUnion()) {
                const TConstUnionArray &values = constantUnionNode->getConstArray();
                switch (values[0].getType()) {
                case EbtBool: {
                    value = values[0].getBConst() ? 1 : 0;
                    break;
                }
                case EbtFloat:
                case EbtFloat16:
                case EbtDouble: {
                    value = uint32_t(values[0].getDConst());
                    break;
                }
                case EbtInt: {
                    value = uint32_t(values[0].getIConst());
                    break;
                }
                default:
                    break;
                }
            }
            break;
        }
        key = it->second;
    }
}

void
Compiler::BasePassShader::convertAllSamplerStates(const TIntermAggregate *samplerStates, void *opaque) const
{
    Fx9__Effect__Sampler *message = static_cast<Fx9__Effect__Sampler *>(opaque);
    const TIntermSequence sequence = samplerStates ? samplerStates->getSequence() : TIntermSequence();
    if (!sequence.empty()) {
        message->n_sampler_states = sequence.size();
        message->sampler_states = m_parent->allocateArray<Fx9__Effect__SamplerState>(message->n_sampler_states);
        size_t index = 0;
        for (auto it2 = sequence.begin(), end2 = sequence.end(); it2 != end2; ++it2) {
            if (TIntermBinary *binaryNode = (*it2)->getAsBinaryNode()) {
                const TIntermSymbol *keyNode = binaryNode->getLeft()->getAsSymbolNode();
                const TString &key = keyNode->getName();
                if (const TIntermTyped *valueNode = binaryNode->getRight()->getAsSymbolNode()) {
                    TString canonicalizedName(keyNode->getName());
                    std::transform(key.begin(), key.end(), canonicalizedName.begin(), ::toupper);
                    auto it = m_parent->m_samplerStateEnumConversions.find(canonicalizedName.c_str());
                    if (it != m_parent->m_samplerStateEnumConversions.end()) {
                        uint32_t value = 0;
                        switch (it->second) {
                        case 1:
                        case 2:
                        case 3: {
                            value = resolveSamplerStateValue(
                                valueNode->getAsSymbolNode(), m_parent->m_samplerStateTextureAddressValueConversions);
                            break;
                        }
                        case 5:
                        case 6:
                        case 7: {
                            value = resolveSamplerStateValue(valueNode->getAsSymbolNode(),
                                m_parent->m_samplerStateTextureFilterTypeValueConversions);
                            break;
                        }
                        default:
                            if (const TIntermConstantUnion *constantUnionNode = valueNode->getAsConstantUnion()) {
                                const TConstUnionArray &values = constantUnionNode->getConstArray();
                                value = uint32_t(values[0].getIConst());
                            }
                            break;
                        }
                        Fx9__Effect__SamplerState *samplerStatePtr;
                        samplerStatePtr = message->sampler_states[index++] =
                            m_parent->allocate<Fx9__Effect__SamplerState>();
                        fx9__effect__sampler_state__init(samplerStatePtr);
                        samplerStatePtr->key = it->second;
                        samplerStatePtr->value = value;
                    }
                }
            }
        }
        message->n_sampler_states = index;
    }
}

bool
Compiler::BasePassShader::generateSPVInstructions(ParserContext &parser,
    const ParserContext::Pass::EntryPoint &entryPoint, InstructionList &instructions, TInfoSink &infoSink)
{
    for (auto it = m_parent->m_includeSourceData.begin(), end = m_parent->m_includeSourceData.end(); it != end; ++it) {
        parser.addIncludeSource(it->first.c_str(), it->second.c_str());
    }
    spv::SpvBuildLogger logger;
    bool succeeded = m_parent->generateSPVInstructions(entryPoint, m_source, parser, instructions, &logger, infoSink);
    if (!succeeded) {
        m_effectProduct.sink.builder = logger.getAllMessages();
        m_effectProduct.sink.merge(entryPoint.first, infoSink);
    }
    return succeeded;
}

bool
Compiler::BasePassShader::compile(
    EShLanguage language, const ParserContext::Pass::EntryPoint &entryPoint, InstructionList &instructions)
{
    bool succeeded = false;
    Fx9__Effect__Pass *message = static_cast<Fx9__Effect__Pass *>(m_opaque);
    Fx9__Effect__Shader *shader = nullptr;
    std::string translatedShaderSource;
    TInfoSink infoSink;
    TIntermediate intermediate(language, m_parent->m_version, m_parent->m_profile);
    const SpvVersion &spv = createSpvVersion();
    intermediate.setSpv(spv);
    intermediate.setSource(EShSourceHlsl);
    std::unique_ptr<TSymbolTable> symbolTable(new TSymbolTable());
    TSymbolTable *symbolTablePtr = symbolTable.get(), &symbolTableRef = *symbolTablePtr;
    symbolTableRef.adoptLevels(*m_parent->m_fragmentSymbolTable);
    {
        HlslParseContext parseContext(symbolTableRef, intermediate, false, m_parent->m_version, m_parent->m_profile,
            spv, language, infoSink, TString(), false, m_parent->m_messages);
        ParserContext parser(&parseContext, m_path, m_parent->m_vertexShaderInputVariables,
            m_parent->m_pixelShaderInputVariables, m_writtenVertexShaderInputVariables);
        parser.intermediate().setSpv(spv);
        configureParserContext(parser);
        succeeded = generateSPVInstructions(parser, entryPoint, instructions, infoSink);
        const TString entryPointName(entryPoint.first.c_str(), entryPoint.first.size());
        if (language == EShLangVertex) {
            shader = message->vertex_shader = m_parent->allocate<Fx9__Effect__Shader>();
            fx9__effect__shader__init(shader);
            shader->type = FX9__EFFECT__SHADER__TYPE__ST_VERTEX;
            m_writtenVertexShaderInputVariables = parser.writtenVertexShaderInputVariables();
        }
        else if (language == EShLangFragment) {
            shader = message->pixel_shader = m_parent->allocate<Fx9__Effect__Shader>();
            fx9__effect__shader__init(shader);
            shader->type = FX9__EFFECT__SHADER__TYPE__ST_PIXEL;
        }
        if (shader) {
            // copyString(translatedShaderSource.c_str(), *allocator, &shader->code);
            const auto &samplerNodes = parser.findSamplerNodes(entryPoint);
            const auto &uniformNodes = parser.uniformNodes();
            if (!uniformNodes.empty() || !samplerNodes.empty()) {
                Fx9__Effect__Symbol **symbols = nullptr, *symbolPtr = nullptr;
                shader->n_symbols = uniformNodes.size() + samplerNodes.size();
                shader->symbols = symbols = m_parent->allocateArray<Fx9__Effect__Symbol>(shader->n_symbols);
                size_t index = 0;
                for (auto it = uniformNodes.begin(), end = uniformNodes.end(); it != end; ++it) {
                    const ParserContext::NodeItem &node = it->second;
                    symbolPtr = symbols[index++] = m_parent->allocate<Fx9__Effect__Symbol>();
                    fx9__effect__symbol__init(symbolPtr);
                    m_parent->copyString(it->first, &symbolPtr->name);
                    const TType *type = node.m_type;
                    int numElements = 1;
                    if (const TArraySizes *sizes = type->getArraySizes()) {
                        numElements = sizes->getOuterSize();
                    }
                    symbolPtr->register_count = (type->isMatrix() ? type->getMatrixRows() : 1) * numElements;
                    symbolPtr->register_index = uint32_t(node.m_index);
                    symbolPtr->register_set = FX9__EFFECT__SYMBOL__REGISTER_SET__RS_FLOAT4;
                }
                for (auto it = samplerNodes.begin(), end = samplerNodes.end(); it != end; ++it) {
                    const TString &samplerName = it->first;
                    const ParserContext::SamplerNodeItem &node = it->second;
                    symbolPtr = symbols[index++] = m_parent->allocate<Fx9__Effect__Symbol>();
                    fx9__effect__symbol__init(symbolPtr);
                    m_parent->copyString(samplerName, &symbolPtr->name);
                    symbolPtr->register_count = 1;
                    symbolPtr->register_index = uint32_t(node.m_samplerIndex);
                    symbolPtr->register_set = FX9__EFFECT__SYMBOL__REGISTER_SET__RS_SAMPLER;
                }
                shader->n_symbols = index;
            }
            if (!uniformNodes.empty()) {
                Fx9__Effect__Uniform **uniforms = nullptr, *uniformPtr = nullptr;
                shader->n_uniforms = uniformNodes.size();
                shader->uniforms = uniforms = m_parent->allocateArray<Fx9__Effect__Uniform>(shader->n_uniforms);
                size_t index = 0;
                for (auto it = uniformNodes.begin(), end = uniformNodes.end(); it != end; ++it) {
                    const ParserContext::NodeItem &node = it->second;
                    uniformPtr = uniforms[index++] = m_parent->allocate<Fx9__Effect__Uniform>();
                    fx9__effect__uniform__init(uniformPtr);
                    m_parent->copyString(it->first, &uniformPtr->name);
                    uniformPtr->index = uint32_t(node.m_index);
                    uniformPtr->type = FX9__EFFECT__UNIFORM__TYPE__UT_FLOAT;
                }
                shader->n_uniforms = index;
            }
            auto &samplerName2Index = m_samplerName2Index[language];
            if (!samplerNodes.empty()) {
                Fx9__Effect__Sampler **samplers = nullptr, *samplerPtr = nullptr;
                shader->n_samplers = samplerNodes.size();
                shader->samplers = samplers = m_parent->allocateArray<Fx9__Effect__Sampler>(shader->n_samplers);
                size_t index = 0;
                for (auto it = samplerNodes.begin(), end = samplerNodes.end(); it != end; ++it) {
                    const ParserContext::SamplerNodeItem &node = it->second;
                    samplerPtr = samplers[index++] = m_parent->allocate<Fx9__Effect__Sampler>();
                    fx9__effect__sampler__init(samplerPtr);
                    convertAllSamplerStates(node.m_samplerStatesNode, samplerPtr);
                    m_parent->copyString(node.m_variableName, &samplerPtr->sampler_name);
                    if (const ParserContext::TextureNodeItem *textureNode = node.m_textureNode) {
                        m_parent->copyString(textureNode->m_name, &samplerPtr->texture_name);
                    }
                    samplerPtr->index = node.m_samplerIndex;
                    samplerName2Index.insert(std::make_pair(samplerPtr->sampler_name, node.m_samplerIndex));
                    switch (node.m_type->getSampler().dim) {
                    case Esd2D:
                        samplerPtr->type = FX9__EFFECT__SAMPLER__TYPE__SAMPLER_2D;
                        break;
                    case Esd3D:
                        samplerPtr->type = FX9__EFFECT__SAMPLER__TYPE__SAMPLER_VOLUME;
                        break;
                    case EsdCube:
                        samplerPtr->type = FX9__EFFECT__SAMPLER__TYPE__SAMPLER_CUBE;
                        break;
                    default:
                        break;
                    }
                }
                shader->n_samplers = index;
            }
            const auto &textureNodes = parser.textureNodes();
            if (!textureNodes.empty()) {
                Fx9__Effect__Texture **textures = nullptr, *texturePtr = nullptr;
                shader->n_textures = textureNodes.size();
                shader->textures = textures = m_parent->allocateArray<Fx9__Effect__Texture>(shader->n_textures);
                size_t index = 0;
                for (auto it = textureNodes.begin(), end = textureNodes.end(); it != end; ++it) {
                    texturePtr = textures[index] = m_parent->allocate<Fx9__Effect__Texture>();
                    fx9__effect__texture__init(texturePtr);
                    m_parent->copyString(it->first, &texturePtr->name);
                    texturePtr->index = index++;
                }
            }
            if (succeeded) {
#if defined(FX9_DUMP) && FX9_DUMP
                std::string filename(entryPoint.first.c_str());
                if (language == EShLangVertex) {
                    filename.append(".vert");
                }
                else if (language == EShLangFragment) {
                    filename.append(".frag");
                }
                if (FILE *fp = fopen(filename.c_str(), "wb")) {
                    fprintf(fp, "%s\n", translatedShaderSource.c_str());
                    fclose(fp);
                }
#endif
            }
        }
    }
    return succeeded;
}

void
Compiler::BasePassShader::convertAllPassStates(const ParserContext::Pass &pass)
{
    Fx9__Effect__Pass *message = static_cast<Fx9__Effect__Pass *>(m_opaque);
    auto states = pass.m_states;
    size_t index = 0;
    if (!states.empty()) {
        message->n_render_states = states.size();
        message->render_states = m_parent->allocateArray<Fx9__Effect__RenderState>(message->n_render_states);
        for (auto state = states.begin(), end = states.end(); state != end; ++state) {
            Fx9__Effect__RenderState *renderStatePtr = nullptr;
            renderStatePtr = message->render_states[index++] = m_parent->allocate<Fx9__Effect__RenderState>();
            fx9__effect__render_state__init(renderStatePtr);
            const TString &name = state->m_name;
            convertPassState(name, *state, renderStatePtr->key, renderStatePtr->value);
        }
    }
}

Compiler::DX9MSPassShader::DX9MSPassShader(
    Compiler *parent, const char *path, const glslang::TString &source, EffectProduct &effectProduct, void *opaque)
    : BasePassShader(parent, path, source, effectProduct, opaque)
{
}

Compiler::DX9MSPassShader::~DX9MSPassShader()
{
}

void
Compiler::DX9MSPassShader::configureParserContext(ParserContext & /* parser */)
{
}

bool
Compiler::DX9MSPassShader::translate(const InstructionList &vertexShaderInstructions,
    const InstructionList &fragmentShaderInstructions, std::string &translatedVertexShaderSource,
    std::string &translatedFragmentShaderSource, EffectProduct::LogSink &sink)
{
    bool succeeded = false;
    if (!vertexShaderInstructions.empty() && !fragmentShaderInstructions.empty()) {
#if defined(FX9_DUMP) && defined(FX9_DISASSEMBLE) && FX9_DUMP && FX9_DISASSEMBLE
        spv::Disassemble(std::cout, instructions);
#endif
        try {
            spirv_cross::CompilerGLSL::Options options;
            options.enable_420pack_extension = false;
            options.es = m_parent->m_profile == EEsProfile;
            options.version = m_parent->m_version;
            options.vertex.fixup_clipspace = true;
            options.fragment.default_float_precision = spirv_cross::CompilerGLSL::Options::Highp;
            options.fragment.default_int_precision = spirv_cross::CompilerGLSL::Options::Highp;
            auto compile = [this, &options, &sink](EShLanguage language, const InstructionList &instructions) {
                InstructionList newInstructions;
                std::unordered_map<uint32_t, std::string> attributes;
                m_parent->saveAttributeMap(instructions, attributes);
                m_parent->optimizeShaderInstructions(instructions, newInstructions, sink);
                spirv_cross::CompilerGLSL compiler(newInstructions);
                compiler.set_common_options(options);
                m_parent->restoreInterfaceVariableNames(language, attributes, compiler);
                return compiler.compile();
            };
            translatedVertexShaderSource = compile(EShLangVertex, vertexShaderInstructions);
            translatedFragmentShaderSource = compile(EShLangFragment, fragmentShaderInstructions);
            Fx9__Effect__Pass *message = static_cast<Fx9__Effect__Pass *>(m_opaque);
            message->vertex_shader->body_case = message->pixel_shader->body_case = FX9__EFFECT__SHADER__BODY_GLSL;
            m_parent->copyString(translatedVertexShaderSource.c_str(), &message->vertex_shader->glsl);
            m_parent->copyString(translatedFragmentShaderSource.c_str(), &message->pixel_shader->glsl);
            succeeded = true;
        } catch (const std::exception &e) {
            sink.translator += e.what();
        }
    }
    return succeeded;
}

Compiler::HLSLPassShader::HLSLPassShader(
    Compiler *parent, const char *path, const glslang::TString &source, EffectProduct &effectProduct, void *opaque)
    : BasePassShader(parent, path, source, effectProduct, opaque)
{
}

Compiler::HLSLPassShader::~HLSLPassShader()
{
}

void
Compiler::HLSLPassShader::configureParserContext(ParserContext &parser)
{
    ParserContext::BuiltInLocationMap value;
    for (auto it : m_parent->m_vertexShaderInputLocations) {
        value.insert(std::make_pair(it.first, 0x7ff - it.second));
    }
    parser.setVertexShaderInputMap(value);
}

bool
Compiler::HLSLPassShader::translate(const InstructionList &vertexShaderInstructions,
    const InstructionList &fragmentShaderInstructions, std::string &translatedVertexShaderSource,
    std::string &translatedFragmentShaderSource, EffectProduct::LogSink &sink)
{
    bool succeeded = false;
    if (!vertexShaderInstructions.empty() && !fragmentShaderInstructions.empty()) {
#if defined(FX9_DUMP) && defined(FX9_DISASSEMBLE) && FX9_DUMP && FX9_DISASSEMBLE
        spv::Disassemble(std::cout, instructions);
#endif
        try {
            spirv_cross::CompilerHLSL::Options options;
            options.shader_model = 41;
            auto compile = [this, &options, &sink](EShLanguage language, const InstructionList &instructions,
                               const std::vector<spirv_cross::HLSLVertexAttributeRemap> &mapping) {
                InstructionList newInstructions;
                m_parent->optimizeShaderInstructions(instructions, newInstructions, sink);
                spirv_cross::CompilerHLSL compiler(newInstructions);
                compiler.set_hlsl_options(options);
                auto &samplerName2Index = m_samplerName2Index[language];
                spirv_cross::ShaderResources resources = compiler.get_shader_resources();
                for (const auto &it : resources.sampled_images) {
                    auto it2 = samplerName2Index.find(it.name);
                    if (it2 != samplerName2Index.end()) {
                        compiler.set_decoration(it.id, spv::DecorationBinding, it2->second);
                    }
                }
                for (const auto &it : mapping) {
                    compiler.add_vertex_attribute_remap(it);
                }
                std::ostringstream stream;
                stream << compiler.compile();
                return stream.str();
            };
            std::vector<spirv_cross::HLSLVertexAttributeRemap> mapping;
            uint32_t offset = 0x7ff;
            mapping.push_back(spirv_cross::HLSLVertexAttributeRemap { offset--, "SV_Position" });
            mapping.push_back(spirv_cross::HLSLVertexAttributeRemap { offset--, "NORMAL" });
            mapping.push_back(spirv_cross::HLSLVertexAttributeRemap { offset--, "TEXCOORD0" });
            mapping.push_back(spirv_cross::HLSLVertexAttributeRemap { offset--, "TEXCOORD1" });
            mapping.push_back(spirv_cross::HLSLVertexAttributeRemap { offset--, "TEXCOORD2" });
            mapping.push_back(spirv_cross::HLSLVertexAttributeRemap { offset--, "TEXCOORD3" });
            mapping.push_back(spirv_cross::HLSLVertexAttributeRemap { offset--, "TEXCOORD4" });
            mapping.push_back(spirv_cross::HLSLVertexAttributeRemap { offset--, "COLOR0" });
            translatedVertexShaderSource = compile(EShLangVertex, vertexShaderInstructions, mapping);
            mapping.clear();
            std::ostringstream s;
            for (uint32_t i = 0; i < 16; i++) {
                s << "TEXCOORD";
                s << i;
                mapping.push_back(spirv_cross::HLSLVertexAttributeRemap { i, s.str() });
                s.str(std::string());
            }
            translatedFragmentShaderSource = compile(EShLangFragment, fragmentShaderInstructions, mapping);
            Fx9__Effect__Pass *message = static_cast<Fx9__Effect__Pass *>(m_opaque);
            message->vertex_shader->body_case = message->pixel_shader->body_case = FX9__EFFECT__SHADER__BODY_HLSL;
            m_parent->copyString(translatedVertexShaderSource.c_str(), &message->vertex_shader->hlsl);
            m_parent->copyString(translatedFragmentShaderSource.c_str(), &message->pixel_shader->hlsl);
            succeeded = true;
        } catch (const std::exception &e) {
            sink.translator += e.what();
        }
    }
    return succeeded;
}

Compiler::MSLPassShader::MSLPassShader(
    Compiler *parent, const char *path, const glslang::TString &source, EffectProduct &effectProduct, void *opaque)
    : BasePassShader(parent, path, source, effectProduct, opaque)
{
}

Compiler::MSLPassShader::~MSLPassShader()
{
}

void
Compiler::MSLPassShader::configureParserContext(ParserContext &parser)
{
    Fx9__Effect__Pass *message = static_cast<Fx9__Effect__Pass *>(m_opaque);
    float pointSize = 0.0f;
    for (size_t i = 0, numStates = message->n_render_states; i < numStates; i++) {
        const Fx9__Effect__RenderState *state = message->render_states[i];
        /* FILLMODE == POINT */
        if (state->key == 8 && state->value == 1) {
            pointSize = std::max(pointSize, 1.0f);
        }
        /* POINTSIZE */
        else if (state->key == 154) {
            pointSize = float(state->value);
        }
    }
    parser.setPointSizeAssignment(pointSize);
    parser.enableUniformBuffer();
    parser.setVertexShaderInputMap(m_parent->m_vertexShaderInputLocations);
}

bool
Compiler::MSLPassShader::translate(const InstructionList &vertexShaderInstructions,
    const InstructionList &fragmentShaderInstructions, std::string &translatedVertexShaderSource,
    std::string &translatedFragmentShaderSource, EffectProduct::LogSink &sink)
{
    bool succeeded = false;
    if (!vertexShaderInstructions.empty() && !fragmentShaderInstructions.empty()) {
#if defined(FX9_DUMP) && defined(FX9_DISASSEMBLE) && FX9_DUMP && FX9_DISASSEMBLE
        spv::Disassemble(std::cout, instructions);
#endif
        try {
            std::unordered_map<std::string, int> builtInVariableMapper;
            int index = 0;
            const auto builtInVariables = { EbvVertex, EbvVertexIndex, EbvNormal, EbvMultiTexCoord0, EbvMultiTexCoord1,
                EbvMultiTexCoord2, EbvMultiTexCoord3, EbvMultiTexCoord4, EbvMultiTexCoord5, EbvMultiTexCoord6,
                EbvMultiTexCoord7, EbvFragDepthGreater, EbvFragDepthLesser, EbvFrontColor, EbvBackColor,
                EbvFrontSecondaryColor, EbvBackSecondaryColor };
            for (const auto variable : builtInVariables) {
                auto it = m_parent->m_pixelShaderInputVariables.find(variable);
                if (it != m_parent->m_pixelShaderInputVariables.end()) {
                    builtInVariableMapper.insert(std::make_pair(it->second, index++));
                }
            }
            auto compile = [this, &sink, &builtInVariableMapper](
                               EShLanguage language, const InstructionList &instructions) {
                std::unordered_map<uint32_t, std::string> attributes;
                InstructionList newInstructions;
                m_parent->saveAttributeMap(instructions, attributes);
                m_parent->optimizeShaderInstructions(instructions, newInstructions, sink);
                spirv_cross::CompilerMSL::Options options;
                spirv_cross::CompilerMSL compiler(newInstructions);
                const spv::ExecutionModel stage =
                    language == EShLangVertex ? spv::ExecutionModelVertex : spv::ExecutionModelFragment;
                const auto &entryPoints = compiler.get_entry_points_and_stages();
                auto it = std::find_if(entryPoints.begin(), entryPoints.end(),
                    [&](const spirv_cross::EntryPoint &item) { return item.execution_model == stage; });
                compiler.rename_entry_point(it->name, m_parent->m_metalShaderEntryPointName, stage);
                compiler.set_msl_options(options);
                auto &samplerName2Index = m_samplerName2Index[language];
                spirv_cross::ShaderResources resources = compiler.get_shader_resources();
                for (const auto &it : resources.sampled_images) {
                    auto it2 = samplerName2Index.find(it.name);
                    if (it2 != samplerName2Index.end()) {
                        spirv_cross::MSLResourceBinding binding = {};
                        binding.stage = stage;
                        binding.binding = it2->second;
                        binding.msl_sampler = binding.msl_texture = it2->second;
                        compiler.add_msl_resource_binding(binding);
                        compiler.set_decoration(it.id, spv::DecorationBinding, it2->second);
                    }
                }
                const spirv_cross::SmallVector<spirv_cross::Resource> *stageResources = nullptr;
                if (language == EShLangVertex) {
                    stageResources = &resources.stage_outputs;
                }
                else if (language == EShLangFragment) {
                    stageResources = &resources.stage_inputs;
                }
                if (stageResources) {
                    for (const auto &it : *stageResources) {
                        auto it2 = builtInVariableMapper.find(it.name);
                        if (it2 != builtInVariableMapper.end()) {
                            compiler.set_decoration(it.id, spv::DecorationLocation, it2->second);
                        }
                    }
                }
                m_parent->restoreInterfaceVariableNames(language, attributes, compiler);
                std::ostringstream os;
                compiler.add_header_line("#pragma clang diagnostic ignored \"-Wunused-variable\"\n"
                                         "#include <metal_math>\n"
                                         "using namespace metal;\n"
                                         "float length(float x) { return sqrt(x * x); }\n");
                os << compiler.compile();
                return os.str();
            };
            translatedVertexShaderSource = compile(EShLangVertex, vertexShaderInstructions);
            translatedFragmentShaderSource = compile(EShLangFragment, fragmentShaderInstructions);
            Fx9__Effect__Pass *message = static_cast<Fx9__Effect__Pass *>(m_opaque);
            message->vertex_shader->body_case = message->pixel_shader->body_case = FX9__EFFECT__SHADER__BODY_MSL;
            m_parent->copyString(translatedVertexShaderSource.c_str(), &message->vertex_shader->msl);
            m_parent->copyString(translatedFragmentShaderSource.c_str(), &message->pixel_shader->msl);
            succeeded = true;
        } catch (const std::exception &e) {
            sink.translator += e.what();
        }
    }
    return succeeded;
}

Compiler::SPIRVPassShader::SPIRVPassShader(
    Compiler *parent, const char *path, const glslang::TString &source, EffectProduct &effectProduct, void *opaque)
    : BasePassShader(parent, path, source, effectProduct, opaque)
{
}

Compiler::SPIRVPassShader::~SPIRVPassShader()
{
}

void
Compiler::SPIRVPassShader::configureParserContext(ParserContext &parser)
{
    parser.enableUniformBuffer();
}

bool
Compiler::SPIRVPassShader::translate(const InstructionList & /* vertexShaderInstructions */,
    const InstructionList & /* fragmentShaderInstructions */, std::string & /* translatedVertexShaderSource */,
    std::string & /* translatedFragmentShaderSource */, EffectProduct::LogSink & /* message */)
{
    Fx9__Effect__Pass *message = static_cast<Fx9__Effect__Pass *>(m_opaque);
    message->vertex_shader->body_case = message->pixel_shader->body_case = FX9__EFFECT__SHADER__BODY_SPIRV;
    return true;
}

void
Compiler::initialize()
{
    ShInitialize();
    InitThread();
}

void
Compiler::terminate()
{
    DetachThread();
    ShFinalize();
}

Compiler::Compiler(EProfile profile, EShMessages messages)
    : m_allocator(new TPoolAllocator())
    , m_messages(messages)
    , m_profile(profile)
{
    m_renderStateEnumConversions.insert(std::make_pair("ZENABLE", 7));
    m_renderStateEnumConversions.insert(std::make_pair("FILLMODE", 8));
    m_renderStateEnumConversions.insert(std::make_pair("SHADEMODE", 9));
    m_renderStateEnumConversions.insert(std::make_pair("ZWRITEENABLE", 14));
    m_renderStateEnumConversions.insert(std::make_pair("ALPHATESTENABLE", 15));
    m_renderStateEnumConversions.insert(std::make_pair("LASTPIXEL", 16));
    m_renderStateEnumConversions.insert(std::make_pair("SRCBLEND", 19));
    m_renderStateEnumConversions.insert(std::make_pair("DESTBLEND", 20));
    m_renderStateEnumConversions.insert(std::make_pair("CULLMODE", 22));
    m_renderStateEnumConversions.insert(std::make_pair("ZFUNC", 23));
    m_renderStateEnumConversions.insert(std::make_pair("ALPHAREF", 24));
    m_renderStateEnumConversions.insert(std::make_pair("ALPHAFUNC", 25));
    m_renderStateEnumConversions.insert(std::make_pair("DITHERENABLE", 26));
    m_renderStateEnumConversions.insert(std::make_pair("ALPHABLENDENABLE", 27));
    m_renderStateEnumConversions.insert(std::make_pair("FOGENABLE", 28));
    m_renderStateEnumConversions.insert(std::make_pair("SPECULARENABLE", 29));
    m_renderStateEnumConversions.insert(std::make_pair("FOGCOLOR", 34));
    m_renderStateEnumConversions.insert(std::make_pair("FOGTABLEMODE", 35));
    m_renderStateEnumConversions.insert(std::make_pair("FOGSTART", 36));
    m_renderStateEnumConversions.insert(std::make_pair("FOGEND", 37));
    m_renderStateEnumConversions.insert(std::make_pair("FOGDENSITY", 38));
    m_renderStateEnumConversions.insert(std::make_pair("RANGEFOGENABLE", 48));
    m_renderStateEnumConversions.insert(std::make_pair("STENCILENABLE", 52));
    m_renderStateEnumConversions.insert(std::make_pair("STENCILFAIL", 53));
    m_renderStateEnumConversions.insert(std::make_pair("STENCILZFAIL", 54));
    m_renderStateEnumConversions.insert(std::make_pair("STENCILPASS", 55));
    m_renderStateEnumConversions.insert(std::make_pair("STENCILFUNC", 56));
    m_renderStateEnumConversions.insert(std::make_pair("STENCILREF", 57));
    m_renderStateEnumConversions.insert(std::make_pair("STENCILMASK", 58));
    m_renderStateEnumConversions.insert(std::make_pair("STENCILWRITEMASK", 59));
    m_renderStateEnumConversions.insert(std::make_pair("TEXTUREFACTOR", 60));
    m_renderStateEnumConversions.insert(std::make_pair("WRAP0", 128));
    m_renderStateEnumConversions.insert(std::make_pair("WRAP1", 129));
    m_renderStateEnumConversions.insert(std::make_pair("WRAP2", 130));
    m_renderStateEnumConversions.insert(std::make_pair("WRAP3", 131));
    m_renderStateEnumConversions.insert(std::make_pair("WRAP4", 132));
    m_renderStateEnumConversions.insert(std::make_pair("WRAP5", 133));
    m_renderStateEnumConversions.insert(std::make_pair("WRAP6", 134));
    m_renderStateEnumConversions.insert(std::make_pair("WRAP7", 135));
    m_renderStateEnumConversions.insert(std::make_pair("CLIPPING", 136));
    m_renderStateEnumConversions.insert(std::make_pair("LIGHTING", 137));
    m_renderStateEnumConversions.insert(std::make_pair("AMBIENT", 139));
    m_renderStateEnumConversions.insert(std::make_pair("FOGVERTEXMODE", 140));
    m_renderStateEnumConversions.insert(std::make_pair("COLORVERTEX", 141));
    m_renderStateEnumConversions.insert(std::make_pair("LOCALVIEWER", 142));
    m_renderStateEnumConversions.insert(std::make_pair("NORMALIZENORMALS", 143));
    m_renderStateEnumConversions.insert(std::make_pair("DIFFUSEMATERIALSOURCE", 145));
    m_renderStateEnumConversions.insert(std::make_pair("SPECULARMATERIALSOURCE", 146));
    m_renderStateEnumConversions.insert(std::make_pair("AMBIENTMATERIALSOURCE", 147));
    m_renderStateEnumConversions.insert(std::make_pair("EMISSIVEMATERIALSOURCE", 148));
    m_renderStateEnumConversions.insert(std::make_pair("VERTEXBLEND", 151));
    m_renderStateEnumConversions.insert(std::make_pair("CLIPPLANEENABLE", 152));
    m_renderStateEnumConversions.insert(std::make_pair("POINTSIZE", 154));
    m_renderStateEnumConversions.insert(std::make_pair("POINTSIZE_MIN", 155));
    m_renderStateEnumConversions.insert(std::make_pair("POINTSPRITEENABLE", 156));
    m_renderStateEnumConversions.insert(std::make_pair("POINTSCALEENABLE", 157));
    m_renderStateEnumConversions.insert(std::make_pair("POINTSCALE_A", 158));
    m_renderStateEnumConversions.insert(std::make_pair("POINTSCALE_B", 159));
    m_renderStateEnumConversions.insert(std::make_pair("POINTSCALE_C", 160));
    m_renderStateEnumConversions.insert(std::make_pair("MULTISAMPLEANTIALIAS", 161));
    m_renderStateEnumConversions.insert(std::make_pair("MULTISAMPLEMASK", 162));
    m_renderStateEnumConversions.insert(std::make_pair("PATCHEDGESTYLE", 163));
    m_renderStateEnumConversions.insert(std::make_pair("DEBUGMONITORTOKEN", 165));
    m_renderStateEnumConversions.insert(std::make_pair("POINTSIZE_MAX", 166));
    m_renderStateEnumConversions.insert(std::make_pair("INDEXEDVERTEXBLENDENABLE", 167));
    m_renderStateEnumConversions.insert(std::make_pair("COLORWRITEENABLE", 168));
    m_renderStateEnumConversions.insert(std::make_pair("TWEENFACTOR", 170));
    m_renderStateEnumConversions.insert(std::make_pair("BLENDOP", 171));
    m_renderStateEnumConversions.insert(std::make_pair("POSITIONDEGREE", 172));
    m_renderStateEnumConversions.insert(std::make_pair("NORMALDEGREE", 173));
    m_renderStateEnumConversions.insert(std::make_pair("SCISSORTESTENABLE", 174));
    m_renderStateEnumConversions.insert(std::make_pair("SLOPESCALEDEPTHBIAS", 175));
    m_renderStateEnumConversions.insert(std::make_pair("ANTIALIASEDLINEENABLE", 176));
    m_renderStateEnumConversions.insert(std::make_pair("MINTESSELLATIONLEVEL", 178));
    m_renderStateEnumConversions.insert(std::make_pair("MAXTESSELLATIONLEVEL", 179));
    m_renderStateEnumConversions.insert(std::make_pair("ADAPTIVETESS_X", 180));
    m_renderStateEnumConversions.insert(std::make_pair("ADAPTIVETESS_Y", 181));
    m_renderStateEnumConversions.insert(std::make_pair("ADAPTIVETESS_Z", 182));
    m_renderStateEnumConversions.insert(std::make_pair("ADAPTIVETESS_W", 183));
    m_renderStateEnumConversions.insert(std::make_pair("ENABLEADAPTIVETESSELLATION", 184));
    m_renderStateEnumConversions.insert(std::make_pair("TWOSIDEDSTENCILMODE", 185));
    m_renderStateEnumConversions.insert(std::make_pair("CCW_STENCILFAIL", 186));
    m_renderStateEnumConversions.insert(std::make_pair("CCW_STENCILZFAIL", 187));
    m_renderStateEnumConversions.insert(std::make_pair("CCW_STENCILPASS", 188));
    m_renderStateEnumConversions.insert(std::make_pair("CCW_STENCILFUNC", 189));
    m_renderStateEnumConversions.insert(std::make_pair("COLORWRITEENABLE1", 190));
    m_renderStateEnumConversions.insert(std::make_pair("COLORWRITEENABLE2", 191));
    m_renderStateEnumConversions.insert(std::make_pair("COLORWRITEENABLE3", 192));
    m_renderStateEnumConversions.insert(std::make_pair("BLENDFACTOR", 193));
    m_renderStateEnumConversions.insert(std::make_pair("SRGBWRITEENABLE", 194));
    m_renderStateEnumConversions.insert(std::make_pair("DEPTHBIAS", 195));
    m_renderStateEnumConversions.insert(std::make_pair("WRAP8", 198));
    m_renderStateEnumConversions.insert(std::make_pair("WRAP9", 199));
    m_renderStateEnumConversions.insert(std::make_pair("WRAP10", 200));
    m_renderStateEnumConversions.insert(std::make_pair("WRAP11", 201));
    m_renderStateEnumConversions.insert(std::make_pair("WRAP12", 202));
    m_renderStateEnumConversions.insert(std::make_pair("WRAP13", 203));
    m_renderStateEnumConversions.insert(std::make_pair("WRAP14", 204));
    m_renderStateEnumConversions.insert(std::make_pair("WRAP15", 205));
    m_renderStateEnumConversions.insert(std::make_pair("SEPARATEALPHABLENDENABLE", 206));
    m_renderStateEnumConversions.insert(std::make_pair("SRCBLENDALPHA", 207));
    m_renderStateEnumConversions.insert(std::make_pair("DESTBLENDALPHA", 208));
    m_renderStateEnumConversions.insert(std::make_pair("BLENDOPALPHA", 209));
    m_renderStateShadeModeValueConversions.insert(std::make_pair("FLAT", 1));
    m_renderStateShadeModeValueConversions.insert(std::make_pair("GOURAUD", 2));
    m_renderStateShadeModeValueConversions.insert(std::make_pair("PHONG", 3));
    m_renderStateFillModeValueConversions.insert(std::make_pair("POINT", 1));
    m_renderStateFillModeValueConversions.insert(std::make_pair("WIREFRAME", 2));
    m_renderStateFillModeValueConversions.insert(std::make_pair("SOLID", 3));
    m_renderStateBlendModeValueConversions.insert(std::make_pair("ZERO", 1));
    m_renderStateBlendModeValueConversions.insert(std::make_pair("ONE", 2));
    m_renderStateBlendModeValueConversions.insert(std::make_pair("SRCCOLOR", 3));
    m_renderStateBlendModeValueConversions.insert(std::make_pair("INVSRCCOLOR", 4));
    m_renderStateBlendModeValueConversions.insert(std::make_pair("SRCALPHA", 5));
    m_renderStateBlendModeValueConversions.insert(std::make_pair("INVSRCALPHA", 6));
    m_renderStateBlendModeValueConversions.insert(std::make_pair("DESTALPHA", 7));
    m_renderStateBlendModeValueConversions.insert(std::make_pair("INVDESTALPHA", 8));
    m_renderStateBlendModeValueConversions.insert(std::make_pair("DESTCOLOR", 9));
    m_renderStateBlendModeValueConversions.insert(std::make_pair("INVDESTCOLOR", 10));
    m_renderStateBlendModeValueConversions.insert(std::make_pair("SRCALPHASAT", 11));
    m_renderStateBlendModeValueConversions.insert(std::make_pair("BOTHSRCALPHA", 12));
    m_renderStateBlendModeValueConversions.insert(std::make_pair("BOTHINVSRCALPHA", 13));
    m_renderStateBlendModeValueConversions.insert(std::make_pair("BLENDFACTOR", 14));
    m_renderStateBlendModeValueConversions.insert(std::make_pair("INVBLENDFACTOR", 15));
    m_renderStateBlendOpValueConversions.insert(std::make_pair("ADD", 1));
    m_renderStateBlendOpValueConversions.insert(std::make_pair("SUBTRACT", 2));
    m_renderStateBlendOpValueConversions.insert(std::make_pair("REVSUBTRACT", 3));
    m_renderStateBlendOpValueConversions.insert(std::make_pair("MIN", 4));
    m_renderStateBlendOpValueConversions.insert(std::make_pair("MAX", 5));
    m_renderStateTextureAddressValueConversions.insert(std::make_pair("WRAP", 1));
    m_renderStateTextureAddressValueConversions.insert(std::make_pair("MIRROR", 2));
    m_renderStateTextureAddressValueConversions.insert(std::make_pair("CLAMP", 3));
    m_renderStateTextureAddressValueConversions.insert(std::make_pair("BORDER", 4));
    m_renderStateTextureAddressValueConversions.insert(std::make_pair("MIRRORONCE", 5));
    m_renderStateCullValueConversions.insert(std::make_pair("NONE", 1));
    m_renderStateCullValueConversions.insert(std::make_pair("CW", 2));
    m_renderStateCullValueConversions.insert(std::make_pair("CCW", 3));
    m_renderStateCmpFuncValueConversions.insert(std::make_pair("NEVER", 1));
    m_renderStateCmpFuncValueConversions.insert(std::make_pair("LESS", 2));
    m_renderStateCmpFuncValueConversions.insert(std::make_pair("EQUAL", 3));
    m_renderStateCmpFuncValueConversions.insert(std::make_pair("LESSEQUAL", 4));
    m_renderStateCmpFuncValueConversions.insert(std::make_pair("GREATER", 5));
    m_renderStateCmpFuncValueConversions.insert(std::make_pair("NOTEQUAL", 6));
    m_renderStateCmpFuncValueConversions.insert(std::make_pair("GREATEREQUAL", 7));
    m_renderStateCmpFuncValueConversions.insert(std::make_pair("ALWAYS", 8));
    m_renderStateStencilOpValueConversions.insert(std::make_pair("KEEP", 1));
    m_renderStateStencilOpValueConversions.insert(std::make_pair("ZERO", 2));
    m_renderStateStencilOpValueConversions.insert(std::make_pair("REPLACE", 3));
    m_renderStateStencilOpValueConversions.insert(std::make_pair("INCRSAT", 4));
    m_renderStateStencilOpValueConversions.insert(std::make_pair("DECRSAT", 5));
    m_renderStateStencilOpValueConversions.insert(std::make_pair("INVERT", 6));
    m_renderStateStencilOpValueConversions.insert(std::make_pair("INCR", 7));
    m_renderStateStencilOpValueConversions.insert(std::make_pair("DECR", 8));
    m_renderStateFogModeValueConversions.insert(std::make_pair("NONE", 0));
    m_renderStateFogModeValueConversions.insert(std::make_pair("EXP", 1));
    m_renderStateFogModeValueConversions.insert(std::make_pair("EXP2", 2));
    m_renderStateFogModeValueConversions.insert(std::make_pair("LINEAR", 3));
    m_renderStateZBufferTypeValueConversions.insert(std::make_pair("FALSE", 0));
    m_renderStateZBufferTypeValueConversions.insert(std::make_pair("TRUE", 1));
    m_renderStateZBufferTypeValueConversions.insert(std::make_pair("USEW", 2));
    m_renderStatePrimitiveTypeValueConversions.insert(std::make_pair("POINTLIST", 1));
    m_renderStatePrimitiveTypeValueConversions.insert(std::make_pair("LINELIST", 2));
    m_renderStatePrimitiveTypeValueConversions.insert(std::make_pair("LINESTRIP", 3));
    m_renderStatePrimitiveTypeValueConversions.insert(std::make_pair("TRIANGLELIST", 4));
    m_renderStatePrimitiveTypeValueConversions.insert(std::make_pair("TRIANGLESTRIP", 5));
    m_renderStatePrimitiveTypeValueConversions.insert(std::make_pair("TRIANGLEFAN", 6));
    m_renderStateTransformStateTypeValueConversions.insert(std::make_pair("VIEW", 2));
    m_renderStateTransformStateTypeValueConversions.insert(std::make_pair("PROJECTION", 3));
    m_renderStateTransformStateTypeValueConversions.insert(std::make_pair("TEXTURE0", 16));
    m_renderStateTransformStateTypeValueConversions.insert(std::make_pair("TEXTURE1", 17));
    m_renderStateTransformStateTypeValueConversions.insert(std::make_pair("TEXTURE2", 18));
    m_renderStateTransformStateTypeValueConversions.insert(std::make_pair("TEXTURE3", 19));
    m_renderStateTransformStateTypeValueConversions.insert(std::make_pair("TEXTURE4", 20));
    m_renderStateTransformStateTypeValueConversions.insert(std::make_pair("TEXTURE5", 21));
    m_renderStateTransformStateTypeValueConversions.insert(std::make_pair("TEXTURE6", 22));
    m_renderStateTransformStateTypeValueConversions.insert(std::make_pair("TEXTURE7", 23));
    m_samplerStateEnumConversions.insert(std::make_pair("ADDRESSU", 1));
    m_samplerStateEnumConversions.insert(std::make_pair("ADDRESSV", 2));
    m_samplerStateEnumConversions.insert(std::make_pair("ADDRESSW", 3));
    m_samplerStateEnumConversions.insert(std::make_pair("BORDERCOLOR", 4));
    m_samplerStateEnumConversions.insert(std::make_pair("MAGFILTER", 5));
    m_samplerStateEnumConversions.insert(std::make_pair("MINFILTER", 6));
    m_samplerStateEnumConversions.insert(std::make_pair("MIPFILTER", 7));
    m_samplerStateEnumConversions.insert(std::make_pair("MIPMAPLODBIAS", 8));
    m_samplerStateEnumConversions.insert(std::make_pair("MAXMIPLEVEL", 9));
    m_samplerStateEnumConversions.insert(std::make_pair("MAXANISOTROPY", 10));
    m_samplerStateEnumConversions.insert(std::make_pair("SRGBTEXTURE", 11));
    m_samplerStateEnumConversions.insert(std::make_pair("ELEMENTINDEX", 12));
    m_samplerStateEnumConversions.insert(std::make_pair("DMAPOFFSET", 13));
    m_samplerStateTextureAddressValueConversions.insert(std::make_pair("WRAP", 1));
    m_samplerStateTextureAddressValueConversions.insert(std::make_pair("MIRROR", 2));
    m_samplerStateTextureAddressValueConversions.insert(std::make_pair("CLAMP", 3));
    m_samplerStateTextureAddressValueConversions.insert(std::make_pair("BORDER", 4));
    m_samplerStateTextureAddressValueConversions.insert(std::make_pair("MIRRORONCE", 5));
    m_samplerStateTextureStageStateValueConversions.insert(std::make_pair("COLOROP", 1));
    m_samplerStateTextureStageStateValueConversions.insert(std::make_pair("COLORARG1", 2));
    m_samplerStateTextureStageStateValueConversions.insert(std::make_pair("COLORARG2", 3));
    m_samplerStateTextureStageStateValueConversions.insert(std::make_pair("ALPHAOP", 4));
    m_samplerStateTextureStageStateValueConversions.insert(std::make_pair("ALPHAARG1", 5));
    m_samplerStateTextureStageStateValueConversions.insert(std::make_pair("ALPHAARG2", 6));
    m_samplerStateTextureStageStateValueConversions.insert(std::make_pair("BUMPENVMAT00", 7));
    m_samplerStateTextureStageStateValueConversions.insert(std::make_pair("BUMPENVMAT01", 8));
    m_samplerStateTextureStageStateValueConversions.insert(std::make_pair("BUMPENVMAT10", 9));
    m_samplerStateTextureStageStateValueConversions.insert(std::make_pair("BUMPENVMAT11", 10));
    m_samplerStateTextureStageStateValueConversions.insert(std::make_pair("TEXCOORDINDEX", 11));
    m_samplerStateTextureStageStateValueConversions.insert(std::make_pair("BUMPENVLSCALE", 22));
    m_samplerStateTextureStageStateValueConversions.insert(std::make_pair("BUMPENVLOFFSET", 23));
    m_samplerStateTextureStageStateValueConversions.insert(std::make_pair("TEXTURETRANSFORMFLAGS", 24));
    m_samplerStateTextureStageStateValueConversions.insert(std::make_pair("COLORARG0", 26));
    m_samplerStateTextureStageStateValueConversions.insert(std::make_pair("ALPHAARG0", 27));
    m_samplerStateTextureStageStateValueConversions.insert(std::make_pair("RESULTARG", 28));
    m_samplerStateTextureStageStateValueConversions.insert(std::make_pair("CONSTANT", 32));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("DISABLE", 1));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("SELECTARG1", 2));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("SELECTARG2", 3));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("MODULATE", 4));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("MODULATE2X", 5));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("MODULATE4X", 6));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("ADD", 7));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("ADDSIGNED", 8));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("ADDSIGNED2X", 9));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("SUBTRACT", 10));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("ADDSMOOTH", 11));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("BLENDDIFFUSEALPHA", 12));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("BLENDTEXTUREALPHA", 13));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("BLENDFACTORALPHA", 14));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("BLENDTEXTUREALPHAPM", 15));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("BLENDCURRENTALPHA", 16));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("PREMODULATE", 17));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("MODULATEALPHA_ADDCOLOR", 18));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("MODULATECOLOR_ADDALPHA", 19));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("MODULATEINVALPHA_ADDCOLOR", 20));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("MODULATEINVCOLOR_ADDALPHA", 21));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("BUMPENVMAP", 22));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("BUMPENVMAPLUMINANCE", 23));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("DOTPRODUCT3", 24));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("MULTIPLYADD", 25));
    m_samplerStateTextureOpValueConversions.insert(std::make_pair("LERP", 26));
    m_samplerStateTextureFilterTypeValueConversions.insert(std::make_pair("NONE", 0));
    m_samplerStateTextureFilterTypeValueConversions.insert(std::make_pair("POINT", 1));
    m_samplerStateTextureFilterTypeValueConversions.insert(std::make_pair("LINEAR", 2));
    m_samplerStateTextureFilterTypeValueConversions.insert(std::make_pair("ANISOTROPIC", 3));
    m_samplerStateTextureFilterTypeValueConversions.insert(std::make_pair("PYRAMIDALQUAD", 6));
    m_samplerStateTextureFilterTypeValueConversions.insert(std::make_pair("GAUSSIANQUAD", 7));
    m_vertexShaderInputVariables.insert(std::make_pair(EbvVertex, "a_position"));
    m_vertexShaderInputVariables.insert(std::make_pair(EbvVertexIndex, "a_index"));
    m_vertexShaderInputVariables.insert(std::make_pair(EbvNormal, "a_normal"));
    m_vertexShaderInputVariables.insert(std::make_pair(EbvMultiTexCoord0, "a_texcoord0"));
    m_vertexShaderInputVariables.insert(std::make_pair(EbvMultiTexCoord1, "a_texcoord1"));
    m_vertexShaderInputVariables.insert(std::make_pair(EbvMultiTexCoord2, "a_texcoord2"));
    m_vertexShaderInputVariables.insert(std::make_pair(EbvMultiTexCoord3, "a_texcoord3"));
    m_vertexShaderInputVariables.insert(std::make_pair(EbvMultiTexCoord4, "a_texcoord4"));
    m_vertexShaderInputVariables.insert(std::make_pair(EbvMultiTexCoord5, "a_texcoord5"));
    m_vertexShaderInputVariables.insert(std::make_pair(EbvMultiTexCoord6, "a_texcoord6"));
    m_vertexShaderInputVariables.insert(std::make_pair(EbvMultiTexCoord7, "a_texcoord7"));
    m_vertexShaderInputVariables.insert(std::make_pair(EbvFragDepthGreater, "a_texcoord8"));
    m_vertexShaderInputVariables.insert(std::make_pair(EbvFragDepthLesser, "a_texcoord9"));
    m_vertexShaderInputVariables.insert(std::make_pair(EbvFrontColor, "a_color0"));
    m_vertexShaderInputVariables.insert(std::make_pair(EbvBackColor, "a_color1"));
    m_vertexShaderInputVariables.insert(std::make_pair(EbvFrontSecondaryColor, "a_color2"));
    m_vertexShaderInputVariables.insert(std::make_pair(EbvBackSecondaryColor, "a_color3"));
    m_pixelShaderInputVariables.insert(std::make_pair(EbvVertex, "v_position"));
    m_pixelShaderInputVariables.insert(std::make_pair(EbvVertexIndex, "v_index"));
    m_pixelShaderInputVariables.insert(std::make_pair(EbvNormal, "v_normal"));
    m_pixelShaderInputVariables.insert(std::make_pair(EbvMultiTexCoord0, "v_texcoord0"));
    m_pixelShaderInputVariables.insert(std::make_pair(EbvMultiTexCoord1, "v_texcoord1"));
    m_pixelShaderInputVariables.insert(std::make_pair(EbvMultiTexCoord2, "v_texcoord2"));
    m_pixelShaderInputVariables.insert(std::make_pair(EbvMultiTexCoord3, "v_texcoord3"));
    m_pixelShaderInputVariables.insert(std::make_pair(EbvMultiTexCoord4, "v_texcoord4"));
    m_pixelShaderInputVariables.insert(std::make_pair(EbvMultiTexCoord5, "v_texcoord5"));
    m_pixelShaderInputVariables.insert(std::make_pair(EbvMultiTexCoord6, "v_texcoord6"));
    m_pixelShaderInputVariables.insert(std::make_pair(EbvMultiTexCoord7, "v_texcoord7"));
    m_pixelShaderInputVariables.insert(std::make_pair(EbvFragDepthGreater, "v_texcoord8"));
    m_pixelShaderInputVariables.insert(std::make_pair(EbvFragDepthLesser, "v_texcoord9"));
    m_pixelShaderInputVariables.insert(std::make_pair(EbvFrontColor, "v_color0"));
    m_pixelShaderInputVariables.insert(std::make_pair(EbvBackColor, "v_color1"));
    m_pixelShaderInputVariables.insert(std::make_pair(EbvFrontSecondaryColor, "v_color2"));
    m_pixelShaderInputVariables.insert(std::make_pair(EbvBackSecondaryColor, "v_color3"));
    const TBuiltInVariable variables[] = { EbvVertex, EbvNormal, EbvMultiTexCoord0, EbvMultiTexCoord1,
        EbvMultiTexCoord2, EbvMultiTexCoord3, EbvMultiTexCoord4, EbvColor };
    int offset = 0;
    for (auto it : variables) {
        m_vertexShaderInputLocations.insert(std::make_pair(it, offset++));
    }
}

Compiler::~Compiler()
{
}

bool
Compiler::compile(const char *path, EffectProduct &effectProduct)
{
    bool result = false;
#if defined(_WIN32)
    wchar_t widePath[MAX_PATH];
    widePath[MultiByteToWideChar(CP_UTF8, 0, path, int(strlen(path)), widePath, ARRAYSIZE(widePath))] = 0;
    HANDLE handle =
        CreateFileW(widePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle != INVALID_HANDLE_VALUE) {
        std::vector<char> buffer;
        DWORD size = GetFileSize(handle, nullptr);
        buffer.resize(size);
        DWORD numReadBytes = 0;
        ReadFile(handle, buffer.data(), size, &numReadBytes, nullptr);
        CloseHandle(handle);
        const std::string source(buffer.data(), buffer.size());
        result = compile(source, path, effectProduct);
    }
#else
    int fd = open(path, O_RDONLY);
    if (fd != -1) {
        std::vector<char> buffer;
        struct stat st;
        fstat(fd, &st);
        if (S_ISREG(st.st_mode)) {
            buffer.resize(st.st_size);
            read(fd, buffer.data(), buffer.size());
            const std::string source(buffer.data(), buffer.size());
            result = compile(source, path, effectProduct);
        }
        close(fd);
    }
#endif
    return result;
}

bool
Compiler::compile(const std::string &source, const char *filename, EffectProduct &effectProduct)
{
    TPoolAllocator &allocator = GetThreadPoolAllocator();
    SetThreadPoolAllocator(m_allocator.get());
    m_allocator->push();
    rebuildAllSymbolTables();
    std::unique_ptr<TSymbolTable> symbolTable(new TSymbolTable());
    TSymbolTable *symbolTablePtr = symbolTable.get(), &symbolTableRef = *symbolTablePtr;
    bool succeeded = false;
    {
        symbolTableRef.adoptLevels(*m_fragmentSymbolTable);
        TInfoSink infoSink;
        TIntermediate intermediate(EShLangVertex, m_version, m_profile);
        const SpvVersion &spv = createSpvVersion();
        intermediate.setSpv(spv);
        intermediate.setSource(EShSourceHlsl);
        HlslParseContext parseContext(symbolTableRef, intermediate, false, m_version, m_profile, spv, EShLangFragment,
            infoSink, TString(), false, EShMsgDefault);
        ParserContext parser(&parseContext, filename, m_vertexShaderInputVariables, m_pixelShaderInputVariables);
        for (auto it = m_includeSourceData.begin(), end = m_includeSourceData.end(); it != end; ++it) {
            parser.addIncludeSource(it->first.c_str(), it->second.c_str());
        }
        TString inputSource(source.cbegin(), source.cend());
        // ensure line feed to prevent "expected newline after header name"
        inputSource.append("\n");
        InstructionList instructions;
        spv::SpvBuildLogger logger;
        if (generateSPVInstructions(
                std::make_pair(std::string(), nullptr), inputSource, parser, instructions, &logger, infoSink)) {
            Fx9__Effect__Effect message = FX9__EFFECT__EFFECT__INIT;
            compileAllTechniques(parser.techniques(), filename, inputSource, effectProduct, &message);
            TUnorderedMap<TString, TString> metadata;
            metadata.insert(std::make_pair("generator", "fx9"));
            convertMetadata(metadata, &message);
            BaseParameterConverter converter(parser, this, effectProduct, &message);
            converter.convertAll();
            convertIncludePathSet(parser, &message);
            effectProduct.message.resize(fx9__effect__effect__get_packed_size(&message));
            fx9__effect__effect__pack(&message, effectProduct.message.data());
            succeeded = effectProduct.numPasses == 0 ||
                (effectProduct.numCompiledPasses > 0 &&
                    effectProduct.numCompiledPasses == effectProduct.numValidatedPasses);
        }
        else {
            effectProduct.sink.info = infoSink.info.c_str();
            effectProduct.sink.debug = infoSink.debug.c_str();
        }
    }
    symbolTable.reset();
    m_allocator->pop();
    SetThreadPoolAllocator(&allocator);
    return succeeded;
}

void
Compiler::addIncludeSource(const std::string &filePath, const std::string &sourceData)
{
    m_includeSourceData.insert(std::make_pair(filePath, sourceData));
}

void
Compiler::setDefineMacro(const std::string &key, const std::string &value)
{
    m_macros[key] = value;
}

bool
Compiler::containsDefineMacro(const std::string &key) const
{
    return m_macros.find(key) != m_macros.end();
}

void
Compiler::removeDefineMacro(const std::string &key)
{
    m_macros.erase(key);
}

int
Compiler::version() const
{
    return m_version;
}

void
Compiler::setVersion(int value)
{
    if (m_version != value) {
        m_version = value;
    }
}

ParserContext::BuiltInLocationMap
Compiler::vertexShaderInputLocations() const
{
    return m_vertexShaderInputLocations;
}

void
Compiler::setVertexShaderInputLocations(const ParserContext::BuiltInLocationMap &value)
{
    m_vertexShaderInputLocations = value;
}

ParserContext::BuiltInVariableMap
Compiler::vertexShaderInputVariables() const
{
    return m_vertexShaderInputVariables;
}

void
Compiler::setVertexShaderInputVariables(const ParserContext::BuiltInVariableMap &value)
{
    m_vertexShaderInputVariables = value;
}

ParserContext::BuiltInVariableMap
Compiler::pixelShaderInputVariables() const
{
    return m_pixelShaderInputVariables;
}

void
Compiler::setPixelShaderInputVariables(const ParserContext::BuiltInVariableMap &value)
{
    m_pixelShaderInputVariables = value;
}

std::string
Compiler::metalShaderEntryPoint() const
{
    return m_metalShaderEntryPointName;
}

void
Compiler::setMetalShaderEntryPoint(const std::string &value)
{
    m_metalShaderEntryPointName = value;
}

std::string
Compiler::metalShaderUniformBufferName() const
{
    return m_metalShaderUniformBufferName;
}

void
Compiler::setMetalShaderUniformBufferName(const std::string &value)
{
    m_metalShaderUniformBufferName = value;
}

Compiler::LanguageType
Compiler::targetLanguage() const
{
    return m_language;
}

void
Compiler::setTargetLanguage(Compiler::LanguageType value)
{
    if (m_language != value) {
        switch (value) {
        case kLanguageTypeESSL:
            m_profile = EEsProfile;
            break;
        default:
            m_profile = ECoreProfile;
            break;
        }
        m_language = value;
    }
}

bool
Compiler::isOptimizeEnabled() const
{
    return m_enableOptimize;
}

void
Compiler::setOptimizeEnabled(bool value)
{
    m_enableOptimize = value;
}

bool
Compiler::isValidationEnabled() const
{
    return m_enableValidation;
}

void
Compiler::setValidationEnabled(bool value)
{
    m_enableValidation = value;
}

template <typename TType>
TType *
Compiler::allocate()
{
    return static_cast<TType *>(m_allocator->allocate(sizeof(TType)));
}

template <typename TType>
TType **
Compiler::allocateArray(size_t numItems)
{
    assert(numItems > 0);
    return static_cast<TType **>(m_allocator->allocate(sizeof(TType) * numItems));
}

uint32_t
Compiler::resolveRenderStateValue(const ParserContext::PassState &state, const Convertions &conversions)
{
    const TIntermSymbol *valueNode = state.m_value->getAsSymbolNode();
    const TString &value = valueNode ? valueNode->getName() : TString();
    TString valueUpperCase(value);
    std::transform(value.begin(), value.end(), valueUpperCase.begin(), ::toupper);
    auto it2 = conversions.find(valueUpperCase.c_str());
    uint32_t resolved = 0;
    if (it2 != conversions.end()) {
        resolved = it2->second;
    }
    return resolved;
}

uint32_t
Compiler::resolveSamplerStateValue(const TIntermSymbol *valueNode, const Convertions &conversions)
{
    const TString &value = valueNode ? valueNode->getName() : TString();
    TString valueUpperCase(value);
    std::transform(value.begin(), value.end(), valueUpperCase.begin(), ::toupper);
    auto it2 = conversions.find(valueUpperCase.c_str());
    uint32_t resolved = 0;
    if (it2 != conversions.end()) {
        resolved = it2->second;
    }
    return resolved;
}

void
Compiler::copyString(const TString &source, TPoolAllocator &allocator, char **destinationPtr)
{
    char *destination = *destinationPtr = static_cast<char *>(allocator.allocate(source.size() + 1));
    strncpy(destination, source.c_str(), source.size());
    destination[source.size()] = 0;
}

void
Compiler::copyString(const glslang::TString &source, char **destination)
{
    copyString(source, *m_allocator, destination);
}

void
Compiler::initializeBuiltInSymbolTable(const TString &builtIn, EShLanguage language, TSymbolTable &outputSymbolTable)
{
    TIntermediate intermediate(language, m_profile);
    TInfoSink infoSink;
    const SpvVersion &spv = createSpvVersion();
    intermediate.setSpv(spv);
    TParseContext parseContext(outputSymbolTable, intermediate, true, m_version, m_profile, spv, language, infoSink);
    TShader::ForbidIncluder includer;
    TPpContext ppContext(parseContext, std::string(), includer);
    TScanContext scanContext(parseContext);
    parseContext.setScanContext(&scanContext);
    parseContext.setPpContext(&ppContext);
    outputSymbolTable.push();
    if (!builtIn.empty()) {
        const char *builtInShaders[] = { builtIn.c_str(), nullptr };
        size_t builtInLengths[] = { builtIn.size(), 0 };
        TInputScanner input(1, builtInShaders, builtInLengths);
        parseContext.parseShaderStrings(ppContext, input);
    }
}

void
Compiler::initializeBuiltInSymbolTable(
    EShLanguage language, TBuiltInParseables &builtIn, TSymbolTable &commonSymbolTable, TSymbolTable &outputSymbolTable)
{
    const SpvVersion &spv = createSpvVersion();
    outputSymbolTable.adoptLevels(commonSymbolTable);
    initializeBuiltInSymbolTable(builtIn.getStageString(language), language, outputSymbolTable);
    builtIn.identifyBuiltIns(m_version, m_profile, spv, language, outputSymbolTable);
}

bool
Compiler::addContextSpecificSymbols(const TBuiltInResource *resources, EShLanguage language, TSymbolTable &symbolTable)
{
    const SpvVersion &spv = createSpvVersion();
    std::unique_ptr<TBuiltInParseables> builtInParseables(new BuiltInParsables(m_language));
    builtInParseables->initialize(*resources, m_version, m_profile, spv, language);
    const TString commonString(builtInParseables->getCommonString());
    initializeBuiltInSymbolTable(commonString, language, symbolTable);
    builtInParseables->identifyBuiltIns(m_version, m_profile, spv, language, symbolTable, *resources);
    return true;
}

void
Compiler::compileAllTechniques(const ParserContext::TechniqueList &techniques, const char *path, const TString &source,
    EffectProduct &effectProduct, void *opaque)
{
    Fx9__Effect__Effect *message = static_cast<Fx9__Effect__Effect *>(opaque);
    if (!techniques.empty()) {
        message->n_techniques = techniques.size();
        message->techniques = allocateArray<Fx9__Effect__Technique>(message->n_techniques);
        size_t index = 0;
        for (auto it = techniques.begin(), end = techniques.end(); it != end; ++it) {
            const ParserContext::Technique &technique = *it;
            Fx9__Effect__Technique *techniquePtr = nullptr;
            techniquePtr = message->techniques[index++] = allocate<Fx9__Effect__Technique>();
            fx9__effect__technique__init(techniquePtr);
            copyString(it->m_name, &techniquePtr->name);
            compileAllPasses(technique, path, source, effectProduct, techniquePtr);
            convertAllAnnotations(technique.m_annotations, &techniquePtr->annotations, &techniquePtr->n_annotations);
        }
    }
}

void
Compiler::compileAllPasses(const ParserContext::Technique &technique, const char *path, const glslang::TString &source,
    EffectProduct &effectProduct, void *opaque)
{
    Fx9__Effect__Technique *message = static_cast<Fx9__Effect__Technique *>(opaque);
    auto passes = technique.m_passes;
    if (!passes.empty()) {
        message->n_passes = passes.size();
        message->passes = allocateArray<Fx9__Effect__Pass>(message->n_passes);
        size_t index = 0;
        for (auto it = passes.begin(), end = passes.end(); it != end; ++it) {
            const ParserContext::Pass &pass = *it;
            Fx9__Effect__Pass *basePassPtr = nullptr;
            basePassPtr = message->passes[index++] = allocate<Fx9__Effect__Pass>();
            fx9__effect__pass__init(basePassPtr);
            copyString(it->m_name, &basePassPtr->name);
            effectProduct.numPasses++;
            InstructionList vertexShaderInstructions, fragmentShaderInstructions;
            std::unique_ptr<BasePassShader> parser;
            if (m_language == kLanguageTypeSPIRV) {
                parser.reset(new SPIRVPassShader(this, path, source, effectProduct, basePassPtr));
            }
            else if (m_language == kLanguageTypeHLSL) {
                parser.reset(new HLSLPassShader(this, path, source, effectProduct, basePassPtr));
            }
            else if (m_language == kLanguageTypeMSL) {
                parser.reset(new MSLPassShader(this, path, source, effectProduct, basePassPtr));
            }
            else {
                parser.reset(new DX9MSPassShader(this, path, source, effectProduct, basePassPtr));
            }
            /* needs for forcePointSizeAssignment detection required by MSL */
            parser->convertAllPassStates(pass);
            if (parser->compile(EShLangVertex, pass.m_vertexShaderEntryPoint, vertexShaderInstructions) &&
                parser->compile(EShLangFragment, pass.m_pixelShaderEntryPoint, fragmentShaderInstructions)) {
                std::unique_ptr<TShader> vertexShader, fragmentShader;
                std::string translatedVertexShaderSource, translatedFragmentShaderSource;
                if (parser->translate(vertexShaderInstructions, fragmentShaderInstructions,
                        translatedVertexShaderSource, translatedFragmentShaderSource, effectProduct.sink)) {
                    convertAllAnnotations(pass.m_annotations, &basePassPtr->annotations, &basePassPtr->n_annotations);
                    effectProduct.numCompiledPasses++;
                    if (m_enableValidation) {
                        TPoolAllocator &allocator = GetThreadPoolAllocator();
                        if (validateShaderSource(EShLangVertex, translatedVertexShaderSource,
                                effectProduct.sink.validator, vertexShader) &&
                            validateShaderSource(EShLangFragment, translatedFragmentShaderSource,
                                effectProduct.sink.validator, fragmentShader)) {
                            TProgram program;
                            program.addShader(vertexShader.get());
                            program.addShader(fragmentShader.get());
                            if (program.link(EShMsgDefault)) {
                                effectProduct.numValidatedPasses++;
                            }
                            else {
                                effectProduct.sink.validator.append(program.getInfoLog());
                            }
                        }
                        SetThreadPoolAllocator(&allocator);
                    }
                    else {
                        effectProduct.numValidatedPasses++;
                    }
                }
            }
        }
    }
}

void
Compiler::convertAnnotation(const TString &name, const TIntermNode *value, void *opaque)
{
    Fx9__Effect__Annotation *annotationPtr = static_cast<Fx9__Effect__Annotation *>(opaque);
    copyString(name, &annotationPtr->name);
    if (value) {
        if (const TIntermAggregate *valueNode = value->getAsAggregate()) {
            const TIntermSequence sequence = valueNode->getSequence();
            TString concatString;
            TVector<const TIntermConstantUnion *> values;
            for (size_t i = 0, numSequences = sequence.size(); i < numSequences; i++) {
                const TIntermNode *firstNode = sequence[i];
                if (const TIntermSymbol *symbolNode = firstNode->getAsSymbolNode()) {
                    concatString.append(symbolNode->getName());
                }
                else if (const TIntermConstantUnion *constantUnion = firstNode->getAsConstantUnion()) {
                    values.push_back(constantUnion);
                }
            }
            if (!concatString.empty()) {
                annotationPtr->value_case = FX9__EFFECT__ANNOTATION__VALUE_SVAL;
                copyString(concatString, &annotationPtr->sval);
            }
            else if (!values.empty()) {
                switch (values[0]->getType().getBasicType()) {
                case EbtBool:
                    annotationPtr->value_case = FX9__EFFECT__ANNOTATION__VALUE_BVAL;
                    annotationPtr->bval = values[0]->getConstArray()[0].getBConst();
                    break;
                case EbtInt:
                    if (values.size() == 1) {
                        annotationPtr->value_case = FX9__EFFECT__ANNOTATION__VALUE_IVAL;
                        annotationPtr->ival = values[0]->getConstArray()[0].getIConst();
                    }
                    else {
                        Fx9__Effect__Vector4i *ivec4 = nullptr;
                        annotationPtr->value_case = FX9__EFFECT__ANNOTATION__VALUE_IVAL4;
                        ivec4 = annotationPtr->ival4 = allocate<Fx9__Effect__Vector4i>();
                        fx9__effect__vector4i__init(ivec4);
                        auto convertToInteger = [](const TIntermConstantUnion *value) {
                            if (value->isFloatingDomain()) {
                                return int(value->getConstArray()[0].getDConst());
                            }
                            else if (value->isIntegerDomain()) {
                                return value->getConstArray()[0].getIConst();
                            }
                            else {
                                return 0;
                            }
                        };
                        for (size_t i = 0, numComponents = values.size(); i < 4; i++) {
                            const int v = i < numComponents ? convertToInteger(values[i]) : 0;
                            setVector4i(ivec4, i, v);
                        }
                    }
                    break;
                case EbtFloat:
                    if (values.size() == 1) {
                        annotationPtr->value_case = FX9__EFFECT__ANNOTATION__VALUE_FVAL;
                        annotationPtr->fval = float(values[0]->getConstArray()[0].getDConst());
                    }
                    else {
                        Fx9__Effect__Vector4f *fvec4 = nullptr;
                        annotationPtr->value_case = FX9__EFFECT__ANNOTATION__VALUE_FVAL4;
                        fvec4 = annotationPtr->fval4 = allocate<Fx9__Effect__Vector4f>();
                        fx9__effect__vector4f__init(fvec4);
                        auto convertToFloat = [](const TIntermConstantUnion *value) {
                            if (value->isFloatingDomain()) {
                                return float(value->getConstArray()[0].getDConst());
                            }
                            else if (value->isIntegerDomain()) {
                                return float(value->getConstArray()[0].getIConst());
                            }
                            else {
                                return 0.0f;
                            }
                        };
                        for (size_t i = 0, numComponents = values.size(); i < 4; i++) {
                            const float v = i < numComponents ? convertToFloat(values[i]) : 0.0f;
                            setVector4f(fvec4, i, v);
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }
        else if (const TIntermConstantUnion *constantUnion = value->getAsConstantUnion()) {
            const TConstUnionArray &values = constantUnion->getConstArray();
            switch (constantUnion->getBasicType()) {
            case EbtBool:
                annotationPtr->value_case = FX9__EFFECT__ANNOTATION__VALUE_BVAL;
                annotationPtr->bval = values[0].getBConst();
                break;
            case EbtInt:
                if (values.size() == 1) {
                    annotationPtr->value_case = FX9__EFFECT__ANNOTATION__VALUE_IVAL;
                    annotationPtr->ival = values[0].getIConst();
                }
                else {
                    Fx9__Effect__Vector4i *ivec4 = nullptr;
                    annotationPtr->value_case = FX9__EFFECT__ANNOTATION__VALUE_IVAL4;
                    ivec4 = annotationPtr->ival4 = allocate<Fx9__Effect__Vector4i>();
                    fx9__effect__vector4i__init(ivec4);
                    for (int i = 0, numComponents = values.size(); i < 4; i++) {
                        const int v = i < numComponents ? values[i].getIConst() : 0;
                        setVector4i(ivec4, i, v);
                    }
                }
                break;
            case EbtFloat:
                if (values.size() == 1) {
                    annotationPtr->value_case = FX9__EFFECT__ANNOTATION__VALUE_FVAL;
                    annotationPtr->fval = float(values[0].getDConst());
                }
                else {
                    Fx9__Effect__Vector4f *fvec4 = nullptr;
                    annotationPtr->value_case = FX9__EFFECT__ANNOTATION__VALUE_FVAL4;
                    fvec4 = annotationPtr->fval4 = allocate<Fx9__Effect__Vector4f>();
                    fx9__effect__vector4f__init(fvec4);
                    for (int i = 0, numComponents = values.size(); i < 4; i++) {
                        const float v = i < numComponents ? float(values[i].getDConst()) : 0.0f;
                        setVector4f(fvec4, i, v);
                    }
                }
                break;
            default:
                break;
            }
        }
    }
}

void
Compiler::convertAllAnnotations(const ParserContext::AnnotationList &annotations, void *opaque, size_t *numAnnotations)
{
    Fx9__Effect__Annotation ***annotationsPtr = static_cast<Fx9__Effect__Annotation ***>(opaque), **p;
    if (!annotations.empty()) {
        *numAnnotations = annotations.size();
        p = allocateArray<Fx9__Effect__Annotation>(annotations.size());
        size_t index = 0;
        for (auto it = annotations.begin(), end = annotations.end(); it != end; ++it) {
            Fx9__Effect__Annotation *annotationPtr = nullptr;
            annotationPtr = allocate<Fx9__Effect__Annotation>();
            p[index++] = annotationPtr;
            fx9__effect__annotation__init(annotationPtr);
            convertAnnotation(it->m_name, it->m_value, annotationPtr);
        }
        *annotationsPtr = p;
    }
}

void
Compiler::convertAllParameterAnnotations(const TIntermTyped *annotationsNode, void *opaque)
{
    Fx9__Effect__Parameter *parameterPtr = static_cast<Fx9__Effect__Parameter *>(opaque);
    const TIntermSequence sequence = annotationsNode->getAsAggregate()->getSequence();
    ParserContext::AnnotationList annotations;
    for (auto it = sequence.begin(), end = sequence.end(); it != end; ++it) {
        if (const TIntermBinary *annotationNode = (*it)->getAsBinaryNode()) {
            const TIntermBinary *keyNode = annotationNode->getLeft()->getAsBinaryNode();
            ParserContext::Annotation annotation(keyNode->getRight()->getAsSymbolNode()->getName());
            annotation.m_value = annotationNode->getRight();
            annotations.push_back(annotation);
        }
    }
    convertAllAnnotations(annotations, &parameterPtr->annotations, &parameterPtr->n_annotations);
}

void
Compiler::convertMetadata(const TUnorderedMap<TString, TString> &value, void *opaque)
{
    Fx9__Effect__Effect *message = static_cast<Fx9__Effect__Effect *>(opaque);
    if (!value.empty()) {
        message->n_metadata = value.size();
        message->metadata = allocateArray<Fx9__Effect__Metadata>(message->n_metadata);
        size_t index = 0;
        for (auto it = value.begin(), end = value.end(); it != end; ++it) {
            Fx9__Effect__Metadata *metadataPtr = nullptr;
            metadataPtr = message->metadata[index++] = allocate<Fx9__Effect__Metadata>();
            fx9__effect__metadata__init(metadataPtr);
            copyString(it->first, &metadataPtr->name);
            copyString(it->second, &metadataPtr->value);
        }
    }
}

void
Compiler::convertIncludePathSet(const ParserContext &parser, void *opaque)
{
    Fx9__Effect__Effect *message = static_cast<Fx9__Effect__Effect *>(opaque);
    const auto &shaderSourcePathList = parser.includedShaderSourcePathList();
    size_t index = 0;
    if (!shaderSourcePathList.empty()) {
        message->n_includes = shaderSourcePathList.size();
        message->includes = allocateArray<Fx9__Effect__Include>(message->n_includes);
        for (auto it = shaderSourcePathList.begin(), end = shaderSourcePathList.end(); it != end; ++it) {
            Fx9__Effect__Include *include = message->includes[index++] = allocate<Fx9__Effect__Include>();
            fx9__effect__include__init(include);
            copyString(*it, &include->location);
        }
    }
}

bool
Compiler::generateSPVInstructions(const ParserContext::Pass::EntryPoint &entryPoint, const glslang::TString &source,
    ParserContext &parser, InstructionList &instructions, spv::SpvBuildLogger *logger, TInfoSink &sink)
{
    HlslParseContext *context = parser.parseContext();
    TBuiltInResource resources = {};
    getDefaultResourceLimit(resources);
    addContextSpecificSymbols(&resources, context->language, context->symbolTable);
    parser.setEntryPoint(entryPoint);
    TString preamble;
    for (const auto &it : m_macros) {
        const std::string &key = it.first, &value = it.second;
        preamble.append("#define ");
        preamble.append(key.c_str(), key.size());
        preamble.append(" ");
        preamble.append(value.c_str(), value.size());
        preamble.append("\n");
    }
    parser.execute(source, preamble);
    const bool ok = context->getNumErrors() == 0;
    if (ok) {
        glslang::SpvOptions options;
        glslang::GlslangToSpv(parser.intermediate(), instructions, logger, &options);
#if 0
        FILE *fp = fopen((entryPoint.first + ".spv").c_str(), "wb");
        fwrite(instructions.data(), instructions.size(), sizeof(instructions[0]), fp);
        fclose(fp);
#endif
    }
    else {
        sink = context->infoSink;
        parser.dump(std::cout);
    }
    context->symbolTable.pop(nullptr);
    return ok;
}

void
Compiler::saveAttributeMap(const InstructionList &instructions, std::unordered_map<uint32_t, std::string> &attributes)
{
    spirv_cross::Compiler tracer(instructions);
    for (auto it : tracer.get_active_interface_variables()) {
        auto type = tracer.get_type_from_variable(it);
        if (type.storage == spv::StorageClassInput || type.storage == spv::StorageClassOutput) {
            attributes.insert(std::make_pair(it, tracer.get_name(it)));
        }
    }
}

void
Compiler::restoreInterfaceVariableNames(
    EShLanguage language, const std::unordered_map<uint32_t, std::string> &attributes, spirv_cross::Compiler &compiler)
{
    if (m_enableOptimize) {
        for (auto it : compiler.get_active_interface_variables()) {
            auto type = compiler.get_type_from_variable(it);
            if (m_language == kLanguageTypeMSL && type.basetype == spirv_cross::SPIRType::Struct &&
                type.storage == spv::StorageClassUniform) {
                static const char *kMemberNames[] = {
                    "vs_uniforms_vec4", // EShLangVertex
                    nullptr, // EShLangTessControl
                    nullptr, // EShLangTessEvaluation
                    nullptr, // EShLangGeometry
                    "ps_uniforms_vec4", // EShLangFragment
                    nullptr, // EShLangCompute
                    nullptr // EShLangCount
                };
                compiler.set_name(it, m_metalShaderUniformBufferName);
                compiler.set_member_name(type.self, 0, kMemberNames[language]);
            }
            else if (type.storage == spv::StorageClassInput || type.storage == spv::StorageClassOutput) {
                auto it2 = attributes.find(it);
                if (it2 != attributes.end()) {
                    compiler.set_name(it, it2->second);
                }
            }
        }
    }
}

void
Compiler::optimizeShaderInstructions(
    const InstructionList &instructions, InstructionList &newInstructions, EffectProduct::LogSink &sink)
{
#if defined(FX9_ENABLE_OPTIMIZER)
    if (m_enableOptimize) {
        spvtools::Optimizer optimizer(SPV_ENV_UNIVERSAL_1_3);
        optimizer.RegisterPerformancePasses();
        optimizer.RegisterSizePasses();
        optimizer.SetMessageConsumer([&sink](spv_message_level_t level, const char * /* source */,
                                         const spv_position_t & /* position */, const char *message) {
            switch (level) {
            case SPV_MSG_FATAL: {
                sink.optimizer = sink.optimizer + "[FATAL] " + message;
                break;
            }
            case SPV_MSG_INTERNAL_ERROR: {
                sink.optimizer = sink.optimizer + "[INTERNAL] " + message;
                break;
            }
            case SPV_MSG_ERROR: {
                sink.optimizer = sink.optimizer + "[ERROR] " + message;
                break;
            }
            default:
                break;
            }
        });
        if (!optimizer.Run(instructions.data(), instructions.size(), &newInstructions)) {
            newInstructions.resize(instructions.size());
            std::copy(instructions.begin(), instructions.end(), newInstructions.begin());
        }
    }
    else
#endif
    {
        newInstructions.resize(instructions.size());
        std::copy(instructions.begin(), instructions.end(), newInstructions.begin());
    }
}

bool
Compiler::validateShaderSource(
    EShLanguage language, const std::string &shaderSource, std::string &message, std::unique_ptr<TShader> &outputShader)
{
    bool succeeded = false;
    if (m_enableValidation) {
        const std::string &inputSource = trimFragColorVariable(shaderSource);
        const char *sources[] = { inputSource.c_str() };
        const int lengths[] = { int(inputSource.size()) };
        static TBuiltInResource resources;
        getDefaultResourceLimit(resources);
        outputShader.reset(new TShader(language));
        outputShader->setStringsWithLengths(sources, lengths, 1);
        TPoolAllocator &allocator = GetThreadPoolAllocator();
        succeeded = outputShader->parse(&resources, m_version, m_profile, true, true, EShMsgDefault);
        SetThreadPoolAllocator(&allocator);
        if (!succeeded) {
            message = outputShader->getInfoLog();
            message.append("```\n");
            message.append(shaderSource);
            message.append("```");
        }
    }
    else {
        succeeded = true;
    }
    return succeeded;
}

void
Compiler::rebuildAllSymbolTables()
{
    BuiltInParsables builtInParseables(m_language);
    const SpvVersion &spv = createSpvVersion();
    builtInParseables.initialize(m_version, m_profile, spv);
    m_commonSymbolTable.reset(new glslang::TSymbolTable());
    m_fragmentSymbolTable.reset(new glslang::TSymbolTable());
    initializeBuiltInSymbolTable(builtInParseables.getCommonString(), EShLangVertex, *m_commonSymbolTable);
    initializeBuiltInSymbolTable(EShLangFragment, builtInParseables, *m_commonSymbolTable, *m_fragmentSymbolTable);
    m_fragmentSymbolTable->readOnly();
}

} /* namespace fx9 */
