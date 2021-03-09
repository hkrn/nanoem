/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/model/Morph.h"

#include "emapp/Constants.h"
#include "emapp/Motion.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace model {

Morph::~Morph() NANOEM_DECL_NOEXCEPT
{
}

void
Morph::bind(nanoem_model_morph_t *morph)
{
    nanoem_parameter_assert(morph, "must not be nullptr");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_user_data_t *userData = nanoemUserDataCreate(&status);
    nanoemUserDataSetOnDestroyModelObjectCallback(userData, &Morph::destroy);
    nanoemUserDataSetOpaqueData(userData, this);
    nanoemModelObjectSetUserData(nanoemModelMorphGetModelObjectMutable(morph), userData);
}

void
Morph::resetLanguage(
    const nanoem_model_morph_t *morph, nanoem_unicode_string_factory_t *factory, nanoem_language_type_t language)
{
    StringUtils::getUtf8String(nanoemModelMorphGetName(morph, language), factory, m_name);
    if (m_canonicalName.empty()) {
        StringUtils::getUtf8String(
            nanoemModelMorphGetName(morph, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), factory, m_canonicalName);
        if (m_canonicalName.empty()) {
            StringUtils::format(
                m_canonicalName, "Morph%d", nanoemModelObjectGetIndex(nanoemModelMorphGetModelObject(morph)));
        }
    }
    if (m_name.empty()) {
        m_name = m_canonicalName;
    }
}

void
Morph::reset() NANOEM_DECL_NOEXCEPT
{
    m_weight = 0;
    m_dirty = false;
}

void
Morph::synchronizeMotion(
    const Motion *motion, const nanoem_unicode_string_t *name, nanoem_frame_index_t frameIndex, nanoem_f32_t amount)
{
    nanoem_parameter_assert(name, "must not be nullptr");
    nanoem_f32_t w0, w1;
    synchronizeWeight(motion, frameIndex, name, w0);
    if (amount > 0) {
        synchronizeWeight(motion, frameIndex + 1, name, w1);
        setWeight(glm::mix(w0, w1, amount));
    }
    else {
        setWeight(w0);
    }
}

Morph *
Morph::cast(const nanoem_model_morph_t *morph) NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_object_t *object = nanoemModelMorphGetModelObject(morph);
    const nanoem_user_data_t *userData = nanoemModelObjectGetUserData(object);
    return static_cast<Morph *>(nanoemUserDataGetOpaqueData(userData));
}

Morph *
Morph::create()
{
    static const PlaceHolder holder = {};
    return nanoem_new(Morph(holder));
}

String
Morph::name() const
{
    return m_name;
}

String
Morph::canonicalName() const
{
    return m_canonicalName;
}

const char *
Morph::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_name.c_str();
}

const char *
Morph::canonicalNameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_canonicalName.c_str();
}

bool
Morph::isDirty() const NANOEM_DECL_NOEXCEPT
{
    return m_dirty;
}

bool
Morph::isDirty(const nanoem_model_morph_t *morph) const NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(morph, "must not be nullptr");
    return nanoemModelMorphGetCategory(morph) != NANOEM_MODEL_MORPH_CATEGORY_BASE && isDirty();
}

void
Morph::setDirty(bool value)
{
    m_dirty = value;
}

nanoem_f32_t
Morph::weight() const NANOEM_DECL_NOEXCEPT
{
    return m_weight;
}

void
Morph::setWeight(nanoem_f32_t value)
{
    m_dirty = glm::abs(m_weight) > Constants::kEpsilon || glm::abs(value) > Constants::kEpsilon;
    m_weight = value;
}

void
Morph::setForcedWeight(nanoem_f32_t value)
{
    m_dirty = false;
    m_weight = value;
}

void
Morph::destroy(void *opaque, nanoem_model_object_t * /* morph */) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(opaque, "must not be nullptr");
    Morph *self = static_cast<Morph *>(opaque);
    nanoem_delete(self);
}

void
Morph::synchronizeWeight(
    const Motion *motion, nanoem_frame_index_t frameIndex, const nanoem_unicode_string_t *name, nanoem_f32_t &weight)
{
    nanoem_parameter_assert(name, "must not be nullptr");
    if (const nanoem_motion_morph_keyframe_t *keyframe = motion->findMorphKeyframe(name, frameIndex)) {
        weight = nanoemMotionMorphKeyframeGetWeight(keyframe);
    }
    else {
        nanoem_motion_morph_keyframe_t *prevKeyframe, *nextKeyframe;
        nanoemMotionSearchClosestMorphKeyframes(motion->data(), name, frameIndex, &prevKeyframe, &nextKeyframe);
        if (prevKeyframe && nextKeyframe) {
            const nanoem_f32_t &coef = Motion::coefficient(prevKeyframe, nextKeyframe, frameIndex);
            weight = glm::mix(nanoemMotionMorphKeyframeGetWeight(prevKeyframe),
                nanoemMotionMorphKeyframeGetWeight(nextKeyframe), coef);
        }
        else {
            weight = 0.0f;
        }
    }
}

Morph::Morph(const PlaceHolder & /* holder */) NANOEM_DECL_NOEXCEPT : m_weight(0), m_dirty(false)
{
}

} /* namespace model */
} /* namespace nanoem */
