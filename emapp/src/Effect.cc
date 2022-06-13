/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/Effect.h"

#if BX_PLATFORM_WINDOWS
#include <d3d11shader.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#if defined(__MINGW32__)
#define D3DCOMPILE_FLAGS2_FORCE_ROOT_SIGNATURE_LATEST 0
#endif
namespace {
typedef HRESULT(WINAPI *pfn_D3DCompile)(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
    CONST D3D_SHADER_MACRO *pDefines, LPD3DINCLUDE pInclude, LPCSTR pEntrypoint, LPCSTR pTarget, UINT Flags1,
    UINT Flags2, LPD3DBLOB *ppCode, LPD3DBLOB *ppErrorMsgs);
void *g_D3DCompilerDll = nullptr;
pfn_D3DCompile g_D3DCompile = nullptr;
} /* namespace anonymous */
#endif /* BX_PLATFORM_WINDOWS */

#include "emapp/Accessory.h"
#include "emapp/AccessoryProgramBundle.h"
#include "emapp/Archiver.h"
#include "emapp/FileUtils.h"
#include "emapp/ICamera.h"
#include "emapp/IEventPublisher.h"
#include "emapp/IFileManager.h"
#include "emapp/ILight.h"
#include "emapp/ImageLoader.h"
#include "emapp/ListUtils.h"
#include "emapp/Model.h"
#include "emapp/ModelProgramBundle.h"
#include "emapp/PixelFormat.h"
#include "emapp/PluginFactory.h"
#include "emapp/Progress.h"
#include "emapp/Project.h"
#include "emapp/ShadowCamera.h"
#include "emapp/StringUtils.h"
#include "emapp/effect/AnimatedImageContainer.h"
#include "emapp/effect/RenderTargetMipmapGenerator.h"
#include "emapp/effect/RenderTargetNormalizer.h"
#include "emapp/internal/BlitPass.h"
#include "emapp/model/Morph.h"
#include "emapp/private/CommonInclude.h"

#include "./effect/EffectCommon.inl"
#include "./protoc/effect.pb-c.h"

#include "bx/hash.h"
#include "glm/gtc/noise.hpp"

namespace nanoem {
namespace effect {
namespace {

static const char *const kObjectKeyLiteral = "Object";
static const char *const kObjectGeometryValueLiteral = "Geometry";
static const char *const kObjectLightValueLiteral = "Light";
static const char *const kObjectCameraValueLiteral = "Camera";
static const char *const kSyncInEditModeKeyLiteral = "SyncInEditMode";
static const char *const kNameKeyLiteral = "name";
static const char *const kItemKeyLiteral = "item";
static const char *const kResourceTypeKeyLiteral = "ResourceType";
static const char *const kResourceType1DValueLiteral = "1D";
static const char *const kResourceType2DValueLiteral = "2D";
static const char *const kResourceType3DValueLiteral = "3D";
static const char *const kResourceTypeCubeValueLiteral = "CUBE";
static const char *const kResourceNameKeyLiteral = "ResourceName";
static const char *const kTextureNameKeyLiteral = "TextureName";
static const char *const kWidthKeyLiteral = "Width";
static const char *const kHeightKeyLiteral = "Height";
static const char *const kDepthKeyLiteral = "Depth";
static const char *const kDimensionsKeyLiteral = "Dimensions";
static const char *const kViewportRatioKeyLiteral = "ViewportRatio";
static const char *const kFormatKeyLiteral = "Format";
static const char *const kMiplevelsKeyLiteral = "Miplevels";
static const char *const kLevelsKeyLiteral = "Levels";
static const char *const kOffsetKeyLiteral = "Offset";
static const char *const kSpeedKeyLiteral = "Speed";
static const char *const kSeekVariableKeyLiteral = "SeekVariable";
static const char *const kClearColorKeyLiteral = "ClearColor";
static const char *const kClearDepthKeyLiteral = "ClearDepth";
static const char *const kAntiAliasKeyLiteral = "AntiAlias";
static const char *const kDescriptionKeyLiteral = "Description";
static const char *const kDefaultEffectKeyLiteral = "DefaultEffect";
static const char *const kScriptOutputKeyLiteral = "ScriptOutput";
static const char *const kScriptOutputColorValueLiteral = "color";
static const char *const kScriptClassKeyLiteral = "ScriptClass";
static const char *const kScriptClassObjectValueLiteral = "object";
static const char *const kScriptClassSceneValueLiteral = "scene";
static const char *const kScriptClassSceneObjectValueLiteral = "sceneorobject";
static const char *const kScriptOrderKeyLiteral = "ScriptOrder";
static const char *const kScriptOrderStandardValueLiteral = "standard";
static const char *const kScriptOrderPreProcessValueLiteral = "preprocess";
static const char *const kScriptOrderPostProcessValueLiteral = "postprocess";
static const char *const kScriptRenderColorTargetValueLiteral = "RenderColorTarget";
static const char *const kScriptRenderColorTarget0ValueLiteral = "RenderColorTarget0";
static const char *const kScriptRenderColorTarget1ValueLiteral = "RenderColorTarget1";
static const char *const kScriptRenderColorTarget2ValueLiteral = "RenderColorTarget2";
static const char *const kScriptRenderColorTarget3ValueLiteral = "RenderColorTarget3";
static const char *const kScriptRenderDepthStencilValueLiteral = "RenderDepthStencilTarget";
static const char *const kScriptClearSetColorValueLiteral = "ClearSetColor";
static const char *const kScriptClearSetDepthValueLiteral = "ClearSetDepth";
static const char *const kScriptClearValueLiteral = "Clear";
static const char *const kScriptScriptExternalValueLiteral = "ScriptExternal";
static const char *const kScriptPassValueLiteral = "Pass";
static const char *const kScriptLoopByCountValueLiteral = "LoopByCount";
static const char *const kScriptLoopGetIndexValueLiteral = "LoopGetIndex";
static const char *const kScriptLoopEndValueLiteral = "LoopEnd";
static const char *const kScriptDrawValueLiteral = "Draw";

struct AttributeUsage {
    const char *m_name;
    int m_variant;
};

class PrivateEffectUtils NANOEM_DECL_SEALED : private NonCopyable {
public:
    static void parseAnnotations(Fx9__Effect__Annotation *const *annotationsPtr, size_t numAnnotations,
        nanoem_unicode_string_factory_t *factory, AnnotationMap &annotations);
    static nanoem_u32_t offsetParameterType(const Fx9__Effect__Parameter *parameter) NANOEM_DECL_NOEXCEPT;
    static ParameterType determineParameterType(const Fx9__Effect__Parameter *parameter) NANOEM_DECL_NOEXCEPT;
    static String concatPrefix(const char *const name, const char *const prefix);
    static void fillFallbackImage(
        sg_image *images, const nanoem_u8_t imageCount, const sg_image &fallbackImage) NANOEM_DECL_NOEXCEPT;
    static int sortRenderStateAscent(const void *left, const void *right) NANOEM_DECL_NOEXCEPT;

    template <typename TShader, typename TSymbol, typename TUniform, typename TAttribute>
    static void appendShaderVariablesHeaderComment(
        const TShader *shaderPtr, const Preshader &preshader, String &newShaderCode);
    static void appendShaderVariablesHeaderComment(const Fx9__Effect__Shader *shaderPtr, String &newShaderCode);
    static void setRegisterIndex(const Fx9__Effect__Symbol *symbolPtr, RegisterIndexMap &registerIndices);
    static void attachShaderSource(const Fx9__Effect__Shader *shaderPtr, const char *techniqueName,
        const char *passName, sg_shader_stage_desc &desc, String &newShaderCode, Error &error);
    static void retrieveShaderSymbols(const Fx9__Effect__Shader *shaderPtr, RegisterIndexMap &registerIndices,
        UniformBufferOffsetMap &uniformBufferOffsetMap);
    static void setImageTypesFromSampler(
        const Fx9__Effect__Shader *shaderPtr, sg_shader_image_desc *shaderSamplers) NANOEM_DECL_NOEXCEPT;
    static void retrievePixelShaderSamplers(const Fx9__Effect__Pass *pass, sg_shader_image_desc *shaderSamplers,
        ImageDescriptionMap &textureDescriptions, SamplerRegisterIndexMap &shaderRegisterIndices);
    static void retrieveVertexShaderSamplers(const Fx9__Effect__Pass *pass, sg_shader_image_desc *shaderSamplers,
        ImageDescriptionMap &textureDescriptions, SamplerRegisterIndexMap &shaderRegisterIndices);
    static void fillShaderImageDescriptions(const char *prefix, sg_shader_image_desc *desc, StringList &names);
    static void parseSubsetString(char *ptr, int numMaterials, Effect::MaterialIndexSet &output);
    static bool hasShaderSource(const sg_shader_desc &desc) NANOEM_DECL_NOEXCEPT;
    static Vector3 angle(const Accessory *accessory);

    static inline void
    countRegisterSet(const Fx9__Effect__Dx9ms__Shader *shaderPtr, GlobalUniform::Buffer &buffer)
    {
        effect::countRegisterSet(shaderPtr->symbols, shaderPtr->n_symbols, buffer);
    }
    static inline void
    countRegisterSet(const Fx9__Effect__Shader *shaderPtr, GlobalUniform::Buffer &buffer)
    {
        effect::countRegisterSet(shaderPtr->symbols, shaderPtr->n_symbols, buffer);
    }
};

void
PrivateEffectUtils::parseAnnotations(Fx9__Effect__Annotation *const *annotationsPtr, size_t numAnnotations,
    nanoem_unicode_string_factory_t *factory, AnnotationMap &annotations)
{
    annotations.clear();
    for (size_t i = 0; i < numAnnotations; i++) {
        const Fx9__Effect__Annotation *annotation = annotationsPtr[i];
        const String name(StringUtils::toLowerCase(annotation->name));
        AnnotationValue value;
        switch (annotation->value_case) {
        case FX9__EFFECT__ANNOTATION__VALUE_BVAL: {
            value.m_bool = annotation->bval != 0;
            annotations.insert(tinystl::make_pair(name, value));
            break;
        }
        case FX9__EFFECT__ANNOTATION__VALUE_FVAL: {
            value.m_float = annotation->fval;
            annotations.insert(tinystl::make_pair(name, value));
            break;
        }
        case FX9__EFFECT__ANNOTATION__VALUE_FVAL4: {
            value.m_vector = glm::make_vec4(&annotation->fval4->x);
            annotations.insert(tinystl::make_pair(name, value));
            break;
        }
        case FX9__EFFECT__ANNOTATION__VALUE_IVAL: {
            value.m_int = annotation->ival;
            annotations.insert(tinystl::make_pair(name, value));
            break;
        }
        case FX9__EFFECT__ANNOTATION__VALUE_IVAL4: {
            value.m_vector = glm::make_vec4(&annotation->ival4->x);
            annotations.insert(tinystl::make_pair(name, value));
            break;
        }
        case FX9__EFFECT__ANNOTATION__VALUE_SVAL: {
            const char *sval = annotation->sval;
            StringUtils::getUtf8String(
                sval, StringUtils::length(sval), NANOEM_CODEC_TYPE_SJIS, factory, value.m_string);
            annotations.insert(tinystl::make_pair(name, value));
            break;
        }
        case FX9__EFFECT__ANNOTATION__VALUE_SVAL_UTF8: {
            value.m_string = annotation->sval_utf8;
            annotations.insert(tinystl::make_pair(name, value));
            break;
        }
        case FX9__EFFECT__ANNOTATION__VALUE__NOT_SET:
        default:
            break;
        }
    }
}

nanoem_u32_t
PrivateEffectUtils::offsetParameterType(const Fx9__Effect__Parameter *parameter) NANOEM_DECL_NOEXCEPT
{
    nanoem_u32_t offset = 0;
    switch (parameter->type_common) {
    case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_BOOL: {
        offset = kParameterTypeBool;
        break;
    }
    case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_INT: {
        offset = kParameterTypeInt;
        break;
    }
    case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_FLOAT: {
        offset = kParameterTypeFloat;
        break;
    }
    default:
        switch (parameter->type_dx9ms) {
        case FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_BOOL: {
            offset = kParameterTypeBool;
            break;
        }
        case FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_INT: {
            offset = kParameterTypeInt;
            break;
        }
        case FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_FLOAT: {
            offset = kParameterTypeFloat;
            break;
        }
        default:
            break;
        }
        break;
    }
    return offset;
}

ParameterType
PrivateEffectUtils::determineParameterType(const Fx9__Effect__Parameter *parameter) NANOEM_DECL_NOEXCEPT
{
    ParameterType type = kParameterTypeUnknown;
    if (parameter->has_class_common && parameter->has_type_common) {
        Fx9__Effect__Parameter__TypeCommon typeCommon = parameter->type_common;
        switch (parameter->class_common) {
        case FX9__EFFECT__PARAMETER__CLASS_COMMON__PC_SCALAR: {
            switch (typeCommon) {
            case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_BOOL: {
                type = kParameterTypeBool;
                break;
            }
            case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_INT: {
                type = kParameterTypeInt;
                break;
            }
            case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_FLOAT: {
                type = kParameterTypeFloat;
                break;
            }
            default:
                return type;
            }
            break;
        }
        case FX9__EFFECT__PARAMETER__CLASS_COMMON__PC_VECTOR: {
            nanoem_u32_t offset = offsetParameterType(parameter);
            type = static_cast<ParameterType>(offset + 3);
            break;
        }
        case FX9__EFFECT__PARAMETER__CLASS_COMMON__PC_MATRIX_COLUMNS:
        case FX9__EFFECT__PARAMETER__CLASS_COMMON__PC_MATRIX_ROWS: {
            nanoem_u32_t offset = offsetParameterType(parameter);
            type = static_cast<ParameterType>(offset +
                glm::clamp(parameter->num_rows * parameter->num_columns, nanoem_u32_t(1), nanoem_u32_t(16)) - 1);
            break;
        }
        case FX9__EFFECT__PARAMETER__CLASS_COMMON__PC_OBJECT: {
            switch (typeCommon) {
            case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_STRING: {
                type = kParameterTypeString;
                break;
            }
            case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_SAMPLER:
            case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_SAMPLER1D:
            case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_SAMPLER2D:
            case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_SAMPLER3D:
            case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_SAMPLERCUBE: {
                type = static_cast<ParameterType>(
                    typeCommon - FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_SAMPLER + kParameterTypeSampler);
                break;
            }
            case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_TEXTURE:
            case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_TEXTURE1D:
            case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_TEXTURE2D:
            case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_TEXTURE3D:
            case FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_TEXTURECUBE: {
                type = static_cast<ParameterType>(
                    typeCommon - FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_TEXTURE + kParameterTypeTexture);
                break;
            }
            default:
                return type;
            }
            break;
        }
        default:
            break;
        }
    }
    else {
        Fx9__Effect__Dx9ms__ParameterType typeDX9MS = parameter->type_dx9ms;
        switch (parameter->class_dx9ms) {
        case FX9__EFFECT__DX9MS__PARAMETER_CLASS__PC_SCALAR: {
            switch (typeDX9MS) {
            case FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_BOOL: {
                type = kParameterTypeBool;
                break;
            }
            case FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_INT: {
                type = kParameterTypeInt;
                break;
            }
            case FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_FLOAT: {
                type = kParameterTypeFloat;
                break;
            }
            default:
                return type;
            }
            break;
        }
        case FX9__EFFECT__DX9MS__PARAMETER_CLASS__PC_VECTOR: {
            nanoem_u32_t offset = offsetParameterType(parameter);
            type = static_cast<ParameterType>(offset + 3);
            break;
        }
        case FX9__EFFECT__DX9MS__PARAMETER_CLASS__PC_MATRIX_COLUMNS:
        case FX9__EFFECT__DX9MS__PARAMETER_CLASS__PC_MATRIX_ROWS: {
            nanoem_u32_t offset = offsetParameterType(parameter);
            type = static_cast<ParameterType>(offset +
                glm::clamp(parameter->num_rows * parameter->num_columns, nanoem_u32_t(1), nanoem_u32_t(16)) - 1);
            break;
        }
        case FX9__EFFECT__DX9MS__PARAMETER_CLASS__PC_OBJECT: {
            switch (parameter->type_dx9ms) {
            case FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_STRING: {
                type = kParameterTypeString;
                break;
            }
            case FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_SAMPLER:
            case FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_SAMPLER1D:
            case FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_SAMPLER2D:
            case FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_SAMPLER3D:
            case FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_SAMPLERCUBE: {
                type = static_cast<ParameterType>(
                    typeDX9MS - FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_SAMPLER + kParameterTypeSampler);
                break;
            }
            case FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_TEXTURE:
            case FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_TEXTURE1D:
            case FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_TEXTURE2D:
            case FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_TEXTURE3D:
            case FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_TEXTURECUBE: {
                type = static_cast<ParameterType>(
                    typeDX9MS - FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_TEXTURE + kParameterTypeTexture);
                break;
            }
            default:
                return type;
            }
            break;
        }
        default:
            break;
        }
    }
    return type;
}

String
PrivateEffectUtils::concatPrefix(const char *const name, const char *const prefix)
{
    String result;
    result.reserve(StringUtils::length(name) + StringUtils::length(prefix));
    result.append(prefix);
    result.append(name);
    return result;
}

void
PrivateEffectUtils::fillFallbackImage(
    sg_image *images, const nanoem_u8_t imageCount, const sg_image &fallbackImage) NANOEM_DECL_NOEXCEPT
{
    for (nanoem_u8_t i = 0; i < imageCount; i++) {
        sg_image &sd = images[i];
        if (!sg::is_valid(sd)) {
            sd = fallbackImage;
        }
    }
}

int
PrivateEffectUtils::sortRenderStateAscent(const void *left, const void *right) NANOEM_DECL_NOEXCEPT
{
    const Fx9__Effect__RenderState *lvalue = *static_cast<Fx9__Effect__RenderState *const *>(left);
    const Fx9__Effect__RenderState *rvalue = *static_cast<Fx9__Effect__RenderState *const *>(right);
    int result = 0;
    if (lvalue->key > rvalue->key) {
        result = 1;
    }
    else if (lvalue->key < rvalue->key) {
        result = -1;
    }
    return result;
}

template <typename TShader, typename TSymbol, typename TUniform, typename TAttribute>
void
PrivateEffectUtils::appendShaderVariablesHeaderComment(
    const TShader *shaderPtr, const Preshader &preshader, String &newShaderCode)
{
    char buffer[128];
    const size_t numShaderSymbols = shaderPtr->n_symbols;
    struct SymbolSorter {
        static int
        sort(const void *left, const void *right)
        {
            const TSymbol *lvalue = *static_cast<TSymbol *const *>(left);
            const TSymbol *rvalue = *static_cast<TSymbol *const *>(right);
            return StringUtils::compare(lvalue->name, rvalue->name);
        }
    };
    tinystl::vector<TSymbol *, TinySTLAllocator> symbols(shaderPtr->symbols, shaderPtr->symbols + numShaderSymbols);
    qsort(symbols.data(), symbols.size(), sizeof(symbols[0]), SymbolSorter::sort);
    for (size_t i = 0; i < numShaderSymbols; i++) {
        const TSymbol *symbolPtr = symbols[i];
        StringUtils::format(buffer, sizeof(buffer), " [shader:symbols:%s]\n", symbolPtr->name);
        newShaderCode.append(buffer);
        StringUtils::format(buffer, sizeof(buffer), " index = %d\n", symbolPtr->register_index);
        newShaderCode.append(buffer);
        StringUtils::format(buffer, sizeof(buffer), " count = %d\n", symbolPtr->register_count);
        newShaderCode.append(buffer);
        switch (static_cast<Fx9__Effect__Dx9ms__RegisterSet>(symbolPtr->register_set)) {
        case FX9__EFFECT__DX9MS__REGISTER_SET__RS_BOOL:
            newShaderCode.append(" type = bool\n");
            break;
        case FX9__EFFECT__DX9MS__REGISTER_SET__RS_FLOAT4:
            newShaderCode.append(" type = float4\n");
            break;
        case FX9__EFFECT__DX9MS__REGISTER_SET__RS_INT4:
            newShaderCode.append(" type = int4\n");
            break;
        case FX9__EFFECT__DX9MS__REGISTER_SET__RS_SAMPLER:
            newShaderCode.append(" type = sampler\n");
            break;
        default:
            break;
        }
    }
    const size_t numPreshaderSymbols = preshader.m_symbols.size();
    for (size_t i = 0; i < numPreshaderSymbols; i++) {
        const Preshader::Symbol &symbol = preshader.m_symbols[i];
        StringUtils::format(buffer, sizeof(buffer), " [preshader:symbols:%s]\n", symbol.m_name.c_str());
        newShaderCode.append(buffer);
        StringUtils::format(buffer, sizeof(buffer), " index = %d\n", symbol.m_index);
        newShaderCode.append(buffer);
        StringUtils::format(buffer, sizeof(buffer), " count = %d\n", symbol.m_count);
        newShaderCode.append(buffer);
        switch (static_cast<Fx9__Effect__Dx9ms__RegisterSet>(symbol.m_set)) {
        case FX9__EFFECT__DX9MS__REGISTER_SET__RS_BOOL:
            newShaderCode.append(" type = bool\n");
            break;
        case FX9__EFFECT__DX9MS__REGISTER_SET__RS_FLOAT4:
            newShaderCode.append(" type = float4\n");
            break;
        case FX9__EFFECT__DX9MS__REGISTER_SET__RS_INT4:
            newShaderCode.append(" type = int4\n");
            break;
        case FX9__EFFECT__DX9MS__REGISTER_SET__RS_SAMPLER:
            newShaderCode.append(" type = sampler\n");
            break;
        default:
            break;
        }
    }
    const size_t numUniforms = shaderPtr->n_uniforms;
    struct UniformSorter {
        static int
        sort(const void *left, const void *right)
        {
            const TUniform *lvalue = *static_cast<TUniform *const *>(left);
            const TUniform *rvalue = *static_cast<TUniform *const *>(right);
            return StringUtils::compare(lvalue->name, rvalue->name);
        }
    };
    tinystl::vector<TUniform *, TinySTLAllocator> uniforms(shaderPtr->uniforms, shaderPtr->uniforms + numUniforms);
    qsort(uniforms.data(), uniforms.size(), sizeof(uniforms[0]), UniformSorter::sort);
    for (size_t i = 0; i < numUniforms; i++) {
        const TUniform *uniformPtr = uniforms[i];
        StringUtils::format(buffer, sizeof(buffer), " [shader:uniform:%s]\n", uniformPtr->name);
        newShaderCode.append(buffer);
        StringUtils::format(buffer, sizeof(buffer), " index = %d\n", uniformPtr->index);
        newShaderCode.append(buffer);
        StringUtils::format(buffer, sizeof(buffer), " constant = %d\n", uniformPtr->constant_index);
        newShaderCode.append(buffer);
    }
    const size_t numAttributes = shaderPtr->n_attributes;
    struct AttributeSorter {
        static int
        sort(const void *left, const void *right)
        {
            const TAttribute *lvalue = *static_cast<TAttribute *const *>(left);
            const TAttribute *rvalue = *static_cast<TAttribute *const *>(right);
            return StringUtils::compare(lvalue->name, rvalue->name);
        }
    };
    tinystl::vector<TAttribute *, TinySTLAllocator> inputs(
        shaderPtr->attributes, shaderPtr->attributes + numAttributes);
    qsort(inputs.data(), inputs.size(), sizeof(inputs[0]), AttributeSorter::sort);
    for (size_t i = 0; i < numAttributes; i++) {
        const TAttribute *attributePtr = inputs[i];
        StringUtils::format(buffer, sizeof(buffer), " [shader:input:%s]\n", attributePtr->name);
        newShaderCode.append(buffer);
        StringUtils::format(buffer, sizeof(buffer), " index = %d\n", attributePtr->index);
        newShaderCode.append(buffer);
        StringUtils::format(buffer, sizeof(buffer), " usage = %d\n", attributePtr->usage);
        newShaderCode.append(buffer);
    }
    const size_t numOutputs = shaderPtr->n_outputs;
    tinystl::vector<TAttribute *, TinySTLAllocator> outputs(shaderPtr->outputs, shaderPtr->outputs + numOutputs);
    qsort(outputs.data(), outputs.size(), sizeof(outputs[0]), AttributeSorter::sort);
    for (size_t i = 0; i < numOutputs; i++) {
        const TAttribute *outputPtr = outputs[i];
        StringUtils::format(buffer, sizeof(buffer), " [shader:output:%s]\n", outputPtr->name);
        newShaderCode.append(buffer);
        StringUtils::format(buffer, sizeof(buffer), " index = %d\n", outputPtr->index);
        newShaderCode.append(buffer);
        StringUtils::format(buffer, sizeof(buffer), " usage = %d\n", outputPtr->usage);
        newShaderCode.append(buffer);
    }
    newShaderCode.append("*/\n");
}

void
PrivateEffectUtils::appendShaderVariablesHeaderComment(const Fx9__Effect__Shader *shaderPtr, String &newShaderCode)
{
    char buffer[128];
    const size_t numShaderSymbols = shaderPtr->n_symbols;
    struct SymbolSorter {
        static int
        sort(const void *left, const void *right)
        {
            const Fx9__Effect__Symbol *lvalue = *static_cast<Fx9__Effect__Symbol *const *>(left);
            const Fx9__Effect__Symbol *rvalue = *static_cast<Fx9__Effect__Symbol *const *>(right);
            return StringUtils::compare(lvalue->name, rvalue->name);
        }
    };
    tinystl::vector<Fx9__Effect__Symbol *, TinySTLAllocator> symbols(
        shaderPtr->symbols, shaderPtr->symbols + numShaderSymbols);
    qsort(symbols.data(), symbols.size(), sizeof(symbols[0]), SymbolSorter::sort);
    for (size_t i = 0; i < numShaderSymbols; i++) {
        const Fx9__Effect__Symbol *symbolPtr = symbols[i];
        StringUtils::format(buffer, sizeof(buffer), " [shader:symbols:%s]\n", symbolPtr->name);
        newShaderCode.append(buffer);
        StringUtils::format(buffer, sizeof(buffer), " index = %d\n", symbolPtr->register_index);
        newShaderCode.append(buffer);
        StringUtils::format(buffer, sizeof(buffer), " count = %d\n", symbolPtr->register_count);
        newShaderCode.append(buffer);
        switch (static_cast<Fx9__Effect__Dx9ms__RegisterSet>(symbolPtr->register_set)) {
        case FX9__EFFECT__DX9MS__REGISTER_SET__RS_BOOL:
            newShaderCode.append(" type = bool\n");
            break;
        case FX9__EFFECT__DX9MS__REGISTER_SET__RS_FLOAT4:
            newShaderCode.append(" type = float4\n");
            break;
        case FX9__EFFECT__DX9MS__REGISTER_SET__RS_INT4:
            newShaderCode.append(" type = int4\n");
            break;
        case FX9__EFFECT__DX9MS__REGISTER_SET__RS_SAMPLER:
            newShaderCode.append(" type = sampler\n");
            break;
        default:
            break;
        }
    }
    const size_t numUniforms = shaderPtr->n_uniforms;
    struct UniformSorter {
        static int
        sort(const void *left, const void *right)
        {
            const Fx9__Effect__Uniform *lvalue = *static_cast<Fx9__Effect__Uniform *const *>(left);
            const Fx9__Effect__Uniform *rvalue = *static_cast<Fx9__Effect__Uniform *const *>(right);
            return StringUtils::compare(lvalue->name, rvalue->name);
        }
    };
    tinystl::vector<Fx9__Effect__Uniform *, TinySTLAllocator> uniforms(
        shaderPtr->uniforms, shaderPtr->uniforms + numUniforms);
    qsort(uniforms.data(), uniforms.size(), sizeof(uniforms[0]), UniformSorter::sort);
    for (size_t i = 0; i < numUniforms; i++) {
        const Fx9__Effect__Uniform *uniformPtr = uniforms[i];
        StringUtils::format(buffer, sizeof(buffer), " [shader:uniform:%s]\n", uniformPtr->name);
        newShaderCode.append(buffer);
        StringUtils::format(buffer, sizeof(buffer), " index = %d\n", uniformPtr->index);
        newShaderCode.append(buffer);
        StringUtils::format(buffer, sizeof(buffer), " constant = %d\n", uniformPtr->constant_index);
        newShaderCode.append(buffer);
    }
    struct AttributeSorter {
        static int
        sort(const void *left, const void *right)
        {
            const Fx9__Effect__Attribute *lvalue = *static_cast<Fx9__Effect__Attribute *const *>(left);
            const Fx9__Effect__Attribute *rvalue = *static_cast<Fx9__Effect__Attribute *const *>(right);
            return StringUtils::compare(lvalue->name, rvalue->name);
        }
    };
    const size_t numAttributes = shaderPtr->n_inputs;
    tinystl::vector<Fx9__Effect__Attribute *, TinySTLAllocator> inputs(
        shaderPtr->inputs, shaderPtr->inputs + numAttributes);
    qsort(inputs.data(), inputs.size(), sizeof(inputs[0]), AttributeSorter::sort);
    for (size_t i = 0; i < numAttributes; i++) {
        const Fx9__Effect__Attribute *attributePtr = inputs[i];
        StringUtils::format(buffer, sizeof(buffer), " [shader:input:%s]\n", attributePtr->name);
        newShaderCode.append(buffer);
        StringUtils::format(buffer, sizeof(buffer), " index = %d\n", attributePtr->index);
        newShaderCode.append(buffer);
        StringUtils::format(buffer, sizeof(buffer), " usage = %d\n", attributePtr->usage);
        newShaderCode.append(buffer);
    }
    const size_t numOutputs = shaderPtr->n_outputs;
    tinystl::vector<Fx9__Effect__Attribute *, TinySTLAllocator> outputs(
        shaderPtr->outputs, shaderPtr->outputs + numOutputs);
    qsort(outputs.data(), outputs.size(), sizeof(outputs[0]), AttributeSorter::sort);
    for (size_t i = 0; i < numOutputs; i++) {
        const Fx9__Effect__Attribute *outputPtr = outputs[i];
        StringUtils::format(buffer, sizeof(buffer), " [shader:output:%s]\n", outputPtr->name);
        newShaderCode.append(buffer);
        StringUtils::format(buffer, sizeof(buffer), " index = %d\n", outputPtr->index);
        newShaderCode.append(buffer);
        StringUtils::format(buffer, sizeof(buffer), " usage = %d\n", outputPtr->usage);
        newShaderCode.append(buffer);
    }
    newShaderCode.append("*/\n");
}

void
PrivateEffectUtils::setRegisterIndex(const Fx9__Effect__Symbol *symbolPtr, RegisterIndexMap &registerIndices)
{
    const String name(symbolPtr->name);
    RegisterIndexMap::iterator it = registerIndices.find(name);
    if (it != registerIndices.end()) {
        RegisterIndex &index = it->second;
        if (index.m_type == nanoem_u32_t(-1)) {
            index.m_index = symbolPtr->register_index;
            index.m_count = symbolPtr->register_count;
            index.m_type = glm::clamp(nanoem_u32_t(symbolPtr->register_set),
                nanoem_u32_t(FX9__EFFECT__SYMBOL__REGISTER_SET__RS_BOOL),
                nanoem_u32_t(FX9__EFFECT__SYMBOL__REGISTER_SET__RS_FLOAT4));
        }
    }
    else {
        const RegisterIndex &registerIndex = createSymbolRegisterIndex(
            symbolPtr, FX9__EFFECT__SYMBOL__REGISTER_SET__RS_BOOL, FX9__EFFECT__SYMBOL__REGISTER_SET__RS_FLOAT4);
        registerIndices.insert(tinystl::make_pair(name, registerIndex));
    }
}

void
PrivateEffectUtils::attachShaderSource(const Fx9__Effect__Shader *shaderPtr, const char *techniqueName,
    const char *passName, sg_shader_stage_desc &desc, String &newShaderCode, Error &error)
{
    switch (shaderPtr->body_case) {
    case FX9__EFFECT__SHADER__BODY_GLSL: {
        appendShaderVariablesHeaderComment(shaderPtr, newShaderCode);
        newShaderCode.append(shaderPtr->glsl);
        desc.source = newShaderCode.c_str();
        break;
    }
    case FX9__EFFECT__SHADER__BODY_HLSL: {
        appendShaderVariablesHeaderComment(shaderPtr, newShaderCode);
        newShaderCode.append(shaderPtr->hlsl);
#if BX_PLATFORM_WINDOWS
        bx::MemoryBlock block(g_emapp_allocator);
        bx::MemoryWriter writer(&block);
        String newFilename(techniqueName);
        newFilename.append("_");
        newFilename.append(passName);
        const char *shaderProfile = nullptr;
        switch (shaderPtr->type) {
        case FX9__EFFECT__SHADER__TYPE__ST_PIXEL: {
            newFilename.append("_PS.hlsl");
            shaderProfile = "ps_4_1";
            break;
        }
        case FX9__EFFECT__SHADER__TYPE__ST_VERTEX: {
            newFilename.append("_VS.hlsl");
            shaderProfile = "vs_4_1";
            break;
        }
        default:
            nanoem_assert(false, "must NOT reach here");
            break;
        }
        const UINT flags = 0
#if defined(NANOEM_ENABLE_DEBUG_LABEL)
            | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG_NAME_FOR_SOURCE
#else
            | D3DCOMPILE_OPTIMIZATION_LEVEL1
#endif
            ;
        if (g_D3DCompile) {
            const UINT flags2 = D3DCOMPILE_FLAGS2_FORCE_ROOT_SIGNATURE_LATEST;
            ID3DBlob *assemblyBlob = nullptr, *errorBlob = nullptr;
            HRESULT result = g_D3DCompile(newShaderCode.c_str(), newShaderCode.size(), newFilename.c_str(), nullptr,
                nullptr, "main", shaderProfile, flags, flags2, &assemblyBlob, &errorBlob);
            if (!FAILED(result)) {
                const char *ptr = static_cast<const char *>(assemblyBlob->GetBufferPointer());
                newShaderCode = String(ptr, assemblyBlob->GetBufferSize());
                desc.bytecode.ptr = reinterpret_cast<const nanoem_u8_t *>(newShaderCode.c_str());
                desc.bytecode.size = newShaderCode.size();
                assemblyBlob->Release();
            }
            else {
                String reason(static_cast<const char *>(errorBlob->GetBufferPointer()), errorBlob->GetBufferSize());
                error = Error(reason.c_str(), result, Error::kDomainTypeOS);
            }
        }
        else {
            error = Error("Failed to load D3DCompile family dll.", 0, Error::kDomainTypeOS);
        }
#else
        BX_UNUSED_3(error, techniqueName, passName);
#endif
        break;
    }
    case FX9__EFFECT__SHADER__BODY_MSL: {
        appendShaderVariablesHeaderComment(shaderPtr, newShaderCode);
        newShaderCode.append(shaderPtr->msl);
        desc.source = newShaderCode.c_str();
        desc.entry = "fx9_metal_main";
        break;
    }
    case FX9__EFFECT__SHADER__BODY_SPIRV: {
        desc.bytecode.ptr = shaderPtr->spirv.data;
        desc.bytecode.size = shaderPtr->spirv.len;
        break;
    }
    case FX9__EFFECT__SHADER__BODY__NOT_SET:
    default:
        break;
    }
}

void
PrivateEffectUtils::retrieveShaderSymbols(const Fx9__Effect__Shader *shaderPtr, RegisterIndexMap &registerIndices,
    UniformBufferOffsetMap &uniformBufferOffsetMap)
{
    const size_t numUniforms = shaderPtr->n_uniforms;
    for (size_t i = 0; i < numUniforms; i++) {
        const Fx9__Effect__Uniform *uniformPtr = shaderPtr->uniforms[i];
        uniformBufferOffsetMap.insert(tinystl::make_pair(uniformPtr->index, uniformPtr->index));
    }
    const size_t numSymbols = shaderPtr->n_symbols;
    for (size_t i = 0; i < numSymbols; i++) {
        const Fx9__Effect__Symbol *symbolPtr = shaderPtr->symbols[i];
        setRegisterIndex(symbolPtr, registerIndices);
    }
}

void
PrivateEffectUtils::setImageTypesFromSampler(
    const Fx9__Effect__Shader *shaderPtr, sg_shader_image_desc *shaderSamplers) NANOEM_DECL_NOEXCEPT
{
    const size_t numSamplers = shaderPtr->n_samplers;
    for (size_t i = 0; i < numSamplers; i++) {
        const Fx9__Effect__Sampler *samplerPtr = shaderPtr->samplers[i];
        const nanoem_u32_t samplerIndex = samplerPtr->index;
        if (samplerIndex < SG_MAX_SHADERSTAGE_IMAGES) {
            sg_shader_image_desc &desc = shaderSamplers[samplerIndex];
            desc.name = samplerPtr->sampler_name;
            switch (samplerPtr->type) {
            case FX9__EFFECT__SAMPLER__TYPE__SAMPLER_2D:
            default:
                desc.image_type = SG_IMAGETYPE_2D;
                break;
            case FX9__EFFECT__SAMPLER__TYPE__SAMPLER_VOLUME:
                desc.image_type = SG_IMAGETYPE_3D;
                break;
            case FX9__EFFECT__SAMPLER__TYPE__SAMPLER_CUBE:
                desc.image_type = SG_IMAGETYPE_CUBE;
                break;
            }
        }
    }
}

void
PrivateEffectUtils::retrievePixelShaderSamplers(const Fx9__Effect__Pass *pass, sg_shader_image_desc *shaderSamplers,
    ImageDescriptionMap &textureDescriptions, SamplerRegisterIndexMap &shaderRegisterIndices)
{
    const Fx9__Effect__Shader *shaderPtr = pass->pixel_shader;
    setImageTypesFromSampler(shaderPtr, shaderSamplers);
    for (size_t i = 0, numSamplers = shaderPtr->n_samplers; i < numSamplers; i++) {
        const Fx9__Effect__Sampler *samplerPtr = shaderPtr->samplers[i];
        const String name(samplerPtr->texture_name);
        const nanoem_u32_t samplerIndex = samplerPtr->index;
        if (!name.empty()) {
            SamplerRegisterIndexMap::iterator it = shaderRegisterIndices.find(name);
            if (it != shaderRegisterIndices.end()) {
                SamplerRegisterIndex &samplerRegisterIndex = it->second;
                samplerRegisterIndex.m_indices.push_back(samplerIndex);
            }
            else {
                SamplerRegisterIndex samplerRegisterIndex;
                samplerRegisterIndex.m_indices.push_back(samplerIndex);
                samplerRegisterIndex.m_type = FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_TEXTURE;
                samplerRegisterIndex.m_name = samplerPtr->sampler_name;
                shaderRegisterIndices.insert(tinystl::make_pair(name, samplerRegisterIndex));
            }
            if (textureDescriptions.find(name) == textureDescriptions.end()) {
                sg_image_desc desc;
                Inline::clearZeroMemory(desc);
                convertImageDescription<Fx9__Effect__Sampler, Fx9__Effect__SamplerState>(samplerPtr, desc);
                if (samplerIndex < SG_MAX_SHADERSTAGE_IMAGES) {
                    desc.type = shaderSamplers[samplerIndex].image_type;
                }
                textureDescriptions.insert(tinystl::make_pair(name, desc));
            }
        }
    }
}

void
PrivateEffectUtils::retrieveVertexShaderSamplers(const Fx9__Effect__Pass *pass, sg_shader_image_desc *shaderSamplers,
    ImageDescriptionMap &textureDescriptions, SamplerRegisterIndexMap &shaderRegisterIndices)
{
    const Fx9__Effect__Shader *shaderPtr = pass->vertex_shader;
    setImageTypesFromSampler(shaderPtr, shaderSamplers);
    for (size_t i = 0, numSamplers = shaderPtr->n_samplers; i < numSamplers; i++) {
        const Fx9__Effect__Sampler *samplerPtr = shaderPtr->samplers[i];
        const String name(samplerPtr->texture_name);
        const nanoem_u32_t samplerIndex = samplerPtr->index;
        if (!name.empty()) {
            SamplerRegisterIndexMap::iterator it = shaderRegisterIndices.find(name);
            if (it != shaderRegisterIndices.end()) {
                SamplerRegisterIndex &samplerRegisterIndex = it->second;
                samplerRegisterIndex.m_indices.push_back(samplerIndex);
            }
            else {
                SamplerRegisterIndex samplerRegisterIndex;
                samplerRegisterIndex.m_indices.push_back(samplerIndex);
                samplerRegisterIndex.m_type = FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_TEXTURE;
                samplerRegisterIndex.m_name = samplerPtr->sampler_name;
                shaderRegisterIndices.insert(tinystl::make_pair(name, samplerRegisterIndex));
            }
            if (textureDescriptions.find(name) == textureDescriptions.end()) {
                sg_image_desc desc;
                Inline::clearZeroMemory(desc);
                convertImageDescription<Fx9__Effect__Sampler, Fx9__Effect__SamplerState>(samplerPtr, desc);
                if (samplerIndex < SG_MAX_SHADERSTAGE_IMAGES) {
                    desc.type = shaderSamplers[samplerIndex].image_type;
                }
                textureDescriptions.insert(tinystl::make_pair(name, desc));
            }
        }
    }
}

void
PrivateEffectUtils::fillShaderImageDescriptions(const char *prefix, sg_shader_image_desc *desc, StringList &names)
{
    bool found = false;
    for (int i = SG_MAX_SHADERSTAGE_IMAGES - 1; i >= 0; i--) {
        sg_shader_image_desc &id = desc[i];
        if (found && id.image_type == _SG_IMAGETYPE_DEFAULT) {
            String name;
            StringUtils::format(name, "%s_%d", prefix, i);
            names.push_back(name);
            id.name = name.c_str();
            id.image_type = SG_IMAGETYPE_2D;
        }
        else if (id.image_type != _SG_IMAGETYPE_DEFAULT) {
            found = true;
        }
    }
}

void
PrivateEffectUtils::parseSubsetString(char *ptr, int numMaterials, Effect::MaterialIndexSet &output)
{
    if (char *q = StringUtils::indexOf(ptr, '-')) {
        *q = '\0';
        q += 1;
        char *startPtr, *endPtr;
        int numMaterialsMinusOne = numMaterials - 1, startMaterialIndex = StringUtils::parseInteger(ptr, &startPtr),
            endMaterialIndex = StringUtils::parseInteger(q, &endPtr);
        if (startMaterialIndex < numMaterials && q == endPtr) {
            endMaterialIndex = numMaterialsMinusOne;
        }
        startMaterialIndex = glm::clamp(startMaterialIndex, 0, numMaterialsMinusOne);
        endMaterialIndex = glm::clamp(endMaterialIndex, 0, numMaterialsMinusOne);
        if (endMaterialIndex > 0 && startMaterialIndex > endMaterialIndex) {
            int tmp = endMaterialIndex;
            endMaterialIndex = startMaterialIndex;
            startMaterialIndex = tmp;
        }
        for (int i = startMaterialIndex; i <= endMaterialIndex; i++) {
            output.insert(i);
        }
    }
    else {
        char *p = nullptr;
        int materialIndex = StringUtils::parseInteger(ptr, &p);
        if (p != ptr && materialIndex >= 0 && materialIndex < numMaterials) {
            output.insert(materialIndex);
        }
    }
}

bool
PrivateEffectUtils::hasShaderSource(const sg_shader_desc &desc) NANOEM_DECL_NOEXCEPT
{
    return (desc.vs.source && desc.fs.source) || (desc.vs.bytecode.size > 0 && desc.fs.bytecode.size > 0);
}

Vector3
PrivateEffectUtils::angle(const Accessory *accessory)
{
    if (accessory->project()->resolveBone(accessory->outsideParent())) {
        return glm::eulerAngles(glm::quat_cast(accessory->fullWorldTransform()));
    }
    else {
        return accessory->orientation();
    }
}

} /* namespace anonymous */
namespace dx9ms {

/* DX9MS */
void createShader(const Fx9__Effect__Dx9ms__Shader *shaderPtr, sg_shader_desc &desc, String &newShaderCode,
    StringMap &shaderOutputVariables);
void retrieveShaderSymbols(const Fx9__Effect__Dx9ms__Shader *shaderPtr, RegisterIndexMap &registerIndices,
    UniformBufferOffsetMap &uniformBufferOffsetMap);
void retrievePixelShaderSamplers(const Fx9__Effect__Dx9ms__Pass *pass, sg_shader_image_desc *pixelShaderSamplers,
    ImageDescriptionMap &pixelShaderSamplerFlags, SamplerRegisterIndexMap &shaderRegisterIndices);
void retrieveVertexShaderSamplers(const Fx9__Effect__Dx9ms__Pass *pass, sg_shader_image_desc *vertexShaderSamplers,
    ImageDescriptionMap &vertexShaderSamplerFlags, SamplerRegisterIndexMap &shaderRegisterIndices);
void parsePreshader(const Fx9__Effect__Dx9ms__Shader *shaderPtr, Preshader &preshader, GlobalUniform::Buffer &buffer,
    RegisterIndexMap &registerIndices);

} /* namespace dx9ms */
} /* namespace effect */

using namespace nanoem::effect;

const String Effect::kPassTypeObject = String("object");
const String Effect::kPassTypeObjectSelfShadow = String("object_ss");
const String Effect::kPassTypeShadow = String("shadow");
const String Effect::kPassTypeEdge = String("edge");
const String Effect::kPassTypeZplot = String("zplot");
const String Effect::kOffscreenOwnerNameMain = String("Main");
const char *const Effect::kSourceFileExtension = "fx";
const char *const Effect::kBinaryFileExtension = "fxn";
const ScriptIndex ScriptIndex::kInvalid = ScriptIndex(SIZE_MAX, SIZE_MAX);

void
Effect::initializeD3DCompiler()
{
#if BX_PLATFORM_WINDOWS
    if (!g_D3DCompilerDll) {
        static const char *kD3DCompilerDllCandidates[] = { "D3DCompiler_47.dll", "D3DCompiler_46.dll" };
        for (size_t i = 0; i < BX_COUNTOF(kD3DCompilerDllCandidates); i++) {
            if (void *handle = bx::dlopen(kD3DCompilerDllCandidates[i])) {
                g_D3DCompilerDll = handle;
                g_D3DCompile = reinterpret_cast<pfn_D3DCompile>(bx::dlsym(handle, "D3DCompile"));
                break;
            }
        }
    }
#endif /* BX_PLATFORM_WINDOWS */
}

void
Effect::terminateD3DCompiler()
{
#if BX_PLATFORM_WINDOWS
    if (g_D3DCompilerDll) {
        bx::dlclose(g_D3DCompilerDll);
        g_D3DCompile = nullptr;
        g_D3DCompilerDll = nullptr;
    }
#endif /* BX_PLATFORM_WINDOWS */
}

StringList
Effect::loadableExtensions()
{
    static const String kLoadableEffectExtensions[] = { String("fx"), String("fxsub"), String() };
    return StringList(
        &kLoadableEffectExtensions[0], &kLoadableEffectExtensions[BX_COUNTOF(kLoadableEffectExtensions) - 1]);
}

StringSet
Effect::loadableExtensionsSet()
{
    return ListUtils::toSetFromList<String>(loadableExtensions());
}

bool
Effect::isLoadableExtension(const String &extension)
{
    return FileUtils::isLoadableExtension(extension, loadableExtensionsSet());
}

bool
Effect::isLoadableExtension(const URI &fileURI)
{
    return isLoadableExtension(fileURI.pathExtension());
}

String
Effect::resolveFilePath(const char *filePath, const char *extension)
{
    String s;
    bx::StringView view(bx::strFindBlock(filePath, '[', ']'));
    if (!view.isEmpty()) {
        const char *lastDirectory = strrchr(filePath, '/');
        if (lastDirectory) {
            s.append(filePath, lastDirectory + 1);
        }
        s.append(view.getTerm() + 1, view.getTerm() + view.getLength());
        s.append(".");
        s.append(extension);
    }
    else if (const char *p = strrchr(filePath, '.')) {
        s.append(filePath, p);
        s.append(".");
        s.append(extension);
    }
    return s;
}

URI
Effect::resolveURI(const URI &fileURI, const char *extension)
{
    if (Project::isArchiveURI(fileURI)) {
        return URI::createFromFilePath(
            fileURI.absolutePath(), resolveFilePath(fileURI.fragmentConstString(), extension));
    }
    else {
        return URI::createFromFilePath(resolveFilePath(fileURI.absolutePathConstString(), extension));
    }
}

URI
Effect::resolveSourceURI(IFileManager *fileManager, const URI &baseURI)
{
    URI fileURI;
    if (plugin::EffectPlugin *plugin = fileManager->sharedEffectPlugin()) {
        PluginFactory::EffectPluginProxy proxy(plugin);
        const StringList &extensionsList = proxy.availableExtensions();
        for (StringList::const_iterator it = extensionsList.begin(), end = extensionsList.end(); it != end; ++it) {
            String sourceEffectPath(baseURI.absolutePathByDeletingPathExtension());
            sourceEffectPath.append(".");
            sourceEffectPath.append(it->c_str());
            if (FileUtils::exists(sourceEffectPath.c_str())) {
                fileURI = URI::createFromFilePath(sourceEffectPath);
                break;
            }
        }
    }
    return fileURI;
}

bool
Effect::compileFromSource(
    const URI &fileURI, IFileManager *fileManager, bool mipmap, ByteArray &output, Progress &progress, Error &error)
{
    bool succeeded = false;
    if (plugin::EffectPlugin *plugin = fileManager->sharedEffectPlugin()) {
        PluginFactory::EffectPluginProxy proxy(plugin);
        proxy.setMipmapEnabled(mipmap, error);
        char text[Inline::kLongNameStackBufferSize];
        StringUtils::format(text, sizeof(text), "Compiling Effect %s", fileURI.lastPathComponent().c_str());
        progress.setText(text);
        succeeded = proxy.compile(fileURI, output);
        if (!succeeded) {
            error = proxy.error();
        }
    }
    return succeeded;
}

bool
Effect::isScriptClassObject(ScriptClassType value) NANOEM_DECL_NOEXCEPT
{
    return value == IEffect::kScriptClassTypeObject || value == IEffect::kScriptClassTypeSceneObject;
}

void
Effect::parseScript(const String &script, effect::ScriptCommandMap &output)
{
    int clearColorScriptIndex, clearDepthScriptIndex;
    bool hasScriptExternal;
    parseScript(script, output, clearColorScriptIndex, clearDepthScriptIndex, hasScriptExternal);
}

void
Effect::parseScript(const String &script, ScriptCommandMap &output, int &clearColorScriptIndex,
    int &clearDepthScriptIndex, bool &hasScriptExternal)
{
    static const String kScriptClearColorValue = "Color", kScriptClearDepthValue = "Depth";
    hasScriptExternal = false;
    if (!script.empty()) {
        MutableString newScriptString;
        StringUtils::copyString(script, newScriptString);
        char *ptr = newScriptString.data();
        bool hasClearViewportColor = true, hasClearViewportDepth = true;
        clearColorScriptIndex = clearDepthScriptIndex = -1;
        while (char *p = StringUtils::indexOf(ptr, ';')) {
            *p = '\0';
            if (char *q = StringUtils::indexOf(ptr, '=')) {
                *q = '\0';
                q += 1;
                const String keyString(StringUtils::skipWhiteSpaces(ptr)), value(StringUtils::skipWhiteSpaces(q));
                const char *key = keyString.c_str();
                if (StringUtils::equals(key, kScriptRenderColorTargetValueLiteral) ||
                    StringUtils::equals(key, kScriptRenderColorTarget0ValueLiteral)) {
                    output.push_back(tinystl::make_pair(kScriptCommandTypeSetRenderColorTarget0, value));
                    if (value.empty()) {
                        hasClearViewportColor = hasClearViewportDepth = false;
                    }
                }
                else if (StringUtils::equals(key, kScriptRenderColorTarget1ValueLiteral)) {
                    output.push_back(tinystl::make_pair(kScriptCommandTypeSetRenderColorTarget1, value));
                }
                else if (StringUtils::equals(key, kScriptRenderColorTarget2ValueLiteral)) {
                    output.push_back(tinystl::make_pair(kScriptCommandTypeSetRenderColorTarget2, value));
                }
                else if (StringUtils::equals(key, kScriptRenderColorTarget3ValueLiteral)) {
                    output.push_back(tinystl::make_pair(kScriptCommandTypeSetRenderColorTarget3, value));
                }
                else if (StringUtils::equals(key, kScriptRenderDepthStencilValueLiteral)) {
                    output.push_back(tinystl::make_pair(kScriptCommandTypeSetRenderDepthStencilTarget, value));
                }
                else if (StringUtils::equals(key, kScriptClearSetColorValueLiteral)) {
                    output.push_back(tinystl::make_pair(kScriptCommandTypeClearSetColor, value));
                }
                else if (StringUtils::equals(key, kScriptClearSetDepthValueLiteral)) {
                    output.push_back(tinystl::make_pair(kScriptCommandTypeClearSetDepth, value));
                }
                else if (StringUtils::equals(key, kScriptClearValueLiteral)) {
                    output.push_back(tinystl::make_pair(kScriptCommandTypeClear, value));
                    if (value == kScriptClearColorValue) {
                        hasClearViewportColor = true;
                    }
                    else if (value == kScriptClearDepthValue) {
                        hasClearViewportDepth = true;
                    }
                }
                else if (StringUtils::equals(key, kScriptScriptExternalValueLiteral)) {
                    output.push_back(tinystl::make_pair(kScriptCommandTypeSetScriptExternal, value));
                    hasScriptExternal = true;
                }
                else if (StringUtils::equals(key, kScriptPassValueLiteral)) {
                    if (!hasClearViewportColor && !hasClearViewportDepth) {
                        clearColorScriptIndex = Inline::saturateInt32(output.size());
                        clearDepthScriptIndex = Inline::saturateInt32(output.size());
                    }
                    else if (!hasClearViewportColor) {
                        clearColorScriptIndex = Inline::saturateInt32(output.size());
                    }
                    else if (!hasClearViewportDepth) {
                        clearDepthScriptIndex = Inline::saturateInt32(output.size());
                    }
                    output.push_back(tinystl::make_pair(kScriptCommandTypeExecutePass, value));
                }
                else if (StringUtils::equals(key, kScriptLoopByCountValueLiteral)) {
                    output.push_back(tinystl::make_pair(kScriptCommandTypePushLoopCounter, value));
                }
                else if (StringUtils::equals(key, kScriptLoopGetIndexValueLiteral)) {
                    output.push_back(tinystl::make_pair(kScriptCommandTypeGetLoopIndex, value));
                }
                else if (StringUtils::equals(key, kScriptLoopEndValueLiteral)) {
                    output.push_back(tinystl::make_pair(kScriptCommandTypePopLoopCounter, String()));
                }
                else if (StringUtils::equals(key, kScriptDrawValueLiteral)) {
                    output.push_back(tinystl::make_pair(kScriptCommandTypeDraw, value));
                }
            }
            ptr = p + 1;
        }
    }
}

void
Effect::parseSubset(const String &script, nanoem_rsize_t numMaterials, MaterialIndexSet &output)
{
    if (!script.empty()) {
        MutableString newScriptString;
        StringUtils::copyString(script, newScriptString);
        char *ptr = newScriptString.data();
        int saturatedNumMaterials = Inline::saturateInt32(numMaterials);
        while (char *p = StringUtils::indexOf(ptr, ',')) {
            *p = '\0';
            PrivateEffectUtils::parseSubsetString(ptr, saturatedNumMaterials, output);
            ptr = p + 1;
        }
        PrivateEffectUtils::parseSubsetString(ptr, saturatedNumMaterials, output);
    }
}

const char *
Effect::toString(ParameterType type) NANOEM_DECL_NOEXCEPT
{
    static const char *const kTypeString[] = { "Unknown", "Void", "String", "Texture", "Texture1D", "Texture2D",
        "Texture3D", "TextureCube", "Sampler", "Sampler1D", "Sampler2D", "Sampler3D", "SamplerCube", "Bool", "Bool2",
        "Bool3", "Bool4", "Bool2x1", "Bool2x2", "Bool2x3", "Bool2x4", "Bool3x1", "Bool3x2", "Bool3x3", "Bool3x4",
        "Bool4x1", "Bool4x2", "Bool4x3", "Bool4x4", "Int", "Int2", "Int3", "Int4", "Int2x1", "Int2x2", "Int2x3",
        "Int2x4", "Int3x1", "Int3x2", "Int3x3", "Int3x4", "Int4x1", "Int4x2", "Int4x3", "Int4x4", "Float", "Float2",
        "Float3", "Float4", "Float2x1", "Float2x2", "Float2x3", "Float2x4", "Float3x1", "Float3x2", "Float3x3",
        "Float3x4", "Float4x1", "Float4x2", "Float4x3", "Float4x4" };
    const char *result = kTypeString[0];
    if (type >= kParameterTypeUnknown && type < kParameterTypeMaxEnum) {
        result = kTypeString[type];
    }
    return result;
}

Effect::Effect(Project *project, GlobalUniform *globalUniformPtr, AccessoryProgramBundle *accessoryProgramBundlePtr,
    ModelProgramBundle *modelProgramBundlePtr)
    : m_project(project)
    , m_globalUniformPtr(globalUniformPtr)
    , m_logger(nullptr)
    , m_fallbackAccessoryProgramBundle(nullptr)
    , m_fallbackModelProgramBundle(nullptr)
    , m_scriptOutput(kScriptOutputColorValueLiteral)
    , m_scriptClass(kScriptClassTypeSceneObject)
    , m_scriptOrder(kScriptOrderTypeStandard)
    , m_userData(nullptr, nullptr)
    , m_clearColor(Vector4(0xff))
    , m_clearDepth(1.0f)
    , m_enabled(false, false)
    , m_enablePassUniformBufferInspection(false)
    , m_initializeGlobal(false)
    , m_hasScriptExternal(false)
    , m_needsBehaviorCompatibility(false)
{
    nanoem_assert(m_project, "must NOT be nullptr");
    nanoem_assert(m_globalUniformPtr, "must NOT be nullptr");
    Inline::clearZeroMemory(m_currentRenderTargetPassDescription);
    Inline::clearZeroMemory(m_currentNamedPrimaryRenderTargetColorImageDescription.second);
    Inline::clearZeroMemory(m_currentNamedDepthStencilImageDescription.second);
    m_fallbackAccessoryProgramBundle = accessoryProgramBundlePtr;
    m_fallbackModelProgramBundle = modelProgramBundlePtr;
    m_firstImageHandle = project->sharedFallbackImage();
    m_logger = nanoem_new(Logger);
    addSemanticParameterHandler("WORLD", &handleWorldMatrixSemantic);
    addSemanticParameterHandler("VIEW", &handleViewMatrixSemantic);
    addSemanticParameterHandler("PROJECTION", &handleProjectMatrixSemantic);
    addSemanticParameterHandler("WORLDVIEW", &handleWorldViewMatrixSemantic);
    addSemanticParameterHandler("VIEWPROJECTION", &handleViewProjectMatrixSemantic);
    addSemanticParameterHandler("WORLDVIEWPROJECTION", &handleWorldViewProjectionMatrixSemantic);
    addSemanticParameterHandler("WORLDINVERSE", &handleWorldMatrixSemanticInverse);
    addSemanticParameterHandler("VIEWINVERSE", &handleViewMatrixSemanticInverse);
    addSemanticParameterHandler("PROJECTIONINVERSE", &handleProjectMatrixSemanticInverse);
    addSemanticParameterHandler("WORLDVIEWINVERSE", &handleWorldViewMatrixSemanticInverse);
    addSemanticParameterHandler("VIEWPROJECTIONINVERSE", &handleViewProjectMatrixSemanticInverse);
    addSemanticParameterHandler("WORLDVIEWPROJECTIONINVERSE", &handleWorldViewProjectionMatrixSemanticInverse);
    addSemanticParameterHandler("WORLDTRANSPOSE", &handleWorldMatrixSemanticTranspose);
    addSemanticParameterHandler("VIEWTRANSPOSE", &handleViewMatrixSemanticTranspose);
    addSemanticParameterHandler("PROJECTIONTRANSPOSE", &handleProjectMatrixSemanticTranspose);
    addSemanticParameterHandler("WORLDVIEWTRANSPOSE", &handleWorldViewMatrixSemanticTranspose);
    addSemanticParameterHandler("VIEWPROJECTIONTRANSPOSE", &handleViewProjectMatrixSemanticTranspose);
    addSemanticParameterHandler("WORLDVIEWPROJECTIONTRANSPOSE", &handleWorldViewProjectionMatrixSemanticTranspose);
    addSemanticParameterHandler("WORLDINVERSETRANSPOSE", &handleWorldMatrixSemanticInverseTranspose);
    addSemanticParameterHandler("VIEWINVERSETRANSPOSE", &handleViewMatrixSemanticInverseTranspose);
    addSemanticParameterHandler("PROJECTIONINVERSETRANSPOSE", &handleProjectMatrixSemanticInverseTranspose);
    addSemanticParameterHandler("WORLDVIEWINVERSETRANSPOSE", &handleWorldViewMatrixSemanticInverseTranspose);
    addSemanticParameterHandler("VIEWPROJECTIONINVERSETRANSPOSE", &handleViewProjectMatrixSemanticInverseTranspose);
    addSemanticParameterHandler(
        "WORLDVIEWPROJECTIONINVERSETRANSPOSE", &handleWorldViewProjectionMatrixSemanticInverseTranspose);
    addSemanticParameterHandler("DIFFUSE", &handleDiffuseSemantic);
    addSemanticParameterHandler("AMBIENT", &handleAmbientSemantic);
    addSemanticParameterHandler("EMISSIVE", &handleEmissiveSemantic);
    addSemanticParameterHandler("SPECULAR", &handleSpecularSemantic);
    addSemanticParameterHandler("SPECULARPOWER", &handleSpecularPowerSemantic);
    addSemanticParameterHandler("TOONCOLOR", &handleToonColorSemantic);
    addSemanticParameterHandler("EDGECOLOR", &handleEdgeColorSemantic);
    addSemanticParameterHandler("GROUNDSHADOWCOLOR", &handleGroundShadowColorSemantic);
    addSemanticParameterHandler("POSITION", &handlePositionSemantic);
    addSemanticParameterHandler("DIRECTION", &handleDirectionSemantic);
    addSemanticParameterHandler("MATERIALTEXTURE", &handleMaterialTextureSemantic);
    addSemanticParameterHandler("MATERIALSPHEREMAP", &handleMaterailSphereMapSemantic);
    addSemanticParameterHandler("MATERIALTOONTEXTURE", &handleMaterialToonTextureSemantic);
    addSemanticParameterHandler("ADDINGTEXTURE", &handleAddingTextureSemantic);
    addSemanticParameterHandler("MULTIPLYINGTEXTURE", &handleMultiplyingTextureSemantic);
    addSemanticParameterHandler("ADDINGSPHERETEXTURE", &handleAddingSphereTextureSemantic);
    addSemanticParameterHandler("MULTIPLYINGSPHERETEXTURE", &handleMultiplyingSphereTextureSemantic);
    addSemanticParameterHandler("VIEWPORTPIXELSIZE", &handleViewportPixelSizeSemantic);
    addSemanticParameterHandler("TIME", &handleTimeSemantic);
    addSemanticParameterHandler("ELAPSEDTIME", &handleElapsedTimeSemantic);
    addSemanticParameterHandler("MOUSEPOSITION", &handleMousePositionSemantic);
    addSemanticParameterHandler("LEFTMOUSEDOWN", &handleLeftMouseDownSemantic);
    addSemanticParameterHandler("MIDDLEMOUSEDOWN", &handleMiddleMouseDownSemantic);
    addSemanticParameterHandler("RIGHTMOUSEDOWN", &handleRightMouseDownSemantic);
    addSemanticParameterHandler("CONTROLOBJECT", &handleControlObjectSemantic);
    addSemanticParameterHandler("RENDERCOLORTARGET", &handleRenderColorTargetSemantic);
    addSemanticParameterHandler("RENDERDEPTHSTENCILTARGET", &handleRenderDepthStencilTargetSemantic);
    addSemanticParameterHandler("ANIMATEDTEXTURE", &handleAnimatedTextureSemantic);
    addSemanticParameterHandler("OFFSCREENRENDERTARGET", &handleOffscreenRenderTargetSemantic);
    addSemanticParameterHandler("TEXTUREVALUE", &handleTextureValueSemantic);
    addSemanticParameterHandler("STANDARDSGLOBAL", &handleStandardsGlobalSemantic);
    addImageFormat("DXT1", SG_PIXELFORMAT_BC1_RGBA);
    addImageFormat("DXT3", SG_PIXELFORMAT_BC2_RGBA);
    addImageFormat("DXT5", SG_PIXELFORMAT_BC3_RGBA);
    addImageFormat("A1", SG_PIXELFORMAT_R8);
    addImageFormat("A8", SG_PIXELFORMAT_R8);
    addImageFormat("L8", SG_PIXELFORMAT_R8);
    addImageFormat("L16", SG_PIXELFORMAT_R16);
    addImageFormat("R16F", SG_PIXELFORMAT_R16F);
    addImageFormat("R32F", SG_PIXELFORMAT_R32F);
    addImageFormat("A8L8", SG_PIXELFORMAT_RG8);
    addImageFormat("G16R16", SG_PIXELFORMAT_RG16);
    addImageFormat("G16R16F", SG_PIXELFORMAT_RG16F);
    addImageFormat("G32R32F", SG_PIXELFORMAT_RG32F);
    addImageFormat("X8R8G8B8", SG_PIXELFORMAT_RGBA8);
    addImageFormat("A8R8G8B8", SG_PIXELFORMAT_RGBA8);
    addImageFormat("A16R16G16B16", SG_PIXELFORMAT_RGBA16);
    addImageFormat("A16B16G16R16", SG_PIXELFORMAT_RGBA16);
    addImageFormat("A16R16G16B16F", SG_PIXELFORMAT_RGBA16F);
    addImageFormat("A16B16G16R16F", SG_PIXELFORMAT_RGBA16F);
    addImageFormat("A32R32G32B32F", SG_PIXELFORMAT_RGBA32F);
    addImageFormat("A32B32G32R32F", SG_PIXELFORMAT_RGBA32F);
    addImageFormat("R5G6B5", SG_PIXELFORMAT_RGBA8);
    addImageFormat("A4R4G4B4", SG_PIXELFORMAT_RGBA8);
    addImageFormat("A1R5G5B5", SG_PIXELFORMAT_RGBA8);
    addImageFormat("A2B10G10R10", SG_PIXELFORMAT_RGB10A2);
    addImageFormat("D16", SG_PIXELFORMAT_DEPTH);
    addImageFormat("D24X8", SG_PIXELFORMAT_DEPTH);
    addImageFormat("D24S8", SG_PIXELFORMAT_DEPTH_STENCIL);
    addImageFormat("D32", SG_PIXELFORMAT_DEPTH);
    addImageFormat("DF16", SG_PIXELFORMAT_DEPTH);
    addImageFormat("DF24", SG_PIXELFORMAT_DEPTH);
    addImageFormat("D32F_LOCKABLE", SG_PIXELFORMAT_DEPTH);
    addImageFormat("S8_LOCKABLE", SG_PIXELFORMAT_DEPTH_STENCIL);
}

Effect::~Effect() NANOEM_DECL_NOEXCEPT
{
    PassList passes;
    for (TechniqueList::iterator it = m_allTechniques.begin(), end = m_allTechniques.end(); it != end; ++it) {
        Technique *technique = *it;
        technique->getAllPasses(passes);
        for (PassList::iterator it2 = passes.begin(), end2 = passes.end(); it2 != end2; ++it2) {
            Pass *pass = *it2;
            nanoem_delete(pass);
        }
        nanoem_delete(technique);
    }
    m_techniqueByPassTypes.clear();
    m_allTechniques.clear();
    for (RenderTargetNormalizerMap::iterator it = m_normalizers.begin(), end = m_normalizers.end(); it != end; ++it) {
        nanoem_delete(it->second);
    }
    m_normalizers.clear();
    nanoem_delete_safe(m_logger);
    m_semanticParameterHandlers.clear();
    m_imageFormats.clear();
    m_fallbackAccessoryProgramBundle = nullptr;
    m_fallbackModelProgramBundle = nullptr;
    m_globalUniformPtr = nullptr;
    m_project = nullptr;
}

ITechnique *
Effect::findTechnique(const String &passType, const nanodxm_material_t *materialPtr, nanoem_rsize_t materialIndex,
    nanoem_rsize_t numMaterials, Accessory *accessory)
{
    nanoem_parameter_assert(materialPtr, "must not be nullptr");
    nanoem_parameter_assert(accessory, "must not be nullptr");
    ITechnique *foundTechnique = nullptr;
    if (isEnabled()) {
        const bool canFindTechnique = !(materialIndex > 0 && hasScriptExternal());
        if (canFindTechnique) {
            const String candidateTypes[] = { passType,
                passType == kPassTypeObjectSelfShadow ? kPassTypeObject : String(), String() };
            for (size_t i = 0; i < BX_COUNTOF(candidateTypes); i++) {
                foundTechnique =
                    internalFindTechnique(candidateTypes[i], numMaterials, materialIndex, materialPtr, accessory);
                if (foundTechnique) {
                    break;
                }
            }
            if (!foundTechnique) {
                foundTechnique = m_fallbackAccessoryProgramBundle->findTechnique(
                    passType, materialPtr, materialIndex, numMaterials, accessory);
            }
        }
    }
    else {
        foundTechnique = m_fallbackAccessoryProgramBundle->findTechnique(
            passType, materialPtr, materialIndex, numMaterials, accessory);
    }
    return foundTechnique;
}

ITechnique *
Effect::findTechnique(const String &passType, const nanoem_model_material_t *materialPtr, nanoem_rsize_t materialIndex,
    nanoem_rsize_t numMaterials, Model *model)
{
    nanoem_parameter_assert(materialPtr, "must not be nullptr");
    nanoem_parameter_assert(model, "must not be nullptr");
    ITechnique *foundTechnique = nullptr;
    if (isEnabled()) {
        const model::Material *material = model::Material::cast(materialPtr);
        const bool canFindTechnique = material && material->isVisible() && !(materialIndex > 0 && hasScriptExternal());
        if (canFindTechnique) {
            const String candidateTypes[] = { passType,
                passType == kPassTypeObjectSelfShadow ? kPassTypeObject : String(), String() };
            for (size_t i = 0; i < BX_COUNTOF(candidateTypes); i++) {
                foundTechnique = internalFindTechnique(candidateTypes[i], numMaterials, materialPtr);
                if (foundTechnique) {
                    break;
                }
            }
            if (!foundTechnique && !m_project->isOffscreenRenderPassActive()) {
                foundTechnique = m_fallbackModelProgramBundle->findTechnique(
                    passType, materialPtr, materialIndex, numMaterials, model);
            }
        }
    }
    else {
        foundTechnique =
            m_fallbackModelProgramBundle->findTechnique(passType, materialPtr, materialIndex, numMaterials, model);
    }
    return foundTechnique;
}

void
Effect::createImageResource(const void *ptr, size_t size, const ImageResourceParameter &parameter)
{
    Error error;
    bool registered = false;
    if (ImageLoader::validateImageSize(parameter.m_desc, parameter.m_name.c_str(), error)) {
        char label[Inline::kMarkerStringLength];
        sg_image_desc imageDescription(parameter.m_desc);
        imageDescription.max_anisotropy = m_project->maxAnisotropyValue();
        imageDescription.data.subimage[0][0].ptr = ptr;
        imageDescription.data.subimage[0][0].size = size;
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(label, sizeof(label), "Effects/%s/%s", nameConstString(), parameter.m_filename.c_str());
            imageDescription.label = label;
        }
        sg_image image = sg::make_image(&imageDescription);
        nanoem_assert(sg::query_image_state(image) == SG_RESOURCESTATE_VALID, "image must be valid");
        if (sg::is_valid(image)) {
            registerImageResource(image, parameter);
            registered = true;
        }
    }
    if (!registered) {
        m_logger->log("Image \"%s\" of \"%s\" in \"%s\" cannot be created: %s", parameter.m_name.c_str(),
            parameter.m_fileURI.absolutePathConstString(), nameConstString(), error.reasonConstString());
    }
}

void
Effect::setAllParameterObjects(const nanoem_motion_effect_parameter_t *const *parameters, nanoem_rsize_t numParameters)
{
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    String name;
    for (nanoem_rsize_t i = 0; i < numParameters; i++) {
        const nanoem_motion_effect_parameter_t *parameter = parameters[i];
        StringUtils::getUtf8String(nanoemMotionEffectParameterGetName(parameter), factory, name);
        switch (nanoemMotionEffectParameterGetType(parameter)) {
        case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_BOOL: {
            int value = *static_cast<const int *>(nanoemMotionEffectParameterGetValue(parameter));
            BoolParameterUniformMap::iterator it = m_boolParameterUniforms.find(name);
            if (it != m_boolParameterUniforms.end()) {
                it->second.m_values[0] = Vector4(value);
            }
            break;
        }
        case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_INT: {
            int value = *static_cast<const int *>(nanoemMotionEffectParameterGetValue(parameter));
            IntParameterUniformMap::iterator it = m_intParameterUniforms.find(name);
            if (it != m_intParameterUniforms.end()) {
                it->second.m_values[0] = Vector4(value);
            }
            break;
        }
        case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_FLOAT: {
            nanoem_f32_t value = *static_cast<const nanoem_f32_t *>(nanoemMotionEffectParameterGetValue(parameter));
            FloatParameterUniformMap::iterator it = m_floatParameterUniforms.find(name);
            if (it != m_floatParameterUniforms.end()) {
                it->second.m_values[0] = Vector4(value);
            }
            break;
        }
        case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_VECTOR4: {
            const Vector4 value(
                glm::make_vec4(static_cast<const nanoem_f32_t *>(nanoemMotionEffectParameterGetValue(parameter))));
            VectorParameterUniformMap::iterator it = m_vectorParameterUniforms.find(name);
            if (it != m_vectorParameterUniforms.end()) {
                it->second.m_values[0] = value;
            }
            break;
        }
        default:
            break;
        }
    }
}

void
Effect::setAllParameterObjects(const nanoem_motion_effect_parameter_t *const *fromParameters,
    nanoem_rsize_t numFromParameters, const nanoem_motion_effect_parameter_t *const *toParameters,
    nanoem_rsize_t numToParameters, nanoem_f32_t coefficient)
{
    /* FIXME: implement this */
    BX_UNUSED_5(fromParameters, numFromParameters, toParameters, numToParameters, coefficient);
}

Effect::ImageResourceList
Effect::allImageResources() const
{
    return m_imageResources;
}

IEffect::ScriptClassType
Effect::scriptClass() const NANOEM_DECL_NOEXCEPT
{
    return m_scriptClass;
}

IEffect::ScriptOrderType
Effect::scriptOrder() const NANOEM_DECL_NOEXCEPT
{
    return m_scriptOrder;
}

bool
Effect::hasScriptExternal() const NANOEM_DECL_NOEXCEPT
{
    return m_hasScriptExternal;
}

bool
Effect::load(const nanoem_u8_t *data, size_t size, Progress &progress, Error &error)
{
    SG_PUSH_GROUPF("Effect::load(name=%s)", nameConstString());
    nanoem_parameter_assert(data, "must not be nullptr");
    bool succeeded = false;
    if (Fx9__Effect__Effect *effect = fx9__effect__effect__unpack(g_protobufc_allocator, size, data)) {
        PassList passes;
        const size_t numTechniques = effect->n_techniques;
        initializeD3DCompiler();
        for (size_t i = 0; i < numTechniques; i++) {
            const Fx9__Effect__Technique *techniquePtr = effect->techniques[i];
            const size_t numPasses = techniquePtr->n_passes;
            for (size_t j = 0; j < numPasses; j++) {
                const Fx9__Effect__Pass *passPtr = techniquePtr->passes[j];
                nanoem_rsize_t numVertexShaderImages = 0, numPixelShaderImages = 0;
                switch (passPtr->implementation_case) {
                case FX9__EFFECT__PASS__IMPLEMENTATION_IMPLEMENTATION_DX9MS: {
                    const Fx9__Effect__Dx9ms__Pass *dx9ms = passPtr->implementation_dx9ms;
                    PrivateEffectUtils::countRegisterSet(
                        dx9ms->vertex_shader, m_globalUniformPtr->m_vertexShaderBuffer);
                    PrivateEffectUtils::countRegisterSet(dx9ms->pixel_shader, m_globalUniformPtr->m_pixelShaderBuffer);
                    numVertexShaderImages = dx9ms->vertex_shader->n_samplers;
                    numPixelShaderImages = dx9ms->pixel_shader->n_samplers;
                    break;
                }
                case FX9__EFFECT__PASS__IMPLEMENTATION__NOT_SET:
                default:
                    PrivateEffectUtils::countRegisterSet(
                        passPtr->vertex_shader, m_globalUniformPtr->m_vertexShaderBuffer);
                    PrivateEffectUtils::countRegisterSet(
                        passPtr->pixel_shader, m_globalUniformPtr->m_pixelShaderBuffer);
                    numVertexShaderImages = passPtr->vertex_shader->n_samplers;
                    numPixelShaderImages = passPtr->pixel_shader->n_samplers;
                    break;
                }
                if (numVertexShaderImages > SG_MAX_SHADERSTAGE_IMAGES) {
                    char message[Error::kMaxReasonLength];
                    StringUtils::format(message, sizeof(message),
                        "Cannot compile the vertex shader in the pass \"%s\" due to exceed (%d < %jd) of samplers",
                        passPtr->name, SG_MAX_SHADERSTAGE_IMAGES, numVertexShaderImages);
                    error = Error(message, "Reduce used samplers in the pass", Error::kDomainTypeApplication);
                    i = numTechniques;
                    break;
                }
                else if (numPixelShaderImages > SG_MAX_SHADERSTAGE_IMAGES) {
                    char message[Error::kMaxReasonLength];
                    StringUtils::format(message, sizeof(message),
                        "Cannot compile the pixel shader in the pass \"%s\" due to exceed (%d < %jd) of samplers",
                        passPtr->name, SG_MAX_SHADERSTAGE_IMAGES, numPixelShaderImages);
                    error = Error(message, "Reduce used samplers in the pass", Error::kDomainTypeApplication);
                    i = numTechniques;
                    break;
                }
            }
        }
        if (!error.hasReason()) {
            nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
            AnnotationMap techniqueAnnotations, passAnnotations;
            StringMap shaderOutputVariables;
            bool needsBehaviorCompatibility = false;
            for (size_t i = 0; i < numTechniques; i++) {
                const Fx9__Effect__Technique *techniquePtr = effect->techniques[i];
                PrivateEffectUtils::parseAnnotations(
                    techniquePtr->annotations, techniquePtr->n_annotations, factory, techniqueAnnotations);
                passes.clear();
                const size_t numPasses = techniquePtr->n_passes;
                for (size_t j = 0; j < numPasses; j++) {
                    const Fx9__Effect__Pass *passPtr = techniquePtr->passes[j];
                    PreshaderPair preshaders;
                    String commonHeaderComment;
                    StringUtils::format(commonHeaderComment,
                        "/*\n"
                        " [shader]\n"
                        " effect = %s\n"
                        " technique = %s\n"
                        " pass = %s\n",
                        nameConstString(), techniquePtr->name, passPtr->name);
                    UniformBufferOffsetMap vertexShaderRegisterUniformBufferOffsetMap,
                        pixelShaderRegisterUniformBufferOffsetMap;
                    String vertexShaderCode, pixelShaderCode;
                    sg_shader_desc shaderDescription;
                    Inline::clearZeroMemory(shaderDescription);
                    GlobalUniform::Buffer &pb = m_globalUniformPtr->m_pixelShaderBuffer;
                    shaderDescription.fs.uniform_blocks[0].size = pb.size();
                    shaderDescription.fs.uniform_blocks[0].uniforms[0] = sg_shader_uniform_desc { "ps_uniforms_vec4",
                        SG_UNIFORMTYPE_FLOAT4, Inline::saturateInt32(pb.elementCount()) };
                    GlobalUniform::Buffer &vb = m_globalUniformPtr->m_vertexShaderBuffer;
                    shaderDescription.vs.uniform_blocks[0].size = vb.size();
                    shaderDescription.vs.uniform_blocks[0].uniforms[0] = sg_shader_uniform_desc { "vs_uniforms_vec4",
                        SG_UNIFORMTYPE_FLOAT4, Inline::saturateInt32(vb.elementCount()) };
                    PipelineDescriptor pd;
                    pd.m_body.depth.write_enabled = true;
                    PassRegisterIndexMap passRegisterIndices;
                    char itemName[Inline::kLongNameStackBufferSize];
                    StringUtils::format(itemName, sizeof(itemName), "Effects/%s/%s/%s", nameConstString(),
                        techniquePtr->name, passPtr->name);
                    if (!progress.tryLoadingItem(itemName)) {
                        error = Error::cancelled();
                        break;
                    }
                    switch (passPtr->implementation_case) {
                    /* for old compatibility */
                    case FX9__EFFECT__PASS__IMPLEMENTATION_IMPLEMENTATION_DX9MS: {
                        const sg_backend backend = sg::query_backend();
                        if (backend == SG_BACKEND_GLCORE33) {
                            const Fx9__Effect__Dx9ms__Pass *dx9ms = passPtr->implementation_dx9ms;
                            shaderOutputVariables.clear();
                            const Fx9__Effect__Dx9ms__Shader *vertexShader = dx9ms->vertex_shader;
                            dx9ms::parsePreshader(vertexShader, preshaders.vertex,
                                m_globalUniformPtr->m_preshaderVertexShaderBuffer,
                                passRegisterIndices.m_vertexPreshader);
                            dx9ms::retrieveShaderSymbols(vertexShader, passRegisterIndices.m_vertexShader,
                                vertexShaderRegisterUniformBufferOffsetMap);
                            dx9ms::retrieveVertexShaderSamplers(dx9ms, shaderDescription.vs.images, m_imageDescriptions,
                                passRegisterIndices.m_vertexShaderSamplers);
                            String vertexShaderHeaderComment(commonHeaderComment);
                            PrivateEffectUtils::appendShaderVariablesHeaderComment<Fx9__Effect__Dx9ms__Shader,
                                Fx9__Effect__Dx9ms__Symbol, Fx9__Effect__Dx9ms__Uniform, Fx9__Effect__Dx9ms__Attribute>(
                                vertexShader, preshaders.vertex, vertexShaderHeaderComment);
                            vertexShaderCode.append("#version 150\n"
                                                    "#define highp\n"
                                                    "#define middlep\n"
                                                    "#define lowp\n"
                                                    "#define attribute in\n"
                                                    "#define varying out\n"
                                                    "#define texture2D(s, v) texture((s), (v))\n"
                                                    "#define vs_o0 gl_Position\n");
                            vertexShaderCode.append(vertexShaderHeaderComment.c_str());
                            dx9ms::createShader(
                                vertexShader, shaderDescription, vertexShaderCode, shaderOutputVariables);
                            const Fx9__Effect__Dx9ms__Shader *pixelShader = dx9ms->pixel_shader;
                            dx9ms::parsePreshader(pixelShader, preshaders.pixel,
                                m_globalUniformPtr->m_preshaderPixelShaderBuffer, passRegisterIndices.m_pixelPreshader);
                            dx9ms::retrieveShaderSymbols(pixelShader, passRegisterIndices.m_pixelShader,
                                pixelShaderRegisterUniformBufferOffsetMap);
                            dx9ms::retrievePixelShaderSamplers(dx9ms, shaderDescription.fs.images, m_imageDescriptions,
                                passRegisterIndices.m_pixelShaderSamplers);
                            String pixelShaderHeaderComment(commonHeaderComment);
                            PrivateEffectUtils::appendShaderVariablesHeaderComment<Fx9__Effect__Dx9ms__Shader,
                                Fx9__Effect__Dx9ms__Symbol, Fx9__Effect__Dx9ms__Uniform, Fx9__Effect__Dx9ms__Attribute>(
                                pixelShader, preshaders.pixel, pixelShaderHeaderComment);
                            pixelShaderCode.append("#version 150\n"
                                                   "#define highp\n"
                                                   "#define middlep\n"
                                                   "#define lowp\n"
                                                   "#define varying in\n"
                                                   "#define texture2D(s, v) texture((s), (v))\n"
                                                   "#define texture2DLod(s, v) texture((s), (v))\n"
                                                   "#define gl_FragColor o_color0\n"
                                                   "out vec4 o_color0;\n");
                            pixelShaderCode.append(pixelShaderHeaderComment.c_str());
                            dx9ms::createShader(pixelShader, shaderDescription, pixelShaderCode, shaderOutputVariables);
                            if (shaderDescription.vs.source && shaderDescription.fs.source) {
                                convertPipeline<Fx9__Effect__Dx9ms__Pass, Fx9__Effect__Dx9ms__RenderState>(
                                    passPtr->implementation_dx9ms, pd);
                                /* DX9MS's uniform name was [pv]s_uniforms_vec (not "[pv]s_uniforms_vec4") */
                                shaderDescription.fs.uniform_blocks[0].uniforms[0].name = "ps_uniforms_vec";
                                shaderDescription.vs.uniform_blocks[0].uniforms[0].name = "vs_uniforms_vec";
                                needsBehaviorCompatibility = true;
                            }
                            else {
                                continue;
                            }
                        }
                        else {
                            error = Error("This effect cannot be compiled due to the renderer is not OpenGL",
                                "Change the renderer to OpenGL or enable effect plugin and load the origin",
                                Error::kDomainTypeApplication);
                            i = numTechniques;
                        }
                        break;
                    }
                    case FX9__EFFECT__PASS__IMPLEMENTATION__NOT_SET:
                    default:
                        vertexShaderCode.append(commonHeaderComment.c_str());
                        pixelShaderCode.append(commonHeaderComment.c_str());
                        const Fx9__Effect__Shader *vertexShader = passPtr->vertex_shader,
                                                  *pixelShader = passPtr->pixel_shader;
                        PrivateEffectUtils::retrieveShaderSymbols(vertexShader, passRegisterIndices.m_vertexShader,
                            vertexShaderRegisterUniformBufferOffsetMap);
                        PrivateEffectUtils::retrieveVertexShaderSamplers(passPtr, shaderDescription.vs.images,
                            m_imageDescriptions, passRegisterIndices.m_vertexShaderSamplers);
                        PrivateEffectUtils::attachShaderSource(vertexShader, techniquePtr->name, passPtr->name,
                            shaderDescription.vs, vertexShaderCode, error);
                        PrivateEffectUtils::retrieveShaderSymbols(
                            pixelShader, passRegisterIndices.m_pixelShader, pixelShaderRegisterUniformBufferOffsetMap);
                        PrivateEffectUtils::retrievePixelShaderSamplers(passPtr, shaderDescription.fs.images,
                            m_imageDescriptions, passRegisterIndices.m_pixelShaderSamplers);
                        PrivateEffectUtils::attachShaderSource(pixelShader, techniquePtr->name, passPtr->name,
                            shaderDescription.fs, pixelShaderCode, error);
                        const size_t numRenderStates = passPtr->n_render_states;
                        typedef tinystl::vector<Fx9__Effect__RenderState *, TinySTLAllocator> RenderStateList;
                        RenderStateList states(numRenderStates);
                        for (size_t i = 0; i < numRenderStates; i++) {
                            states[i] = passPtr->render_states[i];
                        }
                        qsort(
                            states.data(), states.size(), sizeof(states[0]), PrivateEffectUtils::sortRenderStateAscent);
                        SG_PUSH_GROUPF("effect::convertPipeline(size=%d)", states.size());
                        for (RenderStateList::const_iterator it = states.begin(), end = states.end(); it != end; ++it) {
                            const Fx9__Effect__RenderState *state = *it;
                            effect::RenderState::convertPipeline(state->key, state->value, pd);
                        }
                        SG_POP_GROUP();
                        if (const char *name = vertexShader->uniform_block_name) {
                            shaderDescription.vs.uniform_blocks[0].uniforms[0].name = name;
                        }
                        if (const char *name = pixelShader->uniform_block_name) {
                            shaderDescription.fs.uniform_blocks[0].uniforms[0].name = name;
                        }
                        break;
                    }
                    if (!PrivateEffectUtils::hasShaderSource(shaderDescription)) {
                        m_logger->log("Creating the pass Effects/%s/%s/%s failed due to empty source",
                            nameConstString(), techniquePtr->name, passPtr->name);
                    }
                    else if (!error.hasReason()) {
                        shaderDescription.attrs[0] = sg_shader_attr_desc { "a_position", "SV_POSITION", 0 };
                        shaderDescription.attrs[1] = sg_shader_attr_desc { "a_normal", "NORMAL", 0 };
                        shaderDescription.attrs[2] = sg_shader_attr_desc { "a_texcoord0", "TEXCOORD", 0 };
                        shaderDescription.attrs[3] = sg_shader_attr_desc { "a_texcoord1", "TEXCOORD", 1 };
                        shaderDescription.attrs[4] = sg_shader_attr_desc { "a_texcoord2", "TEXCOORD", 2 };
                        shaderDescription.attrs[5] = sg_shader_attr_desc { "a_texcoord3", "TEXCOORD", 3 };
                        shaderDescription.attrs[6] = sg_shader_attr_desc { "a_texcoord4", "TEXCOORD", 4 };
                        shaderDescription.attrs[7] = sg_shader_attr_desc { "a_color0", "COLOR", 0 };
                        const nanoem_rsize_t numSemantics = passPtr->vertex_shader->n_semantics;
                        if (numSemantics > 0) {
                            for (nanoem_rsize_t i = 0; i < numSemantics; i++) {
                                const Fx9__Effect__Semantic *semantic = passPtr->vertex_shader->semantics[i];
                                const char *name = semantic->input_name;
                                int index = -1;
                                if (StringUtils::equalsIgnoreCase(name, "POSITION") ||
                                    StringUtils::equalsIgnoreCase(name, "SV_POSITION")) {
                                    index = 0;
                                }
                                else if (StringUtils::equalsIgnoreCase(name, "NORMAL")) {
                                    index = 1;
                                }
                                else if (StringUtils::equalsIgnoreCase(name, "TEXCOORD4")) {
                                    index = 6;
                                }
                                else if (StringUtils::equalsIgnoreCase(name, "TEXCOORD3")) {
                                    index = 5;
                                }
                                else if (StringUtils::equalsIgnoreCase(name, "TEXCOORD2")) {
                                    index = 4;
                                }
                                else if (StringUtils::equalsIgnoreCase(name, "TEXCOORD1")) {
                                    index = 3;
                                }
                                else if (StringUtils::equalsIgnoreCase(name, "TEXCOORD0") ||
                                    StringUtils::equalsIgnoreCase(name, "TEXCOORD")) {
                                    index = 2;
                                }
                                else if (StringUtils::equalsIgnoreCase(name, "COLOR0") ||
                                    StringUtils::equalsIgnoreCase(name, "COLOR")) {
                                    index = 7;
                                }
                                if (index >= 0) {
                                    shaderDescription.attrs[index] = sg_shader_attr_desc { semantic->parameter_name,
                                        semantic->output_name, Inline::saturateInt32(semantic->index) };
                                }
                            }
                        }
                        if (shaderDescription.fs.images[0].image_type == _SG_IMAGETYPE_DEFAULT) {
                            SamplerRegisterIndex samplerRegisterIndex;
                            samplerRegisterIndex.m_indices.push_back(0);
                            samplerRegisterIndex.m_type = FX9__EFFECT__DX9MS__PARAMETER_TYPE__PT_SAMPLER;
                            passRegisterIndices.m_pixelShaderSamplers.insert(
                                tinystl::make_pair(String("ps_s0"), samplerRegisterIndex));
                            shaderDescription.fs.images[0] =
                                sg_shader_image_desc { "ps_s0", SG_IMAGETYPE_2D, SG_SAMPLERTYPE_FLOAT };
                        }
                        StringList vertexShaderFallbackImageNames, pixelShaderFallbackImageNames;
                        PrivateEffectUtils::fillShaderImageDescriptions(
                            "vs", shaderDescription.vs.images, vertexShaderFallbackImageNames);
                        PrivateEffectUtils::fillShaderImageDescriptions(
                            "ps", shaderDescription.fs.images, pixelShaderFallbackImageNames);
                        PrivateEffectUtils::parseAnnotations(
                            passPtr->annotations, passPtr->n_annotations, factory, passAnnotations);
                        char label[Inline::kMarkerStringLength];
                        StringUtils::format(label, sizeof(label), "Effects/%s/%s/%s", nameConstString(),
                            techniquePtr->name, passPtr->name);
                        shaderDescription.label = label;
                        sg_shader shader = sg::make_shader(&shaderDescription);
                        if (sg::query_shader_state(shader) == SG_RESOURCESTATE_VALID) {
                            setShaderLabel(shader, label);
                            pd.m_body.shader = shader;
                            effect::Pass *pass = nanoem_new(Pass(this, passPtr->name, pd, passRegisterIndices,
                                passAnnotations, vertexShaderRegisterUniformBufferOffsetMap,
                                pixelShaderRegisterUniformBufferOffsetMap, preshaders, shaderDescription));
                            passes.push_back(pass);
                        }
                        else {
                            m_logger->log("Creating the pass Effects/%s/%s/%s failed", nameConstString(),
                                techniquePtr->name, passPtr->name);
                            sg::destroy_shader(shader);
                        }
                    }
                    else {
                        m_logger->log("Compiling the pass Effects/%s/%s/%s failed: %s", nameConstString(),
                            techniquePtr->name, passPtr->name, error.reasonConstString());
                    }
                }
                if (!passes.empty()) {
                    effect::Technique *technique =
                        nanoem_new(Technique(this, techniquePtr->name, techniqueAnnotations, passes));
                    m_hasScriptExternal |= technique->hasScriptExternal();
                    m_allTechniques.push_back(technique);
                    m_techniqueByPassTypes[technique->passType()].push_back(technique);
                }
            }
            m_needsBehaviorCompatibility = needsBehaviorCompatibility;
        }
        if (!error.hasReason()) {
            nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
            const size_t numParameters = effect->n_parameters;
            for (size_t i = 0; i < numParameters; i++) {
                const Fx9__Effect__Parameter *parameterPtr = effect->parameters[i];
                TypedSemanticParameter parameter(parameterPtr->name, parameterPtr->semantic,
                    PrivateEffectUtils::determineParameterType(parameterPtr));
                parameter.m_value.resize(parameterPtr->value.len);
                parameter.m_shared = (parameterPtr->flags & TypedSemanticParameter::kShared) != 0;
                memcpy(parameter.m_value.data(), parameterPtr->value.data, parameterPtr->value.len);
                PrivateEffectUtils::parseAnnotations(
                    parameterPtr->annotations, parameterPtr->n_annotations, factory, parameter.m_annotations);
                m_parameters.insert(tinystl::make_pair(parameter.m_name, parameter));
            }
            StringSet includePathSet;
            if (effect->n_includes > 0) {
                for (size_t i = 0, numPaths = effect->n_includes; i < numPaths; i++) {
                    const Fx9__Effect__Include *include = effect->includes[i];
                    const char *location = include->location;
                    if (includePathSet.find(location) == includePathSet.end()) {
                        m_includePaths.push_back(location);
                        includePathSet.insert(location);
                    }
                }
            }
            else {
                for (size_t i = 0, numPaths = effect->n_compat_included_paths; i < numPaths; i++) {
                    const char *location = effect->compat_included_paths[i];
                    if (includePathSet.find(location) == includePathSet.end()) {
                        m_includePaths.push_back(location);
                        includePathSet.insert(location);
                    }
                }
            }
            succeeded = !error.isCancelled();
        }
        fx9__effect__effect__free_unpacked(effect, g_protobufc_allocator);
    }
    else {
        char message[Error::kMaxReasonLength];
        StringUtils::format(
            message, sizeof(message), "Cannot load this effect due to unacceptable data: size=%jd", size);
        error = Error(message, nullptr, Error::kDomainTypeApplication);
    }
    SG_POP_GROUP();
    return succeeded;
}

bool
Effect::load(const ByteArray &bytes, Progress &progress, Error &error)
{
    return load(bytes.data(), bytes.size(), progress, error);
}

bool
Effect::upload(effect::AttachmentType type, Progress &progress, Error &error)
{
    return upload(type, nullptr, progress, error);
}

bool
Effect::upload(effect::AttachmentType type, const Archiver *archiver, Progress &progress, Error &error)
{
    SG_PUSH_GROUPF("Effect::upload(name=%s)", nameConstString());
    m_errorMessages.clear();
    for (ParameterMap::const_iterator it = m_parameters.begin(), end = m_parameters.end(); it != end; ++it) {
        const String &name = it->first;
        const TypedSemanticParameter &parameter = it->second;
        SemanticParameterHandlerMap::const_iterator it2 = m_semanticParameterHandlers.end();
        size_t semanticLength = parameter.m_semantic.size();
        if (semanticLength > 0) {
            it2 = m_semanticParameterHandlers.find(StringUtils::toUpperCase(parameter.m_semantic));
        }
        if (it2 != m_semanticParameterHandlers.end()) {
            it2->second(this, parameter, progress);
        }
        else {
            switch (parameter.m_type) {
            case kParameterTypeBool: {
                GlobalUniform::Vector4List value;
                const size_t valueSize = parameter.m_value.size();
                if (valueSize > 0 && valueSize % sizeof(int) == 0) {
                    const int *ptr = reinterpret_cast<const int *>(parameter.m_value.data());
                    value.resize(valueSize / sizeof(int));
                    const size_t numValues = value.size();
                    for (size_t i = 0; i < numValues; i++) {
                        value[i] = Vector4(nanoem_f32_t(ptr[i]));
                    }
                }
                else {
                    value.push_back(Constants::kZeroV4);
                }
                m_boolParameterUniforms.insert(
                    tinystl::make_pair(name, NonSemanticParameter(name, value, parameter.m_annotations)));
                break;
            }
            case kParameterTypeInt: {
                GlobalUniform::Vector4List value;
                const size_t valueSize = parameter.m_value.size();
                if (valueSize > 0 && valueSize % sizeof(int) == 0) {
                    const int *ptr = reinterpret_cast<const int *>(parameter.m_value.data());
                    value.resize(valueSize / sizeof(int));
                    const size_t numValues = value.size();
                    for (size_t i = 0; i < numValues; i++) {
                        value[i] = Vector4(nanoem_f32_t(ptr[i]));
                    }
                }
                else {
                    value.push_back(Constants::kZeroV4);
                }
                m_intParameterUniforms.insert(
                    tinystl::make_pair(name, NonSemanticParameter(name, value, parameter.m_annotations)));
                break;
            }
            case kParameterTypeFloat: {
                GlobalUniform::Vector4List value;
                const size_t valueSize = parameter.m_value.size();
                if (valueSize > 0 && valueSize % sizeof(nanoem_f32_t) == 0) {
                    const nanoem_f32_t *ptr = reinterpret_cast<const nanoem_f32_t *>(parameter.m_value.data());
                    value.resize(valueSize / sizeof(nanoem_f32_t));
                    const size_t numValues = value.size();
                    for (size_t i = 0; i < numValues; i++) {
                        value[i] = Vector4(ptr[i]);
                    }
                }
                else {
                    value.push_back(Constants::kZeroV4);
                }
                m_floatParameterUniforms.insert(
                    tinystl::make_pair(name, NonSemanticParameter(name, value, parameter.m_annotations)));
                break;
            }
            case kParameterTypeFloat4: {
                GlobalUniform::Vector4List value;
                const size_t valueSize = parameter.m_value.size();
                if (valueSize > 0) {
                    if (valueSize % sizeof(value[0]) == 0) {
                        value.resize(valueSize / sizeof(value[0]));
                        memcpy(value.data(), parameter.m_value.data(), valueSize);
                    }
                    else if (valueSize % sizeof(Vector3) == 0) {
                        value.reserve(valueSize / sizeof(Vector3));
                        const size_t numElements = valueSize / sizeof(Vector3);
                        for (size_t i = 0; i < numElements; i++) {
                            const Vector3 *element = reinterpret_cast<const Vector3 *>(parameter.m_value.data());
                            value.push_back(Vector4(element[i], 0.0f));
                        }
                    }
                    else if (valueSize % sizeof(Vector2) == 0) {
                        value.reserve(valueSize / sizeof(Vector2));
                        const size_t numElements = valueSize / sizeof(Vector2);
                        for (size_t i = 0; i < numElements; i++) {
                            const Vector2 *element = reinterpret_cast<const Vector2 *>(parameter.m_value.data());
                            value.push_back(Vector4(element[i], 0.0f, 0.0f));
                        }
                    }
                    else {
                        value.push_back(Constants::kZeroV4);
                    }
                    m_vectorParameterUniforms.insert(
                        tinystl::make_pair(name, NonSemanticParameter(name, value, parameter.m_annotations)));
                }
                break;
            }
            case kParameterTypeTexture:
            case kParameterTypeTexture1D:
            case kParameterTypeTexture2D:
            case kParameterTypeTexture3D:
            case kParameterTypeTextureCube: {
                createImageResourceFromArchive(parameter, archiver, progress, error);
                break;
            }
            default:
                break;
            }
        }
    }
    if (type == kAttachmentTypeNone && scriptOrder() != IEffect::kScriptOrderTypePostProcess) {
        for (TechniqueList::const_iterator it = m_allTechniques.begin(), end = m_allTechniques.end(); it != end; ++it) {
            Technique *technique = *it;
            technique->ensureScriptCommandClear();
        }
    }
    bool cancelled = error.isCancelled(), succeeded = m_errorMessages.empty() && !cancelled;
    if (succeeded) {
        m_enabled.first = m_enabled.second = true;
    }
    else if (!cancelled) {
        String message;
        for (StringList::const_iterator it = m_errorMessages.begin(), end = m_errorMessages.end(); it != end; ++it) {
            message.append(it->c_str());
            message.append("\n");
        }
        error = Error(message.c_str(), nullptr, Error::kDomainTypeApplication);
    }
    SG_POP_GROUP();
    return succeeded;
}

void
Effect::resizeAllRenderTargetImages(
    const Vector2UI16 &size, StringSet &sharedRenderColorImageNames, StringSet &sharedOffscreenImageNames)
{
    SG_PUSH_GROUPF("Effect::resizeAllRenderTargetImages(name=%s)", nameConstString());
    bool enableAA = RenderTargetColorImageContainer::kAntialiasEnabled;
    const int sampleCount = enableAA ? m_project->sampleCount() : 1;
    for (DrawableNamedRenderTargetColorImageContainerMap::iterator it = m_drawableNamedRenderTargetColorImages.begin(),
                                                                   end = m_drawableNamedRenderTargetColorImages.end();
         it != end; ++it) {
        NamedRenderTargetColorImageContainerMap &containers = it->second;
        for (NamedRenderTargetColorImageContainerMap::iterator it2 = containers.begin(), end2 = containers.end();
             it2 != end2; ++it2) {
            RenderTargetColorImageContainer *container = it2->second;
            container->resizeWithScale(size);
            container->setSampleCount(sampleCount);
            container->invalidate(this);
            if (container->isSharedTexture()) {
                sharedRenderColorImageNames.insert(container->nameConstString());
            }
        }
    }
    for (RenderTargetDepthStencilImageContainerMap::iterator it = m_renderTargetDepthStencilImages.begin(),
                                                             end = m_renderTargetDepthStencilImages.end();
         it != end; ++it) {
        RenderTargetDepthStencilImageContainer *container = it->second;
        container->resizeWithScale(size);
        container->setSampleCount(sampleCount);
        container->invalidate(this);
    }
    for (OffscreenRenderTargetImageContainerMap::iterator it = m_offscreenRenderTargetImages.begin(),
                                                          end = m_offscreenRenderTargetImages.end();
         it != end; ++it) {
        OffscreenRenderTargetImageContainer *container = it->second;
        container->resizeWithScale(this, size);
        if (container->isSharedTexture()) {
            sharedOffscreenImageNames.insert(container->nameConstString());
        }
    }
    for (StagingBufferMap::const_iterator it = m_imageStagingBuffers.begin(), end = m_imageStagingBuffers.end();
         it != end; ++it) {
        const StagingBuffer &sb = it->second;
        if (!sb.m_immutable) {
            sg::destroy_buffer(sb.m_handle);
            m_imageStagingBuffers.erase(it);
        }
    }
    for (RenderTargetNormalizerMap::iterator it = m_normalizers.begin(), end = m_normalizers.end(); it != end; ++it) {
        RenderTargetNormalizer *pass = it->second;
        pass->destroy();
        nanoem_delete(it->second);
    }
    m_normalizers.clear();
    m_hashes.clear();
    PassList passes;
    for (TechniqueList::iterator it = m_allTechniques.begin(), end = m_allTechniques.end(); it != end; ++it) {
        Technique *technique = *it;
        technique->getAllPasses(passes);
        for (PassList::iterator it2 = passes.begin(), end2 = passes.end(); it2 != end2; ++it2) {
            Pass *pass = *it2;
            pass->resetVertexBuffer();
        }
    }
    resetPassDescription();
    SG_POP_GROUP();
}

void
Effect::resetAllSharedRenderTargetColorImages(const StringSet &names)
{
    sg_image invalid = { SG_INVALID_ID };
    NamedRenderTargetColorImageContainerMap &containers = m_drawableNamedRenderTargetColorImages[nullptr];
    for (StringSet::const_iterator it = names.begin(), end = names.end(); it != end; ++it) {
        const String &name = *it;
        NamedRenderTargetColorImageContainerMap::iterator it2 = containers.find(name);
        if (it2 != containers.end()) {
            RenderTargetColorImageContainer *container = it2->second,
                                            *sharedContainer =
                                                m_project->findSharedRenderTargetImageContainer(name, nullptr);
            if (sharedContainer) {
                container->inherit(sharedContainer);
            }
            else {
                container->setColorImageHandle(invalid);
            }
        }
    }
}

void
Effect::resetAllSharedOffscreenRenderTargets(const StringSet &names)
{
    sg_image invalid = { SG_INVALID_ID };
    for (StringSet::const_iterator it = names.begin(), end = names.end(); it != end; ++it) {
        const String &name = *it;
        OffscreenRenderTargetImageContainerMap::iterator it2 = m_offscreenRenderTargetImages.find(name);
        if (it2 != m_offscreenRenderTargetImages.end()) {
            RenderTargetColorImageContainer *container = it2->second,
                                            *sharedContainer =
                                                m_project->findSharedRenderTargetImageContainer(name, nullptr);
            container->setColorImageHandle(sharedContainer ? sharedContainer->colorImageHandle() : invalid);
        }
    }
}

void
Effect::markAllAnimatedImagesUpdatable()
{
    for (AnimatedImageContainerMap::iterator it = m_animatedTextureImages.begin(), end = m_animatedTextureImages.end();
         it != end; ++it) {
        AnimatedImageContainer *container = it->second;
        container->markDirty();
    }
    for (StagingBufferMap::iterator it = m_imageStagingBuffers.begin(), end = m_imageStagingBuffers.end(); it != end;
         ++it) {
        if (!it->second.m_immutable) {
            it->second.m_read = false;
        }
    }
}

void
Effect::createAllDrawableRenderTargetColorImages(const IDrawable *drawable)
{
    const NamedRenderTargetColorImageContainerMap &sourceContainers = m_drawableNamedRenderTargetColorImages[nullptr];
    NamedRenderTargetColorImageContainerMap &destContainers = m_drawableNamedRenderTargetColorImages[drawable];
    for (NamedRenderTargetColorImageContainerMap::const_iterator it = sourceContainers.begin(),
                                                                 end = sourceContainers.end();
         it != end; ++it) {
        const String &name = it->first;
        const RenderTargetColorImageContainer *sourceContainer = it->second;
        NamedRenderTargetColorImageContainerMap::const_iterator it2 = destContainers.find(name);
        RenderTargetColorImageContainer *destContainer = nullptr;
        if (it2 != destContainers.end()) {
            if (sourceContainer->isSharedTexture()) {
                it2->second->inherit(sourceContainer);
            }
        }
        else {
            String label;
            StringUtils::format(label, "%s/%s", name.c_str(), drawable->nameConstString());
            destContainer = nanoem_new(RenderTargetColorImageContainer(label));
            if (sourceContainer->isSharedTexture()) {
                destContainer->share(sourceContainer);
            }
            else {
                destContainer->setColorImageDescription(sourceContainer->colorImageDescription());
                destContainer->setScaleFactor(sourceContainer->scaleFactor());
                destContainer->create(this);
            }
        }
        if (destContainer) {
            destContainers.insert(tinystl::make_pair(name, destContainer));
        }
    }
}

void
Effect::destroyAllDrawableRenderTargetColorImages(const IDrawable *drawable)
{
    DrawableNamedRenderTargetColorImageContainerMap::iterator it =
        m_drawableNamedRenderTargetColorImages.find(drawable);
    if (it != m_drawableNamedRenderTargetColorImages.end()) {
        destroyAllRenderTargetColorImages(it->second);
    }
}

void
Effect::generateRenderTargetMipmapImagesChain()
{
    if (const NamedRenderTargetColorImageContainerMap *containers =
            findNamedRenderTargetColorImageContainerMap(nullptr)) {
        NamedRenderTargetColorImageContainerMap::const_iterator it =
            containers->find(m_currentNamedPrimaryRenderTargetColorImageDescription.first);
        if (it != containers->end()) {
            generateRenderTargetMipmapImagesChain(nullptr, m_currentNamedPrimaryRenderTargetColorImageDescription.first,
                m_currentNamedDepthStencilImageDescription.first, it->second->colorImageDescription());
        }
    }
}

void
Effect::generateOffscreenMipmapImagesChain(const effect::OffscreenRenderTargetOption &option)
{
    const sg_image_desc &colorImageDesc = option.m_colorImageDescription;
    if (colorImageDesc.num_mipmaps > 1) {
        const String &name = option.m_name;
        OffscreenRenderTargetImageContainerMap::const_iterator it = m_offscreenRenderTargetImages.find(name);
        if (it != m_offscreenRenderTargetImages.end()) {
            SG_PUSH_GROUPF("Effect::generateOffscreenMipmapImagesChain(numImages=%d)", colorImageDesc.num_mipmaps);
            OffscreenRenderTargetImageContainer *offscreenContainer = it->second;
            PixelFormat format;
            format.setColorPixelFormat(colorImageDesc.pixel_format, 0);
            format.setDepthPixelFormat(offscreenContainer->depthStencilImageDescription().pixel_format);
            format.setNumSamples(colorImageDesc.sample_count);
            RenderTargetMipmapGenerator *generator = offscreenContainer->mipmapGenerator();
            if (!generator) {
                generator = nanoem_new(RenderTargetMipmapGenerator(this, name.c_str(), colorImageDesc));
                offscreenContainer->setMipmapGenerator(generator);
            }
            generator->blitSourceImage(this, offscreenContainer->colorImageHandle(), format, name.c_str());
            generator->generateAllMipmapImages(this, format, offscreenContainer, name.c_str());
            SG_POP_GROUP();
        }
    }
}

void
Effect::updateCurrentRenderTargetPixelFormatSampleCount()
{
    m_currentRenderTargetPixelFormat.setNumSamples(m_project->sampleCount());
}

void
Effect::destroy()
{
    SG_PUSH_GROUPF("Effect::destroy(name=%s)", nameConstString());
    if (UserDataDestructor destructor = m_userData.second) {
        destructor(m_userData.first, this);
    }
    destroyAllRenderTargetNormalizers();
    destroyAllOverridenImages();
    destroyAllTechniques();
    destroyAllSemanticImages(m_resourceImages);
    destroyAllRenderTargetDepthStencilImages(m_renderTargetDepthStencilImages);
    destroyAllDrawableNamedRenderTargetColorImages();
    destroyAllOffscreenRenderTargetImages(m_offscreenRenderTargetImages);
    destroyAllAnimatedImages(m_animatedTextureImages);
    destroyAllStagingBuffers(m_imageStagingBuffers);
    m_project->removeAllSharedRenderTargetImageContainers(this);
    m_project->releaseAllOffscreenRenderTarget(this);
    if (!m_namedImageHandles.empty()) {
        SG_PUSH_GROUPF("Effect::destroy(leakedImages=%d)", m_namedImageHandles.size());
        for (NamedHandleMap::const_iterator it = m_namedImageHandles.begin(), end = m_namedImageHandles.end();
             it != end; ++it) {
            SG_INSERT_MARKERF("Effect::destroy(leakedImage=%d, leakedName=%s)", it->first, it->second.c_str());
        }
        SG_POP_GROUP();
        m_namedImageHandles.clear();
    }
    if (!m_namedShaderHandles.empty()) {
        SG_PUSH_GROUPF("Effect::destroy(leakedShaders=%d)", m_namedShaderHandles.size());
        for (NamedHandleMap::const_iterator it = m_namedShaderHandles.begin(), end = m_namedShaderHandles.end();
             it != end; ++it) {
            SG_INSERT_MARKERF("Effect::destroy(leakedShader=%d, leakedName=%s)", it->first, it->second.c_str());
        }
        SG_POP_GROUP();
        m_namedShaderHandles.clear();
    }
    SG_POP_GROUP();
}

const RenderTargetColorImageContainer *
Effect::findRenderTargetImageContainer(const IDrawable *drawable, sg_image value) const NANOEM_DECL_NOEXCEPT
{
    const RenderTargetColorImageContainer *containerPtr = nullptr;
    const NamedRenderTargetColorImageContainerMap *containers = findNamedRenderTargetColorImageContainerMap(drawable);
    for (NamedRenderTargetColorImageContainerMap::const_iterator it = containers->begin(), end = containers->end();
         it != end; ++it) {
        const RenderTargetColorImageContainer *container = it->second;
        if (container->colorImageHandle().id == value.id) {
            containerPtr = container;
            break;
        }
    }
    return containerPtr;
}

const RenderTargetColorImageContainer *
Effect::searchRenderTargetColorImageContainer(const IDrawable *drawable, sg_image value) const NANOEM_DECL_NOEXCEPT
{
    const RenderTargetColorImageContainer *containerPtr = nullptr;
    if (sg::is_valid(value)) {
        if (const RenderTargetColorImageContainer *sharedContainer =
                m_project->findSharedRenderTargetImageContainer(value, nullptr)) {
            containerPtr = sharedContainer;
        }
        else if (const RenderTargetColorImageContainer *container = findRenderTargetImageContainer(drawable, value)) {
            containerPtr = container;
        }
    }
    return containerPtr;
}

const ImageSamplerList *
Effect::findImageSamplerList(const Pass *passPtr) const NANOEM_DECL_NOEXCEPT
{
    ImageSamplerMap::const_iterator it = m_imageSamplers.find(passPtr);
    return it != m_imageSamplers.end() ? &it->second : nullptr;
}

void
Effect::getAllOffscreenRenderTargetOptions(OffscreenRenderTargetOptionList &value) const
{
    value.clear();
    for (OffscreenRenderTargetOptionMap::const_iterator it = m_offscreenRenderTargetOptions.begin(),
                                                        end = m_offscreenRenderTargetOptions.end();
         it != end; ++it) {
        OffscreenRenderTargetOption option = it->second;
        const String &name = option.m_name;
        OffscreenRenderTargetImageContainerMap::const_iterator it2 = m_offscreenRenderTargetImages.find(name);
        const bool found = it2 != m_offscreenRenderTargetImages.end();
        if (found) {
            const OffscreenRenderTargetImageContainer *container = it2->second;
            option.m_colorImage = container->colorImageHandle();
            option.m_depthStencilImage = container->depthStencilImageHandle();
            option.m_colorImageDescription = container->colorImageDescription();
            option.m_depthStencilImageDescription = container->depthStencilImageDescription();
            option.m_sharedImageReferenceCount = m_project->countSharedRenderTargetImageContainer(name, this);
        }
        value.push_back(option);
    }
}

void
Effect::getAllRenderTargetImageContainers(NamedRenderTargetColorImageContainerMap &value) const
{
    DrawableNamedRenderTargetColorImageContainerMap::const_iterator it =
        m_drawableNamedRenderTargetColorImages.find(nullptr);
    if (it != m_drawableNamedRenderTargetColorImages.end()) {
        value = it->second;
    }
}

void
Effect::getAllUIWidgetParameters(UIWidgetParameterList &value)
{
    value.clear();
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    for (IntParameterUniformMap::iterator it = m_intParameterUniforms.begin(), end = m_intParameterUniforms.end();
         it != end; ++it) {
        NonSemanticParameter &parameter = it->second;
        const AnnotationMap &annotations = parameter.m_annotations;
        if (annotations.findAnnotation("UIWidget") != annotations.end() ||
            annotations.findAnnotation("UIVisible") != annotations.end()) {
            value.push_back(UIWidgetParameter(kParameterTypeInt, annotations, &parameter.m_values, factory));
        }
    }
    for (FloatParameterUniformMap::iterator it = m_floatParameterUniforms.begin(), end = m_floatParameterUniforms.end();
         it != end; ++it) {
        NonSemanticParameter &parameter = it->second;
        const AnnotationMap &annotations = parameter.m_annotations;
        if (annotations.findAnnotation("UIWidget") != annotations.end() ||
            annotations.findAnnotation("UIVisible") != annotations.end()) {
            value.push_back(UIWidgetParameter(kParameterTypeFloat, annotations, &parameter.m_values, factory));
        }
    }
    for (VectorParameterUniformMap::iterator it = m_vectorParameterUniforms.begin(),
                                             end = m_vectorParameterUniforms.end();
         it != end; ++it) {
        NonSemanticParameter &parameter = it->second;
        const AnnotationMap &annotations = it->second.m_annotations;
        if (annotations.findAnnotation("UIWidget") != annotations.end() ||
            annotations.findAnnotation("UIVisible") != annotations.end()) {
            value.push_back(UIWidgetParameter(kParameterTypeFloat4, annotations, &parameter.m_values, factory));
        }
    }
}

void
Effect::getPassUniformBuffer(PassUniformBufferMap &value) const
{
    value = m_passUniformBuffer;
}

void
Effect::getPrimaryRenderTargetColorImageSubPixelOffset(Vector4 &value) const
{
    const sg_image_desc &colorImageDesc = m_currentNamedPrimaryRenderTargetColorImageDescription.second;
    const int width = colorImageDesc.width;
    const int height = colorImageDesc.height;
    value = Vector4(Vector2(-0.5f) / Vector2(width, height), 0, 0);
}

void
Effect::attachAllResources(FileEntityMap &allAttachments) const
{
    allAttachments.insert(tinystl::make_pair(filename(), fileURI()));
    attachEffectIncludePathSet(allAttachments);
    attachEffectImageResource(allAttachments);
    Project::LoadedEffectSet renderTargetEffectSet;
    m_project->getAllOffscreenRenderTargetEffects(this, renderTargetEffectSet);
    for (Project::LoadedEffectSet::const_iterator it = renderTargetEffectSet.begin(), end = renderTargetEffectSet.end();
         it != end; ++it) {
        Effect *renderTargetEffect = *it;
        allAttachments.insert(tinystl::make_pair(renderTargetEffect->filename(), renderTargetEffect->fileURI()));
        renderTargetEffect->attachEffectIncludePathSet(allAttachments);
        renderTargetEffect->attachEffectImageResource(allAttachments);
    }
}

void
Effect::setImageLabel(sg_image texture, const String &name)
{
    setImageLabel(texture, name.c_str());
}

void
Effect::setImageLabel(sg_image texture, const char *name)
{
    NamedHandleMap::const_iterator it = m_namedImageHandles.find(texture.id);
    if (it == m_namedImageHandles.end()) {
        char nameBuffer[Inline::kMarkerStringLength];
        StringUtils::format(nameBuffer, sizeof(nameBuffer), "Effects/%s/Images/%s", nameConstString(), name);
        SG_LABEL_IMAGE(texture, nameBuffer);
        SG_INSERT_MARKERF("Effect::setImageLabel(handle=%d, name=%s)", texture.id, nameBuffer);
        m_namedImageHandles.insert(tinystl::make_pair(texture.id, String(nameBuffer)));
    }
}

void
Effect::setShaderLabel(sg_shader shader, const char *name)
{
    NamedHandleMap::const_iterator it = m_namedShaderHandles.find(shader.id);
    if (it == m_namedShaderHandles.end()) {
        char nameBuffer[Inline::kMarkerStringLength];
        StringUtils::format(nameBuffer, sizeof(nameBuffer), "Effects/%s/%s", nameConstString(), name);
        SG_LABEL_SHADER(shader, nameBuffer);
        SG_INSERT_MARKERF("Effect::setShaderLabel(handle=%d, name=%s)", shader.id, nameBuffer);
        m_namedShaderHandles.insert(tinystl::make_pair(shader.id, String(nameBuffer)));
    }
}

void
Effect::removeImageLabel(sg_image texture)
{
    NamedHandleMap::const_iterator it = m_namedImageHandles.find(texture.id);
    if (it != m_namedImageHandles.end()) {
        SG_INSERT_MARKERF("Effect::removeImageLabel(handle=%d, name=%s)", it->first, it->second.c_str());
        m_namedImageHandles.erase(it);
    }
}

void
Effect::removeShaderLabel(sg_shader shader)
{
    NamedHandleMap::const_iterator it = m_namedShaderHandles.find(shader.id);
    if (it != m_namedShaderHandles.end()) {
        SG_INSERT_MARKERF("Effect::removeShaderLabel(handle=%d, name=%s)", it->first, it->second.c_str());
        m_namedShaderHandles.erase(it);
    }
}

const Project *
Effect::project() const NANOEM_DECL_NOEXCEPT
{
    return m_project;
}

Project *
Effect::project() NANOEM_DECL_NOEXCEPT
{
    return m_project;
}

const effect::Logger *
Effect::logger() const NANOEM_DECL_NOEXCEPT
{
    return m_logger;
}

effect::Logger *
Effect::logger() NANOEM_DECL_NOEXCEPT
{
    return m_logger;
}

StringList
Effect::allIncludePaths() const
{
    return m_includePaths;
}

String
Effect::name() const
{
    return m_name;
}

const char *
Effect::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_name.c_str();
}

void
Effect::setName(const String &value)
{
    m_name = value;
}

String
Effect::filename() const
{
    if (Project::isArchiveURI(m_fileURI)) {
        return URI::lastPathComponent(m_fileURI.fragment());
    }
    else if (!m_filename.empty()) {
        return m_filename;
    }
    else {
        return m_fileURI.lastPathComponent();
    }
}

void
Effect::setFilename(String value)
{
    m_filename = value;
}

URI
Effect::fileURI() const
{
    return m_fileURI;
}

URI
Effect::resolvedFileURI() const
{
    return m_project->resolveFileURI(m_fileURI);
}

void
Effect::setFileURI(const URI &value)
{
    m_fileURI = value;
}

Effect::UserData
Effect::userData() const
{
    return m_userData;
}

void
Effect::setUserData(const UserData &value)
{
    m_userData = value;
}

Vector4
Effect::clearColor() const NANOEM_DECL_NOEXCEPT
{
    return m_clearColor;
}

nanoem_f32_t
Effect::clearDepth() const NANOEM_DECL_NOEXCEPT
{
    return m_clearDepth;
}

bool
Effect::canEnable() const NANOEM_DECL_NOEXCEPT
{
    return m_enabled.second;
}

bool
Effect::isEnabled() const NANOEM_DECL_NOEXCEPT
{
    return m_enabled.first && m_enabled.second;
}

void
Effect::setEnabled(bool value)
{
    if (m_enabled.second) {
        m_enabled.first = value;
    }
}

bool
Effect::isPassUniformBufferInspectionEnabled() const NANOEM_DECL_NOEXCEPT
{
    return m_enablePassUniformBufferInspection;
}

void
Effect::setPassUniformBufferInspectionEnabled(bool value)
{
    m_passUniformBuffer.clear();
    m_enablePassUniformBufferInspection = value;
}

GlobalUniform *
Effect::globalUniform()
{
    return m_globalUniformPtr;
}

effect::ViewPassSet
Effect::viewPassSet() const
{
    return m_viewPassSet;
}

void
Effect::setGlobalParameters(const IDrawable *drawable, const Project *project, effect::Pass *pass)
{
    nanoem_parameter_assert(project, "must not be nullptr");
    nanoem_parameter_assert(pass, "must not be nullptr");
    for (BoolParameterUniformMap::const_iterator it = m_boolParameterUniforms.begin(),
                                                 end = m_boolParameterUniforms.end();
         it != end; ++it) {
        const NonSemanticParameter &parameter = it->second;
        writeUniformBuffer(parameter.m_name, pass, parameter);
    }
    for (IntParameterUniformMap::const_iterator it = m_intParameterUniforms.begin(), end = m_intParameterUniforms.end();
         it != end; ++it) {
        const NonSemanticParameter &parameter = it->second;
        writeUniformBuffer(parameter.m_name, pass, parameter);
    }
    for (FloatParameterUniformMap::const_iterator it = m_floatParameterUniforms.begin(),
                                                  end = m_floatParameterUniforms.end();
         it != end; ++it) {
        const NonSemanticParameter &parameter = it->second;
        writeUniformBuffer(parameter.m_name, pass, parameter);
    }
    for (VectorParameterUniformMap::const_iterator it = m_vectorParameterUniforms.begin(),
                                                   end = m_vectorParameterUniforms.end();
         it != end; ++it) {
        const NonSemanticParameter &parameter = it->second;
        writeUniformBuffer(parameter.m_name, pass, parameter);
    }
    if (!m_viewportPixelUniforms.empty()) {
        const Vector4 viewportParameterValue(project->deviceScaleUniformedViewportImageSize(), 0, 0);
        for (SemanticUniformList::const_iterator it = m_viewportPixelUniforms.begin(),
                                                 end = m_viewportPixelUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, viewportParameterValue);
        }
    }
    if (!m_timeUniforms.empty()) {
        const Vector4 time(project->currentLocalFrameIndex() / nanoem_f32_t(project->baseFPS()));
        for (SemanticUniformList::const_iterator it = m_timeUniforms.begin(), end = m_timeUniforms.end(); it != end;
             ++it) {
            writeUniformBuffer(*it, pass, time);
        }
    }
    if (!m_elapsedTimeUniforms.empty()) {
        const Vector4 elapsedTime(project->elapsedLocalFrameIndex() / nanoem_f32_t(project->baseFPS()));
        for (SemanticUniformList::const_iterator it = m_elapsedTimeUniforms.begin(), end = m_elapsedTimeUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, elapsedTime);
        }
    }
    if (!m_systemTimeUniforms.empty()) {
        const Vector4 systemTime(glm::dvec4(project->currentUptimeSeconds()));
        for (SemanticUniformList::const_iterator it = m_systemTimeUniforms.begin(), end = m_systemTimeUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, systemTime);
        }
    }
    if (!m_elapsedSystemTimeUniforms.empty()) {
        const Vector4 elapsedSystemTime(glm::dvec4(project->elapsedUptimeSeconds()));
        for (SemanticUniformList::const_iterator it = m_elapsedSystemTimeUniforms.begin(),
                                                 end = m_elapsedSystemTimeUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, elapsedSystemTime);
        }
    }
    if (!m_mousePositionUniforms.empty()) {
        const Vector4 mousePosition(project->logicalScaleMovingCursorPosition(), 0.0, 0.0f);
        for (SemanticUniformList::const_iterator it = m_mousePositionUniforms.begin(),
                                                 end = m_mousePositionUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, mousePosition);
        }
    }
    if (!m_leftMouseDownUniforms.empty()) {
        const Vector4 leftMouseDown(project->logicalScaleLastCursorPosition(Project::kCursorTypeMouseLeft));
        for (SemanticUniformList::const_iterator it = m_leftMouseDownUniforms.begin(),
                                                 end = m_leftMouseDownUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, leftMouseDown);
        }
    }
    if (!m_middleMouseDownUniforms.empty()) {
        const Vector4 middleMouseDown(project->logicalScaleLastCursorPosition(Project::kCursorTypeMouseMiddle));
        for (SemanticUniformList::const_iterator it = m_middleMouseDownUniforms.begin(),
                                                 end = m_middleMouseDownUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, middleMouseDown);
        }
    }
    if (!m_rightMouseDownUniforms.empty()) {
        const Vector4 rightMouseDown(project->logicalScaleLastCursorPosition(Project::kCursorTypeMouseRight));
        for (SemanticUniformList::const_iterator it = m_rightMouseDownUniforms.begin(),
                                                 end = m_rightMouseDownUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, rightMouseDown);
        }
    }
    const NamedRenderTargetColorImageContainerMap *containers = findNamedRenderTargetColorImageContainerMap(drawable);
    setFoundImageSamplers(m_textureResourceUniforms, m_resourceImages, pass);
    setFoundImageSamplers(m_renderTargetColorUniforms, *containers, pass);
    setFoundImageSamplers(m_offscreenRenderTargetUniforms, m_offscreenRenderTargetImages, pass);
    setTextureValues(m_resourceImages, pass);
    setTextureValues(*containers, pass);
    setTextureValues(m_offscreenRenderTargetImages, pass);
    m_initializeGlobal = true;
}

void
Effect::setCameraParameters(const ICamera *camera, const Matrix4x4 &world, const effect::Pass *pass)
{
    nanoem_parameter_assert(camera, "must not be nullptr");
    nanoem_parameter_assert(pass, "must not be nullptr");
    if (!m_cameraDirectionUniforms.empty()) {
        const Vector4 direction(camera->direction(), 0);
        for (SemanticUniformList::const_iterator it = m_cameraDirectionUniforms.begin(),
                                                 end = m_cameraDirectionUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, direction);
        }
    }
    if (!m_cameraPositionUniforms.empty()) {
        const Vector4 position(camera->position(), 1);
        for (SemanticUniformList::const_iterator it = m_cameraPositionUniforms.begin(),
                                                 end = m_cameraPositionUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, position);
        }
        writeUniformBuffer("Place", pass, position);
    }
    if (!m_cameraMatrixUniforms.empty()) {
        Matrix4x4 view, projection, result;
        camera->getViewTransform(view, projection);
        for (MatrixUniformMap::const_iterator it = m_cameraMatrixUniforms.begin(), end = m_cameraMatrixUniforms.end();
             it != end; ++it) {
            it->second.multiply(world, view, projection, result);
            writeUniformBuffer(it->first, pass, result);
        }
        writeUniformBuffer("matWorld", pass, world);
        writeUniformBuffer("matWorldViewProj", pass, projection * view * world);
    }
}

void
Effect::setLightParameters(const ILight *light, bool adjustment, effect::Pass *pass)
{
    nanoem_parameter_assert(light, "must not be nullptr");
    nanoem_parameter_assert(pass, "must not be nullptr");
    if (!m_lightDirectionUniforms.empty()) {
        const Vector4 direction(glm::normalize(light->direction()), 0);
        for (SemanticUniformList::const_iterator it = m_lightDirectionUniforms.begin(),
                                                 end = m_lightDirectionUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, direction);
        }
        writeUniformBuffer("LightDir", pass, direction);
    }
    if (!m_lightPositionUniforms.empty()) {
        const Vector4 position(-light->direction(), 0);
        for (SemanticUniformList::const_iterator it = m_lightPositionUniforms.begin(),
                                                 end = m_lightPositionUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, position);
        }
    }
    const Vector3 lightColor(light->color());
    if (adjustment) {
        if (!m_lightAmbientUniforms.empty()) {
            const Vector4 ambient(lightColor - Vector3(0.3f), 1);
            for (SemanticUniformList::const_iterator it = m_lightAmbientUniforms.begin(),
                                                     end = m_lightAmbientUniforms.end();
                 it != end; ++it) {
                writeUniformBuffer(*it, pass, ambient);
            }
        }
        if (!m_lightDiffuseUniforms.empty()) {
            const Vector4 diffuse(1);
            for (SemanticUniformList::const_iterator it = m_lightDiffuseUniforms.begin(),
                                                     end = m_lightDiffuseUniforms.end();
                 it != end; ++it) {
                writeUniformBuffer(*it, pass, diffuse);
            }
        }
        if (!m_lightSpecularUniforms.empty()) {
            const Vector4 specular(lightColor, 1);
            for (SemanticUniformList::const_iterator it = m_lightSpecularUniforms.begin(),
                                                     end = m_lightSpecularUniforms.end();
                 it != end; ++it) {
                writeUniformBuffer(*it, pass, specular);
            }
        }
    }
    else {
        if (!m_lightAmbientUniforms.empty()) {
            const Vector4 ambient(lightColor, 1);
            for (SemanticUniformList::const_iterator it = m_lightAmbientUniforms.begin(),
                                                     end = m_lightAmbientUniforms.end();
                 it != end; ++it) {
                writeUniformBuffer(*it, pass, ambient);
            }
        }
        if (!m_lightDiffuseUniforms.empty()) {
            const Vector4 diffuse(0);
            for (SemanticUniformList::const_iterator it = m_lightDiffuseUniforms.begin(),
                                                     end = m_lightDiffuseUniforms.end();
                 it != end; ++it) {
                writeUniformBuffer(*it, pass, diffuse);
            }
        }
        if (!m_lightSpecularUniforms.empty()) {
            const Vector4 specular(lightColor, 1);
            for (SemanticUniformList::const_iterator it = m_lightSpecularUniforms.begin(),
                                                     end = m_lightSpecularUniforms.end();
                 it != end; ++it) {
                writeUniformBuffer(*it, pass, specular);
            }
        }
    }
}

void
Effect::setAllAccessoryParameters(const Accessory *accessory, const Project *project, effect::Pass *pass)
{
    nanoem_parameter_assert(accessory, "must not be nullptr");
    nanoem_parameter_assert(pass, "must not be nullptr");
    for (SemanticUniformList::const_iterator it = m_controlObjectUniforms.begin(), end = m_controlObjectUniforms.end();
         it != end; ++it) {
        const String &parameterName = *it;
        ControlObjectTargetMap::iterator it2 = m_controlObjectTargets.find(*it);
        if (it2 != m_controlObjectTargets.end()) {
            ControlObjectTarget &target = it2->second;
            const String &targetName = target.m_name;
            const Accessory *foundAccessory = nullptr;
            const Model *foundModel = nullptr;
            if (StringUtils::equals(targetName.c_str(), "(self)")) {
                foundAccessory = accessory;
            }
            else if (StringUtils::equals(targetName.c_str(), "(OffscreenOwner)")) {
                const tinystl::pair<const Model *, const Accessory *> &pair =
                    findOffscreenOwnerObject(accessory, project);
                foundModel = pair.first;
                foundAccessory = pair.second;
            }
            else {
                foundAccessory = project->findAccessoryByFilename(targetName);
                foundModel = project->findModelByFilename(targetName);
            }
            if (foundAccessory) {
                setAccessoryParameter(parameterName, foundAccessory, target, pass);
            }
            else if (foundModel) {
                setModelParameter(parameterName, foundModel, target, pass);
            }
            else {
                setDefaultControlParameterValues(parameterName, target, pass);
            }
        }
    }
    nanodxm_rsize_t numVertices, numMaterials;
    nanodxmDocumentGetVertices(accessory->data(), &numVertices);
    writeUniformBuffer("VertexCount", pass, nanoem_f32_t(numVertices));
    nanodxmDocumentGetMaterials(accessory->data(), &numMaterials);
    writeUniformBuffer("SubsetCount", pass, nanoem_f32_t(numMaterials));
    writeUniformBuffer("transp", pass, accessory->isTranslucent());
    writeUniformBuffer("opadd", pass, accessory->isAddBlendEnabled());
}

void
Effect::setAccessoryParameter(
    const String &name, const Accessory *accessory, ControlObjectTarget &target, effect::Pass *pass)
{
    nanoem_parameter_assert(accessory, "must not be nullptr");
    nanoem_parameter_assert(pass, "must not be nullptr");
    const char *item = target.m_item.c_str();
    if (StringUtils::equals(item, "Rxyz")) {
        const Vector4 orientation(PrivateEffectUtils::angle(accessory), 0.0f);
        writeUniformBuffer(name, pass, orientation);
        target.m_value = orientation;
    }
    else if (StringUtils::equals(item, "Rx")) {
        const Vector4 orientationX(PrivateEffectUtils::angle(accessory).x);
        writeUniformBuffer(name, pass, orientationX);
        target.m_value = orientationX;
    }
    else if (StringUtils::equals(item, "Ry")) {
        const Vector4 orientationY(PrivateEffectUtils::angle(accessory).y);
        writeUniformBuffer(name, pass, orientationY);
        target.m_value = orientationY;
    }
    else if (StringUtils::equals(item, "Rz")) {
        const Vector4 orientationZ(PrivateEffectUtils::angle(accessory).z);
        writeUniformBuffer(name, pass, orientationZ);
        target.m_value = orientationZ;
    }
    else if (StringUtils::equals(item, "Si")) {
        const Vector4 scaleFactor(accessory->scaleFactor() * 10.0f);
        writeUniformBuffer(name, pass, scaleFactor);
        target.m_value = scaleFactor;
    }
    else if (StringUtils::equals(item, "Tr")) {
        const Vector4 opacityFactor(accessory->opacity());
        writeUniformBuffer(name, pass, opacityFactor);
        target.m_value = opacityFactor;
    }
    else if (StringUtils::equals(item, "XYZ")) {
        const Vector4 translation(accessory->fullWorldTransform()[3]);
        writeUniformBuffer(name, pass, translation);
        target.m_value = translation;
    }
    else if (StringUtils::equals(item, "X")) {
        const Vector4 translationX(accessory->fullWorldTransform()[3].x);
        writeUniformBuffer(name, pass, translationX);
        target.m_value = translationX;
    }
    else if (StringUtils::equals(item, "Y")) {
        const Vector4 translationY(accessory->fullWorldTransform()[3].y);
        writeUniformBuffer(name, pass, translationY);
        target.m_value = translationY;
    }
    else if (StringUtils::equals(item, "Z")) {
        const Vector4 translationZ(accessory->fullWorldTransform()[3].z);
        writeUniformBuffer(name, pass, translationZ);
        target.m_value = translationZ;
    }
    else if (target.m_item.empty()) {
        switch (target.m_type) {
        case kParameterTypeBool: {
            const Vector4 visible(accessory->isVisible());
            writeUniformBuffer(name, pass, visible);
            target.m_value = visible;
            break;
        }
        case kParameterTypeFloat: {
            const Vector4 scaleFactor(accessory->scaleFactor() * 10.0f);
            writeUniformBuffer(name, pass, scaleFactor);
            target.m_value = scaleFactor;
            break;
        }
        case kParameterTypeFloat4: {
            const Vector4 translation(accessory->fullWorldTransform()[3]);
            writeUniformBuffer(name, pass, translation);
            target.m_value = translation;
            break;
        }
        case kParameterTypeFloat4x4: {
            writeUniformBuffer(name, pass, accessory->fullWorldTransform());
            break;
        }
        default:
            break;
        }
    }
}

void
Effect::setAllModelParameters(const Model *model, const Project *project, Pass *pass)
{
    nanoem_parameter_assert(model, "must not be nullptr");
    nanoem_parameter_assert(pass, "must not be nullptr");
    for (SemanticUniformList::const_iterator it = m_controlObjectUniforms.begin(), end = m_controlObjectUniforms.end();
         it != end; ++it) {
        const String &parameterName = *it;
        ControlObjectTargetMap::iterator it2 = m_controlObjectTargets.find(*it);
        if (it2 != m_controlObjectTargets.end()) {
            ControlObjectTarget &target = it2->second;
            const String &targetName = target.m_name;
            const Model *foundModel = nullptr;
            const Accessory *foundAccessory = nullptr;
            if (StringUtils::equals(targetName.c_str(), "(self)")) {
                foundModel = model;
            }
            else if (StringUtils::equals(targetName.c_str(), "(OffscreenOwner)")) {
                const tinystl::pair<const Model *, const Accessory *> &pair = findOffscreenOwnerObject(model, project);
                foundModel = pair.first;
                foundAccessory = pair.second;
            }
            else {
                foundModel = project->findModelByFilename(targetName);
                foundAccessory = project->findAccessoryByFilename(targetName);
            }
            if (foundModel) {
                setModelParameter(parameterName, foundModel, target, pass);
            }
            else if (foundAccessory) {
                setAccessoryParameter(parameterName, foundAccessory, target, pass);
            }
            else {
                setDefaultControlParameterValues(parameterName, target, pass);
            }
        }
    }
    nanoem_rsize_t numVertices, numMaterials;
    nanoemModelGetAllVertexObjects(model->data(), &numVertices);
    writeUniformBuffer("VertexCount", pass, nanoem_f32_t(numVertices));
    nanoemModelGetAllMaterialObjects(model->data(), &numMaterials);
    writeUniformBuffer("SubsetCount", pass, nanoem_f32_t(numMaterials));
    writeUniformBuffer("transp", pass, model->isTranslucent());
    writeUniformBuffer("opadd", pass, model->isAddBlendEnabled());
}

void
Effect::setModelParameter(const String &name, const Model *model, ControlObjectTarget &target, Pass *pass)
{
    nanoem_parameter_assert(pass, "must not be nullptr");
    nanoem_parameter_assert(model, "must not be nullptr");
    const String &item = target.m_item;
    if (!item.empty()) {
        if (const model::Bone *bone = model::Bone::cast(model->findBone(item))) {
            ParameterType type = target.m_type;
            if (type == kParameterTypeFloat4) {
                const Vector4 position(bone->worldTransformOrigin(), 1);
                writeUniformBuffer(name, pass, position);
                target.m_value = position;
            }
            else if (type == kParameterTypeFloat4x4) {
                writeUniformBuffer(name, pass, bone->worldTransform());
                target.m_value = Vector4(bone->worldTransformOrigin(), 1);
            }
        }
        else if (const model::Morph *morph = model::Morph::cast(model->findMorph(item))) {
            if (target.m_type == kParameterTypeFloat) {
                const Vector4 weight(morph->weight());
                writeUniformBuffer(name, pass, morph->weight());
                target.m_value = weight;
            }
        }
    }
    else {
        switch (target.m_type) {
        case kParameterTypeBool: {
            const Vector4 visible(model->isVisible());
            writeUniformBuffer(name, pass, visible);
            target.m_value = visible;
            break;
        }
        case kParameterTypeFloat: {
            const Vector4 scaleFactor(1);
            writeUniformBuffer(name, pass, scaleFactor);
            target.m_value = scaleFactor;
            break;
        }
        case kParameterTypeFloat4: {
            nanoem_rsize_t numBones;
            if (nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones)) {
                const model::Bone *bone = model::Bone::cast(bones[0]);
                const Vector4 position(bone->worldTransformOrigin(), 1);
                writeUniformBuffer(name, pass, position);
                target.m_value = position;
            }
            break;
        }
        case kParameterTypeFloat4x4: {
            const Matrix4x4 worldTransform(model->worldTransform());
            writeUniformBuffer(name, pass, worldTransform);
            target.m_value = Vector4(worldTransform[3]);
            break;
        }
        default:
            break;
        }
    }
}

void
Effect::setMaterialParameters(const Accessory *accessory, const nanodxm_material_t *materialPtr, Pass *pass)
{
    nanoem_parameter_assert(accessory, "must not be nullptr");
    nanoem_parameter_assert(materialPtr, "must not be nullptr");
    nanoem_parameter_assert(pass, "must not be nullptr");
    const nanodxm_color_t &df = nanodxmMaterialGetDiffuse(materialPtr);
    const nanodxm_color_t &em = nanodxmMaterialGetEmissive(materialPtr);
    const nanodxm_color_t &sp = nanodxmMaterialGetSpecular(materialPtr);
    const Vector4 diffuse(df.r, df.g, df.b, df.a * accessory->opacity()),
        specular(Vector3(sp.r, sp.g, sp.b) * Vector3(0.1f), 1.0f),
        specularPower(nanodxmMaterialGetShininess(materialPtr));
    if (!m_materialAmbientUniforms.empty()) {
        const Vector4 ambient(df.r, df.g, df.b, 1.0f);
        for (SemanticUniformList::const_iterator it = m_materialAmbientUniforms.begin(),
                                                 end = m_materialAmbientUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, ambient);
        }
    }
    if (!m_materialDiffuseUniforms.empty()) {
        for (SemanticUniformList::const_iterator it = m_materialDiffuseUniforms.begin(),
                                                 end = m_materialDiffuseUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, diffuse);
        }
    }
    if (!m_materialEmissiveUniforms.empty()) {
        const Vector4 emissive(em.r, em.g, em.b, 1.0f);
        for (SemanticUniformList::const_iterator it = m_materialEmissiveUniforms.begin(),
                                                 end = m_materialEmissiveUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, emissive);
        }
    }
    if (!m_materialSpecularPowerUniforms.empty()) {
        for (SemanticUniformList::const_iterator it = m_materialSpecularPowerUniforms.begin(),
                                                 end = m_materialSpecularPowerUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, specularPower);
        }
    }
    if (!m_materialSpecularUniforms.empty()) {
        for (SemanticUniformList::const_iterator it = m_materialSpecularUniforms.begin(),
                                                 end = m_materialSpecularUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, specular);
        }
    }
    if (!m_addingDiffuseImageBlendFactorUniforms.empty()) {
        const Vector4 addingDiffuseTextureBlendFactor(0);
        for (SemanticUniformList::const_iterator it = m_addingDiffuseImageBlendFactorUniforms.begin(),
                                                 end = m_addingDiffuseImageBlendFactorUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, addingDiffuseTextureBlendFactor);
        }
    }
    if (!m_addingSphereImageBlendFactorUniforms.empty()) {
        const Vector4 addingSphereTextureBlendFactor(0);
        for (SemanticUniformList::const_iterator it = m_addingSphereImageBlendFactorUniforms.begin(),
                                                 end = m_addingSphereImageBlendFactorUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, addingSphereTextureBlendFactor);
        }
    }
    if (!m_multiplyingDiffuseImageBlendFactorUniforms.empty()) {
        const Vector4 multiplyingDiffuseTextureBlendFactor(1);
        for (SemanticUniformList::const_iterator it = m_multiplyingDiffuseImageBlendFactorUniforms.begin(),
                                                 end = m_multiplyingDiffuseImageBlendFactorUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, multiplyingDiffuseTextureBlendFactor);
        }
    }
    if (!m_multiplyingSphereImageBlendFactorUniforms.empty()) {
        const Vector4 multiplyingSphereTextureBlendFactor(1);
        for (SemanticUniformList::const_iterator it = m_multiplyingSphereImageBlendFactorUniforms.begin(),
                                                 end = m_multiplyingSphereImageBlendFactorUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, multiplyingSphereTextureBlendFactor);
        }
    }
    if (!m_materialToonColorUniforms.empty()) {
        const Vector4 toonColor(1);
        for (SemanticUniformList::const_iterator it = m_materialToonColorUniforms.begin(),
                                                 end = m_materialToonColorUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, toonColor);
        }
    }
    if (const Accessory::Material *material = accessory->findMaterial(materialPtr)) {
        const IImageView *diffuseImage = material->diffuseImage();
        const nanoem_model_material_sphere_map_texture_type_t sphereMapTextureType = material->sphereTextureMapType();
        if (sphereMapTextureType != NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE) {
            for (SemanticUniformList::const_iterator it = m_sphereImageUniforms.begin(),
                                                     end = m_sphereImageUniforms.end();
                 it != end; ++it) {
                setImageUniform(*it, pass, createOverrideImage(*it, diffuseImage, true));
            }
            writeUniformBuffer("use_texture", pass, 0);
            writeUniformBuffer("use_spheremap", pass, 1);
        }
        else {
            for (SemanticUniformList::const_iterator it = m_diffuseImageUniforms.begin(),
                                                     end = m_diffuseImageUniforms.end();
                 it != end; ++it) {
                setImageUniform(*it, pass, createOverrideImage(*it, diffuseImage, true));
            }
            writeUniformBuffer("use_texture", pass, 1);
            writeUniformBuffer("use_spheremap", pass, 0);
        }
        const bool isAddSphereMap = sphereMapTextureType == NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_ADD;
        writeUniformBuffer("spadd", pass, isAddSphereMap);
    }
    else {
        writeUniformBuffer("use_texture", pass, 0);
        writeUniformBuffer("use_spheremap", pass, 0);
        writeUniformBuffer("spadd", pass, 0);
    }
    const Vector3 lightColor(project()->globalLight()->color());
    writeUniformBuffer("EgColor", pass, diffuse * Vector4(lightColor - Vector3(0.3f), 1.0f));
    writeUniformBuffer("SpcColor", pass, Vector4(Vector3(specular) * lightColor, specularPower.x));
    writeUniformBuffer("use_toon", pass, 0);
    writeUniformBuffer("use_subtexture", pass, 0);
    writeUniformBuffer("opadd", pass, 0);
}

void
Effect::setMaterialParameters(const nanoem_model_material_t *materialPtr, const String &target, Pass *pass)
{
    nanoem_parameter_assert(materialPtr, "must not be nullptr");
    nanoem_parameter_assert(pass, "must not be nullptr");
    const model::Material *material = model::Material::cast(materialPtr);
    const model::Material::Color &baseColor = material->base();
    if (!m_materialAmbientUniforms.empty()) {
        const Vector4 ambient(baseColor.m_diffuse, 1);
        for (SemanticUniformList::const_iterator it = m_materialAmbientUniforms.begin(),
                                                 end = m_materialAmbientUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, ambient);
        }
    }
    if (!m_materialDiffuseUniforms.empty()) {
        const Vector4 diffuse(baseColor.m_diffuse, baseColor.m_diffuseOpacity);
        for (SemanticUniformList::const_iterator it = m_materialDiffuseUniforms.begin(),
                                                 end = m_materialDiffuseUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, diffuse);
        }
    }
    if (!m_materialEmissiveUniforms.empty()) {
        const Vector4 emissive(baseColor.m_ambient, 1);
        for (SemanticUniformList::const_iterator it = m_materialEmissiveUniforms.begin(),
                                                 end = m_materialEmissiveUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, emissive);
        }
    }
    if (!m_materialSpecularPowerUniforms.empty()) {
        const Vector4 specularPower(
            glm::abs(baseColor.m_specularPower) < Constants::kEpsilon ? 1.0f : baseColor.m_specularPower);
        for (SemanticUniformList::const_iterator it = m_materialSpecularPowerUniforms.begin(),
                                                 end = m_materialSpecularPowerUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, specularPower);
        }
    }
    if (!m_materialSpecularUniforms.empty()) {
        const Vector4 specular(baseColor.m_specular, 1);
        for (SemanticUniformList::const_iterator it = m_materialSpecularUniforms.begin(),
                                                 end = m_materialSpecularUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, specular);
        }
    }
    const model::Material::Color &addColor = material->add();
    if (!m_addingDiffuseImageBlendFactorUniforms.empty()) {
        const Vector4 addingDiffuseTextureBlendFactor(addColor.m_diffuseTextureBlendFactor);
        for (SemanticUniformList::const_iterator it = m_addingDiffuseImageBlendFactorUniforms.begin(),
                                                 end = m_addingDiffuseImageBlendFactorUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, addingDiffuseTextureBlendFactor);
        }
        writeUniformBuffer("TexCAdd", pass, addingDiffuseTextureBlendFactor);
    }
    if (!m_addingSphereImageBlendFactorUniforms.empty()) {
        const Vector4 addingSphereTextureBlendFactor(addColor.m_sphereTextureBlendFactor);
        for (SemanticUniformList::const_iterator it = m_addingSphereImageBlendFactorUniforms.begin(),
                                                 end = m_addingSphereImageBlendFactorUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, addingSphereTextureBlendFactor);
        }
        writeUniformBuffer("SphCAdd", pass, addingSphereTextureBlendFactor);
    }
    const model::Material::Color &mulColor = material->mul();
    if (!m_multiplyingDiffuseImageBlendFactorUniforms.empty()) {
        const Vector4 multiplyingDiffuseTextureBlendFactor(mulColor.m_diffuseTextureBlendFactor);
        for (SemanticUniformList::const_iterator it = m_multiplyingDiffuseImageBlendFactorUniforms.begin(),
                                                 end = m_multiplyingDiffuseImageBlendFactorUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, multiplyingDiffuseTextureBlendFactor);
        }
        writeUniformBuffer("TexCMul", pass, multiplyingDiffuseTextureBlendFactor);
    }
    if (!m_multiplyingSphereImageBlendFactorUniforms.empty()) {
        const Vector4 multiplyingSphereTextureBlendFactor(mulColor.m_sphereTextureBlendFactor);
        for (SemanticUniformList::const_iterator it = m_multiplyingSphereImageBlendFactorUniforms.begin(),
                                                 end = m_multiplyingSphereImageBlendFactorUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, multiplyingSphereTextureBlendFactor);
        }
        writeUniformBuffer("SphCMul", pass, multiplyingSphereTextureBlendFactor);
    }
    if (!m_materialToonColorUniforms.empty()) {
        const Vector4 toonColor(material->toonColor());
        for (SemanticUniformList::const_iterator it = m_materialToonColorUniforms.begin(),
                                                 end = m_materialToonColorUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, toonColor);
        }
    }
    if (const IImageView *diffuseImage = material->diffuseImage()) {
        for (SemanticUniformList::const_iterator it = m_diffuseImageUniforms.begin(),
                                                 end = m_diffuseImageUniforms.end();
             it != end; ++it) {
            setImageUniform(*it, pass, createOverrideImage(*it, diffuseImage, true));
        }
    }
    if (const IImageView *sphereImage = material->sphereMapImage()) {
        for (SemanticUniformList::const_iterator it = m_sphereImageUniforms.begin(), end = m_sphereImageUniforms.end();
             it != end; ++it) {
            setImageUniform(*it, pass, createOverrideImage(*it, sphereImage, true));
        }
    }
    if (const IImageView *toonImage = material->toonImage()) {
        if (!m_toonImageUniforms.empty() || target == kPassTypeObjectSelfShadow) {
            for (SemanticUniformList::const_iterator it = m_toonImageUniforms.begin(), end = m_toonImageUniforms.end();
                 it != end; ++it) {
                setImageUniform(*it, pass, createOverrideImage(*it, toonImage, false));
            }
        }
        else if (target == kPassTypeObject) {
            m_firstImageHandle = toonImage->handle();
        }
    }
    const Vector3 lightColor(project()->globalLight()->color());
    const nanoem_model_material_sphere_map_texture_type_t sphereMapTextureType = material->sphereMapImage() != nullptr
        ? nanoemModelMaterialGetSphereMapTextureType(materialPtr)
        : NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE;
    const model::Material::Color &mixedColor = material->color();
    const bool isAddSphereMap = sphereMapTextureType == NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_ADD;
    writeUniformBuffer("EgColor", pass,
        glm::clamp(Vector4(mixedColor.m_ambient + mixedColor.m_diffuse * lightColor, mixedColor.m_diffuseOpacity),
            Vector4(0), Vector4(1)));
    writeUniformBuffer("SpcColor", pass, Vector4(mixedColor.m_specular * lightColor, mixedColor.m_specularPower));
    writeUniformBuffer("spadd", pass, isAddSphereMap);
    const bool usingDiffuseTexture = material->diffuseImage() != nullptr;
    writeUniformBuffer("use_texture", pass, usingDiffuseTexture);
    const bool usingSphereMapTexture = sphereMapTextureType != NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE;
    writeUniformBuffer("use_spheremap", pass, usingSphereMapTexture);
    const bool usingSubTexture = sphereMapTextureType == NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_SUB_TEXTURE;
    writeUniformBuffer("use_subtexture", pass, usingSubTexture);
    const bool useToonTexture = material->toonImage() != nullptr;
    writeUniformBuffer("use_toon", pass, useToonTexture);
}

void
Effect::setEdgeParameters(const nanoem_model_material_t *materialPtr, nanoem_f32_t edgeSize, Pass *pass)
{
    nanoem_parameter_assert(materialPtr, "must not be nullptr");
    nanoem_parameter_assert(pass, "must not be nullptr");
    BX_UNUSED_1(edgeSize);
    if (!m_materialEdgeColorUniforms.empty()) {
        const model::Material *material = model::Material::cast(materialPtr);
        const model::Material::Edge &edge = material->edge();
        const Vector4 color(edge.m_color, edge.m_opacity);
        for (SemanticUniformList::const_iterator it = m_materialEdgeColorUniforms.begin(),
                                                 end = m_materialEdgeColorUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, color);
        }
    }
}

void
Effect::setShadowParameters(const ILight *light, const ICamera *camera, const Matrix4x4 &world, Pass *pass)
{
    nanoem_parameter_assert(light, "must not be nullptr");
    nanoem_parameter_assert(pass, "must not be nullptr");
    if (!m_cameraMatrixUniforms.empty()) {
        Matrix4x4 shadow, view, projection, result;
        light->getShadowTransform(shadow);
        camera->getViewTransform(view, projection);
        shadow = shadow * world;
        for (MatrixUniformMap::const_iterator it = m_cameraMatrixUniforms.begin(), end = m_cameraMatrixUniforms.end();
             it != end; ++it) {
            it->second.multiply(shadow, view, projection, result);
            writeUniformBuffer(it->first, pass, result);
        }
        writeUniformBuffer("matWorld", pass, shadow);
        writeUniformBuffer("matWorldViewProj", pass, projection * view * world);
    }
    if (!m_materialGroundColorUniforms.empty()) {
        const Vector4 color(light->groundShadowColor(), 1.0f + light->isTranslucentGroundShadowEnabled() * -0.5f);
        for (SemanticUniformList::const_iterator it = m_materialGroundColorUniforms.begin(),
                                                 end = m_materialGroundColorUniforms.end();
             it != end; ++it) {
            writeUniformBuffer(*it, pass, color);
        }
    }
}

void
Effect::setShadowMapParameters(const ShadowCamera *shadowCamera, const Matrix4x4 &world, Pass *pass)
{
    nanoem_parameter_assert(shadowCamera, "must NOT be nullptr");
    nanoem_parameter_assert(pass, "must not be nullptr");
    if (!m_lightMatrixUniforms.empty()) {
        Matrix4x4 view, projection, result;
        shadowCamera->getViewProjection(view, projection);
        for (MatrixUniformMap::const_iterator it = m_lightMatrixUniforms.begin(), end = m_lightMatrixUniforms.end();
             it != end; ++it) {
            it->second.multiply(world, view, projection, result);
            writeUniformBuffer(it->first, pass, result);
        }
    }
    if (shadowCamera->isEnabled()) {
        const sg_image shadowMapImage = shadowCamera->colorImage();
        bool sameOutput = m_currentRenderTargetPassDescription.color_attachments[0].image.id == shadowMapImage.id;
        m_firstImageHandle = sameOutput ? m_project->sharedFallbackImage() : shadowMapImage;
    }
}

void
Effect::handleWorldMatrixSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeWorld, false, false);
}

void
Effect::handleViewMatrixSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeView, false, false);
}

void
Effect::handleProjectMatrixSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeProjection, false, false);
}

void
Effect::handleWorldViewMatrixSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeWorldView, false, false);
}

void
Effect::handleViewProjectMatrixSemantic(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeViewProjection, false, false);
}

void
Effect::handleWorldViewProjectionMatrixSemantic(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeWorldViewProjection, false, false);
}

void
Effect::handleWorldMatrixSemanticInverse(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeWorld, true, false);
}

void
Effect::handleViewMatrixSemanticInverse(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeView, true, false);
}

void
Effect::handleProjectMatrixSemanticInverse(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeProjection, true, false);
}

void
Effect::handleWorldViewMatrixSemanticInverse(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeWorldView, true, false);
}

void
Effect::handleViewProjectMatrixSemanticInverse(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeViewProjection, true, false);
}

void
Effect::handleWorldViewProjectionMatrixSemanticInverse(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeWorldViewProjection, true, false);
}
void
Effect::handleWorldMatrixSemanticTranspose(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeWorld, false, true);
}

void
Effect::handleViewMatrixSemanticTranspose(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeView, false, true);
}

void
Effect::handleProjectMatrixSemanticTranspose(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeProjection, false, true);
}

void
Effect::handleWorldViewMatrixSemanticTranspose(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeWorldView, false, true);
}

void
Effect::handleViewProjectMatrixSemanticTranspose(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeViewProjection, false, true);
}

void
Effect::handleWorldViewProjectionMatrixSemanticTranspose(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeWorldViewProjection, false, true);
}

void
Effect::handleWorldMatrixSemanticInverseTranspose(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeWorld, true, true);
}

void
Effect::handleViewMatrixSemanticInverseTranspose(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeView, true, true);
}

void
Effect::handleProjectMatrixSemanticInverseTranspose(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeProjection, true, true);
}

void
Effect::handleWorldViewMatrixSemanticInverseTranspose(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeWorldView, true, true);
}

void
Effect::handleViewProjectMatrixSemanticInverseTranspose(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeViewProjection, true, true);
}

void
Effect::handleWorldViewProjectionMatrixSemanticInverseTranspose(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    self->handleMatrixSemantics(parameter, kMatrixTypeWorldViewProjection, true, true);
}

void
Effect::handleDiffuseSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        const AnnotationMap &annotations = parameter.m_annotations;
        const AnnotationMap::const_iterator it = annotations.findAnnotation(kObjectKeyLiteral);
        if (it != annotations.end()) {
            const String &name = parameter.m_name;
            const String &value = it->second.m_string;
            if (StringUtils::equals(value.c_str(), kObjectGeometryValueLiteral)) {
                self->m_materialDiffuseUniforms.insert(name);
            }
            else if (StringUtils::equals(value.c_str(), kObjectLightValueLiteral)) {
                self->m_lightDiffuseUniforms.insert(name);
            }
            else {
                self->addInvalidParameterValueError(value, parameter);
            }
        }
        else {
            self->addMissingParameterKeyError(kObjectKeyLiteral, parameter);
        }
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handleAmbientSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        const AnnotationMap &annotations = parameter.m_annotations;
        const AnnotationMap::const_iterator it = annotations.findAnnotation(kObjectKeyLiteral);
        if (it != annotations.end()) {
            const String &name = parameter.m_name;
            const String &value = it->second.m_string;
            if (StringUtils::equals(value.c_str(), kObjectGeometryValueLiteral)) {
                self->m_materialAmbientUniforms.insert(name);
            }
            else if (StringUtils::equals(value.c_str(), kObjectLightValueLiteral)) {
                self->m_lightAmbientUniforms.insert(name);
            }
            else {
                self->addInvalidParameterValueError(value, parameter);
            }
        }
        else {
            self->addMissingParameterKeyError(kObjectKeyLiteral, parameter);
        }
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handleEmissiveSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        const AnnotationMap &annotations = parameter.m_annotations;
        const AnnotationMap::const_iterator it = annotations.findAnnotation(kObjectKeyLiteral);
        if (it != annotations.end()) {
            const String &value = it->second.m_string;
            if (StringUtils::equals(value.c_str(), kObjectGeometryValueLiteral)) {
                const String &name = parameter.m_name;
                self->m_materialEmissiveUniforms.insert(name);
            }
            else {
                self->addInvalidParameterValueError(value, parameter);
            }
        }
        else {
            self->addMissingParameterKeyError(kObjectKeyLiteral, parameter);
        }
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handleSpecularSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        const AnnotationMap &annotations = parameter.m_annotations;
        const AnnotationMap::const_iterator it = annotations.findAnnotation(kObjectKeyLiteral);
        if (it != annotations.end()) {
            const String &name = parameter.m_name;
            const String &value = it->second.m_string;
            if (StringUtils::equals(value.c_str(), kObjectGeometryValueLiteral)) {
                self->m_materialSpecularUniforms.insert(name);
            }
            else if (StringUtils::equals(value.c_str(), kObjectLightValueLiteral)) {
                self->m_lightSpecularUniforms.insert(name);
            }
            else {
                self->addInvalidParameterValueError(value, parameter);
            }
        }
        else {
            self->addMissingParameterKeyError(kObjectKeyLiteral, parameter);
        }
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handleSpecularPowerSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat) {
        const AnnotationMap &annotations = parameter.m_annotations;
        const AnnotationMap::const_iterator it = annotations.findAnnotation(kObjectKeyLiteral);
        const String &name = parameter.m_name;
        if (it != annotations.end()) {
            const String &value = it->second.m_string;
            if (StringUtils::equals(value.c_str(), kObjectGeometryValueLiteral)) {
                self->m_materialSpecularPowerUniforms.insert(name);
            }
            else {
                self->addInvalidParameterValueError(value, parameter);
            }
        }
        else {
            self->m_materialSpecularPowerUniforms.insert(name);
        }
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat, parameter);
    }
}

void
Effect::handleToonColorSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        const AnnotationMap &annotations = parameter.m_annotations;
        const AnnotationMap::const_iterator it = annotations.findAnnotation(kObjectKeyLiteral);
        const String &name = parameter.m_name;
        if (it != annotations.end()) {
            const String &value = it->second.m_string;
            if (StringUtils::equals(value.c_str(), kObjectGeometryValueLiteral)) {
                self->m_materialToonColorUniforms.insert(name);
            }
            else {
                self->addInvalidParameterValueError(value, parameter);
            }
        }
        else {
            self->m_materialToonColorUniforms.insert(name);
        }
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handleEdgeColorSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        const AnnotationMap &annotations = parameter.m_annotations;
        const AnnotationMap::const_iterator it = annotations.findAnnotation(kObjectKeyLiteral);
        const String &name = parameter.m_name;
        if (it != annotations.end()) {
            const String &value = it->second.m_string;
            if (StringUtils::equals(value.c_str(), kObjectGeometryValueLiteral)) {
                self->m_materialEdgeColorUniforms.insert(name);
            }
            else {
                self->addInvalidParameterValueError(value, parameter);
            }
        }
        else {
            self->m_materialEdgeColorUniforms.insert(name);
        }
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handleGroundShadowColorSemantic(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        self->m_materialGroundColorUniforms.insert(parameter.m_name);
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handlePositionSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        const AnnotationMap &annotations = parameter.m_annotations;
        const AnnotationMap::const_iterator it = annotations.findAnnotation(kObjectKeyLiteral);
        if (it != annotations.end()) {
            const String &name = parameter.m_name;
            const String &value = it->second.m_string;
            if (StringUtils::equals(value.c_str(), kObjectLightValueLiteral)) {
                self->m_lightPositionUniforms.insert(name);
            }
            else if (StringUtils::equals(value.c_str(), kObjectCameraValueLiteral)) {
                self->m_cameraPositionUniforms.insert(name);
            }
            else {
                self->addInvalidParameterValueError(value, parameter);
            }
        }
        else {
            self->addMissingParameterKeyError(kObjectKeyLiteral, parameter);
        }
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handleDirectionSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        const AnnotationMap &annotations = parameter.m_annotations;
        const AnnotationMap::const_iterator it = annotations.findAnnotation(kObjectKeyLiteral);
        if (it != annotations.end()) {
            const String &name = parameter.m_name;
            const String &value = it->second.m_string;
            if (StringUtils::equals(value.c_str(), kObjectLightValueLiteral)) {
                self->m_lightDirectionUniforms.insert(name);
            }
            else if (StringUtils::equals(value.c_str(), kObjectCameraValueLiteral)) {
                self->m_cameraDirectionUniforms.insert(name);
            }
            else {
                self->addInvalidParameterValueError(value, parameter);
            }
        }
        else {
            self->addMissingParameterKeyError(kObjectKeyLiteral, parameter);
        }
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handleMaterialTextureSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    const ParameterType parameterType = parameter.m_type;
    if (parameterType == kParameterTypeTexture || parameterType == kParameterTypeTexture2D) {
        self->m_diffuseImageUniforms.insert(parameter.m_name);
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeTexture, parameter);
    }
}

void
Effect::handleMaterailSphereMapSemantic(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    const ParameterType parameterType = parameter.m_type;
    if (parameterType == kParameterTypeTexture || parameterType == kParameterTypeTexture2D) {
        self->m_sphereImageUniforms.insert(parameter.m_name);
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeTexture, parameter);
    }
}

void
Effect::handleMaterialToonTextureSemantic(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    const ParameterType parameterType = parameter.m_type;
    if (parameterType == kParameterTypeTexture || parameterType == kParameterTypeTexture2D) {
        self->m_toonImageUniforms.insert(parameter.m_name);
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeTexture, parameter);
    }
}

void
Effect::handleAddingTextureSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        self->m_addingDiffuseImageBlendFactorUniforms.insert(parameter.m_name);
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handleMultiplyingTextureSemantic(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        self->m_multiplyingDiffuseImageBlendFactorUniforms.insert(parameter.m_name);
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handleAddingSphereTextureSemantic(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        self->m_addingSphereImageBlendFactorUniforms.insert(parameter.m_name);
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handleMultiplyingSphereTextureSemantic(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        self->m_multiplyingSphereImageBlendFactorUniforms.insert(parameter.m_name);
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handleViewportPixelSizeSemantic(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        self->m_viewportPixelUniforms.insert(parameter.m_name);
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handleTimeSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat) {
        const AnnotationMap &annotations = parameter.m_annotations;
        const AnnotationMap::const_iterator it = annotations.findAnnotation(kSyncInEditModeKeyLiteral);
        const String &name = parameter.m_name;
        if (it != annotations.end()) {
            if (it->second.m_bool) {
                self->m_timeUniforms.insert(name);
                return;
            }
        }
        self->m_systemTimeUniforms.insert(name);
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat, parameter);
    }
}

void
Effect::handleElapsedTimeSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat) {
        const AnnotationMap &annotations = parameter.m_annotations;
        const AnnotationMap::const_iterator it = annotations.findAnnotation(kSyncInEditModeKeyLiteral);
        const String &name = parameter.m_name;
        if (it != annotations.end()) {
            if (it->second.m_bool) {
                self->m_elapsedTimeUniforms.insert(name);
                return;
            }
        }
        self->m_elapsedSystemTimeUniforms.insert(name);
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat, parameter);
    }
}

void
Effect::handleMousePositionSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        self->m_mousePositionUniforms.insert(parameter.m_name);
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handleLeftMouseDownSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        self->m_leftMouseDownUniforms.insert(parameter.m_name);
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handleMiddleMouseDownSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        self->m_middleMouseDownUniforms.insert(parameter.m_name);
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handleRightMouseDownSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        self->m_rightMouseDownUniforms.insert(parameter.m_name);
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handleControlObjectSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    const AnnotationMap &annotations = parameter.m_annotations;
    AnnotationMap::const_iterator it = annotations.findAnnotation(kNameKeyLiteral);
    if (it != annotations.end()) {
        ControlObjectTarget target(it->second.m_string, parameter.m_type);
        AnnotationMap::const_iterator it2 = annotations.findAnnotation(kItemKeyLiteral);
        if (it2 != parameter.m_annotations.end()) {
            target.m_item = it2->second.m_string;
        }
        const String &parameterName = parameter.m_name;
        self->m_controlObjectTargets.insert(tinystl::make_pair(parameterName, target));
        self->m_controlObjectUniforms.insert(parameterName);
    }
    else {
        self->addMissingParameterKeyError(kNameKeyLiteral, parameter);
    }
}

void
Effect::handleRenderColorTargetSemantic(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    const ParameterType parameterType = parameter.m_type;
    if (parameterType == kParameterTypeTexture || parameterType == kParameterTypeTexture2D) {
        const String &name = parameter.m_name;
        Project *project = self->m_project;
        RenderTargetColorImageContainer *sharedContainer = project->findSharedRenderTargetImageContainer(name, nullptr),
                                        *container = nanoem_new(RenderTargetColorImageContainer(name));
        if (sharedContainer) {
            container->share(sharedContainer);
        }
        else {
            Vector2 scaleFactor(0);
            const AnnotationMap &annotations = parameter.m_annotations;
            const Vector4 size(
                self->determineImageSize(annotations, self->scaledViewportImageSize(Vector2(1)), scaleFactor));
            const sg_pixel_format format =
                self->determinePixelFormat(annotations, self->m_project->viewportPixelFormat());
            const nanoem_u8_t numMipLevels = self->determineMipLevels(annotations, size, 1);
            bool enableAA = RenderTargetColorImageContainer::kAntialiasEnabled;
            AnnotationMap::const_iterator it2 = annotations.findAnnotation(kAntiAliasKeyLiteral);
            if (it2 != annotations.end()) {
                enableAA = it2->second.toBool();
            }
            self->setNormalizedColorImageContainer(parameter.m_name, numMipLevels, container);
            int sampleCount = enableAA ? project->sampleCount() : 1;
            container->create(self, size, scaleFactor, numMipLevels, sampleCount, format);
            if (parameter.m_shared) {
                project->setSharedRenderTargetImageContainer(name, self, container);
            }
        }
        self->m_drawableNamedRenderTargetColorImages[nullptr].insert(tinystl::make_pair(name, container));
        self->m_renderTargetColorUniforms.insert(name);
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeTexture, parameter);
    }
}

void
Effect::handleRenderDepthStencilTargetSemantic(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    const ParameterType parameterType = parameter.m_type;
    if (parameterType == kParameterTypeTexture || parameterType == kParameterTypeTexture2D) {
        Vector2 scaleFactor(0);
        const AnnotationMap &annotations = parameter.m_annotations;
        const Vector4 size(
            self->determineImageSize(annotations, self->scaledViewportImageSize(Vector2(1)), scaleFactor));
        const sg_pixel_format format =
            self->determineDepthStencilPixelFormat(annotations, SG_PIXELFORMAT_DEPTH_STENCIL);
        const nanoem_u8_t numMipLevels = self->determineMipLevels(annotations, size, 1);
        const String &name = parameter.m_name;
        bool enableAA = RenderTargetColorImageContainer::kAntialiasEnabled;
        RenderTargetDepthStencilImageContainer *container = nanoem_new(RenderTargetDepthStencilImageContainer(name));
        ImageDescriptionMap::const_iterator it = self->m_imageDescriptions.find(parameter.m_name);
        if (it != self->m_imageDescriptions.end()) {
            container->setImageDescription(it->second);
        }
        int sampleCount = enableAA ? self->m_project->sampleCount() : 1;
        container->create(self, size, scaleFactor, numMipLevels, sampleCount, format);
        self->m_renderTargetDepthStencilUniforms.insert(name);
        self->m_renderTargetDepthStencilImages.insert(tinystl::make_pair(name, container));
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeTexture, parameter);
    }
}

void
Effect::handleAnimatedTextureSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress &progress)
{
    const ParameterType parameterType = parameter.m_type;
    if (parameterType == kParameterTypeTexture || parameterType == kParameterTypeTexture2D) {
        const AnnotationMap &annotations = parameter.m_annotations;
        AnnotationMap::const_iterator it = annotations.findAnnotation(kResourceNameKeyLiteral);
        if (it != annotations.end()) {
            AnnotationMap::const_iterator it2 = annotations.findAnnotation(kSeekVariableKeyLiteral);
            String seekVariable;
            if (it2 != annotations.end()) {
                seekVariable = it2->second.m_string;
            }
            const nanoem_f32_t offset = annotations.floatAnnotation(kOffsetKeyLiteral, 0.0f),
                               speed = annotations.floatAnnotation(kSpeedKeyLiteral, 1.0f);
            Error error;
            String filename;
            FileUtils::canonicalizePathSeparator(it->second.m_string.c_str(), filename);
            const URI &fileURI = Project::resolveArchiveURI(self->fileURI(), filename);
            image::APNG *animation = nullptr;
            FileReaderScope scope(self->m_project->translator());
            if (scope.open(fileURI, error)) {
                animation = ImageLoader::decodeAPNG(scope.reader(), error);
            }
            if (animation) {
                const String &name = parameter.m_name;
                AnimatedImageContainer *container =
                    nanoem_new(AnimatedImageContainer(name, seekVariable, animation, offset, speed));
                container->create();
                self->m_animatedTextureImages.insert(tinystl::make_pair(name, container));
                self->m_animatedTextureUniforms.insert(name);
            }
            else {
                IEventPublisher *eventPublisher = self->m_project->eventPublisher();
                error.notify(eventPublisher);
                Error innerError;
                self->createImageResourceFromArchive(parameter, nullptr, progress, innerError);
                innerError.notify(eventPublisher);
            }
        }
        else {
            self->addMissingParameterKeyError(kObjectKeyLiteral, parameter);
        }
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeTexture, parameter);
    }
}

void
Effect::handleOffscreenRenderTargetSemantic(
    Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    const ParameterType parameterType = parameter.m_type;
    if (parameterType == kParameterTypeTexture || parameterType == kParameterTypeTexture2D) {
        const String &name = parameter.m_name;
        Project *project = self->m_project;
        RenderTargetColorImageContainer *sharedContainer = project->findSharedRenderTargetImageContainer(name, nullptr);
        OffscreenRenderTargetImageContainer *container = nanoem_new(OffscreenRenderTargetImageContainer(name));
        if (sharedContainer) {
            container->share(sharedContainer);
        }
        else {
            Vector2 scaleFactor(0);
            const AnnotationMap &annotations = parameter.m_annotations;
            const Vector4 size(
                self->determineImageSize(annotations, self->scaledViewportImageSize(Vector2(1)), scaleFactor));
            const sg_pixel_format format = self->determinePixelFormat(annotations, SG_PIXELFORMAT_RGBA8);
            const nanoem_u8_t numMipLevels = self->determineMipLevels(annotations, size, 1);
            AnnotationMap::const_iterator it;
            String description;
            Vector4 clearColor(Constants::kZeroV3, 1.0f);
            bool enableAA = false;
            it = annotations.findAnnotation(kClearColorKeyLiteral);
            if (it != annotations.end()) {
                clearColor = glm::clamp(it->second.m_vector, 0.0f, 1.0f);
            }
            nanoem_f32_t clearDepth = glm::clamp(annotations.floatAnnotation(kClearDepthKeyLiteral, 1.0f), 0.0f, 1.0f);
            it = annotations.findAnnotation(kAntiAliasKeyLiteral);
            if (it != annotations.end()) {
                enableAA = it->second.toBool();
            }
            it = annotations.findAnnotation(kDescriptionKeyLiteral);
            if (it != annotations.end()) {
                description = it->second.m_string;
            }
            OffscreenRenderTargetOption option(parameter.m_name, description, clearColor, clearDepth);
            it = annotations.findAnnotation(kDefaultEffectKeyLiteral);
            if (it != annotations.end()) {
                MutableString newValueString;
                StringUtils::copyString(it->second.m_string, newValueString);
                char *ptr = newValueString.data();
                while (char *p = StringUtils::indexOf(ptr, ';')) {
                    *p = '\0';
                    if (char *q = StringUtils::indexOf(ptr, '=')) {
                        *q = '\0';
                        q += 1;
                        option.m_conditions.push_back(
                            tinystl::make_pair(StringUtils::skipWhiteSpaces(ptr), StringUtils::skipWhiteSpaces(q)));
                    }
                    ptr = p + 1;
                }
                if (char *q = StringUtils::indexOf(ptr, '=')) {
                    *q = '\0';
                    q += 1;
                    option.m_conditions.push_back(
                        tinystl::make_pair(StringUtils::skipWhiteSpaces(ptr), StringUtils::skipWhiteSpaces(q)));
                }
            }
            self->setNormalizedColorImageContainer(parameter.m_name, numMipLevels, container);
            int sampleCount = enableAA ? project->sampleCount() : 1;
            container->create(self, size, scaleFactor, numMipLevels, sampleCount, format);
            self->m_offscreenRenderTargetOptions.insert(tinystl::make_pair(name, option));
            if (parameter.m_shared) {
                project->setSharedRenderTargetImageContainer(name, self, container);
            }
        }
        self->m_offscreenRenderTargetUniforms.insert(name);
        self->m_offscreenRenderTargetImages.insert(tinystl::make_pair(name, container));
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeTexture, parameter);
    }
}

void
Effect::handleTextureValueSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat4) {
        AnnotationMap::const_iterator it = parameter.m_annotations.findAnnotation(kResourceNameKeyLiteral);
        if (it != parameter.m_annotations.end()) {
            self->m_textureValueUniforms.insert(parameter.m_name);
            self->m_textureValueTargets.insert(tinystl::make_pair(parameter.m_name, it->second.m_string));
        }
        else {
            AnnotationMap::const_iterator it2 = parameter.m_annotations.findAnnotation(kTextureNameKeyLiteral);
            if (it2 != parameter.m_annotations.end()) {
                self->m_textureValueUniforms.insert(parameter.m_name);
                self->m_textureValueTargets.insert(tinystl::make_pair(parameter.m_name, it2->second.m_string));
            }
            else {
                self->addMissingParameterKeyError(kTextureNameKeyLiteral, parameter);
            }
        }
    }
    else {
        self->addInvalidParameterTypeError(kParameterTypeFloat4, parameter);
    }
}

void
Effect::handleStandardsGlobalSemantic(Effect *self, const TypedSemanticParameter &parameter, Progress & /* progress */)
{
    if (parameter.m_type == kParameterTypeFloat) {
        const AnnotationMap &annotations = parameter.m_annotations;
        AnnotationMap::const_iterator it;
        it = annotations.findAnnotation(kScriptOutputKeyLiteral);
        if (it != annotations.end()) {
            const String &value = it->second.m_string;
            if (StringUtils::equals(value.c_str(), kScriptOutputColorValueLiteral)) {
                self->m_scriptOutput = value;
            }
            else {
                self->addInvalidParameterValueError(value, parameter);
            }
        }
        it = annotations.findAnnotation(kScriptClassKeyLiteral);
        if (it != annotations.end()) {
            const String &value = it->second.m_string;
            if (StringUtils::equals(value.c_str(), kScriptClassObjectValueLiteral)) {
                self->m_scriptClass = kScriptClassTypeObject;
            }
            else if (StringUtils::equals(value.c_str(), kScriptClassSceneValueLiteral)) {
                self->m_scriptClass = kScriptClassTypeScene;
            }
            else if (StringUtils::equals(value.c_str(), kScriptClassSceneObjectValueLiteral)) {
                self->m_scriptClass = kScriptClassTypeSceneObject;
            }
            else {
                self->addInvalidParameterValueError(value, parameter);
            }
        }
        it = annotations.findAnnotation(kScriptOrderKeyLiteral);
        if (it != annotations.end()) {
            const String &value = it->second.m_string;
            if (StringUtils::equals(value.c_str(), kScriptOrderStandardValueLiteral)) {
                self->m_scriptOrder = kScriptOrderTypeStandard;
            }
            else if (StringUtils::equals(value.c_str(), kScriptOrderPostProcessValueLiteral)) {
                self->m_scriptOrder = kScriptOrderTypePostProcess;
            }
            else if (StringUtils::equals(value.c_str(), kScriptOrderPreProcessValueLiteral)) {
                self->m_scriptOrder = kScriptOrderTypePreProcess;
            }
            else {
                self->addInvalidParameterValueError(value, parameter);
            }
        }
        it = annotations.findAnnotation(kScriptKeyLiteral);
        if (it != annotations.end()) {
            const String &value = it->second.m_string;
            Effect::parseScript(value, self->m_standardScript);
        }
    }
    else {
        self->addMissingParameterKeyError(kNameKeyLiteral, parameter);
    }
}

sg_image_type
Effect::determineImageType(const AnnotationMap &annotations)
{
    AnnotationMap::const_iterator it = annotations.findAnnotation(kResourceTypeKeyLiteral);
    sg_image_type type = _SG_IMAGETYPE_DEFAULT;
    if (it != annotations.end()) {
        const String &value = it->second.m_string;
        if (StringUtils::equals(value.c_str(), kResourceType1DValueLiteral)) {
            type = SG_IMAGETYPE_2D;
        }
        else if (StringUtils::equals(value.c_str(), kResourceType2DValueLiteral)) {
            type = SG_IMAGETYPE_2D;
        }
        else if (StringUtils::equals(value.c_str(), kResourceType3DValueLiteral)) {
            type = SG_IMAGETYPE_3D;
        }
        else if (StringUtils::equals(value.c_str(), kResourceTypeCubeValueLiteral)) {
            type = SG_IMAGETYPE_CUBE;
        }
    }
    return type;
}

sg_image_type
Effect::determineImageType(const AnnotationMap &annotations, ParameterType parameterType)
{
    sg_image_type type = _SG_IMAGETYPE_DEFAULT;
    if (parameterType >= kParameterTypeTexture1D && parameterType <= kParameterTypeTextureCube) {
        switch (parameterType) {
        case kParameterTypeTexture1D:
        case kParameterTypeTexture2D:
            type = SG_IMAGETYPE_2D;
            break;
        case kParameterTypeTexture3D:
            type = SG_IMAGETYPE_3D;
            break;
        case kParameterTypeTextureCube:
            type = SG_IMAGETYPE_CUBE;
            break;
        default:
            break;
        }
    }
    else {
        type = determineImageType(annotations);
    }
    return type;
}

nanoem_u8_t
Effect::determineMipLevels(const AnnotationMap &annotations, const Vector2 &size, int defaultLevel)
{
    AnnotationMap::const_iterator it = annotations.findAnnotation(kMiplevelsKeyLiteral);
    int numMipLevels = 1, maxMipLevels = glm::min(int(glm::log2(glm::max(size.x, size.y))), int(SG_MAX_MIPMAPS));
    if (it != annotations.end()) {
        int value = it->second.toInt();
        numMipLevels = value > 0 ? value : maxMipLevels;
    }
    else {
        AnnotationMap::const_iterator it2 = annotations.findAnnotation(kLevelsKeyLiteral);
        if (it2 != annotations.end()) {
            int value = it2->second.toInt();
            numMipLevels = value > 0 ? value : maxMipLevels;
        }
        else {
            numMipLevels = defaultLevel;
        }
    }
    return glm::clamp(numMipLevels, 0, maxMipLevels);
}

const Effect::NamedRenderTargetColorImageContainerMap *
Effect::findNamedRenderTargetColorImageContainerMap(const IDrawable *drawable) const
{
    const NamedRenderTargetColorImageContainerMap *result = nullptr;
    DrawableNamedRenderTargetColorImageContainerMap::const_iterator it =
        m_drawableNamedRenderTargetColorImages.find(drawable);
    if (it != m_drawableNamedRenderTargetColorImages.end() && !it->second.empty()) {
        result = &it->second;
    }
    else {
        static const NamedRenderTargetColorImageContainerMap kNullMap;
        it = m_drawableNamedRenderTargetColorImages.find(nullptr);
        result = it != m_drawableNamedRenderTargetColorImages.end() ? &it->second : &kNullMap;
    }
    return result;
}

effect::Technique *
Effect::internalFindTechnique(const String &passType, nanoem_rsize_t numMaterials, nanoem_rsize_t materialIndex,
    const nanodxm_material_t *material, Accessory *accessory)
{
    TechniqueListMap::const_iterator it = m_techniqueByPassTypes.find(passType);
    effect::Technique *foundTechnique = nullptr;
    if (it != m_techniqueByPassTypes.end()) {
        tinystl::unordered_set<int, TinySTLAllocator> subset;
        const Accessory::Material *materialPtr = accessory->findMaterial(material);
        const nanodxm_color_t color = nanodxmMaterialGetEmissive(material);
        const bool isCullingEnabled = !(color.a < 1.0f);
        const TechniqueList &techniques = it->second;
        for (TechniqueList::const_iterator it2 = techniques.begin(), end2 = techniques.end(); it2 != end2; ++it2) {
            effect::Technique *technique = *it2;
            subset.clear();
            parseSubset(technique->subset(), size_t(numMaterials), subset);
            const bool matchSubset =
                subset.empty() || subset.find(Inline::saturateInt32(materialIndex)) != subset.end();
            if (matchSubset && technique->match(materialPtr)) {
                sg_pipeline_desc &desc = technique->mutablePipelineDescription();
                Accessory::setStandardPipelineDescription(desc);
                if (passType == kPassTypeZplot) {
                    m_project->setCurrentRenderPass(m_project->shadowCamera()->pass());
                }
                else if (passType == kPassTypeShadow) {
                    Project::setShadowDepthStencilState(desc.depth, desc.stencil);
                }
                else if (isCullingEnabled) {
                    desc.face_winding = SG_FACEWINDING_CCW;
                    desc.cull_mode = SG_CULLMODE_BACK;
                }
                resetPassDescription();
                foundTechnique = technique;
                break;
            }
        }
    }
    return foundTechnique;
}

effect::Technique *
Effect::internalFindTechnique(
    const String &passType, nanoem_rsize_t numMaterials, const nanoem_model_material_t *material)
{
    TechniqueListMap::const_iterator it = m_techniqueByPassTypes.find(passType);
    effect::Technique *foundTechnique = nullptr;
    if (it != m_techniqueByPassTypes.end()) {
        tinystl::unordered_set<int, TinySTLAllocator> subset;
        const int materialIndex = model::Material::index(material);
        const TechniqueList &techniques = it->second;
        for (TechniqueList::const_iterator it2 = techniques.begin(), end2 = techniques.end(); it2 != end2; ++it2) {
            effect::Technique *technique = *it2;
            subset.clear();
            parseSubset(technique->subset(), numMaterials, subset);
            const bool matchSubset = subset.empty() || subset.find(materialIndex) != subset.end();
            if (matchSubset && technique->match(material)) {
                sg_pipeline_desc &desc = technique->mutablePipelineDescription();
                const bool isEdge = passType == kPassTypeEdge;
                if (isEdge) {
                    Model::setEdgePipelineDescription(desc);
                    desc.cull_mode = SG_CULLMODE_FRONT;
                }
                else {
                    Model::setStandardPipelineDescription(desc);
                    desc.cull_mode =
                        nanoemModelMaterialIsCullingDisabled(material) ? SG_CULLMODE_NONE : SG_CULLMODE_BACK;
                }
                if (nanoemModelMaterialIsPointDrawEnabled(material)) {
                    desc.primitive_type = SG_PRIMITIVETYPE_POINTS;
                }
                else if (nanoemModelMaterialIsLineDrawEnabled(material)) {
                    desc.primitive_type = SG_PRIMITIVETYPE_LINES;
                }
                if (passType == kPassTypeZplot) {
                    m_project->setCurrentRenderPass(m_project->shadowCamera()->pass());
                }
                else if (passType == kPassTypeShadow) {
                    Project::setShadowDepthStencilState(desc.depth, desc.stencil);
                }
                resetPassDescription();
                foundTechnique = technique;
                break;
            }
        }
    }
    return foundTechnique;
}

IEffect::ImageResourceParameter
Effect::createImageResourceParameter(const TypedSemanticParameter &parameter) const
{
    return createImageResourceParameter(parameter, URI(), String());
}

IEffect::ImageResourceParameter
Effect::createImageResourceParameter(
    const TypedSemanticParameter &parameter, const URI &fileURI, const String &filename) const
{
    Vector2 scaleFactor(0);
    const AnnotationMap &annotations = parameter.m_annotations;
    const Vector4 size(determineImageSize(annotations, Vector4(64, 64, 1, 0), scaleFactor));
    sg_image_desc desc;
    Inline::clearZeroMemory(desc);
    ImageDescriptionMap::const_iterator it2 = m_imageDescriptions.find(parameter.m_name);
    if (it2 != m_imageDescriptions.end()) {
        desc = it2->second;
    }
    const Vector3SI32 imageSize(size);
    desc.width = imageSize.x;
    desc.height = imageSize.y;
    desc.num_slices = imageSize.z;
    desc.pixel_format = determinePixelFormat(annotations, SG_PIXELFORMAT_RGBA8);
    desc.type = determineImageType(annotations, parameter.m_type);
    if (desc.num_mipmaps == 0) {
        desc.num_mipmaps = determineMipLevels(annotations, size, SG_MAX_MIPMAPS);
    }
    return ImageResourceParameter(parameter.m_name, fileURI, filename, desc, parameter.m_shared);
}

void
Effect::createBlankImageResource(const TypedSemanticParameter &parameter)
{
    const ImageResourceParameter resourceParameter(createImageResourceParameter(parameter));
    registerImageResource(sg::make_image(&resourceParameter.m_desc), resourceParameter);
}

void
Effect::createImageResourceFromArchive(
    const TypedSemanticParameter &parameter, const Archiver *archiver, Progress &progress, Error &error)
{
    const AnnotationMap &annotations = parameter.m_annotations;
    AnnotationMap::const_iterator it = annotations.findAnnotation("ResourceName");
    if (it != annotations.end()) {
        createImageResourceFromArchive(it->second.m_string, parameter, archiver, progress, error);
    }
    else {
        createBlankImageResource(parameter);
    }
}

void
Effect::createImageResourceFromArchive(const String &name, const TypedSemanticParameter &parameter,
    const Archiver *archiver, Progress &progress, Error &error)
{
    String filename;
    FileUtils::canonicalizePathSeparator(name.c_str(), filename);
    const URI &imageURI = Project::resolveArchiveURI(fileURI(), filename);
    if (archiver && Project::isArchiveURI(imageURI)) {
        const URI &resolvedURI = m_project->resolveFileURI(imageURI);
        Archiver::Entry entry;
        ByteArray bytes;
        if (!progress.tryLoadingItem(imageURI)) {
            error = Error::cancelled();
        }
        else if (archiver->findEntry(resolvedURI.fragment(), entry, error) && archiver->extract(entry, bytes, error)) {
            const ImageResourceParameter resourceParameter(createImageResourceParameter(parameter, imageURI, filename));
            decodeImageData(bytes, resourceParameter, error);
        }
    }
    else if (FileUtils::exists(imageURI)) {
        FileReaderScope scope(m_project->translator());
        if (!progress.tryLoadingItem(imageURI)) {
            error = Error::cancelled();
        }
        else if (scope.open(imageURI, error)) {
            ByteArray bytes;
            FileUtils::read(scope, bytes, error);
            if (!error.hasReason()) {
                const ImageResourceParameter resourceParameter(
                    createImageResourceParameter(parameter, imageURI, filename));
                decodeImageData(bytes, resourceParameter, error);
            }
        }
    }
    else {
        m_logger->log("Image \"%s\" of \"%s\" in \"%s\" cannot be loaded", name.c_str(),
            imageURI.absolutePathConstString(), nameConstString());
    }
}

void
Effect::registerImageResource(sg_image image, const ImageResourceParameter &parameter)
{
    const String &name = parameter.m_name;
    setImageLabel(image, name);
    m_textureResourceUniforms.insert(name);
    m_resourceImages.insert(tinystl::make_pair(name, image));
    m_imageResources.push_back(parameter);
    RenderTargetColorImageContainer *sharedContainer = m_project->findSharedRenderTargetImageContainer(name, nullptr);
    if (sharedContainer && parameter.m_shared) {
        sharedContainer->setColorImageHandle(image);
    }
}

sg_image
Effect::createOverrideImage(const String &name, const IImageView *image, bool mipmap)
{
    ImageDescriptionMap::const_iterator it = m_imageDescriptions.find(name);
    sg_image handle = image->handle();
    if (it != m_imageDescriptions.end()) {
        const sg_image_desc originImageDescription(image->description());
        bx::HashMurmur2A hash;
        hash.begin();
        hash.add(name.c_str(), Inline::saturateInt32(name.size()));
        hash.add(handle);
        const nanoem_u32_t key = hash.end();
        OverridenImageHandleMap::const_iterator it2 = m_overridenImageHandles.find(key);
        if (it2 != m_overridenImageHandles.end()) {
            handle = it2->second;
        }
        else if (originImageDescription.width > 0 && originImageDescription.height > 0) {
            const sg_image_desc &overridenImageDescription = it->second;
            sg_image_desc imageDescription(originImageDescription);
            const sg_filter minFilterValue = overridenImageDescription.min_filter;
            const int numMipmaps = overridenImageDescription.num_mipmaps;
            if (mipmap && m_project->isMipmapEnabled() && numMipmaps != 1) {
                switch (minFilterValue) {
                case SG_FILTER_LINEAR: {
                    imageDescription.min_filter = SG_FILTER_LINEAR_MIPMAP_LINEAR;
                    break;
                }
                case SG_FILTER_NEAREST: {
                    imageDescription.min_filter = SG_FILTER_NEAREST_MIPMAP_NEAREST;
                    break;
                }
                case SG_FILTER_LINEAR_MIPMAP_LINEAR:
                case SG_FILTER_LINEAR_MIPMAP_NEAREST:
                case SG_FILTER_NEAREST_MIPMAP_LINEAR:
                case SG_FILTER_NEAREST_MIPMAP_NEAREST: {
                    imageDescription.min_filter = minFilterValue;
                    break;
                }
                default:
                    break;
                }
            }
            else {
                imageDescription.min_filter = minFilterValue;
                RenderState::normalizeMinFilter(imageDescription.min_filter);
            }
            if (numMipmaps > 0) {
                imageDescription.num_mipmaps = numMipmaps;
            }
            imageDescription.mag_filter = overridenImageDescription.mag_filter;
            imageDescription.wrap_u = overridenImageDescription.wrap_u;
            imageDescription.wrap_v = overridenImageDescription.wrap_v;
            char suffix[Inline::kMarkerStringLength], label[Inline::kMarkerStringLength];
            StringUtils::format(suffix, sizeof(suffix), "%s/%s", name.c_str(), image->filenameConstString());
            if (Inline::isDebugLabelEnabled()) {
                StringUtils::format(label, sizeof(label), "Effects/%s/%s", nameConstString(), suffix);
                imageDescription.label = label;
            }
            handle = sg::make_image(&imageDescription);
            setImageLabel(handle, suffix);
            m_overridenImageHandles.insert(tinystl::make_pair(key, handle));
        }
    }
    return handle;
}

sg_pixel_format
Effect::determinePixelFormat(const AnnotationMap &annotations, sg_pixel_format defaultFormat) const NANOEM_DECL_NOEXCEPT
{
    AnnotationMap::const_iterator it = annotations.findAnnotation(kFormatKeyLiteral);
    sg_pixel_format choosedFormat = defaultFormat;
    if (it != annotations.end()) {
        ImageFormatMap::const_iterator it2 = m_imageFormats.find(it->second.m_string);
        choosedFormat = it2 != m_imageFormats.end() ? it2->second : defaultFormat;
    }
    return choosedFormat;
}

sg_pixel_format
Effect::determineDepthStencilPixelFormat(
    const AnnotationMap &annotations, sg_pixel_format defaultFormat) const NANOEM_DECL_NOEXCEPT
{
    AnnotationMap::const_iterator it = annotations.findAnnotation(kFormatKeyLiteral);
    sg_pixel_format choosedFormat = defaultFormat;
    if (it != annotations.end()) {
        ImageFormatMap::const_iterator it2 = m_depthStencilImageFormats.find(it->second.m_string);
        choosedFormat = it2 != m_depthStencilImageFormats.end() ? it2->second : defaultFormat;
    }
    return choosedFormat;
}

Vector4
Effect::scaledViewportImageSize(const Vector2 &scaleFactor) const NANOEM_DECL_NOEXCEPT
{
    static const Vector2 kMaxSize(UINT16_MAX), kMinSize(1);
    const Vector2 size(m_project->deviceScaleViewportPrimaryImageSize()),
        normalizedScaleFactor(glm::max(scaleFactor, 0.0f));
    return Vector4(glm::clamp(size * normalizedScaleFactor, kMinSize, kMaxSize), 1, normalizedScaleFactor.x);
}

Vector4
Effect::determineImageSize(
    const AnnotationMap &annotations, const Vector4 &defaultValue, Vector2 &scaleFactor) const NANOEM_DECL_NOEXCEPT
{
    AnnotationMap::const_iterator it = annotations.findAnnotation(kViewportRatioKeyLiteral);
    Vector4 result(defaultValue);
    if (it != annotations.end()) {
        scaleFactor = it->second.m_vector;
        result = scaledViewportImageSize(scaleFactor);
    }
    else {
        it = annotations.findAnnotation(kDimensionsKeyLiteral);
        if (it != annotations.end()) {
            static const Vector3 kMaxDimension(UINT16_MAX, UINT16_MAX, UINT8_MAX), kMinDimension(1, 1, 0);
            const Vector4 dimension(it->second.m_vector);
            result = Vector4(glm::clamp(Vector3(dimension), kMinDimension, kMaxDimension), 0);
        }
        else {
            const sg_limits limits = sg::query_limits();
            AnnotationMap::const_iterator it1 = annotations.findAnnotation(kWidthKeyLiteral);
            AnnotationMap::const_iterator it2 = annotations.findAnnotation(kHeightKeyLiteral);
            AnnotationMap::const_iterator it3 = annotations.findAnnotation(kDepthKeyLiteral);
            if (it1 != annotations.end() && it2 != annotations.end() && it3 != annotations.end()) {
                const int limit3D = Inline::saturateInt32(limits.max_image_size_3d),
                          width = glm::clamp(it1->second.toInt(), 1, limit3D),
                          height = glm::clamp(it2->second.toInt(), 1, limit3D),
                          depth = glm::clamp(it3->second.toInt(), 1, limit3D);
                result = Vector4(width, height, depth, 0);
            }
            else if (it1 != annotations.end() && it2 != annotations.end()) {
                const int limit2D = Inline::saturateInt32(limits.max_image_size_2d),
                          width = glm::clamp(it1->second.toInt(), 1, limit2D),
                          height = glm::clamp(it2->second.toInt(), 1, limit2D);
                result = Vector4(width, height, 1, 0);
            }
            else if (it1 != annotations.end()) {
                const int limit2D = Inline::saturateInt32(limits.max_image_size_2d),
                          width = glm::clamp(it1->second.toInt(), 1, limit2D);
                result = Vector4(width, width, 1, 0);
            }
        }
    }
    return result;
}

void
Effect::setImageUniform(const String &name, const effect::Pass *pass, sg_image handle)
{
    SamplerRegisterIndex::List indices;
    if (pass->findPixelShaderSamplerRegisterIndex(name, indices)) {
        for (SamplerRegisterIndex::List::const_iterator it = indices.begin(), end = indices.end(); it != end; ++it) {
            m_imageSamplers[pass].push_back(ImageSampler(name, SG_SHADERSTAGE_FS, handle, *it));
        }
    }
    else if (pass->findVertexShaderSamplerRegisterIndex(name, indices)) {
        for (SamplerRegisterIndex::List::const_iterator it = indices.begin(), end = indices.end(); it != end; ++it) {
            m_imageSamplers[pass].push_back(ImageSampler(name, SG_SHADERSTAGE_VS, handle, *it));
        }
    }
}

bool
Effect::writeUniformBuffer(
    const RegisterIndex &registerIndex, const void *ptr, size_t size, GlobalUniform::Buffer &bufferPtr)
{
    bool result = true;
    if (registerIndex.m_type != nanoem_u32_t(-1)) {
        size_t bytesToWrite, bufferCapacity, count = registerIndex.m_count, offset = registerIndex.m_index;
        void *mutablePtr = nullptr;
        switch (static_cast<Fx9__Effect__Dx9ms__RegisterSet>(registerIndex.m_type)) {
        case FX9__EFFECT__DX9MS__REGISTER_SET__RS_FLOAT4: {
            bytesToWrite = glm::min(size, sizeof(bufferPtr.m_float4[0]) * count);
            bufferCapacity = bufferPtr.m_float4.size();
            mutablePtr = offset < bufferCapacity ? &bufferPtr.m_float4[offset] : nullptr;
            break;
        }
        case FX9__EFFECT__DX9MS__REGISTER_SET__RS_SAMPLER:
        default:
            bytesToWrite = bufferCapacity = 0;
            break;
        }
        if (mutablePtr && offset + count <= bufferCapacity) {
            memcpy(mutablePtr, ptr, bytesToWrite);
        }
        else {
            result = false;
        }
    }
    return result;
}

void
Effect::writeUniformBuffer(const String &name, const effect::Pass *passPtr, const void *ptr, size_t size)
{
    nanoem_parameter_assert(passPtr, "must not be nullptr");
    nanoem_parameter_assert(ptr, "must not be nullptr");
    RegisterIndex registerIndex;
    bool written = false;
    if (passPtr->findVertexPreshaderRegisterIndex(name, registerIndex)) {
        written = writeUniformBuffer(registerIndex, ptr, size, m_globalUniformPtr->m_preshaderVertexShaderBuffer);
        if (!written) {
            m_logger->log("Parameter \"%s\" in \"Effects/%s/%s/%s\" cannot be written due to size buffer flow\n",
                name.c_str(), nameConstString(), passPtr->technique()->nameConstString(), passPtr->nameConstString());
        }
    }
    if (passPtr->findPixelPreshaderRegisterIndex(name, registerIndex)) {
        written = writeUniformBuffer(registerIndex, ptr, size, m_globalUniformPtr->m_preshaderPixelShaderBuffer);
        if (!written) {
            m_logger->log("Parameter \"%s\" in \"Effects/%s/%s/%s\" cannot be written due to size buffer flow\n",
                name.c_str(), nameConstString(), passPtr->technique()->nameConstString(), passPtr->nameConstString());
        }
    }
    if (passPtr->findVertexShaderRegisterIndex(name, registerIndex)) {
        written = writeUniformBuffer(registerIndex, ptr, size, m_globalUniformPtr->m_vertexShaderBuffer);
        if (!written) {
            m_logger->log("Parameter \"%s\" in \"Effects/%s/%s/%s\" cannot be written due to size buffer flow\n",
                name.c_str(), nameConstString(), passPtr->technique()->nameConstString(), passPtr->nameConstString());
        }
    }
    if (passPtr->findPixelShaderRegisterIndex(name, registerIndex)) {
        written = writeUniformBuffer(registerIndex, ptr, size, m_globalUniformPtr->m_pixelShaderBuffer);
        if (!written) {
            m_logger->log("Parameter \"%s\" in \"Effects/%s/%s/%s\" cannot be written due to size buffer flow\n",
                name.c_str(), nameConstString(), passPtr->technique()->nameConstString(), passPtr->nameConstString());
        }
    }
    if (m_enablePassUniformBufferInspection && written) {
        const nanoem_u8_t *bytes = reinterpret_cast<const nanoem_u8_t *>(ptr);
        m_passUniformBuffer[passPtr->name()][name].assign(bytes, bytes + size);
    }
}

void
Effect::writeUniformBuffer(const String &name, const effect::Pass *passPtr, bool value)
{
    writeUniformBuffer(name, passPtr, Vector4(value));
}

void
Effect::writeUniformBuffer(const String &name, const effect::Pass *passPtr, int value)
{
    writeUniformBuffer(name, passPtr, Vector4(nanoem_f32_t(value)));
}

void
Effect::writeUniformBuffer(const String &name, const effect::Pass *passPtr, nanoem_f32_t value)
{
    writeUniformBuffer(name, passPtr, Vector4(value));
}

void
Effect::writeUniformBuffer(const String &name, const effect::Pass *passPtr, const Vector4 &value)
{
    writeUniformBuffer(name, passPtr, glm::value_ptr(value), sizeof(value));
}

void
Effect::writeUniformBuffer(const String &name, const effect::Pass *passPtr, const Matrix4x4 &value)
{
    writeUniformBuffer(name, passPtr, glm::value_ptr(value), sizeof(value));
}

void
Effect::writeUniformBuffer(const String &name, const effect::Pass *passPtr, const ByteArray *value)
{
    writeUniformBuffer(name, passPtr, value->data(), value->size());
}

void
Effect::writeUniformBuffer(const String &name, const effect::Pass *passPtr, const effect::NonSemanticParameter &value)
{
    writeUniformBuffer(name, passPtr, value.m_values.data(), value.m_values.size() * sizeof(value.m_values[0]));
}

void
Effect::setDefaultControlParameterValues(
    const String &name, const ControlObjectTarget &target, const effect::Pass *pass)
{
    if (Model::isLoadableExtension(URI::pathExtension(target.m_name))) {
        switch (target.m_type) {
        case kParameterTypeBool: {
            writeUniformBuffer(name, pass, false);
            break;
        }
        case kParameterTypeFloat: {
            writeUniformBuffer(name, pass, target.m_item.empty() ? 1.0f : 0.0f);
            break;
        }
        case kParameterTypeFloat4: {
            writeUniformBuffer(name, pass, Vector4(0, 0, 0, 1));
            break;
        }
        case kParameterTypeFloat4x4: {
            writeUniformBuffer(name, pass, Model::kInitialWorldMatrix);
            break;
        }
        default:
            break;
        }
    }
    else {
        switch (target.m_type) {
        case kParameterTypeBool: {
            writeUniformBuffer(name, pass, false);
            break;
        }
        case kParameterTypeFloat: {
            writeUniformBuffer(name, pass, target.m_item.empty() ? 10.0f : 0.0f);
            break;
        }
        case kParameterTypeFloat4: {
            writeUniformBuffer(name, pass, Vector4(0, 0, 0, 1));
            break;
        }
        case kParameterTypeFloat4x4: {
            writeUniformBuffer(name, pass, Accessory::kInitialWorldMatrix);
            break;
        }
        default:
            break;
        }
    }
}

void
Effect::setFoundImageSamplers(
    const SemanticUniformList &uniforms, const SemanticImageMap &images, const effect::Pass *passPtr)
{
    if (!uniforms.empty() && !images.empty()) {
        for (SemanticUniformList::const_iterator it = uniforms.begin(), end = uniforms.end(); it != end; ++it) {
            const String &name = *it;
            SemanticImageMap::const_iterator it2 = images.find(name);
            if (it2 != images.end()) {
                setImageUniform(name, passPtr, it2->second);
            }
        }
    }
}

void
Effect::setFoundImageSamplers(const SemanticUniformList &uniforms,
    const NamedRenderTargetColorImageContainerMap &containers, const effect::Pass *passPtr)
{
    if (!uniforms.empty() && !containers.empty()) {
        for (SemanticUniformList::const_iterator it = uniforms.begin(), end = uniforms.end(); it != end; ++it) {
            const String &name = *it;
            NamedRenderTargetColorImageContainerMap::const_iterator it2 = containers.find(name);
            if (it2 != containers.end()) {
                setImageUniform(name, passPtr, it2->second->colorImageHandle());
            }
        }
    }
}

void
Effect::setFoundImageSamplers(const SemanticUniformList &uniforms,
    const OffscreenRenderTargetImageContainerMap &containers, const effect::Pass *passPtr)
{
    if (!uniforms.empty() && !containers.empty()) {
        for (SemanticUniformList::const_iterator it = uniforms.begin(), end = uniforms.end(); it != end; ++it) {
            const String &name = *it;
            OffscreenRenderTargetImageContainerMap::const_iterator it2 = containers.find(name);
            if (it2 != containers.end()) {
                setImageUniform(name, passPtr, it2->second->colorImageHandle());
            }
        }
    }
}

void
Effect::setFoundImageSamplers(const effect::SemanticUniformList &uniforms, const AnimatedImageContainerMap &containers,
    const effect::Pass *passPtr)
{
    if (!uniforms.empty() && !containers.empty()) {
        RegisterIndex registerIndex;
        const GlobalUniform::Vector4List &fv4 = m_globalUniformPtr->m_pixelShaderBuffer.m_float4;
        for (SemanticUniformList::const_iterator it = uniforms.begin(), end = uniforms.end(); it != end; ++it) {
            const String &name = *it;
            AnimatedImageContainerMap::const_iterator it2 = containers.find(name);
            if (it2 != containers.end()) {
                AnimatedImageContainer *container = it2->second;
                nanoem_f32_t value = 0;
                if (passPtr->findPixelShaderRegisterIndex(container->seekVariableNameConstString(), registerIndex)) {
                    value = fv4[registerIndex.m_index].x;
                }
                else {
                    value = m_project->currentLocalFrameIndex() / nanoem_f32_t(m_project->baseFPS());
                }
                value = glm::max((value * container->speed()) - container->offset(), 0.0f);
                container->update(value);
                setImageUniform(name, passPtr, container->colorImage());
            }
        }
    }
}

void
Effect::setTextureValues(const SemanticImageMap &images, effect::Pass *passPtr)
{
    if (!images.empty() && !m_textureValueUniforms.empty()) {
        SG_PUSH_GROUP("Effect::setTextureValues");
        for (SemanticUniformList::const_iterator it = m_textureValueUniforms.begin(),
                                                 end = m_textureValueUniforms.end();
             it != end; ++it) {
            const String &name = *it;
            StringMap::const_iterator it2 = m_textureValueTargets.find(name);
            if (it2 != m_textureValueTargets.end()) {
                const String &target = it2->second;
                SemanticImageMap::const_iterator it3 = images.find(target);
                if (it3 != images.end()) {
                    sg_image image = it3->second;
                    StagingBufferMap::iterator it4 = m_imageStagingBuffers.find(target);
                    StagingBuffer *sb = nullptr;
                    if (it4 != m_imageStagingBuffers.end()) {
                        sb = &it4->second;
                    }
                    else {
                        Vector4UI16 size(1);
                        for (ImageResourceList::const_iterator it5 = m_imageResources.begin(),
                                                               end2 = m_imageResources.end();
                             it5 != end2; ++it5) {
                            if (it5->m_name == target) {
                                const sg_image_desc &desc = it5->m_desc;
                                size = Vector4UI16(desc.width, desc.height, desc.num_slices, 1);
                                break;
                            }
                        }
                        const nanoem_rsize_t imageDataSize = size.x * size.y * 4 * sizeof(nanoem_f32_t);
                        sb = &m_imageStagingBuffers[target];
                        sb->m_content.resize(imageDataSize);
                        sg_buffer_desc bd;
                        Inline::clearZeroMemory(bd);
                        bd.size = imageDataSize;
                        bd.usage = SG_USAGE_STREAM;
                        char label[Inline::kMarkerStringLength];
                        if (Inline::isDebugLabelEnabled()) {
                            StringUtils::format(label, sizeof(label), "Effects/%s/Images/%s/StagingBuffer",
                                nameConstString(), name.c_str());
                            bd.label = label;
                        }
                        sb->m_handle = sg::make_buffer(&bd);
                        sb->m_read = false;
                        sb->m_immutable = sb->m_read = true;
                        SG_LABEL_BUFFER(sb->m_handle, bd.label);
                        SG_PUSH_GROUPF("Effect::setTextureValues(name=%s, target=%s)", name.c_str(), target.c_str());
                        sg::read_image(image, sb->m_handle, sb->m_content.data(), sb->m_content.size());
                        SG_POP_GROUP();
                    }
                    writeUniformBuffer(name, passPtr, &sb->m_content);
                }
            }
        }
        SG_POP_GROUP();
    }
}

void
Effect::setTextureValues(const NamedRenderTargetColorImageContainerMap &containers, effect::Pass *passPtr)
{
    if (!containers.empty() && !m_textureValueUniforms.empty()) {
        SG_PUSH_GROUP("Effect::setTextureValues");
        for (SemanticUniformList::const_iterator it = m_textureValueUniforms.begin(),
                                                 end = m_textureValueUniforms.end();
             it != end; ++it) {
            const String &name = *it;
            StringMap::const_iterator it2 = m_textureValueTargets.find(name);
            if (it2 != m_textureValueTargets.end()) {
                const String &target = it2->second;
                NamedRenderTargetColorImageContainerMap::const_iterator it3 = containers.find(target);
                if (it3 != containers.end()) {
                    const RenderTargetColorImageContainer *container = it3->second;
                    StagingBufferMap::iterator it4 = m_imageStagingBuffers.find(target);
                    StagingBuffer *sb = nullptr;
                    if (it4 != m_imageStagingBuffers.end()) {
                        sb = &it4->second;
                    }
                    else {
                        const sg_image_desc &desc = container->colorImageDescription();
                        const Vector2UI16 size(desc.width, desc.height);
                        const nanoem_rsize_t imageDataSize = size.x * size.y * 4 * sizeof(nanoem_f32_t);
                        sb = &m_imageStagingBuffers[target];
                        sb->m_content.resize(imageDataSize);
                        sg_buffer_desc bd;
                        Inline::clearZeroMemory(bd);
                        bd.size = imageDataSize;
                        bd.usage = SG_USAGE_STREAM;
                        char label[Inline::kMarkerStringLength];
                        if (Inline::isDebugLabelEnabled()) {
                            StringUtils::format(label, sizeof(label), "Effects/%s/Images/%s/StagingBuffer",
                                nameConstString(), name.c_str());
                            bd.label = label;
                        }
                        sb->m_handle = sg::make_buffer(&bd);
                        sb->m_read = sb->m_immutable = false;
                        SG_LABEL_BUFFER(sb->m_handle, bd.label);
                    }
                    if (!sb->m_read) {
                        SG_PUSH_GROUPF("Effect::setTextureValues(name=%s, target=%s)", name.c_str(), target.c_str());
                        sg_image image = container->colorImageHandle();
                        sg::read_image(image, sb->m_handle, sb->m_content.data(), sb->m_content.size());
                        SG_POP_GROUP();
                        sb->m_read = true;
                    }
                    writeUniformBuffer(name, passPtr, &sb->m_content);
                }
            }
        }
        SG_POP_GROUP();
    }
}

void
Effect::setTextureValues(const OffscreenRenderTargetImageContainerMap &containers, effect::Pass *passPtr)
{
    if (!containers.empty() && !m_textureValueUniforms.empty()) {
        SG_PUSH_GROUP("Effect::setTextureValues");
        for (SemanticUniformList::const_iterator it = m_textureValueUniforms.begin(),
                                                 end = m_textureValueUniforms.end();
             it != end; ++it) {
            const String &name = *it;
            StringMap::const_iterator it2 = m_textureValueTargets.find(name);
            if (it2 != m_textureValueTargets.end()) {
                const String &target = it2->second;
                OffscreenRenderTargetImageContainerMap::const_iterator it3 = containers.find(target);
                if (it3 != containers.end()) {
                    const RenderTargetColorImageContainer *container = it3->second;
                    StagingBufferMap::iterator it4 = m_imageStagingBuffers.find(target);
                    StagingBuffer *sb = nullptr;
                    if (it4 != m_imageStagingBuffers.end()) {
                        sb = &it4->second;
                    }
                    else {
                        const sg_image_desc &desc = container->colorImageDescription();
                        const Vector2UI16 size(desc.width, desc.height);
                        const nanoem_rsize_t imageDataSize = size.x * size.y * 4 * sizeof(nanoem_f32_t);
                        sb = &m_imageStagingBuffers[target];
                        sb->m_content.resize(imageDataSize);
                        sg_buffer_desc bd;
                        Inline::clearZeroMemory(bd);
                        bd.size = imageDataSize;
                        bd.usage = SG_USAGE_STREAM;
                        char label[Inline::kMarkerStringLength];
                        if (Inline::isDebugLabelEnabled()) {
                            StringUtils::format(label, sizeof(label), "Effects/%s/Images/%s/StagingBuffer",
                                nameConstString(), name.c_str());
                            bd.label = label;
                        }
                        sb->m_handle = sg::make_buffer(&bd);
                        sb->m_read = sb->m_immutable = false;
                        SG_LABEL_BUFFER(sb->m_handle, bd.label);
                    }
                    if (!sb->m_read) {
                        SG_PUSH_GROUPF("Effect::setTextureValues(name=%s, target=%s)", name.c_str(), target.c_str());
                        sg_image image = container->colorImageHandle();
                        sg::read_image(image, sb->m_handle, sb->m_content.data(), sb->m_content.size());
                        SG_POP_GROUP();
                        sb->m_read = true;
                    }
                    writeUniformBuffer(name, passPtr, &sb->m_content);
                }
            }
        }
        SG_POP_GROUP();
    }
}

void
Effect::updatePassImageHandles(effect::Pass *pass, sg_bindings &bindings)
{
    nanoem_parameter_assert(pass, "must not be nullptr");
    sg_image *pixelShaderImages = bindings.fs_images, *vertexShaderImages = bindings.vs_images,
             fallbackImage = m_project->sharedFallbackImage();
    pixelShaderImages[0] = m_firstImageHandle;
    ImageSamplerMap::iterator it = m_imageSamplers.find(pass);
    if (it != m_imageSamplers.end()) {
        ImageSamplerList &samplers = it->second;
        for (ImageSamplerList::const_iterator it = samplers.begin(), end = samplers.end(); it != end; ++it) {
            const ImageSampler &sampler = *it;
            const nanoem_u8_t samplerOffset = sampler.m_offset;
            if (samplerOffset < SG_MAX_SHADERSTAGE_IMAGES) {
                switch (sampler.m_stage) {
                case SG_SHADERSTAGE_FS: {
                    pixelShaderImages[samplerOffset] = sampler.m_image;
                    break;
                }
                case SG_SHADERSTAGE_VS: {
                    vertexShaderImages[samplerOffset] = sampler.m_image;
                    break;
                }
                default:
                    break;
                }
            }
        }
        samplers.clear();
    }
    PrivateEffectUtils::fillFallbackImage(pixelShaderImages, pass->pixelShaderImageCount(), fallbackImage);
    PrivateEffectUtils::fillFallbackImage(vertexShaderImages, pass->vertexShaderImageCount(), fallbackImage);
}

void
Effect::updatePassUniformHandles(sg::PassBlock &pb)
{
    GlobalUniform::Buffer &pbuffer = m_globalUniformPtr->m_pixelShaderBuffer;
    GlobalUniform::Buffer &vbuffer = m_globalUniformPtr->m_vertexShaderBuffer;
    pbuffer.apply(pb);
    vbuffer.apply(pb);
    vbuffer.reset();
    pbuffer.reset();
}

void
Effect::handleMatrixSemantics(
    const TypedSemanticParameter &parameter, MatrixType matrixType, bool inversed, bool transposed)
{
    const String &name = parameter.m_name;
    if (m_needsBehaviorCompatibility) {
        transposed = transposed ? false : true;
    }
    if (parameter.m_type == kParameterTypeFloat4x4) {
        AnnotationMap::const_iterator it = parameter.m_annotations.findAnnotation(kObjectKeyLiteral);
        if (it == parameter.m_annotations.end() ||
            StringUtils::equals(it->second.m_string.c_str(), kObjectCameraValueLiteral)) {
            m_cameraMatrixUniforms.insert(
                tinystl::make_pair(name, MatrixUniform(name, matrixType, inversed, transposed)));
        }
        else if (StringUtils::equals(it->second.m_string.c_str(), kObjectLightValueLiteral)) {
            m_lightMatrixUniforms.insert(
                tinystl::make_pair(name, MatrixUniform(name, matrixType, inversed, transposed)));
        }
    }
    else {
        addInvalidParameterTypeError(kParameterTypeFloat4x4, parameter);
    }
}

void
Effect::addMissingParameterKeyError(const String &name, const TypedSemanticParameter &parameter)
{
    String message;
    StringUtils::format(message, "Required key \"%s\" of \"%s : %s\" is missing.", name.c_str(),
        parameter.m_name.c_str(), parameter.m_semantic.c_str());
    m_errorMessages.push_back(message);
}

void
Effect::addInvalidParameterValueError(const String &value, const TypedSemanticParameter &parameter)
{
    String message;
    StringUtils::format(message, "\"%s\" is invalid at \"%s: %s\".", value.c_str(), parameter.m_name.c_str(),
        parameter.m_semantic.c_str());
    m_errorMessages.push_back(message);
}

void
Effect::addInvalidParameterTypeError(ParameterType expected, const TypedSemanticParameter &parameter)
{
    String message;
    StringUtils::format(message, "Expected type \"%s\" but actual \"%s\" at \"%s: %s\".", toString(expected),
        toString(parameter.m_type), parameter.m_name.c_str(), parameter.m_semantic.c_str());
    m_errorMessages.push_back(message);
}

void
Effect::destroyAllRenderTargetNormalizers()
{
    SG_PUSH_GROUPF("Effect::destroyAllRenderTargetNormalizers(size=%d)", m_normalizers.size());
    for (RenderTargetNormalizerMap::iterator it = m_normalizers.begin(), end = m_normalizers.end(); it != end; ++it) {
        RenderTargetNormalizer *normalizer = it->second;
        normalizer->destroy();
        nanoem_delete(it->second);
    }
    m_normalizers.clear();
    m_hashes.clear();
    SG_POP_GROUP();
}

void
Effect::destroyAllOverridenImages()
{
    SG_PUSH_GROUPF("Effect::destroy(overridenImages=%d)", m_overridenImageHandles.size());
    for (OverridenImageHandleMap::iterator it = m_overridenImageHandles.begin(), end = m_overridenImageHandles.end();
         it != end; ++it) {
        removeImageLabel(it->second);
        sg::destroy_image(it->second);
    }
    m_overridenImageHandles.clear();
    SG_POP_GROUP();
}

void
Effect::destroyAllTechniques()
{
    SG_PUSH_GROUPF("Effect::destroyAllTechniques(size=%d)", m_allTechniques.size());
    for (TechniqueList::iterator it = m_allTechniques.begin(), end = m_allTechniques.end(); it != end; ++it) {
        Technique *technique = *it;
        technique->destroy();
        nanoem_delete(technique);
    }
    m_allTechniques.clear();
    SG_POP_GROUP();
}

void
Effect::destroyAllSemanticImages(SemanticImageMap &images)
{
    SG_PUSH_GROUPF("Effect::destroyAllSemanticImages(size=%d)", images.size());
    for (SemanticImageMap::iterator it = images.begin(), end = images.end(); it != end; ++it) {
        removeImageLabel(it->second);
        sg::destroy_image(it->second);
    }
    images.clear();
    SG_POP_GROUP();
}

void
Effect::destroyAllDrawableNamedRenderTargetColorImages()
{
    for (DrawableNamedRenderTargetColorImageContainerMap::iterator it = m_drawableNamedRenderTargetColorImages.begin(),
                                                                   end = m_drawableNamedRenderTargetColorImages.end();
         it != end; ++it) {
        destroyAllRenderTargetColorImages(it->second);
    }
    m_drawableNamedRenderTargetColorImages.clear();
}

void
Effect::destroyAllRenderTargetColorImages(NamedRenderTargetColorImageContainerMap &containers)
{
    SG_PUSH_GROUPF("Effect::destroyAllRenderTargetColorImages(size=%d)", containers.size());
    for (NamedRenderTargetColorImageContainerMap::iterator it = containers.begin(), end = containers.end(); it != end;
         ++it) {
        RenderTargetColorImageContainer *container = it->second;
        container->destroy(this);
        nanoem_delete(container);
    }
    containers.clear();
    SG_POP_GROUP();
}

void
Effect::destroyAllRenderTargetDepthStencilImages(RenderTargetDepthStencilImageContainerMap &containers)
{
    SG_PUSH_GROUPF("Effect::destroyAllRenderTargetDepthStencilImages(size=%d)", containers.size());
    for (RenderTargetDepthStencilImageContainerMap::iterator it = containers.begin(), end = containers.end(); it != end;
         ++it) {
        RenderTargetDepthStencilImageContainer *container = it->second;
        container->destroy(this);
        nanoem_delete(container);
    }
    containers.clear();
    SG_POP_GROUP();
}

void
Effect::destroyAllOffscreenRenderTargetImages(OffscreenRenderTargetImageContainerMap &containers)
{
    SG_PUSH_GROUPF("Effect::destroyAllOffscreenRenderTargetImages(size=%d)", containers.size());
    sg_image invalid = { SG_INVALID_ID };
    for (OffscreenRenderTargetImageContainerMap::iterator it = containers.begin(), end = containers.end(); it != end;
         ++it) {
        OffscreenRenderTargetImageContainer *container = it->second;
        container->destroy(this);
        nanoem_delete(container);
    }
    containers.clear();
    SG_POP_GROUP();
}

void
Effect::destroyAllAnimatedImages(AnimatedImageContainerMap &containers)
{
    SG_PUSH_GROUPF("Effect::destroyAllAnimatedImages(size=%d)", containers.size());
    for (AnimatedImageContainerMap::const_iterator it = containers.begin(), end = containers.end(); it != end; ++it) {
        AnimatedImageContainer *container = it->second;
        container->destroy();
        nanoem_delete(container);
    }
    containers.clear();
    SG_POP_GROUP();
}

void
Effect::destroyAllStagingBuffers(StagingBufferMap &buffers)
{
    SG_PUSH_GROUPF("Effect::destroyAllStagingBuffers(size=%d)", buffers.size());
    for (StagingBufferMap::const_iterator it = buffers.begin(), end = buffers.end(); it != end; ++it) {
        const StagingBuffer &sb = it->second;
        sg::destroy_buffer(sb.m_handle);
    }
    buffers.clear();
    SG_POP_GROUP();
}

void
Effect::setClearColor(const String &parameterName)
{
    VectorParameterUniformMap::const_iterator it = m_vectorParameterUniforms.find(parameterName);
    if (it != m_vectorParameterUniforms.end()) {
        m_clearColor = it->second.m_values.front();
    }
}

void
Effect::setClearDepth(const String &parameterName)
{
    FloatParameterUniformMap::const_iterator it = m_floatParameterUniforms.find(parameterName);
    if (it != m_floatParameterUniforms.end()) {
        m_clearDepth = it->second.m_values.front().x;
    }
}

void
Effect::overridePipelineDescription(sg_pipeline_desc &pd, ScriptClassType classType) const NANOEM_DECL_NOEXCEPT
{
    const sg_pass currentRenderPass = m_project->currentRenderPass();
    sg_pass activeRenderPass;
    if (m_project->isOffscreenRenderPassActive()) {
        activeRenderPass = m_project->currentOffscreenRenderPass();
    }
    /* for script external */
    else if (classType != kScriptClassTypeScene && sg::is_valid(currentRenderPass) &&
        m_currentNamedPrimaryRenderTargetColorImageDescription.first.empty()) {
        activeRenderPass = currentRenderPass;
    }
    else {
        activeRenderPass = { SG_INVALID_ID };
    }
    if (sg::is_valid(activeRenderPass)) {
        const int numSampleCount = m_currentRenderTargetPixelFormat.numSamples();
        const PixelFormat format(m_project->findRenderPassPixelFormat(activeRenderPass, numSampleCount));
        const int numColorAttachments = format.numColorAttachments();
        if (numColorAttachments != m_currentRenderTargetPixelFormat.numColorAttachments()) {
            m_project->overrideOffscreenRenderPass(
                m_currentRenderTargetPassDescription, m_currentRenderTargetPixelFormat);
            const int numColorAttachments = m_currentRenderTargetPixelFormat.numColorAttachments();
            pd.color_count = numColorAttachments;
            for (int i = 0; i < numColorAttachments; i++) {
                pd.colors[i].pixel_format = m_currentRenderTargetPixelFormat.colorPixelFormat(i);
            }
            pd.depth.pixel_format = m_currentRenderTargetPixelFormat.depthPixelFormat();
            pd.sample_count = numSampleCount;
        }
        else {
            pd.color_count = numColorAttachments;
            for (int i = 0; i < numColorAttachments; i++) {
                pd.colors[i].pixel_format = format.colorPixelFormat(i);
            }
            pd.depth.pixel_format = format.depthPixelFormat();
            pd.sample_count = format.numSamples();
        }
    }
    else {
        const int numColorAttachments = m_currentRenderTargetPixelFormat.numColorAttachments();
        pd.color_count = numColorAttachments;
        for (int i = 0; i < numColorAttachments; i++) {
            pd.colors[i].pixel_format = m_currentRenderTargetPixelFormat.colorPixelFormat(i);
        }
        pd.depth.pixel_format = m_currentRenderTargetPixelFormat.depthPixelFormat();
        pd.sample_count = m_currentRenderTargetPixelFormat.numSamples();
    }
    SG_INSERT_MARKERF(
        "Effect::overridePipelineDescription(handle=%d, active=%s, colorFormat=%d, depthFormat=%d, sampleCount=%d, "
        "numColorAttachments=%d)",
        activeRenderPass.id, sg::is_valid(activeRenderPass) ? "true" : "false", pd.colors[0].pixel_format,
        pd.depth.pixel_format, pd.sample_count, pd.color_count);
}

tinystl::pair<const Model *, const Accessory *>
Effect::findOffscreenOwnerObject(const IDrawable *ownerDrawable, const Project *project) const NANOEM_DECL_NOEXCEPT
{
    tinystl::pair<const Model *, const Accessory *> pair(nullptr, nullptr);
    const IEffect *ownerEffect = ownerDrawable->activeEffect();
    const Project::ModelList *models = project->allModels();
    for (Project::ModelList::const_iterator it = models->begin(), end = models->end(); it != end; ++it) {
        Model *model = *it;
        if (model != ownerDrawable && model->activeEffect() == ownerEffect) {
            pair.first = model;
            break;
        }
    }
    const Project::AccessoryList *accessories = project->allAccessories();
    for (Project::AccessoryList::const_iterator it = accessories->begin(), end = accessories->end(); it != end; ++it) {
        Accessory *accessory = *it;
        if (accessory != ownerDrawable && accessory->activeEffect() == ownerEffect) {
            pair.second = accessory;
            break;
        }
    }
    return pair;
}

void
Effect::decodeImageData(const ByteArray &bytes, const ImageResourceParameter &parameter, Error &error)
{
    sg_image_desc desc;
    nanoem_u8_t *decodedImagePtr = nullptr;
    Inline::clearZeroMemory(desc);
    const URI &fileURI = parameter.m_fileURI;
    if (ImageLoader::decodeImageWithSTB(
            bytes.data(), bytes.size(), parameter.m_filename.c_str(), desc, &decodedImagePtr, error)) {
        desc.mag_filter = desc.min_filter = SG_FILTER_LINEAR;
        const sg_range &data = desc.data.subimage[0][0];
        ImageResourceParameter newParameter(parameter);
        newParameter.m_desc.width = desc.width;
        newParameter.m_desc.height = desc.height;
        newParameter.m_desc.pixel_format = desc.pixel_format;
        createImageResource(data.ptr, data.size, newParameter);
        ImageLoader::releaseDecodedImageWithSTB(&decodedImagePtr);
    }
    else if (StringUtils::equalsIgnoreCase(fileURI.pathExtension().c_str(), "dds")) {
        MemoryReader reader(&bytes);
        nanoem_u32_t signature;
        FileUtils::readTyped(&reader, signature, error);
        if (signature == image::DDS::kSignature) {
            reader.seek(0, ISeekable::kSeekTypeBegin, error);
            error = Error();
            if (image::DDS *dds = ImageLoader::decodeDDS(&reader, error)) {
                dds->setImageDescription(desc);
                sg_image image = sg::make_image(&desc);
                nanoem_delete(dds);
                nanoem_assert(sg::query_image_state(image) == SG_RESOURCESTATE_VALID, "image must be valid");
                if (sg::is_valid(image)) {
                    registerImageResource(image, parameter);
                }
            }
        }
    }
    else if (StringUtils::equalsIgnoreCase(fileURI.pathExtension().c_str(), "pfm")) {
        if (image::PFM *pfm = ImageLoader::decodePFM(bytes, error)) {
            pfm->setImageDescription(desc);
            sg_image image = sg::make_image(&desc);
            nanoem_delete(pfm);
            nanoem_assert(sg::query_image_state(image) == SG_RESOURCESTATE_VALID, "image must be valid");
            if (sg::is_valid(image)) {
                registerImageResource(image, parameter);
            }
        }
    }
    else {
        error = Error();
        createImageResource(bytes.data(), bytes.size(), parameter);
    }
}

void
Effect::setNormalizedColorImageContainer(
    const String &name, int numMipLevels, effect::RenderTargetColorImageContainer *container)
{
    ImageDescriptionMap::const_iterator it = m_imageDescriptions.find(name);
    if (it != m_imageDescriptions.end()) {
        const sg_backend backend = sg::query_backend();
        if (backend == SG_BACKEND_GLCORE33 || backend == SG_BACKEND_GLES3) {
            sg_image_desc desc(it->second);
            if (numMipLevels == 1) {
                effect::RenderState::normalizeMinFilter(desc.min_filter);
            }
            container->setColorImageDescription(desc);
        }
        else {
            container->setColorImageDescription(it->second);
        }
    }
}

void
Effect::resetPassDescription()
{
    sg_image_desc &colorImageDesc = m_currentNamedPrimaryRenderTargetColorImageDescription.second;
    Inline::clearZeroMemory(m_currentRenderTargetPassDescription);
    Inline::clearZeroMemory(m_currentNamedDepthStencilImageDescription.second);
    Inline::clearZeroMemory(colorImageDesc);
    m_currentRenderTargetPixelFormat.reset(m_project->sampleCount());
    if (!m_project->getOriginOffscreenRenderPassColorImageDescription(
            m_currentRenderTargetPassDescription, colorImageDesc)) {
        m_project->getViewportRenderPassColorImageDescription(m_currentRenderTargetPassDescription, colorImageDesc);
    }
    if (!m_project->getOriginOffscreenRenderPassDepthImageDescription(
            m_currentRenderTargetPassDescription, colorImageDesc)) {
        m_project->getViewportRenderPassDepthImageDescription(m_currentRenderTargetPassDescription, colorImageDesc);
    }
    m_firstImageHandle = m_project->sharedFallbackImage();
}

sg_pass
Effect::resetRenderPass(
    const IDrawable *drawable, const char *label, Pass *passPtr, RenderTargetNormalizer *&normalizer)
{
    const sg_image_desc &colorImageDesc = m_currentNamedPrimaryRenderTargetColorImageDescription.second;
    const sg_pixel_format primaryColorFormat = colorImageDesc.pixel_format;
    const int numSamples = glm::max(colorImageDesc.sample_count, 1);
    sg_pixel_format normalizedColorFormat = primaryColorFormat;
    const bool validated =
                   validateAllPassColorAttachments(drawable, primaryColorFormat, numSamples, normalizedColorFormat),
               contained = containsPassOutputImageSampler(passPtr),
               normalizeRenderTarget = !sg::query_features().mrt_independent_blend_state && (!validated || contained);
    if (normalizeRenderTarget) {
        bx::HashMurmur2A hash;
        hash.begin();
        hash.add(m_currentRenderTargetPassDescription);
        nanoem_u32_t currentPassDescriptionKey = hash.end();
        HashMap::const_iterator it0 = m_hashes.find(currentPassDescriptionKey);
        nanoem_u32_t originPassDescriptionKey = it0 != m_hashes.end() ? it0->second : 0;
        RenderTargetNormalizerMap::const_iterator it = m_normalizers.find(originPassDescriptionKey);
        if (it != m_normalizers.end()) {
            normalizer = it->second;
            normalizer->normalize(drawable, passPtr, m_currentRenderTargetPixelFormat, normalizedColorFormat,
                m_currentRenderTargetPassDescription);
            m_currentNamedPrimaryRenderTargetColorImageDescription.second.pixel_format =
                m_currentRenderTargetPixelFormat.colorPixelFormat(0);
        }
        else {
            normalizer = nanoem_new(RenderTargetNormalizer(this));
            PixelFormat normalizedPixelFormat(m_currentRenderTargetPixelFormat);
            normalizedPixelFormat.setColorPixelFormat(normalizedColorFormat, 0);
            normalizer->setNormalizedColorImagePixelFormat(normalizedPixelFormat);
            m_normalizers.insert(tinystl::make_pair(currentPassDescriptionKey, normalizer));
            normalizer->normalize(drawable, passPtr, m_currentRenderTargetPixelFormat, normalizedColorFormat,
                m_currentRenderTargetPassDescription);
            m_currentNamedPrimaryRenderTargetColorImageDescription.second.pixel_format = normalizedColorFormat;
            m_currentRenderTargetPixelFormat.setColorPixelFormat(normalizedColorFormat, 0);
            hash.begin();
            hash.add(m_currentRenderTargetPassDescription);
            nanoem_u32_t newPassDescriptionKey = hash.end();
            m_hashes.insert(tinystl::make_pair(newPassDescriptionKey, currentPassDescriptionKey));
            m_hashes.insert(tinystl::make_pair(currentPassDescriptionKey, currentPassDescriptionKey));
        }
    }
    sg_pass pass = { SG_INVALID_ID };
    if (Inline::isDebugLabelEnabled()) {
        sg_pass_desc labeledCurrentPassDescription(m_currentRenderTargetPassDescription);
        labeledCurrentPassDescription.label = label;
        pass = m_project->registerRenderPass(labeledCurrentPassDescription, m_currentRenderTargetPixelFormat);
    }
    else {
        pass = m_project->registerRenderPass(m_currentRenderTargetPassDescription, m_currentRenderTargetPixelFormat);
    }
    SG_INSERT_MARKERF(
        "Effect::resetRenderPass(id=%d, label=%s, normalize=%s, validate=%s, contain=%s, format=%d, RTN=0x%p)", pass.id,
        label, normalizeRenderTarget ? "true" : "false", validated ? "true" : "false", contained ? "true" : "false",
        primaryColorFormat, normalizer);
    return pass;
}

bool
Effect::validateAllPassColorAttachments(const IDrawable *drawable, const sg_pixel_format primaryColorFormat,
    int numSamples, sg_pixel_format &normalizedColorFormat) const NANOEM_DECL_NOEXCEPT
{
    bool result = true;
    for (nanoem_rsize_t i = 1; result && i < SG_MAX_COLOR_ATTACHMENTS; i++) {
        sg_image outputColorImage = m_currentRenderTargetPassDescription.color_attachments[i].image;
        if (const RenderTargetColorImageContainer *containerPtr =
                searchRenderTargetColorImageContainer(drawable, outputColorImage)) {
            const sg_image_desc &desc = containerPtr->colorImageDescription();
            sg_pixel_format outputColorImageFormat = desc.pixel_format;
            if (primaryColorFormat < outputColorImageFormat) {
                normalizedColorFormat = outputColorImageFormat;
                result = false;
            }
            else if (primaryColorFormat > outputColorImageFormat) {
                normalizedColorFormat = primaryColorFormat;
                result = false;
            }
            else if (numSamples != desc.sample_count) {
                result = false;
            }
        }
    }
    return result;
}

bool
Effect::containsPassOutputImageSampler(const Pass *passPtr) const NANOEM_DECL_NOEXCEPT
{
    bool result = false;
    if (passPtr) {
        ImageSamplerMap::const_iterator it = m_imageSamplers.find(passPtr);
        if (it != m_imageSamplers.end()) {
            const ImageSamplerList &images = it->second;
            for (nanoem_rsize_t i = 0; !result && i < SG_MAX_COLOR_ATTACHMENTS; i++) {
                sg_image outputColorImage = m_currentRenderTargetPassDescription.color_attachments[i].image;
                if (sg::is_valid(outputColorImage)) {
                    for (ImageSamplerList::const_iterator it = images.begin(), end = images.end(); it != end; ++it) {
                        const ImageSampler &sampler = *it;
                        if (sampler.m_image.id == outputColorImage.id) {
                            result = true;
                            break;
                        }
                    }
                }
            }
        }
    }
    return result;
}

void
Effect::internalDrawRenderPass(const IDrawable *drawable, effect::Pass *pass, sg::PassBlock::IDrawQueue *drawQueue,
    int offset, int numIndices, sg_pipeline_desc &pd, sg_bindings &bindings, ScriptClassType classType)
{
    nanoem_parameter_assert(pass, "must not be nullptr");
    nanoem_parameter_assert(
        classType == kScriptClassTypeScene || classType == kScriptClassTypeObject, "must be object or scene");
    sg_pass_action pa;
    Inline::clearZeroMemory(pa);
    for (size_t i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
        pa.colors[i].action = SG_ACTION_LOAD;
    }
    pa.depth.action = pa.stencil.action = SG_ACTION_LOAD;
    const sg_pass currentRenderPass = m_project->currentRenderPass();
    sg_pass handle = currentRenderPass;
    RenderTargetNormalizer *renderTargetNormalizer = nullptr;
    RenderPassScope *renderPassScope = pass->technique()->currentRenderPassScope();
    bool isOffscreenRenderPass = m_project->isOffscreenRenderPassActive();
    if (isOffscreenRenderPass || classType == kScriptClassTypeScene ||
        (classType == kScriptClassTypeObject &&
            !m_currentNamedPrimaryRenderTargetColorImageDescription.first.empty())) {
        char nameBuffer[Inline::kMarkerStringLength];
        StringUtils::format(nameBuffer, sizeof(nameBuffer), "Effects/%s/%s/%s/Draw%sPass", nameConstString(),
            pass->technique()->nameConstString(), pass->nameConstString(),
            classType == kScriptClassTypeScene ? "Scene" : "Geometry");
        handle = resetRenderPass(drawable, nameBuffer, pass, renderTargetNormalizer);
        if (renderTargetNormalizer) {
            renderPassScope->reset(renderTargetNormalizer);
        }
        m_project->setRenderPassName(handle, nameBuffer);
    }
    SG_INSERT_MARKERF("Effect::internalDrawRenderPass(id=%d, name=%s, RTN=0x%p)", handle.id,
        m_project->findRenderPassName(handle), renderTargetNormalizer);
    renderPassScope->modifyPipelineDescription(pd);
    {
        sg::PassBlock pb(drawQueue, m_project->beginRenderPass(handle), pa);
        if (pd.colors[0].write_mask == _SG_COLORMASK_DEFAULT) {
#if defined(NANOEM_ENABLE_BLENDOP_MINMAX)
            pd.colors[0].write_mask = SG_COLORMASK_RGBA;
#else
            pd.blend.color_write_mask = !sg::is_valid(handle) && !m_project->isOffscreenRenderPassActive()
                ? SG_COLORMASK_RGB
                : SG_COLORMASK_RGBA;
#endif
        }
        sg_pipeline pipeline = pass->registerPipelineSet(pd);
        /* call animation texture sampler here to fetch "SeekVariable" parameter */
        setFoundImageSamplers(m_animatedTextureUniforms, m_animatedTextureImages, pass);
        updatePassImageHandles(pass, bindings);
        renderPassScope->apply(bindings, drawable, pass, pipeline, pb);
        updatePassUniformHandles(pb);
        pb.draw(offset, numIndices);
    }
    if (classType == kScriptClassTypeScene && !isOffscreenRenderPass) {
        renderPassScope->reset(nullptr);
    }
}

void
Effect::generateRenderTargetMipmapImagesChain(const IDrawable *drawable, const String &colorContainerName,
    const String &depthContainerName, const sg_image_desc &imageDescription)
{
    if (imageDescription.num_mipmaps > 1) {
        const NamedRenderTargetColorImageContainerMap *containers =
            findNamedRenderTargetColorImageContainerMap(drawable);
        NamedRenderTargetColorImageContainerMap::const_iterator it = containers->find(colorContainerName);
        RenderTargetDepthStencilImageContainerMap::iterator it2 =
            m_renderTargetDepthStencilImages.find(depthContainerName);
        if (it != containers->end() && it2 != m_renderTargetDepthStencilImages.end()) {
            SG_PUSH_GROUPF("Effect::generateRenderTargetMipmapImagesChain(numImages=%d)", imageDescription.num_mipmaps);
            RenderTargetColorImageContainer *colorImageContainer = it->second;
            RenderTargetDepthStencilImageContainer *depthStencilImageContainer = it2->second;
            PixelFormat format;
            format.setColorPixelFormat(imageDescription.pixel_format, 0);
            format.setDepthPixelFormat(depthStencilImageContainer->depthStencilImageDescription().pixel_format);
            format.setNumSamples(imageDescription.sample_count);
            RenderTargetMipmapGenerator *generator = colorImageContainer->mipmapGenerator();
            if (!generator) {
                generator = nanoem_new(
                    RenderTargetMipmapGenerator(this, colorImageContainer->nameConstString(), imageDescription));
                colorImageContainer->setMipmapGenerator(generator);
            }
            char label[Inline::kMarkerStringLength];
            const char *name = m_currentNamedPrimaryRenderTargetColorImageDescription.first.c_str();
            StringUtils::format(label, sizeof(label), "Effects/%s/%s", nameConstString(), name);
            generator->blitSourceImage(this, colorImageContainer->colorImageHandle(), format, label);
            generator->generateAllMipmapImages(this, format, colorImageContainer, depthStencilImageContainer, label);
            SG_POP_GROUP();
        }
    }
}

bool
Effect::validateTechnique(const String &passType) const NANOEM_DECL_NOEXCEPT
{
    return isEnabled() && !(passType == kPassTypeZplot && m_scriptClass == kScriptClassTypeScene);
}

bool
Effect::findParameterUniform(const String &name, Vector4 &value) const NANOEM_DECL_NOEXCEPT
{
    BoolParameterUniformMap::const_iterator it = m_boolParameterUniforms.find(name);
    bool found = it != m_boolParameterUniforms.end();
    if (found) {
        value = it->second.m_values.front();
    }
    else {
        IntParameterUniformMap::const_iterator it2 = m_intParameterUniforms.find(name);
        found = it2 != m_intParameterUniforms.end();
        if (found) {
            value = it2->second.m_values.front();
        }
        else {
            FloatParameterUniformMap::const_iterator it3 = m_floatParameterUniforms.find(name);
            found = it3 != m_floatParameterUniforms.end();
            if (found) {
                value = it3->second.m_values.front();
            }
            else {
                ControlObjectTargetMap::const_iterator it4 = m_controlObjectTargets.find(name);
                found = it4 != m_controlObjectTargets.end();
                if (found) {
                    value = it4->second.m_value;
                }
            }
        }
    }
    return found;
}

void
Effect::setParameterUniform(const String &name, const Vector4 &value)
{
    BoolParameterUniformMap::iterator it = m_boolParameterUniforms.find(name);
    if (it != m_boolParameterUniforms.end()) {
        it->second.m_values.front() = value;
    }
    else {
        IntParameterUniformMap::iterator it2 = m_intParameterUniforms.find(name);
        if (it2 != m_intParameterUniforms.end()) {
            it2->second.m_values.front() = value;
        }
        else {
            FloatParameterUniformMap::iterator it3 = m_floatParameterUniforms.find(name);
            if (it3 != m_floatParameterUniforms.end()) {
                it3->second.m_values.front() = value;
            }
            else {
                ControlObjectTargetMap::iterator it4 = m_controlObjectTargets.find(name);
                if (it4 != m_controlObjectTargets.end()) {
                    it4->second.m_value = value;
                }
            }
        }
    }
}

void
Effect::pushLoopCounter(const String &name, size_t scriptIndex, LoopCounter::Stack &counterStack)
{
    if (!name.empty()) {
        Vector4 value;
        if (findParameterUniform(name, value)) {
            const LoopCounter counter(name, size_t(value.x), scriptIndex);
            counterStack.push_back(counter);
            SG_PUSH_GROUPF("Effect::pushLoopCounter(name=%s)", name.c_str());
        }
        else {
            m_logger->log("LoopByCount \"%s\" at %d in \"%s\" cannot found parameter", name.c_str(), scriptIndex,
                nameConstString());
        }
    }
    else {
        m_logger->log("LoopByCount at %d in \"%s\" specified empty parameter", scriptIndex, nameConstString());
    }
}

void
Effect::handleLoopGetIndex(const String &name, const LoopCounter::Stack &counterStack)
{
    if (!counterStack.empty() && !name.empty()) {
        setParameterUniform(name, Vector4(counterStack.back().m_offset));
    }
    else {
        m_logger->log("LoopGetIndex \"%s\" in \"%s\" cannot execute due to underflow", name.c_str(), nameConstString());
    }
}

void
Effect::popLoopCounter(LoopCounter::Stack &counterStack, size_t &scriptIndex)
{
    if (!counterStack.empty()) {
        LoopCounter &counter = counterStack.back();
        if (++counter.m_offset < counter.m_last) {
            scriptIndex = counter.m_gotoScriptIndex;
        }
        else {
            counterStack.pop_back();
            SG_POP_GROUP();
        }
    }
    else {
        m_logger->log("LoopEnd at %d in \"%s\" cannot execute due to underflow", scriptIndex, nameConstString());
    }
}

void
Effect::setRenderTargetColorImageDescription(const IDrawable *drawable, size_t renderTargetIndex, const String &value)
{
    sg_image_desc &destColorImageDescription = m_currentNamedPrimaryRenderTargetColorImageDescription.second;
    if (!value.empty()) {
        SG_PUSH_GROUPF(
            "Effect::setRenderTargetColorImageDescription(index=%d, name=%s)", renderTargetIndex, value.c_str());
        const NamedRenderTargetColorImageContainerMap *containers =
            findNamedRenderTargetColorImageContainerMap(drawable);
        NamedRenderTargetColorImageContainerMap::const_iterator it = containers->find(value);
        if (it != containers->end()) {
            const RenderTargetColorImageContainer *container = it->second;
            const sg_image_desc &sourceColorImageDescription = container->colorImageDescription();
            m_currentRenderTargetPassDescription.color_attachments[renderTargetIndex].image =
                container->colorImageHandle();
            m_currentRenderTargetPixelFormat.setColorPixelFormat(
                sourceColorImageDescription.pixel_format, renderTargetIndex);
            m_currentRenderTargetPixelFormat.setNumSamples(sourceColorImageDescription.sample_count);
            if (renderTargetIndex == 0) {
                const String &name = m_currentNamedDepthStencilImageDescription.first;
                const sg_image_desc &dd = m_currentNamedDepthStencilImageDescription.second;
                destColorImageDescription = sourceColorImageDescription;
                m_currentNamedPrimaryRenderTargetColorImageDescription.first = value;
                if (!name.empty() &&
                    (sourceColorImageDescription.width != dd.width ||
                        sourceColorImageDescription.height != dd.height)) {
                    setRenderTargetDepthStencilImageDescription(name);
                }
            }
        }
        else {
            m_logger->log("Specified render color target \"%s\" at %d in \"%s\" cannot be found", value.c_str(),
                renderTargetIndex, nameConstString());
        }
        SG_POP_GROUP();
    }
    else {
        SG_PUSH_GROUPF("Effect::setRenderTargetColorImageDescription(index=%d, name=(null))", renderTargetIndex);
        m_currentRenderTargetPassDescription.color_attachments[renderTargetIndex].image = { SG_INVALID_ID };
        if (renderTargetIndex == 0) {
            Inline::clearZeroMemory(destColorImageDescription);
            int numSamples = destColorImageDescription.sample_count;
            if (!m_project->getOriginOffscreenRenderPassColorImageDescription(
                    m_currentRenderTargetPassDescription, destColorImageDescription) &&
                !m_project->getScriptExternalRenderPassColorImageDescription(
                    m_currentRenderTargetPassDescription, destColorImageDescription)) {
                m_project->getViewportRenderPassColorImageDescription(
                    m_currentRenderTargetPassDescription, destColorImageDescription);
                numSamples = m_project->sampleCount();
            }
            m_currentRenderTargetPixelFormat.setColorPixelFormat(destColorImageDescription.pixel_format, 0);
            m_currentRenderTargetPixelFormat.setNumSamples(numSamples);
        }
        SG_POP_GROUP();
    }
    m_currentRenderTargetPixelFormat.setNumColorAttachemnts(
        Project::countColorAttachments(m_currentRenderTargetPassDescription));
}

void
Effect::setRenderTargetDepthStencilImageDescription(const String &value)
{
    if (!value.empty()) {
        SG_PUSH_GROUPF("Effect::setRenderTargetDepthStencilImageDescription(name=%s)", value.c_str());
        RenderTargetDepthStencilImageContainerMap::iterator it = m_renderTargetDepthStencilImages.find(value);
        if (it != m_renderTargetDepthStencilImages.end()) {
            const sg_image_desc &colorImageDesc = m_currentNamedPrimaryRenderTargetColorImageDescription.second;
            RenderTargetDepthStencilImageContainer *container = it->second;
            sg_image_desc desc(container->depthStencilImageDescription());
            int width = colorImageDesc.width, height = colorImageDesc.height, sampleCount = colorImageDesc.sample_count;
            if (width > 0 && height > 0 && sampleCount > 0 &&
                (desc.width != width || desc.height != height || desc.sample_count != sampleCount)) {
                desc.width = width;
                desc.height = height;
                desc.sample_count = sampleCount;
            }
            m_currentRenderTargetPassDescription.depth_stencil_attachment.image = container->findImage(this, desc);
            m_currentNamedDepthStencilImageDescription = tinystl::make_pair(value, desc);
        }
        else {
            m_logger->log("Specified render depth stencil target \"%s\" cannot be found", value.c_str());
        }
        SG_POP_GROUP();
    }
    else {
        SG_PUSH_GROUP("Effect::setRenderTargetDepthStencilImageDescription(name=(null))");
        sg_image_desc &colorImageDesc = m_currentNamedPrimaryRenderTargetColorImageDescription.second;
        if (!m_project->getOriginOffscreenRenderPassDepthImageDescription(
                m_currentRenderTargetPassDescription, colorImageDesc)) {
            if (m_scriptOrder == kScriptOrderTypePostProcess ||
                !m_project->getCurrentRenderPassDepthImageDescription(
                    m_currentRenderTargetPassDescription, colorImageDesc)) {
                m_project->getViewportRenderPassDepthImageDescription(
                    m_currentRenderTargetPassDescription, colorImageDesc);
            }
        }
        m_currentNamedDepthStencilImageDescription.first = String();
        m_currentNamedDepthStencilImageDescription.second.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
        SG_POP_GROUP();
    }
}

sg_pass
Effect::resetRenderPass(const IDrawable *drawable)
{
    RenderTargetNormalizer *normalizer = nullptr;
    return resetRenderPass(drawable, nullptr, nullptr, normalizer);
}

void
Effect::drawSceneRenderPass(const IDrawable *drawable, Pass *pass, sg_pipeline_desc &pd, sg_bindings &bindings)
{
    internalDrawRenderPass(
        drawable, pass, m_project->sharedSerialDrawQueue(), 0, 4, pd, bindings, kScriptClassTypeScene);
    generateRenderTargetMipmapImagesChain(drawable, m_currentNamedPrimaryRenderTargetColorImageDescription.first,
        m_currentNamedDepthStencilImageDescription.first,
        m_currentNamedPrimaryRenderTargetColorImageDescription.second);
}

void
Effect::drawGeometryRenderPass(
    const IDrawable *drawable, Pass *pass, int offset, int numIndices, sg_pipeline_desc &pd, sg_bindings &bindings)
{
    internalDrawRenderPass(
        drawable, pass, m_project->sharedBatchDrawQueue(), offset, numIndices, pd, bindings, kScriptClassTypeObject);
}

void
Effect::clearRenderPass(
    const IDrawable *drawable, const char *name, const String &target, RenderPassScope *renderPassScope)
{
    sg_pass_action pa;
    Inline::clearZeroMemory(pa);
    /* supply depth stencil render target with MSAA to prevent validation error */
    if (m_currentNamedPrimaryRenderTargetColorImageDescription.second.sample_count >
        m_currentNamedDepthStencilImageDescription.second.sample_count) {
        const String passName(name);
        sg_image_desc desc(m_currentNamedPrimaryRenderTargetColorImageDescription.second);
        desc.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
        desc.render_target = true;
        RenderTargetDepthStencilImageContainerMap::iterator it = m_renderTargetDepthStencilImages.find(passName);
        if (it != m_renderTargetDepthStencilImages.end()) {
            RenderTargetDepthStencilImageContainer *container = it->second;
            m_currentRenderTargetPassDescription.depth_stencil_attachment.image = container->findImage(this, desc);
            container->setImageDescription(desc);
        }
        else {
            RenderTargetDepthStencilImageContainer *newContainer =
                nanoem_new(RenderTargetDepthStencilImageContainer(name));
            newContainer->setImageDescription(desc);
            m_currentRenderTargetPassDescription.depth_stencil_attachment.image = newContainer->findImage(this, desc);
            m_renderTargetDepthStencilImages.insert(tinystl::make_pair(passName, newContainer));
        }
        m_currentNamedDepthStencilImageDescription.second = desc;
    }
    if (StringUtils::equals(target.c_str(), "Color")) {
        const Vector4 clearColor(m_clearColor);
        for (size_t i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
            sg_color_attachment_action &action = pa.colors[i];
            action.action = SG_ACTION_CLEAR;
            memcpy(&action.value, glm::value_ptr(clearColor), sizeof(action.value));
        }
    }
    else if (StringUtils::equals(target.c_str(), "Depth")) {
        pa.depth.action = pa.stencil.action = SG_ACTION_CLEAR;
        pa.depth.value = m_clearDepth;
    }
    RenderTargetNormalizer *renderTargetNormalizer = nullptr;
    sg_pass pass = resetRenderPass(drawable, name, nullptr, renderTargetNormalizer);
    m_project->setRenderPassName(pass, name);
    if (renderTargetNormalizer) {
        renderPassScope->reset(renderTargetNormalizer);
        m_project->clearRenderPass(
            m_project->sharedSerialDrawQueue(), pass, pa, renderTargetNormalizer->normalizedColorImagePixelFormat());
    }
    else {
        m_project->clearRenderPass(m_project->sharedSerialDrawQueue(), pass, pa, m_currentRenderTargetPixelFormat);
    }
}

void
Effect::attachEffectIncludePathSet(FileEntityMap &allAttachments) const
{
    const StringList &includePaths = allIncludePaths();
    const URI &fileURI = this->fileURI();
    const String &effectBasePath = URI::stringByDeletingLastPathComponent(filename());
    for (StringList::const_iterator it = includePaths.begin(), end = includePaths.end(); it != end; ++it) {
        const String &includedFilePath = *it,
                     &canonicalizedIncludedFilePath = FileUtils::canonicalizePath(effectBasePath, includedFilePath);
        const URI &resolvedIncludedFileURI = Project::resolveArchiveURI(fileURI, includedFilePath);
        allAttachments.insert(tinystl::make_pair(canonicalizedIncludedFilePath, resolvedIncludedFileURI));
    }
}

void
Effect::attachEffectImageResource(FileEntityMap &allAttachments) const
{
    const Effect::ImageResourceList &allTextureResources = this->allImageResources();
    for (Effect::ImageResourceList::const_iterator it = allTextureResources.begin(), end = allTextureResources.end();
         it != end; ++it) {
        const Effect::ImageResourceParameter &parameter = *it;
        allAttachments.insert(tinystl::make_pair(parameter.m_filename, parameter.m_fileURI));
    }
}

void
Effect::addSemanticParameterHandler(const char *const name, SemanticParameterHandler value)
{
    m_semanticParameterHandlers.insert(tinystl::make_pair(String(name), value));
}

void
Effect::addImageFormat(const char *const name, sg_pixel_format value)
{
    static const char *kPrefixes[] = { "", "FMT_", "D3DFMT_" };
    for (size_t i = 0; i < BX_COUNTOF(kPrefixes); i++) {
        m_imageFormats.insert(tinystl::make_pair(PrivateEffectUtils::concatPrefix(name, kPrefixes[i]), value));
    }
}

} /* namespace nanoem */
