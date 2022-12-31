/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/ILight.h"
#include "emapp/internal/LightValueState.h"

using namespace nanoem;
using namespace nanoem::internal;
using namespace test;

TEST_CASE("light_set_color", "[emapp][light]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        Project *project = o.get()->m_project;
        ILight *light = project->globalLight();
        light->setColor(Vector3(0.6, 0.3, 0.2));
        light->setDirection(Vector3(-0.2, 0.9, 0.6));
        std::unique_ptr<LightColorVectorValueState> state(new LightColorVectorValueState(project, light));
        state->setValue(glm::value_ptr(Vector3(0.3, 0.4, 0.8)));
        SECTION("commited and change color")
        {
            CHECK_THAT(light->color(), Equals(Vector3(0.3, 0.4, 0.8)));
            CHECK_THAT(light->direction(), Equals(Vector3(-0.2, 0.9, 0.6)));
        }
        SECTION("undo and rolled back color only")
        {
            state->commit();
            project->handleUndoAction();
            CHECK_THAT(light->color(), Equals(Vector3(0.6, 0.3, 0.2)));
            CHECK_THAT(light->direction(), Equals(Vector3(-0.2, 0.9, 0.6)));
        }
        SECTION("redo and commit again")
        {
            state->commit();
            project->handleUndoAction();
            project->handleRedoAction();
            CHECK_THAT(light->color(), Equals(Vector3(0.3, 0.4, 0.8)));
            CHECK_THAT(light->direction(), Equals(Vector3(-0.2, 0.9, 0.6)));
        }
    }
}
