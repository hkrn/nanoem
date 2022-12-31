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

TEST_CASE("camera_set_distance", "[emapp][camera]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        Project *project = o.get()->m_project;
        ICamera *camera = project->globalCamera();
        camera->setDistance(528);
        CHECK(camera->isDirty());
        CHECK(camera->distance() == Approx(528.0f));
        camera->setDirty(false);
        camera->setDistance(528);
        // same value
        CHECK_FALSE(camera->isDirty());
        camera->reset();
        camera->setDirty(false);
        camera->setLocked(true);
        camera->setDistance(528);
        // locked
        CHECK_FALSE(camera->isDirty());
        CHECK(camera->distance() == Approx(PerspectiveCamera::kInitialDistance));
    }
}

TEST_CASE("camera_set_distance_redo", "[emapp][camera]")
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
        std::unique_ptr<CameraDistanceVectorValueState> state(new CameraDistanceVectorValueState(project, camera));
        state->setValue(glm::value_ptr(Vector4(313.0f)));
        SECTION("commited and change distance")
        {
            state->commit();
            CHECK_THAT(camera->angle(), EqualsRadians(Vector3(71, 135, 42)));
            REQUIRE(camera->distance() == Approx(313));
            REQUIRE(camera->fovRadians() == Approx(glm::radians(31.0f)));
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
            REQUIRE(camera->distance() == Approx(313));
            REQUIRE(camera->fovRadians() == Approx(glm::radians(31.0f)));
            CHECK_THAT(camera->lookAt(), Equals(Vector3(132, 75, 93)));
        }
    }
}
