/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import "CocoaThreadedApplicationService.h"

#import "AudioUnitAudioPlayer.h"
#import "ErrorExtension.h"
#import "MetalBackgroundRenderer.h"
#import "MetalSkinDeformerFactory.h"
#import "MetalVideoRecorder.h"
#import "OpenGLBackgroundRenderer.h"
#import "OpenGLVideoRecorder.h"
#import "Preference.h"

#import <AVFoundation/AVFoundation.h>
#import <AppKit/AppKit.h>
#import <IOKit/hid/IOHIDLib.h>
#import <MetalKit/MetalKit.h>
#if defined(NANOEM_HOCKEYSDK_APP_IDENTIFIER)
#import <HockeySDK/HockeySDK.h>
#else
#include <signal.h>
#endif

#include "emapp/Allocator.h"
#include "emapp/Constants.h"
#include "emapp/EnumUtils.h"
#include "emapp/IBackgroundVideoRenderer.h"
#include "emapp/ICamera.h"
#include "emapp/IDebugCapture.h"
#include "emapp/IModalDialog.h"
#include "emapp/IProjectHolder.h"
#include "emapp/ITranslator.h"
#include "emapp/Model.h"
#include "emapp/StringUtils.h"
#include "emapp/ThreadedApplicationClient.h"
#include "emapp/internal/DecoderPluginBasedBackgroundVideoRenderer.h"
#include "emapp/internal/ImGuiWindow.h"
#include "emapp/private/CommonInclude.h"

#include "emapp/src/protoc/application.pb-c.h"
#include "imgui/imgui.h"

using namespace nanoem::macos;

@interface AVPlayerItemStatusObserver : NSObject {
}

@end

@implementation AVPlayerItemStatusObserver

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary<NSKeyValueChangeKey, id> *)change
                       context:(void *)context
{
    BX_UNUSED_1(change);
    dispatch_semaphore_t semaphore = (__bridge dispatch_semaphore_t) context;
    [object removeObserver:self forKeyPath:keyPath context:context];
    dispatch_semaphore_signal(semaphore);
}

@end

@interface ViewportWindowMetalView : MTKView <NSTextInputClient> {
    ThreadedApplicationClient *m_client;
    NSTextInputContext *m_textInputContext;
    NSMutableAttributedString *m_markedString;
    NSRange m_selectedRange;
}

- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device client:(ThreadedApplicationClient *)client;
- (Vector2SI32)devicePixelScreenPosition:(NSEvent *)event;

@end

@implementation ViewportWindowMetalView

- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device client:(ThreadedApplicationClient *)client
{
    if (self = [super initWithFrame:frameRect device:device]) {
        m_client = client;
        m_textInputContext = [[NSTextInputContext alloc] initWithClient:self];
        m_markedString = nil;
        m_selectedRange = NSMakeRange(NSNotFound, 0);
    }
    return self;
}

- (Vector2SI32)devicePixelScreenPosition:(NSEvent *)event
{
    return CocoaThreadedApplicationService::deviceScaleScreenPosition(event, self.window);
}

/* NSResponder */

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)mouseDown:(NSEvent *)event
{
    const int modifiers = CocoaThreadedApplicationService::convertToCursorModifiers(event),
              button = [event buttonNumber];
    m_client->sendScreenCursorPressMessage([self devicePixelScreenPosition:event], button, modifiers);
}

- (void)mouseMoved:(NSEvent *)event
{
    const int modifiers = CocoaThreadedApplicationService::convertToCursorModifiers(event),
              button = [event buttonNumber];
    m_client->sendScreenCursorMoveMessage([self devicePixelScreenPosition:event], button, modifiers);
}

- (void)mouseDragged:(NSEvent *)event
{
    const int modifiers = CocoaThreadedApplicationService::convertToCursorModifiers(event),
              button = [event buttonNumber];
    m_client->sendScreenCursorMoveMessage([self devicePixelScreenPosition:event], button, modifiers);
}

- (void)mouseUp:(NSEvent *)event
{
    const int modifiers = CocoaThreadedApplicationService::convertToCursorModifiers(event),
              button = [event buttonNumber];
    m_client->sendScreenCursorReleaseMessage([self devicePixelScreenPosition:event], button, modifiers);
}

- (void)rightMouseDown:(NSEvent *)event
{
    [self mouseDown:event];
}

- (void)rightMouseDragged:(NSEvent *)event
{
    [self mouseDragged:event];
}

- (void)rightMouseUp:(NSEvent *)event
{
    [self mouseUp:event];
}

- (void)otherMouseDown:(NSEvent *)event
{
    [self mouseDown:event];
}

- (void)otherMouseDragged:(NSEvent *)event
{
    [self mouseDragged:event];
}

- (void)otherMouseUp:(NSEvent *)event
{
    [self mouseUp:event];
}

- (void)keyDown:(NSEvent *)event
{
    [m_textInputContext handleEvent:event];
}

- (void)keyUp:(NSEvent *)event
{
    BX_UNUSED_1(event);
}

- (void)scrollWheel:(NSEvent *)event
{
    const Vector2 scrollDelta(CocoaThreadedApplicationService::scrollWheelDelta(event));
    int modifiers = CocoaThreadedApplicationService::convertToCursorModifiers(event);
    m_client->sendCursorScrollMessage(Vector2(), scrollDelta, modifiers);
}

/* NSTextInputClient */

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
    BX_UNUSED_1(replacementRange);
    NSString *characters = [string isKindOfClass:[NSAttributedString class]] ? ((NSAttributedString *) string).string
                                                                             : (NSString *) string;
    CocoaThreadedApplicationService::sendUnicodeTextInput(characters, m_client);
}

- (void)doCommandBySelector:(SEL)selector
{
    if ([self respondsToSelector:selector]) {
        [self performSelector:selector withObject:self afterDelay:0.1];
    }
}

- (void)setMarkedText:(id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange
{
    BX_UNUSED_1(replacementRange);
    NSMutableAttributedString *markerText = [string isKindOfClass:[NSAttributedString class]]
        ? [[NSMutableAttributedString alloc] initWithAttributedString:string]
        : [[NSMutableAttributedString alloc] initWithString:string];
    m_markedString = markerText;
    m_selectedRange = selectedRange;
}

- (void)unmarkText
{
    m_markedString.mutableString.string = @"";
}

- (NSRange)selectedRange
{
    return m_selectedRange;
}

- (NSRange)markedRange
{
    return self.hasMarkedText ? NSMakeRange(0, m_markedString.length) : NSMakeRange(NSNotFound, 0);
}

- (BOOL)hasMarkedText
{
    return m_markedString.length > 0;
}

- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)range actualRange:(NSRangePointer)actualRange
{
    BX_UNUSED_2(range, actualRange);
    return nil;
}

- (NSArray<NSAttributedStringKey> *)validAttributesForMarkedText
{
    return @[];
}

- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actualRange
{
    BX_UNUSED_2(range, actualRange);
    return NSMakeRect(0, 0, 0, 0);
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point
{
    BX_UNUSED_1(point);
    return NSNotFound;
}

- (NSAttributedString *)attributedString
{
    return nil;
}

@end

extern "C" {
struct nanoem_application_document_t;
nanoem_application_document_t *nanoemApplicationDocumentCreate(
    nanoem::BaseApplicationService *application, nanoem::IProjectHolder *holder, const char *path);
nanoem_bool_t nanoemApplicationDocumentLoad(nanoem_application_document_t *document, Error *error);
nanoem_bool_t nanoemApplicationDocumentSave(nanoem_application_document_t *document, const char *path, Error *error);
void nanoemApplicationDocumentDestroy(nanoem_application_document_t *document);
}

namespace nanoem {
namespace macos {
namespace {

char s_crashFilePath[PATH_MAX] = {};
char s_redoFilePath[PATH_MAX] = {};

class CocoaBackgroundVideoRendererProxy : public IBackgroundVideoRenderer {
public:
    static const int kPreferredTimeScale;

    CocoaBackgroundVideoRendererProxy(CocoaThreadedApplicationService *service, void *proc);
    ~CocoaBackgroundVideoRendererProxy() noexcept;

    bool load(const URI &fileURI, Error &error) override;
    void draw(sg_pass pass, const Vector4 &rect, nanoem_f32_t scaleFactor, Project *project) override;
    void seek(nanoem_f64_t value) override;
    void flush() override;
    void destroy() override;
    URI fileURI() const noexcept override;

private:
    CocoaThreadedApplicationService *m_service;
    AVPlayer *m_player = nil;
    AVPlayerItem *m_playerItem = nil;
    AVPlayerItemVideoOutput *m_videoOutput = nil;
    ICocoaBackgroundRenderer *m_cocoaBackgroundVideoRenderer = nullptr;
    internal::DecoderPluginBasedBackgroundVideoRenderer *m_decoderPluginBasedBackgroundVideoRenderer = nullptr;
    URI m_fileURI;
    CMTime m_currentTime = kCMTimeZero;
    void *_sgx_get_native_pass_handle = nullptr;
    bool m_durationUpdated = false;
};

const int CocoaBackgroundVideoRendererProxy::kPreferredTimeScale = 48000;

CocoaBackgroundVideoRendererProxy::CocoaBackgroundVideoRendererProxy(
    CocoaThreadedApplicationService *service, void *proc)
    : m_service(service)
    , _sgx_get_native_pass_handle(proc)
{
}

CocoaBackgroundVideoRendererProxy::~CocoaBackgroundVideoRendererProxy() noexcept
{
}

bool
CocoaBackgroundVideoRendererProxy::load(const URI &fileURI, Error &error)
{
    NSString *path = [[NSString alloc] initWithUTF8String:fileURI.absolutePathConstString()];
    NSURL *url = [[NSURL alloc] initFileURLWithPath:path];
    AVAsset *asset = [AVAsset assetWithURL:url];
    bool playable = asset.playable != NO;
    if (playable) {
        @try {
            AVPlayerItem *playerItem = [[AVPlayerItem alloc] initWithAsset:asset];
            dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
            AVPlayerItemStatusObserver *observer = [[AVPlayerItemStatusObserver alloc] init];
            [playerItem addObserver:observer
                         forKeyPath:@"status"
                            options:NSKeyValueObservingOptionOld | NSKeyValueObservingOptionNew
                            context:(__bridge void *) semaphore];
            NSDictionary *settings = @{(NSString *) kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_32BGRA)};
            m_videoOutput = [[AVPlayerItemVideoOutput alloc] initWithOutputSettings:settings];
            [playerItem addOutput:m_videoOutput];
            m_player = [[AVPlayer alloc] initWithPlayerItem:playerItem];
            m_playerItem = playerItem;
            dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
            if (NSError *err = playerItem.error) {
                error = err.nanoem;
                playable = false;
            }
            else {
                const sg_backend backend = sg::query_backend();
                if (backend == SG_BACKEND_METAL_MACOS) {
                    m_cocoaBackgroundVideoRenderer = nanoem_new(MetalBackgroundRenderer(m_service->metalDevice()));
                }
                else if (backend == SG_BACKEND_GLCORE33) {
                    m_cocoaBackgroundVideoRenderer =
                        nanoem_new(OpenGLBackgroundRenderer(m_service->OpenGLContext(), _sgx_get_native_pass_handle));
                }
                m_fileURI = fileURI;
            }
        } @catch (NSException *exception) {
            error = Error(exception.reason.UTF8String, nullptr, Error::kDomainTypeOS);
            playable = false;
        }
    }
    else {
        Error error2;
        m_decoderPluginBasedBackgroundVideoRenderer =
            nanoem_new(internal::DecoderPluginBasedBackgroundVideoRenderer(m_service->defaultFileManager()));
        playable = m_decoderPluginBasedBackgroundVideoRenderer->load(fileURI, error2);
        if (playable) {
            error = Error();
        }
        else {
            m_decoderPluginBasedBackgroundVideoRenderer->destroy();
            nanoem_delete_safe(m_decoderPluginBasedBackgroundVideoRenderer);
            error = error2;
        }
    }
    return playable;
}

void
CocoaBackgroundVideoRendererProxy::draw(sg_pass pass, const Vector4 &rect, nanoem_f32_t scaleFactor, Project *project)
{
    if (m_player) {
        CMTime outputTime;
        if (CVPixelBufferRef pixelBuffer = [m_videoOutput copyPixelBufferForItemTime:m_currentTime
                                                                  itemTimeForDisplay:&outputTime]) {
            SG_PUSH_GROUP("macos::CocoaBackgroundVideoRendererProxy::draw");
            m_cocoaBackgroundVideoRenderer->draw(pass, rect, project, pixelBuffer);
            if (m_playerItem.status == AVPlayerItemStatusReadyToPlay && !m_durationUpdated) {
                const CMTime &duration = m_playerItem.duration;
                project->setBaseDuration(
                    nanoem_frame_index_t((duration.value / duration.timescale) * Constants::kHalfBaseFPS));
                m_durationUpdated = true;
            }
            CVPixelBufferRelease(pixelBuffer);
            SG_POP_GROUP();
        }
    }
    else if (m_decoderPluginBasedBackgroundVideoRenderer) {
        m_decoderPluginBasedBackgroundVideoRenderer->draw(pass, rect, scaleFactor, project);
    }
}

void
CocoaBackgroundVideoRendererProxy::seek(nanoem_f64_t value)
{
    m_currentTime = CMTimeMakeWithSeconds(value, kPreferredTimeScale);
    if (m_player) {
        [m_player seekToTime:m_currentTime toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
    }
    else if (m_decoderPluginBasedBackgroundVideoRenderer) {
        m_decoderPluginBasedBackgroundVideoRenderer->seek(value);
    }
}

void
CocoaBackgroundVideoRendererProxy::flush()
{
    if (m_decoderPluginBasedBackgroundVideoRenderer) {
        m_decoderPluginBasedBackgroundVideoRenderer->flush();
    }
}

void
CocoaBackgroundVideoRendererProxy::destroy()
{
    if (m_decoderPluginBasedBackgroundVideoRenderer) {
        m_decoderPluginBasedBackgroundVideoRenderer->destroy();
        nanoem_delete_safe(m_decoderPluginBasedBackgroundVideoRenderer);
    }
    nanoem_delete_safe(m_cocoaBackgroundVideoRenderer);
    m_player = nil;
    m_playerItem = nil;
    m_videoOutput = nil;
    m_durationUpdated = false;
}

URI
CocoaBackgroundVideoRendererProxy::fileURI() const noexcept
{
    return m_decoderPluginBasedBackgroundVideoRenderer ? m_decoderPluginBasedBackgroundVideoRenderer->fileURI()
                                                       : m_fileURI;
}

struct MetalRendererCapability : Project::IRendererCapability {
    MetalRendererCapability(CocoaThreadedApplicationService *service);
    ~MetalRendererCapability();
    nanoem_u32_t suggestedSampleLevel(nanoem_u32_t value) const noexcept override;
    bool supportsSampleLevel(nanoem_u32_t value) const noexcept override;
    CocoaThreadedApplicationService *m_service;
};

MetalRendererCapability::MetalRendererCapability(CocoaThreadedApplicationService *service)
    : m_service(service)
{
}

MetalRendererCapability::~MetalRendererCapability()
{
}

nanoem_u32_t
MetalRendererCapability::suggestedSampleLevel(nanoem_u32_t value) const noexcept
{
    auto device = m_service->metalDevice();
    uint32_t sampleCount = 1 << value;
    while (![device supportsTextureSampleCount:sampleCount]) {
        value--;
        sampleCount = 1 << value;
    }
    return value;
}

bool
MetalRendererCapability::supportsSampleLevel(nanoem_u32_t value) const noexcept
{
    auto device = m_service->metalDevice();
    const uint32_t sampleCount = 1 << value;
    return [device supportsTextureSampleCount:sampleCount];
}

class MetalDebugCapture : public IDebugCapture {
public:
    MetalDebugCapture(id<MTLDevice> device);

    void start(const char *label) override;
    void stop() override;

private:
    id<MTLDevice> m_device = nil;
};

MetalDebugCapture::MetalDebugCapture(id<MTLDevice> device)
    : m_device(device)
{
    BX_UNUSED_1(m_device);
}

void
MetalDebugCapture::start(const char *label)
{
    if (@available(macOS 10.15, *)) {
        if (label) {
            MTLCaptureManager *capture = [MTLCaptureManager sharedCaptureManager];
            MTLCaptureDescriptor *desc = [[MTLCaptureDescriptor alloc] init];
            desc.captureObject = m_device;
            [capture startCaptureWithDescriptor:desc error:nil];
        }
    }
}

void
MetalDebugCapture::stop()
{
    if (@available(macOS 10.15, *)) {
        MTLCaptureManager *capture = [MTLCaptureManager sharedCaptureManager];
        [capture stopCapture];
    }
}

#if defined(IMGUI_HAS_VIEWPORT)
struct ViewportWindow {
    using BlockFunc = void (^)(ViewportWindow *);

    ViewportWindow();
    ~ViewportWindow();

    void runBlockOnMainQueue(BlockFunc block);

    void createWindow(const ImGuiViewport *viewport);
    void destroyWindow();
    void showWindow();
    ImVec2 windowPos() const noexcept;
    void setWindowPos(const ImVec2 &value);
    ImVec2 windowSize() const noexcept;
    void setWindowSize(const ImVec2 &value);
    bool hasWindowFocus() const noexcept;
    void makeWindowFocus();
    bool isWindowMinimized() const noexcept;
    void setWindowTitle(NSString *value);
    void setWindowAlpha(CGFloat value);
    bool updateWindow();

    float flipY(float y, const NSRect &contentRect) const noexcept;

    ThreadedApplicationClient m_client;
    NSWindow *m_window;
};

ViewportWindow::ViewportWindow()
{
}

ViewportWindow::~ViewportWindow()
{
}

void
ViewportWindow::runBlockOnMainQueue(BlockFunc block)
{
    __block ViewportWindow *self = this;
    dispatch_sync(dispatch_get_main_queue(), ^() {
        block(self);
    });
}

void
ViewportWindow::createWindow(const ImGuiViewport *viewport)
{
    NSWindow *parentWindow = nil;
    if (viewport->ParentViewportId != 0) {
        if (ImGuiViewport *parent = ImGui::FindViewportByID(viewport->ParentViewportId)) {
            parentWindow = (__bridge NSWindow *) parent->PlatformHandleRaw;
        }
    }
    NSUInteger style = 0;
    ImGuiViewportFlags flags = viewport->Flags;
    if (!EnumUtils::isEnabledT<ImGuiViewportFlags>(flags, ImGuiViewportFlags_NoDecoration)) {
        style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable |
            NSWindowStyleMaskResizable;
    }
    NSWindow *window;
    const nanoem_f32_t invertDevicePixelRatio = parentWindow ? (1.0f / parentWindow.backingScaleFactor) : 1.0f;
    const NSRect rect = NSMakeRect(viewport->Pos.x * invertDevicePixelRatio, viewport->Pos.y * invertDevicePixelRatio,
                     viewport->Size.x * invertDevicePixelRatio, viewport->Size.y * invertDevicePixelRatio),
                 frameRect = [NSWindow frameRectForContentRect:rect styleMask:style];
    if (EnumUtils::isEnabledT<ImGuiViewportFlags>(flags, ImGuiViewportFlags_NoTaskBarIcon)) {
        window = [[NSPanel alloc] initWithContentRect:frameRect
                                            styleMask:style
                                              backing:NSBackingStoreBuffered
                                                defer:NO];
    }
    else {
        window = [[NSWindow alloc] initWithContentRect:frameRect
                                             styleMask:style
                                               backing:NSBackingStoreBuffered
                                                 defer:NO];
    }
    window.acceptsMouseMovedEvents = YES;
    window.backgroundColor = [NSColor blackColor];
    window.releasedWhenClosed = NO;
    window.restorable = NO;
    window.movableByWindowBackground = NO;
    window.opaque = NO;
    CocoaThreadedApplicationService *service =
        static_cast<CocoaThreadedApplicationService *>(ImGui::GetIO().BackendRendererUserData);
    id<MTLDevice> device = service->metalDevice();
    MTKView *parentView = service->nativeView();
    MTKView *contentView = [[ViewportWindowMetalView alloc] initWithFrame:window.contentView.bounds
                                                                   device:device
                                                                   client:&m_client];
    contentView.colorPixelFormat = parentView.colorPixelFormat;
    contentView.depthStencilPixelFormat = parentView.depthStencilPixelFormat;
    contentView.enableSetNeedsDisplay = NO;
    contentView.paused = YES;
    window.contentView = contentView;
    [parentWindow addChildWindow:window ordered:NSWindowAbove];
    m_window = window;
    m_client.connect();
}

void
ViewportWindow::destroyWindow()
{
    auto mainWindow = (__bridge NSWindow *) ImGui::GetMainViewport()->PlatformHandleRaw;
    if (mainWindow != m_window) {
        [mainWindow removeChildWindow:m_window];
        [m_window close];
        m_window = nil;
    }
    [mainWindow makeKeyWindow];
}

void
ViewportWindow::showWindow()
{
    [m_window makeKeyAndOrderFront:m_window];
}

ImVec2
ViewportWindow::windowPos() const noexcept
{
    const NSScreen *screen = m_window.screen;
    const NSRect frameRect = [m_window convertRectToBacking:m_window.frame],
                 screenFrameRect = [m_window convertRectToBacking:screen.frame],
                 screenVisibleFrameRect = [m_window convertRectToBacking:screen.visibleFrame];
    const CGFloat offset = screenFrameRect.size.height - screenVisibleFrameRect.size.height;
    CGPoint origin = frameRect.origin;
    origin.y = screenFrameRect.size.height - (frameRect.size.height + origin.y) + offset;
    return ImVec2(origin.x, origin.y);
}

void
ViewportWindow::setWindowPos(const ImVec2 &value)
{
    const NSRect rect = [m_window convertRectFromBacking:NSMakeRect(value.x, value.y, 0, 0)];
    const CGPoint origin = rect.origin;
    const CGFloat y = m_window.contentView.frame.size.height + origin.y;
    const NSRect frameRect =
        [m_window frameRectForContentRect:NSMakeRect(origin.x, m_window.screen.frame.size.height - y, 0, 0)];
    [m_window setFrameOrigin:frameRect.origin];
}

ImVec2
ViewportWindow::windowSize() const noexcept
{
    const NSSize frameSize = [m_window convertRectToBacking:m_window.frame].size;
    return ImVec2(frameSize.width, frameSize.height);
}

void
ViewportWindow::setWindowSize(const ImVec2 &value)
{
    NSRect rect = [m_window convertRectFromBacking:NSMakeRect(0, 0, value.x, value.y)];
    rect.origin = m_window.frame.origin;
    [m_window setFrame:rect display:YES];
}

bool
ViewportWindow::hasWindowFocus() const noexcept
{
    return m_window.keyWindow;
}

void
ViewportWindow::makeWindowFocus()
{
    [m_window makeKeyAndOrderFront:m_window];
}

bool
ViewportWindow::isWindowMinimized() const noexcept
{
    return m_window.miniaturized;
}

void
ViewportWindow::setWindowTitle(NSString *value)
{
    m_window.title = value;
}

void
ViewportWindow::setWindowAlpha(CGFloat value)
{
    m_window.alphaValue = value;
}

bool
ViewportWindow::updateWindow()
{
    [m_window update];
    return false;
}

float
ViewportWindow::flipY(float y, const NSRect &contentRect) const noexcept
{
    NSScreen *screen = m_window.screen;
    return (screen.frame.size.height - (y + contentRect.size.height));
}

struct MetalRendererData {
    ~MetalRendererData()
    {
        m_view = nil;
        m_texture = nil;
    }
    MTKView *m_view;
    id<MTLTexture> m_texture;
};

#endif /* IMGUI_HAS_VIEWPORT */

} /* namespace anonymous */

NSString *CocoaThreadedApplicationService::kDefaultWindowTitle = @"nanoem";

NSApplication *
CocoaThreadedApplicationService::createApplication()
{
    NSApplication *app = nil;
#if 0
    struct sigaction sa;
    sa.sa_flags = SA_RESETHAND;
    sa.sa_handler = [](int /* code */) { handleApplicationCrash(); };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGBUS, &sa, nullptr);
    sigaction(SIGILL, &sa, nullptr);
    sigaction(SIGSEGV, &sa, nullptr);
#endif
    app = [NSApplication sharedApplication];
    app.activationPolicy = NSApplicationActivationPolicyRegular;
    [app activateIgnoringOtherApps:YES];
    return app;
}

NSWindow *
CocoaThreadedApplicationService::createMainWindow()
{
    const Vector2UI16 &minsize = BaseApplicationService::minimumRequiredWindowSize();
    const NSRect minSizeRect = NSMakeRect(0, 0, minsize.x, minsize.y);
    const NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable |
        NSWindowStyleMaskResizable;
    const NSRect frameRect = [NSWindow frameRectForContentRect:minSizeRect styleMask:style];
    NSWindow *window = [[NSWindow alloc] initWithContentRect:frameRect
                                                   styleMask:style
                                                     backing:NSBackingStoreBuffered
                                                       defer:NO];
    window.minSize = frameRect.size;
    window.acceptsMouseMovedEvents = YES;
    window.backgroundColor = [NSColor blackColor];
    window.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary;
    window.releasedWhenClosed = NO;
    window.restorable = NO;
    window.movableByWindowBackground = NO;
    window.opaque = NO;
    window.title = kDefaultWindowTitle;
    return window;
}

NSString *
CocoaThreadedApplicationService::maskUserString(const char *value)
{
    NSString *masked =
        [[[[[NSString alloc] initWithUTF8String:value] stringByReplacingOccurrencesOfString:NSHomeDirectory()
                                                                                 withString:@"{NSHomeDirectory}"]
            stringByReplacingOccurrencesOfString:NSFullUserName()
                                      withString:@"{NSFullUserName}"]
            stringByReplacingOccurrencesOfString:NSUserName()
                                      withString:@"{NSUserName}"];
    return masked;
}

URI
CocoaThreadedApplicationService::canonicalFileURI(NSURL *value)
{
    return URI::createFromFilePath(
        value.path.stringByStandardizingPath.precomposedStringWithCanonicalMapping.UTF8String);
}

nanoem_f32_t
CocoaThreadedApplicationService::currentCPUPercentage(mach_port_t port)
{
    thread_basic_info threadInfo = {};
    thread_act_array_t acts = {};
    mach_msg_type_number_t numThreadInfo = THREAD_BASIC_INFO_COUNT, numActs = 0;
    task_threads(port, &acts, &numActs);
    integer_t totalCPUUsage = 0;
    for (mach_msg_type_number_t i = 0; i < numActs; i++) {
        int result =
            thread_info(acts[i], THREAD_BASIC_INFO, reinterpret_cast<thread_info_t>(&threadInfo), &numThreadInfo);
        if (result == KERN_SUCCESS && !EnumUtils::isEnabledT<integer_t>(threadInfo.flags, TH_FLAGS_IDLE)) {
            totalCPUUsage += threadInfo.cpu_usage;
        }
    }
    return totalCPUUsage / (TH_USAGE_SCALE / 100.0f);
}

nanoem_u64_t
CocoaThreadedApplicationService::currentMemorySize(mach_port_t port)
{
    mach_task_basic_info taskInfo = {};
    mach_msg_type_number_t numTaskInfo = MACH_TASK_BASIC_INFO_COUNT;
    int result = task_info(port, MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&taskInfo), &numTaskInfo);
    tinystl::pair<uint64_t, uint64_t> value;
    return result == KERN_SUCCESS ? taskInfo.resident_size : 0;
}

Vector2
CocoaThreadedApplicationService::scrollWheelDelta(const NSEvent *event)
{
    Vector2 scrollDelta;
    if ([event hasPreciseScrollingDeltas]) {
        scrollDelta = Vector2([event scrollingDeltaX] * 0.1f, [event scrollingDeltaY] * 0.1f);
    }
    else {
        scrollDelta = Vector2([event deltaX], [event deltaY]);
    }
    return scrollDelta;
}

Vector2SI32
CocoaThreadedApplicationService::deviceScaleScreenPosition(const NSEvent *event, NSWindow *window) noexcept
{
    const NSPoint locationInWindow(event.locationInWindow);
    const NSRect rectInWindow(NSMakeRect(locationInWindow.x, locationInWindow.y, 0, 0)),
        screenCursorRect([window convertRectToScreen:rectInWindow]),
        deviceCursorRect([window convertRectToBacking:screenCursorRect]),
        deviceScaleFrameRect = [window convertRectToBacking:window.screen.frame];
    return Vector2SI32(deviceCursorRect.origin.x, deviceScaleFrameRect.size.height - deviceCursorRect.origin.y);
}

const char *
CocoaThreadedApplicationService::sharedRedoFilePath() noexcept
{
    return s_redoFilePath;
}

int
CocoaThreadedApplicationService::convertToCursorModifiers(const NSEvent *event) noexcept
{
    const NSUInteger modifierFlags = event.modifierFlags;
    int flags = 0;
    if (EnumUtils::isEnabledT<NSUInteger>(NSEventModifierFlagCommand, modifierFlags) ||
        EnumUtils::isEnabledT<NSUInteger>(NSEventModifierFlagControl, modifierFlags)) {
        flags |= Project::kCursorModifierTypeControl;
    }
    else if (EnumUtils::isEnabledT<NSUInteger>(NSEventModifierFlagShift, modifierFlags)) {
        flags |= Project::kCursorModifierTypeShift;
    }
    else if (EnumUtils::isEnabledT<NSUInteger>(NSEventModifierFlagOption, modifierFlags)) {
        flags |= Project::kCursorModifierTypeAlt;
    }
    return flags;
}

void
CocoaThreadedApplicationService::sendDummyEvent(NSApplication *app, bool atStart)
{
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
        [app postEvent:event atStart:atStart ? YES : NO];
    }
}

void
CocoaThreadedApplicationService::sendUnicodeTextInput(const NSString *characters, ThreadedApplicationClient *client)
{
    const NSUInteger length = characters.length;
    if (length > 1) {
        for (NSUInteger i = 0; i < length; i++) {
            const unichar codepoint = [characters characterAtIndex:i];
            /* c.f. https://github.com/glfw/glfw/issues/1215 */
            if ((codepoint & 0xFF00) == 0xF700) {
                continue;
            }
            client->sendUnicodeInputMessage(codepoint);
        }
    }
    else if (length == 1) {
        const uint16_t codepoint = [characters characterAtIndex:0];
        switch (codepoint) {
        case 0x8:
            client->sendKeyPressMessage(BaseApplicationService::kKeyType_BACKSPACE);
            break;
        case 0xd:
            client->sendKeyPressMessage(BaseApplicationService::kKeyType_ENTER);
            break;
        case 0x1b:
            client->sendKeyPressMessage(BaseApplicationService::kKeyType_ESCAPE);
            break;
        case 0x1c:
            client->sendKeyPressMessage(BaseApplicationService::kKeyType_LEFT);
            break;
        case 0x1d:
            client->sendKeyPressMessage(BaseApplicationService::kKeyType_RIGHT);
            break;
        case 0x1e:
            client->sendKeyPressMessage(BaseApplicationService::kKeyType_UP);
            break;
        case 0x1f:
            client->sendKeyPressMessage(BaseApplicationService::kKeyType_DOWN);
            break;
        default:
            if ((codepoint > 0x7f && (codepoint & 0xFF00) != 0xF700) || isprint(codepoint)) {
                client->sendUnicodeInputMessage(codepoint);
            }
            break;
        }
    }
}

void
CocoaThreadedApplicationService::setupAllocatorContext(CFAllocatorContext &context, Allocator *allocator)
{
    context = {
        /* version */ 0,
        /* info */ allocator,
        /* retain */ nullptr,
        /* release */ nullptr,
        /* copyDescription */ nullptr,
        /* allocate */
        [](CFIndex allocSize, CFOptionFlags hint, void *info) -> void * {
            BX_UNUSED_1(hint);
            auto allocator = static_cast<Allocator *>(info);
            return bx::alloc(allocator, allocSize);
        },
        /* reallocate */
        [](void *ptr, CFIndex newsize, CFOptionFlags hint, void *info) -> void * {
            BX_UNUSED_1(hint);
            auto allocator = static_cast<Allocator *>(info);
            return bx::realloc(allocator, ptr, newsize);
        },
        /* deallocate */
        [](void *ptr, void *info) {
            auto allocator = static_cast<Allocator *>(info);
            return bx::free(allocator, ptr);
        },
        /* preferredSize */
        [](CFIndex size, CFOptionFlags hint, void *info) -> CFIndex {
            BX_UNUSED_2(hint, info);
            return size;
        },
    };
}

CocoaThreadedApplicationService::CocoaThreadedApplicationService(const JSON_Value *config)
    : ThreadedApplicationService(config)
    , m_displaySyncChanged(false)
{
    const char *redoPath = json_object_dotget_string(json_object(config), "macos.redo.path");
    NSString *crashFilePath = [[[NSString alloc] initWithUTF8String:redoPath] stringByAppendingPathComponent:@"CRASH"];
    strlcpy(s_crashFilePath, crashFilePath.UTF8String, sizeof(s_crashFilePath));
}

CocoaThreadedApplicationService::~CocoaThreadedApplicationService()
{
    nanoemApplicationDocumentDestroy(m_document);
    m_document = nullptr;
}

void
CocoaThreadedApplicationService::setDisplaySyncEnabled(bool value)
{
    m_displaySyncEnabled = value;
    m_displaySyncChanged = true;
}

NSOpenGLContext *
CocoaThreadedApplicationService::OpenGLContext()
{
    return (__bridge NSOpenGLContext *) m_nativeContext;
}

id<MTLDevice>
CocoaThreadedApplicationService::metalDevice()
{
    return (__bridge id<MTLDevice>) m_nativeDevice;
}

id<MTLCommandQueue>
CocoaThreadedApplicationService::metalCommandQueue()
{
    return m_commandQueue;
}

id<CAMetalDrawable>
CocoaThreadedApplicationService::metalCurrentDrawable()
{
    id<CAMetalDrawable> drawable = nil;
    @synchronized(m_nativeView) {
        drawable = ((__bridge MTKView *) m_nativeView).currentDrawable;
    };
    return drawable;
}

MTKView *
CocoaThreadedApplicationService::nativeView()
{
    return (__bridge MTKView *) m_nativeView;
}

Vector2SI32
CocoaThreadedApplicationService::textInputOrigin() const noexcept
{
    return m_textInputOrigin;
}

CVReturn
CocoaThreadedApplicationService::handleDisplayLinkCallback(CVDisplayLinkRef /* displayLink */,
    const CVTimeStamp * /* inNow */, const CVTimeStamp * /* inOutputTime */, CVOptionFlags /* flagsIn */,
    CVOptionFlags * /* flagsOut */, void *displayLinkContext)
{
    auto self = static_cast<CocoaThreadedApplicationService *>(displayLinkContext);
    dispatch_semaphore_signal(self->m_semaphore);
    return kCVReturnSuccess;
}

IAudioPlayer *
CocoaThreadedApplicationService::createAudioPlayer()
{
    return nanoem_new(AudioUnitAudioPlayer(eventPublisher()));
}

IBackgroundVideoRenderer *
CocoaThreadedApplicationService::createBackgroundVideoRenderer()
{
    return nanoem_new(CocoaBackgroundVideoRendererProxy(this, resolveDllProcAddress("sgx_get_native_pass_handle")));
}

IDebugCapture *
CocoaThreadedApplicationService::createDebugCapture()
{
    IDebugCapture *capture = nullptr;
#if defined(NANOEM_ENABLE_DEBUG_LABEL)
    if (sg::query_backend() == SG_BACKEND_METAL_MACOS) {
        capture = nanoem_new(MetalDebugCapture(metalDevice()));
    }
#endif /* NANOEM_ENABLE_DEBUG_LABEL */
    return capture;
}

Project::IRendererCapability *
CocoaThreadedApplicationService::createRendererCapability()
{
    const sg_backend backend = sg::query_backend();
    return backend == SG_BACKEND_METAL_MACOS ? nanoem_new(MetalRendererCapability(this))
                                             : ThreadedApplicationService::createRendererCapability();
}

Project::ISkinDeformerFactory *
CocoaThreadedApplicationService::createSkinDeformerFactory()
{
    Project::ISkinDeformerFactory *factory = nullptr;
    const sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_METAL_MACOS) {
        id<MTLDevice> device = (__bridge id<MTLDevice>) m_nativeDevice;
        if (@available(macOS 10.13, *)) {
            const ApplicationPreference preference(this);
            bool supported = [device supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v3];
            if (supported && preference.isSkinDeformAcceleratorEnabled()) {
                if (!m_commandQueue) {
                    typedef void *(*PFN_sgx_mtl_cmd_queue)(void);
                    if (auto sgx_mtl_cmd_queue =
                            reinterpret_cast<PFN_sgx_mtl_cmd_queue>(resolveDllProcAddress("sgx_mtl_cmd_queue"))) {
                        m_commandQueue = (__bridge id<MTLCommandQueue>) sgx_mtl_cmd_queue();
                    }
                }
                factory = nanoem_new(MetalSkinDeformerFactory(this));
            }
        }
    }
    return factory;
}

IVideoRecorder *
CocoaThreadedApplicationService::createVideoRecorder()
{
    Project *project = projectHolder()->currentProject();
    IVideoRecorder *recorder = nullptr;
    const sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_METAL_MACOS) {
        id<MTLDevice> device = (__bridge id<MTLDevice>) m_nativeDevice;
        recorder = nanoem_new(MetalVideoRecorder(project, device));
    }
    else if (backend == SG_BACKEND_GLCORE33) {
        NSOpenGLContext *context = (__bridge NSOpenGLContext *) m_nativeContext;
        recorder =
            nanoem_new(OpenGLVideoRecorder(project, context, resolveDllProcAddress("sgx_get_native_pass_handle")));
    }
    return recorder;
}

BaseApplicationClient *
CocoaThreadedApplicationService::menubarApplicationClient()
{
#ifndef NDEBUG
    if (!m_menuClient) {
        m_menuClient = nanoem_new(ThreadedApplicationClient);
        m_menuClient->connect();
    }
#endif /* NDEBUG */
    return m_menuClient;
}

bool
CocoaThreadedApplicationService::hasVideoRecorder() const noexcept
{
    const sg_backend backend = sg::query_backend();
    return backend == SG_BACKEND_METAL_MACOS || backend == SG_BACKEND_GLCORE33;
}

bool
CocoaThreadedApplicationService::isRendererAvailable(const char *value) const noexcept
{
    bool result = false;
    if (StringUtils::equals(value, kRendererMetal)) {
        result = Preference::isMetalAvailable();
    }
    else if (StringUtils::equals(value, kRendererOpenGL)) {
        result = true;
    }
    return result;
}

void
CocoaThreadedApplicationService::executeRunUnit(int &exitCode)
{
    @autoreleasepool {
        ThreadedApplicationService::executeRunUnit(exitCode);
    }
}

void
CocoaThreadedApplicationService::handleInitializeApplicationThread()
{
    static Allocator allocator("CocoaAS");
    static CFAllocatorContext context;
    setupAllocatorContext(context, &allocator);
    m_allocator = CFAllocatorCreate(kCFAllocatorUseContext, &context);
    // CFAllocatorSetDefault(m_allocator);
}

void
CocoaThreadedApplicationService::handleDestructApplicationThread()
{
    CFAllocatorSetDefault(kCFAllocatorDefault);
    CFRelease(m_allocator);
    m_allocator = nullptr;
}

void
CocoaThreadedApplicationService::handleSetupGraphicsEngine(sg_desc &desc)
{
    if (m_nativeDevice && m_nativeView) {
        desc.context.metal.user_data = this;
        desc.context.metal.device = m_nativeDevice;
        desc.context.metal.renderpass_descriptor_cb = []() -> const void * { return nullptr; };
        desc.context.metal.drawable_userdata_cb = [](void *userData) -> const void * {
            auto self = static_cast<CocoaThreadedApplicationService *>(userData);
            auto view = (__bridge MTKView *) self->m_nativeView;
            return (__bridge const void *) view.currentDrawable;
        };
    }
    else if (m_nativeContext) {
        BX_UNUSED_1(desc);
        NSOpenGLContext *context = (__bridge NSOpenGLContext *) m_nativeContext;
        [context makeCurrentContext];
        m_semaphore = dispatch_semaphore_create(0);
        CVDisplayLinkCreateWithActiveCGDisplays(&m_displayLink);
        CVDisplayLinkSetOutputCallback(m_displayLink, handleDisplayLinkCallback, this);
        CVDisplayLinkStart(m_displayLink);
    }
}

void
CocoaThreadedApplicationService::handleInitializeApplication()
{
    initializeInputDevices();
    setupNewProject();
    ImGuiIO &io = ImGui::GetIO();
    io.ClipboardUserData = this;
    io.GetClipboardTextFn = [](void *userData) -> const char * {
        auto self = static_cast<CocoaThreadedApplicationService *>(userData);
        NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
        NSString *available = [pasteboard availableTypeFromArray:@[ NSPasteboardTypeString ]], *data = nil;
        if ([available isEqualToString:NSPasteboardTypeString]) {
            data = [pasteboard stringForType:NSPasteboardTypeString];
            self->m_clipboard = data;
        }
        return self->m_clipboard.UTF8String;
    };
    io.SetClipboardTextFn = [](void * /* userData */, const char *value) {
        NSString *data = [[NSString alloc] initWithUTF8String:value];
        NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
        [pasteboard declareTypes:@[ NSPasteboardTypeString ] owner:nil];
        [pasteboard setString:data forType:NSPasteboardTypeString];
    };
#if defined(IMGUI_HAS_VIEWPORT)
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
    io.SetPlatformImeDataFn = [](ImGuiViewport *viewport, ImGuiPlatformImeData *data) {
        const ImGuiIO &io = ImGui::GetIO();
        const nanoem_f32_t fx = data->InputPos.x * (1.0f / io.DisplayFramebufferScale.x),
                           fy = data->InputPos.y * (1.0f / io.DisplayFramebufferScale.y) + data->InputLineHeight;
        auto self = static_cast<CocoaThreadedApplicationService *>(viewport->PlatformHandleRaw);
        self->m_textInputOrigin = Vector2SI32(fx, fy);
    };
    ImGuiPlatformIO &platformIO = ImGui::GetPlatformIO();
    platformIO.Platform_CreateWindow = [](ImGuiViewport *viewport) {
        __block ViewportWindow *userData = IM_NEW(ViewportWindow);
        __block ImGuiViewport *localViewport = viewport;
        userData->runBlockOnMainQueue(^(ViewportWindow *userData) {
            userData->createWindow(localViewport);
        });
        viewport->PlatformUserData = userData;
        viewport->PlatformHandle = ImGui::GetMainViewport()->PlatformHandle;
        viewport->PlatformHandleRaw = (__bridge void *) userData->m_window;
        viewport->PlatformRequestResize = false;
    };
    platformIO.Platform_DestroyWindow = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        userData->runBlockOnMainQueue(^(ViewportWindow *userData) {
            userData->destroyWindow();
        });
        IM_DELETE(userData);
        viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
    };
    platformIO.Platform_ShowWindow = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        userData->runBlockOnMainQueue(^(ViewportWindow *userData) {
            userData->showWindow();
        });
    };
    platformIO.Platform_SetWindowPos = [](ImGuiViewport *viewport, ImVec2 pos) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        __block const ImVec2 localPos(pos);
        userData->runBlockOnMainQueue(^(ViewportWindow *userData) {
            userData->setWindowPos(localPos);
        });
    };
    platformIO.Platform_GetWindowPos = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        __block ImVec2 pos;
        userData->runBlockOnMainQueue(^(ViewportWindow *userData) {
            pos = userData->windowPos();
        });
        return pos;
    };
    platformIO.Platform_SetWindowSize = [](ImGuiViewport *viewport, ImVec2 size) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        __block const ImVec2 localSize(size);
        userData->runBlockOnMainQueue(^(ViewportWindow *userData) {
            userData->setWindowSize(localSize);
        });
    };
    platformIO.Platform_GetWindowSize = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        __block ImVec2 size;
        userData->runBlockOnMainQueue(^(ViewportWindow *userData) {
            size = userData->windowSize();
        });
        return size;
    };
    platformIO.Platform_SetWindowFocus = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        userData->runBlockOnMainQueue(^(ViewportWindow *userData) {
            userData->makeWindowFocus();
        });
    };
    platformIO.Platform_GetWindowFocus = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        __block bool focused = false;
        userData->runBlockOnMainQueue(^(ViewportWindow *userData) {
            focused = userData->hasWindowFocus();
        });
        return focused;
    };
    platformIO.Platform_GetWindowMinimized = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        __block bool minimized = false;
        userData->runBlockOnMainQueue(^(ViewportWindow *userData) {
            minimized = userData->isWindowMinimized();
        });
        return minimized;
    };
    platformIO.Platform_SetWindowTitle = [](ImGuiViewport *viewport, const char *title) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        __block NSString *localTitle = [[NSString alloc] initWithUTF8String:title];
        userData->runBlockOnMainQueue(^(ViewportWindow *userData) {
            userData->setWindowTitle(localTitle);
        });
    };
    platformIO.Platform_SetWindowAlpha = [](ImGuiViewport *viewport, float alpha) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        __block CGFloat localAlpha = alpha;
        userData->runBlockOnMainQueue(^(ViewportWindow *userData) {
            userData->setWindowAlpha(localAlpha);
        });
    };
    platformIO.Platform_UpdateWindow = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        __block bool updated = false;
        userData->runBlockOnMainQueue(^(ViewportWindow *userData) {
            updated = userData->updateWindow();
        });
        viewport->PlatformRequestMove = viewport->PlatformRequestResize = updated;
    };
    platformIO.Platform_GetWindowDpiScale = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        __block float dpiScale = 1.0f;
        userData->runBlockOnMainQueue(^(ViewportWindow *userData) {
            dpiScale = userData->m_window.backingScaleFactor;
        });
        return dpiScale;
    };
    platformIO.Platform_OnChangedViewport = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        BX_UNUSED_1(userData);
    };
    ImGuiViewport *main = ImGui::GetMainViewport();
    NSView *view = (__bridge NSView *) m_nativeView;
    main->PlatformHandle = this;
    main->PlatformHandleRaw = (__bridge void *) view.window;
    ViewportWindow *data = IM_NEW(ViewportWindow);
    data->m_window = view.window;
    main->PlatformUserData = data;
    sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_METAL_MACOS) {
        io.BackendRendererUserData = this;
        platformIO.Renderer_CreateWindow = [](ImGuiViewport *viewport) {
            auto self = static_cast<CocoaThreadedApplicationService *>(ImGui::GetIO().BackendRendererUserData);
            BX_UNUSED_1(self);
            MetalRendererData *userData = IM_NEW(MetalRendererData);
            NSWindow *window = (__bridge NSWindow *) viewport->PlatformHandleRaw;
            MTKView *view = (MTKView *) window.contentView;
            userData->m_view = view;
            userData->m_texture = view.currentDrawable.texture;
            viewport->RendererUserData = userData;
        };
        platformIO.Renderer_DestroyWindow = [](ImGuiViewport *viewport) {
            if (auto userData = static_cast<MetalRendererData *>(viewport->RendererUserData)) {
                IM_DELETE(userData);
            }
            viewport->RendererUserData = nullptr;
        };
        platformIO.Renderer_RenderWindow = [](ImGuiViewport *viewport, void *opaque) {
            if (auto userData = static_cast<MetalRendererData *>(viewport->RendererUserData)) {
                auto self = static_cast<CocoaThreadedApplicationService *>(ImGui::GetIO().BackendRendererUserData);
                if (Project *project = self->projectHolder()->currentProject()) {
                    bool load =
                        EnumUtils::isEnabledT<ImGuiViewportFlags>(viewport->Flags, ImGuiViewportFlags_NoRendererClear);
                    auto window = static_cast<internal::ImGuiWindow *>(opaque);
                    void *view = self->m_nativeView;
                    id<MTLTexture> texture = self->m_texture;
                    self->m_nativeView = (__bridge void *) userData->m_view;
                    self->m_texture = userData->m_texture;
                    window->drawWindow(project, viewport->DrawData, load);
                    self->m_nativeView = view;
                    self->m_texture = texture;
                }
                BX_UNUSED_1(opaque);
            }
        };
        platformIO.Renderer_SetWindowSize = [](ImGuiViewport *viewport, ImVec2 size) {
            if (auto userData = static_cast<MetalRendererData *>(viewport->RendererUserData)) {
                BX_UNUSED_1(size);
            }
        };
        io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
        const ImGuiBackendFlags backendFlags = io.BackendFlags;
        if (EnumUtils::isEnabledT<ImGuiBackendFlags>(backendFlags, ImGuiBackendFlags_PlatformHasViewports) &&
            EnumUtils::isEnabledT<ImGuiBackendFlags>(backendFlags, ImGuiBackendFlags_RendererHasViewports)) {
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        }
    }
    updateAllMonitors();
#else /* IMGUI_HAS_VIEWPORT */
    ImGui::GetMainViewport()->PlatformHandleRaw = this;
    io.SetPlatformImeDataFn = [](ImGuiViewport *viewport, ImGuiPlatformImeData *data) {
        const ImGuiIO &io = ImGui::GetIO();
        const nanoem_f32_t fx = data->InputPos.x * (1.0f / io.DisplayFramebufferScale.x),
                           fy = data->InputPos.y * (1.0f / io.DisplayFramebufferScale.y) + data->InputLineHeight;
        auto self = static_cast<CocoaThreadedApplicationService *>(viewport->PlatformHandleRaw);
        self->m_textInputOrigin = Vector2SI32(fx, fy);
    };
#endif /* IMGUI_HAS_VIEWPORT */
}

void
CocoaThreadedApplicationService::handleNewProject()
{
    ThreadedApplicationService::handleNewProject();
    setupNewProject();
}

void
CocoaThreadedApplicationService::handleSuspendProject()
{
    const sg_backend backend = sg::query_backend();
    if (m_displayLink) {
        CVDisplayLinkStop(m_displayLink);
    }
    else if (backend == SG_BACKEND_METAL_MACOS) {
        [nativeView() releaseDrawables];
    }
}

void
CocoaThreadedApplicationService::handleResumeProject()
{
    if (m_displayLink) {
        CVDisplayLinkStart(m_displayLink);
    }
}

void
CocoaThreadedApplicationService::handleDestructApplication()
{
    for (auto &image : m_colorImages) {
        sg::destroy_image(image.second);
    }
    for (auto &image : m_depthImages) {
        sg::destroy_image(image.second);
    }
    for (auto &pass : m_passes) {
        sg::destroy_pass(pass.second);
    }
    if (m_displayLink) {
        CVDisplayLinkStop(m_displayLink);
        CVDisplayLinkRelease(m_displayLink);
        m_displayLink = nullptr;
    }
}

void
CocoaThreadedApplicationService::handleTerminateApplication()
{
    deleteCrashRecoveryFile();
    if (m_hidManager) {
        IOHIDManagerRef manager = static_cast<IOHIDManagerRef>(m_hidManager);
        IOHIDManagerClose(manager, kIOHIDOptionsTypeNone);
        CFRelease(manager);
        m_hidManager = nullptr;
    }
    if (m_menuClient) {
        m_menuClient->close();
        nanoem_delete_safe(m_menuClient);
    }
    m_nativeContext = nil;
    m_nativeDevice = nil;
    m_nativeView = nil;
}

sg_pixel_format
CocoaThreadedApplicationService::defaultPassPixelFormat() const noexcept
{
    const sg_backend backend = sg::query_backend();
    return backend == SG_BACKEND_METAL_MACOS ? resolveMetalPixelFormat()
                                             : ThreadedApplicationService::defaultPassPixelFormat();
}

void
CocoaThreadedApplicationService::beginDefaultPass(
    nanoem_u32_t windowID, const sg_pass_action &pa, int width, int height, int &sampleCount)
{
    const sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_METAL_MACOS) {
        auto it = m_colorImageDescriptions.find(windowID);
        auto beginPass = [this, windowID, width, height, sampleCount, &pa](sg_image_desc &desc) {
            const bool needsUpdatingDepthImage = desc.width != width || desc.height != height;
            auto view = (__bridge MTKView *) m_nativeView;
            id<MTLTexture> texture = view.currentDrawable.texture;
            desc.width = width;
            desc.height = height;
            desc.pixel_format = resolveMetalPixelFormat();
            desc.render_target = true;
            desc.mtl_textures[0] = (__bridge const void *) texture;
            if (Inline::isDebugLabelEnabled()) {
                desc.label = "@nanoem/Metal/CurrentDrawable/Color";
            }
            auto &colorImage = m_colorImages[windowID];
            sg::destroy_image(colorImage);
            colorImage = sg::make_image(&desc);
            auto &depthImage = m_depthImages[windowID];
            if (needsUpdatingDepthImage) {
                sg_image_desc dd(desc);
                dd.mtl_textures[0] = nullptr;
                dd.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
                dd.sample_count = sampleCount;
                if (Inline::isDebugLabelEnabled()) {
                    dd.label = "@nanoem/Metal/CurrentDrawable/DepthStencil";
                }
                sg::destroy_image(depthImage);
                depthImage = sg::make_image(&dd);
            }
            sg_pass_desc pd = {};
            pd.color_attachments[0].image = colorImage;
            pd.depth_stencil_attachment.image = depthImage;
            if (Inline::isDebugLabelEnabled()) {
                pd.label = "@nanoem/Metal/CurrentDrawable/Pass";
            }
            auto &pass = m_passes[windowID];
            sg::destroy_pass(pass);
            pass = sg::make_pass(&pd);
            sg::begin_pass(pass, &pa);
            m_texture = texture;
        };
        if (it != m_colorImageDescriptions.end()) {
            auto &desc = it->second;
            beginPass(desc);
        }
        else {
            sg_image_desc desc = {};
            beginPass(desc);
            m_colorImageDescriptions.insert(tinystl::make_pair(windowID, desc));
        }
    }
    else if (backend == SG_BACKEND_GLCORE33) {
        ThreadedApplicationService::beginDefaultPass(windowID, pa, width, height, sampleCount);
    }
}

void
CocoaThreadedApplicationService::postEmptyApplicationEvent()
{
    sendDummyEvent([NSApplication sharedApplication], false);
    if (m_menuClient) {
        m_menuClient->receiveAllEventMessages();
    }
}

void
CocoaThreadedApplicationService::presentDefaultPass(const Project *project)
{
    SG_PUSH_GROUP("macos::CocoaThreadedApplicationService::presentDefaultPass");
    const sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_METAL_MACOS) {
        m_texture = nil;
        MTKView *contentView = nativeView();
        [contentView draw];
#if defined(IMGUI_HAS_VIEWPORT)
        for (auto viewport : ImGui::GetPlatformIO().Viewports) {
            if (auto userData = static_cast<MetalRendererData *>(viewport->RendererUserData)) {
                MTKView *viewportView = userData->m_view;
                [viewportView.currentDrawable present];
                [viewportView draw];
                userData->m_texture = viewportView.currentDrawable.texture;
            }
        }
#endif /* IMGUI_HAS_VIEWPORT */
        uint32_t sampleCount = project->sampleCount();
        if (contentView.sampleCount != sampleCount) {
            contentView.sampleCount = sampleCount;
        }
        bool expected = true;
        if (m_displaySyncChanged.compare_exchange_strong(expected, false)) {
            if (@available(macOS 10.13, *)) {
                contentView.currentDrawable.layer.displaySyncEnabled = m_displaySyncEnabled;
            }
        }
    }
    else if (backend == SG_BACKEND_GLCORE33) {
        NSOpenGLContext *context = (__bridge NSOpenGLContext *) m_nativeContext;
        [context makeCurrentContext];
        [context flushBuffer];
        dispatch_semaphore_wait(m_semaphore, DISPATCH_TIME_FOREVER);
        bool expected = true;
        if (m_displaySyncChanged.compare_exchange_strong(expected, false)) {
            int interval = m_displaySyncEnabled ? 1 : 0;
            [context setValues:&interval forParameter:NSOpenGLCPSwapInterval];
        }
    }
    SG_POP_GROUP();
}

bool
CocoaThreadedApplicationService::loadFromFile(
    const URI &fileURI, Project *project, IFileManager::DialogType type, Error &error)
{
    bool result = false;
    if (type == IFileManager::kDialogTypeOpenProject && fileURI.pathExtension() == String("nanoem")) {
        nanoem_application_document_t *document =
            nanoemApplicationDocumentCreate(this, projectHolder(), fileURI.absolutePathConstString());
        if (nanoemApplicationDocumentLoad(document, &error)) {
            nanoem_application_document_t *lastDocument = m_document;
            m_document = document;
            nanoemApplicationDocumentDestroy(lastDocument);
            projectHolder()->currentProject()->writeRedoMessage();
            result = true;
        }
        else {
            nanoemApplicationDocumentDestroy(document);
        }
    }
    else {
        result = ThreadedApplicationService::loadFromFile(fileURI, project, type, error);
    }
    return result;
}

bool
CocoaThreadedApplicationService::saveAsFile(
    const URI &fileURI, Project *project, IFileManager::DialogType type, Error &error)
{
    bool result = false;
    if (type == IFileManager::kDialogTypeSaveProjectFile && fileURI.pathExtension() == String("nanoem")) {
        if (!m_document) {
            m_document = nanoemApplicationDocumentCreate(this, projectHolder(), fileURI.absolutePathConstString());
        }
        if (nanoemApplicationDocumentSave(m_document, fileURI.absolutePathConstString(), &error)) {
            project->writeRedoMessage();
            result = true;
        }
    }
    else {
        result = ThreadedApplicationService::saveAsFile(fileURI, project, type, error);
    }
    return result;
}

URI
CocoaThreadedApplicationService::recoverableRedoFileURI() const
{
    int fd = ::open(s_crashFilePath, O_RDONLY);
    if (fd != -1) {
        ::read(fd, s_redoFilePath, sizeof(s_redoFilePath));
        ::close(fd);
    }
    return URI::createFromFilePath(s_redoFilePath);
}

void
CocoaThreadedApplicationService::deleteCrashRecoveryFile()
{
    FileUtils::deleteFile(s_crashFilePath);
}

sg_pixel_format
CocoaThreadedApplicationService::resolveMetalPixelFormat() const noexcept
{
    nanoem_assert(sg::query_backend() == SG_BACKEND_METAL_MACOS, "must be Metal renderer");
    MTKView *view = (__bridge MTKView *) m_nativeView;
    switch (view.colorPixelFormat) {
    case MTLPixelFormatBGRA8Unorm:
    case MTLPixelFormatBGRA8Unorm_sRGB:
        return SG_PIXELFORMAT_BGRA8;
    case MTLPixelFormatBGR10A2Unorm:
        return SG_PIXELFORMAT_RGB10A2;
    case MTLPixelFormatRGBA16Float:
        return SG_PIXELFORMAT_RGBA16F;
    case MTLPixelFormatRGBA32Float:
        return SG_PIXELFORMAT_RGBA32F;
    default:
        return SG_PIXELFORMAT_NONE;
    }
}

bool
CocoaThreadedApplicationService::isTrackpadOnly() const
{
    static const NSUInteger kHIDVendorIDApple = 0x5ac;
    bool found = false;
    for (id item in m_devices) {
        IOHIDDeviceRef device = (__bridge IOHIDDeviceRef) item;
        NSNumber *vendorID = (__bridge NSNumber *) IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey));
        NSString *productString = (__bridge NSString *) IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
        if (vendorID.unsignedIntValue == kHIDVendorIDApple &&
            [productString rangeOfString:@"Trackpad"].location != NSNotFound) {
            found = true;
            break;
        }
    }
    return found && m_devices.count == 1;
}

void
CocoaThreadedApplicationService::setupNewProject()
{
    Project *project = projectHolder()->currentProject();
    setupRedoFilePath(project);
#if !defined(NANOEM_ENABLE_HIDMANAGER_MONITORING)
    CFSetRef deviceSet = IOHIDManagerCopyDevices(static_cast<IOHIDManagerRef>(m_hidManager));
    NSSet *devices = (__bridge NSSet *) deviceSet;
    [m_devices removeAllObjects];
    for (id device in devices) {
        [m_devices addObject:device];
    }
    CFRelease(deviceSet);
#endif /* NANOEM_ENABLE_HIDMANAGER_MONITORING */
    project->setPrimaryCursorTypeLeft(isTrackpadOnly());
    const sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_METAL_MACOS) {
        project->setViewportPixelFormat(resolveMetalPixelFormat());
        MTKView *view = (__bridge MTKView *) m_nativeView;
        uint32_t sampleCount = project->sampleCount();
        if (view.sampleCount != sampleCount) {
            view.sampleCount = sampleCount;
        }
    }
}

void
CocoaThreadedApplicationService::setupRedoFilePath(Project *project)
{
    const char *redoPath = json_object_dotget_string(json_object(applicationConfiguration()), "macos.redo.path");
    NSURL *redoDirectoryURL = [[NSURL alloc] initFileURLWithPath:[[NSString alloc] initWithUTF8String:redoPath]];
    NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
    formatter.dateFormat = @"yyyy-MM-dd-HHmmss";
    NSString *formattedDateTime = [formatter stringFromDate:[NSDate date]];
    NSString *filename =
        [[NSString alloc] initWithFormat:@"nanoem-%@.%s", formattedDateTime, Project::kRedoLogFileExtension];
    NSURL *redoFileURL = [redoDirectoryURL URLByAppendingPathComponent:filename];
    project->setRedoFileURI(canonicalFileURI(redoFileURL));
    strlcpy(s_redoFilePath, redoFileURL.path.UTF8String, sizeof(s_redoFilePath));
    int fd = ::open(s_crashFilePath, O_WRONLY | O_CREAT | O_EXCL | O_EXLOCK, 0600);
    if (fd != -1) {
        ::write(fd, s_redoFilePath, bx::strLen(s_redoFilePath, sizeof(s_redoFilePath)));
        ::close(fd);
    }
}

void
CocoaThreadedApplicationService::initializeInputDevices()
{
#if defined(NANOEM_ENABLE_HIDMANAGER_MONITORING)
    static CFStringRef kInitializationLoopMode = CFSTR("nanoem.io.init");
    if (!m_hidManager) {
        NSDictionary *dict = @{
            @kIOHIDDeviceUsagePageKey : @(kHIDPage_GenericDesktop),
            @kIOHIDDeviceUsageKey : @(kHIDUsage_GD_Mouse),
        };
        IOHIDManagerRef manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
        IOHIDManagerRegisterDeviceMatchingCallback(
            manager,
            [](void *context, IOReturn res, void *sender, IOHIDDeviceRef device) {
                BX_UNUSED_1(sender);
                if (res == kIOReturnSuccess) {
                    auto self = static_cast<CocoaThreadedApplicationService *>(context);
                    [self->m_devices addObject:(__bridge id) device];
                    if (Project *project = self->projectHolder()->currentProject()) {
                        project->setPrimaryCursorTypeLeft(self->isTrackpadOnly());
                    }
                }
            },
            this);
        IOHIDManagerRegisterDeviceRemovalCallback(
            manager,
            [](void *context, IOReturn res, void *sender, IOHIDDeviceRef device) {
                BX_UNUSED_1(sender);
                if (res == kIOReturnSuccess) {
                    auto self = static_cast<CocoaThreadedApplicationService *>(context);
                    [self->m_devices removeObject:(__bridge id) device];
                    CFRelease(device);
                    if (Project *project = self->projectHolder()->currentProject()) {
                        project->setPrimaryCursorTypeLeft(self->isTrackpadOnly());
                    }
                }
            },
            this);
        IOHIDManagerSetDeviceMatching(manager, (__bridge CFDictionaryRef) dict);
        IOHIDManagerScheduleWithRunLoop(manager, CFRunLoopGetCurrent(), kInitializationLoopMode);
        IOHIDManagerOpen(manager, kIOHIDOptionsTypeNone);
        while (CFRunLoopRunInMode(kInitializationLoopMode, 0, TRUE) == kCFRunLoopRunHandledSource) {
        }
        IOHIDManagerClose(manager, kIOHIDOptionsTypeNone);
        IOHIDManagerUnscheduleFromRunLoop(manager, CFRunLoopGetCurrent(), kInitializationLoopMode);
        IOHIDManagerScheduleWithRunLoop(manager, CFRunLoopGetMain(), kCFRunLoopCommonModes);
        IOHIDManagerOpen(manager, kIOHIDOptionsTypeNone);
        m_hidManager = manager;
    }
#else /* NANOEM_ENABLE_HIDMANAGER_MONITORING */
    if (!m_hidManager) {
        NSDictionary *dict = @{
            @kIOHIDDeviceUsagePageKey : @(kHIDPage_GenericDesktop),
            @kIOHIDDeviceUsageKey : @(kHIDUsage_GD_Mouse),
        };
        IOHIDManagerRef manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
        IOHIDManagerSetDeviceMatching(manager, (__bridge CFDictionaryRef) dict);
        m_hidManager = manager;
    }
#endif /* NANOEM_ENABLE_HIDMANAGER_MONITORING */
}

void
CocoaThreadedApplicationService::updateAllMonitors()
{
#if defined(IMGUI_HAS_VIEWPORT)
    ImGuiPlatformIO &io = ImGui::GetPlatformIO();
    io.Monitors.resize(0);
    dispatch_sync(dispatch_get_main_queue(), ^() {
        const NSScreen *mainScreen = [NSScreen mainScreen];
        for (NSScreen *screen : [NSScreen screens]) {
            ImGuiPlatformMonitor monitor;
            const NSRect rect = [screen convertRectToBacking:screen.frame],
                         visibleRect = [screen convertRectToBacking:screen.visibleFrame];
            monitor.MainPos = ImVec2(rect.origin.x, rect.origin.y);
            monitor.MainSize = ImVec2(rect.size.width, rect.size.height);
            monitor.WorkPos =
                ImVec2(visibleRect.origin.x, visibleRect.origin.y + rect.size.height - visibleRect.size.height);
            monitor.WorkSize = ImVec2(visibleRect.size.width, visibleRect.size.height);
            monitor.DpiScale = screen.backingScaleFactor;
            if (screen == mainScreen) {
                io.Monitors.push_front(monitor);
            }
            else {
                io.Monitors.push_back(monitor);
            }
        }
    });
#endif /* IMGUI_HAS_VIEWPORT */
}

} /* namespace macos */
} /* namespace nanoem */
