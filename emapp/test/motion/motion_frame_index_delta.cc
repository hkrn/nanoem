/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/Motion.h"

using namespace nanoem;

TEST_CASE("motion_add_frame_delta", "[emapp][motion]")
{
    nanoem_frame_index_t newFrameIndex;
    CHECK(Motion::addFrameIndexDelta(9, 42, newFrameIndex));
    CHECK(newFrameIndex == 51);
    CHECK(Motion::addFrameIndexDelta(-9, 42, newFrameIndex));
    CHECK(newFrameIndex == 33);
    CHECK(Motion::addFrameIndexDelta(0x7fffffff, 0x80000000, newFrameIndex));
    CHECK(newFrameIndex == Motion::kMaxFrameIndex);
    CHECK(Motion::addFrameIndexDelta(-42, 42, newFrameIndex));
    CHECK(newFrameIndex == 0);
    CHECK_FALSE(Motion::addFrameIndexDelta(0, 42, newFrameIndex));
    CHECK_FALSE(Motion::addFrameIndexDelta(0x7fffffff, 0x80000001, newFrameIndex));
    CHECK_FALSE(Motion::addFrameIndexDelta(-43, 42, newFrameIndex));
}

TEST_CASE("motion_subtract_frame_delta", "[emapp][motion]")
{
    nanoem_frame_index_t newFrameIndex;
    CHECK(Motion::subtractFrameIndexDelta(9, 42, newFrameIndex));
    CHECK(newFrameIndex == 33);
    CHECK(Motion::subtractFrameIndexDelta(-9, 42, newFrameIndex));
    CHECK(newFrameIndex == 51);
    CHECK(Motion::subtractFrameIndexDelta(0x80000000, 0x7fffffff, newFrameIndex));
    CHECK(newFrameIndex == Motion::kMaxFrameIndex);
    CHECK(Motion::subtractFrameIndexDelta(42, 42, newFrameIndex));
    CHECK(newFrameIndex == 0);
    CHECK_FALSE(Motion::subtractFrameIndexDelta(0, 42, newFrameIndex));
    CHECK_FALSE(Motion::subtractFrameIndexDelta(0x80000000, 0x80000000, newFrameIndex));
    CHECK_FALSE(Motion::subtractFrameIndexDelta(43, 42, newFrameIndex));
}
