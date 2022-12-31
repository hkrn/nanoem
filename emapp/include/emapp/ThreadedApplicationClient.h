/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_THREADEDAPPLICATIONCLIENT_H_
#define NANOEM_EMAPP_THREADEDAPPLICATIONCLIENT_H_

#include "emapp/BaseApplicationClient.h"

namespace nanoem {

class ThreadedApplicationClient NANOEM_DECL_SEALED : public BaseApplicationClient {
public:
    ThreadedApplicationClient();
    ~ThreadedApplicationClient() NANOEM_DECL_NOEXCEPT;

    void connect();
    void receiveAllEventMessages();
    void close();

private:
    void sendCommandMessage(const Nanoem__Application__Command *command) NANOEM_DECL_OVERRIDE;
    void handleSocketError(const char *prefix);

    int m_commandStreamSocket;
    int m_eventStreamSocket;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_THREADEDAPPLICATIONCLIENT_H_ */
