/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

using namespace nanoem;
using namespace test;

struct emapp_motion_correct_all_selected_bone_keyframes_parameter_t {
    const Vector3 base;
    const Vector3 mul;
    const Vector3 add;
    const Vector3 expected;
};

#if 0
ParameterizedTestParameters(emapp, motion_correct_all_selected_bone_keyframes_translation)
{
    static emapp_motion_correct_all_selected_bone_keyframes_parameter_t parameters[] = {
        { Vector3(1, 2, 3), Vector3(0.5f), Vector3(0.3f), Vector3(0.8f, 1.3f, 1.8f) },
        { Vector3(1, 2, 3), Vector3(-0.5f), Vector3(0.3f), Vector3(-0.2f, -0.7f, -1.2f) },
        { Vector3(1, 2, 3), Vector3(0.5f), Vector3(-0.3f), Vector3(0.2f, 0.7f, 1.2f) },
        { Vector3(1, 2, 3), Vector3(-0.5f), Vector3(-0.3f), Vector3(-0.8f, -1.3f, -1.8f) }
    };
    return criterion_test_params(parameters);
}

ParameterizedTest(emapp_motion_correct_all_selected_bone_keyframes_parameter_t *parameter, emapp,
    motion_correct_all_selected_bone_keyframes_translation)
{
    TestScope scope;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first.get()->m_project;
        Model *activeModel = createModel(project);
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        auto findBoneKeyframe = [project](const Model *model, const nanoem_unicode_string_t *name,
                                    nanoem_frame_index_t frameIndex) {
            const Motion *motion = project->resolveMotion(model);
            return nanoemMotionFindBoneKeyframeObject(motion->data(), name, frameIndex);
        };
        {
            model::Bone *bone = model::Bone::cast(activeModel->activeBone());
            cr_assert_not_null(bone);
            bone->setLocalUserTranslation(parameter->base);
            bone->setLocalUserOrientation(Constants::kZeroQ);
            bone->setDirty(true);
            project->performBoneTransform(activeModel, activeModel->activeBone());
            project->addBoneKeyframesBySelectedBoneSet(activeModel);
        }
        const Motion::CorrectionVectorFactor &translation = tinystl::make_pair(parameter->mul, parameter->add);
        const Motion::CorrectionVectorFactor &orientation = tinystl::make_pair(Vector3(0), Vector3(0));
        project->selectAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
        project->correctAllSelectedBoneKeyframes(activeModel, translation, orientation);
        const nanoem_unicode_string_t *name =
            nanoemModelBoneGetName(activeModel->activeBone(), NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        {
            const nanoem_motion_bone_keyframe_t *keyframe = findBoneKeyframe(activeModel, name, 0);
            cr_assert_not_null(keyframe);
            cr_expect(glm::all(glm::epsilonEqual(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(keyframe)),
                parameter->expected, Vector3(0.000001f))));
        }
        project->handleUndoAction();
        {
            const nanoem_motion_bone_keyframe_t *keyframe = findBoneKeyframe(activeModel, name, 0);
            cr_assert_not_null(keyframe);
            cr_expect(glm::all(glm::epsilonEqual(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(keyframe)),
                parameter->base * Constants::kTranslateDirection, Vector3(0.000001f))));
        }
        project->handleRedoAction();
        {
            const nanoem_motion_bone_keyframe_t *keyframe = findBoneKeyframe(activeModel, name, 0);
            cr_assert_not_null(keyframe);
            cr_expect(glm::all(glm::epsilonEqual(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(keyframe)),
                parameter->expected, Vector3(0.000001f))));
        }
    }
}

ParameterizedTestParameters(emapp, motion_correct_all_selected_bone_keyframes_orientation)
{
    static emapp_motion_correct_all_selected_bone_keyframes_parameter_t parameters[] = {
        { Vector3(1, 2, 3), Vector3(0.5f), Vector3(0.3f), Vector3(0.8f, 1.3f, 1.8f) },
        { Vector3(1, 2, 3), Vector3(-0.5f), Vector3(0.3f), Vector3(-0.2f, -0.7f, -1.2f) },
        { Vector3(1, 2, 3), Vector3(0.5f), Vector3(-0.3f), Vector3(0.2f, 0.7f, 1.2f) },
        { Vector3(1, 2, 3), Vector3(-0.5f), Vector3(-0.3f), Vector3(-0.8f, -1.3f, -1.8f) }
    };
    return criterion_test_params(parameters);
}

ParameterizedTest(emapp_motion_correct_all_selected_bone_keyframes_parameter_t *parameter, emapp,
    motion_correct_all_selected_bone_keyframes_orientation)
{
    TestScope scope;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first.get()->m_project;
        Model *activeModel = createModel(project);
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        auto findBoneKeyframe = [project](const Model *model, const nanoem_unicode_string_t *name,
                                    nanoem_frame_index_t frameIndex) {
            const Motion *motion = project->resolveMotion(model);
            return nanoemMotionFindBoneKeyframeObject(motion->data(), name, frameIndex);
        };
        {
            model::Bone *bone = model::Bone::cast(activeModel->activeBone());
            cr_assert_not_null(bone);
            bone->setLocalUserTranslation(Vector3(0));
            bone->setLocalUserOrientation(glm::radians(parameter->base));
            bone->setDirty(true);
            project->performBoneTransform(activeModel, activeModel->activeBone());
            project->addBoneKeyframesBySelectedBoneSet(activeModel);
        }
        const Motion::CorrectionVectorFactor &translation = tinystl::make_pair(Vector3(0), Vector3(0));
        const Motion::CorrectionVectorFactor &orientation = tinystl::make_pair(parameter->mul, parameter->add);
        project->selectAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
        project->correctAllSelectedBoneKeyframes(activeModel, translation, orientation);
        const nanoem_unicode_string_t *name =
            nanoemModelBoneGetName(activeModel->activeBone(), NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        {
            const nanoem_motion_bone_keyframe_t *keyframe = findBoneKeyframe(activeModel, name, 0);
            cr_assert_not_null(keyframe);
            cr_expect(glm::all(glm::epsilonEqual(
                glm::degrees(glm::eulerAngles(glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(keyframe)))),
                parameter->expected, Vector3(0.000001f))));
        }
        project->handleUndoAction();
        {
            const nanoem_motion_bone_keyframe_t *keyframe = findBoneKeyframe(activeModel, name, 0);
            cr_assert_not_null(keyframe);
            cr_expect(glm::all(glm::epsilonEqual(
                glm::degrees(glm::eulerAngles(glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(keyframe)))),
                parameter->base * Vector3(Constants::kOrientateDirection), Vector3(0.000001f))));
        }
        project->handleRedoAction();
        {
            const nanoem_motion_bone_keyframe_t *keyframe = findBoneKeyframe(activeModel, name, 0);
            cr_assert_not_null(keyframe);
            cr_expect(glm::all(glm::epsilonEqual(
                glm::degrees(glm::eulerAngles(glm::make_quat(nanoemMotionBoneKeyframeGetOrientation(keyframe)))),
                parameter->expected, Vector3(0.000001f))));
        }
    }
}
#endif
