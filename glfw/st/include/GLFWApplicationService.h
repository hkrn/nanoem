/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_GLFW_GLFWAPPLICATIONSERVICE_H_
#define NANOEM_EMAPP_GLFW_GLFWAPPLICATIONSERVICE_H_

#include "emapp/BaseApplicationService.h"

#include "GLFWApplicationClient.h"

struct GLFWwindow;

namespace nanoem {

class IVideoRecorder;

namespace glfw {

class GLFWApplicationService final : public BaseApplicationService {
public:
    static Vector2SI32 deviceScaleCursorPosition(double x, double y, GLFWwindow *window) noexcept;
    static Vector2 logicalScaleCursorPosition(double x, double y, GLFWwindow *window) noexcept;
    static Vector2 invertedDevicePixelRatio(GLFWwindow *window) noexcept;
    static int convertCursorModifier(int value);
    static int convertCursorType(int value);

    GLFWApplicationService(const JSON_Value *config, GLFWApplicationClient::Bridge *bridge);
    ~GLFWApplicationService() override;

    void draw();
    void setNativeView(GLFWwindow *window);
    void consumeDispatchAllCommandMessages();
    void dispatchAllEventMessages();

private:
    BaseApplicationClient *menubarApplicationClient() override;
    Project::ISkinDeformerFactory *createSkinDeformerFactory() override;
    IAudioPlayer *createAudioPlayer() override;
    bool isRendererAvailable(const char *value) const noexcept override;
    void handleInitializeApplication() override;
    void sendEventMessage(const Nanoem__Application__Event *event) override;
    void presentDefaultPass(const Project *project) override;

    void updateAllMonitors();

    GLFWApplicationClient m_menubarApplciationClient;
    GLFWApplicationClient::Bridge *m_bridge;
    GLFWwindow *m_nativeView;
};

} /* namespace glfw */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_GLFW_GLFWAPPLICATIONSERVICE_H_ */
