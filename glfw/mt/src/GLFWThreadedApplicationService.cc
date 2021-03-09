/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "GLFWThreadedApplicationService.h"

#include "OpenGLComputeShaderSkinDeformerFactory.h"
#include "OpenGLTransformFeedbackSkinDeformerFactory.h"

#include "emapp/ApplicationPreference.h"
#include "emapp/EnumUtils.h"
#include "emapp/IProjectHolder.h"
#include "emapp/StringUtils.h"
#include "emapp/ThreadedApplicationClient.h"
#include "emapp/internal/ImGuiWindow.h"
#include "emapp/private/CommonInclude.h"

#include "GLFW/glfw3.h"
#include "imgui/imgui.h"

namespace nanoem {
namespace glfw {
namespace {

#if defined(IMGUI_HAS_VIEWPORT) && IMGUI_HAS_VIEWPORT
struct ViewportWindow {
    ViewportWindow(GLFWThreadedApplicationService *parent);
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

    GLFWThreadedApplicationService *m_service = nullptr;
    GLFWwindow *m_window = nullptr;
    ThreadedApplicationClient m_client;
    sg_context m_context = { SG_INVALID_ID };
    int m_pressedCursorType = 0;
    int m_pressedCursorModifiers = 0;
};

ViewportWindow::ViewportWindow(GLFWThreadedApplicationService *parent)
    : m_service(parent)
{
}

ViewportWindow::~ViewportWindow()
{
}

void
ViewportWindow::createWindow(ImGuiViewport *viewport)
{
    BX_UNUSED_1(viewport);
    bx::Semaphore sema;
    m_service->addMainThreadTask([this, viewport, &sema] {
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
        m_service->acquireContextLock();
        if (GLFWwindow *window = glfwCreateWindow(windowSize.x, windowSize.y, "Untitled", nullptr, parentWindow)) {
            glfwSetWindowUserPointer(window, viewport);
            const Vector2SI32 windowPos(viewport->Pos.x, viewport->Pos.y);
            glfwSetWindowPos(window, windowPos.x, windowPos.y);
            glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xpos, double ypos) {
                auto viewport = static_cast<ImGuiViewport *>(glfwGetWindowUserPointer(window));
                auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
                const Vector2SI32 position(xpos, ypos),
                    screenPosition(GLFWThreadedApplicationService::devicePixelScreenPosition(window, position));
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
                    screenPosition(GLFWThreadedApplicationService::devicePixelScreenPosition(window, position));
                int cursorModifiers = GLFWThreadedApplicationService::convertCursorModifier(mods),
                    cursorType = GLFWThreadedApplicationService::convertCursorType(button);
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
                auto viewport = static_cast<ImGuiViewport *>(glfwGetWindowUserPointer(window));
                auto service = static_cast<GLFWThreadedApplicationService *>(viewport->PlatformHandle);
                service->requestViewportWindowMove(window);
            });
            glfwSetWindowSizeCallback(window, [](GLFWwindow *window, int /* width */, int /* height */) {
                auto viewport = static_cast<ImGuiViewport *>(glfwGetWindowUserPointer(window));
                auto service = static_cast<GLFWThreadedApplicationService *>(viewport->PlatformHandle);
                service->requestViewportWindowResize(window);
            });
            glfwSetWindowCloseCallback(window, [](GLFWwindow *window) {
                auto viewport = static_cast<ImGuiViewport *>(glfwGetWindowUserPointer(window));
                auto service = static_cast<GLFWThreadedApplicationService *>(viewport->PlatformHandle);
                service->requestViewportWindowClose(window);
            });
            glfwMakeContextCurrent(window);
            glfwSwapInterval(0);
            m_client.connect();
            m_context = sg::setup_context();
            m_window = window;
        }
        m_service->releaseContextLock();
        sema.post();
    });
    m_service->releaseContextLock();
    sema.wait();
    m_service->acquireContextLock();
}

void
ViewportWindow::destroyWindow()
{
    if (m_window != ImGui::GetMainViewport()->PlatformHandleRaw) {
        bx::Semaphore sema;
        m_service->addMainThreadTask([this, &sema] {
            if (m_window) {
                glfwMakeContextCurrent(m_window);
                sg::discard_context(m_context);
                glfwDestroyWindow(m_window);
                m_client.close();
                m_context = { SG_INVALID_ID };
                m_window = nullptr;
            }
            sema.post();
        });
        m_service->releaseContextLock();
        sema.wait();
        m_service->acquireContextLock();
    }
}

void
ViewportWindow::showWindow()
{
    bx::Semaphore sema;
    m_service->addMainThreadTask([this, &sema] {
        if (m_window) {
            glfwShowWindow(m_window);
        }
        sema.post();
    });
    sema.wait();
}

ImVec2
ViewportWindow::windowPos() const
{
    ImVec2 pos;
    bx::Semaphore sema;
    m_service->addMainThreadTask([this, &pos, &sema] {
        if (m_window) {
            int x, y;
            glfwGetWindowPos(m_window, &x, &y);
            pos.x = static_cast<float>(x);
            pos.y = static_cast<float>(y);
        }
        sema.post();
    });
    sema.wait();
    return pos;
}

void
ViewportWindow::setWindowPos(const ImVec2 &value)
{
    bx::Semaphore sema;
    m_service->addMainThreadTask([this, value, &sema] {
        if (m_window) {
            glfwSetWindowPos(m_window, static_cast<int>(value.x), static_cast<int>(value.y));
        }
        sema.post();
    });
    sema.wait();
}

ImVec2
ViewportWindow::windowSize() const
{
    ImVec2 size;
    bx::Semaphore sema;
    m_service->addMainThreadTask([this, &size, &sema] {
        if (m_window) {
            int width, height;
            glfwGetWindowSize(m_window, &width, &height);
            size.x = static_cast<float>(width);
            size.y = static_cast<float>(height);
        }
        sema.post();
    });
    sema.wait();
    return size;
}

void
ViewportWindow::setWindowSize(const ImVec2 &value)
{
    bx::Semaphore sema;
    m_service->addMainThreadTask([this, value, &sema] {
        if (m_window) {
            glfwSetWindowSize(m_window, static_cast<int>(value.x), static_cast<int>(value.y));
        }
        sema.post();
    });
    sema.wait();
}

bool
ViewportWindow::hasWindowFocus() const
{
    bool focused = false;
    bx::Semaphore sema;
    m_service->addMainThreadTask([this, &focused, &sema] {
        focused = m_window ? glfwGetWindowAttrib(m_window, GLFW_FOCUSED) != 0 : false;
        sema.post();
    });
    sema.wait();
    return focused;
}

void
ViewportWindow::makeWindowFocus()
{
    bx::Semaphore sema;
    m_service->addMainThreadTask([this, &sema]() {
        if (m_window) {
            glfwFocusWindow(m_window);
        }
        sema.post();
    });
    sema.wait();
}

bool
ViewportWindow::isWindowMinimized() const
{
    bool minimized = false;
    bx::Semaphore sema;
    m_service->addMainThreadTask([this, &minimized, &sema]() {
        minimized = m_window ? glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) != 0 : false;
        sema.post();
    });
    sema.wait();
    return minimized;
}

void
ViewportWindow::setWindowTitle(const char *value)
{
    const String title(value);
    bx::Semaphore sema;
    m_service->addMainThreadTask([this, title, &sema]() {
        if (m_window) {
            glfwSetWindowTitle(m_window, title.c_str());
        }
        sema.post();
    });
    sema.wait();
}

void
ViewportWindow::setWindowAlpha(float value)
{
    bx::Semaphore sema;
    m_service->addMainThreadTask([this, value, &sema]() {
        if (m_window) {
            glfwSetWindowOpacity(m_window, value);
        }
        sema.post();
    });
    sema.wait();
}

#endif /* IMGUI_HAS_VIEWPORT */

} /* namespace anonymous */

Vector2SI32
GLFWThreadedApplicationService::devicePixelScreenPosition(GLFWwindow *window, const Vector2SI32 &value) noexcept
{
    Vector2SI32 windowPosition;
    Vector2 contentScale;
    glfwGetWindowPos(window, &windowPosition.x, &windowPosition.y);
    glfwGetWindowContentScale(window, &contentScale.x, &contentScale.y);
    return Vector2(value + windowPosition) * contentScale;
}

Vector2
GLFWThreadedApplicationService::scaleCursorCoordinate(double x, double y, GLFWwindow *window) noexcept
{
#if defined(__APPLE__)
    BX_UNUSED_1(window);
    return Vector2(x, y);
#else
    return Vector2(x, y) * invertedDevicePixelRatio(window);
#endif
}

Vector2
GLFWThreadedApplicationService::invertedDevicePixelRatio(GLFWwindow *window) noexcept
{
    Vector2 contentScale;
    glfwGetWindowContentScale(window, &contentScale.x, &contentScale.y);
    return Vector2(1) / contentScale;
}

int
GLFWThreadedApplicationService::convertCursorModifier(int value)
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
GLFWThreadedApplicationService::convertCursorType(int value)
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

GLFWThreadedApplicationService::GLFWThreadedApplicationService(const JSON_Value *config)
    : ThreadedApplicationService(config)
{
}

GLFWThreadedApplicationService::~GLFWThreadedApplicationService()
{
}

void
GLFWThreadedApplicationService::executeAllTasksOnMainThread()
{
    bx::MutexScope scope(m_taskLock);
    BX_UNUSED_1(scope);
    if (!m_mainThreadTasks.empty()) {
        for (auto task : m_mainThreadTasks) {
            task();
        }
        m_mainThreadTasks.clear();
    }
}

void
GLFWThreadedApplicationService::addMainThreadTask(Task task)
{
    bx::MutexScope scope(m_taskLock);
    BX_UNUSED_1(scope);
    m_mainThreadTasks.push_back(task);
    glfwPostEmptyEvent();
}

void
GLFWThreadedApplicationService::requestViewportWindowClose(GLFWwindow *window)
{
    m_requestWindowClose = window;
}

void
GLFWThreadedApplicationService::requestViewportWindowMove(GLFWwindow *window)
{
    m_requestWindowMove = window;
}

void
GLFWThreadedApplicationService::requestViewportWindowResize(GLFWwindow *window)
{
    m_requestWindowResize = window;
}

void
GLFWThreadedApplicationService::acquireContextLock()
{
    m_contextLock.lock();
    glfwMakeContextCurrent(static_cast<GLFWwindow *>(m_nativeView));
}

void
GLFWThreadedApplicationService::releaseContextLock()
{
    glfwMakeContextCurrent(nullptr);
    m_contextLock.unlock();
}

BaseApplicationClient *
GLFWThreadedApplicationService::menubarApplicationClient()
{
    return &m_menubarApplicationClient;
}

Project::ISkinDeformerFactory *
GLFWThreadedApplicationService::createSkinDeformerFactory()
{
    Project::ISkinDeformerFactory *factory = nullptr;
#if !defined(__APPLE__)
    ApplicationPreference preference(this);
    if (preference.isSkinDeformAcceleratorEnabled()) {
        if (glDispatchCompute) {
            factory = nanoem_new(OpenGLComputeShaderSkinDeformerFactory);
        }
        else {
            factory = nanoem_new(OpenGLTransformFeedbackSkinDeformerFactory);
        }
    }
#endif
    return factory;
}

bool
GLFWThreadedApplicationService::isRendererAvailable(const char *value) const noexcept
{
    return StringUtils::equals(value, kRendererOpenGL);
}

void
GLFWThreadedApplicationService::handleSetupGraphicsEngine(sg_desc &desc)
{
    ThreadedApplicationService::handleSetupGraphicsEngine(desc);
    m_menubarApplicationClient.connect();
}

void
GLFWThreadedApplicationService::handleInitializeApplication()
{
    ThreadedApplicationService::handleInitializeApplication();
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
#if defined(IMGUI_HAS_VIEWPORT) && IMGUI_HAS_VIEWPORT
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
    ImGuiPlatformIO &platformIO = ImGui::GetPlatformIO();
    platformIO.Platform_CreateWindow = [](ImGuiViewport *viewport) {
        auto service = static_cast<GLFWThreadedApplicationService *>(ImGui::GetMainViewport()->PlatformHandle);
        ViewportWindow *userData = IM_NEW(ViewportWindow(service));
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
        userData->setWindowPos(size);
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
    ViewportWindow *data = IM_NEW(ViewportWindow(this));
    data->m_window = view;
    main->PlatformUserData = data;
    const sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_GLCORE33) {
        io.BackendRendererUserData = this;
        platformIO.Renderer_RenderWindow = [](ImGuiViewport *viewport, void *opaque) {
            auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData);
            if (userData && userData->m_window) {
                auto self = static_cast<GLFWThreadedApplicationService *>(ImGui::GetIO().BackendRendererUserData);
                if (Project *project = self->projectHolder()->currentProject()) {
                    auto window = static_cast<internal::ImGuiWindow *>(opaque);
                    bool load =
                        EnumUtils::isEnabledT<ImGuiViewportFlags>(viewport->Flags, ImGuiViewportFlags_NoRendererClear);
                    self->releaseContextLock();
                    glfwMakeContextCurrent(userData->m_window);
                    sg::activate_context(userData->m_context);
                    window->drawWindow(project, viewport->DrawData, load);
                    self->acquireContextLock();
                }
                BX_UNUSED_1(opaque);
            }
        };
        platformIO.Renderer_SwapBuffers = [](ImGuiViewport *viewport, void *) {
            if (auto userData = static_cast<ViewportWindow *>(viewport->PlatformUserData)) {
                if (GLFWwindow *window = userData->m_window) {
                    auto self = static_cast<GLFWThreadedApplicationService *>(ImGui::GetIO().BackendRendererUserData);
                    self->releaseContextLock();
                    glfwMakeContextCurrent(window);
                    glfwSwapBuffers(window);
                    self->acquireContextLock();
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
    m_menubarApplicationClient.connect();
}

void
GLFWThreadedApplicationService::handleDestructApplication()
{
    m_menubarApplicationClient.close();
}

void
GLFWThreadedApplicationService::postEmptyApplicationEvent()
{
    glfwPostEmptyEvent();
}

void
GLFWThreadedApplicationService::beginDrawContext()
{
    acquireContextLock();
    ThreadedApplicationService::beginDrawContext();
}

void
GLFWThreadedApplicationService::endDrawContext()
{
    releaseContextLock();
    ThreadedApplicationService::endDrawContext();
}

void
GLFWThreadedApplicationService::presentDefaultPass(const Project * /* project */)
{
    GLFWwindow *window = static_cast<GLFWwindow *>(m_nativeView);
#if defined(IMGUI_HAS_VIEWPORT) && IMGUI_HAS_VIEWPORT
    bx::MutexScope scope(m_contextLock);
    BX_UNUSED_1(scope);
    if (GLFWwindow *window = m_requestWindowClose.exchange(nullptr)) {
        if (ImGuiViewport *viewport = ImGui::FindViewportByPlatformHandle(window)) {
            viewport->PlatformRequestClose = true;
        }
    }
    if (GLFWwindow *window = m_requestWindowMove.exchange(nullptr)) {
        if (ImGuiViewport *viewport = ImGui::FindViewportByPlatformHandle(window)) {
            viewport->PlatformRequestMove = true;
        }
    }
    if (GLFWwindow *window = m_requestWindowResize.exchange(nullptr)) {
        if (ImGuiViewport *viewport = ImGui::FindViewportByPlatformHandle(window)) {
            viewport->PlatformRequestResize = true;
        }
    }
#endif
    glfwSwapBuffers(window);
    m_menubarApplicationClient.receiveAllEventMessages();
}

URI
GLFWThreadedApplicationService::recoverableRedoFileURI() const
{
    return URI();
}

void
GLFWThreadedApplicationService::updateAllMonitors()
{
#if defined(IMGUI_HAS_VIEWPORT) && IMGUI_HAS_VIEWPORT
    bx::Semaphore sema;
    addMainThreadTask([&sema]() {
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
        sema.post();
    });
    sema.wait();
#endif
}

} /* namespace glfw */
} /* namespace nanoem */
