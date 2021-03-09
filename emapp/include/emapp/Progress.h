/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_PROGRESS_H_
#define NANOEM_EMAPP_PROGRESS_H_

#include "emapp/Forward.h"

namespace nanoem {

class IEventPublisher;
class Progress;
class Project;
class URI;

class ICancelSubscriber {
public:
    virtual void onCancelled() = 0;
};

class ICancelPublisher {
public:
    virtual ~ICancelPublisher() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual void start() = 0;
    virtual void addSubscriber(ICancelSubscriber *subscriber) = 0;
    virtual void removeSubscriber(ICancelSubscriber *subscriber) = 0;
    virtual void stop() = 0;
};

class Progress NANOEM_DECL_SEALED : public ICancelSubscriber, private NonCopyable {
public:
    static const char *const kStreamURI;
    static const nanoem_u32_t kCancelToken;
    static const nanoem_u32_t kEventTypeItem;
    static const nanoem_u32_t kEventTypeText;
    static void requestCancel();

    Progress(Project *project, nanoem_u32_t total);
    Progress(Project *project, const char *title, const char *text, nanoem_u32_t total);
    ~Progress() NANOEM_DECL_NOEXCEPT;

    bool tryLoadingItem(const URI &fileURI);
    bool tryLoadingItem(const char *item);
    void setText(const char *value);
    void increment();
    void complete();

private:
    void onCancelled() NANOEM_DECL_OVERRIDE;

    Project *m_project;
    nanoem_u32_t m_value;
    nanoem_u32_t m_total;
    volatile bool m_cancelled;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_PROGRESS_H_ */
