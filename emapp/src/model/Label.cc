/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/model/Label.h"

#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace model {

Label::~Label() NANOEM_DECL_NOEXCEPT
{
}

void
Label::bind(nanoem_model_label_t *label)
{
    nanoem_parameter_assert(label, "must not be nullptr");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_user_data_t *userData = nanoemUserDataCreate(&status);
    nanoemUserDataSetOnDestroyModelObjectCallback(userData, &Label::destroy);
    nanoemUserDataSetOpaqueData(userData, this);
    nanoemModelObjectSetUserData(nanoemModelLabelGetModelObjectMutable(label), userData);
}

void
Label::resetLanguage(
    const nanoem_model_label_t *label, nanoem_unicode_string_factory_t *factory, nanoem_language_type_t language)
{
    StringUtils::getUtf8String(nanoemModelLabelGetName(label, language), factory, m_name);
    if (m_canonicalName.empty()) {
        StringUtils::getUtf8String(
            nanoemModelLabelGetName(label, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), factory, m_canonicalName);
        if (m_canonicalName.empty()) {
            StringUtils::format(m_canonicalName, "Label%d", index(label));
        }
    }
    if (m_name.empty()) {
        m_name = m_canonicalName;
    }
}

int
Label::index(const nanoem_model_label_t *labelPtr) NANOEM_DECL_NOEXCEPT
{
    return nanoemModelObjectGetIndex(nanoemModelLabelGetModelObject(labelPtr));
}

const char *
Label::nameConstString(const nanoem_model_label_t *labelPtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT
{
    const Label *label = cast(labelPtr);
    return label ? label->nameConstString() : placeHolder;
}

const char *
Label::canonicalNameConstString(
    const nanoem_model_label_t* labelPtr, const char* placeHolder) NANOEM_DECL_NOEXCEPT
{
    const Label *label = cast(labelPtr);
    return label ? label->canonicalNameConstString() : placeHolder;
}

Label *
Label::cast(const nanoem_model_label_t *labelPtr) NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_object_t *object = nanoemModelLabelGetModelObject(labelPtr);
    const nanoem_user_data_t *userData = nanoemModelObjectGetUserData(object);
    return static_cast<Label *>(nanoemUserDataGetOpaqueData(userData));
}

Label *
Label::create()
{
    static const PlaceHolder holder = {};
    return nanoem_new(Label(holder));
}

String
Label::name() const
{
    return m_name;
}

String
Label::canonicalName() const
{
    return m_canonicalName;
}

const char *
Label::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_name.c_str();
}

const char *
Label::canonicalNameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_canonicalName.c_str();
}

void
Label::destroy(void *opaque, nanoem_model_object_t * /* label */) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(opaque, "must not be nullptr");
    Label *self = static_cast<Label *>(opaque);
    nanoem_delete(self);
}

Label::Label(const PlaceHolder & /* holder */) NANOEM_DECL_NOEXCEPT
{
}

} /* namespace model */
} /* namespace nanoem */
