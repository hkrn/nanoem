/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/ModelObjectSelection.h"

#include "emapp/Model.h"
#include "emapp/Project.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/hash.hpp"

namespace nanoem {
namespace internal {

ModelObjectSelection::ModelObjectSelection(Model *parent)
    : m_parent(parent)
    , m_hoveredVertex(nullptr)
    , m_hoveredMaterial(nullptr)
    , m_hoveredBone(nullptr)
    , m_hoveredMorph(nullptr)
    , m_hoveredLabel(nullptr)
    , m_hoveredRigidBody(nullptr)
    , m_hoveredJoint(nullptr)
    , m_hoveredSoftBody(nullptr)
    , m_editingType(kEditingTypeNone)
    , m_targetMode(kSelectTargetModeTypePoint)
    , m_boxSelectedBoneModeEnabled(false)
{
}

ModelObjectSelection::~ModelObjectSelection() NANOEM_DECL_NOEXCEPT
{
}

void
ModelObjectSelection::addVertex(const nanoem_model_vertex_t *value)
{
    m_selectedVertexSet.insert(value);
}

void
ModelObjectSelection::addBone(const nanoem_model_bone_t *value)
{
    m_selectedBoneSet.insert(value);
    m_parent->setActiveBone(value);
}

void
ModelObjectSelection::addMaterial(const nanoem_model_material_t *value)
{
    m_selectedMaterialSet.insert(value);
}

void
ModelObjectSelection::addMorph(const nanoem_model_morph_t *value)
{
    m_selectedMorphSet.insert(value);
}

void
ModelObjectSelection::addLabel(const nanoem_model_label_t *value)
{
    m_selectedLabelSet.insert(value);
}

void
ModelObjectSelection::addRigidBody(const nanoem_model_rigid_body_t *value)
{
    m_selectedRigidBodySet.insert(value);
}

void
ModelObjectSelection::addJoint(const nanoem_model_joint_t *value)
{
    m_selectedJointSet.insert(value);
}

void
ModelObjectSelection::addSoftBody(const nanoem_model_soft_body_t *value)
{
    m_selectedSoftBodySet.insert(value);
}

void
ModelObjectSelection::addFace(const Vector3UI32 &value)
{
    m_selectedFaceSet.insert(value);
    m_vertexIndexSet.insert(value.x);
    m_vertexIndexSet.insert(value.y);
    m_vertexIndexSet.insert(value.z);
}

void
ModelObjectSelection::addAllBones()
{
    nanoem_rsize_t numObjects;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_parent->data(), &numObjects);
    m_selectedBoneSet.clear();
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_bone_t *bone = bones[i];
        if (m_parent->isBoneSelectable(bone)) {
            if (!m_parent->activeBone()) {
                m_parent->setActiveBone(bone);
            }
            m_selectedBoneSet.insert(bone);
        }
    }
}

void
ModelObjectSelection::addAllDirtyBones()
{
    nanoem_rsize_t numObjects;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_parent->data(), &numObjects);
    m_selectedBoneSet.clear();
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_bone_t *bonePtr = bones[i];
        const model::Bone *bone = model::Bone::cast(bonePtr);
        if (bone && bone->isDirty()) {
            if (!m_parent->activeBone()) {
                m_parent->setActiveBone(bonePtr);
            }
            m_selectedBoneSet.insert(bonePtr);
        }
    }
}

void
ModelObjectSelection::addAllMovableBones()
{
    nanoem_rsize_t numObjects;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_parent->data(), &numObjects);
    m_selectedBoneSet.clear();
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_bone_t *bone = bones[i];
        if (nanoemModelBoneIsUserHandleable(bone) && nanoemModelBoneIsMovable(bone) &&
            nanoemModelBoneIsRotateable(bone)) {
            if (!m_parent->activeBone()) {
                m_parent->setActiveBone(bone);
            }
            m_selectedBoneSet.insert(bone);
        }
    }
}

void
ModelObjectSelection::addAllMaterials()
{
    nanoem_rsize_t numObjects;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_parent->data(), &numObjects);
    m_selectedMaterialSet.clear();
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_material_t *materialPtr = materials[i];
        const model::Material *material = model::Material::cast(materialPtr);
        if (material->isVisible()) {
            m_selectedMaterialSet.insert(materialPtr);
        }
    }
}

void
ModelObjectSelection::removeVertex(const nanoem_model_vertex_t *value)
{
    m_selectedVertexSet.erase(value);
}

void
ModelObjectSelection::removeBone(const nanoem_model_bone_t *value)
{
    m_selectedBoneSet.erase(value);
    if (m_parent->activeBone() == value) {
        m_parent->setActiveBone(nullptr);
    }
}

void
ModelObjectSelection::removeMaterial(const nanoem_model_material_t *value)
{
    m_selectedMaterialSet.erase(value);
    if (m_parent->activeMaterial() == value) {
        m_parent->setActiveMaterial(nullptr);
    }
}

void
ModelObjectSelection::removeMorph(const nanoem_model_morph_t *value)
{
    m_selectedMorphSet.erase(value);
}

void
ModelObjectSelection::removeLabel(const nanoem_model_label_t *value)
{
    m_selectedLabelSet.erase(value);
}

void
ModelObjectSelection::removeRigidBody(const nanoem_model_rigid_body_t *value)
{
    m_selectedRigidBodySet.erase(value);
}

void
ModelObjectSelection::removeJoint(const nanoem_model_joint_t *value)
{
    m_selectedJointSet.erase(value);
}

void
ModelObjectSelection::removeSoftBody(const nanoem_model_soft_body_t *value)
{
    m_selectedSoftBodySet.erase(value);
}

void
ModelObjectSelection::removeFace(const Vector3UI32 &value)
{
    m_selectedFaceSet.erase(value);
    m_vertexIndexSet.erase(value.x);
    m_vertexIndexSet.erase(value.y);
    m_vertexIndexSet.erase(value.z);
}

void
ModelObjectSelection::removeAllVertices()
{
    m_selectedVertexSet.clear();
}

void
ModelObjectSelection::removeAllBones()
{
    m_parent->setActiveBone(nullptr);
    m_selectedBoneSet.clear();
}

void
ModelObjectSelection::removeAllMaterials()
{
    m_parent->setActiveMaterial(nullptr);
    m_selectedMaterialSet.clear();
}

void
ModelObjectSelection::removeAllMorphs()
{
    m_selectedMorphSet.clear();
}

void
ModelObjectSelection::removeAllLabels()
{
    m_selectedLabelSet.clear();
}

void
ModelObjectSelection::removeAllRigidBodies()
{
    m_selectedRigidBodySet.clear();
}

void
ModelObjectSelection::removeAllJoints()
{
    m_selectedJointSet.clear();
}

void
ModelObjectSelection::removeAllSoftBodies()
{
    m_selectedSoftBodySet.clear();
}

void
ModelObjectSelection::removeAllFaces()
{
    m_selectedFaceSet.clear();
    m_vertexIndexSet.clear();
}

void
ModelObjectSelection::clearAll()
{
    removeAllBones();
    removeAllJoints();
    removeAllLabels();
    removeAllMorphs();
    removeAllVertices();
    removeAllMaterials();
    removeAllRigidBodies();
    removeAllSoftBodies();
    removeAllFaces();
    m_hoveredBone = nullptr;
    m_hoveredJoint = nullptr;
    m_hoveredLabel = nullptr;
    m_hoveredMorph = nullptr;
    m_hoveredVertex = nullptr;
    m_hoveredMaterial = nullptr;
    m_hoveredRigidBody = nullptr;
    m_hoveredSoftBody = nullptr;
}

void
ModelObjectSelection::toggleSelectAndActiveBone(const nanoem_model_bone_t *bone, bool isMultipleSelection)
{
    if (m_selectedBoneSet.find(bone) == m_selectedBoneSet.end()) {
        if (!isMultipleSelection) {
            removeAllBones();
        }
        addBone(bone);
    }
    else if (isMultipleSelection) {
        removeBone(bone);
    }
    else {
        removeAllBones();
        addBone(bone);
    }
}

void
ModelObjectSelection::setHoveredVertex(nanoem_model_vertex_t *value)
{
    m_hoveredVertex = value;
}

void
ModelObjectSelection::setHoveredMaterial(nanoem_model_material_t *value)
{
    m_hoveredMaterial = value;
}

void
ModelObjectSelection::setHoveredBone(nanoem_model_bone_t *value)
{
    m_hoveredBone = value;
}

void
ModelObjectSelection::setHoveredMorph(nanoem_model_morph_t *value)
{
    m_hoveredMorph = value;
}

void
ModelObjectSelection::setHoveredLabel(nanoem_model_label_t *value)
{
    m_hoveredLabel = value;
}

void
ModelObjectSelection::setHoveredRigidBody(nanoem_model_rigid_body_t *value)
{
    m_hoveredRigidBody = value;
}

void
ModelObjectSelection::setHoveredJoint(nanoem_model_joint_t *value)
{
    m_hoveredJoint = value;
}

void
ModelObjectSelection::setHoveredSoftBody(nanoem_model_soft_body_t *value)
{
    m_hoveredSoftBody = value;
}

void
ModelObjectSelection::resetAllHoveredObjects()
{
    m_hoveredVertex = nullptr;
    m_hoveredMaterial = nullptr;
    m_hoveredBone = nullptr;
    m_hoveredMorph = nullptr;
    m_hoveredLabel = nullptr;
    m_hoveredRigidBody = nullptr;
    m_hoveredJoint = nullptr;
    m_hoveredSoftBody = nullptr;
}

nanoem_model_vertex_t *
ModelObjectSelection::hoveredVertex() NANOEM_DECL_NOEXCEPT
{
    return m_hoveredVertex;
}

nanoem_model_material_t *
ModelObjectSelection::hoveredMaterial() NANOEM_DECL_NOEXCEPT
{
    return m_hoveredMaterial;
}

nanoem_model_bone_t *
ModelObjectSelection::hoveredBone() NANOEM_DECL_NOEXCEPT
{
    return m_hoveredBone;
}

nanoem_model_morph_t *
ModelObjectSelection::hoveredMorph() NANOEM_DECL_NOEXCEPT
{
    return m_hoveredMorph;
}

nanoem_model_label_t *
ModelObjectSelection::hoveredLabel() NANOEM_DECL_NOEXCEPT
{
    return m_hoveredLabel;
}

nanoem_model_rigid_body_t *
ModelObjectSelection::hoveredRigidBody() NANOEM_DECL_NOEXCEPT
{
    return m_hoveredRigidBody;
}

nanoem_model_joint_t *
ModelObjectSelection::hoveredJoint() NANOEM_DECL_NOEXCEPT
{
    return m_hoveredJoint;
}

nanoem_model_soft_body_t *
ModelObjectSelection::hoveredSoftBody() NANOEM_DECL_NOEXCEPT
{
    return m_hoveredSoftBody;
}

model::Vertex::Set
ModelObjectSelection::allVertexSet() const
{
    return m_selectedVertexSet;
}

model::Bone::Set
ModelObjectSelection::allBoneSet() const
{
    return m_selectedBoneSet;
}

model::Material::Set
ModelObjectSelection::allMaterialSet() const
{
    return m_selectedMaterialSet;
}

model::Morph::Set
ModelObjectSelection::allMorphSet() const
{
    return m_selectedMorphSet;
}

model::Label::Set
ModelObjectSelection::allLabelSet() const
{
    return m_selectedLabelSet;
}

model::RigidBody::Set
ModelObjectSelection::allRigidBodySet() const
{
    return m_selectedRigidBodySet;
}

model::Joint::Set
ModelObjectSelection::allJointSet() const
{
    return m_selectedJointSet;
}

model::SoftBody::Set
ModelObjectSelection::allSoftBodySet() const
{
    return m_selectedSoftBodySet;
}

IModelObjectSelection::VertexIndexSet
ModelObjectSelection::allVertexIndexSet() const
{
    return m_vertexIndexSet;
}

bool
ModelObjectSelection::containsVertex(const nanoem_model_vertex_t *value) const NANOEM_DECL_NOEXCEPT
{
    return m_selectedVertexSet.find(value) != m_selectedVertexSet.end();
}

bool
ModelObjectSelection::containsBone(const nanoem_model_bone_t *value) const NANOEM_DECL_NOEXCEPT
{
    return m_selectedBoneSet.find(value) != m_selectedBoneSet.end();
}

bool
ModelObjectSelection::containsMaterial(const nanoem_model_material_t *value) const NANOEM_DECL_NOEXCEPT
{
    return m_selectedMaterialSet.find(value) != m_selectedMaterialSet.end();
}

bool
ModelObjectSelection::containsMorph(const nanoem_model_morph_t *value) const NANOEM_DECL_NOEXCEPT
{
    return m_selectedMorphSet.find(value) != m_selectedMorphSet.end();
}

bool
ModelObjectSelection::containsLabel(const nanoem_model_label_t *value) const NANOEM_DECL_NOEXCEPT
{
    return m_selectedLabelSet.find(value) != m_selectedLabelSet.end();
}

bool
ModelObjectSelection::containsRigidBody(const nanoem_model_rigid_body_t *value) const NANOEM_DECL_NOEXCEPT
{
    return m_selectedRigidBodySet.find(value) != m_selectedRigidBodySet.end();
}

bool
ModelObjectSelection::containsJoint(const nanoem_model_joint_t *value) const NANOEM_DECL_NOEXCEPT
{
    return m_selectedJointSet.find(value) != m_selectedJointSet.end();
}

bool
ModelObjectSelection::containsSoftBody(const nanoem_model_soft_body_t *value) const NANOEM_DECL_NOEXCEPT
{
    return m_selectedSoftBodySet.find(value) != m_selectedSoftBodySet.end();
}

bool
ModelObjectSelection::containsFace(const Vector3UI32 &value) const NANOEM_DECL_NOEXCEPT
{
    return m_selectedFaceSet.find(value) != m_selectedFaceSet.end();
}

bool
ModelObjectSelection::containsVertexIndex(nanoem_u32_t value) const NANOEM_DECL_NOEXCEPT
{
    return m_vertexIndexSet.find(value) != m_vertexIndexSet.end();
}

bool
ModelObjectSelection::areAllBonesMovable() const NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_bone_t *activeBone = m_parent->activeBone();
    bool movable = activeBone ? true : false;
    if (m_selectedBoneSet.size() > 1) {
        for (model::Bone::Set::const_iterator it = m_selectedBoneSet.begin(), end = m_selectedBoneSet.end(); it != end;
             ++it) {
            const nanoem_model_bone_t *bone = *it;
            if (!nanoemModelBoneIsMovable(bone) || !nanoemModelBoneIsUserHandleable(bone)) {
                movable = false;
            }
        }
    }
    else {
        movable = nanoemModelBoneIsMovable(activeBone) != 0;
    }
    return movable && !m_parent->project()->isModelEditingEnabled();
}

bool
ModelObjectSelection::areAllBonesRotateable() const NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_bone_t *activeBone = m_parent->activeBone();
    bool rotateable = activeBone ? true : false;
    if (m_selectedBoneSet.size() > 1) {
        for (model::Bone::Set::const_iterator it = m_selectedBoneSet.begin(), end = m_selectedBoneSet.end(); it != end;
             ++it) {
            const nanoem_model_bone_t *bone = *it;
            if (!nanoemModelBoneIsMovable(bone) || !nanoemModelBoneIsRotateable(bone) ||
                !nanoemModelBoneIsUserHandleable(bone)) {
                rotateable = false;
            }
        }
    }
    else {
        rotateable = nanoemModelBoneIsRotateable(activeBone) != 0;
    }
    return rotateable && !m_parent->project()->isModelEditingEnabled();
}

bool
ModelObjectSelection::containsAnyBone() const NANOEM_DECL_NOEXCEPT
{
    return !m_selectedBoneSet.empty();
}

bool
ModelObjectSelection::isBoxSelectedBoneModeEnabled() const NANOEM_DECL_NOEXCEPT
{
    return m_boxSelectedBoneModeEnabled;
}

void
ModelObjectSelection::setBoxSelectedBoneModeEnabled(bool value)
{
    m_boxSelectedBoneModeEnabled = value;
}

ModelObjectSelection::EditingType
ModelObjectSelection::editingType() const NANOEM_DECL_NOEXCEPT
{
    return m_editingType;
}

void
ModelObjectSelection::setEditingType(EditingType value)
{
    m_editingType = value;
}

ModelObjectSelection::SelectTargetModeType
ModelObjectSelection::targetMode() const NANOEM_DECL_NOEXCEPT
{
    return m_targetMode;
}

void
ModelObjectSelection::setTargetMode(SelectTargetModeType value)
{
    m_targetMode = value;
}

} /* namespace internal */
} /* namespace nanoem */

namespace tinystl {

template <>
inline size_t
hash(const nanoem::Vector3UI32 &value)
{
    return std::hash<nanoem::Vector3UI32>()(value);
}

} /* namespace tinystl */
