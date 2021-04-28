/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/ModelObjectCommand.h"

#include "emapp/Error.h"
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

namespace nanoem {
namespace command {
namespace {

static const nanoem_u8_t kNewObjectPrefixName[] = { 0xe6, 0x96, 0xb0, 0xe8, 0xa6, 0x8f, 0 };

} /* namespace anonymous */

ScopedMutableModel::ScopedMutableModel(Model *model)
    : m_model(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_model = nanoemMutableModelCreateAsReference(model->data(), &status);
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

undo_command_t *
DeleteMaterialCommand::create(
    Project *project, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex)
{
    DeleteMaterialCommand *command =
        nanoem_new(DeleteMaterialCommand(project, materials, materialIndex));
    return command->createCommand();
}

DeleteMaterialCommand::DeleteMaterialCommand(
    Project *project, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex)
    : BaseUndoCommand(project)
    , m_materials(materials)
    , m_materialIndex(materialIndex)
{
}

DeleteMaterialCommand::~DeleteMaterialCommand() NANOEM_DECL_NOEXCEPT
{
}

void
DeleteMaterialCommand::undo(Error &error)
{
}

void
DeleteMaterialCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableMaterial material(m_materials[m_materialIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_rsize_t numMaterials, offset = 0, size = 0;
    nanoem_model_material_t *const *materials =
        nanoemModelGetAllMaterialObjects(nanoemMutableModelGetOriginObject(model), &numMaterials);
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *currentMaterialPtr = materials[i];
        const size_t innerSize = nanoemModelMaterialGetNumVertexIndices(currentMaterialPtr);
        if (currentMaterialPtr == nanoemMutableModelMaterialGetOriginObject(material)) {
            size = innerSize;
            break;
        }
        offset += innerSize;
    }
    nanoem_rsize_t numIndices, rest;
    const nanoem_u32_t *indices = nanoemModelGetAllVertexIndices(nanoemMutableModelGetOriginObject(model), &numIndices);
    tinystl::vector<nanoem_u32_t, TinySTLAllocator> workingBuffer(numIndices);
    rest = numIndices - offset - size;
    memcpy(workingBuffer.data(), indices, numIndices * sizeof(workingBuffer[0]));
    memmove(workingBuffer.data() + offset, workingBuffer.data() + offset + size, rest * sizeof(workingBuffer[0]));
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelSetVertexIndices(model, workingBuffer.data(), numIndices - size, &status);
    nanoemMutableModelRemoveMaterialObject(model, material, &status);
    nanoemMutableModelMaterialDestroy(material);
    ByteArray bytes;
    activeModel->save(bytes, error);
    Progress reloadModelProgress(project, 0);
    activeModel->clear();
    activeModel->load(bytes, error);
    activeModel->setupAllBindings();
    activeModel->upload();
    activeModel->loadAllImages(reloadModelProgress, error);
    if (Motion *motion = project->resolveMotion(activeModel)) {
        motion->initialize(activeModel);
    }
    activeModel->updateStagingVertexBuffer();
    project->rebuildAllTracks();
    project->restart();
    reloadModelProgress.complete();
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
    Project *project, nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex)
    : BaseUndoCommand(project)
    , m_materials(materials)
    , m_materialIndex(materialIndex)
{
}

BaseMoveMaterialCommand::~BaseMoveMaterialCommand() NANOEM_DECL_NOEXCEPT
{
}

void
BaseMoveMaterialCommand::move(int destination, const BaseMoveMaterialCommand::LayoutPosition &from,
    const BaseMoveMaterialCommand::LayoutPosition &to, Model *activeModel)
{
    ScopedMutableMaterial material(m_materials[m_materialIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_rsize_t numIndices;
    const nanoem_u32_t *indices = nanoemModelGetAllVertexIndices(nanoemMutableModelGetOriginObject(model), &numIndices);
    tinystl::vector<nanoem_u32_t, TinySTLAllocator> tempFromBuffer(from.m_size), tempToBuffer(to.m_size);
    memcpy(tempFromBuffer.data(), indices + from.m_offset, from.m_size * sizeof(tempToBuffer[0]));
    memcpy(tempToBuffer.data(), indices + to.m_offset, to.m_size * sizeof(tempToBuffer[0]));
    tinystl::vector<nanoem_u32_t, TinySTLAllocator> workingBuffer(numIndices);
    memcpy(workingBuffer.data(), indices, numIndices * sizeof(workingBuffer[0]));
    memcpy(workingBuffer.data() + from.m_offset, tempFromBuffer.data(), from.m_size * sizeof(tempFromBuffer[0]));
    memcpy(workingBuffer.data() + to.m_offset, tempToBuffer.data(), to.m_size * sizeof(tempToBuffer[0]));
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelSetVertexIndices(model, workingBuffer.data(), numIndices, &status);
    nanoemMutableModelRemoveMaterialObject(model, material, &status);
    nanoemMutableModelInsertMaterialObject(model, material, destination, &status);
}

undo_command_t *
MoveMaterialUpCommand::create(
    Project *project, nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex)
{
    MoveMaterialUpCommand *command = nanoem_new(MoveMaterialUpCommand(project, materials, materialIndex));
    return command->createCommand();
}

MoveMaterialUpCommand::MoveMaterialUpCommand(Project *project, nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex)
    : BaseMoveMaterialCommand(project, materials, materialIndex)
{
}

MoveMaterialUpCommand::~MoveMaterialUpCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveMaterialUpCommand::undo(Error &error)
{
}

void
MoveMaterialUpCommand::redo(Error &error)
{
    const nanoem_model_material_t *activeMaterial = m_materials[m_materialIndex];
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    nanoem_rsize_t numMaterials, offset = 0;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(activeModel->data(), &numMaterials);
    LayoutPosition from, to;
    int destination = 0;
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *currentMaterialPtr = materials[i];
        size_t size = nanoemModelMaterialGetNumVertexIndices(currentMaterialPtr);
        if (currentMaterialPtr == activeMaterial) {
            const nanoem_model_material_t *previousMaterialPtr = materials[i - 1];
            destination = i - 1;
            to.m_size = nanoemModelMaterialGetNumVertexIndices(previousMaterialPtr);
            to.m_offset = offset - to.m_size;
            from.m_offset = offset;
            from.m_size = size;
            break;
        }
        offset += size;
    }
    move(destination, from, to, activeModel);
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
    Project *project, nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex)
{
    MoveMaterialDownCommand *command = nanoem_new(MoveMaterialDownCommand(project, materials, materialIndex));
    return command->createCommand();
}

MoveMaterialDownCommand::MoveMaterialDownCommand(
    Project *project, nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex)
    : BaseMoveMaterialCommand(project, materials, materialIndex)
{
}

MoveMaterialDownCommand::~MoveMaterialDownCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveMaterialDownCommand::undo(Error &error)
{
}

void
MoveMaterialDownCommand::redo(Error &error)
{
    const nanoem_model_material_t *activeMaterial = m_materials[m_materialIndex];
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    nanoem_rsize_t numMaterials, offset = 0;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(activeModel->data(), &numMaterials);
    LayoutPosition from, to;
    int destination = 0;
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *currentMaterialPtr = materials[i];
        size_t size = nanoemModelMaterialGetNumVertexIndices(currentMaterialPtr);
        if (currentMaterialPtr == activeMaterial) {
            const nanoem_model_material_t *nextMaterialPtr = materials[i + 1];
            destination = i + 1;
            to.m_size = nanoemModelMaterialGetNumVertexIndices(nextMaterialPtr);
            to.m_offset = offset + to.m_size;
            from.m_offset = offset;
            from.m_size = size;
            break;
        }
        offset += size;
    }
    move(destination, from, to, activeModel);
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
CreateBoneCommand::create(Project *project, nanoem_rsize_t numBones, int offset, const nanoem_model_bone_t *base)
{
    CreateBoneCommand *command = nanoem_new(CreateBoneCommand(project, numBones, offset, base));
    return command->createCommand();
}

CreateBoneCommand::CreateBoneCommand(
    Project *project, nanoem_rsize_t numBones, int offset, const nanoem_model_bone_t *base)
    : BaseUndoCommand(project)
    , m_base(base)
    , m_numBones(numBones)
    , m_offset(offset)
{
}

CreateBoneCommand::~CreateBoneCommand() NANOEM_DECL_NOEXCEPT
{
}

void
CreateBoneCommand::undo(Error &error)
{
}

void
CreateBoneCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableModel model(activeModel);
    ScopedMutableBone bone(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    char buffer[Inline::kMarkerStringLength];
    if (m_base) {
        nanoemMutableModelBoneCopy(bone, m_base, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, m_numBones + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelBoneSetName(bone, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "NewBone%zu", m_numBones + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelBoneSetName(bone, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    nanoemMutableModelInsertBoneObject(model, bone, m_offset, &status);
    model::Bone *newBone = model::Bone::create();
    nanoem_model_bone_t *bonePtr = nanoemMutableModelBoneGetOriginObject(bone);
    newBone->bind(bonePtr);
    newBone->resetLanguage(nanoemMutableModelBoneGetOriginObject(bone), factory, project->castLanguage());
    activeModel->addBoneReference(bonePtr);
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
DeleteBoneCommand::create(Project *project, nanoem_model_bone_t *const *bones, nanoem_rsize_t boneIndex)
{
    DeleteBoneCommand *command = nanoem_new(DeleteBoneCommand(project, bones, boneIndex));
    return command->createCommand();
}

DeleteBoneCommand::DeleteBoneCommand(Project *project, nanoem_model_bone_t *const *bones, nanoem_rsize_t &boneIndex)
    : BaseUndoCommand(project)
    , m_bones(bones)
    , m_boneIndex(boneIndex)
{
}

DeleteBoneCommand::~DeleteBoneCommand() NANOEM_DECL_NOEXCEPT
{
}

void
DeleteBoneCommand::undo(Error &error)
{
}

void
DeleteBoneCommand::redo(Error &error)
{
    nanoem_model_bone_t *bonePtr = m_bones[m_boneIndex];
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableBone bone(bonePtr);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    if (activeModel->activeBone() == bonePtr) {
        activeModel->setActiveBone(nullptr);
    }
    activeModel->removeBoneReference(bonePtr);
    nanoemMutableModelRemoveBoneObject(model, bone, &status);
    project->rebuildAllTracks();
    if (m_boneIndex > 0) {
        m_boneIndex--;
    }
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

undo_command_t *
MoveBoneDownCommand::create(
    Project *project, nanoem_model_bone_t *const *bones, nanoem_rsize_t &boneIndex)
{
    MoveBoneDownCommand *command = nanoem_new(MoveBoneDownCommand(project, bones, boneIndex));
    return command->createCommand();
}

MoveBoneDownCommand::MoveBoneDownCommand(Project *project, nanoem_model_bone_t *const *bones, nanoem_rsize_t &boneIndex)
    : BaseUndoCommand(project)
    , m_bones(bones)
    , m_boneIndex(boneIndex)
{
}

MoveBoneDownCommand::~MoveBoneDownCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveBoneDownCommand::undo(Error &error)
{
}

void
MoveBoneDownCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableBone bone(m_bones[m_boneIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveBoneObject(model, bone, &status);
    int offset = Inline::saturateInt32(++m_boneIndex);
    nanoemMutableModelInsertBoneObject(model, bone, offset, &status);
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
MoveBoneUpCommand::create(Project *project, nanoem_model_bone_t *const *bones, nanoem_rsize_t &boneIndex)
{
    MoveBoneUpCommand *command = nanoem_new(MoveBoneUpCommand(project, bones, boneIndex));
    return command->createCommand();
}

MoveBoneUpCommand::MoveBoneUpCommand(Project *project, nanoem_model_bone_t *const *bones, nanoem_rsize_t &boneIndex)
    : BaseUndoCommand(project)
    , m_bones(bones)
    , m_boneIndex(boneIndex)
{
}

MoveBoneUpCommand::~MoveBoneUpCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveBoneUpCommand::undo(Error &error)
{
}

void
MoveBoneUpCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableBone bone(m_bones[m_boneIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveBoneObject(model, bone, &status);
    int offset = Inline::saturateInt32(--m_boneIndex);
    nanoemMutableModelInsertBoneObject(model, bone, offset, &status);
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
CreateMorphCommand::create(Project *project, nanoem_rsize_t numMorphs, int offset, const nanoem_model_morph_t *base,
    nanoem_model_morph_type_t type)
{
    CreateMorphCommand *command = nanoem_new(CreateMorphCommand(project, numMorphs, offset, base, type));
    return command->createCommand();
}

CreateMorphCommand::CreateMorphCommand(Project *project, nanoem_rsize_t numMorphs, int offset,
    const nanoem_model_morph_t *base, nanoem_model_morph_type_t type)
    : BaseUndoCommand(project)
    , m_base(base)
    , m_type(type)
    , m_numMorphs(numMorphs)
    , m_offset(offset)
{
}

CreateMorphCommand::~CreateMorphCommand() NANOEM_DECL_NOEXCEPT
{
}

void
CreateMorphCommand::undo(Error &error)
{
}

void
CreateMorphCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableMorph morph(activeModel);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    char buffer[Inline::kMarkerStringLength];
    if (m_base) {
        nanoemMutableModelMorphCopy(morph, m_base, &status);
    }
    else {
        nanoemMutableModelMorphSetType(morph, m_type);
    }
    StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, m_numMorphs + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelMorphSetName(morph, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "NewMorph%zu", m_numMorphs + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelMorphSetName(morph, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    nanoemMutableModelInsertMorphObject(model, morph, m_offset, &status);
    model::Morph *newMorph = model::Morph::create();
    newMorph->bind(nanoemMutableModelMorphGetOriginObject(morph));
    newMorph->resetLanguage(nanoemMutableModelMorphGetOriginObject(morph), factory, project->castLanguage());
    project->rebuildAllTracks();
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
DeleteMorphCommand::create(
    Project *project, nanoem_model_morph_t *const *morphs, nanoem_rsize_t &morphIndex)
{
    DeleteMorphCommand *command = nanoem_new(DeleteMorphCommand(project, morphs, morphIndex));
    return command->createCommand();
}

DeleteMorphCommand::DeleteMorphCommand(
    Project *project, nanoem_model_morph_t *const *morphs, nanoem_rsize_t &morphIndex)
    : BaseUndoCommand(project)
    , m_morphs(morphs)
    , m_morphIndex(morphIndex)
{
}

DeleteMorphCommand::~DeleteMorphCommand() NANOEM_DECL_NOEXCEPT
{
}

void
DeleteMorphCommand::undo(Error &error)
{
}

void
DeleteMorphCommand::redo(Error &error)
{
    nanoem_model_morph_t *morphPtr = m_morphs[m_morphIndex];
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableMorph morph(morphPtr);
    ScopedMutableModel model(activeModel);
    for (int i = NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM; i < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM; i++) {
        nanoem_model_morph_category_t category = static_cast<nanoem_model_morph_category_t>(i);
        if (activeModel->activeMorph(category) == morphPtr) {
            activeModel->setActiveMorph(category, nullptr);
        }
    }
    activeModel->removeMorphReference(morphPtr);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveMorphObject(model, morph, &status);
    project->rebuildAllTracks();
    if (m_morphIndex > 0) {
        m_morphIndex--;
    }
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

undo_command_t *
MoveMorphUpCommand::create(
    Project *project, nanoem_model_morph_t *const *morphs, nanoem_rsize_t &morphIndex)
{
    MoveMorphUpCommand *command = nanoem_new(MoveMorphUpCommand(project, morphs, morphIndex));
    return command->createCommand();
}

MoveMorphUpCommand::MoveMorphUpCommand(
    Project *project, nanoem_model_morph_t *const *morphs, nanoem_rsize_t &morphIndex)
    : BaseUndoCommand(project)
    , m_morphs(morphs)
    , m_morphIndex(morphIndex)
{
}

MoveMorphUpCommand::~MoveMorphUpCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveMorphUpCommand::undo(Error &error)
{
}

void
MoveMorphUpCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableMorph morph(m_morphs[m_morphIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveMorphObject(model, morph, &status);
    int offset = Inline::saturateInt32(--m_morphIndex);
    nanoemMutableModelInsertMorphObject(model, morph, offset, &status);
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
MoveMorphDownCommand::create(Project *project, nanoem_model_morph_t *const *morphs, nanoem_rsize_t &morphIndex)
{
    MoveMorphDownCommand *command = nanoem_new(MoveMorphDownCommand(project, morphs, morphIndex));
    return command->createCommand();
}

MoveMorphDownCommand::MoveMorphDownCommand(
    Project *project, nanoem_model_morph_t *const *morphs, nanoem_rsize_t &morphIndex)
    : BaseUndoCommand(project)
    , m_morphs(morphs)
    , m_morphIndex(morphIndex)
{
}

MoveMorphDownCommand::~MoveMorphDownCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveMorphDownCommand::undo(Error &error)
{
}

void
MoveMorphDownCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableMorph morph(m_morphs[m_morphIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveMorphObject(model, morph, &status);
    int offset = Inline::saturateInt32(--m_morphIndex);
    nanoemMutableModelInsertMorphObject(model, morph, offset, &status);
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
CreateLabelCommand::create(
    Project *project, nanoem_rsize_t numLabels, int offset, const nanoem_model_label_t *base)
{
    CreateLabelCommand *command = nanoem_new(CreateLabelCommand(project, numLabels, offset, base));
    return command->createCommand();
}

CreateLabelCommand::CreateLabelCommand(
    Project *project, nanoem_rsize_t numLabels, int offset, const nanoem_model_label_t *base)
    : BaseUndoCommand(project)
    , m_base(base)
    , m_numLabels(numLabels)
    , m_offset(offset)
{
}

CreateLabelCommand::~CreateLabelCommand() NANOEM_DECL_NOEXCEPT
{
}

void
CreateLabelCommand::undo(Error &error)
{
}

void
CreateLabelCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableLabel label(activeModel);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    char buffer[Inline::kMarkerStringLength];
    if (m_base) {
        nanoemMutableModelLabelCopy(label, m_base, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, m_numLabels + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelLabelSetName(label, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "NewLabel%zu", m_numLabels + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelLabelSetName(label, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    nanoemMutableModelInsertLabelObject(model, label, m_offset, &status);
    model::Label *newLabel = model::Label::create();
    newLabel->bind(nanoemMutableModelLabelGetOriginObject(label));
    newLabel->resetLanguage(nanoemMutableModelLabelGetOriginObject(label), factory, project->castLanguage());
    project->rebuildAllTracks();
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

undo_command_t *
DeleteLabelCommand::create(Project *project, nanoem_model_label_t *const *labels, nanoem_rsize_t &labelIndex)
{
    DeleteLabelCommand *command = nanoem_new(DeleteLabelCommand(project, labels, labelIndex));
    return command->createCommand();
}

DeleteLabelCommand::DeleteLabelCommand(
    Project *project, nanoem_model_label_t *const *labels, nanoem_rsize_t &labelIndex)
    : BaseUndoCommand(project)
    , m_labels(labels)
    , m_labelIndex(labelIndex)
{
}

DeleteLabelCommand::~DeleteLabelCommand() NANOEM_DECL_NOEXCEPT
{
}

void
DeleteLabelCommand::undo(Error &error)
{
}

void
DeleteLabelCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableLabel label(m_labels[m_labelIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveLabelObject(model, label, &status);
    project->rebuildAllTracks();
    if (m_labelIndex > 0) {
        m_labelIndex--;
    }
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

undo_command_t *
MoveLabelUpCommand::create(Project *project, nanoem_model_label_t *const *labels, nanoem_rsize_t &labelIndex)
{
    MoveLabelUpCommand *command = nanoem_new(MoveLabelUpCommand(project, labels, labelIndex));
    return command->createCommand();
}

MoveLabelUpCommand::MoveLabelUpCommand(
    Project *project, nanoem_model_label_t *const *labels, nanoem_rsize_t &labelIndex)
    : BaseUndoCommand(project)
    , m_labels(labels)
    , m_labelIndex(labelIndex)
{
}

MoveLabelUpCommand::~MoveLabelUpCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveLabelUpCommand::undo(Error &error)
{
}

void
MoveLabelUpCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableLabel label(m_labels[m_labelIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveLabelObject(model, label, &status);
    int offset = Inline::saturateInt32(--m_labelIndex);
    nanoemMutableModelInsertLabelObject(model, label, offset, &status);
    project->rebuildAllTracks();
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
MoveLabelDownCommand::create(Project *project, nanoem_model_label_t *const *labels, nanoem_rsize_t &labelIndex)
{
    MoveLabelDownCommand *command = nanoem_new(MoveLabelDownCommand(project, labels, labelIndex));
    return command->createCommand();
}

MoveLabelDownCommand::MoveLabelDownCommand(
    Project *project, nanoem_model_label_t *const *labels, nanoem_rsize_t &labelIndex)
    : BaseUndoCommand(project)
    , m_labels(labels)
    , m_labelIndex(labelIndex)
{
}

MoveLabelDownCommand::~MoveLabelDownCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveLabelDownCommand::undo(Error &error)
{
}

void
MoveLabelDownCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableLabel label(m_labels[m_labelIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveLabelObject(model, label, &status);
    int offset = Inline::saturateInt32(++m_labelIndex);
    nanoemMutableModelInsertLabelObject(model, label, offset, &status);
    project->rebuildAllTracks();
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
CreateRigidBodyCommand::create(
    Project *project, nanoem_rsize_t numRigidBodies, int offset, const nanoem_model_rigid_body_t *base)
{
    CreateRigidBodyCommand *command = nanoem_new(CreateRigidBodyCommand(project, numRigidBodies, offset, base));
    return command->createCommand();
}

CreateRigidBodyCommand::CreateRigidBodyCommand(
    Project *project, nanoem_rsize_t numRigidBodies, int offset, const nanoem_model_rigid_body_t *base)
    : BaseUndoCommand(project)
    , m_base(base)
    , m_numRigidBodies(numRigidBodies)
    , m_offset(offset)
{
}

CreateRigidBodyCommand::~CreateRigidBodyCommand() NANOEM_DECL_NOEXCEPT
{
}

void
CreateRigidBodyCommand::undo(Error &error)
{
}

void
CreateRigidBodyCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableModel model(activeModel);
    ScopedMutableRigidBody body(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    char buffer[Inline::kMarkerStringLength];
    if (m_base) {
        nanoemMutableModelRigidBodyCopy(body, m_base, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, m_numRigidBodies + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelRigidBodySetName(body, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "NewRigidBody%zu", m_numRigidBodies + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelRigidBodySetName(body, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    nanoemMutableModelInsertRigidBodyObject(model, body, m_offset, &status);
    model::RigidBody *newBody = model::RigidBody::create();
    model::RigidBody::Resolver resolver;
    newBody->bind(nanoemMutableModelRigidBodyGetOriginObject(body), nullptr, false, resolver);
    newBody->resetLanguage(nanoemMutableModelRigidBodyGetOriginObject(body), factory, project->castLanguage());
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
DeleteRigidBodyCommand::create(
    Project *project, nanoem_model_rigid_body_t *const *rigidBodies, nanoem_rsize_t &rigidBodyIndex)
{
    DeleteRigidBodyCommand *command = nanoem_new(DeleteRigidBodyCommand(project, rigidBodies, rigidBodyIndex));
    return command->createCommand();
}

DeleteRigidBodyCommand::DeleteRigidBodyCommand(
    Project *project, nanoem_model_rigid_body_t *const *rigidBodies, nanoem_rsize_t &rigidBodyIndex)
    : BaseUndoCommand(project)
    , m_rigidBodies(rigidBodies)
    , m_rigidBodyIndex(rigidBodyIndex)
{
}

DeleteRigidBodyCommand::~DeleteRigidBodyCommand() NANOEM_DECL_NOEXCEPT
{
}

void
DeleteRigidBodyCommand::undo(Error &error)
{
}

void
DeleteRigidBodyCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableRigidBody rigid_body(m_rigidBodies[m_rigidBodyIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveRigidBodyObject(model, rigid_body, &status);
    if (m_rigidBodyIndex > 0) {
        m_rigidBodyIndex--;
    }
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

undo_command_t *
MoveRigidBodyUpCommand::create(
    Project *project, nanoem_model_rigid_body_t *const *rigidBodies, nanoem_rsize_t &rigidBodyIndex)
{
    MoveRigidBodyUpCommand *command = nanoem_new(MoveRigidBodyUpCommand(project, rigidBodies, rigidBodyIndex));
    return command->createCommand();
}

MoveRigidBodyUpCommand::MoveRigidBodyUpCommand(
    Project *project, nanoem_model_rigid_body_t *const *rigidBodies, nanoem_rsize_t &rigidBodyIndex)
    : BaseUndoCommand(project)
    , m_rigidBodies(rigidBodies)
    , m_rigidBodyIndex(rigidBodyIndex)
{
}

MoveRigidBodyUpCommand::~MoveRigidBodyUpCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveRigidBodyUpCommand::undo(Error &error)
{
}

void
MoveRigidBodyUpCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableRigidBody rigid_body(m_rigidBodies[m_rigidBodyIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveRigidBodyObject(model, rigid_body, &status);
    int offset = Inline::saturateInt32(--m_rigidBodyIndex);
    nanoemMutableModelInsertRigidBodyObject(model, rigid_body, offset, &status);
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
MoveRigidBodyDownCommand::create(
    Project *project, nanoem_model_rigid_body_t *const *rigidBodies, nanoem_rsize_t &rigidBodyIndex)
{
    MoveRigidBodyDownCommand *command = nanoem_new(MoveRigidBodyDownCommand(project, rigidBodies, rigidBodyIndex));
    return command->createCommand();
}

MoveRigidBodyDownCommand::MoveRigidBodyDownCommand(
    Project *project, nanoem_model_rigid_body_t *const *rigidBodies, nanoem_rsize_t &rigidBodyIndex)
    : BaseUndoCommand(project)
    , m_rigidBodies(rigidBodies)
    , m_rigidBodyIndex(rigidBodyIndex)
{
}

MoveRigidBodyDownCommand::~MoveRigidBodyDownCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveRigidBodyDownCommand::undo(Error &error)
{
}

void
MoveRigidBodyDownCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableRigidBody rigid_body(m_rigidBodies[m_rigidBodyIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveRigidBodyObject(model, rigid_body, &status);
    int offset = Inline::saturateInt32(++m_rigidBodyIndex);
    nanoemMutableModelInsertRigidBodyObject(model, rigid_body, offset, &status);
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
CreateJointCommand::create(Project *project, nanoem_rsize_t numJoints, int offset, const nanoem_model_joint_t *base)
{
    CreateJointCommand *command = nanoem_new(CreateJointCommand(project, numJoints, offset, base));
    return command->createCommand();
}

CreateJointCommand::CreateJointCommand(
    Project *project, nanoem_rsize_t numJoints, int offset, const nanoem_model_joint_t *base)
    : BaseUndoCommand(project)
    , m_base(base)
    , m_numJoints(numJoints)
    , m_offset(offset)
{
}

CreateJointCommand::~CreateJointCommand() NANOEM_DECL_NOEXCEPT
{
}

void
CreateJointCommand::undo(Error &error)
{
}

void
CreateJointCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableJoint joint(activeModel);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    char buffer[Inline::kMarkerStringLength];
    if (m_base) {
        nanoemMutableModelJointCopy(joint, m_base, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, m_numJoints + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelJointSetName(joint, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "NewRigidBody%zu", m_numJoints + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelJointSetName(joint, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    nanoemMutableModelInsertJointObject(model, joint, m_offset, &status);
    model::Joint *newJoint = model::Joint::create();
    model::RigidBody::Resolver resolver;
    newJoint->bind(nanoemMutableModelJointGetOriginObject(joint), nullptr, resolver);
    newJoint->resetLanguage(nanoemMutableModelJointGetOriginObject(joint), factory, project->castLanguage());
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
DeleteJointCommand::create(
    Project *project, nanoem_model_joint_t *const *joints, nanoem_rsize_t &jointIndex)
{
    DeleteJointCommand *command = nanoem_new(DeleteJointCommand(project, joints, jointIndex));
    return command->createCommand();
}

DeleteJointCommand::DeleteJointCommand(
    Project *project, nanoem_model_joint_t *const *joints, nanoem_rsize_t &jointIndex)
    : BaseUndoCommand(project)
    , m_joints(joints)
    , m_jointIndex(jointIndex)
{
}

DeleteJointCommand::~DeleteJointCommand() NANOEM_DECL_NOEXCEPT
{
}

void
DeleteJointCommand::undo(Error &error)
{
}

void
DeleteJointCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableJoint joint(m_joints[m_jointIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveJointObject(model, joint, &status);
    if (m_jointIndex > 0) {
        m_jointIndex--;
    }
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

undo_command_t *
MoveJointUpCommand::create(Project *project, nanoem_model_joint_t *const *joints, nanoem_rsize_t &jointIndex)
{
    MoveJointUpCommand *command = nanoem_new(MoveJointUpCommand(project, joints, jointIndex));
    return command->createCommand();
}

MoveJointUpCommand::MoveJointUpCommand(
    Project *project, nanoem_model_joint_t *const *joints, nanoem_rsize_t &jointIndex)
    : BaseUndoCommand(project)
    , m_joints(joints)
    , m_jointIndex(jointIndex)
{
}

MoveJointUpCommand::~MoveJointUpCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveJointUpCommand::undo(Error &error)
{
}

void
MoveJointUpCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableJoint joint(m_joints[m_jointIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveJointObject(model, joint, &status);
    int offset = Inline::saturateInt32(--m_jointIndex);
    nanoemMutableModelInsertJointObject(model, joint, offset, &status);
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
MoveJointDownCommand::create(Project *project, nanoem_model_joint_t *const *joints, nanoem_rsize_t &jointIndex)
{
    MoveJointDownCommand *command = nanoem_new(MoveJointDownCommand(project, joints, jointIndex));
    return command->createCommand();
}

MoveJointDownCommand::MoveJointDownCommand(
    Project *project, nanoem_model_joint_t *const *joints, nanoem_rsize_t &jointIndex)
    : BaseUndoCommand(project)
    , m_joints(joints)
    , m_jointIndex(jointIndex)
{
}

MoveJointDownCommand::~MoveJointDownCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveJointDownCommand::undo(Error &error)
{
}

void
MoveJointDownCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableJoint joint(m_joints[m_jointIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveJointObject(model, joint, &status);
    int offset = Inline::saturateInt32(++m_jointIndex);
    nanoemMutableModelInsertJointObject(model, joint, offset, &status);
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
CreateSoftBodyCommand::create(
    Project *project, nanoem_rsize_t numSoftBodys, int offset, const nanoem_model_soft_body_t *base)
{
    CreateSoftBodyCommand *command = nanoem_new(CreateSoftBodyCommand(project, numSoftBodys, offset, base));
    return command->createCommand();
}

CreateSoftBodyCommand::CreateSoftBodyCommand(
    Project *project, nanoem_rsize_t numSoftBodys, int offset, const nanoem_model_soft_body_t *base)
    : BaseUndoCommand(project)
    , m_base(base)
    , m_numSoftBodys(numSoftBodys)
    , m_offset(offset)
{
}

CreateSoftBodyCommand::~CreateSoftBodyCommand() NANOEM_DECL_NOEXCEPT
{
}

void
CreateSoftBodyCommand::undo(Error &error)
{
}

void
CreateSoftBodyCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableSoftBody soft_body(activeModel);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    char buffer[Inline::kMarkerStringLength];
    if (m_base) {
        nanoemMutableModelSoftBodyCopy(soft_body, m_base, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, m_numSoftBodys + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelSoftBodySetName(soft_body, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "NewRigidBody%zu", m_numSoftBodys + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelSoftBodySetName(soft_body, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    nanoemMutableModelInsertSoftBodyObject(model, soft_body, m_offset, &status);
    model::SoftBody *newSoftBody = model::SoftBody::create();
    model::RigidBody::Resolver resolver;
    newSoftBody->bind(nanoemMutableModelSoftBodyGetOriginObject(soft_body), nullptr, resolver);
    newSoftBody->resetLanguage(nanoemMutableModelSoftBodyGetOriginObject(soft_body), factory, project->castLanguage());
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
DeleteSoftBodyCommand::create(
    Project *project, nanoem_model_soft_body_t *const *softBodies, nanoem_rsize_t &soft_bodyIndex)
{
    DeleteSoftBodyCommand *command = nanoem_new(DeleteSoftBodyCommand(project, softBodies, soft_bodyIndex));
    return command->createCommand();
}

DeleteSoftBodyCommand::DeleteSoftBodyCommand(
    Project *project, nanoem_model_soft_body_t *const *softBodies, nanoem_rsize_t &soft_bodyIndex)
    : BaseUndoCommand(project)
    , m_softBodies(softBodies)
    , m_softBodyIndex(soft_bodyIndex)
{
}

DeleteSoftBodyCommand::~DeleteSoftBodyCommand() NANOEM_DECL_NOEXCEPT
{
}

void
DeleteSoftBodyCommand::undo(Error &error)
{
}

void
DeleteSoftBodyCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableSoftBody soft_body(m_softBodies[m_softBodyIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveSoftBodyObject(model, soft_body, &status);
    if (m_softBodyIndex > 0) {
        m_softBodyIndex--;
    }
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

undo_command_t *
MoveSoftBodyUpCommand::create(
    Project *project, nanoem_model_soft_body_t *const *softBodies, nanoem_rsize_t &soft_bodyIndex)
{
    MoveSoftBodyUpCommand *command = nanoem_new(MoveSoftBodyUpCommand(project, softBodies, soft_bodyIndex));
    return command->createCommand();
}

MoveSoftBodyUpCommand::MoveSoftBodyUpCommand(
    Project *project, nanoem_model_soft_body_t *const *softBodies, nanoem_rsize_t &soft_bodyIndex)
    : BaseUndoCommand(project)
    , m_softBodies(softBodies)
    , m_softBodyIndex(soft_bodyIndex)
{
}

MoveSoftBodyUpCommand::~MoveSoftBodyUpCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveSoftBodyUpCommand::undo(Error &error)
{
}

void
MoveSoftBodyUpCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableSoftBody soft_body(m_softBodies[m_softBodyIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveSoftBodyObject(model, soft_body, &status);
    int offset = Inline::saturateInt32(--m_softBodyIndex);
    nanoemMutableModelInsertSoftBodyObject(model, soft_body, offset, &status);
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
MoveSoftBodyDownCommand::create(
    Project *project, nanoem_model_soft_body_t *const *softBodies, nanoem_rsize_t &soft_bodyIndex)
{
    MoveSoftBodyDownCommand *command = nanoem_new(MoveSoftBodyDownCommand(project, softBodies, soft_bodyIndex));
    return command->createCommand();
}

MoveSoftBodyDownCommand::MoveSoftBodyDownCommand(
    Project *project, nanoem_model_soft_body_t *const *softBodies, nanoem_rsize_t &soft_bodyIndex)
    : BaseUndoCommand(project)
    , m_softBodies(softBodies)
    , m_softBodyIndex(soft_bodyIndex)
{
}

MoveSoftBodyDownCommand::~MoveSoftBodyDownCommand() NANOEM_DECL_NOEXCEPT
{
}

void
MoveSoftBodyDownCommand::undo(Error &error)
{
}

void
MoveSoftBodyDownCommand::redo(Error &error)
{
    Project *project = currentProject();
    Model *activeModel = project->activeModel();
    ScopedMutableSoftBody softBody(m_softBodies[m_softBodyIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveSoftBodyObject(model, softBody, &status);
    int offset = Inline::saturateInt32(++m_softBodyIndex);
    nanoemMutableModelInsertSoftBodyObject(model, softBody, offset, &status);
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

} /* namespace command */
} /* namespace nanoem */
