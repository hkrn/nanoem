/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/internal/DraggingBoneState.h"

using namespace nanoem;
using namespace nanoem::internal;
using namespace test;

struct emapp_model_dragging_translate_bone_parameter_t {
    const Model::AxisType axisType;
    const Vector2 cursorFrom;
    const Vector2 cursorTo;
    const Vector3 expectedPosition;
};

TEST_CASE("model_dragging_translate_bone", "[emapp][model]")
{
    const emapp_model_dragging_translate_bone_parameter_t parameters[] = {
        { Model::kAxisX, Vector2(480, 240), Vector2(500, 260), Vector3(1.0048010, 0, 0) },
        { Model::kAxisY, Vector2(480, 240), Vector2(500, 260), Vector3(0, -1.004810, 0) },
        { Model::kAxisZ, Vector2(480, 240), Vector2(500, 260), Vector3(1.0048010, -1.004810, 0) }
    };
    TestScope scope;
    for (const auto &parameter : parameters) {
        {
            ProjectPtr o = scope.createProject();
            Project *project = o->withRecoverable();
            Model *activeModel = o->createModel();
            project->addModel(activeModel);
            project->setActiveModel(activeModel);
            activeModel->setTransformAxisType(parameter.axisType);
            std::unique_ptr<DraggingBoneState> state(
                new TranslateActiveBoneState(project, activeModel, parameter.cursorFrom, parameter.cursorFrom));
            state->transform(parameter.cursorTo);
            state->commit(parameter.cursorTo);
            {
                CHECK(glm::all(glm::epsilonEqual(model::Bone::cast(activeModel->activeBone())->localUserTranslation(),
                    parameter.expectedPosition, Vector3(0.00001f))));
                CHECK_THAT(
                    model::Bone::cast(activeModel->activeBone())->localUserOrientation(), Equals(Constants::kZeroQ));
            }
            project->handleUndoAction();
            {
                CHECK_THAT(
                    model::Bone::cast(activeModel->activeBone())->localUserTranslation(), Equals(Constants::kZeroV3));
                CHECK_THAT(
                    model::Bone::cast(activeModel->activeBone())->localUserOrientation(), Equals(Constants::kZeroQ));
            }
            project->handleRedoAction();
            {
                CHECK(glm::all(glm::epsilonEqual(model::Bone::cast(activeModel->activeBone())->localUserTranslation(),
                    parameter.expectedPosition, Vector3(0.00001f))));
                CHECK_THAT(
                    model::Bone::cast(activeModel->activeBone())->localUserOrientation(), Equals(Constants::kZeroQ));
            }
        }
        {
            ProjectPtr o = scope.createProject();
            Project *project = o.get()->m_project;
            scope.recover(project);
            {
                Model *activeModel = project->activeModel();
                CHECK(glm::all(glm::epsilonEqual(model::Bone::cast(activeModel->activeBone())->localUserTranslation(),
                    parameter.expectedPosition, Vector3(0.00001f))));
                CHECK_THAT(
                    model::Bone::cast(activeModel->activeBone())->localUserOrientation(), Equals(Constants::kZeroQ));
            }
        }
    }
}

TEST_CASE("model_dragging_translate_bone_sequential", "[emapp][model]")
{
    const emapp_model_dragging_translate_bone_parameter_t parameters[] = {
        { Model::kAxisX, Vector2(480, 240), Vector2(520, 280), Vector3(2.009621, 0, 0) },
        { Model::kAxisY, Vector2(480, 240), Vector2(520, 280), Vector3(0, -2.009621, 0) },
        { Model::kAxisZ, Vector2(480, 240), Vector2(520, 280), Vector3(2.009621, -2.009621, 0) }
    };
    TestScope scope;
    for (const auto &parameter : parameters) {
        {
            ProjectPtr o = scope.createProject();
            Project *project = o->withRecoverable();
            Model *activeModel = o->createModel();
            project->addModel(activeModel);
            project->setActiveModel(activeModel);
            activeModel->setTransformAxisType(parameter.axisType);
            std::unique_ptr<DraggingBoneState> state(
                new TranslateActiveBoneState(project, activeModel, parameter.cursorFrom, parameter.cursorFrom));
            state->transform(parameter.cursorTo - glm::vec2(-20));
            state->commit(parameter.cursorTo - glm::vec2(-20));
            state->transform(parameter.cursorTo);
            state->commit(parameter.cursorTo);
            {
                CHECK(glm::all(glm::epsilonEqual(model::Bone::cast(activeModel->activeBone())->localUserTranslation(),
                    parameter.expectedPosition, Vector3(0.00001f))));
                CHECK_THAT(
                    model::Bone::cast(activeModel->activeBone())->localUserOrientation(), Equals(Constants::kZeroQ));
            }
        }
        {
            ProjectPtr o = scope.createProject();
            Project *project = o.get()->m_project;
            scope.recover(project);
            {
                Model *activeModel = project->activeModel();
                CHECK(glm::all(glm::epsilonEqual(model::Bone::cast(activeModel->activeBone())->localUserTranslation(),
                    parameter.expectedPosition, Vector3(0.00001f))));
                CHECK_THAT(
                    model::Bone::cast(activeModel->activeBone())->localUserOrientation(), Equals(Constants::kZeroQ));
            }
        }
    }
}
