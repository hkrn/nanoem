/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "GLFWApplicationService.h"

#include "SoundIOAudioPlayer.h"

#include "emapp/ApplicationPreference.h"
#include "emapp/EnumUtils.h"
#include "emapp/IProjectHolder.h"
#include "emapp/StateController.h"
#include "emapp/StringUtils.h"
#include "emapp/internal/ImGuiWindow.h"
#include "emapp/internal/OpenGLComputeShaderSkinDeformerFactory.h"
#include "emapp/internal/OpenGLTransformFeedbackSkinDeformerFactory.h"
#include "emapp/private/CommonInclude.h"

#include "GLFW/glfw3.h"
#include "imgui/imgui.h"

#include "glm/gtx/string_cast.hpp"

namespace nanoem {
namespace glfw {
namespace {

#if defined(IMGUI_HAS_VIEWPORT)
struct ViewportWindow {
    ViewportWindow(GLFWApplicationService *parent, GLFWApplicationClient::Bridge *bridge);
    ~ViewportWindow();

    void createWindow(ImGuiViewport *viewport);
    void destroyWindow();
    void showWindow();
    ImVec2 windowPos() const;
    void setWindowPos(const ImVec2 &value);
    ImVec2 windowSize() const;
    void setWindowSize(const ImVec2 &value);
    bool hasWindowFocus() const;
    void makeWindowFocus();
    bool isWindowMinimized() const;
    void setWindowTitle(const char *value);
    void setWindowAlpha(float value);

    GLFWApplicationService *m_service = nullptr;
    GLFWwindow *m_window = nullptr;
    GLFWApplicationClient m_client;
    sg_context m_context = { SG_INVALID_ID };
    int m_pressedCursorType = 0;
    int m_pressedCursorModifiers = 0;
};

ViewportWindow::ViewportWindow(GLFWApplicationService *parent, GLFWApplicationClient::Bridge *bridge)
    : m_service(parent)
    , m_client(bridge)
{
}

ViewportWindow::~ViewportWindow()
{
}

void
ViewportWindow::createWindow(ImGuiViewport *viewport)
{
    BX_UNUSED_1(viewport);
    GLFWwindow *parentWindow = nullptr;
    if (viewport->ParentViewportId != 0) {
        if (ImGuiViewport *parent = ImGui::FindViewportByID(viewport->ParentViewportId)) {
            parentWindow = static_cast<GLFWwindow *>(parent->PlatformHandleRaw);
        }
    }
    const ImVec2 &size = viewport->Size;
    glfwWindowHint(GLFW_FOCUSED, 0);
    glfwWindowHint(GLFW_VISIBLE, 0);
#if defined(GLFW_HAS_FOCUS_ON_SHOW) && GLFW_HAS_FOCUS_ON_SHOW
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, 0);
#endif
    glfwWindowHint(GLFW_DECORATED, (viewport->Flags & ImGuiViewportFlags_NoDecoration) ? false : true);
#if defined(GLFW_HAS_WINDOW_TOPMOST) && GLFW_HAS_WINDOW_TOPMOST
    glfwWindowHint(GLFW_FLOATING, (viewport->Flags & ImGuiViewportFlags_TopMost) ? true : false);
#endif
    const Vector2SI32 windowSize(size.x, size.y);
    if (GLFWwindow *window = glfwCreateWindow(windowSize.x, windowSize.y, "Untitled", nullptr, parentWindow)) {
        const Vector2SI32 windowPos(viewport->Pos.x, viewport->Pos.y);
        glfwSetWindowPos(window, windowPos.x, windowPos.y);
        glfwSetWindowUserPointer(window, viewport);
        glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xpos, double ypos) {
            auto viewport = static_cast<ImGuiViewport *>(glfwGetWindowUserPointer(window));
            auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
            const Vector2SI32 position(xpos, ypos),
                screenPosition(GLFWApplicationService::deviceScaleCursorPosition(xpos, ypos, window));
            userData->m_client.sendScreenCursorMoveMessage(
                screenPosition, userData->m_pressedCursorType, userData->m_pressedCursorModifiers);
        });
        glfwSetScrollCallback(window, [](GLFWwindow *window, double x, double y) {
            auto viewport = static_cast<ImGuiViewport *>(glfwGetWindowUserPointer(window));
            auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            const Vector2SI32 position(xpos, ypos), delta(x, y);
            userData->m_client.sendCursorScrollMessage(position, delta, 0);
        });
        glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods) {
            auto viewport = static_cast<ImGuiViewport *>(glfwGetWindowUserPointer(window));
            auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            const Vector2SI32 position(xpos, ypos),
                screenPosition(GLFWApplicationService::deviceScaleCursorPosition(xpos, ypos, window));
            int cursorModifiers = GLFWApplicationService::convertCursorModifier(mods),
                cursorType = GLFWApplicationService::convertCursorType(button);
            switch (action) {
            case GLFW_PRESS: {
                userData->m_client.sendScreenCursorPressMessage(screenPosition, cursorType, cursorModifiers);
                userData->m_pressedCursorType = cursorType;
                userData->m_pressedCursorModifiers = cursorModifiers;
                break;
            }
            case GLFW_RELEASE: {
                userData->m_client.sendScreenCursorReleaseMessage(screenPosition, cursorType, cursorModifiers);
                userData->m_pressedCursorType = 0;
                userData->m_pressedCursorModifiers = 0;
                break;
            }
            default:
                break;
            }
        });
        glfwSetWindowPosCallback(window, [](GLFWwindow *window, int /* x */, int /* y */) {
            if (ImGuiViewport *viewport = ImGui::FindViewportByPlatformHandle(window)) {
                viewport->PlatformRequestMove = true;
            }
        });
        glfwSetWindowSizeCallback(window, [](GLFWwindow *window, int /* width */, int /* height */) {
            if (ImGuiViewport *viewport = ImGui::FindViewportByPlatformHandle(window)) {
                viewport->PlatformRequestResize = true;
            }
        });
        glfwSetWindowCloseCallback(window, [](GLFWwindow *window) {
            if (ImGuiViewport *viewport = ImGui::FindViewportByPlatformHandle(window)) {
                viewport->PlatformRequestClose = true;
            }
        });
        glfwMakeContextCurrent(window);
        glfwSwapInterval(0);
        m_context = sg::setup_context();
        m_window = window;
    }
}

void
ViewportWindow::destroyWindow()
{
    if (m_window != ImGui::GetMainViewport()->PlatformHandleRaw) {
        glfwMakeContextCurrent(m_window);
        sg::discard_context(m_context);
        glfwDestroyWindow(m_window);
        m_context = { SG_INVALID_ID };
        m_window = nullptr;
    }
}

void
ViewportWindow::showWindow()
{
    if (m_window) {
        glfwShowWindow(m_window);
    }
}

ImVec2
ViewportWindow::windowPos() const
{
    Vector2 pos(0);
    if (m_window) {
        Vector2SI32 windowPos;
        glfwGetWindowPos(m_window, &windowPos.x, &windowPos.y);
        pos = Vector2(windowPos);
    }
    return ImVec2(pos.x, pos.y);
}

void
ViewportWindow::setWindowPos(const ImVec2 &value)
{
    if (m_window) {
        glfwSetWindowPos(m_window, static_cast<int>(value.x), static_cast<int>(value.y));
    }
}

ImVec2
ViewportWindow::windowSize() const
{
    Vector2 size(1);
    if (m_window) {
        Vector2SI32 windowSize;
        glfwGetWindowSize(m_window, &windowSize.x, &windowSize.y);
        size = windowSize;
    }
    return ImVec2(size.x, size.y);
}

void
ViewportWindow::setWindowSize(const ImVec2 &value)
{
    if (m_window) {
        glfwSetWindowSize(m_window, static_cast<int>(value.x), static_cast<int>(value.y));
    }
}

bool
ViewportWindow::hasWindowFocus() const
{
    bool focused = false;
    focused = m_window ? glfwGetWindowAttrib(m_window, GLFW_FOCUSED) != 0 : false;
    return focused;
}

void
ViewportWindow::makeWindowFocus()
{
    if (m_window) {
        glfwFocusWindow(m_window);
    }
}

bool
ViewportWindow::isWindowMinimized() const
{
    bool minimized = false;
    minimized = m_window ? glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) != 0 : false;
    return minimized;
}

void
ViewportWindow::setWindowTitle(const char *value)
{
    if (m_window) {
        glfwSetWindowTitle(m_window, value);
    }
}

void
ViewportWindow::setWindowAlpha(float value)
{
    if (m_window) {
        glfwSetWindowOpacity(m_window, value);
    }
}

#endif /* IMGUI_HAS_VIEWPORT */

} /* namespace anonymous */

Vector2SI32
GLFWApplicationService::deviceScaleCursorPosition(double x, double y, GLFWwindow *window) noexcept
{
    Vector2SI32 windowPos;
    glfwGetWindowPos(window, &windowPos.x, &windowPos.y);
    return Vector2SI32(x, y) + windowPos;
}

Vector2
GLFWApplicationService::logicalScaleCursorPosition(double x, double y, GLFWwindow *window) noexcept
{
#if defined(__APPLE__)
    BX_UNUSED_1(window);
    return Vector2(x, y);
#else
    return Vector2(x, y) * invertedDevicePixelRatio(window);
#endif
}

Vector2
GLFWApplicationService::invertedDevicePixelRatio(GLFWwindow *window) noexcept
{
    Vector2 contentScale;
    glfwGetWindowContentScale(window, &contentScale.x, &contentScale.y);
    return Vector2(1) / contentScale;
}

int
GLFWApplicationService::convertCursorModifier(int value)
{
    int cursorModifiers = 0;
    if (EnumUtils::isEnabledT<int>(value, GLFW_MOD_ALT)) {
        cursorModifiers |= Project::kCursorModifierTypeAlt;
    }
    if (EnumUtils::isEnabledT<int>(value, GLFW_MOD_SHIFT)) {
        cursorModifiers |= Project::kCursorModifierTypeShift;
    }
    if (EnumUtils::isEnabledT<int>(value, GLFW_MOD_CONTROL) || EnumUtils::isEnabledT<int>(value, GLFW_MOD_SUPER)) {
        cursorModifiers |= Project::kCursorModifierTypeControl;
    }
    return cursorModifiers;
}

int
GLFWApplicationService::convertCursorType(int value)
{
    int cursorType = 0;
    switch (value) {
    case GLFW_MOUSE_BUTTON_LEFT: {
        cursorType = Project::kCursorTypeMouseLeft;
        break;
    }
    case GLFW_MOUSE_BUTTON_MIDDLE: {
        cursorType = Project::kCursorTypeMouseMiddle;
        break;
    }
    case GLFW_MOUSE_BUTTON_RIGHT: {
        cursorType = Project::kCursorTypeMouseRight;
        break;
    }
    case GLFW_MOUSE_BUTTON_4: {
        cursorType = Project::kCursorTypeMouseUndo;
        break;
    }
    case GLFW_MOUSE_BUTTON_5: {
        cursorType = Project::kCursorTypeMouseRedo;
        break;
    }
    default:
        break;
    }
    return cursorType;
}

GLFWApplicationService::GLFWApplicationService(const JSON_Value *config, GLFWApplicationClient::Bridge *bridge)
    : BaseApplicationService(config)
    , m_menubarApplciationClient(bridge)
    , m_bridge(bridge)
    , m_nativeView(nullptr)
{
}

GLFWApplicationService::~GLFWApplicationService()
{
}

void
GLFWApplicationService::draw()
{
    drawDefaultPass();
}

void
GLFWApplicationService::setNativeView(GLFWwindow *window)
{
    m_nativeView = window;
}

void
GLFWApplicationService::consumeDispatchAllCommandMessages()
{
    Project *project = projectHolder()->currentProject();
    ByteArrayList &commands = m_bridge->m_commands;
    ByteArrayList::iterator begin = commands.begin(), end = commands.end();
    for (ByteArrayList::const_iterator it = begin; it != end; ++it) {
        dispatchCommandMessage(it->data(), it->size(), project, false);
    }
    commands.erase(begin, end);
}

void
GLFWApplicationService::dispatchAllEventMessages()
{
    m_menubarApplciationClient.dispatchAllEventMessages();
}

BaseApplicationClient *
GLFWApplicationService::menubarApplicationClient()
{
    return &m_menubarApplciationClient;
}

Project::ISkinDeformerFactory *
GLFWApplicationService::createSkinDeformerFactory()
{
    Project::ISkinDeformerFactory *factory = nullptr;
    ApplicationPreference preference(this);
    if (preference.isSkinDeformAcceleratorEnabled()) {
        if (glfwGetProcAddress("glDispatchCompute")) {
            factory = nanoem_new(internal::OpenGLComputeShaderSkinDeformerFactory(glfwGetProcAddress));
        }
        else if (glfwGetProcAddress("glTransformFeedbackVaryings")) {
            factory = nanoem_new(internal::OpenGLTransformFeedbackSkinDeformerFactory(glfwGetProcAddress));
        }
    }
    return factory;
}

IAudioPlayer *
GLFWApplicationService::createAudioPlayer()
{
#if defined(NANOEM_HAS_SOUNDIO)
    return nanoem_new(SoundIOAudioPlayer);
#else
    return BaseApplicationService::createAudioPlayer();
#endif
}

bool
GLFWApplicationService::isRendererAvailable(const char *value) const noexcept
{
    return StringUtils::equals(value, kRendererOpenGL);
}

void
GLFWApplicationService::handleInitializeApplication()
{
    BaseApplicationService::handleInitializeApplication();
    ImGuiIO &io = ImGui::GetIO();
    io.ClipboardUserData = static_cast<GLFWwindow *>(m_nativeView);
    io.GetClipboardTextFn = [](void *userData) -> const char * {
        auto window = static_cast<GLFWwindow *>(userData);
        return glfwGetClipboardString(window);
    };
    io.SetClipboardTextFn = [](void *userData, const char *value) {
        auto window = static_cast<GLFWwindow *>(userData);
        glfwSetClipboardString(window, value);
    };
#if defined(IMGUI_HAS_VIEWPORT)
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
    ImGuiPlatformIO &platformIO = ImGui::GetPlatformIO();
    platformIO.Platform_CreateWindow = [](ImGuiViewport *viewport) {
        auto service = static_cast<GLFWApplicationService *>(ImGui::GetMainViewport()->PlatformHandle);
        ViewportWindow *userData = IM_NEW(ViewportWindow(service, service->m_bridge));
        userData->createWindow(viewport);
        viewport->PlatformUserData = userData;
        viewport->PlatformHandle = service;
        viewport->PlatformHandleRaw = userData->m_window;
        viewport->PlatformRequestResize = false;
    };
    platformIO.Platform_DestroyWindow = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        userData->destroyWindow();
        IM_DELETE(userData);
        viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
    };
    platformIO.Platform_ShowWindow = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        userData->showWindow();
    };
    platformIO.Platform_SetWindowPos = [](ImGuiViewport *viewport, ImVec2 pos) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        userData->setWindowPos(pos);
    };
    platformIO.Platform_GetWindowPos = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        return userData->windowPos();
    };
    platformIO.Platform_SetWindowSize = [](ImGuiViewport *viewport, ImVec2 size) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        userData->setWindowSize(size);
    };
    platformIO.Platform_GetWindowSize = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        return userData->windowSize();
    };
    platformIO.Platform_SetWindowFocus = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        userData->makeWindowFocus();
    };
    platformIO.Platform_GetWindowFocus = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        return userData->hasWindowFocus();
    };
    platformIO.Platform_GetWindowMinimized = [](ImGuiViewport *viewport) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        return userData->isWindowMinimized();
    };
    platformIO.Platform_SetWindowTitle = [](ImGuiViewport *viewport, const char *title) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        userData->setWindowTitle(title);
    };
    platformIO.Platform_SetWindowAlpha = [](ImGuiViewport *viewport, float alpha) {
        auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
        userData->setWindowAlpha(alpha);
    };
    ImGuiViewport *main = ImGui::GetMainViewport();
    auto view = static_cast<GLFWwindow *>(m_nativeView);
    main->PlatformHandle = this;
    main->PlatformHandleRaw = view;
    ViewportWindow *data = IM_NEW(ViewportWindow(this, m_bridge));
    data->m_window = view;
    main->PlatformUserData = data;
    const sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_GLCORE33) {
        io.BackendRendererUserData = this;
        platformIO.Renderer_RenderWindow = [](ImGuiViewport *viewport, void *opaque) {
            if (auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData)) {
                auto self = static_cast<GLFWApplicationService *>(ImGui::GetIO().BackendRendererUserData);
                if (Project *project = self->projectHolder()->currentProject()) {
                    auto window = static_cast<internal::ImGuiWindow *>(opaque);
                    bool load =
                        EnumUtils::isEnabledT<ImGuiViewportFlags>(viewport->Flags, ImGuiViewportFlags_NoRendererClear);
                    glfwMakeContextCurrent(userData->m_window);
                    sg::activate_context(userData->m_context);
                    window->drawWindow(project, viewport->DrawData, load);
                }
            }
        };
        platformIO.Renderer_SwapBuffers = [](ImGuiViewport *viewport, void *) {
            if (auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData)) {
                if (GLFWwindow *window = userData->m_window) {
                    glfwMakeContextCurrent(window);
                    glfwSwapBuffers(window);
                }
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
#endif /* IMGUI_HAS_VIEWPORT */
}

void
GLFWApplicationService::updateAllMonitors()
{
#if defined(IMGUI_HAS_VIEWPORT)
    int numMonitors;
    GLFWmonitor **monitors = glfwGetMonitors(&numMonitors);
    ImGuiPlatformIO &io = ImGui::GetPlatformIO();
    io.Monitors.resize(0);
    for (int i = 0; i < numMonitors; i++) {
        GLFWmonitor *monitor = monitors[i];
        ImGuiPlatformMonitor platformMonitor;
        int x, y, width, height;
        glfwGetMonitorPos(monitor, &x, &y);
        const GLFWvidmode *vidmode = glfwGetVideoMode(monitor);
        const Vector2 mainPos(x, y), mainSize(vidmode->width, vidmode->height);
        platformMonitor.MainPos = ImVec2(mainPos.x, mainPos.y);
        platformMonitor.MainSize = ImVec2(mainSize.x, mainSize.y);
        glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);
        const Vector2 workPos(x, y), workSize(width, height);
        platformMonitor.WorkPos = ImVec2(workPos.x, workPos.y);
        platformMonitor.WorkSize = ImVec2(workSize.x, workSize.y);
        float xscale, yscale;
        glfwGetMonitorContentScale(monitor, &xscale, &yscale);
        platformMonitor.DpiScale = xscale;
        io.Monitors.push_back(platformMonitor);
    }
#endif
}

void
GLFWApplicationService::sendEventMessage(const Nanoem__Application__Event *event)
{
    ByteArray bytes(sizeofEventMessage(event));
    packEventMessage(event, bytes.data());
    m_bridge->m_events.push_back(bytes);
}

void
GLFWApplicationService::presentDefaultPass(const Project *project)
{
    glfwMakeContextCurrent(m_nativeView);
    glfwSwapBuffers(m_nativeView);
}

} /* namespace glfw */
} /* namespace nanoem */
