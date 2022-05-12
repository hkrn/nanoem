/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "MainWindow.h"

#include <Pdh.h>
#include <Psapi.h>
#include <ShlObj.h>
#include <VersionHelpers.h>
#include <windowsx.h>

/* IDXGIFactory6 */
#include <dxgi1_6.h>
#if defined(SOKOL_DEBUG) && SOKOL_DEBUG
#include <dxgidebug.h>
#endif /* SOKOL_DEBUG */

#include <d3d11.h>
#if defined(NANOEM_ENABLE_D3D11ON12)
#include <d3d11on12.h>
#endif /* NANOEM_ENABLE_D3D11ON12 */

#include "COMInline.h"
#include "Dialog.h"
#include "Preference.h"

#include "Win32ThreadedApplicationService.h"
#include "bx/commandline.h"
#include "bx/handlealloc.h"
#include "emapp/emapp.h"
#include "emapp/private/CommonInclude.h"

#include "imgui/imgui.h"
#include "sokol/sokol_time.h"

namespace nanoem {
namespace win32 {
namespace {

static char s_escapeTable[256];
static String
escapeString(const char *s)
{
    String r;
    r.reserve(strlen(s));
    for (; *s; s++) {
        if (s_escapeTable[*s]) {
            bx::stringPrintf(r, "%c", s_escapeTable[*s]);
        }
        else {
            bx::stringPrintf(r, "%%%02X", *s);
        }
    }
    return r;
}

static void
createLastError(Error &error)
{
    wchar_t buffer[1024];
    MutableString msg;
    const DWORD err = GetLastError();
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, err, LANG_USER_DEFAULT, buffer, ARRAYSIZE(buffer), 0);
    StringUtils::getMultiBytesString(buffer, msg);
    error = Error(msg.data(), err, Error::kDomainTypeOS);
}

} /* namespace anonymous */

MainWindow::MainWindow(const bx::CommandLine *cmd, const Preference *preference,
    win32::Win32ThreadedApplicationService *service, ThreadedApplicationClient *client, HINSTANCE hInstance,
    const Vector4UI32 &rect, nanoem_f32_t devicePixelRatio)
    : m_preference(preference)
    , m_commandLine(cmd)
    , m_service(service)
    , m_client(client)
    , m_playingThresholder(60, false)
    , m_editingThresholder(0, true)
{
    ZeroMemory(&m_swapChainDesc, sizeof(m_swapChainDesc));
    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = &handleWindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszClassName = Win32ThreadedApplicationService::kRegisterClassName;
    windowClass.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);
    if (RegisterClassExW(&windowClass) != 0) {
        static ACCEL accelerators[] = { { FVIRTKEY | FCONTROL, 'N',
                                            ApplicationMenuBuilder::kMenuItemTypeFileNewProject },
            { FVIRTKEY | FCONTROL, 'O', ApplicationMenuBuilder::kMenuItemTypeFileOpenProject },
            { FVIRTKEY | FCONTROL, 'S', ApplicationMenuBuilder::kMenuItemTypeFileSaveProject },
            { FVIRTKEY | FCONTROL | FSHIFT, 'S', ApplicationMenuBuilder::kMenuItemTypeFileSaveAsProject },
            { FVIRTKEY | FCONTROL, 'P', ApplicationMenuBuilder::kMenuItemTypeFileExportImage },
            { FVIRTKEY | FCONTROL | FSHIFT, 'P', ApplicationMenuBuilder::kMenuItemTypeFileExportVideo },
            { FVIRTKEY | FALT, VK_F4, ApplicationMenuBuilder::kMenuItemTypeFileExit },
            { FVIRTKEY | FCONTROL, 'Z', ApplicationMenuBuilder::kMenuItemTypeEditUndo },
            { FVIRTKEY | FCONTROL, 'Y', ApplicationMenuBuilder::kMenuItemTypeEditRedo },
            { FVIRTKEY | FCONTROL, 'C', ApplicationMenuBuilder::kMenuItemTypeEditCopy },
            { FVIRTKEY | FCONTROL, 'X', ApplicationMenuBuilder::kMenuItemTypeEditCut },
            { FVIRTKEY | FCONTROL, 'V', ApplicationMenuBuilder::kMenuItemTypeEditPaste },
            { FVIRTKEY | FCONTROL, 'A', ApplicationMenuBuilder::kMenuItemTypeEditSelectAll },
            { FVIRTKEY | FCONTROL, '1', ApplicationMenuBuilder::kMenuItemTypeCameraPresetBottom },
            { FVIRTKEY | FCONTROL, '2', ApplicationMenuBuilder::kMenuItemTypeCameraPresetFront },
            { FVIRTKEY | FCONTROL, '4', ApplicationMenuBuilder::kMenuItemTypeCameraPresetLeft },
            { FVIRTKEY | FCONTROL, '5', ApplicationMenuBuilder::kMenuItemTypeCameraPresetTop },
            { FVIRTKEY | FCONTROL, '6', ApplicationMenuBuilder::kMenuItemTypeCameraPresetRight },
            { FVIRTKEY | FCONTROL, '8', ApplicationMenuBuilder::kMenuItemTypeCameraPresetBack },
            { FVIRTKEY | FCONTROL, ' ', ApplicationMenuBuilder::kMenuItemTypeProjectPlay },
            { FVIRTKEY | FCONTROL, '.', ApplicationMenuBuilder::kMenuItemTypeProjectStop } };
        m_accelerators = CreateAcceleratorTableW(accelerators, BX_COUNTOF(accelerators));
        m_devicePixelRatio = devicePixelRatio;
        RECT windowRect = { 0, 0, LONG(rect.z * devicePixelRatio), LONG(rect.w * devicePixelRatio) };
        AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, TRUE, 0);
        m_menuHandle = CreateMenu();
        const ApplicationPreference preference(m_service);
        m_menuBuilder =
            new Win32ApplicationMenuBuilder(this, client, m_service->translator(), preference.isModelEditingEnabled());
        m_menuBuilder->build();
        m_windowHandle = CreateWindowExW(WS_EX_ACCEPTFILES | WS_EX_APPWINDOW, windowClass.lpszClassName,
            windowClass.lpszClassName, WS_OVERLAPPEDWINDOW, Inline::saturateInt32(rect.x),
            Inline::saturateInt32(rect.y), windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
            nullptr, m_menuHandle, hInstance, this);
        if (m_windowHandle) {
            RegisterPowerSettingNotification(m_windowHandle, &GUID_ACDC_POWER_SOURCE, 0);
            RegisterPowerSettingNotification(m_windowHandle, &GUID_POWER_SAVING_STATUS, 0);
            RegisterPowerSettingNotification(m_windowHandle, &GUID_POWERSCHEME_PERSONALITY, 0);
        }
    }
}

MainWindow::~MainWindow()
{
    destroyAllWatchEffectSources();
    delete m_menuBuilder;
    m_menuBuilder = nullptr;
}

bool
MainWindow::initialize(HWND windowHandle, Error &error)
{
    for (size_t i = 0; i < BX_COUNTOF(s_escapeTable); i++) {
        uint8_t c = uint8_t(i);
        bool unescape = isalnum(c) || c == '~' || c == '-' || c == '.' || c == '_';
        s_escapeTable[i] = unescape ? c : 0;
    }
    updateDisplayFrequency();
    const Vector2UI16 windowSize(
        Vector2(BaseApplicationService::minimumRequiredWindowSize()) * Vector2(m_devicePixelRatio));
    String sokolPath(
        json_object_dotget_string(json_object(m_service->applicationConfiguration()), "win32.plugin.path"));
    const ApplicationPreference preference(m_service);
    sg_pixel_format pixelFormat;
    bool isLowPower, result;
#if defined(NANOEM_WIN32_HAS_OPENGL)
    if (StringUtils::equalsIgnoreCase(preference.rendererBackend(), BaseApplicationService::kRendererOpenGL)) {
        result = setupOpenGLRenderer(windowHandle, error);
        sokolPath.append("sokol_glcore33.dll");
        pixelFormat = SG_PIXELFORMAT_RGBA8;
        isLowPower = result = true;
    }
    else
#endif /* NANOEM_WIN32_HAS_OPENGL */
    {
        result = setupDirectXRenderer(windowHandle, windowSize.x, windowSize.y, isLowPower, error);
        sokolPath.append("sokol_d3d11.dll");
        pixelFormat = SG_PIXELFORMAT_BGRA8;
    }
    if (result) {
        m_logicalWindowSize = Vector2(windowSize) * invertedDevicePixelRatio();
        const ApplicationPreference preference(m_service);
        const ApplicationPreference::HighDPIViewportModeType mode = preference.highDPIViewportMode();
        ThreadedApplicationClient::InitializeMessageDescription desc(
            m_logicalWindowSize, pixelFormat, m_devicePixelRatio, sokolPath.c_str());
        desc.m_bufferPoolSize = preference.gfxBufferPoolSize();
        desc.m_imagePoolSize = preference.gfxImagePoolSize();
        desc.m_shaderPoolSize = preference.gfxShaderPoolSize();
        desc.m_passPoolSize = preference.gfxPassPoolSize();
        desc.m_pipelinePoolSize = preference.gfxPipelinePoolSize();
        desc.m_metalGlobalUniformBufferSize = preference.gfxUniformBufferSize();
        if (mode == ApplicationPreference::kHighDPIViewportModeDisabled ||
            (mode == ApplicationPreference::kHighDPIViewportModeAuto && isLowPower)) {
            desc.m_viewportDevicePixelRatio = 1.0f;
        }
        m_client->sendInitializeMessage(desc);
    }
    return result;
}

bool
MainWindow::isRunning() const noexcept
{
    return m_running;
}

void
MainWindow::processMessage(MSG *msg)
{
    recenterCursor();
    m_client->receiveAllEventMessages();
    for (const auto &item : m_watchEffectSourceHandles) {
        HANDLE ch = item.second.first;
        if (WaitForSingleObject(ch, 0) == WAIT_OBJECT_0) {
            const auto &handles = item.second.second;
            for (const auto handle : handles) {
                m_client->sendReloadAccessoryEffectMessage(handle);
                m_client->sendReloadModelEffectMessage(handle);
            }
            FindNextChangeNotification(ch);
        }
    }
    while (PeekMessageW(msg, nullptr, 0, 0, PM_REMOVE) != 0) {
        if (!TranslateAcceleratorW(m_windowHandle, m_accelerators, msg)) {
            TranslateMessage(msg);
            DispatchMessageW(msg);
        }
    }
    if (m_running) {
        WaitMessage();
    }
}

HWND
MainWindow::windowHandle() noexcept
{
    return m_windowHandle;
}

HMENU
MainWindow::menuHandle() noexcept
{
    return m_menuHandle;
}

void
MainWindow::clearTitle()
{
    SetWindowTextW(m_windowHandle, L"nanoem");
}

void
MainWindow::setTitle(const URI &fileURI)
{
    wchar_t title[256];
    MutableWideString ws;
    StringUtils::getWideCharString(fileURI.lastPathComponentConstString(), ws);
    swprintf_s(title, L"%s - nanoem", ws.data());
    SetWindowTextW(m_windowHandle, title);
}

nanoem_f32_t
MainWindow::invertedDevicePixelRatio() const noexcept
{
    return 1.0f / m_devicePixelRatio;
}

void
MainWindow::newProject()
{
    clearTitle();
    m_client->sendNewProjectMessage();
}

void
MainWindow::openProject()
{
    Dialog dialog(m_windowHandle);
    Dialog::FilterList filters;
    filters.push_back(COMDLG_FILTERSPEC { L"All Avaiable Project Format (*.nmm, *.nma, *.pmm)", L"*.nmm;*.nma;*.pmm" });
    filters.push_back(COMDLG_FILTERSPEC { L"nanoem Project File (*.nmm)", L"*.nmm" });
    filters.push_back(COMDLG_FILTERSPEC { L"nanoem Project Archive (*.nma)", L"*.nma" });
    filters.push_back(COMDLG_FILTERSPEC { L"MikuMikuDance Project File (*.pmm)", L"*.pmm" });
    if (dialog.open(filters)) {
        loadProjectFromFile(dialog.filename());
    }
}

void
MainWindow::saveProject()
{
    m_client->sendGetProjectFileURIRequestMessage(
        [](void *userData, const URI &fileURI) {
            auto self = static_cast<MainWindow *>(userData);
            const String &pathExtension = fileURI.pathExtension();
            if (!fileURI.isEmpty() &&
                (pathExtension == String("nma") || pathExtension == String("nmm") || pathExtension == String("pmm"))) {
                self->saveFile(fileURI, IFileManager::kDialogTypeSaveProjectFile);
            }
            else {
                self->saveProjectAs();
            }
        },
        this);
}

void
MainWindow::saveProjectAs()
{
    Dialog dialog(m_windowHandle);
    Dialog::FilterList filters;
    filters.push_back(COMDLG_FILTERSPEC { L"nanoem Project File (*.nmm)", L"*.nmm" });
    filters.push_back(COMDLG_FILTERSPEC { L"nanoem Project Archive (*.nma)", L"*.nma" });
    filters.push_back(COMDLG_FILTERSPEC { L"MikuMikuDance Project File (*.pmm)", L"*.pmm" });
    if (dialog.save(filters, localizedString("nanoem.dialog.filename.untitled"))) {
        setTitle(dialog.fileURI());
        saveFile(dialog, IFileManager::kDialogTypeSaveProjectFile);
    }
    else {
        m_client->clearAllCompleteSavingFileOnceEventListeners();
    }
}

void
MainWindow::loadFile(const Dialog &dialog, IFileManager::DialogType type)
{
    loadFile(dialog.fileURI(), type);
}

void
MainWindow::loadFile(const URI &fileURI, IFileManager::DialogType type)
{
    IProgressDialog *dialog;
    if (openProgressDialog(dialog)) {
        dialog->SetTitle(localizedWideString("nanoem.dialog.progress.load.title"));
        dialog->SetLine(1, localizedWideString("nanoem.dialog.progress.load.message"), FALSE, nullptr);
        MutableWideString filePath;
        StringUtils::getWideCharString(fileURI.absolutePathConstString(), filePath);
        dialog->SetLine(2, filePath.data(), TRUE, nullptr);
        m_client->addCompleteLoadingFileEventListener(
            [](void *userData, const URI & /* fileURI */, uint32_t /* type */, uint64_t /* ticks */) {
                auto self = static_cast<MainWindow *>(userData);
                self->closeProgressDialog();
            },
            this, true);
    }
    m_client->sendLoadFileMessage(fileURI, type);
}

void
MainWindow::saveFile(const Dialog &dialog, IFileManager::DialogType type)
{
    saveFile(dialog.fileURI(), type);
}

void
MainWindow::saveFile(const URI &fileURI, IFileManager::DialogType type)
{
    IProgressDialog *dialog;
    if (openProgressDialog(dialog)) {
        dialog->SetTitle(localizedWideString("nanoem.dialog.progress.save.title"));
        dialog->SetLine(1, localizedWideString("nanoem.dialog.progress.save.message"), FALSE, nullptr);
        MutableWideString filePath;
        StringUtils::getWideCharString(fileURI.absolutePathConstString(), filePath);
        dialog->SetLine(2, filePath.data(), TRUE, nullptr);
        m_client->addCompleteSavingFileEventListener(
            [](void *userData, const URI & /* fileURI */, uint32_t /* type */, uint64_t /* ticks */) {
                auto self = static_cast<MainWindow *>(userData);
                self->closeProgressDialog();
            },
            this, true);
    }
    m_client->sendSaveFileMessage(fileURI, type);
}

void
MainWindow::exportImage()
{
    m_client->addCompleteExportImageConfigurationEventListener(
        [](void *userData, const StringList &availableExtensions) {
            if (!availableExtensions.empty()) {
                auto self = static_cast<MainWindow *>(userData);
                Dialog dialog(self->m_windowHandle);
                Dialog::FilterList filters;
                if (dialog.save("Exportable Image Format (*.%s)", availableExtensions,
                        self->localizedString("nanoem.dialog.filename.untitled"))) {
                    self->m_client->sendExecuteExportingImageMessage(dialog.fileURI());
                }
            }
        },
        this, true);
    m_client->sendRequestExportImageConfigurationMessage();
}

void
MainWindow::exportVideo()
{
    m_client->addAvailableAllExportingVideoExtensionsEvent(
        [](void *userData, const StringList & /* extensions */) {
            auto self = static_cast<MainWindow *>(userData);
            self->m_client->addCompleteExportVideoConfigurationEventListener(
                [](void *userData, const StringList &availableExtensions) {
                    if (!availableExtensions.empty()) {
                        auto self = static_cast<MainWindow *>(userData);
                        Dialog dialog(self->m_windowHandle);
                        if (dialog.save("Exportable Video Format (*.%s)", availableExtensions,
                                self->localizedString("nanoem.dialog.filename.untitled"))) {
                            self->m_client->sendExecuteExportingVideoMessage(dialog.fileURI());
                        }
                    }
                },
                self, true);
            self->m_client->sendRequestExportVideoConfigurationMessage();
        },
        this, true);
    m_client->sendLoadAllEncoderPluginsMessage(cachedAggregateAllPlugins());
}

LRESULT CALLBACK
MainWindow::handleWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = S_OK;
    switch (msg) {
    case WM_CREATE: {
        LPCREATESTRUCTW lpcs = reinterpret_cast<LPCREATESTRUCTW>(lparam);
        if (auto self = static_cast<MainWindow *>(lpcs->lpCreateParams)) {
            Error error;
            if (!self->handleWindowCreate(hwnd, error)) {
                char buffer[256];
                StringUtils::format(buffer, sizeof(buffer),
                    "Failed to initialize nanoem due to failure of setup DirectX/OpenGL: %s",
                    error.reasonConstString());
                MessageBoxA(hwnd, buffer, "nanoem", MB_ICONERROR);
                DestroyAcceleratorTable(self->m_accelerators);
                DestroyMenu(self->m_menuHandle);
                self->m_running = false;
                self->m_client->sendTerminateMessage();
                result = -1;
            }
        }
        break;
    }
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN: {
        if (auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
            const Vector2 coord(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            self->handleMouseDown(hwnd, coord, convertCursorType(msg, wparam));
        }
        break;
    }
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_XBUTTONUP: {
        if (auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
            const Vector2 coord(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            self->handleMouseUp(hwnd, coord, convertCursorType(msg, wparam));
        }
        break;
    }
    case WM_MOUSEMOVE: {
        if (auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
            const Vector2SI32 coord(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            self->handleMouseMove(hwnd, coord, convertCursorType(msg, wparam));
        }
        break;
    }
    case WM_MOUSEWHEEL: {
        if (auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
            const Vector2SI32 delta(0, int16_t(HIWORD(wparam)) / WHEEL_DELTA);
            self->handleMouseWheel(hwnd, delta);
        }
        break;
    }
    case WM_KEYDOWN: {
        if (auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
            self->m_client->sendKeyPressMessage(static_cast<nanoem_u32_t>(translateKey(lparam)));
        }
        break;
    }
    case WM_KEYUP: {
        if (auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
            self->m_client->sendKeyReleaseMessage(static_cast<nanoem_u32_t>(translateKey(lparam)));
        }
        break;
    }
    case WM_CHAR:
    case WM_UNICHAR: {
        auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (self && wparam >= 32) {
            self->m_client->sendUnicodeInputMessage(static_cast<nanoem_u32_t>(wparam));
        }
        break;
    }
    case WM_ACTIVATE: {
        auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (self) {
        }
        break;
    }
    case WM_SIZE: {
        if (auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
            self->handleWindowResize(hwnd, wparam);
        }
        break;
    }
    case WM_MOVE: {
        if (auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
            self->m_service->requestViewportWindowMove(hwnd);
        }
        break;
    }
    case WM_SIZING: {
        auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (self && self->m_initialized && self->m_renderable) {
            self->resizeWindow();
        }
        break;
    }
    case WM_WINDOWPOSCHANGED: {
        if (auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
            self->handleWindowPositionChange(hwnd);
        }
        break;
    }
    case WM_GETMINMAXINFO: {
        if (auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
            LPMINMAXINFO info = reinterpret_cast<LPMINMAXINFO>(lparam);
            self->handleWindowConstraint(hwnd, info);
        }
        break;
    }
    case WM_DROPFILES: {
        if (auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
            HDROP drop = reinterpret_cast<HDROP>(wparam);
            self->handleWindowDropFile(hwnd, drop);
        }
        break;
    }
    case WM_DPICHANGED: {
        if (auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
            auto rect = reinterpret_cast<const RECT *>(lparam);
            const LONG width = rect->right - rect->left, height = rect->bottom - rect->top;
            const nanoem_f32_t devicePixelRatio = LOWORD(wparam) / Win32ThreadedApplicationService::kStandardDPIValue,
                               invertDevicePixelRatio = 1.0f / devicePixelRatio;
            const Vector2UI32 newSize(width * invertDevicePixelRatio, height * invertDevicePixelRatio);
            self->m_devicePixelRatio = devicePixelRatio;
            self->m_client->sendChangeDevicePixelRatioMessage(devicePixelRatio);
            self->m_client->sendResizeWindowMessage(newSize);
            self->m_logicalWindowSize = newSize;
            SetWindowPos(hwnd, NULL, rect->left, rect->top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;
    }
    case WM_DISPLAYCHANGE: {
        if (auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
            self->updateDisplayFrequency();
            self->m_service->requestUpdatingAllMonitors();
        }
        break;
    }
    case WM_SETFOCUS: {
        auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (self && self->m_initialized && !self->m_renderable) {
            self->setFocus();
            self->m_renderable = true;
        }
        break;
    }
    case WM_KILLFOCUS: {
        auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (self && self->m_initialized && self->m_renderable) {
            self->killFocus();
            self->m_renderable = false;
        }
        break;
    }
    case WM_POWERBROADCAST: {
        if (auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
            if (const POWERBROADCAST_SETTING *settings = reinterpret_cast<POWERBROADCAST_SETTING *>(lparam)) {
                self->updatePreferredFPS(settings);
            }
        }
        break;
    }
    case WM_COMMAND: {
        if (auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
            const uint32_t menuID = LOWORD(wparam);
            ApplicationMenuBuilder::MenuItemType menuType = static_cast<ApplicationMenuBuilder::MenuItemType>(menuID);
            self->handleMenuItem(hwnd, menuType);
        }
        return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
#if defined(IMGUI_HAS_VIEWPORT)
    case Win32ThreadedApplicationService::ViewportData::kMessageTypeCreateWindow: {
        auto viewport = reinterpret_cast<const ImGuiViewport *>(wparam);
        if (auto userData = static_cast<Win32ThreadedApplicationService::ViewportData *>(viewport->PlatformUserData)) {
            HWND parentWindow = nullptr;
            if (viewport->ParentViewportId != 0) {
                if (ImGuiViewport *parent = ImGui::FindViewportByID(viewport->ParentViewportId)) {
                    parentWindow = static_cast<HWND>(parent->PlatformHandle);
                }
            }
            RECT rect(Win32ThreadedApplicationService::ViewportData::rect(viewport));
            userData->takeWindowStyle(viewport->Flags);
            AdjustWindowRectEx(&rect, userData->m_style, FALSE, userData->m_styleEx);
            UINT width = Win32ThreadedApplicationService::ViewportData::width(rect),
                 height = Win32ThreadedApplicationService::ViewportData::height(rect);
            if (HWND windowHandle = CreateWindowExW(userData->m_styleEx,
                    Win32ThreadedApplicationService::kRegisterClassName, L"Untitled", userData->m_style, rect.left,
                    rect.top, width, height, parentWindow, nullptr, GetModuleHandleW(nullptr), nullptr)) {
                auto self = static_cast<MainWindow *>(ImGui::GetIO().UserData);
                SetWindowLongPtrW(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
                userData->m_windowHandle = windowHandle;
                userData->m_windowHandleOwned = true;
            }
            userData->signal();
        }
        break;
    }
    case Win32ThreadedApplicationService::ViewportData::kMessageTypeDestroyWindow: {
        auto viewport = reinterpret_cast<const ImGuiViewport *>(wparam);
        if (auto userData = static_cast<Win32ThreadedApplicationService::ViewportData *>(viewport->PlatformUserData)) {
            userData->destroyWindow();
            userData->signal();
        }
        break;
    }
    case Win32ThreadedApplicationService::ViewportData::kMessageTypeShowWindow: {
        auto viewport = reinterpret_cast<const ImGuiViewport *>(wparam);
        if (auto userData = static_cast<Win32ThreadedApplicationService::ViewportData *>(viewport->PlatformUserData)) {
            DWORD flags =
                EnumUtils::isEnabledT<ImGuiViewportFlags>(viewport->Flags, ImGuiViewportFlags_NoFocusOnAppearing)
                ? SW_SHOWNA
                : SW_SHOW;
            ShowWindow(userData->m_windowHandle, flags);
            userData->signal();
        }
        break;
    }
    case Win32ThreadedApplicationService::ViewportData::kMessageTypeSetWindowPos: {
        auto viewport = reinterpret_cast<const ImGuiViewport *>(wparam);
        if (auto userData = static_cast<Win32ThreadedApplicationService::ViewportData *>(viewport->PlatformUserData)) {
            auto pos = reinterpret_cast<const ImVec2 *>(lparam);
            RECT rect = { pos->x, pos->y, pos->x, pos->y };
            AdjustWindowRectEx(&rect, userData->m_style, FALSE, userData->m_styleEx);
            SetWindowPos(userData->m_windowHandle, nullptr, rect.left, rect.top, 0, 0,
                SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            userData->signal();
        }
        break;
    }
    case Win32ThreadedApplicationService::ViewportData::kMessageTypeGetWindowPos: {
        auto viewport = reinterpret_cast<const ImGuiViewport *>(wparam);
        if (auto userData = static_cast<Win32ThreadedApplicationService::ViewportData *>(viewport->PlatformUserData)) {
            auto pos = reinterpret_cast<ImVec2 *>(lparam);
            POINT point = {};
            ClientToScreen(userData->m_windowHandle, &point);
            *pos = ImVec2(point.x, point.y);
            userData->signal();
        }
        break;
    }
    case Win32ThreadedApplicationService::ViewportData::kMessageTypeSetWindowSize: {
        auto viewport = reinterpret_cast<const ImGuiViewport *>(wparam);
        if (auto userData = static_cast<Win32ThreadedApplicationService::ViewportData *>(viewport->PlatformUserData)) {
            auto size = reinterpret_cast<const ImVec2 *>(lparam);
            POINT point = {};
            RECT rect = { 0, 0, size->x, size->y };
            AdjustWindowRectEx(&rect, userData->m_style, FALSE, userData->m_styleEx);
            SetWindowPos(userData->m_windowHandle, nullptr, 0, 0,
                Win32ThreadedApplicationService::ViewportData::width(rect),
                Win32ThreadedApplicationService::ViewportData::height(rect),
                SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
            userData->signal();
        }
        break;
    }
    case Win32ThreadedApplicationService::ViewportData::kMessageTypeGetWindowSize: {
        auto viewport = reinterpret_cast<const ImGuiViewport *>(wparam);
        if (auto userData = static_cast<Win32ThreadedApplicationService::ViewportData *>(viewport->PlatformUserData)) {
            auto size = reinterpret_cast<ImVec2 *>(lparam);
            RECT rect;
            GetClientRect(userData->m_windowHandle, &rect);
            *size = Win32ThreadedApplicationService::ViewportData::size(rect);
            userData->signal();
        }
        break;
    }
    case Win32ThreadedApplicationService::ViewportData::kMessageTypeSetWindowFocus: {
        auto viewport = reinterpret_cast<const ImGuiViewport *>(wparam);
        if (auto userData = static_cast<Win32ThreadedApplicationService::ViewportData *>(viewport->PlatformUserData)) {
            BringWindowToTop(userData->m_windowHandle);
            SetForegroundWindow(userData->m_windowHandle);
            SetFocus(userData->m_windowHandle);
            userData->signal();
        }
        break;
    }
    case Win32ThreadedApplicationService::ViewportData::kMessageTypeGetWindowFocus: {
        auto viewport = reinterpret_cast<const ImGuiViewport *>(wparam);
        if (auto userData = static_cast<Win32ThreadedApplicationService::ViewportData *>(viewport->PlatformUserData)) {
            auto focused = reinterpret_cast<bool *>(lparam);
            *focused = GetForegroundWindow() == userData->m_windowHandle;
            userData->signal();
        }
        break;
    }
    case Win32ThreadedApplicationService::ViewportData::kMessageTypeGetWindowMinimized: {
        auto viewport = reinterpret_cast<const ImGuiViewport *>(wparam);
        if (auto userData = static_cast<Win32ThreadedApplicationService::ViewportData *>(viewport->PlatformUserData)) {
            auto minimized = reinterpret_cast<bool *>(lparam);
            *minimized = IsIconic(userData->m_windowHandle) != 0;
            userData->signal();
        }
        break;
    }
    case Win32ThreadedApplicationService::ViewportData::kMessageTypeSetWindowTitle: {
        auto viewport = reinterpret_cast<const ImGuiViewport *>(wparam);
        if (auto userData = static_cast<Win32ThreadedApplicationService::ViewportData *>(viewport->PlatformUserData)) {
            auto title = reinterpret_cast<const char *>(lparam);
            MutableWideString ws;
            StringUtils::getWideCharString(title, ws);
            SetWindowTextW(userData->m_windowHandle, ws.data());
            userData->signal();
        }
        break;
    }
    case Win32ThreadedApplicationService::ViewportData::kMessageTypeSetWindowAlpha: {
        auto viewport = reinterpret_cast<const ImGuiViewport *>(wparam);
        if (auto userData = static_cast<Win32ThreadedApplicationService::ViewportData *>(viewport->PlatformUserData)) {
            auto alpha = *reinterpret_cast<const float *>(lparam);
            HWND windowHandle = userData->m_windowHandle;
            DWORD style = GetWindowLongW(windowHandle, GWL_EXSTYLE);
            if (alpha < 1.0f) {
                style |= WS_EX_LAYERED;
                SetWindowLongW(windowHandle, GWL_EXSTYLE, style);
                SetLayeredWindowAttributes(windowHandle, 0, alpha * 0xff, LWA_ALPHA);
            }
            else {
                style &= ~WS_EX_LAYERED;
                SetWindowLongW(windowHandle, GWL_EXSTYLE, style);
            }
            userData->signal();
        }
        break;
    }
    case Win32ThreadedApplicationService::ViewportData::kMessageTypeUpdateWindow: {
        auto viewport = reinterpret_cast<const ImGuiViewport *>(wparam);
        if (auto userData = static_cast<Win32ThreadedApplicationService::ViewportData *>(viewport->PlatformUserData)) {
            auto updated = reinterpret_cast<bool *>(lparam);
            DWORD style, styleEx;
            Win32ThreadedApplicationService::ViewportData::getWindowStyle(viewport->Flags, style, styleEx);
            if (userData->m_style != style || userData->m_styleEx != styleEx) {
                userData->m_style = style;
                userData->m_styleEx = style;
                HWND windowHandle = userData->m_windowHandle;
                SetWindowLongW(windowHandle, GWL_STYLE, style);
                SetWindowLongW(windowHandle, GWL_EXSTYLE, styleEx);
                RECT rect(Win32ThreadedApplicationService::ViewportData::rect(viewport));
                AdjustWindowRectEx(&rect, style, FALSE, styleEx);
                SetWindowPos(windowHandle, nullptr, rect.left, rect.top,
                    Win32ThreadedApplicationService::ViewportData::width(rect),
                    Win32ThreadedApplicationService::ViewportData::height(rect),
                    SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
                ShowWindow(windowHandle, SW_SHOWNA);
                *updated = true;
            }
            userData->signal();
        }
        break;
    }
    case Win32ThreadedApplicationService::ViewportData::kMessageTypeGetDpiScale: {
        auto viewport = reinterpret_cast<const ImGuiViewport *>(wparam);
        if (auto userData = static_cast<Win32ThreadedApplicationService::ViewportData *>(viewport->PlatformUserData)) {
            auto dpiScale = reinterpret_cast<float *>(lparam);
            HMONITOR monitor = MonitorFromWindow(userData->m_windowHandle, MONITOR_DEFAULTTONEAREST);
            *dpiScale = Win32ThreadedApplicationService::calculateDevicePixelRatio(monitor);
            userData->signal();
        }
        break;
    }
    case Win32ThreadedApplicationService::ViewportData::kMessageTypeOnChangedViewport: {
        break;
    }
    case Win32ThreadedApplicationService::ViewportData::kMessageTypeSetIMEInputPos: {
        auto viewport = reinterpret_cast<const ImGuiViewport *>(wparam);
        if (auto userData = static_cast<Win32ThreadedApplicationService::ViewportData *>(viewport->PlatformUserData)) {
            auto pos = reinterpret_cast<const ImVec2 *>(lparam);
            COMPOSITIONFORM cf = { CFS_FORCE_POSITION,
                { static_cast<LONG>(pos->x - viewport->Pos.x), static_cast<LONG>(pos->y - viewport->Pos.y) },
                { 0, 0, 0, 0 } };
            if (HWND windowHandle = static_cast<HWND>(viewport->PlatformHandle)) {
                if (HIMC himc = ImmGetContext(windowHandle)) {
                    ImmSetCompositionWindow(himc, &cf);
                    ImmReleaseContext(windowHandle, himc);
                }
            }
            userData->signal();
        }
        break;
    }
#endif /* IMGUI_HAS_VIEWPORT */
    case WM_CLOSE: {
        if (auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
            self->handleWindowClose(hwnd);
        }
        break;
    }
    case WM_DESTROY: {
        if (auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
            self->handleWindowDestroy(hwnd);
        }
        break;
    }
    default:
        result = DefWindowProcW(hwnd, msg, wparam, lparam);
        break;
    }
    if (auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
        if (IProgressDialog *dialog = self->m_progressDialog.first) {
            if (dialog->HasUserCancelled()) {
                Progress::requestCancel();
                self->closeProgressDialog();
            }
        }
    }
    return result;
}

DWORD CALLBACK
MainWindow::collectPerformanceMetricsPeriodically(void *userData)
{
    auto self = static_cast<MainWindow *>(userData);
    PROCESS_MEMORY_COUNTERS counters = {};
    PDH_HQUERY query = nullptr;
    PDH_HCOUNTER counter = nullptr;
    PDH_FMT_COUNTERVALUE value = {};
    GetProcessMemoryInfo(self->m_processHandle, &counters, sizeof(counters));
    self->m_client->sendUpdatePerformanceMonitorMessage(nanoem_f32_t(value.doubleValue), counters.WorkingSetSize, 0);
    PdhOpenQueryW(nullptr, 0, &query);
    PdhAddCounterW(query, L"\\Processor(_Total)\\% Processor Time", 0, &counter);
    PdhCollectQueryData(query);
    while (self->m_running) {
        Sleep(1000);
        PdhCollectQueryData(query);
        PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, nullptr, &value);
        GetProcessMemoryInfo(self->m_processHandle, &counters, sizeof(counters));
        self->m_client->sendUpdatePerformanceMonitorMessage(
            nanoem_f32_t(value.doubleValue), counters.WorkingSetSize, 0);
    }
    PdhCloseQuery(query);
    return 0;
}

BaseApplicationService::KeyType
MainWindow::translateKey(LPARAM lparam) noexcept
{
    BaseApplicationService::KeyType key;
    switch (HIWORD(lparam) & 0x1ff) {
    case 0x00B:
        key = BaseApplicationService::kKeyType_0;
        break;
    case 0x002:
        key = BaseApplicationService::kKeyType_1;
        break;
    case 0x003:
        key = BaseApplicationService::kKeyType_2;
        break;
    case 0x004:
        key = BaseApplicationService::kKeyType_3;
        break;
    case 0x005:
        key = BaseApplicationService::kKeyType_4;
        break;
    case 0x006:
        key = BaseApplicationService::kKeyType_5;
        break;
    case 0x007:
        key = BaseApplicationService::kKeyType_6;
        break;
    case 0x008:
        key = BaseApplicationService::kKeyType_7;
        break;
    case 0x009:
        key = BaseApplicationService::kKeyType_8;
        break;
    case 0x00A:
        key = BaseApplicationService::kKeyType_9;
        break;
    case 0x01E:
        key = BaseApplicationService::kKeyType_A;
        break;
    case 0x030:
        key = BaseApplicationService::kKeyType_B;
        break;
    case 0x02E:
        key = BaseApplicationService::kKeyType_C;
        break;
    case 0x020:
        key = BaseApplicationService::kKeyType_D;
        break;
    case 0x012:
        key = BaseApplicationService::kKeyType_E;
        break;
    case 0x021:
        key = BaseApplicationService::kKeyType_F;
        break;
    case 0x022:
        key = BaseApplicationService::kKeyType_G;
        break;
    case 0x023:
        key = BaseApplicationService::kKeyType_H;
        break;
    case 0x017:
        key = BaseApplicationService::kKeyType_I;
        break;
    case 0x024:
        key = BaseApplicationService::kKeyType_J;
        break;
    case 0x025:
        key = BaseApplicationService::kKeyType_K;
        break;
    case 0x026:
        key = BaseApplicationService::kKeyType_L;
        break;
    case 0x032:
        key = BaseApplicationService::kKeyType_M;
        break;
    case 0x031:
        key = BaseApplicationService::kKeyType_N;
        break;
    case 0x018:
        key = BaseApplicationService::kKeyType_O;
        break;
    case 0x019:
        key = BaseApplicationService::kKeyType_P;
        break;
    case 0x010:
        key = BaseApplicationService::kKeyType_Q;
        break;
    case 0x013:
        key = BaseApplicationService::kKeyType_R;
        break;
    case 0x01F:
        key = BaseApplicationService::kKeyType_S;
        break;
    case 0x014:
        key = BaseApplicationService::kKeyType_T;
        break;
    case 0x016:
        key = BaseApplicationService::kKeyType_U;
        break;
    case 0x02F:
        key = BaseApplicationService::kKeyType_V;
        break;
    case 0x011:
        key = BaseApplicationService::kKeyType_W;
        break;
    case 0x02D:
        key = BaseApplicationService::kKeyType_X;
        break;
    case 0x015:
        key = BaseApplicationService::kKeyType_Y;
        break;
    case 0x02C:
        key = BaseApplicationService::kKeyType_Z;
        break;
    case 0x028:
        key = BaseApplicationService::kKeyType_APOSTROPHE;
        break;
    case 0x02B:
        key = BaseApplicationService::kKeyType_BACKSLASH;
        break;
    case 0x033:
        key = BaseApplicationService::kKeyType_COMMA;
        break;
    case 0x00D:
        key = BaseApplicationService::kKeyType_EQUAL;
        break;
    case 0x029:
        key = BaseApplicationService::kKeyType_GRAVE_ACCENT;
        break;
    case 0x01A:
        key = BaseApplicationService::kKeyType_LEFT_BRACKET;
        break;
    case 0x00C:
        key = BaseApplicationService::kKeyType_MINUS;
        break;
    case 0x034:
        key = BaseApplicationService::kKeyType_PERIOD;
        break;
    case 0x01B:
        key = BaseApplicationService::kKeyType_RIGHT_BRACKET;
        break;
    case 0x027:
        key = BaseApplicationService::kKeyType_SEMICOLON;
        break;
    case 0x035:
        key = BaseApplicationService::kKeyType_SLASH;
        break;
    case 0x056:
        key = BaseApplicationService::kKeyType_WORLD_2;
        break;
    case 0x00E:
        key = BaseApplicationService::kKeyType_BACKSPACE;
        break;
    case 0x153:
        key = BaseApplicationService::kKeyType_DELETE;
        break;
    case 0x14F:
        key = BaseApplicationService::kKeyType_END;
        break;
    case 0x01C:
        key = BaseApplicationService::kKeyType_ENTER;
        break;
    case 0x001:
        key = BaseApplicationService::kKeyType_ESCAPE;
        break;
    case 0x147:
        key = BaseApplicationService::kKeyType_HOME;
        break;
    case 0x152:
        key = BaseApplicationService::kKeyType_INSERT;
        break;
    case 0x15D:
        key = BaseApplicationService::kKeyType_MENU;
        break;
    case 0x151:
        key = BaseApplicationService::kKeyType_PAGE_DOWN;
        break;
    case 0x149:
        key = BaseApplicationService::kKeyType_PAGE_UP;
        break;
    case 0x045:
        key = BaseApplicationService::kKeyType_PAUSE;
        break;
    case 0x146:
        key = BaseApplicationService::kKeyType_PAUSE;
        break;
    case 0x039:
        key = BaseApplicationService::kKeyType_SPACE;
        break;
    case 0x00F:
        key = BaseApplicationService::kKeyType_TAB;
        break;
    case 0x03A:
        key = BaseApplicationService::kKeyType_CAPS_LOCK;
        break;
    case 0x145:
        key = BaseApplicationService::kKeyType_NUM_LOCK;
        break;
    case 0x046:
        key = BaseApplicationService::kKeyType_SCROLL_LOCK;
        break;
    case 0x03B:
        key = BaseApplicationService::kKeyType_F1;
        break;
    case 0x03C:
        key = BaseApplicationService::kKeyType_F2;
        break;
    case 0x03D:
        key = BaseApplicationService::kKeyType_F3;
        break;
    case 0x03E:
        key = BaseApplicationService::kKeyType_F4;
        break;
    case 0x03F:
        key = BaseApplicationService::kKeyType_F5;
        break;
    case 0x040:
        key = BaseApplicationService::kKeyType_F6;
        break;
    case 0x041:
        key = BaseApplicationService::kKeyType_F7;
        break;
    case 0x042:
        key = BaseApplicationService::kKeyType_F8;
        break;
    case 0x043:
        key = BaseApplicationService::kKeyType_F9;
        break;
    case 0x044:
        key = BaseApplicationService::kKeyType_F10;
        break;
    case 0x057:
        key = BaseApplicationService::kKeyType_F11;
        break;
    case 0x058:
        key = BaseApplicationService::kKeyType_F12;
        break;
    case 0x064:
        key = BaseApplicationService::kKeyType_F13;
        break;
    case 0x065:
        key = BaseApplicationService::kKeyType_F14;
        break;
    case 0x066:
        key = BaseApplicationService::kKeyType_F15;
        break;
    case 0x067:
        key = BaseApplicationService::kKeyType_F16;
        break;
    case 0x068:
        key = BaseApplicationService::kKeyType_F17;
        break;
    case 0x069:
        key = BaseApplicationService::kKeyType_F18;
        break;
    case 0x06A:
        key = BaseApplicationService::kKeyType_F19;
        break;
    case 0x06B:
        key = BaseApplicationService::kKeyType_F20;
        break;
    case 0x06C:
        key = BaseApplicationService::kKeyType_F21;
        break;
    case 0x06D:
        key = BaseApplicationService::kKeyType_F22;
        break;
    case 0x06E:
        key = BaseApplicationService::kKeyType_F23;
        break;
    case 0x076:
        key = BaseApplicationService::kKeyType_F24;
        break;
    case 0x038:
        key = BaseApplicationService::kKeyType_LEFT_ALT;
        break;
    case 0x01D:
        key = BaseApplicationService::kKeyType_LEFT_CONTROL;
        break;
    case 0x02A:
        key = BaseApplicationService::kKeyType_LEFT_SHIFT;
        break;
    case 0x15B:
        key = BaseApplicationService::kKeyType_LEFT_SUPER;
        break;
    case 0x137:
        key = BaseApplicationService::kKeyType_PRINT_SCREEN;
        break;
    case 0x138:
        key = BaseApplicationService::kKeyType_RIGHT_ALT;
        break;
    case 0x11D:
        key = BaseApplicationService::kKeyType_RIGHT_CONTROL;
        break;
    case 0x036:
        key = BaseApplicationService::kKeyType_RIGHT_SHIFT;
        break;
    case 0x15C:
        key = BaseApplicationService::kKeyType_RIGHT_SUPER;
        break;
    case 0x150:
        key = BaseApplicationService::kKeyType_DOWN;
        break;
    case 0x14B:
        key = BaseApplicationService::kKeyType_LEFT;
        break;
    case 0x14D:
        key = BaseApplicationService::kKeyType_RIGHT;
        break;
    case 0x148:
        key = BaseApplicationService::kKeyType_UP;
        break;
    case 0x052:
        key = BaseApplicationService::kKeyType_KP_0;
        break;
    case 0x04F:
        key = BaseApplicationService::kKeyType_KP_1;
        break;
    case 0x050:
        key = BaseApplicationService::kKeyType_KP_2;
        break;
    case 0x051:
        key = BaseApplicationService::kKeyType_KP_3;
        break;
    case 0x04B:
        key = BaseApplicationService::kKeyType_KP_4;
        break;
    case 0x04C:
        key = BaseApplicationService::kKeyType_KP_5;
        break;
    case 0x04D:
        key = BaseApplicationService::kKeyType_KP_6;
        break;
    case 0x047:
        key = BaseApplicationService::kKeyType_KP_7;
        break;
    case 0x048:
        key = BaseApplicationService::kKeyType_KP_8;
        break;
    case 0x049:
        key = BaseApplicationService::kKeyType_KP_9;
        break;
    case 0x04E:
        key = BaseApplicationService::kKeyType_KP_ADD;
        break;
    case 0x053:
        key = BaseApplicationService::kKeyType_KP_DECIMAL;
        break;
    case 0x135:
        key = BaseApplicationService::kKeyType_KP_DIVIDE;
        break;
    case 0x11C:
        key = BaseApplicationService::kKeyType_KP_ENTER;
        break;
    case 0x037:
        key = BaseApplicationService::kKeyType_KP_MULTIPLY;
        break;
    case 0x04A:
        key = BaseApplicationService::kKeyType_KP_SUBTRACT;
        break;
    default:
        key = BaseApplicationService::kKeyType_UNKNOWN;
        break;
    }
    return key;
}

Vector2SI32
MainWindow::devicePixelScreenPosition(HWND hwnd, const Vector2SI32 &value) noexcept
{
    POINT point = { value.x, value.y };
    ClientToScreen(hwnd, &point);
    return Vector2SI32(point.x, point.y);
}

int
MainWindow::cursorModifiers() noexcept
{
    int value = 0;
    if (GetKeyState(VK_SHIFT) < 0) {
        value |= Project::kCursorModifierTypeShift;
    }
    if (GetKeyState(VK_CONTROL) < 0) {
        value |= Project::kCursorModifierTypeControl;
    }
    if (GetKeyState(VK_MENU) < 0) {
        value |= Project::kCursorModifierTypeAlt;
    }
    return value;
}

int
MainWindow::convertCursorType(UINT msg, WPARAM wparam) noexcept
{
    int cursorType = -1;
    switch (msg) {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        cursorType = Project::kCursorTypeMouseLeft;
        break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        cursorType = Project::kCursorTypeMouseMiddle;
        break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
        cursorType = Project::kCursorTypeMouseRight;
        break;
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
        if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
            cursorType = Project::kCursorTypeMouseButton4;
        }
        else if (GET_XBUTTON_WPARAM(wparam) == XBUTTON2) {
            cursorType = Project::kCursorTypeMouseButton5;
        }
        break;
    default:
        break;
    }
    return cursorType;
}

void
MainWindow::handleAddingWatchEffectSource(void *userData, nanoem_u16_t handle, const URI &fileURI)
{
    auto self = static_cast<MainWindow *>(userData);
    String effectSourcePath(fileURI.absolutePathByDeletingPathExtension());
    effectSourcePath.append(".fx");
    if (FileUtils::exists(effectSourcePath.c_str())) {
        const String directory(URI::stringByDeletingLastPathComponent(effectSourcePath));
        auto it = self->m_watchEffectSourceHandles.find(directory);
        if (it != self->m_watchEffectSourceHandles.end()) {
            it->second.second.push_back(handle);
        }
        else {
            MutableWideString ws;
            StringUtils::getWideCharString(directory.c_str(), ws);
            HANDLE ch = FindFirstChangeNotificationW(ws.data(), false, FILE_NOTIFY_CHANGE_LAST_WRITE);
            if (ch != INVALID_HANDLE_VALUE) {
                HandleList handles;
                handles.push_back(handle);
                self->m_watchEffectSourceHandles.insert(tinystl::make_pair(directory, tinystl::make_pair(ch, handles)));
            }
        }
    }
}

void
MainWindow::handleRemovingWatchEffectSource(void *userData, uint16_t handle, const char * /* name */)
{
    auto self = static_cast<MainWindow *>(userData);
    for (auto &item : self->m_watchEffectSourceHandles) {
        auto &handles = item.second.second;
        for (auto it = handles.begin(), end = handles.end(); it != end; ++it) {
            if (*it == handle) {
                handles.erase(it);
                break;
            }
        }
    }
}

bool
MainWindow::handleWindowCreate(HWND hwnd, Error &error)
{
    bool result = initialize(hwnd, error);
    if (result) {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        registerAllPrerequisiteEventListeners();
    }
    return result;
}

void
MainWindow::handleWindowResize(HWND hwnd, WPARAM type)
{
    if (m_initialized) {
        if (hwnd == m_windowHandle) {
            bool renderable = true;
            if (type == SIZE_RESTORED) {
                m_client->sendActivateMessage();
            }
            else if (type == SIZE_MINIMIZED) {
                m_client->sendDeactivateMessage();
                renderable = false;
            }
            if (renderable) {
                resizeWindow();
            }
            m_renderable = renderable;
        }
        else {
            m_service->requestViewportWindowResize(hwnd);
        }
    }
}

void
MainWindow::handleWindowPositionChange(HWND hwnd)
{
    if (m_initialized && m_renderable) {
        const sg_backend backend = sg::query_backend();
        if (backend == SG_BACKEND_D3D11) {
            BOOL fullScreenState = FALSE;
            IDXGIOutput *outputTarget = nullptr;
            m_swapChain->GetFullscreenState(&fullScreenState, &outputTarget);
            bool isFullScreen = fullScreenState != 0;
            if (m_isFullScreen != isFullScreen) {
                m_isFullScreen = isFullScreen;
            }
        }
        resizeWindow();
    }
}

void
MainWindow::handleWindowConstraint(HWND hwnd, LPMINMAXINFO info)
{
    if (hwnd == m_windowHandle) {
        const Vector2UI32 &windowSize = ThreadedApplicationService::minimumRequiredWindowSize();
        RECT windowRect = { 0, 0, LONG(windowSize.x * m_devicePixelRatio), LONG(windowSize.y * m_devicePixelRatio) };
        AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, FALSE, 0);
        info->ptMinTrackSize.x = windowRect.right - windowRect.left;
        info->ptMinTrackSize.y = windowRect.bottom - windowRect.top;
    }
}

void
MainWindow::handleWindowDropFile(HWND hwnd, HDROP drop)
{
    UINT numFiles = DragQueryFileW(drop, UINT32_MAX, nullptr, 0);
    for (UINT i = 0; i < numFiles; i++) {
        MutableWideString path;
        MutableString tempFilePath;
        const UINT length = DragQueryFileW(drop, i, nullptr, 0) + 1;
        path.resize(length);
        DragQueryFileW(drop, i, path.data(), length);
        StringUtils::getMultiBytesString(path.data(), tempFilePath);
        String canonicalizedFilePath;
        FileUtils::canonicalizePathSeparator(tempFilePath.data(), canonicalizedFilePath);
        const URI &fileURI = URI::createFromFilePath(canonicalizedFilePath);
        m_client->sendDropFileMessage(fileURI);
        if (Project::isLoadableExtension(fileURI)) {
            setTitle(fileURI);
        }
    }
    BringWindowToTop(m_windowHandle);
    SetForegroundWindow(m_windowHandle);
    SetFocus(m_windowHandle);
}

void
MainWindow::handleWindowClose(HWND hwnd)
{
    if (m_windowHandle == hwnd) {
        m_client->sendIsProjectDirtyRequestMessage(
            [](void *userData, bool dirty) {
                auto self = static_cast<MainWindow *>(userData);
                if (dirty) {
                    ThreadedApplicationClient *client = self->m_client;
                    client->clearAllProjectAfterConfirmOnceEventListeners();
                    client->addSaveProjectAfterConfirmEventListener(
                        [](void *userData) {
                            auto self = static_cast<MainWindow *>(userData);
                            self->m_client->addCompleteSavingFileEventListener(
                                [](void *userData, const URI & /* fileURI */, uint32_t /* type */,
                                    uint64_t /* ticks */) {
                                    auto self = static_cast<MainWindow *>(userData);
                                    self->destroyWindow();
                                },
                                self, true);
                            self->saveProject();
                        },
                        self, true);
                    client->addDiscardProjectAfterConfirmEventListener(
                        [](void *userData) {
                            auto self = static_cast<MainWindow *>(userData);
                            self->destroyWindow();
                        },
                        self, true);
                    client->sendConfirmBeforeExitApplicationMessage();
                }
                else {
                    self->destroyWindow();
                }
            },
            this);
    }
    else {
        m_service->requestViewportWindowClose(hwnd);
    }
}

void
MainWindow::handleWindowDestroy(HWND hwnd)
{
    if (hwnd == m_windowHandle) {
        DestroyAcceleratorTable(m_accelerators);
        m_accelerators = nullptr;
        m_running = false;
        if (m_metricThreadHandle) {
            WaitForSingleObject(m_metricThreadHandle, INFINITE);
            CloseHandle(m_metricThreadHandle);
            m_metricThreadHandle = nullptr;
            CloseHandle(m_processHandle);
            m_processHandle = nullptr;
        }
        if (m_sentryDllHandle) {
            BaseApplicationService::closeSentryDll(m_sentryDllHandle);
            m_sentryDllHandle = nullptr;
        }
        PostQuitMessage(0);
    }
}

void
MainWindow::handleMouseDown(HWND hwnd, const Vector2SI32 &coord, int type)
{
    const Vector2SI32 logicalPosition(Vector2(coord) * invertedDevicePixelRatio());
    const int modifiers = cursorModifiers();
    SetCapture(hwnd);
    m_client->sendScreenCursorPressMessage(devicePixelScreenPosition(hwnd, coord), type, modifiers);
    if (m_windowHandle == hwnd) {
        m_client->sendCursorPressMessage(logicalPosition, type, modifiers);
    }
    setLastLogicalCursorPosition(logicalPosition);
}

void
MainWindow::handleMouseMove(HWND hwnd, const Vector2SI32 &coord, int type)
{
    const Vector2SI32 logicalPosition(Vector2(coord) * invertedDevicePixelRatio());
    const int modifiers = cursorModifiers();
    Vector2SI32 delta(0);
    m_client->sendScreenCursorMoveMessage(devicePixelScreenPosition(hwnd, coord), type, modifiers);
    if (m_disabledCursorState == kDisabledCursorStateInitial) {
        m_disabledCursorState = kDisabledCursorStateMoving;
    }
    else {
        delta = logicalPosition - lastLogicalCursorPosition();
    }
    if (m_windowHandle == hwnd) {
        const Vector2SI32 virtualPosition(virtualLogicalCursorPosition(logicalPosition));
        m_client->sendCursorMoveMessage(virtualPosition, delta, 0, modifiers);
    }
    setLastLogicalCursorPosition(logicalPosition, delta);
}

void
MainWindow::handleMouseUp(HWND hwnd, const Vector2SI32 &coord, int type)
{
    const Vector2SI32 logicalPosition(Vector2(coord) * invertedDevicePixelRatio());
    const int modifiers = cursorModifiers();
    if (!isCursorHidden()) {
        ReleaseCapture();
    }
    m_client->sendScreenCursorReleaseMessage(devicePixelScreenPosition(hwnd, coord), type, modifiers);
    if (m_windowHandle == hwnd) {
        m_client->sendCursorReleaseMessage(logicalPosition, type, modifiers);
    }
    setLastLogicalCursorPosition(logicalPosition);
}

void
MainWindow::handleMouseWheel(HWND hwnd, const Vector2SI32 &delta)
{
    const int modifiers = cursorModifiers();
    if (m_windowHandle == hwnd) {
        m_client->sendCursorScrollMessage(lastLogicalCursorPosition(), delta, modifiers);
    }
}

void
MainWindow::handleMenuItem(HWND hwnd, ApplicationMenuBuilder::MenuItemType menuType)
{
    switch (menuType) {
    case ApplicationMenuBuilder::kMenuItemTypeFileNewProject: {
        m_client->sendIsProjectDirtyRequestMessage(
            [](void *userData, bool dirty) {
                auto self = static_cast<MainWindow *>(userData);
                ThreadedApplicationClient *client = self->m_client;
                if (dirty) {
                    client->clearAllProjectAfterConfirmOnceEventListeners();
                    client->addSaveProjectAfterConfirmEventListener(
                        [](void *userData) {
                            auto self = static_cast<MainWindow *>(userData);
                            self->m_client->addCompleteSavingFileEventListener(
                                [](void *userData, const URI & /* fileURI */, uint32_t /* type */,
                                    uint64_t /* ticks */) {
                                    auto self = static_cast<MainWindow *>(userData);
                                    self->newProject();
                                },
                                self, true);
                            self->saveProject();
                        },
                        self, true);
                    client->addDiscardProjectAfterConfirmEventListener(
                        [](void *userData) {
                            auto self = static_cast<MainWindow *>(userData);
                            self->newProject();
                        },
                        self, true);
                    client->sendConfirmBeforeOpenProjectMessage();
                }
                else {
                    self->newProject();
                }
            },
            this);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileOpenProject: {
        m_client->sendIsProjectDirtyRequestMessage(
            [](void *userData, bool dirty) {
                auto self = static_cast<MainWindow *>(userData);
                ThreadedApplicationClient *client = self->m_client;
                if (dirty) {
                    client->clearAllProjectAfterConfirmOnceEventListeners();
                    client->addSaveProjectAfterConfirmEventListener(
                        [](void *userData) {
                            auto self = static_cast<MainWindow *>(userData);
                            self->m_client->addCompleteSavingFileEventListener(
                                [](void *userData, const URI & /* fileURI */, uint32_t /* type */,
                                    uint64_t /* ticks */) {
                                    auto self = static_cast<MainWindow *>(userData);
                                    self->openProject();
                                },
                                self, true);
                            self->saveProject();
                        },
                        self, true);
                    client->addDiscardProjectAfterConfirmEventListener(
                        [](void *userData) {
                            auto self = static_cast<MainWindow *>(userData);
                            self->openProject();
                        },
                        self, true);
                    client->sendConfirmBeforeOpenProjectMessage();
                }
                else {
                    self->openProject();
                }
            },
            this);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileImportModel: {
        Dialog dialog(m_windowHandle);
        Dialog::FilterList filters;
        filters.push_back(COMDLG_FILTERSPEC { L"MikuMikuDance Model Format (*.pmd *.pmx)", L"*.pmd;*.pmx" });
        if (dialog.open(filters)) {
            loadFile(dialog, IFileManager::kDialogTypeOpenModelFile);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileImportAccessory: {
        Dialog dialog(m_windowHandle);
        Dialog::FilterList filters;
        filters.push_back(COMDLG_FILTERSPEC { L"DirectX Mesh Format (*.x)", L"*.x" });
        if (dialog.open(filters)) {
            loadFile(dialog, IFileManager::kDialogTypeOpenModelFile);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileImportModelPose: {
        Dialog dialog(m_windowHandle);
        Dialog::FilterList filters;
        filters.push_back(COMDLG_FILTERSPEC { L"MikuMikuDance Pose Format (*.vpd)", L"*.vpd" });
        if (dialog.open(filters)) {
            loadFile(dialog, IFileManager::kDialogTypeOpenModelMotionFile);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileImportModelMotion: {
        Dialog dialog(m_windowHandle);
        Dialog::FilterList filters;
        filters.push_back(
            COMDLG_FILTERSPEC { L"All Avaiable Motion Format (*.nmd, *.vmd, *.vpd)", L"*.nmd;*.vmd;*.vpd" });
        filters.push_back(COMDLG_FILTERSPEC { L"nanoem Motion Format (*.nmd) (*.nmd)", L"*.nmd" });
        filters.push_back(COMDLG_FILTERSPEC { L"MikuMikuDance Motion Format (*.vmd)", L"*.vmd" });
        filters.push_back(COMDLG_FILTERSPEC { L"MikuMikuDance Pose Format (*.vpd)", L"*.vpd" });
        if (dialog.open(filters)) {
            loadFile(dialog, IFileManager::kDialogTypeOpenModelMotionFile);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileImportCameraMotion: {
        Dialog dialog(m_windowHandle);
        Dialog::FilterList filters;
        filters.push_back(COMDLG_FILTERSPEC { L"All Avaiable Motion Format (*.nmd, *.vmd)", L"*.nmd;*.vmd" });
        filters.push_back(COMDLG_FILTERSPEC { L"nanoem Motion Format (*.nmd) (*.nmd)", L"*.nmd" });
        filters.push_back(COMDLG_FILTERSPEC { L"MikuMikuDance Motion Format (*.vmd)", L"*.vmd" });
        if (dialog.open(filters)) {
            loadFile(dialog, IFileManager::kDialogTypeOpenCameraMotionFile);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileImportLightMotion: {
        Dialog dialog(m_windowHandle);
        Dialog::FilterList filters;
        filters.push_back(COMDLG_FILTERSPEC { L"All Avaiable Motion Format (*.nmd, *.vmd)", L"*.nmd;*.vmd" });
        filters.push_back(COMDLG_FILTERSPEC { L"nanoem Motion Format (*.nmd) (*.nmd)", L"*.nmd" });
        filters.push_back(COMDLG_FILTERSPEC { L"MikuMikuDance Motion Format (*.vmd)", L"*.vmd" });
        if (dialog.open(filters)) {
            loadFile(dialog, IFileManager::kDialogTypeOpenLightMotionFile);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileImportAudioSource: {
        m_client->addAvailableAllImportingAudioExtensionsEvent(
            [](void *userData, const StringList &extensions) {
                MainWindow *window = static_cast<MainWindow *>(userData);
                Dialog dialog(window->m_windowHandle);
                StringList newExtensions;
                newExtensions.push_back("wav");
                for (auto &item : extensions) {
                    newExtensions.push_back(item);
                }
                if (dialog.open("Importable Audio Format (*.%s)", newExtensions)) {
                    window->loadFile(dialog, IFileManager::kDialogTypeOpenAudioFile);
                }
            },
            this, true);
        m_client->sendLoadAllDecoderPluginsMessage(cachedAggregateAllPlugins());
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileImportBackgroundVideo: {
        m_client->addAvailableAllImportingVideoExtensionsEvent(
            [](void *userData, const StringList &extensions) {
                MainWindow *window = static_cast<MainWindow *>(userData);
                Dialog dialog(window->m_windowHandle);
                if (dialog.open("Importable Video Format (*.%s)", extensions)) {
                    window->loadFile(dialog, IFileManager::kDialogTypeOpenVideoFile);
                }
            },
            this, true);
        m_client->sendLoadAllDecoderPluginsMessage(cachedAggregateAllPlugins());
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileSaveProject: {
        saveProject();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileSaveAsProject: {
        saveProjectAs();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileExportModel: {
        Dialog dialog(m_windowHandle);
        Dialog::FilterList filters;
        filters.push_back(COMDLG_FILTERSPEC { L"MikuMikuDance Model Format (*.pmx)", L"*.pmx" });
        if (dialog.save(filters, localizedString("nanoem.dialog.filename.untitled"))) {
            saveFile(dialog, IFileManager::kDialogTypeSaveModelFile);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileExportModelPose: {
        Dialog dialog(m_windowHandle);
        Dialog::FilterList filters;
        filters.push_back(COMDLG_FILTERSPEC { L"MikuMikuDance Pose Format (*.vpd)", L"*.vpd" });
        if (dialog.save(filters, localizedString("nanoem.dialog.filename.untitled"))) {
            saveFile(dialog, IFileManager::kDialogTypeSaveModelMotionFile);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileExportModelMotion: {
        Dialog dialog(m_windowHandle);
        Dialog::FilterList filters;
        filters.push_back(COMDLG_FILTERSPEC { L"nanoem Motion Format (*.nmd)", L"*.nmd" });
        filters.push_back(COMDLG_FILTERSPEC { L"MikuMikuDance Motion Format (*.vmd)", L"*.vmd" });
        if (dialog.save(filters, localizedString("nanoem.dialog.filename.untitled"))) {
            saveFile(dialog, IFileManager::kDialogTypeSaveModelMotionFile);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileExportCameraMotion: {
        Dialog dialog(m_windowHandle);
        Dialog::FilterList filters;
        filters.push_back(COMDLG_FILTERSPEC { L"nanoem Motion Format (*.nmd)", L"*.nmd" });
        filters.push_back(COMDLG_FILTERSPEC { L"MikuMikuDance Motion Format (*.vmd)", L"*.vmd" });
        if (dialog.save(filters, localizedString("nanoem.dialog.filename.untitled"))) {
            saveFile(dialog, IFileManager::kDialogTypeSaveCameraMotionFile);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileExportLightMotion: {
        Dialog dialog(m_windowHandle);
        Dialog::FilterList filters;
        filters.push_back(COMDLG_FILTERSPEC { L"nanoem Motion Format (*.nmd)", L"*.nmd" });
        filters.push_back(COMDLG_FILTERSPEC { L"MikuMikuDance Motion Format (*.vmd)", L"*.vmd" });
        if (dialog.save(filters, localizedString("nanoem.dialog.filename.untitled"))) {
            saveFile(dialog, IFileManager::kDialogTypeSaveLightMotionFile);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileExportImage: {
        m_client->addSaveProjectAfterConfirmEventListener(
            [](void *userData) {
                auto self = static_cast<MainWindow *>(userData);
                self->m_client->addCompleteSavingFileEventListener(
                    [](void *userData, const URI & /* fileURI */, uint32_t /* type */, uint64_t /* ticks */) {
                        auto self = static_cast<MainWindow *>(userData);
                        self->exportImage();
                    },
                    self, true);
                self->saveProject();
            },
            this, true);
        m_client->addDiscardProjectAfterConfirmEventListener(
            [](void *userData) {
                auto self = static_cast<MainWindow *>(userData);
                self->exportImage();
            },
            this, true);
        m_client->sendConfirmBeforeExportingImageMessage();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileExportVideo: {
        m_client->addSaveProjectAfterConfirmEventListener(
            [](void *userData) {
                auto self = static_cast<MainWindow *>(userData);
                self->m_client->addCompleteSavingFileEventListener(
                    [](void *userData, const URI & /* fileURI */, uint32_t /* type */, uint64_t /* ticks */) {
                        auto self = static_cast<MainWindow *>(userData);
                        self->exportVideo();
                    },
                    self, true);
                self->saveProject();
            },
            this, true);
        m_client->addDiscardProjectAfterConfirmEventListener(
            [](void *userData) {
                auto self = static_cast<MainWindow *>(userData);
                self->exportVideo();
            },
            this, true);
        m_client->sendConfirmBeforeExportingVideoMessage();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeFileExit: {
        auto self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        CloseWindow(self->m_windowHandle);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeWindowMaximize: {
        ShowWindow(hwnd, SW_MAXIMIZE);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeWindowMinimize: {
        ShowWindow(hwnd, SW_MINIMIZE);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeWindowRestore: {
        ShowWindow(hwnd, SW_RESTORE);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeWindowFullscreen: {
        Error error;
        BOOL result = FALSE;
        IDXGISwapChain *swapChain = m_swapChain;
        IDXGIOutput *outputTarget = nullptr;
        swapChain->GetFullscreenState(&result, &outputTarget);
        BOOL toggle = result ? false : true;
        COMInline::wrapCall(swapChain->SetFullscreenState(toggle, outputTarget), error);
        if (!error.hasReason()) {
            m_isFullScreen = toggle != 0;
        }
        else {
            error.addModalDialog(m_service);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeHelpOnline: {
        char buffer[128];
        StringUtils::format(buffer, sizeof(buffer),
            "https://nanoem.readthedocs.io/?utm_source=nanoem-%s&utm_medium=referral", nanoemGetVersionString());
        ShellExecuteA(nullptr, nullptr, buffer, nullptr, nullptr, SW_SHOW);
        break;
    }
    default:
        m_menuBuilder->dispatch(menuType);
        break;
    }
}

bool
MainWindow::setupDirectXRenderer(HWND windowHandle, int width, int height, bool &isLowPower, Error &error)
{
    DXGI_MODE_DESC &mode = m_swapChainDesc.BufferDesc;
    mode.Width = width;
    mode.Height = height;
    const ApplicationPreference preference(m_service);
    switch (preference.defaultColorPixelFormat()) {
    case SG_PIXELFORMAT_RGBA8:
    default: {
        mode.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        break;
    }
    case SG_PIXELFORMAT_RGB10A2: {
        mode.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
        break;
    }
    case SG_PIXELFORMAT_RGBA16F: {
        mode.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        break;
    }
    }
    DXGI_RATIONAL &refreshRate = mode.RefreshRate;
    refreshRate.Denominator = 1;
    refreshRate.Numerator = 60;
    m_swapChainDesc.OutputWindow = windowHandle;
    m_swapChainDesc.Windowed = true;
    m_swapChainDesc.BufferCount = 2;
    m_swapChainDesc.SwapEffect = IsWindows10OrGreater() ? DXGI_SWAP_EFFECT_FLIP_DISCARD : DXGI_SWAP_EFFECT_DISCARD;
    m_swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    DXGI_SAMPLE_DESC &sample = m_swapChainDesc.SampleDesc;
    sample.Count = 1;
    sample.Quality = 0;
    m_swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    static const D3D_FEATURE_LEVEL expectedFeatureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
    };
    const UINT numExpectedFeatureLevels = sizeof(expectedFeatureLevels) / sizeof(expectedFeatureLevels[0]);
    ID3D11DeviceContext *context = nullptr;
    ID3D11Device *device = nullptr;
    IDXGISwapChain *swapChain = nullptr;
    D3D_FEATURE_LEVEL actualFeatureLevel;
    HRESULT rc = 0;
    bool result = false;
    UINT flags = 0;
#if defined(SOKOL_DEBUG) && SOKOL_DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif /* SOKOL_DEBUG */
#if defined(NANOEM_ENABLE_D3D11ON12)
    {
        IDXGIFactory *factory = nullptr;
        CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
        IDXGIAdapter *adapter = nullptr;
        factory->EnumAdapters(0, &adapter);
        D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device12));
        adapter->Release();
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        m_device12->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue));
        rc = D3D11On12CreateDevice(m_device12, flags, expectedFeatureLevels, numExpectedFeatureLevels,
            reinterpret_cast<IUnknown **>(&m_commandQueue), 1, 0, &device, &context, &actualFeatureLevel);
        factory->CreateSwapChain(device, &m_swapChainDesc, &swapChain);
        factory->Release();
        result = !FAILED(rc);
    }
#endif /* NANOEM_ENABLE_D3D11ON12 */
    if (!result) {
        static const D3D_DRIVER_TYPE driverTypes[] = {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
        };
        for (size_t i = 0; i < BX_COUNTOF(driverTypes); i++) {
            rc = D3D11CreateDeviceAndSwapChain(nullptr, driverTypes[i], nullptr, flags, expectedFeatureLevels,
                numExpectedFeatureLevels, D3D11_SDK_VERSION, &m_swapChainDesc, &swapChain, &device, &actualFeatureLevel,
                &context);
            if (!FAILED(rc)) {
                result = true;
                break;
            }
        }
    }
    if (result) {
        m_swapChain = swapChain;
        m_context = context;
        m_device = device;
        IDXGIDevice *deviceDXGI = nullptr;
        device->QueryInterface(IID_PPV_ARGS(&deviceDXGI));
        IDXGIAdapter *adapter = nullptr;
        deviceDXGI->GetParent(IID_PPV_ARGS(&adapter));
        IDXGIFactory *factory = nullptr;
        adapter->GetParent(IID_PPV_ARGS(&factory));
        factory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_PRINT_SCREEN);
        IDXGIFactory6 *factory6 = nullptr;
        if (!FAILED(factory->QueryInterface(IID_PPV_ARGS(&factory6)))) {
            DXGI_ADAPTER_DESC adapterDesc = {};
            adapter->GetDesc(&adapterDesc);
            IDXGIAdapter1 *adapterItem = nullptr;
            UINT index;
            isLowPower = false;
            for (index = 0; factory6->EnumAdapterByGpuPreference(index, DXGI_GPU_PREFERENCE_MINIMUM_POWER,
                                IID_PPV_ARGS(&adapterItem)) != DXGI_ERROR_NOT_FOUND;
                 index++) {
                if (adapterItem) {
                    DXGI_ADAPTER_DESC1 adapterItemDesc = {};
                    adapterItem->GetDesc1(&adapterItemDesc);
                    if (!EnumUtils::isEnabled(adapterItemDesc.Flags, DXGI_ADAPTER_FLAG_SOFTWARE) &&
                        adapterItemDesc.AdapterLuid.HighPart == adapterDesc.AdapterLuid.HighPart &&
                        adapterItemDesc.AdapterLuid.LowPart == adapterDesc.AdapterLuid.LowPart) {
                        isLowPower = true;
                        break;
                    }
                }
            }
        }
        else {
            isLowPower = true;
        }
#if defined(SOKOL_DEBUG) && SOKOL_DEBUG
        static const wchar_t kSwapChainLabel[] = L"IDXGISwapChain";
        swapChain->SetPrivateData(WKPDID_D3DDebugObjectNameW, sizeof(kSwapChainLabel), kSwapChainLabel);
        static const wchar_t kDeviceLabel[] = L"ID3D11Device";
        device->SetPrivateData(WKPDID_D3DDebugObjectNameW, sizeof(kDeviceLabel), kDeviceLabel);
        static const wchar_t kDeviceContextLabel[] = L"ID3D11DeviceContext";
        context->SetPrivateData(WKPDID_D3DDebugObjectNameW, sizeof(kDeviceContextLabel), kDeviceContextLabel);
#endif /* SOKOL_DEBUG */
        ID3D10Multithread *threaded;
        if (!FAILED(context->QueryInterface(IID_PPV_ARGS(&threaded)))) {
            threaded->SetMultithreadProtected(TRUE);
            threaded->Release();
        }
        m_service->setNativeContext(context);
        m_service->setNativeDevice(device);
        m_service->setNativeView(windowHandle);
        m_service->setNativeSwapChain(swapChain);
        m_service->setNativeSwapChainDescription(&m_swapChainDesc);
        DXGI_ADAPTER_DESC adapterDesc = {};
        adapter->GetDesc(&adapterDesc);
        char adapterName[ARRAYSIZE(adapterDesc.Description)];
        WideCharToMultiByte(CP_UTF8, 0, adapterDesc.Description, ARRAYSIZE(adapterDesc.Description), adapterName,
            sizeof(adapterName), 0, false);
        JSON_Object *pending = json_object(m_service->applicationPendingChangeConfiguration());
        json_object_dotset_string(pending, "win32.renderer.name", adapterName);
    }
    else {
        COMInline::wrapCall(rc, error);
    }
    return result;
}

bool
MainWindow::setupOpenGLRenderer(HWND windowHandle, Error &error)
{
#if defined(NANOEM_WIN32_HAS_OPENGL)
    typedef BOOL(WINAPI * pfn_wglSwapIntervalEXT)(int);
    typedef HGLRC(WINAPI * pfn_wglCreateContextAttribsARB)(HDC, HGLRC, const int *);
    static const int WGL_CONTEXT_MAJOR_VERSION_ARB = 0x2091;
    static const int WGL_CONTEXT_MINOR_VERSION_ARB = 0x2092;
    static const int WGL_CONTEXT_FLAGS_ARB = 0x2094;
    static const int WGL_CONTEXT_DEBUG_BIT_ARB = 0x0001;
    static const int WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB = 0x0002;
    static const int WGL_CONTEXT_PROFILE_MASK_ARB = 0x9126;
    static const int WGL_CONTEXT_CORE_PROFILE_BIT_ARB = 0x1;
    static const int kCreateOpenGLContextAttributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB,
        3,
        WGL_CONTEXT_MINOR_VERSION_ARB,
        3,
        WGL_CONTEXT_FLAGS_ARB,
#if defined(SOKOL_DEBUG) && SOKOL_DEBUG
        WGL_CONTEXT_DEBUG_BIT_ARB |
#endif /* SOKOL_DEBUG */
            WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    HDC device = GetDC(windowHandle);
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    int pixelFormat = ChoosePixelFormat(device, &pfd);
    if (!DescribePixelFormat(device, pixelFormat, sizeof(pfd), &pfd) || !SetPixelFormat(device, pixelFormat, &pfd)) {
        createLastError(error);
        return false;
    }
    HGLRC temp = wglCreateContext(device);
    if (!wglMakeCurrent(device, temp)) {
        createLastError(error);
        wglDeleteContext(temp);
        return false;
    }
    if (auto wglSwapIntervalEXT = reinterpret_cast<pfn_wglSwapIntervalEXT>(wglGetProcAddress("wglSwapIntervalEXT"))) {
        wglSwapIntervalEXT(1);
    }
    HGLRC context = nullptr;
    if (auto wglCreateContextAttribsARB =
            reinterpret_cast<pfn_wglCreateContextAttribsARB>(wglGetProcAddress("wglCreateContextAttribsARB"))) {
        context = wglCreateContextAttribsARB(device, temp, kCreateOpenGLContextAttributes);
    }
    if (!context) {
        createLastError(error);
        wglDeleteContext(temp);
        return false;
    }
    else if (HMODULE handle = LoadLibraryW(L"opengl32.dll")) {
        typedef const uint8_t *(WINAPI * pfn_glGetString)(unsigned int);
        static const unsigned GL_RENDERER = 0x1f01;
        if (auto glGetString = reinterpret_cast<pfn_glGetString>(GetProcAddress(handle, "glGetString"))) {
            JSON_Object *pending = json_object(m_service->applicationPendingChangeConfiguration());
            json_object_dotset_string(
                pending, "win32.renderer.name", reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
        }
        FreeLibrary(handle);
    }
    wglMakeCurrent(device, nullptr);
    wglDeleteContext(temp);
    m_context = context;
    m_device = device;
    m_service->setNativeContext(m_context);
    m_service->setNativeDevice(m_device);
    m_service->setNativeView(windowHandle);
    return true;
#else /* NANOEM_WIN32_HAS_OPENGL */
    BX_UNUSED_1(windowHandle);
    return false;
#endif /* NANOEM_WIN32_HAS_OPENGL */
}

void
MainWindow::destroyRenderer()
{
    if (IDXGISwapChain *swapChain = m_swapChain) {
        ID3D11DeviceContext *context = static_cast<ID3D11DeviceContext *>(m_context);
        ID3D11Device *device = static_cast<ID3D11Device *>(m_device);
        swapChain->SetFullscreenState(FALSE, nullptr);
        context->Release();
        device->Release();
        swapChain->Release();
#if defined(NANOEM_ENABLE_D3D11ON12)
        COMInline::safeRelease(m_commandQueue);
        COMInline::safeRelease(m_device12);
#endif /* NANOEM_ENABLE_D3D11ON12 */
#if defined(SOKOL_DEBUG) && SOKOL_DEBUG
        if (sizeof(void *) == 8) {
            typedef HRESULT(WINAPI * pfn_DXGIGetDebugInterface)(const IID, void **);
            if (HMODULE module = GetModuleHandleW(L"dxgidebug.dll")) {
                pfn_DXGIGetDebugInterface DXGIGetDebugInterface =
                    reinterpret_cast<pfn_DXGIGetDebugInterface>(GetProcAddress(module, "DXGIGetDebugInterface"));
                IDXGIDebug *debugger = nullptr;
                DXGIGetDebugInterface(IID_PPV_ARGS(&debugger));
                debugger->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_DETAIL);
                debugger->Release();
            }
        }
#endif /* SOKOL_DEBUG */
    }
#if defined(NANOEM_WIN32_HAS_OPENGL)
    else {
        HGLRC context = static_cast<HGLRC>(m_context);
        HDC device = static_cast<HDC>(m_device);
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(context);
        ReleaseDC(m_windowHandle, device);
    }
#endif /* NANOEM_WIN32_HAS_OPENGL */
}

void
MainWindow::destroyWindow()
{
    IProgressDialog *dialog;
    if (openProgressDialog(dialog)) {
        dialog->SetTitle(localizedWideString("nanoem.dialog.progress.exit.title"));
        dialog->SetLine(1, localizedWideString("nanoem.dialog.progress.exit.message"), FALSE, nullptr);
    }
    m_client->addCompleteDestructionEventListener(
        [](void *userData) {
            auto self = static_cast<MainWindow *>(userData);
            self->m_renderable = false;
            self->m_client->addCompleteTerminationEventListener(
                [](void *userData) {
                    auto self = static_cast<MainWindow *>(userData);
                    self->closeProgressDialog();
                    self->destroyRenderer();
                    DestroyMenu(self->m_menuHandle);
                    DestroyWindow(self->m_windowHandle);
                },
                self, true);
            self->m_client->sendTerminateMessage();
        },
        this, true);
    m_client->sendDestroyMessage();
}

void
MainWindow::registerAllPrerequisiteEventListeners()
{
    m_client->addInitializationCompleteEventListener(
        [](void *userData) {
            auto self = static_cast<MainWindow *>(userData);
            const bx::CommandLine *cmd = self->m_commandLine;
            HWND windowHandle = self->m_windowHandle;
            URIList plugins(self->cachedAggregateAllPlugins());
            self->m_client->sendLoadAllModelPluginsMessage(plugins);
            self->m_client->sendLoadAllMotionPluginsMessage(plugins);
#if defined(NANOEM_ENABLE_DEBUG_LABEL)
            if (cmd->hasArg("bootstrap-project-from-clipboard") && IsClipboardFormatAvailable(CF_UNICODETEXT)) {
                HANDLE clipboard = nullptr;
                if (OpenClipboard(windowHandle) && (clipboard = GetClipboardData(CF_UNICODETEXT))) {
                    MutableWideString ws(GlobalSize(clipboard) + 1);
                    if (const wchar_t *source = static_cast<const wchar_t *>(GlobalLock(clipboard))) {
                        size_t length = wcslen(source);
                        if (length > 2) {
                            wcsncpy(ws.data(), source + 1, length - 2);
                            ws[length - 2] = 0;
                            self->loadProjectFromFile(ws.data());
                        }
                        GlobalUnlock(clipboard);
                        CloseClipboard();
                    }
                }
            }
            else if (cmd->hasArg("bootstrap-project")) {
                MutableWideString ws;
                StringUtils::getWideCharString(cmd->findOption("bootstrap-project"), ws, 932);
                if (!ws.empty()) {
                    self->loadProjectFromFile(ws.data());
                }
            }
            else if (cmd->hasArg("recovery-from-latest")) {
                String path(json_object_dotget_string(
                    json_object(self->m_service->applicationConfiguration()), "win32.redo.path")),
                    basePath(path);
                basePath.append("/*");
                MutableWideString wideBasePath;
                StringUtils::getWideCharString(basePath.c_str(), wideBasePath);
                WIN32_FIND_DATAW data;
                HANDLE handle = FindFirstFileW(wideBasePath.data(), &data);
                if (handle != INVALID_HANDLE_VALUE) {
                    URI fileURI;
                    String pathExtension(Project::kRedoLogFileExtension);
                    FILETIME latest = { 0, 0 };
                    do {
                        MutableString filename;
                        StringUtils::getMultiBytesString(data.cFileName, filename);
                        if (URI::pathExtension(filename.data()) == pathExtension &&
                            CompareFileTime(&data.ftLastWriteTime, &latest) > 0) {
                            String redoPath(path);
                            redoPath.append("/");
                            redoPath.append(filename.data());
                            fileURI = URI::createFromFilePath(redoPath.c_str());
                            latest = data.ftLastWriteTime;
                        }
                    } while (FindNextFileW(handle, &data));
                    FindClose(handle);
                    if (FileUtils::exists(fileURI)) {
                        self->m_client->sendRecoveryMessage(fileURI);
                    }
                }
            }
            else if (cmd->hasArg("recovery-from")) {
                MutableWideString ws;
                StringUtils::getWideCharString(cmd->findOption("recovery-from"), ws, 932);
                MutableString path;
                StringUtils::getMultiBytesString(ws.data(), path);
                FileUtils::canonicalizePathSeparator(path);
                const URI &fileURI = URI::createFromFilePath(path.data());
                if (FileUtils::exists(fileURI)) {
                    self->m_client->sendRecoveryMessage(fileURI);
                }
            }
#endif /* NANOEM_ENABLE_DEBUG_LABEL */
            DWORD processId = GetCurrentProcessId();
            self->m_processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, false, processId);
            if (self->m_processHandle != nullptr) {
                self->m_metricThreadHandle =
                    CreateThread(nullptr, 0, collectPerformanceMetricsPeriodically, self, 0, nullptr);
            }
            ImGui::GetIO().UserData = self;
            const JSON_Object *config = json_object(self->m_service->applicationConfiguration());
            MutableString redoFilePath;
            StringUtils::getMultiBytesString(Win32ThreadedApplicationService::sharedRedoFilePath(), redoFilePath);
            const char *sentryDSN = nullptr;
#if defined(NANOEM_SENTRY_DSN)
            sentryDSN = NANOEM_SENTRY_DSN;
#endif /* NANOEM_SENTRY_DSN */
            JSON_Object *pending = json_object(self->m_service->applicationPendingChangeConfiguration());
            const ApplicationPreference *pref = self->m_preference->applicationPreference();
            if (pref->isCrashReportEnabled()) {
                MutableString redoFilePath;
                StringUtils::getMultiBytesString(Win32ThreadedApplicationService::sharedRedoFilePath(), redoFilePath);
                auto maskString = [](const char *value) {
                    wchar_t userProfilePath[MAX_PATH];
                    ExpandEnvironmentStringsW(L"%USERPROFILE%", userProfilePath, ARRAYSIZE(userProfilePath));
                    wchar_t *p = userProfilePath;
                    while ((p = wcschr(p, L'\\')) != nullptr) {
                        *p = L'/';
                    }
                    MutableString userProfileString;
                    StringUtils::getMultiBytesString(userProfilePath, userProfileString);
                    std::string maskedPathString(value, StringUtils::length(value));
                    size_t startPos = 0, len = userProfileString.size() - 1;
                    while (
                        (startPos = maskedPathString.find(userProfileString.data(), startPos)) != std::string::npos) {
                        maskedPathString.replace(startPos, len, "{HOME}");
                        startPos += len;
                    }
                    return sentry_value_new_string(maskedPathString.c_str());
                };
                BaseApplicationService::SentryDescription desc;
                desc.m_clientUUID = self->m_preference->uuidConstString();
                desc.m_databaseDirectoryPath = json_object_dotget_string(config, "win32.sentry.database.path");
                desc.m_deviceModelName = nullptr;
                desc.m_dllFilePath = "sentry.dll";
                desc.m_dsn = sentryDSN;
                desc.m_handlerFilePath = json_object_dotget_string(config, "win32.sentry.handler.path");
                desc.m_isModelEditingEnabled = pref->isModelEditingEnabled();
                desc.m_localeName = json_object_dotget_string(config, "project.locale");
                desc.m_maskString = maskString;
                desc.m_rendererName = json_object_dotget_string(pending, "win32.renderer.name");
                desc.m_transportSendEnvelope = nullptr;
                desc.m_transportUserData = nullptr;
                self->m_sentryDllHandle = BaseApplicationService::openSentryDll(desc);
                if (self->m_sentryDllHandle) {
                    SYSTEMTIME time = {};
                    GetSystemTime(&time);
                    char timeString[32];
                    StringUtils::format(timeString, sizeof(timeString), "%04d-%02d-%02dT%02d:%02d:%02d", time.wYear,
                        time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
                    sentry_set_extra("initialized", sentry_value_new_string(timeString));
                }
            }
            json_object_dotremove(pending, "win32.renderer.name");
            self->m_initialized = true;
            self->m_client->sendActivateMessage();
            DrawMenuBar(windowHandle);
            ShowWindow(windowHandle, SW_SHOWDEFAULT);
            UpdateWindow(windowHandle);
        },
        this, true);
    m_client->addDisableCursorEventListener(
        [](void *userData, const Vector2SI32 &logicalCursorPosition) {
            auto self = static_cast<MainWindow *>(userData);
            self->disableCursor(logicalCursorPosition);
        },
        this, false);
    m_client->addEnableCursorEventListener(
        [](void *userData, const Vector2SI32 &logicalCursorPosition) {
            auto self = static_cast<MainWindow *>(userData);
            self->enableCursor(logicalCursorPosition);
        },
        this, false);
    m_client->addErrorEventListener(
        [](void *userData, int code, const char *reason, const char *recoverySuggestion) {
            auto self = static_cast<MainWindow *>(userData);
            self->writeErrorLog(code, reason, recoverySuggestion);
        },
        this, false);
    m_client->addQueryOpenSingleFileDialogEventListener(
        [](void *userData, uint32_t types, const StringList &allowedExtensions) {
            auto self = static_cast<MainWindow *>(userData);
            Dialog dialog(self->m_windowHandle);
            if (dialog.open("", allowedExtensions)) {
                self->m_client->sendQueryOpenSingleFileDialogMessage(types, dialog.fileURI());
            }
            else {
                self->m_client->sendQueryOpenSingleFileDialogMessage(types, URI());
            }
        },
        this, false);
    m_client->addQueryOpenMultipleFilesDialogEventListener(
        [](void *userData, uint32_t types, const StringList &allowedExtensions) {
            auto self = static_cast<MainWindow *>(userData);
            URIList fileURIs;
            Dialog dialog(self->m_windowHandle);
            if (dialog.open("", allowedExtensions)) {
                self->m_client->sendQueryOpenMultipleFilesDialogMessage(types, fileURIs);
            }
            else {
                self->m_client->sendQueryOpenMultipleFilesDialogMessage(types, URIList());
            }
        },
        this, false);
    m_client->addQuerySaveFileDialogEventListener(
        [](void *userData, uint32_t types, const StringList &allowedExtensions) {
            auto self = static_cast<MainWindow *>(userData);
            Dialog dialog(self->m_windowHandle);
            if (dialog.save("Available Format (*.%s)", allowedExtensions,
                    self->localizedString("nanoem.dialog.filename.untitled"))) {
                self->m_client->sendQuerySaveFileDialogMessage(types, dialog.fileURI());
            }
            else {
                self->m_client->sendQuerySaveFileDialogMessage(types, URI());
            }
        },
        this, false);
    m_client->addUpdateProgressEventListener(
        [](void *userData, uint32_t value, uint32_t total, uint32_t type, const char *text) {
            auto self = static_cast<MainWindow *>(userData);
            if (IProgressDialog *dialog = self->m_progressDialog.first) {
                dialog->SetProgress(value, total);
                if (!dialog->HasUserCancelled()) {
                    MutableWideString ws;
                    if (type == Progress::kEventTypeText) {
                        StringUtils::getWideCharString(text, ws);
                        dialog->SetLine(3, ws.data(), FALSE, nullptr);
                    }
                    else if (type == Progress::kEventTypeItem) {
                        wchar_t buffer[1024];
                        StringUtils::getWideCharString(text, ws);
                        _snwprintf_s(buffer, BX_COUNTOF(buffer), L"Loading Next Item... %s", ws.data());
                        dialog->SetLine(3, buffer, FALSE, nullptr);
                    }
                }
            }
        },
        this, false);
    m_client->addStartProgressEventListener(
        [](void *userData, const char *title, const char *text, uint32_t total) {
            auto self = static_cast<MainWindow *>(userData);
            IProgressDialog *dialog;
            if (self->openProgressDialog(dialog)) {
                MutableWideString titleWS, textWS;
                StringUtils::getWideCharString(title, titleWS);
                StringUtils::getWideCharString(text, textWS);
                dialog->SetTitle(titleWS.data());
                dialog->SetLine(1, textWS.data(), TRUE, nullptr);
            }
        },
        this, false);
    m_client->addStopProgressEventListener(
        [](void *userData) {
            auto self = static_cast<MainWindow *>(userData);
            self->closeProgressDialog();
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
            self->destroyAllWatchEffectSources();
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
MainWindow::openProgressDialog(IProgressDialog *&dialog)
{
    bool succeeded = true;
    if (m_progressDialog.second > 0) {
        m_progressDialog.second++;
        dialog = m_progressDialog.first;
    }
    else if (!m_progressDialog.first) {
        IProgressDialog *d = nullptr;
        HRESULT result = CoCreateInstance(CLSID_ProgressDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&d));
        if (result == S_OK) {
            d->StartProgressDialog(m_windowHandle, nullptr, 0, nullptr);
            m_progressDialog = tinystl::make_pair(d, 1);
            dialog = d;
        }
        else {
            succeeded = false;
        }
    }
    return succeeded;
}

void
MainWindow::closeProgressDialog()
{
    m_progressDialog.second--;
    if (m_progressDialog.second <= 0) {
        if (IProgressDialog *dialog = m_progressDialog.first) {
            dialog->StopProgressDialog();
            dialog->Release();
        }
        m_progressDialog = tinystl::make_pair(static_cast<IProgressDialog *>(nullptr), 0);
    }
}

Vector2SI32
MainWindow::virtualLogicalCursorPosition(const Vector2SI32 &value) const
{
    return isCursorHidden() ? m_virtualLogicalCursorPosition : value;
}

Vector2SI32
MainWindow::lastLogicalCursorPosition() const
{
    return m_lastLogicalCursorPosition;
}

bool
MainWindow::isCursorHidden() const
{
    return m_disabledCursorState != kDisabledCursorStateNone;
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
MainWindow::getWindowCenterPoint(LPPOINT devicePoint) const
{
    RECT clientRect;
    GetClientRect(m_windowHandle, &clientRect);
    devicePoint->x = LONG(clientRect.right * 0.5f);
    devicePoint->y = LONG(clientRect.bottom * 0.5f);
}

void
MainWindow::setCursorPosition(POINT devicePoint)
{
    ClientToScreen(m_windowHandle, &devicePoint);
    SetCursorPos(devicePoint.x, devicePoint.y);
}

void
MainWindow::centerCursor(LPPOINT devicePoint)
{
    getWindowCenterPoint(devicePoint);
    setCursorPosition(*devicePoint);
}

void
MainWindow::updateClipCursorRect()
{
    RECT rect;
    GetClientRect(m_windowHandle, &rect);
    ClientToScreen(m_windowHandle, reinterpret_cast<POINT *>(&rect.left));
    ClientToScreen(m_windowHandle, reinterpret_cast<POINT *>(&rect.right));
    ClipCursor(&rect);
}

void
MainWindow::lockCursor(LPPOINT devicePoint)
{
    updateClipCursorRect();
    SetCursor(nullptr);
    centerCursor(devicePoint);
}

void
MainWindow::disableCursor(const Vector2SI32 &logicalCursorPosition)
{
    POINT devicePoint;
    lockCursor(&devicePoint);
    setLastLogicalCursorPosition(Vector2(devicePoint.x, devicePoint.y) * invertedDevicePixelRatio());
    m_virtualLogicalCursorPosition = logicalCursorPosition;
    m_restoreHiddenLogicalCursorPosition = logicalCursorPosition;
    m_disabledCursorState = kDisabledCursorStateInitial;
}

void
MainWindow::unlockCursor(const Vector2SI32 &logicalCursorPosition)
{
    const Vector2 deviceCursorPosition(Vector2(logicalCursorPosition) * m_devicePixelRatio);
    POINT devicePoint = { LONG(deviceCursorPosition.x), LONG(deviceCursorPosition.y) };
    setCursorPosition(devicePoint);
    ClipCursor(nullptr);
    ReleaseCapture();
    SetCursor(LoadCursor(nullptr, IDC_ARROW));
}

void
MainWindow::enableCursor(const Vector2SI32 &logicalCursorPosition)
{
    Vector2SI32 position;
    if (logicalCursorPosition.x != 0 && logicalCursorPosition.y != 0) {
        position = logicalCursorPosition;
    }
    else {
        position = m_restoreHiddenLogicalCursorPosition;
    }
    unlockCursor(position);
    setLastLogicalCursorPosition(position);
    m_restoreHiddenLogicalCursorPosition = Vector2SI32();
    m_disabledCursorState = kDisabledCursorStateNone;
}

void
MainWindow::setFocus()
{
    if (m_disabledCursorResigned) {
        POINT devicePoint;
        lockCursor(&devicePoint);
        m_disabledCursorState = kDisabledCursorStateInitial;
        m_disabledCursorResigned = false;
    }
}

void
MainWindow::killFocus()
{
    if (isCursorHidden()) {
        unlockCursor(Vector2SI32(0));
        m_disabledCursorState = kDisabledCursorStateNone;
        m_disabledCursorResigned = true;
    }
}

void
MainWindow::recenterCursor()
{
    if (isCursorHidden()) {
        POINT deviceCenterPoint;
        getWindowCenterPoint(&deviceCenterPoint);
        const Vector2SI32 logicalCenterPoint(
            Vector2(deviceCenterPoint.x, deviceCenterPoint.y) * invertedDevicePixelRatio());
        if (lastLogicalCursorPosition() != logicalCenterPoint) {
            setCursorPosition(deviceCenterPoint);
            setLastLogicalCursorPosition(logicalCenterPoint);
        }
    }
}

void
MainWindow::updateDisplayFrequency()
{
    DEVMODEW dev = {};
    if (EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &dev)) {
        m_displayFrequency = dev.dmDisplayFrequency;
    }
    else {
        m_displayFrequency = 60;
    }
}

void
MainWindow::resizeWindow()
{
    RECT rect;
    GetClientRect(m_windowHandle, &rect);
    const nanoem_f32_t dpr = invertedDevicePixelRatio();
    const Vector2UI32 logicalWindowSize((rect.right - rect.left) * dpr, (rect.bottom - rect.top) * dpr);
    resizeWindow(logicalWindowSize);
}

void
MainWindow::resizeWindow(const Vector2UI32 &logicalWindowSize)
{
    if (m_logicalWindowSize != logicalWindowSize) {
        m_client->sendResizeWindowMessage(logicalWindowSize);
        if (isCursorHidden()) {
            updateClipCursorRect();
        }
        m_logicalWindowSize = logicalWindowSize;
        DrawMenuBar(m_windowHandle);
    }
}

void
MainWindow::updatePreferredFPS(const POWERBROADCAST_SETTING *settings)
{
    if (IsEqualGUID(settings->PowerSetting, GUID_ACDC_POWER_SOURCE)) {
        enablePowerSaving(*reinterpret_cast<const DWORD *>(settings->Data) != PoAc);
    }
    else if (IsEqualGUID(settings->PowerSetting, GUID_POWER_SAVING_STATUS)) {
        enablePowerSaving(*reinterpret_cast<const DWORD *>(settings->Data) != 0);
    }
    else if (IsEqualGUID(settings->PowerSetting, GUID_POWERSCHEME_PERSONALITY)) {
        const GUID *guid = reinterpret_cast<const GUID *>(settings->Data);
        enablePowerSaving(IsEqualGUID(*guid, GUID_MAX_POWER_SAVINGS) != 0);
    }
}

void
MainWindow::enablePowerSaving(bool value)
{
    m_playingThresholder.m_preferred = m_editingThresholder.m_preferred = value ? 30 : 60;
}

bool
MainWindow::isEditingDisplaySyncEnabled() const noexcept
{
    return m_preference->applicationPreference()->preferredEditingFPS() != -1;
}

URIList
MainWindow::cachedAggregateAllPlugins()
{
    if (m_cachedPluginURIs.empty()) {
        const JSON_Object *root = json_object(m_service->applicationConfiguration());
        String path(json_object_dotget_string(root, "win32.plugin.path")), basePath(path);
        basePath.append("/plugin_*");
        MutableWideString wideBasePath;
        StringUtils::getWideCharString(basePath.c_str(), wideBasePath);
        WIN32_FIND_DATAW data;
        HANDLE handle = FindFirstFileW(wideBasePath.data(), &data);
        if (handle != INVALID_HANDLE_VALUE) {
            do {
                MutableString filename;
                StringUtils::getMultiBytesString(data.cFileName, filename);
                String pluginPath(path);
                pluginPath.append(filename.data());
                const URI &fileURI = URI::createFromFilePath(pluginPath.c_str());
                if (fileURI.pathExtension() == String("dll")) {
                    m_cachedPluginURIs.push_back(fileURI);
                }
            } while (FindNextFileW(handle, &data));
            FindClose(handle);
        }
    }
    return m_cachedPluginURIs;
}

const char *
MainWindow::localizedString(const char *text) const
{
    return m_service->translator()->translate(text);
}

const wchar_t *
MainWindow::localizedWideString(const char *text) const
{
    auto it = m_localizedMessageCache.find(text);
    if (it == m_localizedMessageCache.end()) {
        MutableWideString s;
        StringUtils::getWideCharString(m_service->translator()->translate(text), s);
        it = m_localizedMessageCache.insert(tinystl::make_pair(text, s)).first;
    }
    return it->second.data();
}

void
MainWindow::writeErrorLog(int code, const char *reason, const char *recoverySuggestion)
{
#ifndef NDEBUG
    MutableWideString reasonString, recoverySuggestionString;
    StringUtils::getWideCharString(reason, reasonString);
    StringUtils::getWideCharString(recoverySuggestion, recoverySuggestionString);
    wchar_t stackBuffer[1024];
    swprintf(stackBuffer, ARRAYSIZE(stackBuffer), L"[ERROR] code=0x%x reason=\"%s\" suggestion=\"%s\"\n", code,
        reasonString.data(), recoverySuggestionString.data());
    OutputDebugStringW(stackBuffer);
#else
    BX_UNUSED_3(code, reason, recoverySuggestion);
#endif
}

void
MainWindow::loadProjectFromFile(const wchar_t *source)
{
    MutableString path;
    StringUtils::getMultiBytesString(source, path);
    FileUtils::canonicalizePathSeparator(path);
    m_client->sendLoadAllDecoderPluginsMessage(cachedAggregateAllPlugins());
    const URI fileURI(URI::createFromFilePath(path.data()));
    setTitle(fileURI);
    loadFile(fileURI, IFileManager::kDialogTypeOpenProject);
}

void
MainWindow::destroyAllWatchEffectSources()
{
    for (auto it = m_watchEffectSourceHandles.begin(), end = m_watchEffectSourceHandles.end(); it != end; ++it) {
        HANDLE handle = it->second.first;
        FindCloseChangeNotification(handle);
    }
    m_watchEffectSourceHandles.clear();
}

} /* namespace win32 */
} /* namespace nanoem */
