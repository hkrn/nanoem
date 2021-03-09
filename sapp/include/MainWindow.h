/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_SAPP_MAINWINDOW_H_
#define NANOEM_EMAPP_SAPP_MAINWINDOW_H_

#include "ApplicationClient.h"
#include "ApplicationService.h"

#include "sokol/sokol_app.h"

namespace bx {
class CommandLine;
}

namespace nanoem {
namespace sapp {

class MainWindow {
public:
    MainWindow();
    ~MainWindow() noexcept;

    void initialize();
    void draw();
    void handleMouseDown(const Vector2SI32 &position, sapp_mousebutton button, uint32_t modifiers);
    void handleMouseMove(const Vector2SI32 &position, sapp_mousebutton button, uint32_t modifiers);
    void handleMouseUp(const Vector2SI32 &position, sapp_mousebutton button, uint32_t modifiers);
    void handleMouseScroll(float deltaY, uint32_t modifiers);
    void handleKeyDown(sapp_keycode keycode);
    void handleKeyUp(sapp_keycode keycode);
    void handleChar(uint32_t value);
    void handleResize(const Vector2UI32 &value);
    void handleIconify();
    void handleSuspend();
    void handleRestore();
    void handleResume();
    void handleFileDrop(const char *path);
    void handleQuitRequest();

private:
    void destroyWindow();

    JSON_Value *m_root;
    ApplicationClient::Bridge m_bridge;
    ApplicationClient m_client;
    ApplicationService m_service;
    Vector2SI32 m_lastCursorPosition;
    Vector2SI32 m_movingCursorPosition;
};

} /* namespace sapp */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_SAPP_MAINWINDOW_H_ */
