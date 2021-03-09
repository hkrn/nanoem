/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_EMAPP_INTERNAL_DRAGGINGVIDEOSTATE
#define NANOEM_EMAPP_INTERNAL_DRAGGINGVIDEOSTATE

#include "emapp/internal/IDraggingState.h"

namespace nanoem {

class Project;

namespace internal {

class DraggingBackgroundVideoState : public IDraggingState, private NonCopyable {
public:
    DraggingBackgroundVideoState(Project *project, const Vector2SI32 &pressedCursorPosition);

    void commit(const Vector2SI32 &logicalScalePosition) NANOEM_DECL_OVERRIDE;

    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    nanoem_f32_t scaleFactor() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setScaleFactor(nanoem_f32_t value) NANOEM_DECL_OVERRIDE;

protected:
    Vector2SI32 pressedCursorPosition() const NANOEM_DECL_NOEXCEPT;
    Project *project() NANOEM_DECL_NOEXCEPT;
    Vector4SI32 normalizeBackgroundVideoRect() const NANOEM_DECL_NOEXCEPT;

private:
    Project *m_project;
    const Vector2SI32 m_pressedCursorPosition;
    nanoem_f32_t m_scaleFactor;
};

class MoveBackgroundVideoState NANOEM_DECL_SEALED : public DraggingBackgroundVideoState {
public:
    MoveBackgroundVideoState(Project *project, const Vector2SI32 &pressedCursorPosition);

    void transform(const Vector2SI32 &logicalScalePosition) NANOEM_DECL_OVERRIDE;

    Vector2SI32 m_lastCursorPosition;
};

class ZoomBackgroundVideoState NANOEM_DECL_SEALED : public DraggingBackgroundVideoState {
public:
    static const nanoem_f32_t kDefaultScaleFactor;

    ZoomBackgroundVideoState(Project *project, const Vector2SI32 &pressedCursorPosition);

    void transform(const Vector2SI32 &logicalScalePosition) NANOEM_DECL_OVERRIDE;

    Vector4SI32 m_lastBackgroundVideoRect;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_DRAGGINGVIDEOSTATE */
