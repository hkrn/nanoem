/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include <ShlObj.h>
#include <Windows.h>
#include <commdlg.h>
#include <d3d11.h>
#include <dxgi.h>
#include <mfapi.h>
#include <objbase.h>
#include <Pdh.h>
#include <Psapi.h>
#include <shellapi.h>
#include <windowsx.h>
#include <VersionHelpers.h>

#include "bx/commandline.h"
#include "emapp/Allocator.h"

#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#include "spdlog/async.h"
#include "spdlog/cfg/env.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "Win32ThreadedApplicationService.h"
#include "emapp/integration/LoadPMMExecutor.h"

using namespace nanoem;

static const wchar_t kClassName[] = L"nanoem";
static const nanoem_f32_t kStandardDPIValue = 96.0f;

class TestWindow {
public:
    static nanoem_f32_t calculateDevicePixelRatio();

    TestWindow(ThreadedApplicationService *service, ThreadedApplicationClient *client, HINSTANCE hInstance,
        const bx::CommandLine *cmd, const Vector4UI32 &rect, nanoem_f32_t devicePixelRatio);
    ~TestWindow();

    void initialize();
    void processMessage(MSG *msg);
    bool isRunning() const;

private:
    static LRESULT CALLBACK handleWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    static DWORD CALLBACK collectPerformanceMetricsPeriodically(void *userData);
    static test::IExecutor *createExecutor(ThreadedApplicationService *service, ThreadedApplicationClient *client,
        const bx::CommandLine *cmd, bx::Mutex *eventLock);
    static URI generateFileURI(bool archive);
    nanoem_f32_t invertedDevicePixelRatio() const;
    void setupDirectXRenderer(int width, int height);
    void setupOpenGLRenderer(int width, int height);
    void handleInitializeEvent();
    void handleDestroyEvent();
    void handleTerminateEvent();
    void handleWindowDestroy();
    void registerAllPrerequisiteEventListeners();

    const bx::CommandLine *m_command;
    ThreadedApplicationService *m_service = nullptr;
    ThreadedApplicationClient *m_client = nullptr;
    HINSTANCE m_instanceHandle = nullptr;
    HWND m_windowHandle = nullptr;
    HANDLE m_metricThreadHandle = nullptr;
    HANDLE m_processHandle = nullptr;
    void *m_context = nullptr;
    void *m_device = nullptr;
    DXGI_SWAP_CHAIN_DESC m_swapChainDesc;
    IDXGISwapChain *m_swapChain = nullptr;
    test::IExecutor *m_runner;
    bx::Mutex m_eventLock;
    Vector2SI32 m_lastLogicalCursorPosition;
    Vector2SI32 m_virtualLogicalCursorPosition;
    Vector2SI32 m_restoreHiddenDeviceCursorPosition;
    tinystl::pair<bool, bool> m_cursorHidden = tinystl::make_pair(false, false);
    Vector2UI32 m_logicalPixelSize;
    nanoem_f32_t m_devicePixelRatio = 1.0f;
    volatile bool m_running = true;
};

nanoem_f32_t
TestWindow::calculateDevicePixelRatio()
{
    float devicePixelRatio = 1.0f;
    if (HMODULE shcore = LoadLibraryA("shcore.dll")) {
        typedef HRESULT(WINAPI * pfn_GetDpiForMonitor)(HMONITOR, int, UINT *, UINT *);
        if (pfn_GetDpiForMonitor GetDpiForMonitor =
                reinterpret_cast<pfn_GetDpiForMonitor>(GetProcAddress(shcore, "GetDpiForMonitor"))) {
            const POINT point = { 0, 0 };
            HMONITOR monitor = MonitorFromPoint(point, MONITOR_DEFAULTTOPRIMARY);
            UINT dpiX, dpiY;
            GetDpiForMonitor(monitor, 0 /* MDT_EFFECTIVE_DPI */, &dpiX, &dpiY);
            devicePixelRatio = dpiX / kStandardDPIValue;
        }
        FreeLibrary(shcore);
    }
    return devicePixelRatio;
}

TestWindow::TestWindow(ThreadedApplicationService *service, ThreadedApplicationClient *client, HINSTANCE hInstance,
    const bx::CommandLine *cmd, const Vector4UI32 &rect, nanoem_f32_t devicePixelRatio)
    : m_command(cmd)
    , m_instanceHandle(hInstance)
    , m_service(service)
    , m_client(client)
    , m_runner(createExecutor(service, client, cmd, &m_eventLock))
{
    ZeroMemory(&m_swapChainDesc, sizeof(m_swapChainDesc));
    WNDCLASSEXW windowClass;
    ZeroMemory(&windowClass, sizeof(windowClass));
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = &handleWindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = kClassName;
    windowClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    if (RegisterClassExW(&windowClass) != 0) {
        m_devicePixelRatio = devicePixelRatio;
        const Vector2SI32 devicePixelSize(glm::vec2(rect.z, rect.w) * devicePixelRatio);
        m_windowHandle = CreateWindowExW(WS_EX_ACCEPTFILES | WS_EX_APPWINDOW, kClassName, kClassName,
            WS_OVERLAPPEDWINDOW, rect.x, rect.y, devicePixelSize.x, devicePixelSize.y, NULL, NULL, hInstance, this);
    }
}

TestWindow::~TestWindow()
{
    delete m_runner;
    m_runner = 0;
}

void
TestWindow::initialize()
{
    wchar_t executablePath[MAX_PATH];
    GetModuleFileNameW(GetInstanceModule(m_instanceHandle), executablePath, ARRAYSIZE(executablePath));
    RECT clientRect;
    GetClientRect(m_windowHandle, &clientRect);
    const int width = clientRect.right - clientRect.left, height = clientRect.bottom - clientRect.top;
    m_logicalPixelSize = glm::vec2(width, height) * invertedDevicePixelRatio();
    String sokolPath;
    if (const wchar_t *p = wcsrchr(executablePath, L'\\')) {
        wchar_t innerSokolPath[MAX_PATH];
        wcsncpy_s(innerSokolPath, ARRAYSIZE(innerSokolPath), executablePath, p - executablePath);
        MutableString ms;
        StringUtils::getMultiBytesString(innerSokolPath, ms);
        FileUtils::canonicalizePathSeparator(ms);
        sokolPath.append(ms.data());
    }
#ifdef CMAKE_INTDIR
    sokolPath.append("/../../emapp/bundle/sokol/" CMAKE_INTDIR "/");
#else
    sokolPath.append("/../emapp/bundle/sokol/");
#endif
    sg_pixel_format format;
    if (m_command->hasArg("opengl")) {
        setupOpenGLRenderer(width, height);
        sokolPath.append("sokol_glcore33.dll");
        format = SG_PIXELFORMAT_RGBA8;
    }
    else {
        setupDirectXRenderer(width, height);
        sokolPath.append("sokol_d3d11.dll");
        format = SG_PIXELFORMAT_BGRA8;
    }
    const ThreadedApplicationClient::InitializeMessageDescription desc(
        m_logicalPixelSize, format, m_devicePixelRatio, sokolPath);
    m_client->sendInitializeMessage(desc);
    EMLOG_INFO("{}", "Initialize message has been sent");
}

void
TestWindow::processMessage(MSG *msg)
{
    {
        bx::MutexScope scope(m_eventLock);
        BX_UNUSED_1(scope);
        m_client->receiveAllEventMessages();
    }
    while (PeekMessageW(msg, nullptr, 0, 0, PM_REMOVE) != 0) {
        TranslateMessage(msg);
        DispatchMessageW(msg);
    }
    if (m_running) {
        WaitMessage();
    }
}

bool
TestWindow::isRunning() const
{
    return m_running;
}

LRESULT CALLBACK
TestWindow::handleWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = S_OK;
    switch (msg) {
    case WM_CREATE: {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lparam);
        TestWindow *window = static_cast<TestWindow *>(lpcs->lpCreateParams);
        window->registerAllPrerequisiteEventListeners();
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        break;
    }
    case WM_DESTROY: {
        if (TestWindow *window = reinterpret_cast<TestWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
            window->handleWindowDestroy();
        }
        break;
    }
    default:
        result = DefWindowProcW(hwnd, msg, wparam, lparam);
        break;
    }
    return result;
}

DWORD CALLBACK
TestWindow::collectPerformanceMetricsPeriodically(void *userData)
{
    auto self = static_cast<TestWindow *>(userData);
    PROCESS_MEMORY_COUNTERS counters = {};
    PDH_HQUERY query = nullptr;
    PDH_HCOUNTER counter = nullptr;
    PDH_FMT_COUNTERVALUE value = {};
    GetProcessMemoryInfo(self->m_processHandle, &counters, sizeof(counters));
    self->m_client->sendUpdatePerformanceMonitorMessage(nanoem_f32_t(value.doubleValue), counters.WorkingSetSize, 0);
    PdhOpenQueryW(nullptr, 0, &query);
    PdhAddCounterW(query, L"\\Process(nanoem_win32_test)\\% User Time", 0, &counter);
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

test::IExecutor *
TestWindow::createExecutor(ThreadedApplicationService *service, ThreadedApplicationClient *client,
    const bx::CommandLine *cmd, bx::Mutex *eventLock)
{
    test::IExecutor *executor = 0;
    if (cmd->hasArg("models")) {
        const URI &fileURI = URI::createFromFilePath(cmd->findOption("models"));
        executor = new test::LoadAllModelsExecutor(fileURI, cmd, service, client);
    }
    else if (cmd->hasArg("effects")) {
        const URI &fileURI = URI::createFromFilePath(cmd->findOption("effects"));
        executor = new test::LoadAllEffectsExecutor(fileURI, cmd, service, client);
    }
    else {
        const URI &fileURI = generateFileURI(cmd->hasArg("archive"));
        executor = new test::LoadPMMExecutor(fileURI, cmd, service, client, eventLock);
    }
    return executor;
}

URI
TestWindow::generateFileURI(bool archive)
{
    MutableWideString newSourcePath;
    MutableString ms;
    wchar_t workingDirectory[MAX_PATH], date[16], time[16], destinationPath[MAX_PATH];
    GetTempPathW(ARRAYSIZE(workingDirectory), workingDirectory);
    GetDateFormatW(LOCALE_USER_DEFAULT, 0, nullptr, L"yyyy-MM-dd", date, ARRAYSIZE(date));
    GetTimeFormatW(LOCALE_USER_DEFAULT, 0, nullptr, L"HHmmss", time, ARRAYSIZE(time));
    wcscpy_s(destinationPath, ARRAYSIZE(destinationPath), workingDirectory);
    wcscat_s(destinationPath, ARRAYSIZE(destinationPath), L"nanoem-");
    wcscat_s(destinationPath, ARRAYSIZE(destinationPath), date);
    wcscat_s(destinationPath, ARRAYSIZE(destinationPath), L"-");
    wcscat_s(destinationPath, ARRAYSIZE(destinationPath), time);
    wcscat_s(destinationPath, ARRAYSIZE(destinationPath), archive ? L".nma" : L".nmm");
    StringUtils::getMultiBytesString(destinationPath, ms);
    FileUtils::canonicalizePathSeparator(ms);
    return URI::createFromFilePath(ms.data());
}

nanoem_f32_t
TestWindow::invertedDevicePixelRatio() const
{
    return 1.0f / m_devicePixelRatio;
}

void
TestWindow::setupDirectXRenderer(int width, int height)
{
    DXGI_MODE_DESC &mode = m_swapChainDesc.BufferDesc;
    mode.Width = width;
    mode.Height = height;
    mode.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    DXGI_RATIONAL &refreshRate = mode.RefreshRate;
    refreshRate.Denominator = 1;
    refreshRate.Numerator = 60;
    m_swapChainDesc.OutputWindow = m_windowHandle;
    m_swapChainDesc.Windowed = true;
    m_swapChainDesc.BufferCount = 2;
    m_swapChainDesc.SwapEffect = IsWindows10OrGreater() ? DXGI_SWAP_EFFECT_FLIP_DISCARD : DXGI_SWAP_EFFECT_DISCARD;
    m_swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    DXGI_SAMPLE_DESC &sample = m_swapChainDesc.SampleDesc;
    sample.Count = 1;
    sample.Quality = 0;
    m_swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    uint32_t flags = 0;
    if (m_command->hasArg("debug")) {
        flags |= D3D11_CREATE_DEVICE_DEBUG;
        flags |= D3D11_CREATE_DEVICE_DEBUGGABLE;
    }
    ID3D11DeviceContext *context = nullptr;
    ID3D11Device *device = nullptr;
    IDXGISwapChain *swapChain = nullptr;
    D3D_FEATURE_LEVEL level;
    HRESULT rc = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, nullptr, 0,
        D3D11_SDK_VERSION, &m_swapChainDesc, &swapChain, &device, &level, &context);
    if (SUCCEEDED(rc)) {
        m_swapChain = swapChain;
        m_context = context;
        m_device = device;
        if (EnumUtils::isEnabled(D3D11_CREATE_DEVICE_DEBUG, flags)) {
            static const wchar_t kSwapChainLabel[] = L"IDXGISwapChain";
            swapChain->SetPrivateData(WKPDID_D3DDebugObjectNameW, sizeof(kSwapChainLabel), kSwapChainLabel);
            static const wchar_t kDeviceLabel[] = L"ID3D11Device";
            device->SetPrivateData(WKPDID_D3DDebugObjectNameW, sizeof(kDeviceLabel), kDeviceLabel);
            static const wchar_t kDeviceContextLabel[] = L"ID3D11DeviceContext";
            context->SetPrivateData(WKPDID_D3DDebugObjectNameW, sizeof(kDeviceContextLabel), kDeviceContextLabel);
        }
        ID3D10Multithread *threaded;
        if (!FAILED(context->QueryInterface(IID_PPV_ARGS(&threaded)))) {
            threaded->SetMultithreadProtected(TRUE);
            threaded->Release();
        }
        m_service->setNativeContext(context);
        m_service->setNativeDevice(device);
        m_service->setNativeView(m_windowHandle);
        m_service->setNativeSwapChain(swapChain);
        m_service->setNativeSwapChainDescription(&m_swapChainDesc);
    }
}

void
TestWindow::setupOpenGLRenderer(int /* width */, int /* height */)
{
    typedef BOOL (*pfn_wglSwapIntervalEXT)(int interval);
    typedef HGLRC (*pfn_wglCreateContextAttribsARB)(HDC hDC, HGLRC hshareContext, const int *attribList);
    static const int WGL_CONTEXT_MAJOR_VERSION_ARB = 0x2091;
    static const int WGL_CONTEXT_MINOR_VERSION_ARB = 0x2092;
    static const int WGL_CONTEXT_FLAGS_ARB = 0x2094;
    static const int WGL_CONTEXT_DEBUG_BIT_ARB = 0x0001;
    static const int WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB = 0x0002;
    static const int WGL_CONTEXT_PROFILE_MASK_ARB = 0x9126;
    static const int WGL_CONTEXT_CORE_PROFILE_BIT_ARB = 0x1;
    static const int attributes[] = { WGL_CONTEXT_MAJOR_VERSION_ARB, 3, WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB | WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 0 };
    HDC device = GetDC(m_windowHandle);
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(pfd));
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
    DescribePixelFormat(device, pixelFormat, sizeof(pfd), &pfd);
    SetPixelFormat(device, pixelFormat, &pfd);
    HGLRC temp = wglCreateContext(device);
    wglMakeCurrent(device, temp);
    pfn_wglSwapIntervalEXT wglSwapIntervalEXT =
        reinterpret_cast<pfn_wglSwapIntervalEXT>(wglGetProcAddress("wglSwapIntervalEXT"));
    wglSwapIntervalEXT(1);
    pfn_wglCreateContextAttribsARB wglCreateContextAttribsARB =
        reinterpret_cast<pfn_wglCreateContextAttribsARB>(wglGetProcAddress("wglCreateContextAttribsARB"));
    HGLRC context = wglCreateContextAttribsARB(device, temp, attributes);
    wglMakeCurrent(device, nullptr);
    wglDeleteContext(temp);
    m_context = context;
    m_device = device;
    m_service->setNativeContext(m_context);
    m_service->setNativeDevice(m_device);
    m_service->setNativeView(m_windowHandle);
}

void
TestWindow::handleInitializeEvent()
{
    DWORD processId = GetCurrentProcessId();
    m_processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, false, processId);
    if (m_processHandle != nullptr) {
        m_metricThreadHandle = CreateThread(nullptr, 0, collectPerformanceMetricsPeriodically, this, 0, nullptr);
    }
    ShowWindow(m_windowHandle, SW_SHOW);
    m_client->sendActivateMessage();
    m_runner->start();
    EMLOG_INFO("Initialization completed: processID={}", processId);
}

void
TestWindow::handleDestroyEvent()
{
    m_client->addCompleteTerminationEventListener(
        [](void *userData) {
            TestWindow *self = static_cast<TestWindow *>(userData);
            self->handleTerminateEvent();
        },
        this, true);
    m_client->sendTerminateMessage();
    EMLOG_INFO("{}", "Terminate message has been sent");
}

void
TestWindow::handleTerminateEvent()
{
    if (IDXGISwapChain *swapChain = m_swapChain) {
        ID3D11DeviceContext *context = static_cast<ID3D11DeviceContext *>(m_context);
        ID3D11Device *device = static_cast<ID3D11Device *>(m_device);
        context->Release();
        device->Release();
        swapChain->Release();
    }
    else {
        HGLRC context = static_cast<HGLRC>(m_context);
        HDC device = static_cast<HDC>(m_device);
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(context);
        ReleaseDC(m_windowHandle, device);
    }
    DestroyWindow(m_windowHandle);
    EMLOG_INFO("Termination has been completed: window={}", static_cast<const void *>(m_windowHandle));
}

void
TestWindow::handleWindowDestroy()
{
    m_running = false;
    if (m_metricThreadHandle) {
        WaitForSingleObject(m_metricThreadHandle, INFINITE);
        CloseHandle(m_metricThreadHandle);
        m_metricThreadHandle = nullptr;
        CloseHandle(m_processHandle);
        m_processHandle = nullptr;
    }
    PostQuitMessage(0);
    EMLOG_INFO("{}", "Destroying window has been completed");
}

void
TestWindow::registerAllPrerequisiteEventListeners()
{
    m_client->addInitializationCompleteEventListener(
        [](void *userData) {
            TestWindow *self = static_cast<TestWindow *>(userData);
            self->handleInitializeEvent();
        },
        this, true);
    m_client->addCompleteDestructionEventListener(
        [](void *userData) {
            TestWindow *self = static_cast<TestWindow *>(userData);
            self->m_runner->finish();
            self->handleDestroyEvent();
        },
        this, true);
    m_client->addCompleteLoadingFileEventListener(
        [](void *userData, const URI &fileURI, uint32_t type, uint64_t ticks) {
            TestWindow *self = static_cast<TestWindow *>(userData);
            BX_UNUSED_1(self);
            EMLOG_INFO("Loading the file has been completed: type={} fileURI={} seconds={}", type,
                fileURI.absolutePathConstString(), stm_sec(ticks));
        },
        this, false);
    m_client->addCompleteSavingFileEventListener(
        [](void *userData, const URI &fileURI, uint32_t type, uint64_t ticks) {
            TestWindow *self = static_cast<TestWindow *>(userData);
            BX_UNUSED_1(self);
            EMLOG_INFO("Saving the file has been completed: type={} fileURI={} seconds={}", type,
                fileURI.absolutePathConstString(), stm_sec(ticks));
        },
        this, false);
}

template <typename Mutex> class OutputDebugStringSink : public spdlog::sinks::base_sink<Mutex> {
public:
    OutputDebugStringSink() = default;

protected:
    void
    sink_it_(const spdlog::details::log_msg &msg) override
    {
        spdlog::memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        formatted.push_back('\0');
        spdlog::wmemory_buf_t buf;
        spdlog::string_view_t view(formatted.data(), formatted.size());
        spdlog::details::os::utf8_to_wstrbuf(view, buf);
        OutputDebugStringW(buf.data());
    }

    void
    flush_() override
    {
    }
};

static int
runMain(HINSTANCE hInstance, int argc, const char *const *argv)
{
    wchar_t executablePath[MAX_PATH];
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    SetDllDirectoryW(L"");
    SetThreadExecutionState(ES_CONTINUOUS | ES_AWAYMODE_REQUIRED | ES_DISPLAY_REQUIRED);
    HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    MFStartup(MF_VERSION);
    Allocator::initialize();
    ThreadedApplicationService::setup();
    const bx::CommandLine command(argc, argv);
    const char *filename = command.findOption("json", "test.json");
    GetModuleFileNameW(GetInstanceModule(hInstance), executablePath, ARRAYSIZE(executablePath));
    JSON_Value *config = win32::Win32ThreadedApplicationService::loadJSONConfig(executablePath);
    json_object_dotset_string(json_object(config), "plugin.effect.path", "plugin_effect.dll");
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    {
        const bx::CommandLine command(argc, argv);
        win32::Win32ThreadedApplicationService service(config);
        ThreadedApplicationClient client;
        service.start();
        client.connect();
        {
            spdlog::init_thread_pool(1024, 1);
            tinystl::vector<spdlog::sink_ptr, TinySTLAllocator> sinks;
            sinks.push_back(std::make_shared<OutputDebugStringSink<std::mutex>>());
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
            auto logger = std::make_shared<spdlog::async_logger>(
                "emapp", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
            spdlog::register_logger(logger);
            spdlog::cfg::load_env_levels();
        }
        const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        const nanoem_f32_t devicePixelRatio = TestWindow::calculateDevicePixelRatio();
        const Vector2UI32 &windowSize = ThreadedApplicationService::minimumRequiredWindowSize();
        const Vector4UI32 rect((screenWidth - windowSize.x * devicePixelRatio) * 0.5f,
            (screenHeight - windowSize.y * devicePixelRatio) * 0.5f, windowSize);
        TestWindow window(&service, &client, hInstance, &command, rect, devicePixelRatio);
        window.initialize();
        while (window.isRunning()) {
            window.processMessage(&msg);
        }
        int result = service.stop();
        EMLOG_INFO("Stopping application service thread has been completed: result={}", result);
    }
    json_value_free(config);
    MFShutdown();
    CoUninitialize();
    SetThreadExecutionState(ES_CONTINUOUS);
    ThreadedApplicationService::terminate();
    Allocator::destroy();
    return static_cast<int>(msg.wParam);
}

#if !BX_CRT_MSVC
int
main(int argc, char *argv[])
{
    return runMain(nullptr, argc, argv);
}
#elif defined(UNICODE)
int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    BX_UNUSED_2(hPrevInstance, nCmdShow);
    char *argv[64], interm[1024], buffer[1024];
    int argc = 0, actual = WideCharToMultiByte(CP_UTF8, 0, lpCmdLine, -1, interm, sizeof(interm), nullptr, nullptr);
    interm[actual] = 0;
    uint32_t size = static_cast<uint32_t>(actual);
    bx::tokenizeCommandLine(interm, buffer, size, argc, argv, BX_COUNTOF(argv));
    return runMain(hInstance, argc, argv);
}
#else
int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    BX_UNUSED_2(hPrevInstance, nCmdShow);
    char *argv[64], buffer[1024];
    uint32_t size = BX_COUNTOF(buffer);
    int argc = 0;
    bx::tokenizeCommandLine(lpCmdLine, buffer, size, argc, argv, BX_COUNTOF(argv));
    return runMain(hInstance, argc, argv);
}
#endif
