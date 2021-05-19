/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_ACCESSORY_H_
#define NANOEM_EMAPP_ACCESSORY_H_

#include "emapp/Forward.h"

#include "emapp/BoundingBox.h"
#include "emapp/IDrawable.h"
#include "emapp/IEffect.h"
#include "emapp/URI.h"

#include "nanodxm/nanodxm.h"

struct undo_command_t;
struct undo_stack_t;

namespace nanoem {

class AccessoryProgramBundle;
class Archiver;
class Error;
class IEffect;
class ISeekableReader;
class IWriter;
class Image;
class ImageLoader;
class Progress;
class Project;

class Accessory NANOEM_DECL_SEALED : public IDrawable, private NonCopyable {
public:
    typedef void (*UserDataDestructor)(void *userData, const Accessory *accessory);
    typedef tinystl::pair<void *, UserDataDestructor> UserData;
    class Material {
    public:
        Material(sg_image fallbackImage);
        ~Material() NANOEM_DECL_NOEXCEPT;

        void destroy();

        const IImageView *diffuseImage() const;
        void setDiffuseImage(const IImageView *value);
        nanoem_model_material_sphere_map_texture_type_t sphereTextureMapType() const;
        void setSphereTextureMapType(nanoem_model_material_sphere_map_texture_type_t value);

    private:
        const IImageView *m_diffuseImagePtr;
        sg_image m_fallbackImage;
        nanoem_model_material_sphere_map_texture_type_t m_sphereTextureMapType;
    };
    BX_ALIGN_DECL_16(struct)
    VertexUnit
    {
        Vector4 m_position;
        Vector4 m_normal;
        Vector4 m_color;
        Vector4 m_texcoord;
        Vector4 m_uva[4];
        VertexUnit();
        ~VertexUnit() NANOEM_DECL_NOEXCEPT;
    };

    static const Matrix4x4 kInitialWorldMatrix;
    static StringList loadableExtensions();
    static StringSet loadableExtensionsSet();
    static bool isLoadableExtension(const String &extension);
    static bool isLoadableExtension(const URI &fileURI);
    static void setStandardPipelineDescription(sg_pipeline_desc &desc);

    Accessory(Project *project, nanoem_u16_t handle);
    ~Accessory() NANOEM_DECL_NOEXCEPT;

    bool load(const nanoem_u8_t *bytes, size_t length, Error &error);
    bool load(const ByteArray &bytes, Error &error);
    bool save(IWriter *writer, Error &error);
    bool save(ByteArray &bytes, Error &error);
    bool saveArchive(const String &prefix, Archiver &archiver, Error &error);
    void upload();
    bool uploadArchive(const String &entryPoint, const Archiver &archiver, Progress &progress, Error &error);
    bool uploadArchive(const String &entryPoint, ISeekableReader *reader, Progress &progress, Error &error);
    bool uploadArchive(ISeekableReader *reader, Progress &progress, Error &error);
    void loadAllImages(Progress &progress, Error &error);
    void writeLoadCommandMessage(Error &error);
    void writeDeleteCommandMessage(Error &error);
    void pushUndo(undo_command_t *command);
    void destroy();
    void synchronizeMotion(const Motion *motion, nanoem_frame_index_t frameIndex);
    void synchronizeOutsideParent(const nanoem_motion_accessory_keyframe_t *keyframe);
    IImageView *uploadImage(const String &filename, const sg_image_desc &desc) NANOEM_DECL_OVERRIDE;
    const Material *findMaterial(const nanodxm_material_t *material) const NANOEM_DECL_NOEXCEPT;
    void addAttachment(const String &name, const URI &fullPath);
    void removeAttachment(const String &name);
    FileEntityMap attachments() const;

    void draw(DrawType type) NANOEM_DECL_OVERRIDE;
    void reset() NANOEM_DECL_OVERRIDE;
    const IEffect *findOffscreenPassiveRenderTargetEffect(
        const String &offscreenOwnerName) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    IEffect *findOffscreenPassiveRenderTargetEffect(const String &offscreenOwnerName) NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setOffscreenDefaultRenderTargetEffect(const String &ownerName) NANOEM_DECL_OVERRIDE;
    void setOffscreenPassiveRenderTargetEffect(const String &offscreenOwnerName, IEffect *value) NANOEM_DECL_OVERRIDE;
    void removeOffscreenPassiveRenderTargetEffect(const String &offscreenOwnerName) NANOEM_DECL_OVERRIDE;
    bool isOffscreenPassiveRenderTargetEffectEnabled(
        const String &offscreenOwnerName) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setOffscreenPassiveRenderTargetEffectEnabled(
        const String &offscreenOwnerName, bool value) NANOEM_DECL_OVERRIDE;

    const Project *project() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    Project *project();
    nanoem_u16_t handle() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    String name() const NANOEM_DECL_OVERRIDE;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    String canonicalName() const NANOEM_DECL_OVERRIDE;
    const char *canonicalNameConstString() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setName(const String &value);
    StringMap annotations() const;
    void setAnnotations(const StringMap &value);
    UserData userData() const;
    void setUserData(const UserData &value);
    void getAllImageViews(ImageViewMap &value) const NANOEM_DECL_OVERRIDE;
    URI resolveImageURI(const String &filename) const;
    nanodxm_status_t status() const;
    String filename() const;
    const URI *fileURIPtr() const NANOEM_DECL_NOEXCEPT;
    URI fileURI() const NANOEM_DECL_OVERRIDE;
    URI resolvedFileURI() const;
    void setFileURI(const URI &value);
    const IEffect *activeEffect() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    IEffect *activeEffect() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setActiveEffect(IEffect *value) NANOEM_DECL_OVERRIDE;
    const IEffect *passiveEffect() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    IEffect *passiveEffect() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setPassiveEffect(IEffect *value) NANOEM_DECL_OVERRIDE;
    StringPair outsideParent() const;
    void setOutsideParent(const StringPair &value);
    Vector3 translation() const NANOEM_DECL_NOEXCEPT;
    void setTranslation(const Vector3 &value);
    Quaternion orientationQuaternion() const NANOEM_DECL_NOEXCEPT;
    void setOrientationQuaternion(const Quaternion &value);
    Vector3 orientation() const NANOEM_DECL_NOEXCEPT;
    void setOrientation(const Vector3 &value);
    nanodxm_float32_t opacity() const NANOEM_DECL_NOEXCEPT;
    void setOpacity(nanodxm_float32_t value);
    nanodxm_float32_t scaleFactor() const NANOEM_DECL_NOEXCEPT;
    void setScaleFactor(nanodxm_float32_t value);
    bool isGroundShadowEnabled() const NANOEM_DECL_NOEXCEPT;
    void setGroundShadowEnabled(bool value);
    bool isShadowMapEnabled() const NANOEM_DECL_NOEXCEPT;
    void setShadowMapEnabled(bool value);
    bool isAddBlendEnabled() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setAddBlendEnabled(bool value) NANOEM_DECL_OVERRIDE;
    bool isVisible() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setVisible(bool value) NANOEM_DECL_OVERRIDE;
    bool isDirty() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isTranslucent() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isUploaded() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    Matrix4x4 worldTransform() const NANOEM_DECL_NOEXCEPT;
    Matrix4x4 worldTransform(const Matrix4x4 &initialWorldTransform) const NANOEM_DECL_NOEXCEPT;
    Matrix4x4 fullWorldTransform() const NANOEM_DECL_NOEXCEPT;
    Matrix4x4 fullWorldTransform(const Matrix4x4 &initialWorldTransform) const NANOEM_DECL_NOEXCEPT;
    BoundingBox boundingBox() const;
    const undo_stack_t *undoStack() const;
    undo_stack_t *undoStack();
    nanodxm_document_t *data() const;

private:
    struct LoadingImageItem;
    struct OffscreenPassiveRenderTargetEffect {
        IEffect *m_passiveEffect;
        bool m_enabled;
    };
    typedef tinystl::unordered_map<String, Image *, TinySTLAllocator> ImageMap;
    typedef tinystl::vector<LoadingImageItem *, TinySTLAllocator> LoadingImageItemList;
    typedef tinystl::vector<nanoem_u16_t, TinySTLAllocator> Indices16;
    typedef tinystl::vector<Vector3, TinySTLAllocator> Vector3List;
    typedef tinystl::unordered_map<const nanodxm_material_t *, UInt32HashMap, TinySTLAllocator> MaterialIndexHashMap;
    typedef tinystl::unordered_map<String, OffscreenPassiveRenderTargetEffect, TinySTLAllocator>
        OffscreenPassiveRenderTargetEffectMap;

    static void trianguleNormal(const nanodxm_vector3_t *vertices, const glm::uvec3 &index, Vector3List &normalSum);
    const Image *createImage(const nanodxm_uint8_t *path);
    Image *internalUploadImage(const String &filename, const sg_image_desc &desc, bool fileExist);
    void clearAllLoadingImageItems();
    void createAllImages();
    void fillNormalBuffer(const Vector3List &normalSum, nanodxm_rsize_t numVertices, VertexUnit *ptr) const;
    void createVertexBuffer(const Vector3List &normalSum);
    template <typename TIndex>
    void createIndexBuffer(tinystl::vector<TIndex, TinySTLAllocator> &allIndices,
        tinystl::vector<tinystl::vector<TIndex, TinySTLAllocator>, TinySTLAllocator> &indicesPerMaterial);
    void createIndexBuffer(Vector3List &normalSum);
    template <typename TIndex> void calculateNormalSum(const TIndex &indices, Vector3List &normalSum);
    bool saveAllAttachments(
        const String &prefix, const FileEntityMap &allAttachments, Archiver &archiver, Error &error);
    bool getVertexIndexBufferAndTexture(const nanodxm_material_t *materialPtr, IPass::Buffer &buffer,
        sg_image &diffuseTexture, nanoem_model_material_sphere_map_texture_type_t &sphereTextureType) const;
    IEffect *internalEffect();
    void drawColor(bool scriptExternalColor);
    void drawGroundShadow();
    void drawShadowMap();

    const nanoem_u16_t m_handle;
    Project *m_project;
    Image *m_screenImage;
    undo_stack_t *m_undoStack;
    nanodxm_document_t *m_opaque;
    tinystl::pair<IEffect *, IEffect *> m_activeEffectPtrPair;
    StringPair m_outsideParent;
    OffscreenPassiveRenderTargetEffectMap m_offscreenPassiveRenderTargetEffects;
    ImageMap m_imageHandles;
    LoadingImageItemList m_loadingImageItems;
    FileEntityMap m_imageURIMap;
    FileEntityMap m_attachmentURIMap;
    typedef tinystl::unordered_map<const nanodxm_material_t *, Material *, TinySTLAllocator> MaterialList;
    MaterialList m_materials;
    typedef tinystl::unordered_map<int, int, TinySTLAllocator> FaceIndexList;
    FaceIndexList m_faceNormalList;
    BoundingBox m_boundingBox;
    UserData m_userData;
    StringMap m_annotations;
    sg_buffer m_vertexBuffer;
    sg_buffer m_indexBuffer;
    Vector3List m_normals;
    tinystl::vector<size_t, TinySTLAllocator> m_numIndices;
    String m_name;
    String m_canonicalName;
    URI m_fileURI;
    Vector3 m_translation;
    Vector3 m_orientation;
    nanodxm_float32_t m_opacity;
    nanodxm_float32_t m_scaleFactor;
    nanodxm_status_t m_status;
    nanoem_u32_t m_states;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_ACCESSORY_H_ */
