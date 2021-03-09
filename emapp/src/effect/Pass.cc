/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/effect/Pass.h"

#include "emapp/Accessory.h"
#include "emapp/Effect.h"
#include "emapp/Model.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#include "bx/hash.h"

namespace nanoem {
namespace effect {

int
Pass::countShaderImages(const sg_shader_image_desc *images)
{
    int count = SG_MAX_SHADERSTAGE_IMAGES;
    for (int i = 0; i < SG_MAX_SHADERSTAGE_IMAGES; i++) {
        if (images[i].image_type == _SG_IMAGETYPE_DEFAULT) {
            count = i;
            break;
        }
    }
    return count;
}

Pass::Pass(Effect *effect, const String &name, const PipelineDescriptor &pd,
    const PassRegisterIndexMap &registerIndices, const AnnotationMap &annotations,
    const UniformBufferOffsetMap &vertexShaderRegisterUniformBufferOffsetMap,
    const UniformBufferOffsetMap &pixelShaderRegisterUniformBufferOffsetMap, const PreshaderPair &preshaderPair,
    const sg_shader_desc &desc)
    : m_name(name)
    , m_registerIndices(registerIndices)
    , m_vertexShaderRegisterUniformBufferOffsetMap(vertexShaderRegisterUniformBufferOffsetMap)
    , m_pixelShaderBufferUniformBufferOffsetMap(pixelShaderRegisterUniformBufferOffsetMap)
    , m_activeAccessoryPtr(nullptr)
    , m_pipelineDescriptor(pd)
    , m_vertexShaderImageCount(countShaderImages(desc.vs.images))
    , m_pixelShaderImageCount(countShaderImages(desc.fs.images))
    , m_effect(effect)
    , m_techniquePtr(nullptr)
    , m_preshaderPair(preshaderPair)
    , m_renderTargetIndexOffset(SIZE_MAX)
    , m_techniqueScriptIndex(SIZE_MAX)
{
    m_vertexBuffer = { SG_INVALID_ID };
    const String s(annotations.stringAnnotation(kScriptKeyLiteral, String("Draw=Geometry;")));
    Effect::parseScript(s, m_script);
}

Pass::~Pass() NANOEM_DECL_NOEXCEPT
{
}

void
Pass::destroy()
{
    SG_PUSH_GROUPF("effect::Pass::destroy(name=%s)", nameConstString());
    for (PipelineSet::iterator it = m_pipelineSet.begin(), end = m_pipelineSet.end(); it != end; ++it) {
        SG_INSERT_MARKERF("effect::Pass::destroy(pipeline=%d)", it->second.id);
        sg::destroy_pipeline(it->second);
    }
    sg_shader shader = m_pipelineDescriptor.m_body.shader;
    SG_INSERT_MARKERF("effect::Pass::destroy(shader=%d, buffer=%d)", shader.id, m_vertexBuffer.id);
    m_effect->removeShaderLabel(shader);
    sg::destroy_shader(shader);
    sg::destroy_buffer(m_vertexBuffer);
    SG_POP_GROUP();
}

sg_pipeline
Pass::registerPipelineSet(const sg_pipeline_desc &desc)
{
    const nanoem_u32_t key = bx::hash<bx::HashMurmur2A>(desc);
    PipelineSet::const_iterator it = m_pipelineSet.find(key);
    sg_pipeline pipeline;
    if (it != m_pipelineSet.end()) {
        pipeline = it->second;
    }
    else {
        char label[Inline::kMarkerStringLength];
        sg_pipeline_desc newDesc(desc);
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(label, sizeof(label), "Effects/%s/%s/%s/%d", m_effect->nameConstString(),
                m_techniquePtr->nameConstString(), nameConstString(), Inline::saturateInt32(m_pipelineSet.size()));
            newDesc.label = label;
        }
        else {
            *label = 0;
        }
        pipeline = sg::make_pipeline(&newDesc);
        if (sg::query_pipeline_state(pipeline) == SG_RESOURCESTATE_VALID) {
            m_techniquePtr->effect()->project()->setRenderPipelineName(pipeline, label);
            m_pipelineSet.insert(tinystl::make_pair(key, pipeline));
        }
        else {
            m_effect->logger()->log("Creating the pipeline Effects/%s/%s/%s/%d failed", m_effect->nameConstString(),
                m_techniquePtr->nameConstString(), nameConstString(), Inline::saturateInt32(m_pipelineSet.size()));
            sg::destroy_pipeline(pipeline);
            pipeline = { SG_INVALID_ID };
        }
    }
    return pipeline;
}

void
Pass::setAccessoryParameter(const String &name, const Accessory *accessory, ControlObjectTarget &target)
{
    m_effect->setAccessoryParameter(name, accessory, target, this);
}

void
Pass::setModelParameter(const String &name, const Model *model, ControlObjectTarget &target)
{
    m_effect->setModelParameter(name, model, target, this);
}

void
Pass::ensureScriptCommandClearColor()
{
    m_script.insert(m_script.begin(), tinystl::make_pair(kScriptCommandTypeClear, String("Color")));
}

void
Pass::ensureScriptCommandClearDepth()
{
    m_script.insert(m_script.begin(), tinystl::make_pair(kScriptCommandTypeClear, String("Depth")));
}

void
Pass::interpretScriptCommand(ScriptCommandType type, const String &value, const IDrawable *drawable,
    const Buffer &buffer, LoopCounter::Stack &counterStack, size_t &scriptIndex)
{
    switch (type) {
    case kScriptCommandTypePushLoopCounter: {
        SG_INSERT_MARKERF("%d: %s/%s/%s/LoopByCount=%s", scriptIndex, m_effect->nameConstString(),
            m_techniquePtr->nameConstString(), nameConstString(), value.c_str());
        m_effect->pushLoopCounter(value, scriptIndex, counterStack);
        break;
    }
    case kScriptCommandTypeClear: {
        SG_INSERT_MARKERF("%d: %s/%s/%s/Clear=%s", scriptIndex, m_effect->nameConstString(),
            m_techniquePtr->nameConstString(), nameConstString(), value.c_str());
        char nameBuffer[Inline::kMarkerStringLength];
        StringUtils::format(nameBuffer, sizeof(nameBuffer), "Effects/%s/%s/%s/Clear", m_effect->nameConstString(),
            m_techniquePtr->nameConstString(), nameConstString());
        m_effect->clearRenderPass(drawable, nameBuffer, value, m_techniquePtr->currentRenderPassScope());
        break;
    }
    case kScriptCommandTypeSetRenderDepthStencilTarget: {
        SG_INSERT_MARKERF("%d: %s/%s/%s/SetRenderDepthStencilTarget=%s", scriptIndex, m_effect->nameConstString(),
            m_techniquePtr->nameConstString(), nameConstString(), value.c_str());
        m_effect->setRenderTargetDepthStencilImageDescription(value);
        break;
    }
    case kScriptCommandTypeGetLoopIndex: {
        SG_INSERT_MARKERF("%d: %s/%s/%s/GetLoopIndex=%s", scriptIndex, m_effect->nameConstString(),
            m_techniquePtr->nameConstString(), nameConstString(), value.c_str());
        m_effect->handleLoopGetIndex(value, counterStack);
        break;
    }
    case kScriptCommandTypeDraw: {
        SG_INSERT_MARKERF("%d: %s/%s/%s/Draw=%s", scriptIndex, m_effect->nameConstString(),
            m_techniquePtr->nameConstString(), nameConstString(), value.c_str());
        GlobalUniform *globalUniformPtr = m_effect->globalUniform();
        m_preshaderPair.vertex.execute(
            globalUniformPtr->m_preshaderVertexShaderBuffer, globalUniformPtr->m_vertexShaderBuffer);
        m_preshaderPair.pixel.execute(
            globalUniformPtr->m_preshaderPixelShaderBuffer, globalUniformPtr->m_pixelShaderBuffer);
        if (StringUtils::equals(value.c_str(), kDrawGeometryValueLiteral)) {
            if (m_effect->scriptClass() == IEffect::kScriptClassTypeScene) {
                m_effect->logger()->log("Pass \"%s/%s/%s\" tries drawing geometry but script class specified \"scene\"",
                    m_effect->nameConstString(), m_techniquePtr->nameConstString(), nameConstString());
            }
            PipelineDescriptor dest(m_pipelineDescriptor);
            m_techniquePtr->overrideObjectPipelineDescription(drawable, dest);
            sg_bindings bindings;
            Inline::clearZeroMemory(bindings);
            bindings.vertex_buffers[0] = buffer.m_vertexBuffer;
            bindings.index_buffer = buffer.m_indexBuffer;
            m_effect->drawGeometryRenderPass(
                drawable, this, buffer.m_offset, buffer.m_numIndices, dest.m_body, bindings);
        }
        else if (StringUtils::equals(value.c_str(), kDrawBufferValueLiteral)) {
            if (m_effect->scriptClass() == IEffect::kScriptClassTypeObject) {
                m_effect->logger()->log("Pass \"%s/%s/%s\" tries drawing buffer but script class specified \"object\"",
                    m_effect->nameConstString(), m_techniquePtr->nameConstString(), nameConstString());
            }
            PipelineDescriptor dest(m_pipelineDescriptor);
            m_techniquePtr->overrideScenePipelineDescription(drawable, dest);
            if (!sg::is_valid(m_vertexBuffer)) {
                sg_buffer_desc vbd;
                Inline::clearZeroMemory(vbd);
                sg::QuadVertexUnit vertices[4];
                sg::QuadVertexUnit::generateQuadTriStrip(vertices);
                /* add subpixel offset shift due to DirectX9 spec */
                Vector4 offset;
                m_effect->getPrimaryRenderTargetColorImageSubPixelOffset(offset);
                vertices[0].m_texcoord += offset;
                vertices[1].m_texcoord += offset;
                vertices[2].m_texcoord += offset;
                vertices[3].m_texcoord += offset;
                vbd.size = sizeof(vertices);
                vbd.data.ptr = vertices;
                vbd.data.size = vbd.size;
                char label[Inline::kMarkerStringLength];
                if (Inline::isDebugLabelEnabled()) {
                    StringUtils::format(label, sizeof(label), "Effects/%s/%s/%s/VertexBuffer",
                        m_effect->nameConstString(), m_techniquePtr->nameConstString(), nameConstString());
                    vbd.label = label;
                }
                else {
                    *label = 0;
                }
                m_vertexBuffer = sg::make_buffer(&vbd);
                nanoem_assert(sg::query_buffer_state(m_vertexBuffer) == SG_RESOURCESTATE_VALID, "buffer must be valid");
                SG_LABEL_BUFFER(m_vertexBuffer, label);
            }
            sg_bindings bindings;
            Inline::clearZeroMemory(bindings);
            bindings.vertex_buffers[0] = m_vertexBuffer;
            bindings.index_buffer = { SG_INVALID_ID };
            m_effect->drawSceneRenderPass(drawable, this, dest.m_body, bindings);
        }
        break;
    }
    case kScriptCommandTypeClearSetColor: {
        SG_INSERT_MARKERF("%d: %s/%s/%s/ClearSetColor=%s", scriptIndex, m_effect->nameConstString(),
            m_techniquePtr->nameConstString(), nameConstString(), value.c_str());
        m_effect->setClearColor(value);
        break;
    }
    case kScriptCommandTypeClearSetDepth: {
        SG_INSERT_MARKERF("%d: %s/%s/%s/ClearSetDepth=%s", scriptIndex, m_effect->nameConstString(),
            m_techniquePtr->nameConstString(), nameConstString(), value.c_str());
        m_effect->setClearDepth(value);
        break;
    }
    case kScriptCommandTypePopLoopCounter: {
        SG_INSERT_MARKERF("%d: %s/%s/%s/LoopEnd", scriptIndex, m_effect->nameConstString(),
            m_techniquePtr->nameConstString(), nameConstString());
        m_effect->popLoopCounter(counterStack, scriptIndex);
        break;
    }
    case kScriptCommandTypeSetRenderColorTarget0:
    case kScriptCommandTypeSetRenderColorTarget1:
    case kScriptCommandTypeSetRenderColorTarget2:
    case kScriptCommandTypeSetRenderColorTarget3: {
        const size_t renderTargetIndexOffset = type - kScriptCommandTypeSetRenderColorTarget0;
        SG_INSERT_MARKERF("%d: %s/%s/%s/RenderColorTarget%d=%s", scriptIndex, m_effect->nameConstString(),
            m_techniquePtr->nameConstString(), nameConstString(), renderTargetIndexOffset,
            value.empty() ? "(null)" : value.c_str());
        m_effect->setRenderTargetColorImageDescription(drawable, renderTargetIndexOffset, value);
        m_renderTargetIndexOffset = renderTargetIndexOffset;
        break;
    }
    case kScriptCommandTypeExecutePass:
    case kScriptCommandTypeSetScriptExternal:
    case kScriptCommandTypeMaxEnum:
    default:
        m_effect->logger()->log("Command %d in \"%s/%s/%s\" at %d is not accepted therefore ignored", type,
            m_effect->nameConstString(), m_techniquePtr->nameConstString(), nameConstString(), scriptIndex);
        break;
    }
}

void
Pass::resetVertexBuffer()
{
    SG_PUSH_GROUPF("effect::Pass::resetVertexBuffer(name=%s)", nameConstString());
    if (sg::is_valid(m_vertexBuffer)) {
        SG_INSERT_MARKERF("effect::Pass::resetVertexBuffer(buffer=%d)", m_vertexBuffer.id);
        sg::destroy_buffer(m_vertexBuffer);
        m_vertexBuffer.id = SG_INVALID_ID;
    }
    SG_POP_GROUP();
}

void
Pass::setGlobalParameters(const IDrawable *drawable, const Project *project)
{
    m_effect->setGlobalParameters(drawable, project, this);
}

void
Pass::setCameraParameters(const ICamera *camera, const Matrix4x4 &world)
{
    m_effect->setCameraParameters(camera, world, this);
}

void
Pass::setLightParameters(const ILight *light, bool adjustment)
{
    m_effect->setLightParameters(light, adjustment, this);
}

void
Pass::setAllAccessoryParameters(const Accessory *accessory, const Project *project)
{
    m_effect->setAllAccessoryParameters(accessory, project, this);
    m_activeAccessoryPtr = accessory;
}

void
Pass::setAllModelParameters(const Model *model, const Project *project)
{
    m_effect->setAllModelParameters(model, project, this);
}

void
Pass::setMaterialParameters(const nanodxm_material_t *materialPtr)
{
    m_effect->setMaterialParameters(m_activeAccessoryPtr, materialPtr, this);
}

void
Pass::setMaterialParameters(const nanoem_model_material_t *materialPtr)
{
    m_effect->setMaterialParameters(materialPtr, m_techniquePtr->passType(), this);
}

void
Pass::setEdgeParameters(const nanoem_model_material_t *materialPtr, nanoem_f32_t edgeSize)
{
    m_effect->setEdgeParameters(materialPtr, edgeSize, this);
}

void
Pass::setGroundShadowParameters(const ILight *light, const ICamera *camera, const Matrix4x4 &world)
{
    m_effect->setShadowParameters(light, camera, world, this);
}

void
Pass::setShadowMapParameters(const ShadowCamera *shadowCamera, const Matrix4x4 &world)
{
    m_effect->setShadowMapParameters(shadowCamera, world, this);
}

void
Pass::execute(const IDrawable *drawable, const Buffer &buffer)
{
    SG_PUSH_GROUPF("effect::Pass::execute(%s/%s)", m_techniquePtr->nameConstString(), nameConstString());
    LoopCounter::Stack counterStack;
    for (size_t i = 0, numScripts = m_script.size(); i < numScripts; i++) {
        const tinystl::pair<ScriptCommandType, String> &script = m_script[i];
        if (!LoopCounter::isScriptCommandIgnorable(script.first, counterStack)) {
            interpretScriptCommand(script.first, script.second, drawable, buffer, counterStack, i);
        }
    }
    SG_POP_GROUP();
}

const Technique *
Pass::technique() const NANOEM_DECL_NOEXCEPT
{
    return m_techniquePtr;
}

Technique *
Pass::technique() NANOEM_DECL_NOEXCEPT
{
    return m_techniquePtr;
}

String
Pass::name() const
{
    return m_name;
}

const char *
Pass::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_name.c_str();
}

nanoem_u8_t
Pass::vertexShaderImageCount() const NANOEM_DECL_NOEXCEPT
{
    return m_vertexShaderImageCount;
}

nanoem_u8_t
Pass::pixelShaderImageCount() const NANOEM_DECL_NOEXCEPT
{
    return m_pixelShaderImageCount;
}

const PipelineDescriptor &
Pass::pipelineDescriptor() const NANOEM_DECL_NOEXCEPT
{
    return m_pipelineDescriptor;
}

void
Pass::setParentTechnique(Technique *value)
{
    m_techniquePtr = value;
}

void
Pass::setRenderTargetIndexOffset(size_t value)
{
    m_renderTargetIndexOffset = value;
}

void
Pass::setTechniqueScriptIndex(size_t value)
{
    m_techniqueScriptIndex = value;
}

bool
Pass::findVertexPreshaderRegisterIndex(const String &name, RegisterIndex &registerIndex) const
{
    const RegisterIndexMap &map = m_registerIndices.m_vertexPreshader;
    RegisterIndexMap::const_iterator it = map.find(name);
    const bool found = it != map.end();
    if (found) {
        registerIndex = it->second;
    }
    return found;
}

bool
Pass::findPixelPreshaderRegisterIndex(const String &name, RegisterIndex &registerIndex) const
{
    const RegisterIndexMap &map = m_registerIndices.m_pixelPreshader;
    RegisterIndexMap::const_iterator it = map.find(name);
    const bool found = it != map.end();
    if (found) {
        registerIndex = it->second;
    }
    return found;
}

bool
Pass::findVertexShaderRegisterIndex(const String &name, RegisterIndex &registerIndex) const
{
    const RegisterIndexMap &map = m_registerIndices.m_vertexShader;
    RegisterIndexMap::const_iterator it = map.find(name);
    const bool found = it != map.end();
    if (found) {
        registerIndex = it->second;
    }
    return found;
}

bool
Pass::findPixelShaderRegisterIndex(const String &name, RegisterIndex &registerIndex) const
{
    const RegisterIndexMap &map = m_registerIndices.m_pixelShader;
    RegisterIndexMap::const_iterator it = map.find(name);
    const bool found = it != map.end();
    if (found) {
        registerIndex = it->second;
    }
    return found;
}

bool
Pass::findVertexShaderSamplerRegisterIndex(const String &name, SamplerRegisterIndex::List &samplerIndices) const
{
    const SamplerRegisterIndexMap &samplers = m_registerIndices.m_vertexShaderSamplers;
    SamplerRegisterIndexMap::const_iterator it = samplers.find(name);
    const bool found = it != samplers.end();
    if (found) {
        samplerIndices = it->second.m_indices;
    }
    return found;
}

bool
Pass::findPixelShaderSamplerRegisterIndex(const String &name, SamplerRegisterIndex::List &samplerIndices) const
{
    const SamplerRegisterIndexMap &samplers = m_registerIndices.m_pixelShaderSamplers;
    SamplerRegisterIndexMap::const_iterator it = samplers.find(name);
    const bool found = it != samplers.end();
    if (found) {
        samplerIndices = it->second.m_indices;
    }
    return found;
}

} /* namespace effect */
} /* namespace nanoem */
