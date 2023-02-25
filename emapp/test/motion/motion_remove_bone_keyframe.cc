/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/Constants.h"
#include "emapp/CommandRegistrator.h"
#include "emapp/Model.h"

using namespace nanoem;
using namespace test;

TEST_CASE("motion_remove_bone_keyframe", "[emapp][motion]")
{
    TestScope scope;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *other1Model = first->createModel();
        Model *other2Model = first->createModel();
        Model *activeModel = first->createModel();
        {
            project->setBezierCurveAdjustmentEnabled(false);
            project->addModel(other1Model);
            project->addModel(other2Model);
            project->addModel(activeModel);
            project->setActiveModel(activeModel);
            project->seek(1337, true);
            model::Bone *bone = model::Bone::cast(activeModel->activeBone());
            bone->setLocalUserTranslation(Vector3(0.1, 0.2, 0.3));
            bone->setLocalUserOrientation(Quaternion(0.9f, 0.1f, 0.2f, 0.3f));
            bone->setBezierControlPoints(
                NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X, Vector4U8(2, 4, 6, 8));
            bone->setBezierControlPoints(
                NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y, Vector4U8(3, 5, 7, 9));
            bone->setBezierControlPoints(
                NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z, Vector4U8(11, 13, 15, 17));
            bone->setBezierControlPoints(
                NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION, Vector4U8(12, 14, 16, 18));
            bone->setDirty(true);
            activeModel->performAllBonesTransform();
        }
        const nanoem_unicode_string_t *name =
            nanoemModelBoneGetName(activeModel->activeBone(), NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        CommandRegistrator registrator(project);
        registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
        registrator.registerRemoveBoneKeyframesCommandByCurrentLocalFrameIndex(activeModel);
        SECTION("removing all eye morph keyframes")
        {
            CHECK(first->countAllBoneKeyframes(activeModel) == 140);
            CHECK(first->countAllBoneKeyframes(other1Model) == 140);
            CHECK(first->countAllBoneKeyframes(other2Model) == 140);
            CHECK(project->duration() == project->baseDuration());
            CHECK(first->motionDuration(activeModel) == 0);
            CHECK(first->motionDuration(other1Model) == 0);
            CHECK(first->motionDuration(other2Model) == 0);
            CHECK_FALSE(first->findBoneKeyframe(activeModel, name, 1337));
            CHECK_FALSE(first->findBoneKeyframe(other1Model, name, 1337));
            CHECK_FALSE(first->findBoneKeyframe(other2Model, name, 1337));
            CHECK(project->resolveMotion(activeModel)->isDirty());
            CHECK_FALSE(project->resolveMotion(other1Model)->isDirty());
            CHECK_FALSE(project->resolveMotion(other2Model)->isDirty());
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleUndoAction();
        SECTION("undo")
        {
            CHECK(first->countAllBoneKeyframes(activeModel) == 141);
            CHECK(first->countAllBoneKeyframes(other1Model) == 140);
            CHECK(first->countAllBoneKeyframes(other2Model) == 140);
            const nanoem_motion_bone_keyframe_t *keyframe = first->findBoneKeyframe(activeModel, name, 1337);
            CHECK(keyframe);
            CHECK_FALSE(first->findBoneKeyframe(other1Model, name, 1337));
            CHECK_FALSE(first->findBoneKeyframe(other2Model, name, 1337));
            CHECK_THAT(
                glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(keyframe)), Equals(Vector3(0.1, 0.2, 0.3)));
            CHECK_THAT(glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(keyframe)),
                Equals(Quaternion(0.9f, 0.1f, 0.2f, 0.3f)));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X)) == Vector4U8(2, 4, 6, 8));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y)) == Vector4U8(3, 5, 7, 9));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z)) == Vector4U8(11, 13, 15, 17));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION)) == Vector4U8(12, 14, 16, 18));
            CHECK(project->duration() == 1337);
            CHECK(first->motionDuration(activeModel) == 1337);
            CHECK(first->motionDuration(other1Model) == 0);
            CHECK(first->motionDuration(other2Model) == 0);
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleRedoAction();
        SECTION("redo")
        {
            CHECK(first->countAllBoneKeyframes(activeModel) == 140);
            CHECK(first->countAllBoneKeyframes(other1Model) == 140);
            CHECK(first->countAllBoneKeyframes(other2Model) == 140);
            CHECK(project->duration() == project->baseDuration());
            CHECK(first->motionDuration(activeModel) == 0);
            CHECK(first->motionDuration(other1Model) == 0);
            CHECK(first->motionDuration(other2Model) == 0);
            CHECK_FALSE(first->findBoneKeyframe(activeModel, name, 1337));
            CHECK_FALSE(first->findBoneKeyframe(other1Model, name, 1337));
            CHECK_FALSE(first->findBoneKeyframe(other2Model, name, 1337));
            CHECK_FALSE(scope.hasAnyError());
        }
        SECTION("recover")
        {
            ProjectPtr second = scope.createProject();
            Project *project2 = second.get()->m_project;
            scope.recover(project2);
            Model *activeModel2 = project2->activeModel();
            CHECK(second->countAllBoneKeyframes(activeModel2) == 140);
            CHECK(project2->duration() == project->baseDuration());
            CHECK(second->motionDuration(activeModel2) == 0);
            CHECK_FALSE(second->findBoneKeyframe(activeModel2, name, 1337));
            CHECK_FALSE(scope.hasAnyError());
        }
    }
    SECTION("crash (may not be crashed unless ASAN)")
    {
        {
            ProjectPtr first = scope.createProject();
            Project *project = first->m_project;
            Model *activeModel = first->createModel();
            project->addModel(activeModel);
            project->setBezierCurveAdjustmentEnabled(false);
            project->setActiveModel(activeModel);
            project->seek(1337, true);
            model::Bone *bone = model::Bone::cast(activeModel->activeBone());
            bone->setLocalUserTranslation(Vector3(0.1, 0.2, 0.3));
            bone->setLocalUserOrientation(Quaternion(0.9f, 0.1f, 0.2f, 0.3f));
            bone->setBezierControlPoints(
                NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X, Vector4U8(2, 4, 6, 8));
            bone->setBezierControlPoints(
                NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y, Vector4U8(3, 5, 7, 9));
            bone->setBezierControlPoints(
                NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z, Vector4U8(11, 13, 15, 17));
            bone->setBezierControlPoints(
                NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION, Vector4U8(12, 14, 16, 18));
            bone->setDirty(true);
            activeModel->performAllBonesTransform();
            CommandRegistrator registrator(project);
            registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
            registrator.registerRemoveBoneKeyframesCommandByCurrentLocalFrameIndex(activeModel);
            project->removeModel(activeModel);
            project->destroyModel(activeModel);
        }
        SUCCEED("should not be crashed");
    }
}

TEST_CASE("motion_remove_bone_keyframe_at_zero", "[emapp][motion]")
{
    TestScope scope;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *other1Model = first->createModel();
        Model *other2Model = first->createModel();
        Model *activeModel = first->createModel();
        {
            project->setBezierCurveAdjustmentEnabled(false);
            project->addModel(other1Model);
            project->addModel(other2Model);
            project->addModel(activeModel);
            project->setActiveModel(activeModel);
            model::Bone *bone = model::Bone::cast(activeModel->activeBone());
            bone->setLocalUserTranslation(Vector3(0.1, 0.2, 0.3));
            bone->setLocalUserOrientation(Quaternion(0.9f, 0.1f, 0.2f, 0.3f));
            bone->setBezierControlPoints(
                NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X, Vector4U8(2, 4, 6, 8));
            bone->setBezierControlPoints(
                NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y, Vector4U8(3, 5, 7, 9));
            bone->setBezierControlPoints(
                NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z, Vector4U8(11, 13, 15, 17));
            bone->setBezierControlPoints(
                NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION, Vector4U8(12, 14, 16, 18));
            bone->setDirty(true);
            activeModel->performAllBonesTransform();
        }
        const nanoem_unicode_string_t *name =
            nanoemModelBoneGetName(activeModel->activeBone(), NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        CommandRegistrator registrator(project);
        registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
        registrator.registerRemoveBoneKeyframesCommandByCurrentLocalFrameIndex(activeModel);
        SECTION("removing active bone keyframe")
        {
            CHECK(first->countAllBoneKeyframes(activeModel) == 140);
            CHECK(first->countAllBoneKeyframes(other1Model) == 140);
            CHECK(first->countAllBoneKeyframes(other2Model) == 140);
            const nanoem_motion_bone_keyframe_t *keyframe = first->findBoneKeyframe(activeModel, name, 0);
            CHECK_THAT(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(keyframe)), Equals(Constants::kZeroV3));
            CHECK_THAT(glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(keyframe)), Equals(Constants::kZeroQ));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION)) == Vector4U8(20, 20, 107, 107));
            CHECK(keyframe);
            CHECK(project->duration() == project->baseDuration());
            CHECK(first->motionDuration(activeModel) == 0);
            CHECK(first->motionDuration(other1Model) == 0);
            CHECK(first->motionDuration(other2Model) == 0);
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleUndoAction();
        SECTION("undo")
        {
            CHECK(first->countAllBoneKeyframes(activeModel) == 140);
            CHECK(first->countAllBoneKeyframes(other1Model) == 140);
            CHECK(first->countAllBoneKeyframes(other2Model) == 140);
            const nanoem_motion_bone_keyframe_t *keyframe = first->findBoneKeyframe(activeModel, name, 0);
            CHECK(keyframe);
            CHECK_FALSE(first->findBoneKeyframe(other1Model, name, 1337));
            CHECK_FALSE(first->findBoneKeyframe(other2Model, name, 1337));
            CHECK_THAT(
                glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(keyframe)), Equals(Vector3(0.1, 0.2, 0.3)));
            CHECK_THAT(glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(keyframe)),
                Equals(Quaternion(0.9f, 0.1f, 0.2f, 0.3f)));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X)) == Vector4U8(2, 4, 6, 8));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y)) == Vector4U8(3, 5, 7, 9));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z)) == Vector4U8(11, 13, 15, 17));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION)) == Vector4U8(12, 14, 16, 18));
            CHECK(project->duration() == project->baseDuration());
            CHECK(first->motionDuration(activeModel) == 0);
            CHECK(first->motionDuration(other1Model) == 0);
            CHECK(first->motionDuration(other2Model) == 0);
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleRedoAction();
        SECTION("redo")
        {
            CHECK(first->countAllBoneKeyframes(activeModel) == 140);
            CHECK(first->countAllBoneKeyframes(other1Model) == 140);
            CHECK(first->countAllBoneKeyframes(other2Model) == 140);
            const nanoem_motion_bone_keyframe_t *keyframe = first->findBoneKeyframe(activeModel, name, 0);
            CHECK_THAT(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(keyframe)), Equals(Constants::kZeroV3));
            CHECK_THAT(glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(keyframe)), Equals(Constants::kZeroQ));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION)) == Vector4U8(20, 20, 107, 107));
            CHECK(keyframe);
            CHECK(project->duration() == project->baseDuration());
            CHECK(first->motionDuration(activeModel) == 0);
            CHECK(first->motionDuration(other1Model) == 0);
            CHECK(first->motionDuration(other2Model) == 0);
            CHECK_FALSE(scope.hasAnyError());
        }
        SECTION("recover")
        {
            ProjectPtr second = scope.createProject();
            Project *project2 = second.get()->m_project;
            scope.recover(project2);
            Model *activeModel2 = project2->activeModel();
            CHECK(second->countAllBoneKeyframes(activeModel2) == 140);
            const nanoem_motion_bone_keyframe_t *keyframe = second->findBoneKeyframe(activeModel2, name, 0);
            CHECK_THAT(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(keyframe)), Equals(Constants::kZeroV3));
            CHECK_THAT(glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(keyframe)), Equals(Constants::kZeroQ));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z)) == Vector4U8(20, 20, 107, 107));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION)) == Vector4U8(20, 20, 107, 107));
            CHECK(keyframe);
            CHECK(project2->duration() == project->baseDuration());
            CHECK(second->motionDuration(activeModel2) == 0);
            CHECK_FALSE(scope.hasAnyError());
        }
    }
}
