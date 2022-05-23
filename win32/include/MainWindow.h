/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_WIN32_MAINWINDOW_H_
#define NANOEM_EMAPP_WIN32_MAINWINDOW_H_

#include <Windows.h>
#include <dxgi.h>
#include <shellapi.h>

#include "Win32ApplicationMenuBuilder.h"
#include "emapp/BaseApplicationService.h"
#include "emapp/IFileManager.h"

#include "bx/os.h"

struct IProgressDialog;
struct ID3D12CommandQueue;
struct ID3D12Device;

namespace bx {
class CommandLine;
} /* namespace bx */

namespace nanoem {

class ThreadedApplicationClient;

namespace win32 {

class Dialog;
class Preference;
class Win32ThreadedApplicationService;

class MainWindow final {
public:
    MainWindow(const bx::CommandLine *cmd, const Preference *preference,
        win32::Win32ThreadedApplicationService *service, ThreadedApplicationClient *client, HINSTANCE hInstance,
        const Vector4UI32 &rect, nanoem_f32_t devicePixelRatio);
    ~MainWindow();

    bool initialize(HWND windowHandle, Error &error);
    bool isRunning() const noexcept;
    void processMessage(MSG *msg);

    HWND windowHandle() noexcept;
    HMENU menuHandle() noexcept;
    void clearTitle();
    void setTitle(const URI &fileURI);

private:
    enum DisabledCursorState {
        kDisabledCursorStateNone,
        kDisabledCursorStateInitial,
        kDisabledCursorStateMoving,
    };
    struct FPSThresholder {
        FPSThresholder(uint32_t value, bool enabled)
            : m_value(value)
            , m_preferred(value)
            , m_enabled(enabled)
        {
        }
        void
        operator=(uint32_t value)
        {
            m_value = value;
        }
        void
        threshold(uint32_t displayFrequency)
        {
            if (m_enabled) {
                const uint32_t value = m_value > 0 ? m_value : m_preferred;
                if (value > 0 && value < displayFrequency) {
                    bx::sleep(uint32_t((1.0f / (value + 1)) * 1000.0f));
                }
            }
        }
        uint32_t m_value;
        uint32_t m_preferred;
        bool m_enabled;
    };
    typedef tinystl::vector<nanoem_u16_t, TinySTLAllocator> HandleList;
    typedef tinystl::unordered_map<String, tinystl::pair<HANDLE, HandleList>, TinySTLAllocator> FileHandleListMap;

    static LRESULT CALLBACK handleWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    static DWORD CALLBACK collectPerformanceMetricsPeriodically(void *userData);
    static BaseApplicationService::KeyType translateKey(LPARAM lparam) noexcept;
    static Vector2SI32 devicePixelScreenPosition(HWND hwnd, const Vector2SI32 &value) noexcept;
    static int cursorModifiers() noexcept;
    static int convertCursorType(UINT msg, WPARAM wparam) noexcept;
    static void handleAddingWatchEffectSource(void *userData, nanoem_u16_t handle, const URI &fileURI);
    static void handleRemovingWatchEffectSource(void *userData, uint16_t handle, const char *name);

    bool handleWindowCreate(HWND hwnd, Error &error);
    void handleWindowResize(HWND hwnd, WPARAM type);
    void handleWindowPositionChange(HWND hwnd);
    void handleWindowConstraint(HWND hwnd, LPMINMAXINFO info);
    void handleWindowDropFile(HWND hwnd, HDROP drop);
    void handleWindowClose(HWND hwnd);
    void handleWindowDestroy(HWND hwnd);
    void handleMouseDown(HWND hwnd, const Vector2SI32 &coord, int type);
    void handleMouseMove(HWND hwnd, const Vector2SI32 &coord, int type);
    void handleMouseUp(HWND hwnd, const Vector2SI32 &coord, int type);
    void handleMouseWheel(HWND hwnd, const Vector2SI32 &delta);
    void handleMenuItem(HWND hwnd, ApplicationMenuBuilder::MenuItemType menuType);

    nanoem_f32_t invertedDevicePixelRatio() const noexcept;
    void newProject();
    void openProject();
    void saveProject();
    void saveProjectAs();
    void loadFile(const Dialog &dialog, IFileManager::DialogType type);
    void loadFile(const URI &fileURI, IFileManager::DialogType type);
    void saveFile(const Dialog &dialog, IFileManager::DialogType type);
    void saveFile(const URI &fileURI, IFileManager::DialogType type);
    void exportImage();
    void exportVideo();

    bool setupDirectXRenderer(HWND windowHandle, int width, int height, bool &isLowPower, Error &error);
    bool setupOpenGLRenderer(HWND windowHandle, Error &error);
    void destroyRenderer();
    void destroyWindow();
    void registerAllPrerequisiteEventListeners();
    bool openProgressDialog(IProgressDialog *&dialog);
    void closeProgressDialog();

    Vector2SI32 virtualLogicalCursorPosition(const Vector2SI32 &value) const;
    Vector2SI32 lastLogicalCursorPosition() const;
    bool isCursorHidden() const;
    void setLastLogicalCursorPosition(const Vector2SI32 &value);
    void setLastLogicalCursorPosition(const Vector2SI32 &value, const Vector2SI32 &delta);
    void getWindowCenterPoint(LPPOINT devicePoint) const;
    void setCursorPosition(POINT devicePoint);
    void centerCursor(LPPOINT devicePoint);
    void updateClipCursorRect();
    void lockCursor(LPPOINT devicePoint);
    void disableCursor(const Vector2SI32 &logicalCursorPosition);
    void unlockCursor(const Vector2SI32 &logicalCursorPosition);
    void enableCursor(const Vector2SI32 &logicalCursorPosition);
    void setFocus();
    void killFocus();
    void recenterCursor();
    void updateDisplayFrequency();
    void resizeWindow();
    void resizeWindow(const Vector2UI32 &logicalWindowSize);
    void updatePreferredFPS(const POWERBROADCAST_SETTING *settings);
    void enablePowerSaving(bool value);
    bool isEditingDisplaySyncEnabled() const noexcept;

    URIList cachedAggregateAllPlugins();
    const char *localizedString(const char *text) const;
    const wchar_t *localizedWideString(const char *text) const;
    void writeErrorLog(int code, const char *reason, const char *recoverySuggestion);
    void loadProjectFromFile(const wchar_t *source);
    void destroyAllWatchEffectSources();

    using TranslatedMessageCache = tinystl::unordered_map<const char *, MutableWideString, TinySTLAllocator>;
    mutable TranslatedMessageCache m_localizedMessageCache;
    const Preference *m_preference;
    const bx::CommandLine *m_commandLine;
    Win32ThreadedApplicationService *m_service = nullptr;
    Win32ApplicationMenuBuilder *m_menuBuilder = nullptr;
    ThreadedApplicationClient *m_client = nullptr;
    HWND m_windowHandle = nullptr;
    HMENU m_menuHandle = nullptr;
    HANDLE m_metricThreadHandle = nullptr;
    HANDLE m_processHandle = nullptr;
    void *m_sentryDllHandle = nullptr;
    void *m_context = nullptr;
    void *m_device = nullptr;
    DXGI_SWAP_CHAIN_DESC m_swapChainDesc;
    IDXGISwapChain *m_swapChain = nullptr;
    ID3D12Device *m_device12 = nullptr;
    ID3D12CommandQueue *m_commandQueue = nullptr;
    tinystl::pair<IProgressDialog *, int> m_progressDialog =
        tinystl::make_pair(static_cast<IProgressDialog *>(nullptr), 0);
    HACCEL m_accelerators = nullptr;
    RECT m_lastWindowRect;
    URIList m_cachedPluginURIs;
    FileHandleListMap m_watchEffectSourceHandles;
    Vector2SI32 m_lastLogicalCursorPosition;
    Vector2SI32 m_virtualLogicalCursorPosition;
    Vector2SI32 m_restoreHiddenLogicalCursorPosition;
    DisabledCursorState m_disabledCursorState = kDisabledCursorStateNone;
    FPSThresholder m_playingThresholder;
    FPSThresholder m_editingThresholder;
    Vector2UI32 m_logicalWindowSize;
    nanoem_f32_t m_devicePixelRatio = 1.0f;
    uint32_t m_displayFrequency = 0;
    bool m_disabledCursorResigned = false;
    bool m_vsyncAtPlaying = true;
    bool m_initialized = false;
    bool m_isFullScreen = false;
    bool m_renderable = true;
    bool m_running = true;
};

} /* namespace win32 */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_WIN32_MAINWINDOW_H_ */
