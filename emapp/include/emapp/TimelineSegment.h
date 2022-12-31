/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_TIMELINESEGMENT_H_
#define NANOEM_EMAPP_TIMELINESEGMENT_H_

#include "emapp/Forward.h"

namespace nanoem {

class Project;

struct TimelineSegment {
    TimelineSegment();
    TimelineSegment(const Project *project);
    TimelineSegment normalized(nanoem_frame_index_t duration) const NANOEM_DECL_NOEXCEPT;
    nanoem_frame_index_t frameIndexFrom() const NANOEM_DECL_NOEXCEPT;
    nanoem_frame_index_t frameIndexTo(nanoem_frame_index_t duration) const NANOEM_DECL_NOEXCEPT;
    nanoem_frame_index_t m_from;
    nanoem_frame_index_t m_to;
    bool m_enableFrom;
    bool m_enableTo;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_TIMELINESEGMENT_H_ */
