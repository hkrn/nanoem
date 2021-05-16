/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_TRANSFORMMODELCOMMAND_H_
#define NANOEM_EMAPP_COMMAND_TRANSFORMMODELCOMMAND_H_

#include "emapp/command/BaseUndoCommand.h"

struct nanoem_mutable_model_vertex_t;
struct nanoem_mutable_model_bone_t;
struct nanoem_mutable_model_morph_vertex_t;
struct nanoem_mutable_model_morph_bone_t;
struct nanoem_mutable_model_rigid_body_t;
struct nanoem_mutable_model_joint_t;

namespace nanoem {

class Model;

namespace command {

class TransformModelCommand : public BaseUndoCommand {
public:
    static undo_command_t *create(Project *project, const Matrix4x4 &transform);

    TransformModelCommand(Project *project, const Matrix4x4 &transform);
    ~TransformModelCommand() NANOEM_DECL_NOEXCEPT;

    void undo(Error &error) NANOEM_DECL_OVERRIDE;
    void redo(Error &error) NANOEM_DECL_OVERRIDE;
    void read(const void *messagePtr) NANOEM_DECL_OVERRIDE;
    void write(void *messagePtr) NANOEM_DECL_OVERRIDE;
    void release(void *messagePtr) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    struct Vertex {
        nanoem_mutable_model_vertex_t *m_opaque;
        Vector3 m_origin;
    };
    typedef tinystl::vector<Vertex, TinySTLAllocator> VertexList;
    struct Bone {
        nanoem_mutable_model_bone_t *m_opaque;
        Vector3 m_origin;
        Vector3 m_destination;
    };
    typedef tinystl::vector<Bone, TinySTLAllocator> BoneList;
    struct VertexMorph {
        nanoem_mutable_model_morph_vertex_t *m_opaque;
        Vector3 m_origin;
    };
    typedef tinystl::vector<VertexMorph, TinySTLAllocator> VertexMorphList;
    struct BoneMorph {
        nanoem_mutable_model_morph_bone_t *m_opaque;
        Vector3 m_translation;
    };
    typedef tinystl::vector<BoneMorph, TinySTLAllocator> BoneMorphList;
    struct RigidBody {
        nanoem_mutable_model_rigid_body_t *m_opaque;
        Vector3 m_origin;
        Vector3 m_size;
    };
    typedef tinystl::vector<RigidBody, TinySTLAllocator> RigidBodyList;
    struct Joint {
        nanoem_mutable_model_joint_t *m_opaque;
        Vector3 m_origin;
    };
    typedef tinystl::vector<Joint, TinySTLAllocator> JointList;

    void scaleRigidBody(const RigidBody &value, const Matrix4x4 &transform);

    Model *m_activeModel;
    VertexList m_vertices;
    BoneList m_bones;
    VertexMorphList m_vertexMorphs;
    BoneMorphList m_boneMorphs;
    RigidBodyList m_rigidBodies;
    JointList m_joints;
    Matrix4x4 m_transform;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_TRANSFORMMODELCOMMAND_H_ */
