/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/DirectionalLight.h"

#include "emapp/Constants.h"
#include "emapp/Motion.h"
#include "emapp/PerspectiveCamera.h"
#include "emapp/Project.h"
#include "emapp/private/CommonInclude.h"

#include "glm/gtx/vector_query.hpp"

#include "undo/undo.h"

namespace nanoem {

const Vector3 DirectionalLight::kInitialColor = Vector3(154.0f / 255.0f);
const Vector3 DirectionalLight::kInitialDirection = Vector3(-0.5f, -1.0f, 0.5f);

DirectionalLight::DirectionalLight(Project *project)
    : m_project(project)
    , m_undoStack(nullptr)
    , m_color(kInitialColor)
    , m_direction(kInitialDirection)
    , m_translucent(false)
    , m_dirty(false)
{
    nanoem_parameter_assert(m_project, "must NOT be nullptr");
    m_undoStack = undoStackCreateWithSoftLimit(undoStackGetSoftLimit(m_project->undoStack()));
    nanoem_assert(m_undoStack, "must NOT be nullptr");
}

DirectionalLight::~DirectionalLight() NANOEM_DECL_NOEXCEPT
{
    undoStackDestroy(m_undoStack);
    m_undoStack = nullptr;
}

void
DirectionalLight::destroy() NANOEM_DECL_NOEXCEPT
{
    undoStackClear(m_undoStack);
}

void
DirectionalLight::reset() NANOEM_DECL_NOEXCEPT
{
    m_color = kInitialColor;
    m_direction = kInitialDirection;
}

void
DirectionalLight::synchronizeParameters(const Motion *motion, const nanoem_frame_index_t frameIndex)
{
    nanoem_parameter_assert(motion, "must not be nullptr");
    if (const nanoem_motion_light_keyframe_t *keyframe = motion->findLightKeyframe(frameIndex)) {
        setColor(glm::make_vec3(nanoemMotionLightKeyframeGetColor(keyframe)));
        setDirection(glm::make_vec3(nanoemMotionLightKeyframeGetDirection(keyframe)));
    }
    else {
        nanoem_motion_light_keyframe_t *prevKeyframe, *nextKeyframe;
        nanoemMotionSearchClosestLightKeyframes(motion->data(), frameIndex, &prevKeyframe, &nextKeyframe);
        if (prevKeyframe && nextKeyframe) {
            const nanoem_f32_t &coef = Motion::coefficient(prevKeyframe, nextKeyframe, frameIndex);
            const Vector3 color0(glm::make_vec3(nanoemMotionLightKeyframeGetColor(prevKeyframe)));
            const Vector3 color1(glm::make_vec3(nanoemMotionLightKeyframeGetColor(nextKeyframe)));
            const Vector3 direction0(glm::make_vec3(nanoemMotionLightKeyframeGetDirection(prevKeyframe)));
            const Vector3 direction1(glm::make_vec3(nanoemMotionLightKeyframeGetDirection(nextKeyframe)));
            setColor(glm::mix(color0, color1, coef));
            setDirection(glm::mix(direction0, direction1, coef));
        }
    }
}

void
DirectionalLight::getShadowTransform(Matrix4x4 &value) const
{
    const Vector3 position(-m_direction);
    nanoem_f32_t dot = glm::dot(position, Constants::kUnitY);
    const Vector4 m1(Constants::kUnitX * dot - position.x * Constants::kUnitY, 0);
    const Vector4 m2(Constants::kUnitY * dot - position.y * Constants::kUnitY, 0);
    const Vector4 m3(Constants::kUnitZ * dot - position.z * Constants::kUnitY, 0);
    const Vector4 m4(Constants::kZeroV3, dot);
    value = glm::transpose(Matrix4x4(m1, m2, m3, m4));
}

const Project *
DirectionalLight::project() const NANOEM_DECL_NOEXCEPT
{
    return m_project;
}

Project *
DirectionalLight::project() NANOEM_DECL_NOEXCEPT
{
    return m_project;
}

Vector3
DirectionalLight::color() const NANOEM_DECL_NOEXCEPT
{
    return m_color;
}

void
DirectionalLight::setColor(const Vector3 &value)
{
    m_color = value;
    setDirty(true);
}

Vector3
DirectionalLight::direction() const NANOEM_DECL_NOEXCEPT
{
    return m_direction;
}

void
DirectionalLight::setDirection(const Vector3 &value)
{
    m_direction = !glm::isNull(value, Constants::kEpsilon) ? value : kInitialDirection;
    setDirty(true);
}

Vector3
DirectionalLight::groundShadowColor() const NANOEM_DECL_NOEXCEPT
{
    return color();
}

bool
DirectionalLight::isTranslucentGroundShadowEnabled() const NANOEM_DECL_NOEXCEPT
{
    return m_translucent;
}

void
DirectionalLight::setTranslucentGroundShadowEnabled(bool value)
{
    m_translucent = value;
}

bool
DirectionalLight::isDirty() const NANOEM_DECL_NOEXCEPT
{
    return m_dirty;
}

void
DirectionalLight::setDirty(bool value)
{
    m_dirty = value;
}

} /* namespace nanoem */
