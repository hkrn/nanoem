/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import <IOKit/pwr_mgt/IOPMLib.h>
#import <QuartzCore/CVDisplayLink.h>

#import "CocoaApplicationMenuBuilder.h"
#import "MenuItemCollection.h"

#include "emapp/BaseApplicationService.h"
#include "emapp/IFileManager.h"

#include "bx/os.h" /* bx::sleep */
#include <atomic>

@class NSApplication;
@class NSCursor;
@class NSEvent;
@class NSOpenPanel;
@class NSSavePanel;
@class NSWindow;
@class NSTextInputContext;
@class WebView;

namespace bx {
class CommandLine;
}
namespace nanoem {

class ITranslator;
class ThreadedApplicationClient;
class ThreadedApplicationService;

namespace macos {

class CocoaThreadedApplicationService;
class Preference;

class MainWindow : private NonCopyable {
public:
    MainWindow(const bx::CommandLine *cmd, CocoaThreadedApplicationService *service, ThreadedApplicationClient *client,
        NSWindow *nativeWindow, Preference *preference);
    ~MainWindow();

    void initialize();
    void processMessage(NSApplication *app);
    void terminate();

    const ThreadedApplicationClient *client() const noexcept;
    ThreadedApplicationClient *client() noexcept;
    const ThreadedApplicationService *service() const noexcept;
    ThreadedApplicationService *service() noexcept;
    NSWindow *nativeWindow() noexcept;
    NSAttributedString *attributedString();
    NSRange selectedRange() const noexcept;
    NSRange markedRange() const noexcept;
    NSRect firstRectForCharacterRange();
    bool isRunning() const noexcept;
    bool hasMarkedText() const noexcept;

    void handleAssignFocus();
    void handleResignFocus();
    void handleWindowResize();
    void handleWindowChangeDevicePixelRatio();
    void handleMouseDown(const NSEvent *event);
    void handleMouseMoved(const NSEvent *event);
    void handleMouseDragged(const NSEvent *event);
    void handleMouseUp(const NSEvent *event);
    void handleMouseExit(const NSEvent *event);
    void handleRightMouseDown(const NSEvent *event);
    void handleRightMouseDragged(const NSEvent *event);
    void handleRightMouseUp(const NSEvent *event);
    void handleOtherMouseDown(const NSEvent *event);
    void handleOtherMouseDragged(const NSEvent *event);
    void handleOtherMouseUp(const NSEvent *event);
    void handleKeyDown(NSEvent *event);
    void handleKeyUp(NSEvent *event);
    void handleScrollWheel(const NSEvent *event);
    void handleInsertText(id value);
    void handleUnmarkText();
    void handleSetMarkerText(id value, NSRange selectedRange);

    void openProgressWindow(NSString *title, NSString *message, nanoem_u32_t total, bool cancellable);
    void closeProgressWindow();
    void confirmBeforeClose();
    void clearTitle();
    void setWindowDevicePixelRatio(float value);
    void setTitle(NSURL *fileURL);
    void setTitle(NSString *lastPathComponent);

private:
    enum DisabledCursorState {
        kDisabledCursorStateNone,
        kDisabledCursorStateInitial,
        kDisabledCursorStateMoving,
    };
    struct FPSThresholder {
        FPSThresholder(uint32_t value, bool enabled);
        inline void
        operator=(bool value)
        {
            m_enabled = value;
        }
        inline void
        operator=(uint32_t value)
        {
            m_value = value;
        }
        void threshold(uint32_t displayFrequency);
        uint32_t m_value;
        uint32_t m_preferred;
        bool m_enabled;
    };
    typedef tinystl::unordered_map<String, nanoem_u16_t, TinySTLAllocator> FileHandleMap;

    static BaseApplicationService::KeyType translateKeyType(const NSEvent *event) noexcept;
    static void handleAddingWatchEffectSource(void *userData, nanoem_u16_t handle, const URI &fileURI);
    static void handleRemovingWatchEffectSource(void *userData, uint16_t handle, const char *name);

    Vector2SI32 deviceScaleScreenPosition(const NSEvent *event) noexcept;
    float invertedDevicePixelRatio() const noexcept;

    void initializeMetal(id<MTLDevice> device, sg_pixel_format &pixelFormat);
    void initializeOpenGL();
    void getWindowCenterPoint(Vector2SI32 *value);
    bool getLogicalCursorPosition(const NSEvent *event, Vector2SI32 &position, Vector2SI32 &delta);
    void recenterCursorPosition();
    Vector2 logicalCursorLocationInWindow(const NSEvent *event) const;
    Vector2 lastLogicalCursorPosition() const;
    void setLastLogicalCursorPosition(const Vector2SI32 &value);
    void setLastLogicalCursorPosition(const Vector2SI32 &value, const Vector2SI32 &delta);
    void disableCursor(const Vector2SI32 &logicalCursorPosition);
    void enableCursor(const Vector2SI32 &logicalCursorPosition);
    void internalDisableCursor(Vector2SI32 &centerLocation);
    void internalEnableCursor(const Vector2SI32 &logicalCursorPocation);
    void registerAllPrerequisiteEventListeners();
    bool isEditingDisplaySyncEnabled() const noexcept;
    void updateDisplayFrequency(CGDirectDisplayID displayId);
    void updatePreferredFPS();
    void updateProgressDialog(uint32_t value, uint32_t total, uint32_t type, const char *text);
    void sendUnicodeStringInput(NSString *characters);
    void sendPerformanceMonitorPeriodically();
    void sendDestroyMessage();
    void resetWatchEffectSource();
    void destroyWatchEffectSource();
    void resizeDrawableSize();

    const ITranslator *m_translator;
    const bx::CommandLine *m_commandLine;
    macos::CocoaThreadedApplicationService *m_service = nullptr;
    ThreadedApplicationClient *m_client = nullptr;
    NSWindow *m_nativeWindow = nil;
    NSTextInputContext *m_textInputContext = nil;
    tinystl::pair<NSAlert *, int> m_progressWindow = tinystl::make_pair(static_cast<NSAlert *>(nil), 0);
    NSMutableAttributedString *m_markedString = nil;
    NSRange m_selectedRange = NSMakeRange(NSNotFound, 0);
    IOPMAssertionID m_displayKey = 0;
    IOPMAssertionID m_idleKey = 0;
    Preference *m_preference = nullptr;
    FSEventStreamRef m_watchEffectSourceStream = nullptr;
    dispatch_queue_t m_lazyExecutionQueue = nullptr;
    dispatch_queue_t m_metricsQueue = nullptr;
    dispatch_semaphore_t m_metricsSemaphore = nullptr;
    void *m_logger = nullptr;
    void *m_sentryDllHandle = nullptr;
    CocoaApplicationMenuBuilder m_menu;
    FileHandleMap m_watchEffectSourceHandles;
    Vector2SI32 m_lastLogicalCursorPosition = Vector2SI32(0);
    Vector2SI32 m_virtualLogicalCursorPosition = Vector2SI32(0);
    Vector2SI32 m_restoreHiddenLogicalCursorPosition = Vector2SI32(0);
    DisabledCursorState m_disabledCursorState = kDisabledCursorStateNone;
    uint64_t m_quitAt = 0;
    uint32_t m_displayFrequency = 0;
    std::atomic<bool> m_runningMetrics;
    bool m_runningWindow = true;
    bool m_vsyncAtPlaying = true;
    bool m_disabledCursorResigned = false;
};

} /* namespace macos */
} /* namespace nanoem */
