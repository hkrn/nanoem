/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/Accessory.h"

using namespace nanoem;

TEST_CASE("accessory_is_loadable", "[emapp][accessory]")
{
    CHECK(Accessory::isLoadableExtension("x"));
    CHECK(Accessory::isLoadableExtension("X"));
}
