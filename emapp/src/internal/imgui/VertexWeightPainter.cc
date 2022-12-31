/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/VertexWeightPainter.h"

#include "emapp/ICamera.h"
#include "emapp/Model.h"
#include "emapp/Project.h"
#include "emapp/private/CommonInclude.h"

#include "glm/gtc/type_ptr.hpp"

namespace nanoem {
namespace internal {
namespace imgui {

VertexWeightPainter::VertexWeightPainter(Model *model)
    : m_model(model)
    , m_vertexType(NANOEM_MODEL_VERTEX_TYPE_BDEF1)
    , m_type(kTypeAirBrush)
    , m_radius(16.0f)
    , m_delta(0.005f)
    , m_automaticNormalizationEnabled(true)
{
    Inline::clearZeroMemory(m_bones);
    Inline::clearZeroMemory(m_weights);
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
    if (numBones > 0) {
        setVertexBone(bones[0], 0);
        setVertexWeight(1.0f, 0);
    }
}

VertexWeightPainter::~VertexWeightPainter() NANOEM_DECL_NOEXCEPT
{
}

void
VertexWeightPainter::begin()
{
}

void
VertexWeightPainter::paint(const Vector2SI32 &logicalScaleCursorPosition)
{
    Project *project = m_model->project();
    const ICamera *camera = project->activeCamera();
    const Vector2 deviceScaleCursorPosition(Vector2(logicalScaleCursorPosition) * project->windowDevicePixelRatio());
    Model *activeModel = project->activeModel();
    nanoem_rsize_t numVertices;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(activeModel->data(), &numVertices);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (nanoem_rsize_t i = 0; i < numVertices; i++) {
        nanoem_model_vertex_t *vertexPtr = vertices[i];
        const model::Vertex *vertex = model::Vertex::cast(vertexPtr);
        if (vertex && !vertex->isEditingMasked()) {
            const Vector3 origin(glm::make_vec3(nanoemModelVertexGetOrigin(vertexPtr)));
            const Vector2 cursor(camera->toDeviceScreenCoordinateInWindow(origin));
            if (glm::distance(deviceScaleCursorPosition, cursor) < radius()) {
                paintVertex(vertexPtr, &status);
            }
        }
    }
}

void
VertexWeightPainter::end()
{
    for (command::PaintVertexWeightCommand::BoneMappingStateMap::iterator it = m_mappings.begin(),
                                                                          end = m_mappings.end();
         it != end; ++it) {
        const nanoem_model_vertex_t *vertexPtr = it->first;
        command::PaintVertexWeightCommand::BoneMappingState &origin = it->second.first;
        for (nanoem_rsize_t i = 0; i < BX_COUNTOF(m_bones); i++) {
            origin.m_bones[i] = nanoemModelVertexGetBoneObject(vertexPtr, i);
            origin.m_weights[i] = nanoemModelVertexGetBoneWeight(vertexPtr, i);
        }
    }
    undo_command_t *command = command::PaintVertexWeightCommand::create(m_model, m_mappings);
    m_model->pushUndo(command);
}

VertexWeightPainter::Type
VertexWeightPainter::type() const NANOEM_DECL_NOEXCEPT
{
    return m_type;
}

void
VertexWeightPainter::setType(Type value)
{
    m_type = value;
}

nanoem_model_vertex_type_t
VertexWeightPainter::vertexType() const NANOEM_DECL_NOEXCEPT
{
    return m_vertexType;
}

void
VertexWeightPainter::setVertexType(nanoem_model_vertex_type_t value)
{
    m_vertexType = value;
}

const nanoem_model_bone_t *
VertexWeightPainter::vertexBone(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT
{
    return index < BX_COUNTOF(m_bones) ? m_bones[index] : nullptr;
}

void
VertexWeightPainter::setVertexBone(const nanoem_model_bone_t *value, nanoem_rsize_t index)
{
    if (index < BX_COUNTOF(m_bones)) {
        m_bones[index] = value;
    }
}

nanoem_f32_t
VertexWeightPainter::vertexWeight(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT
{
    return index < BX_COUNTOF(m_weights) ? m_weights[index] : 0;
}

void
VertexWeightPainter::setVertexWeight(const nanoem_f32_t value, nanoem_rsize_t index)
{
    if (index < BX_COUNTOF(m_weights)) {
        m_weights[index] = glm::clamp(value, 0.0f, 1.0f);
    }
}

nanoem_f32_t
VertexWeightPainter::radius() const NANOEM_DECL_NOEXCEPT
{
    return m_radius;
}

void
VertexWeightPainter::setRadius(nanoem_f32_t value)
{
    m_radius = value;
}

nanoem_f32_t
VertexWeightPainter::delta() const NANOEM_DECL_NOEXCEPT
{
    return m_delta;
}

void
VertexWeightPainter::setDelta(nanoem_f32_t value)
{
    m_delta = value;
}

bool
VertexWeightPainter::isAutomaticNormalizationEnabled() const NANOEM_DECL_NOEXCEPT
{
    return m_automaticNormalizationEnabled;
}

void
VertexWeightPainter::setAutomaticNormalizationEnabled(bool value)
{
    m_automaticNormalizationEnabled = value;
}

void
VertexWeightPainter::normalizeVertexBoneWeight(
    nanoem_model_vertex_t *vertexPtr, nanoem_rsize_t max, nanoem_mutable_model_vertex_t *mutableVertexPtr)
{
    nanoem_f32_t sum = 0;
    for (nanoem_rsize_t i = 0; i < max; i++) {
        sum += nanoemModelVertexGetBoneWeight(vertexPtr, i);
    }
    if (sum > 1.0f) {
        for (nanoem_rsize_t i = 0; i < max; i++) {
            nanoem_f32_t weight = nanoemModelVertexGetBoneWeight(vertexPtr, i);
            nanoemMutableModelVertexSetBoneWeight(mutableVertexPtr, weight / sum, i);
        }
    }
}

void
VertexWeightPainter::paintVertex(nanoem_model_vertex_t *vertexPtr, nanoem_status_t *status)
{
    nanoem_mutable_model_vertex_t *mutableVertexPtr = nanoemMutableModelVertexCreateAsReference(vertexPtr, status);
    command::PaintVertexWeightCommand::BoneMappingStateMap::const_iterator it = m_mappings.find(vertexPtr);
    if (it == m_mappings.end()) {
        command::PaintVertexWeightCommand::BoneMappingStatePair states;
        Inline::clearZeroMemory(states.first);
        command::PaintVertexWeightCommand::BoneMappingState &origin = states.second;
        for (nanoem_rsize_t i = 0; i < BX_COUNTOF(m_bones); i++) {
            origin.m_bones[i] = nanoemModelVertexGetBoneObject(vertexPtr, i);
            origin.m_weights[i] = nanoemModelVertexGetBoneWeight(vertexPtr, i);
        }
        m_mappings.insert(tinystl::make_pair(vertexPtr, states));
    }
    switch (m_type) {
    case kTypeBaseBrush: {
        paintVertexBaseBrush(mutableVertexPtr);
        break;
    }
    case kTypeAirBrush: {
        paintVertexAirBrush(mutableVertexPtr);
        break;
    }
    default:
        break;
    }
    nanoemMutableModelVertexDestroy(mutableVertexPtr);
}

void
VertexWeightPainter::paintVertexBaseBrush(nanoem_mutable_model_vertex_t *mutableVertexPtr)
{
    nanoemMutableModelVertexSetType(mutableVertexPtr, m_vertexType);
    switch (m_vertexType) {
    case NANOEM_MODEL_VERTEX_TYPE_BDEF1: {
        nanoemMutableModelVertexSetBoneObject(mutableVertexPtr, vertexBone(0), 0);
        nanoemMutableModelVertexSetBoneWeight(mutableVertexPtr, 1.0f, 0);
        for (nanoem_rsize_t i = 1; i < BX_COUNTOF(m_bones); i++) {
            nanoemMutableModelVertexSetBoneObject(mutableVertexPtr, nullptr, i);
            nanoemMutableModelVertexSetBoneWeight(mutableVertexPtr, 0.0f, i);
        }
        break;
    }
    case NANOEM_MODEL_VERTEX_TYPE_BDEF2:
    case NANOEM_MODEL_VERTEX_TYPE_SDEF: {
        for (nanoem_rsize_t i = 0; i < 2; i++) {
            nanoemMutableModelVertexSetBoneObject(mutableVertexPtr, vertexBone(i), i);
            nanoemMutableModelVertexSetBoneWeight(mutableVertexPtr, vertexWeight(i), i);
        }
        for (nanoem_rsize_t i = 2; i < BX_COUNTOF(m_bones); i++) {
            nanoemMutableModelVertexSetBoneObject(mutableVertexPtr, nullptr, i);
            nanoemMutableModelVertexSetBoneWeight(mutableVertexPtr, 0.0f, i);
        }
        break;
    }
    case NANOEM_MODEL_VERTEX_TYPE_BDEF4:
    case NANOEM_MODEL_VERTEX_TYPE_QDEF: {
        for (nanoem_rsize_t i = 0; i < BX_COUNTOF(m_bones); i++) {
            nanoemMutableModelVertexSetBoneObject(mutableVertexPtr, vertexBone(i), i);
            nanoemMutableModelVertexSetBoneWeight(mutableVertexPtr, vertexWeight(i), i);
        }
        break;
    }
    default:
        break;
    }
    updateVertex(nanoemMutableModelVertexGetOriginObject(mutableVertexPtr));
}

void
VertexWeightPainter::paintVertexAirBrush(nanoem_mutable_model_vertex_t *mutableVertexPtr)
{
    nanoem_model_vertex_t *vertexPtr = nanoemMutableModelVertexGetOriginObject(mutableVertexPtr);
    switch (nanoemModelVertexGetType(vertexPtr)) {
    case NANOEM_MODEL_VERTEX_TYPE_BDEF1: {
        break;
    }
    case NANOEM_MODEL_VERTEX_TYPE_BDEF2:
    case NANOEM_MODEL_VERTEX_TYPE_SDEF: {
        setVertexBoneWeight(2, mutableVertexPtr);
        if (isAutomaticNormalizationEnabled()) {
            normalizeVertexBoneWeight(vertexPtr, 2, mutableVertexPtr);
        }
        updateVertex(vertexPtr);
        break;
    }
    case NANOEM_MODEL_VERTEX_TYPE_BDEF4:
    case NANOEM_MODEL_VERTEX_TYPE_QDEF: {
        setVertexBoneWeight(4, mutableVertexPtr);
        if (isAutomaticNormalizationEnabled()) {
            normalizeVertexBoneWeight(vertexPtr, 4, mutableVertexPtr);
        }
        updateVertex(vertexPtr);
        break;
    }
    default:
        break;
    }
}

void
VertexWeightPainter::setVertexBoneWeight(nanoem_rsize_t max, nanoem_mutable_model_vertex_t *mutableVertexPtr)
{
    const nanoem_model_bone_t *bonePtr = vertexBone(0);
    nanoem_model_vertex_t *vertexPtr = nanoemMutableModelVertexGetOriginObject(mutableVertexPtr);
    for (nanoem_rsize_t i = 0; i < max; i++) {
        if (nanoemModelVertexGetBoneObject(vertexPtr, i) == bonePtr) {
            nanoem_f32_t weight = nanoemModelVertexGetBoneWeight(vertexPtr, i);
            weight += m_delta;
            nanoemMutableModelVertexSetBoneWeight(mutableVertexPtr, glm::clamp(weight, 0.0f, 1.0f), i);
            break;
        }
    }
}

void
VertexWeightPainter::updateVertex(nanoem_model_vertex_t *vertexPtr)
{
    if (model::Vertex *vertex = model::Vertex::cast(vertexPtr)) {
        const Vector4 weight(nanoemModelVertexGetBoneWeight(vertexPtr, 0), nanoemModelVertexGetBoneWeight(vertexPtr, 1),
            nanoemModelVertexGetBoneWeight(vertexPtr, 2), nanoemModelVertexGetBoneWeight(vertexPtr, 3));
        vertex->setupBoneBinding(vertexPtr, m_model);
        vertex->m_simd.m_weights = bx::simd_ld(glm::value_ptr(weight));
    }
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
