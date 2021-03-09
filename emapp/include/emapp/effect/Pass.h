/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_EFFECT_PASS_H_
#define NANOEM_EMAPP_EFFECT_PASS_H_

#include "emapp/IPass.h"
#include "emapp/effect/Common.h"

namespace nanoem {

class Accessory;
class ICamera;
class ILight;
class Effect;
class IDrawable;
class Model;
class Project;
class ShadowCamera;

namespace effect {

class Technique;
class Pass NANOEM_DECL_SEALED : public IPass {
public:
    static int countShaderImages(const sg_shader_image_desc *images);

    Pass(Effect *effect, const String &name, const PipelineDescriptor &pd, const PassRegisterIndexMap &indices,
        const AnnotationMap &annotations, const UniformBufferOffsetMap &vertexShaderRegisterUniformBufferOffsetMap,
        const UniformBufferOffsetMap &pixelShaderRegisterUniformBufferOffsetMap, const PreshaderPair &preshaderPair,
        const sg_shader_desc &desc);
    ~Pass() NANOEM_DECL_NOEXCEPT;

    void destroy();
    sg_pipeline registerPipelineSet(const sg_pipeline_desc &desc);
    void setAccessoryParameter(const String &name, const Accessory *model, ControlObjectTarget &target);
    void setModelParameter(const String &name, const Model *model, ControlObjectTarget &target);
    void ensureScriptCommandClearColor();
    void ensureScriptCommandClearDepth();
    void interpretScriptCommand(ScriptCommandType type, const String &value, const IDrawable *drawable,
        const Buffer &buffer, LoopCounter::Stack &counterStack, size_t &scriptIndex);
    void resetVertexBuffer();

    void setGlobalParameters(const IDrawable *drawable, const Project *project) NANOEM_DECL_OVERRIDE;
    void setCameraParameters(const ICamera *camera, const Matrix4x4 &world) NANOEM_DECL_OVERRIDE;
    void setLightParameters(const ILight *light, bool adjustment) NANOEM_DECL_OVERRIDE;
    void setAllAccessoryParameters(const Accessory *accessory, const Project *project) NANOEM_DECL_OVERRIDE;
    void setAllModelParameters(const Model *model, const Project *project) NANOEM_DECL_OVERRIDE;
    void setMaterialParameters(const nanodxm_material_t *materialPtr) NANOEM_DECL_OVERRIDE;
    void setMaterialParameters(const nanoem_model_material_t *materialPtr) NANOEM_DECL_OVERRIDE;
    void setEdgeParameters(const nanoem_model_material_t *materialPtr, nanoem_f32_t edgeSize) NANOEM_DECL_OVERRIDE;
    void setGroundShadowParameters(
        const ILight *light, const ICamera *camera, const Matrix4x4 &world) NANOEM_DECL_OVERRIDE;
    void setShadowMapParameters(const ShadowCamera *shadowCamera, const Matrix4x4 &world) NANOEM_DECL_OVERRIDE;
    void execute(const IDrawable *drawable, const Buffer &buffer) NANOEM_DECL_OVERRIDE;

    const Technique *technique() const NANOEM_DECL_NOEXCEPT;
    Technique *technique() NANOEM_DECL_NOEXCEPT;
    String name() const;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT;
    nanoem_u8_t vertexShaderImageCount() const NANOEM_DECL_NOEXCEPT;
    nanoem_u8_t pixelShaderImageCount() const NANOEM_DECL_NOEXCEPT;

    const PipelineDescriptor &pipelineDescriptor() const NANOEM_DECL_NOEXCEPT;
    void setParentTechnique(Technique *value);
    void setRenderTargetIndexOffset(size_t value);
    void setTechniqueScriptIndex(size_t value);
    bool findVertexPreshaderRegisterIndex(const String &name, RegisterIndex &index) const;
    bool findPixelPreshaderRegisterIndex(const String &name, RegisterIndex &index) const;
    bool findVertexShaderRegisterIndex(const String &name, RegisterIndex &index) const;
    bool findPixelShaderRegisterIndex(const String &name, RegisterIndex &index) const;
    bool findVertexShaderSamplerRegisterIndex(const String &name, SamplerRegisterIndex::List &samplerIndices) const;
    bool findPixelShaderSamplerRegisterIndex(const String &name, SamplerRegisterIndex::List &samplerIndices) const;

private:
    typedef tinystl::unordered_map<nanoem_u32_t, sg_pipeline, TinySTLAllocator> PipelineSet;
    const String m_name;
    const PassRegisterIndexMap m_registerIndices;
    const UniformBufferOffsetMap m_vertexShaderRegisterUniformBufferOffsetMap;
    const UniformBufferOffsetMap m_pixelShaderBufferUniformBufferOffsetMap;
    const Accessory *m_activeAccessoryPtr;
    const PipelineDescriptor m_pipelineDescriptor;
    const nanoem_u8_t m_vertexShaderImageCount;
    const nanoem_u8_t m_pixelShaderImageCount;
    Effect *m_effect;
    Technique *m_techniquePtr;
    PipelineSet m_pipelineSet;
    ScriptCommandMap m_script;
    PreshaderPair m_preshaderPair;
    RenderPassScope m_renderTargetNormalizerScope;
    sg_buffer m_vertexBuffer;
    size_t m_renderTargetIndexOffset;
    size_t m_techniqueScriptIndex;
};
typedef tinystl::vector<Pass *, TinySTLAllocator> PassList;
typedef tinystl::unordered_map<String, Pass *, TinySTLAllocator> PassMap;

} /* namespace effect */
} /* namespace nanoem */

#endif /* NANOEM_EFFECT_PASS_H_ */
