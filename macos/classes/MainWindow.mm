/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import "MainWindow.h"

#import <AVFoundation/AVFoundation.h>
#import <AppKit/AppKit.h>
#import <IOKit/ps/IOPowerSources.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <OpenGL/gl.h>

#import "CocoaThreadedApplicationService.h"
#import "Preference.h"

#include "bx/commandline.h"
#include "emapp/emapp.h"
#include "emapp/private/CommonInclude.h"
#include "sokol/sokol_time.h"

#include <asl.h>
#include <sys/sysctl.h>

#ifndef NANOEM_SENTRY_DSN
#define NANOEM_SENTRY_DSN ""
#endif /* NANOEM_SENTRY_DSN */

using namespace nanoem;

static NSDragOperation
handleDraggingEntered(id<NSDraggingInfo> sender)
{
    NSPasteboard *pboard = [sender draggingPasteboard];
    NSDragOperation operation = NSDragOperationNone;
    if ([pboard.types containsObject:NSFilenamesPboardType]) {
        NSDragOperation mask = [sender draggingSourceOperationMask];
        if (EnumUtils::isEnabledT<NSUInteger>(mask, NSDragOperationLink)) {
            operation = NSDragOperationLink;
        }
        else if (EnumUtils::isEnabledT<NSUInteger>(mask, NSDragOperationCopy)) {
            operation = NSDragOperationCopy;
        }
    }
    return operation;
}

static BOOL
handlePerformDragOperation(id<NSDraggingInfo> sender, macos::MainWindow *window)
{
    BOOL performed = NO;
    NSPasteboard *pboard = [sender draggingPasteboard];
    NSDragOperation mask = [sender draggingSourceOperationMask];
    ThreadedApplicationClient *client = window->client();
    if ([[pboard types] containsObject:NSFilenamesPboardType]) {
        NSArray<NSString *> *files = [pboard propertyListForType:NSFilenamesPboardType];
        if ((EnumUtils::isEnabledT<NSUInteger>(mask, NSDragOperationLink) ||
                EnumUtils::isEnabledT<NSUInteger>(mask, NSDragOperationCopy)) &&
            files.count > 0) {
            for (NSString *filePath in files) {
                const URI &fileURI = URI::createFromFilePath(filePath.UTF8String);
                client->sendDropFileMessage(fileURI);
                if (Project::isLoadableExtension(fileURI)) {
                    window->setTitle(filePath.lastPathComponent);
                }
            }
        }
    }
    return performed;
}

@interface MainWindowOpenGLView : NSOpenGLView {
    macos::MainWindow *m_window;
    NSTrackingArea *m_trackingArea;
}

- (instancetype)initWithFrame:(NSRect)frameRect
                  pixelFormat:(NSOpenGLPixelFormat *)format
                   mainWindow:(macos::MainWindow *)window;
- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender;
- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender;

@end

@interface MainWindowMetalView : MTKView <NSTextInputClient> {
    macos::MainWindow *m_window;
    NSTrackingArea *m_trackingArea;
}

- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device mainWindow:(macos::MainWindow *)window;
- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender;
- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender;

@end

@implementation MainWindowOpenGLView

- (instancetype)initWithFrame:(NSRect)frameRect
                  pixelFormat:(NSOpenGLPixelFormat *)format
                   mainWindow:(macos::MainWindow *)window
{
    if (self = [super initWithFrame:frameRect pixelFormat:format]) {
        m_window = window;
    }
    return self;
}

- (void)updateTrackingAreas
{
    if (m_trackingArea != nil) {
        [self removeTrackingArea:m_trackingArea];
    }
    const NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow |
        NSTrackingEnabledDuringMouseDrag | NSTrackingCursorUpdate | NSTrackingInVisibleRect | NSTrackingAssumeInside;
    m_trackingArea = [[NSTrackingArea alloc] initWithRect:self.bounds options:options owner:self userInfo:nil];
    [self addTrackingArea:m_trackingArea];
    [super updateTrackingAreas];
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    return handleDraggingEntered(sender);
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    return handlePerformDragOperation(sender, m_window);
}

/* NSResponder */

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)mouseDown:(NSEvent *)event
{
    m_window->handleMouseDown(event);
}

- (void)mouseMoved:(NSEvent *)event
{
    m_window->handleMouseMoved(event);
}

- (void)mouseDragged:(NSEvent *)event
{
    m_window->handleMouseDragged(event);
}

- (void)mouseUp:(NSEvent *)event
{
    m_window->handleMouseUp(event);
}

- (void)mouseExited:(NSEvent *)event
{
    m_window->handleMouseExit(event);
}

- (void)rightMouseDown:(NSEvent *)event
{
    m_window->handleRightMouseDown(event);
}

- (void)rightMouseDragged:(NSEvent *)event
{
    m_window->handleRightMouseDragged(event);
}

- (void)rightMouseUp:(NSEvent *)event
{
    m_window->handleRightMouseUp(event);
}

- (void)otherMouseDown:(NSEvent *)event
{
    m_window->handleOtherMouseDown(event);
}

- (void)otherMouseDragged:(NSEvent *)event
{
    m_window->handleOtherMouseDragged(event);
}

- (void)otherMouseUp:(NSEvent *)event
{
    m_window->handleOtherMouseUp(event);
}

- (void)keyDown:(NSEvent *)event
{
    m_window->handleKeyDown(event);
}

- (void)keyUp:(NSEvent *)event
{
    m_window->handleKeyUp(event);
}

- (void)scrollWheel:(NSEvent *)event
{
    m_window->handleScrollWheel(event);
}

/* NSTextInputClient */

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
    BX_UNUSED_1(replacementRange);
    m_window->handleInsertText(string);
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
    m_window->handleSetMarkerText(string, selectedRange);
}

- (void)unmarkText
{
    m_window->handleUnmarkText();
}

- (NSRange)selectedRange
{
    return m_window->selectedRange();
}

- (NSRange)markedRange
{
    return m_window->markedRange();
}

- (BOOL)hasMarkedText
{
    return m_window->hasMarkedText();
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
    return m_window->firstRectForCharacterRange();
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point
{
    BX_UNUSED_1(point);
    return NSNotFound;
}

- (NSAttributedString *)attributedString
{
    return m_window->attributedString();
}

@end

@implementation MainWindowMetalView

- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device mainWindow:(macos::MainWindow *)window
{
    if (self = [super initWithFrame:frameRect device:device]) {
        self.autoResizeDrawable = NO;
        self.enableSetNeedsDisplay = NO;
        self.paused = YES;
        self.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
        m_window = window;
    }
    return self;
}

- (void)updateTrackingAreas
{
    if (m_trackingArea != nil) {
        [self removeTrackingArea:m_trackingArea];
    }
    const NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow |
        NSTrackingEnabledDuringMouseDrag | NSTrackingCursorUpdate | NSTrackingInVisibleRect | NSTrackingAssumeInside;
    m_trackingArea = [[NSTrackingArea alloc] initWithRect:self.bounds options:options owner:self userInfo:nil];
    [self addTrackingArea:m_trackingArea];
    [super updateTrackingAreas];
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    return handleDraggingEntered(sender);
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    return handlePerformDragOperation(sender, m_window);
}

/* NSResponder */

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)mouseDown:(NSEvent *)event
{
    m_window->handleMouseDown(event);
}

- (void)mouseMoved:(NSEvent *)event
{
    m_window->handleMouseMoved(event);
}

- (void)mouseDragged:(NSEvent *)event
{
    m_window->handleMouseDragged(event);
}

- (void)mouseUp:(NSEvent *)event
{
    m_window->handleMouseUp(event);
}

- (void)mouseExited:(NSEvent *)event
{
    m_window->handleMouseExit(event);
}

- (void)rightMouseDown:(NSEvent *)event
{
    m_window->handleRightMouseDown(event);
}

- (void)rightMouseDragged:(NSEvent *)event
{
    m_window->handleRightMouseDragged(event);
}

- (void)rightMouseUp:(NSEvent *)event
{
    m_window->handleRightMouseUp(event);
}

- (void)otherMouseDown:(NSEvent *)event
{
    m_window->handleOtherMouseDown(event);
}

- (void)otherMouseDragged:(NSEvent *)event
{
    m_window->handleOtherMouseDragged(event);
}

- (void)otherMouseUp:(NSEvent *)event
{
    m_window->handleOtherMouseUp(event);
}

- (void)keyDown:(NSEvent *)event
{
    m_window->handleKeyDown(event);
}

- (void)keyUp:(NSEvent *)event
{
    m_window->handleKeyUp(event);
}

- (void)scrollWheel:(NSEvent *)event
{
    m_window->handleScrollWheel(event);
}

/* NSTextInputClient */

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
    BX_UNUSED_1(replacementRange);
    m_window->handleInsertText(string);
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
    m_window->handleSetMarkerText(string, selectedRange);
}

- (void)unmarkText
{
    m_window->handleUnmarkText();
}

- (NSRange)selectedRange
{
    return m_window->selectedRange();
}

- (NSRange)markedRange
{
    return m_window->markedRange();
}

- (BOOL)hasMarkedText
{
    return m_window->hasMarkedText();
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
    return m_window->firstRectForCharacterRange();
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point
{
    BX_UNUSED_1(point);
    return NSNotFound;
}

- (NSAttributedString *)attributedString
{
    return m_window->attributedString();
}

@end

namespace nanoem {
namespace macos {

MainWindow::FPSThresholder::FPSThresholder(uint32_t value, bool enabled)
    : m_value(value)
    , m_preferred(value)
    , m_enabled(enabled)
{
}

void
MainWindow::FPSThresholder::threshold(uint32_t displayFrequency)
{
    if (m_enabled) {
        const uint32_t value = m_value > 0 ? m_value : m_preferred;
        if (value > 0 && value < displayFrequency) {
            bx::sleep(uint32_t((1.0f / (value + 1)) * 1000.0f));
        }
    }
}

MainWindow::MainWindow(const bx::CommandLine *cmd, macos::CocoaThreadedApplicationService *service,
    ThreadedApplicationClient *client, NSWindow *nativeWindow, Preference *preference)
    : m_translator(service->translator())
    , m_commandLine(cmd)
    , m_service(service)
    , m_client(client)
    , m_nativeWindow(nativeWindow)
    , m_preference(preference)
    , m_lazyExecutionQueue(dispatch_queue_create(nullptr, nullptr))
    , m_metricsQueue(dispatch_queue_create(nullptr, nullptr))
    , m_metricsSemaphore(dispatch_semaphore_create(0))
    , m_menu(this, client, m_translator, preference)
    , m_runningMetrics(true)
{
    m_menu.build();
    m_menu.registerDocumentEditEventListener();
    registerAllPrerequisiteEventListeners();
}

MainWindow::~MainWindow()
{
    if (m_logger) {
        asl_close(static_cast<asl_object_t>(m_logger));
        m_logger = nullptr;
    }
}

void
MainWindow::initialize()
{
    CFStringRef reason = CFSTR("nanoem");
    IOPMAssertionCreateWithName(kIOPMAssertPreventUserIdleDisplaySleep, kIOPMAssertionLevelOn, reason, &m_displayKey);
    IOPMAssertionCreateWithName(kIOPMAssertPreventUserIdleSystemSleep, kIOPMAssertionLevelOn, reason, &m_idleKey);
    CGDisplayRegisterReconfigurationCallback(
        [](CGDirectDisplayID display, CGDisplayChangeSummaryFlags flags, void *userInfo) {
            if (EnumUtils::isEnabled(flags, kCGDisplaySetMainFlag)) {
                auto self = static_cast<MainWindow *>(userInfo);
                self->updateDisplayFrequency(display);
            }
        },
        this);
    updateDisplayFrequency(CGMainDisplayID());
    CFRunLoopSourceRef source = IOPSCreateLimitedPowerNotification(
        [](void *userData) {
            auto self = static_cast<MainWindow *>(userData);
            self->updatePreferredFPS();
        },
        this);
    CFRunLoopAddSource(CFRunLoopGetMain(), source, kCFRunLoopDefaultMode);
    CFRelease(source);
    updatePreferredFPS();
    NSURL *bundlePluginsURL = [NSBundle mainBundle].builtInPlugInsURL;
    String pluginPath(bundlePluginsURL.path.UTF8String);
    const char *rendererBackend = m_preference->applicationPreference()->rendererBackend();
    sg_pixel_format pixelFormat = SG_PIXELFORMAT_RGBA8;
    bool isLowPower = true;
    if (StringUtils::equalsIgnoreCase(rendererBackend, BaseApplicationService::kRendererMetal) &&
        Preference::isMetalAvailable()) {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        initializeMetal(device, pixelFormat);
        pluginPath.append("/sokol_metal_macos.dylib");
#ifdef NDEBUG
        isLowPower = device.lowPower == YES;
#endif /* NDEBUG */
        m_textInputContext = [[NSTextInputContext alloc] initWithClient:m_nativeWindow.contentView];
    }
    else {
        initializeOpenGL();
        pluginPath.append("/sokol_glcore33.dylib");
    }
    const ApplicationPreference *preference = m_preference->applicationPreference();
    const CGSize &frameSize = m_nativeWindow.contentView.frame.size;
    const Vector2UI16 logicalWindowSize(frameSize.width, frameSize.height);
    ThreadedApplicationClient::InitializeMessageDescription desc(
        logicalWindowSize, pixelFormat, m_nativeWindow.backingScaleFactor, pluginPath);
    ApplicationPreference::HighDPIViewportModeType highDPIViewportMode = preference->highDPIViewportMode();
    desc.m_bufferPoolSize = preference->gfxBufferPoolSize();
    desc.m_imagePoolSize = preference->gfxImagePoolSize();
    desc.m_shaderPoolSize = preference->gfxShaderPoolSize();
    desc.m_passPoolSize = preference->gfxPassPoolSize();
    desc.m_pipelinePoolSize = preference->gfxPipelinePoolSize();
    desc.m_metalGlobalUniformBufferSize = preference->gfxUniformBufferSize();
    if (highDPIViewportMode == ApplicationPreference::kHighDPIViewportModeDisabled ||
        (highDPIViewportMode == ApplicationPreference::kHighDPIViewportModeAuto && isLowPower)) {
        desc.m_viewportDevicePixelRatio = 1.0f;
    }
    m_client->sendInitializeMessage(desc);
}

void
MainWindow::processMessage(NSApplication *app)
{
    @autoreleasepool {
        m_client->receiveAllEventMessages();
        /* flush and send all NSEvent objects */
        while (NSEvent *event = [app nextEventMatchingMask:NSEventMaskAny
                                                 untilDate:[NSDate distantPast]
                                                    inMode:NSDefaultRunLoopMode
                                                   dequeue:YES]) {
            [app sendEvent:event];
            m_client->receiveAllEventMessages();
        }
        if (m_runningWindow) {
            /* poll a NSEvent event for one second */
            if (NSEvent *event = [app nextEventMatchingMask:NSEventMaskAny
                                                  untilDate:[NSDate dateWithTimeIntervalSinceNow:1]
                                                     inMode:NSDefaultRunLoopMode
                                                    dequeue:YES]) {
                [app sendEvent:event];
            }
            m_client->receiveAllEventMessages();
        }
    }
}

void
MainWindow::terminate()
{
    [m_nativeWindow orderOut:nil];
    destroyWatchEffectSource();
    if (m_displayKey) {
        IOPMAssertionRelease(m_displayKey);
        m_displayKey = 0;
    }
    if (m_idleKey) {
        IOPMAssertionRelease(m_idleKey);
        m_idleKey = 0;
    }
}

const ThreadedApplicationClient *
MainWindow::client() const noexcept
{
    return m_client;
}

ThreadedApplicationClient *
MainWindow::client() noexcept
{
    return m_client;
}

const ThreadedApplicationService *
MainWindow::service() const noexcept
{
    return m_service;
}

ThreadedApplicationService *
MainWindow::service() noexcept
{
    return m_service;
}

NSWindow *
MainWindow::nativeWindow() noexcept
{
    return m_nativeWindow;
}

NSAttributedString *
MainWindow::attributedString()
{

    NSAttributedString *value = nil;
    if (m_markedString.length == 0) {
        value = [[NSAttributedString alloc] init];
    }
    else {
        m_selectedRange = NSMakeRange(0, m_markedString.length);
        value = m_markedString;
    }
    return value;
}

NSRange
MainWindow::selectedRange() const noexcept
{
    return m_selectedRange;
}

NSRange
MainWindow::markedRange() const noexcept
{
    return hasMarkedText() ? NSMakeRange(0, m_markedString.length) : NSMakeRange(NSNotFound, 0);
}

NSRect
MainWindow::firstRectForCharacterRange()
{
#if defined(IMGUI_HAS_VIEWPORT)
    const Vector2SI32 origin(m_service->textInputOrigin());
    return NSMakeRect(origin.x, origin.y, 0, 0);
#else
    const NSRect rect([m_nativeWindow contentRectForFrameRect:m_nativeWindow.frame]);
    const Vector2SI32 origin(m_service->textInputOrigin());
    CGFloat x = rect.origin.x + origin.x, y = rect.origin.y + rect.size.height - origin.y;
    return NSMakeRect(x, y, 0, 0);
#endif /* IMGUI_HAS_VIEWPORT */
}

bool
MainWindow::isRunning() const noexcept
{
    return m_runningWindow;
}

bool
MainWindow::hasMarkedText() const noexcept
{
    return m_markedString.length > 0;
}

void
MainWindow::handleAssignFocus()
{
    if (m_disabledCursorResigned) {
        Vector2SI32 centerPoint;
        internalDisableCursor(centerPoint);
        m_disabledCursorResigned = false;
        m_disabledCursorState = kDisabledCursorStateInitial;
    }
}

void
MainWindow::handleResignFocus()
{
    if (m_disabledCursorState != kDisabledCursorStateNone) {
        internalEnableCursor(m_restoreHiddenLogicalCursorPosition);
        m_disabledCursorState = kDisabledCursorStateNone;
        m_disabledCursorResigned = true;
    }
}

void
MainWindow::handleWindowResize()
{
    resizeDrawableSize();
    const NSSize size = m_nativeWindow.contentView.bounds.size;
    m_client->sendResizeWindowMessage(Vector2UI32(size.width, size.height));
}

void
MainWindow::handleWindowChangeDevicePixelRatio()
{
    resizeDrawableSize();
    m_client->sendChangeDevicePixelRatioMessage(m_nativeWindow.backingScaleFactor);
}

void
MainWindow::handleMouseDown(const NSEvent *event)
{
    Vector2SI32 position, delta;
    if (getLogicalCursorPosition(event, position, delta)) {
        const int modifiers = CocoaThreadedApplicationService::convertToCursorModifiers(event),
                  button = [event buttonNumber];
        m_client->sendScreenCursorPressMessage(deviceScaleScreenPosition(event), button, modifiers);
        if (event.window == m_nativeWindow) {
            m_client->sendCursorPressMessage(position, button, modifiers);
        }
        setLastLogicalCursorPosition(position);
    }
}

void
MainWindow::handleMouseMoved(const NSEvent *event)
{
    Vector2SI32 position, delta;
    if (getLogicalCursorPosition(event, position, delta)) {
        const int modifiers = CocoaThreadedApplicationService::convertToCursorModifiers(event),
                  button = [event buttonNumber];
        m_client->sendScreenCursorMoveMessage(deviceScaleScreenPosition(event), button, modifiers);
        if (event.window == m_nativeWindow) {
            m_client->sendCursorMoveMessage(position, delta, button, modifiers);
        }
        setLastLogicalCursorPosition(position, delta);
    }
}

void
MainWindow::handleMouseDragged(const NSEvent *event)
{
    handleMouseMoved(event);
}

void
MainWindow::handleMouseUp(const NSEvent *event)
{
    Vector2SI32 position, delta;
    if (getLogicalCursorPosition(event, position, delta)) {
        const int modifiers = CocoaThreadedApplicationService::convertToCursorModifiers(event),
                  button = [event buttonNumber];
        m_client->sendScreenCursorReleaseMessage(deviceScaleScreenPosition(event), button, modifiers);
        if (event.window == m_nativeWindow) {
            m_client->sendCursorReleaseMessage(position, button, modifiers);
        }
        setLastLogicalCursorPosition(Vector2());
    }
}

void
MainWindow::handleMouseExit(const NSEvent *event)
{
    handleMouseUp(event);
}

void
MainWindow::handleRightMouseDown(const NSEvent *event)
{
    handleMouseDown(event);
}

void
MainWindow::handleRightMouseDragged(const NSEvent *event)
{
    handleMouseDragged(event);
}

void
MainWindow::handleRightMouseUp(const NSEvent *event)
{
    handleMouseUp(event);
}

void
MainWindow::handleOtherMouseDown(const NSEvent *event)
{
    handleMouseDown(event);
}

void
MainWindow::handleOtherMouseDragged(const NSEvent *event)
{
    handleMouseDragged(event);
}

void
MainWindow::handleOtherMouseUp(const NSEvent *event)
{
    handleMouseUp(event);
}

void
MainWindow::handleKeyDown(NSEvent *event)
{
#if defined(NANOEM_ENABLE_DEBUG_LABEL)
    BaseApplicationService::KeyType type = translateKeyType(event);
    if (type == BaseApplicationService::kKeyType_F12) {
        m_client->sendStartDebugCaptureMessage();
    }
    else
#endif
        if (![m_textInputContext handleEvent:event]) {
        sendUnicodeStringInput(event.charactersIgnoringModifiers);
    }
}

void
MainWindow::handleKeyUp(NSEvent *event)
{
    m_client->sendKeyReleaseMessage(translateKeyType(event));
}

void
MainWindow::handleScrollWheel(const NSEvent *event)
{
    Vector2SI32 position, delta;
    if (getLogicalCursorPosition(event, position, delta)) {
        const Vector2 scrollDelta(CocoaThreadedApplicationService::scrollWheelDelta(event));
        int modifiers = CocoaThreadedApplicationService::convertToCursorModifiers(event);
        m_client->sendCursorScrollMessage(position, scrollDelta, modifiers);
    }
}

void
MainWindow::handleInsertText(id value)
{
    NSString *characters =
        [value isKindOfClass:[NSAttributedString class]] ? ((NSAttributedString *) value).string : (NSString *) value;
    sendUnicodeStringInput(characters);
}

void
MainWindow::handleUnmarkText()
{
    m_markedString.mutableString.string = @"";
}

void
MainWindow::handleSetMarkerText(id value, NSRange selectedRange)
{
    NSMutableAttributedString *markerText = [value isKindOfClass:[NSAttributedString class]]
        ? [[NSMutableAttributedString alloc] initWithAttributedString:value]
        : [[NSMutableAttributedString alloc] initWithString:value];
    m_markedString = markerText;
    m_selectedRange = selectedRange;
}

void
MainWindow::openProgressWindow(NSString *title, NSString *message, nanoem_u32_t total, bool cancellable)
{
    if (m_progressWindow.second > 0) {
        m_progressWindow.second++;
    }
    else if (!m_progressWindow.first) {
        NSAlert *alert = [[NSAlert alloc] init];
        alert.messageText = title;
        alert.informativeText = message;
        alert.alertStyle = NSAlertStyleInformational;
        NSButton *button = [alert addButtonWithTitle:@"Cancel"];
        button.enabled = cancellable ? YES : NO;
        NSUInteger width = m_nativeWindow.frame.size.width * 0.35f;
        NSProgressIndicator *indicator = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(0, 0, width, 20)];
        indicator.style = NSProgressIndicatorBarStyle;
        if (total > 0) {
            [indicator stopAnimation:nil];
            indicator.indeterminate = NO;
            indicator.maxValue = total;
        }
        else {
            indicator.indeterminate = YES;
        }
        [indicator startAnimation:nil];
        alert.accessoryView = indicator;
        m_menu.startModalDialog();
        m_progressWindow = tinystl::make_pair(alert, 1);
        [alert beginSheetModalForWindow:m_nativeWindow
                      completionHandler:^(NSModalResponse response) {
                          if (response == NSAlertFirstButtonReturn) {
                              Progress::requestCancel();
                          }
                      }];
    }
}

void
MainWindow::closeProgressWindow()
{
    m_progressWindow.second--;
    if (m_progressWindow.second <= 0) {
        if (NSAlert *alert = m_progressWindow.first) {
            NSWindow *window = alert.window;
            [m_nativeWindow endSheet:window];
            [m_nativeWindow makeKeyWindow];
            m_menu.stopModalDialog();
        }
        m_progressWindow = tinystl::make_pair(static_cast<NSAlert *>(nil), 0);
    }
}

void
MainWindow::confirmBeforeClose()
{
    m_client->sendIsProjectDirtyRequestMessage(
        [](void *userData, bool dirty) {
            auto self = static_cast<MainWindow *>(userData);
            ThreadedApplicationClient *client = self->client();
            if (dirty) {
                client->clearAllProjectAfterConfirmOnceEventListeners();
                client->addSaveProjectAfterConfirmEventListener(
                    [](void *userData) {
                        auto self = static_cast<MainWindow *>(userData);
                        self->m_client->addCompleteSavingFileEventListener(
                            [](void *userData, const URI & /* fileURI */, uint32_t /* type */, uint64_t /* ticks */) {
                                auto self = static_cast<MainWindow *>(userData);
                                self->sendDestroyMessage();
                            },
                            self, true);
                        self->m_menu.saveProject();
                    },
                    self, true);
                client->addDiscardProjectAfterConfirmEventListener(
                    [](void *userData) {
                        auto self = static_cast<MainWindow *>(userData);
                        self->sendDestroyMessage();
                    },
                    self, true);
                client->sendConfirmBeforeExitApplicationMessage();
            }
            else {
                self->sendDestroyMessage();
            }
        },
        this);
}

void
MainWindow::clearTitle()
{
    m_nativeWindow.title = CocoaThreadedApplicationService::kDefaultWindowTitle;
}

void
MainWindow::setTitle(NSURL *fileURL)
{
    setTitle(fileURL.lastPathComponent.stringByStandardizingPath);
}

void
MainWindow::setTitle(NSString *lastPathComponent)
{
    NSString *title = [[NSString alloc]
        initWithFormat:@"%@ - %@", lastPathComponent, CocoaThreadedApplicationService::kDefaultWindowTitle];
    m_nativeWindow.title = title;
}

BaseApplicationService::KeyType
MainWindow::translateKeyType(const NSEvent *event) noexcept
{
    BaseApplicationService::KeyType key;
    switch (event.keyCode) {
    case 0x1D:
        key = BaseApplicationService::kKeyType_0;
        break;
    case 0x12:
        key = BaseApplicationService::kKeyType_1;
        break;
    case 0x13:
        key = BaseApplicationService::kKeyType_2;
        break;
    case 0x14:
        key = BaseApplicationService::kKeyType_3;
        break;
    case 0x15:
        key = BaseApplicationService::kKeyType_4;
        break;
    case 0x17:
        key = BaseApplicationService::kKeyType_5;
        break;
    case 0x16:
        key = BaseApplicationService::kKeyType_6;
        break;
    case 0x1A:
        key = BaseApplicationService::kKeyType_7;
        break;
    case 0x1C:
        key = BaseApplicationService::kKeyType_8;
        break;
    case 0x19:
        key = BaseApplicationService::kKeyType_9;
        break;
    case 0x00:
        key = BaseApplicationService::kKeyType_A;
        break;
    case 0x0B:
        key = BaseApplicationService::kKeyType_B;
        break;
    case 0x08:
        key = BaseApplicationService::kKeyType_C;
        break;
    case 0x02:
        key = BaseApplicationService::kKeyType_D;
        break;
    case 0x0E:
        key = BaseApplicationService::kKeyType_E;
        break;
    case 0x03:
        key = BaseApplicationService::kKeyType_F;
        break;
    case 0x05:
        key = BaseApplicationService::kKeyType_G;
        break;
    case 0x04:
        key = BaseApplicationService::kKeyType_H;
        break;
    case 0x22:
        key = BaseApplicationService::kKeyType_I;
        break;
    case 0x26:
        key = BaseApplicationService::kKeyType_J;
        break;
    case 0x28:
        key = BaseApplicationService::kKeyType_K;
        break;
    case 0x25:
        key = BaseApplicationService::kKeyType_L;
        break;
    case 0x2E:
        key = BaseApplicationService::kKeyType_M;
        break;
    case 0x2D:
        key = BaseApplicationService::kKeyType_N;
        break;
    case 0x1F:
        key = BaseApplicationService::kKeyType_O;
        break;
    case 0x23:
        key = BaseApplicationService::kKeyType_P;
        break;
    case 0x0C:
        key = BaseApplicationService::kKeyType_Q;
        break;
    case 0x0F:
        key = BaseApplicationService::kKeyType_R;
        break;
    case 0x01:
        key = BaseApplicationService::kKeyType_S;
        break;
    case 0x11:
        key = BaseApplicationService::kKeyType_T;
        break;
    case 0x20:
        key = BaseApplicationService::kKeyType_U;
        break;
    case 0x09:
        key = BaseApplicationService::kKeyType_V;
        break;
    case 0x0D:
        key = BaseApplicationService::kKeyType_W;
        break;
    case 0x07:
        key = BaseApplicationService::kKeyType_X;
        break;
    case 0x10:
        key = BaseApplicationService::kKeyType_Y;
        break;
    case 0x06:
        key = BaseApplicationService::kKeyType_Z;
        break;
    case 0x27:
        key = BaseApplicationService::kKeyType_APOSTROPHE;
        break;
    case 0x2A:
        key = BaseApplicationService::kKeyType_BACKSLASH;
        break;
    case 0x2B:
        key = BaseApplicationService::kKeyType_COMMA;
        break;
    case 0x18:
        key = BaseApplicationService::kKeyType_EQUAL;
        break;
    case 0x32:
        key = BaseApplicationService::kKeyType_GRAVE_ACCENT;
        break;
    case 0x21:
        key = BaseApplicationService::kKeyType_LEFT_BRACKET;
        break;
    case 0x1B:
        key = BaseApplicationService::kKeyType_MINUS;
        break;
    case 0x2F:
        key = BaseApplicationService::kKeyType_PERIOD;
        break;
    case 0x1E:
        key = BaseApplicationService::kKeyType_RIGHT_BRACKET;
        break;
    case 0x29:
        key = BaseApplicationService::kKeyType_SEMICOLON;
        break;
    case 0x2C:
        key = BaseApplicationService::kKeyType_SLASH;
        break;
    case 0x0A:
        key = BaseApplicationService::kKeyType_WORLD_1;
        break;
    case 0x33:
        key = BaseApplicationService::kKeyType_BACKSPACE;
        break;
    case 0x39:
        key = BaseApplicationService::kKeyType_CAPS_LOCK;
        break;
    case 0x75:
        key = BaseApplicationService::kKeyType_DELETE;
        break;
    case 0x7D:
        key = BaseApplicationService::kKeyType_DOWN;
        break;
    case 0x77:
        key = BaseApplicationService::kKeyType_END;
        break;
    case 0x24:
        key = BaseApplicationService::kKeyType_ENTER;
        break;
    case 0x35:
        key = BaseApplicationService::kKeyType_ESCAPE;
        break;
    case 0x7A:
        key = BaseApplicationService::kKeyType_F1;
        break;
    case 0x78:
        key = BaseApplicationService::kKeyType_F2;
        break;
    case 0x63:
        key = BaseApplicationService::kKeyType_F3;
        break;
    case 0x76:
        key = BaseApplicationService::kKeyType_F4;
        break;
    case 0x60:
        key = BaseApplicationService::kKeyType_F5;
        break;
    case 0x61:
        key = BaseApplicationService::kKeyType_F6;
        break;
    case 0x62:
        key = BaseApplicationService::kKeyType_F7;
        break;
    case 0x64:
        key = BaseApplicationService::kKeyType_F8;
        break;
    case 0x65:
        key = BaseApplicationService::kKeyType_F9;
        break;
    case 0x6D:
        key = BaseApplicationService::kKeyType_F10;
        break;
    case 0x67:
        key = BaseApplicationService::kKeyType_F11;
        break;
    case 0x6F:
        key = BaseApplicationService::kKeyType_F12;
        break;
    case 0x69:
        key = BaseApplicationService::kKeyType_F13;
        break;
    case 0x6B:
        key = BaseApplicationService::kKeyType_F14;
        break;
    case 0x71:
        key = BaseApplicationService::kKeyType_F15;
        break;
    case 0x6A:
        key = BaseApplicationService::kKeyType_F16;
        break;
    case 0x40:
        key = BaseApplicationService::kKeyType_F17;
        break;
    case 0x4F:
        key = BaseApplicationService::kKeyType_F18;
        break;
    case 0x50:
        key = BaseApplicationService::kKeyType_F19;
        break;
    case 0x5A:
        key = BaseApplicationService::kKeyType_F20;
        break;
    case 0x73:
        key = BaseApplicationService::kKeyType_HOME;
        break;
    case 0x72:
        key = BaseApplicationService::kKeyType_INSERT;
        break;
    case 0x7B:
        key = BaseApplicationService::kKeyType_LEFT;
        break;
    case 0x3A:
        key = BaseApplicationService::kKeyType_LEFT_ALT;
        break;
    case 0x3B:
        key = BaseApplicationService::kKeyType_LEFT_CONTROL;
        break;
    case 0x38:
        key = BaseApplicationService::kKeyType_LEFT_SHIFT;
        break;
    case 0x37:
        key = BaseApplicationService::kKeyType_LEFT_SUPER;
        break;
    case 0x6E:
        key = BaseApplicationService::kKeyType_MENU;
        break;
    case 0x47:
        key = BaseApplicationService::kKeyType_NUM_LOCK;
        break;
    case 0x79:
        key = BaseApplicationService::kKeyType_PAGE_DOWN;
        break;
    case 0x74:
        key = BaseApplicationService::kKeyType_PAGE_UP;
        break;
    case 0x7C:
        key = BaseApplicationService::kKeyType_RIGHT;
        break;
    case 0x3D:
        key = BaseApplicationService::kKeyType_RIGHT_ALT;
        break;
    case 0x3E:
        key = BaseApplicationService::kKeyType_RIGHT_CONTROL;
        break;
    case 0x3C:
        key = BaseApplicationService::kKeyType_RIGHT_SHIFT;
        break;
    case 0x36:
        key = BaseApplicationService::kKeyType_RIGHT_SUPER;
        break;
    case 0x31:
        key = BaseApplicationService::kKeyType_SPACE;
        break;
    case 0x30:
        key = BaseApplicationService::kKeyType_TAB;
        break;
    case 0x7E:
        key = BaseApplicationService::kKeyType_UP;
        break;
    case 0x52:
        key = BaseApplicationService::kKeyType_KP_0;
        break;
    case 0x53:
        key = BaseApplicationService::kKeyType_KP_1;
        break;
    case 0x54:
        key = BaseApplicationService::kKeyType_KP_2;
        break;
    case 0x55:
        key = BaseApplicationService::kKeyType_KP_3;
        break;
    case 0x56:
        key = BaseApplicationService::kKeyType_KP_4;
        break;
    case 0x57:
        key = BaseApplicationService::kKeyType_KP_5;
        break;
    case 0x58:
        key = BaseApplicationService::kKeyType_KP_6;
        break;
    case 0x59:
        key = BaseApplicationService::kKeyType_KP_7;
        break;
    case 0x5B:
        key = BaseApplicationService::kKeyType_KP_8;
        break;
    case 0x5C:
        key = BaseApplicationService::kKeyType_KP_9;
        break;
    case 0x45:
        key = BaseApplicationService::kKeyType_KP_ADD;
        break;
    case 0x41:
        key = BaseApplicationService::kKeyType_KP_DECIMAL;
        break;
    case 0x4B:
        key = BaseApplicationService::kKeyType_KP_DIVIDE;
        break;
    case 0x4C:
        key = BaseApplicationService::kKeyType_KP_ENTER;
        break;
    case 0x51:
        key = BaseApplicationService::kKeyType_KP_EQUAL;
        break;
    case 0x43:
        key = BaseApplicationService::kKeyType_KP_MULTIPLY;
        break;
    case 0x4E:
        key = BaseApplicationService::kKeyType_KP_SUBTRACT;
        break;
    default:
        key = BaseApplicationService::kKeyType_UNKNOWN;
        break;
    }
    return key;
}

void
MainWindow::handleAddingWatchEffectSource(void *userData, nanoem_u16_t handle, const URI &fileURI)
{
    auto self = static_cast<MainWindow *>(userData);
    String effectSourcePath(fileURI.absolutePathByDeletingPathExtension());
    effectSourcePath.append(".fx");
    if (FileUtils::exists(effectSourcePath.c_str())) {
        self->m_watchEffectSourceHandles.insert(tinystl::make_pair(effectSourcePath, handle));
        self->resetWatchEffectSource();
    }
}

void
MainWindow::handleRemovingWatchEffectSource(void *userData, uint16_t handle, const char * /* name */)
{
    auto self = static_cast<MainWindow *>(userData);
    for (auto it = self->m_watchEffectSourceHandles.begin(), end = self->m_watchEffectSourceHandles.end(); it != end;
         ++it) {
        if (it->second == handle) {
            self->m_watchEffectSourceHandles.erase(it);
            break;
        }
    }
    self->resetWatchEffectSource();
}

Vector2SI32
MainWindow::deviceScaleScreenPosition(const NSEvent *event) noexcept
{
    return CocoaThreadedApplicationService::deviceScaleScreenPosition(event, m_nativeWindow);
}

float
MainWindow::invertedDevicePixelRatio() const noexcept
{
    return 1.0f / m_nativeWindow.backingScaleFactor;
}

void
MainWindow::initializeMetal(id<MTLDevice> device, sg_pixel_format &pixelFormat)
{
    MainWindowMetalView *view = [[MainWindowMetalView alloc] initWithFrame:m_nativeWindow.contentView.bounds
                                                                    device:device
                                                                mainWindow:this];
    [view registerForDraggedTypes:@[ NSFilenamesPboardType ]];
    auto setColorSpaceDisplayP3 = [&view]() {
        CGColorSpaceRef cs = CGColorSpaceCreateWithName(kCGColorSpaceDisplayP3);
        if (@available(macOS 10.12, *)) {
            view.colorspace = cs;
        }
        else {
            CAMetalLayer *layer = (CAMetalLayer *) view.layer;
            layer.colorspace = cs;
        }
        CGColorSpaceRelease(cs);
    };
    switch (m_preference->applicationPreference()->defaultColorPixelFormat()) {
    case SG_PIXELFORMAT_RGB10A2: {
        if (@available(macOS 10.13, *)) {
            view.colorPixelFormat = MTLPixelFormatBGR10A2Unorm;
            pixelFormat = SG_PIXELFORMAT_RGB10A2;
            setColorSpaceDisplayP3();
        }
        else {
            view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
            pixelFormat = SG_PIXELFORMAT_RGBA8;
        }
        break;
    }
    case SG_PIXELFORMAT_RGBA16F: {
        view.colorPixelFormat = MTLPixelFormatRGBA16Float;
        pixelFormat = SG_PIXELFORMAT_RGBA16F;
        setColorSpaceDisplayP3();
        break;
    }
    default:
        view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
        pixelFormat = SG_PIXELFORMAT_BGRA8;
        break;
    }
    if (@available(macOS 10.13, *)) {
        view.currentDrawable.layer.displaySyncEnabled = isEditingDisplaySyncEnabled();
    }
    m_nativeWindow.contentView = view;
    m_service->setNativeDevice((__bridge void *) device);
    m_service->setNativeView((__bridge void *) view);
    JSON_Object *pending = json_object(m_service->applicationPendingChangeConfiguration());
    json_object_dotset_string(pending, "macos.renderer.name", device.name.UTF8String);
}

void
MainWindow::initializeOpenGL()
{
    const NSOpenGLPixelFormatAttribute attributes[] = { NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        NSOpenGLPFAColorSize, 24, NSOpenGLPFAAlphaSize, 8, NSOpenGLPFADepthSize, 24, NSOpenGLPFAStencilSize, 8,
        NSOpenGLPFADoubleBuffer, 1, NSOpenGLPFAClosestPolicy, 1, 0, 0 };
    NSOpenGLPixelFormat *format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
    MainWindowOpenGLView *view = [[MainWindowOpenGLView alloc] initWithFrame:m_nativeWindow.contentView.bounds
                                                                 pixelFormat:format
                                                                  mainWindow:this];
    [view registerForDraggedTypes:@[ NSFilenamesPboardType ]];
    view.wantsBestResolutionOpenGLSurface = YES;
    m_nativeWindow.contentView = view;
    NSOpenGLContext *context = view.openGLContext;
    [context makeCurrentContext];
    int interval = 1;
    [context setValues:&interval forParameter:NSOpenGLCPSwapInterval];
    m_nativeWindow.contentView = view;
    m_service->setNativeContext((__bridge void *) context);
    m_service->setNativeView((__bridge void *) view);
    JSON_Object *pending = json_object(m_service->applicationPendingChangeConfiguration());
    json_object_dotset_string(pending, "macos.renderer.name", reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    [NSOpenGLContext clearCurrentContext];
}

void
MainWindow::getWindowCenterPoint(Vector2SI32 *value)
{
    const NSRect &rect = m_nativeWindow.contentView.frame;
    value->x = rect.size.width * 0.5f;
    value->y = rect.size.height * 0.5f;
}

bool
MainWindow::getLogicalCursorPosition(const NSEvent *event, Vector2SI32 &position, Vector2SI32 &delta)
{
    bool result = true;
    /* prevent generating warped cursor delta values */
    if (m_disabledCursorState == kDisabledCursorStateInitial) {
        position = m_virtualLogicalCursorPosition;
        delta = Vector2SI32(0);
        m_disabledCursorState = kDisabledCursorStateMoving;
    }
    else if (m_disabledCursorState == kDisabledCursorStateMoving) {
        position = m_virtualLogicalCursorPosition;
        delta = Vector2SI32(event.deltaX, event.deltaY);
    }
    else if ([m_nativeWindow isKeyWindow]) {
        position = logicalCursorLocationInWindow(event);
        delta = position - m_lastLogicalCursorPosition;
    }
    else {
        result = false;
    }
    return result;
}

void
MainWindow::recenterCursorPosition()
{
    if (m_disabledCursorState != kDisabledCursorStateNone) {
        Vector2SI32 value;
        getWindowCenterPoint(&value);
        if (m_lastLogicalCursorPosition != value) {
            m_lastLogicalCursorPosition = value;
        }
    }
}

Vector2
MainWindow::logicalCursorLocationInWindow(const NSEvent *event) const
{
    const NSRect &rect = [m_nativeWindow contentRectForFrameRect:m_nativeWindow.frame];
    const NSPoint &location = event.locationInWindow;
    const NSPoint &newLocation = [m_nativeWindow.contentView convertPoint:NSMakePoint(location.x, location.y)
                                                                 fromView:nil];
    return Vector2(newLocation.x, rect.size.height - newLocation.y);
}

Vector2
MainWindow::lastLogicalCursorPosition() const
{
    return m_lastLogicalCursorPosition;
}

void
MainWindow::setLastLogicalCursorPosition(const Vector2SI32 &value)
{
    m_lastLogicalCursorPosition = value;
}

void
MainWindow::setLastLogicalCursorPosition(const Vector2SI32 &value, const Vector2SI32 &delta)
{
    m_virtualLogicalCursorPosition += delta;
    m_lastLogicalCursorPosition = value;
}

void
MainWindow::disableCursor(const Vector2SI32 &logicalCursorPosition)
{
    Vector2SI32 centerPoint;
    internalDisableCursor(centerPoint);
    m_virtualLogicalCursorPosition = logicalCursorPosition;
    m_lastLogicalCursorPosition = centerPoint;
    m_restoreHiddenLogicalCursorPosition = logicalCursorPosition;
    m_disabledCursorState = kDisabledCursorStateInitial;
}

void
MainWindow::enableCursor(const Vector2SI32 &logicalCursorPosition)
{
    if (logicalCursorPosition.x != 0 && logicalCursorPosition.y != 0) {
        internalEnableCursor(logicalCursorPosition);
    }
    else {
        internalEnableCursor(m_restoreHiddenLogicalCursorPosition);
    }
    m_restoreHiddenLogicalCursorPosition = Vector2SI32();
    m_disabledCursorState = kDisabledCursorStateNone;
}

void
MainWindow::internalDisableCursor(Vector2SI32 &centerLocation)
{
    [NSCursor hide];
    getWindowCenterPoint(&centerLocation);
    CGAssociateMouseAndMouseCursorPosition(false);
}

void
MainWindow::internalEnableCursor(const Vector2SI32 &logicalCursorPocation)
{
    const CGDirectDisplayID displayID = CGMainDisplayID();
    const CGFloat viewHeight = m_nativeWindow.contentView.frame.size.height,
                  displayHeight = CGDisplayBounds(displayID).size.height;
    const NSRect &rect = [m_nativeWindow
        convertRectToScreen:NSMakeRect(logicalCursorPocation.x, viewHeight - logicalCursorPocation.y, 0, 0)];
    CGAssociateMouseAndMouseCursorPosition(true);
    CGDisplayMoveCursorToPoint(displayID, CGPointMake(rect.origin.x, displayHeight - rect.origin.y));
    [NSCursor unhide];
}

void
MainWindow::registerAllPrerequisiteEventListeners()
{
    m_client->addInitializationCompleteEventListener(
        [](void *userData) {
            auto self = static_cast<MainWindow *>(userData);
            dispatch_async(self->m_metricsQueue, ^() {
                self->sendPerformanceMonitorPeriodically();
            });
            NSWindow *window = self->m_nativeWindow;
            [window center];
            [window makeKeyAndOrderFront:window];
            char hardwareModelName[128];
            size_t len = sizeof(hardwareModelName);
            sysctlbyname("hw.model", hardwareModelName, &len, NULL, 0);
            const JSON_Object *config = json_object(self->m_service->applicationConfiguration());
            NSURL *libSentryDllURL =
                [[NSBundle mainBundle].builtInPlugInsURL URLByAppendingPathComponent:@"libsentry.dylib"];
            const char *sentryDSN = NANOEM_SENTRY_DSN;
            JSON_Object *pending = json_object(self->m_service->applicationPendingChangeConfiguration());
            const ApplicationPreference *pref = self->m_preference->applicationPreference();
            if (pref->isCrashReportEnabled()) {
                auto maskString = [](const char *value) {
                    NSString *maskedFilePath = [[[[[NSString alloc] initWithUTF8String:value]
                        stringByReplacingOccurrencesOfString:NSHomeDirectory()
                                                  withString:@"{HOME}"]
                        stringByReplacingOccurrencesOfString:NSFullUserName()
                                                  withString:@"{USER}"]
                        stringByReplacingOccurrencesOfString:NSUserName()
                                                  withString:@"{USER}"];
                    return sentry_value_new_string(maskedFilePath.UTF8String);
                };
                auto transportEnvelope = [](sentry_envelope_t *envelope, void * /* state */) {
                    if (NSString *dsnString = [[NSString alloc] initWithUTF8String:NANOEM_SENTRY_DSN]) {
                        if (NSURL *dsn = [[NSURL alloc] initWithString:dsnString]) {
                            int64_t projectID = 0;
                            sscanf(dsn.path.UTF8String, "/%lld", &projectID);
                            NSString *urlString =
                                [[NSString alloc] initWithFormat:@"https://%@/api/%lld/envelope/", dsn.host, projectID];
                            if (NSURL *url = [[NSURL alloc] initWithString:urlString]) {
                                NSMutableURLRequest *request = [[NSMutableURLRequest alloc] initWithURL:url];
                                NSString *clientName = [[NSString alloc] initWithUTF8String:"sentry-native/0.4.4"];
                                request.HTTPMethod = @"POST";
                                [request setValue:@"application/x-sentry-envelope" forHTTPHeaderField:@"Content-Type"];
                                [request setValue:clientName forHTTPHeaderField:@"User-Agent"];
                                NSString *authToken = [[NSString alloc]
                                    initWithFormat:@"Sentry sentry_key=%@, sentry_version=7, sentry_client=%@",
                                    dsn.user, clientName];
                                [request setValue:authToken forHTTPHeaderField:@"X-Sentry-Auth"];
                                size_t envelopeSize = 0;
                                char *envelopeData = sentry_envelope_serialize(envelope, &envelopeSize);
                                request.HTTPBody = [[[NSString alloc] initWithBytes:envelopeData
                                                                             length:envelopeSize
                                                                           encoding:NSUTF8StringEncoding]
                                    dataUsingEncoding:NSUTF8StringEncoding];
                                sentry_free(envelopeData);
                                sentry_envelope_free(envelope);
                                __block dispatch_semaphore_t sema = dispatch_semaphore_create(0);
                                NSURLSession *session = [NSURLSession sharedSession];
                                NSURLSessionTask *task = [session
                                    dataTaskWithRequest:request
                                      completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
                                          BX_UNUSED_3(data, response, error);
                                          dispatch_semaphore_signal(sema);
                                      }];
                                [task resume];
                                dispatch_semaphore_wait(sema, dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC * 30));
                            }
                        }
                    }
                };
                BaseApplicationService::SentryDescription desc;
                NSString *clientUUID = self->m_preference->clientUUID();
                desc.m_clientUUID = clientUUID.UTF8String;
                desc.m_databaseDirectoryPath = json_object_dotget_string(config, "macos.sentry.database.path");
                desc.m_deviceModelName = hardwareModelName;
                desc.m_dllFilePath = libSentryDllURL.path.UTF8String;
                desc.m_dsn = sentryDSN;
                desc.m_handlerFilePath = json_object_dotget_string(config, "macos.sentry.handler.path");
                desc.m_isModelEditingEnabled = pref->isModelEditingEnabled();
                desc.m_localeName = json_object_dotget_string(config, "project.locale");
                desc.m_maskString = maskString;
                desc.m_rendererName = json_object_dotget_string(pending, "macos.renderer.name");
                desc.m_transportSendEnvelope = transportEnvelope;
                desc.m_transportUserData = nullptr;
                self->m_sentryDllHandle = BaseApplicationService::openSentryDll(desc);
                if (self->m_sentryDllHandle) {
                    NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
                    formatter.dateFormat = @"yyyy-MM-dd'T'HH:mm:ssZZZZZ";
                    formatter.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
                    NSString *date = [formatter stringFromDate:[NSDate date]];
                    sentry_set_extra("initialized", sentry_value_new_string(date.UTF8String));
                }
            }
            json_object_dotremove(pending, "macos.renderer.name");
#if defined(NANOEM_TRIAL_EXPIRATION_TIME) && NANOEM_TRIAL_EXPIRATION_TIME > 0
            if (CFAbsoluteTimeGetCurrent() >= CFTimeInterval(NANOEM_TRIAL_EXPIRATION_TIME)) {
                ModalDialogFactory::StandardConfirmDialogCallbackPair pair;
                pair.first = [](void *userData, Project *) -> IModalDialog * {
                    auto self = static_cast<MainWindow *>(userData);
                    self->m_tracker.send("main", "nanoem.macos.expiration", nullptr, nullptr);
                    [NSApp terminate:nil];
                    return 0;
                };
                pair.second = [](void *userData, Project *) -> IModalDialog * {
                    auto self = static_cast<MainWindow *>(userData);
                    self->m_tracker.send("main", "nanoem.macos.expiration", nullptr, nullptr);
                    [NSApp terminate:nil];
                    return 0;
                };
                self->m_service->addModalDialog(ModalDialogFactory::createStandardConfirmDialog(
                    self->m_translator->translate("nanoem.macos.expiration.title"),
                    self->m_translator->translate("nanoem.macos.expiration.description"), pair, self));
            }
#endif /* NANOEM_TRIAL_EXPIRATION_TIME */
            dispatch_async(self->m_lazyExecutionQueue, ^() {
                URIList plugins;
                CocoaApplicationMenuBuilder::aggregateAllPlugins(plugins);
                self->m_client->sendLoadAllModelPluginsMessage(plugins);
                self->m_client->sendLoadAllMotionPluginsMessage(plugins);
            });
#ifdef NANOEM_ENABLE_DEBUG_LABEL
            const bx::CommandLine *cmd = self->m_commandLine;
            if (cmd->hasArg("bootstrap-project-from-clipboard")) {
                URIList plugins;
                CocoaApplicationMenuBuilder::aggregateAllPlugins(plugins);
                NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
                NSArray *classes = @[ [NSString class] ];
                NSArray *fileURLs = [pasteboard readObjectsForClasses:classes options:nil];
                NSCharacterSet *characterSet = [NSCharacterSet whitespaceAndNewlineCharacterSet];
                self->m_client->sendLoadAllDecoderPluginsMessage(plugins);
                for (NSString *item in fileURLs) {
                    NSString *trimmedItem = [item stringByTrimmingCharactersInSet:characterSet];
                    NSURL *fileURL = [[NSURL alloc] initFileURLWithPath:trimmedItem];
                    const URI fileURI(CocoaThreadedApplicationService::canonicalFileURI(fileURL));
                    self->m_client->sendLoadFileMessage(fileURI, IFileManager::kDialogTypeOpenProject);
                    NSString *path = [[NSString alloc] initWithUTF8String:fileURI.absolutePathConstString()];
                    self->setTitle([[NSURL alloc] initFileURLWithPath:path]);
                }
            }
            else if (cmd->hasArg("bootstrap-project")) {
                URIList plugins;
                CocoaApplicationMenuBuilder::aggregateAllPlugins(plugins);
                NSString *filePath = [[NSString alloc] initWithUTF8String:cmd->findOption("bootstrap-project")];
                NSURL *fileURL = [[NSURL alloc] initFileURLWithPath:filePath];
                const URI fileURI(CocoaThreadedApplicationService::canonicalFileURI(fileURL));
                self->m_client->sendLoadAllDecoderPluginsMessage(plugins);
                if (FileUtils::exists(fileURI)) {
                    self->m_client->sendLoadFileMessage(fileURI, IFileManager::kDialogTypeOpenProject);
                    NSString *path = [[NSString alloc] initWithUTF8String:fileURI.absolutePathConstString()];
                    self->setTitle([[NSURL alloc] initFileURLWithPath:path]);
                }
            }
            else if (cmd->hasArg("recovery-from-latest")) {
                const char *redoPath = json_object_dotget_string(
                    json_object(self->m_service->applicationConfiguration()), "macos.redo.path");
                NSURL *redoDirectoryURL =
                    [[NSURL alloc] initFileURLWithPath:[[NSString alloc] initWithUTF8String:redoPath]];
                NSFileManager *fileManager = [NSFileManager defaultManager];
                NSArray *contents = [[fileManager contentsOfDirectoryAtURL:redoDirectoryURL
                                                includingPropertiesForKeys:@[]
                                                                   options:NSDirectoryEnumerationSkipsHiddenFiles
                                                                     error:nil]
                    sortedArrayUsingComparator:^(NSURL *left, NSURL *right) {
                        NSDate *lvalue = [[fileManager attributesOfItemAtPath:left.path
                                                                        error:nil] objectForKey:NSFileModificationDate];
                        NSDate *rvalue = [[fileManager attributesOfItemAtPath:right.path
                                                                        error:nil] objectForKey:NSFileModificationDate];
                        NSComparisonResult result = NSOrderedSame;
                        if (lvalue && rvalue) {
                            result = [rvalue compare:lvalue];
                        }
                        return result;
                    }];
                if (NSURL *found = contents.firstObject) {
                    self->m_client->sendRecoveryMessage(CocoaThreadedApplicationService::canonicalFileURI(found));
                }
            }
            else if (cmd->hasArg("recovery-from")) {
                const URI fileURI(URI::createFromFilePath(cmd->findOption("recovery-from")));
                if (FileUtils::exists(fileURI)) {
                    self->m_client->sendRecoveryMessage(fileURI);
                }
            }
#endif /* NANOEM_ENABLE_DEBUG_LABEL */
            self->m_client->sendActivateMessage();
        },
        this, true);
    m_client->addErrorEventListener(
        [](void *userData, int code, const char *reason, const char *recoverySuggestion) {
            auto self = static_cast<MainWindow *>(userData);
            asl_object_t asl = static_cast<asl_object_t>(self->m_logger);
            if (!asl) {
                self->m_logger = asl_open(nullptr, BaseApplicationService::kOrganizationDomain, ASL_LEVEL_NOTICE);
            }
            asl_log(asl, nullptr, ASL_LEVEL_ERR, "code=%d, reason=\"%s\", suggestion=\"%s\"", code, reason,
                recoverySuggestion);
            if (recoverySuggestion) {
                EMLOG_ERROR(
                    "Received an error: code={} reason=\"{}\" suggestion=\"{}\"", code, reason, recoverySuggestion);
            }
            else if (reason) {
                EMLOG_ERROR("Received an error: code={} reason=\"{}\"", code, reason);
            }
        },
        this, false);
    m_client->addDisableCursorEventListener(
        [](void *userData, const Vector2SI32 &logicalCursorPosition) {
            auto self = static_cast<MainWindow *>(userData);
            self->disableCursor(logicalCursorPosition);
        },
        this, false);
    m_client->addEnableCursorEventListener(
        [](void *userData, const Vector2SI32 &coord) {
            auto self = static_cast<MainWindow *>(userData);
            self->enableCursor(coord);
        },
        this, false);
    m_client->addSetPreferredEditingFPSEvent(
        [](void *userData, uint32_t value) {
            auto self = static_cast<MainWindow *>(userData);
            self->m_preference->mutableApplicationPreference()->setPreferredEditingFPS(value);
            self->m_service->setDisplaySyncEnabled(value != UINT32_MAX);
        },
        this, false);
    m_client->addQueryOpenSingleFileDialogEventListener(
        [](void *userData, uint32_t type, const StringList &allowedExtensions) {
            auto self = static_cast<MainWindow *>(userData);
            NSOpenPanel *panel = [NSOpenPanel openPanel];
            panel.allowedFileTypes = CocoaApplicationMenuBuilder::allowedFileTypesFromStringList(allowedExtensions);
            [panel beginSheetModalForWindow:self->m_nativeWindow
                          completionHandler:^(NSInteger result) {
                              CocoaApplicationMenuBuilder::clearModalPanel(panel, self->m_nativeWindow);
                              if (result == NSModalResponseOK) {
                                  const URI fileURI(CocoaThreadedApplicationService::canonicalFileURI(panel.URL));
                                  self->m_client->sendQueryOpenSingleFileDialogMessage(type, fileURI);
                              }
                              else {
                                  self->m_client->sendQueryOpenSingleFileDialogMessage(type, URI());
                              }
                          }];
        },
        this, false);
    m_client->addQueryOpenMultipleFilesDialogEventListener(
        [](void *userData, uint32_t type, const StringList &allowedExtensions) {
            auto self = static_cast<MainWindow *>(userData);
            NSOpenPanel *panel = [NSOpenPanel openPanel];
            panel.allowedFileTypes = CocoaApplicationMenuBuilder::allowedFileTypesFromStringList(allowedExtensions);
            [panel beginSheetModalForWindow:self->m_nativeWindow
                          completionHandler:^(NSInteger result) {
                              CocoaApplicationMenuBuilder::clearModalPanel(panel, self->m_nativeWindow);
                              if (result == NSModalResponseOK) {
                                  URIList fileURIs;
                                  for (NSURL *fileURL in panel.URLs) {
                                      const URI fileURI(CocoaThreadedApplicationService::canonicalFileURI(fileURL));
                                      fileURIs.push_back(fileURI);
                                  }
                                  self->m_client->sendQueryOpenMultipleFilesDialogMessage(type, fileURIs);
                              }
                          }];
        },
        this, false);
    m_client->addQuerySaveFileDialogEventListener(
        [](void *userData, uint32_t type, const StringList &allowedExtensions) {
            auto self = static_cast<MainWindow *>(userData);
            NSSavePanel *panel = [NSSavePanel savePanel];
            panel.allowedFileTypes = CocoaApplicationMenuBuilder::allowedFileTypesFromStringList(allowedExtensions);
            [panel beginSheetModalForWindow:self->m_nativeWindow
                          completionHandler:^(NSInteger result) {
                              CocoaApplicationMenuBuilder::clearModalPanel(panel, self->m_nativeWindow);
                              if (result == NSModalResponseOK) {
                                  const URI fileURI(CocoaThreadedApplicationService::canonicalFileURI(panel.URL));
                                  self->m_client->sendQuerySaveFileDialogMessage(type, fileURI);
                              }
                              else {
                                  self->m_client->sendQuerySaveFileDialogMessage(type, URI());
                              }
                          }];
        },
        this, false);
    m_client->addUpdateProgressEventListener(
        [](void *userData, uint32_t value, uint32_t total, uint32_t type, const char *text) {
            auto self = static_cast<MainWindow *>(userData);
            self->updateProgressDialog(value, total, type, text);
        },
        this, false);
    m_client->addStartProgressEventListener(
        [](void *userData, const char *title, const char *text, uint32_t total) {
            auto self = static_cast<MainWindow *>(userData);
            NSString *titleString = [[NSString alloc] initWithUTF8String:title];
            NSString *textString = [[NSString alloc] initWithUTF8String:text];
            self->openProgressWindow(titleString, textString, total, true);
        },
        this, false);
    m_client->addStopProgressEventListener(
        [](void *userData) {
            auto self = static_cast<MainWindow *>(userData);
            self->closeProgressWindow();
        },
        this, false);
    m_client->addSetPreferredMotionFPSEvent(
        [](void *userData, nanoem_u32_t /* fps */, bool unlimited) {
            auto self = static_cast<MainWindow *>(userData);
            self->m_vsyncAtPlaying = unlimited ? false : true;
        },
        this, false);
    m_client->addPlayEvent(
        [](void *userData, nanoem_frame_index_t /* duration */, nanoem_frame_index_t /* localFrameIndex */) {
            auto self = static_cast<MainWindow *>(userData);
            self->m_service->setDisplaySyncEnabled(self->m_vsyncAtPlaying);
        },
        this, false);
    m_client->addResumeEvent(
        [](void *userData, nanoem_frame_index_t /* duration */, nanoem_frame_index_t /* localFrameIndex */) {
            auto self = static_cast<MainWindow *>(userData);
            self->m_service->setDisplaySyncEnabled(self->m_vsyncAtPlaying);
        },
        this, false);
    m_client->addPauseEvent(
        [](void *userData, nanoem_frame_index_t /* duration */, nanoem_frame_index_t /* localFrameIndex */) {
            auto self = static_cast<MainWindow *>(userData);
            self->m_service->setDisplaySyncEnabled(self->isEditingDisplaySyncEnabled());
        },
        this, false);
    m_client->addStopEvent(
        [](void *userData, nanoem_frame_index_t /* duration */, nanoem_frame_index_t /* localFrameIndex */) {
            auto self = static_cast<MainWindow *>(userData);
            self->m_service->setDisplaySyncEnabled(self->isEditingDisplaySyncEnabled());
        },
        this, false);
    m_client->addSetupProjectEventListener(
        [](void *userData, const Vector2 & /* windowSize */, nanoem_f32_t /* windowDevicePixelRatio */,
            nanoem_f32_t /* viewportDevicePixelRatio */) {
            auto self = static_cast<MainWindow *>(userData);
            self->destroyWatchEffectSource();
        },
        this, false);
    m_client->addAddAccessoryEventListener(
        [](void *userData, uint16_t handle, const char * /* name */) {
            auto self = static_cast<MainWindow *>(userData);
            self->m_client->sendGetHandleFileURIRequestMessage(handle, handleAddingWatchEffectSource, self);
        },
        this, false);
    m_client->addAddModelEventListener(
        [](void *userData, uint16_t handle, const char * /* name */) {
            auto self = static_cast<MainWindow *>(userData);
            self->m_client->sendGetHandleFileURIRequestMessage(handle, handleAddingWatchEffectSource, self);
        },
        this, false);
    m_client->addRemoveAccessoryEventListener(handleRemovingWatchEffectSource, this, false);
    m_client->addRemoveModelEventListener(handleRemovingWatchEffectSource, this, false);
}

bool
MainWindow::isEditingDisplaySyncEnabled() const noexcept
{
    return m_preference->applicationPreference()->preferredEditingFPS() != -1;
}

void
MainWindow::updateDisplayFrequency(CGDirectDisplayID displayId)
{
    CGDisplayModeRef mode = CGDisplayCopyDisplayMode(displayId);
    m_displayFrequency = uint32_t(CGDisplayModeGetRefreshRate(mode));
    if (!m_displayFrequency) {
        CVDisplayLinkRef displayLink;
        if (CVDisplayLinkCreateWithActiveCGDisplays(&displayLink) == kCVReturnSuccess) {
            const CVTime time = CVDisplayLinkGetNominalOutputVideoRefreshPeriod(displayLink);
            m_displayFrequency = uint32_t(time.timeScale / double(time.timeValue));
            CVDisplayLinkRelease(displayLink);
        }
        else {
            m_displayFrequency = 60;
        }
    }
    CGDisplayModeRelease(mode);
}

void
MainWindow::updatePreferredFPS()
{
#if 0
    CFTypeRef blob = IOPSCopyPowerSourcesInfo();
    CFStringRef source = IOPSGetProvidingPowerSourceType(blob);
    const bool isACPower = CFStringCompare(source, CFSTR(kIOPMACPowerKey), 0) == kCFCompareEqualTo;
    CFRelease(blob);
#endif
}

void
MainWindow::updateProgressDialog(uint32_t value, uint32_t total, uint32_t type, const char *text)
{
    if (NSAlert *alert = m_progressWindow.first) {
        NSProgressIndicator *indicator = (NSProgressIndicator *) alert.accessoryView;
        [indicator stopAnimation:nil];
        indicator.indeterminate = NO;
        indicator.doubleValue = value;
        indicator.maxValue = total;
        if (type == Progress::kEventTypeText) {
            alert.informativeText = text ? [[NSString alloc] initWithUTF8String:text] : @"";
        }
        else if (type == Progress::kEventTypeItem) {
            alert.informativeText = text ? [[NSString alloc] initWithFormat:@"Loading Next Item... %@",
                                                             [[NSString alloc] initWithUTF8String:text]]
                                         : @"";
        }
        CocoaThreadedApplicationService::sendDummyEvent([NSApplication sharedApplication], true);
    }
}

void
MainWindow::sendUnicodeStringInput(NSString *characters)
{
    CocoaThreadedApplicationService::sendUnicodeTextInput(characters, m_client);
}

void
MainWindow::sendPerformanceMonitorPeriodically()
{
    while (m_runningMetrics) {
        mach_port_t port = mach_task_self();
        nanoem_f32_t cpu = CocoaThreadedApplicationService::currentCPUPercentage(port);
        nanoem_u64_t memory = CocoaThreadedApplicationService::currentMemorySize(port);
        m_client->sendUpdatePerformanceMonitorMessage(cpu, memory, 0);
        bx::sleep(1000);
    }
    dispatch_semaphore_signal(m_metricsSemaphore);
}

void
MainWindow::sendDestroyMessage()
{
    NSString *title =
        [[NSString alloc] initWithUTF8String:m_translator->translate("nanoem.dialog.progress.exit.title")];
    NSString *message =
        [[NSString alloc] initWithUTF8String:m_translator->translate("nanoem.dialog.progress.exit.message")];
    m_quitAt = stm_now();
    openProgressWindow(title, message, 0, false);
    m_runningMetrics = false;
    dispatch_semaphore_wait(m_metricsSemaphore, DISPATCH_TIME_FOREVER);
    m_metricsQueue = nullptr;
    m_client->addCompleteDestructionEventListener(
        [](void *userData) {
            auto self = static_cast<MainWindow *>(userData);
            self->m_client->addCompleteTerminationEventListener(
                [](void *userData) {
                    auto self = static_cast<MainWindow *>(userData);
                    if (auto handle = self->m_sentryDllHandle) {
                        BaseApplicationService::closeSentryDll(handle);
                        self->m_sentryDllHandle = nullptr;
                    }
                    self->closeProgressWindow();
                    [self->m_nativeWindow close];
                    self->m_runningWindow = false;
                },
                self, true);
            self->m_client->sendTerminateMessage();
        },
        this, true);
    m_client->sendDestroyMessage();
}

void
MainWindow::resetWatchEffectSource()
{
    destroyWatchEffectSource();
    if (!m_watchEffectSourceHandles.empty()) {
        NSMutableArray *paths = [[NSMutableArray alloc] initWithCapacity:m_watchEffectSourceHandles.size()];
        for (const auto &it : m_watchEffectSourceHandles) {
            NSString *filePath = [[NSString alloc] initWithUTF8String:it.first.c_str()];
            [paths addObject:filePath];
        }
        FSEventStreamContext context = {};
        context.info = this;
        m_watchEffectSourceStream = FSEventStreamCreate(
            nullptr,
            [](ConstFSEventStreamRef /* stream */, void *context, size_t numEvents, void *eventPaths,
                const FSEventStreamEventFlags * /* eventFlags */, const FSEventStreamEventId * /* eventIds */) {
                auto self = static_cast<MainWindow *>(context);
                auto paths = static_cast<const char *const *>(eventPaths);
                for (size_t i = 0; i < numEvents; i++) {
                    const char *path = paths[i];
                    auto it = self->m_watchEffectSourceHandles.find(path);
                    if (it != self->m_watchEffectSourceHandles.end()) {
                        uint16_t handle = it->second;
                        self->m_client->sendReloadAccessoryEffectMessage(handle);
                        self->m_client->sendReloadModelEffectMessage(handle);
                    }
                }
            },
            &context, (__bridge CFArrayRef) paths, kFSEventStreamEventIdSinceNow, 1.0,
            kFSEventStreamCreateFlagFileEvents);
        FSEventStreamScheduleWithRunLoop(m_watchEffectSourceStream, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
        FSEventStreamStart(m_watchEffectSourceStream);
    }
}

void
MainWindow::destroyWatchEffectSource()
{
    if (m_watchEffectSourceStream) {
        FSEventStreamStop(m_watchEffectSourceStream);
        FSEventStreamUnscheduleFromRunLoop(m_watchEffectSourceStream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        FSEventStreamInvalidate(m_watchEffectSourceStream);
        FSEventStreamRelease(m_watchEffectSourceStream);
        m_watchEffectSourceStream = nullptr;
    }
}

void
MainWindow::resizeDrawableSize()
{
    if ([m_nativeWindow.contentView isMemberOfClass:[MainWindowMetalView class]]) {
        const NSSize size = m_nativeWindow.contentView.bounds.size;
        float devicePixelRatio = m_nativeWindow.backingScaleFactor;
        MTKView *view = (MTKView *) m_nativeWindow.contentView;
        view.drawableSize = CGSizeMake(size.width * devicePixelRatio, size.height * devicePixelRatio);
    }
}

} /* namespace macos */
} /* namespace nanoem */
