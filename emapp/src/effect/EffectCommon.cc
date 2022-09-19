/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/effect/Common.h"

#include "emapp/Accessory.h"
#include "emapp/Constants.h"
#include "emapp/Effect.h"
#include "emapp/ImageLoader.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/effect/GlobalUniform.h"
#include "emapp/effect/RenderTargetNormalizer.h"
#include "emapp/internal/BlitPass.h"
#include "emapp/private/CommonInclude.h"

#include "glm/gtc/matrix_inverse.hpp"
#include "sokol/sokol_time.h"

#include "../protoc/effect.pb-c.h"

#include <ctype.h> /* tolower */

namespace nanoem {
namespace effect {
namespace {

static void
appendSwizzle(size_t numElements, int offset, String &s)
{
    switch (numElements) {
    case 1: {
        switch (offset) {
        case 0:
            s.append(".x");
            break;
        case 1:
            s.append(".y");
            break;
        case 2:
            s.append(".z");
            break;
        case 3:
            s.append(".w");
            break;
        default:
            break;
        }
        break;
    }
    case 2: {
        s.append(".xy");
        break;
    }
    case 3: {
        s.append(".xyz");
        break;
    }
    case 4: {
        s.append(".xyzw");
        break;
    }
    default:
        break;
    }
}

static void
setMipFilter(nanoem_u32_t value, sg_filter &filter) NANOEM_DECL_NOEXCEPT
{
    bool linear = filter == SG_FILTER_LINEAR;
    switch (value) {
    case 1: { /* D3DTEXF_POINT */
        filter = linear ? SG_FILTER_LINEAR_MIPMAP_NEAREST : SG_FILTER_NEAREST_MIPMAP_NEAREST;
        break;
    }
    case 2: { /* D3DTEXF_LINEAR */
        filter = linear ? SG_FILTER_LINEAR_MIPMAP_LINEAR : SG_FILTER_NEAREST_MIPMAP_LINEAR;
        break;
    }
    default:
        break;
    }
}

} /* namespace anonymous */

PipelineDescriptor::Stencil::Stencil()
    : m_hasCompareFunc(false)
    , m_hasPassOp(false)
    , m_hasFailOp(false)
    , m_hasDepthFailOp(false)
{
}

PipelineDescriptor::Stencil::~Stencil()
{
}

PipelineDescriptor::PipelineDescriptor()
    : m_hasBlendEnabled(false)
    , m_hasBlendSourceFactorRGB(false)
    , m_hasBlendDestFactorRGB(false)
    , m_hasBlendSourceFactorAlpha(false)
    , m_hasBlendDestFactorAlpha(false)
    , m_hasBlendOpRGB(false)
    , m_hasBlendOpAlpha(false)
    , m_hasColorWriteMask(false)
    , m_hasDepthCompareFunc(false)
    , m_hasDepthWriteEnabled(false)
    , m_hasStencilEnabled(false)
    , m_hasStencilRef(false)
    , m_hasStencilReadMask(false)
    , m_hasStencilWriteMask(false)
{
    Inline::clearZeroMemory(m_body);
}

PipelineDescriptor::PipelineDescriptor(const PipelineDescriptor &source)
    : m_stencilFront(source.m_stencilFront)
    , m_stencilBack(source.m_stencilBack)
    , m_hasBlendEnabled(source.m_hasBlendEnabled)
    , m_hasBlendSourceFactorRGB(source.m_hasBlendSourceFactorRGB)
    , m_hasBlendDestFactorRGB(source.m_hasBlendDestFactorRGB)
    , m_hasBlendSourceFactorAlpha(source.m_hasBlendSourceFactorAlpha)
    , m_hasBlendDestFactorAlpha(source.m_hasBlendDestFactorAlpha)
    , m_hasBlendOpRGB(source.m_hasBlendOpRGB)
    , m_hasBlendOpAlpha(source.m_hasBlendOpAlpha)
    , m_hasColorWriteMask(source.m_hasColorWriteMask)
    , m_hasDepthCompareFunc(source.m_hasDepthCompareFunc)
    , m_hasDepthWriteEnabled(source.m_hasDepthWriteEnabled)
    , m_hasStencilEnabled(source.m_hasStencilEnabled)
    , m_hasStencilRef(source.m_hasStencilRef)
    , m_hasStencilReadMask(source.m_hasStencilReadMask)
    , m_hasStencilWriteMask(source.m_hasStencilWriteMask)
{
    memcpy(&m_body, &source.m_body, sizeof(m_body));
}

PipelineDescriptor::~PipelineDescriptor()
{
}

AnnotationValue::AnnotationValue()
    : m_bool(false)
    , m_int(0)
    , m_float(0)
    , m_vector(0)
{
}

AnnotationValue::~AnnotationValue() NANOEM_DECL_NOEXCEPT
{
}

nanoem_f32_t
AnnotationValue::toFloat() const NANOEM_DECL_NOEXCEPT
{
    nanoem_f32_t value = 0.0f;
    if (m_float != 0) {
        value = m_float;
    }
    else if (m_int != 0) {
        value = static_cast<nanoem_f32_t>(m_int);
    }
    else if (m_bool) {
        value = 1.0f;
    }
    return value;
}

int
AnnotationValue::toInt() const NANOEM_DECL_NOEXCEPT
{
    int value = 0;
    if (m_int != 0) {
        value = m_int;
    }
    else if (m_float != 0) {
        value = static_cast<int>(m_float);
    }
    else if (m_bool) {
        value = 1;
    }
    return value;
}

bool
AnnotationValue::toBool() const NANOEM_DECL_NOEXCEPT
{
    return m_bool || m_int > 0 || m_float > 0;
}

AnnotationMap::const_iterator
AnnotationMap::findAnnotation(const char *const key) const NANOEM_DECL_NOEXCEPT
{
    char loweredKey[128], *ptr = loweredKey;
    const_iterator it = end();
    nanoem_parameter_assert(StringUtils::length(key) < sizeof(loweredKey), "must be less than 128bytes");
    if (StringUtils::length(key) < sizeof(loweredKey)) {
        const char *p = key;
        while (char c = *p++) {
            *ptr++ = ::tolower(c);
        }
        *ptr = 0;
        it = find(loweredKey);
    }
    return it;
}

String
AnnotationMap::stringAnnotation(const char *key, const String &defv) const
{
    const_iterator it = findAnnotation(key);
    if (it != end()) {
        return it->second.m_string;
    }
    return defv;
}

int
AnnotationMap::triBooleanAnnotation(const char *key, int defv) const NANOEM_DECL_NOEXCEPT
{
    const_iterator it = findAnnotation(key);
    if (it != end()) {
        return (it->second.m_bool || StringUtils::equals(it->second.m_string.c_str(), kBooleanTrueValueLiteral)) ? 1
                                                                                                                 : 0;
    }
    return defv;
}

int
AnnotationMap::integerAnnotation(const char *key, int defv) const NANOEM_DECL_NOEXCEPT
{
    const_iterator it = findAnnotation(key);
    if (it != end()) {
        return it->second.m_int;
    }
    return defv;
}

nanoem_f32_t
AnnotationMap::floatAnnotation(const char *key, nanoem_f32_t defv) const NANOEM_DECL_NOEXCEPT
{
    const_iterator it = findAnnotation(key);
    if (it != end()) {
        return it->second.m_float;
    }
    return defv;
}

RegisterIndex::RegisterIndex()
    : m_index(0)
    , m_count(0)
    , m_type(nanoem_u32_t(-1))
    , m_flags(0)
{
}

RegisterIndex::RegisterIndex(nanoem_u32_t index, nanoem_u32_t type)
    : m_index(index)
    , m_count(1)
    , m_type(type)
    , m_flags(0)
{
}

RegisterIndex::~RegisterIndex() NANOEM_DECL_NOEXCEPT
{
}

SamplerRegisterIndex::SamplerRegisterIndex()
    : m_type(0)
    , m_flags(0)
{
}

SamplerRegisterIndex::~SamplerRegisterIndex() NANOEM_DECL_NOEXCEPT
{
}

ScriptIndex::ScriptIndex()
    : m_techniqueIndex(0)
    , m_passIndex(0)
{
}

ScriptIndex::ScriptIndex(size_t techniqueIndex, size_t passIndex)
    : m_techniqueIndex(techniqueIndex)
    , m_passIndex(passIndex)
{
}

ScriptIndex::~ScriptIndex() NANOEM_DECL_NOEXCEPT
{
}

Preshader::Operand::Operand()
    : m_index(0)
    , m_type(0)
{
}

Preshader::Operand::~Operand() NANOEM_DECL_NOEXCEPT
{
}

Preshader::Instruction::Instruction()
    : m_opcode(0)
    , m_numElements(0)
{
}

Preshader::Instruction::~Instruction() NANOEM_DECL_NOEXCEPT
{
}

String
Preshader::Operand::toString(const Preshader::Instruction *instructionPtr, const Preshader *shaderPtr) const
{
    String s;
    switch (static_cast<Fx9__Effect__Dx9ms__OperandType>(m_type)) {
    case FX9__EFFECT__DX9MS__OPERAND_TYPE__PSOT_INPUT: {
        const char *name = "nullptr";
        const size_t numSymbols = shaderPtr->m_symbols.size();
        size_t index = m_index / 4;
        for (size_t i = 0; i < numSymbols; i++) {
            const Symbol &symbol = shaderPtr->m_symbols[i];
            if (symbol.m_index == index) {
                name = symbol.m_name.c_str();
            }
        }
        s.append(name);
        appendSwizzle(instructionPtr->m_numElements, m_index % 4, s);
        break;
    }
    case FX9__EFFECT__DX9MS__OPERAND_TYPE__PSOT_LITERAL: {
        const nanoem_f64_t v = shaderPtr->m_literals[m_index];
        switch (instructionPtr->m_numElements) {
        case 4: {
            StringUtils::format(s, "(%f, %f, %f, %f)", v, v, v, v);
            break;
        }
        case 3: {
            StringUtils::format(s, "(%f, %f, %f)", v, v, v);
            break;
        }
        case 2: {
            StringUtils::format(s, "(%f, %f)", v, v);
            break;
        }
        case 1: {
            StringUtils::format(s, "(%f)", v);
            break;
        }
        default:
            break;
        }
        break;
    }
    case FX9__EFFECT__DX9MS__OPERAND_TYPE__PSOT_OUTPUT: {
        StringUtils::format(s, "o[%d]", m_index / 4);
        appendSwizzle(instructionPtr->m_numElements, m_index % 4, s);
        break;
    }
    case FX9__EFFECT__DX9MS__OPERAND_TYPE__PSOT_TEMP: {
        StringUtils::format(s, "r[%d]", m_index / 4);
        appendSwizzle(instructionPtr->m_numElements, m_index % 4, s);
        break;
    }
    default:
        break;
    }
    return s;
}

const char *
Preshader::Instruction::opcodeString() const
{
    const nanoem_u32_t opcode = m_opcode >= FX9__EFFECT__DX9MS__OPCODE__PSO_SCALAR_OPS
        ? m_opcode + (FX9__EFFECT__DX9MS__OPCODE__PSO_MIN_SCALAR - FX9__EFFECT__DX9MS__OPCODE__PSO_SCALAR_OPS)
        : m_opcode;
    switch (static_cast<Fx9__Effect__Dx9ms__Opcode>(opcode)) {
    case FX9__EFFECT__DX9MS__OPCODE__PSO_ACOS:
        return "ACOS";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_ADD:
        return "ADD";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_ADD_SCALAR:
        return "ADD_SCALAR";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_ASIN:
        return "ASIN";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_ATAN:
        return "ATAN";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_ATAN2:
        return "ATAN2";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_ATAN2_SCALAR:
        return "ATAN2_SCALAR";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_CMP:
        return "CMP";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_COS:
        return "COS";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_DIV:
        return "DIV";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_DIV_SCALAR:
        return "DIV_SCALAR";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_DOT:
        return "DOT";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_DOT_SCALAR:
        return "DOT_SCALAR";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_EXP:
        return "EXP";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_FRC:
        return "FRC";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_GE:
        return "GE";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_GE_SCALAR:
        return "GE_SCALAR";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_LOG:
        return "LOG";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_LT:
        return "LT";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_LT_SCALAR:
        return "LT_SCALAR";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_MAX:
        return "MAX";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_MAX_SCALAR:
        return "MAX_SCALAR";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_MIN:
        return "MIN";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_MIN_SCALAR:
        return "MIN_SCALAR";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_MOV:
        return "MOV";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_MOVC:
        return "MOVC";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_MUL:
        return "MUL";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_MUL_SCALAR:
        return "MUL_SCALAR";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_NEG:
        return "NEG";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_NOISE:
        return "NOISE";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_NOISE_SCALAR:
        return "NOISE_SCALAR";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_NOP:
        return "NOP";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_RCP:
        return "RCP";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_RSQ:
        return "RSQ";
    case FX9__EFFECT__DX9MS__OPCODE__PSO_SIN:
        return "SIN";
    default:
        return "(unknown)";
    }
}

String
Preshader::Instruction::toString(const Preshader *shaderPtr) const
{
    String s;
    const nanoem_u32_t opcode = m_opcode >= FX9__EFFECT__DX9MS__OPCODE__PSO_SCALAR_OPS
        ? m_opcode + (FX9__EFFECT__DX9MS__OPCODE__PSO_MIN_SCALAR - FX9__EFFECT__DX9MS__OPCODE__PSO_SCALAR_OPS)
        : m_opcode;
    switch (static_cast<Fx9__Effect__Dx9ms__Opcode>(opcode)) {
    case FX9__EFFECT__DX9MS__OPCODE__PSO_ACOS:
        StringUtils::format(s, "%s = acos(%s)", m_operands[1].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_ADD:
        StringUtils::format(s, "%s = %s + %s", m_operands[2].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_ADD_SCALAR:
        StringUtils::format(s, "%s = %s + %s", m_operands[2].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_ASIN:
        StringUtils::format(s, "%s = asin(%s)", m_operands[1].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_ATAN:
        StringUtils::format(s, "%s = atan(%s)", m_operands[1].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_ATAN2:
        StringUtils::format(s, "%s = atan2(%s, %s)", m_operands[2].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_ATAN2_SCALAR:
        StringUtils::format(s, "%s = atan2(%s, %s)", m_operands[2].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_CMP:
        StringUtils::format(s, "%s = %s >= 0.0 ? %s : %s", m_operands[3].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str(),
            m_operands[2].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_COS:
        StringUtils::format(s, "%s = cos(%s)", m_operands[1].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_DIV:
        StringUtils::format(s, "%s = %s / %s", m_operands[2].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_DIV_SCALAR:
        StringUtils::format(s, "%s = %s / %s", m_operands[2].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_DOT:
        StringUtils::format(s, "%s = dot(%s, %s)", m_operands[2].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_DOT_SCALAR:
        StringUtils::format(s, "%s = dot(%s, %s)", m_operands[2].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_EXP:
        StringUtils::format(s, "%s = exp(%s)", m_operands[1].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_FRC:
        StringUtils::format(s, "%s = frac(%s)", m_operands[1].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_GE:
        StringUtils::format(s, "%s = %s >= %s", m_operands[2].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_GE_SCALAR:
        StringUtils::format(s, "%s = %s >= %s", m_operands[2].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_RSQ:
        StringUtils::format(s, "%s = inversesqrt(%s)", m_operands[1].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_NEG:
        StringUtils::format(s, "%s = -%s", m_operands[1].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_MAX_SCALAR:
        StringUtils::format(s, "%s = max(%s, %s)", m_operands[2].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_MUL_SCALAR:
        StringUtils::format(s, "%s = %s * %s", m_operands[2].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_LOG:
        StringUtils::format(s, "%s = log(%s)", m_operands[1].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_MIN:
        StringUtils::format(s, "%s = min(%s, %s)", m_operands[2].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_MAX:
        StringUtils::format(s, "%s = max(%s, %s)", m_operands[2].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_LT:
        StringUtils::format(s, "%s = %s < %s", m_operands[2].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_SIN:
        StringUtils::format(s, "%s = sin(%s)", m_operands[1].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_LT_SCALAR:
        StringUtils::format(s, "%s = %s < %s", m_operands[2].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_MIN_SCALAR:
        StringUtils::format(s, "%s = min(%s, %s)", m_operands[2].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_MOV:
        StringUtils::format(s, "%s = %s", m_operands[1].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_MOVC:
        StringUtils::format(s, "%s = %s", m_operands[1].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_MUL:
        StringUtils::format(s, "%s = %s * %s", m_operands[2].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str(), m_operands[1].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_NOISE:
        StringUtils::format(s, "%s = noise(%s)", m_operands[1].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_NOISE_SCALAR:
        StringUtils::format(s, "%s = noise(%s)", m_operands[1].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_NOP:
        StringUtils::format(s, "(NOP)");
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_RCP:
        StringUtils::format(s, "%s = inverse(%s)", m_operands[1].toString(this, shaderPtr).c_str(),
            m_operands[0].toString(this, shaderPtr).c_str());
        break;
    case FX9__EFFECT__DX9MS__OPCODE__PSO_SCALAR_OPS:
    default:
        StringUtils::format(s, "(UNKNOWN)");
        break;
    }
    return s;
}

Preshader::Symbol::Symbol()
    : m_count(0)
    , m_index(0)
    , m_set(0)
{
}

Preshader::Symbol::~Symbol() NANOEM_DECL_NOEXCEPT
{
}

Preshader::Preshader()
    : m_numTemporaryRegisters(0)
{
}

Preshader::~Preshader() NANOEM_DECL_NOEXCEPT
{
}

PreshaderPair::PreshaderPair()
{
}

PreshaderPair::PreshaderPair(const Preshader &v, const Preshader &p)
    : vertex(v)
    , pixel(p)
{
}

PreshaderPair::PreshaderPair(const PreshaderPair &value)
    : vertex(value.vertex)
    , pixel(value.pixel)
{
}

PreshaderPair::~PreshaderPair() NANOEM_DECL_NOEXCEPT
{
}

UIWidgetParameter::UIWidgetParameter(ParameterType parameterType, const AnnotationMap &annotations,
    GlobalUniform::Vector4List *valuePtr, nanoem_unicode_string_factory_t *factory)
    : m_widgetType(UIWidgetParameter::kUIWidgetTypeMaxEnum)
    , m_parameterType(parameterType)
    , m_valuePtr(valuePtr)
    , m_visible(true)
{
    AnnotationMap::const_iterator it = annotations.findAnnotation("UIName");
    if (it != annotations.end()) {
        StringUtils::getUtf8String(it->second.m_string, NANOEM_CODEC_TYPE_SJIS, factory, m_name);
    }
    AnnotationMap::const_iterator widgetType = annotations.findAnnotation("UIWidget");
    AnnotationMap::const_iterator visible = annotations.findAnnotation("UIVisible");
    if (widgetType != annotations.end()) {
        const String &widgetString = widgetType->second.m_string;
        if (StringUtils::equals(widgetString.c_str(), "Color")) {
            m_widgetType = UIWidgetParameter::kUIWidgetTypeColor;
        }
        else if (StringUtils::equals(widgetString.c_str(), "Numeric")) {
            m_widgetType = UIWidgetParameter::kUIWidgetTypeNumeric;
        }
        else if (StringUtils::equals(widgetString.c_str(), "Slider")) {
            m_widgetType = UIWidgetParameter::kUIWidgetTypeSlider;
        }
        else if (StringUtils::equals(widgetString.c_str(), "Spinner")) {
            m_widgetType = UIWidgetParameter::kUIWidgetTypeSpinner;
        }
    }
    else if (visible != annotations.end() && visible->second.toBool()) {
        m_widgetType = UIWidgetParameter::kUIWidgetTypeCheckbox;
    }
    AnnotationMap::const_iterator it3 = annotations.findAnnotation("UIMin");
    if (it3 != annotations.end()) {
        range.x = it3->second.toFloat();
    }
    AnnotationMap::const_iterator it4 = annotations.findAnnotation("UIMax");
    if (it4 != annotations.end()) {
        range.y = it4->second.toFloat();
    }
    if (range.x > range.y) {
        nanoem_f32_t value = range.y;
        range.y = range.x;
        range.x = value;
    }
    AnnotationMap::const_iterator it5 = annotations.findAnnotation("UIVisible");
    if (it5 != annotations.end()) {
        m_visible = it5->second.toBool();
    }
    AnnotationMap::const_iterator it6 = annotations.findAnnotation("UIHelp");
    if (it6 != annotations.end()) {
        m_description = it6->second.m_string;
    }
}

UIWidgetParameter::~UIWidgetParameter() NANOEM_DECL_NOEXCEPT
{
}

void
Preshader::execute(const GlobalUniform::Buffer &inputBuffer, GlobalUniform::Buffer &outputBuffer) const
{
    const size_t numInstructions = m_instructions.size();
    tinystl::vector<nanoem_f32_t, TinySTLAllocator> temp(m_numTemporaryRegisters);
    Vector4 src[4], dst;
    nanoem_u32_t soffset[4];
    for (size_t i = 0; i < numInstructions; i++) {
        const Instruction &instruction = m_instructions[i];
        const size_t numOperands = instruction.m_operands.size();
        const bool isScalarOp = instruction.m_opcode >= FX9__EFFECT__DX9MS__OPCODE__PSO_SCALAR_OPS;
        nanoem_f32_t *outputPtr = nullptr;
        src[0] = src[1] = src[2] = src[3] = dst = Constants::kZeroV4;
        soffset[0] = soffset[1] = soffset[2] = soffset[3] = 0;
        nanoem_assert(numOperands <= 4, "must NOT be greater than 4");
        for (size_t j = 0; j < numOperands; j++) {
            const Preshader::Operand &operand = instruction.m_operands[j];
            const nanoem_u32_t scalarIndex = operand.m_index;
            switch (static_cast<Fx9__Effect__Dx9ms__OperandType>(operand.m_type)) {
            case FX9__EFFECT__DX9MS__OPERAND_TYPE__PSOT_INPUT: {
                const nanoem_u32_t vec4Index = scalarIndex / 4;
                nanoem_assert(vec4Index < inputBuffer.m_float4.size(), "must NOT be out of bounds");
                if (vec4Index < inputBuffer.m_float4.size()) {
                    const Vector4 inputPtr(inputBuffer.m_float4[vec4Index]);
                    src[j] = inputPtr;
                    soffset[j] = scalarIndex % 4;
                }
                break;
            }
            case FX9__EFFECT__DX9MS__OPERAND_TYPE__PSOT_LITERAL: {
                nanoem_assert(scalarIndex < m_literals.size(), "must NOT be out of bounds");
                if (scalarIndex < m_literals.size()) {
                    const nanoem_f32_t *literalPtr = &m_literals[scalarIndex];
                    src[j] = Vector4(*literalPtr);
                    soffset[j] = 0;
                }
                break;
            }
            case FX9__EFFECT__DX9MS__OPERAND_TYPE__PSOT_OUTPUT: {
                const nanoem_u32_t vec4Index = scalarIndex / 4;
                nanoem_assert(vec4Index < outputBuffer.m_float4.size(), "must NOT be out of bounds");
                if (vec4Index < outputBuffer.m_float4.size()) {
                    src[j] = outputBuffer.m_float4[vec4Index];
                    soffset[j] = scalarIndex % 4;
                    if (j == numOperands - 1) {
                        outputPtr = &outputBuffer.m_float4[vec4Index][scalarIndex % 4];
                    }
                }
                break;
            }
            case FX9__EFFECT__DX9MS__OPERAND_TYPE__PSOT_TEMP: {
                nanoem_assert(scalarIndex < temp.size(), "must NOT be out of bounds");
                if (scalarIndex < temp.size()) {
                    src[j] = isScalarOp ? Vector4(temp[scalarIndex]) : glm::make_vec4(&temp[scalarIndex]);
                    soffset[j] = scalarIndex % 4;
                    if (j == numOperands - 1) {
                        outputPtr = &temp[scalarIndex];
                    }
                }
                break;
            }
            default:
                nanoem_assert(false, "must NOT reach here");
                break;
            }
        }
        const nanoem_u32_t opcode = isScalarOp ? instruction.m_opcode +
                (FX9__EFFECT__DX9MS__OPCODE__PSO_MIN_SCALAR - FX9__EFFECT__DX9MS__OPCODE__PSO_SCALAR_OPS)
                                               : instruction.m_opcode;
        const nanoem_u32_t numElements = instruction.m_numElements;
        nanoem_assert(numElements <= 4, "must NOT be greater than 4");
        // fprintf(stderr, "INS[%d]: %s\n", i, instruction.toString(this).c_str());
        if (opcode == FX9__EFFECT__DX9MS__OPCODE__PSO_DOT) {
            switch (numElements) {
            case 4:
                dst = Vector4(glm::dot(src[0], src[1]));
                break;
            case 3:
                dst = Vector4(glm::dot(Vector3(src[0]), Vector3(src[1])));
                break;
            case 2:
                dst = Vector4(glm::dot(Vector2(src[0]), Vector2(src[1])));
                break;
            case 1:
                dst = Vector4(glm::dot(src[0].x, src[1].x));
                break;
            default:
                break;
            }
        }
        else {
            for (nanoem_u32_t j = 0; j < numElements; j++) {
                nanoem_f32_t src0 = src[0][j + soffset[0]];
                nanoem_f32_t src1 = src[1][j + soffset[1]];
                nanoem_f32_t src2 = src[2][j + soffset[2]];
                nanoem_f32_t dst0 = dst[j];
                switch (static_cast<Fx9__Effect__Dx9ms__Opcode>(opcode)) {
                case FX9__EFFECT__DX9MS__OPCODE__PSO_ACOS: {
                    dst0 = glm::acos(src0);
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_ADD:
                case FX9__EFFECT__DX9MS__OPCODE__PSO_ADD_SCALAR: {
                    dst0 = src0 + src1;
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_ASIN: {
                    dst0 = glm::asin(src0);
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_ATAN: {
                    dst0 = glm::atan(src0);
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_ATAN2:
                case FX9__EFFECT__DX9MS__OPCODE__PSO_ATAN2_SCALAR: {
                    dst0 = glm::atan(src0, src1);
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_CMP: {
                    dst0 = glm::mix(src2, src1, src0 >= 0);
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_COS: {
                    dst0 = glm::cos(src0);
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_DIV:
                case FX9__EFFECT__DX9MS__OPCODE__PSO_DIV_SCALAR: {
                    dst0 = src1 != 0.0f ? src0 / src1 : 0.0f;
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_EXP: {
                    dst0 = glm::exp(src0);
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_FRC: {
                    dst0 = glm::fract(src0);
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_GE:
                case FX9__EFFECT__DX9MS__OPCODE__PSO_GE_SCALAR: {
                    dst0 = src0 >= src1;
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_SIN: {
                    dst0 = glm::sin(src0);
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_LT:
                case FX9__EFFECT__DX9MS__OPCODE__PSO_LT_SCALAR: {
                    dst0 = src0 < src1;
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_LOG: {
                    dst0 = glm::log(src0);
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_NOISE_SCALAR: {
                    // dst0 = glm::perlin(src0);
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_MOV: {
                    dst0 = src0;
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_NOISE: {
                    // dst0 = glm::perlin(src0);
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_MAX:
                case FX9__EFFECT__DX9MS__OPCODE__PSO_MAX_SCALAR: {
                    dst0 = glm::max(src0, src1);
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_MIN:
                case FX9__EFFECT__DX9MS__OPCODE__PSO_MIN_SCALAR: {
                    dst0 = glm::min(src0, src1);
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_MOVC:
                    break;
                case FX9__EFFECT__DX9MS__OPCODE__PSO_MUL:
                case FX9__EFFECT__DX9MS__OPCODE__PSO_MUL_SCALAR: {
                    dst0 = src0 * src1;
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_NEG: {
                    dst0 = -src0;
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_NOP:
                    break;
                case FX9__EFFECT__DX9MS__OPCODE__PSO_RCP: {
                    nanoem_f32_t f = src0;
                    dst0 = (f == 0.0f ? 0.0f : (f != 1.0f ? 1.0f / f : f));
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_RSQ: {
                    dst0 = glm::inversesqrt(src0);
                    break;
                }
                case FX9__EFFECT__DX9MS__OPCODE__PSO_DOT:
                case FX9__EFFECT__DX9MS__OPCODE__PSO_DOT_SCALAR:
                case FX9__EFFECT__DX9MS__OPCODE__PSO_SCALAR_OPS:
                default:
                    nanoem_assert(false, "must NOT reach here");
                    break;
                }
                // fprintf(stderr, "  dst[%d] = %3f src0 = %3f src1 = %3f src2 = %3f\n",
                // j, dst0, src0, src1, src2);
            }
        }
        if (outputPtr) {
            const size_t writtenSize = instruction.m_numElements * sizeof(*outputPtr);
            memcpy(outputPtr, glm::value_ptr(dst), writtenSize);
        }
    }
}

AnnotatableParameter::AnnotatableParameter(const String &name)
    : m_name(name)
{
}

AnnotatableParameter::AnnotatableParameter(const String &name, const AnnotationMap &annotations)
    : m_name(name)
    , m_annotations(annotations)
{
}

NonSemanticParameter::NonSemanticParameter(
    const String &name, const GlobalUniform::Vector4List &values, const AnnotationMap &annotations)
    : AnnotatableParameter(name, annotations)
    , m_values(values)
{
}

TypedSemanticParameter::TypedSemanticParameter(const String &name, const String &semantic, ParameterType type)
    : AnnotatableParameter(name)
    , m_semantic(semantic)
    , m_type(type)
    , m_shared(false)
{
}

TypedSemanticParameter::~TypedSemanticParameter() NANOEM_DECL_NOEXCEPT
{
}

MatrixUniform::MatrixUniform(const String &name, MatrixType type, bool inversed, bool transposed)
    : m_name(name)
    , m_type(type)
    , m_inversed(inversed)
    , m_transposed(transposed)
{
}

MatrixUniform::~MatrixUniform() NANOEM_DECL_NOEXCEPT
{
}

Matrix4x4
MatrixUniform::transformed(const Matrix4x4 &value) const NANOEM_DECL_NOEXCEPT
{
    Matrix4x4 transformed(value);
    if (m_inversed) {
        transformed = glm::inverse(transformed);
    }
    if (m_transposed) {
        transformed = glm::transpose(transformed);
    }
    return transformed;
}

void
MatrixUniform::multiply(const Matrix4x4 &world, const Matrix4x4 &view, const Matrix4x4 &projection,
    Matrix4x4 &result) const NANOEM_DECL_NOEXCEPT
{
    switch (m_type) {
    case kMatrixTypeWorld: {
        result = transformed(world);
        break;
    }
    case kMatrixTypeView: {
        result = transformed(view);
        break;
    }
    case kMatrixTypeProjection: {
        result = transformed(projection);
        break;
    }
    case kMatrixTypeWorldView: {
        result = transformed(view * world);
        break;
    }
    case kMatrixTypeViewProjection: {
        result = transformed(projection * view);
        break;
    }
    case kMatrixTypeWorldViewProjection: {
        result = transformed(projection * view * world);
        break;
    }
    default:
        break;
    }
}

ImageSampler::ImageSampler(const String &name, sg_shader_stage stage, sg_image image, nanoem_u32_t offset)
    : m_name(name)
    , m_stage(stage)
    , m_image(image)
    , m_offset(offset)
{
}

ImageSampler::~ImageSampler() NANOEM_DECL_NOEXCEPT
{
}

ControlObjectTarget::ControlObjectTarget(const String &name, ParameterType type)
    : m_name(name)
    , m_type(type)
    , m_value(0)
{
}

ControlObjectTarget::~ControlObjectTarget() NANOEM_DECL_NOEXCEPT
{
}

RenderPassScope::RenderPassScope()
    : m_normalizer(nullptr)
{
}

RenderPassScope::~RenderPassScope()
{
    reset(nullptr);
}

void
RenderPassScope::reset(RenderTargetNormalizer *value)
{
    if (value != m_normalizer) {
        SG_PUSH_GROUP("RenderPassScope::reset");
        if (m_normalizer) {
            m_normalizer->write();
        }
        m_normalizer = value;
        if (value) {
            value->read();
        }
        SG_POP_GROUP();
    }
}

void
RenderPassScope::modifyPipelineDescription(sg_pipeline_desc &pd) const NANOEM_DECL_NOEXCEPT
{
    if (m_normalizer) {
        const PixelFormat format(m_normalizer->normalizedColorImagePixelFormat());
        pd.colors[0].pixel_format = format.colorPixelFormat(0);
    }
}

void
RenderPassScope::apply(
    const sg_bindings &bindings, const IDrawable *drawable, Pass *pass, sg_pipeline pipeline, sg::PassBlock &pb)
{
    if (m_normalizer && sg::is_valid(bindings.index_buffer)) {
        m_normalizer->apply(bindings, drawable, pass, pb);
    }
    else {
        pb.applyPipelineBindings(pipeline, bindings);
    }
}

bool
LoopCounter::isScriptCommandIgnorable(ScriptCommandType type, const Stack &counterStack)
{
    return !counterStack.empty() && counterStack.back().m_last == 0 && type != kScriptCommandTypePopLoopCounter;
}

LoopCounter::LoopCounter(const String name, size_t last, size_t gotoScriptIndex)
    : m_name(name)
    , m_last(last)
    , m_gotoScriptIndex(gotoScriptIndex)
    , m_offset(0)
{
}

LoopCounter::LoopCounter(const LoopCounter &value)
    : m_name(value.m_name)
    , m_last(value.m_last)
    , m_gotoScriptIndex(value.m_gotoScriptIndex)
    , m_offset(value.m_offset)
{
}

LoopCounter::~LoopCounter() NANOEM_DECL_NOEXCEPT
{
}

LoopCounter &
LoopCounter::operator=(const LoopCounter &value)
{
    m_offset = value.m_offset;
    return *this;
}

void
RenderState::convertBlendFactor(nanoem_u32_t value, sg_blend_factor &desc)
{
    switch (value) {
    case 1: { /* D3DBLEND_ZERO */
        desc = SG_BLENDFACTOR_ZERO;
        break;
    }
    case 2: { /* D3DBLEND_ONE */
        desc = SG_BLENDFACTOR_ONE;
        break;
    }
    case 3: { /* D3DBLEND_SRCCOLOR */
        desc = SG_BLENDFACTOR_SRC_COLOR;
        break;
    }
    case 4: { /* D3DBLEND_INVSRCCOLOR */
        desc = SG_BLENDFACTOR_ONE_MINUS_SRC_COLOR;
        break;
    }
    case 5: { /* D3DBLEND_SRCALPHA */
        desc = SG_BLENDFACTOR_SRC_ALPHA;
        break;
    }
    case 6: { /* D3DBLEND_INVSRCALPHA */
        desc = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        break;
    }
    case 7: { /* D3DBLEND_DESTALPHA */
        desc = SG_BLENDFACTOR_DST_ALPHA;
        break;
    }
    case 8: { /* D3DBLEND_INVDESTALPHA */
        desc = SG_BLENDFACTOR_ONE_MINUS_DST_ALPHA;
        break;
    }
    case 9: { /* D3DBLEND_DESTCOLOR */
        desc = SG_BLENDFACTOR_DST_COLOR;
        break;
    }
    case 10: { /* D3DBLEND_INVDESTCOLOR */
        desc = SG_BLENDFACTOR_ONE_MINUS_DST_COLOR;
        break;
    }
    case 11: { /* D3DBLEND_SRCALPHASAT */
        desc = SG_BLENDFACTOR_SRC_ALPHA_SATURATED;
        break;
    }
    default:
        break;
    }
}

void
RenderState::convertBlendOperator(nanoem_u32_t value, sg_blend_op &desc)
{
    switch (value) {
    case 1: { /* D3DBLENDOP_ADD */
        desc = SG_BLENDOP_ADD;
        break;
    }
    case 2: { /* D3DBLENDOP_SUBTRACT */
        desc = SG_BLENDOP_SUBTRACT;
        break;
    }
    case 3: { /* D3DBLENDOP_REVSUBTRACT */
        desc = SG_BLENDOP_REVERSE_SUBTRACT;
        break;
    }
#if defined(NANOEM_ENABLE_BLENDOP_MINMAX)
    case 4: { /* D3DBLENDOP_MIN */
        desc = SG_BLENDOP_MIN;
        break;
    }
    case 5: { /* D3DBLENDOP_MAX */
        desc = SG_BLENDOP_MAX;
        break;
    }
#endif
    default:
        break;
    }
}

void
RenderState::convertWrap(nanoem_u32_t value, sg_wrap &wrap)
{
    switch (value) {
    case 2: { /* D3DTADDRESS_MIRROR */
        wrap = SG_WRAP_MIRRORED_REPEAT;
        break;
    }
    case 3: { /* D3DTADDRESS_CLAMP */
        wrap = SG_WRAP_CLAMP_TO_EDGE;
        break;
    }
    case 4: { /* D3DTADDRESS_BORDER */
        wrap = SG_WRAP_CLAMP_TO_EDGE;
        break;
    }
    default:
        wrap = SG_WRAP_REPEAT;
        break;
    }
}

void
RenderState::convertFilter(nanoem_u32_t value, sg_image_desc &desc, sg_filter &filter)
{
    switch (value) {
    case 2: { /* D3DTEXF_LINEAR */
        filter = SG_FILTER_LINEAR;
        break;
    }
    case 3: { /* D3DTEXF_ANISOTROPIC */
        desc.max_anisotropy = glm::clamp(value, 1u, 16u);
        break;
    }
    default:
        filter = SG_FILTER_NEAREST;
        break;
    }
}

void
RenderState::convertStencilOp(nanoem_u32_t value, sg_stencil_op &op)
{
    switch (value) {
    case 1: { /* D3DSTENCILOP_KEEP */
        op = SG_STENCILOP_KEEP;
        break;
    }
    case 2: { /* D3DSTENCILOP_ZERO */
        op = SG_STENCILOP_ZERO;
        break;
    }
    case 3: { /* D3DSTENCILOP_REPLACE */
        op = SG_STENCILOP_REPLACE;
        break;
    }
    case 4: { /* D3DSTENCILOP_INCRSAT */
        op = SG_STENCILOP_INCR_CLAMP;
        break;
    }
    case 5: { /* D3DSTENCILOP_DECRSAT */
        op = SG_STENCILOP_DECR_CLAMP;
        break;
    }
    case 6: { /* D3DSTENCILOP_INVERT */
        op = SG_STENCILOP_INVERT;
        break;
    }
    case 7: { /* D3DSTENCILOP_INCR */
        op = SG_STENCILOP_INCR_WRAP;
        break;
    }
    case 8: { /* D3DSTENCILOP_DECL */
        op = SG_STENCILOP_DECR_WRAP;
        break;
    }
    default:
        break;
    }
}

void
RenderState::convertCompareFunc(nanoem_u32_t value, sg_compare_func &func)
{
    switch (value) {
    case 1: { /* D3DCMP_NEVER */
        func = SG_COMPAREFUNC_NEVER;
        break;
    }
    case 2: { /* D3DCMP_LESS */
        func = SG_COMPAREFUNC_LESS;
        break;
    }
    case 3: { /* D3DCMP_EQUAL */
        func = SG_COMPAREFUNC_EQUAL;
        break;
    }
    case 4: { /* D3DCMP_LESSEQUAL */
        func = SG_COMPAREFUNC_LESS_EQUAL;
        break;
    }
    case 5: { /* D3DCMP_GREATER */
        func = SG_COMPAREFUNC_GREATER;
        break;
    }
    case 6: { /* D3DCMP_NOTEQUAL */
        func = SG_COMPAREFUNC_NOT_EQUAL;
        break;
    }
    case 7: { /* D3DCMP_GREATEREQUAL */
        func = SG_COMPAREFUNC_GREATER_EQUAL;
        break;
    }
    case 8: { /* D3DCMP_ALWAYS */
        func = SG_COMPAREFUNC_ALWAYS;
        break;
    }
    default:
        break;
    }
}

void
RenderState::convertSamplerState(nanoem_u32_t key, nanoem_u32_t value, sg_image_desc &desc)
{
    switch (key) {
    case 1: { /* D3DSAMP_ADDRESSU */
        convertWrap(value, desc.wrap_u);
        break;
    }
    case 2: { /* D3DSAMP_ADDRESSV */
        convertWrap(value, desc.wrap_v);
        break;
    }
    case 3: { /* D3DSAMP_ADDRESSW */
        convertWrap(value, desc.wrap_w);
        break;
    }
    case 4: { /* D3DSAMP_BORDERCOLOR */
        break;
    }
    case 5: { /* D3DSAMP_MAGFILTER */
        convertFilter(value, desc, desc.mag_filter);
        break;
    }
    case 6: { /* D3DSAMP_MINFILTER */
        convertFilter(value, desc, desc.min_filter);
        break;
    }
    case 7: { /* D3DSAMP_MIPFILTER */
        setMipFilter(value, desc.min_filter);
        if (value != 0) {
            /* set actual mipmap levels later */
            desc.num_mipmaps = 0;
        }
        break;
    }
    case 11: { /* D3DSAMP_SRGBTEXTURE */
        break;
    }
    default:
        break;
    }
}

/* proceeding render state order by key desc */
void
RenderState::convertPipeline(nanoem_u32_t key, nanoem_u32_t value, PipelineDescriptor &pd)
{
    sg_pipeline_desc &desc = pd.m_body;
    switch (key) {
    case 7: { /* D3DRS_ZENABLE */
        SG_INSERT_MARKERF(
            "effect::RenderState::convertPipeline(key=D3DRS_ZENABLE, value=%d)", desc.depth.write_enabled);
        break;
    }
    case 8: { /* D3DFILLMODE */
        switch (value) {
        case 1: { /* D3DFILL_POINT */
            desc.primitive_type = SG_PRIMITIVETYPE_POINTS;
            break;
        }
        case 2: { /* D3DFILL_WIREFRAME */
            desc.primitive_type = SG_PRIMITIVETYPE_LINES;
            break;
        }
        default:
            desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
            break;
        }
        SG_INSERT_MARKERF("effect::RenderState::convertPipeline(key=D3DFILLMODE, value=%d)", desc.primitive_type);
        break;
    }
    case 14: { /* D3DRS_ZWRITEENABLE */
        desc.depth.write_enabled = value != 0;
        pd.m_hasDepthWriteEnabled = true;
        SG_INSERT_MARKERF(
            "effect::RenderState::convertPipeline(key=D3DRS_ZWRITEENABLE, value=%d)", desc.depth.write_enabled);
        break;
    }
    case 19: { /* D3DRS_SRCBLEND */
        convertBlendFactor(value, desc.colors[0].blend.src_factor_rgb);
        pd.m_hasBlendSourceFactorRGB = true;
        SG_INSERT_MARKERF(
            "effect::RenderState::convertPipeline(key=D3DRS_SRCBLEND, value=%d)", desc.colors[0].blend.src_factor_rgb);
        break;
    }
    case 20: { /* D3DRS_DSTBLEND */
        convertBlendFactor(value, desc.colors[0].blend.dst_factor_rgb);
        pd.m_hasBlendDestFactorRGB = true;
        SG_INSERT_MARKERF(
            "effect::RenderState::convertPipeline(key=D3DRS_DSTBLEND, value=%d)", desc.colors[0].blend.dst_factor_rgb);
        break;
    }
    case 22: { /* D3DRS_CULLMODE */
        switch (value) {
        case 1: { /* D3DCULL_NONE */
            desc.cull_mode = SG_CULLMODE_NONE;
            break;
        }
        case 2: { /* D3DCULL_CW */
            desc.cull_mode = SG_CULLMODE_FRONT;
            break;
        }
        case 3: { /* D3DCULL_CCW */
            desc.cull_mode = SG_CULLMODE_BACK;
            break;
        }
        default:
            break;
        }
        SG_INSERT_MARKERF("effect::RenderState::convertPipeline(key=D3DRS_CULLMODE, value=%d)", desc.cull_mode);
        break;
    }
    case 23: { /* D3DRS_ZFUNC */
        sg_depth_state &ds = desc.depth;
        if (ds.compare == _SG_COMPAREFUNC_DEFAULT) {
            convertCompareFunc(value, ds.compare);
        }
        pd.m_hasDepthCompareFunc = true;
        SG_INSERT_MARKERF("effect::RenderState::convertPipeline(key=D3DRS_ZFUNC, value=%d)", ds.compare);
        break;
    }
    case 24: { /* D3DRS_ALPHAREF */
        desc.alpha_to_coverage_enabled = true;
        SG_INSERT_MARKERF("effect::RenderState::convertPipeline(key=D3DRS_ALPHAREF, value=%d)", value);
        break;
    }
    case 27: { /* D3DRS_ALPHABLENDENABLE */
        sg_blend_state &bs = desc.colors[0].blend;
        bs.enabled = value;
        if (value != 0) {
            if (bs.src_factor_rgb == _SG_BLENDFACTOR_DEFAULT) {
                bs.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
            }
            if (bs.dst_factor_rgb == _SG_BLENDFACTOR_DEFAULT) {
                bs.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
            }
        }
        pd.m_hasBlendEnabled = true;
        SG_INSERT_MARKERF("effect::RenderState::convertPipeline(key=D3DRS_ALPHABLENDENABLE, value=%d)", value);
        break;
    }
    case 52: { /* D3DRS_STENCILENABLE  */
        const bool enable = value != 0;
        sg_stencil_state &ds = desc.stencil;
        ds.enabled = enable;
        pd.m_hasStencilEnabled = true;
        SG_INSERT_MARKERF(
            "effect::RenderState::convertPipeline(key=D3DRS_STENCILENABLE, value=%d)", desc.stencil.enabled);
        break;
    }
    case 53: { /* D3DRS_STENCILFAIL */
        convertStencilOp(value, desc.stencil.front.fail_op);
        pd.m_stencilFront.m_hasFailOp = true;
        SG_INSERT_MARKERF(
            "effect::RenderState::convertPipeline(key=D3DRS_STENCILFAIL, value=%d)", desc.stencil.front.fail_op);
        break;
    }
    case 54: { /* D3DRS_STENCILZFAIL */
        convertStencilOp(value, desc.stencil.front.depth_fail_op);
        pd.m_stencilFront.m_hasDepthFailOp = true;
        SG_INSERT_MARKERF(
            "effect::RenderState::convertPipeline(key=D3DRS_STENCILZFAIL, value=%d)", desc.stencil.front.depth_fail_op);
        break;
    }
    case 55: { /* D3DRS_STENCILPASS */
        convertStencilOp(value, desc.stencil.front.pass_op);
        pd.m_stencilFront.m_hasPassOp = true;
        SG_INSERT_MARKERF(
            "effect::RenderState::convertPipeline(key=D3DRS_STENCILPASS, value=%d)", desc.stencil.front.pass_op);
        break;
    }
    case 56: { /* D3DRS_STENCILFUNC */
        convertCompareFunc(value, desc.stencil.front.compare);
        pd.m_stencilFront.m_hasCompareFunc = true;
        SG_INSERT_MARKERF(
            "effect::RenderState::convertPipeline(key=D3DRS_STENCILFUNC, value=%d)", desc.stencil.front.compare);
        break;
    }
    case 57: { /* D3DRS_STENCILREF */
        desc.stencil.ref = nanoem_u8_t(glm::clamp(value, 0u, 0xffu));
        pd.m_hasStencilRef = true;
        SG_INSERT_MARKERF("effect::RenderState::convertPipeline(key=D3DRS_STENCILREF, value=%d)", desc.stencil.ref);
        break;
    }
    case 58: { /* D3DRS_STENCILMASK  */
        desc.stencil.read_mask = nanoem_u8_t(glm::clamp(value, 0u, 0xffu));
        pd.m_hasStencilReadMask = true;
        SG_INSERT_MARKERF(
            "effect::RenderState::convertPipeline(key=D3DRS_STENCILMASK, value=%d)", desc.stencil.read_mask);
        break;
    }
    case 59: { /* D3DRS_STENCILWRITEMASK  */
        desc.stencil.write_mask = nanoem_u8_t(glm::clamp(value, 0u, 0xffu));
        pd.m_hasStencilWriteMask = true;
        SG_INSERT_MARKERF(
            "effect::RenderState::convertPipeline(key=D3DRS_STENCILWRITEMASK, value=%d)", desc.stencil.write_mask);
        break;
    }
    case 168: { /* D3DRS_COLORWRITEENABLE  */
        const bool enabled = value != 0;
        sg_color_mask &writeMask = desc.colors[0].write_mask;
        if (enabled) {
            if (writeMask == _SG_COLORMASK_DEFAULT) {
                writeMask = SG_COLORMASK_RGBA;
            }
        }
        else {
            writeMask = SG_COLORMASK_NONE;
        }
        pd.m_hasColorWriteMask = true;
        SG_INSERT_MARKERF("effect::RenderState::convertPipeline(key=D3DRS_COLORWRITEENABLE, value=%d)", writeMask);
        break;
    }
    case 171: { /* D3DRS_BLENDOP */
        convertBlendOperator(value, desc.colors[0].blend.op_rgb);
        pd.m_hasBlendOpRGB = true;
        SG_INSERT_MARKERF(
            "effect::RenderState::convertPipeline(key=D3DRS_BLENDOP, value=%d)", desc.colors[0].blend.op_rgb);
        break;
    }
    case 186: { /* D3DRS_CCW_STENCILFAIL */
        convertStencilOp(value, desc.stencil.back.fail_op);
        pd.m_stencilBack.m_hasFailOp = true;
        SG_INSERT_MARKERF(
            "effect::RenderState::convertPipeline(key=D3DRS_CCW_STENCILFAIL, value=%d)", desc.stencil.back.pass_op);
        break;
    }
    case 187: { /* D3DRS_CCW_STENCILZFAIL */
        convertStencilOp(value, desc.stencil.back.depth_fail_op);
        pd.m_stencilBack.m_hasDepthFailOp = true;
        SG_INSERT_MARKERF("effect::RenderState::convertPipeline(key=D3DRS_CCW_STENCILZFAIL, value=%d)",
            desc.stencil.back.depth_fail_op);
        break;
    }
    case 188: { /* D3DRS_CCW_STENCILPASS */
        convertStencilOp(value, desc.stencil.back.pass_op);
        pd.m_stencilBack.m_hasPassOp = true;
        SG_INSERT_MARKERF(
            "effect::RenderState::convertPipeline(key=D3DRS_CCW_STENCILPASS, value=%d)", desc.stencil.back.pass_op);
        break;
    }
    case 189: { /* D3DRS_CCW_STENCILFUNC */
        convertCompareFunc(value, desc.stencil.back.compare);
        pd.m_stencilBack.m_hasCompareFunc = true;
        SG_INSERT_MARKERF(
            "effect::RenderState::convertPipeline(key=D3DRS_CCW_STENCILFUNC, value=%d)", desc.stencil.back.compare);
        break;
    }
    case 206: { /* D3DRS_SEPARATEALPHABLENDENABLE */
        SG_INSERT_MARKERF("effect::RenderState::convertPipeline(key=D3DRS_SEPARATEALPHABLENDENABLE, value=%d)", value);
        break;
    }
    case 207: { /* D3DRS_SRCBLENDALPHA */
        convertBlendFactor(value, desc.colors[0].blend.src_factor_alpha);
        pd.m_hasBlendSourceFactorAlpha = true;
        SG_INSERT_MARKERF("effect::RenderState::convertPipeline(key=D3DRS_SRCBLENDALPHA, value=%d)",
            desc.colors[0].blend.src_factor_alpha);
        break;
    }
    case 208: { /* D3DRS_DESTBLENDALPHA */
        convertBlendFactor(value, desc.colors[0].blend.dst_factor_alpha);
        pd.m_hasBlendDestFactorAlpha = true;
        SG_INSERT_MARKERF("effect::RenderState::convertPipeline(key=D3DRS_DESTBLENDALPHA, value=%d)",
            desc.colors[0].blend.dst_factor_alpha);
        break;
    }
    case 209: { /* D3DRS_BLENDOPALPHA */
        convertBlendOperator(value, desc.colors[0].blend.op_alpha);
        pd.m_hasBlendOpAlpha = true;
        SG_INSERT_MARKERF(
            "effect::RenderState::convertPipeline(key=D3DRS_BLENDOPALPHA, value=%d)", desc.colors[0].blend.op_alpha);
        break;
    }
    default:
        SG_INSERT_MARKERF("RenderState::convertPipeline(key=%d, value=%d)", key, value);
        break;
    }
}

void
RenderState::normalizeMinFilter(sg_filter &value)
{
    switch (value) {
    case SG_FILTER_LINEAR_MIPMAP_LINEAR:
    case SG_FILTER_LINEAR_MIPMAP_NEAREST:
        value = SG_FILTER_LINEAR;
        break;
    case SG_FILTER_NEAREST_MIPMAP_LINEAR:
    case SG_FILTER_NEAREST_MIPMAP_NEAREST:
        value = SG_FILTER_NEAREST;
        break;
    default:
        break;
    }
}

Logger::Logger()
{
}

Logger::~Logger() NANOEM_DECL_NOEXCEPT
{
}

void
Logger::add(const char *message, int length)
{
    const nanoem_u32_t key = bx::hash<bx::HashMurmur2A>(message, length);
    ContainerMap::const_iterator it = m_containers.find(key);
    if (it == m_containers.end()) {
        Container container;
        container.m_time = stm_now();
        container.m_body = String(message, length);
        m_containers.insert(tinystl::make_pair(key, container));
        EMLOG_INFO("{}", message);
    }
    SG_INSERT_MARKERF("[INFO] %s", message);
}

void
Logger::log(const char *format, ...)
{
    char message[Inline::kLongNameStackBufferSize];
    va_list args;
    va_start(args, format);
    int length = StringUtils::formatVA(message, sizeof(message), format, args);
    va_end(args);
    add(message, length);
}

} /* namespace effect */
} /* namespace nanoem */
