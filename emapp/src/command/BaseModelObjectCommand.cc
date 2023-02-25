/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/BaseModelObjectCommand.h"

#include "../CommandMessage.inl"
#include "emapp/private/CommonInclude.h"

namespace {

using namespace nanoem;
using namespace nanoem::command;

struct MOObject : public BaseModelObjectCommand::Object {
    MOObject(void *opaque, const BaseModelObjectCommand::Function &undoFunc,
        const BaseModelObjectCommand::Function &redoFunc)
        : m_undoFunction(undoFunc)
        , m_redoFunction(redoFunc)
        , m_opaque(opaque)
    {
    }
    ~MOObject() NANOEM_DECL_NOEXCEPT
    {
        m_opaque = 0;
    }
    BaseModelObjectCommand::Function
    redoFunction() const
    {
        return m_redoFunction;
    }
    BaseModelObjectCommand::Function
    undoFunction() const
    {
        return m_undoFunction;
    }
    const BaseModelObjectCommand::Function m_undoFunction;
    const BaseModelObjectCommand::Function m_redoFunction;
    void *m_opaque;
};

struct MOModel : MOObject {
    MOModel(nanoem_model_t *model, BaseModelObjectCommand::Function undoFunc, BaseModelObjectCommand::Function redoFunc)
        : MOObject(model, undoFunc, redoFunc)
    {
    }
    void *
    createOpaque() NANOEM_DECL_OVERRIDE
    {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        return nanoemMutableModelCreateAsReference(static_cast<nanoem_model_t *>(m_opaque), &status);
    }
    void
    destroyOpaque(void *opaque) NANOEM_DECL_OVERRIDE
    {
        nanoemMutableModelDestroy(static_cast<nanoem_mutable_model_t *>(opaque));
    }
};

struct MOVertex : MOObject {
    MOVertex(nanoem_model_vertex_t *vertex, BaseModelObjectCommand::Function redoFunc,
        BaseModelObjectCommand::Function undoFunc)
        : MOObject(vertex, redoFunc, undoFunc)
    {
    }
    void *
    createOpaque() NANOEM_DECL_OVERRIDE
    {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        return nanoemMutableModelVertexCreateAsReference(static_cast<nanoem_model_vertex_t *>(m_opaque), &status);
    }
    void
    destroyOpaque(void *opaque) NANOEM_DECL_OVERRIDE
    {
        nanoemMutableModelVertexDestroy(static_cast<nanoem_mutable_model_vertex_t *>(opaque));
    }
};

struct MOMaterial : MOObject {
    MOMaterial(nanoem_model_material_t *material, BaseModelObjectCommand::Function redoFunc,
        BaseModelObjectCommand::Function undoFunc)
        : MOObject(material, redoFunc, undoFunc)
    {
    }
    void *
    createOpaque() NANOEM_DECL_OVERRIDE
    {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        return nanoemMutableModelMaterialCreateAsReference(static_cast<nanoem_model_material_t *>(m_opaque), &status);
    }
    void
    destroyOpaque(void *opaque) NANOEM_DECL_OVERRIDE
    {
        nanoemMutableModelMaterialDestroy(static_cast<nanoem_mutable_model_material_t *>(opaque));
    }
};

struct MOBone : MOObject {
    MOBone(
        nanoem_model_bone_t *bone, BaseModelObjectCommand::Function redoFunc, BaseModelObjectCommand::Function undoFunc)
        : MOObject(bone, redoFunc, undoFunc)
    {
    }
    void *
    createOpaque() NANOEM_DECL_OVERRIDE
    {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        return nanoemMutableModelBoneCreateAsReference(static_cast<nanoem_model_bone_t *>(m_opaque), &status);
    }
    void
    destroyOpaque(void *opaque) NANOEM_DECL_OVERRIDE
    {
        nanoemMutableModelBoneDestroy(static_cast<nanoem_mutable_model_bone_t *>(opaque));
    }
};

struct MOMorph : MOObject {
    MOMorph(nanoem_model_morph_t *morph, BaseModelObjectCommand::Function redoFunc,
        BaseModelObjectCommand::Function undoFunc)
        : MOObject(morph, redoFunc, undoFunc)
    {
    }
    void *
    createOpaque() NANOEM_DECL_OVERRIDE
    {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        return nanoemMutableModelMorphCreateAsReference(static_cast<nanoem_model_morph_t *>(m_opaque), &status);
    }
    void
    destroyOpaque(void *opaque) NANOEM_DECL_OVERRIDE
    {
        nanoemMutableModelMorphDestroy(static_cast<nanoem_mutable_model_morph_t *>(opaque));
    }
};

struct MOLabel : MOObject {
    MOLabel(nanoem_model_label_t *label, BaseModelObjectCommand::Function redoFunc,
        BaseModelObjectCommand::Function undoFunc)
        : MOObject(label, redoFunc, undoFunc)
    {
    }
    void *
    createOpaque() NANOEM_DECL_OVERRIDE
    {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        return nanoemMutableModelLabelCreateAsReference(static_cast<nanoem_model_label_t *>(m_opaque), &status);
    }
    void
    destroyOpaque(void *opaque) NANOEM_DECL_OVERRIDE
    {
        nanoemMutableModelLabelDestroy(static_cast<nanoem_mutable_model_label_t *>(opaque));
    }
};

struct MORigidBody : MOObject {
    MORigidBody(nanoem_model_rigid_body_t *rigidBody, BaseModelObjectCommand::Function redoFunc,
        BaseModelObjectCommand::Function undoFunc)
        : MOObject(rigidBody, redoFunc, undoFunc)
    {
    }
    void *
    createOpaque() NANOEM_DECL_OVERRIDE
    {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        return nanoemMutableModelRigidBodyCreateAsReference(
            static_cast<nanoem_model_rigid_body_t *>(m_opaque), &status);
    }
    void
    destroyOpaque(void *opaque) NANOEM_DECL_OVERRIDE
    {
        nanoemMutableModelRigidBodyDestroy(static_cast<nanoem_mutable_model_rigid_body_t *>(opaque));
    }
};

struct MOJoint : MOObject {
    MOJoint(nanoem_model_joint_t *joint, BaseModelObjectCommand::Function redoFunc,
        BaseModelObjectCommand::Function undoFunc)
        : MOObject(joint, redoFunc, undoFunc)
    {
    }
    void *
    createOpaque() NANOEM_DECL_OVERRIDE
    {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        return nanoemMutableModelJointCreateAsReference(static_cast<nanoem_model_joint_t *>(m_opaque), &status);
    }
    void
    destroyOpaque(void *opaque) NANOEM_DECL_OVERRIDE
    {
        nanoemMutableModelJointDestroy(static_cast<nanoem_mutable_model_joint_t *>(opaque));
    }
};
}

namespace nanoem {
namespace command {

BaseModelObjectCommand::Object *
BaseModelObjectCommand::Object::create(nanoem_model_t *model, const Function &undoFunc, const Function &redoFunc)
{
    return nanoem_new(MOModel(model, undoFunc, redoFunc));
}

BaseModelObjectCommand::Object *
BaseModelObjectCommand::Object::create(
    nanoem_model_vertex_t *vertex, const Function &undoFunc, const Function &redoFunc)
{
    return nanoem_new(MOVertex(vertex, undoFunc, redoFunc));
}

BaseModelObjectCommand::Object *
BaseModelObjectCommand::Object::create(
    nanoem_model_material_t *material, const Function &undoFunc, const Function &redoFunc)
{
    return nanoem_new(MOMaterial(material, undoFunc, redoFunc));
}

BaseModelObjectCommand::Object *
BaseModelObjectCommand::Object::create(nanoem_model_bone_t *bone, const Function &undoFunc, const Function &redoFunc)
{
    return nanoem_new(MOBone(bone, undoFunc, redoFunc));
}

BaseModelObjectCommand::Object *
BaseModelObjectCommand::Object::create(nanoem_model_morph_t *morph, const Function &undoFunc, const Function &redoFunc)
{
    return nanoem_new(MOMorph(morph, undoFunc, redoFunc));
}

BaseModelObjectCommand::Object *
BaseModelObjectCommand::Object::create(nanoem_model_label_t *label, const Function &undoFunc, const Function &redoFunc)
{
    return nanoem_new(MOLabel(label, undoFunc, redoFunc));
}

BaseModelObjectCommand::Object *
BaseModelObjectCommand::Object::create(
    nanoem_model_rigid_body_t *rigidBody, const Function &undoFunc, const Function &redoFunc)
{
    return nanoem_new(MORigidBody(rigidBody, redoFunc, undoFunc));
}

BaseModelObjectCommand::Object *
BaseModelObjectCommand::Object::create(nanoem_model_joint_t *joint, const Function &undoFunc, const Function &redoFunc)
{
    return nanoem_new(MOJoint(joint, redoFunc, undoFunc));
}

BaseModelObjectCommand::Object *
BaseModelObjectCommand::Object::create(nanoem_model_t *model, const Function &func)
{
    return nanoem_new(MOModel(model, func, func));
}

BaseModelObjectCommand::Object *
BaseModelObjectCommand::Object::create(nanoem_model_vertex_t *vertex, const Function &func)
{
    return nanoem_new(MOVertex(vertex, func, func));
}

BaseModelObjectCommand::Object *
BaseModelObjectCommand::Object::create(nanoem_model_material_t *material, const Function &func)
{
    return nanoem_new(MOMaterial(material, func, func));
}

BaseModelObjectCommand::Object *
BaseModelObjectCommand::Object::create(nanoem_model_bone_t *bone, const Function &func)
{
    return nanoem_new(MOBone(bone, func, func));
}

BaseModelObjectCommand::Object *
BaseModelObjectCommand::Object::create(nanoem_model_morph_t *morph, const Function &func)
{
    return nanoem_new(MOMorph(morph, func, func));
}

BaseModelObjectCommand::Object *
BaseModelObjectCommand::Object::create(nanoem_model_label_t *label, const Function &func)
{
    return nanoem_new(MOLabel(label, func, func));
}

BaseModelObjectCommand::Object *
BaseModelObjectCommand::Object::create(nanoem_model_rigid_body_t *rigidBody, const Function &func)
{
    return nanoem_new(MORigidBody(rigidBody, func, func));
}

BaseModelObjectCommand::Object *
BaseModelObjectCommand::Object::create(nanoem_model_joint_t *joint, const Function &func)
{
    return nanoem_new(MOJoint(joint, func, func));
}

BaseModelObjectCommand::ExecutionBlock::ExecutionBlock(Object *o, Model *m)
    : m_model(m)
    , m_object(o)
    , m_destructor(0)
    , m_userData(0)
    , m_index(0)
{
    m_newValue.integerValue = m_oldValue.integerValue = 0;
}

BaseModelObjectCommand::ExecutionBlock::ExecutionBlock(Object *o, Model *m, const tinystl::pair<bool, bool> &v)
    : m_model(m)
    , m_object(o)
    , m_destructor(0)
    , m_userData(0)
    , m_index(0)
{
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
    m_newValue.booleanValue = v.first;
    m_oldValue.booleanValue = v.second;
#else
    m_newValue.integerValue = v.first ? 1 : 0;
    m_oldValue.integerValue = v.second ? 1 : 0;
#endif
}

BaseModelObjectCommand::ExecutionBlock::ExecutionBlock(Object *o, Model *m, const tinystl::pair<int, int> &v)
    : m_model(m)
    , m_object(o)
    , m_destructor(0)
    , m_userData(0)
    , m_index(0)
{
    m_newValue.integerValue = v.first;
    m_oldValue.integerValue = v.second;
}

BaseModelObjectCommand::ExecutionBlock::ExecutionBlock(
    Object *o, Model *m, const tinystl::pair<nanoem_f32_t, nanoem_f32_t> &v, nanoem_rsize_t index)
    : m_model(m)
    , m_object(o)
    , m_destructor(0)
    , m_userData(0)
    , m_index(index)
{
    m_newValue.nanoem_f32_tValue = v.first;
    m_oldValue.nanoem_f32_tValue = v.second;
}

BaseModelObjectCommand::ExecutionBlock::ExecutionBlock(Object *o, Model *m, const tinystl::pair<void *, void *> &v,
    destructor_t destructor, void *userData, nanoem_rsize_t index)
    : m_model(m)
    , m_object(o)
    , m_destructor(destructor)
    , m_userData(userData)
    , m_index(index)
{
    m_newValue.mutableObjectValue = v.first;
    m_oldValue.mutableObjectValue = v.second;
}

BaseModelObjectCommand::ExecutionBlock::ExecutionBlock(
    Object *o, Model *m, const tinystl::pair<const void *, const void *> &v, nanoem_rsize_t index)
    : m_model(m)
    , m_object(o)
    , m_destructor(0)
    , m_userData(0)
    , m_index(index)
{
    m_newValue.immutableObjectValue = v.first;
    m_oldValue.immutableObjectValue = v.second;
}

BaseModelObjectCommand::ExecutionBlock::~ExecutionBlock() NANOEM_DECL_NOEXCEPT
{
    if (m_destructor) {
        m_destructor(m_newValue.mutableObjectValue, m_userData);
        m_destructor(m_oldValue.mutableObjectValue, m_userData);
    }
    nanoem_delete(m_object);
    m_object = 0;
}

void
BaseModelObjectCommand::ExecutionBlock::execute(const Value &v, const Function &function)
{
    MOObject *object = static_cast<MOObject *>(m_object);
    void *opaque = object->createOpaque();
    switch (function.type) {
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
    case Function::kTypeBoolean:
        function.u.setBoolean(opaque, v.booleanValue);
        break;
#endif
    case Function::kTypeInteger:
        function.u.setInteger(opaque, v.integerValue);
        break;
    case Function::kTypeFloat:
        function.u.setFloat(opaque, v.nanoem_f32_tValue);
        break;
    case Function::kTypeObject:
        function.u.setObject(opaque, v.immutableObjectValue);
        break;
    case Function::kTypeObjectAtIndex:
        function.u.setObjectAtIndex(opaque, v.immutableObjectValue, m_index);
        break;
    case Function::kTypeMaxEnum:
    default:
        break;
    }
    object->destroyOpaque(opaque);
    m_model->setDirty(true);
}

void
BaseModelObjectCommand::ExecutionBlock::undo(Error &error)
{
    BX_UNUSED_1(error);
    MOObject *object = static_cast<MOObject *>(m_object);
    execute(m_oldValue, object->m_undoFunction);
}

void
BaseModelObjectCommand::ExecutionBlock::redo(Error &error)
{
    BX_UNUSED_1(error);
    MOObject *object = static_cast<MOObject *>(m_object);
    execute(m_newValue, object->m_redoFunction);
}

BaseModelObjectCommand::~BaseModelObjectCommand() NANOEM_DECL_NOEXCEPT
{
    nanoem_delete(m_block);
    m_block = 0;
}

BaseModelObjectCommand::BaseModelObjectCommand(Model *model, ExecutionBlock *block)
    : BaseUndoCommand(model->project())
    , m_model(model)
    , m_block(block)
{
}

} /* namespace command */
} /* namespace nanoem */
