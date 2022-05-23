/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODEL_MORPH_H_
#define NANOEM_EMAPP_MODEL_MORPH_H_

#include "emapp/Forward.h"

namespace nanoem {

class Motion;

namespace model {

class Morph NANOEM_DECL_SEALED : private NonCopyable {
public:
    typedef tinystl::vector<const nanoem_model_morph_t *, TinySTLAllocator> List;
    typedef tinystl::unordered_set<const nanoem_model_morph_t *, TinySTLAllocator> Set;
    typedef tinystl::vector<nanoem_model_morph_t *, TinySTLAllocator> MutableList;
    typedef tinystl::unordered_set<nanoem_model_morph_t *, TinySTLAllocator> MutableSet;

    ~Morph() NANOEM_DECL_NOEXCEPT;

    void bind(nanoem_model_morph_t *morph);
    void resetLanguage(
        const nanoem_model_morph_t *morph, nanoem_unicode_string_factory_t *factory, nanoem_language_type_t language);
    void reset() NANOEM_DECL_NOEXCEPT;
    void synchronizeMotion(const Motion *motion, const nanoem_unicode_string_t *name, nanoem_frame_index_t frameIndex,
        nanoem_f32_t amount);

    static int index(const nanoem_model_morph_t *morphPtr) NANOEM_DECL_NOEXCEPT;
    static const char *nameConstString(
        const nanoem_model_morph_t *morphPtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT;
    static const char *canonicalNameConstString(
        const nanoem_model_morph_t *morphPtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT;
    static Morph *cast(const nanoem_model_morph_t *morphPtr) NANOEM_DECL_NOEXCEPT;
    static Morph *create();

    String name() const;
    String canonicalName() const;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT;
    const char *canonicalNameConstString() const NANOEM_DECL_NOEXCEPT;
    bool isDirty() const NANOEM_DECL_NOEXCEPT;
    bool isDirty(const nanoem_model_morph_t *morph) const NANOEM_DECL_NOEXCEPT;
    void setDirty(bool value);
    nanoem_f32_t weight() const NANOEM_DECL_NOEXCEPT;
    void setWeight(nanoem_f32_t value);
    void setForcedWeight(nanoem_f32_t value);

private:
    struct PlaceHolder { };
    static void destroy(void *opaque, nanoem_model_object_t *morph) NANOEM_DECL_NOEXCEPT;
    static void synchronizeWeight(const Motion *motion, nanoem_frame_index_t frameIndex,
        const nanoem_unicode_string_t *name, nanoem_f32_t &weight);
    Morph(const PlaceHolder &holder) NANOEM_DECL_NOEXCEPT;

    String m_name;
    String m_canonicalName;
    nanoem_f32_t m_weight;
    bool m_dirty;
};

} /* namespace model */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODEL_MORPH_H_ */
