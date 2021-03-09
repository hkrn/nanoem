/*
   Copyright (c) 2015-2021 hkrn All rights reserved

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

bool
ProjectKeyframeSelector::callback(void *userData, int index, const char **out) NANOEM_DECL_NOEXCEPT
{
    const ProjectKeyframeSelector *self = static_cast<const ProjectKeyframeSelector *>(userData);
    return self->select(index, out);
}

ProjectKeyframeSelector::ProjectKeyframeSelector(Project *project)
    : m_project(project)
{
    TrackList tracks(project->allTracks());
    for (Project::TrackList::const_iterator it = tracks.begin(), end = tracks.end(); it != end; ++it) {
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

bool
ProjectKeyframeSelector::select(int index, const char **out) const NANOEM_DECL_NOEXCEPT
{
    switch (index) {
    case kSelectTypeCamera: {
        *out = m_project->translator()->translate("nanoem.project.track.camera");
        break;
    }
    case kSelectTypeLight: {
        *out = m_project->translator()->translate("nanoem.project.track.light");
        break;
    }
    case kSelectTypeSelfShadow: {
        *out = m_project->translator()->translate("nanoem.project.track.self-shadow");
        break;
    }
    default:
        if (index < count()) {
            int offset = index - kSelectTypeMaxEnum;
            *out = m_accessories[offset]->nameConstString();
        }
        else {
            *out = "(Unknown)";
        }
        break;
    }
    return true;
}

int
ProjectKeyframeSelector::count() const NANOEM_DECL_NOEXCEPT
{
    return Inline::saturateInt32(m_accessories.size() + kSelectTypeMaxEnum);
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
