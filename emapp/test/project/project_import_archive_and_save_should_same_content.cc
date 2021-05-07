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

TEST_CASE("project_import_archive_and_save_should_same_content", "[emapp][project]")
{
    TestScope scope;
    Application *application = scope.application();
    ByteArray bytes;
    MemoryWriter writer(&bytes);
    Error error;
    {
        ProjectPtr first = scope.createProject();
        Project *project = first.get()->m_project;
        REQUIRE(application->fileManager()->loadFromFile(
            URI::createFromFilePath(NANOEM_TEST_FIXTURE_PATH "/test.zip", "test/foo/bar/baz/accessory/test.x"),
            IFileManager::kDialogTypeLoadModelFile, project, error));
        REQUIRE(application->fileManager()->loadFromFile(
            URI::createFromFilePath(NANOEM_TEST_FIXTURE_PATH "/test.zip", "test/foo/bar/baz/model/test.pmx"),
            IFileManager::kDialogTypeLoadModelFile, project, error));
        CHECK_FALSE(application->hasModalDialog());
        project->setFileURI(URI::createFromFilePath(NANOEM_TEST_FIXTURE_PATH "/test.zip"));
        CHECK(project->saveAsArchive(&writer, error));
    }
    {
        ProjectPtr first = scope.createProject();
        Project *newProject = first.get()->m_project;
        {
            MemoryReader reader(&bytes);
            const URI &fileURI =
                URI::createFromFilePath(NANOEM_TEST_OUTPUT_PATH "/project_import_archive_and_save.nma");
            CHECK(newProject->loadFromArchive(&reader, fileURI, error));
            CHECK_FALSE(error.hasReason());
        }
        CHECK_FALSE(newProject->isDirty());
        CHECK(newProject->coordinationSystem() == GLM_LEFT_HANDED);
        CHECK(newProject->allAccessories()->size() == 1);
        CHECK(newProject->allModels()->size() == 1);
        CHECK(newProject->drawableOrderList()->size() == 2);
        CHECK(newProject->transformOrderList()->size() == 1);
        const Project::AccessoryList &allAccessories = first->allAccessories();
        const Project::ModelList &allModels = first->allModels();
        REQUIRE(allAccessories.size() == 1);
        REQUIRE(allModels.size() == 1);
        CHECK(allAccessories[0]->fileURI().fragment() == String("Accessory/test/test.x"));
        CHECK(allModels[0]->fileURI().fragment() == modelPath(0, "test.pmx"));
        {
            MemoryReader reader(&bytes);
            Archiver archiver(&reader);
            Archiver::Entry entry;
            ByteArray entryData;
            Error error;
            REQUIRE(archiver.open(error));
            {
                CHECK(archiver.findEntry("Motion/Camera.nmd", entry, error));
                CHECK(archiver.extract(entry, entryData, error));
                CHECK(validateMotion(entryData, newProject));
            }
            {
                CHECK(archiver.findEntry("Motion/Light.nmd", entry, error));
                CHECK(archiver.extract(entry, entryData, error));
                CHECK(validateMotion(entryData, newProject));
            }
            {
                CHECK(archiver.findEntry("Motion/test.nmd", entry, error));
                CHECK(archiver.extract(entry, entryData, error));
                CHECK(validateMotion(entryData, newProject));
            }
            {
                CHECK(archiver.findEntry(modelMotionPath(0), entry, error));
                CHECK(archiver.extract(entry, entryData, error));
                CHECK(validateMotion(entryData, newProject));
            }
            REQUIRE(archiver.close(error));
        }
    }
}
