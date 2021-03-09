/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/Allocator.h"
#include "emapp/emapp.h"

#include "MainWindow.h"
#include "bx/commandline.h"

using namespace nanoem;

namespace {

static bx::CommandLine *s_command;
static sapp::MainWindow *s_window;

static inline Vector2SI32
cursorPosition(const sapp_event *event)
{
    const nanoem_f32_t scale = (1.0f / sapp_dpi_scale());
    return Vector2SI32(Vector2(event->mouse_x, event->mouse_y) * scale);
}

} /* namespace anonymous */

sapp_desc
sokol_main(int argc, char *argv[])
{
    s_command = new bx::CommandLine(argc, argv);
    sapp_desc desc = {};
    desc.init_cb = []() {
        Allocator::initialize();
        BaseApplicationService::setup();
        s_window = new sapp::MainWindow();
        s_window->initialize();
    };
    desc.event_cb = [](const sapp_event *event) {
        switch (event->type) {
        case SAPP_EVENTTYPE_MOUSE_DOWN: {
            s_window->handleMouseDown(cursorPosition(event), event->mouse_button, event->modifiers);
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_MOVE: {
            s_window->handleMouseMove(cursorPosition(event), event->mouse_button, event->modifiers);
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_UP: {
            s_window->handleMouseUp(cursorPosition(event), event->mouse_button, event->modifiers);
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_SCROLL: {
            s_window->handleMouseScroll(event->scroll_y, event->modifiers);
            break;
        }
        case SAPP_EVENTTYPE_KEY_DOWN: {
            s_window->handleKeyDown(event->key_code);
            break;
        }
        case SAPP_EVENTTYPE_KEY_UP: {
            s_window->handleKeyUp(event->key_code);
            break;
        }
        case SAPP_EVENTTYPE_CHAR: {
            s_window->handleChar(event->char_code);
            break;
        }
        case SAPP_EVENTTYPE_RESIZED: {
            const nanoem_f32_t scale = (1.0f / sapp_dpi_scale());
            const Vector2UI32 size(Vector2(event->framebuffer_width, event->framebuffer_height) * scale);
            s_window->handleResize(size);
            break;
        }
        case SAPP_EVENTTYPE_ICONIFIED: {
            s_window->handleIconify();
            break;
        }
        case SAPP_EVENTTYPE_SUSPENDED: {
            s_window->handleSuspend();
            break;
        }
        case SAPP_EVENTTYPE_RESTORED: {
            s_window->handleRestore();
            break;
        }
        case SAPP_EVENTTYPE_RESUMED: {
            s_window->handleResume();
            break;
        }
        case SAPP_EVENTTYPE_FILES_DROPPED: {
            const char *path = sapp_get_dropped_file_path(0);
            s_window->handleFileDrop(path);
            break;
        }
        case SAPP_EVENTTYPE_QUIT_REQUESTED: {
            s_window->handleQuitRequest();
            break;
        }
        default:
            break;
        }
    };
    desc.frame_cb = []() { s_window->draw(); };
    desc.cleanup_cb = []() {
        delete s_window;
        s_window = nullptr;
        delete s_command;
        s_command = nullptr;
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
