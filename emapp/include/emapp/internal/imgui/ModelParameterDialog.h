/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_MODELPARAMETERDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_MODELPARAMETERDIALOG_H_

#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

#include "emapp/IModelObjectSelection.h"
#include "emapp/StringUtils.h"
#include "emapp/command/ModelObjectCommand.h"

namespace nanoem {
namespace internal {
namespace imgui {

class ModelParameterDialog : public BaseNonModalDialogWindow {
public:
    struct SavedState {
        struct ModelState {
            String m_activeBoneName;
            String m_activeMorphName;
            ByteArray m_modelData;
            ByteArray m_motionData;
            char m_datetime[32];
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

    static const char *const kIdentifier;
    static const nanoem_f32_t kMinimumWindowWidth;
    static const int kWindowFrameHeightRowCount;
    static const int kInnerItemListFrameHeightRowCount;

    enum TabType {
        kTabTypeFirstEnum,
        kTabTypeMeasure = kTabTypeFirstEnum,
        kTabTypeSystem,
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

    static IModelObjectSelection::ObjectType convertToObjectType(TabType tab) NANOEM_DECL_NOEXCEPT;
    static bool hasMorphType(const Model *model, nanoem_model_morph_type_t type) NANOEM_DECL_NOEXCEPT;
    static void formatVertexText(char *buffer, nanoem_rsize_t size, const nanoem_model_vertex_t *vertexPtr);

    ModelParameterDialog(Model *model, Project *project, BaseApplicationService *applicationPtr, ImGuiWindow *parent);
    ~ModelParameterDialog();

    bool draw(Project *project) NANOEM_DECL_OVERRIDE;
    void destroy(Project *project) NANOEM_DECL_OVERRIDE;

    void drawMenuBar(Project *project);
    void saveProjectState(Project *project);

    bool isActiveBoneShown() const NANOEM_DECL_NOEXCEPT;
    bool isLocalAxesShown() const NANOEM_DECL_NOEXCEPT;
    bool isFixedAxisShown() const NANOEM_DECL_NOEXCEPT;

private:
    typedef void(APIENTRY *PFN_nanoemMutableModelSetBoneObject)(
        nanoem_mutable_model_bone_t *, const nanoem_model_bone_t *);
    typedef void(APIENTRY *PFN_nanoemMutableModelSetRigidBodyObject)(
        nanoem_mutable_model_joint_t *, const nanoem_model_rigid_body_t *);
    typedef void(APIENTRY *PFN_nanoemMutableModelSetBoneAxis)(nanoem_mutable_model_bone_t *, const nanoem_f32_t *);
    static bool selectNameBasedSymmetricModelObject(const String &name, String &reversedName, StringSet &nameSet);

    void layoutMeasure();
    void layoutHeightBasedBatchTransformPane();
    void layoutNumericInputBatchTransformPane();
    void layoutSystem(Project *project);
    void layoutVertexBoneSelection(const char *label, nanoem_rsize_t i, nanoem_model_bone_t *const *bones,
        nanoem_rsize_t numBones, nanoem_model_vertex_t *vertexPtr);
    void layoutVertexBoneSelection(const char *label, nanoem_rsize_t i, nanoem_model_bone_t *const *bones,
        nanoem_rsize_t numBones, command::BatchChangeAllVertexObjectsCommand::Parameter &parameter);
    void layoutVertexBoneWeights(nanoem_rsize_t numItems, nanoem_model_vertex_t *vertexPtr);
    void layoutVertexBoneWeights(
        nanoem_rsize_t numItems, command::BatchChangeAllVertexObjectsCommand::Parameter &parameter);
    void layoutAllVertices(Project *project);
    void layoutBatchVertexChangePane();
    void layoutVertexPropertyPane(nanoem_model_vertex_t *vertexPtr);
    void layoutAllFaces(Project *project);
    void layoutBatchFaceChangePane();
    void layoutFacePropertyPane(const Vector3UI32 &face);
    void layoutAllMaterials(Project *project);
    void layoutBatchMaterialChangePane();
    void layoutMaterialPropertyPane(nanoem_model_material_t *materialPtr, Project *project);
    void layoutMaterialDiffuseImage(
        const IImageView *image, const String &filename, const nanoem_model_material_t *activeMaterialPtr);
    void layoutMaterialSphereMapImage(
        const IImageView *image, const String &filename, const nanoem_model_material_t *activeMaterialPtr);
    void layoutMaterialToonImage(const IImageView *image, const String &filename);
    void layoutAllBones(Project *project);
    void layoutBatchBoneChangePane();
    void layoutBonePropertyPane(nanoem_model_bone_t *bonePtr, Project *project);
    void layoutBoneConstraintPanel(nanoem_model_bone_t *bonePtr, Project *project);
    void layoutBoneInternalParametersPanel(const nanoem_model_bone_t *bonePtr);
    void layoutAllMorphs(Project *project);
    void layoutBatchMorphChangePane();
    void layoutMorphPropertyPane(nanoem_model_morph_t *morphPtr, Project *project);
    void layoutMorphBonePropertyPane(const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr);
    void layoutMorphFlipPropertyPane(const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr);
    void layoutMorphGroupPropertyPane(const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr);
    void layoutMorphImpulsePropertyPane(const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr);
    void layoutMorphMaterialPropertyPane(const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr);
    void layoutMorphUVPropertyPane(const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr);
    void layoutMorphVertexPropertyPane(const ImVec2 &propertyWindowSize, nanoem_model_morph_t *morphPtr);
    void layoutAllLabels(Project *project);
    void layoutBatchLabelChangePane();
    void layoutLabelPropertyPane(nanoem_model_label_t *labelPtr, Project *project);
    void layoutAllRigidBodies(Project *project);
    void layoutBatchRigidBodyChangePane();
    void layoutRigidBodyPropertyPane(nanoem_model_rigid_body_t *rigidBodyPtr, Project *project);
    void layoutAllJoints(Project *project);
    void layoutBatchJointChangePane();
    void layoutJointPropertyPane(nanoem_model_joint_t *jointPtr, Project *project);
    void layoutAllSoftBodies(Project *project);
    void layoutBatchSoftBodyChangePane();
    void layoutSoftBodyPropertyPane(nanoem_model_soft_body_t *softBodyPtr, Project *project);
    bool layoutName(const nanoem_unicode_string_t *namePtr, Project *project, StringUtils::UnicodeStringScope &scope);
    void layoutTextWithParentBoneValidation(const nanoem_model_bone_t *bonePtr,
        const nanoem_model_bone_t *parentBonePtr, const char *titleID, const char *validationMessageID);
    void layoutBoneComboBox(const char *id, const nanoem_model_bone_t *baseBonePtr, nanoem_model_bone_t *bonePtr,
        PFN_nanoemMutableModelSetBoneObject setBoneCallback);
    void layoutRigidBodyComboBox(const char *id, const nanoem_model_rigid_body_t *baseRigidBodyPtr,
        nanoem_model_joint_t *jointPtr, PFN_nanoemMutableModelSetRigidBodyObject setRigidBodyCallback);
    void layoutBoneAxisMenuItems(nanoem_model_bone_t *bonePtr, PFN_nanoemMutableModelSetBoneAxis setBoneAxisCallback);

    void layoutManipulateVertexMenu(Project *project);
    void layoutCreateMaterialMenu(Project *project);
    void layoutManipulateMaterialMenu();
    void layoutCreateBoneMenu();
    void layoutManipulateBoneMenu();
    void layoutCreateMorphMenu();
    void layoutManipulateMorphMenu();
    void layoutCreateLabelMenu(const nanoem_model_label_t *selectedLabel);
    void layoutManipulateLabelMenu();
    void layoutCreateRigidBodyMenu();
    void layoutManipulateRigidBodyMenu();
    void layoutCreateJointMenu();
    void layoutManipulateJointMenu();
    void layoutCreateSoftBodyMenu();
    void layoutManipulateSoftBodyMenu();

    void toggleTab(TabType value, Project *project);
    void forceUpdateMorph(model::Morph *morph, Project *project);
    void setActiveModel(Model *model, Project *project);
    void resetMeasureState();

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
    void updateSortSelectedVertices(const IModelObjectSelection *selection);
    void updateSortSelectedMaterials(const IModelObjectSelection *selection);
    void updateSortSelectedBones(const IModelObjectSelection *selection);
    void updateSortSelectedMorphs(const IModelObjectSelection *selection);
    void updateSortSelectedRigidBodies(const IModelObjectSelection *selection);
    void updateSortSelectedJoints(const IModelObjectSelection *selection);
    void updateSortSelectedSoftBodies(const IModelObjectSelection *selection);
    void clearSortedSelectedVertices();
    void clearSortedSelectedMaterials();
    void clearSortedSelectedBones();
    void clearSortedSelectedMorphs();
    void clearSortedSelectedRigidBodies();
    void clearSortedSelectedJoints();
    void clearSortedSelectedSoftBodies();

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
    const char *selectedSoftBodyAeroModelType(
        const nanoem_model_soft_body_aero_model_type_t type) const NANOEM_DECL_NOEXCEPT;
    bool hasMorphType(nanoem_model_morph_type_t type) const NANOEM_DECL_NOEXCEPT;
    bool hasModelWithMaterial(const Project *project) const NANOEM_DECL_NOEXCEPT;
    bool isPMX21() const NANOEM_DECL_NOEXCEPT;

    ImGuiWindow *m_parent;
    Model *m_activeModel;
    SavedState *m_savedState;
    command::BatchChangeAllBoneObjectsCommand::Parameter m_batchBoneParameter;
    command::BatchChangeAllJointObjectsCommand::Parameter m_batchJointParameter;
    command::BatchChangeAllMaterialObjectsCommand::Parameter m_batchMaterialParameter;
    command::BatchChangeAllMorphObjectsCommand::Parameter m_batchMorphParameter;
    command::BatchChangeAllRigidBodyObjectsCommand::Parameter m_batchRigidBodyParameter;
    command::BatchChangeAllSoftBodyObjectsCommand::Parameter m_batchSoftBodyParameter;
    command::BatchChangeAllVertexObjectsCommand::Parameter m_batchVertexParameter;
    model::Vertex::List m_sortedSelectedVertices;
    model::Material::List m_sortedSelectedMaterials;
    model::Bone::List m_sortedSelectedBones;
    model::Morph::List m_sortedSelectedMorphs;
    model::Joint::List m_sortedSelectedJoints;
    model::RigidBody::List m_sortedSelectedRigidBodies;
    model::SoftBody::List m_sortedSelectedSoftBodies;
    nanoem_rsize_t m_sortedSelectedVertexIndex;
    nanoem_rsize_t m_sortedSelectedMaterialIndex;
    nanoem_rsize_t m_sortedSelectedBoneIndex;
    nanoem_rsize_t m_sortedSelectedMorphIndex;
    nanoem_rsize_t m_sortedSelectedRigidBodyIndex;
    nanoem_rsize_t m_sortedSelectedJointIndex;
    nanoem_rsize_t m_sortedSelectedSoftBodyIndex;
    int m_language;
    TabType m_tabType;
    TabType m_explicitTabType;
    nanoem_rsize_t m_vertexIndex;
    nanoem_rsize_t m_vertexCandidateUVAIndex;
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
    Vector3 m_savedTranslation;
    Vector3 m_savedRotation;
    Vector3 m_savedScale;
    nanoem_f32_t m_savedCMScaleFactor;
    nanoem_f32_t m_savedModelCorrectionHeight;
    nanoem_f32_t m_savedModelHeight;
    Project::EditingMode m_editingMode;
    bool m_heightBased;
    bool m_showAllVertexPoints;
    bool m_showAllVertexFaces;
    bool m_showAllBones;
    bool m_showAllRigidBodies;
    bool m_showAllJoints;
    bool m_showActiveBone;
    bool m_showFixedAxis;
    bool m_showLocalAxes;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_MODELPARAMETERDIALOG_H_ */
