/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_MODELOBJECTCOMMAND_H_
#define NANOEM_EMAPP_COMMAND_MODELOBJECTCOMMAND_H_

#include "emapp/command/BaseUndoCommand.h"

struct nanoem_mutable_model_t;
struct nanoem_mutable_model_vertex_t;
struct nanoem_mutable_model_material_t;
struct nanoem_mutable_model_bone_t;
struct nanoem_mutable_model_constraint_t;
struct nanoem_mutable_model_constraint_joint_t;
struct nanoem_mutable_model_morph_t;
struct nanoem_mutable_model_morph_bone_t;
struct nanoem_mutable_model_morph_flip_t;
struct nanoem_mutable_model_morph_group_t;
struct nanoem_mutable_model_morph_impulse_t;
struct nanoem_mutable_model_morph_material_t;
struct nanoem_mutable_model_morph_uv_t;
struct nanoem_mutable_model_morph_vertex_t;
struct nanoem_mutable_model_label_t;
struct nanoem_mutable_model_rigid_body_t;
struct nanoem_mutable_model_joint_t;
struct nanoem_mutable_model_soft_body_t;

namespace nanoem {

class Model;

namespace command {

struct ScopedMutableModel {
    ScopedMutableModel(Model *model);
    ~ScopedMutableModel() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_t *m_model;
};

struct ScopedMutableVertex {
    ScopedMutableVertex(Model *model);
    ScopedMutableVertex(nanoem_model_vertex_t *vertexPtr);
    ~ScopedMutableVertex() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_vertex_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_vertex_t *m_vertex;
};

struct ScopedMutableMaterial {
    ScopedMutableMaterial(Model *model);
    ScopedMutableMaterial(nanoem_model_material_t *materialPtr);
    ~ScopedMutableMaterial() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_material_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_material_t *m_material;
};

struct ScopedMutableBone {
    ScopedMutableBone(Model *model);
    ScopedMutableBone(nanoem_model_bone_t *bonePtr);
    ~ScopedMutableBone() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_bone_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_bone_t *m_bone;
};

struct ScopedMutableConstraint {
    ScopedMutableConstraint(Model *model);
    ScopedMutableConstraint(nanoem_model_constraint_t *constraintPtr);
    ~ScopedMutableConstraint() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_constraint_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_constraint_t *m_constraint;
};

struct ScopedMutableConstraintJoint {
    ScopedMutableConstraintJoint(nanoem_model_constraint_joint_t *jointPtr);
    ~ScopedMutableConstraintJoint() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_constraint_joint_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_constraint_joint_t *m_joint;
};

struct ScopedMutableMorph {
    ScopedMutableMorph(Model *model);
    ScopedMutableMorph(nanoem_model_morph_t *morphPtr);
    ~ScopedMutableMorph() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_morph_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_morph_t *m_morph;
};

struct ScopedMutableMorphBone {
    ScopedMutableMorphBone(nanoem_model_morph_bone_t *morphPtr);
    ~ScopedMutableMorphBone() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_morph_bone_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_morph_bone_t *m_morph;
};

struct ScopedMutableMorphFlip {
    ScopedMutableMorphFlip(nanoem_model_morph_flip_t *morphPtr);
    ~ScopedMutableMorphFlip() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_morph_flip_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_morph_flip_t *m_morph;
};

struct ScopedMutableMorphGroup {
    ScopedMutableMorphGroup(nanoem_model_morph_group_t *morphPtr);
    ~ScopedMutableMorphGroup() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_morph_group_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_morph_group_t *m_morph;
};

struct ScopedMutableMorphImpulse {
    ScopedMutableMorphImpulse(nanoem_model_morph_impulse_t *morphPtr);
    ~ScopedMutableMorphImpulse() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_morph_impulse_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_morph_impulse_t *m_morph;
};

struct ScopedMutableMorphMaterial {
    ScopedMutableMorphMaterial(nanoem_model_morph_material_t *morphPtr);
    ~ScopedMutableMorphMaterial() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_morph_material_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_morph_material_t *m_morph;
};

struct ScopedMutableMorphUV {
    ScopedMutableMorphUV(nanoem_model_morph_uv_t *morphPtr);
    ~ScopedMutableMorphUV() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_morph_uv_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_morph_uv_t *m_morph;
};

struct ScopedMutableMorphVertex {
    ScopedMutableMorphVertex(nanoem_model_morph_vertex_t *morphPtr);
    ~ScopedMutableMorphVertex() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_morph_vertex_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_morph_vertex_t *m_morph;
};

struct ScopedMutableLabel {
    ScopedMutableLabel(Model *model);
    ScopedMutableLabel(nanoem_model_label_t *labelPtr);
    ~ScopedMutableLabel() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_label_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_label_t *m_label;
};

struct ScopedMutableRigidBody {
    ScopedMutableRigidBody(Model *model);
    ScopedMutableRigidBody(nanoem_model_rigid_body_t *rigidBodyPtr);
    ~ScopedMutableRigidBody() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_rigid_body_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_rigid_body_t *m_rigidBody;
};

struct ScopedMutableJoint {
    ScopedMutableJoint(Model *model);
    ScopedMutableJoint(nanoem_model_joint_t *jointPtr);
    ~ScopedMutableJoint() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_joint_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_joint_t *m_joint;
};

struct ScopedMutableSoftBody {
    ScopedMutableSoftBody(Model *model);
    ScopedMutableSoftBody(nanoem_model_soft_body_t *softBodyPtr);
    ~ScopedMutableSoftBody() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_soft_body_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_soft_body_t *m_softBody;
};

struct ModelObjectCommand : private NonCopyable {
    virtual ~ModelObjectCommand() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual void execute(Project *project) = 0;
};

struct DeleteMaterialCommand : ModelObjectCommand {
    DeleteMaterialCommand(nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex);
    void execute(Project *project);
    nanoem_model_material_t *const *m_materials;
    nanoem_rsize_t m_materialIndex;
};

struct BaseMoveMaterialCommand : ModelObjectCommand {
    struct LayoutPosition {
        size_t m_offset;
        size_t m_size;
    };
    BaseMoveMaterialCommand(nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex);
    void move(int destination, const LayoutPosition &from, const LayoutPosition &to, Model *activeModel);
    nanoem_model_material_t *const *m_materials;
    nanoem_rsize_t &m_materialIndex;
};

struct MoveMaterialUpCommand : BaseMoveMaterialCommand {
    MoveMaterialUpCommand(nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex);
    void execute(Project *project);
};

struct MoveMaterialDownCommand : BaseMoveMaterialCommand {
    MoveMaterialDownCommand(nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex);
    void execute(Project *project);
};

struct CreateBoneCommand : ModelObjectCommand {
    CreateBoneCommand(nanoem_rsize_t numBones, int offset, const nanoem_model_bone_t *base);
    void execute(Project *project);
    const nanoem_model_bone_t *m_base;
    const nanoem_rsize_t m_numBones;
    const int m_offset;
};

struct DeleteBoneCommand : ModelObjectCommand {
    DeleteBoneCommand(nanoem_model_bone_t *const *bones, nanoem_rsize_t &boneIndex);
    void execute(Project *project);
    nanoem_model_bone_t *const *m_bones;
    nanoem_rsize_t &m_boneIndex;
};

struct MoveBoneDownCommand : ModelObjectCommand {
    MoveBoneDownCommand(nanoem_model_bone_t *const *bones, nanoem_rsize_t &boneIndex);
    void execute(Project *project);
    nanoem_model_bone_t *const *m_bones;
    nanoem_rsize_t &m_boneIndex;
};

struct MoveBoneUpCommand : ModelObjectCommand {
    MoveBoneUpCommand(nanoem_model_bone_t *const *bones, nanoem_rsize_t &boneIndex);
    void execute(Project *project);
    nanoem_model_bone_t *const *m_bones;
    nanoem_rsize_t &m_boneIndex;
};

struct CreateMorphCommand : ModelObjectCommand {
    CreateMorphCommand(
        nanoem_rsize_t numMorphs, int offset, const nanoem_model_morph_t *base, nanoem_model_morph_type_t type);
    void execute(Project *project);
    const nanoem_model_morph_t *m_base;
    const nanoem_model_morph_type_t m_type;
    const nanoem_rsize_t m_numMorphs;
    const int m_offset;
};

struct DeleteMorphCommand : ModelObjectCommand {
    DeleteMorphCommand(nanoem_model_morph_t *const *morphs, nanoem_rsize_t &morphIndex);
    void execute(Project *project);
    nanoem_model_morph_t *const *m_morphs;
    nanoem_rsize_t &m_morphIndex;
};

struct MoveMorphUpCommand : ModelObjectCommand {
    MoveMorphUpCommand(nanoem_model_morph_t *const *morphs, nanoem_rsize_t &morphIndex);
    void execute(Project *project);
    nanoem_model_morph_t *const *m_morphs;
    nanoem_rsize_t &m_morphIndex;
};

struct MoveMorphDownCommand : ModelObjectCommand {
    MoveMorphDownCommand(nanoem_model_morph_t *const *morphs, nanoem_rsize_t &morphIndex);
    void execute(Project *project);
    nanoem_model_morph_t *const *m_morphs;
    nanoem_rsize_t &m_morphIndex;
};

struct CreateLabelCommand : ModelObjectCommand {
    CreateLabelCommand(nanoem_rsize_t numLabels, int offset, const nanoem_model_label_t *base);
    void execute(Project *project);
    const nanoem_model_label_t *m_base;
    const nanoem_rsize_t m_numLabels;
    const int m_offset;
};

struct DeleteLabelCommand : ModelObjectCommand {
    DeleteLabelCommand(nanoem_model_label_t *const *labels, nanoem_rsize_t &labelIndex);
    void execute(Project *project);
    nanoem_model_label_t *const *m_labels;
    nanoem_rsize_t m_labelIndex;
};

struct MoveLabelUpCommand : ModelObjectCommand {
    MoveLabelUpCommand(nanoem_model_label_t *const *labels, nanoem_rsize_t &labelIndex);
    void execute(Project *project);
    nanoem_model_label_t *const *m_labels;
    nanoem_rsize_t &m_labelIndex;
};

struct MoveLabelDownCommand : ModelObjectCommand {
    MoveLabelDownCommand(nanoem_model_label_t *const *labels, nanoem_rsize_t &labelIndex);
    void execute(Project *project);
    nanoem_model_label_t *const *m_labels;
    nanoem_rsize_t &m_labelIndex;
};

struct CreateRigidBodyCommand : ModelObjectCommand {
    CreateRigidBodyCommand(nanoem_rsize_t numRigidBodies, int offset, const nanoem_model_rigid_body_t *base);
    void execute(Project *project);
    const nanoem_model_rigid_body_t *m_base;
    const nanoem_rsize_t m_numRigidBodies;
    const int m_offset;
};

struct DeleteRigidBodyCommand : ModelObjectCommand {
    DeleteRigidBodyCommand(nanoem_model_rigid_body_t *const *rigidBodies, nanoem_rsize_t &rigidBodyIndex);
    void execute(Project *project);
    nanoem_model_rigid_body_t *const *m_rigidBodies;
    nanoem_rsize_t &m_rigidBodyIndex;
};

struct MoveRigidBodyUpCommand : ModelObjectCommand {
    MoveRigidBodyUpCommand(nanoem_model_rigid_body_t *const *rigidBodies, nanoem_rsize_t &rigidBodyIndex);
    void execute(Project *project);
    nanoem_model_rigid_body_t *const *m_rigidBodies;
    nanoem_rsize_t &m_rigidBodyIndex;
};

struct MoveRigidBodyDownCommand : ModelObjectCommand {
    MoveRigidBodyDownCommand(nanoem_model_rigid_body_t *const *rigidBodies, nanoem_rsize_t &rigidBodyIndex);
    void execute(Project *project);
    nanoem_model_rigid_body_t *const *m_rigidBodies;
    nanoem_rsize_t &m_rigidBodyIndex;
};

struct CreateJointCommand : ModelObjectCommand {
    CreateJointCommand(nanoem_rsize_t numJoints, int offset, const nanoem_model_joint_t *base);
    void execute(Project *project);
    const nanoem_model_joint_t *m_base;
    const nanoem_rsize_t m_numJoints;
    const int m_offset;
};

struct DeleteJointCommand : ModelObjectCommand {
    DeleteJointCommand(nanoem_model_joint_t *const *joints, nanoem_rsize_t &jointIndex);
    void execute(Project *project);
    nanoem_model_joint_t *const *m_joints;
    nanoem_rsize_t &m_jointIndex;
};

struct MoveJointUpCommand : ModelObjectCommand {
    MoveJointUpCommand(nanoem_model_joint_t *const *joints, nanoem_rsize_t &jointIndex);
    void execute(Project *project);
    nanoem_model_joint_t *const *m_joints;
    nanoem_rsize_t &m_jointIndex;
};

struct MoveJointDownCommand : ModelObjectCommand {
    MoveJointDownCommand(nanoem_model_joint_t *const *joints, nanoem_rsize_t &jointIndex);
    void execute(Project *project);
    nanoem_model_joint_t *const *m_joints;
    nanoem_rsize_t &m_jointIndex;
};

struct CreateSoftBodyCommand : ModelObjectCommand {
    CreateSoftBodyCommand(nanoem_rsize_t numSoftBodys, int offset, const nanoem_model_soft_body_t *base);
    void execute(Project *project);
    const nanoem_model_soft_body_t *m_base;
    const nanoem_rsize_t m_numSoftBodys;
    const int m_offset;
};

struct DeleteSoftBodyCommand : ModelObjectCommand {
    DeleteSoftBodyCommand(nanoem_model_soft_body_t *const *soft_bodys, nanoem_rsize_t &soft_bodyIndex);
    void execute(Project *project);
    nanoem_model_soft_body_t *const *m_softBodies;
    nanoem_rsize_t &m_softBodyIndex;
};

struct MoveSoftBodyUpCommand : ModelObjectCommand {
    MoveSoftBodyUpCommand(nanoem_model_soft_body_t *const *soft_bodys, nanoem_rsize_t &soft_bodyIndex);
    void execute(Project *project);
    nanoem_model_soft_body_t *const *m_softBodies;
    nanoem_rsize_t &m_softBodyIndex;
};

struct MoveSoftBodyDownCommand : ModelObjectCommand {
    MoveSoftBodyDownCommand(nanoem_model_soft_body_t *const *soft_bodys, nanoem_rsize_t &soft_bodyIndex);
    void execute(Project *project);
    nanoem_model_soft_body_t *const *m_softBodies;
    nanoem_rsize_t &m_softBodyIndex;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_IUNDOCOMMAND_H_ */
