/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_VERTEXWEIGHTPAINTER_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_VERTEXWEIGHTPAINTER_H_

#include "emapp/model/IVertexWeightPainter.h"

#include "emapp/command/ModelObjectCommand.h"

namespace nanoem {

class Model;

namespace internal {
namespace imgui {

class VertexWeightPainter : public model::IVertexWeightPainter {
public:
    VertexWeightPainter(Model *model);
    ~VertexWeightPainter() NANOEM_DECL_NOEXCEPT;

    void begin() NANOEM_DECL_OVERRIDE;
    void paint(const Vector2SI32 &logicalScaleCursorPosition) NANOEM_DECL_OVERRIDE;
    void end() NANOEM_DECL_OVERRIDE;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setType(Type value) NANOEM_DECL_OVERRIDE;
    nanoem_model_vertex_type_t vertexType() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setVertexType(nanoem_model_vertex_type_t value) NANOEM_DECL_OVERRIDE;
    const nanoem_model_bone_t *vertexBone(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setVertexBone(const nanoem_model_bone_t *value, nanoem_rsize_t index) NANOEM_DECL_OVERRIDE;
    nanoem_f32_t vertexWeight(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setVertexWeight(const nanoem_f32_t value, nanoem_rsize_t index) NANOEM_DECL_OVERRIDE;
    nanoem_f32_t radius() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setRadius(nanoem_f32_t value) NANOEM_DECL_OVERRIDE;
    nanoem_f32_t delta() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setDelta(nanoem_f32_t value) NANOEM_DECL_OVERRIDE;
    bool isAutomaticNormalizationEnabled() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setAutomaticNormalizationEnabled(bool value) NANOEM_DECL_OVERRIDE;

private:
    static void normalizeVertexBoneWeight(
        nanoem_model_vertex_t *vertexPtr, nanoem_rsize_t max, nanoem_mutable_model_vertex_t *mutableVertexPtr);
    void paintVertex(nanoem_model_vertex_t *vertexPtr, nanoem_status_t *status);
    void paintVertexBaseBrush(nanoem_mutable_model_vertex_t *mutableVertexPtr);
    void paintVertexAirBrush(nanoem_mutable_model_vertex_t *mutableVertexPtr);
    void setVertexBoneWeight(nanoem_rsize_t max, nanoem_mutable_model_vertex_t *mutableVertexPtr);
    void updateVertex(nanoem_model_vertex_t *vertexPtr);

    Model *m_model;
    command::PaintVertexWeightCommand::BoneMappingStateMap m_mappings;
    const nanoem_model_bone_t *m_bones[4];
    nanoem_f32_t m_weights[4];
    nanoem_model_vertex_type_t m_vertexType;
    Type m_type;
    nanoem_f32_t m_radius;
    nanoem_f32_t m_delta;
    bool m_automaticNormalizationEnabled;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_VERTEXWEIGHTPAINTER_H_ */
