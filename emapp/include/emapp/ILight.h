/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_ILIGHT_H_
#define NANOEM_EMAPP_ILIGHT_H_

#include "emapp/Forward.h"

namespace nanoem {

class Motion;
class Project;

class ILight {
public:
    virtual ~ILight() NANOEM_DECL_NOEXCEPT
    {
    }

    virtual void destroy() NANOEM_DECL_NOEXCEPT = 0;
    virtual void reset() NANOEM_DECL_NOEXCEPT = 0;
    virtual void synchronizeParameters(const Motion *motion, const nanoem_frame_index_t frameIndex) = 0;
    virtual void getShadowTransform(Matrix4x4 &value) const = 0;

    virtual const Project *project() const NANOEM_DECL_NOEXCEPT = 0;
    virtual Project *project() NANOEM_DECL_NOEXCEPT = 0;
    virtual Vector3 color() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setColor(const Vector3 &value) = 0;
    virtual Vector3 direction() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setDirection(const Vector3 &value) = 0;
    virtual Vector3 groundShadowColor() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool isTranslucentGroundShadowEnabled() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setTranslucentGroundShadowEnabled(bool value) = 0;
    virtual bool isDirty() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setDirty(bool value) = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_DIRECTIONALLIGHT_H_ */
