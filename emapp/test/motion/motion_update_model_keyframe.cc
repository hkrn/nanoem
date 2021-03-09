/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/Model.h"

using namespace nanoem;
using namespace test;

TEST_CASE("motion_update_model_keyframe", "[emapp][motion]")
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
            activeModel->setVisible(false);
            activeModel->setEdgeColor(Vector4(0.7, 0.3, 0.5, 0.9));
            activeModel->setEdgeSizeScaleFactor(0.25);
        }
        CommandRegistrator registrator(project);
        registrator.registerAddModelKeyframesCommandByCurrentLocalFrameIndex(activeModel);
        {
            activeModel->setVisible(true);
            activeModel->setEdgeColor(Vector4(0.9, 0.5, 0.3, 0.7));
            activeModel->setEdgeSizeScaleFactor(0.5);
        }
        registrator.registerAddModelKeyframesCommandByCurrentLocalFrameIndex(activeModel);
        SECTION("updating current active model keyframe")
        {
            CHECK(first->countAllModelKeyframes(activeModel) == 2);
            CHECK(first->countAllModelKeyframes(other1Model) == 1);
            CHECK(first->countAllModelKeyframes(other2Model) == 1);
            const nanoem_motion_model_keyframe_t *keyframe =
                nanoemMotionFindModelKeyframeObject(project->resolveMotion(activeModel)->data(), 1337);
            CHECK(keyframe);
            CHECK_FALSE(nanoemMotionFindModelKeyframeObject(project->resolveMotion(other1Model)->data(), 1337));
            CHECK_FALSE(nanoemMotionFindModelKeyframeObject(project->resolveMotion(other2Model)->data(), 1337));
            CHECK_THAT(
                glm::make_vec4(nanoemMotionModelKeyframeGetEdgeColor(keyframe)), Equals(Vector4(0.9, 0.5, 0.3, 0.7)));
            CHECK(nanoemMotionModelKeyframeIsVisible(keyframe) == 1);
            CHECK(nanoemMotionModelKeyframeGetEdgeScaleFactor(keyframe) == Approx(0.5f));
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
            CHECK(first->countAllModelKeyframes(activeModel) == 2);
            CHECK(first->countAllModelKeyframes(other1Model) == 1);
            CHECK(first->countAllModelKeyframes(other2Model) == 1);
            const nanoem_motion_model_keyframe_t *keyframe =
                nanoemMotionFindModelKeyframeObject(project->resolveMotion(activeModel)->data(), 1337);
            CHECK(keyframe);
            CHECK_FALSE(nanoemMotionFindModelKeyframeObject(project->resolveMotion(other1Model)->data(), 1337));
            CHECK_FALSE(nanoemMotionFindModelKeyframeObject(project->resolveMotion(other2Model)->data(), 1337));
            CHECK_THAT(
                glm::make_vec4(nanoemMotionModelKeyframeGetEdgeColor(keyframe)), Equals(Vector4(0.7, 0.3, 0.5, 0.9)));
            CHECK_FALSE(nanoemMotionModelKeyframeIsVisible(keyframe));
            CHECK(nanoemMotionModelKeyframeGetEdgeScaleFactor(keyframe) == Approx(0.25f));
            CHECK(first->motionDuration(activeModel) == 1337);
            CHECK(first->motionDuration(other1Model) == 0);
            CHECK(first->motionDuration(other2Model) == 0);
            CHECK_FALSE(scope.hasAnyError());
        }
        project->handleRedoAction();
        SECTION("redo")
        {
            CHECK(first->countAllModelKeyframes(activeModel) == 2);
            CHECK(first->countAllModelKeyframes(other1Model) == 1);
            CHECK(first->countAllModelKeyframes(other2Model) == 1);
            const nanoem_motion_model_keyframe_t *keyframe =
                nanoemMotionFindModelKeyframeObject(project->resolveMotion(activeModel)->data(), 1337);
            CHECK(keyframe);
            CHECK_FALSE(nanoemMotionFindModelKeyframeObject(project->resolveMotion(other1Model)->data(), 1337));
            CHECK_FALSE(nanoemMotionFindModelKeyframeObject(project->resolveMotion(other2Model)->data(), 1337));
            CHECK_THAT(
                glm::make_vec4(nanoemMotionModelKeyframeGetEdgeColor(keyframe)), Equals(Vector4(0.9, 0.5, 0.3, 0.7)));
            CHECK(nanoemMotionModelKeyframeIsVisible(keyframe) == 1);
            CHECK(nanoemMotionModelKeyframeGetEdgeScaleFactor(keyframe) == Approx(0.5f));
            CHECK(first->motionDuration(activeModel) == 1337);
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
            CHECK(second->countAllModelKeyframes(activeModel2) == 2);
            const nanoem_motion_model_keyframe_t *keyframe =
                nanoemMotionFindModelKeyframeObject(project2->resolveMotion(activeModel2)->data(), 1337);
            CHECK(keyframe);
            CHECK_THAT(
                glm::make_vec4(nanoemMotionModelKeyframeGetEdgeColor(keyframe)), Equals(Vector4(0.9, 0.5, 0.3, 0.7)));
            CHECK(nanoemMotionModelKeyframeIsVisible(keyframe) == 1);
            CHECK(nanoemMotionModelKeyframeGetEdgeScaleFactor(keyframe) == Approx(0.5f));
            CHECK(second->motionDuration(activeModel2) == 1337);
            CHECK_FALSE(scope.hasAnyError());
        }
    }
}
