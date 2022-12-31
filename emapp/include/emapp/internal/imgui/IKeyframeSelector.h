/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_IKEYFRAMESELECTOR_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_IKEYFRAMESELECTOR_H_

#include "emapp/TimelineSegment.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct IKeyframeSelector {
    virtual void handleAction(const TimelineSegment &segment, int index) = 0;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_IKEYFRAMESELECTOR_H_ */
