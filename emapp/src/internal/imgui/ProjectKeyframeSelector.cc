/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/ProjectKeyframeSelector.h"

#include "emapp/Accessory.h"
#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/ITrack.h"
#include "emapp/Project.h"
#include "emapp/private/CommonInclude.h"

#include "imgui/imgui.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *
ProjectKeyframeSelector::callback(void *userData, int index) NANOEM_DECL_NOEXCEPT
{
    const ProjectKeyframeSelector *self = static_cast<const ProjectKeyframeSelector *>(userData);
    return self->select(index);
}

ProjectKeyframeSelector::ProjectKeyframeSelector(Project *project)
    : m_project(project)
{
    const TrackList *tracks = project->allTracks();
    for (Project::TrackList::const_iterator it = tracks->begin(), end = tracks->end(); it != end; ++it) {
        ITrack *track = *it;
        if (track->type() == ITrack::kTypeAccessory) {
            m_accessories.push_back(static_cast<Accessory *>(track->opaque()));
        }
    }
}

bool
ProjectKeyframeSelector::combo(int *index)
{
    return ImGui::Combo("##timeline.select.project", index, callback, this, count());
}

void
ProjectKeyframeSelector::handleAction(const TimelineSegment &segment, int index)
{
    switch (index) {
    case kSelectTypeCamera: {
        m_project->cameraMotion()->selection()->addCameraKeyframes(segment.m_from, segment.m_to);
        break;
    }
    case kSelectTypeLight: {
        m_project->lightMotion()->selection()->addLightKeyframes(segment.m_from, segment.m_to);
        break;
    }
    case kSelectTypeSelfShadow: {
        m_project->selfShadowMotion()->selection()->addSelfShadowKeyframes(segment.m_from, segment.m_to);
        break;
    }
    default:
        if (index < count()) {
            int offset = index - kSelectTypeMaxEnum;
            Accessory *accessory = m_accessories[offset];
            m_project->resolveMotion(accessory)->selection()->addAccessoryKeyframes(segment.m_from, segment.m_to);
        }
        break;
    }
}

const char *
ProjectKeyframeSelector::select(int index) const NANOEM_DECL_NOEXCEPT
{
    const char *name = nullptr;
    switch (index) {
    case kSelectTypeCamera: {
        name = m_project->translator()->translate("nanoem.project.track.camera");
        break;
    }
    case kSelectTypeLight: {
        name = m_project->translator()->translate("nanoem.project.track.light");
        break;
    }
    case kSelectTypeSelfShadow: {
        name = m_project->translator()->translate("nanoem.project.track.self-shadow");
        break;
    }
    default:
        if (index < count()) {
            int offset = index - kSelectTypeMaxEnum;
            name = m_accessories[offset]->nameConstString();
        }
        else {
            name = "(Unknown)";
        }
        break;
    }
    return name;
}

int
ProjectKeyframeSelector::count() const NANOEM_DECL_NOEXCEPT
{
    return Inline::saturateInt32(m_accessories.size() + kSelectTypeMaxEnum);
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
