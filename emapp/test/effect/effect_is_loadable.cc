/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/Effect.h"

using namespace nanoem;

TEST_CASE("effect_is_loadable", "[emapp][effect]")
{
    CHECK(Effect::isLoadableExtension("fx"));
    CHECK(Effect::isLoadableExtension("fX"));
    CHECK(Effect::isLoadableExtension("FX"));
    CHECK(Effect::isLoadableExtension("fxsub"));
    CHECK(Effect::isLoadableExtension("FxSub"));
    CHECK(Effect::isLoadableExtension("FXSUB"));
}
