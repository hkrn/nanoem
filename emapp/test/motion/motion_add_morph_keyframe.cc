/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/Model.h"

using namespace nanoem;
using namespace test;

TEST_CASE("motion_add_morph_keyframe", "[emapp][motion]")
{
    TestScope scope;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first->withRecoverable();
        Model *other1Model = first->createModel();
        Model *other2Model = first->createModel();
        Model *activeModel = first->createModel();
        {
            project->addModel(other1Model);
            project->addModel(other2Model);
            project->addModel(activeModel);
            project->setActiveModel(activeModel);
            project->seek(1337, true);
            const nanoem_model_morph_t *firstMorph = TestScope::findRandomMorph(activeModel);
            activeModel->setActiveMorph(firstMorph);
            model::Morph *morph = model::Morph::cast(firstMorph);
            morph->setWeight(0.42f);
        }
        CommandRegistrator registrator(project);
        registrator.registerAddMorphKeyframesCommandByAllMorphs(activeModel);
        const nanoem_unicode_string_t *name =
            nanoemModelMorphGetName(activeModel->activeMorph(), NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        SECTION("adding morph keyframe")
        {
            CHECK(first->countAllMorphKeyframes(activeModel) == 60);
            CHECK(first->countAllMorphKeyframes(other1Model) == 30);
            CHECK(first->countAllMorphKeyframes(other2Model) == 30);
            const nanoem_motion_morph_keyframe_t *keyframe = first->findMorphKeyframe(activeModel, name, 1337);
            CHECK(keyframe);
            CHECK_FALSE(first->findMorphKeyframe(other1Model, name, 1337));
            CHECK_FALSE(first->findMorphKeyframe(other2Model, name, 1337));
            CHECK(nanoemMotionMorphKeyframeGetWeight(keyframe) == Approx(0.42f));
            CHECK(project->duration() == 1337);
            CHECK(first->motionDuration(activeModel) == 1337);
            CHECK(first->motionDuration(other1Model) == 0);
            CHECK(first->motionDuration(other2Model) == 0);
            CHECK(project->resolveMotion(activeModel)->isDirty());
            CHECK_FALSE(project->resolveMotion(other1Model)->isDirty());
            CHECK_FALSE(project->resolveMotion(other2Model)->isDirty());
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleUndoAction();
        SECTION("undo")
        {
            CHECK(first->countAllMorphKeyframes(activeModel) == 30);
            CHECK(first->countAllMorphKeyframes(other1Model) == 30);
            CHECK(first->countAllMorphKeyframes(other2Model) == 30);
            CHECK(project->duration() == project->baseDuration());
            CHECK(first->motionDuration(activeModel) == 0);
            CHECK(first->motionDuration(other1Model) == 0);
            CHECK(first->motionDuration(other2Model) == 0);
            CHECK_FALSE(first->findMorphKeyframe(activeModel, name, 1337));
            CHECK_FALSE(first->findMorphKeyframe(other1Model, name, 1337));
            CHECK_FALSE(first->findMorphKeyframe(other2Model, name, 1337));
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleRedoAction();
        SECTION("redo")
        {
            CHECK(first->countAllMorphKeyframes(activeModel) == 60);
            CHECK(first->countAllMorphKeyframes(other1Model) == 30);
            CHECK(first->countAllMorphKeyframes(other2Model) == 30);
            const nanoem_motion_morph_keyframe_t *keyframe = first->findMorphKeyframe(activeModel, name, 1337);
            CHECK(keyframe);
            CHECK_FALSE(first->findMorphKeyframe(other1Model, name, 1337));
            CHECK_FALSE(first->findMorphKeyframe(other2Model, name, 1337));
            CHECK(nanoemMotionMorphKeyframeGetWeight(keyframe) == Approx(0.42f));
            CHECK(project->duration() == 1337);
            CHECK(first->motionDuration(activeModel) == 1337);
            CHECK(first->motionDuration(other1Model) == 0);
            CHECK(first->motionDuration(other2Model) == 0);
            CHECK_FALSE(scope.hasAnyError());
        }
        SECTION("recovery")
        {
            ProjectPtr second = scope.createProject();
            Project *project2 = second.get()->m_project;
            scope.recover(project2);
            Model *activeModel2 = project2->activeModel();
            CHECK(second->countAllMorphKeyframes(activeModel2) == 60);
            const nanoem_motion_morph_keyframe_t *keyframe = second->findMorphKeyframe(activeModel2, name, 1337);
            CHECK(keyframe);
            CHECK(nanoemMotionMorphKeyframeGetWeight(keyframe) == Approx(0.42f));
            CHECK(project2->duration() == 1337);
            CHECK(second->motionDuration(activeModel2) == 1337);
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
            const nanoem_model_morph_t *firstMorph = TestScope::findRandomMorph(activeModel);
            activeModel->setActiveMorph(firstMorph);
            model::Morph *morph = model::Morph::cast(firstMorph);
            morph->setWeight(0.42f);
            CommandRegistrator registrator(project);
            registrator.registerAddMorphKeyframesCommandByAllMorphs(activeModel);
            project->removeModel(activeModel);
            project->destroyModel(activeModel);
        }
        SUCCEED("should not be crashed");
    }
}
