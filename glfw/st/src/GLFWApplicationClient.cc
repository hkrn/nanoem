/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "GLFWApplicationClient.h"

namespace nanoem {
namespace glfw {

GLFWApplicationClient::GLFWApplicationClient(GLFWApplicationClient::Bridge *bridge)
    : m_bridge(bridge)
{
}

GLFWApplicationClient::~GLFWApplicationClient() noexcept
{
}

void
GLFWApplicationClient::consumeDispatchAllEventMessages()
{
    dispatchAllEventMessages();
    m_bridge->m_events.clear();
}

void
GLFWApplicationClient::dispatchAllEventMessages()
{
    for (ByteArrayList::const_iterator it = m_bridge->m_events.begin(), end = m_bridge->m_events.end(); it != end;
         ++it) {
        dispatchEventMessage(it->data(), it->size());
        /* update end iterator for added event */
        end = m_bridge->m_events.end();
    }
}

void
GLFWApplicationClient::sendCommandMessage(const Nanoem__Application__Command *command)
{
    ByteArray bytes(sizeofCommandMessage(command));
    packCommandMessage(command, bytes.data());
    m_bridge->m_commands.push_back(bytes);
}

} /* namespace glfw */
} /* namespace nanoem */
