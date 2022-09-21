/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import <AppKit/AppKit.h>
#import <IOKit/pwr_mgt/IOPMLib.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CVDisplayLink.h>

#include "emapp/emapp.h"

#include "emapp/Allocator.h"
#include "emapp/integration/LoadPMMExecutor.h"
#include "nanoem/ext/document.h"

#include "CocoaThreadedApplicationService.h"

#include "bx/commandline.h"
#include "spdlog/async.h"
#include "spdlog/cfg/env.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using namespace nanoem;

namespace {

class TestWindow {
public:
    static nanoem_f32_t calculateDevicePixelRatio();

    TestWindow(const bx::CommandLine *cmd, ThreadedApplicationService *service, ThreadedApplicationClient *client,
        NSWindow *nativeWindow);
    ~TestWindow();

    void initialize();
    void processMessage(NSApplication *app);
    bool renderFrame();
    void terminate();

    bool isRunning() const;
    void setRunning(bool value);

private:
    static test::IExecutor *createExecutor(const bx::CommandLine *cmd, ThreadedApplicationService *service,
        ThreadedApplicationClient *client, bx::Mutex *eventLock);
    static URI generateFileURI(bool archive);
    void registerAllPrerequisiteEventListeners();
    void sendPerformanceMonitorPeriodically();
    void quit();

    const bx::CommandLine *m_commandLine;
    test::IExecutor *m_runner = nullptr;
    ThreadedApplicationService *m_service = nullptr;
    ThreadedApplicationClient *m_client = nullptr;
    NSWindow *m_nativeWindow = nil;
    IOPMAssertionID m_displayKey = 0;
    IOPMAssertionID m_idleKey = 0;
    tinystl::pair<bool, bool> m_cursorHidden = tinystl::make_pair(false, false);
    dispatch_semaphore_t m_metricsSemaphore = nullptr;
    dispatch_queue_t m_metricsQueue = nullptr;
    bx::Mutex m_eventLock;
    bool m_running = true;
};

nanoem_f32_t
TestWindow::calculateDevicePixelRatio()
{
    return 0;
}

TestWindow::TestWindow(const bx::CommandLine *cmd, ThreadedApplicationService *service,
    ThreadedApplicationClient *client, NSWindow *nativeWindow)
    : m_commandLine(cmd)
    , m_runner(createExecutor(cmd, service, client, &m_eventLock))
    , m_service(service)
    , m_client(client)
    , m_nativeWindow(nativeWindow)
    , m_metricsSemaphore(dispatch_semaphore_create(0))
    , m_metricsQueue(dispatch_queue_create(nullptr, nullptr))
{
    registerAllPrerequisiteEventListeners();
}

TestWindow::~TestWindow()
{
    delete m_runner;
    m_runner = nullptr;
}

void
TestWindow::initialize()
{
    CFStringRef reason = CFSTR("nanoem_test");
    IOPMAssertionCreateWithName(kIOPMAssertPreventUserIdleDisplaySleep, kIOPMAssertionLevelOn, reason, &m_displayKey);
    IOPMAssertionCreateWithName(kIOPMAssertPreventUserIdleSystemSleep, kIOPMAssertionLevelOn, reason, &m_idleKey);
    const CGSize &frameSize = m_nativeWindow.contentView.frame.size;
    const glm::u16vec2 logicalWindowSize(frameSize.width, frameSize.height);
    float dpr = m_nativeWindow.backingScaleFactor;
    NSURL *pluginDirectoryURL =
        [[NSBundle mainBundle].bundleURL URLByAppendingPathComponent:@"nanoem.app/Contents/PlugIns/"];
    String sokolPath(pluginDirectoryURL.path.UTF8String);
    sokolPath.append("/");
    sg_pixel_format pixelFormat;
    if (m_commandLine->hasArg("opengl")) {
        const NSOpenGLPixelFormatAttribute attributes[] = { NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
            NSOpenGLPFAColorSize, 24, NSOpenGLPFAAlphaSize, 8, NSOpenGLPFADepthSize, 24, NSOpenGLPFAStencilSize, 8,
            NSOpenGLPFADoubleBuffer, 1, NSOpenGLPFAAccelerated, 1, NSOpenGLPFANoRecovery, 1, NSOpenGLPFAClosestPolicy,
            1, 0, 0 };
        NSOpenGLPixelFormat *format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
        NSOpenGLView *view = [[NSOpenGLView alloc] initWithFrame:m_nativeWindow.contentView.bounds pixelFormat:format];
        [view registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]];
        view.wantsBestResolutionOpenGLSurface = YES;
        m_nativeWindow.contentView = view;
        NSOpenGLContext *context = view.openGLContext;
        [context makeCurrentContext];
        int interval = 1;
        [context setValues:&interval forParameter:NSOpenGLCPSwapInterval];
        m_nativeWindow.contentView = view;
        m_service->setNativeContext((__bridge void *) context);
        m_service->setNativeView((__bridge void *) view);
        [NSOpenGLContext clearCurrentContext];
        sokolPath.append("sokol_glcore33.dylib");
        pixelFormat = SG_PIXELFORMAT_RGBA8;
    }
    else {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        MTKView *view = [[MTKView alloc] initWithFrame:m_nativeWindow.contentView.bounds device:device];
        [view registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]];
        view.enableSetNeedsDisplay = NO;
        view.paused = YES;
        view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
        view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
        m_nativeWindow.contentView = view;
        m_service->setNativeDevice((__bridge void *) device);
        m_service->setNativeView((__bridge void *) view);
        sokolPath.append("sokol_metal_macos.dylib");
        pixelFormat = SG_PIXELFORMAT_BGRA8;
    }
    ThreadedApplicationClient::InitializeMessageDescription desc(logicalWindowSize, pixelFormat, dpr, sokolPath);
    desc.m_viewportDevicePixelRatio = 1.0f;
    m_client->sendInitializeMessage(desc);
    EMLOG_INFO("Initialized an application: runner={}", static_cast<const void *>(m_runner));
}

void
TestWindow::processMessage(NSApplication *app)
{
    {
        bx::MutexScope scope(m_eventLock);
        BX_UNUSED_1(scope);
        m_client->receiveAllEventMessages();
    }
    while (NSEvent *event = [app nextEventMatchingMask:NSUIntegerMax
                                             untilDate:[NSDate distantPast]
                                                inMode:NSDefaultRunLoopMode
                                               dequeue:YES]) {
        [app sendEvent:event];
    }
}

bool
TestWindow::isRunning() const
{
    return m_running;
}

void
TestWindow::setRunning(bool value)
{
    m_running = value;
}

bool
TestWindow::renderFrame()
{
    return true;
}

void
TestWindow::terminate()
{
    m_nativeWindow.delegate = nil;
    [m_nativeWindow orderOut:nil];
    m_client->addCompleteTerminationEventListener(
        [](void *userData) {
            TestWindow *self = static_cast<TestWindow *>(userData);
            self->setRunning(false);
            dispatch_semaphore_wait(self->m_metricsSemaphore, DISPATCH_TIME_FOREVER);
        },
        this, true);
    m_client->sendTerminateMessage();
    IOPMAssertionRelease(m_displayKey);
    IOPMAssertionRelease(m_idleKey);
    EMLOG_INFO("Terminated an application: runner={}", static_cast<const void *>(m_runner));
}

test::IExecutor *
TestWindow::createExecutor(const bx::CommandLine *cmd, ThreadedApplicationService *service,
    ThreadedApplicationClient *client, bx::Mutex *eventLock)
{
    test::IExecutor *executor = 0;
    if (cmd->hasArg("models")) {
        const URI &fileURI = URI::createFromFilePath(cmd->findOption("models"));
        executor = new test::LoadAllModelsExecutor(fileURI, cmd, service, client);
        EMLOG_INFO("Created loading model executor: {}", fileURI.absolutePathConstString());
    }
    else if (cmd->hasArg("effects")) {
        const URI &fileURI = URI::createFromFilePath(cmd->findOption("effects"));
        executor = new test::LoadAllEffectsExecutor(fileURI, cmd, service, client);
        EMLOG_INFO("Created loading effect executor: {}", fileURI.absolutePathConstString());
    }
    else {
        const URI &fileURI = generateFileURI(cmd->hasArg("archive"));
        executor = new test::LoadPMMExecutor(fileURI, cmd, service, client, eventLock);
        EMLOG_INFO("Created loading PMM executor: {}", fileURI.absolutePathConstString());
    }
    return executor;
}

URI
TestWindow::generateFileURI(bool archive)
{
    NSString *directory = NSTemporaryDirectory();
    NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
    formatter.dateFormat = @"yyyy-MM-dd-HHmmss";
    NSString *formattedDateTime = [formatter stringFromDate:[NSDate date]];
    NSString *filename = [[NSString alloc] initWithFormat:@"nanoem-%@.%s", formattedDateTime, archive ? "nma" : "nmm"];
    return URI::createFromFilePath([directory stringByAppendingPathComponent:filename].UTF8String);
}

void
TestWindow::registerAllPrerequisiteEventListeners()
{
    m_client->addInitializationCompleteEventListener(
        [](void *userData) {
            TestWindow *self = static_cast<TestWindow *>(userData);
            NSWindow *window = self->m_nativeWindow;
            [window center];
            [window makeFirstResponder:window];
            [window makeKeyAndOrderFront:window];
            dispatch_async(self->m_metricsQueue, ^() {
                self->sendPerformanceMonitorPeriodically();
            });
            self->m_client->sendActivateMessage();
            self->m_runner->start();
        },
        this, true);
    m_client->addCompleteDestructionEventListener(
        [](void *userData) {
            TestWindow *self = static_cast<TestWindow *>(userData);
            self->m_runner->finish();
            self->terminate();
        },
        this, true);
    m_client->addSeekEvent(
        [](void *userData, nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndexTo,
            nanoem_frame_index_t localFrameIndexFrom) {
            TestWindow *self = static_cast<TestWindow *>(userData);
            BX_UNUSED(self, duration, localFrameIndexTo, localFrameIndexFrom);
        },
        this, false);
    EMLOG_INFO("Registered all event listeners: runner={}", static_cast<const void *>(m_runner));
}

void
TestWindow::sendPerformanceMonitorPeriodically()
{
    while (m_running) {
        mach_port_t port = mach_task_self();
        nanoem_f32_t cpu = macos::CocoaThreadedApplicationService::currentCPUPercentage(port);
        nanoem_u64_t memory = macos::CocoaThreadedApplicationService::currentMemorySize(port);
        m_client->sendUpdatePerformanceMonitorMessage(cpu, memory, 0);
        bx::sleep(1000);
    }
    dispatch_semaphore_signal(m_metricsSemaphore);
}

void
TestWindow::quit()
{
    m_client->sendDestroyMessage();
    EMLOG_INFO("Started quitting an application: runner={}", static_cast<const void *>(m_runner));
}

} /* namespace anonymous */

@interface TestApplicationDelegate : NSObject <NSApplicationDelegate> {
    TestWindow *m_window;
}

- (instancetype)init UNAVAILABLE_ATTRIBUTE;
- (instancetype)initWithWindow:(TestWindow *)window;

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)application;
- (void)applicationDidFinishLaunching:(NSNotification *)notification;

@end

@implementation TestApplicationDelegate

- (instancetype)initWithWindow:(TestWindow *)window
{
    if (self = [super init]) {
        m_window = window;
    }
    return self;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)application
{
    BX_UNUSED_1(application);
    m_window->setRunning(false);
    return NSTerminateCancel;
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    BX_UNUSED_1(notification);
    NSApplication *app = [NSApplication sharedApplication];
    [app stop:app];
    @autoreleasepool {
        NSEvent *event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                            location:NSMakePoint(0, 0)
                                       modifierFlags:0
                                           timestamp:0
                                        windowNumber:0
                                             context:nil
                                             subtype:0
                                               data1:0
                                               data2:0];
        [app postEvent:event atStart:YES];
    }
}

@end

int
main(int argc, char *argv[])
{
    Allocator::initialize();
    ThreadedApplicationService::setup();
    @autoreleasepool {
        NSBundle *mainBundle = [NSBundle mainBundle];
        NSURL *jsonURI = [mainBundle.bundleURL URLByAppendingPathComponent:@"test.json"];
        JSON_Value *config = json_parse_file_with_comments(jsonURI.absoluteURL.path.UTF8String);
        URIList plugins;
        {
            JSON_Object *root = nullptr;
            if (config) {
                root = json_object(config);
            }
            else {
                config = json_value_init_object();
                root = json_object(config);
            }
            NSLocale *locale = [NSLocale currentLocale];
            json_object_dotset_string(root, "project.home", [NSHomeDirectory() UTF8String]);
            json_object_dotset_string(root, "project.locale", locale.localeIdentifier.UTF8String);
            {
                NSURL *url = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory
                                                                    inDomain:NSUserDomainMask
                                                           appropriateForURL:nil
                                                                      create:YES
                                                                       error:nil];
                NSString *domainString =
                    [[NSString alloc] initWithUTF8String:BaseApplicationService::kOrganizationDomain];
                NSURL *applicationDirectoryURL = [url URLByAppendingPathComponent:domainString isDirectory:YES];
                NSURL *redoDirectoryURL = [applicationDirectoryURL URLByAppendingPathComponent:@"redo" isDirectory:YES];
                NSURL *tempDirectoryURL = [applicationDirectoryURL URLByAppendingPathComponent:@"tmp" isDirectory:YES];
                NSError *error = nil;
                NSDictionary *attributes = [[NSDictionary alloc]
                    initWithObjectsAndKeys:[NSNumber numberWithShort:0700], NSFilePosixPermissions, nil];
                [[NSFileManager defaultManager] createDirectoryAtURL:tempDirectoryURL
                                         withIntermediateDirectories:YES
                                                          attributes:attributes
                                                               error:&error];
                [[NSFileManager defaultManager] createDirectoryAtURL:redoDirectoryURL
                                         withIntermediateDirectories:YES
                                                          attributes:attributes
                                                               error:&error];
                json_object_dotset_string(root, "project.tmp.path", tempDirectoryURL.path.UTF8String);
                json_object_dotset_string(root, "macos.redo.path", redoDirectoryURL.path.UTF8String);
            }
            NSURL *pluginDirectoryURL =
                [[NSBundle mainBundle].bundleURL URLByAppendingPathComponent:@"nanoem.app/Contents/PlugIns/"];
            NSURL *pluginURL = [pluginDirectoryURL URLByAppendingPathComponent:@"/plugin_effect.dylib"];
            json_object_dotset_string(root, "plugin.effect.path", pluginURL.path.UTF8String);
            NSEnumerator *enumerator =
                [[NSFileManager defaultManager] enumeratorAtURL:pluginDirectoryURL
                                     includingPropertiesForKeys:nil
                                                        options:NSDirectoryEnumerationSkipsHiddenFiles
                                                   errorHandler:nil];
            for (NSURL *fileURL in enumerator) {
                if ([fileURL.pathExtension isEqualToString:@"dylib"]) {
                    plugins.push_back(URI::createFromFilePath(fileURL.path.UTF8String));
                }
            }
        }
        {
            const bx::CommandLine cmd(argc, argv);
            NSApplication *app = macos::CocoaThreadedApplicationService::createApplication();
            macos::CocoaThreadedApplicationService service(config);
            ThreadedApplicationClient client;
            service.start();
            client.connect();
            spdlog::init_thread_pool(1024, 1);
            tinystl::vector<spdlog::sink_ptr, TinySTLAllocator> sinks;
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
            auto logger = std::make_shared<spdlog::async_logger>(
                "emapp", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
            spdlog::register_logger(logger);
            spdlog::cfg::load_env_levels();
            {
                NSWindow *nativeWindow = macos::CocoaThreadedApplicationService::createMainWindow();
                TestWindow window(&cmd, &service, &client, nativeWindow);
                window.initialize();
                client.sendLoadAllDecoderPluginsMessage(plugins);
                TestApplicationDelegate *applicationDelegate = [[TestApplicationDelegate alloc] initWithWindow:&window];
                app.delegate = applicationDelegate;
                [app run];
                while (window.isRunning()) {
                    window.processMessage(app);
                    if (!window.renderFrame()) {
                        break;
                    }
                }
            }
            client.close();
            service.stop();
        }
        json_value_free(config);
    }
    ThreadedApplicationService::terminate();
    Allocator::destroy();
    return 0;
}
