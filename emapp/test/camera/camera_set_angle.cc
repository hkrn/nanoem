/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/PerspectiveCamera.h"
#include "emapp/internal/CameraValueState.h"

using namespace nanoem;
using namespace nanoem::internal;
using namespace test;

TEST_CASE("camera_set_angle", "[emapp][camera]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        Project *project = o.get()->m_project;
        ICamera *camera = project->globalCamera();
        camera->setAngle(glm::radians(Vector3(71, 135, 42)));
        CHECK(camera->isDirty());
        CHECK_THAT(camera->angle(), Equals(glm::radians(Vector3(71, 135, 42))));
        camera->setDirty(false);
        camera->setAngle(glm::radians(Vector3(71, 135, 42)));
        // same value
        CHECK_FALSE(camera->isDirty());
        camera->reset();
        camera->setDirty(false);
        camera->setLocked(true);
        camera->setAngle(glm::radians(Vector3(71, 135, 42)));
        // locked
        CHECK_FALSE(camera->isDirty());
        CHECK_THAT(camera->lookAt(), Equals(PerspectiveCamera::kInitialLookAt));
    }
}

TEST_CASE("camera_set_angle_redo", "[emapp][camera]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        Project *project = o.get()->m_project;
        ICamera *camera = project->globalCamera();
        camera->setAngle(glm::radians(Vector3(71, 135, 42)));
        camera->setDistance(528);
        camera->setFovRadians(glm::radians(31.0f));
        camera->setLookAt(Vector3(132, 75, 93));
        std::unique_ptr<CameraAngleVectorValueState> state(new CameraAngleVectorValueState(project, camera));
        state->setValue(glm::value_ptr(glm::radians(Vector3(38, 14, 22))));
        SECTION("commited and change angle")
        {
            state->commit();
            CHECK_THAT(camera->angle(), EqualsRadians(Vector3(38, 14, 22)));
            REQUIRE(camera->distance() == Approx(528));
            REQUIRE(camera->fovRadians() == Approx(glm::radians(31.0f)));
            CHECK_THAT(camera->lookAt(), Equals(Vector3(132, 75, 93)));
        }
        SECTION("undo and rolled back angle only")
        {
            state->commit();
            project->handleUndoAction();
            CHECK_THAT(camera->angle(), EqualsRadians(Vector3(71, 135, 42)));
            REQUIRE(camera->distance() == Approx(528));
            REQUIRE(camera->fovRadians() == Approx(glm::radians(31.0f)));
            CHECK_THAT(camera->lookAt(), Equals(Vector3(132, 75, 93)));
        }
        SECTION("redo and commit again")
        {
            state->commit();
            project->handleUndoAction();
            project->handleRedoAction();
            CHECK_THAT(camera->angle(), EqualsRadians(Vector3(38, 14, 22)));
            REQUIRE(camera->distance() == Approx(528));
            REQUIRE(camera->fovRadians() == Approx(glm::radians(31.0f)));
            CHECK_THAT(camera->lookAt(), Equals(Vector3(132, 75, 93)));
        }
    }
}
