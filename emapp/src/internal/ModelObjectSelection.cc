/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/ModelObjectSelection.h"

#include "emapp/ListUtils.h"
#include "emapp/Model.h"
#include "emapp/Project.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/hash.hpp"

namespace nanoem {
namespace internal {
namespace {

class SortUtils : private NonCopyable {
public:
    static int compareVertex(const void *a, const void *b) NANOEM_DECL_NOEXCEPT;
    static int compareMaterial(const void *a, const void *b) NANOEM_DECL_NOEXCEPT;
    static int compareBone(const void *a, const void *b) NANOEM_DECL_NOEXCEPT;
    static int compareMorph(const void *a, const void *b) NANOEM_DECL_NOEXCEPT;
    static int compareRigidBody(const void *a, const void *b) NANOEM_DECL_NOEXCEPT;
    static int compareJoint(const void *a, const void *b) NANOEM_DECL_NOEXCEPT;
    static int compareSoftBody(const void *a, const void *b) NANOEM_DECL_NOEXCEPT;
};

int
SortUtils::compareVertex(const void *a, const void *b) NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_vertex_t *left = *static_cast<const nanoem_model_vertex_t *const *>(a),
                                *right = *static_cast<const nanoem_model_vertex_t *const *>(b);
    return model::Vertex::index(left) - model::Vertex::index(right);
}

int
SortUtils::compareMaterial(const void *a, const void *b) NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_material_t *left = *static_cast<const nanoem_model_material_t *const *>(a),
                                  *right = *static_cast<const nanoem_model_material_t *const *>(b);
    return model::Material::index(left) - model::Material::index(right);
}

int
SortUtils::compareBone(const void *a, const void *b) NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_bone_t *left = *static_cast<const nanoem_model_bone_t *const *>(a),
                              *right = *static_cast<const nanoem_model_bone_t *const *>(b);
    return model::Bone::index(left) - model::Bone::index(right);
}

int
SortUtils::compareMorph(const void *a, const void *b) NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_morph_t *left = *static_cast<const nanoem_model_morph_t *const *>(a),
                               *right = *static_cast<const nanoem_model_morph_t *const *>(b);
    return model::Morph::index(left) - model::Morph::index(right);
}

int
SortUtils::compareRigidBody(const void *a, const void *b) NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_rigid_body_t *left = *static_cast<const nanoem_model_rigid_body_t *const *>(a),
                                    *right = *static_cast<const nanoem_model_rigid_body_t *const *>(b);
    return model::RigidBody::index(left) - model::RigidBody::index(right);
}

int
SortUtils::compareJoint(const void *a, const void *b) NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_joint_t *left = *static_cast<const nanoem_model_joint_t *const *>(a),
                               *right = *static_cast<const nanoem_model_joint_t *const *>(b);
    return model::Joint::index(left) - model::Joint::index(right);
}

int
SortUtils::compareSoftBody(const void *a, const void *b) NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_soft_body_t *left = *static_cast<const nanoem_model_soft_body_t *const *>(a),
                                   *right = *static_cast<const nanoem_model_soft_body_t *const *>(b);
    return model::SoftBody::index(left) - model::SoftBody::index(right);
}

} /* namespace anonymous */

void
ModelObjectSelection::sortedVertexList(const IModelObjectSelection *selection, model::Vertex::List &vertices)
{
    vertices = ListUtils::toListFromSet(*selection->allVertexSet());
    qsort(vertices.data(), vertices.size(), sizeof(vertices[0]), SortUtils::compareVertex);
}

void
ModelObjectSelection::sortedMaterialList(const IModelObjectSelection *selection, model::Material::List &materials)
{
    materials = ListUtils::toListFromSet(*selection->allMaterialSet());
    qsort(materials.data(), materials.size(), sizeof(materials[0]), SortUtils::compareMaterial);
}

void
ModelObjectSelection::sortedBoneList(const IModelObjectSelection *selection, model::Bone::List &bones)
{
    bones = ListUtils::toListFromSet(*selection->allBoneSet());
    qsort(bones.data(), bones.size(), sizeof(bones[0]), SortUtils::compareBone);
}

void
ModelObjectSelection::sortedMorphList(const IModelObjectSelection *selection, model::Morph::List &morphs)
{
    morphs = ListUtils::toListFromSet(*selection->allMorphSet());
    qsort(morphs.data(), morphs.size(), sizeof(morphs[0]), SortUtils::compareMorph);
}

void
ModelObjectSelection::sortedRigidBodyList(const IModelObjectSelection *selection, model::RigidBody::List &rigidBodies)
{
    rigidBodies = ListUtils::toListFromSet(*selection->allRigidBodySet());
    qsort(rigidBodies.data(), rigidBodies.size(), sizeof(rigidBodies[0]), SortUtils::compareRigidBody);
}

void
ModelObjectSelection::sortedJointList(const IModelObjectSelection *selection, model::Joint::List &joints)
{
    joints = ListUtils::toListFromSet(*selection->allJointSet());
    qsort(joints.data(), joints.size(), sizeof(joints[0]), SortUtils::compareJoint);
}

void
ModelObjectSelection::sortedSoftBodyList(const IModelObjectSelection *selection, model::SoftBody::List &softBodies)
{
    softBodies = ListUtils::toListFromSet(*selection->allSoftBodySet());
    qsort(softBodies.data(), softBodies.size(), sizeof(softBodies[0]), SortUtils::compareSoftBody);
}

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
    , m_editingType(kObjectTypeFirstEnum)
    , m_targetMode(kTargetModeTypePoint)
    , m_boxSelectedBoneModeEnabled(false)
{
}

ModelObjectSelection::~ModelObjectSelection() NANOEM_DECL_NOEXCEPT
{
}

void
ModelObjectSelection::addVertex(const nanoem_model_vertex_t *value)
{
    if (value) {
        m_selectedVertexSet.insert(value);
    }
}

void
ModelObjectSelection::addBone(const nanoem_model_bone_t *value)
{
    if (value) {
        m_selectedBoneSet.insert(value);
        m_parent->setActiveBone(value);
    }
}

void
ModelObjectSelection::addMaterial(const nanoem_model_material_t *value)
{
    if (value) {
        m_selectedMaterialSet.insert(value);
    }
}

void
ModelObjectSelection::addMorph(const nanoem_model_morph_t *value)
{
    if (value) {
        m_selectedMorphSet.insert(value);
    }
}

void
ModelObjectSelection::addLabel(const nanoem_model_label_t *value)
{
    if (value) {
        m_selectedLabelSet.insert(value);
    }
}

void
ModelObjectSelection::addRigidBody(const nanoem_model_rigid_body_t *value)
{
    if (value) {
        m_selectedRigidBodySet.insert(value);
    }
}

void
ModelObjectSelection::addJoint(const nanoem_model_joint_t *value)
{
    if (value) {
        m_selectedJointSet.insert(value);
    }
}

void
ModelObjectSelection::addSoftBody(const nanoem_model_soft_body_t *value)
{
    if (value) {
        m_selectedSoftBodySet.insert(value);
    }
}

void
ModelObjectSelection::addFace(const Vector4UI32 &value)
{
    m_selectedFaceSet.insert(value);
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
ModelObjectSelection::removeFace(const Vector4UI32 &value)
{
    m_selectedFaceSet.erase(value);
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

nanoem_rsize_t
ModelObjectSelection::countAllVertices() const NANOEM_DECL_NOEXCEPT
{
    return m_selectedVertexSet.size();
}

nanoem_rsize_t
ModelObjectSelection::countAllMaterials() const NANOEM_DECL_NOEXCEPT
{
    return m_selectedMaterialSet.size();
}

nanoem_rsize_t
ModelObjectSelection::countAllBones() const NANOEM_DECL_NOEXCEPT
{
    return m_selectedBoneSet.size();
}

nanoem_rsize_t
ModelObjectSelection::countAllMorphs() const NANOEM_DECL_NOEXCEPT
{
    return m_selectedMorphSet.size();
}

nanoem_rsize_t
ModelObjectSelection::countAllLabels() const NANOEM_DECL_NOEXCEPT
{
    return m_selectedLabelSet.size();
}

nanoem_rsize_t
ModelObjectSelection::countAllRigidBodies() const NANOEM_DECL_NOEXCEPT
{
    return m_selectedRigidBodySet.size();
}

nanoem_rsize_t
ModelObjectSelection::countAllJoints() const NANOEM_DECL_NOEXCEPT
{
    return m_selectedJointSet.size();
}

nanoem_rsize_t
ModelObjectSelection::countAllSoftBodies() const NANOEM_DECL_NOEXCEPT
{
    return m_selectedSoftBodySet.size();
}

nanoem_rsize_t
ModelObjectSelection::countAllFaces() const NANOEM_DECL_NOEXCEPT
{
    return m_selectedFaceSet.size();
}

const model::Vertex::Set *
ModelObjectSelection::allVertexSet() const NANOEM_DECL_NOEXCEPT
{
    return &m_selectedVertexSet;
}

const model::Bone::Set *
ModelObjectSelection::allBoneSet() const NANOEM_DECL_NOEXCEPT
{
    return &m_selectedBoneSet;
}

const model::Material::Set *
ModelObjectSelection::allMaterialSet() const NANOEM_DECL_NOEXCEPT
{
    return &m_selectedMaterialSet;
}

const model::Morph::Set *
ModelObjectSelection::allMorphSet() const NANOEM_DECL_NOEXCEPT
{
    return &m_selectedMorphSet;
}

const model::Label::Set *
ModelObjectSelection::allLabelSet() const NANOEM_DECL_NOEXCEPT
{
    return &m_selectedLabelSet;
}

const model::RigidBody::Set *
ModelObjectSelection::allRigidBodySet() const NANOEM_DECL_NOEXCEPT
{
    return &m_selectedRigidBodySet;
}

const model::Joint::Set *
ModelObjectSelection::allJointSet() const NANOEM_DECL_NOEXCEPT
{
    return &m_selectedJointSet;
}

const model::SoftBody::Set *
ModelObjectSelection::allSoftBodySet() const NANOEM_DECL_NOEXCEPT
{
    return &m_selectedSoftBodySet;
}

IModelObjectSelection::FaceList
ModelObjectSelection::allFaces() const
{
    return ListUtils::toListFromSet(m_selectedFaceSet);
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
ModelObjectSelection::containsFace(const Vector4UI32 &value) const NANOEM_DECL_NOEXCEPT
{
    return m_selectedFaceSet.find(value) != m_selectedFaceSet.end();
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

ModelObjectSelection::ObjectType
ModelObjectSelection::objectType() const NANOEM_DECL_NOEXCEPT
{
    return m_editingType;
}

void
ModelObjectSelection::setObjectType(ObjectType value)
{
    m_editingType = value;
}

ModelObjectSelection::TargetModeType
ModelObjectSelection::targetMode() const NANOEM_DECL_NOEXCEPT
{
    return m_targetMode;
}

void
ModelObjectSelection::setTargetMode(TargetModeType value)
{
    m_targetMode = value;
}

} /* namespace internal */
} /* namespace nanoem */

namespace tinystl {

template <>
inline size_t
hash(const nanoem::Vector4UI32 &value)
{
    return std::hash<nanoem::Vector4UI32>()(value);
}

} /* namespace tinystl */
