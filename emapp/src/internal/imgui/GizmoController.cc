/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/GizmoController.h"

#include "emapp/Constants.h"
#include "emapp/ICamera.h"
#include "emapp/Model.h"
#include "emapp/Project.h"
#include "emapp/model/IGizmo.h"
#include "emapp/private/CommonInclude.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/matrix_query.hpp"
#include "imgui/imgui.h"
#include "imguizmo/ImGuizmo.h"

namespace nanoem {
namespace internal {
namespace imgui {
namespace {

struct GizmoUtils NANOEM_DECL_SEALED : private NonCopyable {
    static ImGuizmo::OPERATION operation(const Model *activeModel) NANOEM_DECL_NOEXCEPT;
    static ImGuizmo::MODE mode(const Model *activeModel) NANOEM_DECL_NOEXCEPT;
};

ImGuizmo::OPERATION
GizmoUtils::operation(const Model *activeModel) NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(activeModel, "model must NOT be null");
    ImGuizmo::OPERATION op;
    switch (activeModel->gizmo()->operationType()) {
    case model::IGizmo::kOperationTypeTranslate:
    default:
        op = ImGuizmo::TRANSLATE;
        break;
    case model::IGizmo::kOperationTypeRotate:
        op = ImGuizmo::ROTATE;
        break;
    case model::IGizmo::kOperationTypeScale:
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
    switch (activeModel->gizmo()->transformCoordinateType()) {
    case model::IGizmo::kTransformCoordinateTypeGlobal:
        mode = ImGuizmo::WORLD;
        break;
    case model::IGizmo::kTransformCoordinateTypeLocal:
    default:
        mode = ImGuizmo::LOCAL;
        break;
    }
    return mode;
}

class Gizmo NANOEM_DECL_SEALED : public model::IGizmo {
public:
    Gizmo();
    ~Gizmo() NANOEM_DECL_NOEXCEPT;

    Matrix4x4 pivotMatrix() const NANOEM_DECL_NOEXCEPT NANOEM_DECL_OVERRIDE;
    void setPivotMatrix(const Matrix4x4 &value) NANOEM_DECL_OVERRIDE;
    TransformCoordinateType transformCoordinateType() const NANOEM_DECL_NOEXCEPT NANOEM_DECL_OVERRIDE;
    void setTransformCoordinateType(TransformCoordinateType value) NANOEM_DECL_OVERRIDE;
    OperationType operationType() const NANOEM_DECL_NOEXCEPT NANOEM_DECL_OVERRIDE;
    void setOperationType(OperationType value) NANOEM_DECL_OVERRIDE;

private:
    Matrix4x4 m_pivotMatrix;
    TransformCoordinateType m_transformCoordinateType;
    OperationType m_operationType;
};

Gizmo::Gizmo()
    : m_pivotMatrix(0)
    , m_transformCoordinateType(kTransformCoordinateTypeGlobal)
    , m_operationType(kOperationTypeTranslate)
{
}

Gizmo::~Gizmo() NANOEM_DECL_NOEXCEPT
{
}

Matrix4x4
Gizmo::pivotMatrix() const NANOEM_DECL_NOEXCEPT
{
    return m_pivotMatrix;
}

void
Gizmo::setPivotMatrix(const Matrix4x4 &value)
{
    m_pivotMatrix = value;
}

Gizmo::TransformCoordinateType
Gizmo::transformCoordinateType() const NANOEM_DECL_NOEXCEPT
{
    return m_transformCoordinateType;
}

void
Gizmo::setTransformCoordinateType(TransformCoordinateType value)
{
    m_transformCoordinateType = value;
}

Gizmo::OperationType
Gizmo::operationType() const NANOEM_DECL_NOEXCEPT
{
    return m_operationType;
}

void
Gizmo::setOperationType(OperationType value)
{
    m_operationType = value;
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
    ImGuizmo::SetOrthographic(project->activeCamera()->isPerspective() ? false : true);
    project->activeCamera()->getViewTransform(view, projection);
    Model *activeModel = project->activeModel();
    model::IGizmo *gizmo = activeModel->gizmo();
    if (activeModel && !gizmo) {
        gizmo = nanoem_new(Gizmo);
        activeModel->setGizmo(gizmo);
    }
    Matrix4x4 pivotMatrix(activeModel ? activeModel->gizmo()->pivotMatrix() : Matrix4x4(0));
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
                m_transformMatrix = Constants::kIdentity;
            }
            m_state->transform(delta);
            m_transformMatrix *= delta;
            activeModel->gizmo()->setPivotMatrix(pivotMatrix);
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
