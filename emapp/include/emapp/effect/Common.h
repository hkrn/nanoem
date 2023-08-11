/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_EFFECT_COMMON_H_
#define NANOEM_EMAPP_EFFECT_COMMON_H_

#include "emapp/IEffect.h"
#include "emapp/PixelFormat.h"

#include "emapp/effect/GlobalUniform.h"

namespace nanoem {

class Effect;
class PixelFormat;
struct APNGImage;

namespace internal {
class BlitPass;
} /* namespace internal */

namespace effect {

static const char kBooleanTrueValueLiteral[] = "true";
static const char kScriptKeyLiteral[] = "Script";
static const char kDrawGeometryValueLiteral[] = "Geometry";
static const char kDrawBufferValueLiteral[] = "Buffer";

enum AttachmentType {
    kAttachmentTypeFirstEnum,
    kAttachmentTypeNone = kAttachmentTypeFirstEnum,
    kAttachmentTypeOffscreenPassive,
    kAttachmentTypeMaterial,
    kLoadingSourceTypeTypeMaxEnum,
};

struct PipelineDescriptor {
    struct Stencil {
        Stencil();
        ~Stencil();
        bool m_hasCompareFunc;
        bool m_hasPassOp;
        bool m_hasFailOp;
        bool m_hasDepthFailOp;
    };
    PipelineDescriptor();
    PipelineDescriptor(const PipelineDescriptor &source);
    ~PipelineDescriptor();

    sg_pipeline_desc m_body;
    Stencil m_stencilFront;
    Stencil m_stencilBack;
    bool m_hasBlendEnabled;
    bool m_hasBlendSourceFactorRGB;
    bool m_hasBlendDestFactorRGB;
    bool m_hasBlendSourceFactorAlpha;
    bool m_hasBlendDestFactorAlpha;
    bool m_hasBlendOpRGB;
    bool m_hasBlendOpAlpha;
    bool m_hasColorWriteMask;
    bool m_hasDepthCompareFunc;
    bool m_hasDepthWriteEnabled;
    bool m_hasStencilEnabled;
    bool m_hasStencilRef;
    bool m_hasStencilReadMask;
    bool m_hasStencilWriteMask;
};

enum ParameterType {
    kParameterTypeUnknown,
    kParameterTypeVoid,
    kParameterTypeString,
    kParameterTypeTexture,
    kParameterTypeTexture1D,
    kParameterTypeTexture2D,
    kParameterTypeTexture3D,
    kParameterTypeTextureCube,
    kParameterTypeSampler,
    kParameterTypeSampler1D,
    kParameterTypeSampler2D,
    kParameterTypeSampler3D,
    kParameterTypeSamplerCube,
    kParameterTypeBool,
    kParameterTypeInt = kParameterTypeBool + 16,
    kParameterTypeInt4 = kParameterTypeInt + 3,
    kParameterTypeFloat = kParameterTypeInt + 16,
    kParameterTypeFloat4 = kParameterTypeFloat + 3,
    kParameterTypeFloat3x3 = kParameterTypeFloat + 8,
    kParameterTypeFloat4x4 = kParameterTypeFloat + 15,
    kParameterTypeMaxEnum
};

struct AnnotationValue {
    enum Type {
        kAnnotationTypeFirstEnum,
        kAnnotationTypeBoolean = kAnnotationTypeFirstEnum,
        kAnnotationTypeInteger,
        kAnnotationTypeFloat,
        kAnnotationTypeString,
        kAnnotationTypeVector4,
        kAnnotationTypeMaxEnum
    };
    AnnotationValue();
    ~AnnotationValue() NANOEM_DECL_NOEXCEPT;
    nanoem_f32_t toFloat() const NANOEM_DECL_NOEXCEPT;
    int toInt() const NANOEM_DECL_NOEXCEPT;
    bool toBool() const NANOEM_DECL_NOEXCEPT;
    bool m_bool;
    int m_int;
    nanoem_f32_t m_float;
    String m_string;
    Vector4 m_vector;
};

struct AnnotationMap : tinystl::unordered_map<String, AnnotationValue, TinySTLAllocator> {
    const_iterator findAnnotation(const char *const key) const NANOEM_DECL_NOEXCEPT;
    String stringAnnotation(const char *key, const String &defv) const;
    int triBooleanAnnotation(const char *key, int defv) const NANOEM_DECL_NOEXCEPT;
    int integerAnnotation(const char *key, int defv) const NANOEM_DECL_NOEXCEPT;
    nanoem_f32_t floatAnnotation(const char *key, nanoem_f32_t defv) const NANOEM_DECL_NOEXCEPT;
};

struct RegisterIndex {
    RegisterIndex();
    RegisterIndex(nanoem_u32_t index, nanoem_u32_t type);
    ~RegisterIndex() NANOEM_DECL_NOEXCEPT;
    nanoem_u32_t m_index;
    nanoem_u32_t m_count;
    nanoem_u32_t m_type;
    nanoem_u32_t m_flags;
};
struct SamplerRegisterIndex {
    typedef tinystl::vector<tinystl::pair<int, int>, TinySTLAllocator> List;
    SamplerRegisterIndex();
    ~SamplerRegisterIndex() NANOEM_DECL_NOEXCEPT;
    List m_indices;
    String m_name;
    nanoem_u32_t m_type;
    nanoem_u32_t m_flags;
};

typedef tinystl::unordered_map<String, sg_image_desc, TinySTLAllocator> ImageDescriptionMap;
typedef tinystl::unordered_map<String, sg_sampler_desc, TinySTLAllocator> SamplerDescriptionMap;
typedef tinystl::unordered_map<String, RegisterIndex, TinySTLAllocator> RegisterIndexMap;
typedef tinystl::unordered_map<String, SamplerRegisterIndex, TinySTLAllocator> SamplerRegisterIndexMap;
struct PassRegisterIndexMap {
    SamplerRegisterIndexMap m_vertexShaderSamplers;
    SamplerRegisterIndexMap m_pixelShaderSamplers;
    RegisterIndexMap m_vertexShader;
    RegisterIndexMap m_pixelShader;
    RegisterIndexMap m_vertexPreshader;
    RegisterIndexMap m_pixelPreshader;
};
typedef tinystl::unordered_map<nanoem_u32_t, nanoem_u32_t, TinySTLAllocator> UniformBufferOffsetMap;

struct ScriptIndex {
    static const ScriptIndex kInvalid;
    ScriptIndex();
    ScriptIndex(size_t techniqueIndex, size_t passIndex);
    ~ScriptIndex() NANOEM_DECL_NOEXCEPT;
    size_t m_techniqueIndex;
    size_t m_passIndex;
};

struct Preshader {
    struct Instruction;
    struct Operand {
        Operand();
        ~Operand() NANOEM_DECL_NOEXCEPT;
        String toString(const Preshader::Instruction *instructionPtr, const Preshader *shaderPtr) const;
        nanoem_u32_t m_index;
        nanoem_u32_t m_type;
    };
    struct Instruction {
        Instruction();
        ~Instruction() NANOEM_DECL_NOEXCEPT;
        const char *opcodeString() const;
        String toString(const Preshader *shaderPtr) const;
        nanoem_u32_t m_opcode;
        nanoem_u32_t m_numElements;
        tinystl::vector<Operand, TinySTLAllocator> m_operands;
    };
    struct Symbol {
        Symbol();
        ~Symbol() NANOEM_DECL_NOEXCEPT;
        String m_name;
        nanoem_u32_t m_count;
        nanoem_u32_t m_index;
        nanoem_u32_t m_set;
    };
    Preshader();
    ~Preshader() NANOEM_DECL_NOEXCEPT;
    void execute(const GlobalUniform::Buffer &inputBuffer, GlobalUniform::Buffer &outputBuffer) const;

    tinystl::vector<Instruction, TinySTLAllocator> m_instructions;
    tinystl::vector<Symbol, TinySTLAllocator> m_symbols;
    tinystl::vector<nanoem_f32_t, TinySTLAllocator> m_literals;
    nanoem_u32_t m_numTemporaryRegisters;
};

struct PreshaderPair {
    PreshaderPair();
    PreshaderPair(const Preshader &v, const Preshader &p);
    PreshaderPair(const PreshaderPair &value);
    ~PreshaderPair() NANOEM_DECL_NOEXCEPT;

    Preshader vertex;
    Preshader pixel;
};

struct UIWidgetParameter {
    enum Type {
        kUIWidgetTypeFirst,
        kUIWidgetTypeNumeric = kUIWidgetTypeFirst,
        kUIWidgetTypeSlider,
        kUIWidgetTypeSpinner,
        kUIWidgetTypeColor,
        kUIWidgetTypeCheckbox,
        kUIWidgetTypeMaxEnum
    };
    UIWidgetParameter(ParameterType parameterType, const AnnotationMap &annotations,
        GlobalUniform::Vector4List *valuePtr, nanoem_unicode_string_factory_t *factory);
    ~UIWidgetParameter() NANOEM_DECL_NOEXCEPT;

    Type m_widgetType;
    ParameterType m_parameterType;
    String m_name;
    String m_description;
    tinystl::vector<Vector4, TinySTLAllocator> *m_valuePtr;
    Vector2 range;
    bool m_visible;
};
typedef tinystl::vector<UIWidgetParameter, TinySTLAllocator> UIWidgetParameterList;

class RenderTargetNormalizer;
class Pass;
class RenderPassScope {
public:
    RenderPassScope();
    ~RenderPassScope();

    void reset(RenderTargetNormalizer *value);
    void modifyPipelineDescription(sg_pipeline_desc &pd) const NANOEM_DECL_NOEXCEPT;
    void apply(
        const sg_bindings &bindings, const IDrawable *drawable, Pass *pass, sg_pipeline pipeline, sg::PassBlock &pb);

private:
    RenderTargetNormalizer *m_normalizer;
};

struct AnnotatableParameter {
    AnnotatableParameter(const String &name);
    AnnotatableParameter(const String &name, const AnnotationMap &annotations);

    String m_name;
    AnnotationMap m_annotations;
};

struct NonSemanticParameter : AnnotatableParameter {
    NonSemanticParameter(
        const String &name, const GlobalUniform::Vector4List &values, const AnnotationMap &annotations);
    tinystl::vector<Vector4, TinySTLAllocator> m_values;
};

struct TypedSemanticParameter : AnnotatableParameter {
    static const int kShared = 0x1;
    static const int kLiteral = 0x2;
    static const int kAnnotation = 0x4;
    TypedSemanticParameter(const String &name, const String &semantic, ParameterType type);
    ~TypedSemanticParameter() NANOEM_DECL_NOEXCEPT;
    String m_semantic;
    ParameterType m_type;
    ByteArray m_value;
    bool m_shared;
};
typedef tinystl::unordered_map<String, TypedSemanticParameter, TinySTLAllocator> ParameterMap;
typedef tinystl::unordered_map<String, NonSemanticParameter, TinySTLAllocator> BoolParameterUniformMap;
typedef tinystl::unordered_map<String, NonSemanticParameter, TinySTLAllocator> IntParameterUniformMap;
typedef tinystl::unordered_map<String, NonSemanticParameter, TinySTLAllocator> FloatParameterUniformMap;
typedef tinystl::unordered_map<String, NonSemanticParameter, TinySTLAllocator> VectorParameterUniformMap;
typedef tinystl::unordered_set<String, TinySTLAllocator> SemanticUniformList;
typedef tinystl::unordered_map<String, sg_pixel_format, TinySTLAllocator> ImageFormatMap;
typedef tinystl::unordered_map<String, tinystl::pair<sg_image, sg_sampler>, TinySTLAllocator> SemanticImageMap;

enum MatrixType {
    kMatrixTypeWorld,
    kMatrixTypeView,
    kMatrixTypeProjection,
    kMatrixTypeWorldView,
    kMatrixTypeViewProjection,
    kMatrixTypeWorldViewProjection
};

enum ScriptCommandType {
    kScriptCommandTypeFirstEnum,
    /* RenderColorTarget0=(texture) */
    kScriptCommandTypeSetRenderColorTarget0 = kScriptCommandTypeFirstEnum,
    /* RenderColorTarget1=(texture) */
    kScriptCommandTypeSetRenderColorTarget1,
    /* RenderColorTarget2=(texture) */
    kScriptCommandTypeSetRenderColorTarget2,
    /* RenderColorTarget3=(texture) */
    kScriptCommandTypeSetRenderColorTarget3,
    /* RenderDepthStencilTarget=(texture) */
    kScriptCommandTypeSetRenderDepthStencilTarget,
    /* ClearSetColor=(parameter) */
    kScriptCommandTypeClearSetColor,
    /* ClearSetDepth=(parameter) */
    kScriptCommandTypeClearSetDepth,
    /* Clear=Color, Clear=Depth */
    kScriptCommandTypeClear,
    /* ScriptExternal=Color */
    kScriptCommandTypeSetScriptExternal,
    /* Pass=(pass) */
    kScriptCommandTypeExecutePass,
    /* LoopByCount=(parameter) */
    kScriptCommandTypePushLoopCounter,
    /* LoopGetIndex=(parameter) */
    kScriptCommandTypeGetLoopIndex,
    /* LoopEnd= */
    kScriptCommandTypePopLoopCounter,
    /* Draw=Geometry, Draw=Buffer */
    kScriptCommandTypeDraw,
    kScriptCommandTypeMaxEnum
};

struct MatrixUniform {
    MatrixUniform(const String &name, MatrixType type, bool inversed, bool transposed);
    ~MatrixUniform() NANOEM_DECL_NOEXCEPT;
    Matrix4x4 transformed(const Matrix4x4 &value) const NANOEM_DECL_NOEXCEPT;
    void multiply(const Matrix4x4 &world, const Matrix4x4 &view, const Matrix4x4 &projection,
        Matrix4x4 &result) const NANOEM_DECL_NOEXCEPT;
    const String m_name;
    const MatrixType m_type;
    const bool m_inversed;
    const bool m_transposed;
};
typedef tinystl::unordered_map<String, MatrixUniform, TinySTLAllocator> MatrixUniformMap;

struct SampledImage {
    SampledImage(const String &name, sg_shader_stage stage, sg_image image, sg_sampler sampler, nanoem_u32_t imageIndex,
        nanoem_u32_t samplerIndex);
    ~SampledImage() NANOEM_DECL_NOEXCEPT;
    const String m_name;
    const sg_shader_stage m_stage;
    const sg_image m_imageHandle;
    const sg_sampler m_samplerHandle;
    const nanoem_u32_t m_imageIndex;
    const nanoem_u32_t m_samplerIndex;
};
typedef tinystl::vector<SampledImage, TinySTLAllocator> SampledImageList;
typedef tinystl::vector<tinystl::pair<ScriptCommandType, String>, TinySTLAllocator> ScriptCommandMap;

struct ControlObjectTarget {
    ControlObjectTarget(const String &name, ParameterType type);
    ~ControlObjectTarget() NANOEM_DECL_NOEXCEPT;
    const String m_name;
    const ParameterType m_type;
    String m_item;
    Vector4 m_value;
};
typedef tinystl::unordered_map<String, ControlObjectTarget, TinySTLAllocator> ControlObjectTargetMap;

struct LoopCounter {
    typedef tinystl::vector<LoopCounter, TinySTLAllocator> Stack;
    static bool isScriptCommandIgnorable(ScriptCommandType type, const Stack &counterStack);
    LoopCounter(const String name, size_t last, size_t gotoScriptIndex);
    LoopCounter(const LoopCounter &value);
    ~LoopCounter() NANOEM_DECL_NOEXCEPT;
    LoopCounter &operator=(const LoopCounter &value);
    const String m_name;
    const size_t m_last;
    const size_t m_gotoScriptIndex;
    size_t m_offset;
};
typedef tinystl::unordered_set<nanoem_u32_t, TinySTLAllocator> ViewPassSet;

class RenderState NANOEM_DECL_SEALED : private NonCopyable {
public:
    static void convertBlendFactor(nanoem_u32_t value, sg_blend_factor &desc);
    static void convertBlendOperator(nanoem_u32_t value, sg_blend_op &desc);
    static void convertWrap(nanoem_u32_t value, sg_wrap &wrap);
    static void convertFilter(nanoem_u32_t value, sg_sampler_desc &desc, sg_filter &filter);
    static void convertStencilOp(nanoem_u32_t value, sg_stencil_op &op);
    static void convertCompareFunc(nanoem_u32_t value, sg_compare_func &func);
    static void convertSamplerState(nanoem_u32_t key, nanoem_u32_t value, sg_sampler_desc &desc);
    static void convertPipeline(nanoem_u32_t key, nanoem_u32_t value, PipelineDescriptor &desc);
};

class Logger NANOEM_DECL_SEALED : private NonCopyable {
public:
    Logger();
    ~Logger() NANOEM_DECL_NOEXCEPT;

    void add(const char *message, int length);
    void log(const char *format, ...);

private:
    struct Container {
        nanoem_u64_t m_time;
        String m_body;
    };
    typedef tinystl::unordered_map<nanoem_u32_t, Container, TinySTLAllocator> ContainerMap;
    ContainerMap m_containers;
};

} /* namespace effect */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_EFFECT_COMMON_H_ */
