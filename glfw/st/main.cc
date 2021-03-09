/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/Allocator.h"
#include "emapp/emapp.h"

#include "GLFWApplicationClient.h"
#include "GLFWApplicationService.h"
#include "MainWindow.h"

#include "GLFW/glfw3.h"
#include "bx/commandline.h"
#include "bx/os.h"

#include <dirent.h>
#if defined(_WIN32)
#include <objbase.h>
#endif

using namespace nanoem;

static int
runMain(int argc, const char *const *argv)
{
    glfwInit();
    Allocator::initialize();
    BaseApplicationService::setup();
    {
        JSON_Value *config = json_value_init_object();
        JSON_Object *root = json_object(config);
        bx::FilePath basePath(bx::Dir::Current), tempPath(bx::Dir::Temp);
        String effectPath(basePath.getCPtr()), pluginDirPath(basePath.getCPtr());
        pluginDirPath.append("/plugins");
        URIList pluginURIs;
        if (DIR *dir = opendir(pluginDirPath.c_str())) {
            while (dirent *ent = readdir(dir)) {
                if (StringUtils::equals(ent->d_name, "plugin", 6)) {
                    String pluginPath(pluginDirPath);
                    pluginPath.append("/");
                    pluginPath.append(ent->d_name);
                    pluginURIs.push_back(URI::createFromFilePath(pluginPath));
                }
            }
            closedir(dir);
        }
        effectPath.append("/plugins/plugin_effect." BX_DL_EXT);
        json_object_dotset_string(root, "glfw.path", basePath.getCPtr());
        json_object_dotset_string(root, "project.tmp.path", tempPath.getCPtr());
        json_object_dotset_string(root, "plugin.effect.path", effectPath.c_str());
        String sentryCrashpadHandlerPath(basePath.getCPtr()), sentryDllPath(basePath.getCPtr()),
            sentryDatabasePath(basePath.getCPtr());
        sentryCrashpadHandlerPath.append("/sentry/crashpad_handler");
        sentryDllPath.append("/sentry/");
#if defined(_WIN32)
        sentryCrashpadHandlerPath.append(".exe");
        sentryDllPath.append("sentry.dll");
#elif defined(__APPLE__)
        sentryDllPath.append("libsentry.dylib");
#else
        sentryDllPath.append("libsentry.so");
#endif
        sentryDatabasePath.append("/sentry-db");
        json_object_dotset_string(root, "glfw.sentry.database.path", sentryDatabasePath.c_str());
        json_object_dotset_string(root, "glfw.sentry.handler.path", sentryCrashpadHandlerPath.c_str());
        json_object_dotset_string(root, "glfw.sentry.library.path", sentryDllPath.c_str());
        char localeBuffer[32];
        uint32_t localeSize = sizeof(localeBuffer);
        bx::getEnv(localeBuffer, &localeSize, "LANG");
        json_object_dotset_string(root, "project.locale", localeBuffer);
        bx::CommandLine command(argc, argv);
        glfw::GLFWApplicationClient::Bridge bridge;
        glfw::GLFWApplicationClient client(&bridge);
        glfw::GLFWApplicationService service(config, &bridge);
        glfw::MainWindow window(&service, &client);
        if (window.initialize()) {
            client.sendLoadAllDecoderPluginsMessage(pluginURIs);
            client.sendLoadAllEncoderPluginsMessage(pluginURIs);
            client.sendLoadAllModelPluginsMessage(pluginURIs);
            client.sendLoadAllMotionPluginsMessage(pluginURIs);
            while (window.isRunning()) {
                window.processMessage();
            }
        }
        json_value_free(config);
    }
    Allocator::destroy();
    glfwTerminate();
    return 0;
}

#if !defined(_WIN32)
int
main(int argc, char *argv[])
{
    return runMain(argc, argv);
}
#elif defined(UNICODE)
int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    BX_UNUSED_3(hPrevInstance, lpCmdLine, nCmdShow);
    return runMain(0, 0);
}
#else
int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    BX_UNUSED_3(hInstance, hPrevInstance, nCmdShow);
    char *argv[64], buffer[1024];
    uint32_t size = BX_COUNTOF(buffer);
    int argc = 0;
    bx::tokenizeCommandLine(lpCmdLine, buffer, size, argc, argv, BX_COUNTOF(argv));
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    SetDllDirectoryW(L"");
    SetThreadExecutionState(ES_CONTINUOUS | ES_AWAYMODE_REQUIRED | ES_DISPLAY_REQUIRED);
    HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    int result = runMain(argc, argv);
    SetThreadExecutionState(ES_CONTINUOUS);
    CoUninitialize();
    return result;
}
#endif
