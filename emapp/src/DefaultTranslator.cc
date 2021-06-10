/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/DefaultTranslator.h"

#include "emapp/FileUtils.h"
#include "emapp/ResourceBundle.h"
#include "emapp/private/CommonInclude.h"

#include "./protoc/translation.pb-c.h"

namespace nanoem {

DefaultTranslator::DefaultTranslator()
    : m_language(kLanguageTypeMaxEnum)
{
}

DefaultTranslator::~DefaultTranslator() NANOEM_DECL_NOEXCEPT
{
}

void
DefaultTranslator::load()
{
    resources::loadBuiltInTranslation(this);
}

bool
DefaultTranslator::loadFromMemory(const nanoem_u8_t *ptr, size_t length)
{
    if (Nanoem__Translation__Bundle *bundle = nanoem__translation__bundle__unpack(g_protobufc_allocator, length, ptr)) {
        StringMap phrases;
        for (size_t i = 0, numUnits = bundle->n_units; i < numUnits; i++) {
            const Nanoem__Translation__Unit *unit = bundle->units[i];
            const bool match =
                (unit->language == NANOEM__COMMON__LANGUAGE__LC_JAPANESE && m_language == kLanguageTypeJapanese) ||
                (unit->language == NANOEM__COMMON__LANGUAGE__LC_ENGLISH && m_language == kLanguageTypeEnglish);
            if (match) {
                for (size_t j = 0, numPhrases = unit->n_phrases; j < numPhrases; j++) {
                    const Nanoem__Translation__Phrase *phrase = unit->phrases[j];
                    phrases.insert(tinystl::make_pair(String(phrase->id), String(phrase->text)));
                }
            }
        }
        nanoem__translation__bundle__free_unpacked(bundle, g_protobufc_allocator);
        m_phrases = phrases;
    }
    return m_message.empty();
}

const char *
DefaultTranslator::findPhrase(const char *text) const NANOEM_DECL_NOEXCEPT
{
    StringMap::const_iterator it = m_phrases.find(text);
    return it != m_phrases.end() ? it->second.c_str() : text;
}

const char *
DefaultTranslator::errorMessage() const NANOEM_DECL_NOEXCEPT
{
    return m_message.c_str();
}

const char *
DefaultTranslator::translate(const char *text) const NANOEM_DECL_NOEXCEPT
{
    return findPhrase(text);
}

bool
DefaultTranslator::isTranslatable(const char *text) const NANOEM_DECL_NOEXCEPT
{
    return m_phrases.find(text) != m_phrases.end();
}

ITranslator::LanguageType
DefaultTranslator::language() const NANOEM_DECL_NOEXCEPT
{
    return m_language;
}

void
DefaultTranslator::setLanguage(LanguageType value)
{
    if (value != m_language) {
        m_language = value;
        load();
    }
}

bool
DefaultTranslator::isSupportedLanguage(LanguageType value) const NANOEM_DECL_NOEXCEPT
{
    switch (value) {
    case kLanguageTypeJapanese:
    case kLanguageTypeEnglish:
        return true;
    default:
        return false;
    }
}

} /* namespace */
