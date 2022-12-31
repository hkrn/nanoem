/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/Effect.h"
#include "emapp/StringUtils.h"
#include "emapp/effect/Common.h"
#include "emapp/private/CommonInclude.h"

#include "../protoc/effect.pb-c.h"
#include "./EffectCommon.inl"

namespace nanoem {
namespace effect {
namespace dx9ms {

static inline bool
findString(const bx::StringView &line, const char *outPixelCandidate, bx::StringView &p)
{
    p = bx::strFindI(line, outPixelCandidate);
    return !p.isEmpty();
}

static inline void
setSymbolRegisterIndex(const Fx9__Effect__Dx9ms__Symbol *symbolPtr, nanoem_u32_t min, nanoem_u32_t max,
    RegisterIndex &index) NANOEM_DECL_NOEXCEPT
{
    if (index.m_type == nanoem_u32_t(-1)) {
        index.m_index = symbolPtr->register_index;
        index.m_count = symbolPtr->register_count;
        index.m_type = glm::clamp(Inline::saturateInt32U(symbolPtr->register_set), min, max);
    }
}

static void
setImageTypesFromSampler(
    const Fx9__Effect__Dx9ms__Shader *shaderPtr, sg_shader_image_desc *pixelShaderSamplers) NANOEM_DECL_NOEXCEPT
{
    const size_t numSamplers = shaderPtr->n_samplers;
    for (size_t i = 0; i < numSamplers; i++) {
        const Fx9__Effect__Dx9ms__Sampler *samplerPtr = shaderPtr->samplers[i];
        const nanoem_u32_t samplerIndex = Inline::saturateInt32(samplerPtr->index);
        if (samplerIndex < SG_MAX_SHADERSTAGE_IMAGES) {
            sg_shader_image_desc &desc = pixelShaderSamplers[samplerIndex];
            desc.name = samplerPtr->name;
            switch (static_cast<Fx9__Effect__Dx9ms__SamplerType>(samplerPtr->type)) {
            case FX9__EFFECT__DX9MS__SAMPLER_TYPE__SAMPLER_2D:
            default:
                desc.image_type = SG_IMAGETYPE_2D;
                break;
            case FX9__EFFECT__DX9MS__SAMPLER_TYPE__SAMPLER_CUBE:
                desc.image_type = SG_IMAGETYPE_CUBE;
                break;
            case FX9__EFFECT__DX9MS__SAMPLER_TYPE__SAMPLER_VOLUME:
                desc.image_type = SG_IMAGETYPE_3D;
                break;
            }
        }
    }
}

static void
setRegisterIndex(const Fx9__Effect__Dx9ms__Symbol *symbolPtr, RegisterIndexMap &registerIndices)
{
    const String name(symbolPtr->name);
    RegisterIndexMap::iterator it = registerIndices.find(name);
    if (it != registerIndices.end()) {
        setSymbolRegisterIndex(symbolPtr, FX9__EFFECT__DX9MS__REGISTER_SET__RS_BOOL,
            FX9__EFFECT__DX9MS__REGISTER_SET__RS_FLOAT4, it->second);
    }
    else {
        const RegisterIndex &index = createSymbolRegisterIndex(
            symbolPtr, FX9__EFFECT__DX9MS__REGISTER_SET__RS_BOOL, FX9__EFFECT__DX9MS__REGISTER_SET__RS_FLOAT4);
        registerIndices.insert(tinystl::make_pair(name, index));
    }
}

void
rewriteShaderCode(const Fx9__Effect__Dx9ms__Shader *shaderPtr, StringMap &shaderOutputVariables, String &newShaderCode)
{
    char tempBuffer[255], variableName[64], glPrefixedVariableName[64], outPixelCandidate[64] = { 0 };
    switch (shaderPtr->type) {
    case FX9__EFFECT__DX9MS__SHADER_TYPE__ST_VERTEX: {
        const size_t numAttributes = shaderPtr->n_attributes;
        for (size_t i = 0; i < numAttributes; i++) {
            const Fx9__Effect__Dx9ms__Attribute *attributePtr = shaderPtr->attributes[i];
            switch (attributePtr->usage) {
            case FX9__EFFECT__DX9MS__ATTRIBUTE_USAGE__AU_POSITION: {
                newShaderCode.append("attribute highp vec4 a_position;\n");
                StringUtils::format(tempBuffer, sizeof(tempBuffer), "#define %s a_position\n", attributePtr->name);
                newShaderCode.append(tempBuffer);
                break;
            }
            case FX9__EFFECT__DX9MS__ATTRIBUTE_USAGE__AU_NORMAL: {
                newShaderCode.append("attribute mediump vec4 a_normal;\n");
                StringUtils::format(tempBuffer, sizeof(tempBuffer), "#define %s a_normal\n", attributePtr->name);
                newShaderCode.append(tempBuffer);
                break;
            }
            case FX9__EFFECT__DX9MS__ATTRIBUTE_USAGE__AU_TEXCOORD: {
                StringUtils::format(
                    tempBuffer, sizeof(tempBuffer), "attribute highp vec4 a_texcoord%d;\n", attributePtr->index);
                newShaderCode.append(tempBuffer);
                StringUtils::format(tempBuffer, sizeof(tempBuffer), "#define %s a_texcoord%d\n", attributePtr->name,
                    attributePtr->index);
                newShaderCode.append(tempBuffer);
                break;
            }
            case FX9__EFFECT__DX9MS__ATTRIBUTE_USAGE__AU_COLOR: {
                StringUtils::format(
                    tempBuffer, sizeof(tempBuffer), "attribute lowp vec4 a_color%d;\n", attributePtr->index);
                newShaderCode.append(tempBuffer);
                StringUtils::format(
                    tempBuffer, sizeof(tempBuffer), "#define %s a_color%d\n", attributePtr->name, attributePtr->index);
                newShaderCode.append(tempBuffer);
                break;
            }
            case FX9__EFFECT__DX9MS__ATTRIBUTE_USAGE__AU_BLENDWEIGHT: {
                newShaderCode.append("attribute highp vec4 a_weight;\n");
                StringUtils::format(tempBuffer, sizeof(tempBuffer), "#define %s a_weight\n", attributePtr->name);
                newShaderCode.append(tempBuffer);
                break;
            }
            default:
                break;
            }
        }
        const size_t numOutputs = shaderPtr->n_outputs;
        for (size_t i = 0; i < numOutputs; i++) {
            const Fx9__Effect__Dx9ms__Attribute *outputPtr = shaderPtr->outputs[i];
            StringUtils::format(tempBuffer, sizeof(tempBuffer), "varying highp vec4 %s;\n", outputPtr->name);
            switch (outputPtr->usage) {
            case FX9__EFFECT__DX9MS__ATTRIBUTE_USAGE__AU_TEXCOORD: {
                newShaderCode.append(tempBuffer);
                StringUtils::format(tempBuffer, sizeof(tempBuffer), "gl_TexCoord[%d]", outputPtr->index);
                const String name(outputPtr->name);
                shaderOutputVariables.insert(tinystl::make_pair(String(tempBuffer), name));
                if (outputPtr->index == 0) {
                    shaderOutputVariables.insert(tinystl::make_pair(String("gl_TexCoord"), name));
                }
                break;
            }
            case FX9__EFFECT__DX9MS__ATTRIBUTE_USAGE__AU_COLOR: {
                newShaderCode.append(tempBuffer);
                switch (outputPtr->index) {
                case 0: {
                    shaderOutputVariables.insert(tinystl::make_pair(String("gl_Color"), String(outputPtr->name)));
                    break;
                }
                case 1: {
                    shaderOutputVariables.insert(
                        tinystl::make_pair(String("gl_SecondaryColor"), String(outputPtr->name)));
                    break;
                }
                default:
                    break;
                }
                break;
            }
            default:
                break;
            }
        }
        break;
    }
    case FX9__EFFECT__DX9MS__SHADER_TYPE__ST_PIXEL: {
        StringSet varyings;
        for (StringMap::const_iterator it = shaderOutputVariables.begin(), end = shaderOutputVariables.end(); it != end;
             ++it) {
            const String name(it->second);
            if (varyings.find(name) == varyings.end()) {
                StringUtils::format(tempBuffer, sizeof(tempBuffer), "varying highp vec4 %s; /* %s */\n", name.c_str(),
                    it->first.c_str());
                newShaderCode.append(tempBuffer);
                varyings.insert(name);
            }
        }
        break;
    }
    default:
        return;
    }
    const bool isVertexShader = shaderPtr->type == FX9__EFFECT__DX9MS__SHADER_TYPE__ST_VERTEX;
    const bool isPixelShader = shaderPtr->type == FX9__EFFECT__DX9MS__SHADER_TYPE__ST_PIXEL;
    const size_t numUniforms = shaderPtr->n_uniforms;
    nanoem_u32_t maxUniformIndices = 0;
    for (size_t i = 0; i < numUniforms; i++) {
        const Fx9__Effect__Dx9ms__Uniform *uniformPtr = shaderPtr->uniforms[i];
        maxUniformIndices = glm::max(maxUniformIndices, uniformPtr->index);
    }
    static const char kAttributeVSVPrefix[] = "attribute vec4 vs_v";
    static const char kUniformPrefix[] = "uniform vec4 ";
    static const char kOutPixelCandidatePrefix[] = "out vec4 ";
    static const char kDefineVSCPrefix[] = "#define vs_c";
    static const char kDefineVSOPrefix[] = "#define vs_o";
    static const char kDefinePSCPrefix[] = "#define ps_c";
    static const char kDefinePSVPrefix[] = "#define ps_v";
    static const char kVersionPrefix[] = "#version ";
    bx::LineReader reader(shaderPtr->code);
    while (!reader.isDone()) {
        const bx::StringView line = reader.next();
        const char *linePtr = line.getPtr();
        bool isLineProceeded = false;
        if (StringUtils::hasPrefix(line.getPtr(), kVersionPrefix)) {
            isLineProceeded = true;
        }
        else if (StringUtils::hasPrefix(linePtr, kUniformPrefix)) {
            const bx::StringView basePtr(line, sizeof(kDefineVSCPrefix) - 1), fromPtr = bx::strLTrimSpace(basePtr),
                                                                              toPtr = bx::strWord(fromPtr);
            StringUtils::copyString(variableName, fromPtr.getPtr(),
                glm::min(Inline::saturateInt32(sizeof(variableName)), toPtr.getLength()));
            StringUtils::format(
                tempBuffer, sizeof(tempBuffer), "%s%s[%d];\n", kUniformPrefix, variableName, maxUniformIndices + 1);
            newShaderCode.append(tempBuffer);
            isLineProceeded = true;
        }
        else if (isPixelShader) {
            bx::StringView p;
            if (StringUtils::hasPrefix(linePtr, kOutPixelCandidatePrefix)) {
                const char *start = linePtr + sizeof(kOutPixelCandidatePrefix) - 1;
                const bx::StringView block = bx::strFindBlock(start, '[', ']');
                if (!block.isEmpty()) {
                    StringUtils::copyString(outPixelCandidate, block.getPtr(), block.getLength());
                }
                isLineProceeded = true;
            }
            else if (StringUtils::hasPrefix(linePtr, kDefinePSCPrefix)) {
                const bx::StringView basePtr(line, sizeof(kDefinePSCPrefix) - 1);
                char *mutablePtr = nullptr;
                int uniformIndex = StringUtils::parseInteger(basePtr.getPtr(), &mutablePtr);
                const bx::StringView fromPtr = bx::strLTrimSpace(mutablePtr), toPtr = bx::strWord(fromPtr);
                StringUtils::copyString(variableName, fromPtr.getPtr(),
                    glm::min(Inline::saturateInt32(sizeof(variableName)), toPtr.getLength()));
                StringUtils::format(tempBuffer, sizeof(tempBuffer), "%s%d %s[%d]\n", kDefinePSCPrefix, uniformIndex,
                    variableName, uniformIndex);
                newShaderCode.append(tempBuffer);
                isLineProceeded = true;
            }
            else if (StringUtils::hasPrefix(linePtr, kDefinePSVPrefix)) {
                const bx::StringView basePtr(line, sizeof(kDefinePSCPrefix) - 1);
                char *mutablePtr = nullptr;
                int variableIndex = StringUtils::parseInteger(basePtr.getPtr(), &mutablePtr);
                StringUtils::format(variableName, sizeof(variableName), "ps_v%d", variableIndex);
                const bx::StringView fromPtr = bx::strLTrimSpace(mutablePtr);
                const char *strPtr = fromPtr.getPtr();
                while (strPtr != line.getTerm() &&
                    (bx::isAlphaNum(*strPtr) || *strPtr == '_' || *strPtr == '[' || *strPtr == ']')) {
                    strPtr++;
                }
                StringUtils::copyString(
                    glPrefixedVariableName, fromPtr.getPtr(), size_t(strPtr - fromPtr.getPtr() + 1));
                StringMap::const_iterator it = shaderOutputVariables.find(glPrefixedVariableName);
                if (it != shaderOutputVariables.end()) {
                    StringUtils::format(
                        tempBuffer, sizeof(tempBuffer), "#define %s %s\n", variableName, it->second.c_str());
                    newShaderCode.append(tempBuffer);
                }
                isLineProceeded = true;
            }
            else if (*outPixelCandidate && findString(line, outPixelCandidate, p)) {
                newShaderCode.append(linePtr, p.getPtr());
                bool found = !bx::findIdentifierMatch(outPixelCandidate, "_gl_FragData").isEmpty();
                newShaderCode.append(found ? "gl_FragData" : "gl_FragColor");
                newShaderCode.append(p.getPtr() + bx::strLen(outPixelCandidate), p.getTerm());
                isLineProceeded = true;
            }
        }
        else if (isVertexShader) {
            if (StringUtils::hasPrefix(linePtr, kDefineVSCPrefix)) {
                const bx::StringView basePtr(line, sizeof(kDefinePSCPrefix) - 1);
                char *mutablePtr = nullptr;
                int uniformIndex = StringUtils::parseInteger(basePtr.getPtr(), &mutablePtr);
                const bx::StringView fromPtr = bx::strLTrimSpace(mutablePtr), toPtr = bx::strWord(fromPtr);
                StringUtils::copyString(variableName, fromPtr.getPtr(),
                    glm::min(Inline::saturateInt32(sizeof(variableName)), toPtr.getLength()));
                StringUtils::format(tempBuffer, sizeof(tempBuffer), "%s%d %s[%d]\n", kDefineVSCPrefix, uniformIndex,
                    variableName, uniformIndex);
                newShaderCode.append(tempBuffer);
                isLineProceeded = true;
            }
            else if (StringUtils::hasPrefix(linePtr, kAttributeVSVPrefix) ||
                StringUtils::hasPrefix(linePtr, kDefineVSOPrefix)) {
                newShaderCode.append("// ");
                newShaderCode.append(linePtr, line.getTerm());
                newShaderCode.append("\n");
                isLineProceeded = true;
            }
        }
        if (!isLineProceeded && !line.isEmpty() && *line.getPtr() != '\r') {
            newShaderCode.append(linePtr, line.getTerm());
            newShaderCode.append("\n");
        }
    }
}

void
createShader(const Fx9__Effect__Dx9ms__Shader *shaderPtr, sg_shader_desc &desc, String &newShaderCode,
    StringMap &shaderOutputVariables)
{
    if (shaderPtr->code && bx::strLen(shaderPtr->code) > 0) {
        rewriteShaderCode(shaderPtr, shaderOutputVariables, newShaderCode);
        switch (shaderPtr->type) {
        case FX9__EFFECT__DX9MS__SHADER_TYPE__ST_PIXEL: {
            desc.fs.source = newShaderCode.c_str();
            break;
        }
        case FX9__EFFECT__DX9MS__SHADER_TYPE__ST_VERTEX: {
            desc.vs.source = newShaderCode.c_str();
            break;
        }
        default:
            nanoem_assert(false, "must NOT reach here");
            break;
        }
    }
}

void
retrieveShaderSymbols(const Fx9__Effect__Dx9ms__Shader *shaderPtr, RegisterIndexMap &registerIndices,
    UniformBufferOffsetMap &uniformBufferOffsetMap)
{
    const size_t numUniforms = shaderPtr->n_uniforms;
    for (size_t i = 0; i < numUniforms; i++) {
        const Fx9__Effect__Dx9ms__Uniform *uniformPtr = shaderPtr->uniforms[i];
        uniformBufferOffsetMap.insert(tinystl::make_pair(uniformPtr->index, uniformPtr->index));
    }
    const size_t numSymbols = shaderPtr->n_symbols;
    for (size_t i = 0; i < numSymbols; i++) {
        const Fx9__Effect__Dx9ms__Symbol *symbolPtr = shaderPtr->symbols[i];
        setRegisterIndex(symbolPtr, registerIndices);
    }
}

void
retrievePixelShaderSamplers(const Fx9__Effect__Dx9ms__Pass *pass, sg_shader_image_desc *shaderSamplers,
    ImageDescriptionMap &textureDescriptions, SamplerRegisterIndexMap &shaderRegisterIndices)
{
    Fx9__Effect__Dx9ms__Texture *const *textures = pass->textures;
    const Fx9__Effect__Dx9ms__Shader *shaderPtr = pass->pixel_shader;
    const size_t numTextures = pass->n_textures, numSamplers = shaderPtr->n_samplers;
    setImageTypesFromSampler(shaderPtr, shaderSamplers);
    for (size_t i = 0; i < numTextures; i++) {
        const Fx9__Effect__Dx9ms__Texture *texturePtr = textures[i];
        const String name(texturePtr->name);
        const nanoem_u32_t samplerIndex = texturePtr->sampler_index;
        if (samplerIndex < numSamplers) {
            if (shaderRegisterIndices.find(name) == shaderRegisterIndices.end()) {
                SamplerRegisterIndex index;
                index.m_indices.push_back(samplerIndex);
                index.m_type = FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_TEXTURE;
                shaderRegisterIndices.insert(tinystl::make_pair(name, index));
            }
            if (textureDescriptions.find(name) == textureDescriptions.end()) {
                sg_image_desc desc;
                Inline::clearZeroMemory(desc);
                convertImageDescription<Fx9__Effect__Dx9ms__Texture, Fx9__Effect__Dx9ms__SamplerState>(
                    texturePtr, desc);
                if (samplerIndex < SG_MAX_SHADERSTAGE_IMAGES) {
                    desc.type = shaderSamplers[samplerIndex].image_type;
                }
                textureDescriptions.insert(tinystl::make_pair(name, desc));
            }
        }
    }
}

void
retrieveVertexShaderSamplers(const Fx9__Effect__Dx9ms__Pass *pass, sg_shader_image_desc *shaderSamplers,
    ImageDescriptionMap &textureDescriptions, SamplerRegisterIndexMap &shaderRegisterIndices)
{
    Fx9__Effect__Dx9ms__Texture *const *textures = pass->vertex_textures;
    const Fx9__Effect__Dx9ms__Shader *shaderPtr = pass->vertex_shader;
    const size_t numTextures = pass->n_vertex_textures, numSamplers = shaderPtr->n_samplers;
    setImageTypesFromSampler(shaderPtr, shaderSamplers);
    for (size_t i = 0; i < numTextures; i++) {
        const Fx9__Effect__Dx9ms__Texture *texturePtr = textures[i];
        const String name(texturePtr->name);
        const nanoem_u32_t samplerIndex = texturePtr->sampler_index;
        if (samplerIndex < numSamplers) {
            if (shaderRegisterIndices.find(name) == shaderRegisterIndices.end()) {
                SamplerRegisterIndex index;
                index.m_indices.push_back(samplerIndex);
                index.m_type = FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_TEXTURE;
                shaderRegisterIndices.insert(tinystl::make_pair(name, index));
            }
            if (textureDescriptions.find(name) == textureDescriptions.end()) {
                sg_image_desc desc;
                Inline::clearZeroMemory(desc);
                convertImageDescription<Fx9__Effect__Dx9ms__Texture, Fx9__Effect__Dx9ms__SamplerState>(
                    texturePtr, desc);
                if (samplerIndex < SG_MAX_SHADERSTAGE_IMAGES) {
                    desc.type = shaderSamplers[samplerIndex].image_type;
                }
                textureDescriptions.insert(tinystl::make_pair(name, desc));
            }
        }
    }
}

void
parsePreshader(const Fx9__Effect__Dx9ms__Shader *shaderPtr, Preshader &preshader, GlobalUniform::Buffer &buffer,
    RegisterIndexMap &registerIndices)
{
    if (const Fx9__Effect__Dx9ms__Preshader *preshaderPtr = shaderPtr->preshader) {
        const size_t numInstructions = preshaderPtr->n_instructions;
        preshader.m_numTemporaryRegisters = preshaderPtr->num_temporary_registers;
        preshader.m_instructions.resize(numInstructions);
        for (size_t i = 0; i < numInstructions; i++) {
            const Fx9__Effect__Dx9ms__Instruction *instructionPtr = preshaderPtr->instructions[i];
            const nanoem_u32_t numOperands = Inline::saturateInt32U(instructionPtr->n_operands);
            Preshader::Instruction &instruction = preshader.m_instructions[i];
            instruction.m_numElements = glm::clamp(instructionPtr->num_elements, 0u, 4u);
            instruction.m_opcode = instructionPtr->opcode;
            instruction.m_operands.resize(numOperands);
            for (nanoem_u32_t j = 0; j < numOperands; j++) {
                const Fx9__Effect__Dx9ms__Operand *operandPtr = instructionPtr->operands[j];
                Preshader::Operand &operand = instruction.m_operands[j];
                operand.m_index = operandPtr->index;
                operand.m_type = operandPtr->type;
            }
        }
        const size_t numSymbols = preshaderPtr->n_symbols;
        preshader.m_symbols.resize(numSymbols);
        for (size_t i = 0; i < numSymbols; i++) {
            const Fx9__Effect__Dx9ms__Symbol *symbolPtr = preshaderPtr->symbols[i];
            Preshader::Symbol &symbol = preshader.m_symbols[i];
            symbol.m_name = symbolPtr->name;
            symbol.m_count = symbolPtr->register_count;
            symbol.m_index = symbolPtr->register_index;
            symbol.m_set = symbolPtr->register_set;
            setRegisterIndex(symbolPtr, registerIndices);
        }
        countRegisterSet(preshaderPtr->symbols, preshaderPtr->n_symbols, buffer);
        const size_t numLiterals = preshaderPtr->n_literals;
        preshader.m_literals.resize(numLiterals);
        for (size_t i = 0; i < numLiterals; i++) {
            nanoem_f64_t literal = preshaderPtr->literals[i];
            preshader.m_literals[i] = nanoem_f32_t(literal);
        }
    }
}

} /* namespace dx9ms */
} /* namespace effect */
} /* namespace nanoem */
