/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_ERROR_H_
#define NANOEM_EMAPP_ERROR_H_

#include "emapp/Forward.h"

namespace nanoem {

class BaseApplicationService;
class IEventPublisher;
class IModalDialog;
class ITranslator;

class Error NANOEM_DECL_SEALED {
public:
    enum DomainType {
        kDomainTypeFirstEnum,
        kDomainTypeUnknown = kDomainTypeFirstEnum,
        kDomainTypeOS,
        kDomainTypeMinizip,
        kDomainTypeNanoem,
        kDomainTypeNanodxm,
        kDomainTypeNanomqo,
        kDomainTypeApplication,
        kDomainTypePlugin,
        kDomainTypeCancel = 0x7ffffffe,
        kDomainTypeMaxEnum,
    };
    static const int kMaxReasonLength = 7112;
    static const int kMaxRecoverySuggestionLength = 1016;

    static const char *convertStatusToMessage(
        nanoem_status_t status, const ITranslator *translator) NANOEM_DECL_NOEXCEPT;
    static const char *convertDomainToString(DomainType value) NANOEM_DECL_NOEXCEPT;
    static Error cancelled();

    explicit Error() NANOEM_DECL_NOEXCEPT;
    Error(const char *reason, int code, DomainType domain) NANOEM_DECL_NOEXCEPT;
    Error(const char *reason, const char *recoverySuggestion, DomainType domain) NANOEM_DECL_NOEXCEPT;
    Error(const Error &value) NANOEM_DECL_NOEXCEPT;
    ~Error() NANOEM_DECL_NOEXCEPT;

    IModalDialog *createModalDialog(BaseApplicationService *applicationPtr) const;
    void addModalDialog(BaseApplicationService *applicationPtr) const;
    void notify(IEventPublisher *publisher) const;

    bool hasReason() const NANOEM_DECL_NOEXCEPT;
    bool hasRecoverySuggestion() const NANOEM_DECL_NOEXCEPT;
    const char *reasonConstString() const NANOEM_DECL_NOEXCEPT;
    const char *recoverySuggestionConstString() const NANOEM_DECL_NOEXCEPT;
    DomainType domain() const NANOEM_DECL_NOEXCEPT;
    int code() const NANOEM_DECL_NOEXCEPT;
    bool isCancelled() const NANOEM_DECL_NOEXCEPT;

private:
    char m_reason[kMaxReasonLength];
    char m_recoverySuggestion[kMaxRecoverySuggestionLength];
    DomainType m_domain;
    int m_code;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_ERROR_H_ */
