/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/model/Morph.h"

using namespace nanoem;
using namespace test;

struct emapp_motion_correct_all_selected_morph_keyframes_parameter_t {
    const nanoem_f32_t base;
    const nanoem_f32_t mul;
    const nanoem_f32_t add;
    const nanoem_f32_t expected;
};

#if 0
ParameterizedTestParameters(emapp, motion_correct_all_selected_morph_keyframes_translation)
{
    static emapp_motion_correct_all_selected_morph_keyframes_parameter_t parameters[] = { { 1.0f, 0.5f, 0.3f, 0.8f },
        { 1.0f, -0.5f, 0.3f, -0.2f }, { 1.0f, 0.5f, -0.3f, 0.2f }, { 1.0f, -0.5f, -0.3f, -0.8f } };
    return criterion_test_params(parameters);
}

ParameterizedTest(emapp_motion_correct_all_selected_morph_keyframes_parameter_t *parameter, emapp,
    motion_correct_all_selected_morph_keyframes_translation)
{
    TestScope scope;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first.get()->m_project;
        Model *activeModel = createModel(project);
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        auto findMorphKeyframe = [project](const Model *model, const nanoem_unicode_string_t *name,
                                     nanoem_frame_index_t frameIndex) {
            const Motion *motion = project->resolveMotion(model);
            return nanoemMotionFindMorphKeyframeObject(motion->data(), name, frameIndex);
        };
        {
            const nanoem_model_morph_t *firstMorph = findRandomMorph(activeModel);
            cr_assert_not_null(firstMorph);
            activeModel->setActiveMorph(firstMorph);
            model::Morph *morph = model::Morph::cast(firstMorph);
            cr_assert_not_null(morph);
            morph->setWeight(parameter->base);
            project->addMorphKeyframesByAllMorphs(activeModel);
        }
        const Motion::CorrectionScalarFactor &weight = tinystl::make_pair(parameter->mul, parameter->add);
        project->selectAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
        project->correctAllSelectedMorphKeyframes(activeModel, weight);
        const nanoem_unicode_string_t *name =
            nanoemModelMorphGetName(activeModel->activeMorph(), NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        {
            const nanoem_motion_morph_keyframe_t *keyframe = findMorphKeyframe(activeModel, name, 0);
            cr_assert_not_null(keyframe);
            REQUIRE(nanoemMotionMorphKeyframeGetWeight(keyframe) == Approx(parameter->expected));
        }
        project->handleUndoAction();
        {
            const nanoem_motion_morph_keyframe_t *keyframe = findMorphKeyframe(activeModel, name, 0);
            cr_assert_not_null(keyframe);
            REQUIRE(nanoemMotionMorphKeyframeGetWeight(keyframe) == Approx(parameter->base));
        }
        project->handleRedoAction();
        {
            const nanoem_motion_morph_keyframe_t *keyframe = findMorphKeyframe(activeModel, name, 0);
            cr_assert_not_null(keyframe);
            REQUIRE(nanoemMotionMorphKeyframeGetWeight(keyframe) == Approx(parameter->expected));
        }
    }
}
#endif
