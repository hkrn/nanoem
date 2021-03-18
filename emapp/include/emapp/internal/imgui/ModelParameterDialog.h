/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_MODELPARAMETERDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_MODELPARAMETERDIALOG_H_

#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

#include "emapp/Model.h"
#include "emapp/StringUtils.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct ModelParameterDialog : BaseNonModalDialogWindow {
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
    struct ScopedMutableModel {
        ScopedMutableModel(Model *model)
            : m_model(nullptr)
        {
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_model = nanoemMutableModelCreateAsReference(model->data(), &status);
        }
        ~ScopedMutableModel() NANOEM_DECL_NOEXCEPT
        {
            nanoemMutableModelDestroy(m_model);
            m_model = nullptr;
        }
        operator nanoem_mutable_model_t *() NANOEM_DECL_NOEXCEPT
        {
            return m_model;
        }
        nanoem_mutable_model_t *m_model;
    };
    struct ScopedMutableVertex {
        ScopedMutableVertex(Model *model)
            : m_vertex(nullptr)
        {
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_vertex = nanoemMutableModelVertexCreate(model->data(), &status);
        }
        ScopedMutableVertex(nanoem_model_vertex_t *vertexPtr, Model *model)
            : m_vertex(nullptr)
        {
            BX_UNUSED_1(model);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_vertex = nanoemMutableModelVertexCreateAsReference(vertexPtr, &status);
        }
        ~ScopedMutableVertex() NANOEM_DECL_NOEXCEPT
        {
            nanoemMutableModelVertexDestroy(m_vertex);
            m_vertex = nullptr;
        }
        operator nanoem_mutable_model_vertex_t *() NANOEM_DECL_NOEXCEPT
        {
            return m_vertex;
        }
        nanoem_mutable_model_vertex_t *m_vertex;
    };
    struct ScopedMutableMaterial {
        ScopedMutableMaterial(Model *model)
            : m_material(nullptr)
        {
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_material = nanoemMutableModelMaterialCreate(model->data(), &status);
        }
        ScopedMutableMaterial(nanoem_model_material_t *materialPtr, Model *model)
            : m_material(nullptr)
        {
            BX_UNUSED_1(model);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_material = nanoemMutableModelMaterialCreateAsReference(materialPtr, &status);
        }
        ~ScopedMutableMaterial() NANOEM_DECL_NOEXCEPT
        {
            nanoemMutableModelMaterialDestroy(m_material);
            m_material = nullptr;
        }
        operator nanoem_mutable_model_material_t *() NANOEM_DECL_NOEXCEPT
        {
            return m_material;
        }
        nanoem_mutable_model_material_t *m_material;
    };
    struct ScopedMutableBone {
        ScopedMutableBone(Model *model)
            : m_bone(nullptr)
        {
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_bone = nanoemMutableModelBoneCreate(model->data(), &status);
        }
        ScopedMutableBone(nanoem_model_bone_t *bonePtr, Model *model)
            : m_bone(nullptr)
        {
            BX_UNUSED_1(model);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_bone = nanoemMutableModelBoneCreateAsReference(bonePtr, &status);
        }
        ~ScopedMutableBone() NANOEM_DECL_NOEXCEPT
        {
            nanoemMutableModelBoneDestroy(m_bone);
            m_bone = nullptr;
        }
        operator nanoem_mutable_model_bone_t *() NANOEM_DECL_NOEXCEPT
        {
            return m_bone;
        }
        nanoem_mutable_model_bone_t *m_bone;
    };
    struct ScopedMutableConstraint {
        ScopedMutableConstraint(Model *model)
            : m_constraint(nullptr)
        {
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_constraint = nanoemMutableModelConstraintCreate(model->data(), &status);
        }
        ScopedMutableConstraint(nanoem_model_constraint_t *constraintPtr, Model *model)
            : m_constraint(nullptr)
        {
            BX_UNUSED_1(model);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_constraint = nanoemMutableModelConstraintCreateAsReference(constraintPtr, &status);
        }
        ~ScopedMutableConstraint() NANOEM_DECL_NOEXCEPT
        {
            nanoemMutableModelConstraintDestroy(m_constraint);
            m_constraint = nullptr;
        }
        operator nanoem_mutable_model_constraint_t *() NANOEM_DECL_NOEXCEPT
        {
            return m_constraint;
        }
        nanoem_mutable_model_constraint_t *m_constraint;
    };
    struct ScopedMutableConstraintJoint {
        ScopedMutableConstraintJoint(nanoem_model_constraint_joint_t *jointPtr, Model *model)
            : m_joint(nullptr)
        {
            BX_UNUSED_1(model);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_joint = nanoemMutableModelConstraintJointCreateAsReference(jointPtr, &status);
        }
        ~ScopedMutableConstraintJoint() NANOEM_DECL_NOEXCEPT
        {
            nanoemMutableModelConstraintJointDestroy(m_joint);
            m_joint = nullptr;
        }
        operator nanoem_mutable_model_constraint_joint_t *() NANOEM_DECL_NOEXCEPT
        {
            return m_joint;
        }
        nanoem_mutable_model_constraint_joint_t *m_joint;
    };
    struct ScopedMutableMorph {
        ScopedMutableMorph(Model *model)
            : m_morph(nullptr)
        {
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_morph = nanoemMutableModelMorphCreate(model->data(), &status);
        }
        ScopedMutableMorph(nanoem_model_morph_t *morphPtr, Model *model)
            : m_morph(nullptr)
        {
            BX_UNUSED_1(model);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_morph = nanoemMutableModelMorphCreateAsReference(morphPtr, &status);
        }
        ~ScopedMutableMorph() NANOEM_DECL_NOEXCEPT
        {
            nanoemMutableModelMorphDestroy(m_morph);
            m_morph = nullptr;
        }
        operator nanoem_mutable_model_morph_t *() NANOEM_DECL_NOEXCEPT
        {
            return m_morph;
        }
        nanoem_mutable_model_morph_t *m_morph;
    };
    struct ScopedMutableMorphBone {
        ScopedMutableMorphBone(nanoem_model_morph_bone_t *morphPtr, Model *model)
            : m_morph(nullptr)
        {
            BX_UNUSED_1(model);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_morph = nanoemMutableModelMorphBoneCreateAsReference(morphPtr, &status);
        }
        ~ScopedMutableMorphBone() NANOEM_DECL_NOEXCEPT
        {
            nanoemMutableModelMorphBoneDestroy(m_morph);
            m_morph = nullptr;
        }
        operator nanoem_mutable_model_morph_bone_t *() NANOEM_DECL_NOEXCEPT
        {
            return m_morph;
        }
        nanoem_mutable_model_morph_bone_t *m_morph;
    };
    struct ScopedMutableMorphFlip {
        ScopedMutableMorphFlip(nanoem_model_morph_flip_t *morphPtr, Model *model)
            : m_morph(nullptr)
        {
            BX_UNUSED_1(model);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_morph = nanoemMutableModelMorphFlipCreateAsReference(morphPtr, &status);
        }
        ~ScopedMutableMorphFlip() NANOEM_DECL_NOEXCEPT
        {
            nanoemMutableModelMorphFlipDestroy(m_morph);
            m_morph = nullptr;
        }
        operator nanoem_mutable_model_morph_flip_t *() NANOEM_DECL_NOEXCEPT
        {
            return m_morph;
        }
        nanoem_mutable_model_morph_flip_t *m_morph;
    };
    struct ScopedMutableMorphGroup {
        ScopedMutableMorphGroup(nanoem_model_morph_group_t *morphPtr, Model *model)
            : m_morph(nullptr)
        {
            BX_UNUSED_1(model);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_morph = nanoemMutableModelMorphGroupCreateAsReference(morphPtr, &status);
        }
        ~ScopedMutableMorphGroup() NANOEM_DECL_NOEXCEPT
        {
            nanoemMutableModelMorphGroupDestroy(m_morph);
            m_morph = nullptr;
        }
        operator nanoem_mutable_model_morph_group_t *() NANOEM_DECL_NOEXCEPT
        {
            return m_morph;
        }
        nanoem_mutable_model_morph_group_t *m_morph;
    };
    struct ScopedMutableMorphImpulse {
        ScopedMutableMorphImpulse(nanoem_model_morph_impulse_t *morphPtr, Model *model)
            : m_morph(nullptr)
        {
            BX_UNUSED_1(model);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_morph = nanoemMutableModelMorphImpulseCreateAsReference(morphPtr, &status);
        }
        ~ScopedMutableMorphImpulse() NANOEM_DECL_NOEXCEPT
        {
            nanoemMutableModelMorphImpulseDestroy(m_morph);
            m_morph = nullptr;
        }
        operator nanoem_mutable_model_morph_impulse_t *() NANOEM_DECL_NOEXCEPT
        {
            return m_morph;
        }
        nanoem_mutable_model_morph_impulse_t *m_morph;
    };
    struct ScopedMutableMorphMaterial {
        ScopedMutableMorphMaterial(nanoem_model_morph_material_t *morphPtr, Model *model)
            : m_morph(nullptr)
        {
            BX_UNUSED_1(model);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_morph = nanoemMutableModelMorphMaterialCreateAsReference(morphPtr, &status);
        }
        ~ScopedMutableMorphMaterial() NANOEM_DECL_NOEXCEPT
        {
            nanoemMutableModelMorphMaterialDestroy(m_morph);
            m_morph = nullptr;
        }
        operator nanoem_mutable_model_morph_material_t *() NANOEM_DECL_NOEXCEPT
        {
            return m_morph;
        }
        nanoem_mutable_model_morph_material_t *m_morph;
    };
    struct ScopedMutableMorphUV {
        ScopedMutableMorphUV(nanoem_model_morph_uv_t *morphPtr, Model *model)
            : m_morph(nullptr)
        {
            BX_UNUSED_1(model);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_morph = nanoemMutableModelMorphUVCreateAsReference(morphPtr, &status);
        }
        ~ScopedMutableMorphUV() NANOEM_DECL_NOEXCEPT
        {
            nanoemMutableModelMorphUVDestroy(m_morph);
            m_morph = nullptr;
        }
        operator nanoem_mutable_model_morph_uv_t *() NANOEM_DECL_NOEXCEPT
        {
            return m_morph;
        }
        nanoem_mutable_model_morph_uv_t *m_morph;
    };
    struct ScopedMutableMorphVertex {
        ScopedMutableMorphVertex(nanoem_model_morph_vertex_t *morphPtr, Model *model)
            : m_morph(nullptr)
        {
            BX_UNUSED_1(model);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_morph = nanoemMutableModelMorphVertexCreateAsReference(morphPtr, &status);
        }
        ~ScopedMutableMorphVertex() NANOEM_DECL_NOEXCEPT
        {
            nanoemMutableModelMorphVertexDestroy(m_morph);
            m_morph = nullptr;
        }
        operator nanoem_mutable_model_morph_vertex_t *() NANOEM_DECL_NOEXCEPT
        {
            return m_morph;
        }
        nanoem_mutable_model_morph_vertex_t *m_morph;
    };
    struct ScopedMutableLabel {
        ScopedMutableLabel(Model *model)
            : m_label(nullptr)
        {
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_label = nanoemMutableModelLabelCreate(model->data(), &status);
        }
        ScopedMutableLabel(nanoem_model_label_t *labelPtr, Model *model)
            : m_label(nullptr)
        {
            BX_UNUSED_1(model);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_label = nanoemMutableModelLabelCreateAsReference(labelPtr, &status);
        }
        ~ScopedMutableLabel() NANOEM_DECL_NOEXCEPT
        {
            nanoemMutableModelLabelDestroy(m_label);
            m_label = nullptr;
        }
        operator nanoem_mutable_model_label_t *() NANOEM_DECL_NOEXCEPT
        {
            return m_label;
        }
        nanoem_mutable_model_label_t *m_label;
    };
    struct ScopedMutableRigidBody {
        ScopedMutableRigidBody(Model *model)
            : m_rigidBody(nullptr)
        {
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_rigidBody = nanoemMutableModelRigidBodyCreate(model->data(), &status);
        }
        ScopedMutableRigidBody(nanoem_model_rigid_body_t *rigidBodyPtr, Model *model)
            : m_rigidBody(nullptr)
        {
            BX_UNUSED_1(model);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_rigidBody = nanoemMutableModelRigidBodyCreateAsReference(rigidBodyPtr, &status);
        }
        ~ScopedMutableRigidBody() NANOEM_DECL_NOEXCEPT
        {
            nanoemMutableModelRigidBodyDestroy(m_rigidBody);
            m_rigidBody = nullptr;
        }
        operator nanoem_mutable_model_rigid_body_t *() NANOEM_DECL_NOEXCEPT
        {
            return m_rigidBody;
        }
        nanoem_mutable_model_rigid_body_t *m_rigidBody;
    };
    struct ScopedMutableJoint {
        ScopedMutableJoint(Model *model)
            : m_joint(nullptr)
        {
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_joint = nanoemMutableModelJointCreate(model->data(), &status);
        }
        ScopedMutableJoint(nanoem_model_joint_t *jointPtr, Model *model)
            : m_joint(nullptr)
        {
            BX_UNUSED_1(model);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_joint = nanoemMutableModelJointCreateAsReference(jointPtr, &status);
        }
        ~ScopedMutableJoint() NANOEM_DECL_NOEXCEPT
        {
            nanoemMutableModelJointDestroy(m_joint);
            m_joint = nullptr;
        }
        operator nanoem_mutable_model_joint_t *() NANOEM_DECL_NOEXCEPT
        {
            return m_joint;
        }
        nanoem_mutable_model_joint_t *m_joint;
    };
    struct ScopedMutableSoftBody {
        ScopedMutableSoftBody(Model *model)
            : m_softBody(nullptr)
        {
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_softBody = nanoemMutableModelSoftBodyCreate(model->data(), &status);
        }
        ScopedMutableSoftBody(nanoem_model_soft_body_t *softBodyPtr, Model *model)
            : m_softBody(nullptr)
        {
            BX_UNUSED_1(model);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            m_softBody = nanoemMutableModelSoftBodyCreateAsReference(softBodyPtr, &status);
        }
        ~ScopedMutableSoftBody() NANOEM_DECL_NOEXCEPT
        {
            nanoemMutableModelSoftBodyDestroy(m_softBody);
            m_softBody = nullptr;
        }
        operator nanoem_mutable_model_soft_body_t *() NANOEM_DECL_NOEXCEPT
        {
            return m_softBody;
        }
        nanoem_mutable_model_soft_body_t *m_softBody;
    };

    static void formatVertexText(char *buffer, nanoem_rsize_t size, const nanoem_model_vertex_t *vertexPtr);

    ModelParameterDialog(Model *model, Project *project, BaseApplicationService *applicationPtr, ImGuiWindow *parent);

    bool draw(Project *project);
    void layoutInformation(Project *project);
    void layoutVertexBoneSelection(const char *label, nanoem_model_vertex_t *vertexPtr, nanoem_rsize_t i,
        nanoem_model_bone_t *const *bones, nanoem_rsize_t numBones);
    void layoutVertexBoneWeights(nanoem_model_vertex_t *vertexPtr, nanoem_rsize_t numItems);
    void layoutAllVertices(Project *project);
    void layoutVertexPropertyPane(nanoem_model_vertex_t *vertexPtr);
    void layoutAllFaces(Project *project);
    void layoutAllMaterials(Project *project);
    void layoutMaterialPropertyPane(nanoem_model_material_t *materialPtr, Project *project);
    void layoutMaterialImage(const IImageView *image, const char *label, const String &filename);
    void layoutAllBones(Project *project);
    void layoutBonePropertyPane(nanoem_model_bone_t *bonePtr, Project *project);
    void layoutBoneConstraintPanel(nanoem_model_bone_t *bonePtr, Project *project);
    void layoutAllMorphs(Project *project);
    void layoutMorphPropertyPane(nanoem_model_morph_t *morphPtr, Project *project);
    void layoutAllLabels(Project *project);
    void layoutLabelPropertyPane(nanoem_model_label_t *labelPtr, Project *project);
    void layoutAllRigidBodies(Project *project);
    void layoutRigidBodyPropertyPane(nanoem_model_rigid_body_t *rigidBodyPtr, Project *project);
    void layoutAllJoints(Project *project);
    void layoutJointPropertyPane(nanoem_model_joint_t *jointPtr, Project *project);
    void layoutAllSoftBodies(Project *project);
    void layoutSoftBodyPropertyPane(nanoem_model_soft_body_t *softBodyPtr, Project *project);
    bool layoutName(const nanoem_unicode_string_t *namePtr, Project *project, StringUtils::UnicodeStringScope &scope);
    void toggleTab(TabType value, Project *project);
    void beforeToggleTab(Project *project);
    void afterToggleTab(TabType value, Project *project);
    void forceUpdateMorph(model::Morph *morph, Project *project);

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

    ImGuiWindow *m_parent;
    Model *m_activeModel;
    int m_language;
    TabType m_tabType;
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
