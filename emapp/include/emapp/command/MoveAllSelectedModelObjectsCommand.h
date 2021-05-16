/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_MOVEALLSELECTEDMODELOBJECTSCOMMAND_H_
#define NANOEM_EMAPP_COMMAND_MOVEALLSELECTEDMODELOBJECTSCOMMAND_H_

#include "emapp/command/BaseUndoCommand.h"

#include "emapp/IModelObjectSelection.h"

struct nanoem_mutable_model_bone_t;
struct nanoem_mutable_model_joint_t;
struct nanoem_mutable_model_rigid_body_t;
struct nanoem_mutable_model_vertex_t;

namespace nanoem {

class Model;

namespace command {

class MoveAllSelectedModelObjectsCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    class State NANOEM_DECL_SEALED {
    public:
        State(Model *activeModel);
        ~State() NANOEM_DECL_NOEXCEPT;

        void transform(const Matrix4x4 &delta);
        void commit(const Matrix4x4 &delta);
        void reset();
        void setPivotMatrix(const Matrix4x4 &value);

    private:
        struct MutableOffsetBone {
            nanoem_mutable_model_bone_t *m_opaque;
            Vector4 m_origin;
        };
        struct MutableOffsetJoint {
            nanoem_mutable_model_joint_t *m_opaque;
            Vector4 m_origin;
        };
        struct MutableOffsetRigidBody {
            nanoem_mutable_model_rigid_body_t *m_opaque;
            Matrix4x4 m_worldTransform;
            Vector4 m_origin;
        };
        struct MutableOffsetVertex {
            nanoem_mutable_model_vertex_t *m_opaque;
            Vector4 m_origin;
        };
        typedef tinystl::vector<MutableOffsetBone, TinySTLAllocator> MutableOffsetBoneList;
        typedef tinystl::vector<MutableOffsetJoint, TinySTLAllocator> MutableOffsetJointList;
        typedef tinystl::vector<MutableOffsetRigidBody, TinySTLAllocator> MutableOffsetRigidBodyList;
        typedef tinystl::vector<MutableOffsetVertex, TinySTLAllocator> MutableOffsetVertexList;

        Matrix4x4 rigidBodyWorldTransform(const nanoem_model_rigid_body_t *rigidBodyPtr) const;
        void updateRigidBody(
            const Matrix4x4 &delta, const Matrix4x4 &worldTransform, const nanoem_model_rigid_body_t *rigidBodyPtr);

        Model *m_activeModel;
        MutableOffsetBoneList m_bones;
        MutableOffsetJointList m_joints;
        MutableOffsetRigidBodyList m_rigidBodies;
        MutableOffsetVertexList m_vertices;
        IModelObjectSelection::EditingType m_editingType;
    };

    static undo_command_t *create(
        const Matrix4x4 &transformMatrix, const Matrix4x4 &pivotMatrix, Model *activeModel, Project *project);
    ~MoveAllSelectedModelObjectsCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    MoveAllSelectedModelObjectsCommand(
        const Matrix4x4 &transformMatrix, const Matrix4x4 &pivotMatrix, Model *activeModel, Project *project);

    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;

    State m_state;
    Matrix4x4 m_transformMatrix;
    Matrix4x4 m_lastPivotMatrix;
    Matrix4x4 m_currentPivotMatrix;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_MOVEALLSELECTEDMODELOBJECTSCOMMAND_H_ */
