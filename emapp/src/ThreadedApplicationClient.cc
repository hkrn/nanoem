/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/ThreadedApplicationClient.h"

#if defined(NANOEM_ENABLE_NANOMSG)

#include "./protoc/application.pb-c.h"
#include "emapp/ThreadedApplicationService.h"

#include "bx/debug.h"
#include "sokol/sokol_time.h"

#define NN_STATIC_LIB
#include "nanomsg/nn.h"
#include "nanomsg/pubsub.h"

namespace nanoem {

ThreadedApplicationClient::ThreadedApplicationClient()
    : m_commandStreamSocket(-1)
    , m_eventStreamSocket(-1)
{
}

ThreadedApplicationClient::~ThreadedApplicationClient() NANOEM_DECL_NOEXCEPT
{
    close();
}

void
ThreadedApplicationClient::connect()
{
    if (m_commandStreamSocket == -1 && m_eventStreamSocket == -1) {
        m_commandStreamSocket = nn_socket(AF_SP, NN_PUB);
        if (m_commandStreamSocket < 0) {
            handleSocketError("nn_socket");
        }
        m_eventStreamSocket = nn_socket(AF_SP, NN_SUB);
        if (m_eventStreamSocket < 0) {
            handleSocketError("nn_socket");
        }
        if (nn_connect(m_commandStreamSocket, ThreadedApplicationService::kCommandStreamURI) < 0) {
            handleSocketError("nn_connect");
            close();
        }
        if (nn_connect(m_eventStreamSocket, ThreadedApplicationService::kEventStreamURI) < 0) {
            handleSocketError("nn_connect");
            close();
        }
        if (nn_setsockopt(m_eventStreamSocket, NN_SUB, NN_SUB_SUBSCRIBE, "", 0) < 0) {
            handleSocketError("nn_setsockopt");
            close();
        }
    }
}

void
ThreadedApplicationClient::receiveAllEventMessages()
{
    if (m_eventStreamSocket != -1) {
        while (true) {
            size_t receivedBodySize = 0;
            nanoem_u8_t *body = nullptr;
            struct nn_iovec iov = { &body, NN_MSG };
            void *control = nullptr;
            struct nn_msghdr hdr = { &iov, 1, &control, NN_MSG };
            int rc = nn_recvmsg(m_eventStreamSocket, &hdr, NN_DONTWAIT);
            if (rc < 0) {
                if (nn_errno() != EAGAIN) {
                    handleSocketError("nn_recvmsg");
                }
                break;
            }
            else {
                receivedBodySize = size_t(rc);
                dispatchEventMessage(body, receivedBodySize);
                nn_freemsg(body);
                nn_freemsg(control);
            }
        }
    }
}

void
ThreadedApplicationClient::close()
{
    if (m_commandStreamSocket >= 0) {
        nn_close(m_commandStreamSocket);
        m_commandStreamSocket = -1;
    }
    if (m_eventStreamSocket >= 0) {
        nn_close(m_eventStreamSocket);
        m_eventStreamSocket = -1;
    }
}

void
ThreadedApplicationClient::sendCommandMessage(const Nanoem__Application__Command *command)
{
    if (m_commandStreamSocket != -1) {
        const size_t size = nanoem__application__command__get_packed_size(command);
        nanoem_u8_t stackBuffer[128];
        void *msg = nn_allocmsg(size, 0);
        if (size < sizeof(stackBuffer)) {
            nanoem__application__command__pack(command, stackBuffer);
            memcpy(msg, stackBuffer, size);
        }
        else {
            ByteArray heapBuffer(size);
            nanoem__application__command__pack(command, heapBuffer.data());
            memcpy(msg, heapBuffer.data(), size);
        }
        if (nn_send(m_commandStreamSocket, &msg, NN_MSG, 0) < 0) {
            handleSocketError("nn_send");
        }
    }
}

void
ThreadedApplicationClient::handleSocketError(const char *prefix)
{
    bx::debugPrintf("%s: %s\n", prefix, nn_strerror(nn_errno()));
}

} /* namespace nanoem */

#endif /* NANOEM_ENABLE_NANOMSG */
