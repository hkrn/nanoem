/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_WIN32_WIN32THREADEDAPPLICATIONSERVICE_H_
#define NANOEM_EMAPP_WIN32_WIN32THREADEDAPPLICATIONSERVICE_H_

#include "emapp/ThreadedApplicationService.h"

#include <atomic>

#include "imgui/imgui.h"

struct ID3D11DepthStencilView;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;

namespace nanoem {

class IVideoRecorder;

namespace win32 {

class Win32ThreadedApplicationService final : public ThreadedApplicationService {
public:
#if defined(IMGUI_HAS_VIEWPORT)
    struct ViewportData {
        enum MessageType {
            kMessageTypeCreateWindow = WM_USER,
            kMessageTypeDestroyWindow,
            kMessageTypeShowWindow,
            kMessageTypeSetWindowPos,
            kMessageTypeGetWindowPos,
            kMessageTypeSetWindowSize,
            kMessageTypeGetWindowSize,
            kMessageTypeSetWindowFocus,
            kMessageTypeGetWindowFocus,
            kMessageTypeGetWindowMinimized,
            kMessageTypeSetWindowTitle,
            kMessageTypeSetWindowAlpha,
            kMessageTypeUpdateWindow,
            kMessageTypeGetDpiScale,
            kMessageTypeOnChangedViewport,
            kMessageTypeSetIMEInputPos,
        };
        ViewportData();
        ~ViewportData();
        void destroyWindow();
        void takeWindowStyle(ImGuiViewportFlags flags);
        void wait();
        void signal();
        static LONG width(const RECT &rect) noexcept;
        static LONG height(const RECT &rect) noexcept;
        static ImVec2 size(const RECT &rect) noexcept;
        static RECT rect(const ImGuiViewport *viewport) noexcept;
        static void getWindowStyle(ImGuiViewportFlags flags, DWORD &style, DWORD &styleEx) noexcept;
        static void sendMessage(MessageType type, const void *input) noexcept;
        static void sendMessage(MessageType type, const void *input, void *output) noexcept;
        static void sendMessage(MessageType type, const void *input, const void *output) noexcept;
        HWND m_windowHandle = nullptr;
        HANDLE m_eventHandle = nullptr;
        DWORD m_style = 0;
        DWORD m_styleEx = 0;
        bool m_windowHandleOwned = false;
    };
#endif /* IMGUI_HAS_VIEWPORT */

    static const nanoem_f32_t kStandardDPIValue;
    static const wchar_t kRegisterClassName[];

    static void installUnhandledExceptionHandler();
    static void getPluginPath(const wchar_t *executablePath, wchar_t *path, DWORD size);
    static const wchar_t *sharedRedoFilePath();
    static nanoem_f32_t calculateDevicePixelRatio();
    static nanoem_f32_t calculateDevicePixelRatio(HMONITOR monitor);
    static JSON_Value *loadJSONConfig(const wchar_t *executablePath);

    Win32ThreadedApplicationService(const JSON_Value *config);
    ~Win32ThreadedApplicationService();

    int run() override;
    void setDisplaySyncEnabled(bool value);
    void requestUpdatingAllMonitors();
    void requestViewportWindowClose(HWND window);
    void requestViewportWindowMove(HWND window);
    void requestViewportWindowResize(HWND window);

private:
    IAudioPlayer *createAudioPlayer() override;
    IBackgroundVideoRenderer *createBackgroundVideoRenderer() override;
    IDebugCapture *createDebugCapture() override;
    Project::IRendererCapability *createRendererCapability() override;
    Project::ISkinDeformerFactory *createSkinDeformerFactory() override;
    IVideoRecorder *createVideoRecorder() override;
    bool hasVideoRecorder() const noexcept override;
    bool isRendererAvailable(const char *value) const noexcept override;
    void destroyVideoRecorder(IVideoRecorder *videoRecorder) override;

    void handleSetupGraphicsEngine(sg_desc &desc) override;
    void handleInitializeApplication() override;
    void handleNewProject() override;
    void handleTerminateApplication() override;
    void postEmptyApplicationEvent() override;
    void presentDefaultPass(const Project *project) override;
    URI recoverableRedoFileURI() const override;

    void createDefaultRenderTarget(const Vector2UI16 &devicePixelWindowSize) override;
    void resizeDefaultRenderTarget(const Vector2UI16 &devicePixelWindowSize, const Project *project) override;
    void destroyDefaultRenderTarget() override;

    void setupNewProject();
    void updateAllMonitors();

    ID3D11Texture2D *m_d3d11RenderTargetTexture = nullptr;
    ID3D11RenderTargetView *m_d3d11RenderTargetView = nullptr;
    ID3D11Texture2D *m_d3d11DepthStencilTexture = nullptr;
    ID3D11DepthStencilView *m_d3d11DepthStencilView = nullptr;
    std::atomic<HWND> m_requestWindowClose;
    std::atomic<HWND> m_requestWindowMove;
    std::atomic<HWND> m_requestWindowResize;
    std::atomic<UINT> m_displaySyncInterval;
    bool m_requestUpdatingAllMonitors = false;
};

} /* namespace win32 */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_WIN32_WIN32THREADEDAPPLICATIONSERVICE_H_ */
