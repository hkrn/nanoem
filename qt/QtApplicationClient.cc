/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "QtApplicationClient.h"

namespace nanoem {
namespace qt {

QtApplicationClient::QtApplicationClient(QtApplicationClient::Bridge *bridge)
    : m_bridge(bridge)
{
}

QtApplicationClient::~QtApplicationClient() noexcept
{
}

void
QtApplicationClient::consumeDispatchAllEventMessages()
{
    dispatchAllEventMessages();
    m_bridge->m_events.clear();
}

void
QtApplicationClient::dispatchAllEventMessages()
{
    for (ByteArrayList::const_iterator it = m_bridge->m_events.begin(), end = m_bridge->m_events.end(); it != end;
         ++it) {
        dispatchEventMessage(it->data(), it->size());
    }
}

void
QtApplicationClient::sendCommandMessage(const Nanoem__Application__Command *command)
{
    ByteArray bytes(sizeofCommandMessage(command));
    packCommandMessage(command, bytes.data());
    m_bridge->m_commands.push_back(bytes);
}

} /* namespace qt */
} /* namespace nanoem */
