/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/Motion.h"

using namespace nanoem;

TEST_CASE("motion_is_loadable", "[emapp][motion]")
{
    CHECK(Motion::isLoadableExtension("vmd"));
    CHECK(Motion::isLoadableExtension("nmd"));
    CHECK(Motion::isLoadableExtension("Vmd"));
    CHECK(Motion::isLoadableExtension("Nmd"));
    CHECK(Motion::isLoadableExtension("VMD"));
    CHECK(Motion::isLoadableExtension("NMD"));
}
