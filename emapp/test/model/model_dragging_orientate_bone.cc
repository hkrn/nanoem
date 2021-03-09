/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/internal/DraggingBoneState.h"

using namespace nanoem;
using namespace nanoem::internal;
using namespace test;

struct emapp_model_dragging_orientate_bone_parameter_t {
    const Model::AxisType axisType;
    const Vector2 cursorFrom;
    const Vector2 cursorTo;
    const Vector3 expectedAxis;
    const nanoem_f32_t expectedAngle;
};

TEST_CASE("model_dragging_orientate_bone", "[emapp][model]")
{
    const emapp_model_dragging_orientate_bone_parameter_t parameters[] = {
        { Model::kAxisX, Vector2(480, 240), Vector2(500, 260), Vector3(0.999999, 0.000000, 0.000000), 0.349066f },
        { Model::kAxisZ, Vector2(480, 240), Vector2(500, 260), Vector3(0.000000, -0.000000, -0.999999), 0.349066f }
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
                new OrientateActiveBoneState(project, activeModel, parameter.cursorFrom, parameter.cursorFrom));
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
