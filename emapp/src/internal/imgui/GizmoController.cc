/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/GizmoController.h"

#include "emapp/ICamera.h"
#include "emapp/IModelObjectSelection.h"
#include "emapp/Model.h"
#include "emapp/Project.h"
#include "emapp/private/CommonInclude.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/matrix_query.hpp"
#include "imgui/imgui.h"
#include "imguizmo/ImGuizmo.h"

namespace nanoem {
namespace internal {
namespace imgui {
namespace {

struct GizmoUtils : private NonCopyable {
    static ImGuizmo::OPERATION operation(const Model *activeModel) NANOEM_DECL_NOEXCEPT;
    static ImGuizmo::MODE mode(const Model *activeModel) NANOEM_DECL_NOEXCEPT;
};

ImGuizmo::OPERATION
GizmoUtils::operation(const Model *activeModel) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(activeModel, "model must NOT be null");
    ImGuizmo::OPERATION op;
    switch (activeModel->gizmoOperationType()) {
    case Model::kGizmoOperationTypeTranslate:
    default:
        op = ImGuizmo::TRANSLATE;
        break;
    case Model::kGizmoOperationTypeRotate:
        op = ImGuizmo::ROTATE;
        break;
    case Model::kGizmoOperationTypeScale:
        op = ImGuizmo::SCALE;
        break;
    }
    return op;
}

ImGuizmo::MODE
GizmoUtils::mode(const Model *activeModel) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(activeModel, "model must NOT be null");
    ImGuizmo::MODE mode;
    switch (activeModel->gizmoTransformCoordinateType()) {
    case Model::kTransformCoordinateTypeGlobal:
        mode = ImGuizmo::WORLD;
        break;
    case Model::kTransformCoordinateTypeLocal:
    default:
        mode = ImGuizmo::LOCAL;
        break;
    }
    return mode;
}

} /* namespace anonymous */

GizmoController::GizmoController()
    : m_state(nullptr)
    , m_transformMatrix(Constants::kIdentity)
    , m_initialPivotMatrix(Constants::kIdentity)
{
}

GizmoController::~GizmoController()
{
}

void
GizmoController::begin()
{
    ImGuizmo::BeginFrame();
}

bool
GizmoController::draw(ImDrawList *drawList, const ImVec2 &offset, const ImVec2 &size, Project *project)
{
    Matrix4x4 view, projection, delta;
    bool hovered = true;
    ImGuizmo::SetDrawlist(drawList);
    ImGuizmo::SetRect(offset.x, offset.y, size.x, size.y);
    project->activeCamera()->getViewTransform(view, projection);
    Model *activeModel = project->activeModel();
    Matrix4x4 pivotMatrix(activeModel->pivotMatrix());
    if (!glm::isNull(pivotMatrix, Constants::kEpsilon)) {
        ImGuizmo::DrawCubes(glm::value_ptr(view), glm::value_ptr(projection), glm::value_ptr(pivotMatrix), 1);
        if (ImGuizmo::IsOver()) {
            hovered = false;
        }
        if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), GizmoUtils::operation(activeModel),
                GizmoUtils::mode(activeModel), glm::value_ptr(pivotMatrix), glm::value_ptr(delta))) {
            if (!m_state) {
                m_state = nanoem_new(command::MoveAllSelectedModelObjectsCommand::State(activeModel));
                m_initialPivotMatrix = pivotMatrix;
            }
            m_state->transform(delta);
            m_transformMatrix *= delta;
            activeModel->setPivotMatrix(pivotMatrix);
        }
        else if (!ImGuizmo::IsUsing() && m_state) {
            m_state->reset();
            nanoem_delete_safe(m_state);
            activeModel->pushUndo(command::MoveAllSelectedModelObjectsCommand::create(
                m_transformMatrix, m_initialPivotMatrix, activeModel, project));
        }
    }
    return hovered;
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
