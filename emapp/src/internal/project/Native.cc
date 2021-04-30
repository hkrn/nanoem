/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/internal/project/Native.h"

#include "emapp/Accessory.h"
#include "emapp/Effect.h"
#include "emapp/Grid.h"
#include "emapp/IAudioPlayer.h"
#include "emapp/IBackgroundVideoRenderer.h"
#include "emapp/ICamera.h"
#include "emapp/IFileManager.h"
#include "emapp/ILight.h"
#include "emapp/Model.h"
#include "emapp/PhysicsEngine.h"
#include "emapp/PluginFactory.h"
#include "emapp/Progress.h"
#include "emapp/Project.h"
#include "emapp/ShadowCamera.h"
#include "emapp/StringUtils.h"
#include "emapp/UUID.h"
#include "emapp/internal/project/Archive.h"
#include "emapp/private/CommonInclude.h"

#include "../../protoc/project.pb-c.h"
#include "bx/handlealloc.h"

namespace nanoem {

#include "sha256.h"

namespace internal {
namespace project {

struct Native::Context {
    typedef tinystl::unordered_map<nanoem_u32_t, IDrawable *, TinySTLAllocator> DrawableHandleMap;
    typedef tinystl::unordered_map<const IDrawable *, Project::IncludeEffectSourceMap, TinySTLAllocator>
        DrawableIncludeEffectSourceMap;
    typedef tinystl::unordered_map<const Motion *, const Accessory *, TinySTLAllocator> MotionAccessoryMap;
    typedef tinystl::unordered_map<const Motion *, const Model *, TinySTLAllocator> MotionModelMap;
    typedef tinystl::unordered_map<nanoem_u16_t, nanoem_u16_t, TinySTLAllocator> HandleMap;

    static inline void
    copyString(char *&ptr, const String &value)
    {
        ptr = new char[value.size() + 1];
        StringUtils::copyString(ptr, value.c_str(), value.size() + 1);
    }
    static inline Vector2
    toVector2(const Nanoem__Common__Point *value) NANOEM_DECL_NOEXCEPT
    {
        return Vector2(value->x, value->y);
    }
    static inline Vector2
    toVector2(const Nanoem__Common__Size *value) NANOEM_DECL_NOEXCEPT
    {
        return Vector2(value->width, value->height);
    }
    static inline Vector3
    toVector3(const Nanoem__Common__Vector3 *value) NANOEM_DECL_NOEXCEPT
    {
        return Vector3(value->x, value->y, value->z);
    }
    static inline Vector3
    toVector3(const Nanoem__Common__Color *value) NANOEM_DECL_NOEXCEPT
    {
        return glm::clamp(Vector3(value->red, value->green, value->blue), Vector3(0), Vector3(1));
    }
    static inline Vector4
    toVector4(const Nanoem__Common__Color *value) NANOEM_DECL_NOEXCEPT
    {
        return glm::clamp(Vector4(value->red, value->green, value->blue, value->alpha), Vector4(0), Vector4(1));
    }
    static inline URI
    toURI(const Nanoem__Project__URI *value, const URI &baseURI, bool &isAbsolutePath) NANOEM_DECL_NOEXCEPT
    {
        URI fileURI;
        if (value) {
            const char *fragment = value->fragment;
            if (const char *relativePath = value->relative_path) {
                fileURI = URI::createFromFilePath(
                    FileUtils::canonicalizePath(baseURI.absolutePath(), relativePath), fragment ? fragment : "");
                isAbsolutePath = false;
            }
            else if (const char *absolutePath = value->absolute_path) {
                fileURI = URI::createFromFilePath(absolutePath, fragment ? fragment : "");
            }
        }
        return fileURI;
    }
    static inline StringMap
    toAnnotations(Nanoem__Common__Annotation *const *values, nanoem_rsize_t numValues) NANOEM_DECL_NOEXCEPT
    {
        StringMap annotations;
        if (values && numValues > 0) {
            for (nanoem_rsize_t i = 0; i < numValues; i++) {
                const Nanoem__Common__Annotation *item = values[i];
                annotations.insert(tinystl::make_pair(String(item->name), String(item->value)));
            }
        }
        return annotations;
    }
    static inline Nanoem__Common__Point *
    newPoint(const Vector2 &value)
    {
        Nanoem__Common__Point *vector2 = nanoem_new(Nanoem__Common__Point);
        nanoem__common__point__init(vector2);
        vector2->x = value.x;
        vector2->y = value.y;
        return vector2;
    }
    static inline Nanoem__Common__Size *
    newSize(const Vector2 &value)
    {
        Nanoem__Common__Size *size = nanoem_new(Nanoem__Common__Size);
        nanoem__common__size__init(size);
        size->width = value.x;
        size->height = value.y;
        return size;
    }
    static inline Nanoem__Common__Vector3 *
    newVector3(const Vector3 &value)
    {
        Nanoem__Common__Vector3 *vector3 = nanoem_new(Nanoem__Common__Vector3);
        nanoem__common__vector3__init(vector3);
        vector3->x = value.x;
        vector3->y = value.y;
        vector3->z = value.z;
        return vector3;
    }
    static inline Nanoem__Common__Color *
    newColor(const Vector3 &value)
    {
        Nanoem__Common__Color *color = nanoem_new(Nanoem__Common__Color);
        nanoem__common__color__init(color);
        color->red = value.x;
        color->green = value.y;
        color->blue = value.z;
        color->alpha = 1;
        return color;
    }
    static inline Nanoem__Common__Color *
    newColor(const Vector4 &value)
    {
        Nanoem__Common__Color *color = nanoem_new(Nanoem__Common__Color);
        nanoem__common__color__init(color);
        color->red = value.x;
        color->green = value.y;
        color->blue = value.z;
        color->alpha = value.w;
        return color;
    }
    static inline Nanoem__Project__URI *
    newURI(const Project *project, const URI &fileURI, const String &fragment, FileType fileType)
    {
        return newURI(fileURI, project->fileURI(), fragment.c_str(), fileType, project->filePathMode());
    }

    static Nanoem__Project__URI *newURI(const URI &value, const URI &baseURI, const char *fragment, FileType fileType,
        Project::FilePathMode filePathMode);
    static Nanoem__Project__Project *allocate();
    static void releaseAnnotation(Nanoem__Common__Annotation *annotation) NANOEM_DECL_NOEXCEPT;
    static void releaseMaterialEffectAttachment(
        Nanoem__Project__MaterialEffectAttachment *attachment) NANOEM_DECL_NOEXCEPT;
    static void releaseAccessory(Nanoem__Project__Accessory *accessory) NANOEM_DECL_NOEXCEPT;
    static void releaseModel(Nanoem__Project__Model *model) NANOEM_DECL_NOEXCEPT;
    static void releaseMotion(Nanoem__Project__Motion *motion) NANOEM_DECL_NOEXCEPT;
    static void releaseOffscreenRenderTargetEffectAttachment(
        Nanoem__Project__OffscreenRenderTargetEffect__Attachment *attachment) NANOEM_DECL_NOEXCEPT;
    static void releaseOffscreenRenderTargetEffect(
        Nanoem__Project__OffscreenRenderTargetEffect *effect) NANOEM_DECL_NOEXCEPT;
    static void releaseURI(Nanoem__Project__URI *uri) NANOEM_DECL_NOEXCEPT;
    static void releaseAudio(Nanoem__Project__Audio *audio) NANOEM_DECL_NOEXCEPT;
    static void releaseVideo(Nanoem__Project__Video *video) NANOEM_DECL_NOEXCEPT;
    static void releaseCamera(Nanoem__Project__Camera *camera) NANOEM_DECL_NOEXCEPT;
    static void releaseConfirmation(Nanoem__Project__Confirmation *confirmation) NANOEM_DECL_NOEXCEPT;
    static void releaseGrid(Nanoem__Project__Grid *grid) NANOEM_DECL_NOEXCEPT;
    static void releaseLight(Nanoem__Project__Light *light) NANOEM_DECL_NOEXCEPT;
    static void releasePhysicsSimulation(Nanoem__Project__PhysicSimulation *physicsSimulation) NANOEM_DECL_NOEXCEPT;
    static void releaseScreen(Nanoem__Project__Screen *screen) NANOEM_DECL_NOEXCEPT;
    static void releaseTimeline(Nanoem__Project__Timeline *timeline) NANOEM_DECL_NOEXCEPT;
    static void release(Nanoem__Project__Project *p) NANOEM_DECL_NOEXCEPT;
    static nanoem_u16_t resolveHandle(const HandleMap &handles, nanoem_u16_t value);
    static void getAllAnnotations(
        Nanoem__Common__Annotation *const *annotations, nanoem_rsize_t numAnnotations, StringMap &values);
    static void saveAllAnnotations(
        const StringMap &values, Nanoem__Common__Annotation **&annotations, nanoem_rsize_t &numAnnotations);

    Context(Project *project);
    ~Context() NANOEM_DECL_NOEXCEPT;

    bool saveAllAnnotationsWithUUID(
        const void *ptr, StringMap &values, Nanoem__Common__Annotation **&annotations, nanoem_rsize_t &numAnnotations);
    void loadAudio(const Nanoem__Project__Audio *value, Error &error, Project::IDiagnostics *diagnostics);
    void loadVideo(const Nanoem__Project__Video *value, Error &error, Project::IDiagnostics *diagnostics);
    void loadCamera(const Nanoem__Project__Camera *value);
    void loadConfirmation(const Nanoem__Project__Confirmation *value);
    void loadGrid(const Nanoem__Project__Grid *value);
    void loadProjectiveShadow(const Nanoem__Project__ProjectiveShadow *value);
    void loadSelfShadow(const Nanoem__Project__SelfShadow *value);
    void loadLight(const Nanoem__Project__Light *value);
    void loadPhysicsSimulation(const Nanoem__Project__PhysicSimulation *value);
    void loadScreen(const Nanoem__Project__Screen *value);
    void loadTimeline(const Nanoem__Project__Timeline *value);
    void loadAccessory(const Nanoem__Project__Accessory *a, Accessory *accessory, int numDrawables,
        Project::DrawableList &drawableOrderList, Accessory *&activeAccessoryPtr);
    void loadAccessoryFromFile(const Nanoem__Project__Accessory *a, nanoem_rsize_t numDrawables,
        Project::DrawableList &drawableOrderList, HandleMap &handles, Accessory *&activeAccessoryPtr, Error &error,
        Project::IDiagnostics *diagnostics);
    void loadAllAccessories(const Nanoem__Project__Project *p, Project::DrawableList &drawableOrderList,
        HandleMap &handles, FileType fileType, Error &error, Project::IDiagnostics *diagnostics);
    bool attachModelMaterialEffect(Model *model, Effect *effect, nanoem_rsize_t offset);
    void loadAllModelMaterialEffects(
        const Nanoem__Project__Model *m, Model *model, Error &error, Project::IDiagnostics *diagnostics);
    void loadModel(const Nanoem__Project__Model *m, Model *model, int numDrawables,
        Project::DrawableList &drawableOrderList, Project::ModelList &transformOrderList, Model *&activeModelPtr,
        Error &error, Project::IDiagnostics *diagnostics);
    void loadModelFromFile(const Nanoem__Project__Model *m, nanoem_rsize_t numDrawables,
        Project::DrawableList &drawableOrderList, Project::ModelList &transformOrderList, HandleMap &handles,
        Model *&activeModelPtr, Error &error, Project::IDiagnostics *diagnostics);
    void loadAllModels(const Nanoem__Project__Project *p, Model *&activeModelPtr,
        Project::DrawableList &drawableOrderList, Project::ModelList &transformOrderList, HandleMap &handles,
        FileType fileType, Error &error, Project::IDiagnostics *diagnostics);
    bool loadMotionPayload(
        const Nanoem__Project__Motion *m, const ProtobufCBinaryData &payload, Motion *motion, Error &error);
    void loadMotion(const Nanoem__Project__Motion *m, const HandleMap &handles, bool &needsRestart, Error &error);
    void loadAllMotions(const Nanoem__Project__Project *p, const HandleMap &handles, bool &needsRestart, Error &error);
    bool loadOffscreenRenderTargetEffectAttachmentFromFile(
        const URI &fileURI, const char *ownerName, IDrawable *target, Error &error);
    void loadOffscreenRenderTargetEffectAttachment(
        const Nanoem__Project__OffscreenRenderTargetEffect__Attachment *attachment, const char *ownerName,
        Error &error);
    void loadAllOffscreenRenderTargetEffects(const Nanoem__Project__Project *p, Error &error);
    void load(const Nanoem__Project__Project *p, FileType fileType, Error &error, Project::IDiagnostics *diagnostics);

    String canonicalizeFilePath(const URI &fileURI);
    static void calculateFileContentDigest(IReader *reader, ProtobufCBinaryData &checksum);
    bool testFileContentDigest(const URI &fileURI, const ProtobufCBinaryData &checksum, Error &error) const;
    Nanoem__Project__Audio *saveAudio(FileType fileType, Error &error);
    Nanoem__Project__Camera *saveCamera();
    Nanoem__Project__Confirmation *saveConfirmation();
    Nanoem__Project__Grid *saveGrid();
    Nanoem__Project__ProjectiveShadow *saveProjectiveShadow();
    Nanoem__Project__SelfShadow *saveSelfShadow();
    Nanoem__Project__Light *saveLight();
    Nanoem__Project__PhysicSimulation *savePhysicsSimulation();
    Nanoem__Project__Screen *saveScreen();
    Nanoem__Project__Timeline *saveTimeline();
    Nanoem__Project__Video *saveVideo(FileType fileType, Error &error);
    void saveAccessoryPath(const Accessory *accessory, Nanoem__Project__Accessory *&ao);
    Nanoem__Project__Accessory *saveAccessory(Accessory *accessory, FileType fileType, Error &error);
    void saveAllAccessories(Nanoem__Project__Project *p, FileType fileType, MotionAccessoryMap &m2a, Error &error);
    void saveModelPath(const Model *model, Nanoem__Project__Model *&mo);
    void saveAllModelMaterialAttachments(Nanoem__Project__Model *mo, Model *model, FileType fileType, Error &error);
    void saveAllIncludeEffectSources(Nanoem__Project__Model *mo, Model *model);
    Nanoem__Project__Model *saveModel(Model *model, FileType fileType, Error &error);
    void saveAllModels(Nanoem__Project__Project *p, FileType fileType, MotionModelMap &m2m, Error &error);
    Nanoem__Project__Motion *saveMotion(Motion *motion);
    void saveAllMotions(Nanoem__Project__Project *p, FileType fileType, const MotionAccessoryMap &m2a,
        const MotionModelMap &m2m, Error &error);
    Nanoem__Project__OffscreenRenderTargetEffect *saveOffscreenRenderTargetEffect(
        const IDrawable *ownerEffect, const String &ownerName, FileType fileType, Error &error);
    void saveAllEffects(Nanoem__Project__Project *p, FileType fileType, Error &error);
    bool save(Nanoem__Project__Project *p, FileType fileType, Error &error);
    const Project::IncludeEffectSourceMap *findIncludeEffectSource(const IDrawable *drawable) const;
    Project::IncludeEffectSourceMap *findMutableIncludeEffectSource(const IDrawable *drawable);
    void getAllOffscreenRenderTargetEffectAttachments(OffscreenRenderTargetEffectAttachmentList &value) const;
    IDrawable *findDrawable(nanoem_u32_t value);

    Project *m_project;
    DrawableHandleMap m_drawables;
    DrawableIncludeEffectSourceMap m_includeEffectSources;
    OffscreenRenderTargetEffectAttachmentList m_offscreenRenderTargetEffectAttachments;
    StringMap m_annotations;
    URI m_audioURI;
    URI m_videoURI;
    nanoem_motion_format_type_t m_defaultSaveMotionFormat;
    bool m_includeAudioVideoFileContentDigest;
};

Nanoem__Project__URI *
Native::Context::newURI(
    const URI &value, const URI &baseURI, const char *fragment, FileType fileType, Project::FilePathMode filePathMode)
{
    Nanoem__Project__URI *uri = nullptr;
    if (!value.isEmpty()) {
        uri = nanoem_new(Nanoem__Project__URI);
        nanoem__project__uri__init(uri);
        if (fileType == kFileTypeData) {
            const String absolutePath(value.absolutePath());
            bool isRelativePath = false;
            if (filePathMode == Project::kFilePathModeRelative) {
                const String relativePath(FileUtils::relativePath(absolutePath, baseURI.absolutePath()));
                isRelativePath = !relativePath.empty();
                if (isRelativePath) {
                    copyString(uri->relative_path, relativePath);
                }
            }
            if (!isRelativePath) {
                copyString(uri->absolute_path, absolutePath);
            }
        }
        if (value.hasFragment()) {
            copyString(uri->fragment, value.fragment());
        }
        else if (fileType == kFileTypeArchive && fragment) {
            copyString(uri->fragment, fragment);
        }
    }
    return uri;
}

Nanoem__Project__Project *
Native::Context::allocate()
{
    Nanoem__Project__Project *p = nanoem_new(Nanoem__Project__Project);
    nanoem__project__project__init(p);
    return p;
}

void
Native::Context::releaseAnnotation(Nanoem__Common__Annotation *annotation) NANOEM_DECL_NOEXCEPT
{
    if (annotation) {
        delete[] annotation->name;
        delete[] annotation->value;
        nanoem_delete(annotation);
    }
}

void
Native::Context::releaseMaterialEffectAttachment(
    Nanoem__Project__MaterialEffectAttachment *attachment) NANOEM_DECL_NOEXCEPT
{
    if (attachment) {
        delete[] attachment->file_checksum.data;
        delete[] attachment->path_for_legacy_compatibility;
        releaseURI(attachment->file_uri);
        nanoem_delete(attachment);
    }
}

void
Native::Context::releaseAccessory(Nanoem__Project__Accessory *accessory) NANOEM_DECL_NOEXCEPT
{
    if (accessory) {
        for (nanoem_rsize_t i = 0, numAnnotations = accessory->n_annotations; i < numAnnotations; i++) {
            releaseAnnotation(accessory->annotations[i]);
        }
        delete[] accessory->annotations;
        for (nanoem_rsize_t i = 0, numIncludePaths = accessory->n_include_paths; i < numIncludePaths; i++) {
            delete[] accessory->include_paths[i];
        }
        delete[] accessory->include_paths;
        for (nanoem_rsize_t i = 0, numAttachments = accessory->n_material_effect_attachments; i < numAttachments; i++) {
            releaseMaterialEffectAttachment(accessory->material_effect_attachments[i]);
        }
        releaseURI(accessory->file_uri);
        delete[] accessory->file_checksum.data;
        delete[] accessory->material_effect_attachments;
        delete[] accessory->name;
        delete[] accessory->path_for_legacy_compatibility;
        nanoem_delete(accessory);
    }
}

void
Native::Context::releaseModel(Nanoem__Project__Model *model) NANOEM_DECL_NOEXCEPT
{
    if (model) {
        for (nanoem_rsize_t i = 0, numAnnotations = model->n_annotations; i < numAnnotations; i++) {
            releaseAnnotation(model->annotations[i]);
        }
        delete[] model->annotations;
        for (nanoem_rsize_t i = 0, numIncludePaths = model->n_include_paths; i < numIncludePaths; i++) {
            delete[] model->include_paths[i];
        }
        delete[] model->include_paths;
        for (nanoem_rsize_t i = 0, numAttachments = model->n_material_effect_attachments; i < numAttachments; i++) {
            releaseMaterialEffectAttachment(model->material_effect_attachments[i]);
        }
        releaseURI(model->file_uri);
        delete[] model->file_checksum.data;
        delete[] model->material_effect_attachments;
        delete[] model->name;
        delete[] model->path_for_legacy_compatibility;
        nanoem_delete(model);
    }
}

void
Native::Context::releaseMotion(Nanoem__Project__Motion *motion) NANOEM_DECL_NOEXCEPT
{
    if (motion) {
        for (nanoem_rsize_t i = 0, numAnnotations = motion->n_annotations; i < numAnnotations; i++) {
            releaseAnnotation(motion->annotations[i]);
        }
        releaseURI(motion->file_uri);
        delete[] motion->annotations;
        delete[] motion->path_for_legacy_compatibility;
        delete[] motion->target;
        switch (motion->type_case) {
        case NANOEM__PROJECT__MOTION__TYPE_LIGHT: {
            delete[] motion->light->payload.data;
            nanoem_delete(motion->light);
            break;
        }
        case NANOEM__PROJECT__MOTION__TYPE_MODEL: {
            delete[] motion->model->payload.data;
            nanoem_delete(motion->model);
            break;
        }
        case NANOEM__PROJECT__MOTION__TYPE_CAMERA: {
            delete[] motion->camera->payload.data;
            nanoem_delete(motion->camera);
            break;
        }
        case NANOEM__PROJECT__MOTION__TYPE_ACCESSORY: {
            delete[] motion->accessory->payload.data;
            nanoem_delete(motion->accessory);
            break;
        }
        case NANOEM__PROJECT__MOTION__TYPE_SELF_SHADOW: {
            delete[] motion->self_shadow->payload.data;
            nanoem_delete(motion->self_shadow);
            break;
        }
        default:
            break;
        }
        nanoem_delete(motion);
    }
}

void
Native::Context::releaseOffscreenRenderTargetEffectAttachment(
    Nanoem__Project__OffscreenRenderTargetEffect__Attachment *attachment) NANOEM_DECL_NOEXCEPT
{
    if (attachment) {
        for (nanoem_rsize_t i = 0, numIncludePaths = attachment->n_include_paths; i < numIncludePaths; i++) {
            delete[] attachment->include_paths[i];
        }
        releaseURI(attachment->file_uri);
        delete[] attachment->file_checksum.data;
        delete[] attachment->include_paths;
        delete[] attachment->path;
        nanoem_delete(attachment);
    }
}

void
Native::Context::releaseOffscreenRenderTargetEffect(
    Nanoem__Project__OffscreenRenderTargetEffect *effect) NANOEM_DECL_NOEXCEPT
{
    if (effect) {
        for (nanoem_rsize_t i = 0, numAttachments = effect->n_attachments; i < numAttachments; i++) {
            releaseOffscreenRenderTargetEffectAttachment(effect->attachments[i]);
        }
        delete[] effect->attachments;
        delete[] effect->name;
        nanoem_delete(effect);
    }
}

void
Native::Context::releaseURI(Nanoem__Project__URI *uri) NANOEM_DECL_NOEXCEPT
{
    if (uri) {
        delete[] uri->absolute_path;
        delete[] uri->fragment;
        delete[] uri->relative_path;
        nanoem_delete(uri);
    }
}

void
Native::Context::releaseAudio(Nanoem__Project__Audio *audio) NANOEM_DECL_NOEXCEPT
{
    if (audio) {
        releaseURI(audio->file_uri);
        nanoem_delete(audio);
    }
}

void
Native::Context::releaseVideo(Nanoem__Project__Video *video) NANOEM_DECL_NOEXCEPT
{
    if (video) {
        releaseURI(video->file_uri);
        nanoem_delete(video->offset);
        nanoem_delete(video->size);
        nanoem_delete(video);
    }
}

void
Native::Context::releaseCamera(Nanoem__Project__Camera *camera) NANOEM_DECL_NOEXCEPT
{
    if (camera) {
        nanoem_delete(camera->angle);
        nanoem_delete(camera->look_at);
        nanoem_delete(camera);
    }
}

void
Native::Context::releaseConfirmation(Nanoem__Project__Confirmation *confirmation) NANOEM_DECL_NOEXCEPT
{
    nanoem_delete(confirmation);
}

void
Native::Context::releaseGrid(Nanoem__Project__Grid *grid) NANOEM_DECL_NOEXCEPT
{
    if (grid) {
        nanoem_delete(grid->cell);
        nanoem_delete(grid);
    }
}

void
Native::Context::releaseLight(Nanoem__Project__Light *light) NANOEM_DECL_NOEXCEPT
{
    if (light) {
        delete[] light->motion_path;
        nanoem_delete(light->color);
        nanoem_delete(light->direction);
        nanoem_delete(light->projective_shadow);
        nanoem_delete(light->self_shadow->size);
        nanoem_delete(light->self_shadow);
        nanoem_delete(light);
    }
}

void
Native::Context::releasePhysicsSimulation(Nanoem__Project__PhysicSimulation *physicsSimulation) NANOEM_DECL_NOEXCEPT
{
    nanoem_delete(physicsSimulation->direction);
    nanoem_delete(physicsSimulation);
}

void
Native::Context::releaseScreen(Nanoem__Project__Screen *screen) NANOEM_DECL_NOEXCEPT
{
    if (screen) {
        nanoem_delete(screen->viewport->size);
        nanoem_delete(screen->viewport->point);
        nanoem_delete(screen->viewport);
        nanoem_delete(screen->color);
        nanoem_delete(screen);
    }
}

void
Native::Context::releaseTimeline(Nanoem__Project__Timeline *timeline) NANOEM_DECL_NOEXCEPT
{
    nanoem_delete(timeline);
}

void
Native::Context::release(Nanoem__Project__Project *p) NANOEM_DECL_NOEXCEPT
{
    for (nanoem_rsize_t i = 0, numAnnotations = p->n_annotations; i < numAnnotations; i++) {
        releaseAnnotation(p->annotations[i]);
    }
    delete[] p->annotations;
    for (nanoem_rsize_t i = 0, numAccessories = p->n_accessories; i < numAccessories; i++) {
        releaseAccessory(p->accessories[i]);
    }
    delete[] p->accessories;
    for (nanoem_rsize_t i = 0, numModels = p->n_models; i < numModels; i++) {
        releaseModel(p->models[i]);
    }
    delete[] p->models;
    for (nanoem_rsize_t i = 0, numMotions = p->n_motions; i < numMotions; i++) {
        releaseMotion(p->motions[i]);
    }
    delete[] p->motions;
    for (nanoem_rsize_t i = 0, numEffects = p->n_offscreen_render_target_effects; i < numEffects; i++) {
        releaseOffscreenRenderTargetEffect(p->offscreen_render_target_effects[i]);
    }
    delete[] p->offscreen_render_target_effects;
    releaseAudio(p->audio);
    releaseCamera(p->camera);
    releaseConfirmation(p->confirmation);
    releaseGrid(p->grid);
    releaseLight(p->light);
    releasePhysicsSimulation(p->physics_simulation);
    releaseScreen(p->screen);
    releaseTimeline(p->timeline);
    releaseVideo(p->video);
    nanoem_delete(p);
}

nanoem_u16_t
Native::Context::resolveHandle(const HandleMap &handles, nanoem_u16_t value)
{
    HandleMap::const_iterator it2 = handles.find(value);
    return it2 != handles.end() ? it2->second : bx::kInvalidHandle;
}

void
Native::Context::getAllAnnotations(
    Nanoem__Common__Annotation *const *annotations, nanoem_rsize_t numAnnotations, StringMap &values)
{
    for (nanoem_rsize_t i = 0; i < numAnnotations; i++) {
        const Nanoem__Common__Annotation *annotation = annotations[i];
        values.insert(tinystl::make_pair(String(annotation->name), String(annotation->value)));
    }
}

void
Native::Context::saveAllAnnotations(
    const StringMap &values, Nanoem__Common__Annotation **&annotations, nanoem_rsize_t &numAnnotations)
{
    if (!values.empty()) {
        numAnnotations = values.size();
        annotations = new Nanoem__Common__Annotation *[numAnnotations];
        nanoem_rsize_t i = 0;
        for (StringMap::const_iterator it = values.begin(), end = values.end(); it != end; ++it) {
            Nanoem__Common__Annotation *annotation = annotations[i++] = nanoem_new(Nanoem__Common__Annotation);
            nanoem__common__annotation__init(annotation);
            copyString(annotation->name, it->first);
            copyString(annotation->value, it->second);
        }
    }
}

Native::Context::Context(Project *project)
    : m_project(project)
    , m_defaultSaveMotionFormat(NANOEM_MOTION_FORMAT_TYPE_NMD)
    , m_includeAudioVideoFileContentDigest(false)
{
}

Native::Context::~Context() NANOEM_DECL_NOEXCEPT
{
}

bool
Native::Context::saveAllAnnotationsWithUUID(
    const void *ptr, StringMap &values, Nanoem__Common__Annotation **&annotations, nanoem_rsize_t &numAnnotations)
{
    StringMap::const_iterator it = values.find("uuid");
    bool changed = false;
    if (it == values.end()) {
        values.insert(tinystl::make_pair(String("uuid"), m_project->generateUUID(ptr).toString()));
        changed = true;
    }
    saveAllAnnotations(values, annotations, numAnnotations);
    return changed;
}

void
Native::Context::loadAudio(const Nanoem__Project__Audio *value, Error &error, Project::IDiagnostics *diagnostics)
{
    bool isAbsolutePath = true;
    m_project->audioPlayer()->setVolumeGain(value->volume);
    m_audioURI = value->file_uri ? toURI(value->file_uri, m_project->fileURI(), isAbsolutePath)
                                 : URI::createFromFilePath("", Archive::kBGMEntryPath);
    if (!m_audioURI.isEmpty() && !m_audioURI.hasFragment()) {
        if (FileUtils::exists(m_audioURI)) {
            IFileManager *fileManager = m_project->fileManager();
            if (fileManager->loadAudioFile(m_audioURI, m_project, error)) {
                if (isAbsolutePath) {
                    m_project->setFilePathMode(Project::kFilePathModeAbsolute);
                }
            }
        }
        else if (diagnostics) {
            diagnostics->addNotFoundFileURI(m_audioURI);
        }
    }
}

void
Native::Context::loadVideo(const Nanoem__Project__Video *value, Error &error, Project::IDiagnostics *diagnostics)
{
    if (value) {
        const Nanoem__Common__Point *point = value->offset;
        const Nanoem__Common__Size *size = value->size;
        const Vector4 rect(point->x, point->y, size->width, size->height);
        bool isAbsolutePath = true;
        m_project->setBackgroundVideoRect(rect);
        m_project->setBackgroundVideoScaleFactor(value->scale_factor);
        m_videoURI = value->file_uri ? toURI(value->file_uri, m_project->fileURI(), isAbsolutePath) : URI();
        if (!m_videoURI.isEmpty() && !m_videoURI.hasFragment()) {
            if (FileUtils::exists(m_videoURI)) {
                IFileManager *fileManager = m_project->fileManager();
                if (fileManager->loadVideoFile(m_videoURI, m_project, error)) {
                    if (isAbsolutePath) {
                        m_project->setFilePathMode(Project::kFilePathModeAbsolute);
                    }
                }
            }
            else if (diagnostics) {
                diagnostics->addNotFoundFileURI(m_videoURI);
            }
        }
    }
}

void
Native::Context::loadCamera(const Nanoem__Project__Camera *value)
{
    ICamera *camera = m_project->activeCamera();
    m_project->setCameraShared(value->is_shared != 0);
    camera->setAngle(toVector3(value->angle));
    camera->setLookAt(toVector3(value->look_at));
    camera->setDistance(value->distance);
    camera->setFovRadians(value->fov);
    camera->setPerspective(value->is_perspective != 0);
    camera->setDirty(false);
}

void
Native::Context::loadConfirmation(const Nanoem__Project__Confirmation *value)
{
    m_project->setConfirmSeekEnabled(
        NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE, value->enabled_if_dirty_bone_keyframes_found != 0);
    m_project->setConfirmSeekEnabled(
        NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA, value->enabled_if_dirty_camera_keyframes_found != 0);
    m_project->setConfirmSeekEnabled(
        NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT, value->enabled_if_dirty_light_keyframes_found != 0);
    m_project->setConfirmSeekEnabled(
        NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL, value->enabled_if_dirty_model_keyframes_found != 0);
    m_project->setConfirmSeekEnabled(
        NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH, value->enabled_if_dirty_morph_keyframes_found != 0);
}

void
Native::Context::loadGrid(const Nanoem__Project__Grid *value)
{
    Grid *grid = m_project->grid();
    grid->setCell(toVector2(value->cell));
    grid->setOpacity(value->opacity);
    grid->setVisible(value->visible != 0);
}

void
Native::Context::loadProjectiveShadow(const Nanoem__Project__ProjectiveShadow *value)
{
    m_project->setGroundShadowEnabled(value->enabled != 0);
}

void
Native::Context::loadSelfShadow(const Nanoem__Project__SelfShadow *value)
{
    ShadowCamera *shadowCamera = m_project->shadowCamera();
    m_project->setShadowMapEnabled(value->enabled != 0);
    m_project->setShadowMapSize(toVector2(value->size));
    shadowCamera->setDistance(value->distance);
    shadowCamera->setCoverageMode(static_cast<ShadowCamera::CoverageModeType>(value->mode));
    shadowCamera->setDirty(false);
}

void
Native::Context::loadLight(const Nanoem__Project__Light *value)
{
    ILight *light = m_project->activeLight();
    light->setColor(toVector3(value->color));
    light->setDirection(toVector3(value->direction));
    light->setDirty(false);
}

void
Native::Context::loadPhysicsSimulation(const Nanoem__Project__PhysicSimulation *value)
{
    m_project->setPhysicsSimulationEngineDebugFlags(value->debug);
    PhysicsEngine::SimulationModeType mode;
    if (value->has_mode) {
        mode = static_cast<PhysicsEngine::SimulationModeType>(value->mode);
        switch (mode) {
        case PhysicsEngine::kSimulationModeDisable:
        case PhysicsEngine::kSimulationModeEnableAnytime:
        case PhysicsEngine::kSimulationModeEnablePlaying:
        case PhysicsEngine::kSimulationModeEnableTracing:
            break;
        default:
            mode = value->enabled != 0 ? PhysicsEngine::kSimulationModeEnableTracing
                                       : PhysicsEngine::kSimulationModeDisable;
        }
    }
    else {
        mode =
            value->enabled != 0 ? PhysicsEngine::kSimulationModeEnableTracing : PhysicsEngine::kSimulationModeDisable;
    }
    m_project->setPhysicsSimulationMode(mode);
    if (value->has_time_step_factor != 0) {
        m_project->setTimeStepFactor(value->time_step_factor);
    }
    PhysicsEngine *engine = m_project->physicsEngine();
    if (value->has_acceleration) {
        engine->setAcceleration(value->acceleration);
    }
    if (const Nanoem__Common__Vector3 *direction = value->direction) {
        engine->setDirection(toVector3(direction));
    }
    if (value->has_is_noise_enabled) {
        engine->setNoiseEnabled(value->is_noise_enabled != 0);
    }
    if (value->has_noise) {
        engine->setNoise(value->noise);
    }
    if (value->has_is_ground_enabled) {
        engine->setGroundEnabled(value->is_ground_enabled != 0);
    }
}

void
Native::Context::loadScreen(const Nanoem__Project__Screen *value)
{
    m_project->setViewportBackgroundColor(toVector4(value->color));
    m_project->setViewportImageSize(toVector2(value->viewport->size));
    m_project->setSampleLevel(value->samples);
    m_project->setViewportWithTransparentEnabled(
        value->has_is_transparent_enabled ? value->is_transparent_enabled != 0 : value->color->alpha < 1);
}

void
Native::Context::loadTimeline(const Nanoem__Project__Timeline *value)
{
    m_project->setBaseDuration(nanoem_frame_index_t(value->duration));
    m_project->setPreferredMotionFPS(nanoem_u32_t(value->fps), false);
    m_project->setLoopEnabled(value->is_loop_enabled != 0);
    TimelineSegment segment;
    segment.m_enableFrom = value->has_frame_index_from != 0;
    segment.m_from = static_cast<nanoem_frame_index_t>(value->frame_index_from);
    segment.m_enableTo = value->has_frame_index_to != 0;
    segment.m_to = static_cast<nanoem_frame_index_t>(value->frame_index_to);
    m_project->setPlayingSegment(segment);
}

void
Native::Context::loadAccessory(const Nanoem__Project__Accessory *a, Accessory *accessory, int numDrawables,
    Project::DrawableList &drawableOrderList, Accessory *&activeAccessoryPtr)
{
    if (a->draw_order_index >= 0 && numDrawables > a->draw_order_index) {
        drawableOrderList[a->draw_order_index] = accessory;
    }
    if (a->is_active) {
        activeAccessoryPtr = accessory;
    }
    if (a->has_accessory_handle) {
        IDrawable *drawable = accessory;
        m_drawables.insert(tinystl::make_pair(a->accessory_handle, drawable));
    }
    for (nanoem_rsize_t i = 0, numIncludePaths = a->n_include_paths; i < numIncludePaths; i++) {
        const String includePath(a->include_paths[i]);
        m_includeEffectSources[accessory].insert(tinystl::make_pair(includePath, ByteArray()));
    }
    StringMap annotations;
    getAllAnnotations(a->annotations, a->n_annotations, annotations);
    accessory->setAnnotations(annotations);
}

void
Native::Context::loadAccessoryFromFile(const Nanoem__Project__Accessory *a, nanoem_rsize_t numDrawables,
    Project::DrawableList &drawableOrderList, HandleMap &handles, Accessory *&activeAccessoryPtr, Error &error,
    Project::IDiagnostics *diagnostics)
{
    bool isAbsolutePath = true;
    const URI fileURI(toURI(a->file_uri, m_project->fileURI(), isAbsolutePath));
    if (FileUtils::exists(fileURI)) {
        IFileManager *manager = m_project->fileManager();
        if (testFileContentDigest(fileURI, a->file_checksum, error)) {
            if (manager->loadFromFile(fileURI, IFileManager::kDialogTypeLoadModelFile, m_project, error)) {
                Accessory *accessory = m_project->allAccessories().back();
                loadAccessory(a, accessory, Inline::saturateInt32(numDrawables), drawableOrderList, activeAccessoryPtr);
                handles.insert(tinystl::make_pair(static_cast<nanoem_u16_t>(a->accessory_handle), accessory->handle()));
            }
            else if (diagnostics) {
                diagnostics->addNotFoundFileURI(fileURI);
            }
        }
        else if (diagnostics) {
            diagnostics->addDigestMismatchFileURI(fileURI);
        }
        if (isAbsolutePath) {
            m_project->setFilePathMode(Project::kFilePathModeAbsolute);
        }
    }
    else if (diagnostics) {
        diagnostics->addNotFoundFileURI(fileURI);
    }
}

void
Native::Context::loadAllAccessories(const Nanoem__Project__Project *p, Project::DrawableList &drawableOrderList,
    HandleMap &handles, FileType fileType, Error &error, Project::IDiagnostics *diagnostics)
{
    Accessory *activeAccessoryPtr = nullptr;
    int numDrawables = Inline::saturateInt32(drawableOrderList.size());
    switch (fileType) {
    case kFileTypeArchive: {
        for (nanoem_rsize_t i = 0, numAccessories = p->n_accessories; i < numAccessories; i++) {
            const Nanoem__Project__Accessory *a = p->accessories[i];
            if (const char *name = a->name) {
                if (Accessory *accessory = m_project->findAccessoryByName(name)) {
                    loadAccessory(a, accessory, numDrawables, drawableOrderList, activeAccessoryPtr);
                }
                else {
                    loadAccessoryFromFile(
                        a, numDrawables, drawableOrderList, handles, activeAccessoryPtr, error, diagnostics);
                }
            }
        }
        break;
    }
    case kFileTypeData: {
        for (nanoem_rsize_t i = 0, numAccessories = p->n_accessories; i < numAccessories; i++) {
            const Nanoem__Project__Accessory *a = p->accessories[i];
            if (a->has_accessory_handle) {
                if (Accessory *accessory = m_project->findAccessoryByHandle(a->accessory_handle)) {
                    loadAccessory(a, accessory, numDrawables, drawableOrderList, activeAccessoryPtr);
                }
                else {
                    loadAccessoryFromFile(
                        a, numDrawables, drawableOrderList, handles, activeAccessoryPtr, error, diagnostics);
                }
            }
        }
        break;
    }
    default:
        break;
    }
    if (activeAccessoryPtr) {
        m_project->setActiveAccessory(activeAccessoryPtr);
    }
}

bool
Native::Context::attachModelMaterialEffect(Model *model, Effect *effect, nanoem_rsize_t offset)
{
    nanoem_rsize_t numMaterials;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(model->data(), &numMaterials);
    bool attached = false;
    if (offset < numMaterials) {
        model::Material *material = model::Material::cast(materials[offset]);
        m_project->attachModelMaterialEffect(material, effect);
        attached = true;
    }
    return attached;
}

void
Native::Context::loadAllModelMaterialEffects(
    const Nanoem__Project__Model *m, Model *model, Error &error, Project::IDiagnostics *diagnostics)
{
    Progress progress(m_project, m->n_material_effect_attachments);
    size_t numMaterialEffectAttachmentsSucceeded = 0;
    bool isAbsolutePath = true;
    for (nanoem_rsize_t i = 0, numMaterialEffectAttachments = m->n_material_effect_attachments;
         i < numMaterialEffectAttachments; i++) {
        const Nanoem__Project__MaterialEffectAttachment *attachment = m->material_effect_attachments[i];
        const URI fileURI(toURI(attachment->file_uri, m_project->fileURI(), isAbsolutePath));
        if (testFileContentDigest(fileURI, attachment->file_checksum, error)) {
            Effect *effect = m_project->findEffect(fileURI);
            if (effect) {
                attachModelMaterialEffect(model, effect, attachment->offset);
                numMaterialEffectAttachmentsSucceeded++;
            }
            else {
                URI sourceURI;
                effect = m_project->createEffect();
                bool attached = false;
                if (m_project->loadEffectFromSource(fileURI, effect, sourceURI, progress, error)) {
                    effect->setFileURI(sourceURI);
                    attached = attachModelMaterialEffect(model, effect, attachment->offset);
                    numMaterialEffectAttachmentsSucceeded++;
                }
                if (!attached) {
                    m_project->destroyEffect(effect);
                }
            }
        }
        else if (diagnostics) {
            diagnostics->addDigestMismatchFileURI(fileURI);
        }
    }
    if (isAbsolutePath && numMaterialEffectAttachmentsSucceeded > 0) {
        m_project->setFilePathMode(Project::kFilePathModeAbsolute);
    }
}

void
Native::Context::loadModel(const Nanoem__Project__Model *m, Model *model, int numDrawables,
    Project::DrawableList &drawableOrderList, Project::ModelList &transformOrderList, Model *&activeModelPtr,
    Error &error, Project::IDiagnostics *diagnostics)
{
    if (m->draw_order_index >= 0 && numDrawables > m->draw_order_index) {
        drawableOrderList[m->draw_order_index] = model;
    }
    if (m->transform_order_index >= 0 && Inline::saturateInt32(transformOrderList.size()) > m->transform_order_index) {
        transformOrderList[m->transform_order_index] = model;
    }
    if (m->has_model_handle) {
        IDrawable *drawable = model;
        m_drawables.insert(tinystl::make_pair(m->model_handle, drawable));
    }
    if (m->is_active) {
        activeModelPtr = model;
    }
    for (nanoem_rsize_t i = 0, numIncludePaths = m->n_include_paths; i < numIncludePaths; i++) {
        const String includePath(m->include_paths[i]);
        m_includeEffectSources[model].insert(tinystl::make_pair(includePath, ByteArray()));
    }
    loadAllModelMaterialEffects(m, model, error, diagnostics);
    StringMap annotations;
    getAllAnnotations(m->annotations, m->n_annotations, annotations);
    model->setAnnotations(annotations);
}

void
Native::Context::loadModelFromFile(const Nanoem__Project__Model *m, nanoem_rsize_t numDrawables,
    Project::DrawableList &drawableOrderList, Project::ModelList &transformOrderList, HandleMap &handles,
    Model *&activeModelPtr, Error &error, Project::IDiagnostics *diagnostics)
{
    bool isAbsolutePath = true;
    const URI fileURI(toURI(m->file_uri, m_project->fileURI(), isAbsolutePath));
    if (FileUtils::exists(fileURI)) {
        IFileManager *manager = m_project->fileManager();
        if (testFileContentDigest(fileURI, m->file_checksum, error) &&
            manager->loadFromFile(fileURI, IFileManager::kDialogTypeLoadModelFile, m_project, error)) {
            Model *model = m_project->allModels().back();
            loadModel(m, model, Inline::saturateInt32(numDrawables), drawableOrderList, transformOrderList,
                activeModelPtr, error, diagnostics);
            handles.insert(tinystl::make_pair(static_cast<nanoem_u16_t>(m->model_handle), model->handle()));
        }
        if (isAbsolutePath) {
            m_project->setFilePathMode(Project::kFilePathModeAbsolute);
        }
    }
    else if (diagnostics) {
        diagnostics->addNotFoundFileURI(fileURI);
    }
}

void
Native::Context::loadAllModels(const Nanoem__Project__Project *p, Model *&activeModelPtr,
    Project::DrawableList &drawableOrderList, Project::ModelList &transformOrderList, HandleMap &handles,
    FileType fileType, Error &error, Project::IDiagnostics *diagnostics)
{
    int numDrawables = Inline::saturateInt32(drawableOrderList.size());
    switch (fileType) {
    case kFileTypeArchive: {
        for (nanoem_rsize_t i = 0, numModels = p->n_models; i < numModels; i++) {
            const Nanoem__Project__Model *m = p->models[i];
            if (const char *name = m->name) {
                if (Model *model = m_project->findModelByName(name)) {
                    loadModel(m, model, numDrawables, drawableOrderList, transformOrderList, activeModelPtr, error,
                        diagnostics);
                }
                else {
                    loadModelFromFile(m, numDrawables, drawableOrderList, transformOrderList, handles, activeModelPtr,
                        error, diagnostics);
                }
            }
        }
        break;
    }
    case kFileTypeData: {
        for (nanoem_rsize_t i = 0, numModels = p->n_models; i < numModels; i++) {
            const Nanoem__Project__Model *m = p->models[i];
            if (m->has_model_handle) {
                if (Model *model = m_project->findModelByHandle(m->model_handle)) {
                    loadModel(m, model, numDrawables, drawableOrderList, transformOrderList, activeModelPtr, error,
                        diagnostics);
                }
                else {
                    loadModelFromFile(m, numDrawables, drawableOrderList, transformOrderList, handles, activeModelPtr,
                        error, diagnostics);
                }
            }
        }
        break;
    }
    default:
        break;
    }
}

bool
Native::Context::loadMotionPayload(
    const Nanoem__Project__Motion *m, const ProtobufCBinaryData &payload, Motion *motion, Error &error)
{
    bool loaded = false, isAbsolutePath = true;
    if (motion) {
        Error localError;
        motion->clearAllKeyframes();
        bool succeeded = motion->load(payload.data, payload.len, 0, localError);
        if (!succeeded) {
            motion->clearAllKeyframes();
            motion->setFormat(NANOEM_MOTION_FORMAT_TYPE_VMD);
            succeeded = motion->load(payload.data, payload.len, 0, localError);
            motion->setFormat(NANOEM_MOTION_FORMAT_TYPE_NMD);
        }
        if (succeeded) {
            const URI fileURI(toURI(m->file_uri, m_project->fileURI(), isAbsolutePath));
            motion->setAnnotations(toAnnotations(m->annotations, m->n_annotations));
            motion->setFileURI(fileURI);
            if (!fileURI.isEmpty() && isAbsolutePath) {
                m_project->setFilePathMode(Project::kFilePathModeAbsolute);
            }
            loaded = true;
        }
        else {
            error = localError;
        }
    }
    return loaded;
}

void
Native::Context::loadMotion(
    const Nanoem__Project__Motion *m, const HandleMap &handles, bool &needsRestart, Error &error)
{
    switch (m->type_case) {
    case NANOEM__PROJECT__MOTION__TYPE_LIGHT: {
        const Nanoem__Project__Motion__Light *item = m->light;
        if (item->has_payload) {
            Motion *motion = m_project->lightMotion();
            loadMotionPayload(m, item->payload, motion, error);
        }
        break;
    }
    case NANOEM__PROJECT__MOTION__TYPE_MODEL: {
        const Nanoem__Project__Motion__Model *item = m->model;
        if (item->has_payload) {
            const nanoem_u16_t handle = resolveHandle(handles, item->model_handle);
            if (Model *model = m_project->findModelByHandle(handle)) {
                Motion *motion = m_project->resolveMotion(model);
                needsRestart |= loadMotionPayload(m, item->payload, motion, error);
            }
        }
        break;
    }
    case NANOEM__PROJECT__MOTION__TYPE_CAMERA: {
        const Nanoem__Project__Motion__Camera *item = m->camera;
        if (item->has_payload) {
            Motion *motion = m_project->cameraMotion();
            loadMotionPayload(m, item->payload, motion, error);
        }
        break;
    }
    case NANOEM__PROJECT__MOTION__TYPE_ACCESSORY: {
        const Nanoem__Project__Motion__Accessory *item = m->accessory;
        if (item->has_payload) {
            const nanoem_u16_t handle = resolveHandle(handles, item->accessory_handle);
            if (Accessory *accessory = m_project->findAccessoryByHandle(handle)) {
                Motion *motion = m_project->resolveMotion(accessory);
                loadMotionPayload(m, item->payload, motion, error);
            }
        }
        break;
    }
    case NANOEM__PROJECT__MOTION__TYPE_SELF_SHADOW: {
        const Nanoem__Project__Motion__SelfShadow *item = m->self_shadow;
        if (item->has_payload) {
            Motion *motion = m_project->selfShadowMotion();
            loadMotionPayload(m, item->payload, motion, error);
        }
        break;
    }
    default:
        break;
    }
}

void
Native::Context::loadAllMotions(
    const Nanoem__Project__Project *p, const HandleMap &handles, bool &needsRestart, Error &error)
{
    for (nanoem_rsize_t i = 0, numMotions = p->n_motions; i < numMotions; i++) {
        const Nanoem__Project__Motion *m = p->motions[i];
        loadMotion(m, handles, needsRestart, error);
    }
}

bool
Native::Context::loadOffscreenRenderTargetEffectAttachmentFromFile(
    const URI &fileURI, const char *ownerName, IDrawable *target, Error &error)
{
    PluginFactory::EffectPluginProxy proxy(m_project->fileManager()->sharedEffectPlugin());
    Progress progress(m_project, 0);
    ByteArray outputBinary;
    bool succeeded = false, created = false;
    Effect *effect = m_project->findEffect(fileURI);
    if (effect) {
        m_project->setOffscreenPassiveRenderTargetEffect(ownerName, target, effect);
        succeeded = !error.isCancelled() && !error.hasReason();
    }
    else if (proxy.compile(fileURI, outputBinary)) {
        effect = m_project->createEffect();
        effect->setName(fileURI.lastPathComponent());
        created = true;
        if (effect->load(outputBinary, progress, error)) {
            effect->setFileURI(fileURI);
            if (effect->upload(effect::kAttachmentTypeOffscreenPassive, progress, error)) {
                m_project->setOffscreenPassiveRenderTargetEffect(ownerName, target, effect);
                succeeded = !error.isCancelled() && !error.hasReason();
            }
        }
    }
    else {
        error = proxy.error();
    }
    if (!succeeded && created) {
        m_project->destroyEffect(effect);
    }
    return succeeded;
}

void
Native::Context::loadOffscreenRenderTargetEffectAttachment(
    const Nanoem__Project__OffscreenRenderTargetEffect__Attachment *attachment, const char *ownerName, Error &error)
{
    bool isAbsolutePath = true;
    if (IDrawable *target = findDrawable(attachment->handle)) {
        OffscreenRenderTargetEffectAttachment effect;
        effect.m_name = ownerName;
        effect.m_target = target;
        effect.m_filePath = attachment->path;
        for (nanoem_rsize_t i = 0, numIncludePaths = attachment->n_include_paths; i < numIncludePaths; i++) {
            effect.m_includePaths.push_back(attachment->include_paths[i]);
        }
        const Nanoem__Project__URI *uri = attachment->file_uri;
        if (uri && (uri->absolute_path || uri->relative_path)) {
            const URI fileURI(toURI(uri, m_project->fileURI(), isAbsolutePath));
            if (loadOffscreenRenderTargetEffectAttachmentFromFile(fileURI, ownerName, target, error)) {
                target->setOffscreenPassiveRenderTargetEffectEnabled(
                    ownerName, attachment->has_enabled ? attachment->enabled : true);
            }
            if (!fileURI.isEmpty() && isAbsolutePath) {
                m_project->setFilePathMode(Project::kFilePathModeAbsolute);
            }
        }
        else {
            m_offscreenRenderTargetEffectAttachments.push_back(effect);
        }
    }
}

void
Native::Context::loadAllOffscreenRenderTargetEffects(const Nanoem__Project__Project *p, Error &error)
{
    for (nanoem_rsize_t i = 0, numEffects = p->n_offscreen_render_target_effects; i < numEffects; i++) {
        const Nanoem__Project__OffscreenRenderTargetEffect *effect = p->offscreen_render_target_effects[i];
        if (const char *ownerName = effect->name) {
            for (nanoem_rsize_t j = 0, numAttachments = effect->n_attachments; j < numAttachments; j++) {
                const Nanoem__Project__OffscreenRenderTargetEffect__Attachment *attachment = effect->attachments[j];
                loadOffscreenRenderTargetEffectAttachment(attachment, ownerName, error);
            }
        }
    }
}

void
Native::Context::load(
    const Nanoem__Project__Project *p, FileType fileType, Error &error, Project::IDiagnostics *diagnostics)
{
    switch (p->language) {
    case NANOEM__COMMON__LANGUAGE__LC_ENGLISH:
    default:
        m_project->setLanguage(ITranslator::kLanguageTypeEnglish);
        break;
    case NANOEM__COMMON__LANGUAGE__LC_JAPANESE:
        m_project->setLanguage(ITranslator::kLanguageTypeJapanese);
        break;
    case NANOEM__COMMON__LANGUAGE__LC_KOREAN:
        m_project->setLanguage(ITranslator::kLanguageTypeKorean);
        break;
    case NANOEM__COMMON__LANGUAGE__LC_SIMPLIFIED_CHINESE:
        m_project->setLanguage(ITranslator::kLanguageTypeChineseSimplified);
        break;
    case NANOEM__COMMON__LANGUAGE__LC_TRADITIONAL_CHINESE:
        m_project->setLanguage(ITranslator::kLanguageTypeChineseTraditional);
        break;
    }
    getAllAnnotations(p->annotations, p->n_annotations, m_annotations);
    m_project->setDrawType(static_cast<IDrawable::DrawType>(p->draw_type));
    m_project->setEditingMode(static_cast<Project::EditingMode>(p->editing_mode));
    m_project->setEffectPluginEnabled(p->is_effect_plugin_enabled != 0);
    m_project->setMotionMergeEnabled(p->is_motion_merge_enabled != 0);
    m_project->setMultipleBoneSelectionEnabled(p->is_multiple_bone_selection_enabled != 0);
    Project::DrawableList drawableOrderList(p->n_accessories + p->n_models);
    Project::ModelList transformOrderList(p->n_models);
    HandleMap handles;
    Model *activeModelPtr = nullptr;
    loadAllAccessories(p, drawableOrderList, handles, fileType, error, diagnostics);
    loadAllModels(p, activeModelPtr, drawableOrderList, transformOrderList, handles, fileType, error, diagnostics);
    bool needsRestart = false;
    loadAllMotions(p, handles, needsRestart, error);
    if (needsRestart) {
        m_project->restart();
    }
    loadAllOffscreenRenderTargetEffects(p, error);
    m_project->setActiveModel(activeModelPtr);
    m_project->setDrawableOrderList(drawableOrderList);
    m_project->setTransformOrderList(transformOrderList);
    const nanoem_frame_index_t currentLocalFrameIndex(p->timeline->current_frame_index);
    m_project->seek(currentLocalFrameIndex, true);
    m_project->restart(currentLocalFrameIndex);
    m_project->update();
    if (activeModelPtr) {
        activeModelPtr->setTransformAxisType(static_cast<Model::AxisType>(p->axis_type));
        activeModelPtr->setTransformCoordinateType(static_cast<Model::TransformCoordinateType>(p->transform_type));
    }
    loadAudio(p->audio, error, diagnostics);
    loadVideo(p->video, error, diagnostics);
    loadCamera(p->camera);
    loadConfirmation(p->confirmation);
    loadGrid(p->grid);
    loadLight(p->light);
    loadProjectiveShadow(p->light->projective_shadow);
    loadPhysicsSimulation(p->physics_simulation);
    loadScreen(p->screen);
    loadSelfShadow(p->light->self_shadow);
    loadTimeline(p->timeline);
}

String
Native::Context::canonicalizeFilePath(const URI &fileURI)
{
    String path;
    if (Project::isArchiveURI(fileURI)) {
        path = fileURI.fragment();
    }
    else {
        path = fileURI.absolutePath();
        const String directory(m_project->fileURI().absolutePathByDeletingLastPathComponent());
        if (!directory.empty() && StringUtils::equals(path.c_str(), directory.c_str(), directory.size())) {
            path = path.c_str() + directory.size() + 1;
        }
    }
    return path;
}

void
Native::Context::calculateFileContentDigest(IReader *reader, ProtobufCBinaryData &checksum)
{
    SHA256_CTX ctx;
    Error error;
    nanoem_u8_t buffer[Inline::kReadingFileContentsBufferSize];
    nanoem_i32_t actualReadSize;
    sha256_init(&ctx);
    while ((actualReadSize = FileUtils::read(reader, buffer, sizeof(buffer), error)) > 0) {
        sha256_update(&ctx, buffer, actualReadSize);
    }
    checksum.len = SHA256_BLOCK_SIZE;
    checksum.data = new nanoem_u8_t[checksum.len];
    sha256_final(&ctx, checksum.data);
}

bool
Native::Context::testFileContentDigest(const URI &fileURI, const ProtobufCBinaryData &checksum, Error &error) const
{
    FileReaderScope scope(nullptr);
    bool fileChecksumPassed = false;
    if (scope.open(fileURI, error)) {
        ProtobufCBinaryData binary;
        calculateFileContentDigest(scope.reader(), binary);
        fileChecksumPassed = checksum.len == binary.len && memcmp(checksum.data, binary.data, binary.len) == 0;
        if (!fileChecksumPassed) {
            char reason[Error::kMaxRecoverySuggestionLength];
            StringUtils::format(reason, sizeof(reason),
                m_project->translator()->translate("nanoem.window.dialog.error.file-content-digest.reason"),
                fileURI.absolutePathConstString());
            error = Error(reason, "", Error::kDomainTypeApplication);
        }
        delete[] binary.data;
    }
    return fileChecksumPassed;
}

Nanoem__Project__Audio *
Native::Context::saveAudio(FileType fileType, Error &error)
{
    const IAudioPlayer *audioPtr = m_project->audioPlayer();
    Nanoem__Project__Audio *audio = nanoem_new(Nanoem__Project__Audio);
    nanoem__project__audio__init(audio);
    const URI fileURI(audioPtr->fileURI());
    audio->file_uri = newURI(m_project, fileURI, Archive::kBGMEntryPath, fileType);
    audio->volume = audioPtr->volumeGain();
    FileReaderScope scope(m_project->translator());
    if (fileType == kFileTypeData && m_includeAudioVideoFileContentDigest && !fileURI.isEmpty() &&
        scope.open(fileURI, error)) {
        audio->has_file_checksum = 1;
        calculateFileContentDigest(scope.reader(), audio->file_checksum);
    }
    return audio;
}

Nanoem__Project__Camera *
Native::Context::saveCamera()
{
    const ICamera *camera = m_project->activeCamera();
    Nanoem__Project__Camera *c = nanoem_new(Nanoem__Project__Camera);
    nanoem__project__camera__init(c);
    c->angle = newVector3(camera->angle());
    c->look_at = newVector3(camera->lookAt());
    c->distance = camera->distance();
    c->is_shared = m_project->isCameraShared();
    c->fov = camera->fovRadians();
    c->is_perspective = camera->isPerspective();
    return c;
}

Nanoem__Project__Confirmation *
Native::Context::saveConfirmation()
{
    Nanoem__Project__Confirmation *confirmation = nanoem_new(Nanoem__Project__Confirmation);
    nanoem__project__confirmation__init(confirmation);
    confirmation->enabled_if_dirty_bone_keyframes_found =
        m_project->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE);
    confirmation->enabled_if_dirty_camera_keyframes_found =
        m_project->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA);
    confirmation->enabled_if_dirty_light_keyframes_found =
        m_project->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT);
    confirmation->enabled_if_dirty_model_keyframes_found =
        m_project->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL);
    confirmation->enabled_if_dirty_morph_keyframes_found =
        m_project->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH);
    return confirmation;
}

Nanoem__Project__Grid *
Native::Context::saveGrid()
{
    const Grid *g = m_project->grid();
    Nanoem__Project__Grid *grid = nanoem_new(Nanoem__Project__Grid);
    nanoem__project__grid__init(grid);
    grid->cell = newSize(g->cell());
    grid->opacity = g->opacity();
    grid->visible = g->isVisible();
    return grid;
}

Nanoem__Project__ProjectiveShadow *
Native::Context::saveProjectiveShadow()
{
    Nanoem__Project__ProjectiveShadow *projectiveShadow = nanoem_new(Nanoem__Project__ProjectiveShadow);
    nanoem__project__projective_shadow__init(projectiveShadow);
    projectiveShadow->enabled = m_project->isGroundShadowEnabled();
    return projectiveShadow;
}

Nanoem__Project__SelfShadow *
Native::Context::saveSelfShadow()
{
    const ShadowCamera *shadowCameraPtr = m_project->shadowCamera();
    Nanoem__Project__SelfShadow *selfShadow = nanoem_new(Nanoem__Project__SelfShadow);
    nanoem__project__self_shadow__init(selfShadow);
    selfShadow->enabled = m_project->isShadowMapEnabled();
    selfShadow->size = newSize(shadowCameraPtr->imageSize());
    selfShadow->distance = shadowCameraPtr->distance();
    selfShadow->mode = shadowCameraPtr->coverageMode();
    return selfShadow;
}

Nanoem__Project__Light *
Native::Context::saveLight()
{
    const ILight *light = m_project->activeLight();
    Nanoem__Project__Light *l = nanoem_new(Nanoem__Project__Light);
    nanoem__project__light__init(l);
    l->color = newColor(light->color());
    l->direction = newVector3(light->direction());
    l->projective_shadow = saveProjectiveShadow();
    l->self_shadow = saveSelfShadow();
    copyString(l->motion_path, "");
    return l;
}

Nanoem__Project__PhysicSimulation *
Native::Context::savePhysicsSimulation()
{
    const PhysicsEngine *engine = m_project->physicsEngine();
    Nanoem__Project__PhysicSimulation *ps = nanoem_new(Nanoem__Project__PhysicSimulation);
    nanoem__project__physic_simulation__init(ps);
    ps->debug = engine->debugGeometryFlags();
    ps->enabled = engine->isActive();
    ps->has_time_step_factor = m_project->timeStepFactor() > 1;
    ps->time_step_factor = m_project->timeStepFactor();
    ps->has_acceleration = 1;
    ps->acceleration = engine->acceleration();
    ps->direction = newVector3(engine->direction());
    ps->has_is_noise_enabled = 1;
    ps->is_noise_enabled = engine->isNoiseEnabled();
    ps->has_noise = 1;
    ps->noise = engine->noise();
    ps->has_is_ground_enabled = 1;
    ps->is_ground_enabled = engine->isGroundEnabled();
    ps->mode = engine->mode();
    ps->has_mode = 1;
    return ps;
}

Nanoem__Project__Screen *
Native::Context::saveScreen()
{
    Nanoem__Project__Screen *screen = nanoem_new(Nanoem__Project__Screen);
    nanoem__project__screen__init(screen);
    screen->viewport = nanoem_new(Nanoem__Common__Rect);
    nanoem__common__rect__init(screen->viewport);
    screen->viewport->size = nanoem_new(Nanoem__Common__Size);
    nanoem__common__size__init(screen->viewport->size);
    const Vector2UI16 viewportImageSize(m_project->viewportImageSize());
    screen->viewport->size->width = viewportImageSize.x;
    screen->viewport->size->height = viewportImageSize.y;
    screen->viewport->point = newPoint(Vector2(0));
    screen->color = newColor(m_project->viewportBackgroundColor());
    screen->samples = m_project->sampleLevel();
    screen->has_is_transparent_enabled = 1;
    screen->is_transparent_enabled = m_project->isViewportWithTransparentEnabled();
    return screen;
}

Nanoem__Project__Timeline *
Native::Context::saveTimeline()
{
    Nanoem__Project__Timeline *timeline = nanoem_new(Nanoem__Project__Timeline);
    nanoem__project__timeline__init(timeline);
    const nanoem_frame_index_t duration = m_project->duration();
    timeline->current_frame_index = m_project->currentLocalFrameIndex();
    timeline->duration = duration;
    timeline->fps = nanoem_f32_t(m_project->preferredMotionFPS());
    timeline->is_loop_enabled = m_project->isLoopEnabled();
    const TimelineSegment segment(m_project->playingSegment());
    timeline->has_frame_index_from = segment.m_enableFrom ? 1 : 0;
    timeline->frame_index_from = segment.frameIndexFrom();
    timeline->has_frame_index_to = segment.m_enableFrom ? 1 : 0;
    timeline->frame_index_to = segment.frameIndexTo(duration);
    return timeline;
}

Nanoem__Project__Video *
Native::Context::saveVideo(FileType fileType, Error &error)
{
    Nanoem__Project__Video *video = nanoem_new(Nanoem__Project__Video);
    nanoem__project__video__init(video);
    Nanoem__Common__Point *point = nanoem_new(Nanoem__Common__Point);
    nanoem__common__point__init(point);
    const Vector4 rect(m_project->backgroundVideoRect());
    point->x = rect.x;
    point->y = rect.y;
    video->offset = point;
    Nanoem__Common__Size *size = nanoem_new(Nanoem__Common__Size);
    nanoem__common__size__init(size);
    size->width = rect.z;
    size->height = rect.w;
    video->size = size;
    const URI fileURI(m_project->backgroundVideoRenderer()->fileURI());
    String fragmentPath("BackGround/");
    fragmentPath.append(fileURI.lastPathComponentConstString());
    video->file_uri = newURI(m_project, fileURI, fragmentPath, fileType);
    if (!video->file_uri) {
        video->file_uri = nanoem_new(Nanoem__Project__URI);
        nanoem__project__uri__init(video->file_uri);
    }
    FileReaderScope scope(m_project->translator());
    if (fileType == kFileTypeData && m_includeAudioVideoFileContentDigest && !fileURI.isEmpty() &&
        scope.open(fileURI, error)) {
        video->has_file_checksum = 1;
        calculateFileContentDigest(scope.reader(), video->file_checksum);
    }
    video->scale_factor = m_project->backgroundVideoScaleFactor();
    return video;
}

void
Native::Context::saveAccessoryPath(const Accessory *accessory, Nanoem__Project__Accessory *&ao)
{
    String path("Accessory/");
    path.append(accessory->canonicalNameConstString());
    path.append("/");
    path.append(accessory->filename().c_str());
    copyString(ao->path_for_legacy_compatibility, path);
}

Nanoem__Project__Accessory *
Native::Context::saveAccessory(Accessory *accessory, FileType fileType, Error &error)
{
    Nanoem__Project__Accessory *ao = nanoem_new(Nanoem__Project__Accessory);
    nanoem__project__accessory__init(ao);
    StringMap annotations(accessory->annotations());
    if (saveAllAnnotationsWithUUID(accessory, annotations, ao->annotations, ao->n_annotations)) {
        accessory->setAnnotations(annotations);
    }
    ao->draw_order_index = m_project->findDrawableOrderIndex(accessory);
    copyString(ao->name, accessory->canonicalName());
    saveAccessoryPath(accessory, ao);
    ao->is_active = accessory == m_project->activeAccessory();
    ao->has_accessory_handle = 1;
    ao->accessory_handle = accessory->handle();
    const URI fileURI(accessory->fileURI());
    ao->file_uri = newURI(m_project, fileURI, ao->path_for_legacy_compatibility, fileType);
    FileReaderScope scope(m_project->translator());
    if (fileType == kFileTypeData && !fileURI.isEmpty() && scope.open(fileURI, error)) {
        ao->has_file_checksum = 1;
        calculateFileContentDigest(scope.reader(), ao->file_checksum);
    }
    if (const Effect *effect = m_project->resolveEffect(accessory)) {
        const StringList includePaths(effect->allIncludePaths());
        if (!includePaths.empty()) {
            ao->n_include_paths = includePaths.size();
            ao->include_paths = new char *[ao->n_include_paths];
            nanoem_rsize_t index = 0;
            for (StringList::const_iterator it2 = includePaths.begin(), end2 = includePaths.end(); it2 != end2;
                 ++it2, ++index) {
                copyString(ao->include_paths[index], *it2);
                m_includeEffectSources[accessory].insert(tinystl::make_pair(*it2, ByteArray()));
            }
        }
    }
    return ao;
}

void
Native::Context::saveAllAccessories(
    Nanoem__Project__Project *p, FileType fileType, MotionAccessoryMap &m2a, Error &error)
{
    const Project::AccessoryList accessories(m_project->allAccessories());
    if (!accessories.empty()) {
        p->n_accessories = accessories.size();
        p->accessories = new Nanoem__Project__Accessory *[p->n_accessories];
        nanoem_rsize_t index = 0;
        for (Project::AccessoryList::const_iterator it = accessories.begin(), end = accessories.end(); it != end;
             ++it) {
            const Accessory *accessory = *it;
            p->accessories[index++] = saveAccessory(*it, fileType, error);
            m2a.insert(tinystl::make_pair(m_project->resolveMotion(accessory), accessory));
        }
    }
}

void
Native::Context::saveModelPath(const Model *model, Nanoem__Project__Model *&mo)
{
    String path("Model/");
    path.append(model->canonicalNameConstString());
    path.append("/");
    path.append(model->filename().c_str());
    copyString(mo->path_for_legacy_compatibility, path);
}

void
Native::Context::saveAllModelMaterialAttachments(
    Nanoem__Project__Model *mo, Model *model, FileType fileType, Error &error)
{
    FileReaderScope scope(m_project->translator());
    nanoem_rsize_t numMaterials;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(model->data(), &numMaterials);
    if (numMaterials > 0) {
        mo->n_material_effect_attachments = numMaterials;
        mo->material_effect_attachments = new Nanoem__Project__MaterialEffectAttachment *[numMaterials];
        nanoem_rsize_t numActualSaved = 0;
        for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
            const model::Material *material = model::Material::cast(materials[i]);
            if (const Effect *effect = material->effect()) {
                const URI fileURI(effect->fileURI());
                if (!fileURI.isEmpty()) {
                    Nanoem__Project__MaterialEffectAttachment *attachment =
                        mo->material_effect_attachments[numActualSaved++] =
                            new Nanoem__Project__MaterialEffectAttachment;
                    nanoem__project__material_effect_attachment__init(attachment);
                    attachment->offset = i;
                    copyString(attachment->path_for_legacy_compatibility, "");
                    attachment->file_uri =
                        newURI(m_project, fileURI, attachment->path_for_legacy_compatibility, fileType);
                    if (fileType == kFileTypeData && !fileURI.isEmpty() && scope.open(fileURI, error)) {
                        attachment->has_file_checksum = 1;
                        calculateFileContentDigest(scope.reader(), attachment->file_checksum);
                    }
                }
            }
        }
        mo->n_material_effect_attachments = numActualSaved;
    }
}

void
Native::Context::saveAllIncludeEffectSources(Nanoem__Project__Model *mo, Model *model)
{
    if (const Effect *effect = m_project->resolveEffect(model)) {
        const StringList includePaths(effect->allIncludePaths());
        if (!includePaths.empty()) {
            mo->n_include_paths = includePaths.size();
            mo->include_paths = new char *[mo->n_include_paths];
            nanoem_rsize_t index = 0;
            for (StringList::const_iterator it2 = includePaths.begin(), end2 = includePaths.end(); it2 != end2;
                 ++it2, ++index) {
                copyString(mo->include_paths[index], *it2);
                m_includeEffectSources[model].insert(tinystl::make_pair(*it2, ByteArray()));
            }
        }
    }
}

Nanoem__Project__Model *
Native::Context::saveModel(Model *model, FileType fileType, Error &error)
{
    Nanoem__Project__Model *mo = nanoem_new(Nanoem__Project__Model);
    nanoem__project__model__init(mo);
    StringMap annotations(model->annotations());
    if (saveAllAnnotationsWithUUID(model, annotations, mo->annotations, mo->n_annotations)) {
        model->setAnnotations(annotations);
    }
    mo->draw_order_index = m_project->findDrawableOrderIndex(model);
    mo->transform_order_index = m_project->findTransformOrderIndex(model);
    copyString(mo->name, model->canonicalName());
    saveModelPath(model, mo);
    mo->is_active = model == m_project->activeModel();
    mo->has_model_handle = 1;
    mo->model_handle = model->handle();
    const URI fileURI(model->fileURI());
    mo->file_uri = newURI(m_project, fileURI, mo->path_for_legacy_compatibility, fileType);
    FileReaderScope scope(m_project->translator());
    if (fileType == kFileTypeData && !fileURI.isEmpty() && scope.open(fileURI, error)) {
        mo->has_file_checksum = 1;
        calculateFileContentDigest(scope.reader(), mo->file_checksum);
    }
    saveAllModelMaterialAttachments(mo, model, fileType, error);
    saveAllIncludeEffectSources(mo, model);
    return mo;
}

void
Native::Context::saveAllModels(Nanoem__Project__Project *p, FileType fileType, MotionModelMap &m2m, Error &error)
{
    const Project::ModelList models(m_project->allModels());
    if (!models.empty()) {
        p->n_models = models.size();
        p->models = new Nanoem__Project__Model *[p->n_models];
        nanoem_rsize_t index = 0;
        for (Project::ModelList::const_iterator it = models.begin(), end = models.end(); it != end; ++it) {
            const Model *model = *it;
            p->models[index++] = saveModel(*it, fileType, error);
            m2m.insert(tinystl::make_pair(m_project->resolveMotion(model), model));
        }
    }
}

Nanoem__Project__Motion *
Native::Context::saveMotion(Motion *motion)
{
    Nanoem__Project__Motion *mo = nanoem_new(Nanoem__Project__Motion);
    nanoem__project__motion__init(mo);
    StringMap annotations(motion->annotations());
    if (saveAllAnnotationsWithUUID(motion, annotations, mo->annotations, mo->n_annotations)) {
        motion->setAnnotations(annotations);
    }
    copyString(mo->path_for_legacy_compatibility, "");
    mo->has_motion_handle = 1;
    mo->motion_handle = motion->handle();
    return mo;
}

void
Native::Context::saveAllMotions(Nanoem__Project__Project *p, FileType fileType, const MotionAccessoryMap &m2a,
    const MotionModelMap &m2m, Error &error)
{
    const Project::MotionList motions(m_project->allMotions());
    if (!motions.empty()) {
        const bool fillPayload = fileType == kFileTypeData;
        p->n_motions = motions.size();
        p->motions = new Nanoem__Project__Motion *[p->n_motions];
        nanoem_rsize_t index = 0;
        ByteArray bytes;
        for (Project::MotionList::const_iterator it = motions.begin(), end = motions.end(); it != end; ++it) {
            const Model *model = nullptr;
            const Motion *motion = *it;
            Nanoem__Project__Motion *m = p->motions[index++] = saveMotion(*it);
            ProtobufCBinaryData *payload = nullptr;
            MotionAccessoryMap::const_iterator it1 = m2a.find(motion);
            MotionModelMap::const_iterator it2 = m2m.find(motion);
            if (it1 != m2a.end()) {
                m->type_case = NANOEM__PROJECT__MOTION__TYPE_ACCESSORY;
                Nanoem__Project__Motion__Accessory *ptr = m->accessory = nanoem_new(Nanoem__Project__Motion__Accessory);
                nanoem__project__motion__accessory__init(ptr);
                const Accessory *accessory = it1->second;
                ptr->accessory_handle = accessory->handle();
                copyString(m->target, accessory->canonicalName());
                if (fillPayload) {
                    ptr->has_payload = 1;
                    payload = &ptr->payload;
                }
            }
            else if (it2 != m2m.end()) {
                m->type_case = NANOEM__PROJECT__MOTION__TYPE_MODEL;
                Nanoem__Project__Motion__Model *ptr = m->model = nanoem_new(Nanoem__Project__Motion__Model);
                nanoem__project__motion__model__init(ptr);
                model = it2->second;
                ptr->model_handle = model->handle();
                copyString(m->target, model->canonicalName());
                if (fillPayload) {
                    ptr->has_payload = 1;
                    payload = &ptr->payload;
                }
            }
            else if (motion == m_project->cameraMotion()) {
                m->type_case = NANOEM__PROJECT__MOTION__TYPE_CAMERA;
                copyString(m->target, "Camera");
                Nanoem__Project__Motion__Camera *ptr = m->camera = nanoem_new(Nanoem__Project__Motion__Camera);
                nanoem__project__motion__camera__init(ptr);
                if (fillPayload) {
                    ptr->has_payload = 1;
                    payload = &ptr->payload;
                }
            }
            else if (motion == m_project->lightMotion()) {
                m->type_case = NANOEM__PROJECT__MOTION__TYPE_LIGHT;
                copyString(m->target, "Light");
                Nanoem__Project__Motion__Light *ptr = m->light = nanoem_new(Nanoem__Project__Motion__Light);
                nanoem__project__motion__light__init(ptr);
                if (fillPayload) {
                    ptr->has_payload = 1;
                    payload = &ptr->payload;
                }
            }
            else if (motion == m_project->selfShadowMotion()) {
                m->type_case = NANOEM__PROJECT__MOTION__TYPE_SELF_SHADOW;
                copyString(m->target, "SelfShadow");
                Nanoem__Project__Motion__SelfShadow *ptr = m->self_shadow =
                    nanoem_new(Nanoem__Project__Motion__SelfShadow);
                nanoem__project__motion__self_shadow__init(ptr);
                if (fillPayload) {
                    ptr->has_payload = 1;
                    payload = &ptr->payload;
                }
            }
            if (payload) {
                ByteArray bytes;
                if (motion->format() == m_defaultSaveMotionFormat) {
                    motion->save(bytes, model, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
                }
                else {
                    ByteArray tempBytes;
                    const nanoem_motion_format_type_t format = motion->format();
                    motion->save(tempBytes, model, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
                    Motion *tempMotion = m_project->createMotion();
                    tempMotion->setFormat(format);
                    tempMotion->load(tempBytes, 0, error);
                    tempMotion->setFormat(m_defaultSaveMotionFormat);
                    tempMotion->save(bytes, model, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
                    m_project->destroyMotion(tempMotion);
                }
                payload->len = bytes.size();
                payload->data = new nanoem_u8_t[payload->len];
                memcpy(payload->data, bytes.data(), payload->len);
                m->file_uri = newURI(m_project, motion->fileURI(), String(), fileType);
            }
        }
    }
}

Nanoem__Project__OffscreenRenderTargetEffect *
Native::Context::saveOffscreenRenderTargetEffect(
    const IDrawable *ownerEffect, const String &ownerName, FileType fileType, Error &error)
{
    Nanoem__Project__OffscreenRenderTargetEffect *e = nanoem_new(Nanoem__Project__OffscreenRenderTargetEffect);
    nanoem__project__offscreen_render_target_effect__init(e);
    copyString(e->name, ownerName);
    if (ownerEffect) {
        e->has_owner_handle = 1;
        e->owner_handle = ownerEffect->handle();
    }
    const Project::DrawableList drawables(m_project->drawableOrderList());
    nanoem_rsize_t actualItems = 0, index = 0;
    for (Project::DrawableList::const_iterator it = drawables.begin(), end = drawables.end(); it != end; ++it) {
        const IDrawable *drawable = *it;
        const Effect *effect = m_project->upcastEffect(drawable->findOffscreenPassiveRenderTargetEffect(ownerName));
        if (effect && effect->scriptOrder() == IEffect::kScriptOrderTypeStandard) {
            actualItems++;
        }
    }
    if (actualItems > 0) {
        e->n_attachments = actualItems;
        e->attachments = new Nanoem__Project__OffscreenRenderTargetEffect__Attachment *[e->n_attachments];
        for (Project::DrawableList::const_iterator it = drawables.begin(), end = drawables.end(); it != end; ++it) {
            const IDrawable *drawable = *it;
            const Effect *effect = m_project->upcastEffect(drawable->findOffscreenPassiveRenderTargetEffect(ownerName));
            if (effect && effect->scriptOrder() == IEffect::kScriptOrderTypeStandard) {
                Nanoem__Project__OffscreenRenderTargetEffect__Attachment *attachment = e->attachments[index++] =
                    nanoem_new(Nanoem__Project__OffscreenRenderTargetEffect__Attachment);
                nanoem__project__offscreen_render_target_effect__attachment__init(attachment);
                attachment->handle = drawable->handle();
                attachment->enabled = drawable->isOffscreenPassiveRenderTargetEffectEnabled(ownerName);
                attachment->has_enabled = 1;
                const String filename(effect->filename());
                copyString(attachment->path, filename);
                const URI fileURI(effect->fileURI());
                attachment->file_uri = newURI(m_project, fileURI, filename, fileType);
                FileReaderScope scope(m_project->translator());
                if (fileType == kFileTypeData && !fileURI.isEmpty() && scope.open(fileURI, error)) {
                    attachment->has_file_checksum = 1;
                    calculateFileContentDigest(scope.reader(), attachment->file_checksum);
                }
                const StringList includePaths(effect->allIncludePaths());
                if (!includePaths.empty()) {
                    attachment->n_include_paths = includePaths.size();
                    attachment->include_paths = new char *[attachment->n_include_paths];
                    nanoem_rsize_t index = 0;
                    for (StringList::const_iterator it2 = includePaths.begin(), end2 = includePaths.end(); it2 != end2;
                         ++it2, ++index) {
                        copyString(attachment->include_paths[index], *it2);
                    }
                }
            }
        }
    }
    return e;
}

void
Native::Context::saveAllEffects(Nanoem__Project__Project *p, FileType fileType, Error &error)
{
    const Project::DrawableList drawables(m_project->drawableOrderList());
    typedef tinystl::vector<tinystl::pair<effect::OffscreenRenderTargetOption, const IDrawable *>, TinySTLAllocator>
        OffscreenRenderTargetPairList;
    OffscreenRenderTargetPairList allOptions;
    for (Project::DrawableList::const_iterator it = drawables.begin(), end = drawables.end(); it != end; ++it) {
        const IDrawable *drawable = *it;
        if (const Effect *effect = m_project->resolveEffect(drawable)) {
            effect::OffscreenRenderTargetOptionList options;
            effect->getAllOffscreenRenderTargetOptions(options);
            for (effect::OffscreenRenderTargetOptionList::const_iterator it2 = options.begin(), end2 = options.end();
                 it2 != end2; ++it2) {
                const effect::OffscreenRenderTargetOption &option = *it2;
                allOptions.push_back(tinystl::make_pair(option, drawable));
            }
        }
    }
    nanoem_rsize_t index = 0;
    p->n_offscreen_render_target_effects = allOptions.size() + 1;
    p->offscreen_render_target_effects =
        new Nanoem__Project__OffscreenRenderTargetEffect *[p->n_offscreen_render_target_effects];
    p->offscreen_render_target_effects[index++] = saveOffscreenRenderTargetEffect(nullptr, "Main", fileType, error);
    if (!allOptions.empty()) {
        for (OffscreenRenderTargetPairList::const_iterator it = allOptions.begin(), end = allOptions.end(); it != end;
             ++it) {
            const effect::OffscreenRenderTargetOption &option = it->first;
            p->offscreen_render_target_effects[index++] =
                saveOffscreenRenderTargetEffect(it->second, option.m_name, fileType, error);
        }
    }
}

bool
Native::Context::save(Nanoem__Project__Project *p, FileType fileType, Error &error)
{
    if (const Model *model = m_project->activeModel()) {
        p->axis_type = model->transformAxisType();
        p->transform_type = model->transformCoordinateType();
    }
    switch (m_project->language()) {
    case ITranslator::kLanguageTypeEnglish:
        p->language = NANOEM__COMMON__LANGUAGE__LC_ENGLISH;
        break;
    case ITranslator::kLanguageTypeJapanese:
        p->language = NANOEM__COMMON__LANGUAGE__LC_JAPANESE;
        break;
    case ITranslator::kLanguageTypeKorean:
        p->language = NANOEM__COMMON__LANGUAGE__LC_KOREAN;
        break;
    case ITranslator::kLanguageTypeChineseSimplified:
        p->language = NANOEM__COMMON__LANGUAGE__LC_SIMPLIFIED_CHINESE;
        break;
    case ITranslator::kLanguageTypeChineseTraditional:
        p->language = NANOEM__COMMON__LANGUAGE__LC_TRADITIONAL_CHINESE;
        break;
    default:
        break;
    }
    MotionAccessoryMap m2a;
    MotionModelMap m2m;
    saveAllAccessories(p, fileType, m2a, error);
    saveAllModels(p, fileType, m2m, error);
    saveAllMotions(p, fileType, m2a, m2m, error);
    saveAllEffects(p, fileType, error);
    StringMap annotations(m_annotations);
    char dateTimeBuffer[32];
    StringUtils::formatDateTimeUTC(dateTimeBuffer, sizeof(dateTimeBuffer), "%Y-%m-%dT%H:%M:%SZ");
    const String dateTimeString(dateTimeBuffer);
    annotations["datetime.updated"] = dateTimeString;
    if (annotations.find("datetime.created") == annotations.end()) {
        annotations.insert(tinystl::make_pair(String("datetime.created"), dateTimeString));
    }
    saveAllAnnotations(annotations, p->annotations, p->n_annotations);
    p->editing_mode = m_project->editingMode();
    p->is_effect_plugin_enabled = m_project->isEffectPluginEnabled();
    p->is_motion_merge_enabled = m_project->isMotionMergeEnabled();
    p->is_multiple_bone_selection_enabled = m_project->isMultipleBoneSelectionEnabled();
    p->draw_type = m_project->drawType();
    p->audio = saveAudio(fileType, error);
    p->camera = saveCamera();
    p->confirmation = saveConfirmation();
    p->grid = saveGrid();
    p->light = saveLight();
    p->physics_simulation = savePhysicsSimulation();
    p->screen = saveScreen();
    p->timeline = saveTimeline();
    p->video = saveVideo(fileType, error);
    m_project->globalCamera()->setDirty(false);
    m_project->globalLight()->setDirty(false);
    m_project->shadowCamera()->setDirty(false);
    return !error.hasReason();
}

const Project::IncludeEffectSourceMap *
Native::Context::findIncludeEffectSource(const IDrawable *drawable) const
{
    DrawableIncludeEffectSourceMap::const_iterator it = m_includeEffectSources.find(drawable);
    return it != m_includeEffectSources.end() ? &it->second : nullptr;
}

Project::IncludeEffectSourceMap *
Native::Context::findMutableIncludeEffectSource(const IDrawable *drawable)
{
    DrawableIncludeEffectSourceMap::iterator it = m_includeEffectSources.find(drawable);
    return it != m_includeEffectSources.end() ? &it->second : nullptr;
}

void
Native::Context::getAllOffscreenRenderTargetEffectAttachments(OffscreenRenderTargetEffectAttachmentList &value) const
{
    value = m_offscreenRenderTargetEffectAttachments;
}

IDrawable *
Native::Context::findDrawable(nanoem_u32_t value)
{
    DrawableHandleMap::const_iterator it = m_drawables.find(value);
    return it != m_drawables.end() ? it->second : nullptr;
}

Native::Native(Project *project)
    : m_context(nanoem_new(Context(project)))
{
}

Native::~Native() NANOEM_DECL_NOEXCEPT
{
    nanoem_delete_safe(m_context);
}

bool
Native::load(
    const nanoem_u8_t *data, nanoem_rsize_t size, FileType fileType, Error &error, Project::IDiagnostics *diagsnotics)
{
    bool succeeded = false;
    if (Nanoem__Project__Project *p = nanoem__project__project__unpack(g_protobufc_allocator, size, data)) {
        m_context->load(p, fileType, error, diagsnotics);
        nanoem__project__project__free_unpacked(p, g_protobufc_allocator);
        succeeded = true;
    }
    else {
        const ITranslator *translator = m_context->m_project->translator();
        const char *reason = translator->translate("nanoem.error.project.load.reason");
        error = Error(reason, nullptr, Error::kDomainTypeApplication);
    }
    return succeeded;
}

bool
Native::save(ByteArray &bytes, FileType fileType, Error &error)
{
    bool succeeded = false;
    Nanoem__Project__Project *p = Context::allocate();
    if (m_context->save(p, fileType, error) && protobuf_c_message_check(&p->base)) {
        const size_t packedSize = nanoem__project__project__get_packed_size(p);
        bytes.resize(packedSize);
        succeeded = nanoem__project__project__pack(p, bytes.data()) == packedSize;
        Nanoem__Project__Project *v = nanoem__project__project__unpack(g_protobufc_allocator, packedSize, bytes.data());
        succeeded = v != nullptr;
        nanoem__project__project__free_unpacked(v, g_protobufc_allocator);
    }
    Context::release(p);
    if (!succeeded && !error.hasReason()) {
        const ITranslator *translator = m_context->m_project->translator();
        error = Error(translator->translate("nanoem.error.project.save.reason"),
            translator->translate("nanoem.error.project.save.recovery-suggestion"), Error::kDomainTypeApplication);
    }
    return succeeded;
}

const Project::IncludeEffectSourceMap *
Native::findIncludeEffectSource(const IDrawable *drawable) const
{
    return m_context->findIncludeEffectSource(drawable);
}

Project::IncludeEffectSourceMap *
Native::findMutableIncludeEffectSource(const IDrawable *drawable)
{
    return m_context->findMutableIncludeEffectSource(drawable);
}

void
Native::getAllOffscreenRenderTargetEffectAttachments(OffscreenRenderTargetEffectAttachmentList &value) const
{
    m_context->getAllOffscreenRenderTargetEffectAttachments(value);
}

String
Native::findAnnotation(const String &name) const
{
    StringMap::const_iterator it = m_context->m_annotations.find(name);
    return it != m_context->m_annotations.end() ? it->second : String();
}

void
Native::setAnnotation(const String &name, const String &value)
{
    m_context->m_annotations.insert(tinystl::make_pair(name, value));
}

nanoem_motion_format_type_t
Native::defaultSaveMotionFormat() const
{
    return m_context->m_defaultSaveMotionFormat;
}

void
Native::setDefaultSaveMotionFormat(nanoem_motion_format_type_t value)
{
    m_context->m_defaultSaveMotionFormat = value;
}

URI
Native::audioURI() const
{
    return m_context->m_audioURI;
}

URI
Native::videoURI() const
{
    return m_context->m_videoURI;
}

} /* namespace project */
} /* namespace internal */
} /* namespace nanoem */
