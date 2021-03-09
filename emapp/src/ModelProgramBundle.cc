/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/ModelProgramBundle.h"

#include "emapp/Effect.h"
#include "emapp/ICamera.h"
#include "emapp/IImageView.h"
#include "emapp/ILight.h"
#include "emapp/Model.h"
#include "emapp/PixelFormat.h"
#include "emapp/Project.h"
#include "emapp/ResourceBundle.h"
#include "emapp/ShadowCamera.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#include "bx/hash.h"

namespace nanoem {
namespace {
#include "emapp/private/shaders/model_color_fs_glsl_core33.h"
#include "emapp/private/shaders/model_color_fs_glsl_es3.h"
#include "emapp/private/shaders/model_color_fs_msl_macos.h"
#include "emapp/private/shaders/model_color_fs_spirv.h"
#include "emapp/private/shaders/model_color_ps_dxbc.h"
#include "emapp/private/shaders/model_color_vs_dxbc.h"
#include "emapp/private/shaders/model_color_vs_glsl_core33.h"
#include "emapp/private/shaders/model_color_vs_glsl_es3.h"
#include "emapp/private/shaders/model_color_vs_msl_macos.h"
#include "emapp/private/shaders/model_color_vs_spirv.h"
#include "emapp/private/shaders/model_edge_fs_glsl_core33.h"
#include "emapp/private/shaders/model_edge_fs_glsl_es3.h"
#include "emapp/private/shaders/model_edge_fs_msl_macos.h"
#include "emapp/private/shaders/model_edge_fs_spirv.h"
#include "emapp/private/shaders/model_edge_ps_dxbc.h"
#include "emapp/private/shaders/model_edge_vs_dxbc.h"
#include "emapp/private/shaders/model_edge_vs_glsl_core33.h"
#include "emapp/private/shaders/model_edge_vs_glsl_es3.h"
#include "emapp/private/shaders/model_edge_vs_msl_macos.h"
#include "emapp/private/shaders/model_edge_vs_spirv.h"
#include "emapp/private/shaders/model_ground_shadow_fs_glsl_core33.h"
#include "emapp/private/shaders/model_ground_shadow_fs_glsl_es3.h"
#include "emapp/private/shaders/model_ground_shadow_fs_msl_macos.h"
#include "emapp/private/shaders/model_ground_shadow_fs_spirv.h"
#include "emapp/private/shaders/model_ground_shadow_ps_dxbc.h"
#include "emapp/private/shaders/model_ground_shadow_vs_dxbc.h"
#include "emapp/private/shaders/model_ground_shadow_vs_glsl_core33.h"
#include "emapp/private/shaders/model_ground_shadow_vs_glsl_es3.h"
#include "emapp/private/shaders/model_ground_shadow_vs_msl_macos.h"
#include "emapp/private/shaders/model_ground_shadow_vs_spirv.h"
#include "emapp/private/shaders/model_zplot_fs_glsl_core33.h"
#include "emapp/private/shaders/model_zplot_fs_glsl_es3.h"
#include "emapp/private/shaders/model_zplot_fs_msl_macos.h"
#include "emapp/private/shaders/model_zplot_fs_spirv.h"
#include "emapp/private/shaders/model_zplot_ps_dxbc.h"
#include "emapp/private/shaders/model_zplot_vs_dxbc.h"
#include "emapp/private/shaders/model_zplot_vs_glsl_core33.h"
#include "emapp/private/shaders/model_zplot_vs_glsl_es3.h"
#include "emapp/private/shaders/model_zplot_vs_msl_macos.h"
#include "emapp/private/shaders/model_zplot_vs_spirv.h"
#include "emapp/private/shaders/pointed_model_color_vs_msl_macos.h"
const char *const kPrefixName = "@nanoem/ModelProgramBundle";
} /* namespace anonymous */

ModelProgramBundle::ModelProgramBundle()
    : m_objectTechnique(nullptr)
    , m_edgeTechnique(nullptr)
    , m_groundShadowTechnique(nullptr)
    , m_zplotTechnique(nullptr)
{
}

ModelProgramBundle::~ModelProgramBundle() NANOEM_DECL_NOEXCEPT
{
    destroy();
}

ITechnique *
ModelProgramBundle::findTechnique(const String &passType, const nanodxm_material_t *material,
    nanoem_rsize_t materialIndex, nanoem_rsize_t numMaterials, Accessory *accessory)
{
    BX_UNUSED_5(passType, material, materialIndex, numMaterials, accessory);
    nanoem_assert(0, "must NOT be called");
    return nullptr;
}

ITechnique *
ModelProgramBundle::findTechnique(const String &passType, const nanoem_model_material_t *material,
    nanoem_rsize_t materialIndex, nanoem_rsize_t numMaterials, Model *model)
{
    BX_UNUSED_4(material, materialIndex, numMaterials, model);
    BaseTechnique *foundTechnique = nullptr;
    const model::Material *m = model::Material::cast(material);
    if (m && m->isVisible()) {
        if (passType == Effect::kPassTypeObject || passType == Effect::kPassTypeObjectSelfShadow) {
            const bool isPointDraw = nanoemModelMaterialIsPointDrawEnabled(material);
            if (!m_objectTechnique) {
                m_objectTechnique = nanoem_new(ObjectTechnique(this, isPointDraw));
            }
            else if (m_objectTechnique && isPointDraw != m_objectTechnique->isPointDrawEnabled()) {
                nanoem_delete(m_objectTechnique);
                m_objectTechnique = nanoem_new(ObjectTechnique(this, isPointDraw));
            }
            foundTechnique = m_objectTechnique;
        }
        else if (passType == Effect::kPassTypeEdge) {
            if (!m_edgeTechnique) {
                m_edgeTechnique = nanoem_new(EdgeTechnique(this));
            }
            foundTechnique = m_edgeTechnique;
        }
        else if (passType == Effect::kPassTypeShadow) {
            if (!m_groundShadowTechnique) {
                m_groundShadowTechnique = nanoem_new(GroundShadowTechnique(this));
            }
            foundTechnique = m_groundShadowTechnique;
        }
        else if (passType == Effect::kPassTypeZplot) {
            if (!m_zplotTechnique) {
                m_zplotTechnique = nanoem_new(ZplotTechnique(this));
            }
            foundTechnique = m_zplotTechnique;
        }
        if (foundTechnique) {
            foundTechnique->setActiveModel(model);
        }
    }
    return foundTechnique;
}

void
ModelProgramBundle::createImageResource(const void *ptr, size_t size, const ImageResourceParameter &parameter)
{
    BX_UNUSED_3(ptr, size, parameter);
}

void
ModelProgramBundle::setAllParameterObjects(
    const nanoem_motion_effect_parameter_t *const *parameters, nanoem_rsize_t numParameters)
{
    BX_UNUSED_2(parameters, numParameters);
}

void
ModelProgramBundle::setAllParameterObjects(const nanoem_motion_effect_parameter_t *const *fromParameters,
    nanoem_rsize_t numFromParameters, const nanoem_motion_effect_parameter_t *const *toParameters,
    nanoem_rsize_t numToParameters, nanoem_f32_t coefficient)
{
    BX_UNUSED_5(fromParameters, numFromParameters, toParameters, numToParameters, coefficient);
}

IEffect::ImageResourceList
ModelProgramBundle::allImageResources() const
{
    return ImageResourceList();
}

IEffect::ScriptClassType
ModelProgramBundle::scriptClass() const NANOEM_DECL_NOEXCEPT
{
    return kScriptClassTypeObject;
}

IEffect::ScriptOrderType
ModelProgramBundle::scriptOrder() const NANOEM_DECL_NOEXCEPT
{
    return kScriptOrderTypeStandard;
}

bool
ModelProgramBundle::hasScriptExternal() const NANOEM_DECL_NOEXCEPT
{
    return false;
}

void
ModelProgramBundle::destroy()
{
    nanoem_delete_safe(m_objectTechnique);
    nanoem_delete_safe(m_edgeTechnique);
    nanoem_delete_safe(m_groundShadowTechnique);
    nanoem_delete_safe(m_zplotTechnique);
}

ModelProgramBundle::CommonPass::CommonPass(BaseTechnique *parent)
    : m_parentTechnique(parent)
    , m_cullMode(_SG_CULLMODE_DEFAULT)
    , m_primitiveType(SG_PRIMITIVETYPE_TRIANGLES)
    , m_opacity(1.0f)
{
    Inline::clearZeroMemory(m_bindings);
    Inline::clearZeroMemory(m_uniformBuffer);
}

ModelProgramBundle::CommonPass::~CommonPass() NANOEM_DECL_NOEXCEPT
{
    for (PipelineMap::const_iterator it = m_pipelines.begin(), end = m_pipelines.end(); it != end; ++it) {
        sg::destroy_pipeline(it->second);
    }
    m_opacity = 0.0f;
}

void
ModelProgramBundle::CommonPass::setGlobalParameters(const IDrawable *drawable, const Project *project)
{
    BX_UNUSED_2(drawable, project);
}

void
ModelProgramBundle::CommonPass::setCameraParameters(const ICamera *camera, const Matrix4x4 &world)
{
    nanoem_parameter_assert(camera, "must not be nullptr");
    Matrix4x4 v, p;
    camera->getViewTransform(v, p);
    const Matrix4x4 w(m_parentTechnique->model()->worldTransform(world));
    memcpy(glm::value_ptr(m_uniformBuffer[kUniformBufferModelMatrix]), glm::value_ptr(w), sizeof(w));
    memcpy(glm::value_ptr(m_uniformBuffer[kUniformBufferModelViewMatrix]), glm::value_ptr(v * w), sizeof(w));
    memcpy(
        glm::value_ptr(m_uniformBuffer[kUniformBufferModelViewProjectionMatrix]), glm::value_ptr(p * v * w), sizeof(w));
    m_uniformBuffer[kUniformBufferCameraPosition] = Vector4(camera->position(), 0);
}

void
ModelProgramBundle::CommonPass::setLightParameters(const ILight *light, bool adjustment)
{
    BX_UNUSED_1(adjustment);
    nanoem_parameter_assert(light, "must not be nullptr");
    m_uniformBuffer[kUniformBufferLightColor] = Vector4(light->color(), 1);
    m_uniformBuffer[kUniformBufferLightDirection] = Vector4(light->direction(), 0);
}

void
ModelProgramBundle::CommonPass::setAllAccessoryParameters(const Accessory *accessory, const Project *project)
{
    BX_UNUSED_2(accessory, project);
}

void
ModelProgramBundle::CommonPass::setAllModelParameters(const Model *model, const Project *project)
{
    BX_UNUSED_1(project);
    m_opacity = model->opacity();
}

void
ModelProgramBundle::CommonPass::setMaterialParameters(const nanodxm_material_t *material)
{
    BX_UNUSED_1(material);
}

void
ModelProgramBundle::CommonPass::setMaterialParameters(const nanoem_model_material_t *materialPtr)
{
    nanoem_parameter_assert(materialPtr, "must not be nullptr");
    const model::Material *material = model::Material::cast(materialPtr);
    const model::Material::Color &color = material->color();
    m_uniformBuffer[kUniformBufferMaterialAmbient] = Vector4(color.m_ambient, 1.0f);
    m_uniformBuffer[kUniformBufferMaterialDiffuse] = Vector4(color.m_diffuse, color.m_diffuseOpacity * m_opacity);
    m_uniformBuffer[kUniformBufferMaterialSpecular] = Vector4(color.m_specular, color.m_specularPower);
    m_uniformBuffer[kUniformBufferDiffuseTextureBlendFactor] = color.m_diffuseTextureBlendFactor;
    m_uniformBuffer[kUniformBufferSphereTextureBlendFactor] = color.m_sphereTextureBlendFactor;
    m_uniformBuffer[kUniformBufferToonTextureBlendFactor] = color.m_toonTextureBlendFactor;
    const model::Material *userData = model::Material::cast(materialPtr);
    const nanoem_model_material_sphere_map_texture_type_t &textureType = userData->sphereMapImage() != nullptr
        ? nanoemModelMaterialGetSphereMapTextureType(materialPtr)
        : NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE;
    const nanoem_f32_t sphereTextureType[] = {
        textureType == NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_MULTIPLY ? 1.0f : 0.0f,
        textureType == NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_SUB_TEXTURE ? 1.0f : 0.0f,
        textureType == NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_ADD ? 1.0f : 0.0f, 0
    };
    m_uniformBuffer[kUniformBufferSphereTextureType] = glm::make_vec4(sphereTextureType);
    const nanoem_f32_t enableVertexColor = nanoemModelMaterialIsVertexColorEnabled(materialPtr) ? 1.0f : 0.0f;
    m_uniformBuffer[kUniformBufferEnableVertexColor] = Vector4(enableVertexColor);
    setImage(userData->diffuseImage(), CommonPass::kDiffuseTextureSamplerStage,
        m_uniformBuffer[kUniformBufferUseTextureSampler].x);
    setImage(userData->sphereMapImage(), CommonPass::kSphereTextureSamplerStage,
        m_uniformBuffer[kUniformBufferUseTextureSampler].y);
    setImage(userData->toonImage(), CommonPass::kToonTextureSamplerStage,
        m_uniformBuffer[kUniformBufferUseTextureSampler].z);
    if (nanoemModelMaterialIsLineDrawEnabled(materialPtr)) {
        m_primitiveType = SG_PRIMITIVETYPE_LINES;
    }
    else if (nanoemModelMaterialIsPointDrawEnabled(materialPtr)) {
        m_primitiveType = SG_PRIMITIVETYPE_POINTS;
    }
    else {
        m_primitiveType = SG_PRIMITIVETYPE_TRIANGLES;
    }
    switch (m_parentTechnique->techniqueType()) {
    case kTechniqueTypeColor:
    case kTechniqueTypeZplot: {
        m_cullMode = nanoemModelMaterialIsCullingDisabled(materialPtr) ? SG_CULLMODE_NONE : SG_CULLMODE_BACK;
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
ModelProgramBundle::CommonPass::setEdgeParameters(const nanoem_model_material_t *materialPtr, nanoem_f32_t edgeSize)
{
    nanoem_parameter_assert(materialPtr, "must not be nullptr");
    const model::Material *material = model::Material::cast(materialPtr);
    const model::Material::Edge &edge = material->edge();
    const Vector4 edgeColor(edge.m_color, edge.m_opacity);
    m_uniformBuffer[kUniformBufferLightColor] = edgeColor;
    m_uniformBuffer[kUniformBufferLightDirection] = Vector4(edge.m_size * edgeSize);
    m_bindings.fs_images[CommonPass::kShadowMapTextureSamplerStage0] =
        m_parentTechnique->model()->project()->sharedFallbackImage();
}

void
ModelProgramBundle::CommonPass::setGroundShadowParameters(
    const ILight *light, const ICamera *camera, const Matrix4x4 &world)
{
    BX_UNUSED_2(camera, world);
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
        m_parentTechnique->model()->project()->sharedFallbackImage();
}

void
ModelProgramBundle::CommonPass::setShadowMapParameters(const ShadowCamera *shadowCamera, const Matrix4x4 &world)
{
    nanoem_parameter_assert(shadowCamera, "must NOT be nullptr");
    Matrix4x4 view, projection, crop;
    shadowCamera->getViewProjection(view, projection);
    shadowCamera->getCropMatrix(crop);
    const Matrix4x4 shadowMapMatrix(projection * view * world);
    memcpy(glm::value_ptr(m_uniformBuffer[kUniformBufferLightViewProjectionMatrix]),
        glm::value_ptr(crop * shadowMapMatrix), sizeof(crop));
    m_uniformBuffer[kUniformBufferShadowMapSize] =
        Vector4(shadowCamera->imageSize(), 0.005f, shadowCamera->coverageMode());
    m_uniformBuffer[kUniformBufferUseTextureSampler][3] = shadowCamera->isEnabled() ? 1.0f : 0.0f;
    sg_image colorImage;
    if (m_parentTechnique->techniqueType() == kTechniqueTypeZplot) {
        sg_image fallbackImage = shadowCamera->project()->sharedFallbackImage();
        m_bindings.fs_images[CommonPass::kDiffuseTextureSamplerStage] = fallbackImage;
        m_bindings.fs_images[CommonPass::kSphereTextureSamplerStage] = fallbackImage;
        m_bindings.fs_images[CommonPass::kToonTextureSamplerStage] = fallbackImage;
        memcpy(glm::value_ptr(m_uniformBuffer[kUniformBufferModelViewProjectionMatrix]),
            glm::value_ptr(shadowMapMatrix), sizeof(shadowMapMatrix));
        colorImage = m_parentTechnique->model()->project()->sharedFallbackImage();
    }
    else {
        colorImage = shadowCamera->colorImage();
    }
    m_bindings.fs_images[CommonPass::kShadowMapTextureSamplerStage0] = colorImage;
}

void
ModelProgramBundle::CommonPass::execute(const IDrawable * /* drawable */, const Buffer &buffer)
{
    Model *model = m_parentTechnique->model();
    Project *project = model->project();
    const bool isAddBlend = model->isAddBlendEnabled(),
               isOffscreenRenderPassActive = project->isOffscreenRenderPassActive();
    const sg_pass pass =
        isOffscreenRenderPassActive ? project->currentOffscreenRenderPass() : project->currentRenderPass();
    const PixelFormat &format = project->findRenderPassPixelFormat(pass);
    const TechniqueType techniqueType = m_parentTechnique->techniqueType();
    bx::HashMurmur2A hash;
    hash.begin();
    hash.add(m_cullMode);
    hash.add(m_primitiveType);
    hash.add(techniqueType);
    hash.add(isAddBlend);
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
        if (m_parentTechnique->techniqueType() == kTechniqueTypeEdge) {
            Model::setEdgePipelineDescription(desc);
        }
        else {
            Model::setStandardPipelineDescription(desc);
        }
        desc.shader = m_parentTechnique->shader();
        desc.primitive_type = m_primitiveType;
        desc.colors[0].pixel_format = format.m_colorPixelFormats[0];
        desc.depth.pixel_format = format.m_depthPixelFormat;
        desc.color_count = format.m_numColorAttachments;
        if (isAddBlend) {
            Project::setAddBlendMode(desc.colors[0]);
        }
        else {
            Project::setAlphaBlendMode(desc.colors[0]);
        }
        if (!project->isRenderPassViewport()) {
            desc.colors[0].write_mask = SG_COLORMASK_RGBA;
        }
        if (techniqueType == kTechniqueTypeGroundShadow) {
            Project::setShadowDepthStencilState(desc.depth, desc.stencil);
        }
        desc.cull_mode = m_cullMode;
        desc.sample_count = format.m_numSamples;
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
    Inline::clearZeroMemory(action);
    action.colors[0].action = SG_ACTION_LOAD;
    action.depth.action = action.stencil.action = SG_ACTION_LOAD;
    {
        SG_PUSH_GROUPF("ModelProgramBundle::CommonPass::execute(pass=%s)", project->findRenderPassName(pass));
        sg::PassBlock pb(project->sharedBatchDrawQueue(), project->beginRenderPass(pass), action);
        pb.applyPipelineBindings(pipeline, m_bindings);
        pb.applyUniformBlock(m_uniformBuffer, sizeof(m_uniformBuffer));
        pb.draw(buffer.m_offset, buffer.m_numIndices);
        SG_POP_GROUP();
    }
    Inline::clearZeroMemory(m_bindings);
}

void
ModelProgramBundle::CommonPass::setImage(const IImageView *value, TextureSamplerStage stage, nanoem_f32_t &useSampler)
{
    useSampler = value != nullptr;
    m_bindings.fs_images[stage] =
        value ? value->handle() : m_parentTechnique->model()->project()->sharedFallbackImage();
    nanoem_assert(sg::is_valid(m_bindings.fs_images[stage]), "must be valid image");
}

ModelProgramBundle::BaseTechnique::BaseTechnique(ModelProgramBundle *parent)
    : m_parentPtr(parent)
    , m_model(nullptr)
    , m_pass(nullptr)
    , m_techniqueType(kTechniqueTypeFirstEnum)
    , m_executed(false)
{
    nanoem_assert(m_parentPtr, "must NOT be nullptr");
    m_pass = nanoem_new(CommonPass(this));
    m_shader = { SG_INVALID_ID };
}

ModelProgramBundle::BaseTechnique::~BaseTechnique() NANOEM_DECL_NOEXCEPT
{
    nanoem_delete_safe(m_pass);
    sg::destroy_shader(m_shader);
}

void
ModelProgramBundle::BaseTechnique::resetScriptCommandState()
{
}

void
ModelProgramBundle::BaseTechnique::resetScriptExternalColor()
{
}

bool
ModelProgramBundle::BaseTechnique::hasNextScriptCommand() const NANOEM_DECL_NOEXCEPT
{
    return false;
}

void
ModelProgramBundle::BaseTechnique::setActiveModel(Model *model)
{
    m_model = model;
    m_executed = false;
}

Model *
ModelProgramBundle::BaseTechnique::model() NANOEM_DECL_NOEXCEPT
{
    return m_model;
}

ModelProgramBundle::TechniqueType
ModelProgramBundle::BaseTechnique::techniqueType() const NANOEM_DECL_NOEXCEPT
{
    return m_techniqueType;
}

sg_shader
ModelProgramBundle::BaseTechnique::shader() const NANOEM_DECL_NOEXCEPT
{
    return m_shader;
}

void
ModelProgramBundle::BaseTechnique::setupUniformBlock(const char *name, sg_shader_uniform_block_desc &desc)
{
    desc.size = sizeof(Vector4) * CommonPass::kUniformBufferMaxEnum;
#if defined(NANOEM_ENABLE_SHADER_OPTIMIZED)
    desc.uniforms[0] = sg_shader_uniform_desc { name, SG_UNIFORMTYPE_FLOAT4, CommonPass::kUniformBufferMaxEnum };
#else
    BX_UNUSED_1(name);
    desc.uniforms[0] =
        sg_shader_uniform_desc { "model_parameters_t", SG_UNIFORMTYPE_FLOAT4, CommonPass::kUniformBufferMaxEnum };
#endif /* NANOEM_ENABLE_SHADER_OPTIMIZED */
}

void
ModelProgramBundle::BaseTechnique::setupShader(const char *vs, const char *fs, sg_shader_desc &desc)
{
    setupUniformBlock(vs, desc.vs.uniform_blocks[0]);
    setupUniformBlock(fs, desc.fs.uniform_blocks[0]);
    desc.vs.entry = "nanoemVSMain";
    desc.fs.entry = "nanoemPSMain";
#if defined(NANOEM_ENABLE_SHADER_OPTIMIZED)
    desc.fs.images[CommonPass::kShadowMapTextureSamplerStage0] =
        sg_shader_image_desc { "SPIRV_Cross_Combined_3", SG_IMAGETYPE_2D, SG_SAMPLERTYPE_FLOAT };
    desc.fs.images[CommonPass::kDiffuseTextureSamplerStage] =
        sg_shader_image_desc { "SPIRV_Cross_Combined", SG_IMAGETYPE_2D, SG_SAMPLERTYPE_FLOAT };
    desc.fs.images[CommonPass::kSphereTextureSamplerStage] =
        sg_shader_image_desc { "SPIRV_Cross_Combined_1", SG_IMAGETYPE_2D, SG_SAMPLERTYPE_FLOAT };
    desc.fs.images[CommonPass::kToonTextureSamplerStage] =
        sg_shader_image_desc { "SPIRV_Cross_Combined_2", SG_IMAGETYPE_2D, SG_SAMPLERTYPE_FLOAT };
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
    desc.fs.images[CommonPass::kToonTextureSamplerStage] =
        sg_shader_image_desc { "SPIRV_Cross_Combinedu_toonTextureu_toonTextureSampler", SG_IMAGETYPE_2D,
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

ModelProgramBundle::ObjectTechnique::ObjectTechnique(ModelProgramBundle *parent, bool isPointDrawEnabled)
    : ModelProgramBundle::BaseTechnique(parent)
    , m_isPointDrawEnabled(isPointDrawEnabled)
{
    m_techniqueType = kTechniqueTypeColor;
}

ModelProgramBundle::ObjectTechnique::~ObjectTechnique() NANOEM_DECL_NOEXCEPT
{
}

IPass *
ModelProgramBundle::ObjectTechnique::execute(const IDrawable * /* drawable */, bool /* scriptExternalColor */)
{
    CommonPass *state = nullptr;
    if (!m_executed) {
        if (!sg::is_valid(m_shader)) {
            sg_shader_desc sd;
            Inline::clearZeroMemory(sd);
            const sg_backend backend = sg::query_backend();
            if (backend == SG_BACKEND_D3D11) {
                sd.fs.bytecode.ptr = g_nanoem_model_color_ps_dxbc_data;
                sd.fs.bytecode.size = g_nanoem_model_color_ps_dxbc_size;
                sd.vs.bytecode.ptr = g_nanoem_model_color_vs_dxbc_data;
                sd.vs.bytecode.size = g_nanoem_model_color_vs_dxbc_size;
            }
            else if (sg::is_backend_metal(backend)) {
                sd.fs.bytecode.ptr = g_nanoem_model_color_fs_msl_macos_data;
                sd.fs.bytecode.size = g_nanoem_model_color_fs_msl_macos_size;
                if (m_isPointDrawEnabled) {
                    sd.vs.bytecode.ptr = g_nanoem_pointed_model_color_vs_msl_macos_data;
                    sd.vs.bytecode.size = g_nanoem_pointed_model_color_vs_msl_macos_size;
                }
                else {
                    sd.vs.bytecode.ptr = g_nanoem_model_color_vs_msl_macos_data;
                    sd.vs.bytecode.size = g_nanoem_model_color_vs_msl_macos_size;
                }
            }
            else if (backend == SG_BACKEND_GLCORE33) {
                sd.fs.source = reinterpret_cast<const char *>(g_nanoem_model_color_fs_glsl_core33_data);
                sd.vs.source = reinterpret_cast<const char *>(g_nanoem_model_color_vs_glsl_core33_data);
            }
            else if (backend == SG_BACKEND_GLES3) {
                sd.fs.source = reinterpret_cast<const char *>(g_nanoem_model_color_fs_glsl_es3_data);
                sd.vs.source = reinterpret_cast<const char *>(g_nanoem_model_color_vs_glsl_es3_data);
            }
            setupShader("_40", "_39", sd);
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

bool
ModelProgramBundle::ObjectTechnique::isPointDrawEnabled() const NANOEM_DECL_NOEXCEPT
{
    return m_isPointDrawEnabled;
}

ModelProgramBundle::EdgeTechnique::EdgeTechnique(ModelProgramBundle *parent)
    : ModelProgramBundle::BaseTechnique(parent)
{
    m_techniqueType = kTechniqueTypeEdge;
}

ModelProgramBundle::EdgeTechnique::~EdgeTechnique() NANOEM_DECL_NOEXCEPT
{
}

IPass *
ModelProgramBundle::EdgeTechnique::execute(const IDrawable * /* drawable */, bool /* scriptExternalColor */)
{
    CommonPass *state = nullptr;
    if (!m_executed) {
        if (!sg::is_valid(m_shader)) {
            sg_shader_desc sd;
            Inline::clearZeroMemory(sd);
            const sg_backend backend = sg::query_backend();
            if (backend == SG_BACKEND_D3D11) {
                sd.fs.bytecode.ptr = g_nanoem_model_edge_ps_dxbc_data;
                sd.fs.bytecode.size = g_nanoem_model_edge_ps_dxbc_size;
                sd.vs.bytecode.ptr = g_nanoem_model_edge_vs_dxbc_data;
                sd.vs.bytecode.size = g_nanoem_model_edge_vs_dxbc_size;
            }
            else if (sg::is_backend_metal(backend)) {
                sd.fs.bytecode.ptr = g_nanoem_model_edge_fs_msl_macos_data;
                sd.fs.bytecode.size = g_nanoem_model_edge_fs_msl_macos_size;
                sd.vs.bytecode.ptr = g_nanoem_model_edge_vs_msl_macos_data;
                sd.vs.bytecode.size = g_nanoem_model_edge_vs_msl_macos_size;
            }
            else if (backend == SG_BACKEND_GLCORE33) {
                sd.fs.source = reinterpret_cast<const char *>(g_nanoem_model_edge_fs_glsl_core33_data);
                sd.vs.source = reinterpret_cast<const char *>(g_nanoem_model_edge_vs_glsl_core33_data);
            }
            else if (backend == SG_BACKEND_GLES3) {
                sd.fs.source = reinterpret_cast<const char *>(g_nanoem_model_edge_fs_glsl_es3_data);
                sd.vs.source = reinterpret_cast<const char *>(g_nanoem_model_edge_vs_glsl_es3_data);
            }
            setupShader("_42", "_37", sd);
            char label[Inline::kMarkerStringLength];
            if (Inline::isDebugLabelEnabled()) {
                StringUtils::format(label, sizeof(label), "%s/EdgeTechnique", kPrefixName);
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

ModelProgramBundle::GroundShadowTechnique::GroundShadowTechnique(ModelProgramBundle *parent)
    : ModelProgramBundle::BaseTechnique(parent)
{
    m_techniqueType = kTechniqueTypeGroundShadow;
}

ModelProgramBundle::GroundShadowTechnique::~GroundShadowTechnique() NANOEM_DECL_NOEXCEPT
{
}

IPass *
ModelProgramBundle::GroundShadowTechnique::execute(const IDrawable * /* drawable */, bool /* scriptExternalColor */)
{
    CommonPass *state = nullptr;
    if (!m_executed) {
        if (!sg::is_valid(m_shader)) {
            sg_shader_desc sd;
            Inline::clearZeroMemory(sd);
            const sg_backend backend = sg::query_backend();
            if (backend == SG_BACKEND_D3D11) {
                sd.fs.bytecode.ptr = g_nanoem_model_ground_shadow_ps_dxbc_data;
                sd.fs.bytecode.size = g_nanoem_model_ground_shadow_ps_dxbc_size;
                sd.vs.bytecode.ptr = g_nanoem_model_ground_shadow_vs_dxbc_data;
                sd.vs.bytecode.size = g_nanoem_model_ground_shadow_vs_dxbc_size;
            }
            else if (sg::is_backend_metal(backend)) {
                sd.fs.bytecode.ptr = g_nanoem_model_ground_shadow_fs_msl_macos_data;
                sd.fs.bytecode.size = g_nanoem_model_ground_shadow_fs_msl_macos_size;
                sd.vs.bytecode.ptr = g_nanoem_model_ground_shadow_vs_msl_macos_data;
                sd.vs.bytecode.size = g_nanoem_model_ground_shadow_vs_msl_macos_size;
            }
            else if (backend == SG_BACKEND_GLCORE33) {
                sd.fs.source = reinterpret_cast<const char *>(g_nanoem_model_ground_shadow_fs_glsl_core33_data);
                sd.vs.source = reinterpret_cast<const char *>(g_nanoem_model_ground_shadow_vs_glsl_core33_data);
            }
            else if (backend == SG_BACKEND_GLES3) {
                sd.fs.source = reinterpret_cast<const char *>(g_nanoem_model_ground_shadow_fs_glsl_es3_data);
                sd.vs.source = reinterpret_cast<const char *>(g_nanoem_model_ground_shadow_vs_glsl_es3_data);
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

ModelProgramBundle::ZplotTechnique::ZplotTechnique(ModelProgramBundle *parent)
    : ModelProgramBundle::BaseTechnique(parent)
{
    m_techniqueType = kTechniqueTypeZplot;
}

ModelProgramBundle::ZplotTechnique::~ZplotTechnique() NANOEM_DECL_NOEXCEPT
{
}

IPass *
ModelProgramBundle::ZplotTechnique::execute(const IDrawable * /* drawable */, bool /* scriptExternalColor */)
{
    CommonPass *state = nullptr;
    if (!m_executed) {
        if (!sg::is_valid(m_shader)) {
            sg_shader_desc sd;
            Inline::clearZeroMemory(sd);
            const sg_backend backend = sg::query_backend();
            if (backend == SG_BACKEND_D3D11) {
                sd.fs.bytecode.ptr = g_nanoem_model_zplot_ps_dxbc_data;
                sd.fs.bytecode.size = g_nanoem_model_zplot_ps_dxbc_size;
                sd.vs.bytecode.ptr = g_nanoem_model_zplot_vs_dxbc_data;
                sd.vs.bytecode.size = g_nanoem_model_zplot_vs_dxbc_size;
            }
            else if (sg::is_backend_metal(backend)) {
                sd.fs.bytecode.ptr = g_nanoem_model_zplot_fs_msl_macos_data;
                sd.fs.bytecode.size = g_nanoem_model_zplot_fs_msl_macos_size;
                sd.vs.bytecode.ptr = g_nanoem_model_zplot_vs_msl_macos_data;
                sd.vs.bytecode.size = g_nanoem_model_zplot_vs_msl_macos_size;
            }
            else if (backend == SG_BACKEND_GLCORE33) {
                sd.fs.source = reinterpret_cast<const char *>(g_nanoem_model_zplot_fs_glsl_core33_data);
                sd.vs.source = reinterpret_cast<const char *>(g_nanoem_model_zplot_vs_glsl_core33_data);
            }
            else if (backend == SG_BACKEND_GLES3) {
                sd.fs.source = reinterpret_cast<const char *>(g_nanoem_model_zplot_fs_glsl_es3_data);
                sd.vs.source = reinterpret_cast<const char *>(g_nanoem_model_zplot_vs_glsl_es3_data);
            }
            setupShader("_30", "_37", sd);
            char label[Inline::kMarkerStringLength];
            if (Inline::isDebugLabelEnabled()) {
                StringUtils::format(label, sizeof(label), "%s/ZplotTechnique", kPrefixName);
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
