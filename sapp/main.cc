/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/Allocator.h"
#include "emapp/emapp.h"

#include "MainWindow.h"
#include "bx/commandline.h"
#include "whereami.h"

using namespace nanoem;

namespace {

class ApplicationState NANOEM_DECL_SEALED : private NonCopyable {
public:
    ApplicationState(int argc, char *argv[]);
    ~ApplicationState();

    void initialize();

    sapp::MainWindow *window() NANOEM_DECL_NOEXCEPT;

private:
    JSON_Value *m_config;
    sapp::MainWindow *m_window;
    bx::CommandLine m_command;
};

ApplicationState::ApplicationState(int argc, char *argv[])
    : m_config(nullptr)
    , m_window(nullptr)
    , m_command(argc, argv)
{
}

ApplicationState::~ApplicationState()
{
    delete m_window;
    m_window = nullptr;
    json_value_free(m_config);
    m_config = nullptr;
}

void
ApplicationState::initialize()
{
    m_config = json_value_init_object();
    JSON_Object *root = json_value_get_object(m_config);
    char buffer[1024] = { 0 };
    int length;
    wai_getExecutablePath(buffer, sizeof(buffer), &length);
    MutableString ms(buffer, buffer + length);
    ms.push_back(0);
    FileUtils::canonicalizePathSeparator(ms);
    String pluginPath(ms.data());
    pluginPath.append("/plugins");
    String effectPluginPath(pluginPath);
    effectPluginPath.append("/plugin_effect." BX_DL_EXT);
    json_object_dotset_string(root, "sapp.executable.path", buffer);
    json_object_dotset_string(root, "sapp.plugin.path", pluginPath.c_str());
    json_object_dotset_string(root, "plugin.effect.path", effectPluginPath.c_str());
    m_window = new sapp::MainWindow(m_config);
    m_window->initialize();
}

sapp::MainWindow *
ApplicationState::window() NANOEM_DECL_NOEXCEPT
{
    return m_window;
}

static inline nanoem_f32_t
inverseDevicePixelRatio() NANOEM_DECL_NOEXCEPT
{
    return 1.0f / glm::max(sapp_dpi_scale(), 1.0f);
}

static inline Vector2SI32
logicalCursorPosition(const sapp_event *event) NANOEM_DECL_NOEXCEPT
{
    return Vector2SI32(Vector2(event->mouse_x, event->mouse_y) * inverseDevicePixelRatio());
}

} /* namespace anonymous */

sapp_desc
sokol_main(int argc, char *argv[])
{
    Allocator::initialize();
    auto state = new ApplicationState(argc, argv);
    sapp_desc desc = {};
    desc.user_data = state;
    desc.init_userdata_cb = [](void *userData) {
        auto state = static_cast<ApplicationState *>(userData);
        BaseApplicationService::setup();
        state->initialize();
    };
    desc.event_userdata_cb = [](const sapp_event *event, void *userData) {
        auto state = static_cast<ApplicationState *>(userData);
        auto window = state->window();
        switch (event->type) {
        case SAPP_EVENTTYPE_MOUSE_DOWN: {
            window->handleMouseDown(logicalCursorPosition(event), event->mouse_button, event->modifiers);
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_MOVE: {
            window->handleMouseMove(logicalCursorPosition(event), event->modifiers);
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_UP: {
            window->handleMouseUp(logicalCursorPosition(event), event->mouse_button, event->modifiers);
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_SCROLL: {
            window->handleMouseScroll(event->scroll_y, event->modifiers);
            break;
        }
        case SAPP_EVENTTYPE_KEY_DOWN: {
            window->handleKeyDown(event->key_code);
            break;
        }
        case SAPP_EVENTTYPE_KEY_UP: {
            window->handleKeyUp(event->key_code);
            break;
        }
        case SAPP_EVENTTYPE_CHAR: {
            window->handleChar(event->char_code);
            break;
        }
        case SAPP_EVENTTYPE_RESIZED: {
            const Vector2UI32 size(
                Vector2(event->framebuffer_width, event->framebuffer_height) * inverseDevicePixelRatio());
            window->handleResize(size);
            break;
        }
        case SAPP_EVENTTYPE_ICONIFIED: {
            window->handleIconify();
            break;
        }
        case SAPP_EVENTTYPE_SUSPENDED: {
            window->handleSuspend();
            break;
        }
        case SAPP_EVENTTYPE_RESTORED: {
            window->handleRestore();
            break;
        }
        case SAPP_EVENTTYPE_RESUMED: {
            window->handleResume();
            break;
        }
        case SAPP_EVENTTYPE_FILES_DROPPED: {
            const char *path = sapp_get_dropped_file_path(0);
            window->handleFileDrop(path);
            break;
        }
        case SAPP_EVENTTYPE_QUIT_REQUESTED: {
            window->handleQuitRequest();
            break;
        }
        default:
            break;
        }
    };
    desc.frame_userdata_cb = [](void *userData) {
        auto state = static_cast<ApplicationState *>(userData);
        auto window = state->window();
        window->draw();
    };
    desc.cleanup_userdata_cb = [](void *userData) {
        auto state = static_cast<ApplicationState *>(userData);
        delete state;
        Allocator::destroy();
    };
    const Vector2UI16 size(BaseApplicationService::minimumRequiredWindowSize());
    desc.width = size.x;
    desc.height = size.y;
    desc.window_title = "nanoem";
    desc.html5_canvas_resize = true;
    desc.html5_premultiplied_alpha = true;
    desc.html5_ask_leave_site = true;
    desc.high_dpi = true;
    desc.enable_dragndrop = true;
    return desc;
}
