/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "MainWindow.h"
#include "GLFWApplicationClient.h"
#include "GLFWApplicationService.h"

#include "emapp/ApplicationPreference.h"
#include "emapp/DebugUtils.h"
#include "emapp/EnumUtils.h"
#include "emapp/Project.h"

#include "GL/gl3w.h"
#include "GLFW/glfw3.h"

#include "bx/os.h"

namespace nanoem {
namespace glfw {
namespace {

static const char kWindowTitle[] = "nanoem";

static void
buildFileFilter(const StringList &allowedExtensions, String &filter)
{
    size_t numItems = allowedExtensions.size();
    if (!allowedExtensions.empty()) {
        for (size_t i = 0, numItemsMinusOne = numItems - 1; i < numItemsMinusOne; i++) {
            filter.append(allowedExtensions[i].c_str());
            filter.append(",");
        }
        filter.append(allowedExtensions[numItems - 1].c_str());
        filter.append(";");
        for (size_t i = 0, numItemsMinusOne = numItems - 1; i < numItemsMinusOne; i++) {
            filter.append(allowedExtensions[i].c_str());
            filter.append(";");
        }
        filter.append(allowedExtensions[numItems - 1].c_str());
    }
}

} /* namespace anonymous */

MainWindow::MainWindow(GLFWApplicationService *service, GLFWApplicationClient *client)
    : m_service(service)
    , m_client(client)
{
    m_client->addDisableCursorEventListener(
        [](void *userData, const Vector2SI32 &logicalScaleCursorPosition) {
            BX_UNUSED_1(logicalScaleCursorPosition);
            auto self = static_cast<MainWindow *>(userData);
            glfwSetInputMode(self->m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        },
        this, false);
    m_client->addEnableCursorEventListener(
        [](void *userData, const Vector2SI32 &logicalScaleCursorPosition) {
            auto self = static_cast<MainWindow *>(userData);
            GLFWwindow *window = self->m_window;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            if (logicalScaleCursorPosition.x != 0 && logicalScaleCursorPosition.y != 0) {
                Vector2 contentScale;
                glfwGetWindowContentScale(window, &contentScale.x, &contentScale.y);
                const Vector2 deviceScaleCursorPosition(Vector2(logicalScaleCursorPosition) * contentScale);
                glfwSetCursorPos(window, deviceScaleCursorPosition.x, deviceScaleCursorPosition.y);
            }
        },
        this, false);
    m_client->addQuitApplicationEventListener(
        [](void *userData) {
            auto self = static_cast<MainWindow *>(userData);
            handleCloseCallback(self->m_window);
        },
        this, false);
}

MainWindow::~MainWindow()
{
}

bool
MainWindow::initialize()
{
    String pluginPath;
    float xscale, yscale;
    bool succeeded = setupWindow(pluginPath);
    if (succeeded) {
        m_service->setNativeView(m_window);
        glfwGetWindowContentScale(m_window, &xscale, &yscale);
        const BaseApplicationClient::InitializeMessageDescription desc(
            BaseApplicationService::minimumRequiredWindowSize(), SG_PIXELFORMAT_RGBA8, xscale, pluginPath.c_str());
        m_client->addInitializationCompleteEventListener(
            [](void *userData) {
                auto self = static_cast<MainWindow *>(userData);
                const JSON_Object *config = json_object(self->m_service->applicationConfiguration());
                const char *sentryDSN = nullptr;
#if defined(NANOEM_SENTRY_DSN)
                sentryDSN = NANOEM_SENTRY_DSN;
#endif
#if 0
                const BaseApplicationService::SentryDescription desc = { json_object_dotget_string(
                                                                             config, "glfw.sentry.library.path"),
                    sentryDSN, json_object_dotget_string(config, "glfw.sentry.handler.path"),
                    json_object_dotget_string(config, "glfw.sentry.database.path"), nullptr, /* m_redoFilePath */
                    nullptr, /* m_deviceModelName */
                    json_object_dotget_string(config, "project.locale"), nullptr, /* m_rendererName */
                    [](const char *value) { return sentry_value_new_string(value); } };
#endif
                const ApplicationPreference preference(self->m_service);
                BaseApplicationService::SentryDescription desc;
                desc.m_clientUUID = nullptr;
                desc.m_databaseDirectoryPath = json_object_dotget_string(config, "glfw.sentry.database.path");
                desc.m_deviceModelName = nullptr;
                desc.m_dllFilePath = json_object_dotget_string(config, "glfw.sentry.library.path");
                desc.m_dsn = sentryDSN;
                desc.m_handlerFilePath = json_object_dotget_string(config, "glfw.sentry.handler.path");
                desc.m_isModelEditingEnabled = preference.isModelEditingEnabled();
                desc.m_localeName = json_object_dotget_string(config, "project.locale");
                desc.m_maskString = [](const char *value) { return sentry_value_new_string(value); };
                desc.m_rendererName = nullptr;
                desc.m_transportSendEnvelope = nullptr;
                desc.m_transportUserData = nullptr;
                self->m_sentryDllHandle = BaseApplicationService::openSentryDll(desc);
                self->m_client->sendActivateMessage();
                glfwShowWindow(self->m_window);
            },
            this, true);
        m_client->sendInitializeMessage(desc);
    }
    return succeeded;
}

bool
MainWindow::isRunning() const
{
    return m_running;
}

void
MainWindow::processMessage()
{
    m_service->draw();
    m_service->consumeDispatchAllCommandMessages();
    m_service->dispatchAllEventMessages();
    m_client->consumeDispatchAllEventMessages();
    glfwPollEvents();
}

void
MainWindow::handleErrorCallback(int code, const char *message)
{
    DebugUtils::print("GLFW_ERROR(0x%x): %s", code, message);
}

void
MainWindow::handleResizeCallback(GLFWwindow *window, int width, int height)
{
    auto self = static_cast<MainWindow *>(glfwGetWindowUserPointer(window));
    self->m_client->sendResizeWindowMessage(
        glm::vec2(width, height) * GLFWApplicationService::invertedDevicePixelRatio(window));
}

void
MainWindow::handleCloseCallback(GLFWwindow *window)
{
    auto self = static_cast<MainWindow *>(glfwGetWindowUserPointer(window));
    self->m_client->sendIsProjectDirtyRequestMessage(
        [](void *userData, bool dirty) {
            auto self = static_cast<MainWindow *>(userData);
            if (dirty) {
                BaseApplicationClient *client = self->m_client;
                client->clearAllProjectAfterConfirmOnceEventListeners();
                client->addSaveProjectAfterConfirmEventListener(
                    [](void *userData) {
                        auto self = static_cast<MainWindow *>(userData);
                        self->m_client->addCompleteSavingFileEventListener(
                            [](void *userData, const URI & /* fileURI */, nanoem_u32_t /* type */,
                                nanoem_u64_t /* ticks */) {
                                auto self = static_cast<MainWindow *>(userData);
                                self->destroyWindow();
                            },
                            self, true);
                        self->m_client->sendSaveProjectMessage();
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
        self);
    glfwSetWindowShouldClose(window, GLFW_FALSE);
}

void
MainWindow::handleIconifyCallback(GLFWwindow *window, int value)
{
    auto self = static_cast<MainWindow *>(glfwGetWindowUserPointer(window));
    if (value) {
        self->m_client->sendDeactivateMessage();
    }
    else {
        self->m_client->sendActivateMessage();
    }
}

void
MainWindow::handleCursorCallback(GLFWwindow *window, double xpos, double ypos)
{
    auto self = static_cast<MainWindow *>(glfwGetWindowUserPointer(window));
    const Vector2SI32 position(GLFWApplicationService::logicalScaleCursorPosition(xpos, ypos, window)),
        delta(position - self->m_lastCursorPosition),
        screenPosition(GLFWApplicationService::deviceScaleCursorPosition(xpos, ypos, window));
    self->m_client->sendScreenCursorMoveMessage(
        screenPosition, self->m_pressedCursorType, self->m_pressedCursorModifiers);
    self->m_client->sendCursorMoveMessage(position, delta, self->m_pressedCursorType, self->m_pressedCursorModifiers);
    self->m_lastCursorPosition = position;
}

void
MainWindow::handleMouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    auto self = static_cast<MainWindow *>(glfwGetWindowUserPointer(window));
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    const Vector2SI32 position(GLFWApplicationService::logicalScaleCursorPosition(xpos, ypos, window)),
        screenPosition(GLFWApplicationService::deviceScaleCursorPosition(xpos, ypos, window));
    int cursorModifiers = GLFWApplicationService::convertCursorModifier(mods),
        cursorType = GLFWApplicationService::convertCursorType(button);
    switch (action) {
    case GLFW_PRESS: {
        self->m_client->sendScreenCursorPressMessage(screenPosition, cursorType, cursorModifiers);
        self->m_client->sendCursorPressMessage(position, cursorType, cursorModifiers);
        self->m_pressedCursorType = cursorType;
        self->m_pressedCursorModifiers = cursorModifiers;
        break;
    }
    case GLFW_RELEASE: {
        self->m_client->sendScreenCursorReleaseMessage(screenPosition, cursorType, cursorModifiers);
        self->m_client->sendCursorReleaseMessage(position, cursorType, cursorModifiers);
        self->m_pressedCursorType = 0;
        self->m_pressedCursorModifiers = 0;
        break;
    }
    default:
        break;
    }
    self->m_lastCursorPosition = position;
}

void
MainWindow::handleScrollCallback(GLFWwindow *window, double x, double y)
{
    auto self = static_cast<MainWindow *>(glfwGetWindowUserPointer(window));
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    const Vector2SI32 position(GLFWApplicationService::logicalScaleCursorPosition(xpos, ypos, window)), delta(x, y);
    self->m_client->sendCursorScrollMessage(position, delta, 0);
}

void
MainWindow::handleKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    BX_UNUSED_2(scancode, mods);
    auto self = static_cast<MainWindow *>(glfwGetWindowUserPointer(window));
    switch (action) {
    case GLFW_PRESS: {
        self->m_client->sendKeyPressMessage(static_cast<nanoem_u32_t>(key));
        break;
    }
    case GLFW_RELEASE: {
        self->m_client->sendKeyReleaseMessage(static_cast<nanoem_u32_t>(key));
        break;
    }
    default:
        break;
    }
}

void
MainWindow::handleCharCallback(GLFWwindow *window, uint32_t key)
{
    auto self = static_cast<MainWindow *>(glfwGetWindowUserPointer(window));
    self->m_client->sendUnicodeInputMessage(key);
}

void
MainWindow::handleDropCallback(GLFWwindow *window, int count, const char **filenames)
{
    auto self = static_cast<MainWindow *>(glfwGetWindowUserPointer(window));
    for (int i = 0; i < count; i++) {
        const char *filename = filenames[i];
        const URI fileURI(URI::createFromFilePath(filename));
        self->setTitle(fileURI);
        self->m_client->sendDropFileMessage(fileURI);
    }
}

bool
MainWindow::setupWindow(String &pluginPath)
{
    const JSON_Object *config = json_object(m_service->applicationConfiguration());
    pluginPath.append(json_object_dotget_string(config, "glfw.path"));
    glfwSetErrorCallback(handleErrorCallback);
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    int x, y, width, height;
    Vector2 contentScale;
    glfwGetMonitorContentScale(monitor, &contentScale.x, &contentScale.y);
    glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);
    const Vector2SI32 logicalPixelWindowSize(BaseApplicationService::minimumRequiredWindowSize()),
        devicePixelWindowSize(glm::vec2(logicalPixelWindowSize) * contentScale);
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
#if defined(__APPLE__)
    int windowPosX = x + (width - logicalPixelWindowSize.x) / 2,
        windowPosY = y + (height - logicalPixelWindowSize.y) / 2, windowWidth = logicalPixelWindowSize.x,
        windowHeight = logicalPixelWindowSize.y;
#else
    int windowPosX = x + (width - devicePixelWindowSize.x) / 2, windowPosY = y + (height - devicePixelWindowSize.y) / 2,
        windowWidth = devicePixelWindowSize.x, windowHeight = devicePixelWindowSize.y;
#endif
    glfwWindowHint(GLFW_VISIBLE, 0);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 0);
    m_window = glfwCreateWindow(windowWidth, windowHeight, kWindowTitle, nullptr, nullptr);
    if (!m_window) {
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 0);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
        m_window = glfwCreateWindow(windowWidth, windowHeight, kWindowTitle, nullptr, nullptr);
    }
    if (m_window) {
        pluginPath.append("/plugins/sokol_glcore33." BX_DL_EXT);
    }
    else {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        m_window = glfwCreateWindow(windowWidth, windowHeight, kWindowTitle, nullptr, nullptr);
        if (m_window) {
            pluginPath.append("/plugins/sokol_gles3." BX_DL_EXT);
        }
    }
    if (m_window) {
        glfwSetWindowPos(m_window, windowPosX, windowPosY);
        glfwSetWindowSizeLimits(m_window, windowWidth, windowHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);
        glfwSetWindowUserPointer(m_window, this);
        glfwSetWindowCloseCallback(m_window, handleCloseCallback);
        glfwSetWindowIconifyCallback(m_window, handleIconifyCallback);
        glfwSetFramebufferSizeCallback(m_window, handleResizeCallback);
        glfwSetCursorPosCallback(m_window, handleCursorCallback);
        glfwSetMouseButtonCallback(m_window, handleMouseButtonCallback);
        glfwSetScrollCallback(m_window, handleScrollCallback);
        glfwSetKeyCallback(m_window, handleKeyCallback);
        glfwSetCharCallback(m_window, handleCharCallback);
        glfwSetDropCallback(m_window, handleDropCallback);
        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1);
        gl3wInit();
    }
    return m_window != nullptr;
}

void
MainWindow::setTitle(const URI &fileURI)
{
    if (Project::isLoadableExtension(fileURI)) {
        String title;
        StringUtils::format(title, "%s - %s", fileURI.lastPathComponentConstString(), kWindowTitle);
        glfwSetWindowTitle(m_window, title.c_str());
    }
}

void
MainWindow::destroyWindow()
{
    m_client->addCompleteDestructionEventListener(
        [](void *userData) {
            auto self = static_cast<MainWindow *>(userData);
            self->m_client->addCompleteTerminationEventListener(
                [](void *userData) {
                    auto self = static_cast<MainWindow *>(userData);
                    if (auto handle = self->m_sentryDllHandle) {
                        BaseApplicationService::closeSentryDll(handle);
                        self->m_sentryDllHandle = nullptr;
                    }
                    self->m_running = false;
                    glfwDestroyWindow(self->m_window);
                },
                self, true);
            self->m_client->sendTerminateMessage();
        },
        this, true);
    m_client->sendDestroyMessage();
}

} /* namespace glfw */
} /* namespace nanoem */
