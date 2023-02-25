/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_DEFAULTTRANSLATOR_H_
#define NANOEM_EMAPP_DEFAULTTRANSLATOR_H_

#include "emapp/Forward.h"
#include "emapp/ITranslator.h"

namespace nanoem {

class DefaultTranslator NANOEM_DECL_SEALED : public ITranslator, private NonCopyable {
public:
    DefaultTranslator();
    ~DefaultTranslator() NANOEM_DECL_NOEXCEPT;

    void load();
    bool loadFromMemory(const nanoem_u8_t *ptr, size_t length);
    const char *findPhrase(const char *text) const NANOEM_DECL_NOEXCEPT;
    const char *errorMessage() const NANOEM_DECL_NOEXCEPT;

    const char *translate(const char *text) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isTranslatable(const char *text) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    LanguageType language() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setLanguage(LanguageType value) NANOEM_DECL_OVERRIDE;
    bool isSupportedLanguage(LanguageType value) const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    StringMap m_phrases;
    String m_message;
    LanguageType m_language;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_DEFAULTTRANSLATOR_H_ */
