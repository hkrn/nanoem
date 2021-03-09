/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "./project.h"

#include "emapp/Accessory.h"
#include "emapp/DirectionalLight.h"
#include "emapp/FileUtils.h"
#include "emapp/Grid.h"
#include "emapp/IAudioPlayer.h"
#include "emapp/IFileManager.h"
#include "emapp/Model.h"
#include "emapp/PerspectiveCamera.h"
#include "emapp/ShadowCamera.h"

using namespace nanoem;
using namespace test;

TEST_CASE("project_save_and_load_archive_should_same_content", "[emapp][project]")
{
    const URI &fileURI = URI::createFromFilePath(NANOEM_TEST_OUTPUT_PATH "/project_save_and_load_archive.nma");
    TestScope scope;
    Application *application = scope.application();
    BX_UNUSED_1(application);
    ByteArray bytes;
    MemoryWriter writer(&bytes);
    Error error;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first.get()->m_project;
        for (int i = 0; i < 3; i++) {
            Accessory *accessory = first->createAccessory();
            project->addAccessory(accessory);
        }
        for (int i = 0; i < 3; i++) {
            Model *model = first->createModel();
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
        // project->setShadowMapSize(Vector2(512, 512));
        // project->setShadowMapEnabled(true);
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
        project->setFileURI(fileURI);
        REQUIRE(project->resetAllPasses());
        CHECK(project->saveAsArchive(&writer, error));
        CHECK_FALSE(error.hasReason());
    }
    {
        ProjectPtr second = scope.createProject();
        Project *newProject = second.get()->m_project;
        {
            MemoryReader reader(&bytes);
            CHECK(newProject->loadFromArchive(&reader, fileURI, error));
            CHECK(newProject->resetAllPasses());
            CHECK_FALSE(error.hasReason());
        }
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
        // CHECK(newProject->shadowMapSize() == Vector2UI16(512 == 512));
        // CHECK(newProject->isShadowMapEnabled());
        CHECK(newProject->shadowCamera()->coverageMode() == 2);
        CHECK(newProject->physicsEngine()->mode() == PhysicsEngine::kSimulationModeEnableTracing);
        CHECK(newProject->physicsEngine()->debugGeometryFlags() == (1 | 2 | 4 | 16 | 64));
        CHECK_THAT(newProject->viewportBackgroundColor(), Equals(Vector4(0)));
        CHECK(newProject->sampleLevel() == 2);
        CHECK(newProject->sampleCount() == 4);
        CHECK(newProject->baseDuration() == 2560);
        CHECK(newProject->currentLocalFrameIndex() == 864);
        CHECK(newProject->preferredMotionFPS() == 60);
        CHECK(newProject->isLoopEnabled());
        CHECK(newProject->allAccessories().size() == 3);
        CHECK(newProject->allModels().size() == 3);
        CHECK(newProject->drawableOrderList().size() == 6);
        CHECK(newProject->transformOrderList().size() == 3);
        CHECK_THAT(newProject->activeCamera()->angle(), Equals(Vector3(0.2, 0.4, 0.6)));
        CHECK_THAT(newProject->activeCamera()->lookAt(), Equals(Vector3(42, 84, 126)));
        CHECK(newProject->activeCamera()->distance() == 576);
        CHECK(newProject->activeCamera()->fov() == 114);
        CHECK_FALSE(newProject->activeCamera()->isPerspective());
        CHECK_THAT(newProject->activeLight()->color(), Equals(Vector3(0.3, 0.5, 0.7)));
        CHECK_THAT(newProject->activeLight()->direction(), Equals(Vector3(-0.3, 0.9, 0.4)));
        CHECK_FALSE(newProject->isDirty());
        CHECK(newProject->coordinationSystem() == GLM_LEFT_HANDED);
        const Project::AccessoryList &allAccessories = second->allAccessories();
        const Project::ModelList &allModels = second->allModels();
        REQUIRE(allAccessories.size() == 3);
        REQUIRE(allModels.size() == 3);
        CHECK(allAccessories[0]->fileURI().fragment() == String("Accessory/test/test.x"));
        CHECK(allAccessories[1]->fileURI().fragment() == String("Accessory/test-1/test.x"));
        CHECK(allAccessories[2]->fileURI().fragment() == String("Accessory/test-2/test.x"));
        CHECK(allModels[0]->fileURI().fragment() == modelPath(0, "test.pmx"));
        CHECK(allModels[1]->fileURI().fragment() == modelPath(1, "test.pmx"));
        CHECK(allModels[2]->fileURI().fragment() == modelPath(2, "test.pmx"));
        {
            MemoryReader reader(&bytes);
            Archiver archiver(&reader);
            Error error;
            REQUIRE(archiver.open(error));
            expectAllProjectResourcesSame(archiver, newProject);
            REQUIRE(archiver.close(error));
        }
    }
}
