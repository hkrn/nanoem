/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/Model.h"

using namespace nanoem;
using namespace nanoem::internal;
using namespace test;

TEST_CASE("model_load_and_save_post_file", "[emapp][model]")
{
    TestScope scope;
    {
        Error error;
        ProjectPtr o = scope.createProject();
        Project *project = o.get()->m_project;
        Model *activeModel = o->createModel();
        project->addModel(activeModel);
        SECTION("header only should fail")
        {
            static const char content[] = "Vocaloid Pose Data\n";
            CHECK_FALSE(activeModel->loadPose(reinterpret_cast<const nanoem_u8_t *>(content), strlen(content), error));
            CHECK(error.hasReason());
            CHECK_THAT(error.reasonConstString(), Catch::Equals("Invalid .vpd header"));
        }
        SECTION("bone count is not number should fail")
        {
            static const char content[] = "Vocaloid Pose Data file\n"
                                          "target.osm;\n"
                                          "invalid_bone_number\n";
            CHECK_FALSE(activeModel->loadPose(reinterpret_cast<const nanoem_u8_t *>(content), strlen(content), error));
            CHECK(error.hasReason());
            CHECK_THAT(error.reasonConstString(), Catch::Equals("Bone count is not number"));
        }
        SECTION("C++ style one-liner comment can accept")
        {
            static const char content[] = "// This is a signature\n"
                                          "Vocaloid Pose Data file\n"
                                          "// This is target model name\n"
                                          "target.osm;\n"
                                          "// This is number of bones of the target model\n"
                                          "1;\n";
            CHECK(activeModel->loadPose(reinterpret_cast<const nanoem_u8_t *>(content), strlen(content), error));
            CHECK_FALSE(error.hasReason());
        }
        SECTION("saved pose should be able to load")
        {
            ByteArray bytes;
            MemoryWriter writer(&bytes);
            Error error;
            REQUIRE(activeModel->savePose(&writer, error));
            CHECK_FALSE(error.hasReason());
            CHECK(activeModel->loadPose(bytes, error));
            CHECK_FALSE(error.hasReason());
        }
    }
}
