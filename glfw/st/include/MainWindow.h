/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_GLFW_MAINWINDOW_H_
#define NANOEM_EMAPP_GLFW_MAINWINDOW_H_

#include "emapp/IFileManager.h"

#include <functional>
#include <vector>

struct GLFWwindow;

namespace bx {
class CommandLine;
class Semaphore;
}

namespace nanoem {
namespace glfw {

class GLFWApplicationClient;
class GLFWApplicationService;

class MainWindow final {
public:
    MainWindow(const bx::CommandLine *cmd, GLFWApplicationService *service, GLFWApplicationClient *client);
    ~MainWindow();

    bool initialize();

    bool isRunning() const;
    void processMessage();

private:
    static void handleErrorCallback(int code, const char *message);
    static void handleResizeCallback(GLFWwindow *window, int width, int height);
    static void handleCloseCallback(GLFWwindow *window);
    static void handleIconifyCallback(GLFWwindow *window, int value);
    static void handleCursorCallback(GLFWwindow *window, double xpos, double ypos);
    static void handleMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void handleScrollCallback(GLFWwindow *window, double x, double y);
    static void handleKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void handleCharCallback(GLFWwindow *window, uint32_t key);
    static void handleDropCallback(GLFWwindow *window, int count, const char **filenames);

    bool setupWindow(String &pluginPath);
    void setTitle(const URI &fileURI);
    void destroyWindow();

    const bx::CommandLine *m_cmd = nullptr;
    GLFWApplicationService *m_service = nullptr;
    GLFWApplicationClient *m_client = nullptr;
    GLFWwindow *m_window = nullptr;
    Vector2SI32 m_lastCursorPosition = Vector2SI32(0);
    void *m_sentryDllHandle = nullptr;
    int m_pressedCursorType = 0;
    int m_pressedCursorModifiers = 0;
    bool m_running = true;
};

} /* namespace glfw */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_GLFW_MAINWINDOW_H_ */
