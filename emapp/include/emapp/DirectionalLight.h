/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_DIRECTIONALLIGHT_H_
#define NANOEM_EMAPP_DIRECTIONALLIGHT_H_

#include "emapp/ILight.h"

struct undo_stack_t;

namespace nanoem {

class DirectionalLight;
class ITranslator;
class PerspectiveCamera;

class DirectionalLight NANOEM_DECL_SEALED : public ILight, private NonCopyable {
public:
    static const Vector3 kInitialColor;
    static const Vector3 kInitialDirection;

    DirectionalLight(Project *project);
    ~DirectionalLight() NANOEM_DECL_NOEXCEPT;

    void destroy() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void reset() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void synchronizeParameters(const Motion *motion, const nanoem_frame_index_t frameIndex) NANOEM_DECL_OVERRIDE;
    void getShadowTransform(Matrix4x4 &value) const NANOEM_DECL_OVERRIDE;

    const Project *project() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    Project *project() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    Vector3 color() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setColor(const Vector3 &value) NANOEM_DECL_OVERRIDE;
    Vector3 direction() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setDirection(const Vector3 &value) NANOEM_DECL_OVERRIDE;
    Vector3 groundShadowColor() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isTranslucentGroundShadowEnabled() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setTranslucentGroundShadowEnabled(bool value) NANOEM_DECL_OVERRIDE;
    bool isDirty() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setDirty(bool value) NANOEM_DECL_OVERRIDE;

private:
    Project *m_project;
    undo_stack_t *m_undoStack;
    Vector3 m_color;
    Vector3 m_direction;
    bool m_translucent;
    bool m_dirty;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_DIRECTIONALLIGHT_H_ */
