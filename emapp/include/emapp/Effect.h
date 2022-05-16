/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_EFFECT_H_
#define NANOEM_EMAPP_EFFECT_H_

#include "emapp/IEffect.h"
#include "emapp/effect/Common.h"
#include "emapp/effect/OffscreenRenderTargetImageContainer.h"
#include "emapp/effect/OffscreenRenderTargetOption.h"
#include "emapp/effect/Pass.h"
#include "emapp/effect/RenderTargetColorImageContainer.h"
#include "emapp/effect/RenderTargetDepthStencilImageContainer.h"
#include "emapp/effect/Technique.h"

namespace nanoem {

class AccessoryProgramBundle;
class ICamera;
class IFileManager;
class ILight;
class ModelProgramBundle;
class Project;
class Progress;

namespace effect {
class AnimatedImageContainer;
class OffscreenRenderTargetImageContainer;
class Pass;
class RenderTargetDepthStencilImageContainer;
class RenderTargetColorImageContainer;
class RenderTargetNormalizer;
} /* namespace effect */

class Effect NANOEM_DECL_SEALED : public IEffect, private NonCopyable {
public:
    static const String kPassTypeObject;
    static const String kPassTypeObjectSelfShadow;
    static const String kPassTypeShadow;
    static const String kPassTypeEdge;
    static const String kPassTypeZplot;
    static const String kOffscreenOwnerNameMain;
    static const char *const kSourceFileExtension;
    static const char *const kBinaryFileExtension;
    struct StagingBuffer {
        ByteArray m_content;
        sg_buffer m_handle;
        bool m_read;
        bool m_immutable;
    };

    typedef void (*UserDataDestructor)(void *userData, const Effect *effect);
    typedef tinystl::pair<void *, UserDataDestructor> UserData;
    typedef tinystl::unordered_set<int, TinySTLAllocator> MaterialIndexSet;
    typedef tinystl::unordered_map<String, ByteArray, TinySTLAllocator> NamedByteArrayMap;
    typedef tinystl::unordered_map<String, StagingBuffer, TinySTLAllocator> StagingBufferMap;
    typedef tinystl::unordered_map<String, NamedByteArrayMap, TinySTLAllocator> PassUniformBufferMap;
    typedef tinystl::unordered_map<String, effect::AnimatedImageContainer *, TinySTLAllocator>
        AnimatedImageContainerMap;
    typedef tinystl::unordered_map<String, effect::OffscreenRenderTargetImageContainer *, TinySTLAllocator>
        OffscreenRenderTargetImageContainerMap;
    typedef tinystl::unordered_map<String, effect::RenderTargetColorImageContainer *, TinySTLAllocator>
        NamedRenderTargetColorImageContainerMap;
    typedef tinystl::unordered_map<String, effect::RenderTargetDepthStencilImageContainer *, TinySTLAllocator>
        RenderTargetDepthStencilImageContainerMap;

    static void initializeD3DCompiler();
    static void terminateD3DCompiler();

    static StringList loadableExtensions();
    static StringSet loadableExtensionsSet();
    static bool isLoadableExtension(const String &extension);
    static bool isLoadableExtension(const URI &fileURI);
    static String resolveFilePath(const char *filePath, const char *extension);
    static URI resolveURI(const URI &fileURI, const char *extension);
    static URI resolveSourceURI(IFileManager *fileManager, const URI &baseURI);
    static bool compileFromSource(const URI &fileURI, IFileManager *fileManager, bool mipmap, ByteArray &output,
        Progress &progress, Error &failure);
    static const char *toString(effect::ParameterType value) NANOEM_DECL_NOEXCEPT;
    static bool isScriptClassObject(ScriptClassType value) NANOEM_DECL_NOEXCEPT;
    static void parseScript(const String &script, effect::ScriptCommandMap &output);
    static void parseScript(const String &script, effect::ScriptCommandMap &output, int &clearColorScriptIndex,
        int &clearDepthScriptIndex, bool &hasScriptExternal);
    static void parseSubset(const String &script, nanoem_rsize_t numMaterials, MaterialIndexSet &output);

    Effect(Project *project, effect::GlobalUniform *globalUniformPtr, AccessoryProgramBundle *accessoryProgramBundlePtr,
        ModelProgramBundle *modelProgramBundlePtr);
    ~Effect() NANOEM_DECL_NOEXCEPT;

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

    bool load(const nanoem_u8_t *data, size_t size, Progress &progress, Error &error);
    bool load(const ByteArray &bytes, Progress &progress, Error &error);
    bool upload(effect::AttachmentType type, Progress &progress, Error &error);
    bool upload(effect::AttachmentType type, const Archiver *archiver, Progress &progress, Error &error);
    void resizeAllRenderTargetImages(
        const Vector2UI16 &size, StringSet &sharedRenderColorImageNames, StringSet &sharedOffscreenImageNames);
    void resetAllSharedRenderTargetColorImages(const StringSet &names);
    void resetAllSharedOffscreenRenderTargets(const StringSet &names);
    void markAllAnimatedImagesUpdatable();
    void createAllDrawableRenderTargetColorImages(const IDrawable *drawable);
    void destroyAllDrawableRenderTargetColorImages(const IDrawable *drawable);
    void generateRenderTargetMipmapImagesChain();
    void generateOffscreenMipmapImagesChain(const effect::OffscreenRenderTargetOption &option);
    void updateCurrentRenderTargetPixelFormatSampleCount();
    void destroy();

    const effect::RenderTargetColorImageContainer *findRenderTargetImageContainer(
        const IDrawable *drawable, sg_image value) const NANOEM_DECL_NOEXCEPT;
    const effect::RenderTargetColorImageContainer *searchRenderTargetColorImageContainer(
        const IDrawable *drawable, sg_image value) const NANOEM_DECL_NOEXCEPT;
    const effect::ImageSamplerList *findImageSamplerList(const effect::Pass *passPtr) const NANOEM_DECL_NOEXCEPT;
    void getAllOffscreenRenderTargetOptions(effect::OffscreenRenderTargetOptionList &value) const;
    void getAllRenderTargetImageContainers(NamedRenderTargetColorImageContainerMap &value) const;
    void getAllUIWidgetParameters(effect::UIWidgetParameterList &value);
    void getPassUniformBuffer(PassUniformBufferMap &value) const;
    void getPrimaryRenderTargetColorImageSubPixelOffset(Vector4 &value) const;
    void attachAllResources(FileEntityMap &allAttachments) const;
    void setImageLabel(sg_image texture, const String &name);
    void setImageLabel(sg_image texture, const char *name);
    void setShaderLabel(sg_shader shader, const char *name);
    void removeImageLabel(sg_image texture);
    void removeShaderLabel(sg_shader shader);

    const Project *project() const NANOEM_DECL_NOEXCEPT;
    Project *project() NANOEM_DECL_NOEXCEPT;
    const effect::Logger *logger() const NANOEM_DECL_NOEXCEPT;
    effect::Logger *logger() NANOEM_DECL_NOEXCEPT;
    StringList allIncludePaths() const;
    String name() const;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT;
    void setName(const String &value);
    String filename() const;
    void setFilename(String value);
    URI fileURI() const;
    URI resolvedFileURI() const;
    void setFileURI(const URI &value);
    UserData userData() const;
    void setUserData(const UserData &value);
    Vector4 clearColor() const NANOEM_DECL_NOEXCEPT;
    nanoem_f32_t clearDepth() const NANOEM_DECL_NOEXCEPT;
    bool canEnable() const NANOEM_DECL_NOEXCEPT;
    bool isEnabled() const NANOEM_DECL_NOEXCEPT;
    void setEnabled(bool value);
    bool isPassUniformBufferInspectionEnabled() const NANOEM_DECL_NOEXCEPT;
    void setPassUniformBufferInspectionEnabled(bool value);

    bool findParameterUniform(const String &name, Vector4 &value) const NANOEM_DECL_NOEXCEPT;
    void setParameterUniform(const String &name, const Vector4 &value);
    void pushLoopCounter(const String &name, size_t scriptIndex, effect::LoopCounter::Stack &counterStack);
    void handleLoopGetIndex(const String &name, const effect::LoopCounter::Stack &counterStack);
    void popLoopCounter(effect::LoopCounter::Stack &counterStack, size_t &scriptIndex);
    void setRenderTargetColorImageDescription(const IDrawable *drawable, size_t renderTargetIndex, const String &value);
    void setRenderTargetDepthStencilImageDescription(const String &value);
    void beginRenderPass(const sg_bindings &bindings);
    sg_pass resetRenderPass(const IDrawable *drawable);
    void drawSceneRenderPass(
        const IDrawable *drawable, effect::Pass *pass, sg_pipeline_desc &pd, sg_bindings &bindings);
    void drawGeometryRenderPass(const IDrawable *drawable, effect::Pass *pass, int offset, int numIndices,
        sg_pipeline_desc &pd, sg_bindings &bindings);
    void clearRenderPass(
        const IDrawable *drawable, const char *name, const String &target, effect::RenderPassScope *renderPassScope);
    void setClearColor(const String &parameterName);
    void setClearDepth(const String &parameterName);
    void overridePipelineDescription(sg_pipeline_desc &pd, ScriptClassType classType) const NANOEM_DECL_NOEXCEPT;
    void updatePassImageHandles(effect::Pass *pass, sg_bindings &bindings);
    void updatePassUniformHandles(sg::PassBlock &pb);
    effect::GlobalUniform *globalUniform();
    effect::ViewPassSet viewPassSet() const;

    void setGlobalParameters(const IDrawable *drawable, const Project *project, effect::Pass *pass);
    void setCameraParameters(const ICamera *camera, const Matrix4x4 &world, const effect::Pass *pass);
    void setLightParameters(const ILight *light, bool adjustment, effect::Pass *pass);
    void setAllAccessoryParameters(const Accessory *accessory, const Project *project, effect::Pass *pass);
    void setAccessoryParameter(
        const String &name, const Accessory *model, effect::ControlObjectTarget &target, effect::Pass *pass);
    void setAllModelParameters(const Model *model, const Project *project, effect::Pass *pass);
    void setModelParameter(
        const String &name, const Model *model, effect::ControlObjectTarget &target, effect::Pass *pass);
    void setMaterialParameters(const Accessory *accessory, const nanodxm_material_t *materialPtr, effect::Pass *pass);
    void setMaterialParameters(const nanoem_model_material_t *materialPtr, const String &target, effect::Pass *pass);
    void setEdgeParameters(const nanoem_model_material_t *materialPtr, nanoem_f32_t edgeSize, effect::Pass *pass);
    void setShadowParameters(const ILight *light, const ICamera *camera, const Matrix4x4 &world, effect::Pass *pass);
    void setShadowMapParameters(const ShadowCamera *shadowCamera, const Matrix4x4 &world, effect::Pass *pass);

private:
    typedef void (*SemanticParameterHandler)(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    typedef tinystl::unordered_map<String, SemanticParameterHandler, TinySTLAllocator> SemanticParameterHandlerMap;
    typedef tinystl::unordered_map<const effect::Pass *, effect::ImageSamplerList, TinySTLAllocator> ImageSamplerMap;
    typedef tinystl::unordered_map<nanoem_u32_t, nanoem_u32_t, TinySTLAllocator> HashMap;
    typedef tinystl::unordered_map<nanoem_u32_t, sg_image, TinySTLAllocator> OverridenImageHandleMap;
    typedef tinystl::unordered_map<String, effect::TechniqueList, TinySTLAllocator> TechniqueListMap;
    typedef tinystl::unordered_map<const IDrawable *, NamedRenderTargetColorImageContainerMap, TinySTLAllocator>
        DrawableNamedRenderTargetColorImageContainerMap;
    typedef tinystl::unordered_map<nanoem_u32_t, String, TinySTLAllocator> NamedHandleMap;
    typedef tinystl::unordered_map<nanoem_u32_t, effect::RenderTargetNormalizer *, TinySTLAllocator>
        RenderTargetNormalizerMap;

    static void handleWorldMatrixSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleViewMatrixSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleProjectMatrixSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleWorldViewMatrixSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleViewProjectMatrixSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleWorldViewProjectionMatrixSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleWorldMatrixSemanticInverse(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleViewMatrixSemanticInverse(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleProjectMatrixSemanticInverse(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleWorldViewMatrixSemanticInverse(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleViewProjectMatrixSemanticInverse(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleWorldViewProjectionMatrixSemanticInverse(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleWorldMatrixSemanticTranspose(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleViewMatrixSemanticTranspose(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleProjectMatrixSemanticTranspose(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleWorldViewMatrixSemanticTranspose(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleViewProjectMatrixSemanticTranspose(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleWorldViewProjectionMatrixSemanticTranspose(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleWorldMatrixSemanticInverseTranspose(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleViewMatrixSemanticInverseTranspose(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleProjectMatrixSemanticInverseTranspose(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleWorldViewMatrixSemanticInverseTranspose(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleViewProjectMatrixSemanticInverseTranspose(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleWorldViewProjectionMatrixSemanticInverseTranspose(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleDiffuseSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleAmbientSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleEmissiveSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleSpecularSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleSpecularPowerSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleToonColorSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleEdgeColorSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleGroundShadowColorSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handlePositionSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleDirectionSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleMaterialTextureSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleMaterailSphereMapSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleMaterialToonTextureSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleAddingTextureSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleMultiplyingTextureSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleAddingSphereTextureSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleMultiplyingSphereTextureSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleViewportPixelSizeSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleTimeSemantic(Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleElapsedTimeSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleMousePositionSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleLeftMouseDownSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleMiddleMouseDownSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleRightMouseDownSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleControlObjectSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleRenderColorTargetSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleRenderDepthStencilTargetSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleAnimatedTextureSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleOffscreenRenderTargetSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleTextureValueSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static void handleStandardsGlobalSemantic(
        Effect *self, const effect::TypedSemanticParameter &parameter, Progress &progress);
    static sg_image_type determineImageType(const effect::AnnotationMap &annotations);
    static sg_image_type determineImageType(
        const effect::AnnotationMap &annotations, effect::ParameterType parameterType);
    static nanoem_u8_t determineMipLevels(
        const effect::AnnotationMap &annotations, const Vector2 &size, int defaultLevel);

    const NamedRenderTargetColorImageContainerMap *findNamedRenderTargetColorImageContainerMap(
        const IDrawable *drawable) const;
    effect::Technique *internalFindTechnique(const String &passType, nanoem_rsize_t numMaterials,
        nanoem_rsize_t materialIndex, const nanodxm_material_t *material, Accessory *accessory);
    effect::Technique *internalFindTechnique(
        const String &passType, nanoem_rsize_t numMaterials, const nanoem_model_material_t *material);
    ImageResourceParameter createImageResourceParameter(const effect::TypedSemanticParameter &parameter) const;
    ImageResourceParameter createImageResourceParameter(
        const effect::TypedSemanticParameter &parameter, const URI &fileURI, const String &filename) const;
    void createBlankImageResource(const effect::TypedSemanticParameter &parameter);
    void getImageResourceParameter(ImageResourceParameter &parameter);
    void createImageResourceFromArchive(
        const effect::TypedSemanticParameter &parameter, const Archiver *archiver, Progress &progress, Error &error);
    void createImageResourceFromArchive(const String &name, const effect::TypedSemanticParameter &parameter,
        const Archiver *archiver, Progress &progress, Error &error);
    void registerImageResource(sg_image image, const IEffect::ImageResourceParameter &parameter);
    sg_image createOverrideImage(const String &name, const IImageView *image, bool mipmap);
    sg_pixel_format determinePixelFormat(
        const effect::AnnotationMap &annotations, sg_pixel_format defaultFormat) const NANOEM_DECL_NOEXCEPT;
    sg_pixel_format determineDepthStencilPixelFormat(
        const effect::AnnotationMap &annotations, sg_pixel_format defaultFormat) const NANOEM_DECL_NOEXCEPT;
    Vector4 scaledViewportImageSize(const Vector2 &scaleFactor) const NANOEM_DECL_NOEXCEPT;
    Vector4 determineImageSize(const effect::AnnotationMap &annotations, const Vector4 &defaultValue,
        Vector2 &scaleFactor) const NANOEM_DECL_NOEXCEPT;
    void setImageUniform(const String &name, const effect::Pass *pass, sg_image handle);
    bool writeUniformBuffer(
        const effect::RegisterIndex &index, const void *ptr, size_t size, effect::GlobalUniform::Buffer &bufferPtr);
    void writeUniformBuffer(const String &name, const effect::Pass *passPtr, const void *ptr, size_t size);
    void writeUniformBuffer(const String &name, const effect::Pass *passPtr, bool value);
    void writeUniformBuffer(const String &name, const effect::Pass *passPtr, int value);
    void writeUniformBuffer(const String &name, const effect::Pass *passPtr, nanoem_f32_t value);
    void writeUniformBuffer(const String &name, const effect::Pass *passPtr, const Vector4 &value);
    void writeUniformBuffer(const String &name, const effect::Pass *passPtr, const Matrix4x4 &value);
    void writeUniformBuffer(const String &name, const effect::Pass *passPtr, const ByteArray *value);
    void writeUniformBuffer(const String &name, const effect::Pass *passPtr, const effect::NonSemanticParameter &value);
    void setDefaultControlParameterValues(
        const String &name, const effect::ControlObjectTarget &target, const effect::Pass *pass);
    void setFoundImageSamplers(const effect::SemanticUniformList &uniforms, const effect::SemanticImageMap &images,
        const effect::Pass *passPtr);
    void setFoundImageSamplers(const effect::SemanticUniformList &uniforms,
        const NamedRenderTargetColorImageContainerMap &containers, const effect::Pass *passPtr);
    void setFoundImageSamplers(const effect::SemanticUniformList &uniforms,
        const OffscreenRenderTargetImageContainerMap &containers, const effect::Pass *passPtr);
    void setFoundImageSamplers(const effect::SemanticUniformList &uniforms, const AnimatedImageContainerMap &containers,
        const effect::Pass *passPtr);
    void setTextureValues(const effect::SemanticImageMap &images, effect::Pass *passPtr);
    void setTextureValues(const NamedRenderTargetColorImageContainerMap &containers, effect::Pass *passPtr);
    void setTextureValues(const OffscreenRenderTargetImageContainerMap &containers, effect::Pass *passPtr);
    void handleMatrixSemantics(
        const effect::TypedSemanticParameter &parameter, effect::MatrixType matrixType, bool inversed, bool transposed);
    void addMissingParameterKeyError(const String &name, const effect::TypedSemanticParameter &parameter);
    void addInvalidParameterValueError(const String &value, const effect::TypedSemanticParameter &parameter);
    void addInvalidParameterTypeError(effect::ParameterType expected, const effect::TypedSemanticParameter &parameter);
    void destroyAllRenderTargetNormalizers();
    void destroyAllOverridenImages();
    void destroyAllTechniques();
    void destroyAllSemanticImages(effect::SemanticImageMap &images);
    void destroyAllDrawableNamedRenderTargetColorImages();
    void destroyAllRenderTargetColorImages(NamedRenderTargetColorImageContainerMap &containers);
    void destroyAllRenderTargetDepthStencilImages(RenderTargetDepthStencilImageContainerMap &containers);
    void destroyAllOffscreenRenderTargetImages(OffscreenRenderTargetImageContainerMap &containers);
    void destroyAllAnimatedImages(AnimatedImageContainerMap &containers);
    void destroyAllStagingBuffers(StagingBufferMap &buffers);
    tinystl::pair<const Model *, const Accessory *> findOffscreenOwnerObject(
        const IDrawable *ownerDrawable, const Project *project) const NANOEM_DECL_NOEXCEPT;
    void decodeImageData(const ByteArray &bytes, const ImageResourceParameter &parameter, Error &error);
    void setNormalizedColorImageContainer(
        const String &name, int numMipLevels, effect::RenderTargetColorImageContainer *container);
    void resetPassDescription();
    sg_pass resetRenderPass(const IDrawable *drawable, const char *label, effect::Pass *passPtr,
        effect::RenderTargetNormalizer *&normalizer);
    bool validateAllPassColorAttachments(const IDrawable *drawable, const sg_pixel_format primaryColorFormat,
        int numSamples, sg_pixel_format &normalizedColorFormat) const NANOEM_DECL_NOEXCEPT;
    bool containsPassOutputImageSampler(const effect::Pass *passPtr) const NANOEM_DECL_NOEXCEPT;
    void internalDrawRenderPass(const IDrawable *drawable, effect::Pass *pass, sg::PassBlock::IDrawQueue *drawQueue,
        int offset, int numIndices, sg_pipeline_desc &pd, sg_bindings &bindings, ScriptClassType classType);
    void generateRenderTargetMipmapImagesChain(const IDrawable *drawable, const String &colorContainerName,
        const String &depthContainerName, const sg_image_desc &imageDescription);
    bool validateTechnique(const String &passType) const NANOEM_DECL_NOEXCEPT;
    void attachEffectIncludePathSet(FileEntityMap &allAttachments) const;
    void attachEffectImageResource(FileEntityMap &attachments) const;
    void addSemanticParameterHandler(const char *const name, SemanticParameterHandler value);
    void addImageFormat(const char *const name, sg_pixel_format value);

    Project *m_project;
    effect::GlobalUniform *m_globalUniformPtr;
    effect::Logger *m_logger;
    AccessoryProgramBundle *m_fallbackAccessoryProgramBundle;
    ModelProgramBundle *m_fallbackModelProgramBundle;
    SemanticParameterHandlerMap m_semanticParameterHandlers;
    effect::ImageFormatMap m_imageFormats;
    effect::ImageFormatMap m_depthStencilImageFormats;
    effect::ImageDescriptionMap m_imageDescriptions;
    effect::ParameterMap m_parameters;
    effect::MatrixUniformMap m_cameraMatrixUniforms;
    effect::MatrixUniformMap m_lightMatrixUniforms;
    effect::SemanticUniformList m_materialAmbientUniforms;
    effect::SemanticUniformList m_materialDiffuseUniforms;
    effect::SemanticUniformList m_materialEmissiveUniforms;
    effect::SemanticUniformList m_materialSpecularUniforms;
    effect::SemanticUniformList m_materialSpecularPowerUniforms;
    effect::SemanticUniformList m_materialToonColorUniforms;
    effect::SemanticUniformList m_materialEdgeColorUniforms;
    effect::SemanticUniformList m_materialGroundColorUniforms;
    effect::SemanticUniformList m_lightAmbientUniforms;
    effect::SemanticUniformList m_lightDiffuseUniforms;
    effect::SemanticUniformList m_lightSpecularUniforms;
    effect::SemanticUniformList m_cameraPositionUniforms;
    effect::SemanticUniformList m_cameraDirectionUniforms;
    effect::SemanticUniformList m_lightPositionUniforms;
    effect::SemanticUniformList m_lightDirectionUniforms;
    effect::SemanticUniformList m_diffuseImageUniforms;
    effect::SemanticUniformList m_sphereImageUniforms;
    effect::SemanticUniformList m_toonImageUniforms;
    effect::SemanticUniformList m_addingDiffuseImageBlendFactorUniforms;
    effect::SemanticUniformList m_addingSphereImageBlendFactorUniforms;
    effect::SemanticUniformList m_multiplyingDiffuseImageBlendFactorUniforms;
    effect::SemanticUniformList m_multiplyingSphereImageBlendFactorUniforms;
    effect::SemanticUniformList m_viewportPixelUniforms;
    effect::SemanticUniformList m_timeUniforms;
    effect::SemanticUniformList m_systemTimeUniforms;
    effect::SemanticUniformList m_elapsedTimeUniforms;
    effect::SemanticUniformList m_elapsedSystemTimeUniforms;
    effect::SemanticUniformList m_mousePositionUniforms;
    effect::SemanticUniformList m_leftMouseDownUniforms;
    effect::SemanticUniformList m_middleMouseDownUniforms;
    effect::SemanticUniformList m_rightMouseDownUniforms;
    effect::SemanticUniformList m_controlObjectUniforms;
    effect::ControlObjectTargetMap m_controlObjectTargets;
    effect::SemanticUniformList m_textureResourceUniforms;
    effect::SemanticImageMap m_resourceImages;
    ImageResourceList m_imageResources;
    DrawableNamedRenderTargetColorImageContainerMap m_drawableNamedRenderTargetColorImages;
    RenderTargetDepthStencilImageContainerMap m_renderTargetDepthStencilImages;
    effect::SemanticUniformList m_renderTargetColorUniforms;
    effect::SemanticUniformList m_renderTargetDepthStencilUniforms;
    effect::SemanticUniformList m_animatedTextureUniforms;
    AnimatedImageContainerMap m_animatedTextureImages;
    effect::SemanticUniformList m_offscreenRenderTargetUniforms;
    OffscreenRenderTargetImageContainerMap m_offscreenRenderTargetImages;
    effect::OffscreenRenderTargetOptionMap m_offscreenRenderTargetOptions;
    effect::SemanticUniformList m_textureValueUniforms;
    StringMap m_textureValueTargets;
    effect::ScriptCommandMap m_standardScript;
    effect::VectorParameterUniformMap m_vectorParameterUniforms;
    effect::FloatParameterUniformMap m_floatParameterUniforms;
    effect::IntParameterUniformMap m_intParameterUniforms;
    effect::BoolParameterUniformMap m_boolParameterUniforms;
    effect::TechniqueList m_allTechniques;
    TechniqueListMap m_techniqueByPassTypes;
    PassUniformBufferMap m_passUniformBuffer;
    StagingBufferMap m_imageStagingBuffers;
    OverridenImageHandleMap m_overridenImageHandles;
    ImageSamplerMap m_imageSamplers;
    NamedHandleMap m_namedImageHandles;
    NamedHandleMap m_namedShaderHandles;
    HashMap m_hashes;
    StringList m_errorMessages;
    String m_scriptOutput;
    ScriptClassType m_scriptClass;
    ScriptOrderType m_scriptOrder;
    UserData m_userData;
    StringList m_includePaths;
    effect::ViewPassSet m_viewPassSet;
    sg_pass_desc m_currentRenderTargetPassDescription;
    sg_image m_firstImageHandle;
    tinystl::pair<String, sg_image_desc> m_currentNamedPrimaryRenderTargetColorImageDescription;
    tinystl::pair<String, sg_image_desc> m_currentNamedDepthStencilImageDescription;
    PixelFormat m_currentRenderTargetPixelFormat;
    RenderTargetNormalizerMap m_normalizers;
    String m_name;
    URI m_fileURI;
    String m_filename;
    Vector4 m_clearColor;
    nanoem_f32_t m_clearDepth;
    tinystl::pair<bool, bool> m_enabled;
    bool m_enablePassUniformBufferInspection;
    bool m_initializeGlobal;
    bool m_hasScriptExternal;
    bool m_needsBehaviorCompatibility;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_GRID_H_ */
