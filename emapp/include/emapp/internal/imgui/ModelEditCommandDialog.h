/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_MODELEDITCOMMANDDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_MODELEDITCOMMANDDIALOG_H_

#include "emapp/IModelObjectSelection.h"
#include "emapp/Model.h"
#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct ModelEditCommandDialog : BaseNonModalDialogWindow {
    static const char *const kIdentifier;
    static void applyDeltaTransform(const Matrix4x4 &delta, Model *activeModel);
    static void beforeToggleEditingMode(
        IModelObjectSelection::EditingType editingType, Model *activeModel, Project *project);
    static void afterToggleEditingMode(
        IModelObjectSelection::EditingType editingType, Model *activeModel, Project *project);

    ModelEditCommandDialog(Model *model, BaseApplicationService *applicationPtr);

    bool draw(Project *project);
    void addGizmoOperationButton(const char *text, Model::GizmoOperationType type, bool enabled);
    void addGizmoCoordinationButton(const char *text, Model::TransformCoordinateType type, bool enabled);
    void addSelectionButton(const char *text, IModelObjectSelection::EditingType type, Project *project);

    Model *m_activeModel;
};

struct UVEditDialog : BaseNonModalDialogWindow {
    typedef tinystl::unordered_set<nanoem_model_vertex_t *, TinySTLAllocator> VertexSet;
    enum OperationType {
        kOperationTypeSelect,
        kOperationTypeTranslate,
        kOperationTypeRotate,
        kOperationTypeScale,
    };
    struct Selector {
        Selector(const Vector4SI32 &region, const ImVec2 &itemOffset, const ImVec2 &itemSize, VertexSet *vertexSet,
            bool modifiable);
        bool select(nanoem_model_vertex_t *vertex, ImVec2 &pos);
        VertexSet *m_vertexSet;
        Vector4SI32 m_region;
        ImVec2 m_offset;
        ImVec2 m_size;
        bool m_modifiable;
    };
    struct State {
        typedef tinystl::vector<nanoem_mutable_model_vertex_t *, TinySTLAllocator> MutableVertexList;
        State();
        void begin();
        void transform(const Matrix4x4 &delta, Model *activeModel);
        void commit();
        bool isDragging() const NANOEM_DECL_NOEXCEPT;
        OperationType operation() const NANOEM_DECL_NOEXCEPT;
        void setOperation(OperationType value);
        MutableVertexList m_mutableVertices;
        VertexSet m_vertexSet;
        Matrix4x4 m_pivotMatrix;
        Matrix4x4 m_initialPivotMatrix;
        Matrix4x4 m_transformMatrix;
        ImVec4 m_rect;
        OperationType m_operation;
        bool m_dragging;
    };
    static const char *const kIdentifier;
    static const nanoem_f32_t kMinimumHeight;
    static void drawImage(const IImageView *image, nanoem_f32_t scaleFactor);
    static void drawDiffuseImageUVMesh(ImDrawList *drawList, const ImVec2 &itemOffset, const ImVec2 &itemSize,
        const nanoem_model_material_t *activeMaterialPtr, Model *activeModel, State *state, bool hovered);
    static void drawSphereMapImageUVMesh(
        const ImVec2 &itemOffset, const nanoem_model_material_t *activeMaterialPtr, Model *activeModel);
    static void squareSizeConstraint(ImGuiSizeCallbackData *data) NANOEM_DECL_NOEXCEPT;

    UVEditDialog(
        const nanoem_model_material_t *materialPtr, Model *activeModel, BaseApplicationService *applicationPtr);

    bool draw(Project *project);
    void drawDiffuseImageUVMesh(ImDrawList *drawList, const ImVec2 &itemOffset, const ImVec2 &itemSize, bool hovered);
    void drawSelectRegion(ImDrawList *drawList);

    Model *m_activeModel;
    const nanoem_model_material_t *m_activeMaterialPtr;
    State m_selectionState;
    nanoem_f32_t m_scaleFactor;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_MODELEDITCOMMANDDIALOG_H_ */
