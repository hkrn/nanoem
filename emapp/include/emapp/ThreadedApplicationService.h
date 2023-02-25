/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_THREADEDAPPLICATIONSERVICE_H_
#define NANOEM_EMAPP_THREADEDAPPLICATIONSERVICE_H_

#include "emapp/BaseApplicationService.h"
#include "emapp/ModalDialogFactory.h"
#include "emapp/PluginFactory.h"

#include "bx/thread.h"

struct Nanoem__Application__Command;
struct Nanoem__Application__Event;
struct Nanoem__Application__Plugin;

namespace nanoem {

class DefaultCallback;
class DefaultDelegate;
class IProjectHolder;
class IVideoRecorder;

class ThreadedApplicationService : public BaseApplicationService {
public:
    static const char *const kCommandStreamURI;
    static const char *const kEventStreamURI;
    static void terminate();

    ThreadedApplicationService(const JSON_Value *config);
    ~ThreadedApplicationService() NANOEM_DECL_NOEXCEPT;

    void start();
    virtual int run();
    int stop();

    void setNativeContext(void *opaque);
    void setNativeDevice(void *opaque);
    void setNativeView(void *opaque);
    void setNativeSwapChain(void *opaque);
    void setNativeSwapChainDescription(void *opaque);

protected:
    virtual URI recoverableRedoFileURI() const;
    virtual void deleteCrashRecoveryFile();
    virtual void handleStartingApplicationThread();
    virtual void handleInitializeApplicationThread();
    virtual void handleDestructApplicationThread();
    virtual void executeRunUnit(int &exitCode);
    virtual int handleStoppingApplicationThread();

    void handleInitializeApplication() NANOEM_DECL_OVERRIDE;
    bool handleCommandMessage(
        Nanoem__Application__Command *command, Project *project, Error &error) NANOEM_DECL_OVERRIDE;
    void sendEventMessage(const Nanoem__Application__Event *event) NANOEM_DECL_OVERRIDE;

    void *m_nativeContext;
    void *m_nativeDevice;
    void *m_nativeView;
    void *m_nativeSwapChain;
    void *m_nativeSwapChainDescription;

private:
    class EventPublisher;
    static IModalDialog *handleAcceptOnRecovery(void *userData, Project *project);
    static int run(bx::Thread *thread, void *opaque);

    ICancelPublisher *createCancelPublisher() NANOEM_DECL_OVERRIDE;
    void receiveAllCommandMessages(int &exitCode);
    void handleSocketError(const char *prefix);
    void performRedo(undo_command_t *commandPtr, Project *project, undo_command_t *&commandPtrRef);
    void closeAllSockets();
    IModalDialog *startRecoveryFromRedo(const URI &fileURI);

    bx::Thread m_thread;
    int m_recvmsgFlags;
    int m_commandStreamSocket;
    int m_eventStreamSocket;
    int m_commandStreamEndPoint;
    int m_eventStreamEndPoint;
    bool m_running;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_THREADEDAPPLICATIONSERVICE_H_ */
