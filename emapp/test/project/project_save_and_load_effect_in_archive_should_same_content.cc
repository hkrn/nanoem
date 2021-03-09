/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "./project.h"

#include "emapp/Accessory.h"
#include "emapp/DirectionalLight.h"
#include "emapp/FileUtils.h"
#include "emapp/IAudioPlayer.h"
#include "emapp/IFileManager.h"
#include "emapp/Model.h"
#include "emapp/PerspectiveCamera.h"
#include "emapp/ShadowCamera.h"

using namespace nanoem;
using namespace test;

TEST_CASE("project_save_and_load_effect_in_archive_should_same_content", "[emapp][project]")
{
    const URI &fileURI =
        URI::createFromFilePath(NANOEM_TEST_OUTPUT_PATH "/project_save_and_load_effect_in_archive.nma");
    TestScope scope;
    ByteArray bytes;
    MemoryWriter writer(&bytes);
    Error error;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first.get()->m_project;
        project->setEffectPluginEnabled(true);
        {
            Accessory *accessory = first->createAccessory("effects/main.x");
            first->createSourceEffect(accessory, "effects/main.fx");
            project->addAccessory(accessory);
        }
        {
            Accessory *accessory = first->createAccessory("effects/offscreen.x");
            first->createSourceEffect(accessory, "effects/offscreen.fx");
            project->addAccessory(accessory);
        }
        {
            Model *model = first->createModel("effects/main.pmx");
            model->setName("main");
            model->rename("main", NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
            first->createSourceEffect(model, "effects/main.fx");
            project->addModel(model);
        }
        {
            Model *model = first->createModel("effects/offscreen.pmx");
            model->setName("offscreen");
            model->rename("offscreen", NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
            first->createSourceEffect(model, "effects/offscreen.fx");
            project->addModel(model);
        }
        project->setFileURI(fileURI);
        CHECK(project->saveAsArchive(&writer, error));
    }
    {
        ProjectPtr first = scope.createProject();
        Project *newProject = first.get()->m_project;
        {
            MemoryReader reader(&bytes);
            CHECK(newProject->loadFromArchive(&reader, fileURI, error));
            CHECK_FALSE(error.hasReason());
            CHECK(newProject->isEffectPluginEnabled());
            CHECK_FALSE(newProject->isDirty());
            CHECK(newProject->coordinationSystem() == GLM_LEFT_HANDED);
            CHECK(newProject->allAccessories().size() == 2);
            CHECK(newProject->allModels().size() == 2);
            CHECK(newProject->drawableOrderList().size() == 4);
            CHECK(newProject->transformOrderList().size() == 2);
            const Project::AccessoryList &allAccessories = first->allAccessories();
            const Project::ModelList &allModels = first->allModels();
            REQUIRE(allAccessories.size() == 2);
            REQUIRE(allModels.size() == 2);
            {
                CHECK(allAccessories[0]->fileURI().fragment() == String("Accessory/main-1/main.x"));
                CHECK(allAccessories[0]->activeEffect());
            }
            {
                CHECK(allAccessories[1]->fileURI().fragment() == String("Accessory/offscreen-1/offscreen.x"));
                CHECK(allAccessories[1]->activeEffect());
            }
            {
                CHECK(allModels[0]->fileURI().fragment() == String("Model/main/main.pmx"));
                CHECK(allModels[0]->activeEffect());
            }
            {
                CHECK(allModels[1]->fileURI().fragment() == String("Model/offscreen/offscreen.pmx"));
                CHECK(allModels[1]->activeEffect());
            }
        }
        MemoryReader reader(&bytes);
        Archiver archiver(&reader);
        Archiver::Entry entry;
        ByteArray bytes;
        Error error;
        REQUIRE(archiver.open(error));
        expectAllMainEffectResourcesSame(archiver);
        expectAllOffscreenEffectResourcesSame(archiver);
        REQUIRE(archiver.close(error));
    }
}
