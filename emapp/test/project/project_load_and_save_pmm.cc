/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/internal/project/PMM.h"

using namespace nanoem;
using namespace test;

TEST_CASE("project_load_empty_pmm_should_error", "[emapp][project]")
{
    TestScope scope;
    Error error;
    ProjectPtr first = scope.createProject();
    internal::project::PMM pmm(first->m_project);
    CHECK_FALSE(pmm.load(0, 0, error, nullptr));
    CHECK(error.hasReason());
    CHECK_THAT(error.reasonConstString(), Catch::Equals("Loading PMM error: Invalid signature"));
}

TEST_CASE("project_load_pmm", "[emapp][project][test]")
{
    TestScope scope;
    Error error;
    ProjectPtr first = scope.createProject();
    Project *project = first->m_project;
    project->setFileURI(URI::createFromFilePath("standard.pmm"));
    ByteArray bytes;
    {
        FileReaderScope scope(project->translator());
        Error error;
        scope.open(project->fileURI(), error);
        FileUtils::read(scope, bytes, error);
    }
    internal::project::PMM pmm(project);
    pmm.load(bytes.data(), bytes.size(), error, nullptr);
}
