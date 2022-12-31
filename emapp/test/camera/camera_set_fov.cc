/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/PerspectiveCamera.h"
#include "emapp/internal/CameraValueState.h"

using namespace nanoem;
using namespace nanoem::internal;
using namespace test;

TEST_CASE("camera_set_fov", "[emapp][camera]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        Project *project = o.get()->m_project;
        ICamera *camera = project->globalCamera();
        camera->setFov(31);
        CHECK(camera->isDirty());
        CHECK(camera->fov() == 31);
        CHECK(camera->fovRadians() == Approx(glm::radians(31.0f)));
        camera->setDirty(false);
        camera->setFov(31);
        // same value
        CHECK_FALSE(camera->isDirty());
        camera->reset();
        camera->setDirty(false);
        camera->setLocked(true);
        camera->setFov(31);
        // locked
        CHECK_FALSE(camera->isDirty());
        CHECK(camera->fov() == PerspectiveCamera::kInitialFov);
        CHECK(camera->fovRadians() == Approx(PerspectiveCamera::kInitialFovRadian));
    }
}

TEST_CASE("camera_set_fov_redo", "[emapp][camera]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        Project *project = o.get()->m_project;
        ICamera *camera = project->globalCamera();
        camera->setAngle(glm::radians(Vector3(71, 135, 42)));
        camera->setDistance(528);
        camera->setFov(31);
        camera->setLookAt(Vector3(132, 75, 93));
        std::unique_ptr<CameraFovVectorValueState> state(new CameraFovVectorValueState(project, camera));
        state->setValue(glm::value_ptr(Vector4(44.0f)));
        SECTION("commited and change fov")
        {
            state->commit();
            CHECK_THAT(camera->angle(), EqualsRadians(Vector3(71, 135, 42)));
            REQUIRE(camera->distance() == Approx(528));
            REQUIRE(camera->fovRadians() == Approx(glm::radians(44.0f)));
            CHECK_THAT(camera->lookAt(), Equals(Vector3(132, 75, 93)));
        }
        SECTION("undo and rolled back distance only")
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
            CHECK_THAT(camera->angle(), EqualsRadians(Vector3(71, 135, 42)));
            REQUIRE(camera->distance() == Approx(528));
            REQUIRE(camera->fovRadians() == Approx(glm::radians(44.0f)));
            CHECK_THAT(camera->lookAt(), Equals(Vector3(132, 75, 93)));
        }
    }
}
