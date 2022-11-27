/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/TimelineSegment.h"

#include "emapp/Project.h"

namespace nanoem {

TimelineSegment::TimelineSegment()
    : m_from(0)
    , m_to(0)
    , m_enableFrom(false)
    , m_enableTo(false)
{
}

TimelineSegment::TimelineSegment(const Project *project)
    : m_from(0)
    , m_to(project->duration())
    , m_enableFrom(false)
    , m_enableTo(false)
{
}

TimelineSegment
TimelineSegment::normalized(nanoem_frame_index_t duration) const NANOEM_DECL_NOEXCEPT
{
    TimelineSegment s(*this);
    if (m_from > m_to) {
        s.m_from = m_to;
        s.m_to = glm::min(m_from, duration);
    }
    else {
        s.m_to = glm::min(m_to, duration);
    }
    return s;
}

nanoem_frame_index_t
TimelineSegment::frameIndexFrom() const NANOEM_DECL_NOEXCEPT
{
    return m_from * m_enableFrom;
}

nanoem_frame_index_t
TimelineSegment::frameIndexTo(nanoem_frame_index_t duration) const NANOEM_DECL_NOEXCEPT
{
    return m_enableTo ? m_to : duration;
}

} /* namespace nanoem */
