/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/DefaultFileManager.h"

using namespace nanoem;
using namespace test;

TEST_CASE("project_save_binary_immediately_should_success", "[emapp][project]")
{
    TestScope scope;
    Application *application = scope.application();
    BX_UNUSED_1(application);
    ProjectPtr ptr = scope.createProject();
    DefaultFileManager manager(application);
    IFileManager *managerPtr = &manager;
    Error error;
    const URI &fileURI =
        URI::createFromFilePath(NANOEM_TEST_OUTPUT_PATH "/project_save_binary_immediately_should_success.nmm");
    CHECK(managerPtr->saveAsFile(fileURI, IFileManager::kDialogTypeSaveProjectFile, ptr->m_project, error));
    CHECK(!error.hasReason());
    scope.deleteFile(fileURI.absolutePathConstString());
}
