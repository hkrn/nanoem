/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_GLFW_GLFWAPPLICATIONCLIENT_H_
#define NANOEM_EMAPP_GLFW_GLFWAPPLICATIONCLIENT_H_

#include "emapp/BaseApplicationClient.h"

namespace nanoem {
namespace glfw {

class GLFWApplicationClient final : public BaseApplicationClient {
public:
    struct Bridge {
        ByteArrayList m_commands;
        ByteArrayList m_events;
    };

    GLFWApplicationClient(Bridge *bridge);
    ~GLFWApplicationClient() noexcept;

    void consumeDispatchAllEventMessages();
    void dispatchAllEventMessages();

private:
    void sendCommandMessage(const Nanoem__Application__Command *command) override;

    Bridge *m_bridge;
};

} /* namespace glfw */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_GLFW_GLFWAPPLICATIONCLIENT_H_ */
