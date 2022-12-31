/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/Model.h"

using namespace nanoem;
using namespace test;

struct EqualsTranslationInterpolation : Catch::Matchers::Impl::MatcherBase<const nanoem_motion_bone_keyframe_t *> {
    EqualsTranslationInterpolation(const Vector4U8 &v)
        : m_data(v)
    {
    }
    EqualsTranslationInterpolation(const EqualsTranslationInterpolation &v)
        : m_data(v.m_data)
    {
    }
    bool
    match(const nanoem_motion_bone_keyframe_t *const &keyframe) const override
    {
        return glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(
                   keyframe, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X)) == m_data &&
            glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(
                keyframe, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y)) == m_data &&
            glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(
                keyframe, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z)) == m_data &&
            glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe,
                NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION)) == model::Bone::kDefaultBezierControlPoint;
    }
    std::string
    describe() const override
    {
        return "of bone keyframe interpolation equals " + glm::to_string(m_data);
    }
    Vector4U8 m_data;
};

TEST_CASE("project_bezier_curve_adjustment_bone", "[emapp][project]")
{
    TestScope scope;
    Error error;
    ProjectPtr first = scope.createProject();
    Project *project = first->withRecoverable();
    Model *activeModel = first->createModel();
    project->addModel(activeModel);
    project->seek(1337, true);
    CommandRegistrator registrator(project);
    {
        model::Bone *bone = model::Bone::cast(activeModel->activeBone());
        bone->setDirty(true);
        activeModel->performAllBonesTransform();
        registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
    }
    {
        const nanoem_motion_bone_keyframe_t *keyframe = first->findBoneKeyframe(activeModel);
        CHECK_THAT(keyframe, EqualsTranslationInterpolation(model::Bone::kDefaultAutomaticBezierControlPoint));
    }
    {
        const nanoem_motion_bone_keyframe_t *keyframe = first->findBoneKeyframe(activeModel, 1337);
        CHECK_THAT(keyframe, EqualsTranslationInterpolation(model::Bone::kDefaultBezierControlPoint));
    }
    SECTION("split at 25%")
    {
        project->seek(334, true);
        model::Bone *bone = model::Bone::cast(activeModel->activeBone());
        bone->setDirty(true);
        activeModel->performAllBonesTransform();
        registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
        {
            const nanoem_motion_bone_keyframe_t *firstKeyframe = first->findBoneKeyframe(activeModel, 0);
            CHECK_THAT(firstKeyframe, EqualsTranslationInterpolation(Vector4U8(15, 0, 27, 7)));
        }
        {
            const nanoem_motion_bone_keyframe_t *secondKeyframe = first->findBoneKeyframe(activeModel, 334);
            CHECK_THAT(secondKeyframe, EqualsTranslationInterpolation(Vector4U8(79, 127, 67, 55)));
        }
    }
    SECTION("split at 75%")
    {
        project->seek(1002, true);
        model::Bone *bone = model::Bone::cast(activeModel->activeBone());
        bone->setDirty(true);
        activeModel->performAllBonesTransform();
        registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
        {
            const nanoem_motion_bone_keyframe_t *firstKeyframe = first->findBoneKeyframe(activeModel, 0);
            CHECK_THAT(firstKeyframe, EqualsTranslationInterpolation(Vector4U8(47, 0, 59, 71)));
        }
        {
            const nanoem_motion_bone_keyframe_t *secondKeyframe = first->findBoneKeyframe(activeModel, 1002);
            CHECK_THAT(secondKeyframe, EqualsTranslationInterpolation(Vector4U8(111, 127, 99, 119)));
        }
    }
}
