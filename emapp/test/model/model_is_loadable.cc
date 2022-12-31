/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/Model.h"

using namespace nanoem;

TEST_CASE("model_is_loadable", "[emapp][model]")
{
    CHECK(Model::isLoadableExtension("pmd"));
    CHECK(Model::isLoadableExtension("pmx"));
    CHECK(Model::isLoadableExtension("Pmd"));
    CHECK(Model::isLoadableExtension("Pmx"));
    CHECK(Model::isLoadableExtension("PMD"));
    CHECK(Model::isLoadableExtension("PMX"));
}
