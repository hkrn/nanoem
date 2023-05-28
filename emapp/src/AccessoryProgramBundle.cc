/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/AccessoryProgramBundle.h"

#include "emapp/Accessory.h"
#include "emapp/Constants.h"
#include "emapp/Effect.h"
#include "emapp/ICamera.h"
#include "emapp/IImageView.h"
#include "emapp/ILight.h"
#include "emapp/PixelFormat.h"
#include "emapp/Project.h"
#include "emapp/ShadowCamera.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#include "bx/hash.h"

namespace nanoem {
namespace {
#include "emapp/private/shaders/accessory_color_fs_glsl_core33.h"
#include "emapp/private/shaders/accessory_color_fs_glsl_es3.h"
#include "emapp/private/shaders/accessory_color_fs_msl_macos.h"
#include "emapp/private/shaders/accessory_color_fs_spirv.h"
#include "emapp/private/shaders/accessory_color_ps_dxbc.h"
#include "emapp/private/shaders/accessory_color_vs_dxbc.h"
#include "emapp/private/shaders/accessory_color_vs_glsl_core33.h"
#include "emapp/private/shaders/accessory_color_vs_glsl_es3.h"
#include "emapp/private/shaders/accessory_color_vs_msl_macos.h"
#include "emapp/private/shaders/accessory_color_vs_spirv.h"
#include "emapp/private/shaders/accessory_ground_shadow_fs_glsl_core33.h"
#include "emapp/private/shaders/accessory_ground_shadow_fs_glsl_es3.h"
#include "emapp/private/shaders/accessory_ground_shadow_fs_msl_macos.h"
#include "emapp/private/shaders/accessory_ground_shadow_fs_spirv.h"
#include "emapp/private/shaders/accessory_ground_shadow_ps_dxbc.h"
#include "emapp/private/shaders/accessory_ground_shadow_vs_dxbc.h"
#include "emapp/private/shaders/accessory_ground_shadow_vs_glsl_core33.h"
#include "emapp/private/shaders/accessory_ground_shadow_vs_glsl_es3.h"
#include "emapp/private/shaders/accessory_ground_shadow_vs_msl_macos.h"
#include "emapp/private/shaders/accessory_ground_shadow_vs_spirv.h"
static const char *const kPrefixName = "@nanoem/AccessoryProgramBundle";
} /* namespace anonymous */

AccessoryProgramBundle::AccessoryProgramBundle()
    : m_objectTechnique(nullptr)
    , m_groundShadowTechnique(nullptr)
{
}

AccessoryProgramBundle::~AccessoryProgramBundle() NANOEM_DECL_NOEXCEPT
{
    destroy();
}

ITechnique *
AccessoryProgramBundle::findTechnique(const String &passType, const nanodxm_material_t *material,
    nanoem_rsize_t materialIndex, nanoem_rsize_t numMaterials, Accessory *accessory)
{
    BX_UNUSED_2(materialIndex, numMaterials);
    const Project *project = accessory->project();
    BaseTechnique *foundTechnique = nullptr;
    if (passType == Effect::kPassTypeObject || passType == Effect::kPassTypeObjectSelfShadow) {
        if (!m_objectTechnique) {
            m_objectTechnique = nanoem_new(ObjectTechnique(project, material, this));
        }
        foundTechnique = m_objectTechnique;
    }
    else if (passType == Effect::kPassTypeShadow) {
        if (!m_groundShadowTechnique) {
            m_groundShadowTechnique = nanoem_new(GroundShadowTechnique(project, this));
        }
        foundTechnique = m_groundShadowTechnique;
    }
    if (foundTechnique) {
        foundTechnique->setActiveAccessory(accessory);
    }
    return foundTechnique;
}

ITechnique *
AccessoryProgramBundle::findTechnique(const String &passType, const nanoem_model_material_t *material,
    nanoem_rsize_t materialIndex, nanoem_rsize_t numMaterials, Model *model)
{
    BX_UNUSED_5(passType, material, materialIndex, numMaterials, model);
    nanoem_assert(0, "must NOT be called");
    return nullptr;
}

void
AccessoryProgramBundle::createImageResource(const void *ptr, size_t size, const ImageResourceParameter &parameter)
{
    BX_UNUSED_3(ptr, size, parameter);
}

void
AccessoryProgramBundle::setAllParameterObjects(
    const nanoem_motion_effect_parameter_t *const *parameters, nanoem_rsize_t numParameters)
{
    BX_UNUSED_2(parameters, numParameters);
}

void
AccessoryProgramBundle::setAllParameterObjects(const nanoem_motion_effect_parameter_t *const *fromParameters,
    nanoem_rsize_t numFromParameters, const nanoem_motion_effect_parameter_t *const *toParameters,
    nanoem_rsize_t numToParameters, nanoem_f32_t coefficient)
{
    BX_UNUSED_5(fromParameters, numFromParameters, toParameters, numToParameters, coefficient);
}

IEffect::ImageResourceList
AccessoryProgramBundle::allImageResources() const
{
    return ImageResourceList();
}

IEffect::ScriptClassType
AccessoryProgramBundle::scriptClass() const NANOEM_DECL_NOEXCEPT
{
    return kScriptClassTypeObject;
}

IEffect::ScriptOrderType
AccessoryProgramBundle::scriptOrder() const NANOEM_DECL_NOEXCEPT
{
    return kScriptOrderTypeStandard;
}

bool
AccessoryProgramBundle::hasScriptExternal() const NANOEM_DECL_NOEXCEPT
{
    return false;
}

void
AccessoryProgramBundle::destroy()
{
    nanoem_delete_safe(m_objectTechnique);
    nanoem_delete_safe(m_groundShadowTechnique);
}

AccessoryProgramBundle::CommonPass::CommonPass(BaseTechnique *parent)
    : m_parentTechnique(parent)
{
    Inline::clearZeroMemory(m_bindings);
    Inline::clearZeroMemory(m_uniformBuffer);
}

AccessoryProgramBundle::CommonPass::~CommonPass() NANOEM_DECL_NOEXCEPT
{
}

void
AccessoryProgramBundle::CommonPass::setGlobalParameters(const IDrawable *drawable, const Project *project)
{
    BX_UNUSED_2(drawable, project);
}

void
AccessoryProgramBundle::CommonPass::setCameraParameters(const ICamera *camera, const Matrix4x4 &world)
{
    nanoem_parameter_assert(camera, "must not be nullptr");
    Matrix4x4 viewMatrix, projectionMatrix;
    camera->getViewTransform(viewMatrix, projectionMatrix);
    const Matrix4x4 &modelViewMatrix = viewMatrix * world,
                    &modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;
    memcpy(glm::value_ptr(m_uniformBuffer[kUniformBufferModelMatrix]), glm::value_ptr(world), sizeof(world));
    memcpy(glm::value_ptr(m_uniformBuffer[kUniformBufferModelViewMatrix]), glm::value_ptr(modelViewMatrix),
        sizeof(modelViewMatrix));
    memcpy(glm::value_ptr(m_uniformBuffer[kUniformBufferModelViewProjectionMatrix]),
        glm::value_ptr(modelViewProjectionMatrix), sizeof(modelViewProjectionMatrix));
    m_uniformBuffer[kUniformBufferCameraPosition] = Vector4(camera->position(), 0);
}

void
AccessoryProgramBundle::CommonPass::setLightParameters(const ILight *light, bool adjustment)
{
    BX_UNUSED_1(adjustment);
    nanoem_parameter_assert(light, "must not be nullptr");
    m_uniformBuffer[kUniformBufferLightColor] = Vector4(light->color(), 1.0f);
    m_uniformBuffer[kUniformBufferLightDirection] = Vector4(light->direction(), 0);
}

void
AccessoryProgramBundle::CommonPass::setAllAccessoryParameters(const Accessory *accessory, const Project *project)
{
    BX_UNUSED_2(accessory, project);
}

void
AccessoryProgramBundle::CommonPass::setAllModelParameters(const Model *model, const Project *project)
{
    BX_UNUSED_2(model, project);
}

void
AccessoryProgramBundle::CommonPass::setMaterialParameters(const nanodxm_material_t *material)
{
    nanoem_parameter_assert(material, "must not be nullptr");
    const nanodxm_color_t &emissive = nanodxmMaterialGetEmissive(material);
    nanodxm_color_t diffuse = nanodxmMaterialGetDiffuse(material);
    Accessory *accessory = m_parentTechnique->accessory();
    diffuse.a *= accessory->opacity();
    nanodxm_color_t specular = nanodxmMaterialGetSpecular(material);
    nanodxm_color_t newSpecular = { specular.r * 0.1f, specular.g * 0.1f, specular.b * 0.1f,
        glm::max(nanodxmMaterialGetShininess(material), 1.0f) };
    m_uniformBuffer[kUniformBufferMaterialAmbient] = glm::make_vec4(reinterpret_cast<const nanoem_f32_t *>(&emissive));
    m_uniformBuffer[kUniformBufferMaterialDiffuse] = glm::make_vec4(reinterpret_cast<const nanoem_f32_t *>(&diffuse));
    m_uniformBuffer[kUniformBufferMaterialSpecular] =
        glm::make_vec4(reinterpret_cast<const nanoem_f32_t *>(&newSpecular));
    if (const Accessory::Material *materialData = accessory->findMaterial(material)) {
        const nanoem_model_material_sphere_map_texture_type_t textureType = materialData->sphereTextureMapType();
        const nanoem_f32_t sphereTextureTypes[] = {
            textureType == NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_MULTIPLY ? 1.0f : 0.0f,
            textureType == NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_SUB_TEXTURE ? 1.0f : 0.0f,
            textureType == NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_ADD ? 1.0f : 0.0f, 0
        };
        m_uniformBuffer[kUniformBufferSphereTextureType] = glm::make_vec4(sphereTextureTypes);
        sg_image image = accessory->project()->sharedFallbackImage();
        if (const IImageView *diffuseImage = materialData->diffuseImage()) {
            image = diffuseImage->handle();
        }
        m_bindings.fs_images[CommonPass::kDiffuseTextureSamplerStage] = image;
        m_bindings.fs_images[CommonPass::kSphereTextureSamplerStage] = image;
        m_uniformBuffer[kUniformBufferUseTextureSampler].x = materialData->diffuseImage() != nullptr;
        m_uniformBuffer[kUniformBufferUseTextureSampler].y =
            materialData->sphereTextureMapType() != NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE;
    }
    else {
        sg_image fallback = accessory->project()->sharedFallbackImage();
        m_bindings.fs_images[CommonPass::kDiffuseTextureSamplerStage] = fallback;
        m_bindings.fs_images[CommonPass::kSphereTextureSamplerStage] = fallback;
        m_uniformBuffer[kUniformBufferSphereTextureType] = m_uniformBuffer[kUniformBufferUseTextureSampler] =
            Constants::kZeroV4;
    }
    switch (m_parentTechnique->techniqueType()) {
    case kTechniqueTypeColor: {
        const nanodxm_color_t color = nanodxmMaterialGetEmissive(material);
        m_cullMode = color.a < 1.0 ? SG_CULLMODE_NONE : SG_CULLMODE_BACK;
        break;
    }
    case kTechniqueTypeEdge: {
        m_cullMode = SG_CULLMODE_FRONT;
        break;
    }
    case kTechniqueTypeGroundShadow: {
        m_cullMode = SG_CULLMODE_NONE;
        break;
    }
    default:
        break;
    }
}

void
AccessoryProgramBundle::CommonPass::setMaterialParameters(const nanoem_model_material_t *material)
{
    BX_UNUSED_1(material);
}

void
AccessoryProgramBundle::CommonPass::setEdgeParameters(const nanoem_model_material_t *material, nanoem_f32_t edgeSize)
{
    BX_UNUSED_2(material, edgeSize);
}

void
AccessoryProgramBundle::CommonPass::setGroundShadowParameters(
    const ILight *light, const ICamera *camera, const Matrix4x4 &world)
{
    nanoem_parameter_assert(light, "must not be nullptr");
    Matrix4x4 originShadowMatrix, viewMatrix, projectionMatrix;
    camera->getViewTransform(viewMatrix, projectionMatrix);
    light->getShadowTransform(originShadowMatrix);
    const Matrix4x4 shadowMatrix(originShadowMatrix * world);
    memcpy(
        glm::value_ptr(m_uniformBuffer[kUniformBufferModelMatrix]), glm::value_ptr(shadowMatrix), sizeof(shadowMatrix));
    const Matrix4x4 shadowViewMatrix(viewMatrix * shadowMatrix);
    memcpy(glm::value_ptr(m_uniformBuffer[kUniformBufferModelViewMatrix]), glm::value_ptr(shadowViewMatrix),
        sizeof(shadowViewMatrix));
    const Matrix4x4 shadowViewProjectionMatrix(projectionMatrix * shadowViewMatrix);
    memcpy(glm::value_ptr(m_uniformBuffer[kUniformBufferModelViewProjectionMatrix]),
        glm::value_ptr(shadowViewProjectionMatrix), sizeof(shadowViewProjectionMatrix));
    m_uniformBuffer[kUniformBufferLightColor] =
        Vector4(light->groundShadowColor(), 1.0f + light->isTranslucentGroundShadowEnabled() * -0.5f);
    m_bindings.fs_images[CommonPass::kShadowMapTextureSamplerStage0] =
        m_parentTechnique->accessory()->project()->sharedFallbackImage();
}

void
AccessoryProgramBundle::CommonPass::setShadowMapParameters(const ShadowCamera *shadowCamera, const Matrix4x4 &world)
{
    BX_UNUSED_2(shadowCamera, world);
    nanoem_parameter_assert(shadowCamera, "must NOT be nullptr");
    m_bindings.fs_images[CommonPass::kShadowMapTextureSamplerStage0] =
        m_parentTechnique->accessory()->project()->sharedFallbackImage();
}

void
AccessoryProgramBundle::CommonPass::execute(const IDrawable * /* drawable */, const Buffer &buffer)
{
    Accessory *accessory = m_parentTechnique->accessory();
    Project *project = accessory->project();
    const bool isAddBlend = accessory->isAddBlendEnabled(), isDepthEnabled = buffer.m_depthEnabled,
               isOffscreenRenderPassActive = project->isOffscreenRenderPassActive();
    const sg_pass pass =
        isOffscreenRenderPassActive ? project->currentOffscreenRenderPass() : project->currentRenderPass();
    const PixelFormat format(project->findRenderPassPixelFormat(pass, project->sampleCount()));
    const TechniqueType techniqueType = m_parentTechnique->techniqueType();
    bx::HashMurmur2A hash;
    hash.begin();
    hash.add(m_cullMode);
    hash.add(techniqueType);
    hash.add(isAddBlend);
    hash.add(isDepthEnabled);
    hash.add(isOffscreenRenderPassActive);
    format.addHash(hash);
    nanoem_u32_t key = hash.end();
    PipelineMap::const_iterator it = m_pipelines.find(key);
    sg_pipeline pipeline;
    if (it != m_pipelines.end()) {
        pipeline = it->second;
    }
    else {
        sg_pipeline_desc desc;
        Inline::clearZeroMemory(desc);
        Accessory::setStandardPipelineDescription(desc);
        desc.shader = m_parentTechnique->shader();
        desc.color_count = format.numColorAttachments();
        sg_color_state &cs = desc.colors[0];
        cs.pixel_format = format.colorPixelFormat(0);
        sg_depth_state &ds = desc.depth;
        ds.compare = isDepthEnabled ? SG_COMPAREFUNC_LESS_EQUAL : SG_COMPAREFUNC_ALWAYS;
        ds.pixel_format = format.depthPixelFormat();
        ds.write_enabled = isDepthEnabled;
        if (isAddBlend) {
            Project::setAddBlendMode(cs);
        }
        else {
            Project::setAlphaBlendMode(cs);
        }
        if (!project->isRenderPassViewport()) {
            cs.write_mask = SG_COLORMASK_RGBA;
        }
        if (techniqueType == kTechniqueTypeGroundShadow && isDepthEnabled) {
            Project::setShadowDepthStencilState(desc.depth, desc.stencil);
        }
        desc.cull_mode = m_cullMode;
        desc.sample_count = format.numSamples();
        char label[Inline::kMarkerStringLength];
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(
                label, sizeof(label), "%s/Pipelines/%d", kPrefixName, Inline::saturateInt32(m_pipelines.size()));
            desc.label = label;
        }
        else {
            *label = 0;
        }
        pipeline = sg::make_pipeline(&desc);
        nanoem_assert(sg::query_pipeline_state(pipeline) == SG_RESOURCESTATE_VALID, "pipeline must be valid");
        project->setRenderPipelineName(pipeline, label);
        m_pipelines.insert(tinystl::make_pair(key, pipeline));
    }
    m_bindings.vertex_buffers[0] = buffer.m_vertexBuffer;
    m_bindings.index_buffer = buffer.m_indexBuffer;
    sg_pass_action action;
    sg::PassBlock::initializeLoadStoreAction(action);
    {
        SG_PUSH_GROUPF("AccessoryProgramBundle::CommonPass::execute(pass=%s)", project->findRenderPassName(pass));
        sg::PassBlock pb(project->sharedBatchDrawQueue(), project->beginRenderPass(pass), action);
        pb.applyPipelineBindings(pipeline, m_bindings);
        pb.applyUniformBlock(m_uniformBuffer, sizeof(m_uniformBuffer));
        pb.draw(buffer.m_offset, buffer.m_numIndices);
        SG_POP_GROUP();
    }
    Inline::clearZeroMemory(m_bindings);
}

AccessoryProgramBundle::BaseTechnique::BaseTechnique(AccessoryProgramBundle *parent)
    : m_parentPtr(parent)
    , m_accessory(nullptr)
    , m_pass(nullptr)
    , m_techniqueType(kTechniqueTypeFirstEnum)
    , m_executed(false)
{
    nanoem_assert(m_parentPtr, "must NOT be nullptr");
    m_shader = { SG_INVALID_ID };
    m_pass = nanoem_new(CommonPass(this));
}

AccessoryProgramBundle::BaseTechnique::~BaseTechnique() NANOEM_DECL_NOEXCEPT
{
    nanoem_delete_safe(m_pass);
    sg::destroy_shader(m_shader);
}

void
AccessoryProgramBundle::BaseTechnique::resetScriptCommandState()
{
}

void
AccessoryProgramBundle::BaseTechnique::resetScriptExternalColor()
{
}

bool
AccessoryProgramBundle::BaseTechnique::hasNextScriptCommand() const NANOEM_DECL_NOEXCEPT
{
    return false;
}

void
AccessoryProgramBundle::BaseTechnique::setActiveAccessory(Accessory *accessory)
{
    nanoem_parameter_assert(accessory, "must NOT be nullptr");
    m_accessory = accessory;
    m_executed = false;
}

Accessory *
AccessoryProgramBundle::BaseTechnique::accessory() NANOEM_DECL_NOEXCEPT
{
    return m_accessory;
}

AccessoryProgramBundle::TechniqueType
AccessoryProgramBundle::BaseTechnique::techniqueType() const NANOEM_DECL_NOEXCEPT
{
    return m_techniqueType;
}

sg_shader
AccessoryProgramBundle::BaseTechnique::shader() const NANOEM_DECL_NOEXCEPT
{
    return m_shader;
}

void
AccessoryProgramBundle::BaseTechnique::setupUniformBlock(const char *name, sg_shader_uniform_block_desc &desc)
{
    desc.size = sizeof(Vector4) * CommonPass::kUniformBufferMaxEnum;
#if defined(NANOEM_ENABLE_SHADER_OPTIMIZED)
    desc.uniforms[0] = sg_shader_uniform_desc { name, SG_UNIFORMTYPE_FLOAT4, CommonPass::kUniformBufferMaxEnum };
#else
    BX_UNUSED_1(name);
    desc.uniforms[0] =
        sg_shader_uniform_desc { "accessory_parameters_t", SG_UNIFORMTYPE_FLOAT4, CommonPass::kUniformBufferMaxEnum };
#endif /* NANOEM_ENABLE_SHADER_OPTIMIZED */
}

void
AccessoryProgramBundle::BaseTechnique::setupShader(const char *vs, const char *fs, sg_shader_desc &desc)
{
    setupUniformBlock(vs, desc.vs.uniform_blocks[0]);
    setupUniformBlock(fs, desc.fs.uniform_blocks[0]);
    desc.vs.entry = "nanoemVSMain";
    desc.fs.entry = "nanoemPSMain";
#if defined(NANOEM_ENABLE_SHADER_OPTIMIZED)
    desc.fs.images[CommonPass::kShadowMapTextureSamplerStage0] =
        sg_shader_image_desc { "SPIRV_Cross_Combined_2", SG_IMAGETYPE_2D, SG_SAMPLERTYPE_FLOAT };
    desc.fs.images[CommonPass::kDiffuseTextureSamplerStage] =
        sg_shader_image_desc { "SPIRV_Cross_Combined", SG_IMAGETYPE_2D, SG_SAMPLERTYPE_FLOAT };
    desc.fs.images[CommonPass::kSphereTextureSamplerStage] =
        sg_shader_image_desc { "SPIRV_Cross_Combined_1", SG_IMAGETYPE_2D, SG_SAMPLERTYPE_FLOAT };
#else
    desc.fs.images[CommonPass::kShadowMapTextureSamplerStage0] =
        sg_shader_image_desc { "SPIRV_Cross_Combinedu_shadowTextureu_shadowTextureSampler", SG_IMAGETYPE_2D,
            SG_SAMPLERTYPE_FLOAT };
    desc.fs.images[CommonPass::kDiffuseTextureSamplerStage] =
        sg_shader_image_desc { "SPIRV_Cross_Combinedu_diffuseTextureu_diffuseTextureSampler", SG_IMAGETYPE_2D,
            SG_SAMPLERTYPE_FLOAT };
    desc.fs.images[CommonPass::kSphereTextureSamplerStage] =
        sg_shader_image_desc { "SPIRV_Cross_Combinedu_spheremapTextureu_spheremapTextureSampler", SG_IMAGETYPE_2D,
            SG_SAMPLERTYPE_FLOAT };
#endif
    desc.attrs[0] = sg_shader_attr_desc { "a_position", "SV_POSITION", 0 };
    desc.attrs[1] = sg_shader_attr_desc { "a_normal", "NORMAL", 0 };
    desc.attrs[2] = sg_shader_attr_desc { "a_texcoord0", "TEXCOORD", 0 };
    desc.attrs[3] = sg_shader_attr_desc { "a_texcoord1", "TEXCOORD", 1 };
    desc.attrs[4] = sg_shader_attr_desc { "a_texcoord2", "TEXCOORD", 2 };
    desc.attrs[5] = sg_shader_attr_desc { "a_texcoord3", "TEXCOORD", 3 };
    desc.attrs[6] = sg_shader_attr_desc { "a_texcoord4", "TEXCOORD", 4 };
    desc.attrs[7] = sg_shader_attr_desc { "a_color0", "COLOR", 0 };
}

AccessoryProgramBundle::ObjectTechnique::ObjectTechnique(
    const Project *project, const nanodxm_material_t *material, AccessoryProgramBundle *parent)
    : AccessoryProgramBundle::BaseTechnique(parent)
{
    m_techniqueType = kTechniqueTypeColor;
    BX_UNUSED_2(project, material);
}

AccessoryProgramBundle::ObjectTechnique::~ObjectTechnique() NANOEM_DECL_NOEXCEPT
{
}

IPass *
AccessoryProgramBundle::ObjectTechnique::execute(const IDrawable * /* drawable */, bool /* scriptExternalColor */)
{
    CommonPass *state = nullptr;
    if (!m_executed) {
        if (!sg::is_valid(m_shader)) {
            sg_shader_desc sd;
            Inline::clearZeroMemory(sd);
            const sg_backend backend = sg::query_backend();
            if (backend == SG_BACKEND_D3D11) {
                sd.fs.bytecode.ptr = g_nanoem_accessory_color_ps_dxbc_data;
                sd.fs.bytecode.size = g_nanoem_accessory_color_ps_dxbc_size;
                sd.vs.bytecode.ptr = g_nanoem_accessory_color_vs_dxbc_data;
                sd.vs.bytecode.size = g_nanoem_accessory_color_vs_dxbc_size;
            }
            else if (sg::is_backend_metal(backend)) {
                sd.fs.bytecode.ptr = g_nanoem_accessory_color_fs_msl_macos_data;
                sd.fs.bytecode.size = g_nanoem_accessory_color_fs_msl_macos_size;
                sd.vs.bytecode.ptr = g_nanoem_accessory_color_vs_msl_macos_data;
                sd.vs.bytecode.size = g_nanoem_accessory_color_vs_msl_macos_size;
            }
            else if (backend == SG_BACKEND_GLCORE33) {
                sd.fs.source = reinterpret_cast<const char *>(g_nanoem_accessory_color_fs_glsl_core33_data);
                sd.vs.source = reinterpret_cast<const char *>(g_nanoem_accessory_color_vs_glsl_core33_data);
            }
            else if (backend == SG_BACKEND_GLES3) {
                sd.fs.source = reinterpret_cast<const char *>(g_nanoem_accessory_color_fs_glsl_es3_data);
                sd.vs.source = reinterpret_cast<const char *>(g_nanoem_accessory_color_vs_glsl_es3_data);
            }
            setupShader("_41", "_35", sd);
            char label[Inline::kMarkerStringLength];
            if (Inline::isDebugLabelEnabled()) {
                StringUtils::format(label, sizeof(label), "%s/ObjectTechnique", kPrefixName);
                sd.label = label;
            }
            m_shader = sg::make_shader(&sd);
            nanoem_assert(sg::query_shader_state(m_shader) == SG_RESOURCESTATE_VALID, "shader must be valid");
            SG_LABEL_SHADER(m_shader, label);
        }
        state = m_pass;
        m_executed = true;
    }
    return state;
}

AccessoryProgramBundle::GroundShadowTechnique::GroundShadowTechnique(
    const Project *project, AccessoryProgramBundle *parent)
    : AccessoryProgramBundle::BaseTechnique(parent)
{
    m_techniqueType = kTechniqueTypeGroundShadow;
    BX_UNUSED_1(project);
}

AccessoryProgramBundle::GroundShadowTechnique::~GroundShadowTechnique() NANOEM_DECL_NOEXCEPT
{
}

IPass *
AccessoryProgramBundle::GroundShadowTechnique::execute(const IDrawable * /* drawable */, bool /* scriptExternalColor */)
{
    CommonPass *state = nullptr;
    if (!m_executed) {
        if (!sg::is_valid(m_shader)) {
            sg_shader_desc sd;
            Inline::clearZeroMemory(sd);
            const sg_backend backend = sg::query_backend();
            if (backend == SG_BACKEND_D3D11) {
                sd.fs.bytecode.ptr = g_nanoem_accessory_ground_shadow_ps_dxbc_data;
                sd.fs.bytecode.size = g_nanoem_accessory_ground_shadow_ps_dxbc_size;
                sd.vs.bytecode.ptr = g_nanoem_accessory_ground_shadow_vs_dxbc_data;
                sd.vs.bytecode.size = g_nanoem_accessory_ground_shadow_vs_dxbc_size;
            }
            else if (sg::is_backend_metal(backend)) {
                sd.fs.bytecode.ptr = g_nanoem_accessory_ground_shadow_fs_msl_macos_data;
                sd.fs.bytecode.size = g_nanoem_accessory_ground_shadow_fs_msl_macos_size;
                sd.vs.bytecode.ptr = g_nanoem_accessory_ground_shadow_vs_msl_macos_data;
                sd.vs.bytecode.size = g_nanoem_accessory_ground_shadow_vs_msl_macos_size;
            }
            else if (backend == SG_BACKEND_GLCORE33) {
                sd.fs.source = reinterpret_cast<const char *>(g_nanoem_accessory_ground_shadow_fs_glsl_core33_data);
                sd.vs.source = reinterpret_cast<const char *>(g_nanoem_accessory_ground_shadow_vs_glsl_core33_data);
            }
            else if (backend == SG_BACKEND_GLES3) {
                sd.fs.source = reinterpret_cast<const char *>(g_nanoem_accessory_ground_shadow_fs_glsl_es3_data);
                sd.vs.source = reinterpret_cast<const char *>(g_nanoem_accessory_ground_shadow_vs_glsl_es3_data);
            }
            setupShader("_42", "_37", sd);
            char label[Inline::kMarkerStringLength];
            if (Inline::isDebugLabelEnabled()) {
                StringUtils::format(label, sizeof(label), "%s/GroundShadowTechnique", kPrefixName);
                sd.label = label;
            }
            m_shader = sg::make_shader(&sd);
            nanoem_assert(sg::query_shader_state(m_shader) == SG_RESOURCESTATE_VALID, "shader must be valid");
            SG_LABEL_SHADER(m_shader, label);
        }
        state = m_pass;
        m_executed = true;
    }
    return state;
}

} /* namespace nanoem */
