/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_GLFW_GLFWTHREADEDAPPLICATIONSERVICE_H_
#define NANOEM_EMAPP_GLFW_GLFWTHREADEDAPPLICATIONSERVICE_H_

#include "emapp/ThreadedApplicationService.h"

#include "emapp/ThreadedApplicationClient.h"
#include <bx/mutex.h>

#include <atomic>
#include <functional>
#include <vector>

struct GLFWwindow;

namespace bx {
class Semaphore;
}

namespace nanoem {

class IVideoRecorder;

namespace glfw {

class GLFWThreadedApplicationService final : public ThreadedApplicationService {
public:
    using Task = std::function<void()>;
    static Vector2SI32 devicePixelScreenPosition(GLFWwindow *window, const Vector2SI32 &value) noexcept;
    static Vector2 scaleCursorCoordinate(double x, double y, GLFWwindow *window) noexcept;
    static Vector2 invertedDevicePixelRatio(GLFWwindow *window) noexcept;
    static int convertCursorModifier(int value);
    static int convertCursorType(int value);

    GLFWThreadedApplicationService(const JSON_Value *config);
    ~GLFWThreadedApplicationService() override;

    void executeAllTasksOnMainThread();
    void addMainThreadTask(Task task);
    void requestViewportWindowClose(GLFWwindow *window);
    void requestViewportWindowMove(GLFWwindow *window);
    void requestViewportWindowResize(GLFWwindow *window);

    void acquireContextLock();
    void releaseContextLock();

private:
    using MainThreadTaskList = std::vector<Task>;

    BaseApplicationClient *menubarApplicationClient() override;
    Project::ISkinDeformerFactory *createSkinDeformerFactory() override;
    bool isRendererAvailable(const char *value) const noexcept override;

    void handleSetupGraphicsEngine(sg_desc &desc) override;
    void handleInitializeApplication() override;
    void handleDestructApplication() override;
    void postEmptyApplicationEvent() override;
    void beginDrawContext() override;
    void endDrawContext() override;
    void presentDefaultPass(const Project *project) override;
    URI recoverableRedoFileURI() const override;

    void updateAllMonitors();

    ThreadedApplicationClient m_menubarApplicationClient;
    MainThreadTaskList m_mainThreadTasks;
    bx::Mutex m_taskLock;
    bx::Mutex m_contextLock;
    std::atomic<GLFWwindow *> m_requestWindowClose;
    std::atomic<GLFWwindow *> m_requestWindowMove;
    std::atomic<GLFWwindow *> m_requestWindowResize;
};

} /* namespace glfw */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_GLFW_GLFWTHREADEDAPPLICATIONSERVICE_H_ */
