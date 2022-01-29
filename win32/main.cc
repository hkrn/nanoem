/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "MainWindow.h"
#include "Preference.h"
#include "Win32ThreadedApplicationService.h"

#include <Windows.h>
#include <mfapi.h>
#include <objbase.h>
#include <shellapi.h>
#include <string>
#include <windowsx.h>

#include "bx/commandline.h"
#include "emapp/Allocator.h"
#include "emapp/FileUtils.h"
#include "emapp/StringUtils.h"
#include "emapp/ThreadedApplicationClient.h"

using namespace nanoem;
using namespace nanoem::win32;

#if defined(__MINGW32__)
#define _CrtSetDbgFlag(a)
#endif

namespace {

static int
runApplication(HINSTANCE hInstance, int argc, const char *const *argv, const wchar_t *executablePath)
{
    Win32ThreadedApplicationService::installUnhandledExceptionHandler();
    JSON_Value *config = Win32ThreadedApplicationService::loadJSONConfig(executablePath);
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    {
        const bx::CommandLine command(argc, argv);
        Win32ThreadedApplicationService service(config);
        ThreadedApplicationClient client;
        Preference preference(&service, config);
        preference.load();
        service.start();
        client.connect();
        const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        const nanoem_f32_t devicePixelRatio = Win32ThreadedApplicationService::calculateDevicePixelRatio();
        const Vector2UI32 &windowSize = ThreadedApplicationService::minimumRequiredWindowSize();
        const Vector4UI32 rect((screenWidth - windowSize.x * devicePixelRatio) * 0.5f,
            (screenHeight - windowSize.y * devicePixelRatio) * 0.5f, windowSize);
        MainWindow window(&command, &preference, &service, &client, hInstance, rect, devicePixelRatio);
        while (window.isRunning()) {
            window.processMessage(&msg);
        }
        service.stop();
        preference.synchronize();
        preference.save();
    }
    json_value_free(config);
    return static_cast<int>(msg.wParam);
}

static int
runMain(HINSTANCE hInstance, int argc, const char *const *argv)
{
#if defined(NANOEM_WIN32_ATTACH_CONSOLE)
    // https://stackoverflow.com/questions/54536/win32-gui-app-that-writes-usage-text-to-stdout-when-invoked-as-app-exe-help
    if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
#endif /* NANOEM_WIN32_ALLOCATE_CONSOLE */
    wchar_t executablePath[MAX_PATH];
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    SetDllDirectoryW(L"");
    SetThreadExecutionState(ES_CONTINUOUS | ES_AWAYMODE_REQUIRED | ES_DISPLAY_REQUIRED);
    HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    Allocator::initialize();
    ThreadedApplicationService::setup();
    int exitCode = -1;
    if (!FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)) &&
        !FAILED(MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET))) {
        GetModuleFileNameW(GetInstanceModule(hInstance), executablePath, ARRAYSIZE(executablePath));
        exitCode = runApplication(hInstance, argc, argv, executablePath);
    }
    MFShutdown();
    CoUninitialize();
    SetThreadExecutionState(ES_CONTINUOUS);
    ThreadedApplicationService::terminate();
    Allocator::destroy();
    return exitCode;
}

} /* namespace anonymous */

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
    BX_UNUSED_3(hPrevInstance, lpCmdLine, nCmdShow);
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
    BX_UNUSED_3(hPrevInstance, lpCmdLine, nCmdShow);
    char *argv[64], buffer[1024];
    uint32_t size = BX_COUNTOF(buffer);
    int argc = 0;
    bx::tokenizeCommandLine(lpCmdLine, buffer, size, argc, argv, BX_COUNTOF(argv));
    return runMain(hInstance, argc, argv);
}
#endif
