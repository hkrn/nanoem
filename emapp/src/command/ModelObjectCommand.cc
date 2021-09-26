/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/ModelObjectCommand.h"

#include "emapp/Error.h"
#include "emapp/IModelObjectSelection.h"
#include "emapp/ListUtils.h"
#include "emapp/Model.h"
#include "emapp/Progress.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/model/Bone.h"
#include "emapp/model/Constraint.h"
#include "emapp/model/Joint.h"
#include "emapp/model/Label.h"
#include "emapp/model/Material.h"
#include "emapp/model/Morph.h"
#include "emapp/model/RigidBody.h"
#include "emapp/model/SoftBody.h"
#include "emapp/model/Vertex.h"
#include "emapp/private/CommonInclude.h"

#include "nanoem/ext/mutable.h"
#include "tinyobjloader-c/tinyobj_loader_c.h"

namespace nanoem {
namespace command {
namespace {

static const nanoem_u8_t kNewObjectPrefixName[] = { 0xe6, 0x96, 0xb0, 0xe8, 0xa6, 0x8f, 0 };

} /* namespace anonymous */

ScopedMutableModel::ScopedMutableModel(Model *model, nanoem_status_t *status)
    : m_model(nullptr)
{
    m_model = nanoemMutableModelCreateAsReference(model->data(), status);
}

ScopedMutableModel::~ScopedMutableModel() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelDestroy(m_model);
    m_model = nullptr;
}

nanoem::command::ScopedMutableModel::operator nanoem_mutable_model_t *() NANOEM_DECL_NOEXCEPT
{
    return m_model;
}

ScopedMutableVertex::ScopedMutableVertex(Model *model)
    : m_vertex(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_vertex = nanoemMutableModelVertexCreate(model->data(), &status);
}

ScopedMutableVertex::ScopedMutableVertex(nanoem_model_vertex_t *vertexPtr)
    : m_vertex(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_vertex = nanoemMutableModelVertexCreateAsReference(vertexPtr, &status);
}

ScopedMutableVertex::~ScopedMutableVertex() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelVertexDestroy(m_vertex);
    m_vertex = nullptr;
}

nanoem::command::ScopedMutableVertex::operator nanoem_mutable_model_vertex_t *() NANOEM_DECL_NOEXCEPT
{
    return m_vertex;
}

ScopedMutableMaterial::ScopedMutableMaterial(Model *model)
    : m_material(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_material = nanoemMutableModelMaterialCreate(model->data(), &status);
}

ScopedMutableMaterial::ScopedMutableMaterial(nanoem_model_material_t *materialPtr)
    : m_material(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_material = nanoemMutableModelMaterialCreateAsReference(materialPtr, &status);
}

ScopedMutableMaterial::~ScopedMutableMaterial() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelMaterialDestroy(m_material);
    m_material = nullptr;
}

nanoem::command::ScopedMutableMaterial::operator nanoem_mutable_model_material_t *() NANOEM_DECL_NOEXCEPT
{
    return m_material;
}

ScopedMutableBone::ScopedMutableBone(Model *model)
    : m_bone(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_bone = nanoemMutableModelBoneCreate(model->data(), &status);
}

ScopedMutableBone::ScopedMutableBone(nanoem_model_bone_t *bonePtr)
    : m_bone(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_bone = nanoemMutableModelBoneCreateAsReference(bonePtr, &status);
}

ScopedMutableBone::~ScopedMutableBone() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelBoneDestroy(m_bone);
    m_bone = nullptr;
}

nanoem::command::ScopedMutableBone::operator nanoem_mutable_model_bone_t *() NANOEM_DECL_NOEXCEPT
{
    return m_bone;
}

ScopedMutableConstraint::ScopedMutableConstraint(Model *model)
    : m_constraint(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_constraint = nanoemMutableModelConstraintCreate(model->data(), &status);
}

ScopedMutableConstraint::ScopedMutableConstraint(nanoem_model_constraint_t *constraintPtr)
    : m_constraint(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_constraint = nanoemMutableModelConstraintCreateAsReference(constraintPtr, &status);
}

ScopedMutableConstraint::~ScopedMutableConstraint() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelConstraintDestroy(m_constraint);
    m_constraint = nullptr;
}

nanoem::command::ScopedMutableConstraint::operator nanoem_mutable_model_constraint_t *() NANOEM_DECL_NOEXCEPT
{
    return m_constraint;
}

ScopedMutableConstraintJoint::ScopedMutableConstraintJoint(nanoem_model_constraint_joint_t *jointPtr)
    : m_joint(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_joint = nanoemMutableModelConstraintJointCreateAsReference(jointPtr, &status);
}

ScopedMutableConstraintJoint::~ScopedMutableConstraintJoint() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelConstraintJointDestroy(m_joint);
    m_joint = nullptr;
}

nanoem::command::ScopedMutableConstraintJoint::operator nanoem_mutable_model_constraint_joint_t *() NANOEM_DECL_NOEXCEPT
{
    return m_joint;
}

ScopedMutableMorph::ScopedMutableMorph(Model *model)
    : m_morph(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_morph = nanoemMutableModelMorphCreate(model->data(), &status);
}

ScopedMutableMorph::ScopedMutableMorph(nanoem_model_morph_t *morphPtr)
    : m_morph(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_morph = nanoemMutableModelMorphCreateAsReference(morphPtr, &status);
}

ScopedMutableMorph::~ScopedMutableMorph() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelMorphDestroy(m_morph);
    m_morph = nullptr;
}

nanoem::command::ScopedMutableMorph::operator nanoem_mutable_model_morph_t *() NANOEM_DECL_NOEXCEPT
{
    return m_morph;
}

ScopedMutableMorphBone::ScopedMutableMorphBone(nanoem_mutable_model_morph_t *morphPtr)
    : m_morph(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_morph = nanoemMutableModelMorphBoneCreate(morphPtr, &status);
}

ScopedMutableMorphBone::ScopedMutableMorphBone(nanoem_model_morph_bone_t *morphPtr)
    : m_morph(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_morph = nanoemMutableModelMorphBoneCreateAsReference(morphPtr, &status);
}

ScopedMutableMorphBone::~ScopedMutableMorphBone() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelMorphBoneDestroy(m_morph);
    m_morph = nullptr;
}

nanoem::command::ScopedMutableMorphBone::operator nanoem_mutable_model_morph_bone_t *() NANOEM_DECL_NOEXCEPT
{
    return m_morph;
}

ScopedMutableMorphFlip::ScopedMutableMorphFlip(nanoem_mutable_model_morph_t *morphPtr)
    : m_morph(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_morph = nanoemMutableModelMorphFlipCreate(morphPtr, &status);
}

ScopedMutableMorphFlip::ScopedMutableMorphFlip(nanoem_model_morph_flip_t *morphPtr)
    : m_morph(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_morph = nanoemMutableModelMorphFlipCreateAsReference(morphPtr, &status);
}

ScopedMutableMorphFlip::~ScopedMutableMorphFlip() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelMorphFlipDestroy(m_morph);
    m_morph = nullptr;
}

nanoem::command::ScopedMutableMorphFlip::operator nanoem_mutable_model_morph_flip_t *() NANOEM_DECL_NOEXCEPT
{
    return m_morph;
}

ScopedMutableMorphGroup::ScopedMutableMorphGroup(nanoem_mutable_model_morph_t *morphPtr)
    : m_morph(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_morph = nanoemMutableModelMorphGroupCreate(morphPtr, &status);
}

ScopedMutableMorphGroup::ScopedMutableMorphGroup(nanoem_model_morph_group_t *morphPtr)
    : m_morph(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_morph = nanoemMutableModelMorphGroupCreateAsReference(morphPtr, &status);
}

ScopedMutableMorphGroup::~ScopedMutableMorphGroup() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelMorphGroupDestroy(m_morph);
    m_morph = nullptr;
}

nanoem::command::ScopedMutableMorphGroup::operator nanoem_mutable_model_morph_group_t *() NANOEM_DECL_NOEXCEPT
{
    return m_morph;
}

ScopedMutableMorphImpulse::ScopedMutableMorphImpulse(nanoem_mutable_model_morph_t *morphPtr)
    : m_morph(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_morph = nanoemMutableModelMorphImpulseCreate(morphPtr, &status);
}

ScopedMutableMorphImpulse::ScopedMutableMorphImpulse(nanoem_model_morph_impulse_t *morphPtr)
    : m_morph(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_morph = nanoemMutableModelMorphImpulseCreateAsReference(morphPtr, &status);
}

ScopedMutableMorphImpulse::~ScopedMutableMorphImpulse() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelMorphImpulseDestroy(m_morph);
    m_morph = nullptr;
}

nanoem::command::ScopedMutableMorphImpulse::operator nanoem_mutable_model_morph_impulse_t *() NANOEM_DECL_NOEXCEPT
{
    return m_morph;
}

ScopedMutableMorphMaterial::ScopedMutableMorphMaterial(nanoem_mutable_model_morph_t *morphPtr)
    : m_morph(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_morph = nanoemMutableModelMorphMaterialCreate(morphPtr, &status);
}

ScopedMutableMorphMaterial::ScopedMutableMorphMaterial(nanoem_model_morph_material_t *morphPtr)
    : m_morph(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_morph = nanoemMutableModelMorphMaterialCreateAsReference(morphPtr, &status);
}

ScopedMutableMorphMaterial::~ScopedMutableMorphMaterial() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelMorphMaterialDestroy(m_morph);
    m_morph = nullptr;
}

nanoem::command::ScopedMutableMorphMaterial::operator nanoem_mutable_model_morph_material_t *() NANOEM_DECL_NOEXCEPT
{
    return m_morph;
}

ScopedMutableMorphUV::ScopedMutableMorphUV(nanoem_mutable_model_morph_t *morphPtr)
    : m_morph(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_morph = nanoemMutableModelMorphUVCreate(morphPtr, &status);
}

ScopedMutableMorphUV::ScopedMutableMorphUV(nanoem_model_morph_uv_t *morphPtr)
    : m_morph(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_morph = nanoemMutableModelMorphUVCreateAsReference(morphPtr, &status);
}

ScopedMutableMorphUV::~ScopedMutableMorphUV() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelMorphUVDestroy(m_morph);
    m_morph = nullptr;
}

nanoem::command::ScopedMutableMorphUV::operator nanoem_mutable_model_morph_uv_t *() NANOEM_DECL_NOEXCEPT
{
    return m_morph;
}

ScopedMutableMorphVertex::ScopedMutableMorphVertex(nanoem_mutable_model_morph_t *morphPtr)
    : m_morph(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_morph = nanoemMutableModelMorphVertexCreate(morphPtr, &status);
}

ScopedMutableMorphVertex::ScopedMutableMorphVertex(nanoem_model_morph_vertex_t *morphPtr)
    : m_morph(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_morph = nanoemMutableModelMorphVertexCreateAsReference(morphPtr, &status);
}

ScopedMutableMorphVertex::~ScopedMutableMorphVertex() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelMorphVertexDestroy(m_morph);
    m_morph = nullptr;
}

nanoem::command::ScopedMutableMorphVertex::operator nanoem_mutable_model_morph_vertex_t *() NANOEM_DECL_NOEXCEPT
{
    return m_morph;
}

ScopedMutableLabel::ScopedMutableLabel(Model *model)
    : m_label(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_label = nanoemMutableModelLabelCreate(model->data(), &status);
}

ScopedMutableLabel::ScopedMutableLabel(nanoem_model_label_t *labelPtr)
    : m_label(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_label = nanoemMutableModelLabelCreateAsReference(labelPtr, &status);
}

ScopedMutableLabel::~ScopedMutableLabel() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelLabelDestroy(m_label);
    m_label = nullptr;
}

nanoem::command::ScopedMutableLabel::operator nanoem_mutable_model_label_t *() NANOEM_DECL_NOEXCEPT
{
    return m_label;
}

ScopedMutableRigidBody::ScopedMutableRigidBody(Model *model)
    : m_rigidBody(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_rigidBody = nanoemMutableModelRigidBodyCreate(model->data(), &status);
}

ScopedMutableRigidBody::ScopedMutableRigidBody(nanoem_model_rigid_body_t *rigidBodyPtr)
    : m_rigidBody(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_rigidBody = nanoemMutableModelRigidBodyCreateAsReference(rigidBodyPtr, &status);
}

ScopedMutableRigidBody::~ScopedMutableRigidBody() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelRigidBodyDestroy(m_rigidBody);
    m_rigidBody = nullptr;
}

nanoem::command::ScopedMutableRigidBody::operator nanoem_mutable_model_rigid_body_t *() NANOEM_DECL_NOEXCEPT
{
    return m_rigidBody;
}

ScopedMutableJoint::ScopedMutableJoint(Model *model)
    : m_joint(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_joint = nanoemMutableModelJointCreate(model->data(), &status);
}

ScopedMutableJoint::ScopedMutableJoint(nanoem_model_joint_t *jointPtr)
    : m_joint(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_joint = nanoemMutableModelJointCreateAsReference(jointPtr, &status);
}

ScopedMutableJoint::~ScopedMutableJoint() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelJointDestroy(m_joint);
    m_joint = nullptr;
}

nanoem::command::ScopedMutableJoint::operator nanoem_mutable_model_joint_t *() NANOEM_DECL_NOEXCEPT
{
    return m_joint;
}

ScopedMutableSoftBody::ScopedMutableSoftBody(Model *model)
    : m_softBody(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_softBody = nanoemMutableModelSoftBodyCreate(model->data(), &status);
}

ScopedMutableSoftBody::ScopedMutableSoftBody(nanoem_model_soft_body_t *softBodyPtr)
    : m_softBody(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_softBody = nanoemMutableModelSoftBodyCreateAsReference(softBodyPtr, &status);
}

ScopedMutableSoftBody::~ScopedMutableSoftBody() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelSoftBodyDestroy(m_softBody);
    m_softBody = nullptr;
}

nanoem::command::ScopedMutableSoftBody::operator nanoem_mutable_model_soft_body_t *() NANOEM_DECL_NOEXCEPT
{
    return m_softBody;
}

DeletingMaterialState::~DeletingMaterialState() NANOEM_DECL_NOEXCEPT
{
    clear();
}

void
DeletingMaterialState::clear() NANOEM_DECL_NOEXCEPT
{
    for (MaterialMorphList::const_iterator it = m_materialMorphs.begin(), end = m_materialMorphs.end(); it != end;
         ++it) {
        nanoemMutableModelMorphMaterialDestroy(*it);
    }
    m_materialMorphs.clear();
    for (SoftBodyList::const_iterator it = m_softBodies.begin(), end = m_softBodies.end(); it != end; ++it) {
        nanoemMutableModelSoftBodyDestroy(*it);
    }
    m_softBodies.clear();
}

void
DeletingMaterialState::save(const Model *model, const nanoem_model_material_t *materialPtr, nanoem_status_t *status)
{
    nanoem_rsize_t numMorphs;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numMorphs);
    for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
        nanoem_model_morph_t *morphPtr = morphs[i];
        if (nanoemModelMorphGetType(morphPtr) == NANOEM_MODEL_MORPH_TYPE_BONE) {
            nanoem_rsize_t numItems;
            nanoem_model_morph_material_t *const *items =
                nanoemModelMorphGetAllMaterialMorphObjects(morphPtr, &numItems);
            for (nanoem_rsize_t j = 0; j < numItems; j++) {
                nanoem_model_morph_material_t *item = items[i];
                if (nanoemModelMorphMaterialGetMaterialObject(item) == materialPtr) {
                    nanoem_mutable_model_morph_material_t *mutableMorph =
                        nanoemMutableModelMorphMaterialCreateAsReference(item, status);
                    m_materialMorphs.push_back(mutableMorph);
                }
            }
        }
    }
    nanoem_rsize_t numSoftBodies;
    nanoem_model_soft_body_t *const *softBodies = nanoemModelGetAllSoftBodyObjects(model->data(), &numSoftBodies);
    for (nanoem_rsize_t i = 0; i < numSoftBodies; i++) {
        nanoem_model_soft_body_t *softBodyPtr = softBodies[i];
        if (nanoemModelSoftBodyGetMaterialObject(softBodyPtr) == materialPtr) {
            nanoem_mutable_model_soft_body_t *rigidBody =
                nanoemMutableModelSoftBodyCreateAsReference(softBodyPtr, status);
            m_softBodies.push_back(rigidBody);
        }
    }
}

void
DeletingMaterialState::restore(const nanoem_model_material_t *materialPtr)
{
    for (MaterialMorphList::const_iterator it = m_materialMorphs.begin(), end = m_materialMorphs.end(); it != end;
         ++it) {
        nanoemMutableModelMorphMaterialSetMaterialObject(*it, materialPtr);
    }
    for (SoftBodyList::const_iterator it = m_softBodies.begin(), end = m_softBodies.end(); it != end; ++it) {
        nanoemMutableModelSoftBodySetMaterialObject(*it, materialPtr);
    }
}

DeletingBoneState::~DeletingBoneState() NANOEM_DECL_NOEXCEPT
{
    clear();
}

void
DeletingBoneState::clear() NANOEM_DECL_NOEXCEPT
{
    for (VertexList::const_iterator it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
        nanoemMutableModelVertexDestroy(it->first);
    }
    m_vertices.clear();
    for (BoneList::const_iterator it = m_parentBones.begin(), end = m_parentBones.end(); it != end; ++it) {
        nanoemMutableModelBoneDestroy(*it);
    }
    m_parentBones.clear();
    for (BoneList::const_iterator it = m_inherentParentBones.begin(), end = m_inherentParentBones.end(); it != end;
         ++it) {
        nanoemMutableModelBoneDestroy(*it);
    }
    m_inherentParentBones.clear();
    for (BoneList::const_iterator it = m_targetBones.begin(), end = m_targetBones.end(); it != end; ++it) {
        nanoemMutableModelBoneDestroy(*it);
    }
    m_targetBones.clear();
    for (BoneMorphList::const_iterator it = m_boneMorphs.begin(), end = m_boneMorphs.end(); it != end; ++it) {
        nanoemMutableModelMorphBoneDestroy(*it);
    }
    m_boneMorphs.clear();
    for (ConstraintList::const_iterator it = m_constraints.begin(), end = m_constraints.end(); it != end; ++it) {
        nanoemMutableModelConstraintDestroy(*it);
    }
    m_constraints.clear();
    for (ConstraintJointList::const_iterator it = m_constraintJoints.begin(), end = m_constraintJoints.end(); it != end;
         ++it) {
        nanoemMutableModelConstraintJointDestroy(*it);
    }
    m_constraintJoints.clear();
    for (RigidBodyList::const_iterator it = m_rigidBodies.begin(), end = m_rigidBodies.end(); it != end; ++it) {
        nanoemMutableModelRigidBodyDestroy(*it);
    }
    m_rigidBodies.clear();
}

void
DeletingBoneState::save(const Model *model, const nanoem_model_bone_t *bonePtr, nanoem_status_t *status)
{
    nanoem_rsize_t numVertices;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(model->data(), &numVertices);
    for (nanoem_rsize_t i = 0; i < numVertices; i++) {
        nanoem_model_vertex_t *vertexPtr = vertices[i];
        for (nanoem_rsize_t j = 0; j < 4; j++) {
            if (nanoemModelVertexGetBoneObject(vertexPtr, j) == bonePtr) {
                nanoem_mutable_model_vertex_t *mutableVertex =
                    nanoemMutableModelVertexCreateAsReference(vertexPtr, status);
                m_vertices.push_back(tinystl::make_pair(mutableVertex, j));
            }
        }
    }
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        nanoem_model_bone_t *bonePtr = bones[i];
        if (nanoemModelBoneGetParentBoneObject(bonePtr) == bonePtr) {
            nanoem_mutable_model_bone_t *mutableBone = nanoemMutableModelBoneCreateAsReference(bonePtr, status);
            m_parentBones.push_back(mutableBone);
            break;
        }
        if (nanoemModelBoneGetInherentParentBoneObject(bonePtr) == bonePtr) {
            nanoem_mutable_model_bone_t *mutableBone = nanoemMutableModelBoneCreateAsReference(bonePtr, status);
            m_inherentParentBones.push_back(mutableBone);
            break;
        }
        if (nanoemModelBoneGetTargetBoneObject(bonePtr) == bonePtr) {
            nanoem_mutable_model_bone_t *mutableBone = nanoemMutableModelBoneCreateAsReference(bonePtr, status);
            m_targetBones.push_back(mutableBone);
            break;
        }
        if (nanoem_model_constraint_t *constraintPtr = nanoemModelBoneGetConstraintObjectMutable(bonePtr)) {
            nanoem_rsize_t numJoints;
            nanoem_model_constraint_joint_t *const *joints =
                nanoemModelConstraintGetAllJointObjects(constraintPtr, &numJoints);
            for (nanoem_rsize_t j = 0; j < numJoints; j++) {
                nanoem_model_constraint_joint_t *jointPtr = joints[i];
                if (nanoemModelConstraintJointGetBoneObject(jointPtr) == bonePtr) {
                    nanoem_mutable_model_constraint_joint_t *mutableJoint =
                        nanoemMutableModelConstraintJointCreateAsReference(jointPtr, status);
                    m_constraintJoints.push_back(mutableJoint);
                }
            }
            if (nanoemModelConstraintGetEffectorBoneObject(constraintPtr) == bonePtr) {
                nanoem_mutable_model_constraint_t *mutableConstraint =
                    nanoemMutableModelConstraintCreateAsReference(constraintPtr, status);
                m_constraints.push_back(mutableConstraint);
            }
        }
    }
    nanoem_rsize_t numMorphs;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numMorphs);
    for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
        nanoem_model_morph_t *morphPtr = morphs[i];
        if (nanoemModelMorphGetType(morphPtr) == NANOEM_MODEL_MORPH_TYPE_BONE) {
            nanoem_rsize_t numItems;
            nanoem_model_morph_bone_t *const *items = nanoemModelMorphGetAllBoneMorphObjects(morphPtr, &numItems);
            for (nanoem_rsize_t j = 0; j < numItems; j++) {
                nanoem_model_morph_bone_t *item = items[i];
                if (nanoemModelMorphBoneGetBoneObject(item) == bonePtr) {
                    nanoem_mutable_model_morph_bone_t *mutableMorph =
                        nanoemMutableModelMorphBoneCreateAsReference(item, status);
                    m_boneMorphs.push_back(mutableMorph);
                }
            }
        }
    }
    nanoem_rsize_t numRigidBodies;
    nanoem_model_rigid_body_t *const *rigidBodies = nanoemModelGetAllRigidBodyObjects(model->data(), &numRigidBodies);
    for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
        nanoem_model_rigid_body_t *rigidBodyPtr = rigidBodies[i];
        if (nanoemModelRigidBodyGetBoneObject(rigidBodyPtr) == bonePtr) {
            nanoem_mutable_model_rigid_body_t *rigidBody =
                nanoemMutableModelRigidBodyCreateAsReference(rigidBodyPtr, status);
            m_rigidBodies.push_back(rigidBody);
        }
    }
}

void
DeletingBoneState::restore(const nanoem_model_bone_t *bonePtr)
{
    for (VertexList::const_iterator it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
        nanoemMutableModelVertexSetBoneObject(it->first, bonePtr, it->second);
    }
    setParentBone(bonePtr);
    for (BoneList::const_iterator it = m_inherentParentBones.begin(), end = m_inherentParentBones.end(); it != end;
         ++it) {
        nanoemMutableModelBoneSetInherentParentBoneObject(*it, bonePtr);
    }
    for (BoneList::const_iterator it = m_targetBones.begin(), end = m_targetBones.end(); it != end; ++it) {
        nanoemMutableModelBoneSetTargetBoneObject(*it, bonePtr);
    }
    for (BoneMorphList::const_iterator it = m_boneMorphs.begin(), end = m_boneMorphs.end(); it != end; ++it) {
        nanoemMutableModelMorphBoneSetBoneObject(*it, bonePtr);
    }
    for (ConstraintList::const_iterator it = m_constraints.begin(), end = m_constraints.end(); it != end; ++it) {
        nanoemMutableModelConstraintSetEffectorBoneObject(*it, bonePtr);
    }
    for (ConstraintJointList::const_iterator it = m_constraintJoints.begin(), end = m_constraintJoints.end(); it != end;
         ++it) {
        nanoemMutableModelConstraintJointSetBoneObject(*it, bonePtr);
    }
    for (RigidBodyList::const_iterator it = m_rigidBodies.begin(), end = m_rigidBodies.end(); it != end; ++it) {
        nanoemMutableModelRigidBodySetBoneObject(*it, bonePtr);
    }
}

void
DeletingBoneState::setParentBone(const nanoem_model_bone_t *bonePtr)
{
    for (BoneList::const_iterator it = m_parentBones.begin(), end = m_parentBones.end(); it != end; ++it) {
        nanoemMutableModelBoneSetParentBoneObject(*it, bonePtr);
    }
}

DeletingMorphState::~DeletingMorphState() NANOEM_DECL_NOEXCEPT
{
    clear();
}

void
DeletingMorphState::clear() NANOEM_DECL_NOEXCEPT
{
    for (FlipMorphList::const_iterator it = m_flipMorphs.begin(), end = m_flipMorphs.end(); it != end; ++it) {
        nanoemMutableModelMorphFlipDestroy(*it);
    }
    m_flipMorphs.clear();
    for (GroupMorphList::const_iterator it = m_groupMorphs.begin(), end = m_groupMorphs.end(); it != end; ++it) {
        nanoemMutableModelMorphGroupDestroy(*it);
    }
    m_groupMorphs.clear();
}

void
DeletingMorphState::save(const Model *model, const nanoem_model_morph_t *morphPtr, nanoem_status_t *status)
{
    BX_UNUSED_1(morphPtr);
    nanoem_rsize_t numMorphs;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numMorphs);
    for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
        nanoem_model_morph_t *morphPtr = morphs[i];
        if (nanoemModelMorphGetType(morphPtr) == NANOEM_MODEL_MORPH_TYPE_FLIP) {
            nanoem_rsize_t numItems;
            nanoem_model_morph_flip_t *const *items = nanoemModelMorphGetAllFlipMorphObjects(morphPtr, &numItems);
            for (nanoem_rsize_t j = 0; j < numItems; j++) {
                nanoem_model_morph_flip_t *item = items[i];
                if (nanoemModelMorphFlipGetMorphObject(item) == morphPtr) {
                    nanoem_mutable_model_morph_flip_t *mutableMorph =
                        nanoemMutableModelMorphFlipCreateAsReference(item, status);
                    m_flipMorphs.push_back(mutableMorph);
                }
            }
        }
        else if (nanoemModelMorphGetType(morphPtr) == NANOEM_MODEL_MORPH_TYPE_GROUP) {
            nanoem_rsize_t numItems;
            nanoem_model_morph_group_t *const *items = nanoemModelMorphGetAllGroupMorphObjects(morphPtr, &numItems);
            for (nanoem_rsize_t j = 0; j < numItems; j++) {
                nanoem_model_morph_group_t *item = items[i];
                if (nanoemModelMorphGroupGetMorphObject(item) == morphPtr) {
                    nanoem_mutable_model_morph_group_t *mutableMorph =
                        nanoemMutableModelMorphGroupCreateAsReference(item, status);
                    m_groupMorphs.push_back(mutableMorph);
                }
            }
        }
    }
}

void
DeletingMorphState::restore(const nanoem_model_morph_t *morphPtr)
{
    for (FlipMorphList::const_iterator it = m_flipMorphs.begin(), end = m_flipMorphs.end(); it != end; ++it) {
        nanoemMutableModelMorphFlipSetMorphObject(*it, morphPtr);
    }
    for (GroupMorphList::const_iterator it = m_groupMorphs.begin(), end = m_groupMorphs.end(); it != end; ++it) {
        nanoemMutableModelMorphGroupSetMorphObject(*it, morphPtr);
    }
}

DeletingRigidBodyState::~DeletingRigidBodyState() NANOEM_DECL_NOEXCEPT
{
    clear();
}

void
DeletingRigidBodyState::clear() NANOEM_DECL_NOEXCEPT
{
    for (ImpulseMorphList::const_iterator it = m_impulseMorphs.begin(), end = m_impulseMorphs.end(); it != end; ++it) {
        nanoemMutableModelMorphImpulseDestroy(*it);
    }
    m_impulseMorphs.clear();
    for (JointList::const_iterator it = m_jointsA.begin(), end = m_jointsA.end(); it != end; ++it) {
        nanoemMutableModelJointDestroy(*it);
    }
    m_jointsA.clear();
    for (JointList::const_iterator it = m_jointsB.begin(), end = m_jointsB.end(); it != end; ++it) {
        nanoemMutableModelJointDestroy(*it);
    }
    m_jointsB.clear();
}

void
DeletingRigidBodyState::save(const Model *model, const nanoem_model_rigid_body_t *rigidBodyPtr, nanoem_status_t *status)
{
    nanoem_rsize_t numMorphs;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numMorphs);
    for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
        nanoem_model_morph_t *morphPtr = morphs[i];
        if (nanoemModelMorphGetType(morphPtr) == NANOEM_MODEL_MORPH_TYPE_BONE) {
            nanoem_rsize_t numItems;
            nanoem_model_morph_impulse_t *const *items = nanoemModelMorphGetAllImpulseMorphObjects(morphPtr, &numItems);
            for (nanoem_rsize_t j = 0; j < numItems; j++) {
                nanoem_model_morph_impulse_t *item = items[i];
                if (nanoemModelMorphImpulseGetRigidBodyObject(item) == rigidBodyPtr) {
                    nanoem_mutable_model_morph_impulse_t *mutableMorph =
                        nanoemMutableModelMorphImpulseCreateAsReference(item, status);
                    m_impulseMorphs.push_back(mutableMorph);
                }
            }
        }
    }
    nanoem_rsize_t numJoints;
    nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(model->data(), &numJoints);
    for (nanoem_rsize_t i = 0; i < numJoints; i++) {
        nanoem_model_joint_t *jointPtr = joints[i];
        if (nanoemModelJointGetRigidBodyAObject(jointPtr) == rigidBodyPtr) {
            nanoem_mutable_model_joint_t *joint = nanoemMutableModelJointCreateAsReference(jointPtr, status);
            m_jointsA.push_back(joint);
        }
        if (nanoemModelJointGetRigidBodyBObject(jointPtr) == rigidBodyPtr) {
            nanoem_mutable_model_joint_t *joint = nanoemMutableModelJointCreateAsReference(jointPtr, status);
            m_jointsB.push_back(joint);
        }
    }
}

void
DeletingRigidBodyState::restore(const nanoem_model_rigid_body_t *rigidBodyPtr)
{
    for (ImpulseMorphList::const_iterator it = m_impulseMorphs.begin(), end = m_impulseMorphs.end(); it != end; ++it) {
        nanoemMutableModelMorphImpulseSetRigidBodyObject(*it, rigidBodyPtr);
    }
    for (JointList::const_iterator it = m_jointsA.begin(), end = m_jointsA.end(); it != end; ++it) {
        nanoemMutableModelJointSetRigidBodyAObject(*it, rigidBodyPtr);
    }
    for (JointList::const_iterator it = m_jointsB.begin(), end = m_jointsB.end(); it != end; ++it) {
        nanoemMutableModelJointSetRigidBodyBObject(*it, rigidBodyPtr);
    }
}

undo_command_t *
PaintVertexWeightCommand::create(Model *activeModel, const BoneMappingStateMap &mappings)
{
    PaintVertexWeightCommand *command = nanoem_new(PaintVertexWeightCommand(activeModel, mappings));
    return command->createCommand();
}

PaintVertexWeightCommand::PaintVertexWeightCommand(Model *activeModel, const BoneMappingStateMap &mappings)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_mappings(mappings)
{
}

PaintVertexWeightCommand::~PaintVertexWeightCommand() NANOEM_DECL_NOEXCEPT
{
}

void
PaintVertexWeightCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (BoneMappingStateMap::const_iterator it = m_mappings.begin(), end = m_mappings.end(); it != end; ++it) {
        nanoem_model_vertex_t *vertexPtr = it->first;
        const BoneMappingState &state = it->second.second;
        setBoneWeight(vertexPtr, state, &status);
    }
    assignError(status, error);
}

void
PaintVertexWeightCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (BoneMappingStateMap::const_iterator it = m_mappings.begin(), end = m_mappings.end(); it != end; ++it) {
        nanoem_model_vertex_t *vertexPtr = it->first;
        const BoneMappingState &state = it->second.first;
        setBoneWeight(vertexPtr, state, &status);
    }
    assignError(status, error);
}

void
PaintVertexWeightCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
PaintVertexWeightCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
PaintVertexWeightCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
PaintVertexWeightCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "PaintVertexWeightCommand";
}

void
PaintVertexWeightCommand::setBoneWeight(
    nanoem_model_vertex_t *vertexPtr, const BoneMappingState &state, nanoem_status_t *status)
{
    nanoem_mutable_model_vertex_t *mutableVertexPtr = nanoemMutableModelVertexCreateAsReference(vertexPtr, status);
    for (nanoem_rsize_t i = 0; i < 4; i++) {
        nanoemMutableModelVertexSetBoneObject(mutableVertexPtr, state.m_bones[i], i);
        nanoemMutableModelVertexSetBoneWeight(mutableVertexPtr, state.m_weights[i], i);
    }
    if (model::Vertex *vertex = model::Vertex::cast(vertexPtr)) {
        vertex->setupBoneBinding(vertexPtr, m_activeModel);
        vertex->m_simd.m_weights =
            bx::simd_ld(nanoemModelVertexGetBoneWeight(vertexPtr, 0), nanoemModelVertexGetBoneWeight(vertexPtr, 1),
                nanoemModelVertexGetBoneWeight(vertexPtr, 2), nanoemModelVertexGetBoneWeight(vertexPtr, 3));
    }
    nanoemMutableModelVertexDestroy(mutableVertexPtr);
}

undo_command_t *
CreateMaterialCommand::create(
    Model *activeModel, const String &name, const MutableVertexList &vertices, const VertexIndexList &vertexIndices)
{
    CreateMaterialCommand *command = nanoem_new(CreateMaterialCommand(activeModel, name, vertices, vertexIndices));
    return command->createCommand();
}

void
CreateMaterialCommand::setup(nanoem_model_material_t *materialPtr, Project *project)
{
    model::Material *material = model::Material::create(project->sharedFallbackImage());
    material->bind(materialPtr);
    material->reset(materialPtr);
    material->resetDeform();
    material->resetLanguage(materialPtr, project->unicodeStringFactory(), project->castLanguage());
}

CreateMaterialCommand::CreateMaterialCommand(
    Model *activeModel, const String &name, const MutableVertexList &vertices, const VertexIndexList &vertexIndices)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_creatingMaterial(m_activeModel)
    , m_creatingVertices(vertices)
{
    nanoem_rsize_t numVertices, numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones),
                               *firstBone = nullptr;
    if (numBones > 0) {
        firstBone = bones[0];
    }
    nanoemModelGetAllVertexObjects(m_activeModel->data(), &numVertices);
    nanoem_model_material_t *origin = nanoemMutableModelMaterialGetOriginObject(m_creatingMaterial);
    for (MutableVertexList::const_iterator it = vertices.begin(), end = vertices.end(); it != end; ++it) {
        nanoemMutableModelVertexSetType(*it, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
        nanoemMutableModelVertexSetBoneObject(*it, firstBone, 0);
        nanoem_model_vertex_t *vertexPtr = nanoemMutableModelVertexGetOriginObject(*it);
        model::Vertex *vertex = model::Vertex::create();
        vertex->bind(vertexPtr);
        vertex->setupBoneBinding(vertexPtr, m_activeModel);
        vertex->setMaterial(origin);
    }
    for (VertexIndexList::const_iterator it = vertexIndices.begin(), end = vertexIndices.end(); it != end; ++it) {
        m_vertexIndices.push_back(*it + Inline::saturateInt32U(numVertices));
    }
    Project *project = currentProject();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope us(factory);
    if (!name.empty() && StringUtils::tryGetString(factory, name, us)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableModelMaterialSetName(m_creatingMaterial, us.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
        nanoemMutableModelMaterialSetName(m_creatingMaterial, us.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    else {
        static const nanoem_u8_t kNameMaterialInJapanese[] = { 0xe6, 0x9d, 0x90, 0xe8, 0xb3, 0xaa, 0 };
        String japaneseName, englishName;
        nanoem_rsize_t numMaterials;
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemModelGetAllMaterialObjects(m_activeModel->data(), &numMaterials);
        StringUtils::format(japaneseName, "%s%jd", kNameMaterialInJapanese, numMaterials + 1);
        if (StringUtils::tryGetString(factory, japaneseName, us)) {
            nanoemMutableModelMaterialSetName(m_creatingMaterial, us.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
        }
        StringUtils::format(englishName, "Material%jd", numMaterials + 1);
        if (StringUtils::tryGetString(factory, englishName, us)) {
            nanoemMutableModelMaterialSetName(m_creatingMaterial, us.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
        }
    }
    nanoemMutableModelMaterialSetAmbientColor(m_creatingMaterial, glm::value_ptr(Vector4(1)));
    nanoemMutableModelMaterialSetDiffuseColor(m_creatingMaterial, glm::value_ptr(Vector4(1)));
    nanoemMutableModelMaterialSetSpecularColor(m_creatingMaterial, glm::value_ptr(Vector4(1)));
    nanoemMutableModelMaterialSetDiffuseOpacity(m_creatingMaterial, 1.0f);
    nanoemMutableModelMaterialSetSpecularPower(m_creatingMaterial, 1.0f);
    nanoemMutableModelMaterialSetNumVertexIndices(m_creatingMaterial, m_vertexIndices.size());
    nanoemMutableModelMaterialSetCastingShadowEnabled(m_creatingMaterial, true);
    nanoemMutableModelMaterialSetCastingShadowMapEnabled(m_creatingMaterial, true);
    nanoemMutableModelMaterialSetShadowMapEnabled(m_creatingMaterial, true);
    nanoemMutableModelMaterialSetEdgeEnabled(m_creatingMaterial, true);
    setup(nanoemMutableModelMaterialGetOriginObject(m_creatingMaterial), project);
}

CreateMaterialCommand::~CreateMaterialCommand() NANOEM_DECL_NOEXCEPT
{
    for (MutableVertexList::const_iterator it = m_creatingVertices.begin(), end = m_creatingVertices.end(); it != end;
         ++it) {
        nanoemMutableModelVertexDestroy(*it);
    }
    m_creatingVertices.clear();
    m_vertexIndices.clear();
}

void
CreateMaterialCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelRemoveMaterialObject(model, m_creatingMaterial, &status);
    VertexIndexList workingBuffer;
    nanoem_rsize_t numVertexIndices;
    const nanoem_u32_t *vertexIndices = nanoemModelGetAllVertexIndices(m_activeModel->data(), &numVertexIndices);
    workingBuffer.assign(vertexIndices, vertexIndices + numVertexIndices - m_vertexIndices.size());
    nanoemMutableModelSetVertexIndices(model, workingBuffer.data(), workingBuffer.size(), &status);
    for (MutableVertexList::const_iterator it = m_creatingVertices.begin(), end = m_creatingVertices.end(); it != end;
         ++it) {
        nanoemMutableModelRemoveVertexObject(model, *it, &status);
    }
    m_activeModel->rebuildAllVertexBuffers(false);
    m_activeModel->rebuildIndexBuffer();
    assignError(status, error);
}

void
CreateMaterialCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoem_rsize_t numVertexIndices;
    VertexIndexList workingBuffer;
    const nanoem_u32_t *vertexIndices = nanoemModelGetAllVertexIndices(m_activeModel->data(), &numVertexIndices);
    workingBuffer.assign(vertexIndices, vertexIndices + numVertexIndices);
    workingBuffer.insert(workingBuffer.end(), m_vertexIndices.begin(), m_vertexIndices.end());
    nanoemMutableModelSetVertexIndices(model, workingBuffer.data(), workingBuffer.size(), &status);
    nanoemMutableModelInsertMaterialObject(model, m_creatingMaterial, -1, &status);
    for (MutableVertexList::const_iterator it = m_creatingVertices.begin(), end = m_creatingVertices.end(); it != end;
         ++it) {
        nanoemMutableModelInsertVertexObject(model, *it, -1, &status);
    }
    m_activeModel->rebuildAllVertexBuffers(false);
    m_activeModel->rebuildIndexBuffer();
    assignError(status, error);
}

void
CreateMaterialCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateMaterialCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateMaterialCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
CreateMaterialCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "CreateMaterialFromFileCommand";
}

undo_command_t *
CopyMaterialFromModelCommand::create(
    Model *activeModel, const Model *baseModel, const nanoem_model_material_t *baseMaterialPtr)
{
    CopyMaterialFromModelCommand *command =
        nanoem_new(CopyMaterialFromModelCommand(activeModel, baseModel, baseMaterialPtr));
    return command->createCommand();
}

CopyMaterialFromModelCommand::CopyMaterialFromModelCommand(
    Model *activeModel, const Model *baseModel, const nanoem_model_material_t *baseMaterialPtr)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_creatingMaterial(m_activeModel)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelMaterialCopy(m_creatingMaterial, baseMaterialPtr, &status);
    nanoem_rsize_t numMaterials, numVertexIndices;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(baseModel->data(), &numMaterials);
    const nanoem_u32_t *vertexIndices = nanoemModelGetAllVertexIndices(baseModel->data(), &numVertexIndices);
    for (nanoem_rsize_t i = 0, offset = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *materialPtr = materials[i];
        const nanoem_rsize_t numMaterialVertexIndices = nanoemModelMaterialGetNumVertexIndices(materialPtr);
        if (materialPtr == baseMaterialPtr) {
            const nanoem_rsize_t to = offset + numMaterialVertexIndices;
            m_vertexIndices.reserve(numMaterialVertexIndices);
            for (nanoem_rsize_t j = offset; j < to; j++) {
                m_vertexIndices.push_back(vertexIndices[j]);
            }
            break;
        }
        offset += numMaterialVertexIndices;
    }
    CreateMaterialCommand::setup(nanoemMutableModelMaterialGetOriginObject(m_creatingMaterial), currentProject());
}

CopyMaterialFromModelCommand::~CopyMaterialFromModelCommand() NANOEM_DECL_NOEXCEPT
{
}

void
CopyMaterialFromModelCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelRemoveMaterialObject(model, m_creatingMaterial, &status);
    VertexIndexList workingBuffer;
    nanoem_rsize_t numVertexIndices;
    const nanoem_u32_t *vertexIndices = nanoemModelGetAllVertexIndices(m_activeModel->data(), &numVertexIndices);
    workingBuffer.assign(vertexIndices, vertexIndices + numVertexIndices - m_vertexIndices.size());
    nanoemMutableModelSetVertexIndices(model, workingBuffer.data(), workingBuffer.size(), &status);
    m_activeModel->clearAllDrawVertexBuffers();
    m_activeModel->rebuildIndexBuffer();
    assignError(status, error);
}

void
CopyMaterialFromModelCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoem_rsize_t numVertexIndices;
    VertexIndexList workingBuffer;
    const nanoem_u32_t *vertexIndices = nanoemModelGetAllVertexIndices(m_activeModel->data(), &numVertexIndices);
    workingBuffer.assign(vertexIndices, vertexIndices + numVertexIndices);
    workingBuffer.insert(workingBuffer.end(), m_vertexIndices.begin(), m_vertexIndices.end());
    nanoemMutableModelSetVertexIndices(model, workingBuffer.data(), workingBuffer.size(), &status);
    nanoemMutableModelInsertMaterialObject(model, m_creatingMaterial, -1, &status);
    m_activeModel->clearAllDrawVertexBuffers();
    m_activeModel->rebuildIndexBuffer();
    assignError(status, error);
}

void
CopyMaterialFromModelCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CopyMaterialFromModelCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CopyMaterialFromModelCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
CopyMaterialFromModelCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "CopyMaterialFromModelCommand";
}

undo_command_t *
MergeMaterialCommand::create(
    Model *activeModel, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex)
{
    MergeMaterialCommand *command = nanoem_new(MergeMaterialCommand(activeModel, materials, materialIndex));
    return command->createCommand();
}

MergeMaterialCommand::MergeMaterialCommand(
    Model *activeModel, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_baseMaterial(materials[materialIndex - 1])
    , m_deletingMaterial(materials[materialIndex])
    , m_materialIndex(materialIndex)
{
}

MergeMaterialCommand::~MergeMaterialCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MergeMaterialCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    const nanoem_rsize_t numVertexIndices =
        nanoemModelMaterialGetNumVertexIndices(nanoemMutableModelMaterialGetOriginObject(m_baseMaterial)) -
        nanoemModelMaterialGetNumVertexIndices(nanoemMutableModelMaterialGetOriginObject(m_deletingMaterial));
    nanoemMutableModelMaterialSetNumVertexIndices(m_baseMaterial, numVertexIndices);
    nanoemMutableModelInsertMaterialObject(model, m_deletingMaterial, Inline::saturateInt32(m_materialIndex), &status);
    m_activeModel->clearAllDrawVertexBuffers();
    m_activeModel->rebuildIndexBuffer();
    assignError(status, error);
}

void
MergeMaterialCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    const nanoem_rsize_t numVertexIndices =
        nanoemModelMaterialGetNumVertexIndices(nanoemMutableModelMaterialGetOriginObject(m_baseMaterial)) +
        nanoemModelMaterialGetNumVertexIndices(nanoemMutableModelMaterialGetOriginObject(m_deletingMaterial));
    nanoemMutableModelMaterialSetNumVertexIndices(m_baseMaterial, numVertexIndices);
    nanoemMutableModelRemoveMaterialObject(model, m_deletingMaterial, &status);
    m_activeModel->clearAllDrawVertexBuffers();
    m_activeModel->rebuildIndexBuffer();
    assignError(status, error);
}

void
MergeMaterialCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MergeMaterialCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MergeMaterialCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
MergeMaterialCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "MergeMaterialCommand";
}

undo_command_t *
DeleteMaterialCommand::create(
    Model *activeModel, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex)
{
    DeleteMaterialCommand *command = nanoem_new(DeleteMaterialCommand(activeModel, materials, materialIndex));
    return command->createCommand();
}

DeleteMaterialCommand::DeleteMaterialCommand(
    Model *activeModel, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_deletingMaterial(materials[materialIndex])
    , m_deletingVertexIndexOffset(0)
    , m_materialIndex(materialIndex)
{
    nanoem_rsize_t numMaterials, numVertexIndices;
    const nanoem_model_material_t *activeMaterial = nanoemMutableModelMaterialGetOriginObject(m_deletingMaterial);
    const nanoem_u32_t *vertexIndices = nanoemModelGetAllVertexIndices(m_activeModel->data(), &numVertexIndices);
    nanoemModelGetAllMaterialObjects(m_activeModel->data(), &numMaterials);
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *currentMaterialPtr = materials[i];
        const size_t innerSize = nanoemModelMaterialGetNumVertexIndices(currentMaterialPtr);
        if (currentMaterialPtr == activeMaterial) {
            m_deletingVertexIndices.assign(vertexIndices, vertexIndices + innerSize);
            break;
        }
        m_deletingVertexIndexOffset += innerSize;
    }
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_deletingMaterialState.save(m_activeModel, activeMaterial, &status);
}

DeleteMaterialCommand::~DeleteMaterialCommand() NANOEM_DECL_NOEXCEPT
{
    m_deletingVertexIndices.clear();
}

void
DeleteMaterialCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoem_rsize_t numIndices;
    const nanoem_u32_t *indices = nanoemModelGetAllVertexIndices(nanoemMutableModelGetOriginObject(model), &numIndices);
    VertexIndexList workingBuffer(numIndices + m_deletingVertexIndices.size());
    nanoem_u32_t *workingBufferPtr = workingBuffer.data();
    if (m_deletingVertexIndexOffset > 0) {
        memcpy(workingBufferPtr, indices, m_deletingVertexIndexOffset * sizeof(workingBuffer[0]));
        workingBufferPtr += m_deletingVertexIndexOffset;
    }
    memcpy(workingBufferPtr, m_deletingVertexIndices.data(), m_deletingVertexIndices.size() * sizeof(workingBuffer[0]));
    workingBufferPtr += m_deletingVertexIndices.size();
    const nanoem_rsize_t restSize = (numIndices - m_deletingVertexIndexOffset) * sizeof(workingBuffer[0]);
    if (restSize > 0) {
        memcpy(workingBufferPtr, indices + m_deletingVertexIndexOffset, restSize);
    }
    nanoemMutableModelSetVertexIndices(model, workingBuffer.data(), workingBuffer.size(), &status);
    nanoemMutableModelInsertMaterialObject(model, m_deletingMaterial, Inline::saturateInt32(m_materialIndex), &status);
    m_deletingMaterialState.restore(nanoemMutableModelMaterialGetOriginObject(m_deletingMaterial));
    m_activeModel->clearAllDrawVertexBuffers();
    m_activeModel->rebuildIndexBuffer();
    assignError(status, error);
}

void
DeleteMaterialCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoem_rsize_t numIndices, rest, size(m_deletingVertexIndices.size());
    const nanoem_u32_t *indices = nanoemModelGetAllVertexIndices(nanoemMutableModelGetOriginObject(model), &numIndices);
    VertexIndexList workingBuffer(numIndices);
    rest = numIndices - m_deletingVertexIndexOffset - size;
    memcpy(workingBuffer.data(), indices, numIndices * sizeof(workingBuffer[0]));
    memmove(workingBuffer.data() + m_deletingVertexIndexOffset,
        workingBuffer.data() + m_deletingVertexIndexOffset + size, rest * sizeof(workingBuffer[0]));
    nanoemMutableModelSetVertexIndices(model, workingBuffer.data(), numIndices - size, &status);
    nanoemMutableModelRemoveMaterialObject(model, m_deletingMaterial, &status);
    m_activeModel->clearAllDrawVertexBuffers();
    m_activeModel->rebuildIndexBuffer();
    assignError(status, error);
}

void
DeleteMaterialCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
DeleteMaterialCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
DeleteMaterialCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
DeleteMaterialCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "DeleteMaterialCommand";
}

BaseMoveMaterialCommand::BaseMoveMaterialCommand(
    Model *activeModel, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_movingMaterial(materials[materialIndex])
{
}

BaseMoveMaterialCommand::~BaseMoveMaterialCommand() NANOEM_DECL_NOEXCEPT
{
}

void
BaseMoveMaterialCommand::move(ScopedMutableMaterial &material, int destination, const LayoutPosition &from,
    const LayoutPosition &to, Model *activeModel, nanoem_status_t *status)
{
    ScopedMutableModel model(activeModel, status);
    nanoem_rsize_t numIndices;
    const nanoem_u32_t *indices = nanoemModelGetAllVertexIndices(nanoemMutableModelGetOriginObject(model), &numIndices);
    VertexIndexList tempFromBuffer(from.m_size), tempToBuffer(to.m_size);
    memcpy(tempFromBuffer.data(), indices + from.m_offset, from.m_size * sizeof(tempFromBuffer[0]));
    memcpy(tempToBuffer.data(), indices + to.m_offset, to.m_size * sizeof(tempToBuffer[0]));
    VertexIndexList workingBuffer(numIndices);
    memcpy(workingBuffer.data(), indices, numIndices * sizeof(workingBuffer[0]));
    if (from.m_offset < to.m_offset) {
        nanoem_u32_t *bufferPtr = workingBuffer.data() + from.m_offset;
        memcpy(bufferPtr, tempToBuffer.data(), to.m_size * sizeof(tempToBuffer[0]));
        memcpy(bufferPtr + to.m_size, tempFromBuffer.data(), from.m_size * sizeof(tempFromBuffer[0]));
    }
    else {
        nanoem_u32_t *bufferPtr = workingBuffer.data() + to.m_offset;
        memcpy(bufferPtr, tempFromBuffer.data(), from.m_size * sizeof(tempFromBuffer[0]));
        memcpy(bufferPtr + from.m_size, tempToBuffer.data(), to.m_size * sizeof(tempToBuffer[0]));
    }
    const nanoem_model_material_t *materialPtr = nanoemMutableModelMaterialGetOriginObject(material);
    DeletingMaterialState state;
    state.save(activeModel, materialPtr, status);
    nanoemMutableModelSetVertexIndices(model, workingBuffer.data(), numIndices, status);
    nanoemMutableModelRemoveMaterialObject(model, material, status);
    nanoemMutableModelInsertMaterialObject(model, material, destination, status);
    state.restore(materialPtr);
    activeModel->clearAllDrawVertexBuffers();
    activeModel->rebuildIndexBuffer();
}

void
BaseMoveMaterialCommand::moveUp(nanoem_status_t *status)
{
    const nanoem_model_material_t *activeMaterial = nanoemMutableModelMaterialGetOriginObject(m_movingMaterial);
    nanoem_rsize_t numMaterials, offset = 0;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_activeModel->data(), &numMaterials);
    LayoutPosition from = { 0, 0 }, to = { 0, 0 };
    int destination = 0;
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *currentMaterialPtr = materials[i];
        size_t size = nanoemModelMaterialGetNumVertexIndices(currentMaterialPtr);
        if (currentMaterialPtr == activeMaterial) {
            const nanoem_model_material_t *previousMaterialPtr = materials[i - 1];
            destination = Inline::saturateInt32(i - 1);
            to.m_size = nanoemModelMaterialGetNumVertexIndices(previousMaterialPtr);
            to.m_offset = offset - to.m_size;
            from.m_offset = offset;
            from.m_size = size;
            break;
        }
        offset += size;
    }
    move(m_movingMaterial, destination, from, to, m_activeModel, status);
}

void
BaseMoveMaterialCommand::moveDown(nanoem_status_t *status)
{
    const nanoem_model_material_t *activeMaterial = nanoemMutableModelMaterialGetOriginObject(m_movingMaterial);
    nanoem_rsize_t numMaterials, offset = 0;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_activeModel->data(), &numMaterials);
    LayoutPosition from = { 0, 0 }, to = { 0, 0 };
    int destination = 0;
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *currentMaterialPtr = materials[i];
        size_t size = nanoemModelMaterialGetNumVertexIndices(currentMaterialPtr);
        if (currentMaterialPtr == activeMaterial) {
            const nanoem_model_material_t *nextMaterialPtr = materials[i + 1];
            destination = Inline::saturateInt32(i + 1);
            to.m_size = nanoemModelMaterialGetNumVertexIndices(nextMaterialPtr);
            to.m_offset = offset + size;
            from.m_offset = offset;
            from.m_size = size;
            break;
        }
        offset += size;
    }
    move(m_movingMaterial, destination, from, to, m_activeModel, status);
}

undo_command_t *
MoveMaterialUpCommand::create(
    Model *activeModel, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex)
{
    MoveMaterialUpCommand *command = nanoem_new(MoveMaterialUpCommand(activeModel, materials, materialIndex));
    return command->createCommand();
}

MoveMaterialUpCommand::MoveMaterialUpCommand(
    Model *activeModel, nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex)
    : BaseMoveMaterialCommand(activeModel, materials, materialIndex)
{
}

MoveMaterialUpCommand::~MoveMaterialUpCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveMaterialUpCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    moveDown(&status);
    assignError(status, error);
}

void
MoveMaterialUpCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    moveUp(&status);
    assignError(status, error);
}

void
MoveMaterialUpCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveMaterialUpCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveMaterialUpCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
MoveMaterialUpCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "MoveMaterialUpCommand";
}

undo_command_t *
MoveMaterialDownCommand::create(
    Model *activeModel, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex)
{
    MoveMaterialDownCommand *command = nanoem_new(MoveMaterialDownCommand(activeModel, materials, materialIndex));
    return command->createCommand();
}

MoveMaterialDownCommand::MoveMaterialDownCommand(
    Model *activeModel, nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex)
    : BaseMoveMaterialCommand(activeModel, materials, materialIndex)
{
}

MoveMaterialDownCommand::~MoveMaterialDownCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveMaterialDownCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    moveUp(&status);
    assignError(status, error);
}

void
MoveMaterialDownCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    moveDown(&status);
    assignError(status, error);
}

void
MoveMaterialDownCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveMaterialDownCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveMaterialDownCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
MoveMaterialDownCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "MoveMaterialDownCommand";
}

undo_command_t *
CreateBoneCommand::create(Model *activeModel, const Vector3 &origin)
{
    CreateBoneCommand *command = nanoem_new(CreateBoneCommand(activeModel, -1, nullptr, &origin));
    return command->createCommand();
}

undo_command_t *
CreateBoneCommand::create(Model *activeModel, int offset, const nanoem_model_bone_t *base)
{
    CreateBoneCommand *command = nanoem_new(CreateBoneCommand(activeModel, offset, base, nullptr));
    return command->createCommand();
}

void
CreateBoneCommand::setup(nanoem_model_bone_t *bonePtr, Project *project)
{
    model::Bone *newBone = model::Bone::create();
    newBone->bind(bonePtr);
    newBone->resetLanguage(bonePtr, project->unicodeStringFactory(), project->castLanguage());
}

void
CreateBoneCommand::setNameSuffix(nanoem_mutable_model_bone_t *bone, const char *suffix,
    nanoem_unicode_string_factory_t *factory, nanoem_status_t *status)
{
    setNameSuffix(bone, suffix, NANOEM_LANGUAGE_TYPE_JAPANESE, factory, status);
    setNameSuffix(bone, suffix, NANOEM_LANGUAGE_TYPE_ENGLISH, factory, status);
}

void
CreateBoneCommand::setNameSuffix(nanoem_mutable_model_bone_t *bone, const char *suffix, nanoem_language_type_t language,
    nanoem_unicode_string_factory_t *factory, nanoem_status_t *status)
{
    String result;
    const nanoem_model_bone_t *origin = nanoemMutableModelBoneGetOriginObject(bone);
    StringUtils::getUtf8String(nanoemModelBoneGetName(origin, language), factory, result);
    result.append(suffix);
    StringUtils::UnicodeStringScope us(factory);
    if (StringUtils::tryGetString(factory, result, us)) {
        nanoemMutableModelBoneSetName(bone, us.value(), language, status);
    }
}

CreateBoneCommand::CreateBoneCommand(
    Model *activeModel, int offset, const nanoem_model_bone_t *base, const Vector3 *origin)
    : BaseUndoCommand(activeModel->project())
    , m_base(base)
    , m_offset(offset)
    , m_activeModel(activeModel)
    , m_creatingBone(m_activeModel)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    Project *project = currentProject();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    char buffer[Inline::kMarkerStringLength];
    if (m_base) {
        nanoemMutableModelBoneCopy(m_creatingBone, m_base, &status);
    }
    nanoem_rsize_t numBones;
    nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
    StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, numBones + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelBoneSetName(m_creatingBone, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "NewBone%zu", numBones + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelBoneSetName(m_creatingBone, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    if (origin) {
        nanoemMutableModelBoneSetOrigin(m_creatingBone, glm::value_ptr(Vector4(*origin, 1)));
    }
    setup(nanoemMutableModelBoneGetOriginObject(m_creatingBone), project);
}

CreateBoneCommand::~CreateBoneCommand() NANOEM_DECL_NOEXCEPT
{
    m_activeModel = nullptr;
}

void
CreateBoneCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    const nanoem_model_bone_t *bonePtr = nanoemMutableModelBoneGetOriginObject(m_creatingBone);
    nanoemMutableModelRemoveBoneObject(model, m_creatingBone, &status);
    m_activeModel->removeBoneReference(bonePtr);
    assignError(status, error);
}

void
CreateBoneCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoem_model_bone_t *bonePtr = nanoemMutableModelBoneGetOriginObject(m_creatingBone);
    nanoemMutableModelInsertBoneObject(model, m_creatingBone, m_offset, &status);
    m_activeModel->addBoneReference(bonePtr);
    assignError(status, error);
}

void
CreateBoneCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateBoneCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateBoneCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
CreateBoneCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "CreateBoneCommand";
}

undo_command_t *
CreateBoneAsDestinationCommand::create(Model *activeModel, const nanoem_model_bone_t *base)
{
    CreateBoneAsDestinationCommand *command = nanoem_new(CreateBoneAsDestinationCommand(activeModel, base));
    return command->createCommand();
}

CreateBoneAsDestinationCommand::CreateBoneAsDestinationCommand(Model *activeModel, const nanoem_model_bone_t *base)
    : BaseUndoCommand(activeModel->project())
    , m_parent(nanoemModelBoneGetParentBoneObject(base))
    , m_activeModel(activeModel)
    , m_creatingBone(m_activeModel)
    , m_boneIndex(model::Bone::index(base) + 1)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    Project *project = currentProject();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    nanoemMutableModelBoneCopy(m_creatingBone, base, &status);
    nanoemMutableModelBoneSetParentBoneObject(m_creatingBone, base);
    CreateBoneCommand::setNameSuffix(m_creatingBone,
        reinterpret_cast<const char *>(model::Bone::kNameDestinationInJapanese), NANOEM_LANGUAGE_TYPE_JAPANESE, factory,
        &status);
    CreateBoneCommand::setNameSuffix(m_creatingBone, "D", NANOEM_LANGUAGE_TYPE_ENGLISH, factory, &status);
    CreateBoneCommand::setup(nanoemMutableModelBoneGetOriginObject(m_creatingBone), project);
}

CreateBoneAsDestinationCommand::~CreateBoneAsDestinationCommand() NANOEM_DECL_NOEXCEPT
{
}

void
CreateBoneAsDestinationCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    m_activeModel->removeBoneReference(nanoemMutableModelBoneGetOriginObject(m_creatingBone));
    nanoemMutableModelRemoveBoneObject(model, m_creatingBone, &status);
    assignError(status, error);
}

void
CreateBoneAsDestinationCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelInsertBoneObject(model, m_creatingBone, m_boneIndex, &status);
    m_activeModel->addBoneReference(nanoemMutableModelBoneGetOriginObject(m_creatingBone));
    assignError(status, error);
}

void
CreateBoneAsDestinationCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateBoneAsDestinationCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateBoneAsDestinationCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
CreateBoneAsDestinationCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "CreateBoneAsDestinationCommand";
}

undo_command_t *
CreateBoneAsStagingParentCommand::create(Model *activeModel, const nanoem_model_bone_t *base)
{
    CreateBoneAsStagingParentCommand *command = nanoem_new(CreateBoneAsStagingParentCommand(activeModel, base));
    return command->createCommand();
}

CreateBoneAsStagingParentCommand::CreateBoneAsStagingParentCommand(Model *activeModel, const nanoem_model_bone_t *base)
    : BaseUndoCommand(activeModel->project())
    , m_parent(nanoemModelBoneGetParentBoneObject(base))
    , m_activeModel(activeModel)
    , m_creatingBone(m_activeModel)
    , m_boneIndex(model::Bone::index(base) + 1)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    Project *project = currentProject();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    nanoemMutableModelBoneCopy(m_creatingBone, base, &status);
    nanoemMutableModelBoneSetParentBoneObject(m_creatingBone, base);
    CreateBoneCommand::setNameSuffix(m_creatingBone, "+", factory, &status);
    CreateBoneCommand::setup(nanoemMutableModelBoneGetOriginObject(m_creatingBone), project);
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        nanoem_model_bone_t *bonePtr = bones[i];
        if (nanoemModelBoneGetParentBoneObject(bonePtr) == base) {
            nanoem_mutable_model_bone_t *mutableBone = nanoemMutableModelBoneCreateAsReference(bonePtr, &status);
            m_baseBoneChildren.push_back(mutableBone);
            break;
        }
    }
}

CreateBoneAsStagingParentCommand::~CreateBoneAsStagingParentCommand() NANOEM_DECL_NOEXCEPT
{
    for (BoneList::const_iterator it = m_baseBoneChildren.begin(), end = m_baseBoneChildren.end(); it != end; ++it) {
        nanoemMutableModelBoneDestroy(*it);
    }
    m_baseBoneChildren.clear();
}

void
CreateBoneAsStagingParentCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    m_activeModel->removeBoneReference(nanoemMutableModelBoneGetOriginObject(m_creatingBone));
    nanoemMutableModelRemoveBoneObject(model, m_creatingBone, &status);
    for (BoneList::const_iterator it = m_baseBoneChildren.begin(), end = m_baseBoneChildren.end(); it != end; ++it) {
        nanoemMutableModelBoneSetParentBoneObject(*it, m_parent);
    }
    assignError(status, error);
}

void
CreateBoneAsStagingParentCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelInsertBoneObject(model, m_creatingBone, m_boneIndex, &status);
    const nanoem_model_bone_t *origin = nanoemMutableModelBoneGetOriginObject(m_creatingBone);
    for (BoneList::const_iterator it = m_baseBoneChildren.begin(), end = m_baseBoneChildren.end(); it != end; ++it) {
        nanoemMutableModelBoneSetParentBoneObject(*it, origin);
    }
    m_activeModel->addBoneReference(nanoemMutableModelBoneGetOriginObject(m_creatingBone));
    assignError(status, error);
}

void
CreateBoneAsStagingParentCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateBoneAsStagingParentCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateBoneAsStagingParentCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
CreateBoneAsStagingParentCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "CreateBoneAsStagingParentCommand";
}

undo_command_t *
CreateBoneAsStagingChildCommand::create(Model *activeModel, nanoem_model_bone_t *base)
{
    CreateBoneAsStagingChildCommand *command = nanoem_new(CreateBoneAsStagingChildCommand(activeModel, base));
    return command->createCommand();
}

CreateBoneAsStagingChildCommand::CreateBoneAsStagingChildCommand(Model *activeModel, nanoem_model_bone_t *base)
    : BaseUndoCommand(activeModel->project())
    , m_parent(nanoemModelBoneGetParentBoneObject(base))
    , m_activeModel(activeModel)
    , m_creatingBone(m_activeModel)
    , m_baseBone(base)
    , m_boneIndex(model::Bone::index(base))
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    Project *project = currentProject();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    nanoemMutableModelBoneCopy(m_creatingBone, base, &status);
    nanoemMutableModelBoneSetParentBoneObject(m_creatingBone, m_parent);
    CreateBoneCommand::setNameSuffix(m_creatingBone, "-", factory, &status);
    CreateBoneCommand::setup(nanoemMutableModelBoneGetOriginObject(m_creatingBone), project);
}

CreateBoneAsStagingChildCommand::~CreateBoneAsStagingChildCommand() NANOEM_DECL_NOEXCEPT
{
}

void
CreateBoneAsStagingChildCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    m_activeModel->removeBoneReference(nanoemMutableModelBoneGetOriginObject(m_creatingBone));
    nanoemMutableModelRemoveBoneObject(model, m_creatingBone, &status);
    nanoemMutableModelBoneSetParentBoneObject(m_baseBone, m_parent);
    assignError(status, error);
}

void
CreateBoneAsStagingChildCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelInsertBoneObject(model, m_creatingBone, m_boneIndex, &status);
    nanoem_model_bone_t *origin = nanoemMutableModelBoneGetOriginObject(m_creatingBone);
    nanoemMutableModelBoneSetParentBoneObject(m_baseBone, origin);
    m_activeModel->addBoneReference(origin);
    assignError(status, error);
}

void
CreateBoneAsStagingChildCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateBoneAsStagingChildCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateBoneAsStagingChildCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
CreateBoneAsStagingChildCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "CreateBoneAsStagingChildCommand";
}

undo_command_t *
CreateDraggedParentBoneCommand::create(
    Model *activeModel, nanoem_mutable_model_bone_t *dest, nanoem_mutable_model_bone_t *source)
{
    CreateDraggedParentBoneCommand *command = nanoem_new(CreateDraggedParentBoneCommand(activeModel, dest, source));
    return command->createCommand();
}

CreateDraggedParentBoneCommand::CreateDraggedParentBoneCommand(
    Model *activeModel, nanoem_mutable_model_bone_t *dest, nanoem_mutable_model_bone_t *source)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_parentBone(nanoemModelBoneGetParentBoneObject(nanoemMutableModelBoneGetOriginObject(source)))
    , m_addingBone(dest)
    , m_sourceBone(source)
{
}

CreateDraggedParentBoneCommand::~CreateDraggedParentBoneCommand() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelBoneDestroy(m_addingBone);
    m_addingBone = nullptr;
    nanoemMutableModelBoneDestroy(m_sourceBone);
    m_sourceBone = nullptr;
}

void
CreateDraggedParentBoneCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    m_activeModel->removeBoneReference(nanoemMutableModelBoneGetOriginObject(m_addingBone));
    nanoemMutableModelRemoveBoneObject(model, m_addingBone, &status);
    nanoemMutableModelBoneSetParentBoneObject(m_sourceBone, m_parentBone);
    assignError(status, error);
}

void
CreateDraggedParentBoneCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelInsertBoneObject(model, m_addingBone, -1, &status);
    m_activeModel->addBoneReference(nanoemMutableModelBoneGetOriginObject(m_addingBone));
    const nanoem_model_bone_t *originSourceBonePtr = nanoemMutableModelBoneGetOriginObject(m_sourceBone),
                              *originAddingBonePtr = nanoemMutableModelBoneGetOriginObject(m_addingBone);
    nanoemMutableModelBoneSetParentBoneObject(m_sourceBone, originAddingBonePtr);
    nanoemMutableModelBoneSetTargetBoneObject(m_addingBone, originSourceBonePtr);
    assignError(status, error);
}

void
CreateDraggedParentBoneCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateDraggedParentBoneCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateDraggedParentBoneCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
CreateDraggedParentBoneCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "CreateDraggedParentBoneCommand";
}

undo_command_t *
CreateDraggedTargetBoneCommand::create(
    Model *activeModel, nanoem_mutable_model_bone_t *dest, nanoem_mutable_model_bone_t *source)
{
    CreateDraggedTargetBoneCommand *command = nanoem_new(CreateDraggedTargetBoneCommand(activeModel, dest, source));
    return command->createCommand();
}

CreateDraggedTargetBoneCommand::CreateDraggedTargetBoneCommand(
    Model *activeModel, nanoem_mutable_model_bone_t *dest, nanoem_mutable_model_bone_t *source)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_destinationBone(nanoemModelBoneGetTargetBoneObject(nanoemMutableModelBoneGetOriginObject(source)))
    , m_addingBone(dest)
    , m_sourceBone(source)
{
}

CreateDraggedTargetBoneCommand::~CreateDraggedTargetBoneCommand() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelBoneDestroy(m_addingBone);
    m_addingBone = nullptr;
    nanoemMutableModelBoneDestroy(m_sourceBone);
    m_sourceBone = nullptr;
}

void
CreateDraggedTargetBoneCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    m_activeModel->removeBoneReference(nanoemMutableModelBoneGetOriginObject(m_addingBone));
    nanoemMutableModelRemoveBoneObject(model, m_addingBone, &status);
    nanoemMutableModelBoneSetTargetBoneObject(m_sourceBone, m_destinationBone);
    assignError(status, error);
}

void
CreateDraggedTargetBoneCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelInsertBoneObject(model, m_addingBone, -1, &status);
    m_activeModel->addBoneReference(nanoemMutableModelBoneGetOriginObject(m_addingBone));
    const nanoem_model_bone_t *originSourceBonePtr = nanoemMutableModelBoneGetOriginObject(m_sourceBone),
                              *originAddingBonePtr = nanoemMutableModelBoneGetOriginObject(m_addingBone);
    nanoemMutableModelBoneSetParentBoneObject(m_addingBone, originSourceBonePtr);
    nanoemMutableModelBoneSetTargetBoneObject(m_sourceBone, originAddingBonePtr);
    assignError(status, error);
}

void
CreateDraggedTargetBoneCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateDraggedTargetBoneCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateDraggedTargetBoneCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
CreateDraggedTargetBoneCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "CreateDraggedTargetBoneCommand";
}

undo_command_t *
AddBoneToConstraintCommand::create(Model *activeModel, nanoem_model_constraint_t *constraintPtr)
{
    AddBoneToConstraintCommand *command = nanoem_new(AddBoneToConstraintCommand(activeModel, constraintPtr));
    return command->createCommand();
}

AddBoneToConstraintCommand::AddBoneToConstraintCommand(Model *activeModel, nanoem_model_constraint_t *constraintPtr)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_mutableConstraint(constraintPtr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    const model::Bone::Set *boneSet = m_activeModel->selection()->allBoneSet();
    for (model::Bone::Set::const_iterator it = boneSet->begin(), end = boneSet->end(); it != end; ++it) {
        const nanoem_model_bone_t *bonePtr = *it;
        nanoem_mutable_model_constraint_joint_t *jointPtr =
            nanoemMutableModelConstraintJointCreate(m_mutableConstraint, &status);
        nanoemMutableModelConstraintJointSetBoneObject(jointPtr, bonePtr);
        m_joints.push_back(jointPtr);
    }
}

AddBoneToConstraintCommand::~AddBoneToConstraintCommand() NANOEM_DECL_NOEXCEPT
{
    for (ConstraintJointList::const_iterator it = m_joints.begin(), end = m_joints.end(); it != end; ++it) {
        nanoemMutableModelConstraintJointDestroy(*it);
    }
    m_joints.clear();
}

void
AddBoneToConstraintCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ConstraintJointList::const_iterator it = m_joints.begin(), end = m_joints.end(); it != end; ++it) {
        nanoemMutableModelConstraintRemoveJointObject(m_mutableConstraint, *it, &status);
    }
    assignError(status, error);
}

void
AddBoneToConstraintCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ConstraintJointList::const_iterator it = m_joints.begin(), end = m_joints.end(); it != end; ++it) {
        nanoemMutableModelConstraintInsertJointObject(m_mutableConstraint, *it, -1, &status);
    }
    assignError(status, error);
}

void
AddBoneToConstraintCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
AddBoneToConstraintCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
AddBoneToConstraintCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
AddBoneToConstraintCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "AddBoneToConstraintCommand";
}

undo_command_t *
DeleteBoneCommand::create(Model *activeModel, nanoem_rsize_t boneIndex)
{
    DeleteBoneCommand *command = nanoem_new(DeleteBoneCommand(activeModel, boneIndex));
    return command->createCommand();
}

DeleteBoneCommand::DeleteBoneCommand(Model *activeModel, nanoem_rsize_t boneIndex)
    : BaseUndoCommand(activeModel->project())
    , m_parentBonePtr(nullptr)
    , m_boneIndex(boneIndex)
    , m_activeModel(activeModel)
{
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
    nanoem_model_bone_t *deletingBonePtr = bones[m_boneIndex];
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_deletingBone = nanoemMutableModelBoneCreateAsReference(deletingBonePtr, &status);
    m_parentBonePtr = nanoemModelBoneGetParentBoneObject(deletingBonePtr);
    m_deletingBoneState.save(m_activeModel, deletingBonePtr, &status);
}

DeleteBoneCommand::~DeleteBoneCommand() NANOEM_DECL_NOEXCEPT
{
    m_deletingBoneState.clear();
    nanoemMutableModelBoneDestroy(m_deletingBone);
    m_deletingBone = nullptr;
    m_activeModel = nullptr;
}

void
DeleteBoneCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoem_model_bone_t *bonePtr = nanoemMutableModelBoneGetOriginObject(m_deletingBone);
    nanoemMutableModelInsertBoneObject(model, m_deletingBone, Inline::saturateInt32(m_boneIndex), &status);
    m_activeModel->addBoneReference(bonePtr);
    m_deletingBoneState.restore(bonePtr);
    assignError(status, error);
}

void
DeleteBoneCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    const nanoem_model_bone_t *bonePtr = nanoemMutableModelBoneGetOriginObject(m_deletingBone);
    nanoemMutableModelRemoveBoneObject(model, m_deletingBone, &status);
    m_activeModel->removeBoneReference(bonePtr);
    m_deletingBoneState.setParentBone(m_parentBonePtr);
    if (m_activeModel->activeBone() == bonePtr) {
        m_activeModel->setActiveBone(nullptr);
    }
    assignError(status, error);
}

void
DeleteBoneCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
DeleteBoneCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
DeleteBoneCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
DeleteBoneCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "DeleteBoneCommand";
}

void
BaseMoveBoneCommand::undo(Error &error)
{
    move(m_fromBoneIndex, m_toBoneIndex, error);
}

void
BaseMoveBoneCommand::redo(Error &error)
{
    move(m_toBoneIndex, m_fromBoneIndex, error);
}

BaseMoveBoneCommand::BaseMoveBoneCommand(Model *activeModel, nanoem_rsize_t fromBoneIndex, nanoem_rsize_t toBoneIndex)
    : BaseUndoCommand(activeModel->project())
    , m_fromBoneIndex(fromBoneIndex)
    , m_toBoneIndex(toBoneIndex)
    , m_activeModel(activeModel)
{
}

void
BaseMoveBoneCommand::move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
    nanoem_model_bone_t *fromBone = bones[fromIndex];
    ScopedMutableBone bone(fromBone);
    DeletingBoneState state;
    state.save(m_activeModel, fromBone, &status);
    nanoemMutableModelRemoveBoneObject(model, bone, &status);
    int index = Inline::saturateInt32(glm::min(toIndex, numBones));
    nanoemMutableModelInsertBoneObject(model, bone, index, &status);
    state.restore(fromBone);
    assignError(status, error);
}

undo_command_t *
MoveBoneDownCommand::create(Model *activeModel, nanoem_rsize_t boneIndex)
{
    MoveBoneDownCommand *command = nanoem_new(MoveBoneDownCommand(activeModel, boneIndex));
    return command->createCommand();
}

MoveBoneDownCommand::MoveBoneDownCommand(Model *activeModel, nanoem_rsize_t boneIndex)
    : BaseMoveBoneCommand(activeModel, boneIndex, boneIndex + 1)
{
}

MoveBoneDownCommand::~MoveBoneDownCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveBoneDownCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveBoneDownCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveBoneDownCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
MoveBoneDownCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "MoveBoneDownCommand";
}

undo_command_t *
MoveBoneUpCommand::create(Model *activeModel, nanoem_rsize_t boneIndex)
{
    MoveBoneUpCommand *command = nanoem_new(MoveBoneUpCommand(activeModel, boneIndex));
    return command->createCommand();
}

MoveBoneUpCommand::MoveBoneUpCommand(Model *activeModel, nanoem_rsize_t boneIndex)
    : BaseMoveBoneCommand(activeModel, boneIndex, boneIndex > 0 ? boneIndex - 1 : 0)
{
}

MoveBoneUpCommand::~MoveBoneUpCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveBoneUpCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveBoneUpCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveBoneUpCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
MoveBoneUpCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "MoveBoneUpCommand";
}

undo_command_t *
CreateMorphCommand::create(
    Model *activeModel, int offset, const nanoem_model_morph_t *base, nanoem_model_morph_type_t type)
{
    CreateMorphCommand *command = nanoem_new(CreateMorphCommand(activeModel, offset, base, type));
    return command->createCommand();
}

void
CreateMorphCommand::setup(nanoem_model_morph_t *morphPtr, Project *project)
{
    model::Morph *newMorph = model::Morph::create();
    newMorph->bind(morphPtr);
    newMorph->resetLanguage(morphPtr, project->unicodeStringFactory(), project->castLanguage());
}

CreateMorphCommand::CreateMorphCommand(
    Model *activeModel, int offset, const nanoem_model_morph_t *base, nanoem_model_morph_type_t type)
    : BaseUndoCommand(activeModel->project())
    , m_base(base)
    , m_type(type)
    , m_offset(offset)
    , m_activeModel(activeModel)
    , m_creatingMorph(m_activeModel)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    Project *project = currentProject();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    char buffer[Inline::kMarkerStringLength];
    if (m_base) {
        nanoemMutableModelMorphCopy(m_creatingMorph, m_base, &status);
    }
    else {
        nanoemMutableModelMorphSetType(m_creatingMorph, m_type);
    }
    nanoem_rsize_t numMorphs;
    nanoemModelGetAllMorphObjects(m_activeModel->data(), &numMorphs);
    StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, numMorphs + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelMorphSetName(m_creatingMorph, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "NewMorph%zu", numMorphs + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelMorphSetName(m_creatingMorph, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    setup(nanoemMutableModelMorphGetOriginObject(m_creatingMorph), project);
}

CreateMorphCommand::~CreateMorphCommand() NANOEM_DECL_NOEXCEPT
{
    m_activeModel = nullptr;
}

void
CreateMorphCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    const nanoem_model_morph_t *morphPtr = nanoemMutableModelMorphGetOriginObject(m_creatingMorph);
    nanoemMutableModelRemoveMorphObject(model, m_creatingMorph, &status);
    m_activeModel->removeMorphReference(morphPtr);
    assignError(status, error);
}

void
CreateMorphCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    const nanoem_model_morph_t *morphPtr = nanoemMutableModelMorphGetOriginObject(m_creatingMorph);
    nanoemMutableModelInsertMorphObject(model, m_creatingMorph, m_offset, &status);
    m_activeModel->addMorphReference(morphPtr);
    assignError(status, error);
}

void
CreateMorphCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateMorphCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateMorphCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
CreateMorphCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "CreateMorphCommand";
}

undo_command_t *
CreateBoneMorphFromPoseCommand::create(
    Model *activeModel, const model::BindPose::BoneTransformMap &transforms, const String &filename)
{
    CreateBoneMorphFromPoseCommand *command =
        nanoem_new(CreateBoneMorphFromPoseCommand(activeModel, transforms, filename));
    return command->createCommand();
}

CreateBoneMorphFromPoseCommand::CreateBoneMorphFromPoseCommand(
    Model *activeModel, const model::BindPose::BoneTransformMap &transforms, const String &filename)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_creatingMorph(m_activeModel)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    Project *project = currentProject();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope us(factory);
    if (StringUtils::tryGetString(factory, filename, us)) {
        nanoemMutableModelMorphSetName(m_creatingMorph, us.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
        nanoemMutableModelMorphSetName(m_creatingMorph, us.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    nanoemMutableModelMorphSetType(m_creatingMorph, NANOEM_MODEL_MORPH_TYPE_BONE);
    nanoemMutableModelMorphSetCategory(m_creatingMorph, NANOEM_MODEL_MORPH_CATEGORY_OTHER);
    for (model::BindPose::BoneTransformMap::const_iterator it = transforms.begin(), end = transforms.end(); it != end;
         ++it) {
        if (const nanoem_model_bone_t *bonePtr = m_activeModel->findBone(it->first)) {
            const Vector3 &translation = it->second.first;
            const Quaternion &orientation = it->second.second;
            if (glm::all(glm::epsilonNotEqual(translation, Constants::kZeroV3, Constants::kEpsilon)) ||
                glm::all(glm::epsilonNotEqual(orientation, Constants::kZeroQ, Constants::kEpsilon))) {
                command::ScopedMutableMorphBone scoped(m_creatingMorph);
                nanoemMutableModelMorphBoneSetBoneObject(scoped, bonePtr);
                nanoemMutableModelMorphBoneSetTranslation(scoped, glm::value_ptr(Vector4(translation, 0)));
                nanoemMutableModelMorphBoneSetOrientation(scoped, glm::value_ptr(orientation));
                nanoemMutableModelMorphInsertBoneMorphObject(m_creatingMorph, scoped, -1, &status);
            }
        }
    }
    CreateMorphCommand::setup(nanoemMutableModelMorphGetOriginObject(m_creatingMorph), project);
}

CreateBoneMorphFromPoseCommand::~CreateBoneMorphFromPoseCommand() NANOEM_DECL_NOEXCEPT
{
}

void
CreateBoneMorphFromPoseCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    m_activeModel->removeMorphReference(nanoemMutableModelMorphGetOriginObject(m_creatingMorph));
    nanoemMutableModelRemoveMorphObject(model, m_creatingMorph, &status);
    assignError(status, error);
}

void
CreateBoneMorphFromPoseCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelInsertMorphObject(model, m_creatingMorph, -1, &status);
    m_activeModel->addMorphReference(nanoemMutableModelMorphGetOriginObject(m_creatingMorph));
    assignError(status, error);
}

void
CreateBoneMorphFromPoseCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateBoneMorphFromPoseCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateBoneMorphFromPoseCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
CreateBoneMorphFromPoseCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "CreateBoneMorphFromPoseCommand";
}

undo_command_t *
CreateVertexMorphFromModelCommand::create(Model *activeModel, const nanoem_model_t *modelPtr, const String &filename)
{
    CreateVertexMorphFromModelCommand *command =
        nanoem_new(CreateVertexMorphFromModelCommand(activeModel, modelPtr, filename));
    return command->createCommand();
}

CreateVertexMorphFromModelCommand::CreateVertexMorphFromModelCommand(
    Model *activeModel, const nanoem_model_t *modelPtr, const String &filename)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_creatingMorph(m_activeModel)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    Project *project = currentProject();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope us(factory);
    if (StringUtils::tryGetString(factory, filename, us)) {
        nanoemMutableModelMorphSetName(m_creatingMorph, us.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
        nanoemMutableModelMorphSetName(m_creatingMorph, us.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    nanoemMutableModelMorphSetType(m_creatingMorph, NANOEM_MODEL_MORPH_TYPE_VERTEX);
    nanoemMutableModelMorphSetCategory(m_creatingMorph, NANOEM_MODEL_MORPH_CATEGORY_OTHER);
    {
        nanoem_rsize_t numRightVertices, numLeftVertices;
        nanoem_model_vertex_t *const *rightVertices =
            nanoemModelGetAllVertexObjects(m_activeModel->data(), &numLeftVertices);
        nanoem_model_vertex_t *const *leftVertices = nanoemModelGetAllVertexObjects(modelPtr, &numRightVertices);
        for (nanoem_rsize_t i = 0; i < numRightVertices; i++) {
            const nanoem_model_vertex_t *leftVertex = leftVertices[i];
            nanoem_model_vertex_t *rightVertex = rightVertices[i];
            const Vector3 leftOrigin(glm::make_vec3(nanoemModelVertexGetOrigin(leftVertex))),
                rightOrigin(glm::make_vec3(nanoemModelVertexGetOrigin(rightVertex)));
            if (glm::abs(glm::distance(rightOrigin, leftOrigin)) > 0) {
                nanoem_mutable_model_morph_vertex_t *item =
                    nanoemMutableModelMorphVertexCreate(m_creatingMorph, &status);
                nanoemMutableModelMorphVertexSetVertexObject(item, rightVertex);
                nanoemMutableModelMorphVertexSetPosition(item, glm::value_ptr(Vector4(leftOrigin - rightOrigin, 0)));
                nanoemMutableModelMorphInsertVertexMorphObject(m_creatingMorph, item, -1, &status);
                nanoemMutableModelMorphVertexDestroy(item);
            }
        }
    }
    CreateMorphCommand::setup(nanoemMutableModelMorphGetOriginObject(m_creatingMorph), project);
}

CreateVertexMorphFromModelCommand::~CreateVertexMorphFromModelCommand() NANOEM_DECL_NOEXCEPT
{
}

void
CreateVertexMorphFromModelCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    m_activeModel->removeMorphReference(nanoemMutableModelMorphGetOriginObject(m_creatingMorph));
    nanoemMutableModelRemoveMorphObject(model, m_creatingMorph, &status);
    assignError(status, error);
}

void
CreateVertexMorphFromModelCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelInsertMorphObject(model, m_creatingMorph, -1, &status);
    m_activeModel->addMorphReference(nanoemMutableModelMorphGetOriginObject(m_creatingMorph));
    assignError(status, error);
}

void
CreateVertexMorphFromModelCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateVertexMorphFromModelCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateVertexMorphFromModelCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
CreateVertexMorphFromModelCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "CreateVertexMorphFromModelCommand";
}

BaseAddToMorphCommand::BaseAddToMorphCommand(Model *activeModel, nanoem_model_morph_t *morphPtr)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_mutableMorph(morphPtr)
{
}

undo_command_t *
AddVertexToMorphCommand::create(Model *activeModel, nanoem_model_morph_t *morphPtr)
{
    AddVertexToMorphCommand *command = nanoem_new(AddVertexToMorphCommand(activeModel, morphPtr));
    return command->createCommand();
}

AddVertexToMorphCommand::AddVertexToMorphCommand(Model *activeModel, nanoem_model_morph_t *morphPtr)
    : BaseAddToMorphCommand(activeModel, morphPtr)
{
    nanoem_assert(nanoemModelMorphGetType(morphPtr) == NANOEM_MODEL_MORPH_TYPE_VERTEX ||
            nanoemModelMorphGetType(morphPtr) == NANOEM_MODEL_MORPH_TYPE_TEXTURE,
        "type must be vertex");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    const model::Vertex::Set *vertexSet = m_activeModel->selection()->allVertexSet();
    for (model::Vertex::Set::const_iterator it = vertexSet->begin(), end = vertexSet->end(); it != end; ++it) {
        const nanoem_model_vertex_t *vertexPtr = *it;
        nanoem_mutable_model_morph_vertex_t *item = nanoemMutableModelMorphVertexCreate(m_mutableMorph, &status);
        nanoemMutableModelMorphVertexSetVertexObject(item, vertexPtr);
        m_vertices.push_back(item);
    }
}

AddVertexToMorphCommand::~AddVertexToMorphCommand() NANOEM_DECL_NOEXCEPT
{
    for (VertexList::const_iterator it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
        nanoemMutableModelMorphVertexDestroy(*it);
    }
    m_vertices.clear();
}

void
AddVertexToMorphCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (VertexList::const_iterator it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
        nanoemMutableModelMorphRemoveVertexMorphObject(m_mutableMorph, *it, &status);
    }
    assignError(status, error);
}

void
AddVertexToMorphCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (VertexList::const_iterator it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
        nanoemMutableModelMorphInsertVertexMorphObject(m_mutableMorph, *it, -1, &status);
    }
    assignError(status, error);
}

void
AddVertexToMorphCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
AddVertexToMorphCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
AddVertexToMorphCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
AddVertexToMorphCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "AddVertexToMorphCommand";
}

undo_command_t *
AddMaterialToMorphCommand::create(Model *activeModel, nanoem_model_morph_t *morphPtr)
{
    AddMaterialToMorphCommand *command = nanoem_new(AddMaterialToMorphCommand(activeModel, morphPtr));
    return command->createCommand();
}

AddMaterialToMorphCommand::AddMaterialToMorphCommand(Model *activeModel, nanoem_model_morph_t *morphPtr)
    : BaseAddToMorphCommand(activeModel, morphPtr)
{
    nanoem_assert(nanoemModelMorphGetType(morphPtr) == NANOEM_MODEL_MORPH_TYPE_MATERIAL, "type must be material");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    const model::Material::Set *materialSet = m_activeModel->selection()->allMaterialSet();
    for (model::Material::Set::const_iterator it = materialSet->begin(), end = materialSet->end(); it != end; ++it) {
        const nanoem_model_material_t *materialPtr = *it;
        nanoem_mutable_model_morph_material_t *item = nanoemMutableModelMorphMaterialCreate(m_mutableMorph, &status);
        nanoemMutableModelMorphMaterialSetMaterialObject(item, materialPtr);
        m_materials.push_back(item);
    }
}

AddMaterialToMorphCommand::~AddMaterialToMorphCommand() NANOEM_DECL_NOEXCEPT
{
    for (MaterialList::const_iterator it = m_materials.begin(), end = m_materials.end(); it != end; ++it) {
        nanoemMutableModelMorphMaterialDestroy(*it);
    }
    m_materials.clear();
}

void
AddMaterialToMorphCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (MaterialList::const_iterator it = m_materials.begin(), end = m_materials.end(); it != end; ++it) {
        nanoemMutableModelMorphRemoveMaterialMorphObject(m_mutableMorph, *it, &status);
    }
    assignError(status, error);
}

void
AddMaterialToMorphCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (MaterialList::const_iterator it = m_materials.begin(), end = m_materials.end(); it != end; ++it) {
        nanoemMutableModelMorphInsertMaterialMorphObject(m_mutableMorph, *it, -1, &status);
    }
    assignError(status, error);
}

void
AddMaterialToMorphCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
AddMaterialToMorphCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
AddMaterialToMorphCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
AddMaterialToMorphCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "AddMaterialToMorphCommand";
}

undo_command_t *
AddBoneToMorphCommand::create(Model *activeModel, nanoem_model_morph_t *morphPtr)
{
    AddBoneToMorphCommand *command = nanoem_new(AddBoneToMorphCommand(activeModel, morphPtr));
    return command->createCommand();
}

AddBoneToMorphCommand::AddBoneToMorphCommand(Model *activeModel, nanoem_model_morph_t *morphPtr)
    : BaseAddToMorphCommand(activeModel, morphPtr)
{
    nanoem_assert(nanoemModelMorphGetType(morphPtr) == NANOEM_MODEL_MORPH_TYPE_BONE, "type must be bone");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    const model::Bone::Set *boneSet = m_activeModel->selection()->allBoneSet();
    for (model::Bone::Set::const_iterator it = boneSet->begin(), end = boneSet->end(); it != end; ++it) {
        const nanoem_model_bone_t *bonePtr = *it;
        nanoem_mutable_model_morph_bone_t *item = nanoemMutableModelMorphBoneCreate(m_mutableMorph, &status);
        nanoemMutableModelMorphBoneSetBoneObject(item, bonePtr);
        m_bones.push_back(item);
    }
}

AddBoneToMorphCommand::~AddBoneToMorphCommand() NANOEM_DECL_NOEXCEPT
{
    for (BoneList::const_iterator it = m_bones.begin(), end = m_bones.end(); it != end; ++it) {
        nanoemMutableModelMorphBoneDestroy(*it);
    }
    m_bones.clear();
}

void
AddBoneToMorphCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (BoneList::const_iterator it = m_bones.begin(), end = m_bones.end(); it != end; ++it) {
        nanoemMutableModelMorphRemoveBoneMorphObject(m_mutableMorph, *it, &status);
    }
    assignError(status, error);
}

void
AddBoneToMorphCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (BoneList::const_iterator it = m_bones.begin(), end = m_bones.end(); it != end; ++it) {
        nanoemMutableModelMorphInsertBoneMorphObject(m_mutableMorph, *it, -1, &status);
    }
    assignError(status, error);
}

void
AddBoneToMorphCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
AddBoneToMorphCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
AddBoneToMorphCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
AddBoneToMorphCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "AddBoneToMorphCommand";
}

undo_command_t *
AddGroupToMorphCommand::create(Model *activeModel, nanoem_model_morph_t *morphPtr)
{
    AddGroupToMorphCommand *command = nanoem_new(AddGroupToMorphCommand(activeModel, morphPtr));
    return command->createCommand();
}

AddGroupToMorphCommand::AddGroupToMorphCommand(Model *activeModel, nanoem_model_morph_t *morphPtr)
    : BaseAddToMorphCommand(activeModel, morphPtr)
{
    nanoem_assert(nanoemModelMorphGetType(morphPtr) == NANOEM_MODEL_MORPH_TYPE_GROUP, "type must be group");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    const model::Morph::Set *morphSet = m_activeModel->selection()->allMorphSet();
    for (model::Morph::Set::const_iterator it = morphSet->begin(), end = morphSet->end(); it != end; ++it) {
        const nanoem_model_morph_t *morphPtr = *it;
        if (nanoemModelMorphGetType(morphPtr) != NANOEM_MODEL_MORPH_TYPE_GROUP) {
            nanoem_mutable_model_morph_group_t *item = nanoemMutableModelMorphGroupCreate(m_mutableMorph, &status);
            nanoemMutableModelMorphGroupSetMorphObject(item, morphPtr);
            m_groups.push_back(item);
        }
    }
}

AddGroupToMorphCommand::~AddGroupToMorphCommand() NANOEM_DECL_NOEXCEPT
{
    for (GroupList::const_iterator it = m_groups.begin(), end = m_groups.end(); it != end; ++it) {
        nanoemMutableModelMorphGroupDestroy(*it);
    }
    m_groups.clear();
}

void
AddGroupToMorphCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (GroupList::const_iterator it = m_groups.begin(), end = m_groups.end(); it != end; ++it) {
        nanoemMutableModelMorphRemoveGroupMorphObject(m_mutableMorph, *it, &status);
    }
    assignError(status, error);
}

void
AddGroupToMorphCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (GroupList::const_iterator it = m_groups.begin(), end = m_groups.end(); it != end; ++it) {
        nanoemMutableModelMorphInsertGroupMorphObject(m_mutableMorph, *it, -1, &status);
    }
    assignError(status, error);
}

void
AddGroupToMorphCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
AddGroupToMorphCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
AddGroupToMorphCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
AddGroupToMorphCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "AddGroupToMorphCommand";
}

undo_command_t *
AddFlipToMorphCommand::create(Model *activeModel, nanoem_model_morph_t *morphPtr)
{
    AddFlipToMorphCommand *command = nanoem_new(AddFlipToMorphCommand(activeModel, morphPtr));
    return command->createCommand();
}

AddFlipToMorphCommand::AddFlipToMorphCommand(Model *activeModel, nanoem_model_morph_t *morphPtr)
    : BaseAddToMorphCommand(activeModel, morphPtr)
{
    nanoem_assert(nanoemModelMorphGetType(morphPtr) == NANOEM_MODEL_MORPH_TYPE_FLIP, "type must be flip");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    const model::Morph::Set *morphSet = m_activeModel->selection()->allMorphSet();
    for (model::Morph::Set::const_iterator it = morphSet->begin(), end = morphSet->end(); it != end; ++it) {
        const nanoem_model_morph_t *morphPtr = *it;
        nanoem_mutable_model_morph_flip_t *item = nanoemMutableModelMorphFlipCreate(m_mutableMorph, &status);
        nanoemMutableModelMorphFlipSetMorphObject(item, morphPtr);
        m_flips.push_back(item);
    }
}

AddFlipToMorphCommand::~AddFlipToMorphCommand() NANOEM_DECL_NOEXCEPT
{
    for (FlipList::const_iterator it = m_flips.begin(), end = m_flips.end(); it != end; ++it) {
        nanoemMutableModelMorphFlipDestroy(*it);
    }
    m_flips.clear();
}

void
AddFlipToMorphCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (FlipList::const_iterator it = m_flips.begin(), end = m_flips.end(); it != end; ++it) {
        nanoemMutableModelMorphRemoveFlipMorphObject(m_mutableMorph, *it, &status);
    }
    assignError(status, error);
}

void
AddFlipToMorphCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (FlipList::const_iterator it = m_flips.begin(), end = m_flips.end(); it != end; ++it) {
        nanoemMutableModelMorphInsertFlipMorphObject(m_mutableMorph, *it, -1, &status);
    }
    assignError(status, error);
}

void
AddFlipToMorphCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
AddFlipToMorphCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
AddFlipToMorphCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
AddFlipToMorphCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "AddFlipToMorphCommand";
}

undo_command_t *
AddRigidBodyToMorphCommand::create(Model *activeModel, nanoem_model_morph_t *morphPtr)
{
    AddRigidBodyToMorphCommand *command = nanoem_new(AddRigidBodyToMorphCommand(activeModel, morphPtr));
    return command->createCommand();
}

AddRigidBodyToMorphCommand::AddRigidBodyToMorphCommand(Model *activeModel, nanoem_model_morph_t *morphPtr)
    : BaseAddToMorphCommand(activeModel, morphPtr)
{
    nanoem_assert(nanoemModelMorphGetType(morphPtr) == NANOEM_MODEL_MORPH_TYPE_BONE, "type must be rigid_body");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    const model::RigidBody::Set *rigidBodySet = m_activeModel->selection()->allRigidBodySet();
    for (model::RigidBody::Set::const_iterator it = rigidBodySet->begin(), end = rigidBodySet->end(); it != end; ++it) {
        const nanoem_model_rigid_body_t *rigidBodyPtr = *it;
        nanoem_mutable_model_morph_impulse_t *item = nanoemMutableModelMorphImpulseCreate(m_mutableMorph, &status);
        nanoemMutableModelMorphImpulseSetRigidBodyObject(item, rigidBodyPtr);
        m_rigidBodies.push_back(item);
    }
}

AddRigidBodyToMorphCommand::~AddRigidBodyToMorphCommand() NANOEM_DECL_NOEXCEPT
{
    for (RigidBodyList::const_iterator it = m_rigidBodies.begin(), end = m_rigidBodies.end(); it != end; ++it) {
        nanoemMutableModelMorphImpulseDestroy(*it);
    }
    m_rigidBodies.clear();
}

void
AddRigidBodyToMorphCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (RigidBodyList::const_iterator it = m_rigidBodies.begin(), end = m_rigidBodies.end(); it != end; ++it) {
        nanoemMutableModelMorphRemoveImpulseMorphObject(m_mutableMorph, *it, &status);
    }
    assignError(status, error);
}

void
AddRigidBodyToMorphCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (RigidBodyList::const_iterator it = m_rigidBodies.begin(), end = m_rigidBodies.end(); it != end; ++it) {
        nanoemMutableModelMorphInsertImpulseMorphObject(m_mutableMorph, *it, -1, &status);
    }
    assignError(status, error);
}

void
AddRigidBodyToMorphCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
AddRigidBodyToMorphCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
AddRigidBodyToMorphCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
AddRigidBodyToMorphCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "AddRigidBodyToMorphCommand";
}

undo_command_t *
DeleteMorphCommand::create(Model *activeModel, nanoem_rsize_t morphIndex)
{
    DeleteMorphCommand *command = nanoem_new(DeleteMorphCommand(activeModel, morphIndex));
    return command->createCommand();
}

DeleteMorphCommand::DeleteMorphCommand(Model *activeModel, nanoem_rsize_t morphIndex)
    : BaseUndoCommand(activeModel->project())
    , m_morphIndex(morphIndex)
    , m_activeModel(activeModel)
{
    nanoem_rsize_t numMorphs;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_activeModel->data(), &numMorphs);
    nanoem_model_morph_t *morphPtr = morphs[m_morphIndex];
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_deletingMorph = nanoemMutableModelMorphCreateAsReference(morphPtr, &status);
    m_deletingMorphState.save(m_activeModel, morphPtr, &status);
}

DeleteMorphCommand::~DeleteMorphCommand() NANOEM_DECL_NOEXCEPT
{
    m_deletingMorphState.clear();
    nanoemMutableModelMorphDestroy(m_deletingMorph);
    m_deletingMorph = nullptr;
    m_activeModel = nullptr;
}

void
DeleteMorphCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    const nanoem_model_morph_t *morphPtr = nanoemMutableModelMorphGetOriginObject(m_deletingMorph);
    nanoemMutableModelInsertMorphObject(model, m_deletingMorph, Inline::saturateInt32(m_morphIndex), &status);
    m_activeModel->addMorphReference(morphPtr);
    m_deletingMorphState.restore(morphPtr);
    assignError(status, error);
}

void
DeleteMorphCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    const nanoem_model_morph_t *morphPtr = nanoemMutableModelMorphGetOriginObject(m_deletingMorph);
    for (int i = NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM; i < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM; i++) {
        nanoem_model_morph_category_t category = static_cast<nanoem_model_morph_category_t>(i);
        if (m_activeModel->activeMorph(category) == morphPtr) {
            m_activeModel->setActiveMorph(category, nullptr);
        }
    }
    m_activeModel->removeMorphReference(morphPtr);
    nanoemMutableModelRemoveMorphObject(model, m_deletingMorph, &status);
    assignError(status, error);
}

void
DeleteMorphCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
DeleteMorphCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
DeleteMorphCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
DeleteMorphCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "DeleteMorphCommand";
}

void
BaseMoveMorphCommand::undo(Error &error)
{
    move(m_fromMorphIndex, m_toMorphIndex, error);
}

void
BaseMoveMorphCommand::redo(Error &error)
{
    move(m_toMorphIndex, m_fromMorphIndex, error);
}

BaseMoveMorphCommand::BaseMoveMorphCommand(
    Model *activeModel, nanoem_rsize_t fromMorphIndex, nanoem_rsize_t toMorphIndex)
    : BaseUndoCommand(activeModel->project())
    , m_fromMorphIndex(fromMorphIndex)
    , m_toMorphIndex(toMorphIndex)
    , m_activeModel(activeModel)
{
}

void
BaseMoveMorphCommand::move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoem_rsize_t numMorphs;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_activeModel->data(), &numMorphs);
    nanoem_model_morph_t *fromMorph = morphs[fromIndex];
    ScopedMutableMorph morph(fromMorph);
    DeletingMorphState state;
    state.save(m_activeModel, fromMorph, &status);
    nanoemMutableModelRemoveMorphObject(model, morph, &status);
    int index = Inline::saturateInt32(glm::min(toIndex, numMorphs));
    nanoemMutableModelInsertMorphObject(model, morph, index, &status);
    state.restore(fromMorph);
    assignError(status, error);
}

undo_command_t *
MoveMorphUpCommand::create(Model *activeModel, nanoem_rsize_t morphIndex)
{
    MoveMorphUpCommand *command = nanoem_new(MoveMorphUpCommand(activeModel, morphIndex));
    return command->createCommand();
}

MoveMorphUpCommand::MoveMorphUpCommand(Model *activeModel, nanoem_rsize_t morphIndex)
    : BaseMoveMorphCommand(activeModel, morphIndex, morphIndex > 0 ? morphIndex - 1 : 0)
{
}

MoveMorphUpCommand::~MoveMorphUpCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveMorphUpCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveMorphUpCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveMorphUpCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
MoveMorphUpCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "MoveMorphUpCommand";
}

undo_command_t *
MoveMorphDownCommand::create(Model *activeModel, nanoem_rsize_t morphIndex)
{
    MoveMorphDownCommand *command = nanoem_new(MoveMorphDownCommand(activeModel, morphIndex));
    return command->createCommand();
}

MoveMorphDownCommand::MoveMorphDownCommand(Model *activeModel, nanoem_rsize_t morphIndex)
    : BaseMoveMorphCommand(activeModel, morphIndex, morphIndex + 1)
{
}

MoveMorphDownCommand::~MoveMorphDownCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveMorphDownCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveMorphDownCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveMorphDownCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
MoveMorphDownCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "MoveMorphDownCommand";
}

undo_command_t *
CreateLabelCommand::create(Model *activeModel, int offset, const nanoem_model_label_t *base)
{
    CreateLabelCommand *command = nanoem_new(CreateLabelCommand(activeModel, offset, base));
    return command->createCommand();
}

CreateLabelCommand::CreateLabelCommand(Model *activeModel, int offset, const nanoem_model_label_t *base)
    : BaseUndoCommand(activeModel->project())
    , m_base(base)
    , m_offset(offset)
    , m_activeModel(activeModel)
    , m_creatingLabel(m_activeModel)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    Project *project = currentProject();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    char buffer[Inline::kMarkerStringLength];
    if (m_base) {
        nanoemMutableModelLabelCopy(m_creatingLabel, m_base, &status);
    }
    nanoem_rsize_t numLabels;
    nanoemModelGetAllLabelObjects(m_activeModel->data(), &numLabels);
    StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, numLabels + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelLabelSetName(m_creatingLabel, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "NewLabel%zu", numLabels + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelLabelSetName(m_creatingLabel, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    model::Label *newLabel = model::Label::create();
    newLabel->bind(nanoemMutableModelLabelGetOriginObject(m_creatingLabel));
    newLabel->resetLanguage(nanoemMutableModelLabelGetOriginObject(m_creatingLabel), factory, project->castLanguage());
}

CreateLabelCommand::~CreateLabelCommand() NANOEM_DECL_NOEXCEPT
{
    m_activeModel = nullptr;
}

void
CreateLabelCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelRemoveLabelObject(model, m_creatingLabel, &status);
    currentProject()->rebuildAllTracks();
    assignError(status, error);
}

void
CreateLabelCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelInsertLabelObject(model, m_creatingLabel, m_offset, &status);
    currentProject()->rebuildAllTracks();
    assignError(status, error);
}

void
CreateLabelCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateLabelCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateLabelCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
CreateLabelCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "CreateLabelCommand";
}

BaseAddToLabelCommand::BaseAddToLabelCommand(Model *activeModel, nanoem_model_label_t *labelPtr)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_mutableLabel(labelPtr)
{
}

undo_command_t *
AddBoneToLabelCommand::create(Model *activeModel, nanoem_model_label_t *labelPtr)
{
    AddBoneToLabelCommand *command = nanoem_new(AddBoneToLabelCommand(activeModel, labelPtr));
    return command->createCommand();
}

AddBoneToLabelCommand::AddBoneToLabelCommand(Model *activeModel, nanoem_model_label_t *labelPtr)
    : BaseAddToLabelCommand(activeModel, labelPtr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    const model::Bone::Set *boneSet = m_activeModel->selection()->allBoneSet();
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        nanoem_model_bone_t *bonePtr = bones[i];
        if (boneSet->find(bonePtr) != boneSet->end()) {
            m_items.push_back(nanoemMutableModelLabelItemCreateFromBoneObject(m_mutableLabel, bonePtr, &status));
        }
    }
}

AddBoneToLabelCommand::~AddBoneToLabelCommand() NANOEM_DECL_NOEXCEPT
{
    for (ItemList::const_iterator it = m_items.begin(), end = m_items.end(); it != end; ++it) {
        nanoemMutableModelLabelItemDestroy(*it);
    }
    m_items.clear();
}

void
AddBoneToLabelCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ItemList::const_iterator it = m_items.begin(), end = m_items.end(); it != end; ++it) {
        nanoemMutableModelLabelRemoveItemObject(m_mutableLabel, *it, &status);
    }
    assignError(status, error);
}

void
AddBoneToLabelCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ItemList::const_iterator it = m_items.begin(), end = m_items.end(); it != end; ++it) {
        nanoemMutableModelLabelInsertItemObject(m_mutableLabel, *it, -1, &status);
    }
    assignError(status, error);
}

void
AddBoneToLabelCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
AddBoneToLabelCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
AddBoneToLabelCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
AddBoneToLabelCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "AddBoneToLabelCommand";
}

undo_command_t *
AddMorphToLabelCommand::create(Model *activeModel, nanoem_model_label_t *labelPtr)
{
    AddMorphToLabelCommand *command = nanoem_new(AddMorphToLabelCommand(activeModel, labelPtr));
    return command->createCommand();
}

AddMorphToLabelCommand::AddMorphToLabelCommand(Model *activeModel, nanoem_model_label_t *labelPtr)
    : BaseAddToLabelCommand(activeModel, labelPtr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    const model::Morph::Set *morphSet = m_activeModel->selection()->allMorphSet();
    nanoem_rsize_t numMorphs;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_activeModel->data(), &numMorphs);
    for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
        nanoem_model_morph_t *morphPtr = morphs[i];
        if (morphSet->find(morphPtr) != morphSet->end()) {
            m_items.push_back(nanoemMutableModelLabelItemCreateFromMorphObject(m_mutableLabel, morphPtr, &status));
        }
    }
}

AddMorphToLabelCommand::~AddMorphToLabelCommand() NANOEM_DECL_NOEXCEPT
{
    for (ItemList::const_iterator it = m_items.begin(), end = m_items.end(); it != end; ++it) {
        nanoemMutableModelLabelItemDestroy(*it);
    }
    m_items.clear();
}

void
AddMorphToLabelCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ItemList::const_iterator it = m_items.begin(), end = m_items.end(); it != end; ++it) {
        nanoemMutableModelLabelRemoveItemObject(m_mutableLabel, *it, &status);
    }
    assignError(status, error);
}

void
AddMorphToLabelCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ItemList::const_iterator it = m_items.begin(), end = m_items.end(); it != end; ++it) {
        nanoemMutableModelLabelInsertItemObject(m_mutableLabel, *it, -1, &status);
    }
    assignError(status, error);
}

void
AddMorphToLabelCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
AddMorphToLabelCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
AddMorphToLabelCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
AddMorphToLabelCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "AddMorphToLabelCommand";
}

undo_command_t *
DeleteLabelCommand::create(Model *activeModel, nanoem_rsize_t labelIndex)
{
    DeleteLabelCommand *command = nanoem_new(DeleteLabelCommand(activeModel, labelIndex));
    return command->createCommand();
}

DeleteLabelCommand::DeleteLabelCommand(Model *activeModel, nanoem_rsize_t labelIndex)
    : BaseUndoCommand(activeModel->project())
    , m_labelIndex(labelIndex)
    , m_activeModel(activeModel)
{
    nanoem_rsize_t numLabels;
    nanoem_model_label_t *const *labels = nanoemModelGetAllLabelObjects(m_activeModel->data(), &numLabels);
    nanoem_model_label_t *labelPtr = labels[m_labelIndex];
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_deletingLabel = nanoemMutableModelLabelCreateAsReference(labelPtr, &status);
}

DeleteLabelCommand::~DeleteLabelCommand() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelLabelDestroy(m_deletingLabel);
    m_deletingLabel = nullptr;
    m_activeModel = nullptr;
}

void
DeleteLabelCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelInsertLabelObject(model, m_deletingLabel, Inline::saturateInt32(m_labelIndex), &status);
    currentProject()->rebuildAllTracks();
    assignError(status, error);
}

void
DeleteLabelCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelRemoveLabelObject(model, m_deletingLabel, &status);
    currentProject()->rebuildAllTracks();
    assignError(status, error);
}

void
DeleteLabelCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
DeleteLabelCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
DeleteLabelCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
DeleteLabelCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "DeleteLabelCommand";
}

void
BaseMoveLabelCommand::undo(Error &error)
{
    move(m_fromLabelIndex, m_toLabelIndex, error);
}

void
BaseMoveLabelCommand::redo(Error &error)
{
    move(m_toLabelIndex, m_fromLabelIndex, error);
}

BaseMoveLabelCommand::BaseMoveLabelCommand(
    Model *activeModel, nanoem_rsize_t fromLabelIndex, nanoem_rsize_t toLabelIndex)
    : BaseUndoCommand(activeModel->project())
    , m_fromLabelIndex(fromLabelIndex)
    , m_toLabelIndex(toLabelIndex)
    , m_activeModel(activeModel)
{
}

void
BaseMoveLabelCommand::move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoem_rsize_t numLabels;
    nanoem_model_label_t *const *labels = nanoemModelGetAllLabelObjects(m_activeModel->data(), &numLabels);
    ScopedMutableLabel label(labels[fromIndex]);
    nanoemMutableModelRemoveLabelObject(model, label, &status);
    int index = Inline::saturateInt32(glm::min(toIndex, numLabels));
    nanoemMutableModelInsertLabelObject(model, label, index, &status);
    currentProject()->rebuildAllTracks();
    assignError(status, error);
}

undo_command_t *
MoveLabelUpCommand::create(Model *activeModel, nanoem_rsize_t labelIndex)
{
    MoveLabelUpCommand *command = nanoem_new(MoveLabelUpCommand(activeModel, labelIndex));
    return command->createCommand();
}

MoveLabelUpCommand::MoveLabelUpCommand(Model *activeModel, nanoem_rsize_t labelIndex)
    : BaseMoveLabelCommand(activeModel, labelIndex, labelIndex > 0 ? labelIndex - 1 : 0)
{
}

MoveLabelUpCommand::~MoveLabelUpCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveLabelUpCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveLabelUpCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveLabelUpCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
MoveLabelUpCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "MoveLabelUpCommand";
}

undo_command_t *
MoveLabelDownCommand::create(Model *activeModel, nanoem_rsize_t labelIndex)
{
    MoveLabelDownCommand *command = nanoem_new(MoveLabelDownCommand(activeModel, labelIndex));
    return command->createCommand();
}

MoveLabelDownCommand::MoveLabelDownCommand(Model *activeModel, nanoem_rsize_t labelIndex)
    : BaseMoveLabelCommand(activeModel, labelIndex, labelIndex + 1)
{
}

MoveLabelDownCommand::~MoveLabelDownCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveLabelDownCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveLabelDownCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveLabelDownCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
MoveLabelDownCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "MoveLabelDownCommand";
}

undo_command_t *
CreateRigidBodyCommand::create(Model *activeModel, int offset, const nanoem_model_rigid_body_t *base)
{
    CreateRigidBodyCommand *command = nanoem_new(CreateRigidBodyCommand(activeModel, offset, base));
    return command->createCommand();
}

void
CreateRigidBodyCommand::setup(nanoem_model_rigid_body_t *rigidBodyPtr, Project *project)
{
    model::RigidBody *newBody = model::RigidBody::create();
    model::RigidBody::Resolver resolver;
    newBody->bind(rigidBodyPtr, project->physicsEngine(), false, resolver);
    newBody->resetLanguage(rigidBodyPtr, project->unicodeStringFactory(), project->castLanguage());
}

CreateRigidBodyCommand::CreateRigidBodyCommand(Model *activeModel, int offset, const nanoem_model_rigid_body_t *base)
    : BaseUndoCommand(activeModel->project())
    , m_base(base)
    , m_offset(offset)
    , m_activeModel(activeModel)
    , m_creatingRigidBody(m_activeModel)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    Project *project = currentProject();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    char buffer[Inline::kMarkerStringLength];
    if (m_base) {
        nanoemMutableModelRigidBodyCopy(m_creatingRigidBody, m_base, &status);
    }
    nanoem_rsize_t numRigidBodies;
    nanoemModelGetAllRigidBodyObjects(m_activeModel->data(), &numRigidBodies);
    StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, numRigidBodies + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelRigidBodySetName(m_creatingRigidBody, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "NewRigidBody%zu", numRigidBodies + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelRigidBodySetName(m_creatingRigidBody, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    setup(nanoemMutableModelRigidBodyGetOriginObject(m_creatingRigidBody), project);
}

CreateRigidBodyCommand::~CreateRigidBodyCommand() NANOEM_DECL_NOEXCEPT
{
    m_activeModel = nullptr;
}

void
CreateRigidBodyCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelRemoveRigidBodyObject(model, m_creatingRigidBody, &status);
    assignError(status, error);
}

void
CreateRigidBodyCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelInsertRigidBodyObject(model, m_creatingRigidBody, m_offset, &status);
    assignError(status, error);
}

void
CreateRigidBodyCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateRigidBodyCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateRigidBodyCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
CreateRigidBodyCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "CreateRigidBodyCommand";
}

undo_command_t *
DeleteRigidBodyCommand::create(Model *activeModel, nanoem_rsize_t rigidBodyIndex)
{
    DeleteRigidBodyCommand *command = nanoem_new(DeleteRigidBodyCommand(activeModel, rigidBodyIndex));
    return command->createCommand();
}

DeleteRigidBodyCommand::DeleteRigidBodyCommand(Model *activeModel, nanoem_rsize_t rigidBodyIndex)
    : BaseUndoCommand(activeModel->project())
    , m_rigidBodyIndex(rigidBodyIndex)
    , m_activeModel(activeModel)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t numRigidBodies;
    nanoem_model_rigid_body_t *const *rigidBodies =
        nanoemModelGetAllRigidBodyObjects(m_activeModel->data(), &numRigidBodies);
    nanoem_model_rigid_body_t *rigidBodyPtr = rigidBodies[m_rigidBodyIndex];
    m_deletingRigidBody = nanoemMutableModelRigidBodyCreateAsReference(rigidBodyPtr, &status);
    m_deletingRigidBodyState.save(m_activeModel, rigidBodyPtr, &status);
}

DeleteRigidBodyCommand::~DeleteRigidBodyCommand() NANOEM_DECL_NOEXCEPT
{
    m_deletingRigidBodyState.clear();
    nanoemMutableModelRigidBodyDestroy(m_deletingRigidBody);
    m_deletingRigidBody = nullptr;
    m_activeModel = nullptr;
}

void
DeleteRigidBodyCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelInsertRigidBodyObject(
        model, m_deletingRigidBody, Inline::saturateInt32(m_rigidBodyIndex), &status);
    m_deletingRigidBodyState.restore(nanoemMutableModelRigidBodyGetOriginObject(m_deletingRigidBody));
    assignError(status, error);
}

void
DeleteRigidBodyCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelRemoveRigidBodyObject(model, m_deletingRigidBody, &status);
    assignError(status, error);
}

void
DeleteRigidBodyCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
DeleteRigidBodyCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
DeleteRigidBodyCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
DeleteRigidBodyCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "DeleteRigidBodyCommand";
}

void
BaseMoveRigidBodyCommand::undo(Error &error)
{
    move(m_fromRigidBodyIndex, m_toRigidBodyIndex, error);
}

void
BaseMoveRigidBodyCommand::redo(Error &error)
{
    move(m_toRigidBodyIndex, m_fromRigidBodyIndex, error);
}

BaseMoveRigidBodyCommand::BaseMoveRigidBodyCommand(
    Model *activeModel, nanoem_rsize_t fromRigidBodyIndex, nanoem_rsize_t toRigidBodyIndex)
    : BaseUndoCommand(activeModel->project())
    , m_fromRigidBodyIndex(fromRigidBodyIndex)
    , m_toRigidBodyIndex(toRigidBodyIndex)
    , m_activeModel(activeModel)
{
}

void
BaseMoveRigidBodyCommand::move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoem_rsize_t numRigidBodies;
    nanoem_model_rigid_body_t *const *rigidBodies =
        nanoemModelGetAllRigidBodyObjects(m_activeModel->data(), &numRigidBodies);
    nanoem_model_rigid_body_t *fromRigidBody = rigidBodies[fromIndex];
    ScopedMutableRigidBody rigidBody(fromRigidBody);
    DeletingRigidBodyState state;
    state.save(m_activeModel, fromRigidBody, &status);
    nanoemMutableModelRemoveRigidBodyObject(model, rigidBody, &status);
    int index = Inline::saturateInt32(glm::min(toIndex, numRigidBodies));
    nanoemMutableModelInsertRigidBodyObject(model, rigidBody, index, &status);
    state.restore(fromRigidBody);
    assignError(status, error);
}

undo_command_t *
MoveRigidBodyUpCommand::create(Model *activeModel, nanoem_rsize_t rigidBodyIndex)
{
    MoveRigidBodyUpCommand *command = nanoem_new(MoveRigidBodyUpCommand(activeModel, rigidBodyIndex));
    return command->createCommand();
}

MoveRigidBodyUpCommand::MoveRigidBodyUpCommand(Model *activeModel, nanoem_rsize_t rigidBodyIndex)
    : BaseMoveRigidBodyCommand(activeModel, rigidBodyIndex, rigidBodyIndex > 0 ? rigidBodyIndex - 1 : 0)
{
}

MoveRigidBodyUpCommand::~MoveRigidBodyUpCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveRigidBodyUpCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveRigidBodyUpCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveRigidBodyUpCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
MoveRigidBodyUpCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "MoveRigidBodyUpCommand";
}

undo_command_t *
MoveRigidBodyDownCommand::create(Model *activeModel, nanoem_rsize_t rigidBodyIndex)
{
    MoveRigidBodyDownCommand *command = nanoem_new(MoveRigidBodyDownCommand(activeModel, rigidBodyIndex));
    return command->createCommand();
}

MoveRigidBodyDownCommand::MoveRigidBodyDownCommand(Model *activeModel, nanoem_rsize_t rigidBodyIndex)
    : BaseMoveRigidBodyCommand(activeModel, rigidBodyIndex, rigidBodyIndex + 1)
{
}

MoveRigidBodyDownCommand::~MoveRigidBodyDownCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveRigidBodyDownCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveRigidBodyDownCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveRigidBodyDownCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
MoveRigidBodyDownCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "MoveRigidBodyDownCommand";
}

undo_command_t *
CreateJointCommand::create(Model *activeModel, int offset, const nanoem_model_joint_t *base)
{
    CreateJointCommand *command = nanoem_new(CreateJointCommand(activeModel, offset, base));
    return command->createCommand();
}

void
CreateJointCommand::setup(nanoem_model_joint_t *jointPtr, Project *project)
{
    model::Joint *newJoint = model::Joint::create();
    model::RigidBody::Resolver resolver;
    newJoint->bind(jointPtr, project->physicsEngine(), resolver);
    newJoint->resetLanguage(jointPtr, project->unicodeStringFactory(), project->castLanguage());
}

CreateJointCommand::CreateJointCommand(Model *activeModel, int offset, const nanoem_model_joint_t *base)
    : BaseUndoCommand(activeModel->project())
    , m_base(base)
    , m_offset(offset)
    , m_activeModel(activeModel)
    , m_creatingJoint(m_activeModel)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    Project *project = currentProject();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    char buffer[Inline::kMarkerStringLength];
    if (m_base) {
        nanoemMutableModelJointCopy(m_creatingJoint, m_base, &status);
    }
    nanoem_rsize_t numJoints;
    nanoemModelGetAllJointObjects(m_activeModel->data(), &numJoints);
    StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, numJoints + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelJointSetName(m_creatingJoint, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "NewRigidBody%zu", numJoints + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelJointSetName(m_creatingJoint, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    setup(nanoemMutableModelJointGetOriginObject(m_creatingJoint), project);
}

CreateJointCommand::~CreateJointCommand() NANOEM_DECL_NOEXCEPT
{
    m_activeModel = nullptr;
}

void
CreateJointCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelRemoveJointObject(model, m_creatingJoint, &status);
    assignError(status, error);
}

void
CreateJointCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelInsertJointObject(model, m_creatingJoint, m_offset, &status);
    assignError(status, error);
}

void
CreateJointCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateJointCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateJointCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
CreateJointCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "CreateJointCommand";
}

undo_command_t *
CreateIntermediateJointFromTwoRigidBodiesCommand::create(Model *activeModel)
{
    CreateIntermediateJointFromTwoRigidBodiesCommand *command =
        nanoem_new(CreateIntermediateJointFromTwoRigidBodiesCommand(activeModel));
    return command->createCommand();
}

CreateIntermediateJointFromTwoRigidBodiesCommand::CreateIntermediateJointFromTwoRigidBodiesCommand(Model *activeModel)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_creatingJoint(m_activeModel)
{
    nanoem_assert(m_activeModel->selection()->countAllRigidBodies() == 2, "must have two rigid bodies");
    const model::RigidBody::Set *rigidBodySet = m_activeModel->selection()->allRigidBodySet();
    const model::RigidBody::List bodies(ListUtils::toListFromSet(*rigidBodySet));
    int body0Index = model::RigidBody::index(bodies[0]), body1Index = model::RigidBody::index(bodies[1]);
    const nanoem_model_rigid_body_t *bodyA = body0Index < body1Index ? bodies[0] : bodies[1],
                                    *bodyB = body0Index < body1Index ? bodies[1] : bodies[0];
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (nanoem_rsize_t i = NANOEM_LANGUAGE_TYPE_FIRST_ENUM; i < NANOEM_LANGUAGE_TYPE_MAX_ENUM; i++) {
        nanoem_language_type_t language = static_cast<nanoem_language_type_t>(i);
        nanoemMutableModelJointSetName(
            m_creatingJoint, nanoemModelRigidBodyGetName(bodyA, language), language, &status);
    }
    const Vector4 origin(
        (glm::make_vec4(nanoemModelRigidBodyGetOrigin(bodyA)) + glm::make_vec4(nanoemModelRigidBodyGetOrigin(bodyB))) *
        0.5f);
    const Vector4 orientation((glm::make_vec4(nanoemModelRigidBodyGetOrientation(bodyA)) +
                                  glm::make_vec4(nanoemModelRigidBodyGetOrientation(bodyB))) *
        0.5f);
    nanoemMutableModelJointSetOrigin(m_creatingJoint, glm::value_ptr(origin));
    nanoemMutableModelJointSetOrientation(m_creatingJoint, glm::value_ptr(orientation));
    nanoemMutableModelJointSetType(m_creatingJoint, NANOEM_MODEL_JOINT_TYPE_GENERIC_6DOF_SPRING_CONSTRAINT);
    nanoemMutableModelJointSetRigidBodyAObject(m_creatingJoint, bodyA);
    nanoemMutableModelJointSetRigidBodyBObject(m_creatingJoint, bodyB);
    CreateJointCommand::setup(nanoemMutableModelJointGetOriginObject(m_creatingJoint), currentProject());
}

CreateIntermediateJointFromTwoRigidBodiesCommand::~CreateIntermediateJointFromTwoRigidBodiesCommand()
    NANOEM_DECL_NOEXCEPT
{
}

void
CreateIntermediateJointFromTwoRigidBodiesCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelRemoveJointObject(model, m_creatingJoint, &status);
    assignError(status, error);
}

void
CreateIntermediateJointFromTwoRigidBodiesCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelInsertJointObject(model, m_creatingJoint, -1, &status);
    assignError(status, error);
}

void
CreateIntermediateJointFromTwoRigidBodiesCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateIntermediateJointFromTwoRigidBodiesCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateIntermediateJointFromTwoRigidBodiesCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
CreateIntermediateJointFromTwoRigidBodiesCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "CreateIntermediateJointFromTwoRigidBodiesCommand";
}

undo_command_t *
DeleteJointCommand::create(Model *activeModel, nanoem_rsize_t jointIndex)
{
    DeleteJointCommand *command = nanoem_new(DeleteJointCommand(activeModel, jointIndex));
    return command->createCommand();
}

DeleteJointCommand::DeleteJointCommand(Model *activeModel, nanoem_rsize_t jointIndex)
    : BaseUndoCommand(activeModel->project())
    , m_jointIndex(jointIndex)
    , m_activeModel(activeModel)
    , m_deletingJoint(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t numJoints;
    nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(m_activeModel->data(), &numJoints);
    m_deletingJoint = nanoemMutableModelJointCreateAsReference(joints[m_jointIndex], &status);
}

DeleteJointCommand::~DeleteJointCommand() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelJointDestroy(m_deletingJoint);
    m_deletingJoint = nullptr;
    m_activeModel = nullptr;
}

void
DeleteJointCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelInsertJointObject(model, m_deletingJoint, Inline::saturateInt32(m_jointIndex), &status);
    assignError(status, error);
}

void
DeleteJointCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelRemoveJointObject(model, m_deletingJoint, &status);
    assignError(status, error);
}

void
DeleteJointCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
DeleteJointCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
DeleteJointCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
DeleteJointCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "DeleteJointCommand";
}

void
BaseMoveJointCommand::undo(Error &error)
{
    move(m_fromJointIndex, m_toJointIndex, error);
}

void
BaseMoveJointCommand::redo(Error &error)
{
    move(m_toJointIndex, m_fromJointIndex, error);
}

BaseMoveJointCommand::BaseMoveJointCommand(
    Model *activeModel, nanoem_rsize_t fromJointIndex, nanoem_rsize_t toJointIndex)
    : BaseUndoCommand(activeModel->project())
    , m_fromJointIndex(fromJointIndex)
    , m_toJointIndex(toJointIndex)
    , m_activeModel(activeModel)
{
}

void
BaseMoveJointCommand::move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoem_rsize_t numJoints;
    nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(m_activeModel->data(), &numJoints);
    ScopedMutableJoint joint(joints[fromIndex]);
    nanoemMutableModelRemoveJointObject(model, joint, &status);
    int index = Inline::saturateInt32(glm::min(toIndex, numJoints));
    nanoemMutableModelInsertJointObject(model, joint, index, &status);
    assignError(status, error);
}

undo_command_t *
MoveJointUpCommand::create(Model *activeModel, nanoem_rsize_t jointIndex)
{
    MoveJointUpCommand *command = nanoem_new(MoveJointUpCommand(activeModel, jointIndex));
    return command->createCommand();
}

MoveJointUpCommand::MoveJointUpCommand(Model *activeModel, nanoem_rsize_t jointIndex)
    : BaseMoveJointCommand(activeModel, jointIndex, jointIndex > 0 ? jointIndex - 1 : 0)
{
}

MoveJointUpCommand::~MoveJointUpCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveJointUpCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveJointUpCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveJointUpCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
MoveJointUpCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "MoveJointUpCommand";
}

undo_command_t *
MoveJointDownCommand::create(Model *activeModel, nanoem_rsize_t jointIndex)
{
    MoveJointDownCommand *command = nanoem_new(MoveJointDownCommand(activeModel, jointIndex));
    return command->createCommand();
}

MoveJointDownCommand::MoveJointDownCommand(Model *activeModel, nanoem_rsize_t jointIndex)
    : BaseMoveJointCommand(activeModel, jointIndex, jointIndex + 1)
{
}

MoveJointDownCommand::~MoveJointDownCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveJointDownCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveJointDownCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveJointDownCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
MoveJointDownCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "MoveJointDownCommand";
}

undo_command_t *
CreateSoftBodyCommand::create(Model *activeModel, int offset, const nanoem_model_soft_body_t *base)
{
    CreateSoftBodyCommand *command = nanoem_new(CreateSoftBodyCommand(activeModel, offset, base));
    return command->createCommand();
}

CreateSoftBodyCommand::CreateSoftBodyCommand(Model *activeModel, int offset, const nanoem_model_soft_body_t *base)
    : BaseUndoCommand(activeModel->project())
    , m_base(base)
    , m_offset(offset)
    , m_activeModel(activeModel)
    , m_creatingSoftBody(m_activeModel)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    Project *project = currentProject();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    char buffer[Inline::kMarkerStringLength];
    if (m_base) {
        nanoemMutableModelSoftBodyCopy(m_creatingSoftBody, m_base, &status);
    }
    nanoem_rsize_t numSoftBodies;
    nanoemModelGetAllSoftBodyObjects(m_activeModel->data(), &numSoftBodies);
    StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, numSoftBodies + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelSoftBodySetName(m_creatingSoftBody, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "NewRigidBody%zu", numSoftBodies + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelSoftBodySetName(m_creatingSoftBody, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    model::SoftBody *newSoftBody = model::SoftBody::create();
    model::RigidBody::Resolver resolver;
    nanoem_model_soft_body_t *softBodyPtr = nanoemMutableModelSoftBodyGetOriginObject(m_creatingSoftBody);
    newSoftBody->bind(softBodyPtr, nullptr, resolver);
    newSoftBody->resetLanguage(softBodyPtr, factory, project->castLanguage());
}

CreateSoftBodyCommand::~CreateSoftBodyCommand() NANOEM_DECL_NOEXCEPT
{
    m_activeModel = nullptr;
}

void
CreateSoftBodyCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelRemoveSoftBodyObject(model, m_creatingSoftBody, &status);
    assignError(status, error);
}

void
CreateSoftBodyCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelInsertSoftBodyObject(model, m_creatingSoftBody, m_offset, &status);
    assignError(status, error);
}

void
CreateSoftBodyCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateSoftBodyCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
CreateSoftBodyCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
CreateSoftBodyCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "CreateSoftBodyCommand";
}

undo_command_t *
DeleteSoftBodyCommand::create(Model *activeModel, nanoem_rsize_t softBodyIndex)
{
    DeleteSoftBodyCommand *command = nanoem_new(DeleteSoftBodyCommand(activeModel, softBodyIndex));
    return command->createCommand();
}

DeleteSoftBodyCommand::DeleteSoftBodyCommand(Model *activeModel, nanoem_rsize_t softBodyIndex)
    : BaseUndoCommand(activeModel->project())
    , m_softBodyIndex(softBodyIndex)
    , m_activeModel(activeModel)
    , m_deletingSoftBody(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t numSoftBodies;
    nanoem_model_soft_body_t *const *softBodies =
        nanoemModelGetAllSoftBodyObjects(m_activeModel->data(), &numSoftBodies);
    m_deletingSoftBody = nanoemMutableModelSoftBodyCreateAsReference(softBodies[m_softBodyIndex], &status);
}

DeleteSoftBodyCommand::~DeleteSoftBodyCommand() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelSoftBodyDestroy(m_deletingSoftBody);
    m_deletingSoftBody = nullptr;
    m_activeModel = nullptr;
}

void
DeleteSoftBodyCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelInsertSoftBodyObject(model, m_deletingSoftBody, Inline::saturateInt32(m_softBodyIndex), &status);
    assignError(status, error);
}

void
DeleteSoftBodyCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoemMutableModelRemoveSoftBodyObject(model, m_deletingSoftBody, &status);
    assignError(status, error);
}

void
DeleteSoftBodyCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
DeleteSoftBodyCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
DeleteSoftBodyCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
DeleteSoftBodyCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "DeleteSoftBodyCommand";
}

void
BaseMoveSoftBodyCommand::undo(Error &error)
{
    move(m_fromSoftBodyIndex, m_toSoftBodyIndex, error);
}

void
BaseMoveSoftBodyCommand::redo(Error &error)
{
    move(m_toSoftBodyIndex, m_fromSoftBodyIndex, error);
}

BaseMoveSoftBodyCommand::BaseMoveSoftBodyCommand(
    Model *activeModel, nanoem_rsize_t fromSoftBodyIndex, nanoem_rsize_t toSoftBodyIndex)
    : BaseUndoCommand(activeModel->project())
    , m_fromSoftBodyIndex(fromSoftBodyIndex)
    , m_toSoftBodyIndex(toSoftBodyIndex)
    , m_activeModel(activeModel)
{
}

void
BaseMoveSoftBodyCommand::move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel, &status);
    nanoem_rsize_t numSoftBodies;
    nanoem_model_soft_body_t *const *softBodies =
        nanoemModelGetAllSoftBodyObjects(m_activeModel->data(), &numSoftBodies);
    ScopedMutableSoftBody softBody(softBodies[fromIndex]);
    nanoemMutableModelRemoveSoftBodyObject(model, softBody, &status);
    int index = Inline::saturateInt32(glm::min(toIndex, numSoftBodies));
    nanoemMutableModelInsertSoftBodyObject(model, softBody, index, &status);
    assignError(status, error);
}

undo_command_t *
MoveSoftBodyUpCommand::create(Model *activeModel, nanoem_rsize_t softBodyIndex)
{
    MoveSoftBodyUpCommand *command = nanoem_new(MoveSoftBodyUpCommand(activeModel, softBodyIndex));
    return command->createCommand();
}

MoveSoftBodyUpCommand::MoveSoftBodyUpCommand(Model *activeModel, nanoem_rsize_t softBodyIndex)
    : BaseMoveSoftBodyCommand(activeModel, softBodyIndex, softBodyIndex > 0 ? softBodyIndex - 1 : 0)
{
}

MoveSoftBodyUpCommand::~MoveSoftBodyUpCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveSoftBodyUpCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveSoftBodyUpCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveSoftBodyUpCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
MoveSoftBodyUpCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "MoveSoftBodyUpCommand";
}

undo_command_t *
MoveSoftBodyDownCommand::create(Model *activeModel, nanoem_rsize_t softBodyIndex)
{
    MoveSoftBodyDownCommand *command = nanoem_new(MoveSoftBodyDownCommand(activeModel, softBodyIndex));
    return command->createCommand();
}

MoveSoftBodyDownCommand::MoveSoftBodyDownCommand(Model *activeModel, nanoem_rsize_t softBodyIndex)
    : BaseMoveSoftBodyCommand(activeModel, softBodyIndex, softBodyIndex + 1)
{
}

MoveSoftBodyDownCommand::~MoveSoftBodyDownCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveSoftBodyDownCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveSoftBodyDownCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
MoveSoftBodyDownCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
MoveSoftBodyDownCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "MoveSoftBodyDownCommand";
}

undo_command_t *
BatchChangeAllVertexObjectsCommand::create(Model *activeModel, const List &objects, const Parameter &parameter)
{
    BatchChangeAllVertexObjectsCommand *command =
        nanoem_new(BatchChangeAllVertexObjectsCommand(activeModel, objects, parameter));
    return command->createCommand();
}

void
BatchChangeAllVertexObjectsCommand::save(const nanoem_model_vertex_t *vertexPtr, Parameter &parameter)
{
    parameter.m_origin = glm::make_vec3(nanoemModelVertexGetOrigin(vertexPtr));
    parameter.m_normal = glm::make_vec3(nanoemModelVertexGetNormal(vertexPtr));
    parameter.m_texcoord = glm::make_vec2(nanoemModelVertexGetTexCoord(vertexPtr));
    parameter.m_edgeSize = nanoemModelVertexGetEdgeSize(vertexPtr);
    parameter.m_type = nanoemModelVertexGetType(vertexPtr);
    parameter.m_sdefC = glm::make_vec3(nanoemModelVertexGetSdefC(vertexPtr));
    parameter.m_sdefR0 = glm::make_vec3(nanoemModelVertexGetSdefR0(vertexPtr));
    parameter.m_sdefR1 = glm::make_vec3(nanoemModelVertexGetSdefR1(vertexPtr));
    for (nanoem_rsize_t i = 0; i < 4; i++) {
        parameter.m_bones[i] = nanoemModelVertexGetBoneObject(vertexPtr, i);
        parameter.m_weights[i] = nanoemModelVertexGetBoneWeight(vertexPtr, i);
        parameter.m_uva[i] = glm::make_vec4(nanoemModelVertexGetAdditionalUV(vertexPtr, i));
    }
}

void
BatchChangeAllVertexObjectsCommand::restore(const Parameter &parameter, nanoem_mutable_model_vertex_t *vertexPtr)
{
    nanoemMutableModelVertexSetOrigin(vertexPtr, glm::value_ptr(Vector4(parameter.m_origin, 1)));
    nanoemMutableModelVertexSetNormal(vertexPtr, glm::value_ptr(Vector4(parameter.m_normal, 0)));
    nanoemMutableModelVertexSetTexCoord(vertexPtr, glm::value_ptr(Vector4(parameter.m_texcoord, 0, 0)));
    nanoemMutableModelVertexSetEdgeSize(vertexPtr, parameter.m_edgeSize);
    nanoemMutableModelVertexSetType(vertexPtr, parameter.m_type);
    nanoemMutableModelVertexSetSdefC(vertexPtr, glm::value_ptr(Vector4(parameter.m_sdefC, 0)));
    nanoemMutableModelVertexSetSdefR0(vertexPtr, glm::value_ptr(Vector4(parameter.m_sdefR0, 0)));
    nanoemMutableModelVertexSetSdefR1(vertexPtr, glm::value_ptr(Vector4(parameter.m_sdefR1, 0)));
    for (nanoem_rsize_t i = 0; i < 4; i++) {
        nanoemMutableModelVertexSetBoneObject(vertexPtr, parameter.m_bones[i], i);
        nanoemMutableModelVertexSetBoneWeight(vertexPtr, parameter.m_weights[i], i);
        nanoemMutableModelVertexSetAdditionalUV(vertexPtr, glm::value_ptr(parameter.m_uva[i]), i);
    }
}

BatchChangeAllVertexObjectsCommand::BatchChangeAllVertexObjectsCommand(
    Model *activeModel, const List &objects, const Parameter &parameter)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_newParameter(parameter)
{
    for (List::const_iterator it = objects.begin(), end = objects.end(); it != end; ++it) {
        Parameter parameter;
        save(*it, parameter);
        m_objects.insert(tinystl::make_pair(*it, parameter));
    }
}

BatchChangeAllVertexObjectsCommand::~BatchChangeAllVertexObjectsCommand() NANOEM_DECL_NOEXCEPT
{
    m_activeModel = nullptr;
}

void
BatchChangeAllVertexObjectsCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ParameterMap::const_iterator it = m_objects.begin(), end = m_objects.end(); it != end; ++it) {
        ScopedMutableVertex vertex(it->first);
        restore(it->second, vertex);
    }
    assignError(status, error);
}

void
BatchChangeAllVertexObjectsCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ParameterMap::const_iterator it = m_objects.begin(), end = m_objects.end(); it != end; ++it) {
        ScopedMutableVertex vertex(it->first);
        restore(m_newParameter, vertex);
    }
    assignError(status, error);
}

void
BatchChangeAllVertexObjectsCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
BatchChangeAllVertexObjectsCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
BatchChangeAllVertexObjectsCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
BatchChangeAllVertexObjectsCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "BatchChangeAllVertexObjectsCommand";
}

undo_command_t *
BatchChangeAllMaterialObjectsCommand::create(Model *activeModel, const List &objects, const Parameter &parameter)
{
    BatchChangeAllMaterialObjectsCommand *command =
        nanoem_new(BatchChangeAllMaterialObjectsCommand(activeModel, objects, parameter));
    return command->createCommand();
}

void
BatchChangeAllMaterialObjectsCommand::save(const nanoem_model_material_t *materialPtr, Parameter &parameter)
{
    parameter.m_ambientColor = glm::make_vec3(nanoemModelMaterialGetAmbientColor(materialPtr));
    parameter.m_diffuseColor = glm::make_vec3(nanoemModelMaterialGetDiffuseColor(materialPtr));
    parameter.m_specularColor = glm::make_vec3(nanoemModelMaterialGetSpecularColor(materialPtr));
    parameter.m_edgeColor = glm::make_vec3(nanoemModelMaterialGetEdgeColor(materialPtr));
    parameter.m_diffuseOpacity = nanoemModelMaterialGetDiffuseOpacity(materialPtr);
    parameter.m_specularPower = nanoemModelMaterialGetSpecularPower(materialPtr);
    parameter.m_edgeOpacity = nanoemModelMaterialGetEdgeOpacity(materialPtr);
    parameter.m_edgeSize = nanoemModelMaterialGetEdgeSize(materialPtr);
    parameter.m_sphereType = nanoemModelMaterialGetSphereMapTextureType(materialPtr);
    parameter.m_isToonShared = nanoemModelMaterialIsToonShared(materialPtr);
    parameter.m_isCullingDisabled = nanoemModelMaterialIsCullingDisabled(materialPtr);
    parameter.m_isCastingShadowEnabled = nanoemModelMaterialIsCastingShadowEnabled(materialPtr);
    parameter.m_isCastingShadowMapEnabled = nanoemModelMaterialIsCastingShadowMapEnabled(materialPtr);
    parameter.m_isShadowMapEnabled = nanoemModelMaterialIsShadowMapEnabled(materialPtr);
    parameter.m_isEdgeEnabled = nanoemModelMaterialIsEdgeEnabled(materialPtr);
    parameter.m_isVertexColorEnabled = nanoemModelMaterialIsVertexColorEnabled(materialPtr);
    parameter.m_isPointDrawEnabled = nanoemModelMaterialIsPointDrawEnabled(materialPtr);
    parameter.m_isLineDrawEnabled = nanoemModelMaterialIsLineDrawEnabled(materialPtr);
}

void
BatchChangeAllMaterialObjectsCommand::restore(const Parameter &parameter, nanoem_mutable_model_material_t *materialPtr)
{
    nanoemMutableModelMaterialSetAmbientColor(materialPtr, glm::value_ptr(Vector4(parameter.m_ambientColor, 0)));
    nanoemMutableModelMaterialSetDiffuseColor(materialPtr, glm::value_ptr(Vector4(parameter.m_diffuseColor, 0)));
    nanoemMutableModelMaterialSetSpecularColor(materialPtr, glm::value_ptr(Vector4(parameter.m_specularColor, 0)));
    nanoemMutableModelMaterialSetEdgeColor(materialPtr, glm::value_ptr(Vector4(parameter.m_edgeColor, 0)));
    nanoemMutableModelMaterialSetDiffuseOpacity(materialPtr, parameter.m_diffuseOpacity);
    nanoemMutableModelMaterialSetSpecularPower(materialPtr, parameter.m_specularPower);
    nanoemMutableModelMaterialSetEdgeOpacity(materialPtr, parameter.m_edgeOpacity);
    nanoemMutableModelMaterialSetEdgeSize(materialPtr, parameter.m_edgeSize);
    nanoemMutableModelMaterialSetSphereMapTextureType(materialPtr, parameter.m_sphereType);
    nanoemMutableModelMaterialSetToonShared(materialPtr, parameter.m_isToonShared);
    nanoemMutableModelMaterialSetCullingDisabled(materialPtr, parameter.m_isCullingDisabled);
    nanoemMutableModelMaterialSetCastingShadowEnabled(materialPtr, parameter.m_isCastingShadowEnabled);
    nanoemMutableModelMaterialSetCastingShadowMapEnabled(materialPtr, parameter.m_isCastingShadowMapEnabled);
    nanoemMutableModelMaterialSetShadowMapEnabled(materialPtr, parameter.m_isShadowMapEnabled);
    nanoemMutableModelMaterialSetEdgeEnabled(materialPtr, parameter.m_isEdgeEnabled);
    nanoemMutableModelMaterialSetVertexColorEnabled(materialPtr, parameter.m_isVertexColorEnabled);
    nanoemMutableModelMaterialSetPointDrawEnabled(materialPtr, parameter.m_isPointDrawEnabled);
    nanoemMutableModelMaterialSetLineDrawEnabled(materialPtr, parameter.m_isLineDrawEnabled);
}

BatchChangeAllMaterialObjectsCommand::BatchChangeAllMaterialObjectsCommand(
    Model *activeModel, const List &objects, const Parameter &parameter)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_newParameter(parameter)
{
    for (List::const_iterator it = objects.begin(), end = objects.end(); it != end; ++it) {
        Parameter parameter;
        save(*it, parameter);
        m_objects.insert(tinystl::make_pair(*it, parameter));
    }
}

BatchChangeAllMaterialObjectsCommand::~BatchChangeAllMaterialObjectsCommand() NANOEM_DECL_NOEXCEPT
{
    m_activeModel = nullptr;
}

void
BatchChangeAllMaterialObjectsCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ParameterMap::const_iterator it = m_objects.begin(), end = m_objects.end(); it != end; ++it) {
        ScopedMutableMaterial vertex(it->first);
        restore(it->second, vertex);
    }
    assignError(status, error);
}

void
BatchChangeAllMaterialObjectsCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ParameterMap::const_iterator it = m_objects.begin(), end = m_objects.end(); it != end; ++it) {
        ScopedMutableMaterial vertex(it->first);
        restore(m_newParameter, vertex);
    }
    assignError(status, error);
}

void
BatchChangeAllMaterialObjectsCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
BatchChangeAllMaterialObjectsCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
BatchChangeAllMaterialObjectsCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
BatchChangeAllMaterialObjectsCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "BatchChangeAllMaterialObjectsCommand";
}

undo_command_t *
BatchChangeAllBoneObjectsCommand::create(Model *activeModel, const List &objects, const Parameter &parameter)
{
    BatchChangeAllBoneObjectsCommand *command =
        nanoem_new(BatchChangeAllBoneObjectsCommand(activeModel, objects, parameter));
    return command->createCommand();
}

void
BatchChangeAllBoneObjectsCommand::save(const nanoem_model_bone_t *bonePtr, Parameter &parameter)
{
    parameter.m_origin = glm::make_vec3(nanoemModelBoneGetOrigin(bonePtr));
    parameter.m_destinationOrigin = glm::make_vec3(nanoemModelBoneGetDestinationOrigin(bonePtr));
    parameter.m_fixedAxis = glm::make_vec3(nanoemModelBoneGetFixedAxis(bonePtr));
    parameter.m_localAxisX = glm::make_vec3(nanoemModelBoneGetLocalXAxis(bonePtr));
    parameter.m_localAxisZ = glm::make_vec3(nanoemModelBoneGetLocalZAxis(bonePtr));
    parameter.m_parentBone = nanoemModelBoneGetParentBoneObject(bonePtr);
    parameter.m_inherentParentBone = nanoemModelBoneGetInherentParentBoneObject(bonePtr);
    parameter.m_targetBone = nanoemModelBoneGetTargetBoneObject(bonePtr);
    parameter.m_inherentCoefficient = nanoemModelBoneGetInherentCoefficient(bonePtr);
    parameter.m_stageIndex = nanoemModelBoneGetStageIndex(bonePtr);
    parameter.m_hasDestinationOrigin = nanoemModelBoneHasDestinationBone(bonePtr);
    parameter.m_rotateable = nanoemModelBoneIsRotateable(bonePtr);
    parameter.m_movable = nanoemModelBoneIsMovable(bonePtr);
    parameter.m_visible = nanoemModelBoneIsVisible(bonePtr);
    parameter.m_userHandleable = nanoemModelBoneIsUserHandleable(bonePtr);
    parameter.m_hasLocalInherent = nanoemModelBoneHasLocalInherent(bonePtr);
    parameter.m_hasInherentTranslation = nanoemModelBoneHasInherentTranslation(bonePtr);
    parameter.m_hasInherentOrientation = nanoemModelBoneHasInherentOrientation(bonePtr);
    parameter.m_hasFixedAxis = nanoemModelBoneHasFixedAxis(bonePtr);
    parameter.m_hasLocalAxes = nanoemModelBoneHasLocalAxes(bonePtr);
    parameter.m_isAffectedByPhysicsSimulation = nanoemModelBoneIsAffectedByPhysicsSimulation(bonePtr);
    parameter.m_hasExternalParentBone = nanoemModelBoneHasExternalParentBone(bonePtr);
}

void
BatchChangeAllBoneObjectsCommand::restore(const Parameter &parameter, nanoem_mutable_model_bone_t *bonePtr)
{
    nanoemMutableModelBoneSetOrigin(bonePtr, glm::value_ptr(Vector4(parameter.m_origin, 1)));
    nanoemMutableModelBoneSetDestinationOrigin(bonePtr, glm::value_ptr(Vector4(parameter.m_destinationOrigin, 0)));
    nanoemMutableModelBoneSetFixedAxis(bonePtr, glm::value_ptr(Vector4(parameter.m_fixedAxis, 0)));
    nanoemMutableModelBoneSetLocalXAxis(bonePtr, glm::value_ptr(Vector4(parameter.m_localAxisX, 0)));
    nanoemMutableModelBoneSetLocalZAxis(bonePtr, glm::value_ptr(Vector4(parameter.m_localAxisZ, 0)));
    nanoemMutableModelBoneSetParentBoneObject(bonePtr, parameter.m_parentBone);
    nanoemMutableModelBoneSetInherentParentBoneObject(bonePtr, parameter.m_inherentParentBone);
    nanoemMutableModelBoneSetTargetBoneObject(bonePtr, parameter.m_targetBone);
    nanoemMutableModelBoneSetInherentCoefficient(bonePtr, parameter.m_inherentCoefficient);
    nanoemMutableModelBoneSetStageIndex(bonePtr, parameter.m_stageIndex);
    nanoemMutableModelBoneSetRotateable(bonePtr, parameter.m_rotateable);
    nanoemMutableModelBoneSetMovable(bonePtr, parameter.m_movable);
    nanoemMutableModelBoneSetVisible(bonePtr, parameter.m_visible);
    nanoemMutableModelBoneSetUserHandleable(bonePtr, parameter.m_userHandleable);
    nanoemMutableModelBoneSetLocalInherentEnabled(bonePtr, parameter.m_hasLocalInherent);
    nanoemMutableModelBoneSetInherentTranslationEnabled(bonePtr, parameter.m_hasInherentTranslation);
    nanoemMutableModelBoneSetInherentOrientationEnabled(bonePtr, parameter.m_hasInherentOrientation);
    nanoemMutableModelBoneSetFixedAxisEnabled(bonePtr, parameter.m_hasFixedAxis);
    nanoemMutableModelBoneSetLocalAxesEnabled(bonePtr, parameter.m_hasLocalAxes);
    nanoemMutableModelBoneSetAffectedByPhysicsSimulation(bonePtr, parameter.m_isAffectedByPhysicsSimulation);
    nanoemMutableModelBoneEnableExternalParentBone(bonePtr, parameter.m_hasExternalParentBone);
}

BatchChangeAllBoneObjectsCommand::BatchChangeAllBoneObjectsCommand(
    Model *activeModel, const List &objects, const Parameter &parameter)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_newParameter(parameter)
{
    for (List::const_iterator it = objects.begin(), end = objects.end(); it != end; ++it) {
        Parameter parameter;
        save(*it, parameter);
        m_objects.insert(tinystl::make_pair(*it, parameter));
    }
}

BatchChangeAllBoneObjectsCommand::~BatchChangeAllBoneObjectsCommand() NANOEM_DECL_NOEXCEPT
{
    m_activeModel = nullptr;
}

void
BatchChangeAllBoneObjectsCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ParameterMap::const_iterator it = m_objects.begin(), end = m_objects.end(); it != end; ++it) {
        ScopedMutableBone bone(it->first);
        restore(it->second, bone);
    }
    assignError(status, error);
}

void
BatchChangeAllBoneObjectsCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ParameterMap::const_iterator it = m_objects.begin(), end = m_objects.end(); it != end; ++it) {
        ScopedMutableBone bone(it->first);
        restore(m_newParameter, bone);
    }
    assignError(status, error);
}

void
BatchChangeAllBoneObjectsCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
BatchChangeAllBoneObjectsCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
BatchChangeAllBoneObjectsCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
BatchChangeAllBoneObjectsCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "BatchChangeAllBoneObjectsCommand";
}

undo_command_t *
BatchChangeAllMorphObjectsCommand::create(Model *activeModel, const List &objects, const Parameter &parameter)
{
    BatchChangeAllMorphObjectsCommand *command =
        nanoem_new(BatchChangeAllMorphObjectsCommand(activeModel, objects, parameter));
    return command->createCommand();
}

void
BatchChangeAllMorphObjectsCommand::save(const nanoem_model_morph_t *morphPtr, Parameter &parameter)
{
    parameter.m_category = nanoemModelMorphGetCategory(morphPtr);
}

void
BatchChangeAllMorphObjectsCommand::restore(const Parameter &parameter, nanoem_mutable_model_morph_t *morphPtr)
{
    nanoemMutableModelMorphSetCategory(morphPtr, parameter.m_category);
}

BatchChangeAllMorphObjectsCommand::BatchChangeAllMorphObjectsCommand(
    Model *activeModel, const List &objects, const Parameter &parameter)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_newParameter(parameter)
{
    for (List::const_iterator it = objects.begin(), end = objects.end(); it != end; ++it) {
        Parameter parameter;
        save(*it, parameter);
        m_objects.insert(tinystl::make_pair(*it, parameter));
    }
}

BatchChangeAllMorphObjectsCommand::~BatchChangeAllMorphObjectsCommand() NANOEM_DECL_NOEXCEPT
{
    m_activeModel = nullptr;
}

void
BatchChangeAllMorphObjectsCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ParameterMap::const_iterator it = m_objects.begin(), end = m_objects.end(); it != end; ++it) {
        ScopedMutableMorph morph(it->first);
        restore(it->second, morph);
    }
    assignError(status, error);
}

void
BatchChangeAllMorphObjectsCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ParameterMap::const_iterator it = m_objects.begin(), end = m_objects.end(); it != end; ++it) {
        ScopedMutableMorph morph(it->first);
        restore(m_newParameter, morph);
    }
    assignError(status, error);
}

void
BatchChangeAllMorphObjectsCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
BatchChangeAllMorphObjectsCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
BatchChangeAllMorphObjectsCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
BatchChangeAllMorphObjectsCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "BatchChangeAllMorphObjectsCommand";
}

undo_command_t *
BatchChangeAllRigidBodyObjectsCommand::create(Model *activeModel, const List &objects, const Parameter &parameter)
{
    BatchChangeAllRigidBodyObjectsCommand *command =
        nanoem_new(BatchChangeAllRigidBodyObjectsCommand(activeModel, objects, parameter));
    return command->createCommand();
}

void
BatchChangeAllRigidBodyObjectsCommand::save(const nanoem_model_rigid_body_t *rigidBodyPtr, Parameter &parameter)
{
    parameter.m_shapeSize = glm::make_vec3(nanoemModelRigidBodyGetShapeSize(rigidBodyPtr));
    parameter.m_origin = glm::make_vec3(nanoemModelRigidBodyGetOrigin(rigidBodyPtr));
    parameter.m_orientation = glm::make_vec3(nanoemModelRigidBodyGetOrientation(rigidBodyPtr));
    parameter.m_bone = nanoemModelRigidBodyGetBoneObject(rigidBodyPtr);
    parameter.m_shapeType = nanoemModelRigidBodyGetShapeType(rigidBodyPtr);
    parameter.m_transformType = nanoemModelRigidBodyGetTransformType(rigidBodyPtr);
    parameter.m_mass = nanoemModelRigidBodyGetMass(rigidBodyPtr);
    parameter.m_linearDamping = nanoemModelRigidBodyGetLinearDamping(rigidBodyPtr);
    parameter.m_angularDamping = nanoemModelRigidBodyGetAngularDamping(rigidBodyPtr);
    parameter.m_restitution = nanoemModelRigidBodyGetRestitution(rigidBodyPtr);
    parameter.m_friction = nanoemModelRigidBodyGetFriction(rigidBodyPtr);
    parameter.m_collisionGroup = nanoemModelRigidBodyGetCollisionGroupId(rigidBodyPtr);
    parameter.m_collisionMask = nanoemModelRigidBodyGetCollisionMask(rigidBodyPtr);
}

void
BatchChangeAllRigidBodyObjectsCommand::restore(
    const Parameter &parameter, nanoem_mutable_model_rigid_body_t *rigidBodyPtr)
{
    nanoemMutableModelRigidBodySetShapeSize(rigidBodyPtr, glm::value_ptr(Vector4(parameter.m_shapeSize, 0)));
    nanoemMutableModelRigidBodySetOrigin(rigidBodyPtr, glm::value_ptr(Vector4(parameter.m_origin, 0)));
    nanoemMutableModelRigidBodySetOrientation(rigidBodyPtr, glm::value_ptr(Vector4(parameter.m_orientation, 0)));
    nanoemMutableModelRigidBodySetBoneObject(rigidBodyPtr, parameter.m_bone);
    nanoemMutableModelRigidBodySetShapeType(rigidBodyPtr, parameter.m_shapeType);
    nanoemMutableModelRigidBodySetTransformType(rigidBodyPtr, parameter.m_transformType);
    nanoemMutableModelRigidBodySetMass(rigidBodyPtr, parameter.m_mass);
    nanoemMutableModelRigidBodySetLinearDamping(rigidBodyPtr, parameter.m_linearDamping);
    nanoemMutableModelRigidBodySetAngularDamping(rigidBodyPtr, parameter.m_angularDamping);
    nanoemMutableModelRigidBodySetRestitution(rigidBodyPtr, parameter.m_restitution);
    nanoemMutableModelRigidBodySetFriction(rigidBodyPtr, parameter.m_friction);
    nanoemMutableModelRigidBodySetCollisionGroupId(rigidBodyPtr, parameter.m_collisionGroup);
    nanoemMutableModelRigidBodySetCollisionMask(rigidBodyPtr, parameter.m_collisionMask);
}

BatchChangeAllRigidBodyObjectsCommand::BatchChangeAllRigidBodyObjectsCommand(
    Model *activeModel, const List &objects, const Parameter &parameter)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_newParameter(parameter)
{
    for (List::const_iterator it = objects.begin(), end = objects.end(); it != end; ++it) {
        Parameter parameter;
        save(*it, parameter);
        m_objects.insert(tinystl::make_pair(*it, parameter));
    }
}

BatchChangeAllRigidBodyObjectsCommand::~BatchChangeAllRigidBodyObjectsCommand() NANOEM_DECL_NOEXCEPT
{
    m_activeModel = nullptr;
}

void
BatchChangeAllRigidBodyObjectsCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ParameterMap::const_iterator it = m_objects.begin(), end = m_objects.end(); it != end; ++it) {
        ScopedMutableRigidBody rigid_body(it->first);
        restore(it->second, rigid_body);
    }
    assignError(status, error);
}

void
BatchChangeAllRigidBodyObjectsCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ParameterMap::const_iterator it = m_objects.begin(), end = m_objects.end(); it != end; ++it) {
        ScopedMutableRigidBody rigid_body(it->first);
        restore(m_newParameter, rigid_body);
    }
    assignError(status, error);
}

void
BatchChangeAllRigidBodyObjectsCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
BatchChangeAllRigidBodyObjectsCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
BatchChangeAllRigidBodyObjectsCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
BatchChangeAllRigidBodyObjectsCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "BatchChangeAllRigidBodyObjectsCommand";
}

undo_command_t *
BatchChangeAllJointObjectsCommand::create(Model *activeModel, const List &objects, const Parameter &parameter)
{
    BatchChangeAllJointObjectsCommand *command =
        nanoem_new(BatchChangeAllJointObjectsCommand(activeModel, objects, parameter));
    return command->createCommand();
}

void
BatchChangeAllJointObjectsCommand::save(const nanoem_model_joint_t *jointPtr, Parameter &parameter)
{
    parameter.m_origin = glm::make_vec3(nanoemModelJointGetOrigin(jointPtr));
    parameter.m_orientation = glm::make_vec3(nanoemModelJointGetOrientation(jointPtr));
    parameter.m_linearUpperLimit = glm::make_vec3(nanoemModelJointGetLinearUpperLimit(jointPtr));
    parameter.m_linearLowerLimit = glm::make_vec3(nanoemModelJointGetLinearLowerLimit(jointPtr));
    parameter.m_linearStiffness = glm::make_vec3(nanoemModelJointGetLinearStiffness(jointPtr));
    parameter.m_angularUpperLimit = glm::make_vec3(nanoemModelJointGetAngularUpperLimit(jointPtr));
    parameter.m_angularLowerLimit = glm::make_vec3(nanoemModelJointGetAngularLowerLimit(jointPtr));
    parameter.m_angularStiffness = glm::make_vec3(nanoemModelJointGetAngularStiffness(jointPtr));
    parameter.m_bodyA = nanoemModelJointGetRigidBodyAObject(jointPtr);
    parameter.m_bodyB = nanoemModelJointGetRigidBodyBObject(jointPtr);
    parameter.m_type = nanoemModelJointGetType(jointPtr);
}

void
BatchChangeAllJointObjectsCommand::restore(const Parameter &parameter, nanoem_mutable_model_joint_t *jointPtr)
{
    nanoemMutableModelJointSetOrigin(jointPtr, glm::value_ptr(Vector4(parameter.m_origin, 0)));
    nanoemMutableModelJointSetOrientation(jointPtr, glm::value_ptr(Vector4(parameter.m_orientation, 0)));
    nanoemMutableModelJointSetLinearUpperLimit(jointPtr, glm::value_ptr(Vector4(parameter.m_linearUpperLimit, 0)));
    nanoemMutableModelJointSetLinearLowerLimit(jointPtr, glm::value_ptr(Vector4(parameter.m_linearLowerLimit, 0)));
    nanoemMutableModelJointSetLinearStiffness(jointPtr, glm::value_ptr(Vector4(parameter.m_linearStiffness, 0)));
    nanoemMutableModelJointSetAngularUpperLimit(jointPtr, glm::value_ptr(Vector4(parameter.m_angularUpperLimit, 0)));
    nanoemMutableModelJointSetAngularLowerLimit(jointPtr, glm::value_ptr(Vector4(parameter.m_angularLowerLimit, 0)));
    nanoemMutableModelJointSetAngularStiffness(jointPtr, glm::value_ptr(Vector4(parameter.m_angularStiffness, 0)));
    nanoemMutableModelJointSetRigidBodyAObject(jointPtr, parameter.m_bodyA);
    nanoemMutableModelJointSetRigidBodyBObject(jointPtr, parameter.m_bodyB);
    nanoemMutableModelJointSetType(jointPtr, parameter.m_type);
}

BatchChangeAllJointObjectsCommand::BatchChangeAllJointObjectsCommand(
    Model *activeModel, const List &objects, const Parameter &parameter)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_newParameter(parameter)
{
    for (List::const_iterator it = objects.begin(), end = objects.end(); it != end; ++it) {
        Parameter parameter;
        save(*it, parameter);
        m_objects.insert(tinystl::make_pair(*it, parameter));
    }
}

BatchChangeAllJointObjectsCommand::~BatchChangeAllJointObjectsCommand() NANOEM_DECL_NOEXCEPT
{
    m_activeModel = nullptr;
}

void
BatchChangeAllJointObjectsCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ParameterMap::const_iterator it = m_objects.begin(), end = m_objects.end(); it != end; ++it) {
        ScopedMutableJoint joint(it->first);
        restore(it->second, joint);
    }
    assignError(status, error);
}

void
BatchChangeAllJointObjectsCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ParameterMap::const_iterator it = m_objects.begin(), end = m_objects.end(); it != end; ++it) {
        ScopedMutableJoint joint(it->first);
        restore(m_newParameter, joint);
    }
    assignError(status, error);
}

void
BatchChangeAllJointObjectsCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
BatchChangeAllJointObjectsCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
BatchChangeAllJointObjectsCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
BatchChangeAllJointObjectsCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "BatchChangeAllJointObjectsCommand";
}

undo_command_t *
BatchChangeAllSoftBodyObjectsCommand::create(Model *activeModel, const List &objects, const Parameter &parameter)
{
    BatchChangeAllSoftBodyObjectsCommand *command =
        nanoem_new(BatchChangeAllSoftBodyObjectsCommand(activeModel, objects, parameter));
    return command->createCommand();
}

void
BatchChangeAllSoftBodyObjectsCommand::save(const nanoem_model_soft_body_t *softBodyPtr, Parameter &parameter)
{
    BX_UNUSED_2(softBodyPtr, parameter);
}

void
BatchChangeAllSoftBodyObjectsCommand::restore(const Parameter &parameter, nanoem_mutable_model_soft_body_t *softBodyPtr)
{
    BX_UNUSED_2(parameter, softBodyPtr);
}

BatchChangeAllSoftBodyObjectsCommand::BatchChangeAllSoftBodyObjectsCommand(
    Model *activeModel, const List &objects, const Parameter &parameter)
    : BaseUndoCommand(activeModel->project())
    , m_activeModel(activeModel)
    , m_newParameter(parameter)
{
    for (List::const_iterator it = objects.begin(), end = objects.end(); it != end; ++it) {
        Parameter parameter;
        save(*it, parameter);
        m_objects.insert(tinystl::make_pair(*it, parameter));
    }
}

BatchChangeAllSoftBodyObjectsCommand::~BatchChangeAllSoftBodyObjectsCommand() NANOEM_DECL_NOEXCEPT
{
    m_activeModel = nullptr;
}

void
BatchChangeAllSoftBodyObjectsCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ParameterMap::const_iterator it = m_objects.begin(), end = m_objects.end(); it != end; ++it) {
        ScopedMutableSoftBody soft_body(it->first);
        restore(it->second, soft_body);
    }
    assignError(status, error);
}

void
BatchChangeAllSoftBodyObjectsCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    for (ParameterMap::const_iterator it = m_objects.begin(), end = m_objects.end(); it != end; ++it) {
        ScopedMutableSoftBody soft_body(it->first);
        restore(m_newParameter, soft_body);
    }
    assignError(status, error);
}

void
BatchChangeAllSoftBodyObjectsCommand::read(const void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
BatchChangeAllSoftBodyObjectsCommand::write(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

void
BatchChangeAllSoftBodyObjectsCommand::release(void *messagePtr)
{
    BX_UNUSED_1(messagePtr);
}

const char *
BatchChangeAllSoftBodyObjectsCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "BatchChangeAllSoftBodyObjectsCommand";
}

} /* namespace command */
} /* namespace nanoem */
