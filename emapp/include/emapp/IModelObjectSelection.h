/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_IMODELOBJECTSELECTION_H_
#define NANOEM_EMAPP_IMODELOBJECTSELECTION_H_

#include "emapp/Forward.h"

#include "emapp/model/Bone.h"
#include "emapp/model/Joint.h"
#include "emapp/model/Label.h"
#include "emapp/model/Material.h"
#include "emapp/model/Morph.h"
#include "emapp/model/RigidBody.h"
#include "emapp/model/SoftBody.h"
#include "emapp/model/Vertex.h"

namespace nanoem {

class IModelObjectSelection {
public:
    typedef tinystl::vector<Vector4UI32, TinySTLAllocator> FaceList;
    enum EditingType {
        kEditingTypeFirstEnum,
        kEditingTypeNone = kEditingTypeFirstEnum,
        kEditingTypeInfo,
        kEditingTypeVertex,
        kEditingTypeFace,
        kEditingTypeMaterial,
        kEditingTypeBone,
        kEditingTypeMorph,
        kEditingTypeLabel,
        kEditingTypeRigidBody,
        kEditingTypeJoint,
        kEditingTypeSoftBody,
        kEditingTypeMaxEnum
    };
    enum SelectTargetModeType {
        kSelectTargetModeTypeFirstEnum,
        kSelectTargetModeTypePoint = kSelectTargetModeTypeFirstEnum,
        kSelectTargetModeTypeBox,
        kSelectTargetModeTypeCircle,
        kSelectTargetModeTypeMaxEnum
    };

    virtual ~IModelObjectSelection() NANOEM_DECL_NOEXCEPT
    {
    }

    virtual void addVertex(const nanoem_model_vertex_t *value) = 0;
    virtual void addBone(const nanoem_model_bone_t *value) = 0;
    virtual void addMaterial(const nanoem_model_material_t *value) = 0;
    virtual void addMorph(const nanoem_model_morph_t *value) = 0;
    virtual void addLabel(const nanoem_model_label_t *value) = 0;
    virtual void addRigidBody(const nanoem_model_rigid_body_t *value) = 0;
    virtual void addJoint(const nanoem_model_joint_t *value) = 0;
    virtual void addSoftBody(const nanoem_model_soft_body_t *value) = 0;
    virtual void addFace(const Vector4UI32 &value) = 0;
    virtual void addAllBones() = 0;
    virtual void addAllDirtyBones() = 0;
    virtual void addAllMovableBones() = 0;
    virtual void addAllMaterials() = 0;
    virtual void removeVertex(const nanoem_model_vertex_t *value) = 0;
    virtual void removeBone(const nanoem_model_bone_t *value) = 0;
    virtual void removeMaterial(const nanoem_model_material_t *value) = 0;
    virtual void removeMorph(const nanoem_model_morph_t *value) = 0;
    virtual void removeLabel(const nanoem_model_label_t *value) = 0;
    virtual void removeRigidBody(const nanoem_model_rigid_body_t *value) = 0;
    virtual void removeJoint(const nanoem_model_joint_t *value) = 0;
    virtual void removeSoftBody(const nanoem_model_soft_body_t *value) = 0;
    virtual void removeFace(const Vector4UI32 &value) = 0;
    virtual void removeAllVertices() = 0;
    virtual void removeAllBones() = 0;
    virtual void removeAllMaterials() = 0;
    virtual void removeAllMorphs() = 0;
    virtual void removeAllLabels() = 0;
    virtual void removeAllRigidBodies() = 0;
    virtual void removeAllJoints() = 0;
    virtual void removeAllSoftBodies() = 0;
    virtual void removeAllFaces() = 0;
    virtual void clearAll() = 0;
    virtual void toggleSelectAndActiveBone(const nanoem_model_bone_t *bone, bool isMultipleSelection) = 0;

    virtual void setHoveredVertex(nanoem_model_vertex_t *value) = 0;
    virtual void setHoveredMaterial(nanoem_model_material_t *value) = 0;
    virtual void setHoveredBone(nanoem_model_bone_t *value) = 0;
    virtual void setHoveredMorph(nanoem_model_morph_t *value) = 0;
    virtual void setHoveredLabel(nanoem_model_label_t *value) = 0;
    virtual void setHoveredRigidBody(nanoem_model_rigid_body_t *value) = 0;
    virtual void setHoveredJoint(nanoem_model_joint_t *value) = 0;
    virtual void setHoveredSoftBody(nanoem_model_soft_body_t *value) = 0;
    virtual void resetAllHoveredObjects() = 0;
    virtual nanoem_model_vertex_t *hoveredVertex() NANOEM_DECL_NOEXCEPT = 0;
    virtual nanoem_model_material_t *hoveredMaterial() NANOEM_DECL_NOEXCEPT = 0;
    virtual nanoem_model_bone_t *hoveredBone() NANOEM_DECL_NOEXCEPT = 0;
    virtual nanoem_model_morph_t *hoveredMorph() NANOEM_DECL_NOEXCEPT = 0;
    virtual nanoem_model_label_t *hoveredLabel() NANOEM_DECL_NOEXCEPT = 0;
    virtual nanoem_model_rigid_body_t *hoveredRigidBody() NANOEM_DECL_NOEXCEPT = 0;
    virtual nanoem_model_joint_t *hoveredJoint() NANOEM_DECL_NOEXCEPT = 0;
    virtual nanoem_model_soft_body_t *hoveredSoftBody() NANOEM_DECL_NOEXCEPT = 0;

    virtual model::Vertex::Set allVertexSet() const = 0;
    virtual model::Bone::Set allBoneSet() const = 0;
    virtual model::Material::Set allMaterialSet() const = 0;
    virtual model::Morph::Set allMorphSet() const = 0;
    virtual model::Label::Set allLabelSet() const = 0;
    virtual model::RigidBody::Set allRigidBodySet() const = 0;
    virtual model::Joint::Set allJointSet() const = 0;
    virtual model::SoftBody::Set allSoftBodySet() const = 0;
    virtual FaceList allFaces() const = 0;
    virtual bool containsVertex(const nanoem_model_vertex_t *value) const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool containsBone(const nanoem_model_bone_t *value) const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool containsMaterial(const nanoem_model_material_t *value) const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool containsMorph(const nanoem_model_morph_t *value) const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool containsLabel(const nanoem_model_label_t *value) const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool containsRigidBody(const nanoem_model_rigid_body_t *value) const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool containsJoint(const nanoem_model_joint_t *value) const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool containsSoftBody(const nanoem_model_soft_body_t *value) const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool containsFace(const Vector4UI32 &value) const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool containsAnyBone() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool areAllBonesMovable() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool areAllBonesRotateable() const NANOEM_DECL_NOEXCEPT = 0;

    virtual bool isBoxSelectedBoneModeEnabled() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setBoxSelectedBoneModeEnabled(bool value) = 0;
    virtual EditingType editingType() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setEditingType(EditingType value) = 0;
    virtual SelectTargetModeType targetMode() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setTargetMode(SelectTargetModeType value) = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_IMODELOBJECTSELECTION_H_ */
