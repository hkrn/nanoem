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

namespace nanoem {
namespace command {
namespace {

static const nanoem_u8_t kNewObjectPrefixName[] = { 0xe6, 0x96, 0xb0, 0xe8, 0xa6, 0x8f, 0 };

} /* namespace anonymous */

struct ModelObjectCommand : private NonCopyable {
    virtual ~ModelObjectCommand() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual void execute(Project *project) = 0;
};

struct DeleteMaterialCommand : ModelObjectCommand {
    DeleteMaterialCommand(nanoem_model_material_t *const *materials, nanoem_rsize_t materialIndex)
        : m_materials(materials)
        , m_materialIndex(materialIndex)
    {
    }
    void
    execute(Project *project)
    {
        Model *activeModel = project->activeModel();
        ScopedMutableMaterial material(m_materials[m_materialIndex], activeModel);
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
        const nanoem_u32_t *indices =
            nanoemModelGetAllVertexIndices(nanoemMutableModelGetOriginObject(model), &numIndices);
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
    nanoem_model_material_t *const *m_materials;
    nanoem_rsize_t m_materialIndex;
};

struct BaseMoveMaterialCommand : ModelObjectCommand {
    struct LayoutPosition {
        size_t m_offset;
        size_t m_size;
    };
    BaseMoveMaterialCommand(nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex)
        : m_materials(materials)
        , m_materialIndex(materialIndex)
    {
    }
    void
    move(int destination, const LayoutPosition &from, const LayoutPosition &to, Model *activeModel)
    {
        ScopedMutableMaterial material(m_materials[m_materialIndex], activeModel);
        ScopedMutableModel model(activeModel);
        nanoem_rsize_t numIndices;
        const nanoem_u32_t *indices =
            nanoemModelGetAllVertexIndices(nanoemMutableModelGetOriginObject(model), &numIndices);
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
    nanoem_model_material_t *const *m_materials;
    nanoem_rsize_t &m_materialIndex;
};

struct MoveMaterialUpCommand : BaseMoveMaterialCommand {
    MoveMaterialUpCommand(nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex)
        : BaseMoveMaterialCommand(materials, materialIndex)
    {
    }
    void
    execute(Project *project)
    {
        const nanoem_model_material_t *activeMaterial = m_materials[m_materialIndex];
        Model *activeModel = project->activeModel();
        nanoem_rsize_t numMaterials, offset = 0;
        nanoem_model_material_t *const *materials =
            nanoemModelGetAllMaterialObjects(activeModel->data(), &numMaterials);
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
};

struct MoveMaterialDownCommand : BaseMoveMaterialCommand {
    MoveMaterialDownCommand(nanoem_model_material_t *const *materials, nanoem_rsize_t &materialIndex)
        : BaseMoveMaterialCommand(materials, materialIndex)
    {
    }
    void
    execute(Project *project)
    {
        const nanoem_model_material_t *activeMaterial = m_materials[m_materialIndex];
        Model *activeModel = project->activeModel();
        nanoem_rsize_t numMaterials, offset = 0;
        nanoem_model_material_t *const *materials =
            nanoemModelGetAllMaterialObjects(activeModel->data(), &numMaterials);
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
};

struct CreateBoneCommand : ModelObjectCommand {
    CreateBoneCommand(nanoem_rsize_t numBones, int offset, const nanoem_model_bone_t *base)
        : m_base(base)
        , m_numBones(numBones)
        , m_offset(offset)
    {
    }
    void
    execute(Project *project)
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
    const nanoem_model_bone_t *m_base;
    const nanoem_rsize_t m_numBones;
    const int m_offset;
};

struct DeleteBoneCommand : ModelObjectCommand {
    DeleteBoneCommand(nanoem_model_bone_t *const *bones, nanoem_rsize_t &boneIndex)
        : m_bones(bones)
        , m_boneIndex(boneIndex)
    {
    }
    void
    execute(Project *project)
    {
        nanoem_model_bone_t *bonePtr = m_bones[m_boneIndex];
        Model *activeModel = project->activeModel();
        ScopedMutableBone bone(bonePtr, activeModel);
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
    nanoem_model_bone_t *const *m_bones;
    nanoem_rsize_t &m_boneIndex;
};

struct MoveBoneDownCommand : ModelObjectCommand {
    MoveBoneDownCommand(nanoem_model_bone_t *const *bones, nanoem_rsize_t &boneIndex)
        : m_bones(bones)
        , m_boneIndex(boneIndex)
    {
    }
    void
    execute(Project *project)
    {
        Model *activeModel = project->activeModel();
        ScopedMutableBone bone(m_bones[m_boneIndex], activeModel);
        ScopedMutableModel model(activeModel);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableModelRemoveBoneObject(model, bone, &status);
        int offset = Inline::saturateInt32(++m_boneIndex);
        nanoemMutableModelInsertBoneObject(model, bone, offset, &status);
    }
    nanoem_model_bone_t *const *m_bones;
    nanoem_rsize_t &m_boneIndex;
};

struct CreateMorphCommand : ModelObjectCommand {
    CreateMorphCommand(
        nanoem_rsize_t numMorphs, int offset, const nanoem_model_morph_t *base, nanoem_model_morph_type_t type)
        : m_base(base)
        , m_type(type)
        , m_numMorphs(numMorphs)
        , m_offset(offset)
    {
    }
    void
    execute(Project *project)
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
    const nanoem_model_morph_t *m_base;
    const nanoem_model_morph_type_t m_type;
    const nanoem_rsize_t m_numMorphs;
    const int m_offset;
};

struct DeleteMorphCommand : ModelObjectCommand {
    DeleteMorphCommand(nanoem_model_morph_t *const *morphs, nanoem_rsize_t &morphIndex)
        : m_morphs(morphs)
        , m_morphIndex(morphIndex)
    {
    }
    void
    execute(Project *project)
    {
        nanoem_model_morph_t *morphPtr = m_morphs[m_morphIndex];
        Model *activeModel = project->activeModel();
        ScopedMutableMorph morph(morphPtr, activeModel);
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
    nanoem_model_morph_t *const *m_morphs;
    nanoem_rsize_t &m_morphIndex;
};

struct MoveMorphUpCommand : ModelObjectCommand {
    MoveMorphUpCommand(nanoem_model_morph_t *const *morphs, nanoem_rsize_t &morphIndex)
        : m_morphs(morphs)
        , m_morphIndex(morphIndex)
    {
    }
    void
    execute(Project *project)
    {
        Model *activeModel = project->activeModel();
        ScopedMutableMorph morph(m_morphs[m_morphIndex], activeModel);
        ScopedMutableModel model(activeModel);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableModelRemoveMorphObject(model, morph, &status);
        int offset = Inline::saturateInt32(--m_morphIndex);
        nanoemMutableModelInsertMorphObject(model, morph, offset, &status);
    }
    nanoem_model_morph_t *const *m_morphs;
    nanoem_rsize_t &m_morphIndex;
};

struct MoveMorphDownCommand : ModelObjectCommand {
    MoveMorphDownCommand(nanoem_model_morph_t *const *morphs, nanoem_rsize_t &morphIndex)
        : m_morphs(morphs)
        , m_morphIndex(morphIndex)
    {
    }
    void
    execute(Project *project)
    {
        Model *activeModel = project->activeModel();
        ScopedMutableMorph morph(m_morphs[m_morphIndex], activeModel);
        ScopedMutableModel model(activeModel);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableModelRemoveMorphObject(model, morph, &status);
        int offset = Inline::saturateInt32(--m_morphIndex);
        nanoemMutableModelInsertMorphObject(model, morph, offset, &status);
    }
    nanoem_model_morph_t *const *m_morphs;
    nanoem_rsize_t &m_morphIndex;
};

struct CreateLabelCommand : ModelObjectCommand {
    CreateLabelCommand(nanoem_rsize_t numLabels, int offset, const nanoem_model_label_t *base)
        : m_base(base)
        , m_numLabels(numLabels)
        , m_offset(offset)
    {
    }
    void
    execute(Project *project)
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
    const nanoem_model_label_t *m_base;
    const nanoem_rsize_t m_numLabels;
    const int m_offset;
};

struct DeleteLabelCommand : ModelObjectCommand {
    DeleteLabelCommand(nanoem_model_label_t *const *labels, nanoem_rsize_t &labelIndex)
        : m_labels(labels)
        , m_labelIndex(labelIndex)
    {
    }
    void
    execute(Project *project)
    {
        Model *activeModel = project->activeModel();
        ScopedMutableLabel label(m_labels[m_labelIndex], activeModel);
        ScopedMutableModel model(activeModel);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableModelRemoveLabelObject(model, label, &status);
        project->rebuildAllTracks();
        if (m_labelIndex > 0) {
            m_labelIndex--;
        }
    }
    nanoem_model_label_t *const *m_labels;
    nanoem_rsize_t m_labelIndex;
};

struct MoveLabelUpCommand : ModelObjectCommand {
    MoveLabelUpCommand(nanoem_model_label_t *const *labels, nanoem_rsize_t &labelIndex)
        : m_labels(labels)
        , m_labelIndex(labelIndex)
    {
    }
    void
    execute(Project *project)
    {
        Model *activeModel = project->activeModel();
        ScopedMutableLabel label(m_labels[m_labelIndex], activeModel);
        ScopedMutableModel model(activeModel);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableModelRemoveLabelObject(model, label, &status);
        int offset = Inline::saturateInt32(--m_labelIndex);
        nanoemMutableModelInsertLabelObject(model, label, offset, &status);
        project->rebuildAllTracks();
    }
    nanoem_model_label_t *const *m_labels;
    nanoem_rsize_t &m_labelIndex;
};

struct MoveLabelDownCommand : ModelObjectCommand {
    MoveLabelDownCommand(nanoem_model_label_t *const *labels, nanoem_rsize_t &labelIndex)
        : m_labels(labels)
        , m_labelIndex(labelIndex)
    {
    }
    void
    execute(Project *project)
    {
        Model *activeModel = project->activeModel();
        ScopedMutableLabel label(m_labels[m_labelIndex], activeModel);
        ScopedMutableModel model(activeModel);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableModelRemoveLabelObject(model, label, &status);
        int offset = Inline::saturateInt32(++m_labelIndex);
        nanoemMutableModelInsertLabelObject(model, label, offset, &status);
        project->rebuildAllTracks();
    }
    nanoem_model_label_t *const *m_labels;
    nanoem_rsize_t &m_labelIndex;
};

struct CreateRigidBodyCommand : ModelObjectCommand {
    CreateRigidBodyCommand(nanoem_rsize_t numRigidBodies, int offset, const nanoem_model_rigid_body_t *base)
        : m_base(base)
        , m_numRigidBodies(numRigidBodies)
        , m_offset(offset)
    {
    }
    void
    execute(Project *project)
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
    const nanoem_model_rigid_body_t *m_base;
    const nanoem_rsize_t m_numRigidBodies;
    const int m_offset;
};

struct DeleteRigidBodyCommand : ModelObjectCommand {
    DeleteRigidBodyCommand(nanoem_model_rigid_body_t *const *rigidBodies, nanoem_rsize_t &rigidBodyIndex)
        : m_rigidBodies(rigidBodies)
        , m_rigidBodyIndex(rigidBodyIndex)
    {
    }
    void
    execute(Project *project)
    {
        Model *activeModel = project->activeModel();
        ScopedMutableRigidBody rigid_body(m_rigidBodies[m_rigidBodyIndex], activeModel);
        ScopedMutableModel model(activeModel);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableModelRemoveRigidBodyObject(model, rigid_body, &status);
        if (m_rigidBodyIndex > 0) {
            m_rigidBodyIndex--;
        }
    }
    nanoem_model_rigid_body_t *const *m_rigidBodies;
    nanoem_rsize_t &m_rigidBodyIndex;
};

struct MoveRigidBodyUpCommand : ModelObjectCommand {
    MoveRigidBodyUpCommand(nanoem_model_rigid_body_t *const *rigidBodies, nanoem_rsize_t &rigidBodyIndex)
        : m_rigidBodies(rigidBodies)
        , m_rigidBodyIndex(rigidBodyIndex)
    {
    }
    void
    execute(Project *project)
    {
        Model *activeModel = project->activeModel();
        ScopedMutableRigidBody rigid_body(m_rigidBodies[m_rigidBodyIndex], activeModel);
        ScopedMutableModel model(activeModel);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableModelRemoveRigidBodyObject(model, rigid_body, &status);
        int offset = Inline::saturateInt32(--m_rigidBodyIndex);
        nanoemMutableModelInsertRigidBodyObject(model, rigid_body, offset, &status);
    }
    nanoem_model_rigid_body_t *const *m_rigidBodies;
    nanoem_rsize_t &m_rigidBodyIndex;
};

struct MoveRigidBodyDownCommand : ModelObjectCommand {
    MoveRigidBodyDownCommand(nanoem_model_rigid_body_t *const *rigidBodies, nanoem_rsize_t &rigidBodyIndex)
        : m_rigidBodies(rigidBodies)
        , m_rigidBodyIndex(rigidBodyIndex)
    {
    }
    void
    execute(Project *project)
    {
        Model *activeModel = project->activeModel();
        ScopedMutableRigidBody rigid_body(m_rigidBodies[m_rigidBodyIndex], activeModel);
        ScopedMutableModel model(activeModel);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableModelRemoveRigidBodyObject(model, rigid_body, &status);
        int offset = Inline::saturateInt32(++m_rigidBodyIndex);
        nanoemMutableModelInsertRigidBodyObject(model, rigid_body, offset, &status);
    }
    nanoem_model_rigid_body_t *const *m_rigidBodies;
    nanoem_rsize_t &m_rigidBodyIndex;
};

struct CreateJointCommand : ModelObjectCommand {
    CreateJointCommand(nanoem_rsize_t numJoints, int offset, const nanoem_model_joint_t *base)
        : m_base(base)
        , m_numJoints(numJoints)
        , m_offset(offset)
    {
    }
    void
    execute(Project *project)
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
    const nanoem_model_joint_t *m_base;
    const nanoem_rsize_t m_numJoints;
    const int m_offset;
};

struct DeleteJointCommand : ModelObjectCommand {
    DeleteJointCommand(nanoem_model_joint_t *const *joints, nanoem_rsize_t &jointIndex)
        : m_joints(joints)
        , m_jointIndex(jointIndex)
    {
    }
    void
    execute(Project *project)
    {
        Model *activeModel = project->activeModel();
        ScopedMutableJoint joint(m_joints[m_jointIndex], activeModel);
        ScopedMutableModel model(activeModel);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableModelRemoveJointObject(model, joint, &status);
        if (m_jointIndex > 0) {
            m_jointIndex--;
        }
    }
    nanoem_model_joint_t *const *m_joints;
    nanoem_rsize_t &m_jointIndex;
};

struct MoveJointUpCommand : ModelObjectCommand {
    MoveJointUpCommand(nanoem_model_joint_t *const *joints, nanoem_rsize_t &jointIndex)
        : m_joints(joints)
        , m_jointIndex(jointIndex)
    {
    }
    void
    execute(Project *project)
    {
        Model *activeModel = project->activeModel();
        ScopedMutableJoint joint(m_joints[m_jointIndex], activeModel);
        ScopedMutableModel model(activeModel);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableModelRemoveJointObject(model, joint, &status);
        int offset = Inline::saturateInt32(--m_jointIndex);
        nanoemMutableModelInsertJointObject(model, joint, offset, &status);
    }
    nanoem_model_joint_t *const *m_joints;
    nanoem_rsize_t &m_jointIndex;
};

struct MoveJointDownCommand : ModelObjectCommand {
    MoveJointDownCommand(nanoem_model_joint_t *const *joints, nanoem_rsize_t &jointIndex)
        : m_joints(joints)
        , m_jointIndex(jointIndex)
    {
    }
    void
    execute(Project *project)
    {
        Model *activeModel = project->activeModel();
        ScopedMutableJoint joint(m_joints[m_jointIndex], activeModel);
        ScopedMutableModel model(activeModel);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableModelRemoveJointObject(model, joint, &status);
        int offset = Inline::saturateInt32(++m_jointIndex);
        nanoemMutableModelInsertJointObject(model, joint, offset, &status);
    }
    nanoem_model_joint_t *const *m_joints;
    nanoem_rsize_t &m_jointIndex;
};

struct CreateSoftBodyCommand : ModelObjectCommand {
    CreateSoftBodyCommand(nanoem_rsize_t numSoftBodys, int offset, const nanoem_model_soft_body_t *base)
        : m_base(base)
        , m_numSoftBodys(numSoftBodys)
        , m_offset(offset)
    {
    }
    void
    execute(Project *project)
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
        newSoftBody->resetLanguage(
            nanoemMutableModelSoftBodyGetOriginObject(soft_body), factory, project->castLanguage());
    }
    const nanoem_model_soft_body_t *m_base;
    const nanoem_rsize_t m_numSoftBodys;
    const int m_offset;
};

struct DeleteSoftBodyCommand : ModelObjectCommand {
    DeleteSoftBodyCommand(nanoem_model_soft_body_t *const *soft_bodys, nanoem_rsize_t &soft_bodyIndex)
        : m_soft_bodys(soft_bodys)
        , m_softBodyIndex(soft_bodyIndex)
    {
    }
    void
    execute(Project *project)
    {
        Model *activeModel = project->activeModel();
        ScopedMutableSoftBody soft_body(m_soft_bodys[m_softBodyIndex], activeModel);
        ScopedMutableModel model(activeModel);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableModelRemoveSoftBodyObject(model, soft_body, &status);
        if (m_softBodyIndex > 0) {
            m_softBodyIndex--;
        }
    }
    nanoem_model_soft_body_t *const *m_soft_bodys;
    nanoem_rsize_t &m_softBodyIndex;
};

struct MoveSoftBodyUpCommand : ModelObjectCommand {
    MoveSoftBodyUpCommand(nanoem_model_soft_body_t *const *soft_bodys, nanoem_rsize_t &soft_bodyIndex)
        : m_soft_bodys(soft_bodys)
        , m_softBodyIndex(soft_bodyIndex)
    {
    }
    void
    execute(Project *project)
    {
        Model *activeModel = project->activeModel();
        ScopedMutableSoftBody soft_body(m_soft_bodys[m_softBodyIndex], activeModel);
        ScopedMutableModel model(activeModel);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableModelRemoveSoftBodyObject(model, soft_body, &status);
        int offset = Inline::saturateInt32(--m_softBodyIndex);
        nanoemMutableModelInsertSoftBodyObject(model, soft_body, offset, &status);
    }
    nanoem_model_soft_body_t *const *m_soft_bodys;
    nanoem_rsize_t &m_softBodyIndex;
};

struct MoveSoftBodyDownCommand : ModelObjectCommand {
    MoveSoftBodyDownCommand(nanoem_model_soft_body_t *const *soft_bodys, nanoem_rsize_t &soft_bodyIndex)
        : m_soft_bodys(soft_bodys)
        , m_softBodyIndex(soft_bodyIndex)
    {
    }
    void
    execute(Project *project)
    {
        Model *activeModel = project->activeModel();
        ScopedMutableSoftBody soft_body(m_soft_bodys[m_softBodyIndex], activeModel);
        ScopedMutableModel model(activeModel);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableModelRemoveSoftBodyObject(model, soft_body, &status);
        int offset = Inline::saturateInt32(++m_softBodyIndex);
        nanoemMutableModelInsertSoftBodyObject(model, soft_body, offset, &status);
    }
    nanoem_model_soft_body_t *const *m_soft_bodys;
    nanoem_rsize_t &m_softBodyIndex;
};

struct MoveBoneUpCommand : ModelObjectCommand {
    MoveBoneUpCommand(nanoem_model_bone_t *const *bones, nanoem_rsize_t &boneIndex)
        : m_bones(bones)
        , m_boneIndex(boneIndex)
    {
    }
    void
    execute(Project *project)
    {
        Model *activeModel = project->activeModel();
        ScopedMutableBone bone(m_bones[m_boneIndex], activeModel);
        ScopedMutableModel model(activeModel);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableModelRemoveBoneObject(model, bone, &status);
        int offset = Inline::saturateInt32(--m_boneIndex);
        nanoemMutableModelInsertBoneObject(model, bone, offset, &status);
    }
    nanoem_model_bone_t *const *m_bones;
    nanoem_rsize_t &m_boneIndex;
};

} /* namespace command */
} /* namespace nanoem */
