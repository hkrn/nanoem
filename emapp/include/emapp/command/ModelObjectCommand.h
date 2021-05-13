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

class DeleteMaterialCommand : public BaseUndoCommand {
public:
    static undo_command_t *create(
        Project *project, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex);

    DeleteMaterialCommand(Project *project, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex);
    ~DeleteMaterialCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    nanoem_model_material_t *const *m_materials;
    nanoem_rsize_t m_materialIndex;
};

class BaseMoveMaterialCommand : public BaseUndoCommand {
public:
    struct LayoutPosition {
        size_t m_offset;
        size_t m_size;
    };
    BaseMoveMaterialCommand(Project *project, nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex);
    ~BaseMoveMaterialCommand() NANOEM_DECL_NOEXCEPT;

protected:
    void move(int destination, const LayoutPosition &from, const LayoutPosition &to, Model *activeModel, nanoem_status_t *status);

    nanoem_model_material_t *const *m_materials;
    nanoem_rsize_t &m_materialIndex;
};

class MoveMaterialUpCommand : public BaseMoveMaterialCommand {
public:
    static undo_command_t *create(
        Project *project, nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex);

    MoveMaterialUpCommand(Project *project, nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex);
    ~MoveMaterialUpCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class MoveMaterialDownCommand : public BaseMoveMaterialCommand {
public:
    static undo_command_t *create(
        Project *project, nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex);

    MoveMaterialDownCommand(Project *project, nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex);
    ~MoveMaterialDownCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class CreateBoneCommand : public BaseUndoCommand {
public:
    static undo_command_t *create(
        Project *project, nanoem_rsize_t numBones, int offset, const nanoem_model_bone_t *base);
    static void setup(nanoem_model_bone_t *bonePtr, Project *project);

    CreateBoneCommand(Project *project, nanoem_rsize_t numBones, int offset, const nanoem_model_bone_t *base);
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
    ScopedMutableBone m_mutableBone;
};

class CreateBoneAsStagingParentCommand : public BaseUndoCommand {
public:
    static undo_command_t *create(
        Project *project, const nanoem_model_bone_t *base);
    static void setNameSuffix(nanoem_mutable_model_bone_t *bone, const char *suffix, nanoem_unicode_string_factory_t *factory, nanoem_status_t *status);
    static void setNameSuffix(nanoem_mutable_model_bone_t *bone, const char *suffix, nanoem_language_type_t language, nanoem_unicode_string_factory_t *factory, nanoem_status_t *status);

    CreateBoneAsStagingParentCommand(Project *project, const nanoem_model_bone_t *base);
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
    BoneList m_bones;
    ScopedMutableBone m_mutableBone;
    int m_boneIndex;
};

class DeleteBoneCommand : public BaseUndoCommand {
public:
    static undo_command_t *create(Project *project, nanoem_rsize_t boneIndex);

    DeleteBoneCommand(Project *project, nanoem_rsize_t &boneIndex);
    ~DeleteBoneCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const nanoem_rsize_t m_boneIndex;
    Model *m_activeModel;
    nanoem_mutable_model_bone_t *m_mutableBone;
};

class BaseMoveBoneCommand : public BaseUndoCommand {
public:
    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;

protected:
    BaseMoveBoneCommand(Project *project, nanoem_rsize_t fromBoneIndex, nanoem_rsize_t toBoneIndex);

private:
    void move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error);

    const nanoem_rsize_t m_fromBoneIndex;
    const nanoem_rsize_t m_toBoneIndex;
    Model *m_activeModel;
};

class MoveBoneDownCommand : public BaseMoveBoneCommand {
public:
    static undo_command_t *create(Project *project, nanoem_rsize_t boneIndex);

    MoveBoneDownCommand(Project *project, nanoem_rsize_t boneIndex);
    ~MoveBoneDownCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class MoveBoneUpCommand : public BaseMoveBoneCommand {
public:
    static undo_command_t *create(Project *project, nanoem_rsize_t boneIndex);

    MoveBoneUpCommand(Project *project, nanoem_rsize_t boneIndex);
    ~MoveBoneUpCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class CreateMorphCommand : public BaseUndoCommand {
public:
    static undo_command_t *create(Project *project, nanoem_rsize_t numMorphs, int offset,
        const nanoem_model_morph_t *base, nanoem_model_morph_type_t type);

    CreateMorphCommand(Project *project, nanoem_rsize_t numMorphs, int offset, const nanoem_model_morph_t *base,
        nanoem_model_morph_type_t type);
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
    ScopedMutableMorph m_mutableMorph;
};

class CreateBoneMorphFromPoseCommand : public BaseUndoCommand {
public:
    static undo_command_t *create(Project *project, const model::BindPose::BoneTransformMap &transforms, const model::BindPose::MorphWeightMap &weights, const String &filename);

    CreateBoneMorphFromPoseCommand(Project *project, const model::BindPose::BoneTransformMap &transforms, const model::BindPose::MorphWeightMap &weights, const String &filename);
    ~CreateBoneMorphFromPoseCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    Model *m_activeModel;
    ScopedMutableMorph m_mutableMorph;
};

class DeleteMorphCommand : public BaseUndoCommand {
public:
    static undo_command_t *create(Project *project, nanoem_rsize_t morphIndex);

    DeleteMorphCommand(Project *project, nanoem_rsize_t morphIndex);
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
    nanoem_mutable_model_morph_t *m_mutableMorph;
};

class BaseMoveMorphCommand : public BaseUndoCommand {
public:
    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;

protected:
    BaseMoveMorphCommand(Project *project, nanoem_rsize_t fromMorphIndex, nanoem_rsize_t toMorphIndex);

private:
    void move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error);

    const nanoem_rsize_t m_fromMorphIndex;
    const nanoem_rsize_t m_toMorphIndex;
    Model *m_activeModel;
};

class MoveMorphUpCommand : public BaseMoveMorphCommand {
public:
    static undo_command_t *create(Project *project, nanoem_rsize_t morphIndex);

    MoveMorphUpCommand(Project *project, nanoem_rsize_t morphIndex);
    ~MoveMorphUpCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class MoveMorphDownCommand : public BaseMoveMorphCommand {
public:
    static undo_command_t *create(Project *project, nanoem_rsize_t morphIndex);

    MoveMorphDownCommand(Project *project, nanoem_rsize_t morphIndex);
    ~MoveMorphDownCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class CreateLabelCommand : public BaseUndoCommand {
public:
    static undo_command_t *create(
        Project *project, nanoem_rsize_t numLabels, int offset, const nanoem_model_label_t *base);

    CreateLabelCommand(Project *project, nanoem_rsize_t numLabels, int offset, const nanoem_model_label_t *base);
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
    ScopedMutableLabel m_mutableLabel;
};

class DeleteLabelCommand : public BaseUndoCommand {
public:
    static undo_command_t *create(Project *project, nanoem_rsize_t labelIndex);

    DeleteLabelCommand(Project *project, nanoem_rsize_t labelIndex);
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
    nanoem_mutable_model_label_t *m_mutableLabel;
};

class BaseMoveLabelCommand : public BaseUndoCommand {
public:
    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;

protected:
    BaseMoveLabelCommand(Project *project, nanoem_rsize_t fromLabelIndex, nanoem_rsize_t toLabelIndex);

private:
    void move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error);

    const nanoem_rsize_t m_fromLabelIndex;
    const nanoem_rsize_t m_toLabelIndex;
    Model *m_activeModel;
};

class MoveLabelUpCommand : public BaseMoveLabelCommand {
public:
    static undo_command_t *create(Project *project, nanoem_rsize_t labelIndex);

    MoveLabelUpCommand(Project *project, nanoem_rsize_t labelIndex);
    ~MoveLabelUpCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class MoveLabelDownCommand : public BaseMoveLabelCommand {
public:
    static undo_command_t *create(Project *project, nanoem_rsize_t labelIndex);

    MoveLabelDownCommand(Project *project, nanoem_rsize_t labelIndex);
    ~MoveLabelDownCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class CreateRigidBodyCommand : public BaseUndoCommand {
public:
    static undo_command_t *create(
        Project *project, nanoem_rsize_t numRigidBodies, int offset, const nanoem_model_rigid_body_t *base);

    CreateRigidBodyCommand(
        Project *project, nanoem_rsize_t numRigidBodies, int offset, const nanoem_model_rigid_body_t *base);
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
    ScopedMutableRigidBody m_mutableRigidBody;
};

class DeleteRigidBodyCommand : public BaseUndoCommand {
public:
    static undo_command_t *create(
        Project *project, nanoem_rsize_t rigidBodyIndex);

    DeleteRigidBodyCommand(
        Project *project, nanoem_rsize_t rigidBodyIndex);
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
    nanoem_mutable_model_rigid_body_t *m_mutableRigidBody;
};

class BaseMoveRigidBodyCommand : public BaseUndoCommand {
public:
    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;

protected:
    BaseMoveRigidBodyCommand(Project *project, nanoem_rsize_t fromRigidBodyIndex, nanoem_rsize_t toRigidBodyIndex);

private:
    void move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error);

    const nanoem_rsize_t m_fromRigidBodyIndex;
    const nanoem_rsize_t m_toRigidBodyIndex;
    Model *m_activeModel;
};

class MoveRigidBodyUpCommand : public BaseMoveRigidBodyCommand {
public:
    static undo_command_t *create(
        Project *project, nanoem_rsize_t rigidBodyIndex);

    MoveRigidBodyUpCommand(
        Project *project, nanoem_rsize_t rigidBodyIndex);
    ~MoveRigidBodyUpCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class MoveRigidBodyDownCommand : public BaseMoveRigidBodyCommand {
public:
    static undo_command_t *create(
        Project *project, nanoem_rsize_t rigidBodyIndex);

    MoveRigidBodyDownCommand(
        Project *project, nanoem_rsize_t rigidBodyIndex);
    ~MoveRigidBodyDownCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class CreateJointCommand : public BaseUndoCommand {
public:
    static undo_command_t *create(
        Project *project, nanoem_rsize_t numJoints, int offset, const nanoem_model_joint_t *base);

    CreateJointCommand(Project *project, nanoem_rsize_t numJoints, int offset, const nanoem_model_joint_t *base);
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
    ScopedMutableJoint m_mutableJoint;
};

class DeleteJointCommand : public BaseUndoCommand {
public:
    static undo_command_t *create(Project *project, nanoem_rsize_t jointIndex);

    DeleteJointCommand(Project *project, nanoem_rsize_t jointIndex);
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
    nanoem_mutable_model_joint_t *m_mutableJoint;
};

class BaseMoveJointCommand : public BaseUndoCommand {
public:
    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;

protected:
    BaseMoveJointCommand(Project *project, nanoem_rsize_t fromJointIndex, nanoem_rsize_t toJointIndex);

private:
    void move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error);

    const nanoem_rsize_t m_fromJointIndex;
    const nanoem_rsize_t m_toJointIndex;
    Model *m_activeModel;
};

class MoveJointUpCommand : public BaseMoveJointCommand {
public:
    static undo_command_t *create(Project *project, nanoem_rsize_t jointIndex);

    MoveJointUpCommand(Project *project, nanoem_rsize_t jointIndex);
    ~MoveJointUpCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class MoveJointDownCommand : public BaseMoveJointCommand {
public:
    static undo_command_t *create(Project *project, nanoem_rsize_t jointIndex);

    MoveJointDownCommand(Project *project, nanoem_rsize_t jointIndex);
    ~MoveJointDownCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class CreateSoftBodyCommand : public BaseUndoCommand {
public:
    static undo_command_t *create(
        Project *project, nanoem_rsize_t numSoftBodies, int offset, const nanoem_model_soft_body_t *base);

    CreateSoftBodyCommand(
        Project *project, nanoem_rsize_t numSoftBodies, int offset, const nanoem_model_soft_body_t *base);
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
    ScopedMutableSoftBody m_mutableSoftBody;
};

class DeleteSoftBodyCommand : public BaseUndoCommand {
public:
    static undo_command_t *create(Project *project, nanoem_rsize_t softBodyIndex);

    DeleteSoftBodyCommand(
        Project *project, nanoem_rsize_t softBodyIndex);
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
    nanoem_mutable_model_soft_body_t *m_mutableSoftBody;
};

class BaseMoveSoftBodyCommand : public BaseUndoCommand {
public:
    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;

protected:
    BaseMoveSoftBodyCommand(Project *project, nanoem_rsize_t fromSoftBodyIndex, nanoem_rsize_t toSoftBodyIndex);

private:
    void move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error);

    const nanoem_rsize_t m_fromSoftBodyIndex;
    const nanoem_rsize_t m_toSoftBodyIndex;
    Model *m_activeModel;
};

class MoveSoftBodyUpCommand : public BaseMoveSoftBodyCommand {
public:
    static undo_command_t *create(
        Project *project, nanoem_rsize_t softBodyIndex);

    MoveSoftBodyUpCommand(Project *project, nanoem_rsize_t softBodyIndex);
    ~MoveSoftBodyUpCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

class MoveSoftBodyDownCommand : public BaseMoveSoftBodyCommand {
public:
    static undo_command_t *create(
        Project *project, nanoem_rsize_t softBodyIndex);

    MoveSoftBodyDownCommand(
        Project *project, nanoem_rsize_t softBodyIndex);
    ~MoveSoftBodyDownCommand() NANOEM_DECL_NOEXCEPT;

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_IUNDOCOMMAND_H_ */
