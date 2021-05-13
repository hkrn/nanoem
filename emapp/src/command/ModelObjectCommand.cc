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

undo_command_t *
DeleteMaterialCommand::create(Project *project, nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex)
{
    DeleteMaterialCommand *command = nanoem_new(DeleteMaterialCommand(project, materials, materialIndex));
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
    const BaseMoveMaterialCommand::LayoutPosition &to, Model *activeModel, nanoem_status_t *status)
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
    nanoemMutableModelSetVertexIndices(model, workingBuffer.data(), numIndices, status);
    nanoemMutableModelRemoveMaterialObject(model, material, status);
    nanoemMutableModelInsertMaterialObject(model, material, destination, status);
}

undo_command_t *
MoveMaterialUpCommand::create(
    Project *project, nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex)
{
    MoveMaterialUpCommand *command = nanoem_new(MoveMaterialUpCommand(project, materials, materialIndex));
    return command->createCommand();
}

MoveMaterialUpCommand::MoveMaterialUpCommand(
    Project *project, nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex)
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
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    move(destination, from, to, activeModel, &status);
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
    LayoutPosition from = { 0, 0 }, to = { 0, 0 };
    int destination = 0;
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *currentMaterialPtr = materials[i];
        size_t size = nanoemModelMaterialGetNumVertexIndices(currentMaterialPtr);
        if (currentMaterialPtr == activeMaterial) {
            const nanoem_model_material_t *nextMaterialPtr = materials[i + 1];
            destination = Inline::saturateInt32(i + 1);
            to.m_size = nanoemModelMaterialGetNumVertexIndices(nextMaterialPtr);
            to.m_offset = offset + to.m_size;
            from.m_offset = offset;
            from.m_size = size;
            break;
        }
        offset += size;
    }
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    move(destination, from, to, activeModel, &status);
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
CreateBoneCommand::create(Project *project, nanoem_rsize_t numBones, int offset, const nanoem_model_bone_t *base)
{
    CreateBoneCommand *command = nanoem_new(CreateBoneCommand(project, numBones, offset, base));
    return command->createCommand();
}

void
CreateBoneCommand::setup(nanoem_model_bone_t *bonePtr, Project *project)
{
    model::Bone *newBone = model::Bone::create();
    newBone->bind(bonePtr);
    newBone->resetLanguage(bonePtr, project->unicodeStringFactory(), project->castLanguage());
}

CreateBoneCommand::CreateBoneCommand(
    Project *project, nanoem_rsize_t numBones, int offset, const nanoem_model_bone_t *base)
    : BaseUndoCommand(project)
    , m_base(base)
    , m_offset(offset)
    , m_activeModel(project->activeModel())
    , m_mutableBone(m_activeModel)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    char buffer[Inline::kMarkerStringLength];
    if (m_base) {
        nanoemMutableModelBoneCopy(m_mutableBone, m_base, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, numBones + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelBoneSetName(m_mutableBone, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "NewBone%zu", numBones + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelBoneSetName(m_mutableBone, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    setup(nanoemMutableModelBoneGetOriginObject(m_mutableBone), project);
}

CreateBoneCommand::~CreateBoneCommand() NANOEM_DECL_NOEXCEPT
{
    m_activeModel = nullptr;
}

void
CreateBoneCommand::undo(Error &error)
{
    const nanoem_model_bone_t *bonePtr = nanoemMutableModelBoneGetOriginObject(m_mutableBone);
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveBoneObject(model, m_mutableBone, &status);
    m_activeModel->removeBoneReference(bonePtr);
    assignError(status, error);
}

void
CreateBoneCommand::redo(Error &error)
{
    const nanoem_model_bone_t *bonePtr = nanoemMutableModelBoneGetOriginObject(m_mutableBone);
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelInsertBoneObject(model, m_mutableBone, m_offset, &status);
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
CreateBoneAsStagingParentCommand::create(
    Project *project, const nanoem_model_bone_t *base)
{
    CreateBoneAsStagingParentCommand *command = nanoem_new(CreateBoneAsStagingParentCommand(project, base));
    return command->createCommand();
}

void
CreateBoneAsStagingParentCommand::setNameSuffix(nanoem_mutable_model_bone_t *bone, const char *suffix, nanoem_unicode_string_factory_t *factory, nanoem_status_t *status)
{
    setNameSuffix(bone, suffix, NANOEM_LANGUAGE_TYPE_JAPANESE, factory, status);
    setNameSuffix(bone, suffix, NANOEM_LANGUAGE_TYPE_ENGLISH, factory, status);
}

void
CreateBoneAsStagingParentCommand::setNameSuffix(nanoem_mutable_model_bone_t *bone, const char *suffix, nanoem_language_type_t language, nanoem_unicode_string_factory_t *factory, nanoem_status_t *status)
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

CreateBoneAsStagingParentCommand::CreateBoneAsStagingParentCommand(Project *project, const nanoem_model_bone_t *base)
    : BaseUndoCommand(project)
    , m_parent(nanoemModelBoneGetParentBoneObject(base))
    , m_activeModel(project->activeModel())
    , m_mutableBone(m_activeModel)
    , m_boneIndex(model::Bone::index(base) + 1)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    nanoemMutableModelBoneCopy(m_mutableBone, base, &status);
    nanoemMutableModelBoneSetParentBoneObject(m_mutableBone, base);
    setNameSuffix(m_mutableBone, "+", factory, &status);
    CreateBoneCommand::setup(nanoemMutableModelBoneGetOriginObject(m_mutableBone), project);
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        nanoem_model_bone_t *bonePtr = bones[i];
        if (nanoemModelBoneGetParentBoneObject(bonePtr) == base) {
            nanoem_mutable_model_bone_t *mutableBone = nanoemMutableModelBoneCreateAsReference(bonePtr, &status);
            m_bones.push_back(mutableBone);
            break;
        }
    }
}

CreateBoneAsStagingParentCommand::~CreateBoneAsStagingParentCommand() NANOEM_DECL_NOEXCEPT
{
    for (BoneList::const_iterator it = m_bones.begin(), end = m_bones.end(); it != end; ++it) {
        nanoemMutableModelBoneDestroy(*it);
    }
    m_bones.clear();
}

void
CreateBoneAsStagingParentCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel);
    m_activeModel->removeBoneReference(nanoemMutableModelBoneGetOriginObject(m_mutableBone));
    nanoemMutableModelRemoveBoneObject(model, m_mutableBone, &status);
    for (BoneList::const_iterator it = m_bones.begin(), end = m_bones.end(); it != end; ++it) {
        nanoemMutableModelBoneSetParentBoneObject(*it, m_parent);
    }
    assignError(status, error);
}

void
CreateBoneAsStagingParentCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ScopedMutableModel model(m_activeModel);
    nanoemMutableModelInsertBoneObject(model, m_mutableBone, m_boneIndex, &status);
    const nanoem_model_bone_t *origin = nanoemMutableModelBoneGetOriginObject(m_mutableBone);
    for (BoneList::const_iterator it = m_bones.begin(), end = m_bones.end(); it != end; ++it) {
        nanoemMutableModelBoneSetParentBoneObject(*it, origin);
    }
    m_activeModel->addBoneReference(nanoemMutableModelBoneGetOriginObject(m_mutableBone));
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
DeleteBoneCommand::create(Project *project, nanoem_rsize_t boneIndex)
{
    DeleteBoneCommand *command = nanoem_new(DeleteBoneCommand(project, boneIndex));
    return command->createCommand();
}

DeleteBoneCommand::DeleteBoneCommand(Project *project, nanoem_rsize_t &boneIndex)
    : BaseUndoCommand(project)
    , m_boneIndex(boneIndex)
    , m_activeModel(project->activeModel())
{
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
    nanoem_model_bone_t *bonePtr = bones[m_boneIndex];
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_mutableBone = nanoemMutableModelBoneCreateAsReference(bonePtr, &status);
}

DeleteBoneCommand::~DeleteBoneCommand() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelBoneDestroy(m_mutableBone);
    m_mutableBone = nullptr;
    m_activeModel = nullptr;
}

void
DeleteBoneCommand::undo(Error &error)
{
    const nanoem_model_bone_t *bonePtr = nanoemMutableModelBoneGetOriginObject(m_mutableBone);
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelInsertBoneObject(model, m_mutableBone, Inline::saturateInt32(m_boneIndex), &status);
    m_activeModel->addBoneReference(bonePtr);
    assignError(status, error);
}

void
DeleteBoneCommand::redo(Error &error)
{
    const nanoem_model_bone_t *bonePtr = nanoemMutableModelBoneGetOriginObject(m_mutableBone);
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveBoneObject(model, m_mutableBone, &status);
    m_activeModel->removeBoneReference(bonePtr);
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

BaseMoveBoneCommand::BaseMoveBoneCommand(Project *project, nanoem_rsize_t fromBoneIndex, nanoem_rsize_t toBoneIndex)
    : BaseUndoCommand(project)
    , m_fromBoneIndex(fromBoneIndex)
    , m_toBoneIndex(toBoneIndex)
    , m_activeModel(project->activeModel())
{
}

void
BaseMoveBoneCommand::move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error)
{
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
    ScopedMutableBone bone(bones[fromIndex]);
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveBoneObject(model, bone, &status);
    int index = Inline::saturateInt32(glm::min(toIndex, numBones));
    nanoemMutableModelInsertBoneObject(model, bone, index, &status);
    assignError(status, error);
}

undo_command_t *
MoveBoneDownCommand::create(Project *project, nanoem_rsize_t boneIndex)
{
    MoveBoneDownCommand *command = nanoem_new(MoveBoneDownCommand(project, boneIndex));
    return command->createCommand();
}

MoveBoneDownCommand::MoveBoneDownCommand(Project *project, nanoem_rsize_t boneIndex)
    : BaseMoveBoneCommand(project, boneIndex, boneIndex + 1)
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
MoveBoneUpCommand::create(Project *project, nanoem_rsize_t boneIndex)
{
    MoveBoneUpCommand *command = nanoem_new(MoveBoneUpCommand(project, boneIndex));
    return command->createCommand();
}

MoveBoneUpCommand::MoveBoneUpCommand(Project *project, nanoem_rsize_t boneIndex)
    : BaseMoveBoneCommand(project, boneIndex, boneIndex > 0 ? boneIndex - 1 : 0)
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
    , m_offset(offset)
    , m_activeModel(project->activeModel())
    , m_mutableMorph(m_activeModel)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    char buffer[Inline::kMarkerStringLength];
    if (m_base) {
        nanoemMutableModelMorphCopy(m_mutableMorph, m_base, &status);
    }
    else {
        nanoemMutableModelMorphSetType(m_mutableMorph, m_type);
    }
    StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, numMorphs + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelMorphSetName(m_mutableMorph, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "NewMorph%zu", numMorphs + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelMorphSetName(m_mutableMorph, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    model::Morph *newMorph = model::Morph::create();
    newMorph->bind(nanoemMutableModelMorphGetOriginObject(m_mutableMorph));
    newMorph->resetLanguage(nanoemMutableModelMorphGetOriginObject(m_mutableMorph), factory, project->castLanguage());
}

CreateMorphCommand::~CreateMorphCommand() NANOEM_DECL_NOEXCEPT
{
    m_activeModel = nullptr;
}

void
CreateMorphCommand::undo(Error &error)
{
    const nanoem_model_morph_t *morphPtr = nanoemMutableModelMorphGetOriginObject(m_mutableMorph);
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveMorphObject(model, m_mutableMorph, &status);
    m_activeModel->removeMorphReference(morphPtr);
    assignError(status, error);
}

void
CreateMorphCommand::redo(Error &error)
{
    const nanoem_model_morph_t *morphPtr = nanoemMutableModelMorphGetOriginObject(m_mutableMorph);
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelInsertMorphObject(model, m_mutableMorph, m_offset, &status);
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
CreateBoneMorphFromPoseCommand::create(Project *project, const model::BindPose::BoneTransformMap &transforms, const model::BindPose::MorphWeightMap &weights, const String &filename)
{
    CreateBoneMorphFromPoseCommand *command = nanoem_new(CreateBoneMorphFromPoseCommand(project, transforms, weights, filename));
    return command->createCommand();
}

CreateBoneMorphFromPoseCommand::CreateBoneMorphFromPoseCommand(Project *project, const model::BindPose::BoneTransformMap &transforms, const model::BindPose::MorphWeightMap &weights, const String &filename)
    : BaseUndoCommand(project)
    , m_activeModel(project->activeModel())
    , m_mutableMorph(m_activeModel)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope us(factory);
    if (StringUtils::tryGetString(factory, filename, us)) {
        nanoemMutableModelMorphSetName(
            m_mutableMorph, us.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
        nanoemMutableModelMorphSetName(
            m_mutableMorph, us.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    nanoemMutableModelMorphSetType(m_mutableMorph, NANOEM_MODEL_MORPH_TYPE_BONE);
    nanoemMutableModelMorphSetCategory(m_mutableMorph, NANOEM_MODEL_MORPH_CATEGORY_OTHER);
    for (model::BindPose::BoneTransformMap::const_iterator it = transforms.begin(),
                                                           end = transforms.end();
         it != end; ++it) {
        if (const nanoem_model_bone_t *bonePtr = m_activeModel->findBone(it->first)) {
            const Vector3 &translation = it->second.first;
            const Quaternion &orientation = it->second.second;
            if (glm::all(glm::epsilonNotEqual(
                    translation, Constants::kZeroV3, Constants::kEpsilon)) ||
                glm::all(glm::epsilonNotEqual(
                    orientation, Constants::kZeroQ, Constants::kEpsilon))) {
                command::ScopedMutableMorphBone scoped(m_mutableMorph);
                nanoemMutableModelMorphBoneSetBoneObject(scoped, bonePtr);
                nanoemMutableModelMorphBoneSetTranslation(
                    scoped, glm::value_ptr(Vector4(translation, 0)));
                nanoemMutableModelMorphBoneSetOrientation(scoped, glm::value_ptr(orientation));
                nanoemMutableModelMorphInsertBoneMorphObject(m_mutableMorph, scoped, -1, &status);
            }
        }
    }
    nanoem_model_morph_t *morphPtr = nanoemMutableModelMorphGetOriginObject(m_mutableMorph);
    model::Morph *morph = model::Morph::create();
    morph->bind(morphPtr);
    morph->resetLanguage(morphPtr, factory, project->castLanguage());
}

CreateBoneMorphFromPoseCommand::~CreateBoneMorphFromPoseCommand() NANOEM_DECL_NOEXCEPT
{
}

void
CreateBoneMorphFromPoseCommand::undo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    command::ScopedMutableModel m(m_activeModel);
    m_activeModel->removeMorphReference(nanoemMutableModelMorphGetOriginObject(m_mutableMorph));
    nanoemMutableModelRemoveMorphObject(m, m_mutableMorph, &status);
    assignError(status, error);
}

void
CreateBoneMorphFromPoseCommand::redo(Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    command::ScopedMutableModel m(m_activeModel);
    nanoemMutableModelInsertMorphObject(m, m_mutableMorph, -1, &status);
    m_activeModel->addMorphReference(nanoemMutableModelMorphGetOriginObject(m_mutableMorph));
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
DeleteMorphCommand::create(Project *project, nanoem_rsize_t morphIndex)
{
    DeleteMorphCommand *command = nanoem_new(DeleteMorphCommand(project, morphIndex));
    return command->createCommand();
}

DeleteMorphCommand::DeleteMorphCommand(
    Project *project, nanoem_rsize_t morphIndex)
    : BaseUndoCommand(project)
    , m_morphIndex(morphIndex)
    , m_activeModel(project->activeModel())
{
    nanoem_rsize_t numMorphs;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_activeModel->data(), &numMorphs);
    nanoem_model_morph_t *morphPtr = morphs[m_morphIndex];
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_mutableMorph = nanoemMutableModelMorphCreateAsReference(morphPtr, &status);
}

DeleteMorphCommand::~DeleteMorphCommand() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelMorphDestroy(m_mutableMorph);
    m_mutableMorph = nullptr;
    m_activeModel = nullptr;
}

void
DeleteMorphCommand::undo(Error &error)
{
    const nanoem_model_morph_t *morphPtr = nanoemMutableModelMorphGetOriginObject(m_mutableMorph);
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelInsertMorphObject(model, m_mutableMorph, Inline::saturateInt32(m_morphIndex), &status);
    m_activeModel->addMorphReference(morphPtr);
    assignError(status, error);
}

void
DeleteMorphCommand::redo(Error &error)
{
    const nanoem_model_morph_t *morphPtr = nanoemMutableModelMorphGetOriginObject(m_mutableMorph);
    ScopedMutableModel model(m_activeModel);
    for (int i = NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM; i < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM; i++) {
        nanoem_model_morph_category_t category = static_cast<nanoem_model_morph_category_t>(i);
        if (m_activeModel->activeMorph(category) == morphPtr) {
            m_activeModel->setActiveMorph(category, nullptr);
        }
    }
    m_activeModel->removeMorphReference(morphPtr);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveMorphObject(model, m_mutableMorph, &status);
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

BaseMoveMorphCommand::BaseMoveMorphCommand(Project *project, nanoem_rsize_t fromMorphIndex, nanoem_rsize_t toMorphIndex)
    : BaseUndoCommand(project)
    , m_fromMorphIndex(fromMorphIndex)
    , m_toMorphIndex(toMorphIndex)
    , m_activeModel(project->activeModel())
{
}

void
BaseMoveMorphCommand::move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error)
{
    nanoem_rsize_t numMorphs;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_activeModel->data(), &numMorphs);
    ScopedMutableMorph morph(morphs[fromIndex]);
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveMorphObject(model, morph, &status);
    int index = Inline::saturateInt32(glm::min(toIndex, numMorphs));
    nanoemMutableModelInsertMorphObject(model, morph, index, &status);
    assignError(status, error);
}

undo_command_t *
MoveMorphUpCommand::create(Project *project, nanoem_rsize_t morphIndex)
{
    MoveMorphUpCommand *command = nanoem_new(MoveMorphUpCommand(project, morphIndex));
    return command->createCommand();
}

MoveMorphUpCommand::MoveMorphUpCommand(
    Project *project, nanoem_rsize_t morphIndex)
    : BaseMoveMorphCommand(project, morphIndex, morphIndex > 0 ? morphIndex - 1 : 0)
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
MoveMorphDownCommand::create(Project *project, nanoem_rsize_t morphIndex)
{
    MoveMorphDownCommand *command = nanoem_new(MoveMorphDownCommand(project, morphIndex));
    return command->createCommand();
}

MoveMorphDownCommand::MoveMorphDownCommand(
    Project *project, nanoem_rsize_t morphIndex)
    : BaseMoveMorphCommand(project, morphIndex, morphIndex + 1)
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
CreateLabelCommand::create(Project *project, nanoem_rsize_t numLabels, int offset, const nanoem_model_label_t *base)
{
    CreateLabelCommand *command = nanoem_new(CreateLabelCommand(project, numLabels, offset, base));
    return command->createCommand();
}

CreateLabelCommand::CreateLabelCommand(
    Project *project, nanoem_rsize_t numLabels, int offset, const nanoem_model_label_t *base)
    : BaseUndoCommand(project)
    , m_base(base)
    , m_offset(offset)
    , m_activeModel(project->activeModel())
    , m_mutableLabel(m_activeModel)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    char buffer[Inline::kMarkerStringLength];
    if (m_base) {
        nanoemMutableModelLabelCopy(m_mutableLabel, m_base, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, numLabels + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelLabelSetName(m_mutableLabel, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "NewLabel%zu", numLabels + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelLabelSetName(m_mutableLabel, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    model::Label *newLabel = model::Label::create();
    newLabel->bind(nanoemMutableModelLabelGetOriginObject(m_mutableLabel));
    newLabel->resetLanguage(nanoemMutableModelLabelGetOriginObject(m_mutableLabel), factory, project->castLanguage());
}

CreateLabelCommand::~CreateLabelCommand() NANOEM_DECL_NOEXCEPT
{
    m_activeModel = nullptr;
}

void
CreateLabelCommand::undo(Error &error)
{
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveLabelObject(model, m_mutableLabel, &status);
    currentProject()->rebuildAllTracks();
    assignError(status, error);
}

void
CreateLabelCommand::redo(Error &error)
{
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelInsertLabelObject(model, m_mutableLabel, m_offset, &status);
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

undo_command_t *
DeleteLabelCommand::create(Project *project, nanoem_rsize_t labelIndex)
{
    DeleteLabelCommand *command = nanoem_new(DeleteLabelCommand(project, labelIndex));
    return command->createCommand();
}

DeleteLabelCommand::DeleteLabelCommand(
    Project *project,  nanoem_rsize_t labelIndex)
    : BaseUndoCommand(project)
    , m_labelIndex(labelIndex)
    , m_activeModel(project->activeModel())
{
    nanoem_rsize_t numLabels;
    nanoem_model_label_t *const *labels = nanoemModelGetAllLabelObjects(m_activeModel->data(), &numLabels);
    nanoem_model_label_t *labelPtr = labels[m_labelIndex];
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_mutableLabel = nanoemMutableModelLabelCreateAsReference(labelPtr, &status);
}

DeleteLabelCommand::~DeleteLabelCommand() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelLabelDestroy(m_mutableLabel);
    m_mutableLabel = nullptr;
    m_activeModel = nullptr;
}

void
DeleteLabelCommand::undo(Error &error)
{
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelInsertLabelObject(model, m_mutableLabel, Inline::saturateInt32(m_labelIndex), &status);
    currentProject()->rebuildAllTracks();
    assignError(status, error);
}

void
DeleteLabelCommand::redo(Error &error)
{
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveLabelObject(model, m_mutableLabel, &status);
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
    Project *project, nanoem_rsize_t fromLabelIndex, nanoem_rsize_t toLabelIndex)
    : BaseUndoCommand(project)
    , m_fromLabelIndex(fromLabelIndex)
    , m_toLabelIndex(toLabelIndex)
    , m_activeModel(project->activeModel())
{
}

void
BaseMoveLabelCommand::move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error)
{
    nanoem_rsize_t numLabels;
    nanoem_model_label_t *const *labels = nanoemModelGetAllLabelObjects(m_activeModel->data(), &numLabels);
    ScopedMutableLabel label(labels[fromIndex]);
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveLabelObject(model, label, &status);
    int index = Inline::saturateInt32(glm::min(toIndex, numLabels));
    nanoemMutableModelInsertLabelObject(model, label, index, &status);
    currentProject()->rebuildAllTracks();
    assignError(status, error);
}

undo_command_t *
MoveLabelUpCommand::create(Project *project, nanoem_rsize_t labelIndex)
{
    MoveLabelUpCommand *command = nanoem_new(MoveLabelUpCommand(project, labelIndex));
    return command->createCommand();
}

MoveLabelUpCommand::MoveLabelUpCommand(
    Project *project, nanoem_rsize_t labelIndex)
    : BaseMoveLabelCommand(project, labelIndex, labelIndex > 0 ? labelIndex - 1 : 0)
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
MoveLabelDownCommand::create(Project *project, nanoem_rsize_t labelIndex)
{
    MoveLabelDownCommand *command = nanoem_new(MoveLabelDownCommand(project, labelIndex));
    return command->createCommand();
}

MoveLabelDownCommand::MoveLabelDownCommand(Project *project, nanoem_rsize_t labelIndex)
    : BaseMoveLabelCommand(project, labelIndex, labelIndex + 1)
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
    , m_offset(offset)
    , m_activeModel(project->activeModel())
    , m_mutableRigidBody(m_activeModel)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    char buffer[Inline::kMarkerStringLength];
    if (m_base) {
        nanoemMutableModelRigidBodyCopy(m_mutableRigidBody, m_base, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, numRigidBodies + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelRigidBodySetName(m_mutableRigidBody, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "NewRigidBody%zu", numRigidBodies + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelRigidBodySetName(m_mutableRigidBody, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    model::RigidBody *newBody = model::RigidBody::create();
    model::RigidBody::Resolver resolver;
    nanoem_model_rigid_body_t *rigidBodyPtr = nanoemMutableModelRigidBodyGetOriginObject(m_mutableRigidBody);
    newBody->bind(rigidBodyPtr, nullptr, false, resolver);
    newBody->resetLanguage(rigidBodyPtr, factory, project->castLanguage());
}

CreateRigidBodyCommand::~CreateRigidBodyCommand() NANOEM_DECL_NOEXCEPT
{
    m_activeModel = nullptr;
}

void
CreateRigidBodyCommand::undo(Error &error)
{
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveRigidBodyObject(model, m_mutableRigidBody, &status);
    assignError(status, error);
}

void
CreateRigidBodyCommand::redo(Error &error)
{
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelInsertRigidBodyObject(model, m_mutableRigidBody, m_offset, &status);
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
DeleteRigidBodyCommand::create(
    Project *project, nanoem_rsize_t rigidBodyIndex)
{
    DeleteRigidBodyCommand *command = nanoem_new(DeleteRigidBodyCommand(project, rigidBodyIndex));
    return command->createCommand();
}

DeleteRigidBodyCommand::DeleteRigidBodyCommand(
    Project *project, nanoem_rsize_t rigidBodyIndex)
    : BaseUndoCommand(project)
    , m_rigidBodyIndex(rigidBodyIndex)
    , m_activeModel(project->activeModel())
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t numRigidBodies;
    nanoem_model_rigid_body_t *const *rigidBodies = nanoemModelGetAllRigidBodyObjects(m_activeModel->data(), &numRigidBodies);
    m_mutableRigidBody = nanoemMutableModelRigidBodyCreateAsReference(rigidBodies[m_rigidBodyIndex], &status);
}

DeleteRigidBodyCommand::~DeleteRigidBodyCommand() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelRigidBodyDestroy(m_mutableRigidBody);
    m_mutableRigidBody = nullptr;
    m_activeModel = nullptr;
}

void
DeleteRigidBodyCommand::undo(Error &error)
{
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelInsertRigidBodyObject(model, m_mutableRigidBody, Inline::saturateInt32(m_rigidBodyIndex), &status);
    assignError(status, error);
}

void
DeleteRigidBodyCommand::redo(Error &error)
{
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveRigidBodyObject(model, m_mutableRigidBody, &status);
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
    Project *project, nanoem_rsize_t fromRigidBodyIndex, nanoem_rsize_t toRigidBodyIndex)
    : BaseUndoCommand(project)
    , m_fromRigidBodyIndex(fromRigidBodyIndex)
    , m_toRigidBodyIndex(toRigidBodyIndex)
    , m_activeModel(project->activeModel())
{
}

void
BaseMoveRigidBodyCommand::move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error)
{
    nanoem_rsize_t numRigidBodies;
    nanoem_model_rigid_body_t *const *rigidBodies =
        nanoemModelGetAllRigidBodyObjects(m_activeModel->data(), &numRigidBodies);
    ScopedMutableRigidBody rigidBody(rigidBodies[fromIndex]);
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveRigidBodyObject(model, rigidBody, &status);
    int index = Inline::saturateInt32(glm::min(toIndex, numRigidBodies));
    nanoemMutableModelInsertRigidBodyObject(model, rigidBody, index, &status);
    assignError(status, error);
}

undo_command_t *
MoveRigidBodyUpCommand::create(
    Project *project, nanoem_rsize_t rigidBodyIndex)
{
    MoveRigidBodyUpCommand *command = nanoem_new(MoveRigidBodyUpCommand(project, rigidBodyIndex));
    return command->createCommand();
}

MoveRigidBodyUpCommand::MoveRigidBodyUpCommand(
    Project *project, nanoem_rsize_t rigidBodyIndex)
    : BaseMoveRigidBodyCommand(project, rigidBodyIndex, rigidBodyIndex > 0 ? rigidBodyIndex - 1 : 0)
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
MoveRigidBodyDownCommand::create(
    Project *project, nanoem_rsize_t rigidBodyIndex)
{
    MoveRigidBodyDownCommand *command = nanoem_new(MoveRigidBodyDownCommand(project, rigidBodyIndex));
    return command->createCommand();
}

MoveRigidBodyDownCommand::MoveRigidBodyDownCommand(
    Project *project, nanoem_rsize_t rigidBodyIndex)
    : BaseMoveRigidBodyCommand(project, rigidBodyIndex, rigidBodyIndex + 1)
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
CreateJointCommand::create(Project *project, nanoem_rsize_t numJoints, int offset, const nanoem_model_joint_t *base)
{
    CreateJointCommand *command = nanoem_new(CreateJointCommand(project, numJoints, offset, base));
    return command->createCommand();
}

CreateJointCommand::CreateJointCommand(
    Project *project, nanoem_rsize_t numJoints, int offset, const nanoem_model_joint_t *base)
    : BaseUndoCommand(project)
    , m_base(base)
    , m_offset(offset)
    , m_activeModel(project->activeModel())
    , m_mutableJoint(m_activeModel)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    char buffer[Inline::kMarkerStringLength];
    if (m_base) {
        nanoemMutableModelJointCopy(m_mutableJoint, m_base, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, numJoints + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelJointSetName(m_mutableJoint, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "NewRigidBody%zu", numJoints + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelJointSetName(m_mutableJoint, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    model::Joint *newJoint = model::Joint::create();
    model::RigidBody::Resolver resolver;
    nanoem_model_joint_t *jointPtr = nanoemMutableModelJointGetOriginObject(m_mutableJoint);
    newJoint->bind(jointPtr, nullptr, resolver);
    newJoint->resetLanguage(jointPtr, factory, project->castLanguage());
}

CreateJointCommand::~CreateJointCommand() NANOEM_DECL_NOEXCEPT
{
    m_activeModel = nullptr;
}

void
CreateJointCommand::undo(Error &error)
{
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveJointObject(model, m_mutableJoint, &status);
    assignError(status, error);
}

void
CreateJointCommand::redo(Error &error)
{
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelInsertJointObject(model, m_mutableJoint, m_offset, &status);
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
DeleteJointCommand::create(Project *project, nanoem_rsize_t jointIndex)
{
    DeleteJointCommand *command = nanoem_new(DeleteJointCommand(project, jointIndex));
    return command->createCommand();
}

DeleteJointCommand::DeleteJointCommand(
    Project *project, nanoem_rsize_t jointIndex)
    : BaseUndoCommand(project)
    , m_jointIndex(jointIndex)
    , m_activeModel(project->activeModel())
    , m_mutableJoint(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t numJoints;
    nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(m_activeModel->data(), &numJoints);
    m_mutableJoint = nanoemMutableModelJointCreateAsReference(joints[m_jointIndex], &status);
}

DeleteJointCommand::~DeleteJointCommand() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelJointDestroy(m_mutableJoint);
    m_mutableJoint = nullptr;
    m_activeModel = nullptr;
}

void
DeleteJointCommand::undo(Error &error)
{
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelInsertJointObject(model, m_mutableJoint, Inline::saturateInt32(m_jointIndex), &status);
    assignError(status, error);
}

void
DeleteJointCommand::redo(Error &error)
{
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveJointObject(model, m_mutableJoint, &status);
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
    Project *project, nanoem_rsize_t fromJointIndex, nanoem_rsize_t toJointIndex)
    : BaseUndoCommand(project)
    , m_fromJointIndex(fromJointIndex)
    , m_toJointIndex(toJointIndex)
    , m_activeModel(project->activeModel())
{
}

void
BaseMoveJointCommand::move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error)
{
    nanoem_rsize_t numJoints;
    nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(m_activeModel->data(), &numJoints);
    ScopedMutableJoint joint(joints[fromIndex]);
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveJointObject(model, joint, &status);
    int index = Inline::saturateInt32(glm::min(toIndex, numJoints));
    nanoemMutableModelInsertJointObject(model, joint, index, &status);
    assignError(status, error);
}

undo_command_t *
MoveJointUpCommand::create(Project *project, nanoem_rsize_t jointIndex)
{
    MoveJointUpCommand *command = nanoem_new(MoveJointUpCommand(project, jointIndex));
    return command->createCommand();
}

MoveJointUpCommand::MoveJointUpCommand(
    Project *project, nanoem_rsize_t jointIndex)
    : BaseMoveJointCommand(project, jointIndex, jointIndex > 0 ? jointIndex - 1 : 0)
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
MoveJointDownCommand::create(Project *project, nanoem_rsize_t jointIndex)
{
    MoveJointDownCommand *command = nanoem_new(MoveJointDownCommand(project, jointIndex));
    return command->createCommand();
}

MoveJointDownCommand::MoveJointDownCommand(
    Project *project, nanoem_rsize_t jointIndex)
    : BaseMoveJointCommand(project, jointIndex, jointIndex + 1)
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
CreateSoftBodyCommand::create(
    Project *project, nanoem_rsize_t numSoftBodies, int offset, const nanoem_model_soft_body_t *base)
{
    CreateSoftBodyCommand *command = nanoem_new(CreateSoftBodyCommand(project, numSoftBodies, offset, base));
    return command->createCommand();
}

CreateSoftBodyCommand::CreateSoftBodyCommand(
    Project *project, nanoem_rsize_t numSoftBodies, int offset, const nanoem_model_soft_body_t *base)
    : BaseUndoCommand(project)
    , m_base(base)
    , m_offset(offset)
    , m_activeModel(project->activeModel())
    , m_mutableSoftBody(m_activeModel)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    char buffer[Inline::kMarkerStringLength];
    if (m_base) {
        nanoemMutableModelSoftBodyCopy(m_mutableSoftBody, m_base, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "%s%zu", kNewObjectPrefixName, numSoftBodies + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelSoftBodySetName(m_mutableSoftBody, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    StringUtils::format(buffer, sizeof(buffer), "NewRigidBody%zu", numSoftBodies + 1);
    if (StringUtils::tryGetString(factory, buffer, scope)) {
        nanoemMutableModelSoftBodySetName(m_mutableSoftBody, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    model::SoftBody *newSoftBody = model::SoftBody::create();
    model::RigidBody::Resolver resolver;
    nanoem_model_soft_body_t *softBodyPtr = nanoemMutableModelSoftBodyGetOriginObject(m_mutableSoftBody);
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
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveSoftBodyObject(model, m_mutableSoftBody, &status);
    assignError(status, error);
}

void
CreateSoftBodyCommand::redo(Error &error)
{
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelInsertSoftBodyObject(model, m_mutableSoftBody, m_offset, &status);
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
DeleteSoftBodyCommand::create(Project *project, nanoem_rsize_t softBodyIndex)
{
    DeleteSoftBodyCommand *command = nanoem_new(DeleteSoftBodyCommand(project, softBodyIndex));
    return command->createCommand();
}

DeleteSoftBodyCommand::DeleteSoftBodyCommand(Project *project, nanoem_rsize_t softBodyIndex)
    : BaseUndoCommand(project)
    , m_softBodyIndex(softBodyIndex)
    , m_activeModel(project->activeModel())
    , m_mutableSoftBody(nullptr)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t numSoftBodies;
    nanoem_model_soft_body_t *const *softBodies =
        nanoemModelGetAllSoftBodyObjects(m_activeModel->data(), &numSoftBodies);
    m_mutableSoftBody = nanoemMutableModelSoftBodyCreateAsReference(softBodies[m_softBodyIndex], &status);
}

DeleteSoftBodyCommand::~DeleteSoftBodyCommand() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelSoftBodyDestroy(m_mutableSoftBody);
    m_mutableSoftBody = nullptr;
    m_activeModel = nullptr;
}

void
DeleteSoftBodyCommand::undo(Error &error)
{
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelInsertSoftBodyObject(model, m_mutableSoftBody, Inline::saturateInt32(m_softBodyIndex), &status);
    assignError(status, error);
}

void
DeleteSoftBodyCommand::redo(Error &error)
{
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveSoftBodyObject(model, m_mutableSoftBody, &status);
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
    Project *project, nanoem_rsize_t fromSoftBodyIndex, nanoem_rsize_t toSoftBodyIndex)
    : BaseUndoCommand(project)
    , m_fromSoftBodyIndex(fromSoftBodyIndex)
    , m_toSoftBodyIndex(toSoftBodyIndex)
    , m_activeModel(project->activeModel())
{
}

void
BaseMoveSoftBodyCommand::move(nanoem_rsize_t fromIndex, nanoem_rsize_t toIndex, Error &error)
{
    nanoem_rsize_t numSoftBodies;
    nanoem_model_soft_body_t *const *softBodies =
        nanoemModelGetAllSoftBodyObjects(m_activeModel->data(), &numSoftBodies);
    ScopedMutableSoftBody softBody(softBodies[fromIndex]);
    ScopedMutableModel model(m_activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveSoftBodyObject(model, softBody, &status);
    int index = Inline::saturateInt32(glm::min(toIndex, numSoftBodies));
    nanoemMutableModelInsertSoftBodyObject(model, softBody, index, &status);
    assignError(status, error);
}

undo_command_t *
MoveSoftBodyUpCommand::create(
    Project *project, nanoem_rsize_t softBodyIndex)
{
    MoveSoftBodyUpCommand *command = nanoem_new(MoveSoftBodyUpCommand(project, softBodyIndex));
    return command->createCommand();
}

MoveSoftBodyUpCommand::MoveSoftBodyUpCommand(
    Project *project, nanoem_rsize_t softBodyIndex)
    : BaseMoveSoftBodyCommand(project, softBodyIndex, softBodyIndex > 0 ? softBodyIndex - 1 : 0)
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
MoveSoftBodyDownCommand::create(
    Project *project, nanoem_rsize_t softBodyIndex)
{
    MoveSoftBodyDownCommand *command = nanoem_new(MoveSoftBodyDownCommand(project, softBodyIndex));
    return command->createCommand();
}

MoveSoftBodyDownCommand::MoveSoftBodyDownCommand(
    Project *project, nanoem_rsize_t softBodyIndex)
    : BaseMoveSoftBodyCommand(project, softBodyIndex, softBodyIndex + 1)
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

} /* namespace command */
} /* namespace nanoem */
