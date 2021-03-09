
/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_PROJECTKEYFRAMESELECTOR_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_PROJECTKEYFRAMESELECTOR_H_

#include "emapp/internal/imgui/IKeyframeSelector.h"

namespace nanoem {

class Accessory;
class Project;

namespace internal {
namespace imgui {

struct ProjectKeyframeSelector : IKeyframeSelector {
    enum SelectType {
        kSelectTypeCamera,
        kSelectTypeLight,
        kSelectTypeSelfShadow,
        kSelectTypeMaxEnum,
    };

    static bool callback(void *userData, int index, const char **out) NANOEM_DECL_NOEXCEPT;
    ProjectKeyframeSelector(Project *project);
    bool combo(int *index);
    void handleAction(const TimelineSegment &segment, int index) NANOEM_DECL_OVERRIDE;
    bool select(int index, const char **out) const NANOEM_DECL_NOEXCEPT;
    int count() const NANOEM_DECL_NOEXCEPT;

    typedef tinystl::vector<Accessory *, TinySTLAllocator> AccessoryList;
    Project *m_project;
    AccessoryList m_accessories;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_PROJECTKEYFRAMESELECTOR_H_ */
