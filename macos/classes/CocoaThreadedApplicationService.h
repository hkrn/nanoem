/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_EMAPP_MACOS_MACOSTHREADEDAPPLICATIONSERVICE_H_
#define NANOEM_EMAPP_MACOS_MACOSTHREADEDAPPLICATIONSERVICE_H_

#import <CoreVideo/CVDisplayLink.h>
#import <CoreVideo/CVPixelBuffer.h>
#import <Foundation/Foundation.h>

#include "emapp/ThreadedApplicationService.h"

#include "imgui/imgui.h"

#include <atomic>

struct nanoem_application_document_t;

@class MTKView;
@class NSApplication;
@class NSEvent;
@class NSOpenGLContext;
@class NSWindow;
@class NSURL;
@class AVPlayer;
@class AVPlayerItem;
@class AVPlayerItemVideoOutput;
@protocol CAMetalDrawable;
@protocol MTLTexture;
@protocol MTLCommandQueue;

namespace nanoem {

class Allocator;
class ThreadedApplicationClient;

namespace macos {

class IBackgroundRenderer;

class CocoaThreadedApplicationService final : public ThreadedApplicationService {
public:
    static NSString *kDefaultWindowTitle;

    static NSApplication *createApplication();
    static NSWindow *createMainWindow();
    static NSString *maskUserString(const char *value);
    static URI canonicalFileURI(NSURL *value);
    static nanoem_f32_t currentCPUPercentage(mach_port_t port);
    static nanoem_u64_t currentMemorySize(mach_port_t port);
    static Vector2 scrollWheelDelta(const NSEvent *event);
    static Vector2SI32 deviceScaleScreenPosition(const NSEvent *event, NSWindow *window) noexcept;
    static const char *sharedRedoFilePath() noexcept;
    static int convertToCursorModifiers(const NSEvent *event) noexcept;
    static void sendDummyEvent(NSApplication *app, bool atStart);
    static void sendUnicodeTextInput(const NSString *characters, ThreadedApplicationClient *client);
    static void setupAllocatorContext(CFAllocatorContext &context, Allocator *allocator);

    CocoaThreadedApplicationService(const JSON_Value *config);
    ~CocoaThreadedApplicationService() override;

    void setDisplaySyncEnabled(bool value);

    NSOpenGLContext *OpenGLContext();
    id<MTLDevice> metalDevice();
    id<MTLCommandQueue> metalCommandQueue();
    id<CAMetalDrawable> metalCurrentDrawable();
    MTKView *nativeView();
    Vector2SI32 textInputOrigin() const noexcept;

private:
    using ImageMap = tinystl::unordered_map<nanoem_u32_t, sg_image, TinySTLAllocator>;
    using ImageDescriptionMap = tinystl::unordered_map<nanoem_u32_t, sg_image_desc, TinySTLAllocator>;
    using PassMap = tinystl::unordered_map<nanoem_u32_t, sg_pass, TinySTLAllocator>;
    static void handleApplicationCrash();
    static CVReturn handleDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *inNow,
        const CVTimeStamp *inOutputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *displayLinkContext);

    IAudioPlayer *createAudioPlayer() override;
    IBackgroundVideoRenderer *createBackgroundVideoRenderer() override;
    IDebugCapture *createDebugCapture() override;
    Project::IRendererCapability *createRendererCapability() override;
    Project::ISkinDeformerFactory *createSkinDeformerFactory() override;
    IVideoRecorder *createVideoRecorder() override;
    BaseApplicationClient *menubarApplicationClient() override;
    bool hasVideoRecorder() const noexcept override;
    bool isRendererAvailable(const char *value) const noexcept override;

    void executeRunUnit(int &exitCode) override;
    void handleInitializeApplicationThread() override;
    void handleDestructApplicationThread() override;
    void handleSetupGraphicsEngine(sg_desc &desc) override;
    void handleInitializeApplication() override;
    void handleNewProject() override;
    void handleSuspendProject() override;
    void handleResumeProject() override;
    void handleDestructApplication() override;
    void handleTerminateApplication() override;

    sg_pixel_format defaultPassPixelFormat() const noexcept override;
    void beginDefaultPass(
        nanoem_u32_t windowID, const sg_pass_action &pa, int width, int height, int &sampleCount) override;
    void postEmptyApplicationEvent() override;
    void presentDefaultPass(const Project *project) override;
    bool loadFromFile(const URI &fileURI, Project *project, IFileManager::DialogType type, Error &error) override;
    bool saveAsFile(const URI &fileURI, Project *project, IFileManager::DialogType type, Error &error) override;
    URI recoverableRedoFileURI() const override;
    void deleteCrashRecoveryFile() override;

    sg_pixel_format resolveMetalPixelFormat() const noexcept;
    bool isTrackpadOnly() const;
    void setupNewProject();
    void setupRedoFilePath(Project *project);
    void initializeInputDevices();
    void updateAllMonitors();

    NSMutableSet *m_devices = [[NSMutableSet alloc] init];
    NSString *m_clipboard = nil;
    id<MTLCommandQueue> m_commandQueue = nil;
    id<MTLTexture> m_texture = nil;
    CFAllocatorRef m_allocator = nullptr;
    CVDisplayLinkRef m_displayLink = nullptr;
    dispatch_semaphore_t m_semaphore = nullptr;
    ThreadedApplicationClient *m_menuClient = nullptr;
    nanoem_application_document_t *m_document = nullptr;
    Vector2SI32 m_textInputOrigin = Vector2SI32(0);
    void *m_hidManager = nullptr;
    ImageDescriptionMap m_colorImageDescriptions;
    ImageMap m_colorImages;
    ImageMap m_depthImages;
    PassMap m_passes;
    std::atomic<bool> m_displaySyncChanged;
    bool m_displaySyncEnabled = true;
};

} /* namespace macos */
} /* namespace nanoem */

#endif /*  */
