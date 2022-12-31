/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/ICamera.h"
#include "emapp/internal/CameraValueState.h"

using namespace nanoem;
using namespace nanoem::internal;
using namespace test;

TEST_CASE("camera_motion", "[emapp][camera]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        Project *project = o.get()->m_project;
        ICamera *camera = project->globalCamera();
        {
            Motion::FrameIndexList frames;
            frames.push_back(42);
            camera->setAngle(glm::radians(Vector3(71, 135, 42)));
            camera->setDistance(528);
            camera->setFovRadians(glm::radians(31.0f));
            camera->setLookAt(Vector3(132, 75, 93));
            CommandRegistrator registrator(project);
            registrator.registerAddCameraKeyframesCommand(frames, camera, project->cameraMotion());
        }
        {
            Motion::FrameIndexList frames;
            camera->setAngle(glm::radians(Vector3(42, 71, 135)));
            camera->setDistance(285);
            camera->setFovRadians(glm::radians(96.0f));
            camera->setLookAt(Vector3(93, 132, 75));
            frames.push_back(43);
            CommandRegistrator registrator(project);
            registrator.registerAddCameraKeyframesCommand(frames, camera, project->cameraMotion());
        }
        /* to prevent assertion */
        project->setPhysicsSimulationMode(PhysicsEngine::kSimulationModeDisable);
        project->seek(42, 0, true);
        CHECK_THAT(camera->angle(), EqualsRadians(Vector3(71, 135, 42)));
        CHECK(camera->distance() == Approx(528));
        CHECK(camera->fovRadians() == Approx(glm::radians(31.0f)));
        CHECK_THAT(camera->lookAt(), Equals(Vector3(132, 75, 93)));
        project->seek(42, 0.5f, true);
        CHECK_THAT(camera->angle(), EqualsRadians(Vector3(71, 135, 42)));
        CHECK(camera->distance() == Approx(528));
        CHECK(camera->fovRadians() == Approx(glm::radians(31.0f)));
        CHECK_THAT(camera->lookAt(), Equals(Vector3(132, 75, 93)));
    }
}
