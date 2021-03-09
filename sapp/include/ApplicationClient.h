/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_SAPP_APPLICAIONCLIENT_H_
#define NANOEM_EMAPP_SAPP_APPLICAIONCLIENT_H_

#include "emapp/BaseApplicationClient.h"

namespace nanoem {
namespace sapp {

class ApplicationClient final : public BaseApplicationClient {
public:
    struct Bridge {
        ByteArrayList m_commands;
        ByteArrayList m_events;
    };

    ApplicationClient(Bridge *bridge);
    ~ApplicationClient() noexcept;

    void consumeDispatchAllEventMessages();
    void dispatchAllEventMessages();

private:
    void sendCommandMessage(const Nanoem__Application__Command *command) override;

    Bridge *m_bridge;
};

} /* namespace sapp */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_SAPP_APPLICAIONCLIENT_H_ */
