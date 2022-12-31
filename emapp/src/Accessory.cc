/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/Accessory.h"

#include "emapp/AccessoryProgramBundle.h"
#include "emapp/Archiver.h"
#include "emapp/Effect.h"
#include "emapp/EnumUtils.h"
#include "emapp/Error.h"
#include "emapp/FileUtils.h"
#include "emapp/IEventPublisher.h"
#include "emapp/IFileManager.h"
#include "emapp/ILight.h"
#include "emapp/ImageLoader.h"
#include "emapp/ListUtils.h"
#include "emapp/Progress.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#include "CommandMessage.inl"
#include "protoc/command.pb-c.h"

#include "glm/gtx/normal.hpp"
#include "glm/gtx/vector_query.hpp"
#include "sokol/sokol_time.h"
#include "undo/undo.h"

namespace nanoem {
namespace {

static const Matrix4x4 kShadowWorldMatrix(glm::scale(Constants::kIdentity, Vector3(10.0f)));

enum PrivateStateFlags {
    kPrivateStateVisible = 1 << 1,
    kPrivateStateUploaded = 1 << 2,
    kPrivateStateAddBlendEnabled = 1 << 3,
    kPrivateStateShadowMapEnabled = 1 << 4,
    kPrivateStateGroundShadowEnabled = 1 << 5,
    kPrivateStateReserved = 1 << 31,
};
static const nanoem_u32_t kPrivateStateInitialValue = kPrivateStateGroundShadowEnabled;

} /* namespace anonymous */

struct Accessory::LoadingImageItem {
    LoadingImageItem(const URI &fileURI, const String &filename)
        : m_fileURI(fileURI)
        , m_filename(filename)
        , m_usingWhiteFallback(true)
    {
        if (const char *p = StringUtils::indexOf(filename, '.')) {
            m_usingWhiteFallback = !StringUtils::equals(p, ".spa");
        }
    }
    URI m_fileURI;
    String m_filename;
    bool m_usingWhiteFallback;
};

Accessory::Material::Material(sg_image fallbackImage)
    : m_diffuseImagePtr(nullptr)
    , m_sphereTextureMapType(NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE)
{
    m_fallbackImage = fallbackImage;
}

Accessory::Material::~Material() NANOEM_DECL_NOEXCEPT
{
}

void
Accessory::Material::destroy()
{
}

const IImageView *
Accessory::Material::diffuseImage() const
{
    return m_diffuseImagePtr;
}

void
Accessory::Material::setDiffuseImage(const IImageView *value)
{
    m_diffuseImagePtr = value;
}

nanoem_model_material_sphere_map_texture_type_t
Accessory::Material::sphereTextureMapType() const
{
    return m_sphereTextureMapType;
}

void
Accessory::Material::setSphereTextureMapType(nanoem_model_material_sphere_map_texture_type_t value)
{
    m_sphereTextureMapType = value;
}

Accessory::VertexUnit::VertexUnit()
    : m_position(Constants::kZeroV4)
    , m_normal(Constants::kZeroV4)
    , m_color(Constants::kZeroV4)
    , m_texcoord(Constants::kZeroV4)
{
    Inline::clearZeroMemory(m_uva);
}

Accessory::VertexUnit::~VertexUnit() NANOEM_DECL_NOEXCEPT
{
}

const Matrix4x4 Accessory::kInitialWorldMatrix = glm::scale(Matrix4x4(1), Vector3(10.0f));

StringList
Accessory::loadableExtensions()
{
    static const String kLoadableAccessoryExtensions[] = { String("x"), String() };
    return StringList(
        &kLoadableAccessoryExtensions[0], &kLoadableAccessoryExtensions[BX_COUNTOF(kLoadableAccessoryExtensions) - 1]);
}

StringSet
Accessory::loadableExtensionsSet()
{
    return ListUtils::toSetFromList<String>(loadableExtensions());
}

bool
Accessory::isLoadableExtension(const String &extension)
{
    return FileUtils::isLoadableExtension(extension, loadableExtensionsSet());
}

bool
Accessory::isLoadableExtension(const URI &fileURI)
{
    return isLoadableExtension(fileURI.pathExtension());
}

void
Accessory::setStandardPipelineDescription(sg_pipeline_desc &desc)
{
    sg_layout_desc &ld = desc.layout;
    ld.buffers[0].stride = sizeof(VertexUnit);
    ld.attrs[0] =
        sg_vertex_attr_desc { 0, Inline::saturateInt32(offsetof(VertexUnit, m_position)), SG_VERTEXFORMAT_FLOAT3 };
    ld.attrs[1] =
        sg_vertex_attr_desc { 0, Inline::saturateInt32(offsetof(VertexUnit, m_normal)), SG_VERTEXFORMAT_FLOAT3 };
    ld.attrs[2] =
        sg_vertex_attr_desc { 0, Inline::saturateInt32(offsetof(VertexUnit, m_texcoord)), SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[3] =
        sg_vertex_attr_desc { 0, Inline::saturateInt32(offsetof(VertexUnit, m_uva[0])), SG_VERTEXFORMAT_FLOAT4 };
    ld.attrs[4] =
        sg_vertex_attr_desc { 0, Inline::saturateInt32(offsetof(VertexUnit, m_uva[1])), SG_VERTEXFORMAT_FLOAT4 };
    ld.attrs[5] =
        sg_vertex_attr_desc { 0, Inline::saturateInt32(offsetof(VertexUnit, m_uva[2])), SG_VERTEXFORMAT_FLOAT4 };
    ld.attrs[6] =
        sg_vertex_attr_desc { 0, Inline::saturateInt32(offsetof(VertexUnit, m_uva[3])), SG_VERTEXFORMAT_FLOAT4 };
    ld.attrs[7] =
        sg_vertex_attr_desc { 0, Inline::saturateInt32(offsetof(VertexUnit, m_color)), SG_VERTEXFORMAT_FLOAT4 };
    desc.index_type = SG_INDEXTYPE_UINT32;
    Project::setStandardDepthStencilState(desc.depth, desc.stencil);
}

Accessory::Accessory(Project *project, nanoem_u16_t handle)
    : m_handle(handle)
    , m_project(project)
    , m_screenImage(nullptr)
    , m_undoStack(nullptr)
    , m_opaque(nullptr)
    , m_userData(nullptr, nullptr)
    , m_translation(Constants::kZeroV3)
    , m_orientation(Constants::kZeroV3)
    , m_opacity(1.0f)
    , m_scaleFactor(1.0f)
    , m_status(NANODXM_STATUS_SUCCESS)
    , m_states(kPrivateStateInitialValue)
{
    nanoem_assert(m_project, "must not be nullptr");
    m_activeEffectPtrPair.first = m_project->sharedResourceRepository()->accessoryProgramBundle();
    m_activeEffectPtrPair.second = nullptr;
    m_opaque = nanodxmDocumentCreate();
    m_undoStack = undoStackCreateWithSoftLimit(undoStackGetSoftLimit(m_project->undoStack()));
    m_vertexBuffer = m_indexBuffer = { SG_INVALID_ID };
}

Accessory::~Accessory() NANOEM_DECL_NOEXCEPT
{
    for (MaterialList::const_iterator it = m_materials.begin(), end = m_materials.end(); it != end; ++it) {
        nanoem_delete(it->second);
    }
    m_materials.clear();
    nanoem_delete_safe(m_screenImage);
    undoStackDestroy(m_undoStack);
    m_undoStack = nullptr;
    nanodxmDocumentDestroy(m_opaque);
    m_opaque = nullptr;
    m_opacity = 0.0f;
    m_scaleFactor = 0.0f;
    m_opaque = nullptr;
    m_states = 0;
    m_project = nullptr;
}

bool
Accessory::load(const nanoem_u8_t *bytes, size_t length, Error &error)
{
    nanoem_parameter_assert(bytes, "must not be nullptr");
    nanodxm_buffer_t *buffer = nanodxmBufferCreate(bytes, length);
    nanodxm_status_t status = m_status = nanodxmDocumentParse(m_opaque, buffer);
    bool succeeded = status == NANODXM_STATUS_SUCCESS;
    nanodxmBufferDestroy(buffer);
    if (!succeeded) {
        char message[Error::kMaxReasonLength];
        StringUtils::format(message, sizeof(message), "Cannot load the accessory: %d", status);
        error = Error(message, status, Error::kDomainTypeNanodxm);
    }
    return succeeded;
}

bool
Accessory::load(const ByteArray &bytes, Error &error)
{
    return load(bytes.data(), bytes.size(), error);
}

bool
Accessory::save(IWriter *writer, Error &error)
{
    nanoem_parameter_assert(writer, "must NOT be nullptr");
    const URI &uri = fileURI();
    bool succeeded = false;
    if (Project::isArchiveURI(uri)) {
        const URI &baseURI = resolvedFileURI();
        FileReaderScope scope(m_project->translator());
        if (scope.open(baseURI, error)) {
            Archiver sourceArchive(scope.reader());
            if (sourceArchive.open(error)) {
                Archiver::Entry entry;
                ByteArray bytes;
                if (sourceArchive.findEntry(baseURI.fragment(), entry, error) &&
                    sourceArchive.extract(entry, bytes, error)) {
                    FileUtils::write(writer, bytes, error);
                    succeeded = true;
                }
                sourceArchive.close(error);
            }
        }
    }
    else {
        FileReaderScope scope(m_project->translator());
        if (scope.open(uri, error)) {
            ByteArray bytes;
            FileUtils::read(scope, bytes, error);
            FileUtils::write(writer, bytes, error);
            succeeded = true;
        }
    }
    return succeeded;
}

bool
Accessory::save(ByteArray &bytes, Error &error)
{
    MemoryWriter writer(&bytes);
    return save(&writer, error);
}

bool
Accessory::saveArchive(const String &prefix, Archiver &archiver, Error &error)
{
    Archiver::Entry entry;
    ByteArray bytes;
    bool succeeded = save(bytes, error);
    if (succeeded) {
        String path(prefix);
        path.append(filename().c_str());
        entry.m_path = path;
        if (archiver.addEntry(entry, bytes, error)) {
            FileEntityMap allAttachments;
            for (FileEntityMap::const_iterator it = m_imageURIMap.begin(), end = m_imageURIMap.end(); it != end; ++it) {
                allAttachments.insert(tinystl::make_pair(it->first, it->second));
            }
            for (FileEntityMap::const_iterator it = m_attachmentURIMap.begin(), end = m_attachmentURIMap.end();
                 it != end; ++it) {
                allAttachments.insert(tinystl::make_pair(it->first, it->second));
            }
            if (const Effect *effect = m_project->resolveEffect(this)) {
                effect->attachAllResources(allAttachments);
                for (OffscreenPassiveRenderTargetEffectMap::const_iterator
                         it = m_offscreenPassiveRenderTargetEffects.begin(),
                         end = m_offscreenPassiveRenderTargetEffects.end();
                     it != end; ++it) {
                    const OffscreenPassiveRenderTargetEffect &effect = it->second;
                    if (const Effect *passiveEffect = m_project->upcastEffect(effect.m_passiveEffect)) {
                        passiveEffect->attachAllResources(allAttachments);
                    }
                }
            }
            succeeded &= saveAllAttachments(prefix, allAttachments, archiver, error);
        }
        else {
            succeeded = false;
        }
    }
    return succeeded;
}

void
Accessory::upload()
{
    SG_PUSH_GROUPF("Accessory::upload(name=%s)", canonicalNameConstString());
    MaterialIndexHashMap materialIndexHash;
    Indices16 allIndices;
    tinystl::vector<Indices16, TinySTLAllocator> indicesPerMaterial;
    nanodxm_rsize_t numVertices;
    nanodxmDocumentGetVertices(m_opaque, &numVertices);
    Vector3List normalSum;
    createIndexBuffer(normalSum);
    createVertexBuffer(normalSum);
    createAllImages();
    setActiveEffect(m_project->sharedResourceRepository()->accessoryProgramBundle());
    setVisible(true);
    EnumUtils::setEnabled(kPrivateStateUploaded, m_states, true);
    SG_POP_GROUP();
}

bool
Accessory::uploadArchive(ISeekableReader *reader, Progress &progress, Error &error)
{
    Archiver archiver(reader);
    bool succeeded = false;
    if (archiver.open(error)) {
        const Archiver::EntryList &entries = archiver.allEntries(error);
        for (Archiver::EntryList::const_iterator it = entries.begin(), end = entries.end(); it != end; ++it) {
            const Archiver::Entry &entry = *it;
            const String &path = entry.m_path;
            const char *p = strrchr(path.c_str(), '.');
            if (p && isLoadableExtension(p + 1)) {
                archiver.close(error);
                succeeded = uploadArchive(path, reader, progress, error);
                break;
            }
        }
    }
    return succeeded;
}

bool
Accessory::uploadArchive(const String &entryPoint, const Archiver &archiver, Progress &progress, Error &error)
{
    Archiver::Entry entry;
    bool succeeded = false;
    if (archiver.findEntry(entryPoint, entry, error)) {
        ByteArray bytes;
        archiver.extract(entry, bytes, error);
        if (archiver.extract(entry, bytes, error) && !bytes.empty() && load(bytes.data(), bytes.size(), error)) {
            SG_PUSH_GROUPF("Accessory::uploadArchive(name=%s)", canonicalNameConstString());
            upload();
            ImageLoader *imageLoader = m_project->sharedImageLoader();
            for (LoadingImageItemList::const_iterator it = m_loadingImageItems.begin(), end = m_loadingImageItems.end();
                 it != end; ++it) {
                const LoadingImageItem *item = *it;
                const URI &fileURI = item->m_fileURI;
                const String &filename = fileURI.fragment();
                if (!progress.tryLoadingItem(fileURI)) {
                    error = Error::cancelled();
                    break;
                }
                else if (archiver.findEntry(filename, entry, error) && archiver.extract(entry, bytes, error)) {
                    imageLoader->decode(bytes, item->m_filename, this, SG_WRAP_REPEAT, 0, error);
                }
                else {
                    sg_image_desc desc;
                    const nanoem_u32_t pixel = item->m_usingWhiteFallback ? 0xffffffff : 0x0;
                    ImageLoader::fill1x1PixelImage(&pixel, desc);
                    internalUploadImage(item->m_filename, desc, false);
                }
            }
            SG_POP_GROUP();
            clearAllLoadingImageItems();
            succeeded = !error.hasReason();
        }
    }
    return succeeded;
}

bool
Accessory::uploadArchive(const String &entryPoint, ISeekableReader *reader, Progress &progress, Error &error)
{
    bool succeeded = false;
    if (!entryPoint.empty()) {
        Archiver archiver(reader);
        if (archiver.open(error)) {
            succeeded = uploadArchive(entryPoint, archiver, progress, error);
            archiver.close(error);
        }
    }
    return succeeded;
}

void
Accessory::loadAllImages(Progress &progress, Error &error)
{
    SG_PUSH_GROUPF("Accessory::loadAllImages(name=%s)", canonicalNameConstString());
    ImageLoader *imageLoader = m_project->sharedImageLoader();
    for (LoadingImageItemList::const_iterator it = m_loadingImageItems.begin(), end = m_loadingImageItems.end();
         it != end; ++it) {
        const LoadingImageItem *item = *it;
        const URI &fileURI = item->m_fileURI;
        if (!progress.tryLoadingItem(fileURI)) {
            error = Error::cancelled();
            break;
        }
        else if (!imageLoader->load(fileURI, this, SG_WRAP_REPEAT, ImageLoader::kFlagsEnableMipmap, error)) {
            sg_image_desc desc;
            const nanoem_u32_t pixel = item->m_usingWhiteFallback ? 0xffffffff : 0x0;
            ImageLoader::fill1x1PixelImage(&pixel, desc);
            uploadImage(item->m_filename, desc);
        }
        progress.increment();
    }
    clearAllLoadingImageItems();
    SG_POP_GROUP();
}

void
Accessory::writeLoadCommandMessage(Error &error)
{
    Nanoem__Application__RedoLoadAccessoryCommand command = NANOEM__APPLICATION__REDO_LOAD_ACCESSORY_COMMAND__INIT;
    MutableString pathString, fragmentString, nameString;
    command.content_case = NANOEM__APPLICATION__REDO_LOAD_ACCESSORY_COMMAND__CONTENT_FILE_URI;
    Nanoem__Application__URI uri = NANOEM__APPLICATION__URI__INIT;
    uri.absolute_path = StringUtils::cloneString(m_fileURI.absolutePathConstString(), pathString);
    uri.fragment = StringUtils::cloneString(m_fileURI.fragmentConstString(), fragmentString);
    command.file_uri = &uri;
    command.accessory_handle = m_handle;
    command.name = StringUtils::cloneString(nameConstString(), nameString);
    Nanoem__Application__Command action = NANOEM__APPLICATION__COMMAND__INIT;
    action.timestamp = stm_now();
    action.redo_load_accessory = &command;
    action.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REDO_LOAD_ACCESSORY;
    m_project->writeRedoMessage(&action, error);
}

void
Accessory::writeDeleteCommandMessage(Error &error)
{
    Nanoem__Application__RedoDeleteAccessoryCommand command = NANOEM__APPLICATION__REDO_DELETE_ACCESSORY_COMMAND__INIT;
    command.accessory_handle = m_handle;
    Nanoem__Application__Command action = NANOEM__APPLICATION__COMMAND__INIT;
    action.timestamp = stm_now();
    action.redo_delete_accessory = &command;
    action.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REDO_DELETE_ACCESSORY;
    m_project->writeRedoMessage(&action, error);
}

void
Accessory::pushUndo(undo_command_t *command)
{
    nanoem_assert(!m_project->isPlaying(), "must not be called while playing");
    if (!m_project->isPlaying()) {
        undoStackPushCommand(m_project->undoStack(), command);
        m_project->eventPublisher()->publishPushUndoCommandEvent(command);
    }
    else {
        undoCommandDestroy(command);
    }
}

void
Accessory::destroy()
{
    SG_PUSH_GROUPF("Accessory::destroy(name=%s", canonicalNameConstString());
    setVisible(false);
    undoStackClear(m_undoStack);
    if (UserDataDestructor destructor = m_userData.second) {
        destructor(m_userData.first, this);
    }
    for (MaterialList::const_iterator it = m_materials.begin(), end = m_materials.end(); it != end; ++it) {
        if (Material *material = it->second) {
            material->destroy();
        }
    }
    for (ImageMap::const_iterator it = m_imageHandles.begin(), end = m_imageHandles.end(); it != end; ++it) {
        Image *image = it->second;
        SG_INSERT_MARKERF("Accessory::destroy(image=%d, name=%s)", image->handle().id, it->first.c_str());
        image->destroy();
        nanoem_delete(image);
    }
    SG_INSERT_MARKERF("Accessory::destroy(vertex=%d, index=%d)", m_vertexBuffer.id, m_indexBuffer.id);
    sg::destroy_buffer(m_vertexBuffer);
    sg::destroy_buffer(m_indexBuffer);
    m_imageHandles.clear();
    nanoem_delete_safe(m_screenImage);
    SG_POP_GROUP();
}

void
Accessory::synchronizeMotion(const Motion *motion, nanoem_frame_index_t frameIndex)
{
    const nanoem_motion_accessory_keyframe_t *keyframe;
    if (motion) {
        keyframe = motion->findAccessoryKeyframe(frameIndex);
        m_outsideParent = StringPair();
        if (keyframe) {
            setTranslation(glm::make_vec3(nanoemMotionAccessoryKeyframeGetTranslation(keyframe)));
            setOrientationQuaternion(glm::make_quat(nanoemMotionAccessoryKeyframeGetOrientation(keyframe)));
            setOpacity(nanoemMotionAccessoryKeyframeGetOpacity(keyframe));
            setScaleFactor(nanoemMotionAccessoryKeyframeGetScaleFactor(keyframe));
            setVisible(nanoemMotionAccessoryKeyframeIsVisible(keyframe) != 0);
            setAddBlendEnabled(nanoemMotionAccessoryKeyframeIsAddBlendEnabled(keyframe) != 0);
            setGroundShadowEnabled(nanoemMotionAccessoryKeyframeIsShadowEnabled(keyframe) != 0);
            synchronizeOutsideParent(keyframe);
            if (IEffect *effect = activeEffect()) {
                nanoem_rsize_t numParameters;
                nanoem_motion_effect_parameter_t *const *parameters =
                    nanoemMotionAccessoryKeyframeGetAllEffectParameterObjects(keyframe, &numParameters);
                effect->setAllParameterObjects(parameters, numParameters);
            }
        }
        else {
            nanoem_motion_accessory_keyframe_t *prevKeyframe, *nextKeyframe;
            nanoemMotionSearchClosestAccessoryKeyframes(motion->data(), frameIndex, &prevKeyframe, &nextKeyframe);
            if (prevKeyframe && nextKeyframe) {
                const nanoem_f32_t &coef = Motion::coefficient(prevKeyframe, nextKeyframe, frameIndex);
                setTranslation(glm::mix(glm::make_vec3(nanoemMotionAccessoryKeyframeGetTranslation(prevKeyframe)),
                    glm::make_vec3(nanoemMotionAccessoryKeyframeGetTranslation(nextKeyframe)), coef));
                setOrientationQuaternion(
                    glm::slerp(glm::make_quat(nanoemMotionAccessoryKeyframeGetOrientation(prevKeyframe)),
                        glm::make_quat(nanoemMotionAccessoryKeyframeGetOrientation(nextKeyframe)), coef));
                setOpacity(glm::mix(nanoemMotionAccessoryKeyframeGetOpacity(prevKeyframe),
                    nanoemMotionAccessoryKeyframeGetOpacity(nextKeyframe), coef));
                setScaleFactor(glm::mix(nanoemMotionAccessoryKeyframeGetScaleFactor(prevKeyframe),
                    nanoemMotionAccessoryKeyframeGetScaleFactor(nextKeyframe), coef));
                setVisible(nanoemMotionAccessoryKeyframeIsVisible(prevKeyframe) != 0);
                setAddBlendEnabled(nanoemMotionAccessoryKeyframeIsAddBlendEnabled(prevKeyframe) != 0);
                setGroundShadowEnabled(nanoemMotionAccessoryKeyframeIsShadowEnabled(prevKeyframe) != 0);
                synchronizeOutsideParent(prevKeyframe);
                if (IEffect *effect = activeEffect()) {
                    nanoem_rsize_t numFromParameters, numToParameters;
                    nanoem_motion_effect_parameter_t *const *fromParameters =
                        nanoemMotionAccessoryKeyframeGetAllEffectParameterObjects(prevKeyframe, &numFromParameters);
                    nanoem_motion_effect_parameter_t *const *toParameters =
                        nanoemMotionAccessoryKeyframeGetAllEffectParameterObjects(nextKeyframe, &numToParameters);
                    effect->setAllParameterObjects(
                        fromParameters, numFromParameters, toParameters, numToParameters, coef);
                }
            }
        }
    }
}

void
Accessory::synchronizeOutsideParent(const nanoem_motion_accessory_keyframe_t *keyframe)
{
    if (const nanoem_motion_outside_parent_t *op = nanoemMotionAccessoryKeyframeGetOutsideParent(keyframe)) {
        nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
        String modelName;
        StringUtils::getUtf8String(nanoemMotionOutsideParentGetTargetObjectName(op), factory, modelName);
        if (const Model *model = m_project->findModelByName(modelName)) {
            m_outsideParent.first = model->name();
        }
        StringUtils::getUtf8String(nanoemMotionOutsideParentGetTargetBoneName(op), factory, m_outsideParent.second);
    }
}

IImageView *
Accessory::uploadImage(const String &filename, const sg_image_desc &desc)
{
    return internalUploadImage(filename, desc, true);
}

const Accessory::Material *
Accessory::findMaterial(const nanodxm_material_t *material) const NANOEM_DECL_NOEXCEPT
{
    Accessory::MaterialList::const_iterator it = m_materials.find(material);
    return it != m_materials.end() ? it->second : nullptr;
}

void
Accessory::addAttachment(const String &name, const URI &fullPath)
{
    m_attachmentURIMap.insert(tinystl::make_pair(name, fullPath));
}

void
Accessory::removeAttachment(const String &name)
{
    FileEntityMap::const_iterator it = m_attachmentURIMap.find(name);
    if (it != m_attachmentURIMap.end()) {
        m_attachmentURIMap.erase(it);
    }
}

void
Accessory::reset()
{
    setTranslation(Constants::kZeroV3);
    setOrientation(Constants::kZeroV3);
    setScaleFactor(1.0f);
    setOpacity(1.0f);
}

void
Accessory::draw(DrawType type)
{
    if (isVisible()) {
        switch (type) {
        case IDrawable::kDrawTypeColor:
        case IDrawable::kDrawTypeScriptExternalColor:
            drawColor(type == IDrawable::kDrawTypeScriptExternalColor);
            break;
        case IDrawable::kDrawTypeGroundShadow: {
            if (isGroundShadowEnabled()) {
                drawGroundShadow();
            }
            break;
        }
        case IDrawable::kDrawTypeShadowMap:
            if (isShadowMapEnabled()) {
                drawShadowMap();
            }
            break;
        case IDrawable::kDrawTypeEdge:
        case IDrawable::kDrawTypeMaxEnum:
        default:
            break;
        }
    }
}

const IEffect *
Accessory::findOffscreenPassiveRenderTargetEffect(const String &ownerName) const NANOEM_DECL_NOEXCEPT
{
    OffscreenPassiveRenderTargetEffectMap::const_iterator it = m_offscreenPassiveRenderTargetEffects.find(ownerName);
    return it != m_offscreenPassiveRenderTargetEffects.end() ? it->second.m_passiveEffect : nullptr;
}

IEffect *
Accessory::findOffscreenPassiveRenderTargetEffect(const String &ownerName) NANOEM_DECL_NOEXCEPT
{
    OffscreenPassiveRenderTargetEffectMap::const_iterator it = m_offscreenPassiveRenderTargetEffects.find(ownerName);
    return it != m_offscreenPassiveRenderTargetEffects.end() ? it->second.m_passiveEffect : nullptr;
}

void
Accessory::setOffscreenDefaultRenderTargetEffect(const String &ownerName)
{
    setOffscreenPassiveRenderTargetEffect(ownerName, m_project->sharedResourceRepository()->accessoryProgramBundle());
}

void
Accessory::setOffscreenPassiveRenderTargetEffect(const String &ownerName, IEffect *value)
{
    if (!ownerName.empty() && value) {
        if (ownerName == Effect::kOffscreenOwnerNameMain) {
            setActiveEffect(value);
        }
        else {
            OffscreenPassiveRenderTargetEffectMap::iterator it = m_offscreenPassiveRenderTargetEffects.find(ownerName);
            if (it != m_offscreenPassiveRenderTargetEffects.end()) {
                OffscreenPassiveRenderTargetEffect &effect = it->second;
                effect.m_passiveEffect = value;
            }
            else {
                const OffscreenPassiveRenderTargetEffect effect = { value, true };
                m_offscreenPassiveRenderTargetEffects.insert(tinystl::make_pair(ownerName, effect));
                if (Effect *innerEffect = m_project->upcastEffect(value)) {
                    innerEffect->createAllDrawableRenderTargetColorImages(this);
                }
            }
        }
    }
}

void
Accessory::removeOffscreenPassiveRenderTargetEffect(const String &ownerName)
{
    if (ownerName == Effect::kOffscreenOwnerNameMain) {
        setActiveEffect(nullptr);
    }
    else {
        OffscreenPassiveRenderTargetEffectMap::const_iterator it =
            m_offscreenPassiveRenderTargetEffects.find(ownerName);
        if (it != m_offscreenPassiveRenderTargetEffects.end()) {
            m_offscreenPassiveRenderTargetEffects.erase(it);
        }
    }
}

bool
Accessory::isOffscreenPassiveRenderTargetEffectEnabled(const String &ownerName) const NANOEM_DECL_NOEXCEPT
{
    OffscreenPassiveRenderTargetEffectMap::const_iterator it = m_offscreenPassiveRenderTargetEffects.find(ownerName);
    return it != m_offscreenPassiveRenderTargetEffects.end() ? it->second.m_enabled : false;
}

void
Accessory::setOffscreenPassiveRenderTargetEffectEnabled(const String &ownerName, bool value)
{
    if (!ownerName.empty()) {
        OffscreenPassiveRenderTargetEffectMap::iterator it = m_offscreenPassiveRenderTargetEffects.find(ownerName);
        if (it != m_offscreenPassiveRenderTargetEffects.end()) {
            it->second.m_enabled = value;
        }
        else {
            OffscreenPassiveRenderTargetEffect effect = { nullptr, value };
            m_offscreenPassiveRenderTargetEffects.insert(tinystl::make_pair(ownerName, effect));
        }
    }
}

FileEntityMap
Accessory::attachments() const
{
    return m_attachmentURIMap;
}

const Project *
Accessory::project() const NANOEM_DECL_NOEXCEPT
{
    return m_project;
}

Project *
Accessory::project()
{
    return m_project;
}

nanoem_u16_t
Accessory::handle() const NANOEM_DECL_NOEXCEPT
{
    return m_handle;
}

String
Accessory::name() const
{
    return m_name;
}

const char *
Accessory::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_name.c_str();
}

String
Accessory::canonicalName() const
{
    return m_name.empty() ? m_canonicalName : m_name;
}

const char *
Accessory::canonicalNameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_name.empty() ? m_canonicalName.c_str() : m_name.c_str();
}

void
Accessory::setName(const String &value)
{
    if (!(m_name == value)) {
        m_name = value;
    }
}

StringMap
Accessory::annotations() const
{
    return m_annotations;
}

void
Accessory::setAnnotations(const StringMap &value)
{
    m_annotations = value;
}

Accessory::UserData
Accessory::userData() const
{
    return m_userData;
}

void
Accessory::setUserData(const UserData &value)
{
    m_userData = value;
}

void
Accessory::getAllImageViews(ImageViewMap &value) const
{
    value.clear();
    for (ImageMap::const_iterator it = m_imageHandles.begin(), end = m_imageHandles.end(); it != end; ++it) {
        value.insert(tinystl::make_pair(it->first, static_cast<IImageView *>(it->second)));
    }
}

URI
Accessory::resolveImageURI(const String &filename) const
{
    nanoem_parameter_assert(!filename.empty(), "must not be empty");
    FileEntityMap::const_iterator it = m_imageURIMap.find(filename);
    return it != m_imageURIMap.end() ? it->second : URI();
}

nanodxm_status_t
Accessory::status() const
{
    return m_status;
}

String
Accessory::filename() const
{
    const URI &uri = fileURI();
    return Project::isArchiveURI(uri) ? URI::lastPathComponent(uri.fragment()) : uri.lastPathComponent();
}

const URI *
Accessory::fileURIPtr() const NANOEM_DECL_NOEXCEPT
{
    return &m_fileURI;
}

URI
Accessory::fileURI() const
{
    return m_fileURI;
}

URI
Accessory::resolvedFileURI() const
{
    return m_project->resolveFileURI(m_fileURI);
}

void
Accessory::setFileURI(const URI &value)
{
    m_fileURI = value;
    m_canonicalName = URI::lastPathComponent(value.absolutePath());
}

const IEffect *
Accessory::activeEffect() const NANOEM_DECL_NOEXCEPT
{
    nanoem_assert(m_activeEffectPtrPair.first, "must be called after Accessory::setActiveEffect");
    return m_activeEffectPtrPair.first;
}

IEffect *
Accessory::activeEffect() NANOEM_DECL_NOEXCEPT
{
    nanoem_assert(m_activeEffectPtrPair.first, "must be called after Accessory::setActiveEffect");
    return m_activeEffectPtrPair.first;
}

void
Accessory::setActiveEffect(IEffect *value)
{
    AccessoryProgramBundle *bundle = m_project->sharedResourceRepository()->accessoryProgramBundle();
    OffscreenPassiveRenderTargetEffect effect = { nullptr, true };
    if (value && value != bundle) {
        effect.m_passiveEffect = value;
        m_activeEffectPtrPair.first = value;
    }
    else {
        effect.m_passiveEffect = static_cast<IEffect *>(bundle);
        m_activeEffectPtrPair.first = bundle;
    }
    m_offscreenPassiveRenderTargetEffects[Effect::kOffscreenOwnerNameMain] = effect;
}

const IEffect *
Accessory::passiveEffect() const NANOEM_DECL_NOEXCEPT
{
    return m_activeEffectPtrPair.second;
}

IEffect *
Accessory::passiveEffect() NANOEM_DECL_NOEXCEPT
{
    return m_activeEffectPtrPair.second;
}

void
Accessory::setPassiveEffect(IEffect *value)
{
    if (m_activeEffectPtrPair.second != value) {
        if (Effect *effect = m_project->upcastEffect(value)) {
            effect->createAllDrawableRenderTargetColorImages(this);
        }
        m_activeEffectPtrPair.second = value;
    }
}

StringPair
Accessory::outsideParent() const
{
    return m_outsideParent;
}

void
Accessory::setOutsideParent(const StringPair &value)
{
    m_outsideParent = value;
}

Vector3
Accessory::translation() const NANOEM_DECL_NOEXCEPT
{
    return m_translation;
}

void
Accessory::setTranslation(const Vector3 &value)
{
    m_translation = value;
}

Quaternion
Accessory::orientationQuaternion() const NANOEM_DECL_NOEXCEPT
{
    return Quaternion(m_orientation);
}

void
Accessory::setOrientationQuaternion(const Quaternion &value)
{
    m_orientation = glm::eulerAngles(value);
}

Vector3
Accessory::orientation() const NANOEM_DECL_NOEXCEPT
{
    return m_orientation;
}

void
Accessory::setOrientation(const Vector3 &value)
{
    m_orientation = value;
}

nanodxm_float32_t
Accessory::opacity() const NANOEM_DECL_NOEXCEPT
{
    return m_opacity;
}

void
Accessory::setOpacity(nanodxm_float32_t value)
{
    m_opacity = value;
}

nanodxm_float32_t
Accessory::scaleFactor() const NANOEM_DECL_NOEXCEPT
{
    return m_scaleFactor;
}

void
Accessory::setScaleFactor(nanodxm_float32_t value)
{
    m_scaleFactor = value;
}

bool
Accessory::isGroundShadowEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateGroundShadowEnabled, m_states);
}

void
Accessory::setGroundShadowEnabled(bool value)
{
    if (isGroundShadowEnabled() != value) {
        EnumUtils::setEnabled(kPrivateStateGroundShadowEnabled, m_states, value);
        if (m_project->activeAccessory() == this) {
            m_project->eventPublisher()->publishToggleActiveAccessoryGroundShadowEnabledEvent(value);
        }
    }
}

bool
Accessory::isShadowMapEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateShadowMapEnabled, m_states);
}

void
Accessory::setShadowMapEnabled(bool value)
{
    if (isShadowMapEnabled() != value) {
        EnumUtils::setEnabled(kPrivateStateShadowMapEnabled, m_states, value);
    }
}

bool
Accessory::isAddBlendEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateAddBlendEnabled, m_states);
}

void
Accessory::setAddBlendEnabled(bool value)
{
    if (isAddBlendEnabled() != value) {
        EnumUtils::setEnabled(kPrivateStateAddBlendEnabled, m_states, value);
        if (m_project->activeAccessory() == this) {
            m_project->eventPublisher()->publishToggleActiveAccessoryAddBlendEnabledEvent(value);
        }
    }
}

bool
Accessory::isVisible() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateVisible, m_states);
}

void
Accessory::setVisible(bool value)
{
    if (isVisible() != value) {
        EnumUtils::setEnabled(kPrivateStateVisible, m_states, value);
        if (Effect *effect = m_project->resolveEffect(this)) {
            effect->setEnabled(value);
        }
        if (m_project->activeAccessory() == this) {
            m_project->eventPublisher()->publishToggleActiveAccessoryVisibleEvent(value);
        }
    }
}

bool
Accessory::isDirty() const NANOEM_DECL_NOEXCEPT
{
    return undoStackIsDirty(m_undoStack) ? true : false;
}

bool
Accessory::isTranslucent() const NANOEM_DECL_NOEXCEPT
{
    return opacity() - Constants::kEpsilon < 0.5f;
}

bool
Accessory::isUploaded() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateUploaded, m_states);
}

Matrix4x4
Accessory::worldTransform() const NANOEM_DECL_NOEXCEPT
{
    return worldTransform(kInitialWorldMatrix);
}

Matrix4x4
Accessory::worldTransform(const Matrix4x4 &initialWorldTransform) const NANOEM_DECL_NOEXCEPT
{
    const model::Bone *bonePtr = model::Bone::cast(m_project->resolveBone(m_outsideParent));
    const Vector3 scaleFactor(glm::max(Constants::kZeroV3, Vector3(m_scaleFactor)));
    const Matrix4x4 worldMatrix(glm::scale(bonePtr ? bonePtr->worldTransform() : Constants::kIdentity, scaleFactor));
    return worldMatrix * initialWorldTransform;
}

Matrix4x4
Accessory::fullWorldTransform() const NANOEM_DECL_NOEXCEPT
{
    return fullWorldTransform(kInitialWorldMatrix);
}

Matrix4x4
Accessory::fullWorldTransform(const Matrix4x4 &initialWorldTransform) const NANOEM_DECL_NOEXCEPT
{
    const model::Bone *bonePtr = model::Bone::cast(m_project->resolveBone(m_outsideParent));
    const Vector3 scaleFactor(glm::max(Constants::kZeroV3, Vector3(m_scaleFactor)));
    const Matrix4x4 boneTransform(bonePtr ? bonePtr->worldTransform() : Constants::kIdentity);
    const Matrix4x4 translateMatrix(glm::translate(Constants::kIdentity, m_translation));
    const Matrix4x4 rotateX(glm::rotate(Constants::kIdentity, m_orientation.x, Constants::kUnitX));
    const Matrix4x4 rotateY(glm::rotate(Constants::kIdentity, m_orientation.y, Constants::kUnitY));
    const Matrix4x4 rotateZ(glm::rotate(Constants::kIdentity, m_orientation.z, Constants::kUnitZ));
    const Matrix4x4 rotateMatrix(rotateZ * rotateY * rotateX);
    const Matrix4x4 scaleMatrix(glm::scale(Constants::kIdentity, scaleFactor));
    const Matrix4x4 &worldMatrix =
        boneTransform * (translateMatrix * rotateMatrix * scaleMatrix) * initialWorldTransform;
    return worldMatrix;
}

BoundingBox
Accessory::boundingBox() const
{
    return m_boundingBox;
}

const undo_stack_t *
Accessory::undoStack() const
{
    return m_undoStack;
}

undo_stack_t *
Accessory::undoStack()
{
    return m_undoStack;
}

nanodxm_document_t *
Accessory::data() const
{
    return m_opaque;
}

void
Accessory::trianguleNormal(const nanodxm_vector3_t *vertices, const glm::uvec3 &index, Vector3List &normalSum)
{
    nanoem_parameter_assert(vertices, "must not be nullptr");
    const nanodxm_vector3_t &v0 = vertices[index.x], &v1 = vertices[index.y], &v2 = vertices[index.z];
    const Vector3 p0(v0.x, v0.y, v0.z), p1(v1.x, v1.y, v1.z), p2(v2.x, v2.y, v2.z);
    if (!glm::isNull(p0, Constants::kEpsilon) && !glm::isNull(p1, Constants::kEpsilon) &&
        !glm::isNull(p2, Constants::kEpsilon)) {
        const Vector3 normal(glm::triangleNormal(p0, p1, p2));
        normalSum[index.x] += normal;
        normalSum[index.y] += normal;
        normalSum[index.z] += normal;
    }
}

const Image *
Accessory::createImage(const nanodxm_uint8_t *path)
{
    nanoem_parameter_assert(path, "must not be nullptr");
    const char *tempPath = reinterpret_cast<const char *>(path);
    String tempString, rawFilename, filename;
    FileUtils::canonicalizePathSeparator(tempPath, tempString);
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    const char *pathPtr = reinterpret_cast<const char *>(path);
    StringUtils::getUtf8String(pathPtr, StringUtils::length(pathPtr), NANOEM_CODEC_TYPE_SJIS, factory, rawFilename);
    FileUtils::canonicalizePathSeparator(rawFilename, filename);
    const Image *imagePtr = nullptr;
    ImageMap::const_iterator it = m_imageHandles.find(filename);
    if (it != m_imageHandles.end()) {
        imagePtr = it->second;
    }
    else {
        const URI &imageURI = Project::resolveArchiveURI(resolvedFileURI(), filename);
        Image *image = nanoem_new(Image);
        image->setFilename(filename);
        m_imageHandles.insert(tinystl::make_pair(filename, image));
        m_imageURIMap.insert(tinystl::make_pair(filename, imageURI));
        m_loadingImageItems.push_back(nanoem_new(LoadingImageItem(imageURI, filename)));
        imagePtr = image;
    }
    return imagePtr;
}

Image *
Accessory::internalUploadImage(const String &filename, const sg_image_desc &desc, bool fileExist)
{
    SG_PUSH_GROUPF("Accessory::internalUploadImage(filename=%s, width=%d, height=%d, fileExist=%d)", filename.c_str(),
        desc.width, desc.height, fileExist);
    Image *image = nullptr;
    ImageMap::iterator it = m_imageHandles.find(filename);
    if (it != m_imageHandles.end()) {
        image = it->second;
        ImageLoader::copyImageDescrption(desc, image);
        if (Inline::isDebugLabelEnabled()) {
            char label[Inline::kMarkerStringLength];
            StringUtils::format(
                label, sizeof(label), "Accessories/%s/%s", canonicalNameConstString(), filename.c_str());
            image->setLabel(label);
        }
        image->setFileExist(fileExist);
        image->create();
        EMLOG_DEBUG("The image is allocated: name={} ID={}", filename.c_str(), image->handle().id);
    }
    SG_POP_GROUP();
    return image;
}

void
Accessory::clearAllLoadingImageItems()
{
    for (LoadingImageItemList::iterator it = m_loadingImageItems.begin(), end = m_loadingImageItems.end(); it != end;
         ++it) {
        nanoem_delete(*it);
    }
    m_loadingImageItems.clear();
}

void
Accessory::createAllImages()
{
    nanodxm_rsize_t numMaterials;
    nanodxm_material_t *const *materials = nanodxmDocumentGetMaterials(m_opaque, &numMaterials);
    clearAllLoadingImageItems();
    for (nanodxm_rsize_t i = 0; i < numMaterials; i++) {
        const nanodxm_material_t *materialPtr = materials[i];
        if (const nanodxm_uint8_t *path = nanodxmMaterialGetTextureFilename(materialPtr)) {
            MaterialList::const_iterator it = m_materials.find(materialPtr);
            Material *material = 0;
            if (it != m_materials.end()) {
                material = it->second;
            }
            else {
                material = nanoem_new(Accessory::Material(m_project->sharedFallbackImage()));
                m_materials.insert(tinystl::make_pair(materialPtr, material));
            }
            if (ImageLoader::isScreenBMP(reinterpret_cast<const char *>(path))) {
                m_screenImage = nanoem_new(Image);
                m_screenImage->setFilename(Project::kViewportSecondaryName);
                m_screenImage->setHandle(m_project->viewportSecondaryImage());
                material->setDiffuseImage(m_screenImage);
            }
            else {
                material->setDiffuseImage(createImage(path));
            }
            if (const char *suffix = StringUtils::indexOf(reinterpret_cast<const char *>(path), '.')) {
                if (StringUtils::equalsIgnoreCase(suffix, ".sph")) {
                    material->setSphereTextureMapType(NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_MULTIPLY);
                }
                else if (StringUtils::equalsIgnoreCase(suffix, ".spa")) {
                    material->setSphereTextureMapType(NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_ADD);
                }
            }
        }
    }
}

void
Accessory::fillNormalBuffer(const Vector3List &normalSum, nanodxm_rsize_t numVertices, VertexUnit *ptr) const
{
    nanodxm_rsize_t numNormals, numVertexFaces, numNormalFaces;
    const nanodxm_vector3_t *normals = nanodxmDocumentGetNormals(m_opaque, &numNormals);
    const nanodxm_face_t *vertexFaces = nanodxmDocumentGetVertexFaces(m_opaque, &numVertexFaces);
    const nanodxm_face_t *normalFaces = nanodxmDocumentGetNormalFaces(m_opaque, &numNormalFaces);
    if (normals && vertexFaces && normalFaces) {
        for (nanodxm_rsize_t i = 0; i < numVertexFaces; i++) {
            const nanodxm_face_t &vertexFace = vertexFaces[i];
            const nanodxm_face_t &normalFace = normalFaces[i];
            if (vertexFace.num_indices == normalFace.num_indices) {
                for (nanodxm_rsize_t j = 0; j < normalFace.num_indices; j++) {
                    int vertexIndex = vertexFace.indices[j];
                    int normalIndex = normalFace.indices[j];
                    if (vertexIndex >= 0 && nanodxm_rsize_t(vertexIndex) < numVertices && normalIndex >= 0 &&
                        nanodxm_rsize_t(normalIndex) < numNormals) {
                        const nanodxm_vector3_t &normal = normals[normalIndex];
                        VertexUnit &p = ptr[vertexIndex];
                        memcpy(&p.m_normal, &normal, sizeof(normal));
                        p.m_normal *= Vector4(Constants::kTranslateDirection, 0);
                    }
                }
            }
        }
    }
    else if (normals && numVertices == numNormals) {
        for (nanodxm_rsize_t i = 0; i < numNormals; i++) {
            VertexUnit &p = ptr[i];
            const nanodxm_vector3_t &normal = normals[i];
            memcpy(&p.m_normal, &normal, sizeof(normal));
            p.m_normal *= Vector4(Constants::kTranslateDirection, 0);
        }
    }
    else {
        const size_t numNormalSum = normalSum.size();
        for (size_t i = 0; i < numNormalSum; i++) {
            const Vector3 ns(normalSum[i]);
            const Vector3 normal(!glm::isNull(ns, Constants::kEpsilon) ? glm::normalize(ns) : Constants::kZeroV3);
            VertexUnit &p = ptr[i];
            p.m_normal = Vector4(normal * Constants::kTranslateDirection, 0);
        }
    }
}

void
Accessory::createVertexBuffer(const Vector3List &normalSum)
{
    nanodxm_rsize_t numVertices, numColors, numTexCoords;
    const nanodxm_vector3_t *vertices = nanodxmDocumentGetVertices(m_opaque, &numVertices);
    if (numVertices > 0 && vertices) {
        ByteArray vertexBufferData(numVertices * sizeof(VertexUnit));
        VertexUnit *ptr = reinterpret_cast<VertexUnit *>(vertexBufferData.data());
        fillNormalBuffer(normalSum, numVertices, ptr);
        const nanodxm_color_t *colors = nanodxmDocumentGetColors(m_opaque, &numColors);
        const nanodxm_texcoord_t *texcoords = nanodxmDocumentGetTexCoords(m_opaque, &numTexCoords);
        for (nanodxm_rsize_t i = 0; i < numVertices; i++) {
            const nanodxm_vector3_t &vertex = vertices[i];
            VertexUnit &p = ptr[i];
            memcpy(&p.m_position, &vertex, sizeof(vertex));
            m_boundingBox.set(glm::make_vec3(&vertex.x), glm::make_vec3(&vertex.x));
            if (colors && i < numColors) {
                const nanodxm_color_t &color = colors[i];
                memcpy(&p.m_color, &color, sizeof(color));
            }
            else {
                p.m_color = Vector4(1.0f);
            }
            if (texcoords && i < numTexCoords) {
                const nanodxm_texcoord_t &texcoord = texcoords[i];
                p.m_texcoord.x = Inline::fract(texcoord.u);
                p.m_texcoord.y = Inline::fract(texcoord.v);
            }
            else {
                p.m_texcoord = Constants::kZeroV4;
            }
            p.m_position *= Vector4(Constants::kTranslateDirection, 1);
        }
        sg_buffer_desc desc;
        Inline::clearZeroMemory(desc);
        char label[Inline::kMarkerStringLength];
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(label, sizeof(label), "Accessories/%s/VertexBuffer", canonicalNameConstString());
            desc.label = label;
        }
        else {
            *label = 0;
        }
        desc.size = vertexBufferData.size();
        desc.data.ptr = vertexBufferData.data();
        desc.data.size = desc.size;
        m_vertexBuffer = sg::make_buffer(&desc);
        nanoem_assert(sg::query_buffer_state(m_vertexBuffer) == SG_RESOURCESTATE_VALID, "vertex buffer must be valid");
        SG_LABEL_BUFFER(m_vertexBuffer, label);
    }
}

template <typename TIndex>
void
Accessory::createIndexBuffer(tinystl::vector<TIndex, TinySTLAllocator> &allIndices,
    tinystl::vector<tinystl::vector<TIndex, TinySTLAllocator>, TinySTLAllocator> &indicesPerMaterial)
{
    nanodxm_rsize_t numVertexFaces, numMaterialIndices, numMaterials;
    const nanodxm_face_t *vertexFaces = nanodxmDocumentGetVertexFaces(m_opaque, &numVertexFaces);
    const int *materialIndices = nanodxmDocumentGetFaceMaterialIndices(m_opaque, &numMaterialIndices);
    int countIndices = 0;
    nanodxmDocumentGetMaterials(m_opaque, &numMaterials);
    indicesPerMaterial.resize(glm::max(numMaterials, numMaterialIndices));
    for (nanodxm_rsize_t i = 0; i < numVertexFaces; i++) {
        const nanodxm_face_t &vertexFace = vertexFaces[i];
        const int *vertexFaceIndices = vertexFace.indices;
        int materialIndex = materialIndices ? materialIndices[i] : 0;
        tinystl::vector<TIndex, TinySTLAllocator> *indices = &indicesPerMaterial[materialIndex];
        int size = 0;
        if (vertexFace.num_indices == 4) {
            int i0 = vertexFaceIndices[0], i1 = vertexFaceIndices[1], i2 = vertexFaceIndices[2],
                i3 = vertexFaceIndices[3];
            indices->push_back(i0);
            indices->push_back(i1);
            indices->push_back(i2);
            indices->push_back(i0);
            indices->push_back(i2);
            indices->push_back(i3);
            size = 6;
        }
        else if (vertexFace.num_indices == 3) {
            int i0 = vertexFaceIndices[0], i1 = vertexFaceIndices[1], i2 = vertexFaceIndices[2];
            indices->push_back(i0);
            indices->push_back(i1);
            indices->push_back(i2);
            size = 3;
        }
        countIndices += size;
    }
    allIndices.resize(countIndices);
    int offset = 0;
    for (nanodxm_rsize_t i = 0; i < numMaterials; i++) {
        const tinystl::vector<TIndex, TinySTLAllocator> *indices = &indicesPerMaterial[i];
        const TIndex *indicesData = indices->data();
        const int numActualMaterialIndices = Inline::saturateInt32(indices->size());
        for (int j = 0; j < numActualMaterialIndices; j++) {
            allIndices[offset++] = indicesData[j];
        }
    }
}

void
Accessory::createIndexBuffer(Vector3List &normalSum)
{
    VertexIndexList allIndices;
    tinystl::vector<VertexIndexList, TinySTLAllocator> indicesPerMaterial;
    createIndexBuffer(allIndices, indicesPerMaterial);
    size_t size = allIndices.size() * sizeof(allIndices[0]);
    if (size > 0) {
        sg_buffer_desc desc;
        Inline::clearZeroMemory(desc);
        char label[Inline::kMarkerStringLength];
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(label, sizeof(label), "Accessories/%s/IndexBuffer", canonicalNameConstString());
            desc.label = label;
        }
        else {
            *label = 0;
        }
        desc.size = size;
        desc.data.ptr = allIndices.data();
        desc.data.size = desc.size;
        desc.type = SG_BUFFERTYPE_INDEXBUFFER;
        m_indexBuffer = sg::make_buffer(&desc);
        nanoem_assert(sg::query_buffer_state(m_indexBuffer) == SG_RESOURCESTATE_VALID, "index buffer must be valid");
        SG_LABEL_BUFFER(m_indexBuffer, label);
    }
    m_numIndices.resize(indicesPerMaterial.size());
    for (size_t i = 0, size = indicesPerMaterial.size(); i < size; i++) {
        const size_t numIndices = indicesPerMaterial[i].size();
        m_numIndices[i] = numIndices;
    }
    calculateNormalSum(allIndices, normalSum);
}

template <typename TIndex>
void
Accessory::calculateNormalSum(const TIndex &indices, Vector3List &normalSum)
{
    nanodxm_rsize_t numVertices, numNormals, numNormalFaces, numMaterials;
    const nanodxm_vector3_t *vertices = nanodxmDocumentGetVertices(m_opaque, &numVertices);
    nanodxmDocumentGetNormals(m_opaque, &numNormals);
    const nanodxm_face_t *normalFaces = nanodxmDocumentGetNormalFaces(m_opaque, &numNormalFaces);
    nanodxmDocumentGetMaterials(m_opaque, &numMaterials);
    normalSum.resize(glm::max(numNormals, numVertices));
    if (normalFaces) {
        for (nanodxm_rsize_t i = 0; i < numNormalFaces; i++) {
            const nanodxm_face_t &normalFace = normalFaces[i];
            const int *normalFaceIndices = normalFace.indices;
            if (normalFace.num_indices == 4) {
                nanoem_rsize_t i0 = normalFaceIndices[0], i1 = normalFaceIndices[1], i2 = normalFaceIndices[2],
                               i3 = normalFaceIndices[3];
                if (i0 < numVertices && i1 < numVertices && i2 < numVertices && i3 < numVertices) {
                    trianguleNormal(vertices, Vector3UI32(i0, i1, i2), normalSum);
                    trianguleNormal(vertices, Vector3UI32(i2, i3, i0), normalSum);
                }
            }
            else if (normalFace.num_indices == 3) {
                nanoem_rsize_t i0 = normalFaceIndices[0], i1 = normalFaceIndices[1], i2 = normalFaceIndices[2];
                if (i0 < numVertices && i1 < numVertices && i2 < numVertices) {
                    trianguleNormal(vertices, Vector3UI32(i0, i1, i2), normalSum);
                }
            }
        }
    }
    else {
        const nanodxm_rsize_t numIndices = indices.size();
        for (nanodxm_rsize_t i = 0; i < numIndices; i += 3) {
            nanoem_rsize_t i0 = indices[i + 0], i1 = indices[i + 1], i2 = indices[i + 2];
            if (i0 < numVertices && i1 < numVertices && i2 < numVertices) {
                trianguleNormal(vertices, Vector3UI32(i0, i1, i2), normalSum);
            }
        }
    }
}

bool
Accessory::saveAllAttachments(
    const String &prefix, const FileEntityMap &allAttachments, Archiver &archiver, Error &error)
{
    String newFileName;
    const nanoem_u8_t *ptr;
    bool succeeded = true;
    if (Project::isArchiveURI(fileURI())) {
        const URI &baseURI = resolvedFileURI();
        FileReaderScope scope(m_project->translator());
        if (scope.open(baseURI, error)) {
            Archiver sourceArchive(scope.reader());
            if (sourceArchive.open(error)) {
                Archiver::Entry entry;
                ByteArray bytes;
                for (FileEntityMap::const_iterator it = allAttachments.begin(), end = allAttachments.end();
                     succeeded && it != end; ++it) {
                    ByteArray attachmentData;
                    const URI &fileURI = it->second;
                    if (!it->first.empty() && sourceArchive.findEntry(fileURI.fragment(), entry, error) &&
                        sourceArchive.extract(entry, attachmentData, error)) {
                        newFileName = prefix;
                        newFileName.append(it->first.c_str());
                        entry.m_path = newFileName;
                        ptr = attachmentData.data();
                        bytes.assign(ptr, ptr + attachmentData.size());
                        succeeded &= archiver.addEntry(entry, bytes, error);
                    }
                }
                sourceArchive.close(error);
            }
            else {
                succeeded = false;
            }
        }
        else {
            succeeded = false;
        }
    }
    else {
        Archiver::Entry entry;
        ByteArray bytes;
        for (FileEntityMap::const_iterator it = allAttachments.begin(), end = allAttachments.end();
             succeeded && it != end; ++it) {
            ByteArray attachmentData;
            const URI &fileURI = it->second;
            if (FileUtils::exists(fileURI)) {
                FileReaderScope scope(m_project->translator());
                if (scope.open(fileURI, error)) {
                    FileUtils::read(scope, attachmentData, error);
                    newFileName = prefix;
                    newFileName.append(it->first.c_str());
                    entry.m_path = newFileName;
                    ptr = attachmentData.data();
                    bytes.assign(ptr, ptr + attachmentData.size());
                    succeeded &= archiver.addEntry(entry, bytes, error);
                }
            }
        }
    }
    return succeeded;
}

bool
Accessory::getVertexIndexBufferAndTexture(const nanodxm_material_t *materialPtr, IPass::Buffer &buffer,
    sg_image &diffuseTexture, nanoem_model_material_sphere_map_texture_type_t &sphereTextureType) const
{
    nanoem_parameter_assert(materialPtr, "must not be nullptr");
    MaterialList::const_iterator it = m_materials.find(materialPtr);
    diffuseTexture.id = SG_INVALID_ID;
    if (it != m_materials.end()) {
        const Material *material = it->second;
        const IImageView *image = material->diffuseImage();
        diffuseTexture = image ? image->handle() : m_project->sharedFallbackImage();
        sphereTextureType = material->sphereTextureMapType();
    }
    buffer.m_vertexBuffer = m_vertexBuffer;
    buffer.m_indexBuffer = m_indexBuffer;
    return true;
}

IEffect *
Accessory::internalEffect()
{
    IEffect *passiveEffectPtr = passiveEffect();
    return passiveEffectPtr ? passiveEffectPtr : activeEffect();
}

void
Accessory::drawColor(bool scriptExternalColor)
{
    SG_PUSH_GROUPF("Accessory::drawColor(name=%s, scriptExternalColor=%s)", canonicalNameConstString(),
        scriptExternalColor ? "true" : "false");
    const ICamera *activeCamera = m_project->activeCamera();
    const ILight *globalLight = m_project->globalLight();
    const ShadowCamera *shadowCamera = m_project->shadowCamera();
    const String &passType = isGroundShadowEnabled() && m_project->isGroundShadowEnabled()
        ? Effect::kPassTypeObjectSelfShadow
        : Effect::kPassTypeObject;
    nanodxm_rsize_t numMaterials, indexOffset = 0;
    nanodxm_material_t *const *materials = nanodxmDocumentGetMaterials(m_opaque, &numMaterials);
    if (m_screenImage) {
        m_screenImage->setHandle(m_project->viewportSecondaryImage());
    }
    sg_image diffuseTexture = { SG_INVALID_ID };
    for (nanodxm_rsize_t i = 0; i < numMaterials; i++) {
        const nanodxm_material_t *material = materials[i];
        const size_t numIndices = m_numIndices[i];
        if (numIndices > 0) {
            nanoem_model_material_sphere_map_texture_type_t sphereTextureType =
                NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE;
            IPass::Buffer buffer(numIndices, indexOffset, true);
            if (getVertexIndexBufferAndTexture(material, buffer, diffuseTexture, sphereTextureType)) {
                IEffect *effect = internalEffect();
                if (ITechnique *technique = effect->findTechnique(passType, material, i, numMaterials, this)) {
                    SG_PUSH_GROUPF("Accessory::drawColor(offset=%d)", i);
                    while (IPass *pass = technique->execute(this, scriptExternalColor)) {
                        pass->setGlobalParameters(this, m_project);
                        pass->setCameraParameters(activeCamera, fullWorldTransform(kInitialWorldMatrix));
                        pass->setLightParameters(globalLight, true);
                        pass->setAllAccessoryParameters(this, m_project);
                        pass->setMaterialParameters(material);
                        pass->setShadowMapParameters(shadowCamera, kShadowWorldMatrix);
                        pass->execute(this, buffer);
                    }
                    if (!technique->hasNextScriptCommand() && !scriptExternalColor) {
                        technique->resetScriptCommandState();
                        technique->resetScriptExternalColor();
                    }
                    SG_POP_GROUP();
                }
            }
            indexOffset += numIndices;
        }
    }
    SG_POP_GROUP();
}

void
Accessory::drawGroundShadow()
{
    SG_PUSH_GROUPF("Accessory::drawGroundShadow(name=%s)", canonicalNameConstString());
    nanoem_rsize_t numMaterials, indexOffset = 0;
    const ICamera *activeCamera = m_project->activeCamera();
    const ILight *globalLight = m_project->globalLight();
    nanodxm_material_t *const *materials = nanodxmDocumentGetMaterials(m_opaque, &numMaterials);
    sg_image diffuseTexture = { SG_INVALID_ID };
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        const nanodxm_material_t *material = materials[i];
        const size_t numIndices = m_numIndices[i];
        if (numIndices > 0) {
            nanoem_model_material_sphere_map_texture_type_t sphereTextureType =
                NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE;
            IPass::Buffer buffer(numIndices, indexOffset, true);
            if (getVertexIndexBufferAndTexture(material, buffer, diffuseTexture, sphereTextureType)) {
                IEffect *effect = internalEffect();
                if (ITechnique *technique =
                        effect->findTechnique(Effect::kPassTypeShadow, material, i, numMaterials, this)) {
                    SG_PUSH_GROUPF("Accessory::drawGroundShadow(offset=%d)", i);
                    while (IPass *pass = technique->execute(this, false)) {
                        pass->setGlobalParameters(this, m_project);
                        pass->setCameraParameters(activeCamera, kInitialWorldMatrix);
                        pass->setLightParameters(globalLight, false);
                        pass->setAllAccessoryParameters(this, m_project);
                        pass->setMaterialParameters(material);
                        pass->setGroundShadowParameters(globalLight, activeCamera, kInitialWorldMatrix);
                        pass->execute(this, buffer);
                    }
                    if (!technique->hasNextScriptCommand()) {
                        technique->resetScriptCommandState();
                    }
                    SG_POP_GROUP();
                }
            }
            indexOffset += numIndices;
        }
    }
    SG_POP_GROUP();
}

void
Accessory::drawShadowMap()
{
    SG_PUSH_GROUPF("Accessory::drawShadowMap(name=%s)", canonicalNameConstString());
    const ICamera *activeCamera = m_project->activeCamera();
    const ILight *globalLight = m_project->globalLight();
    const ShadowCamera *shadowCamera = m_project->shadowCamera();
    nanodxm_rsize_t numMaterials, indexOffset = 0;
    nanodxm_material_t *const *materials = nanodxmDocumentGetMaterials(m_opaque, &numMaterials);
    sg_image diffuseTexture = { SG_INVALID_ID };
    for (nanodxm_rsize_t i = 0; i < numMaterials; i++) {
        const nanodxm_material_t *material = materials[i];
        const nanodxm_color_t &diffuse = nanodxmMaterialGetDiffuse(material);
        const bool enableCastingShadowMap = glm::abs(diffuse.a - 0.98f) > Constants::kEpsilon;
        const size_t numIndices = m_numIndices[i];
        if (numIndices > 0 && enableCastingShadowMap) {
            nanoem_model_material_sphere_map_texture_type_t sphereTextureType =
                NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE;
            IPass::Buffer buffer(numIndices, indexOffset, true);
            if (getVertexIndexBufferAndTexture(material, buffer, diffuseTexture, sphereTextureType)) {
                IEffect *effect = internalEffect();
                if (ITechnique *technique =
                        effect->findTechnique(Effect::kPassTypeZplot, material, i, numMaterials, this)) {
                    SG_PUSH_GROUPF("Accessory::drawShadowMap(offset=%d)", i);
                    while (IPass *pass = technique->execute(this, false)) {
                        pass->setGlobalParameters(this, m_project);
                        pass->setCameraParameters(activeCamera, kInitialWorldMatrix);
                        pass->setLightParameters(globalLight, true);
                        pass->setAllAccessoryParameters(this, m_project);
                        pass->setShadowMapParameters(shadowCamera, kShadowWorldMatrix);
                        pass->execute(this, buffer);
                    }
                    if (!technique->hasNextScriptCommand()) {
                        technique->resetScriptCommandState();
                    }
                    SG_POP_GROUP();
                }
            }
            indexOffset += numIndices;
        }
    }
    SG_POP_GROUP();
}

} /* namespace nanoem */
