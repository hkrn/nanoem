/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/LazyPublishCanPasteEventCommand.h"

#include "emapp/IEventPublisher.h"

namespace nanoem {
namespace internal {
namespace imgui {

LazyPublishCanPasteEventCommand::LazyPublishCanPasteEventCommand(bool value)
    : m_value(value)
{
}

void
LazyPublishCanPasteEventCommand::execute(Project *project)
{
    project->eventPublisher()->publishCanPasteEvent(m_value);
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
