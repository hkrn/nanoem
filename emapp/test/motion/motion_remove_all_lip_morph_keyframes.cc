/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/Model.h"

using namespace nanoem;
using namespace test;

TEST_CASE("motion_remove_all_lip_morph_keyframes", "[emapp][motion]")
{
    TestScope scope;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *activeModel = first->createModel();
        CommandRegistrator registrator(project);
        {
            project->setBezierCurveAdjustmentEnabled(false);
            project->addModel(activeModel);
            project->setActiveModel(activeModel);
            /* number of morph keyframes will be 180 (= 30 * 6) */
            const nanoem_model_morph_t *morph = first->findFirstMorph(activeModel, NANOEM_MODEL_MORPH_CATEGORY_LIP);
            Motion::MorphFrameIndexSetMap morphs;
            Motion::FrameIndexSet frameIndices;
            frameIndices.insert(0);
            model::Morph::cast(morph)->setWeight(0.8f);
            morphs.insert(tinystl::make_pair(morph, frameIndices));
            registrator.registerAddMorphKeyframesCommand(morphs, activeModel, project->resolveMotion(activeModel));
            project->seek(2, true);
            registrator.registerAddMorphKeyframesCommandByAllMorphs(activeModel);
            project->seek(5, true);
            registrator.registerAddMorphKeyframesCommandByAllMorphs(activeModel);
            project->seek(9, true);
            registrator.registerAddMorphKeyframesCommandByAllMorphs(activeModel);
            project->seek(14, true);
            registrator.registerAddMorphKeyframesCommandByAllMorphs(activeModel);
            project->seek(20, true);
            registrator.registerAddMorphKeyframesCommandByAllMorphs(activeModel);
        }
        registrator.registerRemoveAllLipMorphKeyframesCommand(activeModel);
        SECTION("remove all lip morph keyframes")
        {
            CHECK(first->motionDuration(activeModel) == 20);
            CHECK(first->countAllMorphKeyframes(activeModel) == 125);
            CHECK(first->countAllMorphKeyframesByCategory(activeModel, NANOEM_MODEL_MORPH_CATEGORY_EYE) == 66);
            CHECK(first->countAllMorphKeyframesByCategory(activeModel, NANOEM_MODEL_MORPH_CATEGORY_EYEBROW) == 36);
            CHECK(first->countAllMorphKeyframesByCategory(activeModel, NANOEM_MODEL_MORPH_CATEGORY_LIP) == 11);
            CHECK(first->countAllMorphKeyframesByCategory(activeModel, NANOEM_MODEL_MORPH_CATEGORY_OTHER) == 12);
            CHECK(first->findFirstMorphKeyframeWeight(activeModel, NANOEM_MODEL_MORPH_CATEGORY_LIP) == Approx(0.0f));
            CHECK(project->resolveMotion(activeModel)->isDirty());
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleUndoAction();
        SECTION("undo")
        {
            CHECK(first->motionDuration(activeModel) == 20);
            CHECK(first->countAllMorphKeyframes(activeModel) == 180);
            CHECK(first->countAllMorphKeyframesByCategory(activeModel, NANOEM_MODEL_MORPH_CATEGORY_EYE) == 66);
            CHECK(first->countAllMorphKeyframesByCategory(activeModel, NANOEM_MODEL_MORPH_CATEGORY_EYEBROW) == 36);
            CHECK(first->countAllMorphKeyframesByCategory(activeModel, NANOEM_MODEL_MORPH_CATEGORY_LIP) == 66);
            CHECK(first->countAllMorphKeyframesByCategory(activeModel, NANOEM_MODEL_MORPH_CATEGORY_OTHER) == 12);
            CHECK(first->findFirstMorphKeyframeWeight(activeModel, NANOEM_MODEL_MORPH_CATEGORY_LIP) == Approx(0.8f));
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleRedoAction();
        SECTION("redo")
        {
            CHECK(first->motionDuration(activeModel) == 20);
            CHECK(first->countAllMorphKeyframes(activeModel) == 125);
            CHECK(first->countAllMorphKeyframesByCategory(activeModel, NANOEM_MODEL_MORPH_CATEGORY_EYE) == 66);
            CHECK(first->countAllMorphKeyframesByCategory(activeModel, NANOEM_MODEL_MORPH_CATEGORY_EYEBROW) == 36);
            CHECK(first->countAllMorphKeyframesByCategory(activeModel, NANOEM_MODEL_MORPH_CATEGORY_LIP) == 11);
            CHECK(first->countAllMorphKeyframesByCategory(activeModel, NANOEM_MODEL_MORPH_CATEGORY_OTHER) == 12);
            CHECK(first->findFirstMorphKeyframeWeight(activeModel, NANOEM_MODEL_MORPH_CATEGORY_LIP) == Approx(0.0f));
            CHECK_FALSE(scope.hasAnyError());
        }
        SECTION("recover")
        {
            ProjectPtr second = scope.createProject();
            Project *project2 = second.get()->m_project;
            scope.recover(project2);
            Model *activeModel2 = project2->activeModel();
            CHECK(second->motionDuration(activeModel2) == 20);
            CHECK(second->countAllMorphKeyframes(activeModel2) == 125);
            CHECK(second->countAllMorphKeyframesByCategory(activeModel2, NANOEM_MODEL_MORPH_CATEGORY_EYE) == 66);
            CHECK(second->countAllMorphKeyframesByCategory(activeModel2, NANOEM_MODEL_MORPH_CATEGORY_EYEBROW) == 36);
            CHECK(second->countAllMorphKeyframesByCategory(activeModel2, NANOEM_MODEL_MORPH_CATEGORY_LIP) == 11);
            CHECK(second->countAllMorphKeyframesByCategory(activeModel2, NANOEM_MODEL_MORPH_CATEGORY_OTHER) == 12);
            CHECK(second->findFirstMorphKeyframeWeight(activeModel2, NANOEM_MODEL_MORPH_CATEGORY_LIP) == Approx(0.0f));
            CHECK_FALSE(scope.hasAnyError());
        }
    }
}
