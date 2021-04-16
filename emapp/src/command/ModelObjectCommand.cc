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

DeleteMaterialCommand::DeleteMaterialCommand(nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex)
    : m_materials(materials)
    , m_materialIndex(materialIndex)
{
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

void
DeleteMaterialCommand::execute(Project *project)
{
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
    Error error;
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

BaseMoveMaterialCommand::BaseMoveMaterialCommand(
    nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex)
    : m_materials(materials)
    , m_materialIndex(materialIndex)
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

MoveMaterialUpCommand::MoveMaterialUpCommand(nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex)
    : BaseMoveMaterialCommand(materials, materialIndex)
{
}

void
MoveMaterialUpCommand::execute(Project *project)
{
    const nanoem_model_material_t *activeMaterial = m_materials[m_materialIndex];
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

MoveMaterialDownCommand::MoveMaterialDownCommand(
    nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex)
    : BaseMoveMaterialCommand(materials, materialIndex)
{
}

void
MoveMaterialDownCommand::execute(Project *project)
{
    const nanoem_model_material_t *activeMaterial = m_materials[m_materialIndex];
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

CreateBoneCommand::CreateBoneCommand(nanoem_rsize_t numBones, int offset, const nanoem_model_bone_t *base)
    : m_base(base)
    , m_numBones(numBones)
    , m_offset(offset)
{
}

void
CreateBoneCommand::execute(Project *project)
{
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
    activeModel->addBone(bonePtr);
}

DeleteBoneCommand::DeleteBoneCommand(nanoem_model_bone_t *const *bones, nanoem_rsize_t &boneIndex)
    : m_bones(bones)
    , m_boneIndex(boneIndex)
{
}

void
DeleteBoneCommand::execute(Project *project)
{
    nanoem_model_bone_t *bonePtr = m_bones[m_boneIndex];
    Model *activeModel = project->activeModel();
    ScopedMutableBone bone(bonePtr);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    if (activeModel->activeBone() == bonePtr) {
        activeModel->setActiveBone(nullptr);
    }
    activeModel->removeBone(bonePtr);
    nanoemMutableModelRemoveBoneObject(model, bone, &status);
    project->rebuildAllTracks();
    if (m_boneIndex > 0) {
        m_boneIndex--;
    }
}

MoveBoneDownCommand::MoveBoneDownCommand(nanoem_model_bone_t *const *bones, nanoem_rsize_t &boneIndex)
    : m_bones(bones)
    , m_boneIndex(boneIndex)
{
}

void
MoveBoneDownCommand::execute(Project *project)
{
    Model *activeModel = project->activeModel();
    ScopedMutableBone bone(m_bones[m_boneIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveBoneObject(model, bone, &status);
    int offset = Inline::saturateInt32(++m_boneIndex);
    nanoemMutableModelInsertBoneObject(model, bone, offset, &status);
}

MoveBoneUpCommand::MoveBoneUpCommand(nanoem_model_bone_t *const *bones, nanoem_rsize_t &boneIndex)
    : m_bones(bones)
    , m_boneIndex(boneIndex)
{
}

void
MoveBoneUpCommand::execute(Project *project)
{
    Model *activeModel = project->activeModel();
    ScopedMutableBone bone(m_bones[m_boneIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveBoneObject(model, bone, &status);
    int offset = Inline::saturateInt32(--m_boneIndex);
    nanoemMutableModelInsertBoneObject(model, bone, offset, &status);
}

CreateMorphCommand::CreateMorphCommand(
    nanoem_rsize_t numMorphs, int offset, const nanoem_model_morph_t *base, nanoem_model_morph_type_t type)
    : m_base(base)
    , m_type(type)
    , m_numMorphs(numMorphs)
    , m_offset(offset)
{
}

void
CreateMorphCommand::execute(Project *project)
{
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

DeleteMorphCommand::DeleteMorphCommand(nanoem_model_morph_t *const *morphs, nanoem_rsize_t &morphIndex)
    : m_morphs(morphs)
    , m_morphIndex(morphIndex)
{
}

void
DeleteMorphCommand::execute(Project *project)
{
    nanoem_model_morph_t *morphPtr = m_morphs[m_morphIndex];
    Model *activeModel = project->activeModel();
    ScopedMutableMorph morph(morphPtr);
    ScopedMutableModel model(activeModel);
    for (int i = NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM; i < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM; i++) {
        nanoem_model_morph_category_t category = static_cast<nanoem_model_morph_category_t>(i);
        if (activeModel->activeMorph(category) == morphPtr) {
            activeModel->setActiveMorph(category, nullptr);
        }
    }
    activeModel->removeMorph(morphPtr);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveMorphObject(model, morph, &status);
    project->rebuildAllTracks();
    if (m_morphIndex > 0) {
        m_morphIndex--;
    }
}

MoveMorphUpCommand::MoveMorphUpCommand(nanoem_model_morph_t *const *morphs, nanoem_rsize_t &morphIndex)
    : m_morphs(morphs)
    , m_morphIndex(morphIndex)
{
}

void
MoveMorphUpCommand::execute(Project *project)
{
    Model *activeModel = project->activeModel();
    ScopedMutableMorph morph(m_morphs[m_morphIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveMorphObject(model, morph, &status);
    int offset = Inline::saturateInt32(--m_morphIndex);
    nanoemMutableModelInsertMorphObject(model, morph, offset, &status);
}

MoveMorphDownCommand::MoveMorphDownCommand(nanoem_model_morph_t *const *morphs, nanoem_rsize_t &morphIndex)
    : m_morphs(morphs)
    , m_morphIndex(morphIndex)
{
}

void
MoveMorphDownCommand::execute(Project *project)
{
    Model *activeModel = project->activeModel();
    ScopedMutableMorph morph(m_morphs[m_morphIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveMorphObject(model, morph, &status);
    int offset = Inline::saturateInt32(--m_morphIndex);
    nanoemMutableModelInsertMorphObject(model, morph, offset, &status);
}

CreateLabelCommand::CreateLabelCommand(nanoem_rsize_t numLabels, int offset, const nanoem_model_label_t *base)
    : m_base(base)
    , m_numLabels(numLabels)
    , m_offset(offset)
{
}

void
CreateLabelCommand::execute(Project *project)
{
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

DeleteLabelCommand::DeleteLabelCommand(nanoem_model_label_t *const *labels, nanoem_rsize_t &labelIndex)
    : m_labels(labels)
    , m_labelIndex(labelIndex)
{
}

void
DeleteLabelCommand::execute(Project *project)
{
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

MoveLabelUpCommand::MoveLabelUpCommand(nanoem_model_label_t *const *labels, nanoem_rsize_t &labelIndex)
    : m_labels(labels)
    , m_labelIndex(labelIndex)
{
}

void
MoveLabelUpCommand::execute(Project *project)
{
    Model *activeModel = project->activeModel();
    ScopedMutableLabel label(m_labels[m_labelIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveLabelObject(model, label, &status);
    int offset = Inline::saturateInt32(--m_labelIndex);
    nanoemMutableModelInsertLabelObject(model, label, offset, &status);
    project->rebuildAllTracks();
}

MoveLabelDownCommand::MoveLabelDownCommand(nanoem_model_label_t *const *labels, nanoem_rsize_t &labelIndex)
    : m_labels(labels)
    , m_labelIndex(labelIndex)
{
}

void
MoveLabelDownCommand::execute(Project *project)
{
    Model *activeModel = project->activeModel();
    ScopedMutableLabel label(m_labels[m_labelIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveLabelObject(model, label, &status);
    int offset = Inline::saturateInt32(++m_labelIndex);
    nanoemMutableModelInsertLabelObject(model, label, offset, &status);
    project->rebuildAllTracks();
}

CreateRigidBodyCommand::CreateRigidBodyCommand(
    nanoem_rsize_t numRigidBodies, int offset, const nanoem_model_rigid_body_t *base)
    : m_base(base)
    , m_numRigidBodies(numRigidBodies)
    , m_offset(offset)
{
}

void
CreateRigidBodyCommand::execute(Project *project)
{
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

DeleteRigidBodyCommand::DeleteRigidBodyCommand(
    nanoem_model_rigid_body_t *const *rigidBodies, nanoem_rsize_t &rigidBodyIndex)
    : m_rigidBodies(rigidBodies)
    , m_rigidBodyIndex(rigidBodyIndex)
{
}

void
DeleteRigidBodyCommand::execute(Project *project)
{
    Model *activeModel = project->activeModel();
    ScopedMutableRigidBody rigid_body(m_rigidBodies[m_rigidBodyIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveRigidBodyObject(model, rigid_body, &status);
    if (m_rigidBodyIndex > 0) {
        m_rigidBodyIndex--;
    }
}

MoveRigidBodyUpCommand::MoveRigidBodyUpCommand(
    nanoem_model_rigid_body_t *const *rigidBodies, nanoem_rsize_t &rigidBodyIndex)
    : m_rigidBodies(rigidBodies)
    , m_rigidBodyIndex(rigidBodyIndex)
{
}

void
MoveRigidBodyUpCommand::execute(Project *project)
{
    Model *activeModel = project->activeModel();
    ScopedMutableRigidBody rigid_body(m_rigidBodies[m_rigidBodyIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveRigidBodyObject(model, rigid_body, &status);
    int offset = Inline::saturateInt32(--m_rigidBodyIndex);
    nanoemMutableModelInsertRigidBodyObject(model, rigid_body, offset, &status);
}

MoveRigidBodyDownCommand::MoveRigidBodyDownCommand(
    nanoem_model_rigid_body_t *const *rigidBodies, nanoem_rsize_t &rigidBodyIndex)
    : m_rigidBodies(rigidBodies)
    , m_rigidBodyIndex(rigidBodyIndex)
{
}

void
MoveRigidBodyDownCommand::execute(Project *project)
{
    Model *activeModel = project->activeModel();
    ScopedMutableRigidBody rigid_body(m_rigidBodies[m_rigidBodyIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveRigidBodyObject(model, rigid_body, &status);
    int offset = Inline::saturateInt32(++m_rigidBodyIndex);
    nanoemMutableModelInsertRigidBodyObject(model, rigid_body, offset, &status);
}

CreateJointCommand::CreateJointCommand(nanoem_rsize_t numJoints, int offset, const nanoem_model_joint_t *base)
    : m_base(base)
    , m_numJoints(numJoints)
    , m_offset(offset)
{
}

void
CreateJointCommand::execute(Project *project)
{
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

DeleteJointCommand::DeleteJointCommand(nanoem_model_joint_t *const *joints, nanoem_rsize_t &jointIndex)
    : m_joints(joints)
    , m_jointIndex(jointIndex)
{
}

void
DeleteJointCommand::execute(Project *project)
{
    Model *activeModel = project->activeModel();
    ScopedMutableJoint joint(m_joints[m_jointIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveJointObject(model, joint, &status);
    if (m_jointIndex > 0) {
        m_jointIndex--;
    }
}

MoveJointUpCommand::MoveJointUpCommand(nanoem_model_joint_t *const *joints, nanoem_rsize_t &jointIndex)
    : m_joints(joints)
    , m_jointIndex(jointIndex)
{
}

void
MoveJointUpCommand::execute(Project *project)
{
    Model *activeModel = project->activeModel();
    ScopedMutableJoint joint(m_joints[m_jointIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveJointObject(model, joint, &status);
    int offset = Inline::saturateInt32(--m_jointIndex);
    nanoemMutableModelInsertJointObject(model, joint, offset, &status);
}

MoveJointDownCommand::MoveJointDownCommand(nanoem_model_joint_t *const *joints, nanoem_rsize_t &jointIndex)
    : m_joints(joints)
    , m_jointIndex(jointIndex)
{
}

void
MoveJointDownCommand::execute(Project *project)
{
    Model *activeModel = project->activeModel();
    ScopedMutableJoint joint(m_joints[m_jointIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveJointObject(model, joint, &status);
    int offset = Inline::saturateInt32(++m_jointIndex);
    nanoemMutableModelInsertJointObject(model, joint, offset, &status);
}

CreateSoftBodyCommand::CreateSoftBodyCommand(
    nanoem_rsize_t numSoftBodys, int offset, const nanoem_model_soft_body_t *base)
    : m_base(base)
    , m_numSoftBodys(numSoftBodys)
    , m_offset(offset)
{
}

void
CreateSoftBodyCommand::execute(Project *project)
{
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

DeleteSoftBodyCommand::DeleteSoftBodyCommand(
    nanoem_model_soft_body_t *const *soft_bodys, nanoem_rsize_t &soft_bodyIndex)
    : m_softBodies(soft_bodys)
    , m_softBodyIndex(soft_bodyIndex)
{
}

void
DeleteSoftBodyCommand::execute(Project *project)
{
    Model *activeModel = project->activeModel();
    ScopedMutableSoftBody soft_body(m_softBodies[m_softBodyIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveSoftBodyObject(model, soft_body, &status);
    if (m_softBodyIndex > 0) {
        m_softBodyIndex--;
    }
}

MoveSoftBodyUpCommand::MoveSoftBodyUpCommand(
    nanoem_model_soft_body_t *const *soft_bodys, nanoem_rsize_t &soft_bodyIndex)
    : m_softBodies(soft_bodys)
    , m_softBodyIndex(soft_bodyIndex)
{
}

void
MoveSoftBodyUpCommand::execute(Project *project)
{
    Model *activeModel = project->activeModel();
    ScopedMutableSoftBody soft_body(m_softBodies[m_softBodyIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveSoftBodyObject(model, soft_body, &status);
    int offset = Inline::saturateInt32(--m_softBodyIndex);
    nanoemMutableModelInsertSoftBodyObject(model, soft_body, offset, &status);
}

MoveSoftBodyDownCommand::MoveSoftBodyDownCommand(
    nanoem_model_soft_body_t *const *soft_bodys, nanoem_rsize_t &soft_bodyIndex)
    : m_softBodies(soft_bodys)
    , m_softBodyIndex(soft_bodyIndex)
{
}

void
MoveSoftBodyDownCommand::execute(Project *project)
{
    Model *activeModel = project->activeModel();
    ScopedMutableSoftBody softBody(m_softBodies[m_softBodyIndex]);
    ScopedMutableModel model(activeModel);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelRemoveSoftBodyObject(model, softBody, &status);
    int offset = Inline::saturateInt32(++m_softBodyIndex);
    nanoemMutableModelInsertSoftBodyObject(model, softBody, offset, &status);
}

} /* namespace command */
} /* namespace nanoem */
