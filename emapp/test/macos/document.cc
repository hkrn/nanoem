/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/Accessory.h"
#include "emapp/DirectionalLight.h"
#include "emapp/FileUtils.h"
#include "emapp/Grid.h"
#include "emapp/IAudioPlayer.h"
#include "emapp/IFileManager.h"
#include "emapp/IProjectHolder.h"
#include "emapp/Model.h"
#include "emapp/PerspectiveCamera.h"
#include "emapp/ShadowCamera.h"

using namespace nanoem;
using namespace test;

extern "C" {
struct nanoem_application_document_t;
nanoem_application_document_t *nanoemApplicationDocumentCreate(
    BaseApplicationService *application, IProjectHolder *holder, const char *path);
nanoem_bool_t nanoemApplicationDocumentLoad(nanoem_application_document_t *document, Error *error);
nanoem_bool_t nanoemApplicationDocumentSave(nanoem_application_document_t *document, const char *path, Error *error);
Project *nanoemApplicationDocumentGetProject(nanoem_application_document_t *document);
void nanoemApplicationDocumentDeleteFile(nanoem_application_document_t *document);
void nanoemApplicationDocumentDestroy(nanoem_application_document_t *document);
}

class DocumentWrapper : public IProjectHolder {
public:
    DocumentWrapper(TestScope &scope, const char *path)
        : m_project(scope.createProject())
        , m_opaque(nanoemApplicationDocumentCreate(scope.application(), this, path))
    {
    }
    ~DocumentWrapper()
    {
        nanoemApplicationDocumentDestroy(m_opaque);
    }

    bool
    load()
    {
        Error error;
        return nanoemApplicationDocumentLoad(m_opaque, &error);
    }
    bool
    save(const char *path)
    {
        Error error;
        return nanoemApplicationDocumentSave(m_opaque, path, &error);
    }
    void
    clear()
    {
        nanoemApplicationDocumentDeleteFile(m_opaque);
    }

    const Project *
    currentProject() const override
    {
        return m_project.get()->m_project;
    }
    Project *
    currentProject() override
    {
        return m_project.get()->m_project;
    }

private:
    ProjectPtr m_project;
    nanoem_application_document_t *m_opaque;
};

TEST_CASE("document_save_and_load", "emapp")
{
    TestScope scope;
    Application *application = scope.application();
    BX_UNUSED_1(application);
    {
        std::unique_ptr<DocumentWrapper> wrapper(
            new DocumentWrapper(scope, NANOEM_TEST_OUTPUT_PATH "/document_save_and_load.nanoem"));
        wrapper->clear();
        Project *project = wrapper->currentProject();
        for (int i = 0; i < 3; i++) {
            Accessory *accessory = TestScope::createAccessory(project, "test.x");
            project->addAccessory(accessory);
        }
        for (int i = 0; i < 3; i++) {
            Model *model = TestScope::createModel(project, "test.pmx");
            project->addModel(model);
        }
        project->audioPlayer()->setVolumeGain(0.4f);
        project->setDrawType(IDrawable::kDrawTypeColor);
        project->setEditingMode(Project::kEditingModeRotate);
        project->setConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE, true);
        project->setConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA, true);
        project->setConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT, true);
        project->setConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL, true);
        project->setConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH, true);
        project->setEffectPluginEnabled(true);
        project->setMotionMergeEnabled(true);
        project->setMultipleBoneSelectionEnabled(true);
        project->grid()->setCell(Vector2(42, 84));
        project->grid()->setOpacity(0.6f);
        project->grid()->setVisible(false);
        project->setLanguage(ITranslator::kLanguageTypeEnglish);
        project->setGroundShadowEnabled(false);
        project->setShadowMapSize(Vector2(512, 512));
        project->setShadowMapEnabled(true);
        project->shadowCamera()->setCoverageMode(ShadowCamera::kCoverageModeType2);
        project->setPhysicsSimulationMode(PhysicsEngine::kSimulationModeEnableTracing);
        project->setPhysicsSimulationEngineDebugFlags(1 | 2 | 4 | 16 | 64);
        project->setViewportBackgroundColor(Vector4(0));
        project->setSampleLevel(2);
        project->setBaseDuration(2560);
        project->seek(864, true);
        project->setPreferredMotionFPS(60, false);
        project->setLoopEnabled(true);
        project->activeCamera()->setAngle(Vector3(0.2, 0.4, 0.6));
        project->activeCamera()->setLookAt(Vector3(42, 84, 126));
        project->activeCamera()->setDistance(576);
        project->activeCamera()->setFov(114);
        project->activeCamera()->setPerspective(false);
        project->activeLight()->setColor(Vector3(0.3, 0.5, 0.7));
        project->activeLight()->setDirection(Vector3(-0.3, 0.9, 0.4));
        REQUIRE(project->resetAllPasses());
        CHECK(wrapper->save(NANOEM_TEST_OUTPUT_PATH "/document_save_and_load.nanoem"));
    }
    {
        std::unique_ptr<DocumentWrapper> wrapper(
            new DocumentWrapper(scope, NANOEM_TEST_OUTPUT_PATH "/document_save_and_load.nanoem"));
        CHECK(wrapper->load());
        Project *newProject = wrapper->currentProject();
        CHECK(newProject->resetAllPasses());
        CHECK(newProject->audioPlayer()->volumeGain() == 0.4f);
        CHECK(newProject->drawType() == IDrawable::kDrawTypeColor);
        CHECK(newProject->editingMode() == Project::kEditingModeRotate);
        CHECK(newProject->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE));
        CHECK(newProject->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA));
        CHECK(newProject->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT));
        CHECK(newProject->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL));
        CHECK(newProject->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH));
        CHECK(newProject->isEffectPluginEnabled());
        CHECK(newProject->isMotionMergeEnabled());
        CHECK(newProject->isMultipleBoneSelectionEnabled());
        CHECK_THAT(newProject->grid()->cell(), Equals(Vector2(42, 84)));
        CHECK(newProject->grid()->opacity() == 0.6f);
        CHECK_FALSE(newProject->grid()->isVisible());
        CHECK(newProject->language() == ITranslator::kLanguageTypeEnglish);
        CHECK(newProject->castLanguage() == NANOEM_LANGUAGE_TYPE_ENGLISH);
        CHECK_FALSE(newProject->isGroundShadowEnabled());
        // CHECK(newProject->shadowMapSize() == Vector2UI16(512, 512));
        // CHECK(newProject->isShadowMapEnabled());
        CHECK(newProject->shadowCamera()->coverageMode() == 2);
        CHECK(newProject->physicsEngine()->simulationMode() == PhysicsEngine::kSimulationModeEnableTracing);
        CHECK(newProject->physicsEngine()->debugGeometryFlags() == (1 | 2 | 4 | 16 | 64));
        CHECK_THAT(newProject->viewportBackgroundColor(), Equals(Vector4(0)));
        CHECK(newProject->sampleLevel() == 2);
        CHECK(newProject->sampleCount() == 4);
        CHECK(newProject->baseDuration() == 2560);
        CHECK(newProject->currentLocalFrameIndex() == 864);
        CHECK(newProject->preferredMotionFPS() == 60);
        CHECK(newProject->isLoopEnabled());
        CHECK(newProject->allAccessories()->size() == 3);
        CHECK(newProject->allModels()->size() == 3);
        CHECK(newProject->drawableOrderList()->size() == 6);
        CHECK(newProject->transformOrderList()->size() == 3);
        CHECK_THAT(newProject->activeCamera()->angle(), Equals(Vector3(0.2, 0.4, 0.6)));
        CHECK_THAT(newProject->activeCamera()->lookAt(), Equals(Vector3(42, 84, 126)));
        CHECK(newProject->activeCamera()->distance() == 576);
        CHECK(newProject->activeCamera()->fov() == 114);
        CHECK_FALSE(newProject->activeCamera()->isPerspective());
        CHECK_THAT(newProject->activeLight()->color(), Equals(Vector3(0.3, 0.5, 0.7)));
        CHECK_THAT(newProject->activeLight()->direction(), Equals(Vector3(-0.3, 0.9, 0.4)));
        CHECK_FALSE(newProject->isDirty());
        const Project::AccessoryList &allAccessories = TestScope::allAccessories(newProject);
        const Project::ModelList &allModels = TestScope::allModels(newProject);
        CHECK(allAccessories[0]->fileURI().lastPathComponent() == String("document_save_and_load.nanoem"));
        CHECK(URI::lastPathComponent(allAccessories[0]->fileURI().fragment()) == String("test.x"));
        CHECK(allAccessories[1]->fileURI().lastPathComponent() == String("document_save_and_load.nanoem"));
        CHECK(URI::lastPathComponent(allAccessories[1]->fileURI().fragment()) == String("test.x"));
        CHECK(allAccessories[2]->fileURI().lastPathComponent() == String("document_save_and_load.nanoem"));
        CHECK(URI::lastPathComponent(allAccessories[2]->fileURI().fragment()) == String("test.x"));
        CHECK(allModels[0]->fileURI().lastPathComponent() == String("document_save_and_load.nanoem"));
        CHECK(URI::lastPathComponent(allModels[0]->fileURI().fragment()) == String("test.pmx"));
        CHECK(allModels[1]->fileURI().lastPathComponent() == String("document_save_and_load.nanoem"));
        CHECK(URI::lastPathComponent(allModels[1]->fileURI().fragment()) == String("test.pmx"));
        CHECK(allModels[2]->fileURI().lastPathComponent() == String("document_save_and_load.nanoem"));
        CHECK(URI::lastPathComponent(allModels[2]->fileURI().fragment()) == String("test.pmx"));
        wrapper->clear();
    }
}

TEST_CASE("document_resave", "[emapp][project][macos]")
{
    TestScope scope;
    Application *application = scope.application();
    BX_UNUSED_1(application);
    {
        std::unique_ptr<DocumentWrapper> wrapper(
            new DocumentWrapper(scope, NANOEM_TEST_OUTPUT_PATH "/document_resave.nanoem"));
        wrapper->clear();
        Project *project = wrapper->currentProject();
        for (int i = 0; i < 3; i++) {
            Accessory *accessory = TestScope::createAccessory(project, "test.x");
            project->addAccessory(accessory);
        }
        for (int i = 0; i < 3; i++) {
            Model *model = TestScope::createModel(project, "test.pmx");
            project->addModel(model);
        }
        project->audioPlayer()->setVolumeGain(0.4f);
        project->setDrawType(IDrawable::kDrawTypeColor);
        project->setEditingMode(Project::kEditingModeRotate);
        project->setConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE, true);
        project->setConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA, true);
        project->setConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT, true);
        project->setConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL, true);
        project->setConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH, true);
        project->setEffectPluginEnabled(true);
        project->setMotionMergeEnabled(true);
        project->setMultipleBoneSelectionEnabled(true);
        project->grid()->setCell(Vector2(42, 84));
        project->grid()->setOpacity(0.6f);
        project->grid()->setVisible(false);
        project->setLanguage(ITranslator::kLanguageTypeEnglish);
        project->setGroundShadowEnabled(false);
        project->setShadowMapSize(Vector2(512, 512));
        project->setShadowMapEnabled(true);
        project->shadowCamera()->setCoverageMode(ShadowCamera::kCoverageModeType2);
        project->setPhysicsSimulationMode(PhysicsEngine::kSimulationModeEnableTracing);
        project->setPhysicsSimulationEngineDebugFlags(1 | 2 | 4 | 16 | 64);
        project->setViewportBackgroundColor(Vector4(0));
        project->setSampleLevel(2);
        project->setBaseDuration(2560);
        project->seek(864, true);
        project->setPreferredMotionFPS(60, false);
        project->setLoopEnabled(true);
        project->activeCamera()->setAngle(Vector3(0.2, 0.4, 0.6));
        project->activeCamera()->setLookAt(Vector3(42, 84, 126));
        project->activeCamera()->setDistance(576);
        project->activeCamera()->setFov(114);
        project->activeCamera()->setPerspective(false);
        project->activeLight()->setColor(Vector3(0.3, 0.5, 0.7));
        project->activeLight()->setDirection(Vector3(-0.3, 0.9, 0.4));
        REQUIRE(project->resetAllPasses());
        CHECK(wrapper->save(NANOEM_TEST_OUTPUT_PATH "/transient.nanoem"));
    }
    {
        std::unique_ptr<DocumentWrapper> wrapper(
            new DocumentWrapper(scope, NANOEM_TEST_OUTPUT_PATH "/transient.nanoem"));
        CHECK(wrapper->load());
        REQUIRE(wrapper->currentProject()->resetAllPasses());
        CHECK(wrapper->save(NANOEM_TEST_OUTPUT_PATH "/document_resave.nanoem"));
        wrapper->clear();
    }
    {
        std::unique_ptr<DocumentWrapper> wrapper(
            new DocumentWrapper(scope, NANOEM_TEST_OUTPUT_PATH "/document_resave.nanoem"));
        CHECK(wrapper->load());
        Project *newProject = wrapper->currentProject();
        CHECK(newProject->resetAllPasses());
        CHECK(newProject->audioPlayer()->volumeGain() == 0.4f);
        CHECK(newProject->drawType() == IDrawable::kDrawTypeColor);
        CHECK(newProject->editingMode() == Project::kEditingModeRotate);
        CHECK(newProject->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE));
        CHECK(newProject->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA));
        CHECK(newProject->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT));
        CHECK(newProject->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL));
        CHECK(newProject->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH));
        CHECK(newProject->isEffectPluginEnabled());
        CHECK(newProject->isMotionMergeEnabled());
        CHECK(newProject->isMultipleBoneSelectionEnabled());
        CHECK_THAT(newProject->grid()->cell(), Equals(Vector2(42, 84)));
        CHECK(newProject->grid()->opacity() == 0.6f);
        CHECK_FALSE(newProject->grid()->isVisible());
        CHECK(newProject->language() == ITranslator::kLanguageTypeEnglish);
        CHECK(newProject->castLanguage() == NANOEM_LANGUAGE_TYPE_ENGLISH);
        CHECK_FALSE(newProject->isGroundShadowEnabled());
        // CHECK(newProject->shadowMapSize() == Vector2UI16(512, 512));
        // CHECK(newProject->isShadowMapEnabled());
        CHECK(newProject->shadowCamera()->coverageMode() == 2);
        CHECK(newProject->physicsEngine()->simulationMode() == PhysicsEngine::kSimulationModeEnableTracing);
        CHECK(newProject->physicsEngine()->debugGeometryFlags() == (1 | 2 | 4 | 16 | 64));
        CHECK_THAT(newProject->viewportBackgroundColor(), Equals(Vector4(0)));
        CHECK(newProject->sampleLevel() == 2);
        CHECK(newProject->sampleCount() == 4);
        CHECK(newProject->baseDuration() == 2560);
        CHECK(newProject->currentLocalFrameIndex() == 864);
        CHECK(newProject->preferredMotionFPS() == 60);
        CHECK(newProject->isLoopEnabled());
        CHECK(newProject->allAccessories()->size() == 3);
        CHECK(newProject->allModels()->size() == 3);
        CHECK(newProject->drawableOrderList()->size() == 6);
        CHECK(newProject->transformOrderList()->size() == 3);
        CHECK_THAT(newProject->activeCamera()->angle(), Equals(Vector3(0.2, 0.4, 0.6)));
        CHECK_THAT(newProject->activeCamera()->lookAt(), Equals(Vector3(42, 84, 126)));
        CHECK(newProject->activeCamera()->distance() == 576);
        CHECK(newProject->activeCamera()->fov() == 114);
        CHECK_FALSE(newProject->activeCamera()->isPerspective());
        CHECK_THAT(newProject->activeLight()->color(), Equals(Vector3(0.3, 0.5, 0.7)));
        CHECK_THAT(newProject->activeLight()->direction(), Equals(Vector3(-0.3, 0.9, 0.4)));
        CHECK_FALSE(newProject->isDirty());
        const Project::AccessoryList &allAccessories = TestScope::allAccessories(newProject);
        const Project::ModelList &allModels = TestScope::allModels(newProject);
        CHECK(allAccessories[0]->fileURI().lastPathComponent() == String("document_resave.nanoem"));
        CHECK(URI::lastPathComponent(allAccessories[0]->fileURI().fragment()) == String("test.x"));
        CHECK(allAccessories[1]->fileURI().lastPathComponent() == String("document_resave.nanoem"));
        CHECK(URI::lastPathComponent(allAccessories[1]->fileURI().fragment()) == String("test.x"));
        CHECK(allAccessories[2]->fileURI().lastPathComponent() == String("document_resave.nanoem"));
        CHECK(URI::lastPathComponent(allAccessories[2]->fileURI().fragment()) == String("test.x"));
        CHECK(allModels[0]->fileURI().lastPathComponent() == String("document_resave.nanoem"));
        CHECK(URI::lastPathComponent(allModels[0]->fileURI().fragment()) == String("test.pmx"));
        CHECK(allModels[1]->fileURI().lastPathComponent() == String("document_resave.nanoem"));
        CHECK(URI::lastPathComponent(allModels[1]->fileURI().fragment()) == String("test.pmx"));
        CHECK(allModels[2]->fileURI().lastPathComponent() == String("document_resave.nanoem"));
        CHECK(URI::lastPathComponent(allModels[2]->fileURI().fragment()) == String("test.pmx"));
        wrapper->clear();
    }
}

TEST_CASE("document_save_and_load_effect", "[emapp][project][macos]")
{
    TestScope scope;
    Application *application = scope.application();
    BX_UNUSED_1(application);
    {
        std::unique_ptr<DocumentWrapper> wrapper(
            new DocumentWrapper(scope, NANOEM_TEST_OUTPUT_PATH "/document_save_and_load_effect.nanoem"));
        wrapper->clear();
        Project *project = wrapper->currentProject();
        project->setEffectPluginEnabled(true);
        {
            Accessory *accessory = TestScope::createAccessory(project, "effects/main.x");
            TestScope::createSourceEffect(project, accessory, "effects/main.fx");
            project->addAccessory(accessory);
        }
        {
            Accessory *accessory = TestScope::createAccessory(project, "effects/offscreen.x");
            TestScope::createSourceEffect(project, accessory, "effects/offscreen.fx");
            project->addAccessory(accessory);
        }
        {
            Model *model = TestScope::createModel(project, "effects/main.pmx");
            model->setName("main");
            TestScope::createSourceEffect(project, model, "effects/main.fx");
            project->addModel(model);
        }
        {
            Model *model = TestScope::createModel(project, "effects/offscreen.pmx");
            model->setName("offscreen");
            TestScope::createSourceEffect(project, model, "effects/offscreen.fx");
            project->addModel(model);
        }
        CHECK(wrapper->save(NANOEM_TEST_OUTPUT_PATH "/document_save_and_load_effect.nanoem"));
    }
    {
        std::unique_ptr<DocumentWrapper> wrapper(
            new DocumentWrapper(scope, NANOEM_TEST_OUTPUT_PATH "/document_save_and_load_effect.nanoem"));
        CHECK(wrapper->load());
        Project *newProject = wrapper->currentProject();
        REQUIRE(newProject->isEffectPluginEnabled());
        CHECK_FALSE(newProject->isDirty());
        CHECK(newProject->allAccessories()->size() == 2);
        CHECK(newProject->allModels()->size() == 2);
        CHECK(newProject->drawableOrderList()->size() == 4);
        CHECK(newProject->transformOrderList()->size() == 2);
        const Project::AccessoryList &allAccessories = TestScope::allAccessories(newProject);
        const Project::ModelList &allModels = TestScope::allModels(newProject);
        REQUIRE(allAccessories.size() == 2);
        REQUIRE(allModels.size() == 2);
        {
            CHECK(allAccessories[0]->fileURI().lastPathComponent() == String("document_save_and_load_effect.nanoem"));
            CHECK(URI::lastPathComponent(allAccessories[0]->fileURI().fragment()) == String("main.x"));
            CHECK(allAccessories[0]->activeEffect());
        }
        {
            CHECK(allAccessories[1]->fileURI().lastPathComponent() == String("document_save_and_load_effect.nanoem"));
            CHECK(URI::lastPathComponent(allAccessories[1]->fileURI().fragment()) == String("offscreen.x"));
            CHECK(allAccessories[1]->activeEffect());
        }
        {
            CHECK(allModels[0]->fileURI().lastPathComponent() == String("document_save_and_load_effect.nanoem"));
            CHECK(URI::lastPathComponent(allModels[0]->fileURI().fragment()) == String("main.pmx"));
            CHECK(allModels[0]->activeEffect());
        }
        {
            CHECK(allModels[1]->fileURI().lastPathComponent() == String("document_save_and_load_effect.nanoem"));
            CHECK(URI::lastPathComponent(allModels[1]->fileURI().fragment()) == String("offscreen.pmx"));
            CHECK(allModels[1]->activeEffect());
        }
        wrapper->clear();
    }
}

TEST_CASE("document_resave_effect", "[emapp][project][macos]")
{
    TestScope scope;
    Application *application = scope.application();
    BX_UNUSED_1(application);
    {
        std::unique_ptr<DocumentWrapper> wrapper(
            new DocumentWrapper(scope, NANOEM_TEST_OUTPUT_PATH "/document_resave_effect.nanoem"));
        wrapper->clear();
        Project *project = wrapper->currentProject();
        project->setEffectPluginEnabled(true);
        {
            Accessory *accessory = TestScope::createAccessory(project, "effects/main.x");
            TestScope::createSourceEffect(project, accessory, "effects/main.fx");
            project->addAccessory(accessory);
        }
        {
            Accessory *accessory = TestScope::createAccessory(project, "effects/offscreen.x");
            TestScope::createSourceEffect(project, accessory, "effects/offscreen.fx");
            project->addAccessory(accessory);
        }
        {
            Model *model = TestScope::createModel(project, "effects/main.pmx");
            model->setName("main");
            TestScope::createSourceEffect(project, model, "effects/main.fx");
            project->addModel(model);
        }
        {
            Model *model = TestScope::createModel(project, "effects/offscreen.pmx");
            model->setName("offscreen");
            TestScope::createSourceEffect(project, model, "effects/offscreen.fx");
            project->addModel(model);
        }
        CHECK(wrapper->save(NANOEM_TEST_OUTPUT_PATH "/transient_effect.nanoem"));
    }
    {
        std::unique_ptr<DocumentWrapper> wrapper(
            new DocumentWrapper(scope, NANOEM_TEST_OUTPUT_PATH "/transient_effect.nanoem"));
        CHECK(wrapper->load());
        CHECK(wrapper->save(NANOEM_TEST_OUTPUT_PATH "/document_resave_effect.nanoem"));
        wrapper->clear();
    }
    {
        std::unique_ptr<DocumentWrapper> wrapper(
            new DocumentWrapper(scope, NANOEM_TEST_OUTPUT_PATH "/document_resave_effect.nanoem"));
        CHECK(wrapper->load());
        Project *newProject = wrapper->currentProject();
        REQUIRE(newProject->isEffectPluginEnabled());
        CHECK_FALSE(newProject->isDirty());
        CHECK(newProject->allAccessories()->size() == 2);
        CHECK(newProject->allModels()->size() == 2);
        CHECK(newProject->drawableOrderList()->size() == 4);
        CHECK(newProject->transformOrderList()->size() == 2);
        const Project::AccessoryList &allAccessories = TestScope::allAccessories(newProject);
        const Project::ModelList &allModels = TestScope::allModels(newProject);
        REQUIRE(allAccessories.size() == 2);
        REQUIRE(allModels.size() == 2);
        {
            CHECK(allAccessories[0]->fileURI().lastPathComponent() == String("document_resave_effect.nanoem"));
            CHECK(URI::lastPathComponent(allAccessories[0]->fileURI().fragment()) == String("main.x"));
            CHECK(allAccessories[0]->activeEffect());
        }
        {
            CHECK(allAccessories[1]->fileURI().lastPathComponent() == String("document_resave_effect.nanoem"));
            CHECK(URI::lastPathComponent(allAccessories[1]->fileURI().fragment()) == String("offscreen.x"));
            CHECK(allAccessories[1]->activeEffect());
        }
        {
            CHECK(allModels[0]->fileURI().lastPathComponent() == String("document_resave_effect.nanoem"));
            CHECK(URI::lastPathComponent(allModels[0]->fileURI().fragment()) == String("main.pmx"));
            CHECK(allModels[0]->activeEffect());
        }
        {
            CHECK(allModels[1]->fileURI().lastPathComponent() == String("document_resave_effect.nanoem"));
            CHECK(URI::lastPathComponent(allModels[1]->fileURI().fragment()) == String("offscreen.pmx"));
            CHECK(allModels[1]->activeEffect());
        }
        wrapper->clear();
    }
}

TEST_CASE("document_import_archive_and_save", "[emapp][project][macos]")
{
    TestScope scope;
    Application *application = scope.application();
    {
        std::unique_ptr<DocumentWrapper> wrapper(
            new DocumentWrapper(scope, NANOEM_TEST_OUTPUT_PATH "/document_import_archive_and_save.nanoem"));
        wrapper->clear();
        Project *project = wrapper->currentProject();
        Error error;
        CHECK(application->fileManager()->loadFromFile(
            URI::createFromFilePath(NANOEM_TEST_FIXTURE_PATH "/test.zip", "test/foo/bar/baz/accessory/test.x"),
            IFileManager::kDialogTypeLoadModelFile, project, error));
        CHECK(application->fileManager()->loadFromFile(
            URI::createFromFilePath(NANOEM_TEST_FIXTURE_PATH "/test.zip", "test/foo/bar/baz/model/test.pmx"),
            IFileManager::kDialogTypeLoadModelFile, project, error));
        CHECK_FALSE(application->hasModalDialog());
        CHECK(wrapper->save(NANOEM_TEST_OUTPUT_PATH "/document_import_archive_and_save.nanoem"));
    }
    {
        std::unique_ptr<DocumentWrapper> wrapper(
            new DocumentWrapper(scope, NANOEM_TEST_OUTPUT_PATH "/document_import_archive_and_save.nanoem"));
        CHECK(wrapper->load());
        Project *newProject = wrapper->currentProject();
        const Project::AccessoryList *allAccessories = newProject->allAccessories();
        REQUIRE(allAccessories->size() == 1);
        const Project::ModelList *allModels = newProject->transformOrderList();
        REQUIRE(allModels->size() == 1);
        CHECK_FALSE(newProject->isDirty());
        CHECK(allAccessories->data()[0]->fileURI().lastPathComponent() ==
            String("document_import_archive_and_save.nanoem"));
        CHECK(URI::lastPathComponent(allAccessories->data()[0]->fileURI().fragment()) == String("test.x"));
        CHECK(allModels->data()[0]->fileURI().lastPathComponent() == String("document_import_archive_and_save.nanoem"));
        CHECK(URI::lastPathComponent(allModels->data()[0]->fileURI().fragment()) == String("test.pmx"));
        wrapper->clear();
    }
}
