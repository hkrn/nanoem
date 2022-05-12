/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "Win32ThreadedApplicationService.h"

#include <ShlObj.h>
#include <d3d11.h>
#include <dxgi.h>

#include "COMInline.h"
#include "D3D11BackgroundDrawer.h"
#include "D3D11SkinDeformerFactory.h"
#include "D3D11VideoRecorder.h"

#include "WASAPIAudioPlayer.h"
#include "emapp/ApplicationPreference.h"
#include "emapp/Effect.h"
#include "emapp/EnumUtils.h"
#include "emapp/Error.h"
#include "emapp/IDebugCapture.h"
#include "emapp/IModalDialog.h"
#include "emapp/IProjectHolder.h"
#include "emapp/ITranslator.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/internal/DecoderPluginBasedBackgroundVideoRenderer.h"
#include "emapp/internal/ImGuiWindow.h"
#include "emapp/internal/OpenGLComputeShaderSkinDeformerFactory.h"
#include "emapp/internal/OpenGLTransformFeedbackSkinDeformerFactory.h"
#include "emapp/private/CommonInclude.h"

#if defined(NANOEM_ENABLE_RENDERDOC)
#include "renderdoc_app.h"
#endif /* NANOEM_ENABLE_RENDERDOC */

namespace nanoem {
namespace win32 {
namespace {

static wchar_t s_crashFilePath[MAX_PATH];
static wchar_t s_redoFilePath[MAX_PATH];

class Win32BackgroundVideoRendererProxy : public IBackgroundVideoRenderer {
public:
    Win32BackgroundVideoRendererProxy(Win32ThreadedApplicationService *service);
    ~Win32BackgroundVideoRendererProxy() noexcept override;

    bool load(const URI &fileURI, Error &error) override;
    void draw(sg_pass pass, const Vector4 &rect, nanoem_f32_t scaleFactor, Project *project) override;
    void seek(nanoem_f64_t value) override;
    void flush() override;
    void destroy() override;
    URI fileURI() const noexcept override;

private:
    Win32ThreadedApplicationService *m_service;
    D3D11BackgroundVideoDrawer *m_d3d11BackgroundVideoDrawer = nullptr;
    internal::DecoderPluginBasedBackgroundVideoRenderer *m_decoderPluginBasedBackgroundVideoRenderer = nullptr;
    bool m_durationUpdated = false;
};

Win32BackgroundVideoRendererProxy::Win32BackgroundVideoRendererProxy(Win32ThreadedApplicationService *service)
    : m_service(service)
{
}

Win32BackgroundVideoRendererProxy::~Win32BackgroundVideoRendererProxy() noexcept
{
}

bool
Win32BackgroundVideoRendererProxy::load(const URI &fileURI, Error &error)
{
    bool playable = false;
#if WINVER >= _WIN32_WINNT_WIN7
    sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_D3D11) {
        m_d3d11BackgroundVideoDrawer = nanoem_new(D3D11BackgroundVideoDrawer(m_service));
        playable = m_d3d11BackgroundVideoDrawer->load(fileURI, error);
        if (!playable) {
            m_d3d11BackgroundVideoDrawer->destroy();
            nanoem_delete_safe(m_d3d11BackgroundVideoDrawer);
        }
    }
#else
    m_decoderPluginBasedBackgroundVideoRenderer =
        nanoem_new(internal::DecoderPluginBasedBackgroundVideoRenderer(m_service->defaultFileManager()));
    playable = m_decoderPluginBasedBackgroundVideoRenderer->load(fileURI, error);
    if (!playable) {
        m_decoderPluginBasedBackgroundVideoRenderer->destroy();
        nanoem_delete_safe(m_decoderPluginBasedBackgroundVideoRenderer);
    }
#endif /* WINVER >= _WIN32_WINNT_WIN7 */
    return playable;
}

void
Win32BackgroundVideoRendererProxy::draw(sg_pass pass, const Vector4 &rect, nanoem_f32_t scaleFactor, Project *project)
{
    if (m_d3d11BackgroundVideoDrawer) {
        m_d3d11BackgroundVideoDrawer->draw(pass, rect, scaleFactor, project);
    }
    else if (m_decoderPluginBasedBackgroundVideoRenderer) {
        m_decoderPluginBasedBackgroundVideoRenderer->draw(pass, rect, scaleFactor, project);
    }
}

void
Win32BackgroundVideoRendererProxy::seek(nanoem_f64_t value)
{
    if (m_d3d11BackgroundVideoDrawer) {
        m_d3d11BackgroundVideoDrawer->seek(value);
    }
    else if (m_decoderPluginBasedBackgroundVideoRenderer) {
        m_decoderPluginBasedBackgroundVideoRenderer->seek(value);
    }
}

void
Win32BackgroundVideoRendererProxy::flush()
{
    if (m_d3d11BackgroundVideoDrawer) {
        m_d3d11BackgroundVideoDrawer->flush();
    }
    else if (m_decoderPluginBasedBackgroundVideoRenderer) {
        m_decoderPluginBasedBackgroundVideoRenderer->flush();
    }
}

void
Win32BackgroundVideoRendererProxy::destroy()
{
    if (m_d3d11BackgroundVideoDrawer) {
        m_d3d11BackgroundVideoDrawer->destroy();
        nanoem_delete_safe(m_d3d11BackgroundVideoDrawer);
    }
    if (m_decoderPluginBasedBackgroundVideoRenderer) {
        m_decoderPluginBasedBackgroundVideoRenderer->destroy();
        nanoem_delete_safe(m_decoderPluginBasedBackgroundVideoRenderer);
    }
    m_durationUpdated = false;
}

URI
Win32BackgroundVideoRendererProxy::fileURI() const noexcept
{
    URI fileURI;
    if (m_d3d11BackgroundVideoDrawer) {
        fileURI = m_d3d11BackgroundVideoDrawer->fileURI();
    }
    else if (m_decoderPluginBasedBackgroundVideoRenderer) {
        fileURI = m_decoderPluginBasedBackgroundVideoRenderer->fileURI();
    }
    return fileURI;
}

struct D3D11RendererCapability : Project::IRendererCapability {
    D3D11RendererCapability(ID3D11Device *device);
    ~D3D11RendererCapability();
    nanoem_u32_t suggestedSampleLevel(nanoem_u32_t value) const noexcept override;
    bool supportsSampleLevel(nanoem_u32_t value) const noexcept override;

    ID3D11Device *m_device;
};

D3D11RendererCapability::D3D11RendererCapability(ID3D11Device *device)
    : m_device(device)
{
}

D3D11RendererCapability::~D3D11RendererCapability()
{
}

nanoem_u32_t
D3D11RendererCapability::suggestedSampleLevel(nanoem_u32_t value) const noexcept
{
    uint32_t sampleCount = 1 << value, numQualityLevels;
    while (
        FAILED(m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_B8G8R8A8_UNORM, sampleCount, &numQualityLevels)) ||
        numQualityLevels == 0) {
        value--;
        sampleCount = 1 << value;
    }
    return value;
}

bool
D3D11RendererCapability::supportsSampleLevel(nanoem_u32_t value) const noexcept
{
    uint32_t sampleCount = 1 << value, numQualityLevels;
    HRESULT result =
        m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_B8G8R8A8_UNORM, sampleCount, &numQualityLevels);
    return !FAILED(result) && numQualityLevels > 0;
}

#if defined(NANOEM_ENABLE_RENDERDOC)
class RenderDocDebugCapture : public IDebugCapture {
public:
    RenderDocDebugCapture();

    void start(const char *label) override;
    void stop() override;

private:
    RENDERDOC_API_1_0_0 *m_renderDocInstance = nullptr;
};

RenderDocDebugCapture::RenderDocDebugCapture()
{
    if (HMODULE mod = GetModuleHandleA("renderdoc.dll")) {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI) GetProcAddress(mod, "RENDERDOC_GetAPI");
        int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_0_0, (void **) &m_renderDocInstance);
        assert(ret == 1);
    }
}

void
RenderDocDebugCapture::start(const char *label)
{
    if (m_renderDocInstance) {
        m_renderDocInstance->SetCaptureOptionU32(eRENDERDOC_Option_APIValidation, 1);
        m_renderDocInstance->SetCaptureOptionU32(eRENDERDOC_Option_CaptureCallstacksOnlyDraws, 1);
        m_renderDocInstance->SetCaptureOptionU32(eRENDERDOC_Option_VerifyBufferAccess, 1);
        m_renderDocInstance->StartFrameCapture(nullptr, nullptr);
    }
}

void
RenderDocDebugCapture::stop()
{
    if (m_renderDocInstance) {
        m_renderDocInstance->EndFrameCapture(nullptr, nullptr);
    }
}
#endif /* NANOEM_ENABLE_RENDERDOC */

} /* namespace anonymous */

const nanoem_f32_t Win32ThreadedApplicationService::kStandardDPIValue = 96.0f;
const wchar_t Win32ThreadedApplicationService::kRegisterClassName[] = L"nanoem";

#if defined(IMGUI_HAS_VIEWPORT)

Win32ThreadedApplicationService::ViewportData::ViewportData()
{
    m_eventHandle = CreateEventW(nullptr, true, false, L"Win32ThreadedApplicationService::ViewportData::m_eventHandle");
}

Win32ThreadedApplicationService::ViewportData::~ViewportData()
{
    CloseHandle(m_eventHandle);
}

void
Win32ThreadedApplicationService::ViewportData::destroyWindow()
{
    if (GetCapture() == m_windowHandle) {
        ReleaseCapture();
        SetCapture(static_cast<HWND>(ImGui::GetMainViewport()->PlatformHandle));
    }
    if (m_windowHandleOwned) {
        DestroyWindow(m_windowHandle);
        m_windowHandle = nullptr;
    }
}

void
Win32ThreadedApplicationService::ViewportData::takeWindowStyle(ImGuiViewportFlags flags)
{
    getWindowStyle(flags, m_style, m_styleEx);
}

void
Win32ThreadedApplicationService::ViewportData::wait()
{
    static const DWORD kWaitEventTimeout = 60000;
    WaitForSingleObject(m_eventHandle, kWaitEventTimeout);
    ResetEvent(m_eventHandle);
}

void
Win32ThreadedApplicationService::ViewportData::signal()
{
    SetEvent(m_eventHandle);
}

LONG
Win32ThreadedApplicationService::ViewportData::width(const RECT &rect) noexcept
{
    return rect.right - rect.left;
}

LONG
Win32ThreadedApplicationService::ViewportData::height(const RECT &rect) noexcept
{
    return rect.bottom - rect.top;
}

ImVec2
Win32ThreadedApplicationService::ViewportData::size(const RECT &rect) noexcept
{
    return ImVec2 { static_cast<float>(width(rect)), static_cast<float>(height(rect)) };
}

RECT
Win32ThreadedApplicationService::ViewportData::rect(const ImGuiViewport *viewport) noexcept
{
    return RECT { static_cast<LONG>(viewport->Pos.x), static_cast<LONG>(viewport->Pos.y),
        static_cast<LONG>(viewport->Pos.x + viewport->Size.x), static_cast<LONG>(viewport->Pos.y + viewport->Size.y) };
}

void
Win32ThreadedApplicationService::ViewportData::getWindowStyle(
    ImGuiViewportFlags flags, DWORD &style, DWORD &styleEx) noexcept
{
    style = EnumUtils::isEnabledT<ImGuiViewportFlags>(flags, ImGuiViewportFlags_NoDecoration) ? WS_POPUP
                                                                                              : WS_OVERLAPPEDWINDOW;
    styleEx = EnumUtils::isEnabledT<ImGuiViewportFlags>(flags, ImGuiViewportFlags_NoTaskBarIcon) ? WS_EX_TOOLWINDOW
                                                                                                 : WS_EX_APPWINDOW;
    if (EnumUtils::isEnabledT<ImGuiViewportFlags>(flags, ImGuiViewportFlags_TopMost)) {
        styleEx |= WS_EX_TOPMOST;
    }
}

void
Win32ThreadedApplicationService::ViewportData::sendMessage(MessageType type, const void *input) noexcept
{
    HWND windowHandle = static_cast<HWND>(ImGui::GetMainViewport()->PlatformHandleRaw);
    SendMessageW(windowHandle, type, reinterpret_cast<WPARAM>(input), 0);
}

void
Win32ThreadedApplicationService::ViewportData::sendMessage(MessageType type, const void *input, void *output) noexcept
{
    HWND windowHandle = static_cast<HWND>(ImGui::GetMainViewport()->PlatformHandleRaw);
    SendMessageW(windowHandle, type, reinterpret_cast<WPARAM>(input), reinterpret_cast<LPARAM>(output));
}

void
Win32ThreadedApplicationService::ViewportData::sendMessage(
    MessageType type, const void *input, const void *output) noexcept
{
    HWND windowHandle = static_cast<HWND>(ImGui::GetMainViewport()->PlatformHandleRaw);
    SendMessageW(windowHandle, type, reinterpret_cast<WPARAM>(input), reinterpret_cast<LPARAM>(output));
}

#endif /* IMGUI_HAS_VIEWPORT */

void
Win32ThreadedApplicationService::installUnhandledExceptionHandler()
{
    SetUnhandledExceptionFilter([](LPEXCEPTION_POINTERS /* eip */) -> LONG {
        HANDLE file =
            CreateFileW(s_crashFilePath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file != INVALID_HANDLE_VALUE) {
            size_t length = wcsnlen_s(s_redoFilePath, ARRAYSIZE(s_redoFilePath)) * sizeof(s_redoFilePath[0]);
            WriteFile(file, s_redoFilePath, Inline::saturateInt32U(length), nullptr, nullptr);
            CloseHandle(file);
        }
        return EXCEPTION_CONTINUE_SEARCH;
    });
}

void
Win32ThreadedApplicationService::getPluginPath(const wchar_t *executablePath, wchar_t *path, DWORD size)
{
    wchar_t *p = path;
    wcscpy_s(path, size, executablePath);
    while (*p) {
        if (*p == L'\\') {
            *p = L'/';
        }
        p++;
    }
    if (wchar_t *ptr = wcsrchr(path, L'/')) {
        size_t rest = size - (ptr - path);
        wcscpy_s(ptr, rest, L"/plugins/");
    }
}

const wchar_t *
Win32ThreadedApplicationService::sharedRedoFilePath()
{
    return s_redoFilePath;
}

nanoem_f32_t
Win32ThreadedApplicationService::calculateDevicePixelRatio()
{
    const POINT point = { 0, 0 };
    HMONITOR monitor = MonitorFromPoint(point, MONITOR_DEFAULTTOPRIMARY);
    return calculateDevicePixelRatio(monitor);
}

nanoem_f32_t
Win32ThreadedApplicationService::calculateDevicePixelRatio(HMONITOR monitor)
{
    float devicePixelRatio = 1.0f;
    if (HMODULE shcore = LoadLibraryA("shcore.dll")) {
        UINT dpiX, dpiY;
        typedef HRESULT(WINAPI * pfn_GetDpiForMonitor)(HMONITOR, int, UINT *, UINT *);
        if (auto GetDpiForMonitor =
                reinterpret_cast<pfn_GetDpiForMonitor>(GetProcAddress(shcore, "GetDpiForMonitor"))) {
            GetDpiForMonitor(monitor, 0 /* MDT_EFFECTIVE_DPI */, &dpiX, &dpiY);
            devicePixelRatio = dpiX / kStandardDPIValue;
        }
        FreeLibrary(shcore);
    }
    return devicePixelRatio;
}

JSON_Value *
Win32ThreadedApplicationService::loadJSONConfig(const wchar_t *executablePath)
{
    static const wchar_t kConfigPathSuffix[] = L"\\config.json";
    wchar_t configPath[MAX_PATH];
    if (const wchar_t *p = wcsrchr(executablePath, L'\\')) {
        wcsncpy_s(configPath, ARRAYSIZE(configPath), executablePath, p - executablePath);
        wcscat_s(configPath, ARRAYSIZE(configPath), kConfigPathSuffix);
    }
    MutableString newConfigPath, roamingAppDataPath;
#if (WINVER >= _WIN32_WINNT_WIN7)
    PWSTR wideRoamingAppDataPath;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &wideRoamingAppDataPath);
    StringUtils::getMultiBytesString(wideRoamingAppDataPath, roamingAppDataPath);
    CoTaskMemFree(wideRoamingAppDataPath);
#else
    wchar_t wideRoamingAppDataPath[MAX_PATH];
    SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, wideRoamingAppDataPath);
    StringUtils::getMultiBytesString(wideRoamingAppDataPath, roamingAppDataPath);
#endif
    StringUtils::getMultiBytesString(configPath, newConfigPath);
    const String basePath(newConfigPath.data(), newConfigPath.size() - ARRAYSIZE(kConfigPathSuffix));
    String newBasePath;
    FileUtils::canonicalizePathSeparator(basePath.c_str(), newBasePath);
    String newRoamingAppDataPath;
    FileUtils::canonicalizePathSeparator(roamingAppDataPath.data(), newRoamingAppDataPath);
    newRoamingAppDataPath.append("/nanoem");
    CreateDirectoryA(newRoamingAppDataPath.c_str(), nullptr);
    String redoDirectory(newRoamingAppDataPath);
    redoDirectory.append("/redo");
    CreateDirectoryA(redoDirectory.c_str(), nullptr);
    SetLastError(0);
    String tempDirectory(newRoamingAppDataPath);
    tempDirectory.append("/tmp");
    CreateDirectoryA(tempDirectory.c_str(), nullptr);
    SetLastError(0);
    JSON_Value *config = json_parse_file_with_comments(newConfigPath.data());
    JSON_Object *root = nullptr;
    if (config) {
        root = json_object(config);
    }
    else {
        config = json_value_init_object();
        root = json_object(config);
        json_object_dotset_string(root, "project.home", newBasePath.c_str());
    }
    String globalPreferencePath(newRoamingAppDataPath);
    globalPreferencePath.append("/nanoem.ini");
    json_object_dotset_boolean(root, "project.compute.enabled", 0);
    json_object_dotset_boolean(root, "project.grid.visible", 1);
    json_object_dotset_number(root, "project.grid.cell.x", 5);
    json_object_dotset_number(root, "project.grid.cell.y", 5);
    json_object_dotset_number(root, "project.grid.opacity", 1);
    json_object_dotset_number(root, "project.screen.sample", 0);
    json_object_dotset_string(root, "project.tmp.path", tempDirectory.c_str());
    wchar_t pluginPath[MAX_PATH];
    MutableString newPluginPath;
    getPluginPath(executablePath, pluginPath, ARRAYSIZE(pluginPath));
    SetDllDirectoryW(pluginPath);
    StringUtils::getMultiBytesString(pluginPath, newPluginPath);
    json_object_dotset_string(root, "win32.preference.path", globalPreferencePath.c_str());
    json_object_dotset_string(root, "win32.plugin.path", newPluginPath.data());
    json_object_dotset_string(root, "win32.redo.path", redoDirectory.c_str());
    String handlerPath(newPluginPath.data(), newPluginPath.size() - 1), databasePath(newRoamingAppDataPath);
    handlerPath.append("crashpad_handler.exe");
    databasePath.append("/sentry-db");
    json_object_dotset_string(root, "win32.sentry.handler.path", handlerPath.c_str());
    json_object_dotset_string(root, "win32.sentry.database.path", databasePath.c_str());
    char language[8], country[8], identity[17];
    GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, country, sizeof(country));
    GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, language, sizeof(language));
    bx::snprintf(identity, sizeof(identity), "%s_%s", language, country);
    json_object_dotset_string(root, "project.locale", identity);
    return config;
}

Win32ThreadedApplicationService::Win32ThreadedApplicationService(const JSON_Value *config)
    : ThreadedApplicationService(config)
    , m_requestWindowClose(nullptr)
    , m_requestWindowMove(nullptr)
    , m_requestWindowResize(nullptr)
    , m_displaySyncInterval(1)
{
    const char *storagePath = json_object_dotget_string(json_object(applicationConfiguration()), "win32.redo.path");
    MutableWideString ws;
    StringUtils::getWideCharString(storagePath, ws);
    const wchar_t filename[] = L"/CRASH";
    ws.insert(ws.end() - 1, filename, filename + ARRAYSIZE(filename));
    wcscpy_s(s_crashFilePath, ARRAYSIZE(s_crashFilePath), ws.data());
}

Win32ThreadedApplicationService::~Win32ThreadedApplicationService()
{
}

int
Win32ThreadedApplicationService::run()
{
    int exitCode = -1;
    if (!FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) {
        exitCode = ThreadedApplicationService::run();
        CoUninitialize();
    }
    return exitCode;
}

void
Win32ThreadedApplicationService::setDisplaySyncEnabled(bool value)
{
    const UINT interval = value ? 1 : 0;
    m_displaySyncInterval = interval;
}

void
Win32ThreadedApplicationService::requestUpdatingAllMonitors()
{
    m_requestUpdatingAllMonitors = true;
}

void
Win32ThreadedApplicationService::requestViewportWindowClose(HWND window)
{
    m_requestWindowClose = window;
}

void
Win32ThreadedApplicationService::requestViewportWindowMove(HWND window)
{
    m_requestWindowMove = window;
}

void
Win32ThreadedApplicationService::requestViewportWindowResize(HWND window)
{
    m_requestWindowResize = window;
}

IAudioPlayer *
Win32ThreadedApplicationService::createAudioPlayer()
{
    return nanoem_new(WASAPIAudioPlayer(eventPublisher()));
}

IBackgroundVideoRenderer *
Win32ThreadedApplicationService::createBackgroundVideoRenderer()
{
    return nanoem_new(Win32BackgroundVideoRendererProxy(this));
}

IDebugCapture *
Win32ThreadedApplicationService::createDebugCapture()
{
#if defined(NANOEM_ENABLE_RENDERDOC)
    return nanoem_new(RenderDocDebugCapture);
#else
    return nullptr;
#endif /* NANOEM_ENABLE_RENDERDOC */
}

Project::IRendererCapability *
Win32ThreadedApplicationService::createRendererCapability()
{
    const sg_backend backend = sg::query_backend();
    return backend == SG_BACKEND_D3D11 ? nanoem_new(D3D11RendererCapability((ID3D11Device *) m_nativeDevice))
                                       : ThreadedApplicationService::createRendererCapability();
}

Project::ISkinDeformerFactory *
Win32ThreadedApplicationService::createSkinDeformerFactory()
{
    Project::ISkinDeformerFactory *factory = nullptr;
    ApplicationPreference preference(this);
    if (preference.isSkinDeformAcceleratorEnabled()) {
        sg_backend backend = sg::query_backend();
        if (backend == SG_BACKEND_D3D11) {
            auto device = (ID3D11Device *) m_nativeDevice;
            auto context = (ID3D11DeviceContext *) m_nativeContext;
            D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS options = {};
            HRESULT rc = device->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &options, sizeof(options));
            if (rc == S_OK && options.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x) {
                factory = nanoem_new(D3D11SkinDeformerFactory(device, context));
            }
        }
        else if (backend == SG_BACKEND_GLCORE33 || backend == SG_BACKEND_GLES3) {
            if (wglGetProcAddress("glDispatchCompute")) {
                factory = nanoem_new(internal::OpenGLComputeShaderSkinDeformerFactory(
                    reinterpret_cast<internal::OpenGLComputeShaderSkinDeformerFactory::PFN_GetProcAddress>(
                        wglGetProcAddress)));
            }
            else if (wglGetProcAddress("glTransformFeedbackVaryings")) {
                factory = nanoem_new(internal::OpenGLTransformFeedbackSkinDeformerFactory(
                    reinterpret_cast<internal::OpenGLTransformFeedbackSkinDeformerFactory::PFN_GetProcAddress>(
                        wglGetProcAddress)));
            }
        }
    }
    return factory;
}

bool
Win32ThreadedApplicationService::hasVideoRecorder() const noexcept
{
    bool available = false;
#if WINVER >= _WIN32_WINNT_WIN7
    if (HMODULE mfplat = LoadLibraryA("mfplat.dll")) {
        if (GetProcAddress(mfplat, "MFCreateDXGISurfaceBuffer")) {
            sg_backend backend = sg::query_backend();
            available = backend == SG_BACKEND_D3D11;
        }
        FreeLibrary(mfplat);
    }
#endif
    return available;
}

bool
Win32ThreadedApplicationService::isRendererAvailable(const char *value) const noexcept
{
    return StringUtils::equals(value, kRendererOpenGL) || StringUtils::equals(value, kRendererDirectX);
}

IVideoRecorder *
Win32ThreadedApplicationService::createVideoRecorder()
{
    IVideoRecorder *videoRecorder = nullptr;
#if WINVER >= _WIN32_WINNT_WIN7
    if (!videoRecorder) {
        Project *project = projectHolder()->currentProject();
        sg_backend backend = sg::query_backend();
        if (backend == SG_BACKEND_D3D11) {
            auto device = (ID3D11Device *) m_nativeDevice;
            videoRecorder = nanoem_new(D3D11VideoRecorder(project, device));
        }
    }
#endif /* WINVER >= _WIN32_WINNT_WIN7 */
    return videoRecorder;
}

void
Win32ThreadedApplicationService::destroyVideoRecorder(IVideoRecorder *videoRecorder)
{
    nanoem_delete(videoRecorder);
}

void
Win32ThreadedApplicationService::handleSetupGraphicsEngine(sg_desc &desc)
{
    ThreadedApplicationService::handleSetupGraphicsEngine(desc);
    if (m_nativeSwapChain && m_nativeSwapChainDescription) {
        auto &d3d11 = desc.context.d3d11;
        d3d11.device = m_nativeDevice;
        d3d11.device_context = m_nativeContext;
        d3d11.depth_stencil_view_userdata_cb = [](void *userData) {
            auto self = static_cast<Win32ThreadedApplicationService *>(userData);
            return reinterpret_cast<const void *>(self->m_d3d11DepthStencilView);
        };
        d3d11.render_target_view_userdata_cb = [](void *userData) {
            auto self = static_cast<Win32ThreadedApplicationService *>(userData);
            return reinterpret_cast<const void *>(self->m_d3d11RenderTargetView);
        };
        d3d11.user_data = this;
    }
#if defined(NANOEM_WIN32_HAS_OPENGL)
    else {
        auto context = (HGLRC) m_nativeContext;
        auto device = (HDC) m_nativeDevice;
        wglMakeCurrent(device, context);
    }
#endif /* NANOEM_WIN32_HAS_OPENGL */
}

void
Win32ThreadedApplicationService::handleInitializeApplication()
{
    ThreadedApplicationService::handleInitializeApplication();
    setupNewProject();
    HWND window = (HWND) m_nativeView;
#if defined(IMGUI_HAS_VIEWPORT)
    ImGuiIO &io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
    ImGuiPlatformIO &platformIO = ImGui::GetPlatformIO();
    platformIO.Platform_CreateWindow = [](ImGuiViewport *viewport) {
        Win32ThreadedApplicationService::ViewportData *userData = IM_NEW(Win32ThreadedApplicationService::ViewportData);
        viewport->PlatformUserData = userData;
        ViewportData::sendMessage(ViewportData::kMessageTypeCreateWindow, viewport, &userData);
        userData->wait();
        viewport->PlatformHandle = viewport->PlatformHandleRaw = userData->m_windowHandle;
        viewport->PlatformRequestResize = false;
    };
    platformIO.Platform_DestroyWindow = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportData *>(viewport->PlatformUserData);
        ViewportData::sendMessage(ViewportData::kMessageTypeDestroyWindow, viewport);
        userData->wait();
        IM_DELETE(userData);
        viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
    };
    platformIO.Platform_ShowWindow = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportData *>(viewport->PlatformUserData);
        ViewportData::sendMessage(ViewportData::kMessageTypeShowWindow, viewport);
        userData->wait();
    };
    platformIO.Platform_SetWindowPos = [](ImGuiViewport *viewport, ImVec2 pos) {
        auto userData = static_cast<ViewportData *>(viewport->PlatformUserData);
        ViewportData::sendMessage(ViewportData::kMessageTypeSetWindowPos, viewport, &pos);
        userData->wait();
    };
    platformIO.Platform_GetWindowPos = [](ImGuiViewport *viewport) {
        ImVec2 pos;
        auto userData = static_cast<ViewportData *>(viewport->PlatformUserData);
        ViewportData::sendMessage(ViewportData::kMessageTypeGetWindowPos, viewport, &pos);
        userData->wait();
        return pos;
    };
    platformIO.Platform_SetWindowSize = [](ImGuiViewport *viewport, ImVec2 size) {
        auto userData = static_cast<ViewportData *>(viewport->PlatformUserData);
        ViewportData::sendMessage(ViewportData::kMessageTypeSetWindowSize, viewport, &size);
        userData->wait();
    };
    platformIO.Platform_GetWindowSize = [](ImGuiViewport *viewport) {
        ImVec2 size;
        auto userData = static_cast<ViewportData *>(viewport->PlatformUserData);
        ViewportData::sendMessage(ViewportData::kMessageTypeGetWindowSize, viewport, &size);
        userData->wait();
        return size;
    };
    platformIO.Platform_SetWindowFocus = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportData *>(viewport->PlatformUserData);
        ViewportData::sendMessage(ViewportData::kMessageTypeSetWindowFocus, viewport);
        userData->wait();
    };
    platformIO.Platform_GetWindowFocus = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportData *>(viewport->PlatformUserData);
        bool focused = false;
        ViewportData::sendMessage(ViewportData::kMessageTypeGetWindowFocus, viewport, &focused);
        userData->wait();
        return focused;
    };
    platformIO.Platform_GetWindowMinimized = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportData *>(viewport->PlatformUserData);
        bool minimized = false;
        ViewportData::sendMessage(ViewportData::kMessageTypeGetWindowMinimized, viewport, &minimized);
        userData->wait();
        return minimized;
    };
    platformIO.Platform_SetWindowTitle = [](ImGuiViewport *viewport, const char *title) {
        auto userData = static_cast<ViewportData *>(viewport->PlatformUserData);
        ViewportData::sendMessage(ViewportData::kMessageTypeSetWindowTitle, viewport, title);
        userData->wait();
    };
    platformIO.Platform_SetWindowAlpha = [](ImGuiViewport *viewport, float alpha) {
        auto userData = static_cast<ViewportData *>(viewport->PlatformUserData);
        ViewportData::sendMessage(ViewportData::kMessageTypeSetWindowAlpha, viewport, &alpha);
        userData->wait();
    };
    platformIO.Platform_UpdateWindow = [](ImGuiViewport *viewport) {
        bool updated = false;
        auto userData = static_cast<ViewportData *>(viewport->PlatformUserData);
        ViewportData::sendMessage(ViewportData::kMessageTypeUpdateWindow, viewport, &updated);
        userData->wait();
        viewport->PlatformRequestMove = viewport->PlatformRequestResize = updated;
    };
    platformIO.Platform_GetWindowDpiScale = [](ImGuiViewport *viewport) {
        float dpiScale = 1.0f;
        auto userData = static_cast<ViewportData *>(viewport->PlatformUserData);
        ViewportData::sendMessage(ViewportData::kMessageTypeGetDpiScale, viewport, &dpiScale);
        userData->wait();
        return dpiScale;
    };
    platformIO.Platform_OnChangedViewport = [](ImGuiViewport *viewport) {};
    platformIO.Platform_SetImeInputPos = [](ImGuiViewport *viewport, ImVec2 pos) {
        auto userData = static_cast<ViewportData *>(viewport->PlatformUserData);
        ViewportData::sendMessage(ViewportData::kMessageTypeSetIMEInputPos, viewport, &pos);
        userData->wait();
    };
    ImGuiViewport *main = ImGui::GetMainViewport();
    main->PlatformHandle = main->PlatformHandleRaw = window;
    ViewportData *data = IM_NEW(ViewportData);
    data->m_windowHandle = window;
    main->PlatformUserData = data;
    sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_D3D11) {
        struct D3D11RendererData {
            D3D11RendererData(ID3D11Device *device)
                : m_device(device)
            {
                m_device->AddRef();
            }
            ~D3D11RendererData()
            {
                releaseRenderTargetView();
                releaseDepthStencilView();
                releaseSwapChain();
                COMInline::safeRelease(m_device);
            }
            void
            createRenderTargetView()
            {
                ID3D11Texture2D *colorBackBuffer = NULL;
                if (!FAILED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&colorBackBuffer)))) {
                    m_device->CreateRenderTargetView(colorBackBuffer, nullptr, &m_renderTargetView);
                    colorBackBuffer->Release();
                }
            }
            void
            createDepthStencilView(UINT width, UINT height)
            {
                D3D11_TEXTURE2D_DESC td = {};
                td.Width = width;
                td.Height = height;
                td.MipLevels = td.ArraySize = 1;
                td.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
                td.SampleDesc.Count = 1;
                td.Usage = D3D11_USAGE_DEFAULT;
                td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
                D3D11_DEPTH_STENCIL_VIEW_DESC dsvd = {};
                dsvd.Format = td.Format;
                dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                ID3D11Texture2D *depthStencilBackBuffer;
                if (!FAILED(m_device->CreateTexture2D(&td, nullptr, &depthStencilBackBuffer))) {
                    m_device->CreateDepthStencilView(depthStencilBackBuffer, nullptr, &m_depthStencilView);
                    depthStencilBackBuffer->Release();
                }
            }
            bool
            createSwapChain(HWND windowHandle, const ImVec2 &size)
            {
                DXGI_SWAP_CHAIN_DESC sd = {};
                sd.BufferCount = 2;
                sd.BufferDesc.Width = static_cast<UINT>(size.x);
                sd.BufferDesc.Height = static_cast<UINT>(size.y);
                sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                sd.SampleDesc.Count = 1;
                sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                sd.OutputWindow = windowHandle;
                sd.Windowed = TRUE;
                sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
                IDXGIDevice *deviceDXGI = nullptr;
                m_device->QueryInterface(IID_PPV_ARGS(&deviceDXGI));
                IDXGIAdapter *adapter = nullptr;
                deviceDXGI->GetParent(IID_PPV_ARGS(&adapter));
                IDXGIFactory *factory = nullptr;
                adapter->GetParent(IID_PPV_ARGS(&factory));
                bool result = false;
                if (factory->CreateSwapChain(m_device, &sd, &m_swapChain) >= 0) {
                    createRenderTargetView();
                    createDepthStencilView(sd.BufferDesc.Width, sd.BufferDesc.Height);
                    result = true;
                }
                return result;
            }
            void
            resizeSwapChain(UINT width, UINT height)
            {
                releaseRenderTargetView();
                releaseDepthStencilView();
                m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
                createRenderTargetView();
                createDepthStencilView(width, height);
            }
            void
            releaseSwapChain()
            {
                COMInline::safeRelease(m_swapChain);
            }
            void
            releaseDepthStencilView()
            {
                COMInline::safeRelease(m_depthStencilView);
            }
            void
            releaseRenderTargetView()
            {
                COMInline::safeRelease(m_renderTargetView);
            }
            ID3D11Device *m_device = nullptr;
            IDXGISwapChain *m_swapChain = nullptr;
            ID3D11RenderTargetView *m_renderTargetView = nullptr;
            ID3D11DepthStencilView *m_depthStencilView = nullptr;
        };
        io.BackendRendererUserData = this;
        platformIO.Renderer_CreateWindow = [](ImGuiViewport *viewport) {
            auto self = static_cast<Win32ThreadedApplicationService *>(ImGui::GetIO().BackendRendererUserData);
            ID3D11Device *device = (ID3D11Device *) self->m_nativeDevice;
            D3D11RendererData *userData = IM_NEW(D3D11RendererData(device));
            HWND windowHandle = static_cast<HWND>(viewport->PlatformHandleRaw);
            if (userData->createSwapChain(windowHandle, viewport->Size)) {
                viewport->RendererUserData = userData;
            }
            else {
                IM_DELETE(userData);
            }
        };
        platformIO.Renderer_DestroyWindow = [](ImGuiViewport *viewport) {
            if (auto userData = static_cast<D3D11RendererData *>(viewport->RendererUserData)) {
                IM_DELETE(userData);
            }
            viewport->RendererUserData = nullptr;
        };
        platformIO.Renderer_RenderWindow = [](ImGuiViewport *viewport, void *opaque) {
            if (auto userData = static_cast<D3D11RendererData *>(viewport->RendererUserData)) {
                auto self = static_cast<Win32ThreadedApplicationService *>(ImGui::GetIO().BackendRendererUserData);
                auto context = (ID3D11DeviceContext *) self->m_nativeContext;
                if (Project *project = self->projectHolder()->currentProject()) {
                    bool load =
                        EnumUtils::isEnabledT<ImGuiViewportFlags>(viewport->Flags, ImGuiViewportFlags_NoRendererClear);
                    auto window = static_cast<internal::ImGuiWindow *>(opaque);
                    auto context = (ID3D11DeviceContext *) self->m_nativeContext;
                    auto dsv = self->m_d3d11DepthStencilView;
                    auto rtv = self->m_d3d11RenderTargetView;
                    self->m_d3d11DepthStencilView = userData->m_depthStencilView;
                    self->m_d3d11RenderTargetView = userData->m_renderTargetView;
                    window->drawWindow(project, viewport->DrawData, load);
                    self->m_d3d11DepthStencilView = dsv;
                    self->m_d3d11RenderTargetView = rtv;
                }
            }
        };
        platformIO.Renderer_SetWindowSize = [](ImGuiViewport *viewport, ImVec2 size) {
            if (auto userData = static_cast<D3D11RendererData *>(viewport->RendererUserData)) {
                userData->resizeSwapChain(size.x, size.y);
            }
        };
        platformIO.Renderer_SwapBuffers = [](ImGuiViewport *viewport, void *) {
            if (auto userData = static_cast<D3D11RendererData *>(viewport->RendererUserData)) {
                userData->m_swapChain->Present(0, 0);
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
#else
    ImGui::GetMainViewport()->PlatformHandleRaw = window;
#endif /* IMGUI_HAS_VIEWPORT */
}

void
Win32ThreadedApplicationService::handleNewProject()
{
    ThreadedApplicationService::handleNewProject();
    Effect::terminateD3DCompiler();
    setupNewProject();
}

void
Win32ThreadedApplicationService::handleTerminateApplication()
{
    ThreadedApplicationService::handleTerminateApplication();
    MutableString filePath;
    StringUtils::getMultiBytesString(s_crashFilePath, filePath);
    FileUtils::deleteFile(filePath.data());
    destroyDefaultRenderTarget();
}

void
Win32ThreadedApplicationService::postEmptyApplicationEvent()
{
    HWND window = (HWND) m_nativeView;
    PostMessageW(window, WM_NULL, 0, 0);
}

void
Win32ThreadedApplicationService::presentDefaultPass(const Project * /* project */)
{
    SG_PUSH_GROUP("win32::Win32ThreadedApplicationService::presentDefaultPass");
    HWND window = (HWND) m_nativeView;
    sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_D3D11) {
        auto swapChain = (IDXGISwapChain *) m_nativeSwapChain;
        swapChain->Present(m_displaySyncInterval, 0);
    }
#if defined(NANOEM_WIN32_HAS_OPENGL)
    else if (backend == SG_BACKEND_GLCORE33) {
        auto device = (HDC) m_nativeDevice;
        auto context = (HGLRC) m_nativeContext;
        wglMakeCurrent(device, context);
        SwapBuffers(device);
    }
#endif /* NANOEM_WIN32_HAS_OPENGL */
    if (m_requestUpdatingAllMonitors) {
        updateAllMonitors();
        m_requestUpdatingAllMonitors = false;
    }
#if defined(IMGUI_HAS_VIEWPORT)
    if (HWND window = m_requestWindowClose.exchange(nullptr)) {
        if (ImGuiViewport *viewport = ImGui::FindViewportByPlatformHandle(window)) {
            viewport->PlatformRequestClose = true;
        }
    }
    if (HWND window = m_requestWindowMove.exchange(nullptr)) {
        if (ImGuiViewport *viewport = ImGui::FindViewportByPlatformHandle(window)) {
            viewport->PlatformRequestMove = true;
        }
    }
    if (HWND window = m_requestWindowResize.exchange(nullptr)) {
        if (ImGuiViewport *viewport = ImGui::FindViewportByPlatformHandle(window)) {
            viewport->PlatformRequestResize = true;
        }
    }
#endif
    SG_POP_GROUP();
}

URI
Win32ThreadedApplicationService::recoverableRedoFileURI() const
{
    HANDLE file = CreateFileW(s_crashFilePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_READONLY | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
    URI fileURI;
    if (file != INVALID_HANDLE_VALUE && ReadFile(file, s_redoFilePath, sizeof(s_redoFilePath), nullptr, nullptr)) {
        CloseHandle(file);
        MutableString ms;
        StringUtils::getMultiBytesString(s_redoFilePath, ms);
        fileURI = URI::createFromFilePath(ms.data());
    }
    return fileURI;
}

void
Win32ThreadedApplicationService::createDefaultRenderTarget(const Vector2UI16 &devicePixelWindowSize)
{
    sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_D3D11) {
        auto swapChain = (IDXGISwapChain *) m_nativeSwapChain;
        auto device = (ID3D11Device *) m_nativeDevice;
        if (!FAILED(swapChain->GetBuffer(0, IID_ID3D11Texture2D, (void **) &m_d3d11RenderTargetTexture))) {
            device->CreateRenderTargetView(m_d3d11RenderTargetTexture, nullptr, &m_d3d11RenderTargetView);
        }
        D3D11_TEXTURE2D_DESC td = {};
        auto swapChainDesc = (const DXGI_SWAP_CHAIN_DESC *) m_nativeSwapChainDescription;
        td.Width = devicePixelWindowSize.x;
        td.Height = devicePixelWindowSize.y;
        td.MipLevels = td.ArraySize = 1;
        td.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        td.SampleDesc = swapChainDesc->SampleDesc;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvd = {};
        dsvd.Format = td.Format;
        dsvd.ViewDimension =
            swapChainDesc->SampleDesc.Count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
        if (!FAILED(device->CreateTexture2D(&td, nullptr, &m_d3d11DepthStencilTexture))) {
            device->CreateDepthStencilView(m_d3d11DepthStencilTexture, nullptr, &m_d3d11DepthStencilView);
        }
#if !defined(NDEBUG)
        static const wchar_t kRenderTargetTextureLabel[] = L"@nanoem/MainWindow/RenderTarget/Texture";
        m_d3d11RenderTargetTexture->SetPrivateData(
            WKPDID_D3DDebugObjectNameW, sizeof(kRenderTargetTextureLabel), kRenderTargetTextureLabel);
        static const wchar_t kRenderTargetViewLabelView[] = L"@nanoem/MainWindow/RenderTarget/View";
        m_d3d11RenderTargetView->SetPrivateData(
            WKPDID_D3DDebugObjectNameW, sizeof(kRenderTargetViewLabelView), kRenderTargetViewLabelView);
        static const wchar_t kDepthStencilTextureLabel[] = L"@nanoem/MainWindow/DepthStencil/Texture";
        m_d3d11DepthStencilTexture->SetPrivateData(
            WKPDID_D3DDebugObjectNameW, sizeof(kDepthStencilTextureLabel), kDepthStencilTextureLabel);
        static const wchar_t kDepthStencilViewLabel[] = L"@nanoem/MainWindow/DepthStencil/View";
        m_d3d11DepthStencilView->SetPrivateData(
            WKPDID_D3DDebugObjectNameW, sizeof(kDepthStencilViewLabel), kDepthStencilViewLabel);
#endif
    }
}

void
Win32ThreadedApplicationService::resizeDefaultRenderTarget(
    const Vector2UI16 &devicePixelWindowSize, const Project *project)
{
    sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_D3D11) {
        auto context = (ID3D11DeviceContext *) m_nativeContext;
        ID3D11RenderTargetView *nullRenderTargetView = nullptr;
        context->OMSetRenderTargets(1, &nullRenderTargetView, nullptr);
        destroyDefaultRenderTarget();
        auto swapChain = (IDXGISwapChain *) m_nativeSwapChain;
        auto swapChainDesc = (DXGI_SWAP_CHAIN_DESC *) m_nativeSwapChainDescription;
        swapChain->ResizeBuffers(
            2, devicePixelWindowSize.x, devicePixelWindowSize.y, swapChainDesc->BufferDesc.Format, 0);
        createDefaultRenderTarget(devicePixelWindowSize);
    }
}

void
Win32ThreadedApplicationService::destroyDefaultRenderTarget()
{
    COMInline::safeRelease(m_d3d11RenderTargetView);
    COMInline::safeRelease(m_d3d11RenderTargetTexture);
    COMInline::safeRelease(m_d3d11DepthStencilView);
    COMInline::safeRelease(m_d3d11DepthStencilTexture);
}

void
Win32ThreadedApplicationService::setupNewProject()
{
    char path[MAX_PATH];
    const char *storagePath = json_object_dotget_string(json_object(applicationConfiguration()), "win32.redo.path");
    SYSTEMTIME time;
    GetLocalTime(&time);
    bx::snprintf(path, sizeof(path), "%s/nanoem-%04d-%02d-%02d-%02d%02d%02d.%s", storagePath, time.wYear, time.wMonth,
        time.wDay, time.wHour, time.wMinute, time.wSecond, Project::kRedoLogFileExtension);
    Project *project = projectHolder()->currentProject();
    project->setRedoFileURI(URI::createFromFilePath(path));
    MutableWideString ws;
    StringUtils::getWideCharString(path, ws);
    wcscpy_s(s_redoFilePath, ARRAYSIZE(s_redoFilePath), ws.data());
}

void
Win32ThreadedApplicationService::updateAllMonitors()
{
#if defined(IMGUI_HAS_VIEWPORT)
    ImGui::GetPlatformIO().Monitors.resize(0);
    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR hmonitor, HDC, LPRECT, LPARAM) {
            MONITORINFO info = {};
            info.cbSize = sizeof(info);
            if (GetMonitorInfoW(hmonitor, &info)) {
                ImGuiPlatformMonitor monitor;
                const RECT &monitorRect = info.rcMonitor;
                monitor.MainPos = ImVec2(monitorRect.left, monitorRect.top);
                monitor.MainSize = ImVec2(monitorRect.right - monitorRect.left, monitorRect.bottom - monitorRect.top);
                const RECT &workRect = info.rcMonitor;
                monitor.WorkPos = ImVec2(workRect.left, workRect.top);
                monitor.WorkSize = ImVec2(workRect.right - workRect.left, workRect.bottom - workRect.top);
                ImGuiPlatformIO &io = ImGui::GetPlatformIO();
                if (EnumUtils::isEnabledT<DWORD>(info.dwFlags, MONITORINFOF_PRIMARY)) {
                    io.Monitors.push_front(monitor);
                }
                else {
                    io.Monitors.push_back(monitor);
                }
            }
            return TRUE;
        },
        0);
#endif /* IMGUI_HAS_VIEWPORT */
}

} /* namespace win32 */
} /* namespace nanoem */
