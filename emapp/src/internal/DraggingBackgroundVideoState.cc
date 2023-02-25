/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/DraggingBackgroundVideoState.h"

#include "emapp/IEventPublisher.h"
#include "emapp/Project.h"

namespace nanoem {
namespace internal {

const nanoem_f32_t ZoomBackgroundVideoState::kDefaultScaleFactor = 0.001f;

DraggingBackgroundVideoState::DraggingBackgroundVideoState(Project *project, const Vector2SI32 &pressedCursorPosition)
    : m_project(project)
    , m_pressedCursorPosition(pressedCursorPosition)
    , m_scaleFactor(1.0f)
{
    nanoem_parameter_assert(m_project, "must NOT be NULL");
    m_project->eventPublisher()->publishDisableCursorEvent(pressedCursorPosition);
}

void
DraggingBackgroundVideoState::commit(const Vector2SI32 & /* logicalScalePosition */)
{
    m_project->eventPublisher()->publishEnableCursorEvent(Vector2SI32(0));
}

const char *
DraggingBackgroundVideoState::name() const NANOEM_DECL_NOEXCEPT
{
    return "nanoem.gui.viewport.translation.video";
}

nanoem_f32_t
DraggingBackgroundVideoState::scaleFactor() const NANOEM_DECL_NOEXCEPT
{
    return m_scaleFactor;
}

void
DraggingBackgroundVideoState::setScaleFactor(nanoem_f32_t value)
{
    m_scaleFactor = value;
}

Vector2SI32
DraggingBackgroundVideoState::pressedCursorPosition() const NANOEM_DECL_NOEXCEPT
{
    return m_pressedCursorPosition;
}

Project *
DraggingBackgroundVideoState::project() NANOEM_DECL_NOEXCEPT
{
    return m_project;
}

Vector4SI32
DraggingBackgroundVideoState::normalizeBackgroundVideoRect() const NANOEM_DECL_NOEXCEPT
{
    Vector4SI32 rect(m_project->backgroundVideoRect());
    if (rect.z == 0 && rect.w == 0) {
        const Vector4SI32 &viewport = m_project->deviceScaleUniformedViewportLayoutRect();
        rect.z = viewport.z;
        rect.w = viewport.w;
    }
    return rect;
}

MoveBackgroundVideoState::MoveBackgroundVideoState(Project *project, const Vector2SI32 &pressedCursorPosition)
    : DraggingBackgroundVideoState(project, pressedCursorPosition)
    , m_lastCursorPosition(pressedCursorPosition)
{
}

void
MoveBackgroundVideoState::transform(const Vector2SI32 &logicalScalePosition)
{
    const Vector2SI32 delta(logicalScalePosition - m_lastCursorPosition);
    Vector4SI32 rect(normalizeBackgroundVideoRect());
    rect.x += delta.x;
    rect.y -= delta.y;
    project()->setBackgroundVideoRect(rect);
    m_lastCursorPosition = logicalScalePosition;
}

ZoomBackgroundVideoState::ZoomBackgroundVideoState(Project *project, const Vector2SI32 &pressedCursorPosition)
    : DraggingBackgroundVideoState(project, pressedCursorPosition)
    , m_lastBackgroundVideoRect(normalizeBackgroundVideoRect())
{
    setScaleFactor(kDefaultScaleFactor);
}

void
ZoomBackgroundVideoState::transform(const Vector2SI32 &logicalScalePosition)
{
    const Vector2 delta(Vector2(pressedCursorPosition() - logicalScalePosition) * scaleFactor());
    const nanoem_f32_t scaleFactor = 1.0f + delta.y;
    Vector4 rect(m_lastBackgroundVideoRect);
    rect.z *= scaleFactor;
    rect.w *= scaleFactor;
    project()->setBackgroundVideoRect(rect);
}

} /* namespace internal */
} /* namespace nanoem */
