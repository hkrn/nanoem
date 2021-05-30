/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_MODELOBJECTCOMMAND_H_
#define NANOEM_EMAPP_COMMAND_MODELOBJECTCOMMAND_H_

#include "emapp/command/BaseUndoCommand.h"
#include "emapp/model/BindPose.h"

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
struct nanoem_mutable_model_label_item_t;
struct nanoem_mutable_model_rigid_body_t;
struct nanoem_mutable_model_joint_t;
struct nanoem_mutable_model_soft_body_t;

namespace nanoem {

class Model;

namespace command {

struct ScopedMutableModel {
    ScopedMutableModel(Model *model, nanoem_status_t *status);
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
    ScopedMutableMorphBone(nanoem_mutable_model_morph_t *morphPtr);
    ScopedMutableMorphBone(nanoem_model_morph_bone_t *morphPtr);
    ~ScopedMutableMorphBone() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_morph_bone_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_morph_bone_t *m_morph;
};

struct ScopedMutableMorphFlip {
    ScopedMutableMorphFlip(nanoem_mutable_model_morph_t *morphPtr);
    ScopedMutableMorphFlip(nanoem_model_morph_flip_t *morphPtr);
    ~ScopedMutableMorphFlip() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_morph_flip_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_morph_flip_t *m_morph;
};

struct ScopedMutableMorphGroup {
    ScopedMutableMorphGroup(nanoem_mutable_model_morph_t *morphPtr);
    ScopedMutableMorphGroup(nanoem_model_morph_group_t *morphPtr);
    ~ScopedMutableMorphGroup() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_morph_group_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_morph_group_t *m_morph;
};

struct ScopedMutableMorphImpulse {
    ScopedMutableMorphImpulse(nanoem_mutable_model_morph_t *morphPtr);
    ScopedMutableMorphImpulse(nanoem_model_morph_impulse_t *morphPtr);
    ~ScopedMutableMorphImpulse() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_morph_impulse_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_morph_impulse_t *m_morph;
};

struct ScopedMutableMorphMaterial {
    ScopedMutableMorphMaterial(nanoem_mutable_model_morph_t *morphPtr);
    ScopedMutableMorphMaterial(nanoem_model_morph_material_t *morphPtr);
    ~ScopedMutableMorphMaterial() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_morph_material_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_morph_material_t *m_morph;
};

struct ScopedMutableMorphUV {
    ScopedMutableMorphUV(nanoem_mutable_model_morph_t *morphPtr);
    ScopedMutableMorphUV(nanoem_model_morph_uv_t *morphPtr);
    ~ScopedMutableMorphUV() NANOEM_DECL_NOEXCEPT;
    operator nanoem_mutable_model_morph_uv_t *() NANOEM_DECL_NOEXCEPT;
    nanoem_mutable_model_morph_uv_t *m_morph;
};

struct ScopedMutableMorphVertex {
    ScopedMutableMorphVertex(nanoem_mutable_model_morph_t *morphPtr);
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

class DeletingMaterialState : private NonCopyable {
public:
    ~DeletingMaterialState() NANOEM_DECL_NOEXCEPT;

    void clear() NANOEM_DECL_NOEXCEPT;
    void save(const Model *model, const nanoem_model_material_t *materialPtr, nanoem_status_t *status);
    void restore(const nanoem_model_material_t *materialPtr);

private:
    typedef tinystl::vector<nanoem_mutable_model_morph_material_t *, TinySTLAllocator> MaterialMorphList;
    typedef tinystl::vector<nanoem_mutable_model_soft_body_t *, TinySTLAllocator> SoftBodyList;
    MaterialMorphList m_materialMorphs;
    SoftBodyList m_softBodies;
};

class DeletingBoneState : private NonCopyable {
public:
    ~DeletingBoneState() NANOEM_DECL_NOEXCEPT;

    void clear() NANOEM_DECL_NOEXCEPT;
    void save(const Model *model, const nanoem_model_bone_t *bone, nanoem_status_t *status);
    void restore(const nanoem_model_bone_t *bonePtr);
    void setParentBone(const nanoem_model_bone_t *bonePtr);

private:
    typedef tinystl::pair<nanoem_mutable_model_vertex_t *, nanoem_rsize_t> VertexPair;
    typedef tinystl::vector<VertexPair, TinySTLAllocator> VertexList;
    typedef tinystl::vector<nanoem_mutable_model_bone_t *, TinySTLAllocator> BoneList;
    typedef tinystl::vector<nanoem_mutable_model_constraint_t *, TinySTLAllocator> ConstraintList;
    typedef tinystl::vector<nanoem_mutable_model_constraint_joint_t *, TinySTLAllocator> ConstraintJointList;
    typedef tinystl::vector<nanoem_mutable_model_morph_bone_t *, TinySTLAllocator> BoneMorphList;
    typedef tinystl::vector<nanoem_mutable_model_rigid_body_t *, TinySTLAllocator> RigidBodyList;

    VertexList m_vertices;
    BoneList m_parentBones;
    BoneList m_inherentParentBones;
    BoneList m_targetBones;
    ConstraintList m_constraints;
    ConstraintJointList m_constraintJoints;
    BoneMorphList m_boneMorphs;
    RigidBodyList m_rigidBodies;
};

class DeletingMorphState : private NonCopyable {
public:
    ~DeletingMorphState() NANOEM_DECL_NOEXCEPT;

    void clear() NANOEM_DECL_NOEXCEPT;
    void save(const Model *model, const nanoem_model_morph_t *morphPtr, nanoem_status_t *status);
    void restore(const nanoem_model_morph_t *morphPtr);

private:
    typedef tinystl::vector<nanoem_mutable_model_morph_flip_t *, TinySTLAllocator> FlipMorphList;
    typedef tinystl::vector<nanoem_mutable_model_morph_group_t *, TinySTLAllocator> GroupMorphList;
    FlipMorphList m_flipMorphs;
    GroupMorphList m_groupMorphs;
};

class DeletingRigidBodyState : private NonCopyable {
public:
    ~DeletingRigidBodyState() NANOEM_DECL_NOEXCEPT;

    void clear() NANOEM_DECL_NOEXCEPT;
    void save(const Model *model, const nanoem_model_rigid_body_t *rigidBodyPtr, nanoem_status_t *status);
    void restore(const nanoem_model_rigid_body_t *rigidBodyPtr);

private:
    typedef tinystl::vector<nanoem_mutable_model_morph_impulse_t *, TinySTLAllocator> ImpulseMorphList;
    typedef tinystl::vector<nanoem_mutable_model_joint_t *, TinySTLAllocator> JointList;
    ImpulseMorphList m_impulseMorphs;
    JointList m_jointsA;
    JointList m_jointsB;
};

class CreateMaterialCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    typedef tinystl::vector<nanoem_mutable_model_vertex_t *, TinySTLAllocator> MutableVertexList;

    static undo_command_t *create(
        Model *activeModel, const String &name, const MutableVertexList &vertices, const VertexIndexList &vertexIndices);

    CreateMaterialCommand(Model *activeModel, const String &name, const MutableVertexList &vertices,
        const VertexIndexList &vertexIndices);
    ~CreateMaterialCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    Model *m_activeModel;
    ScopedMutableMaterial m_creatingMaterial;
    MutableVertexList m_deletingVertices;
    VertexIndexList m_vertexIndices;
};

class CopyMaterialFromModelCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(
        Model *activeModel, const Model *baseModel, const nanoem_model_material_t *baseMaterialPtr);

    CopyMaterialFromModelCommand(
        Model *activeModel, const Model *baseModel, const nanoem_model_material_t *baseMaterialPtr);
    ~CopyMaterialFromModelCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    Model *m_activeModel;
    ScopedMutableMaterial m_creatingMaterial;
    VertexIndexList m_vertexIndices;
};

class MergeMaterialCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(
        Model *activeModel, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex);

    MergeMaterialCommand(Model *activeModel, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex);
    ~MergeMaterialCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    Model *m_activeModel;
    ScopedMutableMaterial m_baseMaterial;
    ScopedMutableMaterial m_deletingMaterial;
    nanoem_rsize_t m_materialIndex;
};

class DeleteMaterialCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(
        Model *activeModel, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex);

    DeleteMaterialCommand(Model *activeModel, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex);
    ~DeleteMaterialCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    Model *m_activeModel;
    ScopedMutableMaterial m_deletingMaterial;
    VertexIndexList m_deletingVertexIndices;
    DeletingMaterialState m_deletingMaterialState;
    nanoem_rsize_t m_deletingVertexIndexOffset;
    nanoem_rsize_t m_materialIndex;
};

class BaseMoveMaterialCommand : public BaseUndoCommand {
public:
    BaseMoveMaterialCommand(Model *activeModel, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex);
    ~BaseMoveMaterialCommand() NANOEM_DECL_NOEXCEPT;

protected:
    struct LayoutPosition {
        size_t m_offset;
        size_t m_size;
    };
    static void move(ScopedMutableMaterial &material, int destination, const LayoutPosition &from,
        const LayoutPosition &to, Model *activeModel, nanoem_status_t *status);

    void moveUp(nanoem_status_t *status);
    void moveDown(nanoem_status_t *status);

    Model *m_activeModel;
    ScopedMutableMaterial m_movingMaterial;
};

class MoveMaterialUpCommand NANOEM_DECL_SEALED : public BaseMoveMaterialCommand {
public:
    static undo_command_t *create(
        Model *activeModel, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex);

    MoveMaterialUpCommand(Model *activeModel, nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex);
    ~MoveMaterialUpCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class MoveMaterialDownCommand NANOEM_DECL_SEALED : public BaseMoveMaterialCommand {
public:
    static undo_command_t *create(
        Model *activeModel, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex);

    MoveMaterialDownCommand(Model *activeModel, nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex);
    ~MoveMaterialDownCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class CreateBoneCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(Model *activeModel, const Vector3 &origin);
    static undo_command_t *create(Model *activeModel, int offset, const nanoem_model_bone_t *base);
    static void setup(nanoem_model_bone_t *bonePtr, Project *project);
    static void setNameSuffix(nanoem_mutable_model_bone_t *bone, const char *suffix,
        nanoem_unicode_string_factory_t *factory, nanoem_status_t *status);
    static void setNameSuffix(nanoem_mutable_model_bone_t *bone, const char *suffix, nanoem_language_type_t language,
        nanoem_unicode_string_factory_t *factory, nanoem_status_t *status);

    CreateBoneCommand(Model *activeModel, int offset, const nanoem_model_bone_t *base, const Vector3 *origin);
    ~CreateBoneCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const nanoem_model_bone_t *m_base;
    const int m_offset;
    Model *m_activeModel;
    ScopedMutableBone m_creatingBone;
};

class CreateBoneAsDestinationCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(Model *activeModel, const nanoem_model_bone_t *base);

    CreateBoneAsDestinationCommand(Model *activeModel, const nanoem_model_bone_t *base);
    ~CreateBoneAsDestinationCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const nanoem_model_bone_t *m_parent;
    Model *m_activeModel;
    ScopedMutableBone m_creatingBone;
    int m_boneIndex;
};

class CreateBoneAsStagingParentCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(Model *activeModel, const nanoem_model_bone_t *base);

    CreateBoneAsStagingParentCommand(Model *activeModel, const nanoem_model_bone_t *base);
    ~CreateBoneAsStagingParentCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    typedef tinystl::vector<nanoem_mutable_model_bone_t *, TinySTLAllocator> BoneList;
    const nanoem_model_bone_t *m_parent;
    Model *m_activeModel;
    BoneList m_baseBoneChildren;
    ScopedMutableBone m_creatingBone;
    int m_boneIndex;
};

class CreateBoneAsStagingChildCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_model_bone_t *base);

    CreateBoneAsStagingChildCommand(Model *activeModel, nanoem_model_bone_t *base);
    ~CreateBoneAsStagingChildCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    typedef tinystl::vector<nanoem_mutable_model_bone_t *, TinySTLAllocator> BoneList;
    const nanoem_model_bone_t *m_parent;
    Model *m_activeModel;
    ScopedMutableBone m_creatingBone;
    ScopedMutableBone m_baseBone;
    int m_boneIndex;
};

class AddBoneCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_mutable_model_bone_t *bone);

    AddBoneCommand(Model *activeModel, nanoem_mutable_model_bone_t *base);
    ~AddBoneCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    Model *m_activeModel;
    nanoem_mutable_model_bone_t *m_addingBone;
};

class AddBoneToConstraintCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_model_constraint_t *constraintPtr);

    AddBoneToConstraintCommand(Model *activeModel, nanoem_model_constraint_t *constraintPtr);
    ~AddBoneToConstraintCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    typedef tinystl::vector<nanoem_mutable_model_constraint_joint_t *, TinySTLAllocator> ConstraintJointList;
    Model *m_activeModel;
    ScopedMutableConstraint m_mutableConstraint;
    ConstraintJointList m_joints;
};

class DeleteBoneCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_rsize_t boneIndex);

    DeleteBoneCommand(Model *activeModel, nanoem_rsize_t boneIndex);
    ~DeleteBoneCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const nanoem_model_bone_t *m_parentBonePtr;
    const nanoem_rsize_t m_boneIndex;
    Model *m_activeModel;
    nanoem_mutable_model_bone_t *m_deletingBone;
    DeletingBoneState m_deletingBoneState;
};

class BaseMoveBoneCommand : public BaseUndoCommand {
public:
    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;

protected:
    BaseMoveBoneCommand(Model *activeModel, nanoem_rsize_t fromBoneIndex, nanoem_rsize_t toBoneIndex);

private:
    void move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error);

    const nanoem_rsize_t m_fromBoneIndex;
    const nanoem_rsize_t m_toBoneIndex;
    Model *m_activeModel;
};

class MoveBoneDownCommand NANOEM_DECL_SEALED : public BaseMoveBoneCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_rsize_t boneIndex);

    MoveBoneDownCommand(Model *activeModel, nanoem_rsize_t boneIndex);
    ~MoveBoneDownCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class MoveBoneUpCommand NANOEM_DECL_SEALED : public BaseMoveBoneCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_rsize_t boneIndex);

    MoveBoneUpCommand(Model *activeModel, nanoem_rsize_t boneIndex);
    ~MoveBoneUpCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class CreateMorphCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(
        Model *activeModel, int offset, const nanoem_model_morph_t *base, nanoem_model_morph_type_t type);
    static void setup(nanoem_model_morph_t *morphPtr, Project *project);

    CreateMorphCommand(Model *activeModel, int offset, const nanoem_model_morph_t *base, nanoem_model_morph_type_t type);
    ~CreateMorphCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const nanoem_model_morph_t *m_base;
    const nanoem_model_morph_type_t m_type;
    const int m_offset;
    Model *m_activeModel;
    ScopedMutableMorph m_creatingMorph;
};

class CreateBoneMorphFromPoseCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(
        Model *activeModel, const model::BindPose::BoneTransformMap &transforms, const String &filename);

    CreateBoneMorphFromPoseCommand(
        Model *activeModel, const model::BindPose::BoneTransformMap &transforms, const String &filename);
    ~CreateBoneMorphFromPoseCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    Model *m_activeModel;
    ScopedMutableMorph m_creatingMorph;
};

class CreateVertexMorphFromModelCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(Model *activeModel, const nanoem_model_t *modelPtr, const String &filename);

    CreateVertexMorphFromModelCommand(Model *activeModel, const nanoem_model_t *modelPtr, const String &filename);
    ~CreateVertexMorphFromModelCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    Model *m_activeModel;
    ScopedMutableMorph m_creatingMorph;
};

class BaseAddToMorphCommand : public BaseUndoCommand {
public:
    BaseAddToMorphCommand(Model *activeModel, nanoem_model_morph_t *morphPtr);

protected:
    Model *m_activeModel;
    ScopedMutableMorph m_mutableMorph;
};

class AddVertexToMorphCommand NANOEM_DECL_SEALED : public BaseAddToMorphCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_model_morph_t *morphPtr);

    AddVertexToMorphCommand(Model *activeModel, nanoem_model_morph_t *morphPtr);
    ~AddVertexToMorphCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    typedef tinystl::vector<nanoem_mutable_model_morph_vertex_t *, TinySTLAllocator> VertexList;
    VertexList m_vertices;
};

class AddMaterialToMorphCommand NANOEM_DECL_SEALED : public BaseAddToMorphCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_model_morph_t *morphPtr);

    AddMaterialToMorphCommand(Model *activeModel, nanoem_model_morph_t *morphPtr);
    ~AddMaterialToMorphCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    typedef tinystl::vector<nanoem_mutable_model_morph_material_t *, TinySTLAllocator> MaterialList;
    MaterialList m_materials;
};

class AddBoneToMorphCommand NANOEM_DECL_SEALED : public BaseAddToMorphCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_model_morph_t *morphPtr);

    AddBoneToMorphCommand(Model *activeModel, nanoem_model_morph_t *morphPtr);
    ~AddBoneToMorphCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    typedef tinystl::vector<nanoem_mutable_model_morph_bone_t *, TinySTLAllocator> BoneList;
    BoneList m_bones;
};

class AddGroupToMorphCommand NANOEM_DECL_SEALED : public BaseAddToMorphCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_model_morph_t *morphPtr);

    AddGroupToMorphCommand(Model *activeModel, nanoem_model_morph_t *morphPtr);
    ~AddGroupToMorphCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    typedef tinystl::vector<nanoem_mutable_model_morph_group_t *, TinySTLAllocator> GroupList;
    GroupList m_groups;
};

class AddFlipToMorphCommand NANOEM_DECL_SEALED : public BaseAddToMorphCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_model_morph_t *morphPtr);

    AddFlipToMorphCommand(Model *activeModel, nanoem_model_morph_t *morphPtr);
    ~AddFlipToMorphCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    typedef tinystl::vector<nanoem_mutable_model_morph_flip_t *, TinySTLAllocator> FlipList;
    FlipList m_flips;
};

class AddRigidBodyToMorphCommand NANOEM_DECL_SEALED : public BaseAddToMorphCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_model_morph_t *morphPtr);

    AddRigidBodyToMorphCommand(Model *activeModel, nanoem_model_morph_t *morphPtr);
    ~AddRigidBodyToMorphCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    typedef tinystl::vector<nanoem_mutable_model_morph_impulse_t *, TinySTLAllocator> RigidBodyList;
    RigidBodyList m_rigidBodies;
};

class DeleteMorphCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_rsize_t morphIndex);

    DeleteMorphCommand(Model *activeModel, nanoem_rsize_t morphIndex);
    ~DeleteMorphCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const nanoem_rsize_t m_morphIndex;
    Model *m_activeModel;
    nanoem_mutable_model_morph_t *m_deletingMorph;
    DeletingMorphState m_deletingMorphState;
};

class BaseMoveMorphCommand : public BaseUndoCommand {
public:
    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;

protected:
    BaseMoveMorphCommand(Model *activeModel, nanoem_rsize_t fromMorphIndex, nanoem_rsize_t toMorphIndex);

private:
    void move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error);

    const nanoem_rsize_t m_fromMorphIndex;
    const nanoem_rsize_t m_toMorphIndex;
    Model *m_activeModel;
};

class MoveMorphUpCommand NANOEM_DECL_SEALED : public BaseMoveMorphCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_rsize_t morphIndex);

    MoveMorphUpCommand(Model *activeModel, nanoem_rsize_t morphIndex);
    ~MoveMorphUpCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class MoveMorphDownCommand NANOEM_DECL_SEALED : public BaseMoveMorphCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_rsize_t morphIndex);

    MoveMorphDownCommand(Model *activeModel, nanoem_rsize_t morphIndex);
    ~MoveMorphDownCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class CreateLabelCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(Model *activeModel, int offset, const nanoem_model_label_t *base);

    CreateLabelCommand(Model *activeModel, int offset, const nanoem_model_label_t *base);
    ~CreateLabelCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const nanoem_model_label_t *m_base;
    const int m_offset;
    Model *m_activeModel;
    ScopedMutableLabel m_creatingLabel;
};

class BaseAddToLabelCommand : public BaseUndoCommand {
public:
    BaseAddToLabelCommand(Model *activeModel, nanoem_model_label_t *labelPtr);

protected:
    typedef tinystl::vector<nanoem_mutable_model_label_item_t *, TinySTLAllocator> ItemList;

    Model *m_activeModel;
    ItemList m_items;
    ScopedMutableLabel m_mutableLabel;
};

class AddBoneToLabelCommand NANOEM_DECL_SEALED : public BaseAddToLabelCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_model_label_t *labelPtr);

    AddBoneToLabelCommand(Model *activeModel, nanoem_model_label_t *labelPtr);
    ~AddBoneToLabelCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class AddMorphToLabelCommand NANOEM_DECL_SEALED : public BaseAddToLabelCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_model_label_t *labelPtr);

    AddMorphToLabelCommand(Model *activeModel, nanoem_model_label_t *labelPtr);
    ~AddMorphToLabelCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class DeleteLabelCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_rsize_t labelIndex);

    DeleteLabelCommand(Model *activeModel, nanoem_rsize_t labelIndex);
    ~DeleteLabelCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const nanoem_rsize_t m_labelIndex;
    Model *m_activeModel;
    nanoem_mutable_model_label_t *m_deletingLabel;
};

class BaseMoveLabelCommand : public BaseUndoCommand {
public:
    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;

protected:
    BaseMoveLabelCommand(Model *activeModel, nanoem_rsize_t fromLabelIndex, nanoem_rsize_t toLabelIndex);

private:
    void move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error);

    const nanoem_rsize_t m_fromLabelIndex;
    const nanoem_rsize_t m_toLabelIndex;
    Model *m_activeModel;
};

class MoveLabelUpCommand NANOEM_DECL_SEALED : public BaseMoveLabelCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_rsize_t labelIndex);

    MoveLabelUpCommand(Model *activeModel, nanoem_rsize_t labelIndex);
    ~MoveLabelUpCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class MoveLabelDownCommand NANOEM_DECL_SEALED : public BaseMoveLabelCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_rsize_t labelIndex);

    MoveLabelDownCommand(Model *activeModel, nanoem_rsize_t labelIndex);
    ~MoveLabelDownCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class CreateRigidBodyCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(Model *activeModel, int offset, const nanoem_model_rigid_body_t *base);
    static void setup(nanoem_model_rigid_body_t *rigidBodyPtr, Project *project);

    CreateRigidBodyCommand(Model *activeModel, int offset, const nanoem_model_rigid_body_t *base);
    ~CreateRigidBodyCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const nanoem_model_rigid_body_t *m_base;
    const int m_offset;
    Model *m_activeModel;
    ScopedMutableRigidBody m_creatingRigidBody;
};

class DeleteRigidBodyCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_rsize_t rigidBodyIndex);

    DeleteRigidBodyCommand(Model *activeModel, nanoem_rsize_t rigidBodyIndex);
    ~DeleteRigidBodyCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const nanoem_rsize_t m_rigidBodyIndex;
    Model *m_activeModel;
    nanoem_mutable_model_rigid_body_t *m_deletingRigidBody;
    DeletingRigidBodyState m_deletingRigidBodyState;
};

class BaseMoveRigidBodyCommand : public BaseUndoCommand {
public:
    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;

protected:
    BaseMoveRigidBodyCommand(Model *activeModel, nanoem_rsize_t fromRigidBodyIndex, nanoem_rsize_t toRigidBodyIndex);

private:
    void move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error);

    const nanoem_rsize_t m_fromRigidBodyIndex;
    const nanoem_rsize_t m_toRigidBodyIndex;
    Model *m_activeModel;
};

class MoveRigidBodyUpCommand NANOEM_DECL_SEALED : public BaseMoveRigidBodyCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_rsize_t rigidBodyIndex);

    MoveRigidBodyUpCommand(Model *activeModel, nanoem_rsize_t rigidBodyIndex);
    ~MoveRigidBodyUpCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class MoveRigidBodyDownCommand NANOEM_DECL_SEALED : public BaseMoveRigidBodyCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_rsize_t rigidBodyIndex);

    MoveRigidBodyDownCommand(Model *activeModel, nanoem_rsize_t rigidBodyIndex);
    ~MoveRigidBodyDownCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class CreateJointCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(Model *activeModel, int offset, const nanoem_model_joint_t *base);
    static void setup(nanoem_model_joint_t *jointPtr, Project *project);

    CreateJointCommand(Model *activeModel, int offset, const nanoem_model_joint_t *base);
    ~CreateJointCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const nanoem_model_joint_t *m_base;
    const int m_offset;
    Model *m_activeModel;
    ScopedMutableJoint m_creatingJoint;
};

class CreateIntermediateJointFromTwoRigidBodiesCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(Model *activeModel);

    CreateIntermediateJointFromTwoRigidBodiesCommand(Model *activeModel);
    ~CreateIntermediateJointFromTwoRigidBodiesCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    Model *m_activeModel;
    ScopedMutableJoint m_creatingJoint;
};

class DeleteJointCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_rsize_t jointIndex);

    DeleteJointCommand(Model *activeModel, nanoem_rsize_t jointIndex);
    ~DeleteJointCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const nanoem_rsize_t m_jointIndex;
    Model *m_activeModel;
    nanoem_mutable_model_joint_t *m_deletingJoint;
};

class BaseMoveJointCommand : public BaseUndoCommand {
public:
    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;

protected:
    BaseMoveJointCommand(Model *activeModel, nanoem_rsize_t fromJointIndex, nanoem_rsize_t toJointIndex);

private:
    void move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error);

    const nanoem_rsize_t m_fromJointIndex;
    const nanoem_rsize_t m_toJointIndex;
    Model *m_activeModel;
};

class MoveJointUpCommand NANOEM_DECL_SEALED : public BaseMoveJointCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_rsize_t jointIndex);

    MoveJointUpCommand(Model *activeModel, nanoem_rsize_t jointIndex);
    ~MoveJointUpCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class MoveJointDownCommand NANOEM_DECL_SEALED : public BaseMoveJointCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_rsize_t jointIndex);

    MoveJointDownCommand(Model *activeModel, nanoem_rsize_t jointIndex);
    ~MoveJointDownCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class CreateSoftBodyCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(Model *activeModel, int offset, const nanoem_model_soft_body_t *base);

    CreateSoftBodyCommand(Model *activeModel, int offset, const nanoem_model_soft_body_t *base);
    ~CreateSoftBodyCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const nanoem_model_soft_body_t *m_base;
    const int m_offset;
    Model *m_activeModel;
    ScopedMutableSoftBody m_creatingSoftBody;
};

class DeleteSoftBodyCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_rsize_t softBodyIndex);

    DeleteSoftBodyCommand(Model *activeModel, nanoem_rsize_t softBodyIndex);
    ~DeleteSoftBodyCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const nanoem_rsize_t m_softBodyIndex;
    Model *m_activeModel;
    nanoem_mutable_model_soft_body_t *m_deletingSoftBody;
};

class BaseMoveSoftBodyCommand : public BaseUndoCommand {
public:
    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;

protected:
    BaseMoveSoftBodyCommand(Model *activeModel, nanoem_rsize_t fromSoftBodyIndex, nanoem_rsize_t toSoftBodyIndex);

private:
    void move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error);

    const nanoem_rsize_t m_fromSoftBodyIndex;
    const nanoem_rsize_t m_toSoftBodyIndex;
    Model *m_activeModel;
};

class MoveSoftBodyUpCommand NANOEM_DECL_SEALED : public BaseMoveSoftBodyCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_rsize_t softBodyIndex);

    MoveSoftBodyUpCommand(Model *activeModel, nanoem_rsize_t softBodyIndex);
    ~MoveSoftBodyUpCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class MoveSoftBodyDownCommand NANOEM_DECL_SEALED : public BaseMoveSoftBodyCommand {
public:
    static undo_command_t *create(Model *activeModel, nanoem_rsize_t softBodyIndex);

    MoveSoftBodyDownCommand(Model *activeModel, nanoem_rsize_t softBodyIndex);
    ~MoveSoftBodyDownCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_MODELOBJECTCOMMAND_H_ */
