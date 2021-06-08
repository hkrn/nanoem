/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODEL_IVERTEXWEIGHTBRUSH_H_
#define NANOEM_EMAPP_MODEL_IVERTEXWEIGHTBRUSH_H_

#include "emapp/Forward.h"

namespace nanoem {
namespace model {

class IVertexWeightPainter : private NonCopyable {
public:
    enum Type {
        kTypeFirstEnum,
        kTypeBaseBrush = kTypeFirstEnum,
        kTypeAirBrush,
        kTypeMaxEnum,
    };
    virtual ~IVertexWeightPainter() NANOEM_DECL_NOEXCEPT
    {
    }

    virtual void begin() = 0;
    virtual void paint(const Vector2SI32 &logicalScaleCursorPosition) = 0;
    virtual void end() = 0;

    virtual Type type() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setType(Type value) = 0;
    virtual nanoem_model_vertex_type_t vertexType() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setVertexType(nanoem_model_vertex_type_t value) = 0;
    virtual const nanoem_model_bone_t *vertexBone(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setVertexBone(const nanoem_model_bone_t *value, nanoem_rsize_t index) = 0;
    virtual nanoem_f32_t vertexWeight(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setVertexWeight(const nanoem_f32_t value, nanoem_rsize_t index) = 0;
    virtual nanoem_f32_t radius() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setRadius(nanoem_f32_t value) = 0;
    virtual nanoem_f32_t delta() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setDelta(nanoem_f32_t value) = 0;
    virtual bool isAutomaticNormalizationEnabled() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setAutomaticNormalizationEnabled(bool value) = 0;
};

} /* namespace model */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODEL_IVERTEXWEIGHTBRUSH_H_ */
