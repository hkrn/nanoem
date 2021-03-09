/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/DefaultFileManager.h"
#include "emapp/internal/project/Native.h"

using namespace nanoem;
using namespace test;

TEST_CASE("project_load_vmd_from_binary_should_success", "[emapp][project]")
{
    TestScope scope;
    Application *application = scope.application();
    BX_UNUSED_1(application);
    ProjectPtr ptr = scope.createProject();
    Project *project = ptr->m_project;
    Error error;
    ByteArray bytes;
    internal::project::Native saver(project);
    saver.setDefaultSaveMotionFormat(NANOEM_MOTION_FORMAT_TYPE_VMD);
    saver.save(bytes, internal::project::Native::kFileTypeData, error);
    internal::project::Native loader(project);
    loader.load(bytes.data(), bytes.size(), internal::project::Native::kFileTypeData, error, nullptr);
    CHECK(!error.hasReason());
    /* resave also should success */
    saver.setDefaultSaveMotionFormat(NANOEM_MOTION_FORMAT_TYPE_NMD);
    saver.save(bytes, internal::project::Native::kFileTypeData, error);
    loader.load(bytes.data(), bytes.size(), internal::project::Native::kFileTypeData, error, nullptr);
    CHECK(!error.hasReason());
}
