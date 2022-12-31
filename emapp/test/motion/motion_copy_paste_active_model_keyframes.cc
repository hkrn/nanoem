/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/IModelObjectSelection.h"
#include "emapp/internal/DraggingBoneState.h"
#include "emapp/internal/DraggingMorphState.h"

using namespace nanoem;
using namespace nanoem::internal;
using namespace test;

TEST_CASE("motion_copy_paste_active_model_keyframes", "[emapp][motion]")
{
    TestScope scope;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first.get()->m_project;
        Model *activeModel = first->createModel();
        const nanoem_model_bone_t *bonePtr;
        const nanoem_model_morph_t *morphPtr;
        {
            project->addModel(activeModel);
            project->setActiveModel(activeModel);
            bonePtr = TestScope::findFirstBone(activeModel);
            morphPtr = TestScope::findRandomMorph(activeModel);
            activeModel->selection()->addBone(bonePtr);
            {
                std::unique_ptr<DraggingBoneState> state(
                    new TranslateActiveBoneState(project, activeModel, Vector2(480, 240), Vector2(480, 240)));
                state->transform(Vector2(500, 260));
                state->commit(Vector2(500, 260));
                activeModel->setTransformAxisType(Model::kAxisTypeX);
                state.reset(new OrientateActiveBoneState(project, activeModel, Vector2(480, 240), Vector2(480, 240)));
                state->transform(Vector2(500, 260));
                state->commit(Vector2(500, 260));
            }
            CommandRegistrator registrator(project);
            registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
            {
                std::unique_ptr<DraggingMorphSliderState> state(
                    new DraggingMorphSliderState(morphPtr, activeModel, project, 0.0f));
                state->deform(0.8f);
                state->commit();
            }
            registrator.registerAddMorphKeyframesCommandByAllMorphs(activeModel);
            activeModel->selection()->removeAllBones();
        }
        project->selectAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
        Error error;
        project->copyAllSelectedKeyframes(error);
        CHECK_FALSE(error.hasReason());
        project->pasteAllSelectedKeyframes(1337, error);
        CHECK_FALSE(error.hasReason());
        {
            CHECK(first->countAllMorphKeyframes(activeModel) == 60);
            CHECK(project->duration() == 1337);
            CHECK(first->motionDuration(activeModel) == 1337);
            const nanoem_motion_bone_keyframe_t *boneKeyframe = first->findBoneKeyframe(activeModel, bonePtr, 1337);
            CHECK(boneKeyframe);
            CHECK(glm::all(glm::epsilonEqual(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(boneKeyframe)),
                Vector3(1.004809, -1.004809, 0), Vector3(0.000001f))));
            CHECK(glm::all(glm::epsilonEqual(glm::make_vec4(nanoemMotionBoneKeyframeGetOrientation(boneKeyframe)),
                Vector4(0.173648, 0, 0, 0.984808), Vector4(0.000001f))));
            const nanoem_motion_morph_keyframe_t *morphKeyframe = first->findMorphKeyframe(activeModel, morphPtr, 1337);
            CHECK(morphKeyframe);
            CHECK(nanoemMotionMorphKeyframeGetWeight(morphKeyframe) == Approx(0.8f));
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleUndoAction();
        {
            CHECK(first->countAllMorphKeyframes(activeModel) == 30);
            CHECK(project->duration() == project->baseDuration());
            CHECK(first->motionDuration(activeModel) == 0);
            CHECK_FALSE(first->findBoneKeyframe(activeModel, bonePtr, 1337));
            CHECK_FALSE(first->findMorphKeyframe(activeModel, morphPtr, 1337));
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleRedoAction();
        {
            CHECK(first->countAllMorphKeyframes(activeModel) == 60);
            CHECK(project->duration() == 1337);
            CHECK(first->motionDuration(activeModel) == 1337);
            const nanoem_motion_bone_keyframe_t *boneKeyframe = first->findBoneKeyframe(activeModel, bonePtr, 1337);
            CHECK(boneKeyframe);
            CHECK(glm::all(glm::epsilonEqual(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(boneKeyframe)),
                Vector3(1.004809, -1.004809, 0), Vector3(0.000001f))));
            CHECK(glm::all(glm::epsilonEqual(glm::make_vec4(nanoemMotionBoneKeyframeGetOrientation(boneKeyframe)),
                Vector4(0.173648, 0, 0, 0.984808), Vector4(0.000001f))));
            const nanoem_motion_morph_keyframe_t *morphKeyframe = first->findMorphKeyframe(activeModel, morphPtr, 1337);
            CHECK(morphKeyframe);
            CHECK(nanoemMotionMorphKeyframeGetWeight(morphKeyframe) == Approx(0.8f));
            CHECK_FALSE(scope.hasAnyError());
        }
    }
}
