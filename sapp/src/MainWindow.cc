/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "MainWindow.h"

#include "bx/commandline.h"
#include "bx/os.h"
#include "emapp/StringUtils.h"

#include <dirent.h>

namespace nanoem {
namespace sapp {

MainWindow::MainWindow(const bx::CommandLine *cmd, const JSON_Value *root)
    : m_cmd(cmd)
    , m_client(&m_bridge)
    , m_service(root, &m_bridge)
    , m_lastCursorPosition(0)
    , m_movingCursorPosition(0)
{
}

MainWindow::~MainWindow() noexcept
{
}

void
MainWindow::initialize()
{
    String sokolPath(json_object_dotget_string(json_object(m_service.applicationConfiguration()), "sapp.plugin.path"));
#if defined(_WIN32)
    sokolPath.append("/sokol_d3d11." BX_DL_EXT);
    const sg_pixel_format colorFormat = SG_PIXELFORMAT_BGRA8;
#elif defined(__APPLE__)
    sokolPath.append("/sokol_metal_macos." BX_DL_EXT);
    const sg_pixel_format colorFormat = SG_PIXELFORMAT_BGRA8;
#else
    sokolPath.append("/sokol_glcore33." BX_DL_EXT);
    const sg_pixel_format colorFormat = SG_PIXELFORMAT_RGBA8;
#endif
    m_client.addInitializationCompleteEventListener(
        [](void *userData) {
            auto self = static_cast<MainWindow *>(userData);
            self->loadAllPlugins();
#if defined(NANOEM_ENABLE_DEBUG_LABEL)
            auto cmd = self->m_cmd;
            if (cmd->hasArg("bootstrap-project")) {
                const char *path = cmd->findOption("bootstrap-project");
                const URI fileURI(URI::createFromFilePath(path));
                self->m_client.sendLoadFileMessage(fileURI, IFileManager::kDialogTypeOpenProject);
            }
#endif /* NANOEM_ENABLE_DEBUG_LABEL */
            self->m_client.sendActivateMessage();
        },
        this, true);
    m_client.addQuitApplicationEventListener(
        [](void *userData) {
            auto self = static_cast<MainWindow *>(userData);
            self->handleQuitRequest();
        },
        this, false);
    const Vector2UI32 logicalWindowSize(Vector2(sapp_width(), sapp_height()) / Vector2(sapp_dpi_scale()));
    const BaseApplicationClient::InitializeMessageDescription desc(
        logicalWindowSize, colorFormat, sapp_dpi_scale(), sokolPath.c_str());
    m_client.sendInitializeMessage(desc);
}

void
MainWindow::draw()
{
    m_service.draw();
    m_service.consumeDispatchAllCommandMessages();
    m_service.dispatchAllEventMessages();
    m_client.consumeDispatchAllEventMessages();
}

void
MainWindow::handleMouseDown(const Vector2SI32 &position, sapp_mousebutton button, uint32_t modifiers)
{
    if (button != SAPP_MOUSEBUTTON_INVALID) {
        m_client.sendCursorPressMessage(position, button, modifiers);
        m_lastCursorPosition = position;
        m_currentPressedMouseButton = button;
    }
}

void
MainWindow::handleMouseMove(const Vector2SI32 &position, uint32_t modifiers)
{
    Vector2SI32 delta(0);
    if (m_currentPressedMouseButton != SAPP_MOUSEBUTTON_INVALID) {
        delta = position - m_movingCursorPosition;
    }
    m_client.sendCursorMoveMessage(position, delta, 0, modifiers);
    m_movingCursorPosition = position;
    if (m_currentPressedMouseButton != SAPP_MOUSEBUTTON_INVALID) {
        m_lastCursorPosition = position;
    }
}

void
MainWindow::handleMouseUp(const Vector2SI32 &position, sapp_mousebutton button, uint32_t modifiers)
{
    if (button != SAPP_MOUSEBUTTON_INVALID) {
        m_client.sendCursorReleaseMessage(position, button, modifiers);
        m_lastCursorPosition = Vector2SI32(0);
        m_currentPressedMouseButton = SAPP_MOUSEBUTTON_INVALID;
    }
}

void
MainWindow::handleMouseScroll(float deltaY, uint32_t modifiers)
{
    m_client.sendCursorScrollMessage(m_movingCursorPosition, Vector2SI32(0, deltaY), modifiers);
}

void
MainWindow::handleKeyDown(sapp_keycode keycode)
{
    m_client.sendKeyPressMessage(keycode);
}

void
MainWindow::handleKeyUp(sapp_keycode keycode)
{
    m_client.sendKeyReleaseMessage(keycode);
}

void
MainWindow::handleChar(uint32_t value)
{
    m_client.sendUnicodeInputMessage(value);
}

void
MainWindow::handleResize(const Vector2UI32 &value)
{
    m_client.sendResizeWindowMessage(value);
}

void
MainWindow::handleIconify()
{
    m_client.sendDeactivateMessage();
}

void
MainWindow::handleSuspend()
{
    m_client.sendDeactivateMessage();
}

void
MainWindow::handleRestore()
{
    m_client.sendActivateMessage();
}

void
MainWindow::handleResume()
{
    m_client.sendActivateMessage();
}

void
MainWindow::handleFileDrop(const char *path)
{
    m_client.sendDropFileMessage(URI::createFromFilePath(path));
}

void
MainWindow::handleQuitRequest()
{
    m_client.sendIsProjectDirtyRequestMessage(
        [](void *userData, bool dirty) {
            auto self = static_cast<MainWindow *>(userData);
            if (dirty) {
                BaseApplicationClient &client = self->m_client;
                client.clearAllProjectAfterConfirmOnceEventListeners();
                client.addSaveProjectAfterConfirmEventListener(
                    [](void *userData) {
                        auto self = static_cast<MainWindow *>(userData);
                        self->m_client.addCompleteSavingFileEventListener(
                            [](void *userData, const URI & /* fileURI */, nanoem_u32_t /* type */,
                                nanoem_u64_t /* ticks */) {
                                auto self = static_cast<MainWindow *>(userData);
                                self->destroyWindow();
                            },
                            self, true);
                        self->m_client.sendSaveProjectMessage();
                    },
                    self, true);
                client.addDiscardProjectAfterConfirmEventListener(
                    [](void *userData) {
                        auto self = static_cast<MainWindow *>(userData);
                        self->destroyWindow();
                    },
                    self, true);
                client.sendConfirmBeforeExitApplicationMessage();
            }
            else {
                self->destroyWindow();
            }
        },
        this);
    sapp_cancel_quit();
}

void
MainWindow::loadAllPlugins()
{
    const JSON_Object *config = json_object(m_service.applicationConfiguration());
    const char *pluginDirPath = json_object_dotget_string(config, "sapp.plugin.path");
    URIList pluginURIs;
    if (DIR *dir = ::opendir(pluginDirPath)) {
        while (dirent *ent = ::readdir(dir)) {
            if (StringUtils::equals(ent->d_name, "plugin", 6)) {
                String pluginPath(pluginDirPath);
                pluginPath.append("/");
                pluginPath.append(ent->d_name);
                pluginURIs.push_back(URI::createFromFilePath(pluginPath));
            }
        }
        ::closedir(dir);
    }
    m_client.sendLoadAllDecoderPluginsMessage(pluginURIs);
    m_client.sendLoadAllEncoderPluginsMessage(pluginURIs);
    m_client.sendLoadAllModelPluginsMessage(pluginURIs);
    m_client.sendLoadAllMotionPluginsMessage(pluginURIs);
}

void
MainWindow::destroyWindow()
{
    m_client.addCompleteDestructionEventListener(
        [](void *userData) {
            auto self = static_cast<MainWindow *>(userData);
            self->m_client.addCompleteTerminationEventListener([](void * /* userData */) { sapp_quit(); }, self, true);
            self->m_client.sendTerminateMessage();
        },
        this, true);
    m_client.sendDestroyMessage();
}

} /* namespace sapp */
} /* namespace nanoem */
