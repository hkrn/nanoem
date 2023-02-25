/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/Constants.h"
#include "emapp/internal/DraggingBoneState.h"

using namespace nanoem;
using namespace nanoem::internal;
using namespace test;

struct emapp_model_dragging_axis_aligned_translate_bone_parameter_t {
    const int axisType;
    const Vector2 cursorFrom;
    const Vector2 cursorTo;
    const Vector3 expectedPosition;
};

TEST_CASE("model_dragging_axis_aligned_translate_bone", "[emapp][model]")
{
    const emapp_model_dragging_axis_aligned_translate_bone_parameter_t parameters[] = { // move cursor to bottom-right
        { 0, Vector2(480, 240), Vector2(500, 260), Vector3(-1.000000, 0, 0) },
        { 1, Vector2(480, 240), Vector2(500, 260), Vector3(0, -1.000000, 0) },
        { 2, Vector2(480, 240), Vector2(500, 260), Vector3(0, 0, -1.000000) },
        // move cursor to top-left
        { 0, Vector2(480, 240), Vector2(460, 220), Vector3(1.000000, 0, 0) },
        { 1, Vector2(480, 240), Vector2(460, 220), Vector3(0, 1.000000, 0) },
        { 2, Vector2(480, 240), Vector2(460, 220), Vector3(0, 0, 1.000000) }
    };
    TestScope scope;
    for (const auto &parameter : parameters) {
        {
            ProjectPtr o = scope.createProject();
            Project *project = o->withRecoverable();
            Model *activeModel = o->createModel();
            project->addModel(activeModel);
            project->setActiveModel(activeModel);
            std::unique_ptr<DraggingBoneState> state(new AxisAlignedTranslateActiveBoneState(
                project, activeModel, parameter.cursorFrom, parameter.axisType));
            state->transform(parameter.cursorTo);
            state->commit(parameter.cursorTo);
            {
                CHECK_THAT(model::Bone::cast(activeModel->activeBone())->localUserTranslation(),
                    Equals(parameter.expectedPosition));
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
                CHECK_THAT(model::Bone::cast(activeModel->activeBone())->localUserTranslation(),
                    Equals(parameter.expectedPosition));
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
                CHECK_THAT(model::Bone::cast(activeModel->activeBone())->localUserTranslation(),
                    Equals(parameter.expectedPosition));
                CHECK_THAT(
                    model::Bone::cast(activeModel->activeBone())->localUserOrientation(), Equals(Constants::kZeroQ));
            }
        }
    }
}
