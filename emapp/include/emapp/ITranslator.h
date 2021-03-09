/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_ITRANSLATOR_H_
#define NANOEM_EMAPP_ITRANSLATOR_H_

#include "emapp/Forward.h"

namespace nanoem {

class ITranslator {
public:
    enum LanguageType {
        kLanguageTypeFirstEnum,
        kLanguageTypeJapanese = kLanguageTypeFirstEnum,
        kLanguageTypeEnglish,
        kLanguageTypeChineseSimplified,
        kLanguageTypeChineseTraditional,
        kLanguageTypeKorean,
        kLanguageTypeMaxEnum
    };
    virtual ~ITranslator() NANOEM_DECL_NOEXCEPT
    {
    }

    virtual const char *translate(const char *text) const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool isTranslatable(const char *text) const NANOEM_DECL_NOEXCEPT = 0;

    virtual LanguageType language() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setLanguage(LanguageType value) = 0;
    virtual bool isSupportedLanguage(LanguageType value) const NANOEM_DECL_NOEXCEPT = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_ITRANSLATOR_H_ */
