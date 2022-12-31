/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_MODELKEYFRAMESELECTOR_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_MODELKEYFRAMESELECTOR_H_

#include "emapp/TimelineSegment.h"
#include "emapp/internal/imgui/IKeyframeSelector.h"
#include "emapp/model/Bone.h"
#include "emapp/model/Morph.h"

namespace nanoem {

class IMotionKeyframeSelection;
class ITrack;
class Project;

namespace internal {
namespace imgui {

struct ModelKeyframeSelector : IKeyframeSelector {
    enum SelectType {
        kSelectTypeAll,
        kSelectTypeLabelRoot,
        kSelectTypeModel,
        kSelectTypeBones,
        kSelectTypeMorphs,
        kSelectTypeAllMorphs,
        kSelectTypeMaxEnum,
    };

    static bool callback(void *userData, int index, const char **out) NANOEM_DECL_NOEXCEPT;
    ModelKeyframeSelector(Project *project);
    bool combo(int *index);
    void handleAction(const TimelineSegment &segment, int index) NANOEM_DECL_OVERRIDE;
    bool select(int index, const char **out) const NANOEM_DECL_NOEXCEPT;
    int count() const NANOEM_DECL_NOEXCEPT;
    void selectRootTrack(const TimelineSegment &segment, IMotionKeyframeSelection *selection);

    const ITrack *m_rootTrack;
    Project *m_project;
    model::Bone::List m_bones;
    model::Morph::List m_morphs;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_MODELKEYFRAMESELECTOR_H_ */
