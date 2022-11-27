/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include <assert.h>

#include "emapp/emapp.h"
#include "emapp/internal/StubEventPublisher.h"
#include "emapp/internal/project/Redo.h"
#include "emapp/private/CommonInclude.h"

#include "bx/rng.h"
#include "bx/timer.h"
#include "debug-draw/debug_draw.hpp"

using namespace nanoem;

namespace test {

class FileManager : public DefaultFileManager {
public:
    FileManager(BaseApplicationService *applicationPtr);
    ~FileManager() override;

    virtual plugin::EffectPlugin *sharedEffectPlugin() override;

private:
    BaseApplicationService *m_applicationPtr;
    plugin::EffectPlugin *m_effectPlugin = nullptr;
};

FileManager::FileManager(BaseApplicationService *applicationPtr)
    : DefaultFileManager(applicationPtr)
    , m_applicationPtr(applicationPtr)
{
}

FileManager::~FileManager()
{
    if (m_effectPlugin) {
        PluginFactory::destroyEffectPlugin(m_effectPlugin);
        m_effectPlugin = nullptr;
    }
}

plugin::EffectPlugin *
FileManager::sharedEffectPlugin()
{
    if (!m_effectPlugin) {
        const URI &fileURI = URI::createFromFilePath(
#ifdef CMAKE_INTDIR
            NANOEM_TEST_FIXTURE_PATH "/../../../emapp/plugins/effect/" CMAKE_INTDIR "/plugin_effect." BX_DL_EXT
#else
            NANOEM_TEST_FIXTURE_PATH "/../../emapp/plugins/effect/plugin_effect." BX_DL_EXT
#endif
        );
        Error error;
        IEventPublisher *publisher = m_applicationPtr->eventPublisher();
        m_effectPlugin = PluginFactory::createEffectPlugin(fileURI, publisher, error);
        error.notify(publisher);
    }
    return m_effectPlugin;
}

Application::Application(const JSON_Value *root)
    : BaseApplicationService(root)
    , m_applicationArguments(0, nullptr)
    , m_defaultFileManager(new FileManager(this))
    , m_publisher(new internal::StubEventPublisher())
{
    setEventPublisher(m_publisher.get());
    setFileManager(m_defaultFileManager.get());
}

Application::~Application()
{
}

Error
Application::lastError() const
{
    return m_publisher->lastError();
}

bool
Application::hasAnyError() const
{
    return m_publisher->hasError();
}

void
Application::sendEventMessage(const Nanoem__Application__Event * /* event */)
{
}

TestScope::Object::Object(BaseApplicationService *applicationPtr)
    : m_applicationPtr(applicationPtr)
    , m_project(nullptr)
{
    dd::initialize(nullptr);
    m_project = applicationPtr->createProject(Vector2UI16(1), SG_PIXELFORMAT_RGBA8, 1.0f, 1.0f, nullptr);
}

TestScope::Object::~Object()
{
    m_applicationPtr->destroyProject(m_project);
    m_project = nullptr;
}

Accessory *
TestScope::Object::createAccessory(const char *filename)
{
    return TestScope::createAccessory(m_project, filename);
}

Model *
TestScope::Object::createModel(const char *filename)
{
    return TestScope::createModel(m_project, filename);
}

Effect *
TestScope::Object::createBinaryEffect(IDrawable *drawable, const char *filename)
{
    return TestScope::createBinaryEffect(m_project, drawable, filename);
}

Effect *
TestScope::Object::createSourceEffect(IDrawable *drawable, const char *filename, bool inspection)
{
    return TestScope::createSourceEffect(m_project, drawable, filename, inspection);
}

Project::AccessoryList
TestScope::Object::allAccessories()
{
    return TestScope::allAccessories(m_project);
}

Project::ModelList
TestScope::Object::allModels()
{
    return TestScope::allModels(m_project);
}

nanoem_rsize_t
TestScope::Object::countAllAccessoryKeyframes(const Accessory *accessory) const
{
    nanoem_rsize_t numKeyframes = 0;
    nanoemMotionGetAllAccessoryKeyframeObjects(m_project->resolveMotion(accessory)->data(), &numKeyframes);
    return numKeyframes;
}

nanoem_rsize_t
TestScope::Object::countAllBoneKeyframes(const Model *model) const
{
    nanoem_rsize_t numKeyframes = 0;
    nanoemMotionGetAllBoneKeyframeObjects(m_project->resolveMotion(model)->data(), &numKeyframes);
    return numKeyframes;
}

nanoem_rsize_t
TestScope::Object::countAllModelKeyframes(const Model *model) const
{
    nanoem_rsize_t numKeyframes = 0;
    nanoemMotionGetAllModelKeyframeObjects(m_project->resolveMotion(model)->data(), &numKeyframes);
    return numKeyframes;
}

nanoem_rsize_t
TestScope::Object::countAllMorphKeyframes(const Model *model) const
{
    nanoem_rsize_t numKeyframes = 0;
    nanoemMotionGetAllMorphKeyframeObjects(m_project->resolveMotion(model)->data(), &numKeyframes);
    return numKeyframes;
}

nanoem_rsize_t
TestScope::Object::countAllMorphKeyframesByCategory(const Model *model, nanoem_model_morph_category_t category) const
{
    nanoem_rsize_t numKeyframes = 0, numActualKeyframes = 0;
    nanoem_motion_morph_keyframe_t *const *keyframes =
        nanoemMotionGetAllMorphKeyframeObjects(m_project->resolveMotion(model)->data(), &numKeyframes);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        if (nanoemModelMorphGetCategory(model->findMorph(nanoemMotionMorphKeyframeGetName(keyframes[i]))) == category) {
            numActualKeyframes++;
        }
    }
    return numActualKeyframes;
};

nanoem_frame_index_t
TestScope::Object::motionDuration(const IDrawable *drawable) const
{
    return m_project->resolveMotion(drawable)->duration();
}

const nanoem_motion_accessory_keyframe_t *
TestScope::Object::findAccessoryKeyframe(const Accessory *accessory, nanoem_frame_index_t frameIndex) const
{
    return nanoemMotionFindAccessoryKeyframeObject(m_project->resolveMotion(accessory)->data(), frameIndex);
}

const nanoem_motion_bone_keyframe_t *
TestScope::Object::findBoneKeyframe(
    const Model *model, const nanoem_unicode_string_t *name, nanoem_frame_index_t frameIndex) const
{
    const Motion *motion = m_project->resolveMotion(model);
    return motion->findBoneKeyframe(name, frameIndex);
}

const nanoem_motion_bone_keyframe_t *
TestScope::Object::findBoneKeyframe(const Model *model) const
{
    return findBoneKeyframe(model, 0);
}

const nanoem_motion_bone_keyframe_t *
TestScope::Object::findBoneKeyframe(const Model *model, nanoem_frame_index_t frameIndex) const
{
    const nanoem_model_bone_t *bonePtr = model->activeBone();
    return findBoneKeyframe(model, nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), frameIndex);
}

const nanoem_motion_bone_keyframe_t *
TestScope::Object::findBoneKeyframe(
    const Model *model, const nanoem_model_bone_t *bonePtr, nanoem_frame_index_t frameIndex) const
{
    return findBoneKeyframe(model, nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), frameIndex);
}

const nanoem_motion_morph_keyframe_t *
TestScope::Object::findMorphKeyframe(
    const Model *model, const nanoem_unicode_string_t *name, nanoem_frame_index_t frameIndex) const
{
    const Motion *motion = m_project->resolveMotion(model);
    return motion->findMorphKeyframe(name, frameIndex);
}

const nanoem_motion_morph_keyframe_t *
TestScope::Object::findMorphKeyframe(
    const Model *model, const nanoem_model_morph_t *morphPtr, nanoem_frame_index_t frameIndex) const
{
    return findMorphKeyframe(model, nanoemModelMorphGetName(morphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), frameIndex);
}

const nanoem_model_morph_t *
TestScope::Object::findFirstMorph(const Model *model, nanoem_model_morph_category_t category) const
{
    nanoem_rsize_t numMorphs = 0;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numMorphs);
    for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
        if (nanoemModelMorphGetCategory(morphs[i]) == category) {
            return morphs[i];
        }
    }
    return nullptr;
}

nanoem_f32_t
TestScope::Object::findFirstMorphKeyframeWeight(const Model *model, nanoem_model_morph_category_t category) const
{
    const nanoem_motion_t *motion = m_project->resolveMotion(model)->data();
    const nanoem_unicode_string_t *name =
        nanoemModelMorphGetName(findFirstMorph(model, category), NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
    return nanoemMotionMorphKeyframeGetWeight(nanoemMotionFindMorphKeyframeObject(motion, name, 0));
}

Project *
TestScope::Object::withRecoverable(const char *path)
{
    FileUtils::deleteFile(path);
    m_project->setRedoFileURI(URI::createFromFilePath(path));
    return m_project;
}

std::string
TestScope::Object::toString(const nanoem_unicode_string_t *value) const
{
    String s;
    StringUtils::getUtf8String(value, m_project->unicodeStringFactory(), s);
    return std::string(s.c_str(), s.size());
}

static nanoem_rsize_t
rnd(nanoem_rsize_t numObjects)
{
    nanoem_i64_t value = bx::getHPCounter();
    bx::RngMwc rng((value >> 32) & 0xffffffff, value & 0xffffffff);
    return nanoem_rsize_t(bx::frnd(&rng) * (numObjects - 1));
}

const nanoem_model_bone_t *
TestScope::findFirstBone(const Model *model)
{
    nanoem_rsize_t numObjects;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numObjects);
    return bones[0];
}

const nanoem_model_morph_t *
TestScope::findFirstMorph(const Model *model)
{
    nanoem_rsize_t numObjects;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numObjects);
    return morphs[0];
}

const nanoem_model_bone_t *
TestScope::findRandomBone(const Model *model)
{
    nanoem_rsize_t numObjects;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numObjects);
    return bones[rnd(numObjects)];
}

const nanoem_model_morph_t *
TestScope::findRandomMorph(const Model *model)
{
    nanoem_i64_t value = bx::getHPCounter();
    bx::RngMwc rng((value >> 32) & 0xffffffff, value & 0xffffffff);
    nanoem_rsize_t numObjects;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numObjects);
    return morphs[rnd(numObjects)];
}

Accessory *
TestScope::createAccessory(Project *project, const char *filename)
{
    Accessory *accessory = project->createAccessory();
    String path(NANOEM_TEST_FIXTURE_PATH);
    path.append("/");
    path.append(filename);
    FileReaderScope scope(project->translator());
    Error error;
    const URI &fileURI = URI::createFromFilePath(path);
    if (scope.open(fileURI, error)) {
        ByteArray bytes;
        FileUtils::read(scope, bytes, error);
        accessory->setName(URI::stringByDeletingPathExtension(fileURI.lastPathComponent()));
        accessory->setFileURI(fileURI);
        if (accessory->load(bytes, error)) {
            accessory->writeLoadCommandMessage(error);
            accessory->upload();
        }
        else {
            WARN(error.reasonConstString());
            project->destroyAccessory(accessory);
            accessory = nullptr;
        }
    }
    return accessory;
}

Model *
TestScope::createModel(Project *project, const char *filename)
{
    Model *model = project->createModel();
    String path(NANOEM_TEST_FIXTURE_PATH);
    path.append("/");
    path.append(filename);
    FileReaderScope scope(project->translator());
    Error error;
    const URI &fileURI = URI::createFromFilePath(path);
    if (scope.open(fileURI, error)) {
        ByteArray bytes;
        FileUtils::read(scope, bytes, error);
        model->setFileURI(fileURI);
        if (model->load(bytes, error)) {
            model->writeLoadCommandMessage(error);
            model->setupAllBindings();
            model->createAllImages();
            model->upload();
            Progress progress(project, 0);
            model->loadAllImages(progress, error);
            model->setVisible(true);
        }
        else {
            WARN(error.reasonConstString());
            project->destroyModel(model);
            model = nullptr;
        }
    }
    return model;
}

Effect *
TestScope::createBinaryEffect(Project *project, IDrawable *drawable, const char *filename)
{
    Effect *effect = project->createEffect();
    String path;
    path.append(NANOEM_TEST_FIXTURE_PATH);
    path.append("/");
    path.append(filename);
    FileReaderScope scope(project->translator());
    Error error;
    const URI &fileURI = URI::createFromFilePath(path);
    if (scope.open(fileURI, error)) {
        ByteArray bytes;
        Progress progress(project, 0);
        FileUtils::read(scope, bytes, error);
        effect->setFileURI(fileURI);
        if (effect->load(bytes, progress, error) && effect->upload(effect::kAttachmentTypeNone, progress, error)) {
            project->attachActiveEffect(drawable, effect, Project::IncludeEffectSourceMap(), progress, error);
        }
        else {
            WARN(error.reasonConstString());
            project->destroyEffect(effect);
            effect = nullptr;
        }
    }
    return effect;
}

Effect *
TestScope::createSourceEffect(Project *project, IDrawable *drawable, const char *filename, bool inspection)
{
    Effect *effect = project->createEffect();
    String path;
    path.append(NANOEM_TEST_FIXTURE_PATH);
    path.append("/");
    path.append(filename);
    FileReaderScope scope(project->translator());
    Error error;
    const URI &fileURI = URI::createFromFilePath(path);
    if (scope.open(fileURI, error)) {
        ByteArray bytes, output;
        Progress progress(project, 0);
        FileUtils::read(scope, bytes, error);
        IFileManager *fileManager = project->fileManager();
        const URI &resolvedURI = Effect::resolveSourceURI(fileManager, fileURI);
        if (Effect::compileFromSource(resolvedURI, fileManager, false, output, progress, error)) {
            effect->setPassUniformBufferInspectionEnabled(inspection);
            effect->setFileURI(resolvedURI);
            effect->load(output, progress, error) && effect->upload(effect::kAttachmentTypeNone, progress, error);
            if (!error.hasReason()) {
                project->attachActiveEffect(drawable, effect, Project::IncludeEffectSourceMap(), progress, error);
            }
        }
        if (error.hasReason()) {
            WARN(error.reasonConstString());
            project->destroyEffect(effect);
            effect = nullptr;
        }
    }
    return effect;
}

Project::AccessoryList
TestScope::allAccessories(Project *project)
{
    const Project::DrawableList *allDrawables = project->drawableOrderList();
    Project::AccessoryList allAccessories;
    const StringSet &loadables = Accessory::loadableExtensionsSet();
    for (Project::DrawableList::const_iterator it = allDrawables->begin(), end = allDrawables->end(); it != end; ++it) {
        IDrawable *drawable = *it;
        const URI &fileURI = drawable->fileURI();
        if (loadables.find(fileURI.pathExtension()) != loadables.end() ||
            loadables.find(URI::pathExtension(fileURI.fragment())) != loadables.end()) {
            allAccessories.push_back(static_cast<Accessory *>(drawable));
        }
    }
    return allAccessories;
}

Project::ModelList
TestScope::allModels(Project *project)
{
    const Project::DrawableList *allDrawables = project->drawableOrderList();
    Project::ModelList allModels;
    const StringSet &loadables = Model::loadableExtensionsSet();
    for (Project::DrawableList::const_iterator it = allDrawables->begin(), end = allDrawables->end(); it != end; ++it) {
        IDrawable *drawable = *it;
        const URI &fileURI = drawable->fileURI();
        if (loadables.find(fileURI.pathExtension()) != loadables.end() ||
            loadables.find(URI::pathExtension(fileURI.fragment())) != loadables.end()) {
            allModels.push_back(static_cast<Model *>(drawable));
        }
    }
    return allModels;
}

TestScope::TestScope()
{
    m_config = json_value_init_object();
    m_application = new Application(m_config);
    m_application->initialize(1.0f, 1.0f);
}

TestScope::~TestScope()
{
    m_application->destroy();
    delete m_application;
    m_application = nullptr;
    json_value_free(m_config);
    m_config = nullptr;
}

void
TestScope::recover(Project *project, const char *path)
{
    Error error;
    IFileReader *reader = FileUtils::createFileReader(m_application->translator());
    bool result = reader->open(URI::createFromFilePath(path), error);
    BX_UNUSED_1(result);
    assert(result);
    internal::project::Redo redo(project);
    redo.loadAll(reader, m_application, error);
    assert(!error.hasReason());
    FileUtils::destroyFileReader(reader);
    FileUtils::deleteFile(path);
}

void
TestScope::deleteFile(const char *path)
{
    FileUtils::deleteFile(path);
}

Application *
TestScope::application()
{
    return m_application;
}

ProjectPtr
TestScope::createProject()
{
    const Vector2UI16 windowSize(960, 480);
    ProjectPtr p(new TestScope::Object(m_application));
    Project *project = p.get()->m_project;
    project->setBaseDuration(1337);
    project->setUniformedViewportImageSizeEnabled(false);
    project->setViewportPixelFormat(SG_PIXELFORMAT_BGRA8);
    project->resizeUniformedViewportLayout(Vector4UI16(0, 0, windowSize));
    project->resizeUniformedViewportImage(windowSize);
    project->resizeWindowSize(windowSize);
    project->resetAllPasses();
    return p;
}

bool
TestScope::hasAnyError() const
{
    return m_application->hasAnyError();
}

} /* namespace test */
