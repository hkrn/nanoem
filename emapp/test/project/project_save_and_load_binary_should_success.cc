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
#include "emapp/ListUtils.h"
#include "emapp/Model.h"
#include "emapp/PerspectiveCamera.h"
#include "emapp/PhysicsEngine.h"
#include "emapp/ShadowCamera.h"

#include "bx/handlealloc.h"

using namespace nanoem;
using namespace test;

struct LoadBlock {
    using Callback = void(Project *);
    LoadBlock(TestScope &scope, ByteArray &bytes, Callback callback)
    {
        Error error;
        ProjectPtr ptr = scope.createProject();
        ptr->m_project->loadFromBinary(bytes, Project::kBinaryFormatNative, error, nullptr);
        callback(ptr->m_project);
    }
    ~LoadBlock()
    {
    }
};

struct SaveBlock {
    using Callback = void(Project *);
    SaveBlock(TestScope &scope, ByteArray &bytes, Callback callback)
        : m_bytes(bytes)
    {
        m_ptr = scope.createProject();
        callback(m_ptr->m_project);
    }
    ~SaveBlock()
    {
        Error error;
        m_ptr->m_project->saveAsBinary(m_bytes, Project::kBinaryFormatNative, error);
    }
    ProjectPtr m_ptr;
    ByteArray &m_bytes;
};

TEST_CASE("project_save_and_load_binary_should_success", "[emapp][project]")
{
    TestScope scope;
    Application *application = scope.application();
    BX_UNUSED_1(application);
    SECTION("accessory")
    {
        static const int kNumAccessories = 17;
        static const int kActiveAccessory = 11;
        ByteArray bytes;
        SaveBlock(scope, bytes, [](Project *project) {
            char name[128];
            for (int i = 0; i < kNumAccessories; i++) {
                Accessory *accessory = project->createAccessory();
                bx::snprintf(name, sizeof(name), "accessory_%d", i);
                accessory->setName(name);
                accessory->setFileURI(URI::createFromFilePath(NANOEM_TEST_FIXTURE_PATH "/test.x"));
                project->addAccessory(accessory);
                Motion::FrameIndexList indices;
                indices.push_back(i + 1);
                CommandRegistrator registrator(project);
                registrator.registerAddAccessoryKeyframesCommand(indices, accessory, project->resolveMotion(accessory));
                if (i == kActiveAccessory) {
                    project->setActiveAccessory(accessory);
                }
            }
        });
        LoadBlock(scope, bytes, [](Project *newProject) {
            const Project::AccessoryList &accessories = newProject->allAccessories();
            CHECK(accessories.size() == kNumAccessories);
            nanoem_rsize_t offset = 0;
            for (Project::AccessoryList::const_iterator it = accessories.begin(), end = accessories.end(); it != end;
                 it++) {
                const Accessory *accessory = *it;
                const Motion *motion = newProject->resolveMotion(accessory);
                const StringMap &annotations = accessory->annotations();
                CHECK(accessory->handle() == (offset * 2) + 4);
                CHECK(annotations.find("uuid") != annotations.end());
                CHECK(motion->findAccessoryKeyframe(offset + 1));
                offset++;
            }
            CHECK(newProject->activeAccessory());
            CHECK(newProject->allMotions().size() == kNumAccessories + 3);
        });
    }
    SECTION("model")
    {
        static const int kNumModels = 8;
        static const int kActiveModel = 3;
        ByteArray bytes;
        SaveBlock(scope, bytes, [](Project *project) {
            char name[128];
            for (int i = 0; i < kNumModels; i++) {
                Model *model = project->createModel();
                bx::snprintf(name, sizeof(name), "model_%d", i);
                model->setName(name);
                model->setFileURI(URI::createFromFilePath(NANOEM_TEST_FIXTURE_PATH "/test.pmx"));
                project->addModel(model);
                Motion::FrameIndexList indices;
                indices.push_back(i + 1);
                CommandRegistrator registrator(project);
                registrator.registerAddModelKeyframesCommand(indices, model, project->resolveMotion(model));
                if (i == kActiveModel) {
                    project->setActiveModel(model);
                    model->setTransformAxisType(Model::kAxisY);
                    model->setTransformCoordinateType(Model::kTransformCoordinateTypeGlobal);
                }
            }
        });
        LoadBlock(scope, bytes, [](Project *newProject) {
            const Project::ModelList &models = newProject->allModels();
            CHECK(models.size() == kNumModels);
            nanoem_frame_index_t offset = 0;
            for (Project::ModelList::const_iterator it = models.begin(), end = models.end(); it != end; it++) {
                const Model *model = *it;
                const Motion *motion = newProject->resolveMotion(model);
                const StringMap &annotations = model->annotations();
                CHECK(model->handle() == (offset * 2) + 4);
                CHECK(annotations.find("uuid") != annotations.end());
                CHECK(motion->findModelKeyframe(offset + 1));
                offset++;
            }
            CHECK(newProject->activeModel());
            CHECK(newProject->activeModel()->transformAxisType() == Model::kAxisY);
            CHECK(newProject->activeModel()->transformCoordinateType() == Model::kTransformCoordinateTypeGlobal);
            CHECK(newProject->allMotions().size() == kNumModels + 3);
        });
    }
    SECTION("camera")
    {
        ByteArray bytes;
        SaveBlock(scope, bytes, [](Project *project) {
            project->activeCamera()->setAngle(Vector3(0.2, 0.4, 0.6));
            project->activeCamera()->setLookAt(Vector3(42, 84, 126));
            project->activeCamera()->setDistance(576);
            project->activeCamera()->setFov(114);
            project->activeCamera()->setPerspective(false);
            Motion::FrameIndexList indices;
            indices.push_back(42);
            CommandRegistrator registrator(project);
            registrator.registerAddCameraKeyframesCommand(indices, project->globalCamera(), project->cameraMotion());
            Error error;
        });
        LoadBlock(scope, bytes, [](Project *newProject) {
            CHECK_THAT(newProject->activeCamera()->angle(), Equals(Vector3(0.2, 0.4, 0.6)));
            CHECK_THAT(newProject->activeCamera()->lookAt(), Equals(Vector3(42, 84, 126)));
            CHECK(newProject->activeCamera()->distance() == 576);
            CHECK(newProject->activeCamera()->fov() == 114);
            CHECK_FALSE(newProject->activeCamera()->isPerspective());
            CHECK(newProject->cameraMotion()->findCameraKeyframe(42));
        });
    }
    SECTION("light")
    {
        ByteArray bytes;
        SaveBlock(scope, bytes, [](Project *project) {
            project->activeLight()->setColor(Vector3(0.3, 0.5, 0.7));
            project->activeLight()->setDirection(Vector3(-0.3, 0.9, 0.4));
            project->setGroundShadowEnabled(false);
            project->setShadowMapSize(Vector2(512, 512));
            project->setShadowMapEnabled(true);
            project->shadowCamera()->setCoverageMode(ShadowCamera::kCoverageModeType2);
            Motion::FrameIndexList indices;
            indices.push_back(42);
            CommandRegistrator registrator(project);
            registrator.registerAddLightKeyframesCommand(indices, project->globalLight(), project->lightMotion());
            registrator.registerAddSelfShadowKeyframesCommand(
                indices, project->shadowCamera(), project->selfShadowMotion());
        });
        LoadBlock(scope, bytes, [](Project *newProject) {
            CHECK_THAT(newProject->activeLight()->color(), Equals(Vector3(0.3, 0.5, 0.7)));
            CHECK_THAT(newProject->activeLight()->direction(), Equals(Vector3(-0.3, 0.9, 0.4)));
            CHECK_FALSE(newProject->isGroundShadowEnabled());
            CHECK_THAT(newProject->shadowMapSize(), Equals(Vector2(512, 512)));
            CHECK(newProject->isShadowMapEnabled());
            CHECK(newProject->shadowCamera()->coverageMode() == 2);
            CHECK(newProject->lightMotion()->findLightKeyframe(42));
            CHECK(newProject->selfShadowMotion()->findSelfShadowKeyframe(42));
        });
    }
    SECTION("grid")
    {
        ByteArray bytes;
        SaveBlock(scope, bytes, [](Project *project) {
            project->grid()->setCell(Vector2(42, 84));
            project->grid()->setOpacity(0.6f);
            project->grid()->setVisible(false);
        });
        LoadBlock(scope, bytes, [](Project *newProject) {
            CHECK(newProject->grid()->opacity() == 0.6f);
            CHECK_THAT(newProject->grid()->cell(), Equals(Vector2(42, 84)));
            CHECK_FALSE(newProject->grid()->isVisible());
        });
    }
    SECTION("physics")
    {
        ByteArray bytes;
        SaveBlock(scope, bytes, [](Project *project) {
            project->setPhysicsSimulationMode(PhysicsEngine::kSimulationModeEnableTracing);
            project->setPhysicsSimulationEngineDebugFlags(1 | 2 | 4 | 16 | 64);
            project->physicsEngine()->setAcceleration(0.42f);
            project->physicsEngine()->setDirection(glm::normalize(Vector3(0.1, 0.2, 0.3)));
            project->physicsEngine()->setNoise(0.6);
            project->physicsEngine()->setNoiseEnabled(true);
            project->physicsEngine()->setGroundEnabled(false);
        });
        LoadBlock(scope, bytes, [](Project *newProject) {
            CHECK(newProject->physicsEngine()->mode() == PhysicsEngine::kSimulationModeEnableTracing);
            CHECK(newProject->physicsEngine()->debugGeometryFlags() == (1 | 2 | 4 | 16 | 64));
            CHECK(newProject->physicsEngine()->acceleration() == 0.42f);
            CHECK_THAT(newProject->physicsEngine()->direction(), Equals(glm::normalize(Vector3(0.1, 0.2, 0.3))));
            CHECK(newProject->physicsEngine()->noise() == 0.6f);
            CHECK(newProject->physicsEngine()->isNoiseEnabled());
            CHECK_FALSE(newProject->physicsEngine()->isGroundEnabled());
        });
    }
    SECTION("screen")
    {
        ByteArray bytes;
        SaveBlock(scope, bytes, [](Project *project) {
            project->setViewportImageSize(Vector2(1920, 1080));
            project->setViewportBackgroundColor(Vector4(0));
            project->setSampleLevel(2);
            CHECK(project->resetAllPasses());
        });
        LoadBlock(scope, bytes, [](Project *newProject) {
            CHECK(newProject->resetAllPasses());
            CHECK_THAT(newProject->viewportImageSize(), Equals(Vector2(1920, 1080)));
            CHECK_THAT(newProject->viewportBackgroundColor(), Equals(Vector4(0)));
            CHECK(newProject->sampleLevel() == 2);
            CHECK(newProject->sampleCount() == 4);
        });
    }
    SECTION("timeline")
    {
        ByteArray bytes;
        static TimelineSegment expectedSegment;
        expectedSegment.m_enableFrom = expectedSegment.m_enableTo = true;
        expectedSegment.m_from = 384;
        expectedSegment.m_to = 1536;
        SaveBlock(scope, bytes, [](Project *project) {
            project->setBaseDuration(2560);
            project->seek(864, true);
            project->setPreferredMotionFPS(60, false);
            project->setLoopEnabled(true);
            project->setPlayingSegment(expectedSegment);
        });
        LoadBlock(scope, bytes, [](Project *newProject) {
            CHECK(newProject->baseDuration() == 2560);
            CHECK(newProject->currentLocalFrameIndex() == 864);
            CHECK(newProject->preferredMotionFPS() == 60);
            CHECK(newProject->isLoopEnabled());
            const TimelineSegment &actualSegment = newProject->playingSegment();
            CHECK(actualSegment.m_enableFrom);
            CHECK(actualSegment.m_enableTo);
            CHECK(actualSegment.m_from == expectedSegment.m_from);
            CHECK(actualSegment.m_to == expectedSegment.m_to);
        });
    }
    SECTION("confirmation")
    {
        ByteArray bytes;
        SaveBlock(scope, bytes, [](Project *project) {
            project->setConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE, true);
            project->setConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA, true);
            project->setConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT, true);
            project->setConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL, true);
            project->setConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH, true);
        });
        LoadBlock(scope, bytes, [](Project *newProject) {
            CHECK(newProject->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE));
            CHECK(newProject->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA));
            CHECK(newProject->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT));
            CHECK(newProject->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL));
            CHECK(newProject->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH));
        });
    }
    SECTION("audio")
    {
        ByteArray bytes;
        SaveBlock(scope, bytes, [](Project *project) { project->audioPlayer()->setVolumeGain(0.4f); });
        LoadBlock(scope, bytes, [](Project *newProject) { CHECK(newProject->audioPlayer()->volumeGain() == 0.4f); });
    }
    SECTION("video")
    {
        ByteArray bytes;
        SaveBlock(scope, bytes, [](Project *project) { BX_UNUSED_1(project); });
        LoadBlock(scope, bytes, [](Project *newProject) { BX_UNUSED_1(newProject); });
    }
    SECTION("global")
    {
        ByteArray bytes;
        SaveBlock(scope, bytes, [](Project *project) {
            project->setDrawType(IDrawable::kDrawTypeColor);
            project->setEditingMode(Project::kEditingModeRotate);
            project->setEffectPluginEnabled(true);
            project->setMotionMergeEnabled(true);
            project->setMultipleBoneSelectionEnabled(true);
            project->setLanguage(ITranslator::kLanguageTypeJapanese);
        });
        LoadBlock(scope, bytes, [](Project *newProject) {
            CHECK(newProject->drawType() == IDrawable::kDrawTypeColor);
            CHECK(newProject->editingMode() == Project::kEditingModeRotate);
            CHECK(newProject->isEffectPluginEnabled());
            CHECK(newProject->isMotionMergeEnabled());
            CHECK(newProject->isMultipleBoneSelectionEnabled());
            CHECK(newProject->language() == ITranslator::kLanguageTypeJapanese);
            CHECK(newProject->castLanguage() == NANOEM_LANGUAGE_TYPE_JAPANESE);
            CHECK(newProject->coordinationSystem() == GLM_LEFT_HANDED);
        });
    }
    SECTION("delete_accessory")
    {
        ProjectPtr ptr = scope.createProject();
        Project *project = ptr->m_project;
        ByteArray bytes;
        Error error;
        char name[128];
        for (int i = 0; i < 3; i++) {
            Accessory *accessory = project->createAccessory();
            bx::snprintf(name, sizeof(name), "accessory_%d", i);
            accessory->setName(name);
            accessory->setFileURI(URI::createFromFilePath(NANOEM_TEST_FIXTURE_PATH "/test.x"));
            project->addAccessory(accessory);
        }
        Accessory *removingTarget = project->allAccessories()[1];
        Motion *removingMotion = project->resolveMotion(removingTarget);
        project->removeAccessory(removingTarget);
        project->destroyAccessory(removingTarget);
        Project::MotionList motions(project->allMotions());
        CHECK(motions.size() == 5); /* models (2) + camera + light + SS */
        CHECK(ListUtils::indexOf(removingMotion, motions) == -1);
        CHECK(project->saveAsBinary(bytes, Project::kBinaryFormatNative, error));
    }
    SECTION("delete_model")
    {
        ProjectPtr ptr = scope.createProject();
        Project *project = ptr->m_project;
        ByteArray bytes;
        Error error;
        char name[128];
        for (int i = 0; i < 3; i++) {
            Model *model = project->createModel();
            bx::snprintf(name, sizeof(name), "model_%d", i);
            model->setName(name);
            model->setFileURI(URI::createFromFilePath(NANOEM_TEST_FIXTURE_PATH "/test.pmx"));
            project->addModel(model);
        }
        Model *removingTarget = project->allModels()[1];
        Motion *removingMotion = project->resolveMotion(removingTarget);
        project->removeModel(removingTarget);
        project->destroyModel(removingTarget);
        Project::MotionList motions(project->allMotions());
        CHECK(motions.size() == 5); /* models (2) + camera + light + SS */
        CHECK(ListUtils::indexOf(removingMotion, motions) == -1);
        CHECK(project->saveAsBinary(bytes, Project::kBinaryFormatNative, error));
    }
}
