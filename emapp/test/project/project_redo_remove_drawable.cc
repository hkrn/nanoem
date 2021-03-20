/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/Accessory.h"
#include "emapp/CommandRegistrator.h"
#include "emapp/Model.h"

using namespace nanoem;
using namespace test;

TEST_CASE("project_remove_drawable", "[emapp][project]")
{
    {
        TestScope scope;
        ProjectPtr first = scope.createProject();
        Project *firstProject = first->withRecoverable();
        Accessory *removingAccessory = first->createAccessory();
        firstProject->addAccessory(removingAccessory);
        Model *removingModel = first->createModel();
        firstProject->addModel(removingModel);
        firstProject->seek(1337, true);
        firstProject->setBezierCurveAdjustmentEnabled(false);
        CommandRegistrator registrator(firstProject);
        {
            Accessory *activeAccessory = first->createAccessory();
            firstProject->addAccessory(activeAccessory);
            activeAccessory->setTranslation(Vector3(42, 84, 126));
            activeAccessory->setOrientation(glm::radians(Vector3(15, 30, 45)));
            activeAccessory->setOpacity(0.4f);
            activeAccessory->setScaleFactor(0.8f);
            activeAccessory->setShadowMapEnabled(true);
            activeAccessory->setAddBlendEnabled(true);
            activeAccessory->setVisible(false);
            registrator.registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(activeAccessory);
        }
        {
            Model *activeModel = first->createModel();
            firstProject->addModel(activeModel);
            firstProject->setActiveModel(activeModel);
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
            registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
        }
        firstProject->handleUndoAction();
        firstProject->handleRedoAction();
        firstProject->setActiveModel(nullptr);
        firstProject->handleUndoAction();
        firstProject->handleRedoAction();
        firstProject->removeAccessory(removingAccessory);
        firstProject->removeModel(removingModel);
        Error error;
        removingAccessory->writeDeleteCommandMessage(error);
        removingModel->writeDeleteCommandMessage(error);
        firstProject->destroyAccessory(removingAccessory);
        firstProject->destroyModel(removingModel);
    }
    INFO("start recoverying project")
    {
        TestScope scope;
        ProjectPtr second = scope.createProject();
        Project *secondProject = second.get()->m_project;
        scope.recover(secondProject);
        CHECK(secondProject->allAccessories().size() == 1);
        CHECK(secondProject->allModels().size() == 1);
        {
            Accessory *accessory = secondProject->allAccessories()[0];
            const nanoem_motion_accessory_keyframe_t *keyframe = second->findAccessoryKeyframe(accessory, 1337);
            CHECK_THAT(
                glm::make_vec3(nanoemMotionAccessoryKeyframeGetTranslation(keyframe)), Equals(Vector3(42, 84, 126)));
            CHECK(glm::make_quat(nanoemMotionAccessoryKeyframeGetOrientation(keyframe)) ==
                Quaternion(glm::radians(Vector3(15, 30, 45))));
            CHECK(nanoemMotionAccessoryKeyframeGetOpacity(keyframe) == Approx(0.4));
            CHECK(nanoemMotionAccessoryKeyframeGetScaleFactor(keyframe) == Approx(0.8));
            CHECK(nanoemMotionAccessoryKeyframeIsAddBlendEnabled(keyframe));
            CHECK(nanoemMotionAccessoryKeyframeIsShadowEnabled(keyframe));
            CHECK_FALSE(nanoemMotionAccessoryKeyframeIsVisible(keyframe));
            CHECK(second->motionDuration(accessory) == 1337);
        }
        {
            Model *model = secondProject->allModels()[0];
            const nanoem_unicode_string_t *name =
                nanoemModelBoneGetName(model->activeBone(), NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
            CHECK(second->countAllBoneKeyframes(model) == 141);
            const nanoem_motion_bone_keyframe_t *keyframe = second->findBoneKeyframe(model, name, 1337);
            CHECK_THAT(
                glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(keyframe)), Equals(Vector3(0.1, 0.2, 0.3)));
            CHECK(
                glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(keyframe)) == Quaternion(0.9f, 0.1f, 0.2f, 0.3f));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X)) == Vector4U8(2, 4, 6, 8));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y)) == Vector4U8(3, 5, 7, 9));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z)) == Vector4U8(11, 13, 15, 17));
            CHECK(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                      NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION)) == Vector4U8(12, 14, 16, 18));
            CHECK(second->motionDuration(model) == 1337);
        }
    }
}

TEST_CASE("project_remove_null_accessory", "[emapp][project]")
{
    {
        TestScope scope;
        ProjectPtr first = scope.createProject();
        Project *firstProject = first->withRecoverable();
        Accessory *removingAccessory = first->createAccessory();
        firstProject->addAccessory(removingAccessory);
        firstProject->seek(1337, true);
        firstProject->removeAccessory(removingAccessory);
        Error error;
        removingAccessory->writeDeleteCommandMessage(error);
        /* write twice to check null pointer access */
        removingAccessory->writeDeleteCommandMessage(error);
        firstProject->destroyAccessory(removingAccessory);
    }
    INFO("start recoverying project")
    {
        TestScope scope;
        ProjectPtr second = scope.createProject();
        Project *secondProject = second.get()->m_project;
        scope.recover(secondProject);
        CHECK(secondProject->allAccessories().size() == 0);
    }
}

TEST_CASE("project_remove_null_model", "[emapp][project]")
{
    {
        TestScope scope;
        ProjectPtr first = scope.createProject();
        Project *firstProject = first->withRecoverable();
        Model *removingModel = first->createModel();
        firstProject->addModel(removingModel);
        firstProject->seek(1337, true);
        firstProject->removeModel(removingModel);
        Error error;
        removingModel->writeDeleteCommandMessage(error);
        /* write twice to check null pointer access */
        removingModel->writeDeleteCommandMessage(error);
        firstProject->destroyModel(removingModel);
    }
    INFO("start recoverying project")
    {
        TestScope scope;
        ProjectPtr second = scope.createProject();
        Project *secondProject = second.get()->m_project;
        scope.recover(secondProject);
        CHECK(secondProject->allModels().size() == 0);
    }
}
