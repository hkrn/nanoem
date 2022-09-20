/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#define CATCH_CONFIG_FAST_COMPILE
#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"

#include "emapp/Allocator.h"
#include "emapp/ThreadedApplicationService.h"

#include "bx/os.h"

#if defined(NANOEM_ENABLE_LOGGING)
#include "spdlog/cfg/env.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#endif /* NANOEM_ENABLE_LOGGING */

using namespace nanoem;

extern "C" int
nanoemTestSuiteRun(int argc, char *argv[])
{
    Allocator::initialize();
    ThreadedApplicationService::setup();
#if defined(NANOEM_ENABLE_LOGGING)
    spdlog::set_level(spdlog::level::off);
    spdlog::stdout_color_mt("emapp");
    spdlog::cfg::load_env_levels();
#endif /* NANOEM_ENABLE_LOGGING */
    void *dllHandle = sg::openSharedLibrary(
#ifdef CMAKE_INTDIR
        NANOEM_TEST_FIXTURE_PATH "/../../../emapp/bundle/sokol/" CMAKE_INTDIR "/sokol_noop." BX_DL_EXT
#else
        NANOEM_TEST_FIXTURE_PATH "/../../emapp/bundle/sokol/sokol_noop." BX_DL_EXT
#endif
    );
    sg_desc desc = {};
    desc.buffer_pool_size = 1024u;
    desc.image_pool_size = 4096u;
    desc.shader_pool_size = 1024u;
    desc.pipeline_pool_size = 1024u;
    desc.pass_pool_size = 512u;
    sg::setup(&desc);
    int result = Catch::Session().run(argc, argv);
    sg::shutdown();
    sg::closeSharedLibrary(dllHandle);
    ThreadedApplicationService::terminate();
    Allocator::destroy();
    return result;
}
