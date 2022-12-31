/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/ICamera.h"
#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/Model.h"

#include "undo/undo.h"

using namespace nanoem;
using namespace test;

static int kKeyframeIndexRegistrationOrder[] = { 1335, 1337, 1339, 1341, 1343 };
static int kKeyframeIndexShouldChanged[] = { 1337, 1339, 1341 };
static int kKeyframeIndexShouldNotChanged[] = { 1335, 1343 };
static int kKeyframeIndexSelectBegin = kKeyframeIndexShouldChanged[0];
static int kKeyframeIndexSelectEnd = kKeyframeIndexShouldChanged[2];

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

static void
clearAllDirtyStates(Motion *motion)
{
    undoStackClear(motion->project()->undoStack());
    motion->selection()->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
}

TEST_CASE("project_register_correct_all_motion_keyframes_command", "[emapp][project]")
{
    TestScope scope;
    Error error;
    ProjectPtr first = scope.createProject();
    Project *project = first->m_project;
    CommandRegistrator registrator(project);
    SECTION("bone")
    {
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        const nanoem_model_bone_t *activeBonePtr = activeModel->activeBone();
        model::Bone *activeBone = model::Bone::cast(activeBonePtr);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            activeBone->setLocalUserTranslation(Vector3(1, 2, 3));
            activeBone->setLocalUserOrientation(glm::radians(Vector3(45, 30, 60)));
            activeModel->performAllBonesTransform();
            registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
        }
        Motion *motion = project->resolveMotion(activeModel);
        clearAllDirtyStates(motion);
        motion->selection()->addBoneKeyframes(activeBonePtr, kKeyframeIndexSelectBegin, kKeyframeIndexSelectEnd);
        registrator.registerCorrectAllSelectedBoneKeyframesCommand(activeModel,
            Motion::CorrectionVectorFactor(Vector3(0.4f), Vector3(0.2f)),
            Motion::CorrectionVectorFactor(Vector3(0.75f), Vector3(12)), error);
        CHECK(!error.hasReason());
        CHECK(project->canUndo());
        const nanoem_unicode_string_t *name = nanoemModelBoneGetName(activeBonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        for (auto it : kKeyframeIndexShouldChanged) {
            const nanoem_motion_bone_keyframe_t *k = motion->findBoneKeyframe(name, it);
            CHECK_THAT(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(k)), EqualsVector3(0.6, 1, 1.4));
            CHECK_THAT(glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(k)), EqualsQuaternion(45.75, 34.5, 57));
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
        ICamera *camera = project->globalCamera();
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            camera->setAngle(Vector3(0.1, 0.2, 0.3));
            camera->setLookAt(Vector3(1, 2, 3));
            camera->setDistance(42);
            camera->setFov(21);
            registrator.registerAddCameraKeyframesCommandByCurrentLocalFrameIndex();
        }
        Motion *motion = project->cameraMotion();
        clearAllDirtyStates(motion);
        motion->selection()->addCameraKeyframes(kKeyframeIndexSelectBegin, kKeyframeIndexSelectEnd);
        registrator.registerCorrectAllSelectedCameraKeyframesCommand(
            Motion::CorrectionVectorFactor(Vector3(0.7f), Vector3(-0.3f)),
            Motion::CorrectionVectorFactor(Vector3(-0.3f), Vector3(0.7f)), Motion::CorrectionScalarFactor(0.8f, 0.2f),
            error);
        CHECK(!error.hasReason());
        CHECK(project->canUndo());
        for (auto it : kKeyframeIndexShouldChanged) {
            const nanoem_motion_camera_keyframe_t *k = motion->findCameraKeyframe(it);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(k)), EqualsVector3(-0.67f, 0.64f, 0.61f));
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(k)), EqualsVector3(0.4f, 1.1f, 1.8f));
            CHECK(nanoemMotionCameraKeyframeGetDistance(k) == Approx(-33.4f));
            CHECK(nanoemMotionCameraKeyframeGetFov(k) == 21);
        }
        for (auto it : kKeyframeIndexShouldNotChanged) {
            const nanoem_motion_camera_keyframe_t *k = motion->findCameraKeyframe(it);
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(k)), EqualsVector3(-0.1f, 0.2f, 0.3f));
            CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(k)), EqualsVector3(1, 2, 3));
            CHECK(nanoemMotionCameraKeyframeGetDistance(k) == Approx(-42));
            CHECK(nanoemMotionCameraKeyframeGetFov(k) == 21);
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("morph")
    {
        Model *activeModel = first->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        const nanoem_model_morph_t *activeMorphPtr = TestScope::findRandomMorph(activeModel);
        model::Morph *activeMorph = model::Morph::cast(activeMorphPtr);
        for (auto it : kKeyframeIndexRegistrationOrder) {
            project->seek(it, true);
            activeMorph->setWeight(0.42f);
            registrator.registerAddMorphKeyframesCommandByAllMorphs(activeModel);
        }
        Motion *motion = project->resolveMotion(activeModel);
        clearAllDirtyStates(motion);
        motion->selection()->addMorphKeyframes(activeMorphPtr, kKeyframeIndexSelectBegin, kKeyframeIndexSelectEnd);
        registrator.registerCorrectAllSelectedMorphKeyframesCommand(
            activeModel, Motion::CorrectionScalarFactor(0.75f, 0.25f), error);
        CHECK(!error.hasReason());
        CHECK(project->canUndo());
        const nanoem_unicode_string_t *name = nanoemModelMorphGetName(activeMorphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        for (auto it : kKeyframeIndexShouldChanged) {
            const nanoem_motion_morph_keyframe_t *k = motion->findMorphKeyframe(name, it);
            CHECK(nanoemMotionMorphKeyframeGetWeight(k) == Approx(0.565f));
        }
        for (auto it : kKeyframeIndexShouldNotChanged) {
            const nanoem_motion_morph_keyframe_t *k = motion->findMorphKeyframe(name, it);
            CHECK(nanoemMotionMorphKeyframeGetWeight(k) == Approx(0.42f));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
}

TEST_CASE("project_register_correct_all_motion_keyframes_redo_command", "[emapp][project]")
{
    TestScope scope;
    Error error;
    SECTION("bone")
    {
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
        }
        {
            ProjectPtr second = scope.createProject();
            Project *project = second->m_project;
            scope.recover(project);
            Model *activeModel = project->activeModel();
            const nanoem_model_bone_t *activeBonePtr = activeModel->activeBone();
            Motion *motion = project->resolveMotion(activeModel);
            motion->selection()->addBoneKeyframes(activeBonePtr, kKeyframeIndexSelectBegin, kKeyframeIndexSelectEnd);
            CommandRegistrator registrator(project);
            registrator.registerCorrectAllSelectedBoneKeyframesCommand(activeModel,
                Motion::CorrectionVectorFactor(Vector3(0.4f), Vector3(0.2f)),
                Motion::CorrectionVectorFactor(Vector3(0.75f), Vector3(12)), error);
            CHECK(!error.hasReason());
            CHECK(project->canUndo());
            const nanoem_unicode_string_t *name =
                nanoemModelBoneGetName(activeBonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
            for (auto it : kKeyframeIndexShouldChanged) {
                const nanoem_motion_bone_keyframe_t *k = motion->findBoneKeyframe(name, it);
                CHECK_THAT(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(k)), EqualsVector3(0.6f, 1, 1.4f));
                CHECK_THAT(
                    glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(k)), EqualsQuaternion(45.75f, 34.5f, 57));
            }
            for (auto it : kKeyframeIndexShouldNotChanged) {
                const nanoem_motion_bone_keyframe_t *k = motion->findBoneKeyframe(name, it);
                CHECK_THAT(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(k)), EqualsVector3(1, 2, 3));
                CHECK_THAT(glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(k)), EqualsQuaternion(45, 30, 60));
            }
            CHECK_FALSE(scope.hasAnyError());
        }
    }
    SECTION("camera")
    {
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
        }
        {
            ProjectPtr second = scope.createProject();
            Project *project = second->m_project;
            scope.recover(project);
            Motion *motion = project->cameraMotion();
            motion->selection()->addCameraKeyframes(kKeyframeIndexSelectBegin, kKeyframeIndexSelectEnd);
            CommandRegistrator registrator(project);
            registrator.registerCorrectAllSelectedCameraKeyframesCommand(
                Motion::CorrectionVectorFactor(Vector3(0.7f), Vector3(-0.3f)),
                Motion::CorrectionVectorFactor(Vector3(-0.3f), Vector3(0.7f)),
                Motion::CorrectionScalarFactor(0.8f, 0.2f), error);
            CHECK(!error.hasReason());
            CHECK(project->canUndo());
            for (auto it : kKeyframeIndexShouldChanged) {
                const nanoem_motion_camera_keyframe_t *k = motion->findCameraKeyframe(it);
                CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(k)), EqualsVector3(-0.67f, 0.64f, 0.61f));
                CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(k)), EqualsVector3(0.4f, 1.1f, 1.8f));
                CHECK(nanoemMotionCameraKeyframeGetDistance(k) == Approx(-33.4f));
                CHECK(nanoemMotionCameraKeyframeGetFov(k) == 21);
            }
            for (auto it : kKeyframeIndexShouldNotChanged) {
                const nanoem_motion_camera_keyframe_t *k = motion->findCameraKeyframe(it);
                CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(k)), EqualsVector3(-0.1f, 0.2f, 0.3f));
                CHECK_THAT(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(k)), EqualsVector3(1, 2, 3));
                CHECK(nanoemMotionCameraKeyframeGetDistance(k) == Approx(-42));
                CHECK(nanoemMotionCameraKeyframeGetFov(k) == 21);
            }
            CHECK_FALSE(scope.hasAnyError());
        }
    }
    SECTION("morph")
    {
        {
            ProjectPtr first = scope.createProject();
            Project *project = first->withRecoverable();
            Model *activeModel = first->createModel();
            project->addModel(activeModel);
            project->setActiveModel(activeModel);
            const nanoem_model_morph_t *activeMorphPtr = TestScope::findFirstMorph(activeModel);
            model::Morph *activeMorph = model::Morph::cast(activeMorphPtr);
            CommandRegistrator registrator(project);
            for (auto it : kKeyframeIndexRegistrationOrder) {
                project->seek(it, true);
                activeMorph->setWeight(0.42f);
                registrator.registerAddMorphKeyframesCommandByAllMorphs(activeModel);
            }
        }
        {
            ProjectPtr second = scope.createProject();
            Project *project = second->m_project;
            scope.recover(project);
            Model *activeModel = project->activeModel();
            const nanoem_model_morph_t *activeMorphPtr = TestScope::findFirstMorph(activeModel);
            Motion *motion = project->resolveMotion(activeModel);
            motion->selection()->addMorphKeyframes(activeMorphPtr, kKeyframeIndexSelectBegin, kKeyframeIndexSelectEnd);
            CommandRegistrator registrator(project);
            registrator.registerCorrectAllSelectedMorphKeyframesCommand(
                activeModel, Motion::CorrectionScalarFactor(0.75f, 0.25f), error);
            CHECK(!error.hasReason());
            CHECK(project->canUndo());
            const nanoem_unicode_string_t *name =
                nanoemModelMorphGetName(activeMorphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
            for (auto it : kKeyframeIndexShouldChanged) {
                const nanoem_motion_morph_keyframe_t *k = motion->findMorphKeyframe(name, it);
                CHECK(nanoemMotionMorphKeyframeGetWeight(k) == Approx(0.565f));
            }
            for (auto it : kKeyframeIndexShouldNotChanged) {
                const nanoem_motion_morph_keyframe_t *k = motion->findMorphKeyframe(name, it);
                CHECK(nanoemMotionMorphKeyframeGetWeight(k) == Approx(0.42f));
            }
            CHECK_FALSE(scope.hasAnyError());
        }
    }
}
