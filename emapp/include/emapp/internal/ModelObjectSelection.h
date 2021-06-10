/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_MODELOBJECTSELECTION_H_
#define NANOEM_EMAPP_INTERNAL_MODELOBJECTSELECTION_H_

#include "emapp/IModelObjectSelection.h"

namespace nanoem {
namespace internal {

class ModelObjectSelection NANOEM_DECL_SEALED : public IModelObjectSelection {
public:
    static void sortedVertexList(const IModelObjectSelection *selection, model::Vertex::List &vertices);
    static void sortedMaterialList(const IModelObjectSelection *selection, model::Material::List &materials);
    static void sortedBoneList(const IModelObjectSelection *selection, model::Bone::List &bones);
    static void sortedMorphList(const IModelObjectSelection *selection, model::Morph::List &morphs);
    static void sortedRigidBodyList(const IModelObjectSelection *selection, model::RigidBody::List &rigidBodies);
    static void sortedJointList(const IModelObjectSelection *selection, model::Joint::List &joints);
    static void sortedSoftBodyList(const IModelObjectSelection *selection, model::SoftBody::List &softBodies);

    ModelObjectSelection(Model *parent);
    ~ModelObjectSelection() NANOEM_DECL_NOEXCEPT;

    void addVertex(const nanoem_model_vertex_t *value) NANOEM_DECL_OVERRIDE;
    void addBone(const nanoem_model_bone_t *value) NANOEM_DECL_OVERRIDE;
    void addMaterial(const nanoem_model_material_t *value) NANOEM_DECL_OVERRIDE;
    void addMorph(const nanoem_model_morph_t *value) NANOEM_DECL_OVERRIDE;
    void addLabel(const nanoem_model_label_t *value) NANOEM_DECL_OVERRIDE;
    void addRigidBody(const nanoem_model_rigid_body_t *value) NANOEM_DECL_OVERRIDE;
    void addJoint(const nanoem_model_joint_t *value) NANOEM_DECL_OVERRIDE;
    void addSoftBody(const nanoem_model_soft_body_t *value) NANOEM_DECL_OVERRIDE;
    void addFace(const Vector4UI32 &value) NANOEM_DECL_OVERRIDE;
    void addAllBones() NANOEM_DECL_OVERRIDE;
    void addAllDirtyBones() NANOEM_DECL_OVERRIDE;
    void addAllMovableBones() NANOEM_DECL_OVERRIDE;
    void addAllMaterials() NANOEM_DECL_OVERRIDE;
    void removeVertex(const nanoem_model_vertex_t *value) NANOEM_DECL_OVERRIDE;
    void removeBone(const nanoem_model_bone_t *value) NANOEM_DECL_OVERRIDE;
    void removeMaterial(const nanoem_model_material_t *value) NANOEM_DECL_OVERRIDE;
    void removeMorph(const nanoem_model_morph_t *value) NANOEM_DECL_OVERRIDE;
    void removeLabel(const nanoem_model_label_t *value) NANOEM_DECL_OVERRIDE;
    void removeRigidBody(const nanoem_model_rigid_body_t *value) NANOEM_DECL_OVERRIDE;
    void removeJoint(const nanoem_model_joint_t *value) NANOEM_DECL_OVERRIDE;
    void removeSoftBody(const nanoem_model_soft_body_t *value) NANOEM_DECL_OVERRIDE;
    void removeFace(const Vector4UI32 &value) NANOEM_DECL_OVERRIDE;
    void removeAllVertices() NANOEM_DECL_OVERRIDE;
    void removeAllBones() NANOEM_DECL_OVERRIDE;
    void removeAllMaterials() NANOEM_DECL_OVERRIDE;
    void removeAllMorphs() NANOEM_DECL_OVERRIDE;
    void removeAllLabels() NANOEM_DECL_OVERRIDE;
    void removeAllRigidBodies() NANOEM_DECL_OVERRIDE;
    void removeAllJoints() NANOEM_DECL_OVERRIDE;
    void removeAllSoftBodies() NANOEM_DECL_OVERRIDE;
    void removeAllFaces() NANOEM_DECL_OVERRIDE;
    void clearAll() NANOEM_DECL_OVERRIDE;
    void toggleSelectAndActiveBone(const nanoem_model_bone_t *bone, bool isMultipleSelection) NANOEM_DECL_OVERRIDE;

    nanoem_rsize_t countAllVertices() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    nanoem_rsize_t countAllMaterials() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    nanoem_rsize_t countAllBones() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    nanoem_rsize_t countAllMorphs() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    nanoem_rsize_t countAllLabels() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    nanoem_rsize_t countAllRigidBodies() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    nanoem_rsize_t countAllJoints() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    nanoem_rsize_t countAllSoftBodies() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    nanoem_rsize_t countAllFaces() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

    const model::Vertex::Set *allVertexSet() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const model::Bone::Set *allBoneSet() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const model::Material::Set *allMaterialSet() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const model::Morph::Set *allMorphSet() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const model::Label::Set *allLabelSet() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const model::RigidBody::Set *allRigidBodySet() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const model::Joint::Set *allJointSet() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const model::SoftBody::Set *allSoftBodySet() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    FaceList allFaces() const NANOEM_DECL_OVERRIDE;
    Matrix4x4 pivotMatrix() const NANOEM_DECL_OVERRIDE;

    bool containsVertex(const nanoem_model_vertex_t *value) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool containsBone(const nanoem_model_bone_t *value) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool containsMaterial(const nanoem_model_material_t *value) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool containsMorph(const nanoem_model_morph_t *value) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool containsLabel(const nanoem_model_label_t *value) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool containsRigidBody(const nanoem_model_rigid_body_t *value) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool containsJoint(const nanoem_model_joint_t *value) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool containsSoftBody(const nanoem_model_soft_body_t *value) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool containsFace(const Vector4UI32 &value) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool areAllBonesMovable() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool areAllBonesRotateable() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool containsAnyBone() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

    bool isBoxSelectedBoneModeEnabled() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setBoxSelectedBoneModeEnabled(bool value) NANOEM_DECL_OVERRIDE;
    ObjectType objectType() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setObjectType(ObjectType value) NANOEM_DECL_OVERRIDE;
    TargetModeType targetMode() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setTargetMode(TargetModeType value) NANOEM_DECL_OVERRIDE;

private:
    typedef tinystl::unordered_set<Vector4UI32, TinySTLAllocator> FaceSet;
    static void assignAxisAlignedBoundingBox(
        const Vector3 &value, Vector3 &aabbMin, Vector3 &aabbMax) NANOEM_DECL_NOEXCEPT;
    static void assignPivotMatrixFromAABB(
        const Vector3 &aabbMin, const Vector3 &aabbMax, Matrix4x4 &matrix) NANOEM_DECL_NOEXCEPT;

    Model *m_parent;
    model::Vertex::Set m_selectedVertexSet;
    model::Bone::Set m_selectedBoneSet;
    model::Material::Set m_selectedMaterialSet;
    model::Morph::Set m_selectedMorphSet;
    model::Label::Set m_selectedLabelSet;
    model::RigidBody::Set m_selectedRigidBodySet;
    model::Joint::Set m_selectedJointSet;
    model::SoftBody::Set m_selectedSoftBodySet;
    FaceSet m_selectedFaceSet;
    nanoem_model_vertex_t *m_hoveredVertex;
    nanoem_model_material_t *m_hoveredMaterial;
    nanoem_model_bone_t *m_hoveredBone;
    nanoem_model_morph_t *m_hoveredMorph;
    nanoem_model_label_t *m_hoveredLabel;
    nanoem_model_rigid_body_t *m_hoveredRigidBody;
    nanoem_model_joint_t *m_hoveredJoint;
    nanoem_model_soft_body_t *m_hoveredSoftBody;
    ObjectType m_editingType;
    TargetModeType m_targetMode;
    bool m_boxSelectedBoneModeEnabled;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_MODELOBJECTSELECTION_H_ */
