/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "./project.h"

#include "emapp/Accessory.h"
#include "emapp/DirectionalLight.h"
#include "emapp/FileUtils.h"
#include "emapp/Grid.h"
#include "emapp/IAudioPlayer.h"
#include "emapp/IFileManager.h"
#include "emapp/Model.h"
#include "emapp/PerspectiveCamera.h"
#include "emapp/ShadowCamera.h"

using namespace nanoem;
using namespace test;

TEST_CASE("project_save_and_restore_should_not_restore_different", "[emapp][project]")
{
    TestScope scope;
    Application *application = scope.application();
    BX_UNUSED_1(application);
    Project::SaveState *state = nullptr;
    ProjectPtr first = scope.createProject();
    Accessory *accessory = first->createAccessory();
    first->m_project->addAccessory(accessory);
    first->m_project->setActiveAccessory(accessory);
    Model *model = first->createModel();
    first->m_project->addModel(model);
    first->m_project->setActiveModel(model);
    first->m_project->saveState(state);
    ProjectPtr second = scope.createProject();
    second->m_project->restoreState(state, false);
    second->m_project->destroyState(state);
    REQUIRE(second->m_project->activeAccessory() == nullptr);
    REQUIRE(second->m_project->activeModel() == nullptr);
}
