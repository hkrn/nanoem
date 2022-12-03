/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/ThreadedApplicationService.h"

#if defined(NANOEM_ENABLE_NANOMSG)

#include "./protoc/application.pb-c.h"
#include "emapp/Accessory.h"
#include "emapp/BaseApplicationService.h"
#include "emapp/DefaultFileManager.h"
#include "emapp/IEventPublisher.h"
#include "emapp/IModalDialog.h"
#include "emapp/IVideoRecorder.h"
#include "emapp/ModalDialogFactory.h"
#include "emapp/Model.h"
#include "emapp/Motion.h"
#include "emapp/Progress.h"
#include "emapp/StateController.h"
#include "emapp/StringUtils.h"
#include "emapp/internal/CapturingPassState.h"
#include "emapp/internal/project/Redo.h"
#include "emapp/model/Bone.h"
#include "emapp/model/Morph.h"
#include "emapp/private/CommonInclude.h"

#include "bx/handlealloc.h"
#include "sokol/sokol_time.h"

#define NN_STATIC_LIB
#include "nanomsg/nn.h"
#include "nanomsg/pubsub.h"
#if defined(NANOEM_ENABLE_TBB)
#include "tbb/task_group.h"
#else /* NANOEM_ENABLE_TBB */
#include <new>
namespace tbb {
struct task_group {
    template <typename F>
    static void
    run_and_wait(const F &task)
    {
        task();
    }
};
} /* namespace tbb */
#endif /* NANOEM_ENABLE_TBB */

namespace nanoem {
namespace {

struct RecoveryCallbackArgument {
    RecoveryCallbackArgument(const URI &fileURI, ThreadedApplicationService *servicePtr)
        : m_fileURI(fileURI)
        , m_servicePtr(servicePtr)
    {
    }
    const URI m_fileURI;
    ThreadedApplicationService *m_servicePtr;
};

struct RecoveryWorker {
    static IModalDialog *cancel(void *userData, Project *project);

    RecoveryWorker(ThreadedApplicationService *service, Project *project, const URI &fileURI);
    ~RecoveryWorker() NANOEM_DECL_NOEXCEPT;

    void operator()() const;

    ThreadedApplicationService *m_service;
    Project *m_project;
    IModalDialog *m_dialog;
    bx::Thread m_thread;
    URI m_fileURI;
    volatile bool m_cancelled;
};

IModalDialog *
RecoveryWorker::cancel(void *userData, Project * /* project */)
{
    RecoveryWorker *self = static_cast<RecoveryWorker *>(userData);
    self->m_cancelled = true;
    return nullptr;
}

RecoveryWorker::RecoveryWorker(ThreadedApplicationService *service, Project *project, const URI &fileURI)
    : m_service(service)
    , m_project(project)
    , m_dialog(nullptr)
    , m_fileURI(fileURI)
    , m_cancelled(false)
{
}

RecoveryWorker::~RecoveryWorker() NANOEM_DECL_NOEXCEPT
{
}

void
RecoveryWorker::operator()() const
{
    FileReaderScope scope(nullptr);
    Error error;
    if (scope.open(m_fileURI, error)) {
        internal::project::Redo redo(m_project);
        redo.loadAllAsync(scope.reader(), m_dialog, &m_cancelled, error);
    }
    if (!m_cancelled) {
        m_service->clearAllModalDialog();
    }
}

class CancelPublisherWorker NANOEM_DECL_SEALED : public ICancelPublisher, private NonCopyable {
public:
    CancelPublisherWorker();
    ~CancelPublisherWorker();

    void start() NANOEM_DECL_OVERRIDE;
    void addSubscriber(ICancelSubscriber *subscriber) NANOEM_DECL_OVERRIDE;
    void removeSubscriber(ICancelSubscriber *subscriber) NANOEM_DECL_OVERRIDE;
    void stop() NANOEM_DECL_OVERRIDE;

private:
    typedef tinystl::unordered_set<ICancelSubscriber *, TinySTLAllocator> Subscribers;
    static nanoem_i32_t execute(bx::Thread *thread, void *userData);

    void connect();
    bool dispatch();
    void close();

    Subscribers m_subscribers;
    bx::Thread m_thread;
    bx::Mutex m_mutex;
    int m_streamSocket;
    int m_streamEndpoint;
    volatile bool m_running;
};

CancelPublisherWorker::CancelPublisherWorker()
    : m_streamSocket(-1)
    , m_streamEndpoint(-1)
    , m_running(true)
{
}

CancelPublisherWorker::~CancelPublisherWorker()
{
    stop();
}

void
CancelPublisherWorker::addSubscriber(ICancelSubscriber *subscriber)
{
    bx::MutexScope locker(m_mutex);
    BX_UNUSED_1(locker);
    m_subscribers.insert(subscriber);
}

void
CancelPublisherWorker::removeSubscriber(ICancelSubscriber *subscriber)
{
    bx::MutexScope locker(m_mutex);
    BX_UNUSED_1(locker);
    m_subscribers.erase(subscriber);
}

void
CancelPublisherWorker::start()
{
    char name[Inline::kNameStackBufferSize];
    StringUtils::format(name, sizeof(name), "%s.CancelWorker", ThreadedApplicationService::kOrganizationDomain);
    m_thread.init(execute, this, 0, name);
}

void
CancelPublisherWorker::stop()
{
    if (m_running) {
        m_running = false;
        Progress::requestCancel();
        m_thread.shutdown();
    }
}

nanoem_i32_t
CancelPublisherWorker::execute(bx::Thread * /* thread */, void *userData)
{
    CancelPublisherWorker *self = static_cast<CancelPublisherWorker *>(userData);
    self->connect();
    while (self->dispatch()) {
    }
    self->close();
    return 0;
}

void
CancelPublisherWorker::connect()
{
    if (m_streamSocket == -1) {
        m_streamSocket = nn_socket(AF_SP, NN_SUB);
        if (m_streamSocket == -1) {
            EMLOG_ERROR("nn_socket: {}", nn_strerror(nn_errno()));
            close();
        }
        m_streamEndpoint = nn_bind(m_streamSocket, Progress::kStreamURI);
        if (m_streamEndpoint == -1) {
            EMLOG_ERROR("nn_bind: {}", nn_strerror(nn_errno()));
            close();
        }
        if (nn_setsockopt(m_streamSocket, NN_SUB, NN_SUB_SUBSCRIBE, "", 0) < 0) {
            EMLOG_ERROR("nn_setsockopt: {}", nn_strerror(nn_errno()));
            close();
        }
    }
}

bool
CancelPublisherWorker::dispatch()
{
    nanoem_u32_t value = 0;
    int rc = nn_recv(m_streamSocket, &value, sizeof(value), 0);
    bool result = true;
    if (rc < 0) {
        EMLOG_ERROR("nn_recvmsg: {}", nn_strerror(nn_errno()));
        result = false;
    }
    else if (value == Progress::kCancelToken) {
        bx::MutexScope locker(m_mutex);
        BX_UNUSED_1(locker);
        for (Subscribers::const_iterator it = m_subscribers.begin(), end = m_subscribers.end(); it != end; ++it) {
            ICancelSubscriber *subscriber = *it;
            subscriber->onCancelled();
        }
    }
    return result && m_running;
}

void
CancelPublisherWorker::close()
{
    if (m_streamEndpoint != -1) {
        nn_shutdown(m_streamSocket, m_streamEndpoint);
        m_streamEndpoint = -1;
    }
    if (m_streamSocket != -1) {
        nn_close(m_streamSocket);
        m_streamSocket = -1;
    }
}

class LoadingAllModelIOPluginsWorker NANOEM_DECL_SEALED : private NonCopyable {
public:
    LoadingAllModelIOPluginsWorker(int language, const URIList &locations, ThreadedApplicationService *service);
    ~LoadingAllModelIOPluginsWorker() NANOEM_DECL_NOEXCEPT;

    void operator()() const;

private:
    const URIList m_locations;
    ThreadedApplicationService *m_service;
    int m_language;
};

LoadingAllModelIOPluginsWorker::LoadingAllModelIOPluginsWorker(
    int language, const URIList &locations, ThreadedApplicationService *service)
    : m_locations(locations)
    , m_service(service)
    , m_language(language)
{
}

LoadingAllModelIOPluginsWorker::~LoadingAllModelIOPluginsWorker() NANOEM_DECL_NOEXCEPT
{
}

void
LoadingAllModelIOPluginsWorker::operator()() const
{
    m_service->defaultFileManager()->initializeAllModelIOPlugins(m_locations);
    m_service->sendLoadingAllModelIOPluginsEventMessage(m_language);
}

class LoadingAllMotionIOPluginsWorker NANOEM_DECL_SEALED : private NonCopyable {
public:
    LoadingAllMotionIOPluginsWorker(int language, const URIList &locations, ThreadedApplicationService *service);
    ~LoadingAllMotionIOPluginsWorker() NANOEM_DECL_NOEXCEPT;

    void operator()() const;

private:
    const URIList m_locations;
    ThreadedApplicationService *m_service;
    int m_language;
};

LoadingAllMotionIOPluginsWorker::LoadingAllMotionIOPluginsWorker(
    int language, const URIList &locations, ThreadedApplicationService *service)
    : m_locations(locations)
    , m_service(service)
    , m_language(language)
{
}

LoadingAllMotionIOPluginsWorker::~LoadingAllMotionIOPluginsWorker() NANOEM_DECL_NOEXCEPT
{
}

void
LoadingAllMotionIOPluginsWorker::operator()() const
{
    m_service->defaultFileManager()->initializeAllMotionIOPlugins(m_locations);
    m_service->sendLoadingAllMotionIOPluginsEventMessage(m_language);
}

} /* namespace anonymous */

const char *const ThreadedApplicationService::kCommandStreamURI = "inproc://nanoem-command-stream";
const char *const ThreadedApplicationService::kEventStreamURI = "inproc://nanoem-event-stream";

void
ThreadedApplicationService::terminate()
{
#if defined(NANOEM_ENABLE_LOGGING)
    spdlog::drop_all();
    spdlog::shutdown();
#endif /* NANOEM_ENABLE_LOGGING */
    nn_term();
}

ThreadedApplicationService::ThreadedApplicationService(const JSON_Value *config)
    : BaseApplicationService(config)
    , m_nativeContext(nullptr)
    , m_nativeDevice(nullptr)
    , m_nativeView(nullptr)
    , m_nativeSwapChain(nullptr)
    , m_nativeSwapChainDescription(nullptr)
    , m_recvmsgFlags(NN_DONTWAIT)
    , m_commandStreamSocket(-1)
    , m_eventStreamSocket(-1)
    , m_commandStreamEndPoint(-1)
    , m_eventStreamEndPoint(-1)
    , m_running(false)
{
}

ThreadedApplicationService::~ThreadedApplicationService() NANOEM_DECL_NOEXCEPT
{
    closeAllSockets();
    m_nativeContext = nullptr;
    m_nativeDevice = nullptr;
    m_nativeView = nullptr;
    m_nativeSwapChain = nullptr;
    m_nativeSwapChainDescription = nullptr;
}

void
ThreadedApplicationService::start()
{
    if (m_commandStreamSocket == -1 && m_eventStreamSocket == -1) {
        m_commandStreamSocket = nn_socket(AF_SP_RAW, NN_SUB);
        if (m_commandStreamSocket < 0) {
            handleSocketError("nn_socket");
        }
        m_eventStreamSocket = nn_socket(AF_SP_RAW, NN_PUB);
        if (m_eventStreamSocket < 0) {
            handleSocketError("nn_socket");
        }
        m_commandStreamEndPoint = nn_bind(m_commandStreamSocket, kCommandStreamURI);
        if (m_commandStreamEndPoint < 0) {
            handleSocketError("nn_bind");
            closeAllSockets();
        }
        m_eventStreamEndPoint = nn_bind(m_eventStreamSocket, kEventStreamURI);
        if (m_eventStreamEndPoint < 0) {
            handleSocketError("nn_bind");
            closeAllSockets();
        }
        if (nn_setsockopt(m_commandStreamSocket, NN_SUB, NN_SUB_SUBSCRIBE, "", 0) < 0) {
            handleSocketError("nn_setsockopt");
            closeAllSockets();
        }
        m_running = true;
        handleStartingApplicationThread();
    }
}

int
ThreadedApplicationService::run()
{
    int exitCode = 0;
    handleInitializeApplicationThread();
    while (m_running) {
        executeRunUnit(exitCode);
    }
    handleDestructApplicationThread();
    return exitCode;
}

int
ThreadedApplicationService::stop()
{
    return handleStoppingApplicationThread();
}

void
ThreadedApplicationService::setNativeContext(void *opaque)
{
    m_nativeContext = opaque;
}

void
ThreadedApplicationService::setNativeDevice(void *opaque)
{
    m_nativeDevice = opaque;
}

void
ThreadedApplicationService::setNativeView(void *opaque)
{
    m_nativeView = opaque;
}

void
ThreadedApplicationService::setNativeSwapChain(void *opaque)
{
    m_nativeSwapChain = opaque;
}

void
ThreadedApplicationService::setNativeSwapChainDescription(void *opaque)
{
    m_nativeSwapChainDescription = opaque;
}

URI
ThreadedApplicationService::recoverableRedoFileURI() const
{
    return URI();
}

void
ThreadedApplicationService::deleteCrashRecoveryFile()
{
    /* do nothing */
}

void
ThreadedApplicationService::handleStartingApplicationThread()
{
    char name[Inline::kNameStackBufferSize];
    StringUtils::format(name, sizeof(name), "%s.ThreadedApplicationService", kOrganizationDomain);
    m_thread.init(run, this, 0, name);
}

void
ThreadedApplicationService::handleInitializeApplicationThread()
{
    /* do nothing */
}

void
ThreadedApplicationService::handleDestructApplicationThread()
{
    /* do nothing */
}

void
ThreadedApplicationService::executeRunUnit(int &exitCode)
{
    drawDefaultPass();
    receiveAllCommandMessages(exitCode);
}

int
ThreadedApplicationService::handleStoppingApplicationThread()
{
    int result = -1;
    if (m_thread.isRunning()) {
        m_thread.shutdown();
        result = m_thread.getExitCode();
    }
    return result;
}

void
ThreadedApplicationService::handleInitializeApplication()
{
    const URI fileURI(recoverableRedoFileURI());
    if (!fileURI.isEmpty() && FileUtils::exists(fileURI)) {
        const ITranslator *tr = translator();
        const String &title = tr->translate("nanoem.window.dialog.redo.confirm.title");
        const String &message = tr->translate("nanoem.window.dialog.redo.confirm.message");
        ModalDialogFactory::StandardConfirmDialogCallbackPair pair(handleAcceptOnRecovery);
        RecoveryCallbackArgument *arg = nanoem_new(RecoveryCallbackArgument(fileURI, this));
        IModalDialog *dialog = ModalDialogFactory::createStandardConfirmDialog(this, title, message, pair, arg);
        addModalDialog(dialog);
    }
}

bool
ThreadedApplicationService::handleCommandMessage(Nanoem__Application__Command *command, Project *project, Error &error)
{
    bool succeeded = true;
    switch (command->type_case) {
    case NANOEM__APPLICATION__COMMAND__TYPE_ACTIVATE: {
        if (project) {
            succeeded = BaseApplicationService::handleCommandMessage(command, project, error);
            m_recvmsgFlags |= NN_DONTWAIT;
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_DEACTIVATE: {
        if (project) {
            succeeded = BaseApplicationService::handleCommandMessage(command, project, error);
            m_recvmsgFlags &= ~NN_DONTWAIT;
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_TERMINATE: {
        succeeded = BaseApplicationService::handleCommandMessage(command, project, error);
        m_running = false;
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_RECOVERY: {
        const Nanoem__Application__URI *uri = command->recovery->file_uri;
        IModalDialog *dialog = startRecoveryFromRedo(URI::createFromFilePath(uri->absolute_path, uri->fragment));
        addModalDialog(dialog);
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_LOAD_ALL_MODEL_IO_PLUGINS: {
#if !defined(NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN)
        const Nanoem__Application__LoadAllModelIOPluginsCommand *commandPtr = command->load_all_model_io_plugins;
        URIList fileURIs;
        for (size_t i = 0, numPlugins = commandPtr->n_file_uris; i < numPlugins; i++) {
            const Nanoem__Application__URI *uri = commandPtr->file_uris[i];
            fileURIs.push_back(URI::createFromFilePath(uri->absolute_path, uri->fragment));
        }
        if (!fileURIs.empty()) {
            const nanoem_u64_t started = stm_now();
            BX_UNUSED_1(started);
            int language = project ? project->castLanguage() : 0;
            tbb::task_group tg;
            tg.run_and_wait(LoadingAllModelIOPluginsWorker(language, fileURIs, this));
            EMLOG_INFO("All {} model I/O plugins area loaded in {} msecs", fileURIs.size(), stm_ms(stm_since(started)));
        }
#endif /* NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN */
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_LOAD_ALL_MOTION_IO_PLUGINS: {
#if !defined(NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN)
        const Nanoem__Application__LoadAllMotionIOPluginsCommand *commandPtr = command->load_all_motion_io_plugins;
        URIList fileURIs;
        for (size_t i = 0, numPlugins = commandPtr->n_file_uris; i < numPlugins; i++) {
            const Nanoem__Application__URI *uri = commandPtr->file_uris[i];
            fileURIs.push_back(URI::createFromFilePath(uri->absolute_path, uri->fragment));
        }
        if (!fileURIs.empty()) {
            const nanoem_u64_t started = stm_now();
            BX_UNUSED_1(started);
            int language = project ? project->castLanguage() : 0;
            tbb::task_group tg;
            tg.run_and_wait(LoadingAllMotionIOPluginsWorker(language, fileURIs, this));
            EMLOG_DEBUG(
                "All {} motion I/O plugins area loaded in {} msecs", fileURIs.size(), stm_ms(stm_since(started)));
        }
#endif /* NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN */
        break;
    }
    default:
        succeeded = BaseApplicationService::handleCommandMessage(command, project, error);
    }
    return succeeded;
}

void
ThreadedApplicationService::sendEventMessage(const Nanoem__Application__Event *event)
{
    size_t size = nanoem__application__event__get_packed_size(event);
    nanoem_u8_t stackBuffer[128];
    void *msg = nn_allocmsg(size, 0);
    if (size < sizeof(stackBuffer)) {
        nanoem__application__event__pack(event, stackBuffer);
        memcpy(msg, stackBuffer, size);
    }
    else {
        ByteArray heapBuffer(size);
        nanoem__application__event__pack(event, heapBuffer.data());
        memcpy(msg, heapBuffer.data(), size);
    }
    if (nn_send(m_eventStreamSocket, &msg, NN_MSG, 0) < 0) {
        handleSocketError("nn_send");
    }
}

int
ThreadedApplicationService::run(bx::Thread * /* thread */, void *opaque)
{
    ThreadedApplicationService *self = static_cast<ThreadedApplicationService *>(opaque);
    return self->run();
}

IModalDialog *
ThreadedApplicationService::handleAcceptOnRecovery(void *userData, Project * /* project */)
{
    RecoveryCallbackArgument *arg = static_cast<RecoveryCallbackArgument *>(userData);
    ThreadedApplicationService *service = arg->m_servicePtr;
    const URI &fileURI = arg->m_fileURI;
    IModalDialog *dialog = service->startRecoveryFromRedo(fileURI);
    service->deleteCrashRecoveryFile();
    nanoem_delete(arg);
    return dialog;
}

ICancelPublisher *
ThreadedApplicationService::createCancelPublisher()
{
    return nanoem_new(CancelPublisherWorker);
}

void
ThreadedApplicationService::receiveAllCommandMessages(int &exitCode)
{
    while (true) {
        nanoem_u8_t *body = nullptr;
        struct nn_iovec iov = { &body, NN_MSG };
        void *control = nullptr;
        struct nn_msghdr hdr = { &iov, 1, &control, NN_MSG };
        size_t receivedBodySize;
        int rc = nn_recvmsg(m_commandStreamSocket, &hdr, m_recvmsgFlags);
        if (rc < 0) {
            if (nn_errno() != EAGAIN) {
                nn_freemsg(control);
                handleSocketError("nn_recvmsg");
                exitCode = -1;
            }
            break;
        }
        else {
            receivedBodySize = size_t(rc);
        }
        dispatchCommandMessage(body, receivedBodySize, projectHolder()->currentProject(), false);
        nn_freemsg(body);
        nn_freemsg(control);
    }
}

void
ThreadedApplicationService::handleSocketError(const char *prefix)
{
    BX_UNUSED_1(prefix);
    EMLOG_ERROR("%s -> %d: %s\n", prefix, nn_errno(), nn_strerror(nn_errno()));
}

void
ThreadedApplicationService::closeAllSockets()
{
    if (m_commandStreamEndPoint != -1) {
        nn_shutdown(m_commandStreamSocket, m_commandStreamEndPoint);
        m_commandStreamEndPoint = -1;
    }
    if (m_eventStreamEndPoint != -1) {
        nn_shutdown(m_eventStreamSocket, m_eventStreamEndPoint);
        m_eventStreamEndPoint = -1;
    }
    if (m_commandStreamSocket != -1) {
        nn_close(m_commandStreamSocket);
        m_commandStreamSocket = -1;
    }
    if (m_eventStreamSocket != -1) {
        nn_close(m_eventStreamSocket);
        m_eventStreamSocket = -1;
    }
}

IModalDialog *
ThreadedApplicationService::startRecoveryFromRedo(const URI &fileURI)
{
    const ITranslator *tr = translator();
    const String &title = tr->translate("nanoem.window.dialog.redo.progress.title");
    const String &message = tr->translate("nanoem.window.dialog.redo.progress.message");
    RecoveryWorker worker(this, projectHolder()->currentProject(), fileURI);
    worker.m_dialog = ModalDialogFactory::createProgressDialog(this, title, message, RecoveryWorker::cancel, &worker);
    tbb::task_group tg;
    tg.run_and_wait(worker);
    IModalDialog *dialog = nullptr;
#if defined(NANOEM_ENABLE_TBB)
    dialog = worker.m_dialog;
#endif /* NANOEM_ENABLE_TBB */
    return dialog;
}

} /* namespace nanoem */

#endif /* NANOEM_ENABLE_NANOMSG */
