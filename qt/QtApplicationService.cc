/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "QtApplicationService.h"

#include "emapp/IProjectHolder.h"
#include "emapp/private/CommonInclude.h"

#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QSemaphore>
#include <QSurface>
#include <QWindow>

namespace nanoem {
namespace qt {

QtApplicationService::QtApplicationService(const JSON_Value *config, QtApplicationClient::Bridge *bridge)
    : QObject()
    , BaseApplicationService(config)
    , m_bridge(bridge)
    , m_menubarApplciationClient(bridge)
{
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(3, 3);
    QSurfaceFormat::setDefaultFormat(format);
}

QtApplicationService::~QtApplicationService()
{
}

void
QtApplicationService::draw()
{
    drawDefaultPass();
    if (Project *project = projectHolder()->currentProject()) {
        project->resetAllPasses();
    }
}

void
QtApplicationService::consumeDispatchAllCommandMessages()
{
    Project *project = projectHolder()->currentProject();
    ByteArrayList &commands = m_bridge->m_commands;
    for (ByteArrayList::const_iterator it = commands.begin(), end = commands.end(); it != end; ++it) {
        dispatchCommandMessage(it->data(), it->size(), project, false);
    }
    commands.clear();
}

void
QtApplicationService::dispatchAllEventMessages()
{
    m_menubarApplciationClient.dispatchAllEventMessages();
}

void
QtApplicationService::sendEventMessage(const Nanoem__Application__Event *event)
{
    ByteArray bytes(sizeofEventMessage(event));
    packEventMessage(event, bytes.data());
    m_bridge->m_events.push_back(bytes);
}

} /* namespace qt */
} /* namespace nanoem */
