/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "./project.h"

#include "emapp/Accessory.h"
#include "emapp/CommandRegistrator.h"
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

TEST_CASE("project_resave_archive_should_same_content", "[emapp][project]")
{
    TestScope scope;
    Application *application = scope.application();
    BX_UNUSED_1(application);
    ByteArray bytes;
    MemoryWriter writer(&bytes);
    Error error;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first.get()->m_project;
        CommandRegistrator registrator(project);
        for (int i = 0; i < 3; i++) {
            Accessory *accessory = first->createAccessory();
            project->addAccessory(accessory);
            accessory->setTranslation(Vector3(0.1, 0.2, 0.3));
            accessory->setOrientation(Vector3(0.4, 0.5, 0.6));
            accessory->setScaleFactor(0.7f);
            accessory->setOpacity(0.8f);
            registrator.registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(accessory);
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
        {
            const URI &fileURI = URI::createFromFilePath(NANOEM_TEST_OUTPUT_PATH "/project_resave_archive.nma");
            ByteArray transientBytes;
            MemoryWriter transientWriter(&transientBytes);
            project->setFileURI(fileURI);
            REQUIRE(project->resetAllPasses());
            REQUIRE(project->saveAsArchive(&transientWriter, error));
            REQUIRE_FALSE(error.hasReason());
            {
                FILE *fp = fopen(fileURI.absolutePathConstString(), "wb");
                fwrite(transientBytes.data(), transientBytes.size(), 1, fp);
                fclose(fp);
            }
            ProjectPtr second = scope.createProject();
            Project *transientProject = second.get()->m_project;
            MemoryReader transientReader(&transientBytes);
            REQUIRE(transientProject->loadFromArchive(&transientReader, fileURI, error));
            REQUIRE_FALSE(error.hasReason());
            transientProject->setFileURI(fileURI);
            REQUIRE(transientProject->resetAllPasses());
            REQUIRE(transientProject->saveAsArchive(&writer, error));
            REQUIRE_FALSE(error.hasReason());
        }
    }
    {
        ProjectPtr first = scope.createProject();
        Project *newProject = first.get()->m_project;
        {
            MemoryReader reader(&bytes);
            const URI &fileURI = URI::createFromFilePath(NANOEM_TEST_OUTPUT_PATH "/project_resave_archive.nma");
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
        // CHECK(newProject->shadowMapSize() == Vector2UI16(512, 512));
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
        CHECK(newProject->allAccessories().size() == 3);
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
        const Project::AccessoryList &allAccessories = first->allAccessories();
        const Project::ModelList &allModels = first->allModels();
        REQUIRE(allAccessories.size() == 3);
        REQUIRE(allModels.size() == 3);
        CHECK(allAccessories[0]->fileURI().fragment() == String("Accessory/test/test.x"));
        CHECK(allAccessories[1]->fileURI().fragment() == String("Accessory/test-1/test.x"));
        CHECK(allAccessories[2]->fileURI().fragment() == String("Accessory/test-2/test.x"));
        CHECK_THAT(allAccessories[0]->translation(), Equals(Vector3(0.1, 0.2, 0.3)));
        CHECK_THAT(allAccessories[1]->translation(), Equals(Vector3(0.1, 0.2, 0.3)));
        CHECK_THAT(allAccessories[2]->translation(), Equals(Vector3(0.1, 0.2, 0.3)));
        CHECK(
            glm::all(glm::epsilonEqual(allAccessories[0]->orientation(), Vector3(0.4, 0.5, 0.6), Constants::kEpsilon)));
        CHECK(
            glm::all(glm::epsilonEqual(allAccessories[1]->orientation(), Vector3(0.4, 0.5, 0.6), Constants::kEpsilon)));
        CHECK(
            glm::all(glm::epsilonEqual(allAccessories[2]->orientation(), Vector3(0.4, 0.5, 0.6), Constants::kEpsilon)));
        CHECK(allAccessories[0]->scaleFactor() == Approx(0.7f));
        CHECK(allAccessories[1]->scaleFactor() == Approx(0.7f));
        CHECK(allAccessories[2]->scaleFactor() == Approx(0.7f));
        CHECK(allAccessories[0]->opacity() == Approx(0.8f));
        CHECK(allAccessories[1]->opacity() == Approx(0.8f));
        CHECK(allAccessories[2]->opacity() == Approx(0.8f));
        CHECK(allAccessories[0]->isVisible());
        CHECK(allAccessories[1]->isVisible());
        CHECK(allAccessories[2]->isVisible());
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
        scope.deleteFile(NANOEM_TEST_OUTPUT_PATH "/project_resave_archive.nma");
    }
}
