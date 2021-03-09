/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#if !defined(_WIN32)
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

TEST_CASE("project_save_twice_at_same_location_should_not_corrupt", "[emapp][project]")
{
    TestScope scope;
    Application *application = scope.application();
    {
        ProjectPtr first = scope.createProject();
        Project *project = first.get()->m_project;
        {
            project->setEffectPluginEnabled(true);
            for (int i = 0; i < 3; i++) {
                Accessory *accessory = first->createAccessory();
                project->addAccessory(accessory);
            }
            for (int i = 0; i < 3; i++) {
                Model *model = first->createModel();
                project->addModel(model);
            }
            Accessory *accessory = first->createAccessory("effects/offscreen.x");
            first->createSourceEffect(accessory, "effects/offscreen.fx");
            project->addAccessory(accessory);
            Model *model = first->createModel("effects/offscreen.pmx");
            model->setName("offscreen");
            model->rename("offscreen", NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
            first->createSourceEffect(model, "effects/offscreen.fx");
            project->addModel(model);
        }
        const URI &fileURI =
            URI::createFromFilePath(NANOEM_TEST_OUTPUT_PATH "/project_save_twice_at_same_location.nma");
        Error error;
        /* save project twice */
        {
            CHECK(application->fileManager()->saveAsFile(
                fileURI, IFileManager::kDialogTypeSaveProjectFile, project, error));
            CHECK_FALSE(error.hasReason());
            const FileUtils::TransientPath &transientPath = project->transientPath();
            CHECK(application->fileManager()->saveAsFile(
                fileURI, IFileManager::kDialogTypeSaveProjectFile, project, error));
            CHECK_FALSE(error.hasReason());
            CHECK_FALSE(FileUtils::exists(transientPath.m_path.c_str()));
        }
        ProjectPtr second = scope.createProject();
        Project *newProject = second.get()->m_project;
        CHECK(
            application->fileManager()->loadFromFile(fileURI, IFileManager::kDialogTypeOpenProject, newProject, error));
        CHECK_FALSE(error.hasReason());
        CHECK(newProject->allAccessories().size() == 4);
        CHECK(newProject->allModels().size() == 4);
        CHECK(newProject->drawableOrderList().size() == 8);
        CHECK(newProject->transformOrderList().size() == 4);
        const Project::AccessoryList &allAccessories = second->allAccessories();
        const Project::ModelList &allModels = second->allModels();
        REQUIRE(allAccessories.size() == 4);
        REQUIRE(allModels.size() == 4);
        CHECK(allAccessories[0]->fileURI().fragment() == String("Accessory/test/test.x"));
        CHECK(allAccessories[1]->fileURI().fragment() == String("Accessory/test-1/test.x"));
        CHECK(allAccessories[2]->fileURI().fragment() == String("Accessory/test-2/test.x"));
        CHECK(allAccessories[3]->fileURI().fragment() == String("Accessory/offscreen-1/offscreen.x"));
        CHECK(allAccessories[3]->activeEffect());
        CHECK(allModels[0]->fileURI().fragment() == modelPath(0, "test.pmx"));
        CHECK(allModels[1]->fileURI().fragment() == modelPath(1, "test.pmx"));
        CHECK(allModels[2]->fileURI().fragment() == modelPath(2, "test.pmx"));
        CHECK(allModels[3]->fileURI().fragment() == String("Model/offscreen/offscreen.pmx"));
        CHECK(allModels[3]->activeEffect());
        {
            IFileReader *reader = FileUtils::createFileReader(project->translator());
            Error error;
            REQUIRE(reader->open(fileURI, error));
            expectAllOffscreenEffectResourcesSame(reader);
            FileUtils::destroyFileReader(reader);
        }
        scope.deleteFile(fileURI.absolutePathConstString());
    }
}

#endif /* _WIN32 */
