/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/emapp.h"

#include "undo/undo.h"

using namespace nanoem;
using namespace test;

struct EqualsVector3 : Catch::Matchers::Impl::MatcherBase<Vector3> {
    EqualsVector3(nanoem_f32_t x, nanoem_f32_t y, nanoem_f32_t z)
        : m_data(x, y, z)
    {
    }
    EqualsVector3(const EqualsVector3 &v)
        : m_data(v.m_data)
    {
    }
    bool
    match(const Vector3 &value) const override
    {
        return value.x == Approx(m_data.x) && value.y == Approx(m_data.y) && value.z == Approx(m_data.z);
    }
    std::string
    describe() const override
    {
        return "equals " + glm::to_string(m_data);
    }
    Vector3 m_data;
};

struct EqualsQuaternion : Catch::Matchers::Impl::MatcherBase<Quaternion> {
    EqualsQuaternion(nanoem_f32_t x, nanoem_f32_t y, nanoem_f32_t z)
        : m_data(glm::radians(Vector3(x, y, z)))
    {
    }
    EqualsQuaternion(const EqualsQuaternion &v)
        : m_data(v.m_data)
    {
    }
    bool
    match(const Quaternion &value) const override
    {
        return value.x == Approx(m_data.x) && value.y == Approx(m_data.y) && value.z == Approx(m_data.z) &&
            value.w == Approx(m_data.w);
    }
    std::string
    describe() const override
    {
        return "equals " + glm::to_string(m_data);
    }
    Quaternion m_data;
};

static int kKeyframeIndexRegistrationOrder[] = { 1335, 1337, 1339, 1341, 1343 };
static int kKeyframeIndexShouldChanged[] = { 1339, 1341 };
static int kKeyframeIndexShouldNotChanged[] = { 1335, 1337, 1343 };
static int kFrameIndexDeltaPlus = 10;
static int kFrameIndexDeltaMinus = -kFrameIndexDeltaPlus;
static int kKeyframeIndexShouldNotChangedForZero[] = { 0, kFrameIndexDeltaPlus };

TEST_CASE("project_move_all_keyframes_command_plus", "[emapp][project]")
{
    Error error;
    TestScope scope;
    Application *application = scope.application();
    BX_UNUSED_1(application);
    SECTION("accessory")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Accessory *activeAccessory = first->createAccessory();
        CommandRegistrator registrator(project);
        project->addAccessory(activeAccessory);
        project->setActiveAccessory(activeAccessory);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            activeAccessory->setTranslation(Vector3(1, 2, 3));
            activeAccessory->setOrientation(glm::radians(Vector3(15, 30, 45)));
            activeAccessory->setOpacity(0.4f);
            activeAccessory->setScaleFactor(5.0f);
            registrator.registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(activeAccessory);
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeAccessory);
        motion->selection()->addAccessoryKeyframes(1339, 1341);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        for (auto it : kKeyframeIndexShouldChanged) {
            const nanoem_motion_accessory_keyframe_t *k = motion->findAccessoryKeyframe(it + kFrameIndexDeltaPlus);
            CHECK_THAT(glm::make_vec3(nanoemMotionAccessoryKeyframeGetTranslation(k)), EqualsVector3(1, 2, 3));
            CHECK_THAT(glm::make_quat(nanoemMotionAccessoryKeyframeGetOrientation(k)), EqualsQuaternion(15, 30, 45));
            CHECK(nanoemMotionAccessoryKeyframeGetOpacity(k) == Approx(0.4f));
            CHECK(nanoemMotionAccessoryKeyframeGetScaleFactor(k) == Approx(5));
            CHECK_FALSE(motion->findAccessoryKeyframe(it));
        }
        for (auto it : kKeyframeIndexShouldNotChanged) {
            const nanoem_motion_accessory_keyframe_t *k = motion->findAccessoryKeyframe(it);
            CHECK_THAT(glm::make_vec3(nanoemMotionAccessoryKeyframeGetTranslation(k)), EqualsVector3(1, 2, 3));
            CHECK_THAT(glm::make_quat(nanoemMotionAccessoryKeyframeGetOrientation(k)), EqualsQuaternion(15, 30, 45));
            CHECK(nanoemMotionAccessoryKeyframeGetOpacity(k) == Approx(0.4f));
            CHECK(nanoemMotionAccessoryKeyframeGetScaleFactor(k) == Approx(5));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("bone")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        const nanoem_model_bone_t *activeBonePtr = activeModel->activeBone();
        model::Bone *activeBone = model::Bone::cast(activeBonePtr);
        CommandRegistrator registrator(project);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            activeBone->setLocalUserTranslation(Vector3(1, 2, 3));
            activeBone->setLocalUserOrientation(glm::radians(Vector3(45, 30, 60)));
            activeModel->performAllBonesTransform();
            registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeModel);
        motion->selection()->addBoneKeyframes(activeBonePtr, 1339, 1341);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        const nanoem_unicode_string_t *name = nanoemModelBoneGetName(activeBonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        for (auto it : kKeyframeIndexShouldChanged) {
            const nanoem_motion_bone_keyframe_t *k = motion->findBoneKeyframe(name, it + kFrameIndexDeltaPlus);
            CHECK_THAT(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(k)), EqualsVector3(1, 2, 3));
            CHECK_THAT(glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(k)), EqualsQuaternion(45, 30, 60));
            CHECK_FALSE(motion->findBoneKeyframe(name, it));
        }
        for (auto it : kKeyframeIndexShouldNotChanged) {
            const nanoem_motion_bone_keyframe_t *k = motion->findBoneKeyframe(name, it);
            CHECK_THAT(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(k)), EqualsVector3(1, 2, 3));
            CHECK_THAT(glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(k)), EqualsQuaternion(45, 30, 60));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("camera")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ICamera *camera = project->globalCamera();
        CommandRegistrator registrator(project);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            camera->setAngle(Vector3(0.1f, 0.2f, 0.3f));
            camera->setLookAt(Vector3(1, 2, 3));
            camera->setDistance(42);
            camera->setFov(21);
            registrator.registerAddCameraKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->cameraMotion();
        motion->selection()->addCameraKeyframes(1339, 1341);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        for (auto it : kKeyframeIndexShouldChanged) {
            const nanoem_motion_camera_keyframe_t *k = motion->findCameraKeyframe(it + kFrameIndexDeltaPlus);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(k)), EqualsVector3(0.1f, 0.2f, 0.3f));
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(k)), EqualsVector3(1, 2, 3));
            CHECK(nanoemMotionCameraKeyframeGetDistance(k) == Approx(-42));
            CHECK(nanoemMotionCameraKeyframeGetFov(k) == 21);
            CHECK_FALSE(motion->findCameraKeyframe(it));
        }
        for (auto it : kKeyframeIndexShouldNotChanged) {
            const nanoem_motion_camera_keyframe_t *k = motion->findCameraKeyframe(it);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(k)), EqualsVector3(0.1f, 0.2f, 0.3f));
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(k)), EqualsVector3(1, 2, 3));
            CHECK(nanoemMotionCameraKeyframeGetDistance(k) == Approx(-42));
            CHECK(nanoemMotionCameraKeyframeGetFov(k) == 21);
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("light")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ILight *light = project->globalLight();
        CommandRegistrator registrator(project);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            light->setColor(Vector3(0.1f, 0.2f, 0.3f));
            light->setDirection(Vector3(0.4f, 0.5f, 0.6f));
            registrator.registerAddLightKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->lightMotion();
        motion->selection()->addLightKeyframes(1339, 1341);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        for (auto it : kKeyframeIndexShouldChanged) {
            const nanoem_motion_light_keyframe_t *k = motion->findLightKeyframe(it + kFrameIndexDeltaPlus);
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetColor(k)), EqualsVector3(0.1f, 0.2f, 0.3f));
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetDirection(k)), EqualsVector3(0.4f, 0.5f, 0.6f));
            CHECK_FALSE(motion->findLightKeyframe(it));
        }
        for (auto it : kKeyframeIndexShouldNotChanged) {
            const nanoem_motion_light_keyframe_t *k = motion->findLightKeyframe(it);
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetColor(k)), EqualsVector3(0.1f, 0.2f, 0.3f));
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetDirection(k)), EqualsVector3(0.4f, 0.5f, 0.6f));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("model")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        CommandRegistrator registrator(project);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            activeModel->setVisible(false);
            registrator.registerAddModelKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeModel);
        motion->selection()->addModelKeyframes(1339, 1341);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        for (auto it : kKeyframeIndexShouldChanged) {
            const nanoem_motion_model_keyframe_t *k = motion->findModelKeyframe(it + kFrameIndexDeltaPlus);
            CHECK_FALSE(nanoemMotionModelKeyframeIsVisible(k));
            CHECK_FALSE(motion->findModelKeyframe(it));
        }
        for (auto it : kKeyframeIndexShouldNotChanged) {
            const nanoem_motion_model_keyframe_t *k = motion->findModelKeyframe(it);
            CHECK_FALSE(nanoemMotionModelKeyframeIsVisible(k));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("morph")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        activeModel->setActiveMorph(scope.findRandomMorph(activeModel));
        const nanoem_model_morph_t *activeMorphPtr = activeModel->activeMorph();
        model::Morph *activeMorph = model::Morph::cast(activeMorphPtr);
        CommandRegistrator registrator(project);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            activeMorph->setWeight(0.42f);
            activeModel->performAllBonesTransform();
            registrator.registerAddMorphKeyframesCommandByActiveMorph(activeModel, NANOEM_MODEL_MORPH_CATEGORY_BASE);
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeModel);
        motion->selection()->addMorphKeyframes(activeMorphPtr, 1339, 1341);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        const nanoem_unicode_string_t *name = nanoemModelMorphGetName(activeMorphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        for (auto it : kKeyframeIndexShouldChanged) {
            const nanoem_motion_morph_keyframe_t *k = motion->findMorphKeyframe(name, it + kFrameIndexDeltaPlus);
            CHECK(nanoemMotionMorphKeyframeGetWeight(k) == Approx(0.42f));
            CHECK_FALSE(motion->findMorphKeyframe(name, it));
        }
        for (auto it : kKeyframeIndexShouldNotChanged) {
            const nanoem_motion_morph_keyframe_t *k = motion->findMorphKeyframe(name, it);
            CHECK(nanoemMotionMorphKeyframeGetWeight(k) == 0.42f);
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("self shadow")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ShadowCamera *shadow = project->shadowCamera();
        CommandRegistrator registrator(project);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            shadow->setDistance(8026.0f);
            shadow->setCoverageMode(ShadowCamera::kCoverageModeType2);
            registrator.registerAddSelfShadowKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->selfShadowMotion();
        motion->selection()->addSelfShadowKeyframes(1339, 1341);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        for (auto it : kKeyframeIndexShouldChanged) {
            const nanoem_motion_self_shadow_keyframe_t *k = motion->findSelfShadowKeyframe(it + kFrameIndexDeltaPlus);
            CHECK(nanoemMotionSelfShadowKeyframeGetDistance(k) == Approx(-8026.0f));
            CHECK(nanoemMotionSelfShadowKeyframeGetMode(k) == 2);
            CHECK_FALSE(motion->findLightKeyframe(it));
        }
        for (auto it : kKeyframeIndexShouldNotChanged) {
            const nanoem_motion_self_shadow_keyframe_t *k = motion->findSelfShadowKeyframe(it);
            CHECK(nanoemMotionSelfShadowKeyframeGetDistance(k) == Approx(-8026.0f));
            CHECK(nanoemMotionSelfShadowKeyframeGetMode(k) == 2);
        }
        CHECK_FALSE(scope.hasAnyError());
    }
}

TEST_CASE("project_move_all_keyframes_command_minus", "[emapp][project]")
{
    Error error;
    TestScope scope;
    Application *application = scope.application();
    BX_UNUSED_1(application);
    SECTION("accessory")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Accessory *activeAccessory = first->createAccessory();
        project->addAccessory(activeAccessory);
        project->setActiveAccessory(activeAccessory);
        CommandRegistrator registrator(project);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            activeAccessory->setTranslation(Vector3(1, 2, 3));
            activeAccessory->setOrientation(glm::radians(Vector3(15, 30, 45)));
            activeAccessory->setOpacity(0.4f);
            activeAccessory->setScaleFactor(5.0f);
            registrator.registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(activeAccessory);
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeAccessory);
        motion->selection()->addAccessoryKeyframes(1339, 1341);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaMinus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        for (auto it : kKeyframeIndexShouldChanged) {
            const nanoem_motion_accessory_keyframe_t *k = motion->findAccessoryKeyframe(it + kFrameIndexDeltaMinus);
            CHECK_THAT(glm::make_vec3(nanoemMotionAccessoryKeyframeGetTranslation(k)), EqualsVector3(1, 2, 3));
            CHECK_THAT(glm::make_quat(nanoemMotionAccessoryKeyframeGetOrientation(k)), EqualsQuaternion(15, 30, 45));
            CHECK(nanoemMotionAccessoryKeyframeGetOpacity(k) == Approx(0.4f));
            CHECK(nanoemMotionAccessoryKeyframeGetScaleFactor(k) == Approx(5));
            CHECK_FALSE(motion->findAccessoryKeyframe(it));
        }
        for (auto it : kKeyframeIndexShouldNotChanged) {
            const nanoem_motion_accessory_keyframe_t *k = motion->findAccessoryKeyframe(it);
            CHECK_THAT(glm::make_vec3(nanoemMotionAccessoryKeyframeGetTranslation(k)), EqualsVector3(1, 2, 3));
            CHECK_THAT(glm::make_quat(nanoemMotionAccessoryKeyframeGetOrientation(k)), EqualsQuaternion(15, 30, 45));
            CHECK(nanoemMotionAccessoryKeyframeGetOpacity(k) == Approx(0.4f));
            CHECK(nanoemMotionAccessoryKeyframeGetScaleFactor(k) == Approx(5));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("bone")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        const nanoem_model_bone_t *activeBonePtr = activeModel->activeBone();
        model::Bone *activeBone = model::Bone::cast(activeBonePtr);
        CommandRegistrator registrator(project);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            activeBone->setLocalUserTranslation(Vector3(1, 2, 3));
            activeBone->setLocalUserOrientation(glm::radians(Vector3(45, 30, 60)));
            activeModel->performAllBonesTransform();
            registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeModel);
        motion->selection()->addBoneKeyframes(activeBonePtr, 1339, 1341);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaMinus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        const nanoem_unicode_string_t *name = nanoemModelBoneGetName(activeBonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        for (auto it : kKeyframeIndexShouldChanged) {
            const nanoem_motion_bone_keyframe_t *k = motion->findBoneKeyframe(name, it + kFrameIndexDeltaMinus);
            CHECK_THAT(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(k)), EqualsVector3(1, 2, 3));
            CHECK_THAT(glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(k)), EqualsQuaternion(45, 30, 60));
            CHECK_FALSE(motion->findBoneKeyframe(name, it));
        }
        for (auto it : kKeyframeIndexShouldNotChanged) {
            const nanoem_motion_bone_keyframe_t *k = motion->findBoneKeyframe(name, it);
            CHECK_THAT(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(k)), EqualsVector3(1, 2, 3));
            CHECK_THAT(glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(k)), EqualsQuaternion(45, 30, 60));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("camera")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ICamera *camera = project->globalCamera();
        CommandRegistrator registrator(project);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            camera->setAngle(Vector3(0.1f, 0.2f, 0.3f));
            camera->setLookAt(Vector3(1, 2, 3));
            camera->setDistance(42);
            camera->setFov(21);
            registrator.registerAddCameraKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->cameraMotion();
        motion->selection()->addCameraKeyframes(1339, 1341);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaMinus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        for (auto it : kKeyframeIndexShouldChanged) {
            const nanoem_motion_camera_keyframe_t *k = motion->findCameraKeyframe(it + kFrameIndexDeltaMinus);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(k)), EqualsVector3(0.1f, 0.2f, 0.3f));
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(k)), EqualsVector3(1, 2, 3));
            CHECK(nanoemMotionCameraKeyframeGetDistance(k) == Approx(-42));
            CHECK(nanoemMotionCameraKeyframeGetFov(k) == 21);
            CHECK_FALSE(motion->findCameraKeyframe(it));
        }
        for (auto it : kKeyframeIndexShouldNotChanged) {
            const nanoem_motion_camera_keyframe_t *k = motion->findCameraKeyframe(it);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(k)), EqualsVector3(0.1f, 0.2f, 0.3f));
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(k)), EqualsVector3(1, 2, 3));
            CHECK(nanoemMotionCameraKeyframeGetDistance(k) == Approx(-42));
            CHECK(nanoemMotionCameraKeyframeGetFov(k) == 21);
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("light")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ILight *light = project->globalLight();
        CommandRegistrator registrator(project);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            light->setColor(Vector3(0.1f, 0.2f, 0.3f));
            light->setDirection(Vector3(0.4f, 0.5f, 0.6f));
            registrator.registerAddLightKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->lightMotion();
        motion->selection()->addLightKeyframes(1339, 1341);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaMinus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        for (auto it : kKeyframeIndexShouldChanged) {
            const nanoem_motion_light_keyframe_t *k = motion->findLightKeyframe(it + kFrameIndexDeltaMinus);
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetColor(k)), EqualsVector3(0.1f, 0.2f, 0.3f));
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetDirection(k)), EqualsVector3(0.4f, 0.5f, 0.6f));
            CHECK_FALSE(motion->findLightKeyframe(it));
        }
        for (auto it : kKeyframeIndexShouldNotChanged) {
            const nanoem_motion_light_keyframe_t *k = motion->findLightKeyframe(it);
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetColor(k)), EqualsVector3(0.1f, 0.2f, 0.3f));
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetDirection(k)), EqualsVector3(0.4f, 0.5f, 0.6f));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("model")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        CommandRegistrator registrator(project);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            activeModel->setVisible(false);
            registrator.registerAddModelKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeModel);
        motion->selection()->addModelKeyframes(1339, 1341);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaMinus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        for (auto it : kKeyframeIndexShouldChanged) {
            const nanoem_motion_model_keyframe_t *k = motion->findModelKeyframe(it + kFrameIndexDeltaMinus);
            CHECK_FALSE(nanoemMotionModelKeyframeIsVisible(k));
            CHECK_FALSE(motion->findModelKeyframe(it));
        }
        for (auto it : kKeyframeIndexShouldNotChanged) {
            const nanoem_motion_model_keyframe_t *k = motion->findModelKeyframe(it);
            CHECK_FALSE(nanoemMotionModelKeyframeIsVisible(k));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("morph")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        activeModel->setActiveMorph(scope.findRandomMorph(activeModel));
        const nanoem_model_morph_t *activeMorphPtr = activeModel->activeMorph();
        model::Morph *activeMorph = model::Morph::cast(activeMorphPtr);
        CommandRegistrator registrator(project);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            activeMorph->setWeight(0.42f);
            activeModel->performAllBonesTransform();
            registrator.registerAddMorphKeyframesCommandByActiveMorph(activeModel, NANOEM_MODEL_MORPH_CATEGORY_BASE);
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeModel);
        motion->selection()->addMorphKeyframes(activeMorphPtr, 1339, 1341);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaMinus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        const nanoem_unicode_string_t *name = nanoemModelMorphGetName(activeMorphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        for (auto it : kKeyframeIndexShouldChanged) {
            const nanoem_motion_morph_keyframe_t *k = motion->findMorphKeyframe(name, it + kFrameIndexDeltaMinus);
            CHECK(nanoemMotionMorphKeyframeGetWeight(k) == Approx(0.42f));
            CHECK_FALSE(motion->findMorphKeyframe(name, it));
        }
        for (auto it : kKeyframeIndexShouldNotChanged) {
            const nanoem_motion_morph_keyframe_t *k = motion->findMorphKeyframe(name, it);
            CHECK(nanoemMotionMorphKeyframeGetWeight(k) == 0.42f);
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("self shadow")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ShadowCamera *shadow = project->shadowCamera();
        CommandRegistrator registrator(project);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            shadow->setDistance(8026.0f);
            shadow->setCoverageMode(ShadowCamera::kCoverageModeType2);
            registrator.registerAddSelfShadowKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->selfShadowMotion();
        motion->selection()->addSelfShadowKeyframes(1339, 1341);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaMinus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        for (auto it : kKeyframeIndexShouldChanged) {
            const nanoem_motion_self_shadow_keyframe_t *k = motion->findSelfShadowKeyframe(it + kFrameIndexDeltaMinus);
            CHECK(nanoemMotionSelfShadowKeyframeGetDistance(k) == Approx(-8026.0f));
            CHECK(nanoemMotionSelfShadowKeyframeGetMode(k) == 2);
            CHECK_FALSE(motion->findLightKeyframe(it));
        }
        for (auto it : kKeyframeIndexShouldNotChanged) {
            const nanoem_motion_self_shadow_keyframe_t *k = motion->findSelfShadowKeyframe(it);
            CHECK(nanoemMotionSelfShadowKeyframeGetDistance(k) == Approx(-8026.0f));
            CHECK(nanoemMotionSelfShadowKeyframeGetMode(k) == 2);
        }
        CHECK_FALSE(scope.hasAnyError());
    }
}

TEST_CASE("project_move_all_keyframes_command_zero", "[emapp][project]")
{
    Error error;
    TestScope scope;
    Application *application = scope.application();
    BX_UNUSED_1(application);
    SECTION("accessory")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Accessory *activeAccessory = first->createAccessory();
        CommandRegistrator registrator(project);
        project->addAccessory(activeAccessory);
        project->setActiveAccessory(activeAccessory);
        {
            project->seek(0, true);
            activeAccessory->setTranslation(Vector3(1, 2, 3));
            activeAccessory->setOrientation(glm::radians(Vector3(15, 30, 45)));
            activeAccessory->setOpacity(0.4f);
            activeAccessory->setScaleFactor(5.0f);
            registrator.registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(activeAccessory);
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeAccessory);
        motion->selection()->addAccessoryKeyframes(0, 0);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        for (auto it : kKeyframeIndexShouldNotChangedForZero) {
            const nanoem_motion_accessory_keyframe_t *k = motion->findAccessoryKeyframe(it);
            CHECK_THAT(glm::make_vec3(nanoemMotionAccessoryKeyframeGetTranslation(k)), EqualsVector3(1, 2, 3));
            CHECK_THAT(glm::make_quat(nanoemMotionAccessoryKeyframeGetOrientation(k)), EqualsQuaternion(15, 30, 45));
            CHECK(nanoemMotionAccessoryKeyframeGetOpacity(k) == Approx(0.4f));
            CHECK(nanoemMotionAccessoryKeyframeGetScaleFactor(k) == Approx(5));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("bone")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        const nanoem_model_bone_t *activeBonePtr = activeModel->activeBone();
        model::Bone *activeBone = model::Bone::cast(activeBonePtr);
        CommandRegistrator registrator(project);
        {
            project->seek(0, true);
            activeBone->setLocalUserTranslation(Vector3(1, 2, 3));
            activeBone->setLocalUserOrientation(glm::radians(Vector3(45, 30, 60)));
            activeModel->performAllBonesTransform();
            registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeModel);
        motion->selection()->addBoneKeyframes(activeBonePtr, 0, 0);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        const nanoem_unicode_string_t *name = nanoemModelBoneGetName(activeBonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        for (auto it : kKeyframeIndexShouldNotChangedForZero) {
            const nanoem_motion_bone_keyframe_t *k = motion->findBoneKeyframe(name, it);
            CHECK_THAT(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(k)), EqualsVector3(1, 2, 3));
            CHECK_THAT(glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(k)), EqualsQuaternion(45, 30, 60));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("camera")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ICamera *camera = project->globalCamera();
        CommandRegistrator registrator(project);
        {
            project->seek(0, true);
            camera->setAngle(Vector3(0.1f, 0.2f, 0.3f));
            camera->setLookAt(Vector3(1, 2, 3));
            camera->setDistance(42);
            camera->setFov(21);
            registrator.registerAddCameraKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->cameraMotion();
        motion->selection()->addCameraKeyframes(0, 0);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        for (auto it : kKeyframeIndexShouldNotChangedForZero) {
            const nanoem_motion_camera_keyframe_t *k = motion->findCameraKeyframe(it);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(k)), EqualsVector3(0.1f, 0.2f, 0.3f));
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(k)), EqualsVector3(1, 2, 3));
            CHECK(nanoemMotionCameraKeyframeGetDistance(k) == Approx(-42));
            CHECK(nanoemMotionCameraKeyframeGetFov(k) == 21);
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("light")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ILight *light = project->globalLight();
        CommandRegistrator registrator(project);
        {
            project->seek(0, true);
            light->setColor(Vector3(0.1f, 0.2f, 0.3f));
            light->setDirection(Vector3(0.4f, 0.5f, 0.6f));
            registrator.registerAddLightKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->lightMotion();
        motion->selection()->addLightKeyframes(0, 0);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        for (auto it : kKeyframeIndexShouldNotChangedForZero) {
            const nanoem_motion_light_keyframe_t *k = motion->findLightKeyframe(it);
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetColor(k)), EqualsVector3(0.1f, 0.2f, 0.3f));
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetDirection(k)), EqualsVector3(0.4f, 0.5f, 0.6f));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("model")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        CommandRegistrator registrator(project);
        {
            project->seek(0, true);
            activeModel->setVisible(false);
            registrator.registerAddModelKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeModel);
        motion->selection()->addModelKeyframes(0, 0);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        for (auto it : kKeyframeIndexShouldNotChangedForZero) {
            const nanoem_motion_model_keyframe_t *k = motion->findModelKeyframe(it);
            CHECK_FALSE(nanoemMotionModelKeyframeIsVisible(k));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("morph")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        activeModel->setActiveMorph(scope.findRandomMorph(activeModel));
        const nanoem_model_morph_t *activeMorphPtr = activeModel->activeMorph();
        model::Morph *activeMorph = model::Morph::cast(activeMorphPtr);
        CommandRegistrator registrator(project);
        {
            project->seek(0, true);
            activeMorph->setWeight(0.42f);
            activeModel->performAllBonesTransform();
            registrator.registerAddMorphKeyframesCommandByActiveMorph(activeModel, NANOEM_MODEL_MORPH_CATEGORY_BASE);
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeModel);
        motion->selection()->addMorphKeyframes(activeMorphPtr, 0, 0);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        const nanoem_unicode_string_t *name = nanoemModelMorphGetName(activeMorphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        for (auto it : kKeyframeIndexShouldNotChangedForZero) {
            const nanoem_motion_morph_keyframe_t *k = motion->findMorphKeyframe(name, it);
            CHECK(nanoemMotionMorphKeyframeGetWeight(k) == 0.42f);
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("self shadow")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ShadowCamera *shadow = project->shadowCamera();
        CommandRegistrator registrator(project);
        {
            project->seek(0, true);
            shadow->setDistance(8026.0f);
            shadow->setCoverageMode(ShadowCamera::kCoverageModeType2);
            registrator.registerAddSelfShadowKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->selfShadowMotion();
        motion->selection()->addSelfShadowKeyframes(0, 0);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        for (auto it : kKeyframeIndexShouldNotChangedForZero) {
            const nanoem_motion_self_shadow_keyframe_t *k = motion->findSelfShadowKeyframe(it);
            CHECK(nanoemMotionSelfShadowKeyframeGetDistance(k) == Approx(-8026.0f));
            CHECK(nanoemMotionSelfShadowKeyframeGetMode(k) == 2);
        }
        CHECK_FALSE(scope.hasAnyError());
    }
}

TEST_CASE("project_move_all_keyframes_command_forward_override", "[emapp][project]")
{
    Error error;
    TestScope scope;
    Application *application = scope.application();
    BX_UNUSED_1(application);
    SECTION("accessory")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Accessory *activeAccessory = first->createAccessory();
        CommandRegistrator registrator(project);
        project->addAccessory(activeAccessory);
        project->setActiveAccessory(activeAccessory);
        {
            project->seek(kFrameIndexDeltaPlus, true);
            activeAccessory->setTranslation(Vector3(1, 2, 3));
            activeAccessory->setOrientation(glm::radians(Vector3(15, 30, 45)));
            activeAccessory->setOpacity(0.4f);
            activeAccessory->setScaleFactor(5.0f);
            registrator.registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(activeAccessory);
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeAccessory);
        motion->selection()->addAccessoryKeyframes(0, kFrameIndexDeltaPlus);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaMinus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        {
            const nanoem_motion_accessory_keyframe_t *k = motion->findAccessoryKeyframe(0);
            CHECK_THAT(glm::make_vec3(nanoemMotionAccessoryKeyframeGetTranslation(k)), EqualsVector3(1, 2, 3));
            CHECK_THAT(glm::make_quat(nanoemMotionAccessoryKeyframeGetOrientation(k)), EqualsQuaternion(15, 30, 45));
            CHECK(nanoemMotionAccessoryKeyframeGetOpacity(k) == Approx(0.4f));
            CHECK(nanoemMotionAccessoryKeyframeGetScaleFactor(k) == Approx(5));
            CHECK_FALSE(motion->findAccessoryKeyframe(kFrameIndexDeltaPlus));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("bone")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        const nanoem_model_bone_t *activeBonePtr = activeModel->activeBone();
        model::Bone *activeBone = model::Bone::cast(activeBonePtr);
        CommandRegistrator registrator(project);
        {
            project->seek(kFrameIndexDeltaPlus, true);
            activeBone->setLocalUserTranslation(Vector3(1, 2, 3));
            activeBone->setLocalUserOrientation(glm::radians(Vector3(45, 30, 60)));
            activeModel->performAllBonesTransform();
            registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeModel);
        motion->selection()->addBoneKeyframes(activeBonePtr, 0, kFrameIndexDeltaPlus);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaMinus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        const nanoem_unicode_string_t *name = nanoemModelBoneGetName(activeBonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        {
            const nanoem_motion_bone_keyframe_t *k = motion->findBoneKeyframe(name, 0);
            CHECK_THAT(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(k)), EqualsVector3(1, 2, 3));
            CHECK_THAT(glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(k)), EqualsQuaternion(45, 30, 60));
            CHECK_FALSE(motion->findBoneKeyframe(name, kFrameIndexDeltaPlus));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("camera")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ICamera *camera = project->globalCamera();
        CommandRegistrator registrator(project);
        {
            project->seek(kFrameIndexDeltaPlus, true);
            camera->setAngle(Vector3(0.1f, 0.2f, 0.3f));
            camera->setLookAt(Vector3(1, 2, 3));
            camera->setDistance(42);
            camera->setFov(21);
            registrator.registerAddCameraKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->cameraMotion();
        motion->selection()->addCameraKeyframes(0, kFrameIndexDeltaPlus);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaMinus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        {
            const nanoem_motion_camera_keyframe_t *k = motion->findCameraKeyframe(0);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(k)), EqualsVector3(0.1f, 0.2f, 0.3f));
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(k)), EqualsVector3(1, 2, 3));
            CHECK(nanoemMotionCameraKeyframeGetDistance(k) == Approx(-42));
            CHECK(nanoemMotionCameraKeyframeGetFov(k) == 21);
            CHECK_FALSE(motion->findCameraKeyframe(kFrameIndexDeltaPlus));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("light")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ILight *light = project->globalLight();
        CommandRegistrator registrator(project);
        {
            project->seek(kFrameIndexDeltaPlus, true);
            light->setColor(Vector3(0.1f, 0.2f, 0.3f));
            light->setDirection(Vector3(0.4f, 0.5f, 0.6f));
            registrator.registerAddLightKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->lightMotion();
        motion->selection()->addLightKeyframes(0, kFrameIndexDeltaPlus);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaMinus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        {
            const nanoem_motion_light_keyframe_t *k = motion->findLightKeyframe(0);
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetColor(k)), EqualsVector3(0.1f, 0.2f, 0.3f));
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetDirection(k)), EqualsVector3(0.4f, 0.5f, 0.6f));
            CHECK_FALSE(motion->findLightKeyframe(kFrameIndexDeltaPlus));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("model")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        CommandRegistrator registrator(project);
        {
            project->seek(kFrameIndexDeltaPlus, true);
            activeModel->setVisible(false);
            registrator.registerAddModelKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeModel);
        motion->selection()->addModelKeyframes(0, kFrameIndexDeltaPlus);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaMinus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        {
            const nanoem_motion_model_keyframe_t *k = motion->findModelKeyframe(0);
            CHECK_FALSE(nanoemMotionModelKeyframeIsVisible(k));
            CHECK_FALSE(motion->findModelKeyframe(kFrameIndexDeltaPlus));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("morph")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        activeModel->setActiveMorph(scope.findRandomMorph(activeModel));
        const nanoem_model_morph_t *activeMorphPtr = activeModel->activeMorph();
        model::Morph *activeMorph = model::Morph::cast(activeMorphPtr);
        CommandRegistrator registrator(project);
        {
            project->seek(kFrameIndexDeltaPlus, true);
            activeMorph->setWeight(0.42f);
            activeModel->performAllBonesTransform();
            registrator.registerAddMorphKeyframesCommandByActiveMorph(activeModel, NANOEM_MODEL_MORPH_CATEGORY_BASE);
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeModel);
        motion->selection()->addMorphKeyframes(activeMorphPtr, 0, kFrameIndexDeltaPlus);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaMinus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        const nanoem_unicode_string_t *name = nanoemModelMorphGetName(activeMorphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        {
            const nanoem_motion_morph_keyframe_t *k = motion->findMorphKeyframe(name, 0);
            CHECK(nanoemMotionMorphKeyframeGetWeight(k) == 0.42f);
            CHECK_FALSE(motion->findMorphKeyframe(name, kFrameIndexDeltaPlus));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("self shadow")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ShadowCamera *shadow = project->shadowCamera();
        CommandRegistrator registrator(project);
        {
            project->seek(kFrameIndexDeltaPlus, true);
            shadow->setDistance(8026.0f);
            shadow->setCoverageMode(ShadowCamera::kCoverageModeType2);
            registrator.registerAddSelfShadowKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->selfShadowMotion();
        motion->selection()->addSelfShadowKeyframes(0, kFrameIndexDeltaPlus);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaMinus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        {
            const nanoem_motion_self_shadow_keyframe_t *k = motion->findSelfShadowKeyframe(0);
            CHECK(nanoemMotionSelfShadowKeyframeGetDistance(k) == Approx(-8026.0f));
            CHECK(nanoemMotionSelfShadowKeyframeGetMode(k) == 2);
            CHECK_FALSE(motion->findSelfShadowKeyframe(kFrameIndexDeltaPlus));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
}

TEST_CASE("project_move_all_keyframes_command_backward_override", "[emapp][project]")
{
    Error error;
    TestScope scope;
    Application *application = scope.application();
    BX_UNUSED_1(application);
    SECTION("accessory")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Accessory *activeAccessory = first->createAccessory();
        CommandRegistrator registrator(project);
        project->addAccessory(activeAccessory);
        project->setActiveAccessory(activeAccessory);
        {
            project->seek(kFrameIndexDeltaPlus, true);
            activeAccessory->setTranslation(Vector3(1, 2, 3));
            activeAccessory->setOrientation(glm::radians(Vector3(15, 30, 45)));
            activeAccessory->setOpacity(0.4f);
            activeAccessory->setScaleFactor(5.0f);
            registrator.registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(activeAccessory);
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeAccessory);
        motion->selection()->addAccessoryKeyframes(0, kFrameIndexDeltaPlus);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        {
            const nanoem_motion_accessory_keyframe_t *k =
                motion->findAccessoryKeyframe(kFrameIndexDeltaPlus + kFrameIndexDeltaPlus);
            CHECK_THAT(glm::make_vec3(nanoemMotionAccessoryKeyframeGetTranslation(k)), EqualsVector3(1, 2, 3));
            CHECK_THAT(glm::make_quat(nanoemMotionAccessoryKeyframeGetOrientation(k)), EqualsQuaternion(15, 30, 45));
            CHECK(nanoemMotionAccessoryKeyframeGetOpacity(k) == Approx(0.4f));
            CHECK(nanoemMotionAccessoryKeyframeGetScaleFactor(k) == Approx(5));
            const nanoem_motion_accessory_keyframe_t *k2 = motion->findAccessoryKeyframe(kFrameIndexDeltaPlus);
            CHECK_THAT(glm::make_vec3(nanoemMotionAccessoryKeyframeGetTranslation(k2)), EqualsVector3(0, 0, 0));
            CHECK_THAT(glm::make_quat(nanoemMotionAccessoryKeyframeGetOrientation(k2)), EqualsQuaternion(0, 0, 0));
            CHECK(nanoemMotionAccessoryKeyframeGetOpacity(k2) == Approx(1));
            CHECK(nanoemMotionAccessoryKeyframeGetScaleFactor(k2) == Approx(1));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("bone")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        const nanoem_model_bone_t *activeBonePtr = activeModel->activeBone();
        model::Bone *activeBone = model::Bone::cast(activeBonePtr);
        CommandRegistrator registrator(project);
        {
            project->seek(kFrameIndexDeltaPlus, true);
            activeBone->setLocalUserTranslation(Vector3(1, 2, 3));
            activeBone->setLocalUserOrientation(glm::radians(Vector3(45, 30, 60)));
            activeModel->performAllBonesTransform();
            registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeModel);
        motion->selection()->addBoneKeyframes(activeBonePtr, 0, kFrameIndexDeltaPlus);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        const nanoem_unicode_string_t *name = nanoemModelBoneGetName(activeBonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        {
            const nanoem_motion_bone_keyframe_t *k =
                motion->findBoneKeyframe(name, kFrameIndexDeltaPlus + kFrameIndexDeltaPlus);
            CHECK_THAT(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(k)), EqualsVector3(1, 2, 3));
            CHECK_THAT(glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(k)), EqualsQuaternion(45, 30, 60));
            const nanoem_motion_bone_keyframe_t *k2 = motion->findBoneKeyframe(name, kFrameIndexDeltaPlus);
            CHECK_THAT(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(k2)), EqualsVector3(0, 0, 0));
            CHECK_THAT(glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(k2)), EqualsQuaternion(0, 0, 0));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("camera")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ICamera *camera = project->globalCamera();
        CommandRegistrator registrator(project);
        {
            project->seek(kFrameIndexDeltaPlus, true);
            camera->setAngle(Vector3(0.1f, 0.2f, 0.3f));
            camera->setLookAt(Vector3(1, 2, 3));
            camera->setDistance(42);
            camera->setFov(21);
            registrator.registerAddCameraKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->cameraMotion();
        motion->selection()->addCameraKeyframes(0, kFrameIndexDeltaPlus);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        {
            const nanoem_motion_camera_keyframe_t *k =
                motion->findCameraKeyframe(kFrameIndexDeltaPlus + kFrameIndexDeltaPlus);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(k)), EqualsVector3(0.1f, 0.2f, 0.3f));
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(k)), EqualsVector3(1, 2, 3));
            CHECK(nanoemMotionCameraKeyframeGetDistance(k) == Approx(-42));
            CHECK(nanoemMotionCameraKeyframeGetFov(k) == 21);
            const nanoem_motion_camera_keyframe_t *k2 = motion->findCameraKeyframe(kFrameIndexDeltaPlus);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(k2)), EqualsVector3(0, 0, 0));
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(k2)), EqualsVector3(0, 10, 0));
            CHECK(nanoemMotionCameraKeyframeGetDistance(k2) == Approx(-45));
            CHECK(nanoemMotionCameraKeyframeGetFov(k2) == 30);
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("light")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ILight *light = project->globalLight();
        CommandRegistrator registrator(project);
        {
            project->seek(kFrameIndexDeltaPlus, true);
            light->setColor(Vector3(0.1f, 0.2f, 0.3f));
            light->setDirection(Vector3(0.4f, 0.5f, 0.6f));
            registrator.registerAddLightKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->lightMotion();
        motion->selection()->addLightKeyframes(0, kFrameIndexDeltaPlus);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        {
            const nanoem_motion_light_keyframe_t *k =
                motion->findLightKeyframe(kFrameIndexDeltaPlus + kFrameIndexDeltaPlus);
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetColor(k)), EqualsVector3(0.1f, 0.2f, 0.3f));
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetDirection(k)), EqualsVector3(0.4f, 0.5f, 0.6f));
            const nanoem_motion_light_keyframe_t *k2 = motion->findLightKeyframe(kFrameIndexDeltaPlus);
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetColor(k2)), Equals(DirectionalLight::kInitialColor));
            CHECK_THAT(glm::make_vec3(nanoemMotionLightKeyframeGetDirection(k2)), EqualsVector3(-0.5f, -1.0f, 0.5f));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("model")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        CommandRegistrator registrator(project);
        {
            project->seek(kFrameIndexDeltaPlus, true);
            activeModel->setVisible(false);
            registrator.registerAddModelKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeModel);
        motion->selection()->addModelKeyframes(0, kFrameIndexDeltaPlus);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        {
            const nanoem_motion_model_keyframe_t *k =
                motion->findModelKeyframe(kFrameIndexDeltaPlus + kFrameIndexDeltaPlus);
            CHECK_FALSE(nanoemMotionModelKeyframeIsVisible(k));
            const nanoem_motion_model_keyframe_t *k2 = motion->findModelKeyframe(kFrameIndexDeltaPlus);
            CHECK(nanoemMotionModelKeyframeIsVisible(k2));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("morph")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        activeModel->setActiveMorph(scope.findRandomMorph(activeModel));
        const nanoem_model_morph_t *activeMorphPtr = activeModel->activeMorph();
        model::Morph *activeMorph = model::Morph::cast(activeMorphPtr);
        CommandRegistrator registrator(project);
        {
            project->seek(kFrameIndexDeltaPlus, true);
            activeMorph->setWeight(0.42f);
            activeModel->performAllBonesTransform();
            registrator.registerAddMorphKeyframesCommandByActiveMorph(activeModel, NANOEM_MODEL_MORPH_CATEGORY_BASE);
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->resolveMotion(activeModel);
        motion->selection()->addMorphKeyframes(activeMorphPtr, 0, kFrameIndexDeltaPlus);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        const nanoem_unicode_string_t *name = nanoemModelMorphGetName(activeMorphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        {
            const nanoem_motion_morph_keyframe_t *k =
                motion->findMorphKeyframe(name, kFrameIndexDeltaPlus + kFrameIndexDeltaPlus);
            CHECK(nanoemMotionMorphKeyframeGetWeight(k) == Approx(0.42f));
            const nanoem_motion_morph_keyframe_t *k2 = motion->findMorphKeyframe(name, kFrameIndexDeltaPlus);
            CHECK(nanoemMotionMorphKeyframeGetWeight(k2) == Approx(0));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("self shadow")
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        ShadowCamera *shadow = project->shadowCamera();
        CommandRegistrator registrator(project);
        {
            project->seek(kFrameIndexDeltaPlus, true);
            shadow->setDistance(8026.0f);
            shadow->setCoverageMode(ShadowCamera::kCoverageModeType2);
            registrator.registerAddSelfShadowKeyframesCommandByCurrentLocalFrameIndex();
        }
        undoStackClear(project->undoStack());
        Motion *motion = project->selfShadowMotion();
        motion->selection()->addSelfShadowKeyframes(0, kFrameIndexDeltaPlus);
        registrator.registerMoveAllSelectedKeyframesCommand(kFrameIndexDeltaPlus, error);
        CHECK_FALSE(error.hasReason());
        CHECK(project->canUndo());
        {
            const nanoem_motion_self_shadow_keyframe_t *k =
                motion->findSelfShadowKeyframe(kFrameIndexDeltaPlus + kFrameIndexDeltaPlus);
            CHECK(nanoemMotionSelfShadowKeyframeGetDistance(k) == Approx(-8026.0f));
            CHECK(nanoemMotionSelfShadowKeyframeGetMode(k) == 2);
            const nanoem_motion_self_shadow_keyframe_t *k2 = motion->findSelfShadowKeyframe(kFrameIndexDeltaPlus);
            CHECK(nanoemMotionSelfShadowKeyframeGetDistance(k2) == Approx(ShadowCamera::kInitialDistance));
            CHECK(nanoemMotionSelfShadowKeyframeGetMode(k2) == 1);
        }
        CHECK_FALSE(scope.hasAnyError());
    }
}
