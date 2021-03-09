/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/IModalDialog.h"
#include "emapp/internal/DraggingBoneState.h"
#include "emapp/internal/project/Redo.h"

using namespace nanoem;
using namespace nanoem::internal;
using namespace test;

struct emapp_model_dragging_axis_aligned_orientate_bone_parameter_t {
    const int axisType;
    const Vector2 cursorFrom;
    const Vector2 cursorTo;
    const Vector3 expectedAxis;
    const nanoem_f32_t expectedAngle;
};

TEST_CASE("model_dragging_axis_aligned_orientate_bone", "[emapp][model]")
{
    const emapp_model_dragging_axis_aligned_orientate_bone_parameter_t parameters[] = {
        // move cursor to bottom-right
        { 0, Vector2(480, 240), Vector2(500, 260), Vector3(1.000003, 0.000000, 0.000000), 0.087266f } /*,
         { 1, Vector2(480, 240), Vector2(500, 260), Vector3(0.000000, 1.000003, 0.000000), 0.087266f },
         { 2, Vector2(480, 240), Vector2(500, 260), Vector3(0.000000, 0.000000, -1.000003), 0.087266f },
         // move cursor to top-left
         { 0, Vector2(480, 240), Vector2(460, 220), Vector3(-1.000003, 0.000000, 0.000000), 0.087266f },
         { 1, Vector2(480, 240), Vector2(460, 220), Vector3(0.000000, -1.000003, 0.000000), 0.087266f },
         { 2, Vector2(480, 240), Vector2(460, 220), Vector3(0.000000, 0.000000, 1.000003), 0.087266f } */
    };
    TestScope scope;
    for (const auto &parameter : parameters) {
        {
            ProjectPtr o = scope.createProject();
            Project *project = o->withRecoverable();
            Model *activeModel = o->createModel();
            project->addModel(activeModel);
            project->setActiveModel(activeModel);
            std::unique_ptr<DraggingBoneState> state(new AxisAlignedOrientateActiveBoneState(
                project, activeModel, parameter.cursorFrom, parameter.axisType));
            state->transform(parameter.cursorTo);
            state->commit(parameter.cursorTo);
            {
                const Quaternion q(model::Bone::cast(activeModel->activeBone())->localUserOrientation());
                CHECK_THAT(
                    model::Bone::cast(activeModel->activeBone())->localUserTranslation(), Equals(Constants::kZeroV3));
                CHECK(glm::all(glm::epsilonEqual(glm::axis(q), parameter.expectedAxis, Vector3(0.00001f))));
                CHECK(glm::angle(q) == Approx(parameter.expectedAngle));
            }
            project->handleUndoAction();
            {
                const Quaternion q(model::Bone::cast(activeModel->activeBone())->localUserOrientation());
                CHECK_THAT(
                    model::Bone::cast(activeModel->activeBone())->localUserTranslation(), Equals(Constants::kZeroV3));
                CHECK_THAT(glm::axis(q), Equals(Vector3(0, 0, 1)));
                CHECK(glm::angle(q) == Approx(0.0f));
            }
            project->handleRedoAction();
            {
                const Quaternion q(model::Bone::cast(activeModel->activeBone())->localUserOrientation());
                CHECK_THAT(
                    model::Bone::cast(activeModel->activeBone())->localUserTranslation(), Equals(Constants::kZeroV3));
                CHECK(glm::all(glm::epsilonEqual(glm::axis(q), parameter.expectedAxis, Vector3(0.00001f))));
                CHECK(glm::angle(q) == Approx(parameter.expectedAngle));
            }
        }
        {
            ProjectPtr o = scope.createProject();
            Project *project = o.get()->m_project;
            scope.recover(project);
            {
                Model *activeModel = project->activeModel();
                const Quaternion q(model::Bone::cast(activeModel->activeBone())->localUserOrientation());
                CHECK_THAT(
                    model::Bone::cast(activeModel->activeBone())->localUserTranslation(), Equals(Constants::kZeroV3));
                CHECK(glm::all(glm::epsilonEqual(glm::axis(q), parameter.expectedAxis, Vector3(0.00001f))));
                CHECK(glm::angle(q) == Approx(parameter.expectedAngle));
            }
        }
    }
}
