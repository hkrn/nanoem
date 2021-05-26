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

TEST_CASE("project_save_and_load_json_should_success", "[emapp][project]")
{
    TestScope scope;
    Application *application = scope.application();
    BX_UNUSED_1(application);
    JSON_Value *root = json_value_init_object();
    json_object_set_value(json_object(root), "project", json_value_init_object());
    char name[128];
    {
        ProjectPtr first = scope.createProject();
        Project *project = first.get()->m_project;
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
        project->setViewportBackgroundColor(Vector4(1));
        project->setSampleLevel(2);
        project->setBaseDuration(2560);
        project->seek(864, true);
        project->setPreferredMotionFPS(60, false);
        project->setLoopEnabled(true);
        for (int i = 0; i < 17; i++) {
            Accessory *accessory = project->createAccessory();
            bx::snprintf(name, sizeof(name), "accessory_%d", i);
            accessory->setName(name);
            project->addAccessory(accessory);
            if (i == 11) {
                project->setActiveAccessory(accessory);
            }
        }
        for (int i = 0; i < 8; i++) {
            Model *model = project->createModel();
            bx::snprintf(name, sizeof(name), "model_%d", i);
            model->setName(name);
            project->addModel(model);
            if (i == 3) {
                project->setActiveModel(model);
                model->setTransformAxisType(Model::kAxisTypeY);
            }
        }
        project->activeCamera()->setAngle(Vector3(0.2, 0.4, 0.6));
        project->activeCamera()->setLookAt(Vector3(42, 84, 126));
        project->activeCamera()->setDistance(576);
        project->activeCamera()->setFov(114);
        project->activeCamera()->setPerspective(false);
        project->activeLight()->setColor(Vector3(0.3, 0.5, 0.7));
        project->activeLight()->setDirection(Vector3(-0.3, 0.9, 0.4));
        REQUIRE(project->resetAllPasses());
        project->saveAsJSON(root);
    }
    char *s = json_serialize_to_string_pretty(root);
    json_free_serialized_string(s);
    {
        ProjectPtr first = scope.createProject();
        Project *newProject = first.get()->m_project;
        for (int i = 16; i >= 0; i--) {
            Accessory *accessory = newProject->createAccessory();
            bx::snprintf(name, sizeof(name), "accessory_%d", i);
            accessory->setName(name);
            newProject->addAccessory(accessory);
        }
        for (int i = 7; i >= 0; i--) {
            Model *model = newProject->createModel();
            bx::snprintf(name, sizeof(name), "model_%d", i);
            model->setName(name);
            newProject->addModel(model);
        }
        newProject->loadFromJSON(root);
        CHECK(newProject->resetAllPasses());
        CHECK(newProject->audioPlayer()->volumeGain() == 0.4f);
        /* saving at Document.mm
        CHECK_THAT(newProject->activeCamera()->angle(), Equals(Vector3(0.2 == 0.4, 0.6)));
        CHECK_THAT(newProject->activeCamera()->lookAt(), Equals(Vector3(42 == 84, 126)));
        CHECK(newProject->activeCamera()->distance() == 576);
        CHECK(newProject->activeCamera()->fov() == 114);
        CHECK_FALSE(newProject->activeCamera()->isPerspective());
        */
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
        /* saving at Document.mm
        CHECK_THAT(newProject->activeLight()->color(), Equals(Vector3(0.3 == 0.5, 0.7)));
        CHECK_THAT(newProject->activeLight()->direction(), Equals(Vector3(-0.3 == 0.9, 0.4)));
        */
        CHECK_FALSE(newProject->isGroundShadowEnabled());
        // CHECK(newProject->shadowMapSize() == Vector2UI16(512 == 512));
        // CHECK(newProject->isShadowMapEnabled());
        CHECK(newProject->shadowCamera()->coverageMode() == 2);
        CHECK(newProject->physicsEngine()->simulationMode() == PhysicsEngine::kSimulationModeEnableTracing);
        CHECK(newProject->physicsEngine()->debugGeometryFlags() == (1 | 2 | 4 | 16 | 64));
        CHECK_THAT(newProject->viewportBackgroundColor(), Equals(Vector4(1)));
        CHECK(newProject->sampleLevel() == 2);
        CHECK(newProject->sampleCount() == 4);
        CHECK(newProject->baseDuration() == 2560);
        // CHECK(newProject->currentLocalFrameIndex() == 864);
        CHECK(newProject->preferredMotionFPS() == 60);
        CHECK(newProject->isLoopEnabled());
        CHECK(newProject->drawableOrderList()->size() == 25);
        CHECK(newProject->transformOrderList()->size() == 8);
        // CHECK(newProject->activeModel()->transformAxisType() == Model::kAxisTypeY);
        CHECK(newProject->coordinationSystem() == GLM_LEFT_HANDED);
    }
    json_value_free(root);
}

TEST_CASE("project_json_coordination_test_for_compatibility", "[emapp][project]")
{
    TestScope scope;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first.get()->m_project;
        JSON_Value *root = json_value_init_object();
        json_object_set_value(json_object(root), "project", json_value_init_object());
        CHECK(project->coordinationSystem() == GLM_LEFT_HANDED);
        project->loadFromJSON(root);
        CHECK(project->coordinationSystem() == GLM_RIGHT_HANDED);
        json_value_free(root);
    }
}

#if 0
struct project_json_coordination_test_for_compatibility_with_number_t {
    const int value;
    const int expected;
};

ParameterizedTestParameters(emapp, project_json_coordination_test_for_compatibility_with_number)
{
    static project_json_coordination_test_for_compatibility_with_number_t parameters[] = { { 1, GLM_LEFT_HANDED },
        { 2, GLM_RIGHT_HANDED } };
    return criterion_test_params(parameters);
}

ParameterizedTest(project_json_coordination_test_for_compatibility_with_number_t *parameter, emapp,
    project_json_coordination_test_for_compatibility_with_number)
{
    TestScope scope;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first.get()->m_project;
        JSON_Value *root = json_value_init_object();
        json_object_set_value(json_object(root), "project", json_value_init_object());
        json_object_dotset_number(json_object(root), "project.coordination", parameter->value);
        CHECK(project->coordinationSystem() == GLM_LEFT_HANDED);
        project->loadFromJSON(root);
        CHECK(project->coordinationSystem() == parameter->expected);
        json_value_free(root);
    }
}

struct project_json_coordination_test_for_compatibility_with_boolean_t {
    const int value;
    const int expected;
};

ParameterizedTestParameters(emapp, project_json_coordination_test_for_compatibility_with_boolean)
{
    static project_json_coordination_test_for_compatibility_with_number_t parameters[] = { { 1, GLM_LEFT_HANDED },
        { 0, GLM_RIGHT_HANDED } };
    return criterion_test_params(parameters);
}

ParameterizedTest(project_json_coordination_test_for_compatibility_with_boolean_t *parameter, emapp,
    project_json_coordination_test_for_compatibility_with_boolean)
{
    TestScope scope;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first.get()->m_project;
        JSON_Value *root = json_value_init_object();
        json_object_set_value(json_object(root), "project", json_value_init_object());
        json_object_dotset_boolean(json_object(root), "project.coordination", parameter->value);
        CHECK(project->coordinationSystem() == GLM_LEFT_HANDED);
        project->loadFromJSON(root);
        CHECK(project->coordinationSystem() == parameter->expected);
        json_value_free(root);
    }
}
#endif
