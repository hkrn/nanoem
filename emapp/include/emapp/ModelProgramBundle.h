/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODELPROGRAMBUNDLE_H_
#define NANOEM_EMAPP_MODELPROGRAMBUNDLE_H_

#include "emapp/IDrawable.h"
#include "emapp/IEffect.h"
#include "emapp/IPass.h"
#include "emapp/ITechnique.h"

namespace nanoem {

class Accessory;
class ICamera;
class ILight;
class Image;
class ShadowCamera;

class ModelProgramBundle NANOEM_DECL_SEALED : public IEffect, private NonCopyable {
public:
    ModelProgramBundle();
    ~ModelProgramBundle() NANOEM_DECL_NOEXCEPT;

    ITechnique *findTechnique(const String &passType, const nanodxm_material_t *material, nanoem_rsize_t materialIndex,
        nanoem_rsize_t numMaterials, Accessory *accessory) NANOEM_DECL_OVERRIDE;
    ITechnique *findTechnique(const String &passType, const nanoem_model_material_t *material,
        nanoem_rsize_t materialIndex, nanoem_rsize_t numMaterials, Model *model) NANOEM_DECL_OVERRIDE;
    void createImageResource(
        const void *ptr, size_t size, const ImageResourceParameter &parameter) NANOEM_DECL_OVERRIDE;
    void setAllParameterObjects(
        const nanoem_motion_effect_parameter_t *const *parameters, nanoem_rsize_t numParameters) NANOEM_DECL_OVERRIDE;
    void setAllParameterObjects(const nanoem_motion_effect_parameter_t *const *fromParameters,
        nanoem_rsize_t numFromParameters, const nanoem_motion_effect_parameter_t *const *toParameters,
        nanoem_rsize_t numToParameters, nanoem_f32_t coefficient) NANOEM_DECL_OVERRIDE;
    ImageResourceList allImageResources() const NANOEM_DECL_OVERRIDE;
    ScriptClassType scriptClass() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    ScriptOrderType scriptOrder() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool hasScriptExternal() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void destroy();

private:
    enum TechniqueType {
        kTechniqueTypeFirstEnum,
        kTechniqueTypeColor = kTechniqueTypeFirstEnum,
        kTechniqueTypeEdge,
        kTechniqueTypeGroundShadow,
        kTechniqueTypeZplot,
        kTechniqueTypeMaxEnum
    };

    class CommonPass;
    class BaseTechnique : public ITechnique {
    public:
        BaseTechnique(ModelProgramBundle *parent);
        ~BaseTechnique() NANOEM_DECL_NOEXCEPT;

        void resetScriptCommandState() NANOEM_DECL_OVERRIDE;
        void resetScriptExternalColor() NANOEM_DECL_OVERRIDE;
        bool hasNextScriptCommand() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

        void setActiveModel(Model *model);
        Model *model() NANOEM_DECL_NOEXCEPT;
        TechniqueType techniqueType() const NANOEM_DECL_NOEXCEPT;
        sg_shader shader() const NANOEM_DECL_NOEXCEPT;

    protected:
        void setupUniformBlock(const char *name, sg_shader_uniform_block_desc &desc);
        void setupShader(const char *vs, const char *fs, sg_shader_desc &desc);

        ModelProgramBundle *m_parentPtr;
        Model *m_model;
        CommonPass *m_pass;
        TechniqueType m_techniqueType;
        sg_shader m_shader;
        bool m_executed;
    };
    class ObjectTechnique : public BaseTechnique {
    public:
        ObjectTechnique(ModelProgramBundle *parent, bool isPointDrawEnabled);
        ~ObjectTechnique() NANOEM_DECL_NOEXCEPT;

        IPass *execute(const IDrawable *drawable, bool scriptExternalColor) NANOEM_DECL_OVERRIDE;
        bool isPointDrawEnabled() const NANOEM_DECL_NOEXCEPT;

    private:
        bool m_isPointDrawEnabled;
    };
    class EdgeTechnique : public BaseTechnique {
    public:
        EdgeTechnique(ModelProgramBundle *parent);
        ~EdgeTechnique() NANOEM_DECL_NOEXCEPT;

        IPass *execute(const IDrawable *drawable, bool scriptExternalColor) NANOEM_DECL_OVERRIDE;
    };
    class GroundShadowTechnique : public BaseTechnique {
    public:
        GroundShadowTechnique(ModelProgramBundle *parent);
        ~GroundShadowTechnique() NANOEM_DECL_NOEXCEPT;

        IPass *execute(const IDrawable *drawable, bool scriptExternalColor) NANOEM_DECL_OVERRIDE;
    };
    class ZplotTechnique : public BaseTechnique {
    public:
        ZplotTechnique(ModelProgramBundle *parent);
        ~ZplotTechnique() NANOEM_DECL_NOEXCEPT;

        IPass *execute(const IDrawable *drawable, bool scriptExternalColor) NANOEM_DECL_OVERRIDE;
    };
    class CommonPass : public IPass {
    public:
        enum UniformBuffer {
            kUniformBufferFirstEnum,
            kUniformBufferModelMatrix = kUniformBufferFirstEnum,
            kUniformBufferModelViewMatrix = kUniformBufferModelMatrix + 4,
            kUniformBufferModelViewProjectionMatrix = kUniformBufferModelViewMatrix + 4,
            kUniformBufferLightViewProjectionMatrix = kUniformBufferModelViewProjectionMatrix + 4,
            kUniformBufferLightColor = kUniformBufferLightViewProjectionMatrix + 4,
            kUniformBufferLightDirection,
            kUniformBufferCameraPosition,
            kUniformBufferMaterialAmbient,
            kUniformBufferMaterialDiffuse,
            kUniformBufferMaterialSpecular,
            kUniformBufferEnableVertexColor,
            kUniformBufferDiffuseTextureBlendFactor,
            kUniformBufferSphereTextureBlendFactor,
            kUniformBufferToonTextureBlendFactor,
            kUniformBufferUseTextureSampler,
            kUniformBufferSphereTextureType,
            kUniformBufferShadowMapSize,
            kUniformBufferMaxEnum
        };
        enum TextureSamplerStage {
            kTextureSamplerStageFirstEnum = 0,
            kShadowMapTextureSamplerStage0 = kTextureSamplerStageFirstEnum,
            kDiffuseTextureSamplerStage,
            kSphereTextureSamplerStage,
            kToonTextureSamplerStage,
            kTextureSamplerStageMaxEnum
        };

        CommonPass(BaseTechnique *parent);
        ~CommonPass() NANOEM_DECL_NOEXCEPT;

        void setGlobalParameters(const IDrawable *drawable, const Project *project) NANOEM_DECL_OVERRIDE;
        void setCameraParameters(const ICamera *camera, const Matrix4x4 &world) NANOEM_DECL_OVERRIDE;
        void setLightParameters(const ILight *light, bool adjustment) NANOEM_DECL_OVERRIDE;
        void setAllAccessoryParameters(const Accessory *model, const Project *project) NANOEM_DECL_OVERRIDE;
        void setAllModelParameters(const Model *model, const Project *project) NANOEM_DECL_OVERRIDE;
        void setMaterialParameters(const nanodxm_material_t *material) NANOEM_DECL_OVERRIDE;
        void setMaterialParameters(const nanoem_model_material_t *materialPtr) NANOEM_DECL_OVERRIDE;
        void setEdgeParameters(const nanoem_model_material_t *materialPtr, nanoem_f32_t edgeSize) NANOEM_DECL_OVERRIDE;
        void setGroundShadowParameters(
            const ILight *light, const ICamera *camera, const Matrix4x4 &world) NANOEM_DECL_OVERRIDE;
        void setShadowMapParameters(const ShadowCamera *shadowCamera, const Matrix4x4 &world) NANOEM_DECL_OVERRIDE;
        void execute(const IDrawable *drawable, const Buffer &buffer) NANOEM_DECL_OVERRIDE;

    private:
        void setImage(const IImageView *value, TextureSamplerStage stage, nanoem_f32_t &useSampler);

        BaseTechnique *m_parentTechnique;
        Vector4 m_uniformBuffer[kUniformBufferMaxEnum];
        PipelineMap m_pipelines;
        sg_bindings m_bindings;
        sg_cull_mode m_cullMode;
        sg_primitive_type m_primitiveType;
        nanoem_f32_t m_opacity;
    };

    ObjectTechnique *m_objectTechnique;
    EdgeTechnique *m_edgeTechnique;
    GroundShadowTechnique *m_groundShadowTechnique;
    ZplotTechnique *m_zplotTechnique;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODELPROGRAMBUNDLE_H_ */
