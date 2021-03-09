/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/Progress.h"

#include "emapp/IEventPublisher.h"
#include "emapp/Project.h"
#include "emapp/URI.h"
#include "emapp/private/CommonInclude.h"

#include "protoc/application.pb-c.h"

#if defined(NANOEM_ENABLE_NANOMSG)
#define NN_STATIC_LIB
#include "nanomsg/nn.h"
#include "nanomsg/pubsub.h"

#include "emapp/StringUtils.h"
#include "emapp/ThreadedApplicationService.h"

#endif /* NANOEM_ENABLE_NANOMSG */

namespace nanoem {

const char *const Progress::kStreamURI = "inproc://nanoem-cancel-stream";
const nanoem_u32_t Progress::kCancelToken = 0xffffffff;
const nanoem_u32_t Progress::kEventTypeItem = NANOEM__APPLICATION__UPDATE_PROGRESS_EVENT__TYPE_ITEM;
const nanoem_u32_t Progress::kEventTypeText = NANOEM__APPLICATION__UPDATE_PROGRESS_EVENT__TYPE_TEXT;

void
Progress::requestCancel()
{
#if defined(NANOEM_ENABLE_NANOMSG)
    int socket = nn_socket(AF_SP, NN_PUB);
    if (socket == -1) {
        bx::debugPrintf("nn_socket: %s", nn_strerror(nn_errno()));
    }
    if (nn_connect(socket, kStreamURI) < 0) {
        bx::debugPrintf("nn_connect: %s", nn_strerror(nn_errno()));
    }
    nanoem_u32_t value = kCancelToken;
    int sent = nn_send(socket, &value, sizeof(value), 0);
    if (sent < 0) {
        bx::debugPrintf("nn_send: %s", nn_strerror(nn_errno()));
    }
    bx::sleep(100);
    nn_close(socket);
#endif
}

Progress::Progress(Project *project, nanoem_u32_t total)
    : m_project(project)
    , m_value(0)
    , m_total(total)
    , m_cancelled(false)
{
    const ITranslator *translator = project->translator();
    m_project->eventPublisher()->publishStartProgressEvent(translator->translate("nanoem.dialog.progress.load.title"),
        translator->translate("nanoem.dialog.progress.load.message"), total);
    if (ICancelPublisher *publisher = m_project->sharedCancelPublisherRepository()->cancelPublisher()) {
        publisher->addSubscriber(this);
    }
}

Progress::Progress(Project *project, const char *title, const char *text, nanoem_u32_t total)
    : m_project(project)
    , m_value(0)
    , m_total(total)
    , m_cancelled(false)
{
    m_project->eventPublisher()->publishStartProgressEvent(title, text, total);
    if (ICancelPublisher *publisher = m_project->sharedCancelPublisherRepository()->cancelPublisher()) {
        publisher->addSubscriber(this);
    }
}

Progress::~Progress() NANOEM_DECL_NOEXCEPT
{
    if (ICancelPublisher *publisher = m_project->sharedCancelPublisherRepository()->cancelPublisher()) {
        publisher->removeSubscriber(this);
    }
    m_project->eventPublisher()->publishStopProgressEvent();
    m_project->setCancelRequested(false);
}

bool
Progress::tryLoadingItem(const URI &fileURI)
{
    return tryLoadingItem(fileURI.hasFragment() ? URI::lastPathComponent(fileURI.fragment()).c_str()
                                                : fileURI.lastPathComponent().c_str());
}

bool
Progress::tryLoadingItem(const char *item)
{
    m_project->eventPublisher()->publishUpdateProgressEvent(glm::min(m_value, m_total), m_total, kEventTypeItem, item);
    return m_cancelled ? false : true;
}

void
Progress::setText(const char *value)
{
    m_project->eventPublisher()->publishUpdateProgressEvent(glm::min(m_value, m_total), m_total, kEventTypeText, value);
}

void
Progress::increment()
{
    m_project->eventPublisher()->publishUpdateProgressEvent(
        glm::min(++m_value, m_total), m_total, NANOEM__APPLICATION__UPDATE_PROGRESS_EVENT__TYPE__NOT_SET, "");
}

void
Progress::complete()
{
    m_project->eventPublisher()->publishUpdateProgressEvent(
        m_total, m_total, NANOEM__APPLICATION__UPDATE_PROGRESS_EVENT__TYPE__NOT_SET, "");
}

void
Progress::onCancelled()
{
    m_cancelled = true;
}

} /* namespace nanoem */
