/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODEL_LABEL_H_
#define NANOEM_EMAPP_MODEL_LABEL_H_

#include "emapp/Forward.h"
#include "emapp/model/RigidBody.h"

namespace nanoem {
namespace model {

class Label NANOEM_DECL_SEALED : private NonCopyable {
public:
    typedef tinystl::vector<const nanoem_model_label_t *, TinySTLAllocator> List;
    typedef tinystl::unordered_set<const nanoem_model_label_t *, TinySTLAllocator> Set;
    typedef tinystl::vector<nanoem_model_label_t *, TinySTLAllocator> MutableList;
    typedef tinystl::unordered_set<nanoem_model_label_t *, TinySTLAllocator> MutableSet;
    static const nanoem_u8_t kNameExpressionInJapanese[];

    ~Label() NANOEM_DECL_NOEXCEPT;

    void bind(nanoem_model_label_t *label);
    void resetLanguage(
        const nanoem_model_label_t *label, nanoem_unicode_string_factory_t *factory, nanoem_language_type_t language);
    static int index(const nanoem_model_label_t *labelPtr) NANOEM_DECL_NOEXCEPT;
    static const char *nameConstString(
        const nanoem_model_label_t *labelPtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT;
    static const char *canonicalNameConstString(
        const nanoem_model_label_t *labelPtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT;
    static Label *cast(const nanoem_model_label_t *labelPtr) NANOEM_DECL_NOEXCEPT;
    static Label *create();

    String name() const;
    String canonicalName() const;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT;
    const char *canonicalNameConstString() const NANOEM_DECL_NOEXCEPT;

private:
    struct PlaceHolder { };
    static void destroy(void *opaque, nanoem_model_object_t *morph) NANOEM_DECL_NOEXCEPT;
    Label(const PlaceHolder &holder) NANOEM_DECL_NOEXCEPT;

    String m_name;
    String m_canonicalName;
};

} /* namespace model */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODEL_LABEL_H_ */
