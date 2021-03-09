/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_QT_QTTHREADEDAPPLICATIONSERVICE_H_
#define NANOEM_EMAPP_QT_QTTHREADEDAPPLICATIONSERVICE_H_

#include "emapp/BaseApplicationService.h"

#include "QtApplicationClient.h"
#include <QThread>

class QSemaphore;
class QOpenGLContext;
class QOpenGLFramebufferObject;
class QOffscreenSurface;

namespace nanoem {
namespace qt {

class QtApplicationService final : public QObject, public BaseApplicationService {
    Q_OBJECT
public:
    QtApplicationService(const JSON_Value *config, QtApplicationClient::Bridge *bridge);
    ~QtApplicationService() override;

    void draw();
    void consumeDispatchAllCommandMessages();
    void dispatchAllEventMessages();

private:
    void sendEventMessage(const Nanoem__Application__Event *event) override;

    QtApplicationClient::Bridge *m_bridge;
    QtApplicationClient m_menubarApplciationClient;
};

} /* namespace qt */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_QT_QTTHREADEDAPPLICATIONSERVICE_H_ */
