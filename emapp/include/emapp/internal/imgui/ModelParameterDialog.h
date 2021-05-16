/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_MODELPARAMETERDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_MODELPARAMETERDIALOG_H_

#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

#include "emapp/IModelObjectSelection.h"
#include "emapp/Model.h"
#include "emapp/StringUtils.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct ModelParameterDialog : BaseNonModalDialogWindow {
    typedef void(APIENTRY *PFN_nanoemMutableModelSetBoneObject)(
        nanoem_mutable_model_bone_t *, const nanoem_model_bone_t *);
    typedef void(APIENTRY *PFN_nanoemMutableModelSetBoneAxis)(nanoem_mutable_model_bone_t *, const nanoem_f32_t *);

    static const char *const kIdentifier;
    static const nanoem_f32_t kMinimumWindowWidth;
    static const int kWindowFrameHeightRowCount;
    static const int kInnerItemListFrameHeightRowCount;

    enum TabType {
        kTabTypeFirstEnum,
        kTabTypeInfo = kTabTypeFirstEnum,
        kTabTypeVertex,
        kTabTypeFace,
        kTabTypeMaterial,
        kTabTypeBone,
        kTabTypeMorph,
        kTabTypeLabel,
        kTabTypeRigidBody,
        kTabTypeJoint,
        kTabTypeSoftBody,
        kTabTypeMaxEnum,
    };

    static IModelObjectSelection::EditingType convertToEditingType(TabType tab) NANOEM_DECL_NOEXCEPT;
    static bool hasMorphType(const Model *model, nanoem_model_morph_type_t type) NANOEM_DECL_NOEXCEPT;
    static void formatVertexText(char *buffer, nanoem_rsize_t size, const nanoem_model_vertex_t *vertexPtr);

    ModelParameterDialog(Model *model, Project *project, BaseApplicationService *applicationPtr, ImGuiWindow *parent);
    ~ModelParameterDialog();

    bool draw(Project *project) NANOEM_DECL_OVERRIDE;
    void destroy(Project *project) NANOEM_DECL_OVERRIDE;

    void layoutInformation(Project *project);
    void layoutVertexBoneSelection(const char *label, nanoem_model_vertex_t *vertexPtr, nanoem_rsize_t i,
        nanoem_model_bone_t *const *bones, nanoem_rsize_t numBones);
    void layoutVertexBoneWeights(nanoem_model_vertex_t *vertexPtr, nanoem_rsize_t numItems);
    void layoutAllVertices(Project *project);
    void layoutVertexPropertyPane(nanoem_model_vertex_t *vertexPtr);
    void layoutAllFaces(Project *project);
    void layoutFacePropertyPane(const Vector3UI32 &face);
    void layoutAllMaterials(Project *project);
    void layoutMaterialPropertyPane(nanoem_model_material_t *materialPtr, Project *project);
    void layoutMaterialDiffuseImage(
        const IImageView *image, const String &filename, const nanoem_model_material_t *activeMaterialPtr);
    void layoutMaterialSphereMapImage(
        const IImageView *image, const String &filename, const nanoem_model_material_t *activeMaterialPtr);
    void layoutMaterialToonImage(const IImageView *image, const String &filename);
    void layoutAllBones(Project *project);
    void layoutBonePropertyPane(nanoem_model_bone_t *bonePtr, Project *project);
    void layoutBoneConstraintPanel(nanoem_model_bone_t *bonePtr, Project *project);
    void layoutBoneInternalParametersPanel(const nanoem_model_bone_t *bonePtr);
    void layoutAllMorphs(Project *project);
    void layoutMorphPropertyPane(nanoem_model_morph_t *morphPtr, Project *project);
    void layoutMorphBonePropertyPane(
        const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr, Project *project);
    void layoutMorphFlipPropertyPane(
        const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr, Project *project);
    void layoutMorphGroupPropertyPane(
        const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr, Project *project);
    void layoutMorphImpulsePropertyPane(
        const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr, Project *project);
    void layoutMorphMaterialPropertyPane(
        const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr, Project *project);
    void layoutMorphUVPropertyPane(const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr, Project *project);
    void layoutMorphVertexPropertyPane(
        const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr, Project *project);
    void layoutAllLabels(Project *project);
    void layoutLabelPropertyPane(nanoem_model_label_t *labelPtr, Project *project);
    void layoutAllRigidBodies(Project *project);
    void layoutRigidBodyPropertyPane(nanoem_model_rigid_body_t *rigidBodyPtr, Project *project);
    void layoutAllJoints(Project *project);
    void layoutJointPropertyPane(nanoem_model_joint_t *jointPtr, Project *project);
    void layoutAllSoftBodies(Project *project);
    void layoutSoftBodyPropertyPane(nanoem_model_soft_body_t *softBodyPtr, Project *project);
    bool layoutName(const nanoem_unicode_string_t *namePtr, Project *project, StringUtils::UnicodeStringScope &scope);
    void layoutTextWithParentBoneValidation(const nanoem_model_bone_t *bonePtr,
        const nanoem_model_bone_t *parentBonePtr, const char *titleID, const char *validationMessageID);
    void layoutBoneComboBox(const char *id, const nanoem_model_bone_t *baseBonePtr, nanoem_model_bone_t *bonePtr,
        PFN_nanoemMutableModelSetBoneObject setBoneCallback);
    void layoutBoneAxisMenuItems(nanoem_model_bone_t *bonePtr, PFN_nanoemMutableModelSetBoneAxis setBoneAxisCallback);

    void layoutManipulateVertexMenu(Project *project);
    void layoutCreateMaterialMenu(Project *project);
    void layoutManipulateMaterialMenu(Project *project);
    void layoutCreateBoneMenu(Project *project);
    void layoutManipulateBoneMenu(Project *project);
    void layoutCreateMorphMenu(Project *project);
    void layoutManipulateMorphMenu(Project *project);
    void layoutCreateLabelMenu(Project *project, const nanoem_model_label_t *selectedLabel);
    void layoutManipulateLabelMenu(Project *project);
    void layoutCreateRigidBodyMenu(Project *project);
    void layoutManipulateRigidBodyMenu(Project *project);
    void layoutCreateJointMenu(Project *project);
    void layoutManipulateJointMenu(Project *project);
    void layoutCreateSoftBodyMenu(Project *project);
    void layoutManipulateSoftBodyMenu(Project *project);

    void toggleTab(TabType value, Project *project);
    void forceUpdateMorph(model::Morph *morph, Project *project);
    void setActiveModel(Model *model, Project *project);
    void saveProjectState(Project *project);
    void restoreProjectState(Project *project);

    void removeAllVertexSelectionIfNeeded(IModelObjectSelection *selection);
    void removeAllFaceSelectionIfNeeded(IModelObjectSelection *selection);
    void removeAllMaterialSelectionIfNeeded(IModelObjectSelection *selection);
    void removeAllBoneSelectionIfNeeded(IModelObjectSelection *selection);
    void removeAllMorphSelectionIfNeeded(IModelObjectSelection *selection);
    void removeAllLabelSelectionIfNeeded(IModelObjectSelection *selection);
    void removeAllRigidBodySelectionIfNeeded(IModelObjectSelection *selection);
    void removeAllJointSelectionIfNeeded(IModelObjectSelection *selection);
    void removeAllSoftBodySelectionIfNeeded(IModelObjectSelection *selection);
    void selectAllVerticesByType(IModelObjectSelection *selection, nanoem_model_vertex_type_t type);
    void toggleBone(const nanoem_model_bone_t *bonePtr);
    void toggleMaterial(const nanoem_model_material_t *materialPtr);
    void toggleRigidBody(const nanoem_model_rigid_body_t *rigidBodyPtr);
    void toggleVertex(const nanoem_model_vertex_t *vertexPtr);

    const char *selectedFormatType(const nanoem_model_format_type_t type) const NANOEM_DECL_NOEXCEPT;
    const char *selectedCodecType(const nanoem_codec_type_t type) const NANOEM_DECL_NOEXCEPT;
    const char *selectedVertexType(const nanoem_model_vertex_type_t type) const NANOEM_DECL_NOEXCEPT;
    const char *selectedMaterialPrimitiveType(const nanoem_model_material_t *materialPtr) const NANOEM_DECL_NOEXCEPT;
    const char *selectedMaterialSphereMapType(
        const nanoem_model_material_sphere_map_texture_type_t value) const NANOEM_DECL_NOEXCEPT;
    const char *selectedMorphCategory(nanoem_model_morph_category_t value) const NANOEM_DECL_NOEXCEPT;
    const char *selectedMorphType(nanoem_model_morph_type_t value) const NANOEM_DECL_NOEXCEPT;
    const char *selectedMorphMaterialOperationType(
        nanoem_model_morph_material_operation_type_t value) const NANOEM_DECL_NOEXCEPT;
    const char *selectedJointType(nanoem_model_joint_type_t value) const NANOEM_DECL_NOEXCEPT;
    const char *selectedSoftBodyAeroMdoelType(
        const nanoem_model_soft_body_aero_model_type_t type) const NANOEM_DECL_NOEXCEPT;
    bool hasMorphType(nanoem_model_morph_type_t type) const NANOEM_DECL_NOEXCEPT;
    bool hasModelWithMaterial(const Project *project) const NANOEM_DECL_NOEXCEPT;
    bool isPMX21() const NANOEM_DECL_NOEXCEPT;

    struct SavedState {
        struct ModelState {
            const nanoem_model_bone_t *m_activeBone;
            const nanoem_model_morph_t *m_activeMorph;
            ByteArray m_motion;
        };
        SavedState(Project::EditingMode editingMode)
            : m_activeModel(nullptr)
            , m_state(nullptr)
            , m_lastEditingMode(editingMode)
        {
        }
        typedef tinystl::unordered_map<Model *, ModelState, TinySTLAllocator> ModelStateMap;
        Model *m_activeModel;
        Project::SaveState *m_state;
        Project::EditingMode m_lastEditingMode;
        ModelStateMap m_modelStates;
        model::BindPose m_bindPose;
    };

    ImGuiWindow *m_parent;
    Model *m_activeModel;
    SavedState *m_savedState;
    int m_language;
    TabType m_tabType;
    TabType m_explicitTabType;
    nanoem_rsize_t m_vertexIndex;
    nanoem_rsize_t m_faceIndex;
    nanoem_rsize_t m_materialIndex;
    nanoem_rsize_t m_boneIndex;
    nanoem_rsize_t m_constraintJointIndex;
    nanoem_rsize_t m_constraintJointCandidateIndex;
    nanoem_rsize_t m_morphIndex;
    nanoem_rsize_t m_morphItemIndex;
    nanoem_rsize_t m_morphItemCandidateBoneIndex;
    nanoem_rsize_t m_morphItemCandidateMaterialIndex;
    nanoem_rsize_t m_morphItemCandidateMorphIndex;
    nanoem_rsize_t m_morphItemCandidateRigidBodyIndex;
    nanoem_rsize_t m_labelIndex;
    nanoem_rsize_t m_labelItemIndex;
    nanoem_rsize_t m_labelItemCandidateBoneIndex;
    nanoem_rsize_t m_labelItemCandidateMorphIndex;
    nanoem_rsize_t m_rigidBodyIndex;
    nanoem_rsize_t m_jointIndex;
    nanoem_rsize_t m_softBodyIndex;
    Project::EditingMode m_editingMode;
    bool m_showAllVertexPoints;
    bool m_showAllVertexFaces;
    bool m_showAllBones;
    bool m_showAllRigidBodies;
    bool m_showAllJoints;
    bool m_showAllSoftBodies;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_MODELPARAMETERDIALOG_H_ */
