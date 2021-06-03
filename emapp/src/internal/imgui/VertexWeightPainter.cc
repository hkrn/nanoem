/*
   Copyright (c) 2015-2021 hkrn All rights reserved

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
    : m_activeBonePtr(nullptr)
    , m_model(model)
    , m_radius(16.0f)
    , m_delta(0.005f)
    , m_protectBDEF1Enabled(true)
    , m_automaticNormalizationEnabled(true)
{
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
    if (numBones > 0) {
        setActiveBone(bones[0]);
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
    const model::IVertexWeightPainter *painter = activeModel->vertexWeightPainter();
    nanoem_rsize_t numVertices;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(activeModel->data(), &numVertices);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (nanoem_rsize_t i = 0; i < numVertices; i++) {
        nanoem_model_vertex_t *vertexPtr = vertices[i];
        const Vector3 origin(glm::make_vec3(nanoemModelVertexGetOrigin(vertexPtr)));
        const Vector2 cursor(camera->toDeviceScreenCoordinateInWindow(origin));
        if (glm::distance(deviceScaleCursorPosition, cursor) < painter->radius()) {
            paintVertex(painter, vertexPtr, &status);
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
        for (nanoem_rsize_t i = 0; i < 4; i++) {
            origin.m_bones[i] = nanoemModelVertexGetBoneObject(vertexPtr, i);
            origin.m_weights[i] = nanoemModelVertexGetBoneWeight(vertexPtr, i);
        }
    }
    undo_command_t *command = command::PaintVertexWeightCommand::create(m_model, m_mappings);
    m_model->pushUndo(command);
}

const nanoem_model_bone_t *
VertexWeightPainter::activeBone() const NANOEM_DECL_NOEXCEPT
{
    return m_activeBonePtr;
}

void
VertexWeightPainter::setActiveBone(const nanoem_model_bone_t *value)
{
    m_activeBonePtr = value;
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
VertexWeightPainter::isProtectBDEF1Enabled() const NANOEM_DECL_NOEXCEPT
{
    return m_protectBDEF1Enabled;
}

void
VertexWeightPainter::setProtectBDEF1Enabled(bool value)
{
    m_protectBDEF1Enabled = value;
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
VertexWeightPainter::paintVertex(
    const model::IVertexWeightPainter *brush, nanoem_model_vertex_t *vertexPtr, nanoem_status_t *status)
{
    nanoem_mutable_model_vertex_t *mutableVertexPtr = nanoemMutableModelVertexCreateAsReference(vertexPtr, status);
    command::PaintVertexWeightCommand::BoneMappingStateMap::const_iterator it = m_mappings.find(vertexPtr);
    if (it == m_mappings.end()) {
        command::PaintVertexWeightCommand::BoneMappingStatePair states;
        Inline::clearZeroMemory(states.first);
        command::PaintVertexWeightCommand::BoneMappingState &origin = states.second;
        for (nanoem_rsize_t i = 0; i < 4; i++) {
            origin.m_bones[i] = nanoemModelVertexGetBoneObject(vertexPtr, i);
            origin.m_weights[i] = nanoemModelVertexGetBoneWeight(vertexPtr, i);
        }
        m_mappings.insert(tinystl::make_pair(vertexPtr, states));
    }
    switch (nanoemModelVertexGetType(vertexPtr)) {
    case NANOEM_MODEL_VERTEX_TYPE_BDEF1: {
        if (!brush->isProtectBDEF1Enabled()) {
            const nanoem_model_bone_t *activeBone = brush->activeBone();
            nanoemMutableModelVertexSetBoneObject(mutableVertexPtr, activeBone, 0);
            updateVertex(vertexPtr);
        }
        break;
    }
    case NANOEM_MODEL_VERTEX_TYPE_BDEF2:
    case NANOEM_MODEL_VERTEX_TYPE_SDEF: {
        setVertexBoneWeight(brush, vertexPtr, 2, mutableVertexPtr);
        if (brush->isAutomaticNormalizationEnabled()) {
            normalizeVertexBoneWeight(vertexPtr, 2, mutableVertexPtr);
        }
        updateVertex(vertexPtr);
        break;
    }
    case NANOEM_MODEL_VERTEX_TYPE_BDEF4:
    case NANOEM_MODEL_VERTEX_TYPE_QDEF: {
        setVertexBoneWeight(brush, vertexPtr, 4, mutableVertexPtr);
        if (brush->isAutomaticNormalizationEnabled()) {
            normalizeVertexBoneWeight(vertexPtr, 4, mutableVertexPtr);
        }
        updateVertex(vertexPtr);
        break;
    }
    default:
        break;
    }
    nanoemMutableModelVertexDestroy(mutableVertexPtr);
}

void
VertexWeightPainter::setVertexBoneWeight(const model::IVertexWeightPainter *brush, nanoem_model_vertex_t *vertexPtr,
    nanoem_rsize_t max, nanoem_mutable_model_vertex_t *mutableVertexPtr)
{
    const nanoem_model_bone_t *activeBone = brush->activeBone();
    for (nanoem_rsize_t i = 0; i < max; i++) {
        if (nanoemModelVertexGetBoneObject(vertexPtr, i) == activeBone) {
            nanoem_f32_t weight = nanoemModelVertexGetBoneWeight(vertexPtr, i);
            weight += brush->delta();
            nanoemMutableModelVertexSetBoneWeight(mutableVertexPtr, glm::clamp(weight, 0.0f, 1.0f), i);
            break;
        }
    }
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
